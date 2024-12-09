#pragma once

#include <cstdint>
#include <iostream>
#include <stdexcept>

#include "vec.h"

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

        Type& operator()(size_t row, size_t column) {
            return data[column * Y + row];
        }

        const Type& operator()(size_t row, size_t column) const {
            return data[column * Y + row];
        }

        Type& get(size_t row, size_t column) {
            return data[column * Y + row];
        }

        const Type& get(size_t row, size_t column) const {
            return data[column * Y + row];
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
            out *= static_cast<Type>(scalar);
            return out;
        }

        _Mat& operator*=(double scalar) {
            for (size_t i{ 0 }; i < Y * X; ++i) {
                data[i] *= static_cast<Type>(scalar);
            }
            return *this;
        }

        _Mat operator/(double scalar) const {
            _Mat out{ *this };
            out /= static_cast<Type>(scalar);
            return out;
        }

        _Mat& operator/=(double scalar) {
            for (size_t i{ 0 }; i < Y * X; ++i) {
                data[i] /= static_cast<Type>(scalar);
            }
            return *this;
        }

        template<size_t X2>
        _Mat<Y, X2, Type> operator*(const _Mat<X, X2, Type>& other) const {
            _Mat<Y, X2, Type> out{ 0 };

            for (size_t i{ 0 }; i < Y; ++i) {
                for (size_t j{ 0 }; j < X2; ++j) {
                    for (size_t k{ 0 }; k < X; ++k) {
                        out(i, j) += (*this)(i, k) * other(k, j);
                    }
                }
            }

            return out;
        }

        bool operator==(const _Mat& other) const {
            for (size_t i{ 0 }; i < Y * X; ++i) {
                if (data[i] != other.data[i]) {
                    return false;
                }
            }
            return true;
        }

        bool operator!=(const _Mat& other) const {
            return !((*this) == other);
        }

        _Mat<Y, X, Type> transposed() const {
            _Mat<X, Y, Type> out{};
            for (size_t i = 0; i < Y; ++i) {
                for (size_t j = 0; j < X; ++j) {
                    out(j, i) = (*this)(i, j);
                }
            }
            return out;
        }

        static _Mat identity() {
            _Mat out{ 0 };
            constexpr size_t BIGGER{ X > Y ? X : Y };
            for (size_t i{}; i < BIGGER; ++i) {
                out(i, i) = static_cast<Type>(1);
            }
            return out;
        }

        Type determinant() const {
            if constexpr (Y != X) {
                throw std::logic_error("Determinant is defined only for square matrices.");
            }

            if constexpr (Y == 2) {
                return data[0] * data[3] - data[1] * data[2];
            }
            else if constexpr (Y == 3) {
                return data[0] * (data[4] * data[8] - data[5] * data[7])
                    - data[1] * (data[3] * data[8] - data[5] * data[6])
                    + data[2] * (data[3] * data[7] - data[4] * data[6]);
            }
            else if constexpr (Y == 4) {
                return data[0] * (data[5] * (data[10] * data[15] - data[11] * data[14]) -
                    data[6] * (data[9] * data[15] - data[11] * data[13]) +
                    data[7] * (data[9] * data[14] - data[10] * data[13]))
                    - data[1] * (data[4] * (data[10] * data[15] - data[11] * data[14]) -
                        data[6] * (data[8] * data[15] - data[11] * data[12]) +
                        data[7] * (data[8] * data[14] - data[10] * data[12]))
                    + data[2] * (data[4] * (data[9] * data[15] - data[11] * data[13]) -
                        data[5] * (data[8] * data[15] - data[11] * data[12]) +
                        data[7] * (data[8] * data[13] - data[9] * data[12]))
                    - data[3] * (data[4] * (data[9] * data[14] - data[10] * data[13]) -
                        data[5] * (data[8] * data[14] - data[10] * data[12]) +
                        data[6] * (data[8] * data[13] - data[9] * data[12]));
            }
            else {
                Type det{ 0 };
                for (size_t col{ 0 }; col < X; ++col) {
                    _Mat<Y - 1, X - 1, Type> submatrix;
                    size_t submatrix_row{ 0 };
                    for (size_t row{ 1 }; row < Y; ++row) {
                        size_t submatrix_col{ 0 };
                        for (size_t j{ 0 }; j < X; ++j) {
                            if (j == col) continue;
                            submatrix(submatrix_row, submatrix_col) = (*this)(row, j);
                            ++submatrix_col;
                        }
                        ++submatrix_row;
                    }

                    det += (col % 2 == 0 ? 1 : -1) * (*this)(0, col) * submatrix.determinant();
                }
                return det;
            }
        }

        _Mat cofactor() const {
            if constexpr (Y != X) {
                throw std::logic_error("Cofactor is defined only for square matrices.");
            }

            _Mat cofactorMatrix;

            for (size_t row{ 0 }; row < Y; ++row) {
                for (size_t col{ 0 }; col < X; ++col) {
                    _Mat<X - 1, Y - 1, Type> minorMatrix;
                    size_t minorRow{ 0 };

                    for (size_t i{ 0 }; i < Y; ++i) {
                        if (i == row) continue;
                        size_t minorCol{ 0 };

                        for (size_t j{ 0 }; j < X; ++j) {
                            if (j == col) continue;
                            minorMatrix(minorRow, minorCol) = (*this)(i, j);
                            ++minorCol;
                        }

                        ++minorRow;
                    }

                    Type minorDeterminant{ minorMatrix.determinant() };

                    cofactorMatrix(row, col) = ((row + col) % 2 == 0 ? 1 : -1) * minorDeterminant;
                }
            }

            return cofactorMatrix;
        }

        _Mat inverse() const {
            if constexpr (Y != X) {
                throw std::logic_error("Inverse is defined only for square matrices.");
            }

            Type det = this->determinant();
            if (det == 0) {
                return _Mat::identity();
            }

            _Mat adj = this->cofactor().transposed();
            return adj / det;
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
    inline std::ostream& operator<<(std::ostream& os, const _Mat<Y, X, Type>& matrix) {
        os << "{";
        for (size_t i{ 0 }; i < X; ++i) {
            for (size_t j{ 0 }; j < Y; ++j) {
                os << matrix(i, j) << ",";
            }
            if (i != X - 1) {
                os << "\n";
            }
        }
        os << "}\n";

        return os;
    }

    template<typename Type>
    inline _Vec2<Type> operator*(const _Mat<3, 3, Type>& mat, const _Vec2<Type>& vec) {
        return _Vec2<Type>(
            mat(0, 0) * vec.x + mat(0, 1) * vec.y,
            mat(1, 0) * vec.x + mat(1, 1) * vec.y
        );
    }

    template<typename Type>
    inline _Vec3<Type> operator*(const _Mat<3, 3, Type>& mat, const _Vec3<Type>& vec) {
        return _Vec3<Type>(
            mat(0, 0) * vec.x + mat(0, 1) * vec.y + mat(0, 2) * vec.z,
            mat(1, 0) * vec.x + mat(1, 1) * vec.y + mat(1, 2) * vec.z,
            mat(2, 0) * vec.x + mat(2, 1) * vec.y + mat(2, 2) * vec.z
        );
    }

    template<typename Type>
    inline _Vec4<Type> operator*(const _Mat<4, 4, Type>& mat, const _Vec4<Type>& vec) {
        return _Vec4<Type>(
            mat(0, 0) * vec.x + mat(0, 1) * vec.y + mat(0, 2) * vec.z + mat(0, 3) * vec.w,
            mat(1, 0) * vec.x + mat(1, 1) * vec.y + mat(1, 2) * vec.z + mat(1, 3) * vec.w,
            mat(2, 0) * vec.x + mat(2, 1) * vec.y + mat(2, 2) * vec.z + mat(2, 3) * vec.w,
            mat(3, 0) * vec.x + mat(3, 1) * vec.y + mat(3, 2) * vec.z + mat(3, 3) * vec.w
        );
    }

}
