#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "solve/StepByStepSolver.hpp"

#include <memory>

#include <doctest/doctest.h>

#include "io/PBQP_Serializer.hpp"

namespace pbqppapa {

// This is the problem instance which KAPS could not solve
TEST_CASE("simpleSolve") {
	auto graph = std::unique_ptr<PBQPGraph<InfinityWrapper<unsigned int>>>(
			PBQP_Serializer<InfinityWrapper<unsigned int>>::loadFromFile("test/testData/problematic.json"));
	auto copy = std::make_unique<PBQPGraph<InfinityWrapper<unsigned int>>>(graph.get());
	StepByStepSolver<unsigned int> solver(graph.get());
	auto sol = std::unique_ptr<PBQPSolution<InfinityWrapper<unsigned int>>>(solver.solveFully());
	InfinityWrapper<unsigned int> cost = sol->getTotalCost(copy.get());
}

} // namespace pbqppapa
