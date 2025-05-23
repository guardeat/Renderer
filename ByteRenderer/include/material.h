#pragma once

#include <type_traits>
#include <unordered_map>

#include "shader.h"
#include "vec.h"
#include "texture.h"

namespace Byte {

	struct MaterialData {
		using ShaderMap = std::unordered_map<std::string, ShaderTag>;
		ShaderMap shaderMap;

		Vec4 albedo;

		float metallic{ 0.5f };
		float roughness{ 0.5f };
		float ambientOcclusion{ 0.1f };
		float emission{ 0.0f };

		Texture* albedoTexture{ nullptr };
		Texture* materialTexture{ nullptr };

		ShadowMode shadow{ ShadowMode::ENABLED };
		TransparencyMode transparency{ TransparencyMode::BINARY };
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

		Vec4 albedo() const {
			return _data.albedo;
		}

		void albedo(Vec4 value) {
			_data.albedo = value;
		}

		void albedo(Vec3 value) {
			_data.albedo = Vec4(value.x,value.y,value.z,1.0f);
		}

		const MaterialData::ShaderMap& shaderMap() const {
			return _data.shaderMap;
		}

		MaterialData::ShaderMap& shaderMap() {
			return _data.shaderMap;
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
			return *_data.albedoTexture;
		}

		const Texture& materialTexture() const {
			return *_data.materialTexture;
		}

		Texture& albedoTexture() {
			return *_data.albedoTexture;
		}

		Texture& materialTexture() {
			return *_data.materialTexture;
		}

		bool hasAlbedoTexture() const {
			return static_cast<bool>(_data.albedoTexture);
		}

		bool hasMaterialTexture() const {
			return static_cast<bool>(_data.materialTexture);
		}

		void albedoTexture(Texture& texture) {
			_data.albedoTexture = &texture;
		}

		void materialTexture(Texture& texture) {
			_data.materialTexture = &texture;
		}

		TextureID albedoTextureID() const {
			if (!_data.albedoTexture) {
				return 0;
			}
			return _data.albedoTexture->id();
		}

		TextureID materialTextureID() const {
			if (!_data.materialTexture) {
				return 0;
			}
			return _data.materialTexture->id();
		}

		ShadowMode shadowMode() const {
			return _data.shadow;
		}

		void shadowMode(ShadowMode mode) {
			_data.shadow = mode;
		}

		TransparencyMode transparencyMode() const {
			return _data.transparency;
		}

		void transparencyMode(TransparencyMode mode) {
			_data.transparency = mode;
		}

	};

}
