#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <cstdint>
#include <set>
#include <vector>

#include <doctest/doctest.h>

#include "graph/PBQPEdge.hpp"
#include "graph/PBQPGraph.hpp"
#include "graph/PBQPNode.hpp"
#include "graph/PBQPSolution.hpp"
#include "graph/Vector.hpp"
#include "io/PBQP_Serializer.hpp"
#include "reduction/degree/DegreeZeroReducer.hpp"
#include "reduction/InfinityFilter.hpp"
#include "util/TestUtils.hpp"

namespace pbqppapa {

TEST_CASE("reduction") {
	PBQP_Serializer<uint64_t> serial{};

	PBQPGraph<uint64_t> graph = PBQPGraph<uint64_t>();
	for (int i = 0; i < 20; i++) {
		uint64_t arr[]{3, 1};
		Vector<uint64_t> vek(2, arr);
		PBQPNode<uint64_t>* node = graph.addNode(vek);
	}
	DegreeZeroReducer<uint64_t> zeroReducer(&graph);
	std::vector<PBQPGraph<uint64_t>*> result = zeroReducer.reduce();
	CHECK_EQ(result.size(), 1);
	PBQPGraph<uint64_t>* resultGraph = result[0];
	CHECK_EQ(resultGraph->getNodeCount(), 0);
}

} // namespace pbqppapa
