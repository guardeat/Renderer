#pragma once

#include <type_traits>
#include <unordered_map>
#include <string>

#include "math/vec.h"
#include "core/core_types.h"

namespace Byte {

	enum class ShadowMode : uint8_t {
		DISABLED,
		ENABLED,
	};

	enum class TransparencyMode : uint8_t {
		BINARY,
		SORTED,
		UNSORTED,
		ORDER_INDEPENDENT,
	};

	class Texture;

	using ShaderTag = std::string;
	using TextureTag = std::string;

	struct MaterialData {
		using ShaderMap = std::unordered_map<std::string, ShaderTag>;
		ShaderMap shaderMap;

		Vec4 albedo{ 1.0f,1.0f,1.0f,1.0f };

		float metallic{ 0.5f };
		float roughness{ 0.5f };
		float ambientOcclusion{ 0.1f };
		float emission{ 0.0f };

		using TextureMap = std::unordered_map<TextureTag, Texture*>;
		TextureMap textureMap;

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

		const Texture& texture(const TextureTag& tag) const {
			return *_data.textureMap.at(tag);
		}

		void texture(const TextureTag& tag, Texture& texture) {
			_data.textureMap.emplace(tag, &texture);
		}

		const MaterialData::TextureMap& textureMap() const {
			return _data.textureMap;
		}

		MaterialData::TextureMap& textureMap() {
			return _data.textureMap;
		}

		bool hasTexture(const TextureTag& tag) const {
			return _data.textureMap.find(tag) != _data.textureMap.end();
		}

		ShadowMode shadow() const {
			return _data.shadow;
		}

		void shadow(ShadowMode mode) {
			_data.shadow = mode;
		}

		TransparencyMode transparency() const {
			return _data.transparency;
		}

		void transparency(TransparencyMode mode) {
			_data.transparency = mode;
		}

	};

}
