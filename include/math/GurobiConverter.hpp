#pragma once

#include "graph/PBQPGraph.hpp"

#if PBQP_USE_GUROBI

#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include <gurobi_c++.h>

#include "graph/PBQPEdge.hpp"
#include "graph/PBQPNode.hpp"
#include "graph/PBQPSolution.hpp"
#include "math/InfinityWrapper.hpp"

namespace pbqppapa {

template <typename T>
class PBQPGraph;
template <typename T>
class PBQPNode;
template <typename T>
class PBQPEdge;
template <typename T>
class InfinityWrapper;
template <typename T>
class Vector;
template <typename T>
class PBQPSolution;

template <typename T>
class GurobiConverter {

private:
	GRBEnv env;
	GRBModel model = GRBModel(env);
	std::vector<std::unique_ptr<GRBVar[]>> nodeSelections;
	std::vector<std::unique_ptr<GRBVar[]>> edgeSelections;
	std::vector<uint16_t> nodeVectorLengths;
	std::vector<uint16_t> edgeSelectionColumnCounts;
	std::map<PBQPNode<InfinityWrapper<T>>*, size_t> nodeToGrbVarMap;
	size_t nodeCount;
	const PBQPGraph<InfinityWrapper<T>>* graph;

#ifndef NDEBUG
	bool ranYet = false;
#endif

public:
	/**
	 * Createa a new instance to solve the given graph. Note that you can only call a solve function ONCE.
	 * If you call one multiple times on the same instance bad things will happen
	 */
	GurobiConverter(const PBQPGraph<InfinityWrapper<T>>* graph) : graph(graph) {}

	/**
	 * Attempts to solve the PBQP instance using Gurobi linear programming
	 */
	PBQPSolution<InfinityWrapper<T>>* solveGurobiLinear() {
#ifndef NDEBUG
		assert(!ranYet);
		ranYet = true;
#endif
		setup(graph);
		// create variables
		createNodeSelection(graph);
		// only one selection per node
		limitSelectionPerNode();
		GRBLinExpr totalCost = 0;
		// cost at the nodes
		GRBLinExpr nodeCost = getNodeSelectionCost(graph);
		totalCost += nodeCost;
		// create edge variables
		setupEdgeSelectionLinear(graph);
		// tie edge selection to node selection
		limitEdgeSelectionLinear(graph);
		// add edge cost
		GRBLinExpr edgeCost = getEdgeSelectionLinear(graph);
		totalCost += edgeCost;
		model.setObjective(totalCost, GRB_MINIMIZE);
		model.optimize();
		// retrieve solution
		PBQPSolution<InfinityWrapper<T>>* solution = retrieveSolution(graph);
		tearDown();
		tearDownEdgesLinear(graph);
		return solution;
	}

	/**
	 * Attempts to solve the PBQP instance using Gurobi quadratic programming
	 */
	PBQPSolution<InfinityWrapper<T>>* solveGurobiQuadratic() {
#ifndef NDEBUG
		assert(!ranYet);
		ranYet = true;
#endif
		setup(graph);
		// create variables
		createNodeSelection(graph);
		// only one selection per node
		limitSelectionPerNode();

		GRBQuadExpr totalCost = 0;
		// cost at the nodes
		GRBLinExpr nodeCost = getNodeSelectionCost(graph);
		totalCost += nodeCost;
		// cost at the edges
		GRBQuadExpr edgeCost = getEdgeSelectionCostQuadratic(graph);
		totalCost += edgeCost;
		model.setObjective(totalCost, GRB_MINIMIZE);
		model.optimize();

		// retrieve solution
		PBQPSolution<InfinityWrapper<T>>* solution = retrieveSolution(graph);
		tearDown();

		return solution;
	}

private:
	void setup(const PBQPGraph<InfinityWrapper<T>>* graph) {
		nodeCount = graph->getNodeCount();
		nodeSelections.clear();
		nodeSelections.resize(nodeCount);
		nodeVectorLengths.clear();
		nodeVectorLengths.resize(nodeCount);
		nodeToGrbVarMap.clear();
	}

	void tearDown() {
		nodeSelections.clear();
		nodeVectorLengths.clear();
		nodeToGrbVarMap.clear();
	}

	void tearDownEdgesLinear(const PBQPGraph<InfinityWrapper<T>>* graph) {
		(void)graph;
		edgeSelections.clear();
		edgeSelectionColumnCounts.clear();
	}

	[[nodiscard]] GRBVar& getEdgeSelection(size_t edgeIndex, uint16_t row, uint16_t column) const {
		return edgeSelections[edgeIndex][row * edgeSelectionColumnCounts[edgeIndex] + column];
	}

	void createNodeSelection(const PBQPGraph<InfinityWrapper<T>>* graph) {
		// create array of selections. Every contained array represents a node and the variables
		// in it represent the possible selections
		size_t counter = 0;
		for (auto node : graph->nodes()) {
			nodeToGrbVarMap.insert({node, counter});
			const uint16_t vecLength = node->getVectorDegree();
			nodeSelections[counter] = std::make_unique<GRBVar[]>(vecLength);
			nodeVectorLengths[counter] = vecLength;
			for (int i = 0; i < vecLength; i++) {
				nodeSelections[counter][i] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
			}
			counter++;
		}
	}

	void limitSelectionPerNode() {
		// Only select exactly one solution per node
		for (size_t i = 0; i < nodeCount; ++i) {
			GRBLinExpr expr = 0;
			for (int k = 0; k < nodeVectorLengths[i]; ++k) {
				expr += nodeSelections[i][k];
			}
			model.addConstr(expr, GRB_EQUAL, 1.0);
		}
	}

	GRBLinExpr getNodeSelectionCost(const PBQPGraph<InfinityWrapper<T>>* graph) {
		size_t counter = 0;
		GRBLinExpr totalCost = 0;
		for (auto node : graph->nodes()) {
			for (int i = 0; i < nodeVectorLengths[counter]; i++) {
				InfinityWrapper<T> value = node->getVector().get(i);
				if (value.isInfinite()) {
					GRBLinExpr expr = 0;
					// disallow infinite values by forcing its selection to be 0
					expr += nodeSelections[counter][i];
					model.addConstr(expr, GRB_EQUAL, 0.0);
				} else {
					// adding specific selection of a cost vector to the total cost
					totalCost += value.getValue() * nodeSelections[counter][i];
				}
			}
			counter++;
		}
		return totalCost;
	}

	GRBQuadExpr getEdgeSelectionCostQuadratic(const PBQPGraph<InfinityWrapper<T>>* graph) {
		GRBQuadExpr totalCost = 0;
		// Add the edge costs to the total goal
		for (auto edge : graph->edges()) {
			PBQPNode<InfinityWrapper<T>>* source = edge->getSource();
			PBQPNode<InfinityWrapper<T>>* target = edge->getTarget();
			size_t sourceGrbIndex = nodeToGrbVarMap.find(source)->second;
			size_t targetGrbIndex = nodeToGrbVarMap.find(target)->second;
			for (uint16_t row = 0; row < edge->getMatrix().getRowCount(); ++row) {
				for (uint16_t col = 0; col < edge->getMatrix().getColumnCount(); ++col) {
					InfinityWrapper<T> value = edge->getMatrix().get(row, col);
					if (value.isInfinite()) {
						// disallow infinite values in matrix by forcing the sum of the selections to be 0
						GRBQuadExpr expr = nodeSelections[sourceGrbIndex][row] * nodeSelections[targetGrbIndex][col];
						model.addQConstr(expr, GRB_EQUAL, 0.0);
					} else {
						// add specific selection for this edge to the total cost
						totalCost += value.getValue() * nodeSelections[sourceGrbIndex][row] *
									 nodeSelections[targetGrbIndex][col];
					}
				}
			}
		}
		return totalCost;
	}

	PBQPSolution<InfinityWrapper<T>>* retrieveSolution(const PBQPGraph<InfinityWrapper<T>>* graph) {
		auto solution = std::make_unique<PBQPSolution<InfinityWrapper<T>>>(graph->getNodeIndexCounter());
		size_t inc = 0;
		size_t loops = 0;
		for (auto node : graph->nodes()) {
			size_t nodeIndex = nodeToGrbVarMap.find(node)->second;
			for (int i = 0; i < nodeVectorLengths[nodeIndex]; i++) {
				if (nodeSelections[nodeIndex][i].get(GRB_DoubleAttr_X) > 0.5) {
					solution->setSolution(node->getIndex(), i);
					inc++;
				}
				loops++;
			}
		}
		return solution.release();
	}

	void setupEdgeSelectionLinear(const PBQPGraph<InfinityWrapper<T>>* graph) {
		const size_t edgeCount = graph->getEdgeCount();
		edgeSelections.clear();
		edgeSelections.resize(edgeCount);
		edgeSelectionColumnCounts.clear();
		edgeSelectionColumnCounts.resize(edgeCount);
		size_t counter = 0;
		for (auto edge : graph->edges()) {
			const uint16_t rowCount = edge->getMatrix().getRowCount();
			const uint16_t columnCount = edge->getMatrix().getColumnCount();
			edgeSelections[counter] = std::make_unique<GRBVar[]>(rowCount * columnCount);
			edgeSelectionColumnCounts[counter] = columnCount;
			for (uint16_t row = 0; row < rowCount; ++row) {
				for (uint16_t column = 0; column < columnCount; ++column) {
					getEdgeSelection(counter, row, column) = model.addVar(0.0, 1.0, 0.0, GRB_BINARY);
				}
			}
			++counter;
		}
	}

	GRBLinExpr getEdgeSelectionLinear(const PBQPGraph<InfinityWrapper<T>>* graph) {
		GRBLinExpr totalCost = 0;
		size_t counter = 0;
		for (auto edge : graph->edges()) {
			const uint16_t rowCount = edge->getMatrix().getRowCount();
			const uint16_t columnCount = edge->getMatrix().getColumnCount();
			for (uint16_t row = 0; row < rowCount; ++row) {
				for (uint16_t column = 0; column < columnCount; ++column) {
					InfinityWrapper<T> selectionCost = edge->getMatrix().get(row, column);
					if (selectionCost.isInfinite()) {
						GRBLinExpr tempSum = 0;
						tempSum += getEdgeSelection(counter, row, column);
						model.addConstr(tempSum, GRB_EQUAL, 0.0);
					} else {
						totalCost += getEdgeSelection(counter, row, column) * selectionCost.getValue();
					}
				}
			}
			++counter;
		}
		return totalCost;
	}

	void limitEdgeSelectionLinear(const PBQPGraph<InfinityWrapper<T>>* graph) {
		size_t counter = 0;
		for (auto edge : graph->edges()) {
			PBQPNode<InfinityWrapper<T>>* source = edge->getSource();
			PBQPNode<InfinityWrapper<T>>* target = edge->getTarget();
			size_t sourceGrbIndex = nodeToGrbVarMap.find(source)->second;
			size_t targetGrbIndex = nodeToGrbVarMap.find(target)->second;
			const uint16_t rowCount = edge->getMatrix().getRowCount();
			const uint16_t columnCount = edge->getMatrix().getColumnCount();
			// sum of row selections equals selection in that row in the source node
			for (uint16_t row = 0; row < rowCount; ++row) {
				GRBLinExpr cost = 0;
				for (uint16_t column = 0; column < columnCount; ++column) {
					cost += getEdgeSelection(counter, row, column);
				}
				cost -= nodeSelections[sourceGrbIndex][row];
				model.addConstr(cost, GRB_EQUAL, 0.0);
			}
			// sum of column selections equals selection in that row in the target node
			for (uint16_t column = 0; column < columnCount; ++column) {
				GRBLinExpr cost = 0;
				for (uint16_t row = 0; row < rowCount; ++row) {
					cost += getEdgeSelection(counter, row, column);
				}
				cost -= nodeSelections[targetGrbIndex][column];
				model.addConstr(cost, GRB_EQUAL, 0.0);
			}
			++counter;
		}
	}
};

} // namespace pbqppapa

#endif
