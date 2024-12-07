#pragma once

#include "renderer.h"

namespace Byte {

	struct RendererGenerator {
		static Renderer deferred(Window& window) {
			Renderer renderer{ Renderer::build<SkyboxPass,ShadowPass,GeometryPass,LightingPass,DrawPass>() };
			RenderConfig config;

			config.shaderPaths["default_deferred"] = { "shadowed_vertex.glsl","shadowed_deferred_geometry.glsl" };
			config.shaderPaths["default_forward"] = { "default_vertex.glsl","forward_fragment.glsl" };
			config.shaderPaths["quad_shader"] = { "quad_vertex.glsl","quad_fragment.glsl" };
			config.shaderPaths["lighting_shader"] = { "quad_vertex.glsl","lighting_fragment.glsl" };
			config.shaderPaths["point_light_shader"] = { "point_light_vertex.glsl","point_light_fragment.glsl" };
			config.shaderPaths["instanced_deferred"] = { "instanced_shadowed_vertex.glsl","shadowed_deferred_geometry.glsl" };
			config.shaderPaths["depth_shader"] = { "depth_vertex.glsl","depth_fragment.glsl" };
			config.shaderPaths["instanced_depth"] = { "instanced_depth_vertex.glsl","depth_fragment.glsl" };
			config.shaderPaths["quad_depth_shader"] = { "quad_vertex.glsl","quad_depth_fragment.glsl" };
			config.shaderPaths["procedural_skybox"] = { "procedural_skybox_vertex.glsl","procedural_skybox_fragment.glsl" };

			config.parameters.emplace("render_skybox", true);
			config.parameters.emplace("render_shadow", true);
			config.parameters.emplace("clear_gbuffer", true);

			config.meshes.emplace("cube", MeshBuilder::cube());
			config.meshes.emplace("quad", MeshBuilder::quad());
			config.meshes.emplace("sphere", MeshBuilder::sphere(1, 20));

			FramebufferConfig gBufferConfig;

			gBufferConfig.width = window.width();
			gBufferConfig.height = window.height();

			gBufferConfig.attachments = {
				{ "position", 0, GL_RGBA16F, GL_RGBA, GL_FLOAT },
				{ "normal", 1, GL_RGBA16F, GL_RGBA, GL_FLOAT },
				{ "albedoSpecular", 2, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE }
			};

			config.frameBufferConfigs["gBuffer"] = gBufferConfig;

			FramebufferConfig colorBufferConfig;

			colorBufferConfig.width = window.width();
			colorBufferConfig.height = window.height();

			colorBufferConfig.attachments = {
				{ "albedoSpecular", 0, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE },
			};

			config.frameBufferConfigs["colorBuffer"] = colorBufferConfig;

			FramebufferConfig depthBufferConfig;

			depthBufferConfig.width = 2048;
			depthBufferConfig.height = 2048;
			depthBufferConfig.depthMap = true;

			config.frameBufferConfigs["depthBuffer"] = depthBufferConfig;

			renderer.initialize(window, config);

			return renderer;
		}
	};

}