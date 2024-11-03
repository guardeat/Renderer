#pragma once

#include <cstdint>

#include "opengl_api.h"
#include "render_type.h"

namespace Byte {

	class RenderArray {
	private:
		RArrayData _data;

	public:
		RenderArray() = default;

		RenderArray(RArrayData rArrayData)
			:_data{rArrayData} {
		}

		RenderArray(RenderArray&& right) noexcept
			: _data{right._data} {

			right._data.VAO = 0;
			right._data.VBuffers.clear();
			right._data.EBO = 0;
		}

		RenderArray& operator=(RenderArray&& right) noexcept {
			clear();

			_data = std::move(right._data);

			right._data.VAO = 0;
			right._data.VBuffers.clear();
			right._data.EBO = 0;

			return *this;
		}

		~RenderArray() {
			clear();
		}

		void bind() const {
			OpenglAPI::RArray::bind(_data.VAO);
		}

		void unbind() const {
			OpenglAPI::RArray::unbind();
		}

		const RArrayData& data() const {
			return _data;
		}

		void clear() {
			OpenglAPI::RArray::release(_data);

			_data.VAO = 0;
			_data.VBuffers.clear();
			_data.EBO	= 0;
		}
	};

}
