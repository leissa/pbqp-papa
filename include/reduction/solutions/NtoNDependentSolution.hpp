#pragma once

#include <vector>

#include "reduction/solutions/DependentSolution.hpp"

namespace pbqppapa {

template <typename T>
class PBQPSolution;
template <typename T>
class PBQPNode;
template <typename T>
class DependentSolution;

/**
 * Stores the solution of a set of nodes A dependent on the solution of another set of nodes B
 *
 * The exact sets A and B are given to the constructor and then for each possible selection in A,
 * a selection in B must be given.
 *
 * This allows removing B from the graph, solving the remaining (simpler) graph and retrieving
 * a solution for B once A (which is part of the remaining graph) is solved
 */
template <typename T>
class NtoNDependentSolution: public DependentSolution<T> {
private:
	std::vector<PBQPNode<T>*> solutionNodes;
	std::vector<PBQPNode<T>*> dependencyNodes;
	std::vector<uint16_t> solutions;
	std::vector<PBQPEdge<T>> edges;
	std::vector<Vector<T>> dependencyVectorsPreChange;

public:
	NtoNDependentSolution(
			const std::vector<PBQPNode<T>*>& dependencyNodes, const std::vector<PBQPNode<T>*>& solutionNodes) :
			dependencyNodes(dependencyNodes), solutionNodes(solutionNodes) {
		const size_t length = dependencyNodes.size();
		size_t solutionAmount = 1;
		for (size_t i = 0; i < length; i++) {
			PBQPNode<T>* node = dependencyNodes.at(i);
			dependencyVectorsPreChange.push_back(node->getVector());
			// TODO fix this
			solutionAmount *= node->getVectorDegree();
		}

		solutions = std::vector<uint16_t>(solutionAmount * solutionNodes.size());
	}

	void setSolution(
			const std::vector<uint16_t>& dependencySelections, const std::vector<uint16_t>& solutionSelection) {
		size_t index = resolveIndex(dependencySelections);
		std::copy(solutionSelection.begin(), solutionSelection.end(), solutions.data() + index);
	}

	void solve(PBQPSolution<T>* solution) const {
		std::vector<uint16_t> dependencySolution(dependencyNodes.size());
		for (size_t i = 0; i < dependencyNodes.size(); i++) {
			dependencySolution.at(i) = solution->getSolution(dependencyNodes.at(i)->getIndex());
		}
		size_t index = resolveIndex(dependencySolution);
		for (size_t i = 0; i < solutionNodes.size(); i++) {
			solution->setSolution(solutionNodes.at(i)->getIndex(), solutions.at(index + i));
		}
	}

	void revertChange(PBQPGraph<T>* graph) override {
		// TODO
	}

private:
	size_t resolveIndex(const std::vector<uint16_t>& dependencySelections) const {
		size_t index = 0;
		size_t offset = 1;
		for (size_t i = 0; i < dependencyNodes.size(); i++) {
			index += offset * dependencySelections.at(i);
			offset *= dependencyNodes.at(i)->getVectorDegree();
		}
		return index;
	}
};

} // namespace pbqppapa
