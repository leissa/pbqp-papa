#ifndef DEBUG_GRAPHVISUALIZERTESTS_CPP_
#define DEBUG_GRAPHVISUALIZERTESTS_CPP_

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <algorithm>
#include <memory>

#include <doctest/doctest.h>

#include "debug/GraphVisualizer.hpp"
#include "graph/PBQPGraph.hpp"
#include "io/PBQP_Serializer.hpp"
#include "util/TestUtils.hpp"

namespace pbqppapa {

TEST_CASE("basic") {
#if PBQP_USE_GVC
	PBQP_Serializer<InfinityWrapper<unsigned int>> serial;
	auto graph = std::unique_ptr<PBQPGraph<InfinityWrapper<unsigned int>>>(
			serial.loadFromFile("test/testData/smallLibfirmGraph.json"));
	GraphVisualizer<InfinityWrapper<unsigned int>>::dump(graph.get(), "test_build/mintput.svg");
}

TEST_CASE("loaded") {
	PBQP_Serializer<InfinityWrapper<unsigned int>> serial;
	auto graph = std::unique_ptr<PBQPGraph<InfinityWrapper<unsigned int>>>(
			serial.loadFromFile("test/testData/normalLibfirmGraph.json"));
	GraphVisualizer<InfinityWrapper<unsigned int>>::dump(graph.get(), "test_build/basicOutput.svg");
}

TEST_CASE("manual") {
	auto graph = std::make_unique<PBQPGraph<InfinityWrapper<unsigned int>>>();
	InfinityWrapper<unsigned int> arr[2];
	arr[0] = InfinityWrapper<unsigned int>(2);
	arr[1] = InfinityWrapper<unsigned int>(3);
	Vector<InfinityWrapper<unsigned int>> vector(2, arr);
	PBQPNode<InfinityWrapper<unsigned int>>* node = graph->addNode(vector);
	InfinityWrapper<unsigned int> arr2[2];
	arr2[0] = InfinityWrapper<unsigned int>(2);
	arr2[1] = InfinityWrapper<unsigned int>(1);
	Vector<InfinityWrapper<unsigned int>> vector2(2, arr2);
	PBQPNode<InfinityWrapper<unsigned int>>* node2 = graph->addNode(vector2);
	InfinityWrapper<unsigned int> arr3[4];
	arr3[0] = InfinityWrapper<unsigned int>(2);
	arr3[1] = InfinityWrapper<unsigned int>(1);
	arr3[2] = InfinityWrapper<unsigned int>(0);
	arr3[3] = InfinityWrapper<unsigned int>(1);
	Matrix<InfinityWrapper<unsigned int>> mat(2, 2, arr3);
	graph->addEdge(node, node2, mat);
	GraphVisualizer<InfinityWrapper<unsigned int>>::dump(graph.get(), "test_build/the.svg", true);
#endif
}

} // namespace pbqppapa

#endif /* DEBUG_GRAPHVISUALIZERTESTS_CPP_ */
