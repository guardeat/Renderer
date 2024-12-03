#pragma once

#include "context.h"

namespace Byte {

	struct RenderData {
		size_t height{ 0 };
		size_t width{ 0 };

		using FramebufferMap = std::unordered_map<FramebufferTag, Framebuffer>;
		FramebufferMap frameBuffers;

		using ShaderMap = std::unordered_map<ShaderTag, Shader>;
		ShaderMap shaders;

		Mesh quad;
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
			auto [camera, cTransform] = context.camera();
			Mat4 projection{ camera->perspective(aspectRatio) };
			Mat4 view{ cTransform->view() };

			Mat4 orthographic{ camera->orthographic(-80.0f, 80.0f, -45.0f, 45.0f) };
			auto [_, dlTransform] = context.directionalLight();
			Mat4 lightSpace{ orthographic * dlTransform->view() };

			Framebuffer& gBuffer{ data.frameBuffers["gBuffer"] };
			gBuffer.bind();
			gBuffer.clearContent();

			renderEntities(context, data, projection, view, lightSpace);
			renderInstances(context, data, projection, view, lightSpace);

			gBuffer.unbind();
		}

	private:
		void renderEntities(
			RenderContext& context, 
			RenderData& data, 
			const Mat4& projection, 
			const Mat4& view,
			const Mat4& lightSpace) {
			Framebuffer& depthBuffer{ data.frameBuffers["depthBuffer"] };

			for (size_t i{ 0 }; i < context.entityCount(); ++i) {
				auto [mesh, material, transform] = context.entity(i);

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

				shader.uniform("uDepthMap", 0);
				OpenglAPI::Texture::bind(depthBuffer.textureID("depth"), GL_TEXTURE0);

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
			const Mat4& lightSpace) {

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

				shader.uniform("uDepthMap", 0);
				OpenglAPI::Texture::bind(depthBuffer.textureID("depth"), GL_TEXTURE0);

				OpenglAPI::Draw::instancedElements(mesh.indices().size(), pair.second.size());

				shader.unbind();
				mesh.renderArray().unbind();
			}
		}

	};

	class ShadowPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			Shader& depthShader{ data.shaders["depth_shader"] };
			Shader& instancedDepthShader{ data.shaders["instanced_depth"] };
			Framebuffer& depthBuffer{ data.frameBuffers["depthBuffer"] };

			auto [camera, cTransform] = context.camera();
			Mat4 projection{ camera->orthographic(-80.0f, 80.0f, -45.0f, 45.0f) };

			auto [_, dlTransform] = context.directionalLight();

			Mat4 lightSpace{ projection * dlTransform->view() };

			depthBuffer.bind();
			depthBuffer.clearContent();

			depthShader.bind();
			depthShader.uniform<Mat4>("uLightSpace", lightSpace);

			OpenglAPI::cullFront();

			renderEntities(context, depthShader);

			instancedDepthShader.bind();
			instancedDepthShader.uniform<Mat4>("uLightSpace", lightSpace);
			renderInstances(context);

			OpenglAPI::cullBack();

			depthBuffer.unbind();
		}

		void renderEntities(RenderContext& context, const Shader& shader) {
			for (size_t i{ 0 }; i < context.entityCount(); ++i) {
				auto [mesh, material, transform] = context.entity(i);

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

	class LightingPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			Shader& lightingShader{ data.shaders["lighting_shader"] };
			lightingShader.bind();

			Framebuffer& gBuffer{ data.frameBuffers["gBuffer"] };
			Framebuffer& colorBuffer{ data.frameBuffers["colorBuffer"] };

			colorBuffer.bind();
			colorBuffer.clearContent();

			lightingShader.uniform("uPosition", 0);
			lightingShader.uniform("uNormal", 1);
			lightingShader.uniform("uAlbedoSpec", 2);

			auto [_, cTransform] = context.camera();
			auto [directionalLight, dlTransform] = context.directionalLight();

			lightingShader.uniform<Vec3>("uViewPos", cTransform->position());

			lightingShader.uniform<Vec3>("uDirectionalLight.direction", dlTransform->front());
			lightingShader.uniform<Vec3>("uDirectionalLight.color", directionalLight->color);
			lightingShader.uniform<float>("uDirectionalLight.intensity", directionalLight->intensity);

			OpenglAPI::Texture::bind(gBuffer.textureID("position"), GL_TEXTURE0);
			OpenglAPI::Texture::bind(gBuffer.textureID("normal"), GL_TEXTURE1);
			OpenglAPI::Texture::bind(gBuffer.textureID("albedoSpecular"), GL_TEXTURE2);

			data.quad.renderArray().bind();

			OpenglAPI::Draw::quad();

			data.quad.renderArray().unbind();

			lightingShader.unbind();

			colorBuffer.unbind();
		}

	};

	class PointLightPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			if (!context.pointLightCount()) {
				return;
			}

			Framebuffer& gBuffer{ data.frameBuffers["gBuffer"] };
			Framebuffer& colorBuffer{ data.frameBuffers["colorBuffer"] };

			Shader& plShader{ data.shaders["point_light_shader"] };
			plShader.bind();
			colorBuffer.bind();

			OpenglAPI::enableBlend();

			OpenglAPI::setBlend(GL_ONE, GL_ONE);

			plShader.uniform<Vec2>(
				"uViewPortSize",
				Vec2{ static_cast<float>(data.width),static_cast<float>(data.height) });

			float aspectRatio{ static_cast<float>(data.width) / static_cast<float>(data.height) };
			auto [camera, cTransform] = context.camera();
			Mat4 projection{ camera->perspective(aspectRatio) };
			Mat4 view{ cTransform->view() };

			plShader.uniform<Mat4>("uProjection", projection);
			plShader.uniform<Mat4>("uView", view);

			plShader.uniform("uSPosition", 0);
			plShader.uniform("uSNormal", 1);
			plShader.uniform("uSAlbedoSpec", 2);

			OpenglAPI::Texture::bind(gBuffer.textureID("position"), GL_TEXTURE0);
			OpenglAPI::Texture::bind(gBuffer.textureID("normal"), GL_TEXTURE1);
			OpenglAPI::Texture::bind(gBuffer.textureID("albedoSpecular"), GL_TEXTURE2);

			data.sphere.renderArray().bind();

			OpenglAPI::disableDepth();

			for (size_t i{ 0 }; i < context.pointLightCount(); ++i) {

				auto [pointLight, _transform] = context.pointLight(i);

				Transform transform{ *_transform };

				float radius{ pointLight->radius() };
				transform.scale(Vec3{ radius,radius,radius });

				plShader.uniform<Vec3>("uPosition", transform.position());
				plShader.uniform<Vec3>("uScale", transform.scale());
				plShader.uniform<Quaternion>("uRotation", transform.rotation());

				plShader.uniform<Vec3>("uPointLight.position", transform.position());
				plShader.uniform<Vec3>("uPointLight.color", pointLight->color);
				plShader.uniform<float>("uPointLight.constant", pointLight->constant);
				plShader.uniform<float>("uPointLight.linear", pointLight->linear);
				plShader.uniform<float>("uPointLight.quadratic", pointLight->quadratic);

				OpenglAPI::Draw::elements(data.sphere.indices().size());

			}
			data.sphere.renderArray().unbind();
			plShader.unbind();

			OpenglAPI::enableDepth();
			OpenglAPI::disableBlend();
		}

	};

	class DrawPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			OpenglAPI::Framebuffer::unbind();
			OpenglAPI::Framebuffer::clear(0);

			Shader& quadShader{ data.shaders["quad_shader"] };
			Framebuffer& colorBuffer{ data.frameBuffers["colorBuffer"] };

			quadShader.bind();
			quadShader.uniform("uAlbedoSpecular", 0);
			OpenglAPI::Texture::bind(colorBuffer.textureID("albedoSpecular"), GL_TEXTURE0);

			data.quad.renderArray().bind();

			OpenglAPI::Draw::quad();

			data.quad.renderArray().unbind();
		}

	};

	class DebugPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			OpenglAPI::Framebuffer::unbind();
			OpenglAPI::Framebuffer::clear(0);

			Shader& quadShader{ data.shaders["quad_depth_shader"] };
			Framebuffer& colorBuffer{ data.frameBuffers["depthBuffer"] };

			quadShader.bind();
			quadShader.uniform("uAlbedoSpecular", 0);
			OpenglAPI::Texture::bind(colorBuffer.textureID("depth"), GL_TEXTURE0);

			data.quad.renderArray().bind();

			OpenglAPI::Draw::quad();

			data.quad.renderArray().unbind();
		}

	};

}