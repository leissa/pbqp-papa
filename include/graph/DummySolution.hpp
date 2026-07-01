#pragma once

template <typename T>
class PBQPSolution;

template <typename T>
class DummySolution: public PBQPSolution<T> {

	T totalCost;

public:
	DummySolution(T cost) : PBQPSolution<T>(0), totalCost(cost) {}

	T getTotalCost(const PBQPGraph<T>* graph) override {
		return totalCost;
	}
};
