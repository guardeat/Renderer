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
#include "scene.h"
#include "loader.h"

namespace Byte {
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

		scene.pointLights.push_back(std::make_unique<PointLight>());
		scene.pointLightTransforms.push_back(std::make_unique<Transform>());
		scene.pointLights.back()->color = Vec3{ 10.0f, 10.f, 10.0f };
		scene.pointLightTransforms.back()->position(Vec3{ 0.0f, 1.0f, 0.0f });

		InstancedEntity grass;
		grass.mesh = buildGrass();
		grass.material.albedo(Vec3{ 0.09f, 0.65f, 0.05f });
		grass.material.shadow(ShadowMode::DISABLED);

		scene.textures["height_map"] = readTerrain("texture/height_map.txt");
		scene.textures["height_map_albedo"] = readTerrain("texture/height_map_diffuse.txt",3);

		Texture& heightMap{ scene.textures.at("height_map") };

		size_t xCount{ 2000 };
		size_t yCount{ 2000 };

		for (float i{}; i < xCount; ++i) {
			for (float j{}; j < yCount; ++j) {
				float offsetX{ ((rand() % 1000) / 1000.0f - 0.5f) * 0.5f - 500 };
				float offsetZ{ ((rand() % 1000) / 1000.0f - 0.5f) * 0.5f - 500 };
				float x{ static_cast<float>(i) / 2 + offsetX };
				float z{ static_cast<float>(j) / 2 + offsetZ };

				float currentHeight{ getHeight(heightMap, x, z) };

				float slopeThreshold{ 0.5f };
				float sampleOffset{ 1.0f }; 
				float maxHeightDiff{ 0.0f };

				std::vector<std::pair<float, float>> offsets{
					{-sampleOffset, -sampleOffset}, {0.0f, -sampleOffset}, {sampleOffset, -sampleOffset},
					{-sampleOffset, 0.0f},                                 {sampleOffset, 0.0f},
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

				Transform transform{};
				transform.position(Vec3{ x, currentHeight + 0.7f, z });

				float rotationY{ static_cast<float>(rand() % 360) };
				transform.rotation(Vec3{ 0.0f, rotationY, 0.0f });

				float scaleValue{ 2.2f + ((rand() % 1000) / 1000.0f) * 1.4f };
				transform.scale(Vec3{ 0.25f, scaleValue, 1.0f });

				grass.transforms.push_back(transform);
			}
		}

		scene.instancedEntities["grass"] = std::move(grass);

		renderer.data().shaders.emplace("grass", Shader{ "shader/grass.vert","../ByteRenderer/shader/deferred.frag" });
		renderer.data().shaders.emplace("particle", Shader{ "shader/particle.vert","../ByteRenderer/shader/forward.frag" });
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

		Entity terrain;
		terrain.material.metallic(0.01f);
		terrain.material.roughness(0.99f);
		terrain.transform.scale(Vec3(1.0f, 5.0f, 1.0f));
		terrain.mesh = buildTerrain(1000, 1000, 80);
		terrain.material.shaderMap().emplace("geometry", "height_map");
		terrain.material.texture("height_map", scene.textures.at("height_map"));
		terrain.material.texture("albedo", scene.textures.at("height_map_albedo"));
		terrain.renderer.primitive(PrimitiveType::PATCHES);
		terrain.material.ambientOcclusion(0.2f);
		terrain.collider = std::make_unique<TerrainCollider>(TerrainCollider{ &scene.textures.at("height_map") });
		terrain.collider->type = ColliderType::TERRAIN;

		scene.entities["height_map"] = std::move(terrain);
		scene.entities["height_map"].collider->transform = &scene.entities["height_map"].transform;

		scene.setContext(renderer);

		return scene;
	}

}