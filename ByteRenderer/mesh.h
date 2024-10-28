#pragma once

#include <cstdint>
#include <cmath>
#include <utility> 
#include <memory>

#include "typedefs.h"
#include "render_array.h"

namespace Byte {

	struct MeshGeometry {
		Buffer<float> vertices;
		//Buffer<float> uv;

		Buffer<uint32_t> indices;
	};

	enum class MeshMode : uint8_t {
		STATIC,
		DYNAMIC,
	};

	class Mesh {
	private:
		MeshGeometry _geometry;
		MeshMode _meshMode{ MeshMode::STATIC };
		RenderArray _renderArray;

	public:
		Mesh() = default;

		Mesh(MeshGeometry&& geometry, MeshMode meshMode)
			: _geometry{ std::move(geometry) }, _meshMode{ meshMode }
		{}

		const Buffer<float>& vertices() const {
			return _geometry.vertices;
		}

		const Buffer<uint32_t>& indices() const {
			return _geometry.indices;
		}

		MeshMode meshMode() const {
			return _meshMode;
		}

		const RenderArray& renderArray() const {
			return _renderArray;
		}

		void renderArray(RenderArray&& renderArray) {
			_renderArray = std::move(renderArray);
		}

		const MeshGeometry& geometry() const {
			return _geometry;
		}
	};

    struct MeshBuilder {
        static Mesh sphere(float radius, size_t numSegments) {
            size_t numVertices = (numSegments + 1) * (numSegments + 1);
            size_t numTriangles = numSegments * numSegments * 2;

            Buffer<float> vertexData(numVertices * (3 + 3 + 2));
            Buffer<uint32_t> indices(numTriangles * 3);

            for (size_t i = 0; i <= numSegments; ++i) {
                float phi = pi<float>() * static_cast<float>(i) / numSegments;
                for (size_t j = 0; j <= numSegments; ++j) {
                    float theta = 2.0f * pi<float>() * static_cast<float>(j) / numSegments;

                    float x = radius * std::sin(phi) * std::cos(theta);
                    float y = radius * std::sin(phi) * std::sin(theta);
                    float z = radius * std::cos(phi);

                    size_t index = i * (numSegments + 1) + j;
                    size_t offset = index * (3 + 3 + 2); 

                    vertexData[offset] = x;
                    vertexData[offset + 1] = y;
                    vertexData[offset + 2] = z;

                    Vec3 normal = Vec3(x, y, z).normalized();
                    vertexData[offset + 3] = normal.x;
                    vertexData[offset + 4] = normal.y;
                    vertexData[offset + 5] = normal.z;

                    vertexData[offset + 6] = static_cast<float>(j) / numSegments;
                    vertexData[offset + 7] = static_cast<float>(i) / numSegments;
                }
            }

            for (size_t i = 0; i < numSegments; ++i) {
                for (size_t j = 0; j < numSegments; ++j) {
                    uint32_t first = static_cast<uint32_t>(i * (numSegments + 1) + j);
                    uint32_t second = static_cast<uint32_t>(first + numSegments + 1);

                    indices.push_back(first);
                    indices.push_back(second);
                    indices.push_back(first + 1);

                    indices.push_back(second);
                    indices.push_back(second + 1);
                    indices.push_back(first + 1);
                }
            }

            MeshGeometry geometry{ std::move(vertexData), std::move(indices) };
            return Mesh{ std::move(geometry), MeshMode::STATIC };
        }

        static Mesh plane(float width, float height, size_t numSegments) {
            size_t numVertices = (numSegments + 1) * (numSegments + 1);
            size_t numTriangles = numSegments * numSegments * 2;

            std::vector<float> vertexData(numVertices * (3 + 3 + 2));
            std::vector<uint32_t> indices(numTriangles * 3);

            for (int i = 0; i <= numSegments; ++i) {
                float y = static_cast<float>(i) / numSegments * height - height / 2.0f;
                for (int j = 0; j <= numSegments; ++j) {
                    float x = static_cast<float>(j) / numSegments * width - width / 2.0f;
                    size_t index = i * (numSegments + 1) + j;
                    size_t offset = index * (3 + 3 + 2); 

                    vertexData[offset] = x;
                    vertexData[offset + 1] = y;
                    vertexData[offset + 2] = 0.0f;

                    vertexData[offset + 3] = 0.0f;
                    vertexData[offset + 4] = 0.0f;
                    vertexData[offset + 5] = 1.0f;

                    vertexData[offset + 6] = static_cast<float>(j) / numSegments;
                    vertexData[offset + 7] = static_cast<float>(i) / numSegments;
                }
            }

            for (int i = 0; i < numSegments; ++i) {
                for (int j = 0; j < numSegments; ++j) {
                    uint32_t topLeft = static_cast<uint32_t>(i * (numSegments + 1) + j);
                    uint32_t topRight = topLeft + 1;
                    uint32_t bottomLeft = static_cast<uint32_t>((i + 1) * (numSegments + 1) + j);
                    uint32_t bottomRight = bottomLeft + 1;

                    size_t index = (i * numSegments + j) * 6;

                    indices[index] = topLeft;
                    indices[index + 1] = bottomLeft;
                    indices[index + 2] = topRight;

                    indices[index + 3] = topRight;
                    indices[index + 4] = bottomLeft;
                    indices[index + 5] = bottomRight;
                }
            }

            MeshGeometry geometry{ std::move(vertexData), std::move(indices) };
            return Mesh{ std::move(geometry), MeshMode::STATIC };
        }
    };


}
