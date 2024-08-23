#pragma once

#include <cstdint>
#include <iostream>
#include <cmath>

namespace Byte {

	template<typename Type>
	struct _Vec2 {
		Type x{};
		Type y{};

		_Vec2() = default;

		_Vec2(Type x, Type y)
			:x{ x }, y{ y } {
		}

		_Vec2(const _Vec2& other)
			:x{ other.x }, y{ other.y } {
		}

		_Vec2 operator+(const _Vec2& other) const {
			_Vec2 out{ *this };
			out += other;

			return out;
		}

		_Vec2 operator-(const _Vec2& other) const {
			_Vec2 out{ *this };
			out -= other;

			return out;
		}

		_Vec2& operator+=(const _Vec2& other) {
			x += other.x;
			y += other.y;

			return *this;
		}

		_Vec2& operator-=(const _Vec2& other) {
			x -= other.x;
			y -= other.y;

			return *this;
		}

		template<typename Scalar>
		_Vec2 operator*(Scalar scalar) const {
			_Vec2 out{ *this };
			out *= static_cast<Type>(scalar);
			return out;
		}

		template<typename Scalar>
		_Vec2& operator*=(Scalar scalar) {
			x *= static_cast<Type>(scalar);
			y *= static_cast<Type>(scalar);

			return *this;
		}

		template<typename Scalar>
		_Vec2 operator/(Scalar scalar) const {
			_Vec2 out{ *this };
			out /= static_cast<Type>(scalar);
			return out;
		}

		template<typename Scalar>
		_Vec2& operator/=(Scalar scalar) {
			x /= static_cast<Type>(scalar);
			y /= static_cast<Type>(scalar);

			return *this;
		}

		_Vec2 operator*(const _Vec2& other) const {
			_Vec2 out{ x * other.x, y * other.y };
			return out;
		}

		_Vec2& operator*=(const _Vec2& other) {
			x *= other.x;
			y *= other.y;

			return *this;
		}

		_Vec2 operator/(const _Vec2& other) const {
			_Vec2 out{ x / other.x, y / other.y };
			return out;
		}

		_Vec2& operator/=(const _Vec2& other) {
			x /= other.x;
			y /= other.y;

			return *this;
		}

		_Vec2 operator-() const {
			return _Vec2{ -x,-y };
		}

		bool operator==(const _Vec2& other) const {
			return x == other.x && y = other.y;
		}

		bool operator!=(const _Vec2& other) const {
			return !((*this) == other);
		}

		template<typename LengthType = Type>
		LengthType length() const {
			return static_cast<LengthType>(std::sqrt(x * x + y * y));
		}

		void normalize() {
			*this = normalized();
		}

		_Vec2 normalized() const {
			Type len{ length() };

			if (len == 0) {
				return _Vec2{};
			}

			return _Vec2{ x / len, y / len };
		}

		Type dot(const _Vec2& other) const {
			return x * other.x + y * other.y;
		}
	};

	template<typename Type>
	struct _Vec3 {
		Type x{};
		Type y{};
		Type z{};

		_Vec3() = default;

		_Vec3(Type x, Type y, Type z)
			:x{ x }, y{ y }, z{ z } {
		}

		_Vec3(const _Vec3& other)
			:x{ other.x }, y{ other.y }, z{ other.z } {
		}

		_Vec3 operator+(const _Vec3& other) const {
			_Vec3 out{ *this };
			out += other;

			return out;
		}

		_Vec3 operator-(const _Vec3& other) const {
			_Vec3 out{ *this };
			out -= other;

			return out;
		}

		_Vec3& operator+=(const _Vec3& other) {
			x += other.x;
			y += other.y;
			z += other.z;

			return *this;
		}

		_Vec3& operator-=(const _Vec3& other) {
			x -= other.x;
			y -= other.y;
			z -= other.z;

			return *this;
		}

		template<typename Scalar>
		_Vec3 operator*(Scalar scalar) const {
			_Vec3 out{ *this };
			out *= static_cast<Type>(scalar);
			return out;
		}

		template<typename Scalar>
		_Vec3& operator*=(Scalar scalar) {
			x *= static_cast<Type>(scalar);
			y *= static_cast<Type>(scalar);
			z *= static_cast<Type>(scalar);

			return *this;
		}

		template<typename Scalar>
		_Vec3 operator/(Scalar scalar) const {
			_Vec3 out{ *this };
			out /= static_cast<Type>(scalar);
			return out;
		}

		template<typename Scalar>
		_Vec3& operator/=(Scalar scalar) {
			x /= static_cast<Type>(scalar);
			y /= static_cast<Type>(scalar);
			z /= static_cast<Type>(scalar);

			return *this;
		}

		_Vec3 operator*(const _Vec3& other) const {
			_Vec3 out{ x * other.x, y * other.y, z * other.z };
			return out;
		}

		_Vec3& operator*=(const _Vec3& other) {
			x *= other.x;
			y *= other.y;
			z *= other.z;

			return *this;
		}

		_Vec3 operator/(const _Vec3& other) const {
			_Vec3 out{ x / other.x, y / other.y, z / other.z };
			return out;
		}

		_Vec3& operator/=(const _Vec3& other) {
			x /= other.x;
			y /= other.y;
			z /= other.z;

			return *this;
		}

		_Vec3 operator-() const {
			return _Vec3{ -x,-y,-z };
		}

		bool operator==(const _Vec3& other) const {
			return x == other.x && y = other.y && z == other.z;
		}

		bool operator!=(const _Vec3& other) const {
			return !((*this) == other);
		}

		template<typename LengthType = Type>
		LengthType length() const {
			return static_cast<LengthType>(std::sqrt(x * x + y * y + z * z));
		}

		void normalize() {
			*this = normalized();
		}

		_Vec3 normalized() const {
			Type len{ length() };

			if (len == 0) {
				return _Vec3{};
			}

			return _Vec3{ x / len, y / len, z / len };
		}

		Type dot(const _Vec3& other) const {
			return x * other.x + y * other.y + z * other.z;
		}

		_Vec3 cross(const _Vec3& other) const {
			return _Vec3{
				y * other.z - z * other.y,
				z * other.x - x * other.z,
				x * other.y - y * other.x };
		}
	};

	template<typename Type>
	struct _Vec4 {
		Type x{};
		Type y{};
		Type z{};
		Type w{};

		_Vec4() = default;

		_Vec4(Type x, Type y, Type z, Type w)
			:x{ x }, y{ y }, z{ z }, w{ w } {
		}

		_Vec4(const _Vec4& other)
			:x{ other.x }, y{ other.y }, z{ other.z }, w{ other.w } {
		}

		_Vec4 operator+(const _Vec4& other) const {
			_Vec4 out{ *this };
			out += other;

			return out;
		}

		_Vec4 operator-(const _Vec4& other) const {
			_Vec4 out{ *this };
			out -= other;

			return out;
		}

		_Vec4& operator+=(const _Vec4& other) {
			x += other.x;
			y += other.y;
			z += other.z;
			w += other.w;

			return *this;
		}

		_Vec4& operator-=(const _Vec4& other) {
			x -= other.x;
			y -= other.y;
			z -= other.z;
			w -= other.w;

			return *this;
		}

		template<typename Scalar>
		_Vec4 operator*(Scalar scalar) const {
			_Vec4 out{ *this };
			out *= static_cast<Type>(scalar);
			return out;
		}

		template<typename Scalar>
		_Vec4& operator*=(Scalar scalar) {
			x *= static_cast<Type>(scalar);
			y *= static_cast<Type>(scalar);
			z *= static_cast<Type>(scalar);
			w *= static_cast<Type>(scalar);

			return *this;
		}

		template<typename Scalar>
		_Vec4 operator/(Scalar scalar) const {
			_Vec4 out{ *this };
			out /= static_cast<Type>(scalar);
			return out;
		}

		template<typename Scalar>
		_Vec4& operator/=(Scalar scalar) {
			x /= static_cast<Type>(scalar);
			y /= static_cast<Type>(scalar);
			z /= static_cast<Type>(scalar);
			w /= static_cast<Type>(scalar);

			return *this;
		}

		_Vec4 operator*(const _Vec4& other) const {
			_Vec4 out{ x * other.x, y * other.y, z * other.z, w * other.w };
			return out;
		}

		_Vec4& operator*=(const _Vec4& other) {
			x *= other.x;
			y *= other.y;
			z *= other.z;
			w *= other.w;

			return *this;
		}

		_Vec4 operator/(const _Vec4& other) const {
			_Vec4 out{ x / other.x, y / other.y, z / other.z, w / other.w };
			return out;
		}

		_Vec4& operator/=(const _Vec4& other) {
			x /= other.x;
			y /= other.y;
			z /= other.z;
			w /= other.w;

			return *this;
		}

		_Vec4 operator-() const {
			return _Vec4{ -x,-y,-z,-w };
		}

		bool operator==(const _Vec4& other) const {
			return x == other.x && y = other.y && z == other.z && w == other.w;
		}

		bool operator!=(const _Vec4& other) const {
			return !((*this) == other);
		}

		template<typename LengthType = Type>
		LengthType length() const {
			return static_cast<LengthType>(std::sqrt(x * x + y * y + z * z + w * w));
		}

		void normalize() {
			*this = normalized();
		}

		_Vec4 normalized() const {
			Type len{ length() };

			if (len == 0) {
				return _Vec4{};
			}

			return _Vec4{ x / len, y / len, z / len, w / len };
		}

		Type dot(const _Vec4& other) const {
			return x * other.x + y * other.y + z * other.z + w * other.w;
		}
	};

	using Vec2 = _Vec2<float>;
	using Vec3 = _Vec3<float>;
	using Vec4 = _Vec4<float>;

	using Vec2d = _Vec2<double>;
	using Vec3d = _Vec3<double>;
	using Vec4d = _Vec4<double>;

	template<typename Type>
	inline std::ostream& operator<<(std::ostream& os, const _Vec2<Type>& vec2) {
		os << "{" << vec2.x << ", " << vec2.y << "}";
		return os;
	}

	template<typename Type>
	inline std::ostream& operator<<(std::ostream& os, const _Vec3<Type>& vec3) {
		os << "{" << vec3.x << ", " << vec3.y << ", " << vec3.z << "}";
		return os;
	}

	template<typename Type>
	inline std::ostream& operator<<(std::ostream& os, const _Vec4<Type>& vec4) {
		os << "{" << vec4.x << ", " << vec4.y << ", " << vec4.z << ", " << vec4.w << "}";
		return os;
	}
}
