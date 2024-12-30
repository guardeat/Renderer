#include <chrono>

#include "test.h"
#include "render.h"

using namespace Byte;

int main() {
	glfwInit();

	Window window{ 1336,768,"Test" };

	Renderer renderer{ RendererGenerator::deferred(window) };

	Camera camera;
	Transform transform;
	transform.position(Vec3{ -10.0f,10.0f,5.0f });
	FPSCamera fpsCamera;

	const int gridSize{ 10 };
	const float sphereRadius{ 1.4f };
	const float spacing{ 3.0f };

	std::vector<Transform> sphereTransforms(gridSize * gridSize * gridSize);

	Mesh sphere{ MeshBuilder::sphere(sphereRadius, 20) };
	Material sphereMaterial{};
	sphereMaterial.albedo(Vec3{ 1.0f,1.0f,0.0f });

	renderer.context().createInstance("spheres_1",sphere,sphereMaterial);

	for (int x = 0; x < gridSize; ++x) {
		for (int y = 0; y < gridSize; ++y) {
			for (int z = 0; z < gridSize; ++z) {
				int index = x + y * gridSize + z * gridSize * gridSize;

				Transform transform;
				transform.position(Vec3(x * spacing, y * spacing + 1.0f, z * spacing));
				sphereTransforms[index] = transform;

				renderer.context().submit("spheres_1",transform);
			}
		}
	}

	std::vector<Transform> cubeTransforms(gridSize * gridSize * gridSize);

	Mesh cube{ MeshBuilder::cube() };
	Material cubeMaterial{};
	cubeMaterial.albedo(Vec3{ 1.0f,0.0f,1.0f });

	renderer.context().createInstance("cubes_1", cube, cubeMaterial);

	for (int x = 0; x < gridSize; ++x) {
		for (int y = 0; y < gridSize; ++y) {
			for (int z = 0; z < gridSize; ++z) {
				int index = x + y * gridSize + z * gridSize * gridSize;

				Transform transform;
				transform.scale(Vec3{ 2.4f,2.4f,2.4f });
				transform.position(Vec3{ x * spacing - 35, y * spacing + 1.0f, z * spacing - 35 });
				cubeTransforms[index] = transform;

				renderer.context().submit("cubes_1", transform);
			}
		}
	}

	DirectionalLight dLight;
	Transform dLightTransform;
	dLightTransform.rotate(Vec3{ -45.0f,20.0f,0.0f });
	dLightTransform.position(Vec3{ 50.0f,80.0f,80.0f });

	renderer.context().submit(camera, transform);
	renderer.context().submit(dLight,dLightTransform);

	Mesh plane{ MeshBuilder::plane(100,100,1) };
	Material pMaterial;
	pMaterial.albedo(Vec3{ 0.2f, 0.7f, 0.2f });
	Transform planeTransform;
	planeTransform.rotate(Vec3{ 270.0f, 0.0f, 0.0f });
	
	renderer.context().submit(plane,pMaterial,planeTransform);
	
	PointLight pl;
	Transform plTransform;

	renderer.context().submit(pl,plTransform);

	Mesh lightMesh{ MeshBuilder::sphere(0.1f,100) };
	Material lmMaterial;
	lmMaterial.albedo(Vec3{ 1.0f, 1.0f, 1.0f });
	lmMaterial.emission(1.0f);
	lmMaterial.ambientOcclusion(1.0f);

	renderer.context().submit(lightMesh,lmMaterial,plTransform);

	float lightAngle{ 0.0f };
	const float lightSpeed{ 1.0f };
	const float circleRadius{ 20.0f };

	auto lastTime{ std::chrono::high_resolution_clock::now() };
	int frameCount{ 0 };
	float fpsTimer{ 0.0f };

	while (!glfwWindowShouldClose(window.glfwWindow)) {
		renderer.render();
		renderer.update(window);
		fpsCamera.update(window, transform);
		glfwPollEvents();

		auto currentTime{ std::chrono::high_resolution_clock::now() };
		float deltaTime{ std::chrono::duration<float>(currentTime - lastTime).count() };
		lastTime = currentTime;

		lightAngle += lightSpeed * deltaTime;
		if (lightAngle >= 2 * 3.141592f) {
			lightAngle -= 2 * 3.141592f;  
		}

		float x{ circleRadius * std::cos(lightAngle) };
		float z{ circleRadius * std::sin(lightAngle) };
		plTransform.position(Vec3(x + 13.5f, 1.0f, z + 13.5f));
		//dLightTransform.rotate(Vec3(0.01f, 0.0f, 0.0f));

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