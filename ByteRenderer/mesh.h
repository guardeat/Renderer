#pragma once

#include <cstdint>

#include "solid_geometry.h"
#include "buffer.h"
#include "render_array.h"

namespace Byte {

	enum class MeshMode : uint8_t {
		STATIC,
		DYNAMIC,
	};

	struct MeshData {
		SolidGeometry geometry;

		MeshMode meshMode{ MeshMode::STATIC };

		RenderArray renderArray;
	};

	class Mesh {
	private:
		MeshData data;

		friend class Renderer;

	public:
		Mesh() = default;

		Mesh(
			Buffer<float>&& _position,
			Buffer<float>&& _normal,
			Buffer<float>&& _uv,
			Buffer<uint32_t>&& _index,
			MeshMode _meshMode = MeshMode::STATIC) 
			: data{ {_position,_normal,_uv,_index},_meshMode } {
		}

		const Buffer<float>& position() const {
			return data.geometry.position;
		}

		const Buffer<float>& normal() const {
			return data.geometry.normal;
		}

		const Buffer<float>& uv1() const {
			return data.geometry.uv1;
		}

		const Buffer<uint32_t>& index() const {
			return data.geometry.index;
		}

		MeshMode meshMode() const {
			return data.meshMode;
		}

		const RenderArray& renderArray() const {
			return data.renderArray;
		}

		const Buffer<float>& uv2() const {
			return data.geometry.uv2;
		}
	};

}
