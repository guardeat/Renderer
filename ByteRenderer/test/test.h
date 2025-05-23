#pragma once

#include <chrono>
#include <exception>
#include <cstdlib>

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
			stbi_set_flip_vertically_on_load(true);

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

			if (channels == 3) {
				texture.internalFormat = ColorFormat::RGB;
				texture.format = ColorFormat::RGB;
			}
			else {
				texture.internalFormat = ColorFormat::RGBA;
				texture.format = ColorFormat::RGBA;
			}

			size_t dataSize{ static_cast<size_t>(width * height * channels) };
			texture.data.assign(imgData, imgData + dataSize);

			stbi_image_free(imgData);

			return texture;
		}
	};

	struct Entity {
		RenderMesh mesh;
		Material material;
		Transform transform;
	};

	struct InstancedEntity {
		RenderMesh mesh;
		Material material;
		std::vector<Transform> transforms;
	};

	struct Scene {
		Camera camera;
		Transform cameraTransform;

		DirectionalLight directionalLight;
		Transform directionalLightTransform;

		std::unordered_map<std::string, Entity> entities;

		std::unordered_map<std::string, InstancedEntity> instancedEntities;

		std::vector<std::unique_ptr<PointLight>> pointLights;
		std::vector<std::unique_ptr<Transform>> pointLightTransforms;

		void setContext(Renderer& renderer) {
			renderer.context().clear();

			renderer.context().submit(camera, cameraTransform);
			renderer.context().submit(directionalLight, directionalLightTransform);

			for (auto& pair : entities) {
				renderer.context().submit(pair.second.mesh, pair.second.material, pair.second.transform);
			}

			for (size_t i{}; i < pointLights.size(); ++i) {
				renderer.context().submit(*pointLights[i], *pointLightTransforms[i]);
			}

			for (auto& pair : instancedEntities) {
				renderer.context().createInstance(pair.first,pair.second.mesh,pair.second.material);

				for (auto& transform : pair.second.transforms) {
					renderer.context().submit(pair.first, transform);
				}
			}
		}
	};

	inline RenderMesh buildGrass() {
		std::vector<float> vertices{
		   -0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
			0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
			0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
		   -0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,   0.0f, 1.0f,
			0.0f, 0.75f,  0.5f, 0.0f, 0.0f, 1.0f,   0.0f, 1.0f,
		};

		std::vector<uint32_t> indices{
			0, 1, 2,
			0, 2, 3,
			3, 4, 2,
		};

		MeshData data{ std::move(vertices), std::move(indices), MeshMode::STATIC };
		return RenderMesh{ std::move(data) };
	}

	inline Scene buildCustomScene(Renderer& renderer) {
		Scene scene;

		scene.directionalLightTransform.rotation(Vec3{ -45.0f, 0.0f, 0.0f });

		Entity plane;
		plane.mesh = MeshBuilder::plane(200, 200, 1);
		plane.material.albedo(Vec3{ 0.47f, 0.85f, 0.15f });

		scene.entities["plane"] = std::move(plane);

		scene.pointLights.push_back(std::make_unique<PointLight>());
		scene.pointLightTransforms.push_back(std::make_unique<Transform>());
		scene.pointLights.back()->color = Vec3{ 10.0f, 10.f, 10.0f };
		scene.pointLightTransforms.back()->position(Vec3{ 0.0f, 1.0f, 0.0f });

		InstancedEntity grass;
		grass.mesh = buildGrass();
		grass.material.albedo(Vec3{ 0.27f, 0.95f, 0.15f });
		grass.material.shadowMode(ShadowMode::DISABLED);

		size_t xCount{ 400 };
		size_t yCount{ 400 };

		auto roadZ = [](float x) {
			float a{ 0.001f };
			float b{ -0.01f };
			float c{ 0.3f };
			float d{ 0.0f };
			return a * x * x * x + b * x * x + c * x + d;
			};

		for (float i{}; i < xCount; ++i) {
			for (float j{}; j < yCount; ++j) {
				float offsetX{ ((rand() % 1000) / 1000.0f - 0.5f) * 0.5f - 100 };
				float offsetZ{ ((rand() % 1000) / 1000.0f - 0.5f) * 0.5f - 100 };

				float x{ static_cast<float>(i) / 2 + offsetX };
				float z{ static_cast<float>(j) / 2 + offsetZ };

				float closestDist{ 1000.0f };
				for (float sampleX{ x - 2.0f }; sampleX <= x + 2.0f; sampleX += 0.2f) {
					float sampleZ{ roadZ(sampleX) };
					float dx{ sampleX - x };
					float dz{ sampleZ - z };
					float dist{ std::sqrt(dx * dx + dz * dz) };
					if (dist < closestDist) {
						closestDist = dist;
					}
				}

				if (closestDist < 1.0f) {
					continue;
				}
				else if (closestDist < 3.0f) {
					float chance{ (closestDist - 1.0f) / 2.0f };
					float r{ (rand() % 1000) / 1000.0f };
					if (r > chance) {
						continue;
					}
				}

				Transform transform;

				transform.position(Vec3{ x, 0.5f, z });

				float rotationY{ static_cast<float>(rand() % 360) };
				transform.rotation(Vec3{ 0.0f, rotationY, 0.0f });

				float scaleValue{ 2.2f + ((rand() % 1000) / 1000.0f) * 1.4f };
				transform.scale(Vec3{ 0.2f, scaleValue, 1.0f });

				grass.transforms.push_back(transform);
			}
		}

		scene.instancedEntities["grass"] = std::move(grass);

		renderer.data().shaders.emplace("grass", Shader{ "test/shader/grass.vert","resource/shader/deferred.frag" });
		renderer.data().shaders.at("grass").addUniform(Uniform{ "uTime",UniformType::FLOAT });
		renderer.context().shaderInputMap().emplace("uTime", ShaderInput<float>{0.0f, UniformType::FLOAT});
		renderer.data().shaders.at("grass").addUniform(Uniform{ "uWind",UniformType::VEC3 });
		renderer.context().shaderInputMap().emplace("uWind", ShaderInput<Vec3>{Vec3(1.0f, 0.0, 0.0f), UniformType::VEC3});
		renderer.compileShaders();
		scene.instancedEntities.at("grass").material.shaderMap().emplace("geometry", "grass");
		scene.setContext(renderer);

		return scene;
	}

}