#pragma once

#include "vec.h"
#include "quaternion.h"

namespace Byte {

	class Transform {
	private:
		bool _changed{ true };

		Vec3 _localPos;
		Vec3 _localScale{ 1, 1, 1 };
		Quaternion _localRot;

		Vec3 _globalPos;
		Vec3 _globalScale{ 1, 1, 1 };
		Quaternion _globalRot;

		Vec3 _oldParentScale{ 1,1,1 };
		Quaternion _oldParentRot;

	public:
		Transform() = default;

		const Vec3& position() const {
			return _globalPos;
		}

		const Vec3& scale() const {
			return _globalScale;
		}

		const Quaternion& rotation() const {
			return _globalRot;
		}

		const Vec3& localPosition() const {
			return _localPos;
		}

		const Vec3& localScale() const {
			return _globalScale;
		}

		const Quaternion& localRotation() const {
			return _localRot;
		}

		void position(const Vec3& newPos) {
			Vec3 oldLocal{ _localPos };
			_localPos += newPos - _globalPos;
			_globalPos -= oldLocal - _localPos;
			_changed = true;
		}

		void scale(const Vec3& newScale) {
			Vec3 oldLocal{ _localScale };
			_localScale *= newScale / _globalScale;
			_globalScale /= oldLocal / _localScale;
			_changed = true;
		}

		void rotation(const Vec3& euler) {
			rotation(Quaternion{ euler });
		}

		void rotation(const Quaternion& quat) {
			_globalRot = _localRot.conjugated() * _globalRot;
			_localRot = quat;
			_globalRot = quat * _globalRot;

			_localRot.normalize();
			_globalRot.normalize();
			_changed = true;
		}

		void rotate(const Vec3& euler) {
			rotate(Quaternion{ euler });
		}

		void rotate(const Quaternion& quat) {
			_globalRot = quat * _globalRot;
			_localRot = quat * _localRot;

			_localRot.normalize();
			_globalRot.normalize();
			_changed = true;
		}

		Vec3 front() const {
			return _globalRot * Vec3{ 0,0,-1 };
		}

		Vec3 up() const {
			return _globalRot * Vec3{ 0,1,0 };
		}

		Vec3 right() const {
			return _globalRot * Vec3{ 1,0,0 };
		}

		Mat4 view() {
			Vec3 f{ front().normalized() };
			Vec3 r{ right().normalized() };

			Vec3 u{ r.cross(f) };

			return Mat4::lookAt(_globalPos, _globalPos + f, u);
		}
	};

}