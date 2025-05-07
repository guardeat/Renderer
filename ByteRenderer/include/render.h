#pragma once

#include "renderer.h"
#include "render_type.h"
#include "default_pass.h"

namespace Byte {

	struct RendererGenerator {
		static Renderer deferred(Window& window) {
			Renderer renderer{ Renderer::build<SkyboxPass, ShadowPass, GeometryPass, LightingPass, BloomPass, DrawPass>() };
			RenderConfig config;

			size_t width{ window.width() };
			size_t height{ window.height() };

			config.shaderPaths["quad"] = { "resource/shader/quad.vert", "resource/shader/quad.frag" };
			config.shaderPaths["quad_depth"] = { "resource/shader/quad.vert", "resource/shader/quad_depth.frag" };
			config.shaderPaths["lighting"] = { "resource/shader/quad.vert", "resource/shader/lighting.frag" };
			config.shaderPaths["point_light"] = { "resource/shader/point_light.vert", "resource/shader/point_light.frag" };
			config.shaderPaths["depth"] = { "resource/shader/depth.vert", "resource/shader/depth.frag" };
			config.shaderPaths["instanced_depth"] = { "resource/shader/instanced_depth.vert", "resource/shader/depth.frag" };
			config.shaderPaths["procedural_skybox"] = { "resource/shader/procedural_skybox.vert", "resource/shader/procedural_skybox.frag" };
			config.shaderPaths["deferred"] = { "resource/shader/default.vert", "resource/shader/deferred.frag" };
			config.shaderPaths["instanced_deferred"] = { "resource/shader/instanced.vert", "resource/shader/deferred.frag" };
			config.shaderPaths["bloom_upsample"] = { "resource/shader/quad.vert", "resource/shader/bloom_upsample.frag" };
			config.shaderPaths["bloom_downsample"] = { "resource/shader/quad.vert", "resource/shader/bloom_downsample.frag" };

			config.parameters.emplace("render_skybox", true);
			config.parameters.emplace("render_shadow", true);
			config.parameters.emplace("clear_gbuffer", true);
			config.parameters.emplace("gamma", 2.2f);
			config.parameters.emplace("cascade_count", 4U);
			config.parameters.emplace("cascade_divisor_1", 1.0f);
			config.parameters.emplace("cascade_divisor_2", 4.0f);
			config.parameters.emplace("cascade_divisor_3", 8.0f);
			config.parameters.emplace("cascade_divisor_4", 20.0f);
			config.parameters.emplace("cascade_light_1", Mat4{});
			config.parameters.emplace("cascade_light_2", Mat4{});
			config.parameters.emplace("cascade_light_3", Mat4{});
			config.parameters.emplace("cascade_light_4", Mat4{});
			config.parameters.emplace("current_shadow_draw_frame", 0U);
			config.parameters.emplace("shadow_draw_frame", 4U);
			config.parameters.emplace("render_bloom", true);
			config.parameters.emplace("bloom_mip_count", 5U);

			config.meshes.emplace("cube", MeshBuilder::cube());
			config.meshes.emplace("quad", MeshBuilder::quad());
			config.meshes.emplace("sphere", MeshBuilder::sphere(1, 10));
			config.meshes.emplace("low_poly_sphere", MeshBuilder::sphere(1, 4));

			FramebufferData gBufferData;
			gBufferData.width = width;
			gBufferData.height = height;
			gBufferData.textures = {
				{ "normal",   { AttachmentType::COLOR_0, ColorFormat::RGB16F, ColorFormat::RGB, DataType::FLOAT } },
				{ "albedo",   { AttachmentType::COLOR_1, ColorFormat::RGB16F, ColorFormat::RGB, DataType::FLOAT } },
				{ "material", { AttachmentType::COLOR_2, ColorFormat::RGBA,   ColorFormat::RGBA, DataType::UNSIGNED_BYTE } },
				{ "depth",    { AttachmentType::DEPTH,   ColorFormat::DEPTH,  ColorFormat::DEPTH, DataType::FLOAT } }
			};
			config.frameBuffers["gBuffer"] = gBufferData;

			FramebufferData colorBufferData;
			colorBufferData.width = width;
			colorBufferData.height = height;
			colorBufferData.textures = {
				{ "color", { AttachmentType::COLOR_0, ColorFormat::R11F_G11F_B10F, ColorFormat::RGB, DataType::FLOAT } }
			};
			config.frameBuffers["colorBuffer"] = colorBufferData;

			FramebufferData depthBufferData;
			depthBufferData.width = 1024;
			depthBufferData.height = 1024;
			depthBufferData.resize = false;
			depthBufferData.textures = {
				{ "depth", { AttachmentType::DEPTH, ColorFormat::DEPTH, ColorFormat::DEPTH, DataType::FLOAT } }
			};
			config.frameBuffers["depthBuffer1"] = depthBufferData;
			config.frameBuffers["depthBuffer2"] = depthBufferData;
			config.frameBuffers["depthBuffer3"] = depthBufferData;
			config.frameBuffers["depthBuffer4"] = depthBufferData;

			FramebufferData bloomBufferData;
			bloomBufferData.width = width;
			bloomBufferData.height = height;
			bloomBufferData.textures = {
				{ "color", { AttachmentType::COLOR_0, ColorFormat::R11F_G11F_B10F, ColorFormat::RGB, DataType::FLOAT } }
			};

			for (size_t i = 0; i < std::get<uint32_t>(config.parameters.at("bloom_mip_count")); ++i) {
				bloomBufferData.width /= 2;
				bloomBufferData.height /= 2;
				bloomBufferData.resizeFactor /= 2.0f;

				config.frameBuffers["bloomBuffer" + std::to_string(i + 1)] = bloomBufferData;
			}

			renderer.initialize(window, config);
			return renderer;
		}
	};

}
