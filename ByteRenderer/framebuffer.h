#pragma once

#include "typedefs.h"
#include "opengl_api.h"

namespace Byte {

	class Framebuffer {
	private:
		FramebufferData _data;

	public:
		Framebuffer() = default;

		Framebuffer(FramebufferData&& _data)
			:_data{ _data } {
		}

		Framebuffer(Framebuffer&& right) noexcept
			: _data{ std::move(right._data) } {
			right._data.id = 0;
		}

		Framebuffer& operator=(Framebuffer&& right) noexcept {
			clear();
			_data = std::move(right._data);
			right._data.id = 0;

			return *this;
		}

		~Framebuffer() {
			clear();
		}

		void bind() {
			OpenglAPI::Framebuffer::bind(_data.id);
		}

		void unbind() {
			OpenglAPI::Framebuffer::unbind();
		}

		const FramebufferData& data() const {
			return _data;
		}

		void clearContent() {
			OpenglAPI::Framebuffer::clear(_data.id);
		}

		void clear() {
			OpenglAPI::Framebuffer::release(_data);

			_data.id = 0;
			_data.textures.clear();
		}
	};

}
