#ifndef STEPBYSTEPSOLVER_HPP_
#define STEPBYSTEPSOLVER_HPP_

#include <memory>
#include <queue>
#include <vector>

#include "debug/DebugTimer.hpp"
#include "debug/StatKeeper.hpp"
#include "graph/PBQPGraph.hpp"
#include "graph/PBQPSolution.hpp"
#include "math/InfinityWrapper.hpp"
#include "reduction/degree/DegreeOneReducer.hpp"
#include "reduction/degree/DegreeTwoReducer.hpp"
#include "reduction/degree/DegreeZeroReducer.hpp"
#include "reduction/heuristic/DegreeNReductor.hpp"
#include "reduction/solutions/NtoNDependentSolution.hpp"

namespace pbqppapa {

template <typename T>
class PBQPGraph;
template <typename T>
class PBQPSolution;
template <typename T>
class PBQPNode;
template <typename T>
class VektorDegreeOneReducer;
template <typename T>
class VectorDegreeZeroReducer;
template <typename T>
class VectorDegreeZeroReducer;
template <typename T>
class DegreeOneReducer;
template <typename T>
class DegreeZeroReducer;
template <typename T>
class DegreeTwoReducer;
template <typename T>
class InfinityWrapper;

template <typename T>
class StepByStepSolver {
	enum Reduction { R0, R1, R2, RN, NONE };

private:
	PBQPGraph<InfinityWrapper<T>>* graph;
	std::unique_ptr<PBQPGraph<InfinityWrapper<T>>> originalGraph;
	std::vector<std::unique_ptr<DependentSolution<InfinityWrapper<T>>>> localSolutions;
	bool useRnAlready;
	std::queue<PBQPNode<InfinityWrapper<T>>*> nodeQueue;
	std::vector<PBQPNode<InfinityWrapper<T>>*> nodeVector;
	unsigned int nodeIndex;
	unsigned int peoIndex;
	unsigned int backtrackIndex;
	bool isBackTracking;
	Reduction lastReduction;

public:
	StepByStepSolver(PBQPGraph<InfinityWrapper<T>>* graph) :
			graph(graph), useRnAlready(false), nodeIndex(0), peoIndex(0), backtrackIndex(0), isBackTracking(false),
			lastReduction(NONE) {
		originalGraph = std::make_unique<PBQPGraph<InfinityWrapper<T>>>(graph);
		assert(graph->getPEO().size() == graph->getNodeCount());
		localSolutions.reserve(graph->getNodeCount());
		nodeVector.reserve(graph->getNodeCount());
		// copy all nodes in a vector to allow going back on reduction steps
		for (auto nodeIter = graph->getNodeBegin(); nodeIter != graph->getNodeEnd(); ++nodeIter) {
			nodeVector.push_back(*nodeIter);
		}
	}

	~StepByStepSolver() = default;

private:
	StepByStepSolver(const StepByStepSolver&) = delete;
	StepByStepSolver& operator=(const StepByStepSolver&) = delete;

public:
	PBQPNode<InfinityWrapper<T>>* stepForward() {
		if (graph->getNodeCount() == 0) {
		}
		while (true) {
			if (nodeQueue.empty()) {
				if (useRnAlready) {
					while (peoIndex < graph->getPEO().size()) {
						PBQPNode<InfinityWrapper<T>>* peoNode = graph->getPEO().at(peoIndex++);
						if (!peoNode->isDeleted()) {
							applyRN(peoNode);
							return peoNode;
						}
					}
					// entire peo has been solved
					backtrackIndex = localSolutions.size() - 1;
					isBackTracking = true;
					return nullptr;
				} else {
					if (nodeIndex >= nodeVector.size()) {
						useRnAlready = true;
						continue;
					}
					PBQPNode<InfinityWrapper<T>>* node = nodeVector.at(nodeIndex++);
					nodeQueue.push(node);
				}
			}
			PBQPNode<InfinityWrapper<T>>* node = nodeQueue.front();
			nodeQueue.pop();
			if (node->isDeleted()) {
				continue;
			}
			if (applyR0R1R2(node)) {
				return node;
			}
		}
	}

	bool applyR0R1R2(PBQPNode<InfinityWrapper<T>>* node) {
		unsigned short degree = node->getDegree();
		switch (degree) {
		case 2: {
			std::vector<PBQPNode<InfinityWrapper<T>>*> neighbors = node->getAdjacentNodes(false);
			unsigned short oldDegree = neighbors.at(0)->getDegree();
			localSolutions.emplace_back(DegreeTwoReducer<T>::reduceDegreeTwoInf(node, this->graph));
			if (neighbors.at(0)->getDegree() != oldDegree) {
				// this happens if the edge created by the R2 reduction is merged into an existing edge
				nodeQueue.push(neighbors.at(0));
				nodeQueue.push(neighbors.at(1));
			}
			lastReduction = R2;
			return true;
		}
		case 0:
			localSolutions.emplace_back(DegreeZeroReducer<T>::reduceDegreeZeroInf(node, this->graph));
			lastReduction = R0;
			return true;
		case 1: {
			PBQPNode<InfinityWrapper<T>>* other = node->getAdjacentNodes().at(0);
			localSolutions.emplace_back(DegreeOneReducer<T>::reduceDegreeOneInf(node, this->graph));
			nodeQueue.push(other);
			lastReduction = R1;
			return true;
		}
		default:
			return false;
		}
	}

	void applyRN(PBQPNode<InfinityWrapper<T>>* node) {
		for (PBQPNode<InfinityWrapper<T>>* neighbor : node->getAdjacentNodes()) {
			nodeQueue.push(neighbor);
		}
		auto sol = std::unique_ptr<DependentSolution<InfinityWrapper<T>>>(
				DegreeNReducer<InfinityWrapper<T>>::reduceRNEarlyDecision(node, this->graph));
		if (!sol) {
			std::cout << "EROR, could not solve node";
		}
		localSolutions.push_back(std::move(sol));
		lastReduction = RN;
	}

	[[nodiscard]] Reduction getLastReductionType() const {
		return lastReduction;
	}

	[[nodiscard]] PBQPSolution<InfinityWrapper<T>>* solveFully() {
		while (stepForward()) {
		}
		return retrieveSolution();
	}

	std::vector<PBQPNode<InfinityWrapper<T>>*> stepForward(int howMany) {
		std::vector<PBQPNode<InfinityWrapper<T>>*> result;
		for (unsigned int i = 0; i < howMany; i++) {
			result.push_back(stepForward());
		}
		return result;
	}

	PBQPNode<InfinityWrapper<T>>* stepBackward() {
		if (localSolutions.empty()) {
			return nullptr;
		}
		unsigned int index = localSolutions.size() - 1;
		std::unique_ptr<DependentSolution<InfinityWrapper<T>>> sol = std::move(localSolutions.at(index));
		localSolutions.pop_back();
		sol->revertChange(graph);
		PBQPNode<InfinityWrapper<T>>* node = sol->getReducedNode();
		if (useRnAlready) {
			// revert peo
			peoIndex = revertIterator(peoIndex, graph->getPEO(), node);
		} else {
			// revert general node iterator
			nodeIndex = revertIterator(nodeIndex, nodeVector, node);
		}
		return node;
	}

	std::vector<PBQPNode<InfinityWrapper<T>>*> stepBackward(unsigned int howMany) {
		std::vector<PBQPNode<InfinityWrapper<T>>*> result;
		for (unsigned int i = 0; i < howMany; i++) {
			result.push_back(stepBackward());
		}
		return result;
	}

	unsigned int revertIterator(unsigned int currentIndex, std::vector<PBQPNode<InfinityWrapper<T>>*>& vector,
			PBQPNode<InfinityWrapper<T>>* value) {
		unsigned int revertedIndex = currentIndex;
		while (revertedIndex > 0) {
			revertedIndex--;
			if (vector.at(revertedIndex) == value) {
				return revertedIndex;
			}
		}
		// no reverting needed, element wasnt iterated over yet
		return currentIndex;
	}

	void dump(std::string path) {
		// TODO
	}

	[[nodiscard]] bool isBacktracking() const {
		return isBackTracking;
	}

	[[nodiscard]] PBQPSolution<InfinityWrapper<T>>* retrieveSolution() {
		auto solution = std::make_unique<PBQPSolution<InfinityWrapper<T>>>(this->graph->getNodeIndexCounter());
		for (int i = localSolutions.size() - 1; i >= 0; i--) {
			localSolutions.at(i)->solve(solution.get());
		}
		return solution.release();
	}

	[[nodiscard]] PBQPGraph<InfinityWrapper<T>>* getGraph() {
		return graph;
	}

	[[nodiscard]] PBQPGraph<InfinityWrapper<T>>* getOriginalGraph() {
		return originalGraph.get();
	}

	[[nodiscard]] bool isSolvable() {
		auto solution = std::make_unique<PBQPSolution<InfinityWrapper<T>>>(this->graph->getNodeIndexCounter());
		unsigned int localPeoIndex = this->peoIndex;
		for (; localPeoIndex != graph->getPEO().size(); localPeoIndex++) {
			PBQPNode<InfinityWrapper<T>>* node = graph->getPEO().at(localPeoIndex);
			if (node->isDeleted()) {
				continue;
			}
			bool solvedNode = false;
			unsigned short const length = node->getVectorDegree();
			for (unsigned short i = 0; i < length; i++) {
				if (node->getVector().get(i).isInfinite()) {
					continue;
				}
				bool validSelection = true;
				for (PBQPEdge<InfinityWrapper<T>>* edge : node->getAdjacentEdges()) {
					PBQPNode<InfinityWrapper<T>>* otherEnd = edge->getOtherEnd(node);
					if (solution->hasSolution(otherEnd)) {
						InfinityWrapper<T> cost;
						if (edge->isSource(node)) {
							cost = edge->getMatrix().get(i, solution->getSolution(otherEnd));
						} else {
							cost = edge->getMatrix().get(solution->getSolution(otherEnd), i);
						}
						if (cost.isInfinite()) {
							validSelection = false;
							break;
						}
					}
				}
				if (validSelection) {
					solution->setSolution(node->getIndex(), i);
					solvedNode = true;
					break;
				}
			}
			if (!solvedNode) {
				return false;
			}
		}
		std::cout << " ---\n";
		std::cout << std::to_string(solution->getCurrentCost(originalGraph.get()).getValue()) << "\n";
		std::cout << " ---\n";
		for (auto iter = localSolutions.rbegin(); iter != localSolutions.rend(); ++iter) {
			(*iter)->solve(solution.get());
			InfinityWrapper<T> costX = solution->getCurrentCost(originalGraph.get());
			std::cout << std::to_string(costX.getValue()) << "\n";
			if (costX.isInfinite()) {
				std::cout << "broke" << "\n";
			}
		}
		std::cout << " ---\n";
		std::cout << std::to_string(solution->getTotalCost(originalGraph.get()).getValue()) << "\n";
		std::cout << std::to_string(InfinityWrapper<T>::getInfinite().getValue()) << "\n";
		return solution->getTotalCost(originalGraph.get()) != InfinityWrapper<T>::getInfinite();
	}
};

} // namespace pbqppapa

#endif /* STEPBYSTEPSOLVER_HPP_ */
