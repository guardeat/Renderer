#pragma once

#include <chrono>
#include <exception>
#include <cstdlib>

#include "render.h"
#include "render/mesh_renderer.h"
#include "particle.h"
#include "terrain.h"
#include "fps_camera.h"

namespace Byte {

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
				renderer.context().submit(pair.second.mesh, pair.second.material, pair.second.transform, pair.second.renderer);
			}

			for (size_t i{}; i < pointLights.size(); ++i) {
				renderer.context().submit(*pointLights[i], *pointLightTransforms[i]);
			}

			for (auto& pair : instancedEntities) {
				renderer.context().createInstance(pair.first, pair.second.mesh, pair.second.material, pair.second.renderer);

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
			Texture& heightMap{ textures.at("height_map") };
			auto& group{ particleSystem.groups().at("grass_particle") };
			Particle particle{};
			particle.lifeTime = 5.0f;
			particle.velocity = renderer.context().input<Vec3>("uWind") * 3 + Vec3{ 0.0f, 1.0f, 0.0f };

			for (size_t i{}; i < 500 * dt; i++) {
				float x{ static_cast<float>(std::rand()) / RAND_MAX * 1000.0f - 500 };
				float z{ static_cast<float>(std::rand()) / RAND_MAX * 1000.0f - 500 };

				float terrainHeight{ getHeight(heightMap, x, z) };

				particle.transform.position(Vec3{ x, terrainHeight + 0.5f, z });
				group.particles.push_back(particle);
			}
		}
	};

}
