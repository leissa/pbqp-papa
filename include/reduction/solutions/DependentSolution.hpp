#ifndef REDUCTION_SOLUTIONS_DEPENDENTSOLUTION_HPP_
#define REDUCTION_SOLUTIONS_DEPENDENTSOLUTION_HPP_

namespace pbqppapa {

template <typename T>
class PBQPSolution;
template <typename T>
class PBQPNode;
template <typename T>
class PBQPGraph;

template <typename T>
class DependentSolution {

public:
	virtual ~DependentSolution() = default;

	virtual void solve([[maybe_unused]] PBQPSolution<T>* solution) {}

	virtual void revertChange([[maybe_unused]] PBQPGraph<T>* graph) {}

	virtual PBQPNode<T>* getReducedNode() {
		return nullptr;
	}
};

} // namespace pbqppapa

#endif /* REDUCTION_SOLUTIONS_DEPENDENTSOLUTION_HPP_ */
