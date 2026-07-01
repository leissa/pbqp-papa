#ifndef DEBUG_GRAPHVISUALIZERTESTS_CPP_
#define DEBUG_GRAPHVISUALIZERTESTS_CPP_

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <algorithm>
#include <cstdint>
#include <memory>

#include <doctest/doctest.h>

#include "debug/GraphVisualizer.hpp"
#include "graph/PBQPGraph.hpp"
#include "io/PBQP_Serializer.hpp"
#include "util/TestUtils.hpp"

namespace pbqppapa {

TEST_CASE("basic") {
#if PBQP_USE_GVC
	PBQP_Serializer<InfinityWrapper<uint32_t>> serial;
	auto graph = std::unique_ptr<PBQPGraph<InfinityWrapper<uint32_t>>>(
			serial.loadFromFile("test/testData/smallLibfirmGraph.json"));
	GraphVisualizer<InfinityWrapper<uint32_t>>::dump(graph.get(), "test_build/mintput.svg");
}

TEST_CASE("loaded") {
	PBQP_Serializer<InfinityWrapper<uint32_t>> serial;
	auto graph = std::unique_ptr<PBQPGraph<InfinityWrapper<uint32_t>>>(
			serial.loadFromFile("test/testData/normalLibfirmGraph.json"));
	GraphVisualizer<InfinityWrapper<uint32_t>>::dump(graph.get(), "test_build/basicOutput.svg");
}

TEST_CASE("manual") {
	auto graph = std::make_unique<PBQPGraph<InfinityWrapper<uint32_t>>>();
	InfinityWrapper<uint32_t> arr[2];
	arr[0] = InfinityWrapper<uint32_t>(2);
	arr[1] = InfinityWrapper<uint32_t>(3);
	Vector<InfinityWrapper<uint32_t>> vector(2, arr);
	PBQPNode<InfinityWrapper<uint32_t>>* node = graph->addNode(vector);
	InfinityWrapper<uint32_t> arr2[2];
	arr2[0] = InfinityWrapper<uint32_t>(2);
	arr2[1] = InfinityWrapper<uint32_t>(1);
	Vector<InfinityWrapper<uint32_t>> vector2(2, arr2);
	PBQPNode<InfinityWrapper<uint32_t>>* node2 = graph->addNode(vector2);
	InfinityWrapper<uint32_t> arr3[4];
	arr3[0] = InfinityWrapper<uint32_t>(2);
	arr3[1] = InfinityWrapper<uint32_t>(1);
	arr3[2] = InfinityWrapper<uint32_t>(0);
	arr3[3] = InfinityWrapper<uint32_t>(1);
	Matrix<InfinityWrapper<uint32_t>> mat(2, 2, arr3);
	graph->addEdge(node, node2, mat);
	GraphVisualizer<InfinityWrapper<uint32_t>>::dump(graph.get(), "test_build/the.svg", true);
#endif
}

} // namespace pbqppapa

#endif /* DEBUG_GRAPHVISUALIZERTESTS_CPP_ */
