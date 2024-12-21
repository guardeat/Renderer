#pragma once

#include <variant>
#include <unordered_map>
#include <cstdint>

#include "context.h"

namespace Byte {

	struct RenderData {
		size_t height{ 0 };
		size_t width{ 0 };

		using FramebufferMap = std::unordered_map<FramebufferTag, Framebuffer>;
		FramebufferMap frameBuffers;

		using ShaderMap = std::unordered_map<ShaderTag, Shader>;
		ShaderMap shaders;

		using Variant = std::variant<std::string, uint32_t, int32_t, bool, float>;
		using ParameterTag = std::string;
		using ParameterMap = std::unordered_map<ParameterTag, Variant>;
		ParameterMap parameters;

		using MeshMap = std::unordered_map<MeshTag, Mesh>;
		MeshMap meshes;

		template<typename Type>
		Type& parameter(const ParameterTag& tag) {
			return std::get<Type>(parameters.at(tag));
		}
	};

	class RenderPass {
	public:
		virtual void render(RenderContext& context, RenderData& data) = 0;
	};

	class SkyboxPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			if (!data.parameter<bool>("render_skybox")) {
				return;
			}

			Framebuffer& gBuffer{ data.frameBuffers["gBuffer"] };

			gBuffer.bind();
			gBuffer.clearContent();

			data.parameter<bool>("clear_gbuffer") = false;

			Shader& skyboxShader{ data.shaders["procedural_skybox"] };

			float aspectRatio{ static_cast<float>(data.width) / static_cast<float>(data.height) };
			auto [camera, cTransform] = context.camera();
			Mat4 projection{ camera->perspective(aspectRatio) };
			Mat4 view{ cTransform->view() };

			auto [dl, dlTransform] = context.directionalLight();
			skyboxShader.bind();

			OpenglAPI::disableDepth();

			skyboxShader.uniform<Mat4>("uProjection", projection);
			skyboxShader.uniform<Quaternion>("uRotation", cTransform->rotation());
			skyboxShader.uniform<Vec3>("uDirectionalLight.direction", dlTransform->front());
			skyboxShader.uniform<Vec3>("uDirectionalLight.color", dl->color);
			skyboxShader.uniform<float>("uDirectionalLight.intensity", dl->intensity);

			data.meshes.at("cube").renderArray().bind();

			OpenglAPI::Draw::elements(data.meshes.at("cube").indices().size());

			data.meshes.at("cube").renderArray().unbind();
			gBuffer.unbind();

			OpenglAPI::enableDepth();
		}
	};

	class ShadowPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			if (!data.parameter<bool>("render_shadow")) {
				return;
			}

			Shader& depthShader{ data.shaders["depth"] };
			Shader& instancedDepthShader{ data.shaders["instanced_depth"] };
			Framebuffer& depthBuffer{ data.frameBuffers["depthBuffer"] };
			
			float aspectRatio{ static_cast<float>(data.width) / static_cast<float>(data.height) };
			auto [camera, cTransform] = context.camera();
			Mat4 projection{ camera->perspective(aspectRatio) };

			auto [_, dlTransform] = context.directionalLight();

			Mat4 lightSpace{ camera->frustumSpace(projection,cTransform->view(),*dlTransform) };

			depthBuffer.bind();
			depthBuffer.clearContent();

			OpenglAPI::enableCulling();
			OpenglAPI::cullFront();

			depthShader.bind();
			depthShader.uniform<Mat4>("uLightSpace", lightSpace);

			renderEntities(context, depthShader);

			instancedDepthShader.bind();
			instancedDepthShader.uniform<Mat4>("uLightSpace", lightSpace);
			renderInstances(context);

			OpenglAPI::cullBack();
			OpenglAPI::disableCulling();

			depthBuffer.unbind();
		}

	private:
		void renderEntities(RenderContext& context, const Shader& shader) {
			for (auto& pair: context.renderEntities()) {
				auto [mesh, material, transform] = pair.second;

				mesh->renderArray().bind();

				shader.uniform<Vec3>("uPosition", transform->position());
				shader.uniform<Vec3>("uScale", transform->scale());
				shader.uniform<Quaternion>("uRotation", transform->rotation());

				OpenglAPI::Draw::elements(mesh->indices().size());

				mesh->renderArray().unbind();
			}
		}

		void renderInstances(RenderContext& context) {
			for (auto& pair : context.instances()) {
				Mesh& mesh{ pair.second.mesh() };
				Material& material{ pair.second.material() };

				mesh.renderArray().bind();

				OpenglAPI::Draw::instancedElements(mesh.indices().size(), pair.second.size());

				mesh.renderArray().unbind();
			}
		}

	};

	class GeometryPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			float aspectRatio{ static_cast<float>(data.width) / static_cast<float>(data.height) };
			auto [camera, cTransform] = context.camera();
			Mat4 projection{ camera->perspective(aspectRatio) };
			Mat4 view{ cTransform->view() };

			auto [_, dlTransform] = context.directionalLight();
			Mat4 lightSpace{ camera->frustumSpace(projection,view, *dlTransform) };

			Framebuffer& gBuffer{ data.frameBuffers["gBuffer"] };
			gBuffer.bind();

			if (data.parameter<bool>("clear_gbuffer")) {
				gBuffer.clearContent();
			}

			renderEntities(context, data, projection, view, lightSpace, dlTransform->front());
			renderInstances(context, data, projection, view, lightSpace, dlTransform->front());

			gBuffer.unbind();

			data.parameter<bool>("clear_gbuffer") = true;
		}

	private:
		void renderEntities(
			RenderContext& context, 
			RenderData& data, 
			const Mat4& projection, 
			const Mat4& view,
			const Mat4& lightSpace,
			const Vec3& lightDir) {
			Framebuffer& depthBuffer{ data.frameBuffers["depthBuffer"] };

			for (auto& pair : context.renderEntities()) {
				auto [mesh, material, transform] = pair.second;

				Shader& shader{ data.shaders[material->shaderTag()] };
				shader.bind();

				mesh->renderArray().bind();

				shader.uniform<Vec4>("uAlbedo", material->albedo());

				shader.uniform<Vec3>("uPosition", transform->position());
				shader.uniform<Vec3>("uScale", transform->scale());
				shader.uniform<Quaternion>("uRotation", transform->rotation());

				shader.uniform<Mat4>("uProjection", projection);
				shader.uniform<Mat4>("uView", view);
				shader.uniform<Mat4>("uLightSpace", lightSpace);

				shader.uniform<Vec3>("uLightDir", lightDir);

				shader.uniform("uDepthMap", 0);
				OpenglAPI::Texture::bind(depthBuffer.textureID("depth"), TextureUnit::T0);

				OpenglAPI::Draw::elements(mesh->indices().size());

				shader.unbind();
				mesh->renderArray().unbind();
			}
		}

		void renderInstances(
			RenderContext& context, 
			RenderData& data, 
			const Mat4& projection, 
			const Mat4& view,
			const Mat4& lightSpace,
			const Vec3& lightDir) {

			Framebuffer& depthBuffer{ data.frameBuffers["depthBuffer"] };
			for (auto& pair : context.instances()) {
				Mesh& mesh{ pair.second.mesh() };
				Material& material{ pair.second.material() };

				Shader& shader{ data.shaders[material.shaderTag()] };
				shader.bind();

				mesh.renderArray().bind();

				shader.uniform<Vec4>("uAlbedo", material.albedo());

				shader.uniform<Mat4>("uProjection", projection);
				shader.uniform<Mat4>("uView", view);
				shader.uniform<Mat4>("uLightSpace", lightSpace);

				shader.uniform<Vec3>("uLightDir", lightDir);

				shader.uniform("uDepthMap", 0);
				OpenglAPI::Texture::bind(depthBuffer.textureID("depth"), TextureUnit::T0);

				OpenglAPI::Draw::instancedElements(mesh.indices().size(), pair.second.size());

				shader.unbind();
				mesh.renderArray().unbind();
			}
		}

	};

	class LightingPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			Shader& lightingShader{ data.shaders["lighting"] };
			Shader& plShader{ data.shaders["point_light"] };

			Framebuffer& gBuffer{ data.frameBuffers["gBuffer"] };
			Framebuffer& colorBuffer{ data.frameBuffers["colorBuffer"] };

			colorBuffer.bind();
			colorBuffer.clearContent();

			lightingShader.bind();

			bindGBufferTextures(gBuffer, lightingShader, "uPosition", "uNormal", "uAlbedoSpec");

			auto [_, cTransform] = context.camera();
			auto [directionalLight, dlTransform] = context.directionalLight();

			lightingShader.uniform<Vec3>("uViewPos", cTransform->position());
			lightingShader.uniform<Vec3>("uDirectionalLight.direction", dlTransform->front());
			lightingShader.uniform<Vec3>("uDirectionalLight.color", directionalLight->color);
			lightingShader.uniform<float>("uDirectionalLight.intensity", directionalLight->intensity);

			data.meshes.at("quad").renderArray().bind();
			OpenglAPI::Draw::quad();
			data.meshes.at("quad").renderArray().unbind();

			lightingShader.unbind();

			if (!context.pointLights().empty()) {
				plShader.bind();

				OpenglAPI::enableBlend();
				OpenglAPI::setBlend(1, 1);
				OpenglAPI::enableCulling();
				OpenglAPI::cullFront();

				plShader.uniform<Vec2>(
					"uViewPortSize",
					Vec2{ static_cast<float>(data.width), static_cast<float>(data.height) });

				float aspectRatio{ static_cast<float>(data.width) / static_cast<float>(data.height) };
				auto [camera, cTransform] = context.camera();
				Mat4 projection{ camera->perspective(aspectRatio) };
				Mat4 view{ cTransform->view() };

				plShader.uniform<Mat4>("uProjection", projection);
				plShader.uniform<Mat4>("uView", view);

				bindGBufferTextures(gBuffer, plShader, "uSPosition", "uSNormal", "uSAlbedoSpec");

				data.meshes.at("sphere").renderArray().bind();

				OpenglAPI::disableDepth();

				for (auto& pair: context.pointLights()) {
					auto [pointLight, _transform] = pair.second;
					Transform transform{ *_transform };

					float radius{ pointLight->radius() };
					transform.scale(Vec3{ radius, radius, radius });

					plShader.uniform<Vec3>("uPosition", transform.position());
					plShader.uniform<Vec3>("uScale", transform.scale());
					plShader.uniform<Quaternion>("uRotation", transform.rotation());

					plShader.uniform<Vec3>("uPointLight.position", transform.position());
					plShader.uniform<Vec3>("uPointLight.color", pointLight->color);
					plShader.uniform<float>("uPointLight.constant", pointLight->constant);
					plShader.uniform<float>("uPointLight.linear", pointLight->linear);
					plShader.uniform<float>("uPointLight.quadratic", pointLight->quadratic);

					OpenglAPI::Draw::elements(data.meshes.at("sphere").indices().size());
				}

				data.meshes.at("sphere").renderArray().unbind();
				plShader.unbind();

				OpenglAPI::enableDepth();
				OpenglAPI::disableBlend();
				OpenglAPI::cullBack();
				OpenglAPI::disableCulling();
			}

			colorBuffer.unbind();
		}

	private:
		void bindGBufferTextures(
			Framebuffer& gBuffer, 
			Shader& shader,
			const std::string& positionName,
			const std::string& normalName,
			const std::string& albedoSpecName) {
			shader.uniform(positionName, 0);
			shader.uniform(normalName, 1);
			shader.uniform(albedoSpecName, 2);

			OpenglAPI::Texture::bind(gBuffer.textureID("position"), TextureUnit::T0);
			OpenglAPI::Texture::bind(gBuffer.textureID("normal"), TextureUnit::T1);
			OpenglAPI::Texture::bind(gBuffer.textureID("albedoSpecular"), TextureUnit::T2);
		}
	};


	class DrawPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			OpenglAPI::Framebuffer::unbind();
			OpenglAPI::Framebuffer::clear(0);

			Shader& quadShader{ data.shaders["quad"] };
			Framebuffer& colorBuffer{ data.frameBuffers["colorBuffer"] };

			quadShader.bind();
			quadShader.uniform("uAlbedoSpecular", 0);
			OpenglAPI::Texture::bind(colorBuffer.textureID("albedoSpecular"), TextureUnit::T0);

			data.meshes.at("quad").renderArray().bind();

			OpenglAPI::Draw::quad();

			data.meshes.at("quad").renderArray().unbind();
		}

	};

	class DebugPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			OpenglAPI::Framebuffer::unbind();
			OpenglAPI::Framebuffer::clear(0);

			Shader& quadShader{ data.shaders["quad_depth"] };
			Framebuffer& colorBuffer{ data.frameBuffers["depthBuffer"] };

			quadShader.bind();
			quadShader.uniform("uAlbedoSpecular", 0);
			OpenglAPI::Texture::bind(colorBuffer.textureID("depth"), TextureUnit::T0);

			data.meshes.at("quad").renderArray().bind();

			OpenglAPI::Draw::quad();

			data.meshes.at("quad").renderArray().unbind();
		}

	};

}