#pragma once

#include <cstdint>

#include "opengl_api.h"
#include "typedefs.h"

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
			right._data.PBO = 0;
			right._data.NBO = 0;
			right._data.UVBO = 0;
			right._data.EBO = 0;
			right._data.UV2BO = 0;
		}

		RenderArray& operator=(RenderArray&& right) noexcept {
			clear();

			_data = right._data;

			right._data.VAO = 0;
			right._data.PBO = 0;
			right._data.NBO = 0;
			right._data.UVBO = 0;
			right._data.EBO = 0;
			right._data.UV2BO = 0;

			return *this;
		}

		~RenderArray() {
			clear();
		}

		void bind() const {
			OpenglAPI::bindRenderArray(_data.VAO);
		}

		void unbind() const {
			OpenglAPI::unbindRenderArray();
		}

		const RArrayData& data() const {
			return _data;
		}

		void clear() {
			OpenglAPI::deleteRenderArray(_data);

			_data.VAO = 0;
			_data.PBO = 0;
			_data.NBO = 0;
			_data.UVBO = 0;
			_data.EBO	= 0;
			_data.UV2BO = 0;
		}
	};

}
