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


//TODO: Need shader parser and better shader system. 
//Use this parser to automaticaly input uniforms from predefined sources (Context).
//Create shaderInputMap = std::unordered_map<Tag, struct{Variant<Input Types>, Type enum}>
//Shaders will have struct{Tag, Type enum}