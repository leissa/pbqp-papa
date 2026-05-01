#ifndef GRAPH_MATRIX_H_
#define GRAPH_MATRIX_H_

#include <algorithm>
#include <cassert>
#include <memory>

#ifndef PBQP_USE_GUROBI
#define PBQP_USE_GUROBI 0
#endif
#ifndef PBQP_USE_GVC
#define PBQP_USE_GVC 0
#endif

namespace pbqppapa {

template <typename T>
class InfinityWrapper;

template <typename T>
class Matrix {

protected:
	unsigned short rows;
	unsigned short columns;
	std::unique_ptr<T[]> content;

public:
	/**
	 * Creates a new n x m matrix with n rows and m columns.
	 * The given data is expected to be an array of length n * m,
	 * row by row, for example:
	 *
	 *  0 1
	 *  2 3
	 *
	 */
	Matrix(unsigned short rows, unsigned short columns, T* data) :
			rows(rows), columns(columns), content(std::make_unique<T[]>(rows * columns)) {
		const unsigned long elementCount = rows * columns;
		if (elementCount != 0) {
			std::copy_n(data, elementCount, content.get());
		}
	}

	Matrix() : rows(0), columns(0), content(std::make_unique<T[]>(0)) {}

	/**
	 * Creates a new matrix with uninitialized content
	 */
	Matrix(unsigned short rows, unsigned short columns) :
			rows(rows), columns(columns), content(std::make_unique<T[]>(rows * columns)) {}

	Matrix(const Matrix& matrix) :
			rows(matrix.rows), columns(matrix.columns), content(std::make_unique<T[]>(rows * columns)) {
		const unsigned long elementCount = rows * columns;
		if (elementCount != 0) {
			std::copy_n(matrix.content.get(), elementCount, content.get());
		}
	}

	Matrix(Matrix&& other) noexcept : Matrix() {
		swap(*this, other);
	}

	~Matrix() = default;

	friend void swap(Matrix& a, Matrix& b) noexcept {
		using std::swap;
		swap(a.rows, b.rows);
		swap(a.columns, b.columns);
		swap(a.content, b.content);
	}

	Matrix& operator=(Matrix other) noexcept {
		swap(*this, other);
		return *this;
	}

	/**
	 * Adds the given matrix to this one
	 */
	Matrix& operator+=(const Matrix& other) {
		assert(other.rows == this->rows);
		assert(other.columns == this->columns);
		const unsigned long length = rows * columns;
		for (unsigned long i = 0; i < length; i++) {
			content[i] += other.content[i];
		}
		return *this;
	}

	/**
	 * Subtracts the given matrix from this one
	 */
	Matrix& operator-=(const Matrix& other) {
		assert(other.rows == this->rows);
		assert(other.columns == this->columns);
		const unsigned long length = rows * columns;
		for (unsigned long i = 0; i < length; i++) {
			content[i] -= other.content[i];
		}
		return *this;
	}

	[[nodiscard]] bool operator==(const Matrix& other) const {
		if (other.getRowCount() != this->getRowCount() || other.getColumnCount() != this->getColumnCount()) {
			return false;
		}
		const unsigned long length = rows * columns;
		for (unsigned long i = 0; i < length; i++) {
			if (content[i] != other.content[i]) {
				return false;
			}
		}
		return true;
	}

	/**
	 * Multiplies each value in the entire matrix by a given factor
	 */
	Matrix& operator*=(const T& factor) {
		const unsigned long length = rows * columns;
		for (unsigned long i = 0; i < length; i++) {
			content[i] *= factor;
		}
		return *this;
	}

	/**
	 * Divides each value in the entire matrix by a given divisor
	 */
	Matrix& operator/=(const T& quotient) {
		const unsigned long length = rows * columns;
		for (unsigned long i = 0; i < length; i++) {
			content[i] /= quotient;
		}
		return *this;
	}

	/**
	 *  Creates a transposed version of this matrix.
	 */
	[[nodiscard]] Matrix transpose() const {
		const unsigned long size = columns * rows;
		Matrix result(columns, rows);
		for (unsigned long n = 0; n < size; n++) {
			int i = n / rows;
			int j = n % rows;
			result.getRaw(n) = content[columns * j + i];
		}
		return result;
	}

	/**
	 * Retrieves a single element by position
	 */
	[[nodiscard]] inline T& get(unsigned short row, unsigned short column) const {
		assert(row < rows);
		assert(column < columns);
		return content[(row * columns) + column];
	}

	/**
	 * Retrieves an element by its raw index in the content array
	 */
	[[nodiscard]] inline T& getRaw(unsigned int index) const {
		assert(index < getElementCount());
		return content[index];
	}

	/**
	 * Gets the amount of rows
	 */
	[[nodiscard]] unsigned short getRowCount() const {
		return rows;
	}

	/**
	 * Gets the amount of columns
	 */
	[[nodiscard]] unsigned short getColumnCount() const {
		return columns;
	}

	/**
	 * Gets the amount of elements
	 */
	[[nodiscard]] inline unsigned int getElementCount() const {
		return columns * rows;
	}

	/**
	 * Multiplies a matrix by placing multiple instances of it below each other, for example:
	 *
	 * A B
	 * C D
	 *
	 * (with multiplier 2) --->
	 *
	 * A B
	 * C D
	 * A B
	 * C D
	 *
	 *
	 */
	[[nodiscard]] Matrix multiplyRows(const unsigned short multiplier) const {
		Matrix result(rows * multiplier, columns);
		const unsigned long sectorSize = rows * columns;
		for (unsigned short i = 0; i < multiplier; i++) {
			std::copy_n(content.get(), sectorSize, result.content.get() + (i * sectorSize));
		}
		return result;
	}

	/**
	 * Multiplies a matrix by placing multiple instances of it below each other by multiplying
	 * rows individually, for example:
	 *
	 * A B
	 * C D
	 *
	 * (with multiplier 2) --->
	 *
	 * A B
	 * A B
	 * C D
	 * C D
	 *
	 *
	 */
	[[nodiscard]] Matrix multiplyRowsIndividually(const unsigned short multiplier) const {
		Matrix result(rows * multiplier, columns);
		const unsigned long rowLength = columns;
		const unsigned long sectionLength = rowLength * multiplier;
		for (unsigned short i = 0; i < rows; i++) {
			for (unsigned short offset = 0; offset < multiplier; offset++) {
				std::copy_n(content.get() + (rowLength * i), rowLength,
						result.content.get() + (rowLength * offset) + (sectionLength * i));
			}
		}
		return result;
	}

	/**
	 * Multiplies a matrix by placing multiple instances of it bext to each other by multiplying
	 * columns individually, for example:
	 *
	 * A B
	 * C D
	 *
	 * (with multiplier 2) --->
	 *
	 * A A B B
	 * C C D D
	 *
	 *
	 */
	[[nodiscard]] Matrix multiplyColumnsIndividually(const unsigned short multiplier) const {
		Matrix result(rows, columns * multiplier);
		const unsigned long sectorLength = rows * columns;
		for (unsigned long i = 0; i < rows * columns; i++) {
			for (int column = 0; column < multiplier; column++) {
				result.getRaw(i + (column * sectorLength)) = getRaw(i);
			}
		}
		return result;
	}

	/**
	 * Multiplies a matrix by placing multiple instances of it next to each other, for example:
	 *
	 * A B
	 * C D
	 *
	 * (with multiplier 2) --->
	 *
	 * A B A B
	 * C D C D
	 *
	 *
	 */
	[[nodiscard]] Matrix multiplyColumns(const unsigned short multiplier) const {
		Matrix result(rows, columns * multiplier);
		const unsigned long rowLength = columns;
		const unsigned long rowDataLength = rowLength * multiplier;
		for (unsigned short i = 0; i < rows; i++) {
			for (int column = 0; column < multiplier; column++) {
				std::copy_n(content.get() + (rowLength * i), rowLength,
						result.content.get() + (rowDataLength * i) + (column * rowLength));
			}
		}
		return result;
	}
};

} // namespace pbqppapa

#endif /* GRAPH_MATRIX_H_ */
