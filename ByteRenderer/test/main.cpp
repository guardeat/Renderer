#include <chrono>

#include "test.h"
#include "render.h"

using namespace Byte;

int main() {
	glfwInit();

	Window window{ 1336,768,"Test" };

	Renderer renderer{ RendererGenerator::deferred(window) };

	FPSCamera fpsCamera;

	Scene scene{ buildCustomScene() };
	renderer.data().shaders.emplace("grass", Shader{ "test/shader/grass.vert","resource/shader/deferred.frag" });
	renderer.compileShaders();
	scene.instancedEntities.at("grass").material.shaderMap().emplace("geometry", "grass");
	scene.setContext(renderer);

	auto lastTime{ std::chrono::high_resolution_clock::now() };
	int frameCount{ 0 };
	float fpsTimer{ 0.0f };

	renderer.load();

	//scene.directionalLight.intensity = 0.0f;

	while (!glfwWindowShouldClose(window.glfwWindow)) {
		auto currentTime{ std::chrono::high_resolution_clock::now() };
		float deltaTime{ std::chrono::duration<float>(currentTime - lastTime).count() };
		lastTime = currentTime;

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
				std::cout << "GRAPHIC ERROR: " << error << "std::endl";
			}
			frameCount = 0;
			fpsTimer = 0.0f;
		}
	}

	return 0;
}