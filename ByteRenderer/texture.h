#pragma once

#include <memory>
#include <cstdint>
#include <string>

#include "texture_data.h"

namespace Byte {

	class Texture {
	private:
		TextureID id{ 0 };

		using STextureData = std::shared_ptr<TextureData>;
		STextureData data;

	public:
		Texture() = default;

		Texture(TextureID id, TextureData& data) 
			:data{ &data } {
		}

		size_t height() const {
			return data->height;
		}

		size_t width() const {
			return data->width;
		}

		size_t channels() const {
			return data->channels;
		}

		const Path& path() const {
			return data->path;
		}
	};

}
