#pragma once

#include <variant>
#include <unordered_map>
#include <cstdint>

#include "context.h"
#include "opengl_api.h"

namespace Byte {

	struct RenderData {
		size_t height{ 0 };
		size_t width{ 0 };

		using FramebufferMap = std::unordered_map<FramebufferTag, Framebuffer>;
		FramebufferMap frameBuffers;

		using ShaderMap = std::unordered_map<ShaderTag, Shader>;
		ShaderMap shaders;

		using Variant = std::variant<std::string, uint32_t, int32_t, bool, float, Mat4>;
		using ParameterTag = std::string;
		using ParameterMap = std::unordered_map<ParameterTag, Variant>;
		ParameterMap parameters;

		using MeshMap = std::unordered_map<MeshTag, Mesh>;
		MeshMap meshes;

		template<typename Type>
		Type& parameter(const ParameterTag& tag) {
			return std::get<Type>(parameters.at(tag));
		}
	};

	class RenderPass {
	public:
		virtual void render(RenderContext& context, RenderData& data) = 0;
	};

}