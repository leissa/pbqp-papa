#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "solve/StepByStepSolver.hpp"

#include <cstdint>
#include <memory>

#include <doctest/doctest.h>

#include "io/PBQP_Serializer.hpp"

namespace pbqppapa {

// This is the problem instance which KAPS could not solve
TEST_CASE("simpleSolve") {
	auto graph = std::unique_ptr<PBQPGraph<InfinityWrapper<uint32_t>>>(
			PBQP_Serializer<InfinityWrapper<uint32_t>>::loadFromFile("test/testData/problematic.json"));
	auto copy = std::make_unique<PBQPGraph<InfinityWrapper<uint32_t>>>(graph.get());
	StepByStepSolver<uint32_t> solver(graph.get());
	auto sol = std::unique_ptr<PBQPSolution<InfinityWrapper<uint32_t>>>(solver.solveFully());
	InfinityWrapper<uint32_t> cost = sol->getTotalCost(copy.get());
}

} // namespace pbqppapa
