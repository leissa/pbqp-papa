#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include <tuple>

#include <doctest/doctest.h>
#include <nlohmann/json.hpp>

#include "graph/PBQPGraph.hpp"
#include "io/PBQP_Serializer.hpp"

namespace pbqppapa {

using Cost = InfinityWrapper<uint32_t>;

static std::string slurp(const std::string& path) {
	std::ifstream in(path);
	return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

// The graph does not preserve node and edge order.
static nlohmann::json canonical(const std::string& path) {
	nlohmann::json json = nlohmann::json::parse(slurp(path));
	auto& nodes = json["nodes"];
	auto& edges = json["edges"];
	std::sort(nodes.begin(), nodes.end(), [](const nlohmann::json& a, const nlohmann::json& b) {
		return a["index"].get<size_t>() < b["index"].get<size_t>();
	});
	std::sort(edges.begin(), edges.end(), [](const nlohmann::json& a, const nlohmann::json& b) {
		return std::tuple(a["source"].get<size_t>(), a["target"].get<size_t>()) <
			   std::tuple(b["source"].get<size_t>(), b["target"].get<size_t>());
	});
	return json;
}

TEST_CASE("v1 strings round-trip through v2 numbers") {
	auto v1 = std::unique_ptr<PBQPGraph<Cost>>(
			PBQP_Serializer<Cost>::loadFromFile("test/testData/normalLibfirmGraph.json"));
	PBQP_Serializer<Cost>::saveToFile("test_build/serializer_a.json", v1.get());

	auto v2 = std::unique_ptr<PBQPGraph<Cost>>(PBQP_Serializer<Cost>::loadFromFile("test_build/serializer_a.json"));
	PBQP_Serializer<Cost>::saveToFile("test_build/serializer_b.json", v2.get());

	const std::string written = slurp("test_build/serializer_a.json");
	CHECK(v2->getNodeCount() == v1->getNodeCount());
	CHECK(v2->getEdgeCount() == v1->getEdgeCount());
	CHECK(canonical("test_build/serializer_a.json") == canonical("test_build/serializer_b.json"));
	CHECK(written.find("\"inf\"") != std::string::npos);
	CHECK(written.find("\"0\"") == std::string::npos);
	CHECK(written.find("\"version\":2") != std::string::npos);
}

} // namespace pbqppapa
