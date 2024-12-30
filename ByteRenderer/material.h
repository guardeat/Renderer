#pragma once

#include "shader.h"
#include "vec.h"

namespace Byte {

	struct MaterialData {
		ShaderTag shaderTag;
		Vec3 albedo;
		float metallic{ 0.5f };
		float roughness{ 0.5f };
		float ambientOcclusion{ 0.1f };
		float emission{ 0.0f };
	};

	class Material {
	private:
		MaterialData _data;

	public:
		Vec3 albedo() const {
			return _data.albedo;
		}

		void albedo(Vec3 value) {
			_data.albedo = value;
		}

		const ShaderTag& shaderTag() const {
			return _data.shaderTag;
		}

		void shaderTag(const ShaderTag& tag) {
			_data.shaderTag = tag;
		}

		float metallic() const {
			return _data.metallic;
		}

		void metallic(float value) {
			_data.metallic = value;
		}

		float roughness() const {
			return _data.roughness;
		}

		void roughness(float value) {
			_data.roughness = value;
		}

		float ambientOcclusion() const {
			return _data.ambientOcclusion;
		}

		void ambientOcclusion(float value) {
			_data.ambientOcclusion = value;
		}

		float emission() const {
			return _data.emission;
		}

		void emission(float value) {
			_data.emission = value;
		}

		const MaterialData& data() const {
			return _data;
		}

		void data(const MaterialData& materialData) {
			_data = materialData;
		}
	};

}
