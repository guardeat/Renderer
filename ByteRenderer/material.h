#pragma once

#include "shader.h"
#include "vec.h"

namespace Byte {

	struct MaterialData {
		ShaderTag shaderTag;
		Vec4 albedo;
		float metallic{ 1.0f };
		float roughness{ 1.0f };
	};

	class Material {
	private:
		MaterialData _data;

	public:
		Vec4 albedo() const {
			return _data.albedo;
		}

		void albedo(Vec4 value) {
			_data.albedo = value;
		}

		const ShaderTag& shaderTag() const {
			return _data.shaderTag;
		}

		void shaderTag(const ShaderTag& tag) {
			_data.shaderTag = tag;
		}
	};

}
