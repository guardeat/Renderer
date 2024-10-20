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

		Framebuffer gBuffer{};
		Framebuffer colorBuffer{};

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
			Mat4 projection{ context.mainCamera->projection(aspectRatio) };
			Mat4 view{ context.mainCamera->view(*context.mainCameraTransform) };

			data.gBuffer.bind();
			data.gBuffer.clearContent();

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

			data.gBuffer.unbind();
		}

	};

	class LightingPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			Shader& lightingShader = data.shaders["lighting_shader"];
			lightingShader.bind();

			data.colorBuffer.bind();
			data.colorBuffer.clearContent();

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

			OpenglAPI::Texture::bind(data.gBuffer.data().textures.at("position"), GL_TEXTURE0);
			OpenglAPI::Texture::bind(data.gBuffer.data().textures.at("normal"), GL_TEXTURE1);
			OpenglAPI::Texture::bind(data.gBuffer.data().textures.at("albedoSpecular"), GL_TEXTURE2);

			data.quad.bind();

			OpenglAPI::Draw::quad();

			data.quad.unbind();

			lightingShader.unbind();

			data.colorBuffer.unbind();

			OpenglAPI::Framebuffer::blit(
				data.colorBuffer.data().id, 
				data.gBuffer.data().id, 
				data.width, 
				data.height,
				GL_COLOR_ATTACHMENT0,
				GL_COLOR_ATTACHMENT2);
		}

	};

	class PointLightPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			Shader& pointLightShader{ data.shaders["point_light_shader"] };
			pointLightShader.bind();
			data.colorBuffer.bind();

			pointLightShader.uniform<Vec2>("uViewPortSize",
				Vec2{ static_cast<float>(data.width),static_cast<float>(data.height) });

			float aspectRatio{ static_cast<float>(data.width) / static_cast<float>(data.height) };
			Mat4 projection{ context.mainCamera->projection(aspectRatio) };
			Mat4 view{ context.mainCamera->view(*context.mainCameraTransform) };

			pointLightShader.uniform<Mat4>("uProjection", projection);
			pointLightShader.uniform<Mat4>("uView", view);

			pointLightShader.uniform("uSPosition", 0);
			pointLightShader.uniform("uSNormal", 1);
			pointLightShader.uniform("uSAlbedoSpec", 2);

			OpenglAPI::Texture::bind(data.gBuffer.data().textures.at("position"), GL_TEXTURE0);
			OpenglAPI::Texture::bind(data.gBuffer.data().textures.at("normal"), GL_TEXTURE1);
			OpenglAPI::Texture::bind(data.gBuffer.data().textures.at("albedoSpecular"), GL_TEXTURE2);

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

			OpenglAPI::Framebuffer::blit(
				data.colorBuffer.data().id,
				data.gBuffer.data().id,
				data.width,
				data.height,
				GL_COLOR_ATTACHMENT0,
				GL_COLOR_ATTACHMENT2);
		}

	};

	class DrawPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			OpenglAPI::Framebuffer::unbind();
			OpenglAPI::Framebuffer::clear(0);

			Shader& quadShader{data.shaders["quad_shader"]};

			quadShader.bind();
			quadShader.uniform("uAlbedoSpecular", 0);
			OpenglAPI::Texture::bind(data.gBuffer.data().textures.at("albedoSpecular"), GL_TEXTURE0);

			data.quad.bind();

			OpenglAPI::Draw::quad();

			data.quad.unbind();
		}

	};

}
