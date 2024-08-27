#pragma once

#include <cstdint>
#include <cmath>
#include <utility> 

#include "buffer.h"
#include "render_array.h"

namespace Byte {

	struct MeshGeometry {
		Buffer<float> position;
		Buffer<float> normal;
		Buffer<float> uv1;
		Buffer<uint32_t> index;
		Buffer<float> uv2;
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

		friend class Renderer;

	public:
		Mesh() = default;

		Mesh(
			Buffer<float>&& position,
			Buffer<float>&& normal,
			Buffer<float>&& uv1,
			Buffer<uint32_t>&& index,
			MeshMode meshMode = MeshMode::STATIC)
			: _geometry{ std::move(position), std::move(normal), std::move(uv1), std::move(index), {} },
			_meshMode{ meshMode }
		{}

		const Buffer<float>& position() const {
			return _geometry.position;
		}

		const Buffer<float>& normal() const {
			return _geometry.normal;
		}

		const Buffer<float>& uv1() const {
			return _geometry.uv1;
		}

		const Buffer<uint32_t>& index() const {
			return _geometry.index;
		}

		const Buffer<float>& uv2() const {
			return _geometry.uv2;
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

		static Mesh sphere(float radius, size_t numSegments) {
			size_t numVertices = (numSegments + 1) * (numSegments + 1);
			size_t numTriangles = numSegments * numSegments * 2;

			Buffer<float> vertices(numVertices * 3);
			Buffer<float> normals(numVertices * 3);
			Buffer<float> texCoords(numVertices * 2);
			Buffer<uint32_t> indices(numTriangles * 3);

			for (size_t i = 0; i <= numSegments; ++i) {
				float phi = pi<float>() * static_cast<float>(i) / numSegments;
				for (size_t j = 0; j <= numSegments; ++j) {
					float theta = 2.0f * pi<float>() * static_cast<float>(j) / numSegments;

					float x = radius * std::sin(phi) * std::cos(theta);
					float y = radius * std::sin(phi) * std::sin(theta);
					float z = radius * std::cos(phi);

					size_t index = i * (numSegments + 1) + j;

					vertices[index * 3] = x;
					vertices[index * 3 + 1] = y;
					vertices[index * 3 + 2] = z;

					Vec3 normal{ Vec3(x, y, z).normalized() };
					normals[index * 3] = normal.x;
					normals[index * 3 + 1] = normal.y;
					normals[index * 3 + 2] = normal.z;

					texCoords[index * 2] = static_cast<float>(j) / numSegments;
					texCoords[index * 2 + 1] = static_cast<float>(i) / numSegments;
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

			return Mesh{ std::move(vertices), std::move(normals), std::move(texCoords), std::move(indices) };
		}

		static Mesh plane(float width, float height, int numSegments) {
			size_t numVertices = (numSegments + 1) * (numSegments + 1);
			size_t numTriangles = numSegments * numSegments * 2;

			std::vector<float> vertices(numVertices * 3);
			std::vector<float> normals(numVertices * 3, 0.0f);
			std::vector<float> texCoords(numVertices * 2);
			std::vector<uint32_t> indices(numTriangles * 3);

			for (int i = 0; i <= numSegments; ++i) {
				float y = static_cast<float>(i) / numSegments * height - height / 2.0f;
				for (int j = 0; j <= numSegments; ++j) {
					float x = static_cast<float>(j) / numSegments * width - width / 2.0f;
					size_t index = i * (numSegments + 1) + j;

					vertices[index * 3] = x;
					vertices[index * 3 + 1] = y;
					vertices[index * 3 + 2] = 0.0f;

					normals[index * 3 + 2] = 1.0f;

					texCoords[index * 2] = static_cast<float>(j) / numSegments;
					texCoords[index * 2 + 1] = static_cast<float>(i) / numSegments;
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

			return Mesh{ std::move(vertices), std::move(normals), std::move(texCoords), std::move(indices) };
		}
	};

}
