#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <cstdint>
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
	PBQPGraph<int32_t> graph = PBQPGraph<int32_t>();
	int32_t vekData[] = {2, 2};
	Vector<int32_t> vek = Vector<int32_t>(2, vekData);
	graph.addNode(vek);
	ConnectedComponentSeparator<int32_t> sep = ConnectedComponentSeparator<int32_t>(&graph);
	std::vector<PBQPGraph<int32_t>*> components = sep.reduce();
	PBQPSolution<int32_t> sol(0);
	sep.solve(sol);
	CHECK_EQ(components.size(), 1);
	PBQPGraph<int32_t>* retrievedGraph = components[0];
	CHECK_EQ(0, retrievedGraph->getEdgeCount());
	CHECK_EQ(1, retrievedGraph->getNodeCount());
	CHECK_EQ((*(graph.begin()))->getIndex(), (*(retrievedGraph->begin()))->getIndex());
	if (retrievedGraph != &graph) {
		// not neccessary for our implementation, but just to make sure
		std::unique_ptr<PBQPGraph<int32_t>> ownedGraph(retrievedGraph);
	}
}

TEST_CASE("emptyGraphTest") {
	PBQPGraph<int32_t> graph = PBQPGraph<int32_t>();
	ConnectedComponentSeparator<int32_t> sep(&graph);
	std::vector<PBQPGraph<int32_t>*> components = sep.reduce();
	PBQPSolution<int32_t> sol(0);
	sep.solve(sol);
	CHECK_EQ(components.size(), 1);
	PBQPGraph<int32_t>* retrievedGraph = components[0];
	CHECK_EQ(0, retrievedGraph->getEdgeCount());
	CHECK_EQ(0, retrievedGraph->getNodeCount());
}

TEST_CASE("basicNodeTest") {
	PBQPGraph<int32_t> graph = PBQPGraph<int32_t>();
	int size = 50;
	for (int i = 0; i < size; i++) {
		int arr[] = {2, 2};
		Vector<int32_t> vek = Vector<int32_t>(2, arr);
		graph.addNode(vek);
	}
	ConnectedComponentSeparator<int32_t> sep = ConnectedComponentSeparator<int32_t>(&graph);
	std::vector<PBQPGraph<int32_t>*> components = sep.reduce();
	PBQPSolution<int32_t> sol = PBQPSolution<int>(0);
	sep.solve(sol);
	CHECK_EQ(components.size(), size);
	std::set<int32_t> nodeIndices = std::set<int32_t>();
	for (int i = 0; i < size; i++) {
		PBQPGraph<int32_t>* retrievedGraph = components[i];
		CHECK_EQ(0, retrievedGraph->getEdgeCount());
		CHECK_EQ(1, retrievedGraph->getNodeCount());
		auto index = (*(retrievedGraph->begin()))->getIndex();
		CHECK_EQ(0, nodeIndices.count(index));
		nodeIndices.insert(index);
		if (retrievedGraph != &graph) {
			std::unique_ptr<PBQPGraph<int32_t>> ownedGraph(retrievedGraph);
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
			Vector<int32_t> vek = Vector<int32_t>(2, arr);
			PBQPNode<int32_t>* node = graph->addNode(vek);
			otherNodes.push_back(node);
			for (PBQPNode<int32_t>* otherNode : otherNodes) {
				int arr2[] = {3, 2, 5, 8};
				Matrix<int32_t> mat = Matrix<int>(2, 2, arr2);
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
		for (auto node : retrievedGraph->nodes()) {
			auto index = node->getIndex();
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
