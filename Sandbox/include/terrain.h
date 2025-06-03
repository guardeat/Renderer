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

    inline TextureData readTerrain(const Path& path, int channels = 1) {
        TextureData out;

        std::ifstream file(path.string(), std::ios::binary);
        if (!file) {
            std::cerr << "Cannot open file\n";
            return out;
        }

        std::vector<uint8_t> rawData((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());

        if (rawData.empty()) {
            std::cerr << "File is empty or read failed\n";
            return out;
        }

        if (rawData.size() % (2 * channels) != 0) {
            std::cerr << "Invalid data size, must be multiple of 2 * channels\n";
            return out;
        }

        size_t pixelCount = rawData.size() / (2 * channels);
        size_t dimension = static_cast<size_t>(std::sqrt(pixelCount));
        if (dimension * dimension != pixelCount) {
            std::cerr << "Warning: Texture is not a perfect square\n";
        }

        out.width = dimension;
        out.height = dimension;

        out.attachment = AttachmentType::COLOR_0;
        out.layerCount = 1;
        out.type = TextureType::TEXTURE_2D;
        out.wrapS = TextureWrap::CLAMP_TO_EDGE;
        out.wrapT = TextureWrap::CLAMP_TO_EDGE;
        out.minFilter = TextureFilter::LINEAR;
        out.magFilter = TextureFilter::LINEAR;
        out.dataType = DataType::UNSIGNED_SHORT;
        out.path = path;

        if (channels == 1) {
            out.internalFormat = ColorFormat::R16;
            out.format = ColorFormat::RED;
        }
        else if (channels == 3) {
            out.internalFormat = ColorFormat::RGB16;
            out.format = ColorFormat::RGB;
        }
        else if (channels == 4) {
            out.internalFormat = ColorFormat::RGBA16;
            out.format = ColorFormat::RGBA;
        }
        else {
            std::cerr << "Unsupported channel count\n";
            return out;
        }

        out.data = Buffer<uint8_t>(rawData.begin(), rawData.end());

        return out;
    }

    inline float getHeight(const Texture& heightMap, float worldX, float worldZ, float scale = 5.0f,
        float terrainMinX = -500.0f, float terrainMaxX = 500.0f,
        float terrainMinZ = -500.0f, float terrainMaxZ = 500.0f) {

        float u{ (worldX - terrainMinX) / (terrainMaxX - terrainMinX) };
        float v{ (worldZ - terrainMinZ) / (terrainMaxZ - terrainMinZ) };

        u = std::clamp(u, 0.0f, 1.0f);
        v = std::clamp(v, 0.0f, 1.0f);

        size_t texX{ static_cast<size_t>(u * (heightMap.width() - 1)) };
        size_t texY{ static_cast<size_t>(v * (heightMap.height() - 1)) };
        size_t pixelIndex{ texX + texY * heightMap.width() };

        const uint16_t* heightPtr{ reinterpret_cast<const uint16_t*>(heightMap.data().data.data()) };
        uint16_t heightValue{ heightPtr[pixelIndex] };
        float normalizedHeight{ static_cast<float>(heightValue) / 65535.0f };

        return (normalizedHeight * 64.0f - 16.0f) * 5.0f;
    }

}
