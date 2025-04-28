#pragma once

#include "renderer.h"
#include "render_type.h"

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
			{ "default.vert", "deferred.frag" };
			config.shaderPaths["instanced_deferred"] =
			{ "instanced.vert", "deferred.frag" };

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
			config.meshes.emplace("low_poly_sphere", MeshBuilder::sphere(1, 4));

			FramebufferData gBufferData;
			gBufferData.width = window.width();
			gBufferData.height = window.height();

			gBufferData.textures = {
				{ "normal", {AttachmentType::COLOR_0,ColorFormat::RGB16F, ColorFormat::RGB, DataType::FLOAT} },
				{ "albedo", {AttachmentType::COLOR_1, ColorFormat::RGB16F, ColorFormat::RGB, DataType::FLOAT} },
				{ "material", {AttachmentType::COLOR_2, ColorFormat::RGBA, ColorFormat::RGBA, DataType::UNSIGNED_BYTE} },
				{ "depth", {AttachmentType::DEPTH, ColorFormat::DEPTH, ColorFormat::DEPTH, DataType::FLOAT} },
			};

			config.frameBuffers["gBuffer"] = gBufferData;

			FramebufferData colorBufferData;
			colorBufferData.width = window.width();
			colorBufferData.height = window.height();

			colorBufferData.textures = {
				{ "albedo", {AttachmentType::COLOR_0, ColorFormat::RGB16F, ColorFormat::RGB, DataType::FLOAT}},
			};

			config.frameBuffers["colorBuffer"] = colorBufferData;

			FramebufferData depthBufferData;
			depthBufferData.textures = {
				{ "depth", {AttachmentType::DEPTH, ColorFormat::DEPTH, ColorFormat::DEPTH, DataType::FLOAT} },
			};

			depthBufferData.width = 1024;
			depthBufferData.height = 1024;
			
			depthBufferData.dynamicResize = false;

			config.frameBuffers["depthBuffer1"] = depthBufferData;
			config.frameBuffers["depthBuffer2"] = depthBufferData;
			config.frameBuffers["depthBuffer3"] = depthBufferData;
			config.frameBuffers["depthBuffer4"] = depthBufferData;

			renderer.initialize(window, config);

			return renderer;
		}

	};
	
};