#pragma once

#include <memory>
#include <vector>

#include "graph/PBQPGraph.hpp"
#include "graph/PBQPSolution.hpp"
#include "math/InfinityWrapper.hpp"
#include "reduction/solutions/NtoNDependentSolution.hpp"

namespace pbqppapa {

template <typename T>
class PBQPGraph;
template <typename T>
class PBQPSolution;
template <typename T>
class PBQPNode;
template <typename T>
class InfinityWrapper;

/**
 * Attempts to solve a PBQP instance through a branch and bound approach.
 * Note that this does not work for bigger graph or graphs with very little cost variation
 */
template <typename T>
class BranchBoundSolver {

private:
	PBQPGraph<InfinityWrapper<T>>* graph;
	std::vector<PBQPNode<InfinityWrapper<T>>*> nodes;

public:
	/**
	 * Creates a new instance to solve the given graph
	 */
	BranchBoundSolver(PBQPGraph<InfinityWrapper<T>>* graph) : graph(graph) {
		for (auto node : graph->nodes()) {
			nodes.push_back(node);
		}
	}

	~BranchBoundSolver() = default;

public:
	/**
	 * Attempts to find a solution. May not finish before the heat death of the universe if your graph is big
	 */
	[[nodiscard]] PBQPSolution<InfinityWrapper<T>>* solve() {
		InfinityWrapper<T> localCost = InfinityWrapper<T>(0);
		auto localSolution = std::make_unique<PBQPSolution<InfinityWrapper<T>>>(graph->getNodeIndexCounter());
		return recursiveSolve(localCost, localSolution.get(), 0);
	}

private:
	PBQPSolution<InfinityWrapper<T>>* recursiveSolve(
			InfinityWrapper<T> currentCost, PBQPSolution<InfinityWrapper<T>>* sol, size_t nodeCounter) {
		auto localSolution = std::make_unique<PBQPSolution<InfinityWrapper<T>>>(*sol);
		std::unique_ptr<PBQPSolution<InfinityWrapper<T>>> minSolution;
		PBQPNode<InfinityWrapper<T>>* node = nodes.at(nodeCounter);
		InfinityWrapper<T> localCost = InfinityWrapper<T>(0);
		InfinityWrapper<T> localMin = InfinityWrapper<T>::getInfinite();
		uint16_t minSelection = 0;
		for (uint16_t index = 0; index < node->getVectorDegree(); index++) {
			InfinityWrapper<T> value = node->getVector().get(index);
			if (value.isInfinite()) {
				continue;
			}
			bool allowedViaEdges = true;
			InfinityWrapper<T> edgeSum(0);
			for (auto edge : node->getAdjacentEdges()) {
				PBQPNode<InfinityWrapper<T>>* otherEnd = nodes.at(nodeCounter);
				if (sol->hasSolution(otherEnd->getIndex())) {
					InfinityWrapper<T> matrixValue;
					if (edge->getSource() == node) {
						matrixValue = edge->getMatrix().get(index, sol->getSolution(otherEnd));
					} else {
						matrixValue = edge->getMatrix().get(sol->getSolution(otherEnd), index);
					}
					if (matrixValue.isInfinite()) {
						allowedViaEdges = false;
						break;
					}
					edgeSum += matrixValue;
				}
			}
			if (!allowedViaEdges) {
				continue;
			}
			localSolution->setSolution(node->getIndex(), index);
			localCost += value;
			localCost += edgeSum;
			if (nodeCounter == nodes.size() - 1) {
				return localSolution.release();
			}
			auto retSolution = std::unique_ptr<PBQPSolution<InfinityWrapper<T>>>(
					recursiveSolve(currentCost + localCost, localSolution.get(), nodeCounter + 1));
			if (retSolution == nullptr) {
				continue;
			}
			InfinityWrapper<T> possibleMin = retSolution->getCurrentCost(graph);
			if (possibleMin < localMin) {
				localMin = possibleMin;
				minSelection = index;
				minSolution = std::move(retSolution);
			}
		}
		return minSolution.release();
	}
};

} // namespace pbqppapa
