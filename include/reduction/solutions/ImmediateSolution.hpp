#pragma once

#include <vector>

namespace pbqppapa {

template <typename T>
class PBQPSolution;
template <typename T>
class PBQPNode;
template <typename T>
class DependentSolution;

template <typename T>
class ImmediateSolution: public DependentSolution<T> {

private:
	uint16_t selection;
	PBQPNode<T>* node;
	std::vector<PBQPEdge<T>> edges;

public:
	ImmediateSolution(PBQPNode<T>* node, uint16_t selection) : selection(selection), node(node) {
		for (PBQPEdge<T>* edge : node->getAdjacentEdges()) {
			edges.push_back(*edge);
		}
	}

	~ImmediateSolution() = default;

	void solve(PBQPSolution<T>* solution) override {
		solution->setSolution(node->getIndex(), selection);
	}

	void revertChange(PBQPGraph<T>* graph) override {
		graph->addNode(node);
		for (PBQPEdge<T> edge : edges) {
			graph->addEdge(edge.getSource(), edge.getTarget(), edge.getMatrix());
		}
	}

	PBQPNode<T>* getReducedNode() override {
		return node;
	}
};
} // namespace pbqppapa
