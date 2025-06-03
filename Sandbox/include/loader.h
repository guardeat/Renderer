#pragma once

#include "stb_image.h"

#include "render.h"

namespace Byte {

	struct Loader {
		static TextureData loadTexture(const Path& path, DataType type, size_t forceChannels = 0) {
			//stbi_set_flip_vertically_on_load(true);
			int force{ static_cast<int>(forceChannels) };

			TextureData texture;
			texture.path = path;
			texture.dataType = type;

			int width, height, channels;
			std::string pathString{ path.string() };

			void* imgData{ nullptr };

			if (type == DataType::SHORT || type == DataType::UNSIGNED_SHORT) {
				imgData = stbi_load_16(pathString.c_str(), &width, &height, &channels, force);
			}
			else if (type == DataType::FLOAT) {
				imgData = stbi_loadf(pathString.c_str(), &width, &height, &channels, force);
			}
			else {
				imgData = stbi_load(pathString.c_str(), &width, &height, &channels, force);
			}

			if (!imgData) {
				throw std::runtime_error{ "Failed to load texture: " + pathString };
			}

			texture.width = static_cast<size_t>(width);
			texture.height = static_cast<size_t>(height);

			channels = (force != 0) ? force : channels;

			if (channels == 1) {
				texture.internalFormat = ColorFormat::R32F;
				texture.format = ColorFormat::RED;
			}
			else if (channels == 3) {
				texture.internalFormat = ColorFormat::RGB32F;
				texture.format = ColorFormat::RGB;
			}
			else {
				texture.internalFormat = ColorFormat::RGBA32F;
				texture.format = ColorFormat::RGBA;
			}

			size_t bytesPerChannel{ 1 };
			if (type == DataType::SHORT || type == DataType::UNSIGNED_SHORT) {
				bytesPerChannel = 2;
			}
			else if (type == DataType::FLOAT) {
				bytesPerChannel = 4;
			}

			size_t dataSize{ static_cast<size_t>(width * height * channels) * bytesPerChannel };

			texture.data.resize(dataSize);
			std::memcpy(texture.data.data(), imgData, dataSize);

			stbi_image_free(imgData);

			return texture;
		}

	};

}
