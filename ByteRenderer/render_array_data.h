#pragma once

#include <cstdint>

namespace Byte {

	using RArrayID = uint32_t;
	using RBufferID = uint32_t;

	struct RArrayData {
		RArrayID VAO{ 0 };
		RBufferID VBO{ 0 };
		RBufferID NBO{ 0 };
		RBufferID UVBO{ 0 };
		RBufferID EBO{ 0 };
		RBufferID UV2BO{ 0 };
	};

}
