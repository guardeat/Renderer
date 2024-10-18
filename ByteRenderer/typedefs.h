#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>

namespace Byte {

	using Path = std::filesystem::path;

	template<typename T>
	using Buffer = std::vector<T>;

	using TextureID = uint32_t;
	using BufferID = uint32_t;

	using RArrayID = uint32_t;
	using RBufferID = uint32_t;

	struct TextureData {
		size_t height;
		size_t width;
		size_t channels;

		Path path;

		std::unique_ptr<uint8_t[]> data;
	};

	struct ShadowBufferData {
		BufferID id;

		TextureID depth;
	};

	struct GBufferData {
		BufferID id;

		TextureID position;
		TextureID normal;
		TextureID albedoSpecular;
	};

	struct RArrayData {
		RArrayID VAO{ 0 };
		RBufferID PBO{ 0 };
		RBufferID NBO{ 0 };
		RBufferID UVBO{ 0 };
		RBufferID EBO{ 0 };
		RBufferID UV2BO{ 0 };
	};

}