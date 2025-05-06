#pragma once

#include <type_traits>

#include "shader.h"
#include "vec.h"
#include "texture.h"

namespace Byte {
	
	enum class ShadowMode : uint8_t {
		DISABLED,
		FULL,
	};

	struct MaterialData {
		ShaderTag shaderTag;
		Vec3 albedo;

		float metallic{ 0.5f };
		float roughness{ 0.5f };
		float ambientOcclusion{ 0.1f };
		float emission{ 0.0f };

		Texture albedoTexture;
		Texture materialTexture;

		ShadowMode shadow{ ShadowMode::FULL };
	};

	class Material {
	private:
		MaterialData _data;

	public:
		Material() = default;

		Material(MaterialData&& data)
			:_data{ std::forward<MaterialData>(data) }
		{
		}

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

		const Texture& albedoTexture() const {
			return _data.albedoTexture;
		}

		const Texture& materialTexture() const {
			return _data.materialTexture;
		}

		Texture& albedoTexture() {
			return _data.albedoTexture;
		}

		Texture& materialTexture() {
			return _data.materialTexture;
		}

		void albedoTexture(Texture texture) {
			_data.albedoTexture = texture;
		}

		void materialTexture(Texture texture) {
			_data.materialTexture = texture;
		}

		TextureID albedoTextureID() const {
			if (!_data.albedoTexture) {
				return 0;
			}
			return _data.albedoTexture.id();
		}

		TextureID materialTextureID() const {
			if (!_data.materialTexture) {
				return 0;
			}
			return _data.materialTexture.id();
		}

		void albedoTextureID(TextureID id) {
			_data.albedoTexture.id(id);
		}

		void materialTextureID(TextureID id) {
			_data.materialTexture.id(id);
		}

		ShadowMode shadowMode() const {
			return _data.shadow;
		}

		void shadowMode(ShadowMode mode) {
			_data.shadow = mode;
		}

	};

}
