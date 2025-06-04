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

        for (size_t i{ 0 }; i < resolution; ++i) {
            for (size_t j{ 0 }; j < resolution; ++j) {
                float x0{ -static_cast<float>(width) / 2.0f + static_cast<float>(width) * static_cast<float>(i) / 
                    static_cast<float>(resolution) };
                float x1{ -static_cast<float>(width) / 2.0f + static_cast<float>(width) * static_cast<float>(i + 1) / 
                    static_cast<float>(resolution) };

                float z0{ -static_cast<float>(height) / 2.0f + static_cast<float>(height) * static_cast<float>(j) /
                    static_cast<float>(resolution) };
                float z1{ -static_cast<float>(height) / 2.0f + static_cast<float>(height) * static_cast<float>(j + 1) /
                    static_cast<float>(resolution) };

                float u0{ static_cast<float>(i) / static_cast<float>(resolution) };
                float u1{ static_cast<float>(i + 1) / static_cast<float>(resolution) };
                float v0{ static_cast<float>(j) / static_cast<float>(resolution) };
                float v1{ static_cast<float>(j + 1) / static_cast<float>(resolution) };

                vertices.insert(vertices.end(), {
                    x0, 0.0f, z0, u0, v0,
                    x1, 0.0f, z0, u1, v0,
                    x0, 0.0f, z1, u0, v1,
                    x1, 0.0f, z1, u1, v1
                    });

                uint32_t indexOffset{ static_cast<uint32_t>((i * resolution + j) * 4) };
                indices.insert(indices.end(), {
                    indexOffset + 0, indexOffset + 1, indexOffset + 2,
                    indexOffset + 3
                    });
            }
        }

        MeshData data{
            std::move(vertices),
            std::move(indices),
            MeshMode::STATIC,
            1000.0f,
            {3, 2}
        };

        return Mesh{ std::move(data) };
    }

    inline TextureData readTerrain(const Path& path, int channels = 1) {
        TextureData out;

        std::ifstream file(path.string(), std::ios::binary | std::ios::ate);
        if (!file) {
            std::cerr << "Cannot open file\n";
            return out;
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        if (size <= 0) {
            std::cerr << "File is empty or read failed\n";
            return out;
        }

        std::vector<uint8_t> rawData(size);
        if (!file.read(reinterpret_cast<char*>(rawData.data()), size)) {
            std::cerr << "Failed to read file data\n";
            return out;
        }

        size_t elementSize = 2 * channels;
        if (rawData.size() % elementSize != 0) {
            std::cerr << "Invalid data size, must be multiple of 2 * channels\n";
            return out;
        }

        size_t pixelCount = rawData.size() / elementSize;
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

        out.data = std::move(rawData);
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

        return (normalizedHeight * 64.0f - 16.0f) * scale;
    }

}
