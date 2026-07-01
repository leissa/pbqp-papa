#pragma once

#include "graph/Matrix.hpp"

namespace pbqppapa {

template <typename T>
class InfinityWrapper;

template <class T>
class Matrix;

/**
 * A matrix with only a single column.
 */
template <class T>
class Vector: public Matrix<T> {
public:
	/**
	 * Creates a new Vector (matrix with one column) with the given amount of rows. The given data
	 * must be an array with matching length
	 */
	Vector(uint16_t length, T* data) : Matrix<T>(length, 1, data) {}

	Vector() : Matrix<T>() {}

	/**
	 * Creates a new Vector with uninitialized content
	 */
	Vector(uint16_t length) : Matrix<T>(length, 1) {}

	Vector(const Vector& vek) : Matrix<T>(vek) {}

	Vector(Vector&& vek) noexcept : Matrix<T>(std::move(vek)) {}

	Vector& operator=(Vector vek) noexcept {
		swap(*this, vek);
		return *this;
	}

	/**
	 * Gets the index (row number) of the smallest element within the Vector.
	 * Will encounter undefined behavior if the Vector is of length 0.
	 */
	[[nodiscard]] uint16_t getIndexOfSmallestElement() const {
		uint16_t minimumIndex = 0;
		T minimumValue = this->content[0];
		for (uint16_t i = 0; i < this->rows; i++) {
			if (this->content[i] < minimumValue) {
				minimumValue = this->content[i];
				minimumIndex = i;
			}
		}
		return minimumIndex;
	}

	/**
	 * Gets an element in the vector by its index/row
	 */
	[[nodiscard]] T& get(uint16_t index) const {
		assert(index < this->rows);
		return this->content[index];
	}
};

} // namespace pbqppapa
