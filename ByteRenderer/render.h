#pragma once

#include "renderer.h"

namespace Byte {

	struct RendererGenerator {
		static Renderer deferred(Window& window) {
			Renderer renderer{ Renderer::build<SkyboxPass,ShadowPass,GeometryPass,LightingPass,DrawPass>() };
			RenderConfig config;

			config.shaderPaths["quad"] =
			{ "quad.vert", "quad.frag" };
			config.shaderPaths["quad_depth"] =
			{ "quad.vert", "quad_depth.frag" };
			config.shaderPaths["lighting"] =
			{ "quad.vert", "lighting.frag" };
			config.shaderPaths["point_light"] =
			{ "point_light.vert", "point_light.frag" };
			config.shaderPaths["depth"] =
			{ "depth.vert", "depth.frag" };
			config.shaderPaths["instanced_depth"] =
			{ "instanced_depth.vert", "depth.frag" };
			config.shaderPaths["procedural_skybox"] =
			{ "procedural_skybox.vert", "procedural_skybox.frag" };
			config.shaderPaths["deferred"] =
			{ "default.vert", "cascaded_deferred.frag" };
			config.shaderPaths["instanced_deferred"] =
			{ "instanced.vert", "cascaded_deferred.frag" };

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

			config.meshes.emplace("cube", MeshBuilder::cube());
			config.meshes.emplace("quad", MeshBuilder::quad());
			config.meshes.emplace("sphere", MeshBuilder::sphere(1, 10));

			FramebufferConfig gBufferConfig;
			gBufferConfig.width = window.width();
			gBufferConfig.height = window.height();

			gBufferConfig.attachments = {
				{ "position", AttachmentType::COLOR_0, ColorFormat::RGBA16F, ColorFormat::RGBA, DataType::FLOAT },
				{ "normal", AttachmentType::COLOR_1,ColorFormat::RGBA16F, ColorFormat::RGBA, DataType::FLOAT },
				{ "albedoSpecular", AttachmentType::COLOR_2, ColorFormat::RGBA16F, ColorFormat::RGBA, DataType::UNSIGNED_BYTE }
			};

			config.frameBufferConfigs["gBuffer"] = gBufferConfig;

			FramebufferConfig colorBufferConfig;
			colorBufferConfig.width = window.width();
			colorBufferConfig.height = window.height();

			colorBufferConfig.attachments = {
				{ "albedoSpecular", AttachmentType::COLOR_0, ColorFormat::RGBA, ColorFormat::RGBA, DataType::UNSIGNED_BYTE },
			};

			config.frameBufferConfigs["colorBuffer"] = colorBufferConfig;

			FramebufferConfig depthBufferConfig;
			depthBufferConfig.attachments = {
				{ "depth", AttachmentType::DEPTH, ColorFormat::DEPTH, ColorFormat::DEPTH, DataType::FLOAT },
			};

			depthBufferConfig.width = 1024;
			depthBufferConfig.height = 1024;

			config.frameBufferConfigs["depthBuffer1"] = depthBufferConfig;
			config.frameBufferConfigs["depthBuffer2"] = depthBufferConfig;
			config.frameBufferConfigs["depthBuffer3"] = depthBufferConfig;
			config.frameBufferConfigs["depthBuffer4"] = depthBufferConfig;

			renderer.initialize(window, config);

			return renderer;
		}

	};
	
};