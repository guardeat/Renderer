#pragma once

#include <variant>
#include <unordered_map>
#include <cstdint>

#include "context.h"
#include "render_api.h"

namespace Byte {

	struct RenderData {
		size_t height{ 0 };
		size_t width{ 0 };

		using FramebufferMap = std::unordered_map<FramebufferTag, Framebuffer>;
		FramebufferMap frameBuffers;

		using ShaderMap = std::unordered_map<ShaderTag, Shader>;
		ShaderMap shaders;

		using Parameter = std::variant<std::string, uint32_t, int32_t, bool, float, Mat4>;
		using ParameterMap = std::unordered_map<ParameterTag, Parameter>;
		ParameterMap parameters;

		using MeshMap = std::unordered_map<MeshTag, RenderMesh>;
		MeshMap meshes;

		template<typename Type>
		Type& parameter(const std::string& tag) {
			return std::get<Type>(parameters.at(tag));
		}
	};

	class RenderPass {
	public:
		virtual void render(RenderContext& context, RenderData& data) = 0;
	};

}