#pragma once

#include <memory>
#include <cstdint>
#include <string>

#include "render_component.h"

namespace Byte {

	class Texture {
	private:
		using STextureData = std::shared_ptr<TextureData>;
		STextureData data;

	public:
		Texture() = default;

		Texture(TextureData& data) 
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
