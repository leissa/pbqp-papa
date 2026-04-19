#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "solve/StepByStepSolver.hpp"
#include "io/PBQP_Serializer.hpp"

namespace pbqppapa {

//This is the problem instance which KAPS could not solve
TEST_CASE("simpleSolve") {
	PBQPGraph<InfinityWrapper<unsigned int>>* graph = PBQP_Serializer<InfinityWrapper<unsigned int>>::loadFromFile(
	"test/testData/problematic.json");
	PBQPGraph<InfinityWrapper<unsigned int>>* copy = new PBQPGraph<InfinityWrapper<unsigned int>>(graph);
	StepByStepSolver<unsigned int> solver (graph);
	PBQPSolution<InfinityWrapper<unsigned int>>* sol = solver.solveFully();
	InfinityWrapper<unsigned int> cost = sol->getTotalCost(copy);
}

}
