#pragma once

#include "mesh.h"
#include "material.h"
#include "transform.h"
#include "mat.h"
#include "typedefs.h"
#include "opengl_api.h"
#include "texture.h"
#include "framebuffer.h"
#include "light.h"
#include "camera.h"

namespace Byte {

	struct RenderConfig {
		using ShaderPathMap = std::unordered_map<ShaderTag, ShaderPath>;
		ShaderPathMap shaderPaths;

		using FramebufferConfigMap = std::unordered_map<FramebufferTag, FramebufferConfig>;
		FramebufferConfigMap frameBufferConfigs;
	};

	struct RenderContext {
		Buffer<Mesh*> meshes;
		Buffer<Material*> materials;
		Buffer<Transform*> transforms;

		Camera* mainCamera{};
		Transform* mainCameraTransform{};

		DirectionalLight* directionalLight{};
		Transform* directionalLightTransform{};

		Buffer<PointLight*> pointLights;
		Buffer<Transform*> pointLightTransforms;
	};

	struct RenderData {
		size_t height{ 0 };
		size_t width{ 0 };

		using FramebufferMap = std::unordered_map<FramebufferTag, Framebuffer>;
		FramebufferMap frameBuffers;

		using ShaderMap = std::unordered_map<ShaderTag, Shader>;
		ShaderMap shaders;

		RenderArray quad;
		Mesh sphere;
	};

	class RenderPass {
	public:
		virtual void render(RenderContext& context, RenderData& data) = 0;
	};

	class GeometryPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			float aspectRatio{ static_cast<float>(data.width) / static_cast<float>(data.height) };
			Mat4 projection{ context.mainCamera->perspective(aspectRatio) };
			Mat4 view{ context.mainCamera->view(*context.mainCameraTransform) };

			Framebuffer& gBuffer{ data.frameBuffers["gBuffer"] };

			gBuffer.bind();
			gBuffer.clearContent();

			for (size_t i{ 0 }; i < context.meshes.size(); ++i) {
				Mesh& mesh{ *context.meshes[i] };
				Material& material{ *context.materials[i] };
				Transform& transform{ *context.transforms[i] };

				Shader& shader{ data.shaders[material.shaderTag()] };

				shader.bind();

				mesh.renderArray().bind();

				shader.uniform<Vec4>("uAlbedo", material.albedo());

				shader.uniform<Vec3>("uPosition", transform.position());
				shader.uniform<Vec3>("uScale", transform.scale());
				shader.uniform<Quaternion>("uRotation", transform.rotation());

				shader.uniform<Mat4>("uProjection", projection);
				shader.uniform<Mat4>("uView", view);

				OpenglAPI::Draw::elements(mesh.index().size());

				shader.unbind();
				mesh.renderArray().unbind();
			}

			gBuffer.bind();
		}

	};

	class ShadowPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {

		}
	};

	class LightingPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			Shader& lightingShader = data.shaders["lighting_shader"];
			lightingShader.bind();

			Framebuffer& gBuffer{ data.frameBuffers["gBuffer"] };
			Framebuffer& colorBuffer{ data.frameBuffers["colorBuffer"] };

			colorBuffer.bind();
			colorBuffer.clearContent();

			lightingShader.uniform("uPosition", 0);
			lightingShader.uniform("uNormal", 1);
			lightingShader.uniform("uAlbedoSpec", 2);

			lightingShader.uniform<Vec3>("uViewPos", context.mainCameraTransform->position());

			lightingShader.uniform<Vec3>(
				"uDirectionalLight.direction",
				context.directionalLightTransform->front());
			lightingShader.uniform<Vec3>(
				"uDirectionalLight.color",
				context.directionalLight->color);
			lightingShader.uniform<float>(
				"uDirectionalLight.intensity",
				context.directionalLight->intensity);

			OpenglAPI::Texture::bind(gBuffer.textureID("position"), GL_TEXTURE0);
			OpenglAPI::Texture::bind(gBuffer.textureID("normal"), GL_TEXTURE1);
			OpenglAPI::Texture::bind(gBuffer.textureID("albedoSpecular"), GL_TEXTURE2);

			data.quad.bind();

			OpenglAPI::Draw::quad();

			data.quad.unbind();

			lightingShader.unbind();

			colorBuffer.unbind();
		}

	};

	class PointLightPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			if (context.pointLights.empty()) {
				return;
			}

			Framebuffer& gBuffer{ data.frameBuffers["gBuffer"] };
			Framebuffer& colorBuffer{ data.frameBuffers["colorBuffer"] };

			Shader& pointLightShader{ data.shaders["point_light_shader"] };
			pointLightShader.bind();
			colorBuffer.bind();

			pointLightShader.uniform<Vec2>("uViewPortSize",
				Vec2{ static_cast<float>(data.width),static_cast<float>(data.height) });

			float aspectRatio{ static_cast<float>(data.width) / static_cast<float>(data.height) };
			Mat4 projection{ context.mainCamera->perspective(aspectRatio) };
			Mat4 view{ context.mainCamera->view(*context.mainCameraTransform) };

			pointLightShader.uniform<Mat4>("uProjection", projection);
			pointLightShader.uniform<Mat4>("uView", view);

			pointLightShader.uniform("uSPosition", 0);
			pointLightShader.uniform("uSNormal", 1);
			pointLightShader.uniform("uSAlbedoSpec", 2);

			OpenglAPI::Texture::bind(gBuffer.textureID("position"), GL_TEXTURE0);
			OpenglAPI::Texture::bind(gBuffer.textureID("normal"), GL_TEXTURE1);
			OpenglAPI::Texture::bind(gBuffer.textureID("albedoSpecular"), GL_TEXTURE2);

			data.sphere.renderArray().bind();

			OpenglAPI::disableDepth();

			for (size_t i{ 0 }; i < context.pointLights.size(); ++i) {
				PointLight& pointLight{ *context.pointLights[i] };
				Transform transform{ *context.pointLightTransforms[i] };

				float radius{ pointLight.radius() };
				transform.scale(Vec3{ radius,radius,radius });

				pointLightShader.uniform<Vec3>("uPosition", transform.position());
				pointLightShader.uniform<Vec3>("uScale", transform.scale());
				pointLightShader.uniform<Quaternion>("uRotation", transform.rotation());

				pointLightShader.uniform<Vec3>("uPointLight.position", transform.position());
				pointLightShader.uniform<Vec3>("uPointLight.color", pointLight.color);
				pointLightShader.uniform<float>("uPointLight.constant", pointLight.constant);
				pointLightShader.uniform<float>("uPointLight.linear", pointLight.linear);
				pointLightShader.uniform<float>("uPointLight.quadratic", pointLight.quadratic);

				OpenglAPI::Draw::elements(data.sphere.index().size());

			}
			data.sphere.renderArray().unbind();
			pointLightShader.unbind();

			OpenglAPI::enableDepth();
		}

	};

	class DrawPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			OpenglAPI::Framebuffer::unbind();
			OpenglAPI::Framebuffer::clear(0);

			Shader& quadShader{ data.shaders["quad_shader"] };
			Framebuffer& gBuffer{ data.frameBuffers["gBuffer"] };
			Framebuffer& colorBuffer{ data.frameBuffers["colorBuffer"] };

			quadShader.bind();
			quadShader.uniform("uAlbedoSpecular", 0);
			quadShader.uniform("uDirectionalLightAlbedo", 1);
			quadShader.uniform("uPointLightAlbedo", 2);
			OpenglAPI::Texture::bind(gBuffer.textureID("albedoSpecular"), GL_TEXTURE0);
			OpenglAPI::Texture::bind(colorBuffer.textureID("albedoSpecular1"), GL_TEXTURE1);
			OpenglAPI::Texture::bind(colorBuffer.textureID("albedoSpecular2"), GL_TEXTURE2);

			data.quad.bind();

			OpenglAPI::Draw::quad();

			data.quad.unbind();
		}

	};

}
