#ifndef PBQPGRAPH_H_
#define PBQPGRAPH_H_

#include <cassert>
#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <ranges>
#include <unordered_set>
#include <vector>

#include "graph/PBQPEdge.hpp"
#include "graph/PBQPNode.hpp"

namespace pbqppapa {

template <typename T>
class PBQPEdge;
template <typename T>
class PBQPSolution;
template <typename T>
class PBQPNode;
template <typename T>
class Matrix;
template <typename T>
class Vector;

/**
 * Hash and equality that give a set of std::unique_ptr<U> pointer-identity semantics
 * (matching the old std::set<U*> behavior) while enabling heterogeneous lookup by a
 * raw U* via the transparent (is_transparent) overloads.
 */
template <typename U>
struct OwningPtrHash {
	using is_transparent = void;
	[[nodiscard]] std::size_t operator()(const std::unique_ptr<U>& ptr) const noexcept {
		return std::hash<const U*>()(ptr.get());
	}
	[[nodiscard]] std::size_t operator()(const U* ptr) const noexcept {
		return std::hash<const U*>()(ptr);
	}
};

template <typename U>
struct OwningPtrEqual {
	using is_transparent = void;
	[[nodiscard]] bool operator()(const std::unique_ptr<U>& a, const std::unique_ptr<U>& b) const noexcept {
		return a.get() == b.get();
	}
	[[nodiscard]] bool operator()(const std::unique_ptr<U>& a, const U* b) const noexcept {
		return a.get() == b;
	}
	[[nodiscard]] bool operator()(const U* a, const std::unique_ptr<U>& b) const noexcept {
		return a == b.get();
	}
	[[nodiscard]] bool operator()(const U* a, const U* b) const noexcept {
		return a == b;
	}
};

/**
 * A graph representing a PBQP. The template type represents the data type of the numbers in the cost vectors
 * and cost matrices. It is consistent throughout the entire graph; all edges and all nodes.
 * The graph owns its nodes and edges through std::unique_ptr. Removing a node (with cleanUp) does not
 * destroy it immediately: it is moved into deletedNodeSet so back-substitution can reactivate it, and it is
 * destroyed together with the graph.
 *
 * Iterate the nodes with `for (auto* node : graph->nodes())` (or directly `for (auto* node : *graph)`) and the
 * edges with `for (auto* edge : graph->edges())`. Both yield raw pointers.
 *
 * You should never manually delete nodes or edges, leave that entirely to a graph instance
 */
template <typename T>
class PBQPGraph {
private:
	using NodeSet =
			std::unordered_set<std::unique_ptr<PBQPNode<T>>, OwningPtrHash<PBQPNode<T>>, OwningPtrEqual<PBQPNode<T>>>;
	using EdgeSet =
			std::unordered_set<std::unique_ptr<PBQPEdge<T>>, OwningPtrHash<PBQPEdge<T>>, OwningPtrEqual<PBQPEdge<T>>>;

	// Unwrap an owning unique_ptr into the raw pointer the public API iterates over.
	static PBQPNode<T>* nodeToRaw(const std::unique_ptr<PBQPNode<T>>& ptr) {
		return ptr.get();
	}
	static PBQPEdge<T>* edgeToRaw(const std::unique_ptr<PBQPEdge<T>>& ptr) {
		return ptr.get();
	}

	using NodeView = std::ranges::transform_view<std::ranges::ref_view<NodeSet>,
			PBQPNode<T>* (*)(const std::unique_ptr<PBQPNode<T>>&)>;
	using EdgeView = std::ranges::transform_view<std::ranges::ref_view<EdgeSet>,
			PBQPEdge<T>* (*)(const std::unique_ptr<PBQPEdge<T>>&)>;

	unsigned int indexMaximum = 0;
	NodeSet nodeSet;
	EdgeSet edgeSet;
	NodeSet deletedNodeSet;
	std::vector<PBQPNode<T>*> peo;
	// Views that present the owning sets as ranges of raw pointers. Kept as members so begin()/end()
	// hand out iterators into a stable view (transform_view iterators reference their parent view).
	NodeView nodeView{nodeSet | std::views::transform(&nodeToRaw)};
	EdgeView edgeView{edgeSet | std::views::transform(&edgeToRaw)};

public:
	/**
	 * Create a new empty graph with no nodes
	 */
	PBQPGraph() = default;

	/**
	 * The owning unique_ptr sets free all nodes and edges automatically
	 */
	~PBQPGraph() = default;

	// The graph owns its nodes/edges and holds views into them, so it must not be trivially copied.
	// Use the pointer constructor below for a deep copy.
	PBQPGraph(const PBQPGraph<T>&) = delete;
	PBQPGraph<T>& operator=(const PBQPGraph<T>&) = delete;

	/**
	 * Copy constructor deep copies all nodes, edges and PEO
	 */
	PBQPGraph(const PBQPGraph<T>* graph) : indexMaximum(graph->indexMaximum) {
		std::map<PBQPNode<T>*, PBQPNode<T>*> nodeReMapping;
		for (const std::unique_ptr<PBQPNode<T>>& nodePtr : graph->nodeSet) {
			PBQPNode<T>* oldNode = nodePtr.get();
			auto createdNode = std::make_unique<PBQPNode<T>>(oldNode);
			nodeReMapping.insert({oldNode, createdNode.get()});
			nodeSet.insert(std::move(createdNode));
		}
		for (const std::unique_ptr<PBQPEdge<T>>& edgePtr : graph->edgeSet) {
			PBQPEdge<T>* edge = edgePtr.get();
			PBQPNode<T>* newSource = nodeReMapping.find(edge->getSource())->second;
			PBQPNode<T>* newTarget = nodeReMapping.find(edge->getTarget())->second;
			auto createdEdge = std::make_unique<PBQPEdge<T>>(newSource, newTarget, edge);
			newSource->addEdge(createdEdge.get());
			newTarget->addEdge(createdEdge.get());
			edgeSet.insert(std::move(createdEdge));
		}
		for (PBQPNode<T>* oldNode : graph->peo) {
			peo.push_back(nodeReMapping.find(oldNode)->second);
		}
	}

	/**
	 * Completly resets the graph, removes all nodes, edges and peo and deletes them
	 */
	void clear() {
		edgeSet.clear();
		nodeSet.clear();
		deletedNodeSet.clear();
		peo.clear();
	}

	/**
	 * Creates a new node with the given cost Vector and adds it to the graph.
	 * The new node will not have any edges initially
	 */
	PBQPNode<T>* addNode(Vector<T>& vector) {
		auto node = std::make_unique<PBQPNode<T>>(indexMaximum++, vector);
		PBQPNode<T>* nodePtr = node.get();
		nodeSet.insert(std::move(node));
		return nodePtr;
	}

	/**
	 * Adopts a preexisting node into this graph, taking ownership of it. No checks are done on the internal
	 * state of the node or its possibly referenced edges. The user must ensure that this is handled properly
	 *
	 * A node this graph previously removed (and moved to deletedNodeSet) is reactivated and its ownership stays
	 * with this graph. A node that was released from another graph (via removeNode with cleanUp == false) is
	 * newly adopted. It is not okay to add a node that is still owned by another graph.
	 */
	void addNode(PBQPNode<T>* node) {
		assert(node);
		if (node->isDeleted()) {
			auto it = deletedNodeSet.find(node);
			assert(it != deletedNodeSet.end());
			auto handle = deletedNodeSet.extract(it);
			node->setDeleted(false);
			nodeSet.insert(std::move(handle));
		} else {
			nodeSet.insert(std::unique_ptr<PBQPNode<T>>(node));
		}
		if (node->getIndex() >= indexMaximum) {
			indexMaximum = node->getIndex() + 1;
		}
	}

	/**
	 * Adopts a preexisting edge into this graph, taking ownership of it. No checks are done on the internal
	 * state of the edge or whether the nodes incident to it are even in the graph. The user must ensure that
	 * this is handled properly. The edge must not still be owned by another graph.
	 */
	void addEdge(PBQPEdge<T>* edge) {
		assert(edge);
		edgeSet.insert(std::unique_ptr<PBQPEdge<T>>(edge));
	}

	/**
	 * Creates a new edge and inserts it into the graph. The first node given is the
	 * edges source, the second one its target and the given matrix is the cost matrix that
	 * will be associated with the created edge
	 *
	 * Note that PBQPGraph ensures to never have cycles or multiedges. This means if you try to add an
	 * edge which already exists, the cost of the matrix you gave will be added to the existing edge and
	 * the preexisting edge will be returned.
	 * If the source and target node are identical the diagonal of the given matrix will be added to this
	 * node and nullptr will be returned
	 */
	PBQPEdge<T>* addEdge(PBQPNode<T>* source, PBQPNode<T>* target, Matrix<T>& matrix) {
		assert(source);
		assert(target);
		assert(source->getVectorDegree() == matrix.getRowCount());
		assert(target->getVectorDegree() == matrix.getColumnCount());
		if (source == target) {
			for (unsigned short i = 0; i < source->getVectorDegree(); i++) {
				source->getVector().get(i) += matrix.get(i, i);
			}
			return nullptr;
		}
		// add to existing edge if one exists
		for (PBQPEdge<T>* edge : source->getAdjacentEdges(false)) {
			if (edge->getOtherEnd(source) == target) {
				if (edge->getSource() == source) {
					edge->getMatrix() += matrix;
				} else {
					edge->getMatrix() += matrix.transpose();
				}
				return edge;
			}
		}
		auto edge = std::make_unique<PBQPEdge<T>>(source, target, matrix);
		PBQPEdge<T>* edgePtr = edge.get();
		source->addEdge(edgePtr);
		target->addEdge(edgePtr);
		edgeSet.insert(std::move(edge));
		return edgePtr;
	}

	/**
	 * Removes the given node and all edges connected to it from the graph
	 *
	 * If the cleanUp flag is set, the node is moved into deletedNodeSet (still owned by the graph) and its
	 * edges are destroyed. If not, the node and its edges are released from this graph's ownership and left
	 * alone otherwise, ready to be adopted by another graph.
	 *
	 * This means that if cleanUp is not set, nodes within the graph may be connected
	 * to nodes that are not in the graph anymore officially, but still known through
	 * the edge references of the nodes. Ensuring consistency and something that makes sense
	 * is up to the user when setting cleanUp to false
	 */
	void removeNode(PBQPNode<T>* node, bool cleanUp = true) {
		for (PBQPEdge<T>* edge : std::vector<PBQPEdge<T>*>(node->getAdjacentEdges(false))) {
			auto edgeIter = edgeSet.find(edge);
			if (cleanUp) {
				edge->getOtherEnd(node)->removeEdge(edge);
				node->removeEdge(edge);
				if (edgeIter != edgeSet.end()) {
					edgeSet.erase(edgeIter); // destroys the edge
				}
			} else if (edgeIter != edgeSet.end()) {
				edgeSet.extract(edgeIter).value().release(); // release ownership, edge survives
			}
		}
		auto nodeIter = nodeSet.find(node);
		if (cleanUp) {
			if (nodeIter != nodeSet.end()) {
				auto handle = nodeSet.extract(nodeIter);
				node->setDeleted(true);
				deletedNodeSet.insert(std::move(handle));
			}
		} else if (nodeIter != nodeSet.end()) {
			nodeSet.extract(nodeIter).value().release(); // release ownership, node survives
		}
	}

	/**
	 * Removes the given edge from the graph and deletes it.
	 * The edge is deleted from adjacent nodes as well, but the adjacent nodes stay in the graph
	 */
	void removeEdge(PBQPEdge<T>* edge) {
		edge->getSource()->removeEdge(edge);
		edge->getTarget()->removeEdge(edge);
		auto edgeIter = edgeSet.find(edge);
		if (edgeIter != edgeSet.end()) {
			edgeSet.erase(edgeIter); // destroys the edge
		}
	}

	/**
	 * A range over the graph's nodes, yielding raw PBQPNode<T>* handles
	 * (e.g. `for (auto* node : graph->nodes())`). The underlying storage owns the nodes; the view just
	 * unwraps the owning unique_ptr. An iterator stays valid as long as the element it points to is not
	 * removed, so the reduction pattern of advancing past a node before removing it remains safe.
	 */
	[[nodiscard]] NodeView nodes() const {
		return nodeView;
	}

	/**
	 * A range over the graph's edges, yielding raw PBQPEdge<T>* handles
	 * (e.g. `for (auto* edge : graph->edges())`).
	 */
	[[nodiscard]] EdgeView edges() const {
		return edgeView;
	}

	/**
	 * The graph itself is a range of its nodes, so `for (auto* node : *graph)` works too.
	 */
	[[nodiscard]] auto begin() const {
		return nodeView.begin();
	}

	[[nodiscard]] auto end() const {
		return nodeView.end();
	}

	/**
	 * Gets the amount of nodes currently in the graph
	 */
	[[nodiscard]] unsigned int getNodeCount() const {
		return nodeSet.size();
	}

	/**
	 * Gets the amount of edges currently in the graph
	 */
	[[nodiscard]] unsigned int getEdgeCount() const {
		return edgeSet.size();
	}

	/**
	 * Gets the internal counter for node indices. All nodes that ever existed in any graph so far
	 * will have an index smaller than this counter. To ensure that every node has a unique number, even
	 * if we split up graphs and parallelize work on the smaller pieces, this counter is global
	 */
	[[nodiscard]] unsigned int getNodeIndexCounter() const {
		return indexMaximum;
	}

	[[nodiscard]] std::vector<PBQPNode<T>*>& getPEO() {
		// this should return a const vector, but doesnt due because that'd make its
		// iterators const_iterator which introduced problems in other places and led to
		// code duplication
		return peo;
	}

	/**
	 * Replaces the current PEO with the given one
	 */
	void setPEO(std::vector<PBQPNode<T>*> newPeo) {
		peo = std::move(newPeo);
	}
};
} // namespace pbqppapa

#endif /* PBQPGRAPH_H_ */
