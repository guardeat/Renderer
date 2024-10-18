#pragma once

#include "mesh.h"
#include "material.h"
#include "transform.h"
#include "mat.h"
#include "typedefs.h"
#include "opengl_api.h"
#include "texture.h"
#include "g_buffer.h"
#include "light.h"
#include "camera.h"

namespace Byte {

	struct RenderContext {
		Buffer<Mesh*> meshes;
		Buffer<Material*> materials;
		Buffer<Transform*> transforms;

		Camera* mainCamera;
		Transform* mainCameraTransform;

		DirectionalLight* directionalLight;
		Transform* directionalLightTransform;
	};

	struct RenderData {
		size_t height{ 0 };
		size_t width{ 0 };

		GBuffer gBuffer{};
		Texture shadowMap;

		using ShaderMap = std::unordered_map<ShaderTag, Shader>;
		ShaderMap shaderMap;

		RenderArray quad;
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

				Shader& shader{ data.shaderMap[material.shaderTag()] };

				shader.bind();

				mesh.renderArray().bind();

				shader.uniform<Vec4>("uAlbedo", material.albedo());

				shader.uniform<Vec3>("uPosition", transform.position());
				shader.uniform<Vec3>("uScale", transform.scale());
				shader.uniform<Quaternion>("uRotation", transform.rotation());

				shader.uniform<Mat4>("uProjection", projection);
				shader.uniform<Mat4>("uView", view);

				OpenglAPI::drawElements(mesh.index().size());

				shader.unbind();
				mesh.renderArray().unbind();
			}

			data.gBuffer.unbind();
		}

	};

	class LightingPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			OpenglAPI::clearBuffer();

			Shader& lightingShader = data.shaderMap["lighting_shader"];
			lightingShader.bind();

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

			OpenglAPI::bindTexture(data.gBuffer.data().position, GL_TEXTURE0);
			OpenglAPI::bindTexture(data.gBuffer.data().normal, GL_TEXTURE1);
			OpenglAPI::bindTexture(data.gBuffer.data().albedoSpecular, GL_TEXTURE2);

			data.quad.bind();

			OpenglAPI::drawQuad();

			data.quad.unbind();

			lightingShader.unbind();

			OpenglAPI::updateDepth(data.gBuffer.data(), data.width, data.height);

			OpenglAPI::blitToGBuffer(data.gBuffer.data(), data.width, data.height);
		}

	};

	class ShadowPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {
			
		}

	};

}
