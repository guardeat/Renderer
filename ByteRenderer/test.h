#pragma once

#include <chrono>
#include <exception>

#include "stb_image.h"

#include "renderer.h"
#include "window.h"

namespace Byte {

	class FPSCamera {
	private:
		float yaw{};
		float pitch{};
		float oldX{};
		float oldY{};

		float speed{ 50.0f };
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

		void update(Window& window, Transform& transform, float dt) {

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
				transform.position(transform.position() + offset.normalized() * speed * dt);
			}

			glfwSetInputMode(window.glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
	};

	struct Loader {
		static TextureData loadTexture(const Path& path) {
			TextureData texture;
			texture.path = path;

			int width, height, channels;
			std::string pathString{ path.string() };
			uint8_t* imgData{ stbi_load(pathString.c_str(), &width, &height, &channels, 0) };

			if (!imgData) {
				throw std::exception{ "Failed to load texture" };
			}

			texture.width = static_cast<size_t>(width);
			texture.height = static_cast<size_t>(height);
			texture.channels = static_cast<size_t>(channels);

			size_t dataSize{ static_cast<size_t>(width * height * channels) };
			texture.data = std::make_unique<uint8_t[]>(dataSize);
			std::memcpy(texture.data.get(), imgData, dataSize);

			stbi_image_free(imgData);

			return texture;
		}
	};

}