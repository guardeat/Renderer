#pragma once

#include "render.h"

namespace Byte {

	namespace {
		void setupShaders(Renderer& renderer) {
			auto& shaders{ renderer.data().shaders };

			// Basic rendering shaders
			shaders["quad"] = {
				"../ByteRenderer/shader/quad.vert",
				"../ByteRenderer/shader/quad.frag"
			};
			shaders["quad_depth"] = {
				"../ByteRenderer/shader/quad.vert",
				"../ByteRenderer/shader/quad_depth.frag"
			};
			shaders["deferred"] = {
				"../ByteRenderer/shader/default.vert",
				"../ByteRenderer/shader/deferred.frag"
			};
			shaders["instanced_deferred"] = {
				"../ByteRenderer/shader/instanced.vert",
				"../ByteRenderer/shader/deferred.frag"
			};

			// Lighting shaders
			shaders["lighting"] = {
				"../ByteRenderer/shader/quad.vert",
				"../ByteRenderer/shader/lighting.frag"
			};
			shaders["point_light"] = {
				"../ByteRenderer/shader/point_light.vert",
				"../ByteRenderer/shader/point_light.frag"
			};

			// Shadow shaders
			shaders["depth"] = {
				"../ByteRenderer/shader/depth.vert",
				"../ByteRenderer/shader/depth.frag"
			};
			shaders["instanced_depth"] = {
				"../ByteRenderer/shader/instanced_depth.vert",
				"../ByteRenderer/shader/depth.frag"
			};

			// Skybox shader
			shaders["procedural_skybox"] = {
				"../ByteRenderer/shader/procedural_skybox.vert",
				"../ByteRenderer/shader/procedural_skybox.frag"
			};

			// Transparency shaders
			shaders["transparency"] = {
				"../ByteRenderer/shader/default.vert",
				"../ByteRenderer/shader/forward.frag"
			};
			shaders["instanced_transparency"] = {
				"../ByteRenderer/shader/instanced.vert",
				"../ByteRenderer/shader/forward.frag"
			};

			// Post-processing shaders
			shaders["bloom_upsample"] = {
				"../ByteRenderer/shader/quad.vert",
				"../ByteRenderer/shader/bloom_upsample.frag"
			};
			shaders["bloom_downsample"] = {
				"../ByteRenderer/shader/quad.vert",
				"../ByteRenderer/shader/bloom_downsample.frag"
			};
			shaders["ssao"] = {
				"../ByteRenderer/shader/quad.vert",
				"../ByteRenderer/shader/ssao.frag"
			};
			shaders["blur"] = {
				"../ByteRenderer/shader/quad.vert",
				"../ByteRenderer/shader/blur.frag"
			};
			shaders["fxaa"] = {
				"../ByteRenderer/shader/quad.vert",
				"../ByteRenderer/shader/fxaa.frag"
			};
		}

		void setupRenderingParameters(Renderer& renderer) {
			auto& params{ renderer.data().parameters };

			// Basic rendering toggles
			params.emplace("render_skybox", true);
			params.emplace("render_shadow", true);
			params.emplace("clear_gbuffer", true);
			params.emplace("gamma", 2.2f);

			// Shadow cascade parameters
			params.emplace("cascade_count", 4U);
			params.emplace("cascade_divisor_1", 1.0f);
			params.emplace("cascade_divisor_2", 4.0f);
			params.emplace("cascade_divisor_3", 8.0f);
			params.emplace("cascade_divisor_4", 20.0f);
			params.emplace("cascade_light_1", Mat4{});
			params.emplace("cascade_light_2", Mat4{});
			params.emplace("cascade_light_3", Mat4{});
			params.emplace("cascade_light_4", Mat4{});
			params.emplace("current_shadow_draw_frame", 0U);
			params.emplace("shadow_draw_frame", 4U);

			// Post-processing parameters
			params.emplace("render_bloom", true);
			params.emplace("bloom_mip_count", 5U);
			params.emplace("bloom_strength", 0.3f);
			params.emplace("render_ssao", true);
			params.emplace("render_fxaa", true);
		}

		void setupMeshes(Renderer& renderer) {
			auto& meshes{ renderer.data().meshes };

			meshes.emplace("cube", RenderData::RenderMesh{ MeshBuilder::cube(), MeshRenderer{} });
			meshes.emplace("quad", RenderData::RenderMesh{ MeshBuilder::quad(), MeshRenderer{} });
			meshes.emplace("sphere", RenderData::RenderMesh{ MeshBuilder::sphere(1, 10), MeshRenderer{} });
			meshes.emplace("low_poly_sphere", RenderData::RenderMesh{ MeshBuilder::sphere(1, 4), MeshRenderer{} });
		}

		void setupFramebuffers(Renderer& renderer, size_t width, size_t height) {
			auto& frameBuffers{ renderer.data().frameBuffers };

			// G-Buffer for deferred rendering
			FramebufferData gBufferData;
			gBufferData.width = width;
			gBufferData.height = height;
			gBufferData.textures = {
				{
					"normal",
					{
						AttachmentType::COLOR_0,
						ColorFormat::RGB16F,
						ColorFormat::RGB,
						DataType::FLOAT
					}
				},
				{
					"albedo",
					{
						AttachmentType::COLOR_1,
						ColorFormat::RGB16F,
						ColorFormat::RGB,
						DataType::FLOAT
					}
				},
				{
					"material",
					{
						AttachmentType::COLOR_2,
						ColorFormat::RGBA,
						ColorFormat::RGBA,
						DataType::UNSIGNED_BYTE
					}
				},
				{
					"depth",
					{
						AttachmentType::DEPTH,
						ColorFormat::DEPTH,
						ColorFormat::DEPTH,
						DataType::FLOAT
					}
				}
			};
			frameBuffers.emplace("gBuffer", std::move(gBufferData));

			// Main color buffer
			FramebufferData colorBufferData;
			colorBufferData.width = width;
			colorBufferData.height = height;
			colorBufferData.textures = {
				{
					"color",
					{
						AttachmentType::COLOR_0,
						ColorFormat::R11F_G11F_B10F,
						ColorFormat::RGB,
						DataType::FLOAT
					}
				}
			};
			frameBuffers.emplace("colorBuffer", std::move(colorBufferData));

			// Shadow depth buffers (cascade shadow maps)
			FramebufferData depthBufferData;
			depthBufferData.width = 1024;
			depthBufferData.height = 1024;
			depthBufferData.resize = false;
			depthBufferData.textures = {
				{
					"depth",
					{
						AttachmentType::DEPTH,
						ColorFormat::DEPTH,
						ColorFormat::DEPTH,
						DataType::FLOAT
					}
				}
			};
			frameBuffers.emplace("depthBuffer1", FramebufferData{ depthBufferData });
			frameBuffers.emplace("depthBuffer2", FramebufferData{ depthBufferData });
			frameBuffers.emplace("depthBuffer3", FramebufferData{ depthBufferData });
			frameBuffers.emplace("depthBuffer4", FramebufferData{ depthBufferData });

			// Bloom buffers (mip chain)
			FramebufferData bloomBufferData;
			bloomBufferData.width = width;
			bloomBufferData.height = height;
			bloomBufferData.textures = {
				{
					"color",
					{
						AttachmentType::COLOR_0,
						ColorFormat::R11F_G11F_B10F,
						ColorFormat::RGB,
						DataType::FLOAT
					}
				}
			};

			for (size_t i{ 0 }; i < renderer.parameter<uint32_t>("bloom_mip_count"); ++i) {
				bloomBufferData.width /= 2;
				bloomBufferData.height /= 2;
				bloomBufferData.resizeFactor /= 2.0f;
				frameBuffers.emplace(
					"bloomBuffer" + std::to_string(i + 1),
					FramebufferData{ bloomBufferData }
				);
			}

			// SSAO buffer
			FramebufferData ssaoBufferData;
			ssaoBufferData.width = width;
			ssaoBufferData.height = height;
			ssaoBufferData.textures = {
				{
					"color",
					{
						AttachmentType::COLOR_0,
						ColorFormat::RED,
						ColorFormat::RED,
						DataType::FLOAT
					}
				}
			};
			frameBuffers.emplace("ssaoBuffer", std::move(ssaoBufferData));

			// Blur buffer (for SSAO blur)
			FramebufferData blurBufferData;
			blurBufferData.width = width;
			blurBufferData.height = height;
			blurBufferData.textures = {
				{
					"color",
					{
						AttachmentType::COLOR_0,
						ColorFormat::RED,
						ColorFormat::RED,
						DataType::FLOAT
					}
				}
			};
			frameBuffers.emplace("blurBuffer", std::move(blurBufferData));
		}
	}

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
				DrawPass>()
		};

		const size_t width{ window.width() };
		const size_t height{ window.height() };

		setupShaders(renderer);
		setupRenderingParameters(renderer);
		setupMeshes(renderer);
		setupFramebuffers(renderer, width, height);

		renderer.initialize(window);
		return renderer;
	}

}