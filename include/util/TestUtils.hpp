#include <memory>
#include <set>
#include <vector>

#include "graph/PBQPEdge.hpp"
#include "graph/PBQPGraph.hpp"
#include "graph/PBQPNode.hpp"
#include "graph/Vector.hpp"

namespace pbqppapa {

// Generates a Kn (n nodes, each connected with each other and themselves), useful for testing
PBQPGraph<int>* genGraph(unsigned int size) {
	auto graph = std::make_unique<PBQPGraph<int>>();
	for (unsigned int i = 1; i <= size; i++) {
		int arr1[] = {3, 2};
		Vector<int> vector = Vector<int>(2, arr1);
		graph->addNode(vector);
	}
	for (auto node1 : graph->nodes()) {
		for (auto node2 : graph->nodes()) {
			int matrixCost[] = {3, 2, 5, 8};
			Matrix<int> matrix = Matrix<int>(2, 2, matrixCost);
			graph->addEdge(node1, node2, matrix);
		}
	}
	return graph.release();
}

} // namespace pbqppapa
