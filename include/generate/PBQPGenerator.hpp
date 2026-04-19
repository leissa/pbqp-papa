#ifndef GENERATE_PBQPGENERATOR_HPP_
#define GENERATE_PBQPGENERATOR_HPP_

#include <random>

#include "graph/PBQPGraph.hpp"

namespace pbqppapa {

template <typename T>
class PBQPGraph;

/**
 * Utility class for testing purposes which can generated randomized PBQP instances
 */
template <typename T>
class PBQPGenerator {

private:
	unsigned long nodeCount;
	float nodeDegree;
	int upperValueBound;
	unsigned short upperVectorLengthBound;
	unsigned short lowerVectorLengthBound;
	mutable std::mt19937 rng;

public:
	PBQPGenerator(unsigned long nodeCount, float nodeDegree, int upperValueBound, unsigned short upperVectorLengthBound,
			unsigned short lowerVectorLengthBound) :
			nodeCount(nodeCount), nodeDegree(nodeDegree), upperValueBound(upperValueBound),
			upperVectorLengthBound(upperVectorLengthBound), lowerVectorLengthBound(lowerVectorLengthBound),
			rng(std::random_device{}()) {}

	PBQPGenerator() : PBQPGenerator(20, 2.5f, 100, 6, 2) {}

	~PBQPGenerator() = default;

	[[nodiscard]] PBQPGraph<T>* generate() const {
		PBQPGraph<T>* graph = new PBQPGraph<T>();
		for (unsigned long i = 0; i < nodeCount; i++) {
			addRandomNode(graph);
		}
		const long edgeCount = nodeCount * static_cast<long>(nodeDegree);
		for (int i = 0; i < edgeCount; i++) {
			genRandomEdge(graph);
		}
		return graph;
	}

	void addRandomNode(PBQPGraph<T>* graph) const {
		std::uniform_int_distribution<int> lengthDist(0, upperVectorLengthBound - lowerVectorLengthBound - 1);
		int length = lengthDist(rng) + lowerVectorLengthBound;
		Vector<T> vec(length);
		for (unsigned short i = 0; i < length; i++) {
			vec.get(i) = genRandomNumber();
		}
		graph->addNode(vec);
	}

	void genRandomEdge(PBQPGraph<T>* graph) const {
		if (graph->getNodeCount() < 2) {
			return;
		}
		std::uniform_int_distribution<int> nodeDist(0, graph->getNodeCount() - 1);
		int firstIndex = nodeDist(rng);
		int secondIndex = nodeDist(rng);
		PBQPNode<T>* sourceNode = nullptr;
		PBQPNode<T>* targetNode = nullptr;
		int counter = 0;
		for (auto iter = graph->getNodeBegin(); iter != graph->getNodeEnd(); ++iter) {
			PBQPNode<T>* node = *iter;
			if (counter == firstIndex) {
				sourceNode = node;
			}
			if (counter == secondIndex) {
				targetNode = node;
			}
			if (sourceNode != nullptr && targetNode != nullptr) {
				break;
			}
			counter++;
		}
		if (sourceNode == nullptr || targetNode == nullptr) {
			return;
		}
		Matrix<T> mat(sourceNode->getVectorDegree(), targetNode->getVectorDegree());
		for (unsigned short i = 0; i < sourceNode->getVectorDegree(); i++) {
			for (unsigned short k = 0; k < targetNode->getVectorDegree(); k++) {
				mat.get(i, k) = genRandomNumber();
			}
		}
		graph->addEdge(sourceNode, targetNode, mat);
	}

	[[nodiscard]] T genRandomNumber() const {
		std::uniform_int_distribution<int> valueDist(0, upperValueBound - 1);
		return static_cast<T>(valueDist(rng));
	}

	[[nodiscard]] unsigned long getNodeCount() const {
		return nodeCount;
	}

	[[nodiscard]] float getNodeDegree() const {
		return nodeDegree;
	}
};

} // namespace pbqppapa

#endif /* GENERATE_PBQPGENERATOR_HPP_ */
