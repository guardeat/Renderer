#pragma once

#include "mat.h"
#include "trigonometry.h"
#include "vec.h"
#include "transform.h"

namespace Byte {

    class Camera {
    private:
        float fov{ 45.0f };
        float nearPlane{ 0.5f };
        float farPlane{ 400.0f };

    public:
        Camera() = default;

        Camera(float fov, float nearPlane, float farPlane)
            : fov{ fov }, nearPlane{ nearPlane }, farPlane{ farPlane } {
        }

        Mat4 perspective(float aspectRatio) const {
            Mat4 out{ 0 };
            float tanHalfFov{ std::tan(radians(fov) / 2.0f) };

            out[0][0] = 1.0f / (aspectRatio * tanHalfFov);
            out[1][1] = 1.0f / tanHalfFov;
            out[2][2] = -(farPlane + nearPlane) / (farPlane - nearPlane);
            out[3][2] = -1.0f;
            out[2][3] = -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane);

            return out;
        }

        Mat4 orthographic(float left, float right, float bottom, float top) const {
            Mat4 out{ 0 };

            out[0][0] = 2.0f / (right - left);
            out[1][1] = 2.0f / (top - bottom);
            out[2][2] = -2.0f / (farPlane - nearPlane);
            out[0][3] = -(right + left) / (right - left);
            out[1][3] = -(top + bottom) / (top - bottom);
            out[2][3] = -(farPlane + nearPlane) / (farPlane - nearPlane);
            out[3][3] = 1.0f;

            return out;
        }

        Mat4 view(const Transform& transform) const {
            Vec3 position{ transform.position() };
            Vec3 r{ transform.right() };
            Vec3 u{ transform.up() };
            Vec3 f{ -transform.front() };

            Mat4 dir{ 0.0f };

            dir[0][0] = r.x;
            dir[0][1] = r.y;
            dir[0][2] = r.z;
            dir[1][0] = u.x;
            dir[1][1] = u.y;
            dir[1][2] = u.z;
            dir[2][0] = f.x;
            dir[2][1] = f.y;
            dir[2][2] = f.z;
            dir[3][3] = 1.0f;

            Mat4 pos{ Mat4::identity() };

            pos[0][3] = -position.x;
            pos[1][3] = -position.y;
            pos[2][3] = -position.z;

            return dir * pos;
        }
    };

}
