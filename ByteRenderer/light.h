#pragma once

#include "vec.h"

namespace Byte {

	struct DirectionalLight {
		Vec3 color{ 1.0f, 1.0f, 1.0f };
		float intensity{ 0.5f };
	};

}