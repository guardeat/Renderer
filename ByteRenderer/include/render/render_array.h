#pragma once

#include <cstdint>

#include "render_api.h"
#include "render_type.h"

namespace Byte {

	class RenderArray {
	private:
		RenderArrayData _data;

	public:
		RenderArray() = default;

		RenderArray(RenderArrayData&& rArrayData)
			:_data{std::move(rArrayData)} {
		}

		RenderArray(RenderArray&& right) noexcept
			: _data{std::move(right._data)} {

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
			RenderAPI::RenderArray::bind(_data.VAO);
		}

		void unbind() const {
			RenderAPI::RenderArray::unbind();
		}

		const RenderArrayData& data() const {
			return _data;
		}

		void clear() {
			RenderAPI::RenderArray::release(_data);

			_data.VAO = 0;
			_data.VBuffers.clear();
			_data.EBO	= 0;
		}
	};

}
