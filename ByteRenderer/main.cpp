#include "renderer.h"
#include "render_pass.h"
#include "window.h"
#include "test.h"

using namespace Byte;

int main() {
	glfwInit();

	Window window{ 1336,768,"Test" };

	Renderer renderer{ Renderer::build<GeometryPass,LightingPass>() };
	RenderConfig config;

	config.shaderPathMap["default_defered"] = { "default_vertex.glsl","defered_geometry.glsl" };
	config.shaderPathMap["default_forward"] = { "default_vertex.glsl","forward_fragment.glsl" };
	config.shaderPathMap["quad_shader"] = { "quad_vertex.glsl","quad_fragment.glsl" };
	config.shaderPathMap["lighting_shader"] = { "lighting_vertex.glsl","lighting_fragment.glsl" };

	renderer.initialize(window, config);

	SceneContext context;

	Camera camera;
	Transform transform;
	FPSCamera fpsCamera;

	const int gridSize = 10;
	const float sphereRadius = 1.0f;
	const float spacing = 3.0f * sphereRadius; 

	std::vector<Mesh> spheres(gridSize * gridSize * gridSize);
	std::vector<Material> sphereMaterials(gridSize * gridSize * gridSize);
	std::vector<Transform> sphereTransforms(gridSize * gridSize * gridSize);

	for (int x = 0; x < gridSize; ++x) {
		for (int y = 0; y < gridSize; ++y) {
			for (int z = 0; z < gridSize; ++z) {
				int index = x + y * gridSize + z * gridSize * gridSize;

				spheres[index] = Mesh::sphere(sphereRadius, 20);

				sphereMaterials[index] = Material{};
				sphereMaterials[index].shaderTag("default_defered");
				sphereMaterials[index].albedo(Vec4{ static_cast<float>(x) / gridSize, static_cast<float>(y) / gridSize, static_cast<float>(z) / gridSize, 0.0f });

				sphereTransforms[index] = Transform{};
				sphereTransforms[index].position(Vec3(x * spacing, y * spacing, z * spacing)); 

				context.meshes.push_back(&spheres[index]);
				context.materials.push_back(&sphereMaterials[index]);
				context.transforms.push_back(&sphereTransforms[index]);
			}
		}
	}

	DirectionalLight dLight;
	Transform dLightTransform;
	dLightTransform.rotate(Vec3(-135.0f, 0.0f, 0.0f));

	context.mainCamera = &camera;
	context.mainCameraTransform = &transform;
	context.directionalLight = &dLight;
	context.directionalLightTransform = &dLightTransform;

	Mesh plane(Mesh::plane(10000,10000,1));
	Material pMaterial;
	pMaterial.shaderTag("default_defered");
	pMaterial.albedo(Vec4(0.4f, 0.3f, 0.2f, 1.0f));
	Transform planeTransform;
	planeTransform.rotate(Vec3(270.0f, 0.0f, 0.0f));

	context.meshes.push_back(&plane);
	context.materials.push_back(&pMaterial);
	context.transforms.push_back(&planeTransform);

	while (!glfwWindowShouldClose(window.glfwWindow)) {
		renderer.render(context);
		renderer.update(window);
		fpsCamera.update(window, transform);
		glfwPollEvents();
	}

	return 0;
}