#pragma once

#include <memory>
#include <vector>

#include "graph/PBQPGraph.hpp"
#include "math/InfinityWrapper.hpp"

namespace pbqppapa {

template <typename T>
class PBQP_Reduction;
template <typename T>
class PBQPGraph;
template <typename T>
class PBQPSolution;

template <typename T>
class InfinityFilter: public PBQP_Reduction<InfinityWrapper<T>> {

public:
	InfinityFilter(PBQPGraph<InfinityWrapper<T>>* graph) : PBQP_Reduction<T>(graph) {}

	std::vector<PBQPGraph<InfinityWrapper<T>>*>& reduce() override {
		for (auto edge : this->graph->edges()) {
			PBQPNode<InfinityWrapper<T>>* source = edge->getSource();
			PBQPNode<InfinityWrapper<T>>* target = edge->getTarget();
			Matrix<InfinityWrapper<T>>* valueMatrix = &(edge->getMatrix());
			Vector<InfinityWrapper<T>>* sourceVector = &(source->getVector());
			Vector<InfinityWrapper<T>>* targetVector = &(target->getVector());
			uint16_t sourceFactor = 0;
			uint16_t targetFactor = 0;
			for (uint16_t i = 0; i < source->getVectorDegree(); i++) {
				if (!sourceVector->get(i).isInfinite()) {
					sourceFactor++;
				}
			}
			for (uint16_t i = 0; i < target->getVectorDegree(); i++) {
				if (!targetVector->get(i).isInfinite()) {
					targetFactor++;
				}
			}
			size_t targetMatrixSize = sourceFactor * targetFactor;
			if (targetMatrixSize == (source->getVectorDegree() * target->getVectorDegree())) {
				continue;
			}
			auto targetData = std::make_unique<InfinityWrapper<T>[]>(targetMatrixSize);
			size_t counter = 0;
			for (uint16_t i = 0; i < source->getVectorDegree(); i++) {
				for (uint16_t k = 0; k < target->getVectorDegree(); k++) {
					if (sourceVector->get(i).isInfinite() || targetVector->get(k).isInfinite()) {
						continue;
					}
					targetData[counter++] = valueMatrix->get(i, k);
				}
			}
			edge->getMatrix() = Matrix<InfinityWrapper<T>>(sourceFactor, targetFactor, targetData.get());
		}
		for (auto node : this->graph->nodes()) {
			uint16_t length = 0;
			Vector<InfinityWrapper<T>>* vector = &(node->getVector());
			for (uint16_t i = 0; i < vector->getRowCount(); i++) {
				if (!vector->get(i).isInfinite()) {
					length++;
				}
			}
			auto newData = std::make_unique<InfinityWrapper<T>[]>(length);
			uint16_t counter = 0;
			for (uint16_t i = 0; i < vector->getRowCount(); i++) {
				if (!vector->get(i).isInfinite()) {
					newData[counter++] = vector->get(i);
				}
			}
			node->getVector() = Vector<InfinityWrapper<T>>(length, newData.get());
		}

		this->result.push_back(this->graph);
		return this->result;
	}

	void solve(PBQPSolution<InfinityWrapper<T>>& solution) override {
		// don't need to do anything
	}
};

} // namespace pbqppapa
