#pragma once

#include <cstdint>
#include <string>
#include <exception>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "opengl_api.h"

namespace Byte {

	struct Window {
		GLFWwindow* glfwWindow = nullptr;

		Window() = default;

		Window(size_t width, size_t height, const std::string& title) {
			glfwWindow = glfwCreateWindow(
				static_cast<int>(width),
				static_cast<int>(height),
				title.c_str(),
				nullptr,
				nullptr);
			if (glfwWindow == nullptr) {
				throw std::exception("Failed to create GLFW window");
			}
		}

		~Window() {
			glfwDestroyWindow(glfwWindow);
		}

		size_t width() const {
			int width, height;
			glfwGetWindowSize(glfwWindow, &width, &height);

			return static_cast<size_t>(width);
		}

		size_t height() const {
			int width, height;
			glfwGetWindowSize(glfwWindow, &width, &height);

			return static_cast<size_t>(height);
		}
	};

}
