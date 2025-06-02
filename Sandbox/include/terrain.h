#pragma once

#include <vector>
#include <cmath>
#include <vector>
#include <algorithm>

#include "core/mesh.h"

namespace Byte {

	inline Mesh buildTerrain(size_t width, size_t height, size_t resolution) {
		std::vector<float> vertices{};
		std::vector<uint32_t> indices{};

		for (size_t i = 0; i < resolution; ++i) {
			for (size_t j = 0; j < resolution; ++j) {
				float x0 = -static_cast<float>(width) / 2.0f + static_cast<float>(width) * i / static_cast<float>(resolution);
				float x1 = -static_cast<float>(width) / 2.0f + static_cast<float>(width) * (i + 1) / static_cast<float>(resolution);

				float z0 = -static_cast<float>(height) / 2.0f + static_cast<float>(height) * j / static_cast<float>(resolution);
				float z1 = -static_cast<float>(height) / 2.0f + static_cast<float>(height) * (j + 1) / static_cast<float>(resolution);

				float u0 = i / static_cast<float>(resolution);
				float u1 = (i + 1) / static_cast<float>(resolution);
				float v0 = j / static_cast<float>(resolution);
				float v1 = (j + 1) / static_cast<float>(resolution);

				std::vector<float> quadVertices{
					x0, 0.0f, z0, u0, v0,
					x1, 0.0f, z0, u1, v0,
					x0, 0.0f, z1, u0, v1,
					x1, 0.0f, z1, u1, v1
				};
				vertices.insert(vertices.end(), quadVertices.begin(), quadVertices.end());

				uint32_t indexOffset = static_cast<uint32_t>((i * resolution + j) * 4);
				std::vector<uint32_t> quadIndices{
					indexOffset + 0, indexOffset + 1, indexOffset + 2, indexOffset + 3
				};
				indices.insert(indices.end(), quadIndices.begin(), quadIndices.end());
			}
		}

		MeshData data{ std::move(vertices), std::move(indices), MeshMode::STATIC, 1000.0f, {3,2} };
		return Mesh{ std::move(data) };
	}
}
