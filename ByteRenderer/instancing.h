#pragma once

#include <string>

#include "mesh.h"
#include "material.h"
#include "transform.h"

namespace Byte {

    using InstanceTag = std::string;

    class RenderInstance {
    private:
        Mesh* _mesh{};
        Material* _material{};

        Buffer<float> _data{};
        bool _change{ false };

        size_t _size{ 0 };
        size_t _bufferCapacity{ 0 };

    public:
        RenderInstance() = default;

        RenderInstance(Mesh& mesh, Material& material)
            : _mesh{ &mesh }, _material{&material} {
        }

        Mesh& mesh() {
            return *_mesh;
        }

        const Mesh& mesh() const {
            return *_mesh;
        }

        void mesh(Mesh& newMesh) {
            _mesh = &newMesh;
        }

        Material& material() {
            return *_material;
        }

        const Material& material() const {
            return *_material;
        }

        void material(Material& newMaterial) {
            _material = &newMaterial;
        }

        void add(Transform& transform) {
            _change = true;
            ++_size;

            _data.push_back(transform.position().x);
            _data.push_back(transform.position().y);
            _data.push_back(transform.position().z);

            _data.push_back(transform.scale().x);
            _data.push_back(transform.scale().y);
            _data.push_back(transform.scale().z);

            _data.push_back(transform.rotation().x);
            _data.push_back(transform.rotation().y);
            _data.push_back(transform.rotation().z);
            _data.push_back(transform.rotation().w);
            
        }

        Buffer<float>& data() {
            return _data;
        }

        const Buffer<float>& data() const {
            return _data;
        }

        bool change() const {
            return _change;
        }

        void resetInstanceBuffer() {
            RBufferID bufferID{ _mesh->renderArray().data().VBuffers[1].id };

            if (_size > _bufferCapacity) {
                OpenglAPI::RArray::bufferData(bufferID, _data, _size * 20, true);
                _bufferCapacity = _size * 2;
            }
            else {
                OpenglAPI::RArray::subBufferData(bufferID, _data, 0);
            }
            _change = false;
        }

        size_t size() const {
            return _size;
        }

        void reset() {
            _data.clear();
            _size = 0;
            _change = false;
        }
    };

}
