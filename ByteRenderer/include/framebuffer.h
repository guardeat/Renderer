#pragma once

#include "render_type.h"
#include "render_api.h"

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

		size_t width() const {
			return _data.width;
		}

		size_t height() const {
			return _data.height;
		}

		void bind() {
			RenderAPI::Framebuffer::bind(_data);
		}

		void unbind() {
			RenderAPI::Framebuffer::unbind();
		}

		void resize(size_t width, size_t height) {
			if (_data.resize) {
				RenderAPI::Framebuffer::release(_data);
				_data.attachments.clear();
				_data.width = static_cast<size_t>(static_cast<float>(width) * _data.resizeFactor);
				_data.height = static_cast<size_t>(static_cast<float>(height) * _data.resizeFactor);
				RenderAPI::Framebuffer::build(_data);
			}
		}

		FramebufferData& data() {
			return _data;
		}

		const FramebufferData& data() const {
			return _data;
		}

		void clearContent() {
			RenderAPI::Framebuffer::clear(_data.id);
		}

		void clearDepth() {
			RenderAPI::Framebuffer::clearDepth(_data.id);
		}

		void clear() {
			RenderAPI::Framebuffer::release(_data);

			_data.textures.clear();
			_data.attachments.clear();
		}

		TextureID textureID(const std::string& name) const {
			return _data.textures.at(name).id;
		}

		FramebufferID id() const {
			return _data.id;
		}
	};

}
