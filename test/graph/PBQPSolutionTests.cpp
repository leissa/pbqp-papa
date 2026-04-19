#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "graph/PBQPSolution.hpp"

namespace pbqppapa {

TEST_CASE("simpleSetGet") {
	const int size = 50;
	PBQPSolution<int> sol = PBQPSolution<int>(size);
	for(int i = 0; i < size; i++) {
		sol.setSolution(i, i * 3);
	}
	for(int i = 0; i < size; i++) {
		CHECK_EQ(sol.getSolution(i), i * 3);
	}
}

}
