#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <algorithm>
#include <memory>

#include <doctest/doctest.h>

#include "graph/PBQPEdge.hpp"
#include "graph/PBQPGraph.hpp"
#include "graph/PBQPNode.hpp"
#include "graph/Vector.hpp"
#include "util/TestUtils.hpp"

namespace pbqppapa {

TEST_CASE("basicEdgeGeneration") {
	PBQPGraph<int> graph = PBQPGraph<int>();
	CHECK_EQ(graph.getEdgeCount(), 0);
	CHECK_EQ(graph.getNodeCount(), 0);

	// generate a bunch of nodes
	for (int i = 1; i <= 50; i++) {
		int arr[] = {3, 2};
		Vector<int> vector = Vector<int>(2, arr);
		PBQPNode<int>* node = graph.addNode(vector);
		CHECK_EQ(graph.getEdgeCount(), 0);
		CHECK_EQ(graph.getNodeCount(), i);
		CHECK_EQ(node->getIndex(), i - 1);
		CHECK(vector == node->getVector());
		CHECK_EQ(node->getDegree(), 0);
		CHECK_EQ(node->getVectorDegree(), 2);
	}

	// generate a bunch of edges
	PBQPNode<int>* node1 = *(graph.getNodeBegin());
	int counter = 0;
	for (std::set<PBQPNode<int>*>::iterator it = graph.getNodeBegin(); it != graph.getNodeEnd(); it++) {
		PBQPNode<int>* node2 = *it;
		if (node2 == node1) {
			continue;
		}
		int matArr[] = {3, 2, 5, 8};
		Matrix<int> matrix = Matrix<int>(2, 2, matArr);
		CHECK_EQ(0, node2->getDegree());
		std::vector<PBQPNode<int>*> adjaNodes = node1->getAdjacentNodes(true);
		CHECK(std::find(adjaNodes.begin(), adjaNodes.end(), node2) == adjaNodes.end());
		PBQPEdge<int>* edge = graph.addEdge(node1, node2, matrix);
		CHECK_EQ(graph.getEdgeCount(), counter + 1);
		CHECK_EQ(graph.getNodeCount(), 50);
		CHECK_EQ(counter + 1, node1->getDegree());
		CHECK_EQ(1, node2->getDegree());

		adjaNodes = node1->getAdjacentNodes(true);
		CHECK(std::find(adjaNodes.begin(), adjaNodes.end(), node2) != adjaNodes.end());
		CHECK_EQ(adjaNodes.size(), counter + 1);
		adjaNodes = node1->getAdjacentNodes(false);
		CHECK(std::find(adjaNodes.begin(), adjaNodes.end(), node2) != adjaNodes.end());
		CHECK_EQ(adjaNodes.size(), counter + 1);
		if (counter != 0) {
			// exclude initial cycle
			adjaNodes = node2->getAdjacentNodes(true);
			CHECK(std::find(adjaNodes.begin(), adjaNodes.end(), node1) == adjaNodes.end());
			CHECK_EQ(adjaNodes.size(), 0);
		}
		adjaNodes = node2->getAdjacentNodes(false);
		CHECK(std::find(adjaNodes.begin(), adjaNodes.end(), node1) != adjaNodes.end());
		CHECK_EQ(adjaNodes.size(), 1);
		const std::vector<PBQPEdge<int>*> adjaEdge = node1->getAdjacentEdges(true);
		CHECK(std::find(adjaEdge.begin(), adjaEdge.end(), edge) != adjaEdge.end());
		CHECK_EQ(adjaEdge.size(), counter + 1);
		counter++;
	}
}

TEST_CASE("advancedEdgeGeneration") {
	int size = 20;
	auto graph = std::unique_ptr<PBQPGraph<int>>(genGraph(size));
	CHECK_EQ(graph->getEdgeCount(), size * (size - 1) / 2);
	CHECK_EQ(graph->getNodeCount(), size);
	for (std::set<PBQPNode<int>*>::iterator it = graph->getNodeBegin(); it != graph->getNodeEnd(); it++) {
		PBQPNode<int>* node = *it;
		CHECK_EQ(node->getDegree(), size - 1);
		CHECK_EQ(node->getAdjacentNodes(false).size(), size - 1);
	}
}
/* TODO fix this maybe some day
TEST_CASE("advancedEdgeRemoval") {
	int size = 20;
	PBQPGraph<int>* graph = genGraph(size);
	PBQPNode<int>* node = *(graph->getNodeBegin());
	int removed = 0;
	std::vector<PBQPNode<int>*> adjaNodes;
	int ogEdgeCount = size / 2 * size + size/2;
	for (PBQPEdge<int>* edge : node->getAdjacentEdges(true)) {
		PBQPNode<int>* other = edge->getOtherEnd(node);
		adjaNodes = other->getAdjacentNodes(true);
		graph->removeEdge(edge);
		removed++;
		CHECK_EQ(graph->getEdgeCount(), ogEdgeCount - removed);
		CHECK_EQ(node->getDegree(), size - removed);
		CHECK_EQ(other->getDegree(), size - 1);
		adjaNodes = node->getAdjacentNodes(true);
		CHECK(
				std::find(adjaNodes.begin(), adjaNodes.end(), other)
						== adjaNodes.end());
		adjaNodes = other->getAdjacentNodes(true);
	}
	delete graph;
} */

} // namespace pbqppapa
