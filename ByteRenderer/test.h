#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "vec.h"
#include "quaternion.h"
#include "window.h"
#include "transform.h"

namespace Byte {

    inline Mesh createSphere(float radius, int numSegments) {
        std::vector<float> vertices;
        std::vector<float> normals;
        std::vector<float> texCoords;
        std::vector<uint32_t> indices;

        for (size_t i{}; i <= numSegments; ++i) {
            float phi = pi<float>() * static_cast<float>(i) / numSegments;
            for (size_t j{}; j <= numSegments; ++j) {
                float theta = 2.0f * pi<float>() * static_cast<float>(j) / numSegments;

                float x = radius * std::sin(phi) * std::cos(theta);
                float y = radius * std::sin(phi) * std::sin(theta);
                float z = radius * std::cos(phi);

                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);

                Vec3 normal{ Vec3(x, y, z).normalized() };
                normals.push_back(normal.x);
                normals.push_back(normal.y);
                normals.push_back(normal.z);

                float u = static_cast<float>(j) / numSegments;
                float v = static_cast<float>(i) / numSegments;

                texCoords.push_back(u);
                texCoords.push_back(v);
            }
        }

        for (size_t i{}; i < numSegments; ++i) {
            for (size_t j{}; j < numSegments; ++j) {
                uint32_t first{ static_cast<uint32_t>(i * (numSegments + 1) + j) };
                uint32_t second{ static_cast<uint32_t>(first + numSegments + 1) };

                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(first + 1);

                indices.push_back(second);
                indices.push_back(second + 1);
                indices.push_back(first + 1);
            }
        }

        return Mesh{ std::move(vertices), std::move(normals), std::move(texCoords), std::move(indices) };
    }

    inline Mesh createPlane(float width, float height, int numSegments) {
        std::vector<float> vertices;
        std::vector<float> normals;
        std::vector<float> texCoords;
        std::vector<uint32_t> indices;

        for (size_t i{}; i <= numSegments; ++i) {
            float y = static_cast<float>(i) / numSegments * height - height / 2.0f;
            for (size_t j{}; j <= numSegments; ++j) {
                float x = static_cast<float>(j) / numSegments * width - width / 2.0f;
                float z = 0.0f;

                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);

                normals.push_back(0.0f);
                normals.push_back(0.0f);
                normals.push_back(1.0f);

                float u = static_cast<float>(j) / numSegments;
                float v = static_cast<float>(i) / numSegments;

                texCoords.push_back(u);
                texCoords.push_back(v);
            }
        }

        for (size_t i{}; i < numSegments; ++i) {
            for (size_t j{}; j < numSegments; ++j) {
                uint32_t topLeft = static_cast<uint32_t>(i * (numSegments + 1) + j);
                uint32_t topRight = topLeft + 1;
                uint32_t bottomLeft = static_cast<uint32_t>((i + 1) * (numSegments + 1) + j);
                uint32_t bottomRight = bottomLeft + 1;

                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }

        return  Mesh{ std::move(vertices), std::move(normals), std::move(texCoords), std::move(indices) };
    }

    class FPSCamera {
    private:
        float yaw{};
        float pitch{};
        float oldX{};
        float oldY{};

        float speed{ 0.02f };
        float sensitivity{ 0.1f };

    public:
        FPSCamera() = default;

        FPSCamera(float speed, float sensitivity)
            :speed{ speed }, sensitivity{ sensitivity } {
        }

        Quaternion calculateRotation(float newX, float newY) {
            float offsetX{ newX - oldX };
            float offsetY{ newY - oldY };
            oldX = newX;
            oldY = newY;

            offsetX *= sensitivity;
            offsetY *= sensitivity;

            yaw -= offsetX;
            pitch -= offsetY;

            if (pitch > 89.0f)
            {
                pitch = 89.0f;
            }
            if (pitch < -89.0f)
            {
                pitch = -89.0f;
            }

            Quaternion pitchQuaternion(Vec3{ 1, 0, 0 }, pitch);
            Quaternion yawQuaternion(Vec3{ 0, 1, 0 }, yaw);

            return yawQuaternion * pitchQuaternion;
        }

        void update(Window& window, Transform& transform) {

            double xpos, ypos;
            glfwGetCursorPos(window.glfwWindow, &xpos, &ypos);

            transform.rotation(calculateRotation(static_cast<float>(xpos), static_cast<float>(ypos)));

            Vec3 offset{};
            if (glfwGetKey(window.glfwWindow, GLFW_KEY_W) == GLFW_PRESS) {
                offset += transform.front();
            }
            if (glfwGetKey(window.glfwWindow, GLFW_KEY_S) == GLFW_PRESS) {
                offset -= transform.front();
            }
            if (glfwGetKey(window.glfwWindow, GLFW_KEY_A) == GLFW_PRESS) {
                offset -= transform.right();
            }
            if (glfwGetKey(window.glfwWindow, GLFW_KEY_D) == GLFW_PRESS) {
                offset += transform.right();
            }

            if (offset.length() > 0) {
                transform.position(transform.position() + offset.normalized() * speed);
            }

            glfwSetInputMode(window.glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    };
}