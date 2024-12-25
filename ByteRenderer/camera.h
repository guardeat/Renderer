#pragma once

#include "mat.h"
#include "trigonometry.h"
#include "vec.h"
#include "transform.h"

namespace Byte {

    class Camera {
    private:
        float _fov{ 45.0f };
        float _nearPlane{ 0.5f };
        float _farPlane{ 500.0f };

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

        Mat4 perspective(float aspectRatio, float near, float far) const {
            Mat4 out{ 0 };
            float tanHalfFov{ std::tan(radians(_fov) / 2.0f) };

            out(0, 0) = 1.0f / (aspectRatio * tanHalfFov);
            out(1, 1) = 1.0f / tanHalfFov;
            out(2, 2) = -(far + near) / (far - near);
            out(3, 2) = -1.0f;
            out(2, 3) = -(2.0f * far * near) / (far - near);

            return out;
        }

        Mat4 perspective(float aspectRatio) const {
            return perspective(aspectRatio,_nearPlane,_farPlane);
        }

        Mat4 orthographic(float left, float right, float bottom, float top, float near, float far) const {
            Mat4 out{ 0 };

            out(0, 0) = 2.0f / (right - left);
            out(1, 1) = 2.0f / (top - bottom);
            out(2, 2) = -2.0f / (far - near);
            out(0, 3) = -(right + left) / (right - left);
            out(1, 3) = -(top + bottom) / (top - bottom);
            out(2, 3) = -(far + near) / (far - near);
            out(3, 3) = 1.0f;

            return out;
        }

        Mat4 orthographic(float left, float right, float bottom, float top) const {
            return orthographic(left, right, bottom, top, _nearPlane, _farPlane);
        }

        Mat4 frustumSpace(const Mat4& projection, const Mat4& view, const Transform& lightTransform) const {
            const auto inv{ (projection * view).inverse() };

            Buffer<Vec4> corners;
            for (unsigned int x = 0; x < 2; ++x) {
                for (unsigned int y = 0; y < 2; ++y) {
                    for (unsigned int z = 0; z < 2; ++z) {
                        const Vec4 pt{
                            inv * Vec4(
                                2.0f * x - 1.0f,
                                2.0f * y - 1.0f,
                                2.0f * z - 1.0f,
                                1.0f) };
                        corners.push_back(pt / pt.w);
                    }
                }
            }

            Vec3 center{};
            for (const auto& v : corners) {
                center += Vec3(v.x, v.y, v.z);
            }
            center /= corners.size();

            const auto lightView{ Mat4::lookAt(
                center - lightTransform.front(),
                center,
                lightTransform.up()
            ) };

            float minX{ std::numeric_limits<float>::max() };
            float maxX{ std::numeric_limits<float>::lowest() };
            float minY{ std::numeric_limits<float>::max() };
            float maxY{ std::numeric_limits<float>::lowest() };
            float minZ{ std::numeric_limits<float>::max() };
            float maxZ{ std::numeric_limits<float>::lowest() };

            for (const auto& v : corners) {
                const auto trf{ lightView * v };
                minX = std::min(minX, trf.x);
                maxX = std::max(maxX, trf.x);
                minY = std::min(minY, trf.y);
                maxY = std::max(maxY, trf.y);
                minZ = std::min(minZ, trf.z);
                maxZ = std::max(maxZ, trf.z);
            }

            float zMult{ _farPlane / 100.0f };
            if (minZ < 0)
            {
                minZ *= zMult;
            }
            else
            {
                minZ /= zMult;
            }
            if (maxZ < 0)
            {
                maxZ /= zMult;
            }
            else
            {
                maxZ *= zMult;
            }

            Mat4 lightProjection{ orthographic(minX, maxX, minY, maxY, minZ, maxZ) };

            return lightProjection * lightView;
        }
    };

}
