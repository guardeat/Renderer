#pragma once

#include <chrono>
#include <exception>
#include <cstdlib>

#include "stb_image.h"

#include "render.h"
#include "render/mesh_renderer.h"
#include "particle.h"
#include "terrain.h"
#include "fps_camera.h"

namespace Byte {

	struct Loader {
		static TextureData loadTexture(const Path& path, DataType type, size_t forceChannels = 0) {
			stbi_set_flip_vertically_on_load(true);
			int force{ static_cast<int>(forceChannels) };

			TextureData texture;
			texture.path = path;
			texture.dataType = type;

			int width, height, channels;
			std::string pathString{ path.string() };

			void* imgData{ nullptr };

			if (type == DataType::SHORT || type == DataType::UNSIGNED_SHORT) {
				imgData = stbi_load_16(pathString.c_str(), &width, &height, &channels, force);
			}
			else if (type == DataType::FLOAT) {
				imgData = stbi_loadf(pathString.c_str(), &width, &height, &channels, force);
			}
			else {
				imgData = stbi_load(pathString.c_str(), &width, &height, &channels, force);
			}

			if (!imgData) {
				throw std::runtime_error{ "Failed to load texture: " + pathString };
			}

			texture.width = static_cast<size_t>(width);
			texture.height = static_cast<size_t>(height);

			channels = (force != 0) ? force : channels;
			
			if (channels == 1) {
				texture.internalFormat = ColorFormat::R32F;
				texture.format = ColorFormat::RED;
			}
			else if (channels == 3) {
				texture.internalFormat = ColorFormat::RGB32F;
				texture.format = ColorFormat::RGB;
			}
			else {
				texture.internalFormat = ColorFormat::RGBA32F;
				texture.format = ColorFormat::RGBA;
			}

			size_t bytesPerChannel{ 1 };
			if (type == DataType::SHORT || type == DataType::UNSIGNED_SHORT) {
				bytesPerChannel = 2;
			}
			else if (type == DataType::FLOAT) {
				bytesPerChannel = 4;
			}

			size_t dataSize{ static_cast<size_t>(width * height * channels) * bytesPerChannel };

			texture.data.resize(dataSize);
			std::memcpy(texture.data.data(), imgData, dataSize);

			stbi_image_free(imgData);

			return texture;
		}

	};

	struct Entity {
		Mesh mesh;
		Material material;
		Transform transform;
		MeshRenderer renderer;
	};

	struct InstancedEntity {
		Mesh mesh;
		Material material;
		std::vector<Transform> transforms;
		MeshRenderer renderer;
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

		std::unordered_map<TextureTag, Texture> textures;

		ParticleSystem particleSystem;
		FPSCamera fpsCamera;

		void setContext(Renderer& renderer) {
			renderer.context().clear();

			renderer.context().submit(camera, cameraTransform);
			renderer.context().submit(directionalLight, directionalLightTransform);

			for (auto& pair : entities) {
				renderer.context().submit(pair.second.mesh, pair.second.material, pair.second.transform,pair.second.renderer);
			}

			for (size_t i{}; i < pointLights.size(); ++i) {
				renderer.context().submit(*pointLights[i], *pointLightTransforms[i]);
			}

			for (auto& pair : instancedEntities) {
				renderer.context().createInstance(pair.first,pair.second.mesh,pair.second.material, pair.second.renderer);

				for (auto& transform : pair.second.transforms) {
					renderer.context().submit(pair.first, transform);
				}
			}
		}

		void update(float dt, Renderer& renderer, Window& window) {
			particleSystem.update(dt, renderer);
			fpsCamera.update(window, cameraTransform, dt);

			renderer.context().input<float>("uTime") += dt;

			renderer.render();
			renderer.update(window);
			glfwPollEvents();

			//APP specific:
			auto& group{ particleSystem.groups().at("grass_particle") };
			Particle particle;
			particle.lifeTime = 5.0f;
			particle.velocity = renderer.context().input<Vec3>("uWind") * 3 + Vec3{ 0.0f,1.0f,0.0f };
			for (size_t i{}; i < 100 * dt; i++) {
				float x{ static_cast<float>(std::rand()) / RAND_MAX * 200.0f - 100 };
				float z{ static_cast<float>(std::rand()) / RAND_MAX * 200.0f - 100 };
				particle.transform.position(Vec3{ x,0.5f,z });
				group.particles.push_back(particle);
			}

			Vec3 axis{ 1.0f, 1.0f, 1.0f };

			Quaternion deltaRot{ Quaternion(axis, 45.0f * dt) };

			for (auto& pair : entities) {
				if (pair.first.find("cube_") == 0) {
					Transform& transform{ pair.second.transform };
					Quaternion currentRot{ transform.rotation() };

					Quaternion newRot{ deltaRot * currentRot };

					transform.rotation(newRot.normalized());
				}
			}
		}
	};

	inline Mesh buildGrass() {
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
		return Mesh{ std::move(data) };
	}

	inline Scene buildCustomScene(Renderer& renderer) {
		Scene scene;

		scene.directionalLightTransform.rotation(Vec3{ -45.0f, 0.0f, 0.0f });

		Entity plane;
		plane.mesh = MeshBuilder::plane(200, 200, 1);
		plane.material.albedo(Vec3{ 0.47f, 0.85f, 0.15f });

		//scene.entities["plane"] = std::move(plane);

		scene.pointLights.push_back(std::make_unique<PointLight>());
		scene.pointLightTransforms.push_back(std::make_unique<Transform>());
		scene.pointLights.back()->color = Vec3{ 10.0f, 10.f, 10.0f };
		scene.pointLightTransforms.back()->position(Vec3{ 0.0f, 1.0f, 0.0f });

		InstancedEntity grass;
		grass.mesh = buildGrass();
		grass.material.albedo(Vec3{ 0.27f, 0.95f, 0.15f });
		grass.material.shadow(ShadowMode::DISABLED);

		size_t xCount{ 0 };
		size_t yCount{ 0 };

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
				transform.scale(Vec3{ 0.25f, scaleValue, 1.0f });

				grass.transforms.push_back(transform);
			}
		}

		scene.instancedEntities["grass"] = std::move(grass);

		renderer.data().shaders.emplace("grass", Shader{ "test/shader/grass.vert","../ByteRenderer/shader/deferred.frag" });
		renderer.data().shaders.emplace("particle", Shader{ "test/shader/particle.vert","../ByteRenderer/shader/forward.frag" });
		renderer.data().shaders.at("grass").include(Uniform{ "uTime",UniformType::FLOAT });
		renderer.context().input("uTime", ShaderInput<float>{0.0f, UniformType::FLOAT});
		renderer.data().shaders.at("grass").include(Uniform{ "uWind",UniformType::VEC3 });
		renderer.context().input("uWind", ShaderInput<Vec3>{Vec3(1.0f, 0.0, 0.0f), UniformType::VEC3});
		renderer.compileShaders();
		scene.instancedEntities.at("grass").material.shaderMap().emplace("geometry", "grass");
		scene.particleSystem.groups().emplace("grass_particle", ParticleGroup{ MeshBuilder::plane(0.2f,0.2f,1), Material{} });
		scene.particleSystem.groups().at("grass_particle").material.albedo(Vec4{ 0.27f, 0.95f, 0.15f,0.1f });
		scene.particleSystem.groups().at("grass_particle").material.ambientOcclusion(0.2f);
		scene.particleSystem.groups().at("grass_particle").material.shaderMap().emplace("geometry", "particle");
		scene.particleSystem.groups().at("grass_particle").material.shadow(ShadowMode::DISABLED);
		scene.particleSystem.groups().at("grass_particle").material.transparency(TransparencyMode::UNSORTED);

		for (size_t i{}; i < 0; ++i) {
			Entity cube;
			cube.mesh = MeshBuilder::cube();
			cube.material.albedo(Vec3{
				static_cast<float>(rand() % 1000) / 1000.0f,
				static_cast<float>(rand() % 1000) / 1000.0f,
				static_cast<float>(rand() % 1000) / 1000.0f
				});

			float x{ static_cast<float>(rand() % 20000) / 100.0f - 100.0f };
			float y{ static_cast<float>(rand() % 1500) / 100.0f + 5.0f };
			float z{ static_cast<float>(rand() % 20000) / 100.0f - 100.0f };

			cube.transform.position(Vec3{ x, y, z });
			scene.entities["cube_" + std::to_string(i)] = std::move(cube);
		}

		scene.textures["height_map"] = Loader::loadTexture("test/texture/height_map_high.bmp", DataType::UNSIGNED_SHORT);
		scene.textures["height_map_albedo"] = Loader::loadTexture("test/texture/height_map_diffuse.png", DataType::UNSIGNED_SHORT);

		Entity terrain;
		terrain.material.metallic(0.01f);
		terrain.material.roughness(0.99f);
		terrain.transform.scale(Vec3(1.0f, 1.0f, 1.0f));
		terrain.mesh = buildTerrain(100, 100, 40);
		terrain.material.shaderMap().emplace("geometry", "height_map");
		terrain.material.texture("height_map", scene.textures.at("height_map"));
		terrain.material.texture("albedo", scene.textures.at("height_map_albedo"));
		terrain.renderer.primitive(PrimitiveType::PATCHES);

		scene.entities["height_map"] = std::move(terrain);

		scene.setContext(renderer);

		return scene;
	}

}