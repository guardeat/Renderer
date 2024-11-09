#include <chrono>

#include "test.h"

using namespace Byte;

int main() {
	glfwInit();

	Window window{ 1336,768,"Test" };

	Renderer renderer{ Renderer::build<GeometryPass,ShadowPass,LightingPass,PointLightPass,DrawPass>() };
	RenderConfig config;

	config.shaderPaths["default_deferred"] = { "default_vertex.glsl","deferred_geometry.glsl" };
	config.shaderPaths["default_forward"] = { "default_vertex.glsl","forward_fragment.glsl" };
	config.shaderPaths["quad_shader"] = { "quad_vertex.glsl","quad_fragment.glsl" };
	config.shaderPaths["lighting_shader"] = { "quad_vertex.glsl","lighting_fragment.glsl" };
	config.shaderPaths["point_light_shader"] = { "point_light_vertex.glsl","point_light_fragment.glsl" };
	config.shaderPaths["instanced_deferred"] = { "instanced_vertex.glsl","deferred_geometry.glsl" };
	config.shaderPaths["depth_shader"] = { "depth_vertex.glsl","depth_fragment.glsl" };
	config.shaderPaths["instanced_depth"] = { "instanced_depth_vertex.glsl","depth_fragment.glsl" };

	FramebufferConfig gBufferConfig;

	gBufferConfig.width = window.width();
	gBufferConfig.height = window.height();

	gBufferConfig.attachments = {
		{ "position", 0, GL_RGBA16F, GL_RGBA, GL_FLOAT },
		{ "normal", 1, GL_RGBA16F, GL_RGBA, GL_FLOAT },
		{ "albedoSpecular", 2, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE }
	};

	config.frameBufferConfigs["gBuffer"] = gBufferConfig;

	FramebufferConfig colorBufferConfig;

	colorBufferConfig.width = window.width();
	colorBufferConfig.height = window.height();

	colorBufferConfig.attachments = {
		{ "albedoSpecular", 0, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE },
	};

	config.frameBufferConfigs["colorBuffer"] = colorBufferConfig;

	FramebufferConfig depthBufferConfig;

	depthBufferConfig.width = window.width();
	depthBufferConfig.height = window.height();
	depthBufferConfig.depthMap = true;

	config.frameBufferConfigs["depthBuffer"] = depthBufferConfig;

	renderer.initialize(window, config);

	RenderContext context;

	Camera camera;
	Transform transform;
	transform.position(Vec3{ -10.0f,10.0f,5.0f });
	FPSCamera fpsCamera;

	const int gridSize = 10;
	const float sphereRadius = 1.0f;
	const float spacing = 3.0f * sphereRadius; 

	std::vector<Transform> sphereTransforms(gridSize * gridSize * gridSize);

	Mesh sphere{ MeshBuilder::sphere(sphereRadius, 20) };
	Material sphereMaterial{};
	sphereMaterial.shaderTag("instanced_deferred");
	sphereMaterial.albedo(Vec4{ 1.0f,1.0f,0.0f,0.0f });

	context.instances()["spheres_1"] = RenderInstance{ sphere,sphereMaterial };

	for (int x = 0; x < gridSize; ++x) {
		for (int y = 0; y < gridSize; ++y) {
			for (int z = 0; z < gridSize; ++z) {
				int index = x + y * gridSize + z * gridSize * gridSize;

				Transform transform;
				transform.position(Vec3(x * spacing, y * spacing + 1.0f, z * spacing));
				sphereTransforms[index] = transform;

				context.instances()["spheres_1"].add(transform);
			}
		}
	}

	DirectionalLight dLight;
	Transform dLightTransform;
	dLightTransform.rotate(Vec3(-135.0f, 0.0f, 0.0f));

	context.submit(camera, transform);
	context.submit(dLight,dLightTransform);

	Mesh plane(MeshBuilder::plane(10000,10000,1));
	Material pMaterial;
	pMaterial.shaderTag("default_deferred");
	pMaterial.albedo(Vec4(0.4f, 0.3f, 0.2f, 1.0f));
	Transform planeTransform;
	planeTransform.rotate(Vec3(270.0f, 0.0f, 0.0f));
	
	context.submit(plane,pMaterial,planeTransform);
	
	PointLight pl;
	Transform plTransform;

	context.submit(pl,plTransform);

	Mesh lightMesh{ MeshBuilder::sphere(0.1f,100) };
	Material lmMaterial;
	lmMaterial.shaderTag("default_deferred");
	lmMaterial.albedo(Vec4(1.0f, 1.0f, 1.0f, 1.0f));

	context.submit(lightMesh,lmMaterial,plTransform);

	float lightAngle = 0.0f;
	const float lightSpeed = 1.0f;
	const float circleRadius = 20.0f;

	auto lastTime = std::chrono::high_resolution_clock::now();
	int frameCount = 0;

	while (!glfwWindowShouldClose(window.glfwWindow)) {
		renderer.render(context);
		renderer.update(window);
		fpsCamera.update(window, transform);
		glfwPollEvents();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
		lastTime = currentTime; 

		lightAngle += lightSpeed * deltaTime;
		if (lightAngle >= 2 * 3.141592f) {
			lightAngle -= 2 * 3.141592f;  
		}

		float x = circleRadius * std::cos(lightAngle);
		float z = circleRadius * std::sin(lightAngle);
		plTransform.position(Vec3(x + 13.5f, 1.0f, z + 13.5f));

		frameCount++;
		static float fpsTimer = 0.0f;
		fpsTimer += deltaTime;

		if (fpsTimer >= 1.0f) {
			std::cout << "FPS: " << frameCount << std::endl;
			frameCount = 0;
			fpsTimer = 0.0f;
		}
	}

	return 0;
}