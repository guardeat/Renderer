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

	RenderContext context;

	Camera camera;
	Transform transform;
	FPSCamera fpsCamera;

	Mesh cube{ createSphere(1,20) };
	Material cubeMaterial{};
	cubeMaterial.shaderTag("default_defered");
	cubeMaterial.albedo(Vec4{ 1.0f,0.0f,0.0f,0.0f });
	Transform cubeTransform;

	DirectionalLight dLight;
	Transform dLightTransform;

	context.mainCamera = &camera;
	context.mainCameraTransform = &transform;
	context.meshes.push_back(&cube);
	context.materials.push_back(&cubeMaterial);
	context.transforms.push_back(&cubeTransform);
	context.directionalLight = &dLight;
	context.directionalLightTransform = &dLightTransform;

	while (!glfwWindowShouldClose(window.glfwWindow)) {
		renderer.render(context);
		renderer.update(window);
		fpsCamera.update(window, transform);
		glfwPollEvents();
	}

	return 0;
}

