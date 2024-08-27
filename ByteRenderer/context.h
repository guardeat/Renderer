#pragma once

#include <cstdint>
#include <unordered_map>

#include "buffer.h"
#include "mesh.h"
#include "transform.h"
#include "material.h"
#include "camera.h"
#include "light.h"
#include "g_buffer.h"
#include "texture.h"

namespace Byte {

	struct SceneContext {
		Buffer<Mesh*> meshes;
		Buffer<Material*> materials;
		Buffer<Transform*> transforms;

		Camera* mainCamera;
		Transform* mainCameraTransform;

		DirectionalLight* directionalLight;
		Transform* directionalLightTransform;
	};

	struct RenderContext {
		size_t height{ 0 };
		size_t width{ 0 };

		GBuffer gBuffer{};
		Texture shadowMap;

		using ShaderMap = std::unordered_map<ShaderTag, Shader>;
		ShaderMap shaderMap;

		RenderArray quad;
	};

}
