#pragma once

#include <memory>
#include <cstdint>
#include <string>

#include "render_type.h"
#include "render_api.h"

namespace Byte {

	class Texture {
	private:
		TextureData _data{};

	public:
		Texture() = default;

		Texture(TextureData&& data) 
			:_data{ std::move(data) } {
		}

		Texture(const Texture& left) = delete;

		Texture(Texture&& right) 
			:_data{std::move(right._data)} {
			right._data.id = 0;
		}

		Texture& operator=(const Texture& left) = delete;

		Texture& operator=(Texture&& right) {
			_data = std::move(right._data);
			right._data.id = 0;
		}

		~Texture() {
			if (_data.id) {
				RenderAPI::Texture::release(_data.id);
			}
		}

		const TextureData& data() const {
			return _data;
		}

		TextureData& data() {
			return _data;
		}

		size_t height() const {
			return _data.height;
		}

		size_t width() const {
			return _data.width;
		}

		const Path& path() const {
			return _data.path;
		}

		TextureID id() const {
			return _data.id;
		}

		void id(TextureID id) {
			_data.id = id;
		}

		void bind(TextureUnit unit = TextureUnit::T0) const {
			RenderAPI::Texture::bind(_data.id, unit);
		}

		void unbind() const {
			RenderAPI::Texture::unbind();
		}

		void clear() {
			if (_data.id) {
				RenderAPI::Texture::release(_data.id);
			}

			_data.id = 0;
			_data.path.clear();
			_data.data.clear();
			_data.height = 0;
			_data.width = 0;
			_data.layerCount = 0;
		}
	};

}
