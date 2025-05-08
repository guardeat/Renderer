#pragma once

#include "renderer.h"
#include "render_type.h"
#include "default_pass.h"

namespace Byte {

	struct RendererGenerator {
		static Renderer deferred(Window& window) {
			Renderer renderer{ Renderer::build<SkyboxPass, ShadowPass, GeometryPass, LightingPass, BloomPass, DrawPass>() };
			size_t width{ window.width() };
			size_t height{ window.height() };

			renderer.data().shaders["quad"] = { "resource/shader/quad.vert", "resource/shader/quad.frag" };
			renderer.data().shaders["quad_depth"] = { "resource/shader/quad.vert", "resource/shader/quad_depth.frag" };
			renderer.data().shaders["lighting"] = { "resource/shader/quad.vert", "resource/shader/lighting.frag" };
			renderer.data().shaders["point_light"] = { "resource/shader/point_light.vert", "resource/shader/point_light.frag" };
			renderer.data().shaders["depth"] = { "resource/shader/depth.vert", "resource/shader/depth.frag" };
			renderer.data().shaders["instanced_depth"] = { "resource/shader/instanced_depth.vert", "resource/shader/depth.frag" };
			renderer.data().shaders["procedural_skybox"] = { "resource/shader/procedural_skybox.vert", "resource/shader/procedural_skybox.frag" };
			renderer.data().shaders["deferred"] = { "resource/shader/default.vert", "resource/shader/deferred.frag" };
			renderer.data().shaders["instanced_deferred"] = { "resource/shader/instanced.vert", "resource/shader/deferred.frag" };
			renderer.data().shaders["bloom_upsample"] = { "resource/shader/quad.vert", "resource/shader/bloom_upsample.frag" };
			renderer.data().shaders["bloom_downsample"] = { "resource/shader/quad.vert", "resource/shader/bloom_downsample.frag" };

			renderer.data().parameters.emplace("render_skybox", true);
			renderer.data().parameters.emplace("render_shadow", true);
			renderer.data().parameters.emplace("clear_gbuffer", true);
			renderer.data().parameters.emplace("gamma", 2.2f);
			renderer.data().parameters.emplace("cascade_count", 4U);
			renderer.data().parameters.emplace("cascade_divisor_1", 1.0f);
			renderer.data().parameters.emplace("cascade_divisor_2", 4.0f);
			renderer.data().parameters.emplace("cascade_divisor_3", 8.0f);
			renderer.data().parameters.emplace("cascade_divisor_4", 20.0f);
			renderer.data().parameters.emplace("cascade_light_1", Mat4{});
			renderer.data().parameters.emplace("cascade_light_2", Mat4{});
			renderer.data().parameters.emplace("cascade_light_3", Mat4{});
			renderer.data().parameters.emplace("cascade_light_4", Mat4{});
			renderer.data().parameters.emplace("current_shadow_draw_frame", 0U);
			renderer.data().parameters.emplace("shadow_draw_frame", 4U);
			renderer.data().parameters.emplace("render_bloom", true);
			renderer.data().parameters.emplace("bloom_mip_count", 5U);
			renderer.data().parameters.emplace("bloom_strength", 0.3f);

			renderer.data().meshes.emplace("cube", MeshBuilder::cube());
			renderer.data().meshes.emplace("quad", MeshBuilder::quad());
			renderer.data().meshes.emplace("sphere", MeshBuilder::sphere(1, 10));
			renderer.data().meshes.emplace("low_poly_sphere", MeshBuilder::sphere(1, 4));

			FramebufferData gBufferData;
			gBufferData.width = width;
			gBufferData.height = height;
			gBufferData.textures = {
				{ "normal",   { AttachmentType::COLOR_0, ColorFormat::RGB16F, ColorFormat::RGB, DataType::FLOAT } },
				{ "albedo",   { AttachmentType::COLOR_1, ColorFormat::RGB16F, ColorFormat::RGB, DataType::FLOAT } },
				{ "material", { AttachmentType::COLOR_2, ColorFormat::RGBA,   ColorFormat::RGBA, DataType::UNSIGNED_BYTE } },
				{ "depth",    { AttachmentType::DEPTH,   ColorFormat::DEPTH,  ColorFormat::DEPTH, DataType::FLOAT } }
			};
			renderer.data().frameBuffers.emplace("gBuffer", std::move(gBufferData));

			FramebufferData colorBufferData;
			colorBufferData.width = width;
			colorBufferData.height = height;
			colorBufferData.textures = {
				{ "color", { AttachmentType::COLOR_0, ColorFormat::R11F_G11F_B10F, ColorFormat::RGB, DataType::FLOAT } }
			};
			renderer.data().frameBuffers.emplace("colorBuffer", std::move(colorBufferData));

			FramebufferData depthBufferData;
			depthBufferData.width = 1024;
			depthBufferData.height = 1024;
			depthBufferData.resize = false;
			depthBufferData.textures = {
				{ "depth", { AttachmentType::DEPTH, ColorFormat::DEPTH, ColorFormat::DEPTH, DataType::FLOAT } }
			};
			renderer.data().frameBuffers.emplace("depthBuffer1", FramebufferData{depthBufferData});
			renderer.data().frameBuffers.emplace("depthBuffer2", FramebufferData{depthBufferData});
			renderer.data().frameBuffers.emplace("depthBuffer3", FramebufferData{depthBufferData});
			renderer.data().frameBuffers.emplace("depthBuffer4", FramebufferData{depthBufferData});

			FramebufferData bloomBufferData;
			bloomBufferData.width = width;
			bloomBufferData.height = height;
			bloomBufferData.textures = {
				{ "color", { AttachmentType::COLOR_0, ColorFormat::R11F_G11F_B10F, ColorFormat::RGB, DataType::FLOAT } }
			};

			for (size_t i{ 0 }; i < renderer.parameter<uint32_t>("bloom_mip_count"); ++i) {
				bloomBufferData.width /= 2;
				bloomBufferData.height /= 2;
				bloomBufferData.resizeFactor /= 2.0f;

				renderer.data().frameBuffers.emplace("bloomBuffer" + std::to_string(i + 1), FramebufferData{ bloomBufferData });
			}

			renderer.initialize(window);
			return renderer;
		}
	};

}
