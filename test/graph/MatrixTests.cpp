#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <stdlib.h>
#include <algorithm>
#include <array>
#include <stdlib.h>

#include "graph/PBQPGraph.hpp"
#include "graph/Matrix.hpp"
#include "graph/PBQPNode.hpp"
#include "graph/PBQPEdge.hpp"

namespace pbqppapa {

/**
 * Generates a n x n matrix filled with only 1
 */
template<typename T>
Matrix<T> genMatrix(int n, int value) {
	T* data = new T[n * n];
	Matrix<T> mat = Matrix<T>(n, n, data);
	delete data;
	for (int i = 0; i < n * n; i++) {
		mat.getRaw(i) = value;
	}
	return mat;
}

template<typename T>
Matrix<T> genMatrixAscending(int n, int value) {
	T* data = new T[n * n];
	Matrix<T> matrix = Matrix<T>(n, n, data);
	delete data;
	for (int i = 0; i < n * n; i++) {
		matrix.getRaw(i) = value++;
	}
	return matrix;
}

Matrix<int> genMatrixRandom(int maxLength) {
	srand(time(NULL));
	int rows = rand() % maxLength + 1;
	int columns = rand() % maxLength + 1;
	Matrix<int> matrix = Matrix<int>(rows, columns);
	for (int row = 0; row < rows; row++) {
		for (int column = 0; column < columns; column++) {
			matrix.get(row, column) = rand();
		}
	}
	return matrix;
}

//Not used anywhere at the moment, but intentionally left here, because it massively eases debugging
void printMatrix(Matrix<int>& matrix) {
	MESSAGE("---------");
	MESSAGE("Rows" << matrix.getRowCount());
	MESSAGE("Columns" << matrix.getColumnCount());
	for (int row = 0; row < matrix.getRowCount(); row++) {
		for (int column = 0; column < matrix.getColumnCount(); column++) {
			MESSAGE(
					"Column: " << column << " ; " << "Row: " << row << "  " << matrix.get(row, column));
		}
		MESSAGE("---");
	}
}

TEST_CASE("basicMatrixGeneration") {
	for (int i = 0; i <= 15; i++) {
		Matrix<int> matrix = genMatrix<int>(i, i);
		CHECK_EQ(matrix.getColumnCount(), i);
		CHECK_EQ(matrix.getRowCount(), i);
		CHECK_EQ(matrix.getElementCount(), i * i);
		int count = 0;
		for (int row = 0; row < i; row++) {
			for (int column = 0; column < i; column++) {
				CHECK_EQ(matrix.get(row, column), i);
				CHECK_EQ(matrix.getRaw(count++), i);
			}
		}
	}
}

TEST_CASE("matrixPlus") {
	int size = 15;
	int firstValue = 2358;
	int secondValue = 2734;
	Matrix<int> matrix = genMatrixAscending<int>(size, firstValue);
	Matrix<int> matrix2 = genMatrixAscending<int>(size, secondValue);
	matrix += matrix2;
	int counter = 0;
	for (int row = 0; row < size; row++) {
		for (int column = 0; column < size; column++) {
			CHECK_EQ(matrix.get(row, column),
					firstValue + secondValue + (counter++ * 2));
		}
	}
}

TEST_CASE("matrixMinus") {
	int size = 15;
	int firstValue = 473235;
	int secondValue = 9284736;
	Matrix<int> matrix = genMatrixAscending<int>(size, firstValue);
	Matrix<int> matrix2 = genMatrixAscending<int>(size, secondValue);
	matrix -= matrix2;
	for (int row = 0; row < size; row++) {
		for (int column = 0; column < size; column++) {
			CHECK_EQ(matrix.get(row, column),
					firstValue - secondValue);
		}
	}
}

TEST_CASE("matrixMultiply") {
	int size = 15;
	int value = 425327;
	int factor = 18;
	Matrix<int> matrix = genMatrixAscending<int>(size, value);
	matrix *= factor;
	int counter = 0;
	for (int row = 0; row < size; row++) {
		for (int column = 0; column < size; column++) {
			CHECK_EQ(matrix.get(row, column),
					(value + counter++) * factor);
		}
	}
}

TEST_CASE("matrixDivide") {
	int size = 15;
	int value = 38482;
	int divisor = 3;
	Matrix<int> matrix = genMatrixAscending<int>(size, value);
	matrix /= divisor;
	int counter = 0;
	for (int row = 0; row < size; row++) {
		for (int column = 0; column < size; column++) {
			CHECK_EQ(matrix.get(row, column),
					(value + counter++) / divisor);
		}
	}
}

template<typename T>
void checkMatrixTranspose(Matrix<T>& m1, Matrix<T>& m2) {
	CHECK_EQ(m1.getColumnCount(), m2.getRowCount());
	CHECK_EQ(m1.getRowCount(), m2.getColumnCount());
	for (int row = 0; row < m1.getRowCount(); row++) {
		for (int column = 0; column < m1.getColumnCount(); column++) {
			CHECK_EQ(m1.get(row, column), m2.get(column, row));
		}
	}
}

TEST_CASE("matrixTranspose") {
	for (int i = 0; i < 20; i++) {
		Matrix<int> matrix = genMatrixRandom(20);
		Matrix<int> transposed = matrix.transpose();
		checkMatrixTranspose<int>(matrix, transposed);
	}
}

}

