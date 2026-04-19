#ifndef PBQPNODE_H_
#define PBQPNODE_H_

#include <algorithm>
#include <vector>

#include "graph/Vector.hpp"

namespace pbqppapa {

template <typename T>
class PBQPEdge;
template <typename T>
class Vector;

template <typename T>
class PBQPNode {
private:
	Vector<T> values;
	std::vector<PBQPEdge<T>*> incidentEdges;
	std::vector<PBQPEdge<T>*> outgoingEdges;
	const unsigned int index;
	bool deleted;

public:
	/**
	 * Should only be used by PBQPGraph internally. Index counter is held by PBQPGraph instance
	 */
	PBQPNode(const unsigned int index, Vector<T>& values) : values(values), index(index), deleted(false) {}

	/**
	 * Copy constructor will not copy edges, that would not make sense
	 */
	PBQPNode(PBQPNode<T>* node) : values(node->values), index(node->index), deleted(false) {}

	/**
	 * Gets all incident edges, optionally only the outgoing ones
	 */
	[[nodiscard]] const std::vector<PBQPEdge<T>*>& getAdjacentEdges(const bool respectDirection = false) const {
		if (respectDirection) {
			return outgoingEdges;
		} else {
			return incidentEdges;
		}
	}

	/**
	 * Gets all adjacent nodes, optionally only the ones that are the target of the edge connecting them to this node
	 */
	[[nodiscard]] std::vector<PBQPNode<T>*> getAdjacentNodes(const bool respectDirection = false) {
		std::set<PBQPNode<T>*> resultSet;
		std::vector<PBQPNode<T>*> nodes;
		const std::vector<PBQPEdge<T>*>& edgesToLookAt = (respectDirection ? outgoingEdges : incidentEdges);
		for (PBQPEdge<T>* edge : edgesToLookAt) {
			PBQPNode<T>* other = edge->getOtherEnd(this);
			if (resultSet.insert(other).second) {
				nodes.push_back(other);
			}
		}
		return nodes;
	}

	/**
	 * Index identifies this node uniquely in the graph it was created in, even after the node is deleted
	 */
	[[nodiscard]] unsigned int getIndex() const {
		return index;
	}

	/**
	 * Gets the length (amount of rows) of the cost Vector
	 */
	[[nodiscard]] unsigned short getVectorDegree() const {
		return values.getRowCount();
	}

	/**
	 * Gets the amount of edges connected to this node
	 */
	[[nodiscard]] unsigned int getDegree() const {
		return incidentEdges.size();
	}

	/**
	 * Gets the cost Vector associated with this node
	 */
	[[nodiscard]] Vector<T>& getVector() {
		return values;
	}

	/**
	 * Compares based on index, needed so we can sort nodes into maps
	 */
	bool operator<(const PBQPNode<T>& e) const {
		return this->index < e.index;
	}

	/**
	 * Compares based on index,
	 */
	bool operator==(const PBQPNode<T>& e) const {
		return this->index == e.index;
	}

	/**
	 * Should only be used by PBQPGraph internally.
	 */
	void addEdge(PBQPEdge<T>* edge) {
		incidentEdges.push_back(edge);
		if (edge->getSource() == this) {
			outgoingEdges.push_back(edge);
		}
	}

	/**
	 * Should only be used by PBQPGraph internally.
	 */
	void removeEdge(PBQPEdge<T>* edge) {
		std::erase(incidentEdges, edge);
		if (edge->getSource() == this) {
			std::erase(outgoingEdges, edge);
		}
	}

	void setDeleted(bool state) {
		deleted = state;
	}

	[[nodiscard]] bool isDeleted() const {
		return deleted;
	}
};

} // namespace pbqppapa

#endif /* PBQPNODE_H_ */
