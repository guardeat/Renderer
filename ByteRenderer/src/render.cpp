#pragma once

#include "render.h"

namespace Byte {

	Renderer deferredRenderer(Window& window) {
		Renderer renderer{
				Renderer::build<
				FrustumCullingPass,
				SkyboxPass,
				ShadowPass,
				OpaquePass,
				SSAOPass,
				LightingPass,
				TransparentPass,
				BloomPass,
				DrawPass>() };
		size_t width{ window.width() };
		size_t height{ window.height() };

		renderer.data().shaders["quad"] = { "../ByteRenderer/shader/quad.vert", "../ByteRenderer/shader/quad.frag" };
		renderer.data().shaders["quad_depth"] = { "../ByteRenderer/shader/quad.vert", "../ByteRenderer/shader/quad_depth.frag" };
		renderer.data().shaders["lighting"] = { "../ByteRenderer/shader/quad.vert", "../ByteRenderer/shader/lighting.frag" };
		renderer.data().shaders["point_light"] = { "../ByteRenderer/shader/point_light.vert", "../ByteRenderer/shader/point_light.frag" };
		renderer.data().shaders["depth"] = { "../ByteRenderer/shader/depth.vert", "../ByteRenderer/shader/depth.frag" };
		renderer.data().shaders["instanced_depth"] = { "../ByteRenderer/shader/instanced_depth.vert", "../ByteRenderer/shader/depth.frag" };
		renderer.data().shaders["procedural_skybox"] = { "../ByteRenderer/shader/procedural_skybox.vert", "../ByteRenderer/shader/procedural_skybox.frag" };
		renderer.data().shaders["deferred"] = { "../ByteRenderer/shader/default.vert", "../ByteRenderer/shader/deferred.frag" };
		renderer.data().shaders["instanced_deferred"] = { "../ByteRenderer/shader/instanced.vert", "../ByteRenderer/shader/deferred.frag" };
		renderer.data().shaders["transparency"] = { "../ByteRenderer/shader/default.vert", "../ByteRenderer/shader/forward.frag" };
		renderer.data().shaders["instanced_transparency"] = { "../ByteRenderer/shader/instanced.vert", "../ByteRenderer/shader/forward.frag" };
		renderer.data().shaders["bloom_upsample"] = { "../ByteRenderer/shader/quad.vert", "../ByteRenderer/shader/bloom_upsample.frag" };
		renderer.data().shaders["bloom_downsample"] = { "../ByteRenderer/shader/quad.vert", "../ByteRenderer/shader/bloom_downsample.frag" };
		renderer.data().shaders["ssao"] = { "../ByteRenderer/shader/quad.vert", "../ByteRenderer/shader/ssao.frag" };
		renderer.data().shaders["blur"] = { "../ByteRenderer/shader/quad.vert", "../ByteRenderer/shader/blur.frag" };
		renderer.data().shaders["fxaa"] = { "../ByteRenderer/shader/quad.vert", "../ByteRenderer/shader/fxaa.frag" };


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
		renderer.data().parameters.emplace("render_ssao", true);
		renderer.data().parameters.emplace("render_fxaa", true);

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
		renderer.data().frameBuffers.emplace("depthBuffer1", FramebufferData{ depthBufferData });
		renderer.data().frameBuffers.emplace("depthBuffer2", FramebufferData{ depthBufferData });
		renderer.data().frameBuffers.emplace("depthBuffer3", FramebufferData{ depthBufferData });
		renderer.data().frameBuffers.emplace("depthBuffer4", FramebufferData{ depthBufferData });

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

		FramebufferData ssaoBufferData;
		ssaoBufferData.width = width;
		ssaoBufferData.height = height;
		ssaoBufferData.textures = {
			{ "color", { AttachmentType::COLOR_0, ColorFormat::RED, ColorFormat::RED, DataType::FLOAT } }
		};

		renderer.data().frameBuffers.emplace("ssaoBuffer", std::move(ssaoBufferData));

		FramebufferData blurBufferData;
		blurBufferData.width = width;
		blurBufferData.height = height;
		blurBufferData.textures = {
			{ "color", { AttachmentType::COLOR_0, ColorFormat::RED, ColorFormat::RED, DataType::FLOAT } }
		};

		renderer.data().frameBuffers.emplace("blurBuffer", std::move(blurBufferData));

		renderer.initialize(window);
		return renderer;
	}

}