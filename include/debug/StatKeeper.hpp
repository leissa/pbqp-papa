#pragma once

#include <string>

#include "graph/PBQPGraph.hpp"

namespace pbqppapa {

class StatKeeper {

private:
	size_t r0Applied = 0;
	size_t r1Applied = 0;
	size_t r2Applied = 0;
	size_t rNEarlyApplied = 0;
	size_t nodeCount;
	size_t edgeCount;
	float averageVectorDegree;

public:
	template <typename T>
	void submitGraphBefore(PBQPGraph<T>* graph) {
		nodeCount = graph->getNodeCount();
		edgeCount = graph->getEdgeCount();
		float vectorDegreeSum = 0;
		for (auto node : graph->nodes()) {
			vectorDegreeSum += static_cast<float>(node->getVectorDegree());
		}
		averageVectorDegree = vectorDegreeSum / static_cast<float>(nodeCount);
	}

	void applyR0();

	void applyR1();

	void applyR2();

	void applyRNEarly();

	[[nodiscard]] std::string getSumUp();

	[[nodiscard]] std::string getGraphDescription();
};

} // namespace pbqppapa
