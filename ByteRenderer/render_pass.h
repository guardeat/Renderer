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

		using Variant = std::variant<std::string, uint32_t, int32_t, bool, float, Mat4>;
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

			float oldFov{ camera->fov() };
			camera->fov(45.0f);
			Mat4 projection{ camera->perspective(aspectRatio) };
			camera->fov(oldFov);

			auto [dl, dlTransform] = context.directionalLight();
			skyboxShader.bind();

			OpenGLAPI::disableDepth();

			skyboxShader.uniform<Mat4>("uProjection", projection);
			skyboxShader.uniform<Quaternion>("uRotation", cTransform->rotation());
			skyboxShader.uniform<Vec3>("uDirectionalLight.direction", dlTransform->front());
			skyboxShader.uniform<Vec3>("uDirectionalLight.color", dl->color);
			skyboxShader.uniform<float>("uDirectionalLight.intensity", dl->intensity);

			data.meshes.at("sphere").renderArray().bind();

			OpenGLAPI::Draw::elements(data.meshes.at("sphere").indices().size());

			data.meshes.at("sphere").renderArray().unbind();
			gBuffer.unbind();

			OpenGLAPI::enableDepth();
		}
	};

	class ShadowPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			if (!data.parameter<bool>("render_shadow")) {
				return;
			}

			size_t drawFrame{ data.parameter<uint32_t>("shadow_draw_frame") };
			size_t current{ data.parameter<uint32_t>("current_shadow_draw_frame")++ % drawFrame };
			if (current != 0) {
				return;
			}

			Shader& depthShader{ data.shaders["depth"] };
			Shader& instancedDepthShader{ data.shaders["instanced_depth"] };

			float aspectRatio{ static_cast<float>(data.width) / static_cast<float>(data.height) };

			size_t cascadeCount{ data.parameter<uint32_t>("cascade_count") };

			Buffer<Framebuffer*> depthBuffers;
			for (size_t i = 0; i < cascadeCount; ++i) {
				depthBuffers.push_back(&data.frameBuffers.at("depthBuffer" + std::to_string(i + 1)));
			}

			updateLightMatrices(aspectRatio, data, context);

			OpenGLAPI::enableCulling();
			OpenGLAPI::cullFront();

			for (size_t i{}; i < depthBuffers.size(); ++i) {
				Mat4 lightSpace{ data.parameter<Mat4>("cascade_light_" + std::to_string(i + 1)) };

				depthBuffers[i]->bind();
				depthBuffers[i]->clearContent();

				depthShader.bind();
				depthShader.uniform<Mat4>("uLightSpace", lightSpace);

				renderEntities(context, depthShader);

				instancedDepthShader.bind();
				instancedDepthShader.uniform<Mat4>("uLightSpace", lightSpace);
				renderInstances(context);

				depthBuffers[i]->unbind();
			}

			OpenGLAPI::cullBack();
			OpenGLAPI::disableCulling();
		}

	private:
		void renderEntities(RenderContext& context, const Shader& shader) const {
			for (auto& pair: context.renderEntities()) {
				auto [mesh, material, transform] = pair.second;

				mesh->renderArray().bind();

				shader.uniform<Vec3>("uPosition", transform->position());
				shader.uniform<Vec3>("uScale", transform->scale());
				shader.uniform<Quaternion>("uRotation", transform->rotation());

				OpenGLAPI::Draw::elements(mesh->indices().size());

				mesh->renderArray().unbind();
			}
		}

		void renderInstances(RenderContext& context) const {
			for (auto& pair : context.instances()) {
				Mesh& mesh{ pair.second.mesh() };
				Material& material{ pair.second.material() };

				mesh.renderArray().bind();

				OpenGLAPI::Draw::instancedElements(mesh.indices().size(), pair.second.size());

				mesh.renderArray().unbind();
			}
		}

		void updateLightMatrices(float aspectRatio, RenderData& data, RenderContext& context) {
			size_t cascadeCount{ data.parameter<uint32_t>("cascade_count") };

			auto [_, dlTransform] = context.directionalLight();

			auto [camera, cTransform] = context.camera();
			Mat4 view{ cTransform->view() };

			float far{ camera->farPlane() };
			float near{ camera->nearPlane() };

			for (size_t i{}; i < cascadeCount; ++i) {
				float divisor{ data.parameter<float>("cascade_divisor_" + std::to_string(i + 1)) };
				Mat4 projection{ camera->perspective(aspectRatio,near,far / divisor) };
				Mat4 lightSpace{ frustumSpace(projection, view, *dlTransform, far) };
				data.parameter<Mat4>("cascade_light_" + std::to_string(i + 1)) = lightSpace;
			}
		}

		Mat4 frustumSpace(
			const Mat4& projection, 
			const Mat4& view, 
			const Transform& lightTransform, 
			float far) const {
			const auto inv{ (projection * view).inverse() };

			Buffer<Vec4> corners;
			for (unsigned int x = 0; x < 2; ++x) {
				for (unsigned int y = 0; y < 2; ++y) {
					for (unsigned int z = 0; z < 2; ++z) {
						const Vec4 pt{
							inv * Vec4(
								2.0f * x - 1.0f,
								2.0f * y - 1.0f,
								2.0f * z - 1.0f,
								1.0f) };
						corners.push_back(pt / pt.w);
					}
				}
			}

			Vec3 center{};
			for (const auto& v : corners) {
				center += Vec3(v.x, v.y, v.z);
			}
			center /= corners.size();

			const auto lightView{ Mat4::lookAt(
				center - lightTransform.front(),
				center,
				lightTransform.up()
			) };

			float minX{ std::numeric_limits<float>::max() };
			float maxX{ std::numeric_limits<float>::lowest() };
			float minY{ std::numeric_limits<float>::max() };
			float maxY{ std::numeric_limits<float>::lowest() };
			float minZ{ std::numeric_limits<float>::max() };
			float maxZ{ std::numeric_limits<float>::lowest() };

			for (const auto& v : corners) {
				const auto trf{ lightView * v };
				minX = std::min(minX, trf.x);
				maxX = std::max(maxX, trf.x);
				minY = std::min(minY, trf.y);
				maxY = std::max(maxY, trf.y);
				minZ = std::min(minZ, trf.z);
				maxZ = std::max(maxZ, trf.z);
			}

			minZ = std::min(minZ, -far);
			maxZ = std::max(maxZ, far);

			Mat4 lightProjection{ Mat4::orthographic(minX, maxX, minY, maxY, minZ, maxZ) };

			return lightProjection * lightView;
		}

	};

	class GeometryPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			float aspectRatio{ static_cast<float>(data.width) / static_cast<float>(data.height) };
			auto [camera, cTransform] = context.camera();
			Mat4 projection{ camera->perspective(aspectRatio) };
			Mat4 view{ cTransform->view() };

			Framebuffer& gBuffer{ data.frameBuffers["gBuffer"] };
			gBuffer.bind();

			if (data.parameter<bool>("clear_gbuffer")) {
				gBuffer.clearContent();
			}

			Shader& shader{ data.shaders["deferred"] };
			shader.bind();

			shader.uniform<Mat4>("uProjection", projection);
			shader.uniform<Mat4>("uView", view);

			renderEntities(context, shader);

			Shader& instancedShader{ data.shaders["instanced_deferred"] };
			instancedShader.bind();

			instancedShader.uniform<Mat4>("uProjection", projection);
			instancedShader.uniform<Mat4>("uView", view);
			renderInstances(context, instancedShader);

			gBuffer.unbind();

			data.parameter<bool>("clear_gbuffer") = true;
		}

	private:
		void renderEntities(RenderContext& context, Shader& shader) const {

			for (auto& pair : context.renderEntities()) {
				auto [mesh, material, transform] = pair.second;

				mesh->renderArray().bind();

				shader.uniform<Vec3>("uAlbedo", material->albedo());
				shader.uniform<float>("uMetallic", material->metallic());
				shader.uniform<float>("uRoughness", material->roughness());
				shader.uniform<float>("uAO", material->ambientOcclusion());
				shader.uniform<float>("uEmission", material->emission());

				shader.uniform<Vec3>("uPosition", transform->position());
				shader.uniform<Vec3>("uScale", transform->scale());
				shader.uniform<Quaternion>("uRotation", transform->rotation());

				OpenGLAPI::Draw::elements(mesh->indices().size());

				mesh->renderArray().unbind();
			}
		}

		void renderInstances(RenderContext& context, Shader& shader) const {
			for (auto& pair : context.instances()) {
				Mesh& mesh{ pair.second.mesh() };
				Material& material{ pair.second.material() };

				mesh.renderArray().bind();

				shader.uniform<Vec3>("uAlbedo", material.albedo());
				shader.uniform<float>("uMetallic", material.metallic());
				shader.uniform<float>("uRoughness", material.roughness());
				shader.uniform<float>("uAO", material.ambientOcclusion());
				shader.uniform<float>("uEmission", material.emission());

				OpenGLAPI::Draw::instancedElements(mesh.indices().size(), pair.second.size());

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

			float aspectRatio{ static_cast<float>(data.width)/ static_cast<float>(data.height) };

			auto [camera, cTransform] = context.camera();
			auto [directionalLight, dlTransform] = context.directionalLight();

			Mat4 projection{ camera->perspective(aspectRatio) };
			Mat4 view{ cTransform->view() };
			Mat4 inverseView{ view.inverse() };
			Mat4 inverseProjection{ projection.inverse() };

			OpenGLAPI::Texture::bind(gBuffer.textureID("normal"), TextureUnit::T0);
			OpenGLAPI::Texture::bind(gBuffer.textureID("albedo"), TextureUnit::T1);
			OpenGLAPI::Texture::bind(gBuffer.textureID("material"), TextureUnit::T2);
			OpenGLAPI::Texture::bind(gBuffer.textureID("depth"), TextureUnit::T3);

			setupGBufferTextures(lightingShader);
			setupCascades(data, lightingShader, *camera);
			bindCommon(lightingShader, view, inverseView, inverseProjection);

			lightingShader.uniform<Vec3>("uViewPos", cTransform->position());
			lightingShader.uniform<Vec3>("uDirectionalLight.direction", dlTransform->front());
			lightingShader.uniform<Vec3>("uDirectionalLight.color", directionalLight->color);
			lightingShader.uniform<float>("uDirectionalLight.intensity", directionalLight->intensity);

			data.meshes.at("quad").renderArray().bind();
			OpenGLAPI::Draw::quad();
			data.meshes.at("quad").renderArray().unbind();

			lightingShader.unbind();

			if (!context.pointLights().empty()) {
				renderPointLights(context,data,view,inverseView,inverseProjection,projection);
			}

			colorBuffer.unbind();
		}
	private:
		void renderPointLights(
			RenderContext& context, 
			RenderData& data, 
			Mat4& view, 
			Mat4& inverseView, 
			Mat4& inverseProjection, 
			Mat4& projection) {

			Shader& plShader{ data.shaders["point_light"] };
			plShader.bind();

			OpenGLAPI::enableBlend();
			OpenGLAPI::setBlend(1, 1);
			OpenGLAPI::enableCulling();
			OpenGLAPI::cullFront();

			plShader.uniform<Vec2>(
				"uViewPortSize",
				Vec2{ static_cast<float>(data.width), static_cast<float>(data.height) });

			bindCommon(plShader, view, inverseView, inverseProjection);
			plShader.uniform<Mat4>("uProjection", projection);

			setupGBufferTextures(plShader);

			data.meshes.at("sphere").renderArray().bind();

			OpenGLAPI::disableDepth();

			for (auto& pair : context.pointLights()) {
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

				OpenGLAPI::Draw::elements(data.meshes.at("sphere").indices().size());
			}

			data.meshes.at("sphere").renderArray().unbind();
			plShader.unbind();

			OpenGLAPI::enableDepth();
			OpenGLAPI::disableBlend();
			OpenGLAPI::cullBack();
			OpenGLAPI::disableCulling();
		}


		void setupGBufferTextures(Shader& shader) {
			shader.uniform("uNormal", 0);
			shader.uniform("uAlbedo", 1);
			shader.uniform("uMaterial", 2);
			shader.uniform("uDepth", 3);
		}

		void setupCascades(RenderData& data, Shader& shader, Camera& camera) {
			size_t cascadeCount{ data.parameter<uint32_t>("cascade_count") };

			float far{ camera.farPlane() };
			Buffer<float> farPlanes;
			Buffer<Mat4> lightSpaces{};
			Buffer<TextureID> dbTextures;

			for (size_t i{ 0 }; i < cascadeCount; ++i) {
				float divisor{ data.parameter<float>("cascade_divisor_" + std::to_string(i + 1)) };
				farPlanes.push_back(far / divisor);
				lightSpaces.push_back(data.parameter<Mat4>("cascade_light_" + std::to_string(i + 1)));
				dbTextures.push_back(data.frameBuffers.at("depthBuffer" + std::to_string(i + 1)).textureID("depth"));
			}

			for (size_t i{}; i < dbTextures.size(); ++i) {
				shader.uniform("uDepthMaps[" + std::to_string(i) + "]", static_cast<int>(i + 4));
				TextureUnit unit{ static_cast<TextureUnit>(static_cast<uint32_t>(TextureUnit::T4) + i) };
				OpenGLAPI::Texture::bind(dbTextures[i], unit);
			}

			shader.uniform<Mat4>("uLightSpaces", lightSpaces);
			shader.uniform<float>("uCascadeFars", farPlanes);
			shader.uniform<int>("uCascadeCount", static_cast<int>(cascadeCount));
		}

		void bindCommon(Shader& shader, Mat4& view, Mat4& iView, Mat4& iProjection) {
			shader.uniform<Mat4>("uView", view);
			shader.uniform<Mat4>("uInverseView", iView);
			shader.uniform<Mat4>("uInverseProjection", iProjection);
		}

	};

	class DrawPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			OpenGLAPI::Framebuffer::clear(0);

			Shader& quadShader{ data.shaders["quad"] };
			Framebuffer& colorBuffer{ data.frameBuffers["colorBuffer"] };

			quadShader.bind();
			quadShader.uniform("uAlbedo", 0);
			quadShader.uniform("uGamma", data.parameter<float>("gamma"));
			OpenGLAPI::Texture::bind(colorBuffer.textureID("albedo"), TextureUnit::T0);

			data.meshes.at("quad").renderArray().bind();

			OpenGLAPI::Draw::quad();

			data.meshes.at("quad").renderArray().unbind();
		}

	};

}