#pragma once

#include "core/transform.h"
#include "math/mat.h"
#include "math/vec.h"

namespace Byte {

    class Camera {
    private:
        float _fov{ 45.0f };
        float _nearPlane{ 0.5f };
        float _farPlane{ 600.0f };

    public:
        Camera() = default;

        Camera(float fov, float nearPlane, float farPlane)
            : _fov{ fov }, _nearPlane{ nearPlane }, _farPlane{ farPlane } {
        }

        float nearPlane() const {
            return _nearPlane;
        }

        float farPlane() const {
            return _farPlane;
        }

        float fov() const {
            return _fov;
        }

        float nearPlane(float near) {
            return _nearPlane = near;
        }

        float farPlane(float far) {
            return _farPlane = far;
        }

        float fov(float fov) {
            return _fov = fov;
        }

        Mat4 perspective(float aspectRatio, float near, float far) const {
            return Mat4::perspective(aspectRatio,_fov,near,far);
        }

        Mat4 perspective(float aspectRatio) const {
            return Mat4::perspective(aspectRatio,_fov,_nearPlane,_farPlane);
        }

        Mat4 orthographic(float left, float right, float bottom, float top, float near, float far) const {
            return Mat4::orthographic(left, right, bottom, top, near, far);
        }

        Mat4 orthographic(float left, float right, float bottom, float top) const {
            return Mat4::orthographic(left,right,bottom,top,_nearPlane,_farPlane);
        }
    };

}
