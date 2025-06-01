#pragma once

#include <cmath>
#include <iostream>

#include "vec.h"
#include "trigonometry.h"

namespace Byte {

	template<typename Type>
	class _Quaternion {
	public:
		Type w;
		Type x;
		Type y;
		Type z;

	public:
		_Quaternion()
			:w{ 1 }, x{ 0 }, y{ 0 }, z{ 0 } {
		}

		_Quaternion(Type w, Type x, Type y, Type z)
			:w{ w }, x{ x }, y{ y }, z{ z } {
		}

		_Quaternion(const _Vec3<Type>& angles) {
			Type cx{ std::cos(radians(angles.x / 2)) };
			Type sx{ std::sin(radians(angles.x / 2)) };
			Type cy{ std::cos(radians(angles.y / 2)) };
			Type sy{ std::sin(radians(angles.y / 2)) };
			Type cz{ std::cos(radians(angles.z / 2)) };
			Type sz{ std::sin(radians(angles.z / 2)) };

			w = cx * cy * cz + sx * sy * sz;
			x = sx * cy * cz - cx * sy * sz;
			y = cx * sy * cz + sx * cy * sz;
			z = cx * cy * sz - sx * sy * cz;

			normalize();
		}

		_Quaternion(Type xAngle, Type yAngle, Type zAngle)
			:_Quaternion{ _Vec3<Type>{xAngle,yAngle,zAngle} } {
		}

		_Quaternion(const _Vec3<Type>& source, const _Vec3<Type>& dest) {
			_Vec3<Type> cross{ source.cross(dest) };
			x = cross.x;
			y = cross.y;
			z = cross.z;

			double sourceL{ source.length() };
			double destL{ dest.length() };

			w = static_cast<Type>(std::sqrt(sourceL * sourceL + destL * destL) + source.dot(dest));

			normalize();
		}

		_Quaternion(const Vec3& axis, Type angle) {
			Vec3 n{ axis.normalized() };
			Type halfAngle{ angle * 0.5f };

			Type s{ std::sin(radians(halfAngle)) };
			Type c{ std::cos(radians(halfAngle)) };

			w = c;
			x = s * n.x;
			y = s * n.y;
			z = s * n.z;
		}

		_Quaternion operator*(const _Quaternion& other) const {
			return {
			   w * other.w - x * other.x - y * other.y - z * other.z,
			   w * other.x + x * other.w + y * other.z - z * other.y,
			   w * other.y - x * other.z + y * other.w + z * other.x,
			   w * other.z + x * other.y - y * other.x + z * other.w
			};
		}

		_Vec3<Type> operator*(const _Vec3<Type>& vec3) const {
			_Quaternion result{ (*this) * _Quaternion(0, vec3.x, vec3.y, vec3.z) * conjugated() };

			return _Vec3<Type>{ result.x, result.y, result.z };
		}

		void operator*=(const _Quaternion& other) {
			*this = other * (*this);
		}

		void conjugate() {
			(*this) = conjugated();
		}

		_Quaternion conjugated() const {
			return _Quaternion{ w, -x, -y, -z };
		}

		double length() const {
			return std::sqrt(w * w + x * x + y * y + z * z);
		}

		void normalize() {
			*this = normalized();
		}

		_Quaternion normalized() const {
			Type len{ static_cast<Type>(length()) };
			return _Quaternion{ w / len, x / len, y / len, z / len };
		}
	};

	using Quaternion = _Quaternion<float>;
	using Quaterniond = _Quaternion<double>;

	template<typename Type>
	inline std::ostream& operator<<(std::ostream& os, const _Quaternion<Type>& quat) {
		os << "{" << quat.x << ", " << quat.y << ", " << quat.z << ", " << quat.w << "}";
		return os;
	}

}