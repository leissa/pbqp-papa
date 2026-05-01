#ifndef REDUCTION_INFINITYFILTER_HPP_
#define REDUCTION_INFINITYFILTER_HPP_

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
		auto iter = this->graph->getEdgeBegin();
		while (iter != this->graph->getEdgeEnd()) {
			PBQPEdge<InfinityWrapper<T>>* edge = *iter;
			PBQPNode<InfinityWrapper<T>>* source = edge->getSource();
			PBQPNode<InfinityWrapper<T>>* target = edge->getTarget();
			Matrix<InfinityWrapper<T>>* valueMatrix = &(edge->getMatrix());
			Vector<InfinityWrapper<T>>* sourceVector = &(source->getVector());
			Vector<InfinityWrapper<T>>* targetVector = &(target->getVector());
			unsigned short sourceFactor = 0;
			unsigned short targetFactor = 0;
			for (unsigned short i = 0; i < source->getVectorDegree(); i++) {
				if (!sourceVector->get(i).isInfinite()) {
					sourceFactor++;
				}
			}
			for (unsigned short i = 0; i < target->getVectorDegree(); i++) {
				if (!targetVector->get(i).isInfinite()) {
					targetFactor++;
				}
			}
			unsigned long targetMatrixSize = sourceFactor * targetFactor;
			if (targetMatrixSize == (source->getVectorDegree() * target->getVectorDegree())) {
				++iter;
				continue;
			}
			auto targetData = std::make_unique<InfinityWrapper<T>[]>(targetMatrixSize);
			unsigned long counter = 0;
			for (unsigned short i = 0; i < source->getVectorDegree(); i++) {
				for (unsigned short k = 0; k < target->getVectorDegree(); k++) {
					if (sourceVector->get(i).isInfinite() || targetVector->get(k).isInfinite()) {
						continue;
					}
					targetData[counter++] = valueMatrix->get(i, k);
				}
			}
			edge->getMatrix() = Matrix<InfinityWrapper<T>>(sourceFactor, targetFactor, targetData.get());
			++iter;
		}
		auto nodeIter = this->graph->getNodeBegin();
		while (nodeIter != this->graph->getNodeEnd()) {
			PBQPNode<InfinityWrapper<T>>* node = *nodeIter;
			unsigned short length = 0;
			Vector<InfinityWrapper<T>>* vector = &(node->getVector());
			for (unsigned short i = 0; i < vector->getRowCount(); i++) {
				if (!vector->get(i).isInfinite()) {
					length++;
				}
			}
			auto newData = std::make_unique<InfinityWrapper<T>[]>(length);
			unsigned short counter = 0;
			for (unsigned short i = 0; i < vector->getRowCount(); i++) {
				if (!vector->get(i).isInfinite()) {
					newData[counter++] = vector->get(i);
				}
			}
			node->getVector() = Vector<InfinityWrapper<T>>(length, newData.get());
			++nodeIter;
		}

		this->result.push_back(this->graph);
		return this->result;
	}

	void solve(PBQPSolution<InfinityWrapper<T>>& solution) override {
		// don't need to do anything
	}
};

} // namespace pbqppapa

#endif /* REDUCTION_INFINITYFILTER_HPP_ */
