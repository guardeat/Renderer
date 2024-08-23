#pragma once

#include <cstdint>
#include <string>

#include "buffer.h"

namespace Byte {

	struct SolidGeometry {
		Buffer<float> position;
		Buffer<float> normal;
		Buffer<float> uv1;

		Buffer<uint32_t> index;

		Buffer<float> uv2;
	};

}
