#pragma once

#include <chrono>
#include <exception>
#include <cstdlib>

#include "render.h"
#include "render/mesh_renderer.h"
#include "particle.h"
#include "terrain.h"
#include "fps_camera.h"
#include "physics.h"

namespace Byte {

	struct Entity {
		Mesh mesh;
		Material material;
		Transform transform;
		MeshRenderer renderer;
		std::unique_ptr<Collider> collider;
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

			std::vector<Collider*> colliders;

			for (auto& [tag, entity] : entities) {
				if (entity.collider) {
					colliders.push_back(entity.collider.get());
				}
			}

			Physics::applyGravity(colliders, dt);
			Physics::solve(colliders);

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

			const float spawnRadius{ 100.0f };
			Vec3 cameraPos{ cameraTransform.position() };

			for (size_t i{}; i < static_cast<size_t>(200 * dt); i++) {
				float angle{ static_cast<float>(std::rand()) / RAND_MAX * 2.0f * 3.14159265f };
				float radius{ static_cast<float>(std::rand()) / RAND_MAX * spawnRadius };

				float x{ cameraPos.x + std::cos(angle) * radius };
				float z{ cameraPos.z + std::sin(angle) * radius };

				float currentHeight{ getHeight(heightMap, x, z) };

				float slopeThreshold{ 0.5f };
				float sampleOffset{ 1.0f };
				float maxHeightDiff{ 0.0f };

				std::vector<std::pair<float, float>> offsets{
					{-sampleOffset, -sampleOffset}, {0.0f, -sampleOffset}, {sampleOffset, -sampleOffset},
					{-sampleOffset, 0.0f},                               {sampleOffset, 0.0f},
					{-sampleOffset, sampleOffset},  {0.0f, sampleOffset},  {sampleOffset, sampleOffset}
				};

				for (const auto& [dx, dz] : offsets) {
					float neighborHeight{ getHeight(heightMap, x + dx, z + dz) };
					float heightDiff{ std::abs(currentHeight - neighborHeight) };
					maxHeightDiff = std::max(maxHeightDiff, heightDiff);
				}

				if (maxHeightDiff > slopeThreshold) {
					continue;
				}

				particle.transform.position(Vec3{ x, currentHeight + 0.5f, z });
				group.particles.push_back(particle);
			}

			if (glfwGetKey(window.glfwWindow, GLFW_KEY_E) == GLFW_PRESS) {
				Entity sphere;
				sphere.mesh = MeshBuilder::sphere(1,10);
				sphere.transform = Transform{ cameraTransform };
				sphere.renderer = MeshRenderer{};

				sphere.collider = std::make_unique<SphereCollider>(SphereCollider{ 1.0f });
				sphere.collider->type = ColliderType::SPHERE;

				std::string name = "sphere_" + std::to_string(entities.size());
				entities[name] = std::move(sphere);

				Entity& carried{ entities[name] };

				float r{ static_cast<float>(rand()) / RAND_MAX };
				float g{ static_cast<float>(rand()) / RAND_MAX };
				float b{ static_cast<float>(rand()) / RAND_MAX };
				carried.material.albedo(Vec3{ r, g, b });

				carried.collider->transform = &carried.transform;

				renderer.context().submit(carried.mesh, carried.material, carried.transform, carried.renderer);
			}

			if (glfwGetKey(window.glfwWindow, GLFW_KEY_Q) == GLFW_PRESS) {
				Entity sphere;
				sphere.mesh = MeshBuilder::sphere(1, 10);
				sphere.transform = Transform{ cameraTransform };
				sphere.renderer = MeshRenderer{};

				sphere.collider = std::make_unique<SphereCollider>(SphereCollider{ 1.0f });
				sphere.collider->type = ColliderType::SPHERE;

				std::string name = "sphere_" + std::to_string(entities.size());
				entities[name] = std::move(sphere);

				Entity& carried{ entities[name] };

				float r{ static_cast<float>(rand()) / RAND_MAX };
				float g{ static_cast<float>(rand()) / RAND_MAX };
				float b{ static_cast<float>(rand()) / RAND_MAX };
				carried.material.albedo(Vec3{ r, g, b });

				carried.collider->transform = &carried.transform;

				renderer.context().submit(carried.mesh, carried.material, carried.transform, carried.renderer);
			}

		}
	};

}
