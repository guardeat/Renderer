#pragma once

#include "math/quaternion.h"
#include "math/vec.h"
#include "math/trigonometry.h"
#include "math/mat.h"
#include "context.h"
#include "render_api.h"
#include "render_data.h"
#include "texture.h"

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
		void render(RenderContext& context, RenderData& data) override;

	private:
		Frustum createFrustum(const Camera& camera, const Transform& transform, float aspectRatio) const;

		bool inside(const Frustum& frustum, const Transform& transform, float radius) const;

	};

	class SkyboxPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override;
	};

	class ShadowPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override;

	private:
		void renderEntities(RenderContext& context, const Shader& shader) const;

		void renderInstances(RenderContext& context) const;

		void updateLightMatrices(float aspectRatio, RenderData& data, RenderContext& context);

		Mat4 frustumSpace(
			const Mat4& projection,
			const Mat4& view,
			const Transform& lightTransform,
			float far) const;

	};

	class GeometryPass : public RenderPass {
	protected:
		void renderEntities(
			RenderContext& context,
			RenderData& data,
			const Mat4& projection,
			const Mat4& view,
			const ShaderTag& defaultTag,
			TransparencyMode mode) const;

		void renderInstances(
			RenderContext& context,
			RenderData& data,
			const Mat4& projection,
			const Mat4& view,
			const ShaderTag& defaultTag,
			TransparencyMode mode) const;

	};

	class OpaquePass : public GeometryPass {
	public:
		void render(RenderContext& context, RenderData& data) override;
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
			}};

	public:
		SSAOPass();

		void render(RenderContext& context, RenderData& data) override;
	};

	class LightingPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override;

	private:
		void setupGBufferTextures(Shader& shader);

		void setupCascades(RenderData& data, Shader& shader, Camera& camera);

		void bindCommon(Shader& shader, Mat4& view, Mat4& iView, Mat4& iProjection, Vec3& viewPos);

	};

	class TransparentPass : public GeometryPass {
	public:
		void render(RenderContext& context, RenderData& data) override;
	};

	class BloomPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override;
	};

	class DrawPass : public RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override;
	};

}