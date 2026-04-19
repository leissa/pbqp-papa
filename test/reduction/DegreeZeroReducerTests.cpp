#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <set>
#include <vector>

#include <doctest/doctest.h>

#include "graph/PBQPEdge.hpp"
#include "graph/PBQPGraph.hpp"
#include "graph/PBQPNode.hpp"
#include "graph/PBQPSolution.hpp"
#include "graph/Vector.hpp"
#include "reduction/degree/DegreeZeroReducer.hpp"
#include "reduction/PBQPReduction.hpp"
#include "util/TestUtils.hpp"

namespace pbqppapa {

TEST_CASE("emptyGraphTest") {
	// make sure this doesnt explode
	PBQPGraph<int> graph;
	DegreeZeroReducer<int> zeroReducer(&graph);
	std::vector<PBQPGraph<int>*> result = zeroReducer.reduce();
}

TEST_CASE("simpleNodeReduction") {
	PBQPGraph<int> graph = PBQPGraph<int>();
	for (int i = 0; i < 20; i++) {
		int arr[]{3, 1};
		Vector<int> vek(2, arr);
		PBQPNode<int>* node = graph.addNode(vek);
	}
	DegreeZeroReducer<int> zeroReducer(&graph);
	std::vector<PBQPGraph<int>*> result = zeroReducer.reduce();
	CHECK_EQ(result.size(), 1);
	PBQPGraph<int>* resultGraph = result[0];
	CHECK_EQ(resultGraph->getNodeCount(), 0);
}

} // namespace pbqppapa
