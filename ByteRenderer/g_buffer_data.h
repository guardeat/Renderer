#pragma once

#include <cstdint>

namespace Byte {

	struct GBufferData {
		uint32_t id;

		uint32_t position;
		uint32_t normal;
		uint32_t albedoSpecular;
	};

}