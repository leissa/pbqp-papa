#pragma once

#include <memory>
#include <vector>

#include "math/InfinityWrapper.hpp"
#include "reduction/PBQPReduction.hpp"
#include "reduction/solutions/ImmediateSolution.hpp"
#include "reduction/solutions/NtoNDependentSolution.hpp"

namespace pbqppapa {

template <typename T>
class PBQPGraph;
template <typename T>
class NtoNDependentSolution;
template <typename T>
class PBQPSolution;
template <typename T>
class PBQPNode;

template <typename T>
class DegreeZeroReducer: public PBQP_Reduction<T> {
private:
	std::unique_ptr<NtoNDependentSolution<T>> solution;
	// we need this a lot, so keeping it around instead of recreating is good
	static const std::vector<uint16_t> emptyIntVector;

public:
	DegreeZeroReducer(PBQPGraph<T>* graph) : PBQP_Reduction<T>(graph) {}

	std::vector<PBQPGraph<T>*>& reduce() override {
		std::vector<PBQPNode<T>*> targetNodes;
		std::vector<uint16_t> nodeSolution;
		auto iter = this->graph->begin();
		while (iter != this->graph->end()) {
			PBQPNode<T>* node = *iter;
			++iter;
			if (node->getDegree() == 0) {
				nodeSolution.push_back(node->getVector().getIndexOfSmallestElement());
				targetNodes.push_back(node);
				this->graph->removeNode(node);
			}
		}
		solution = std::make_unique<NtoNDependentSolution<T>>(std::vector<PBQPNode<T>*>(), targetNodes);
		solution->setSolution(emptyIntVector, nodeSolution);
		this->result.push_back(this->graph);
		return this->result;
	}

	void solve(PBQPSolution<T>& solution) override {
		this->solution->solve(&solution);
	}

	/**
	 * Reduces a given node of degree 0. Useful when combining reductions working
	 * on different degrees to save overhead of creating lots of reduction instances
	 */
	static NtoNDependentSolution<T>* reduceDegreeZero(PBQPNode<T>* node, PBQPGraph<T>* graph) {
		std::vector<PBQPNode<T>*> dependencyNodes;
		std::vector<PBQPNode<T>*> solutionNodes;
		solutionNodes.push_back(node);
		auto solution = std::make_unique<NtoNDependentSolution<T>>(dependencyNodes, solutionNodes);
		std::vector<uint16_t> nodeSolution;
		nodeSolution.push_back(node->getVector().getIndexOfSmallestElement());
		solution->setSolution(emptyIntVector, nodeSolution);
		graph->removeNode(node);
		return solution.release();
	}

	static ImmediateSolution<InfinityWrapper<T>>* reduceDegreeZeroInf(
			PBQPNode<InfinityWrapper<T>>* node, PBQPGraph<InfinityWrapper<T>>* graph) {
		uint16_t minimum = node->getVector().getIndexOfSmallestElement();
		if (node->getVector().get(minimum).isInfinite()) {
		}
		auto solution = std::make_unique<ImmediateSolution<InfinityWrapper<T>>>(node, minimum);
		graph->removeNode(node);
		return solution.release();
	}
};

// dark magic to initialize static members of a template
template <typename T>
const std::vector<uint16_t> DegreeZeroReducer<T>::emptyIntVector;
} // namespace pbqppapa
