#pragma once

#include "renderer.h"

namespace Byte {

	struct RendererGenerator {
		static Renderer deferred(Window& window) {
			Renderer renderer{ Renderer::build<SkyboxPass,CascadedShadowPass,GeometryPass,LightingPass,DrawPass>() };
			RenderConfig config;

			config.shaderPaths["default_deferred"] =
			{ "shadowed.vert", "shadowed_deferred.frag" };
			config.shaderPaths["default_forward"] =
			{ "default.vert", "forward.frag" };
			config.shaderPaths["quad"] =
			{ "quad.vert", "quad.frag" };
			config.shaderPaths["lighting"] =
			{ "quad.vert", "lighting.frag" };
			config.shaderPaths["point_light"] =
			{ "point_light.vert", "point_light.frag" };
			config.shaderPaths["instanced_deferred"] =
			{ "instanced_shadowed.vert", "shadowed_deferred.frag" };
			config.shaderPaths["depth"] =
			{ "depth.vert", "depth.frag" };
			config.shaderPaths["instanced_depth"] =
			{ "instanced_depth.vert", "depth.frag" };
			config.shaderPaths["quad_depth"] =
			{ "quad.vert", "quad_depth.frag" };
			config.shaderPaths["procedural_skybox"] =
			{ "procedural_skybox.vert", "procedural_skybox.frag" };
			config.shaderPaths["cascaded_deferred"] =
			{ "cascaded.vert", "cascaded_deferred.frag" };
			config.shaderPaths["instanced_cascaded_deferred"] =
			{ "instanced_cascaded.vert", "cascaded_deferred.frag" };

			config.parameters.emplace("render_skybox", true);
			config.parameters.emplace("render_shadow", true);
			config.parameters.emplace("clear_gbuffer", true);
			config.parameters.emplace("cascade_count", 2U);
			config.parameters.emplace("cascade_divisor_1", 1.0f);
			config.parameters.emplace("cascade_divisor_2", 5.0f);
			config.parameters.emplace("cascade_divisor_3", 10.0f);
			config.parameters.emplace("cascade_divisor_4", 25.0f);

			config.meshes.emplace("cube", MeshBuilder::cube());
			config.meshes.emplace("quad", MeshBuilder::quad());
			config.meshes.emplace("sphere", MeshBuilder::sphere(1, 20));

			FramebufferConfig gBufferConfig;

			gBufferConfig.width = window.width();
			gBufferConfig.height = window.height();

			gBufferConfig.attachments = {
				{ "position", AttachmentType::COLOR_0, ColorFormat::RGBA16F, ColorFormat::RGBA, DataType::FLOAT },
				{ "normal", AttachmentType::COLOR_1,ColorFormat::RGBA16F, ColorFormat::RGBA, DataType::FLOAT },
				{ "albedoSpecular", AttachmentType::COLOR_2, ColorFormat::RGBA, ColorFormat::RGBA, DataType::UNSIGNED_BYTE }
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

			depthBufferConfig.width = 2048;
			depthBufferConfig.height = 2048;

			config.frameBufferConfigs["depthBuffer"] = depthBufferConfig;

			renderer.initialize(window, config);

			return renderer;
		}
	};

}