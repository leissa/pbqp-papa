#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "math/GurobiConverter.hpp"

#include <memory>
#include <string>

#include <doctest/doctest.h>

#include "debug/DebugTimer.hpp"
#include "io/PBQP_Serializer.hpp"

namespace pbqppapa {

// we need to at least have an empty test case if gurobi is not enabled, otherwise the test suite will fail entirely
TEST_CASE("empty") {
#if PBQP_USE_GUROBI
	PBQP_Serializer<InfinityWrapper<unsigned int>> serial;
	std::vector<std::string> paths;
	paths.push_back("test/testData/smallLibfirmGraph.json");
	paths.push_back("test/testData/problematic.json");
	std::cout << "Linear solving: " << '\n';
	for (std::string path : paths) {
		std::cout << "Processing " << path << '\n';
		DebugTimer timer("Total timer: " + path);
		auto ogGraph = std::unique_ptr<PBQPGraph<InfinityWrapper<unsigned int>>>(serial.loadFromFile(path));
		GurobiConverter<unsigned int> gConv(ogGraph.get());
		timer.startTimer();
		auto solution = std::unique_ptr<PBQPSolution<InfinityWrapper<unsigned int>>>(gConv.solveGurobiLinear());
		timer.stopTimer();
		std::cout << timer.getOutput(true) << std::to_string(solution->getTotalCost(ogGraph.get()).getValue()) << '\n';
	}
	std::cout << "Quadratic solving: " << '\n';
	for (std::string path : paths) {
		std::cout << "Processing " << path << '\n';
		DebugTimer timer("Total timer: " + path);
		auto ogGraph = std::unique_ptr<PBQPGraph<InfinityWrapper<unsigned int>>>(serial.loadFromFile(path));
		GurobiConverter<unsigned int> gConv(ogGraph.get());
		timer.startTimer();
		auto solution = std::unique_ptr<PBQPSolution<InfinityWrapper<unsigned int>>>(gConv.solveGurobiQuadratic());
		timer.stopTimer();

		std::cout << timer.getOutput(true) << std::to_string(solution->getTotalCost(ogGraph.get()).getValue()) << '\n';
	}
#endif
}
} // namespace pbqppapa
