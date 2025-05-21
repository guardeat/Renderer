#include <chrono>

#include "test.h"
#include "render.h"

using namespace Byte;

extern "C" {
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

int main() {
	glfwInit();

	Window window{ 1336,768,"Test" };

	Renderer renderer{ RendererGenerator::deferred(window) };

	FPSCamera fpsCamera;

	Scene scene{ buildCustomScene(renderer) };

	std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << "\n";
	std::cout << "Version: " << glGetString(GL_VERSION) << "\n";

	auto lastTime{ std::chrono::high_resolution_clock::now() };
	int frameCount{ 0 };
	float fpsTimer{ 0.0f };
	glClearColor(0.0f, 0.1f, 0.8f,1.0f);
	renderer.load();

	while (!glfwWindowShouldClose(window.glfwWindow)) {
		auto currentTime{ std::chrono::high_resolution_clock::now() };
		float deltaTime{ std::chrono::duration<float>(currentTime - lastTime).count() };
		lastTime = currentTime;

		renderer.context().input<float>("uTime") += deltaTime;

		renderer.render();
		renderer.update(window);
		fpsCamera.update(window, scene.cameraTransform, deltaTime);
		glfwPollEvents();

		frameCount++;
		fpsTimer += deltaTime;

		if (fpsTimer >= 1.0f) {
			std::cout << "FPS: " << frameCount << std::endl;
			GLenum error{ glGetError() };
			if (error) {
				std::cout << "GRAPHIC ERROR: " << error << std::endl;
			}
			frameCount = 0;
			fpsTimer = 0.0f;
		}
	}

	return 0;
}