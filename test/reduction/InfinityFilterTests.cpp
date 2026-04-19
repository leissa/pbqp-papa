#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <vector>
#include <set>

#include "graph/PBQPGraph.hpp"
#include "graph/Vector.hpp"
#include "graph/PBQPNode.hpp"
#include "graph/PBQPEdge.hpp"
#include "reduction/InfinityFilter.hpp"
#include "reduction/degree/DegreeZeroReducer.hpp"
#include "graph/PBQPSolution.hpp"
#include "io/PBQP_Serializer.hpp"

#include "util/TestUtils.hpp"

namespace pbqppapa {


TEST_CASE("reduction") {
	PBQP_Serializer<unsigned long> serial{};



	PBQPGraph<unsigned long> graph = PBQPGraph<unsigned long>();
	for (int i = 0; i < 20; i++) {
		unsigned long arr[] { 3, 1 };
		Vector<unsigned long> vek (2, arr);
		PBQPNode<unsigned long>* node = graph.addNode(vek);
	}
	DegreeZeroReducer<unsigned long> zeroReducer(&graph);
	std::vector<PBQPGraph<unsigned long>*> result = zeroReducer.reduce();
	CHECK_EQ(result.size(), 1);
	PBQPGraph<unsigned long>* resultGraph = result [0];
	CHECK_EQ(resultGraph->getNodeCount(), 0);
}


}
