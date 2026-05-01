#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <memory>
#include <set>
#include <vector>

#include <doctest/doctest.h>

#include "graph/PBQPEdge.hpp"
#include "graph/PBQPGraph.hpp"
#include "graph/PBQPNode.hpp"
#include "graph/PBQPSolution.hpp"
#include "graph/Vector.hpp"
#include "reduction/ConnectedComponentSeparator.hpp"
#include "reduction/PBQPReduction.hpp"

namespace pbqppapa {

TEST_CASE("singleNodeTest") {
	PBQPGraph<signed int> graph = PBQPGraph<signed int>();
	signed int vekData[] = {2, 2};
	Vector<signed int> vek = Vector<signed int>(2, vekData);
	graph.addNode(vek);
	ConnectedComponentSeparator<signed int> sep = ConnectedComponentSeparator<signed int>(&graph);
	std::vector<PBQPGraph<signed int>*> components = sep.reduce();
	PBQPSolution<signed int> sol(0);
	sep.solve(sol);
	CHECK_EQ(components.size(), 1);
	PBQPGraph<signed int>* retrievedGraph = components[0];
	CHECK_EQ(0, retrievedGraph->getEdgeCount());
	CHECK_EQ(1, retrievedGraph->getNodeCount());
	CHECK_EQ((*(graph.getNodeBegin()))->getIndex(), (*(retrievedGraph->getNodeBegin()))->getIndex());
	if (retrievedGraph != &graph) {
		// not neccessary for our implementation, but just to make sure
		std::unique_ptr<PBQPGraph<signed int>> ownedGraph(retrievedGraph);
	}
}

TEST_CASE("emptyGraphTest") {
	PBQPGraph<signed int> graph = PBQPGraph<signed int>();
	ConnectedComponentSeparator<signed int> sep(&graph);
	std::vector<PBQPGraph<signed int>*> components = sep.reduce();
	PBQPSolution<signed int> sol(0);
	sep.solve(sol);
	CHECK_EQ(components.size(), 1);
	PBQPGraph<signed int>* retrievedGraph = components[0];
	CHECK_EQ(0, retrievedGraph->getEdgeCount());
	CHECK_EQ(0, retrievedGraph->getNodeCount());
}

TEST_CASE("basicNodeTest") {
	PBQPGraph<signed int> graph = PBQPGraph<signed int>();
	int size = 50;
	for (int i = 0; i < size; i++) {
		int arr[] = {2, 2};
		Vector<signed int> vek = Vector<signed int>(2, arr);
		graph.addNode(vek);
	}
	ConnectedComponentSeparator<signed int> sep = ConnectedComponentSeparator<signed int>(&graph);
	std::vector<PBQPGraph<signed int>*> components = sep.reduce();
	PBQPSolution<signed int> sol = PBQPSolution<int>(0);
	sep.solve(sol);
	CHECK_EQ(components.size(), size);
	std::set<signed int> nodeIndices = std::set<signed int>();
	for (int i = 0; i < size; i++) {
		PBQPGraph<signed int>* retrievedGraph = components[i];
		CHECK_EQ(0, retrievedGraph->getEdgeCount());
		CHECK_EQ(1, retrievedGraph->getNodeCount());
		unsigned int index = (*(retrievedGraph->getNodeBegin()))->getIndex();
		CHECK_EQ(0, nodeIndices.count(index));
		nodeIndices.insert(index);
		if (retrievedGraph != &graph) {
			std::unique_ptr<PBQPGraph<signed int>> ownedGraph(retrievedGraph);
		}
	}
}

TEST_CASE("advancedNodeTest") {
	auto graph = std::make_unique<PBQPGraph<int>>();
	int subgraphs = 10;
	int localSize = 10;
	int edgeCount = 0;
	for (int i = 0; i < subgraphs; i++) {
		edgeCount = 0;
		std::vector<PBQPNode<int>*> otherNodes = std::vector<PBQPNode<int>*>();
		for (int k = 0; k < localSize; k++) {
			int arr[] = {2, 2};
			Vector<signed int> vek = Vector<signed int>(2, arr);
			PBQPNode<signed int>* node = graph->addNode(vek);
			otherNodes.push_back(node);
			for (PBQPNode<signed int>* otherNode : otherNodes) {
				int arr2[] = {3, 2, 5, 8};
				Matrix<signed int> mat = Matrix<int>(2, 2, arr2);
				graph->addEdge(node, otherNode, mat);
				edgeCount++;
			}
		}
	}
	ConnectedComponentSeparator<int> sep = ConnectedComponentSeparator<int>(graph.get());
	std::vector<PBQPGraph<int>*> components = sep.reduce();
	PBQPSolution<int> sol = PBQPSolution<int>(0);
	sep.solve(sol);
	CHECK_EQ(components.size(), subgraphs);
	std::set<int> nodeIndices = std::set<int>();
	for (int i = 0; i < subgraphs; i++) {
		PBQPGraph<int>* retrievedGraph = components[i];
		CHECK_EQ(edgeCount - localSize, retrievedGraph->getEdgeCount());
		CHECK_EQ(localSize, retrievedGraph->getNodeCount());
		// ensure each node is only in one subgraph
		for (auto iter = retrievedGraph->getNodeBegin(); iter != retrievedGraph->getNodeEnd(); ++iter) {
			unsigned int index = (*iter)->getIndex();
			CHECK_EQ(0, nodeIndices.count(index));
			nodeIndices.insert(index);
		}
	}
	for (PBQPGraph<int>* subGraph : components) {
		if (subGraph != graph.get()) {
			std::unique_ptr<PBQPGraph<int>> ownedGraph(subGraph);
		}
	}
}

} // namespace pbqppapa
