#pragma once

#include <cmath>

#include "vec.h"

namespace Byte {

	struct DirectionalLight {
		Vec3 color{ 1.0f, 1.0f, 1.0f };
		float intensity{ 0.9f };
	};

	struct PointLight {
		Vec3 color{ 1.0f,1.0f,1.0f };

		float constant = 1.0f;
		float linear = 0.3f;
		float quadratic = 0.2f;

		float radius() {
			float lightMax{ std::fmaxf(std::fmaxf(color.x, color.y), color.z) };
			float radius{ 
				-linear + std::sqrtf(linear * linear - 4 * quadratic * (constant - (256.0f / 1.0f) * lightMax)) };
				
			radius /= (2 * quadratic);

			return radius;
		}
	};

}