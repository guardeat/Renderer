#pragma once

#include <cmath>
#include <algorithm>

#include "vec.h"

namespace Byte {

	struct DirectionalLight {
		Vec3 color{ 1.0f, 1.0f, 1.0f };
		float intensity{ 1.0f };
	};

	struct PointLight {
		Vec3 color{ 1.0f,1.0f,1.0f };

		float constant = 1.0f;
		float linear = 0.01f;
		float quadratic = 0.005f;

		float radius() {
			float maxColorIntensity{ std::fmaxf(std::fmaxf(color.x, color.y), color.z) };

			float lightStrength{ (256.0f / 5.0f) * maxColorIntensity };

			float discriminant{ std::sqrtf(linear * linear - 4 * quadratic * (constant - lightStrength)) };

			return (-linear + discriminant) / (2 * quadratic);
		}
	};

}