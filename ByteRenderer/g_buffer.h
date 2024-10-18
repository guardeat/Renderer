#pragma once

#include "typedefs.h"
#include "opengl_api.h"

namespace Byte {

	class GBuffer {
	private:
		GBufferData _data;

	public:
		GBuffer() = default;

		GBuffer(GBufferData&& _data)
			:_data{ _data } {
		}

		GBuffer(GBuffer&& right) noexcept
			:_data{right._data} {
			right._data.id = 0;
			right._data.position = 0;
			right._data.normal = 0;
			right._data.albedoSpecular = 0;
		}

		GBuffer& operator=(GBuffer&& right) noexcept {
			clear();

			_data = right._data;

			right._data.id = 0;
			right._data.position = 0;
			right._data.normal = 0;
			right._data.albedoSpecular = 0;

			return *this;
		}

		~GBuffer() {
			clear();
		}

		void bind() {
			OpenglAPI::bindGBuffer(_data);
		}

		void unbind() {
			OpenglAPI::unbindGBuffer();
		}

		const GBufferData& data() const {
			return _data;
		}

		void clearContent() {
			OpenglAPI::clearBuffer(_data);
		}

		void clear() {
			OpenglAPI::deleteGbuffer(_data);

			_data.id = 0;
			_data.position = 0;
			_data.normal = 0;
			_data.albedoSpecular = 0;
		}
	};

}
