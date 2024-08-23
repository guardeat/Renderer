#pragma once

#include <cstdint>
#include <memory>
#include <filesystem>

namespace Byte {

	using Path = std::filesystem::path;

	struct TextureData {
		size_t height;
		size_t width;
		size_t channels;

		Path path;

		std::unique_ptr<uint8_t[]> data;
	};

	using TextureID = uint32_t;

}