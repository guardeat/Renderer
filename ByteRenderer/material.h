#pragma once

#include "shader.h"
#include "vec.h"

namespace Byte {

	struct MaterialData {
		ShaderTag shaderTag;
		Vec4 albedo;
	};

	class Material {
	private:
		MaterialData data;

	public:
		Vec4 albedo() const {
			return data.albedo;
		}

		void albedo(Vec4 value) {
			data.albedo = value;
		}

		const ShaderTag& shaderTag() const {
			return data.shaderTag;
		}

		void shaderTag(const ShaderTag& tag) {
			data.shaderTag = tag;
		}
	};

}
