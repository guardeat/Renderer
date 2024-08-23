#pragma once

#include <unordered_map>

#include "texture.h"
#include "g_buffer.h"
#include "shader.h"

namespace Byte {

	struct Quad {
		Buffer<float> positions = {
			-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
			1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
			1.0f, -1.0f, 0.0f,  1.0f, 0.0f
		};

		Buffer<uint32_t> indices = {
			0, 1, 2,
			1, 3, 2
		};

		RenderArray renderArray;
	};

	struct RenderData {
		size_t height{ 0 };
		size_t width{ 0 };

		GBuffer gBuffer{};
		Texture shadowMap;

		using ShaderMap = std::unordered_map<ShaderTag, Shader>;
		ShaderMap shaderMap;

		Quad quad;
	};

}