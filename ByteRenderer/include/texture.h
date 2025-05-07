#pragma once

#include <memory>
#include <cstdint>
#include <string>

#include "render_type.h"
#include "render_api.h"

namespace Byte {

	class Texture {
	private:
		TextureData* _data{ nullptr };

	public:
		Texture() = default;

		Texture(TextureData& data) 
			:_data{ &data } {
		}

		explicit operator bool() const {
			return static_cast<bool>(_data);
		}

		const TextureData& data() const {
			return *_data;
		}

		TextureData& data() {
			return *_data;
		}

		size_t height() const {
			return _data->height;
		}

		size_t width() const {
			return _data->width;
		}

		const Path& path() const {
			return _data->path;
		}

		TextureID id() const {
			return _data->id;
		}

		void id(TextureID id) {
			_data->id = id;
		}

		void bind(TextureUnit unit = TextureUnit::T0) const {
			RenderAPI::Texture::bind(_data->id, unit);
		}
	};

}
