#pragma once

#include "context.h"
#include "quaternion.h"
#include "vec.h"
#include "trigonometry.h"
#include "mat.h"
#include "render_api.h"
#include "render_data.h"

namespace Byte {
	class RenderPass {
	public:
		virtual ~RenderPass() = default;

		virtual void render(RenderContext& context, RenderData& data) = 0;
	};

	class FrustumCullingPass : public RenderPass {
	private:
		struct Plane {
			Vec3 normal;
			float distance{ 0 };
		};

		struct Frustum {
			Plane planes[6];
		};

	public:
		void render(RenderContext& context, RenderData& data) override {
			float aspectRatio{ static_cast<float>(data.width) / static_cast<float>(data.height) };
			auto [camera, cameraTransform] = context.camera();

			Frustum frustum{ createFrustum(*camera, *cameraTransform, aspectRatio) };

			for (auto& pair : context.renderEntities()) {
				auto [mesh, material, transform, mode] = pair.second;

				if (!inside(frustum, *transform, mesh->data().boundingRadius)) {
					pair.second.mode = RenderMode::DISABLED;
				}
				else {
					pair.second.mode = RenderMode::ENABLED;
				}
			}
		}

	private:
	private:
		Frustum createFrustum(const Camera& camera, const Transform& transform, float aspectRatio) {
			Frustum frustum{};

			float fov{ camera.fov() };
			float nearDist{ camera.nearPlane() };
			float farDist{ camera.farPlane() };

			float tanFov{ tan(radians(fov) * 0.5f) };

			float nearHeight{ 2.0f * tanFov * nearDist };
			float nearWidth{ nearHeight * aspectRatio };
			float farHeight{ 2.0f * tanFov * farDist };
			float farWidth{ farHeight * aspectRatio };

			Vec3 position{ transform.position() };
			Vec3 front{ transform.front().normalized() };
			Vec3 right{ transform.right().normalized() };
			Vec3 up{ transform.up().normalized() };

			Vec3 nearCenter{ position + front * nearDist };
			Vec3 farCenter{ position + front * farDist };

			Vec3 nearTopLeft{ nearCenter + (up * (nearHeight * 0.5f)) - (right * (nearWidth * 0.5f)) };
			Vec3 nearTopRight{ nearCenter + (up * (nearHeight * 0.5f)) + (right * (nearWidth * 0.5f)) };
			Vec3 nearBottomLeft{ nearCenter - (up * (nearHeight * 0.5f)) - (right * (nearWidth * 0.5f)) };
			Vec3 nearBottomRight{ nearCenter - (up * (nearHeight * 0.5f)) + (right * (nearWidth * 0.5f)) };

			Vec3 farTopLeft{ farCenter + (up * (farHeight * 0.5f)) - (right * (farWidth * 0.5f)) };
			Vec3 farTopRight{ farCenter + (up * (farHeight * 0.5f)) + (right * (farWidth * 0.5f)) };
			Vec3 farBottomLeft{ farCenter - (up * (farHeight * 0.5f)) - (right * (farWidth * 0.5f)) };
			Vec3 farBottomRight{ farCenter - (up * (farHeight * 0.5f)) + (right * (farWidth * 0.5f)) };

			auto computePlane = [](const Vec3& p1, const Vec3& p2, const Vec3& p3) -> Plane {
				Vec3 normal{ -(p2 - p1).cross(p3 - p1).normalized() };
				float distance{ -normal.dot(p1) };
				return Plane{ normal, distance };
				};

			frustum.planes[0] = computePlane(nearTopRight, nearTopLeft, nearBottomLeft);
			frustum.planes[1] = computePlane(farTopLeft, farTopRight, farBottomRight);
			frustum.planes[2] = computePlane(nearTopLeft, farTopLeft, farBottomLeft);
			frustum.planes[3] = computePlane(farTopRight, nearTopRight, nearBottomRight);
			frustum.planes[4] = computePlane(nearTopLeft, nearTopRight, farTopRight);
			frustum.planes[5] = computePlane(nearBottomRight, nearBottomLeft, farBottomLeft);

			return frustum;
		}

		bool inside(const Frustum& frustum, const Transform& transform, float radius) {
			Vec3 position{ transform.position() };

			Vec3 scale{ transform.scale() };
			float maxScale{ std::max(std::max(scale.x, scale.y), scale.z) };
			float scaledRadius{ radius * maxScale };

			for (const auto& plane : frustum.planes) {
				float distance{ plane.normal.dot(position) + plane.distance };
				if (distance < -scaledRadius) {
					return false;
				}
			}

			return true;
		}

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

			RenderAPI::disableDepth();

			skyboxShader.uniform<Mat4>("uProjection", projection);
			skyboxShader.uniform<Quaternion>("uRotation", cTransform->rotation());
			skyboxShader.uniform<Vec3>("uDirectionalLight.direction", dlTransform->front());
			skyboxShader.uniform<Vec3>("uDirectionalLight.color", dl->color);
			skyboxShader.uniform<float>("uDirectionalLight.intensity", dl->intensity);

			data.meshes.at("sphere").renderArray().bind();

			RenderAPI::Draw::elements(data.meshes.at("sphere").indices().size());

			data.meshes.at("sphere").renderArray().unbind();
			gBuffer.unbind();

			RenderAPI::enableDepth();
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

			RenderAPI::enableCulling();
			RenderAPI::cullFront();

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

			RenderAPI::cullBack();
			RenderAPI::disableCulling();
		}

	private:
		void renderEntities(RenderContext& context, const Shader& shader) const {
			for (auto& pair : context.renderEntities()) {
				auto [mesh, material, transform, mode] = pair.second;

				if (material->shadow() == ShadowMode::ENABLED) {
					mesh->renderArray().bind();

					shader.uniform<Vec3>("uPosition", transform->position());
					shader.uniform<Vec3>("uScale", transform->scale());
					shader.uniform<Quaternion>("uRotation", transform->rotation());

					RenderAPI::Draw::elements(mesh->indices().size());

					mesh->renderArray().unbind();
				}
			}
		}

		void renderInstances(RenderContext& context) const {
			for (auto& pair : context.instances()) {
				Mesh& mesh{ pair.second.mesh() };
				Material& material{ pair.second.material() };

				if (material.shadow() == ShadowMode::ENABLED) {
					mesh.renderArray().bind();

					RenderAPI::Draw::instancedElements(mesh.indices().size(), pair.second.size());

					mesh.renderArray().unbind();
				}
			}
		}

		void updateLightMatrices(float aspectRatio, RenderData& data, RenderContext& context) {
			size_t cascadeCount{ data.parameter<uint32_t>("cascade_count") };

			auto [dl, dlTransform] = context.directionalLight();

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

			const auto lightView{ Mat4::view(
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
	protected:
		void bindMaterial(Shader& shader, const Material& material) const {

			if (!material.albedoTextureID() && !material.materialTextureID()) {
				shader.uniform<int>("uDataMode", 0);
				shader.uniform<float>("uMetallic", material.metallic());
				shader.uniform<float>("uRoughness", material.roughness());
				shader.uniform<float>("uAO", material.ambientOcclusion());
				shader.uniform<float>("uEmission", material.emission());
			}

			else if (material.albedoTextureID() && !material.materialTextureID()) {
				shader.uniform<int>("uDataMode", 1);
				shader.uniform<int>("uAlbedoTexture", 0);

				material.albedoTexture().bind();
				shader.uniform<float>("uMetallic", material.metallic());
				shader.uniform<float>("uRoughness", material.roughness());
				shader.uniform<float>("uAO", material.ambientOcclusion());
				shader.uniform<float>("uEmission", material.emission());
			}


			else if (!material.albedoTextureID() && material.materialTextureID()) {
				shader.uniform<int>("uDataMode", 2);
				shader.uniform<int>("uMaterialTexture", 0);

				material.materialTexture().bind();
			}

			else {
				shader.uniform<int>("uDataMode", 3);
				shader.uniform<int>("uAlbedoTexture", 0);
				shader.uniform<int>("uMaterialTexture", 1);

				material.albedoTexture().bind();
				material.materialTexture().bind(TextureUnit::T1);
			}

			shader.uniform<Vec4>("uAlbedo", material.albedo());
		}

		void renderEntities(
			RenderContext& context,
			RenderData& data,
			const Mat4& projection,
			const Mat4& view,
			const ShaderTag& defaultTag,
			TransparencyMode mode) const {

			for (auto& pair : context.renderEntities()) {
				auto [mesh, material, transform, renderMode] = pair.second;

				if (material->transparency() != mode || renderMode == RenderMode::DISABLED) {
					continue;
				}

				Shader* shader;

				auto result{ material->shaderMap().find("geometry") };
				if (result != material->shaderMap().end()) {
					shader = &data.shaders.at(result->second);
				}
				else {
					shader = &data.shaders.at(defaultTag);
				}

				shader->bind();

				shader->uniform(context.shaderInputMap());

				shader->uniform<Mat4>("uProjection", projection);
				shader->uniform<Mat4>("uView", view);

				mesh->renderArray().bind();
				bindMaterial(*shader, *material);

				shader->uniform<Vec3>("uPosition", transform->position());
				shader->uniform<Vec3>("uScale", transform->scale());
				shader->uniform<Quaternion>("uRotation", transform->rotation());

				RenderAPI::Draw::elements(mesh->indices().size());

				mesh->renderArray().unbind();
			}
		}

		void renderInstances(
			RenderContext& context,
			RenderData& data,
			const Mat4& projection,
			const Mat4& view,
			const ShaderTag& defaultTag,
			TransparencyMode mode) const {

			for (auto& pair : context.instances()) {
				Mesh& mesh{ pair.second.mesh() };
				Material& material{ pair.second.material() };

				if (material.transparency() != mode || pair.second.renderMode() == RenderMode::DISABLED) {
					continue;
				}

				Shader* shader;

				auto result{ material.shaderMap().find("geometry") };
				if (result != material.shaderMap().end()) {
					shader = &data.shaders.at(result->second);
				}
				else {
					shader = &data.shaders.at(defaultTag);
				}

				shader->bind();
				shader->uniform(context.shaderInputMap());

				shader->uniform<Mat4>("uProjection", projection);
				shader->uniform<Mat4>("uView", view);

				mesh.renderArray().bind();
				bindMaterial(*shader, material);

				RenderAPI::Draw::instancedElements(mesh.indices().size(), pair.second.size());

				mesh.renderArray().unbind();
			}
		}

	};

	class OpaquePass : public GeometryPass {
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

			renderEntities(context, data, projection, view, "deferred", TransparencyMode::BINARY);

			renderInstances(context, data, projection, view, "instanced_deferred", TransparencyMode::BINARY);

			gBuffer.unbind();

			data.parameter<bool>("clear_gbuffer") = true;
		}
	};

	class SSAOPass : public RenderPass {
	private:
		std::vector<Vec3> _kernel;
		std::vector<Vec3> _noise;

		Texture _noiseTexture{
			TextureData{
				AttachmentType::COLOR_0, ColorFormat::RGB16F, ColorFormat::RGB,
				DataType::FLOAT, 4U, 4U,
				TextureWrap::REPEAT, TextureWrap::REPEAT,
				} };

	public:
		SSAOPass() {
			std::uniform_real_distribution<GLfloat> urd(0.0, 1.0);
			std::default_random_engine generator;
			for (size_t i{ 0 }; i < 64; ++i)
			{
				Vec3 sample{ urd(generator) * 2.0f - 1.0f, urd(generator) * 2.0f - 1.0f, urd(generator) };
				sample.normalize();
				sample *= urd(generator);
				float scale{ static_cast<float>(i) / 64.0f };

				scale = 0.1f + scale * scale * 0.9f;
				sample *= scale;
				_kernel.push_back(sample);
			}

			for (size_t i{ 0 }; i < 16; i++)
			{
				Vec3 sample(urd(generator) * 2.0f - 1.0f, urd(generator) * 2.0f - 1.0f, 0.0f);

				const uint8_t* bytes{ reinterpret_cast<const uint8_t*>(&sample) };
				_noiseTexture.data().data.insert(_noiseTexture.data().data.end(), bytes, bytes + sizeof(Vec3));
			}
		}

		void render(RenderContext& context, RenderData& data) override {
			if (!_noiseTexture.data().id) {
				RenderAPI::Texture::build(_noiseTexture.data());
			}

			if (!data.parameter<bool>("render_ssao")) {
				return;
			}

			Shader& ssaoShader{ data.shaders.at("ssao") };

			Framebuffer& gBuffer{ data.frameBuffers.at("gBuffer") };

			Framebuffer& ssaoBuffer{ data.frameBuffers.at("ssaoBuffer") };
			ssaoBuffer.bind();
			ssaoBuffer.clearContent();

			ssaoShader.bind();

			float aspectRatio{ static_cast<float>(data.width) / static_cast<float>(data.height) };

			auto [camera, cTransform] = context.camera();
			auto [directionalLight, dlTransform] = context.directionalLight();

			Mat4 projection{ camera->perspective(aspectRatio) };
			Mat4 view{ cTransform->view() };
			Mat4 inverseView{ view.inverse() };
			Mat4 inverseProjection{ projection.inverse() };

			Vec2 screenSize{ static_cast<float>(data.width), static_cast<float>(data.height) };

			ssaoShader.uniform<Mat4>("uProjection", projection);
			ssaoShader.uniform<Mat4>("uInverseView", inverseView);
			ssaoShader.uniform<Mat4>("uInverseProjection", inverseProjection);
			ssaoShader.uniform<Vec2>("uScreenSize", screenSize);
			ssaoShader.uniform<Vec3>("uSamples", _kernel);

			RenderAPI::Texture::bind(gBuffer.textureID("normal"), TextureUnit::T0);
			RenderAPI::Texture::bind(_noiseTexture.id(), TextureUnit::T1);
			RenderAPI::Texture::bind(gBuffer.textureID("depth"), TextureUnit::T2);

			ssaoShader.uniform("uNormal", 0);
			ssaoShader.uniform("uNoise", 1);
			ssaoShader.uniform("uDepth", 2);

			data.meshes.at("quad").renderArray().bind();

			RenderAPI::Draw::quad();

			data.meshes.at("quad").renderArray().unbind();

			Framebuffer& blurBuffer{ data.frameBuffers.at("blurBuffer") };
			blurBuffer.bind();
			blurBuffer.clearContent();
			Shader& blurShader{ data.shaders.at("blur") };
			blurShader.bind();

			RenderAPI::Texture::bind(ssaoBuffer.textureID("color"), TextureUnit::T0);
			RenderAPI::Texture::bind(gBuffer.textureID("depth"), TextureUnit::T1);

			blurShader.uniform("uSrcTexture", 0);
			blurShader.uniform("uDepth", 1);

			data.meshes.at("quad").renderArray().bind();

			RenderAPI::Draw::quad();

			data.meshes.at("quad").renderArray().unbind();
		}
	};

	class LightingPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			Shader& lightingShader{ data.shaders.at("lighting") };

			Framebuffer& gBuffer{ data.frameBuffers.at("gBuffer") };
			Framebuffer& colorBuffer{ data.frameBuffers.at("colorBuffer") };

			colorBuffer.bind();
			colorBuffer.clearContent();

			lightingShader.bind();

			float aspectRatio{ static_cast<float>(data.width) / static_cast<float>(data.height) };

			auto [camera, cTransform] = context.camera();
			auto [directionalLight, dlTransform] = context.directionalLight();

			Mat4 projection{ camera->perspective(aspectRatio) };
			Mat4 view{ cTransform->view() };
			Mat4 inverseView{ view.inverse() };
			Mat4 inverseProjection{ projection.inverse() };
			Vec3 viewPos{ cTransform->position() };

			RenderAPI::Texture::bind(gBuffer.textureID("normal"), TextureUnit::T0);
			RenderAPI::Texture::bind(gBuffer.textureID("albedo"), TextureUnit::T1);
			RenderAPI::Texture::bind(gBuffer.textureID("material"), TextureUnit::T2);
			RenderAPI::Texture::bind(gBuffer.textureID("depth"), TextureUnit::T3);

			if (data.parameter<bool>("render_ssao")) {
				size_t cascadeCount{ data.parameter<uint32_t>("cascade_count") };
				TextureUnit ssaoUnit{ static_cast<TextureUnit>(
					static_cast<uint32_t>(TextureUnit::T4) + cascadeCount
				) };

				Framebuffer& blurBuffer{ data.frameBuffers.at("blurBuffer") };
				RenderAPI::Texture::bind(blurBuffer.textureID("color"), ssaoUnit);
				lightingShader.uniform("uSSAO", static_cast<int>(ssaoUnit));
			}

			setupGBufferTextures(lightingShader);
			setupCascades(data, lightingShader, *camera);
			bindCommon(lightingShader, view, inverseView, inverseProjection, viewPos);

			lightingShader.uniform<Vec3>("uDirectionalLight.direction", dlTransform->front());
			lightingShader.uniform<Vec3>("uDirectionalLight.color", directionalLight->color);
			lightingShader.uniform<float>("uDirectionalLight.intensity", directionalLight->intensity);

			lightingShader.uniform<bool>("uUseSSAO", data.parameter<bool>("render_ssao"));

			data.meshes.at("quad").renderArray().bind();
			RenderAPI::Draw::quad();
			data.meshes.at("quad").renderArray().unbind();

			lightingShader.unbind();

			if (!context.pointLights().empty()) {
				Shader& plShader{ data.shaders.at("point_light") };
				plShader.bind();

				RenderAPI::enableBlend();
				RenderAPI::setBlendAdditive();
				RenderAPI::enableCulling();
				RenderAPI::cullFront();
				RenderAPI::disableDepth();

				plShader.uniform<Vec2>(
					"uViewPortSize",
					Vec2{ static_cast<float>(data.width), static_cast<float>(data.height) });

				bindCommon(plShader, view, inverseView, inverseProjection, viewPos);
				plShader.uniform<Mat4>("uProjection", projection);

				setupGBufferTextures(plShader);

				float halfFar{ camera->farPlane() / 2 };

				data.meshes.at("low_poly_sphere").renderArray().bind();

				for (auto& pair : context.pointLights()) {
					auto [pointLight, _transform] = pair.second;

					Transform transform{ *_transform };

					float radius{ std::min(pointLight->radius(),halfFar) };
					transform.scale(Vec3{ radius, radius, radius });

					plShader.uniform<Vec3>("uPosition", transform.position());
					plShader.uniform<Vec3>("uScale", transform.scale());
					plShader.uniform<Quaternion>("uRotation", transform.rotation());

					plShader.uniform<Vec3>("uPointLight.position", transform.position());
					plShader.uniform<Vec3>("uPointLight.color", pointLight->color);
					plShader.uniform<float>("uPointLight.constant", pointLight->constant);
					plShader.uniform<float>("uPointLight.linear", pointLight->linear);
					plShader.uniform<float>("uPointLight.quadratic", pointLight->quadratic);

					RenderAPI::Draw::elements(data.meshes.at("low_poly_sphere").indices().size());
				}

				data.meshes.at("low_poly_sphere").renderArray().unbind();
				plShader.unbind();

				RenderAPI::enableDepth();
				RenderAPI::disableBlend();
				RenderAPI::cullBack();
				RenderAPI::disableCulling();
			}

			colorBuffer.unbind();
		}
	private:
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
				RenderAPI::Texture::bind(dbTextures[i], unit);
			}

			shader.uniform<Mat4>("uLightSpaces", lightSpaces);
			shader.uniform<float>("uCascadeFars", farPlanes);
			shader.uniform<int>("uCascadeCount", static_cast<int>(cascadeCount));
		}

		void bindCommon(Shader& shader, Mat4& view, Mat4& iView, Mat4& iProjection, Vec3& viewPos) {
			shader.uniform<Mat4>("uView", view);
			shader.uniform<Mat4>("uInverseView", iView);
			shader.uniform<Mat4>("uInverseProjection", iProjection);
			shader.uniform<Vec3>("uViewPos", viewPos);
		}

	};

	class TransparentPass : public GeometryPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			float aspectRatio{ static_cast<float>(data.width) / static_cast<float>(data.height) };
			auto [camera, cTransform] = context.camera();
			Mat4 projection{ camera->perspective(aspectRatio) };
			Mat4 view{ cTransform->view() };

			Framebuffer& gBuffer{ data.frameBuffers.at("gBuffer") };
			Framebuffer& colorBuffer{ data.frameBuffers.at("colorBuffer") };

			RenderAPI::Framebuffer::blitDepth(gBuffer.data(), colorBuffer.data());

			colorBuffer.bind();

			RenderAPI::disableDepthMask();
			RenderAPI::enableBlend();
			RenderAPI::setBlendTransparency();

			renderEntities(context, data, projection, view, "transparency", TransparencyMode::UNSORTED);

			renderInstances(context, data, projection, view, "instanced_transparency", TransparencyMode::UNSORTED);

			RenderAPI::enableDepthMask();
			RenderAPI::disableBlend();

			colorBuffer.unbind();

			data.parameter<bool>("clear_gbuffer") = true;
		}
	};

	class BloomPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			if (!data.parameter<bool>("render_bloom")) {
				return;
			}

			size_t mipCount{ data.parameter<uint32_t>("bloom_mip_count") };

			Shader& downsampleShader{ data.shaders.at("bloom_downsample") };
			downsampleShader.bind();
			downsampleShader.uniform<float>("uInvGamma", 1.0f / data.parameter<float>("gamma"));
			downsampleShader.uniform<bool>(" uKarisAvarage", true);

			Framebuffer* src{ &data.frameBuffers.at("colorBuffer") };

			for (size_t i{ 1 }; i <= mipCount; ++i) {
				Framebuffer* dest{ &data.frameBuffers.at("bloomBuffer" + std::to_string(i)) };

				float srcWidth{ static_cast<float>(src->width()) };
				float srcHeight{ static_cast<float>(src->height()) };

				dest->bind();
				dest->clearContent();

				downsampleShader.uniform("uSrcTexture", 0);
				RenderAPI::Texture::bind(src->textureID("color"));

				downsampleShader.uniform<Vec2>("uSrcResolution", Vec2{ srcWidth,srcHeight });

				data.meshes.at("quad").renderArray().bind();

				RenderAPI::Draw::quad();

				data.meshes.at("quad").renderArray().unbind();

				src = dest;

				downsampleShader.uniform<bool>(" uKarisAvarage", false);
			}

			RenderAPI::enableBlend();
			RenderAPI::disableDepth();

			Shader& upsampleShader{ data.shaders.at("bloom_upsample") };

			upsampleShader.bind();
			upsampleShader.uniform<float>("uFilterRadius", 0.01f);
			upsampleShader.uniform("uSrcTexture", 0);
			RenderAPI::Texture::bind(src->textureID("color"));

			src = &data.frameBuffers["bloomBuffer" + std::to_string(mipCount)];
			for (size_t i{ mipCount }; i > 1; --i) {
				Framebuffer* dest{ &data.frameBuffers.at("bloomBuffer" + std::to_string(i - 1)) };

				dest->bind();

				data.meshes.at("quad").renderArray().bind();

				RenderAPI::Draw::quad();

				data.meshes.at("quad").renderArray().unbind();

				src = dest;
			}

			float strength{ data.parameter<float>("bloom_strength") };

			RenderAPI::setBlend(strength, 1.0f - strength);

			data.frameBuffers.at("colorBuffer").bind();

			data.meshes.at("quad").renderArray().bind();
			RenderAPI::Draw::quad();
			data.meshes.at("quad").renderArray().unbind();

			RenderAPI::disableBlend();
			RenderAPI::enableDepth();
		}
	};

	class DrawPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			RenderAPI::viewPort(data.width, data.height);
			RenderAPI::Framebuffer::clear(0);

			Shader* shader;

			if (data.parameter<bool>("render_fxaa")) {
				shader = &data.shaders.at("fxaa");
				shader->bind();
				Vec2 screenSize{ static_cast<float>(data.width), static_cast<float>(data.height) };
				shader->uniform("uScreenSize", screenSize);
			}
			else {
				shader = &data.shaders.at("quad");
				shader->bind();
			}

			Framebuffer& colorBuffer{ data.frameBuffers.at("colorBuffer") };

			shader->uniform("uGamma", data.parameter<float>("gamma"));
			shader->uniform("uAlbedo", 0);
			RenderAPI::Texture::bind(colorBuffer.textureID("color"), TextureUnit::T0);

			data.meshes.at("quad").renderArray().bind();

			RenderAPI::Draw::quad();

			data.meshes.at("quad").renderArray().unbind();
		}

	};

}