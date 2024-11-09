#pragma once

#include <cstdint>
#include <iostream>

namespace Byte {

	template<size_t Y, size_t X, typename Type>
	struct _Mat {
		Type data[Y * X];

		_Mat() = default;

		_Mat(Type fill) {
			for (size_t i{ 0 }; i < Y * X; ++i) {
				data[i] = fill;
			}
		}

		_Mat(const _Mat& left) {
			for (size_t i{ 0 }; i < Y * X; ++i) {
				data[i] = left.data[i];
			}
		}

		Type* operator[](size_t y) {
			return &data[y * X];
		}

		const Type* operator[](size_t y) const {
			return &data[y * X];
		}

		_Mat operator+(const _Mat& other) const {
			_Mat out{ *this };
			out += other;
			return out;
		}

		_Mat& operator+=(const _Mat& other) {
			for (size_t i{ 0 }; i < Y * X; ++i) {
				data[i] += other.data[i];
			}

			return *this;
		}

		_Mat operator-(const _Mat& other) const {
			_Mat out{ *this };
			out -= other;
			return out;
		}

		_Mat& operator-=(const _Mat& other) {
			for (size_t i{ 0 }; i < Y * X; ++i) {
				data[i] -= other.data[i];
			}

			return *this;
		}

		_Mat operator*(double scalar) const {
			_Mat out{ *this };
			out *= scalar;
			return out;
		}

		_Mat& operator*=(double scalar) {
			for (size_t i{ 0 }; i < Y * X; ++i) {
				data[i] *= scalar;
			}

			return *this;
		}

		_Mat operator/(double scalar) const {
			_Mat out{ *this };
			out /= scalar;
			return out;
		}

		_Mat& operator/=(double scalar) {
			for (size_t i{ 0 }; i < Y * X; ++i) {
				data[i] /= scalar;
			}

			return *this;
		}

		template<typename size_t X2>
		_Mat<Y, X2, Type> operator*(const _Mat<X, X2, Type>& other) const {
			_Mat<Y, X2, Type> out{ 0 };

			for (size_t i{ 0 }; i < Y; ++i) {
				for (size_t j{ 0 }; j < X2; ++j) {
					for (size_t k{ 0 }; k < X; ++k) {
						out[i][j] += (*this)[i][k] * other[k][j];
					}
				}
			}

			return out;
		}

		bool operator==(const _Mat& other) const
		{
			for (size_t i{ 0 }; i < Y * X; ++i) {
				if ((*this)[i % X][i / X] != other[i / X][i % X]) {
					return false;
				}
			}

			return true;
		}

		bool operator!=(const _Mat& other) const {
			return !((*this) == other);
		}

		_Mat<Y, X, Type> transposed() const {
			_Mat<Y, X, Type> out{};

			for (size_t i{ 0 }; i < Y * X; ++i) {
				out[i % X][i / X] = (*this)[i / X][i % X];
			}

			return out;
		}

		static _Mat identity() {
			_Mat out{ 0 };

			constexpr size_t BIGGER{ X > Y ? X : Y };

			for (size_t i{}; i < BIGGER; ++i) {
				out[i][i] = static_cast<Type>(1);
			}

			return out;
		}

	};


	template<size_t Y, size_t X, typename Type>
	using Mat = _Mat<Y, X, Type>;

	template<size_t N>
	using MatN = _Mat<N, N, float>;

	template<size_t N>
	using MatNd = _Mat<N, N, double>;

	using Mat2 = MatN<2>;
	using Mat3 = MatN<3>;
	using Mat4 = MatN<4>;

	using Mat4d = MatNd<2>;
	using Mat2d = MatNd<3>;
	using Mat3d = MatNd<4>;

	template<size_t Y, size_t X, typename Type>
	std::ostream& operator<<(std::ostream& os, const _Mat<Y, X, Type>& matrix) {
		os << "{";
		for (size_t i{ 0 }; i < Y; ++i) {
			for (size_t j{ 0 }; j < X; ++j) {
				os << matrix[i][j] << ",";
			}
			if (i != Y - 1) {
				os << "\n";
			}
		}
		os << "}\n";

		return os;
	}

}