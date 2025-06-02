#pragma once

#include "core/mesh.h"
#include "render_array.h"
#include "render_type.h"

namespace Byte {

	class MeshRenderer {
	private:
        RenderArray _renderArray;
        PrimitiveType _primitiveType{ PrimitiveType::TRIANGLES };

    public:
        MeshRenderer() = default;

        void upload(const Mesh& mesh) {
            if (_renderArray.data().VAO) {
                _renderArray.clear();
            }

            bool isStatic{ mesh.mode() == MeshMode::STATIC };

            auto atts{ RenderAPI::RenderArray::buildAttributes(mesh.data().vertexLayout) };

            _renderArray = RenderAPI::RenderArray::build(mesh.vertices(), mesh.indices(), atts, isStatic);
        }

        void uploadInstanced(const Mesh& mesh, const Buffer<uint8_t>& layout) {
            if (_renderArray.data().VAO) {
                _renderArray.clear();
            }

            bool isStatic{ mesh.mode() == MeshMode::STATIC };

            auto atts{ RenderAPI::RenderArray::buildAttributes(mesh.data().vertexLayout) };

            auto iAtts{ RenderAPI::RenderArray::buildAttributes(
                layout,
                static_cast<uint8_t>(mesh.data().vertexLayout.size())) };

            auto& vertices{ mesh.vertices() };
            auto& indices{ mesh.indices() };
            _renderArray = RenderAPI::RenderArray::build(vertices,indices,atts,iAtts,isStatic);
        }

        PrimitiveType primitive() const {
            return _primitiveType;
        }

        void primitive(PrimitiveType newPrimitive) {
            _primitiveType = newPrimitive;
        }

        void bind() const {
            _renderArray.bind();
        }

        void unbind() const {
            _renderArray.unbind();
        }

        const RenderArray& renderArray() const { 
            return _renderArray; 
        }

        bool drawable() const { 
            return _renderArray.data().VAO != 0; 
        }

	};

}