#pragma once

#include <string>
#include <numeric>

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
        size_t _stride{};
        Buffer<uint8_t> _layout{};

        bool _change{ false };

        size_t _size{ 0 };
        size_t _bufferCapacity{ 0 };

        Buffer<RenderID> _renderIDs;

    public:
        RenderInstance() = default;

        RenderInstance(Mesh& mesh, Material& material, Buffer<uint8_t>&& layout = { 3,3,4 })
            : _mesh{ &mesh }, _material{ &material }, _layout{ std::forward<Buffer<uint8_t>>(layout) } {
            _stride = std::accumulate(layout.begin(), layout.end(), 0);
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

        void add(const Transform& transform, RenderID id) {
            Buffer<float> values;
            values.reserve(10);

            values.push_back(transform.position().x);
            values.push_back(transform.position().y);
            values.push_back(transform.position().z);

            values.push_back(transform.scale().x);
            values.push_back(transform.scale().y);
            values.push_back(transform.scale().z);

            values.push_back(transform.rotation().x);
            values.push_back(transform.rotation().y);
            values.push_back(transform.rotation().z);
            values.push_back(transform.rotation().w);
            
            add(values, id);
        }

        void add(const Buffer<float>& values, RenderID id) {
            _change = true;
            ++_size;

            _data.insert(_data.end(), values.begin(), values.end());

            _renderIDs.push_back(id);
        }

        bool erase(RenderID id) {
            auto it{ std::find(_renderIDs.begin(), _renderIDs.end(), id) };
            if (it == _renderIDs.end()) {
                return false;
            }

            size_t index{ static_cast<size_t>(std::distance(_renderIDs.begin(), it)) };

            size_t dataStart{ index * _stride };

            _data.erase(_data.begin() + dataStart, _data.begin() + dataStart + _stride);

            _renderIDs.erase(it);

            --_size;
            _change = true;

            return true;
        }

        Buffer<float>& data() {
            return _data;
        }

        const Buffer<float>& data() const {
            return _data;
        }

        Buffer<uint8_t>& layout() {
            return _layout;
        }

        const Buffer<uint8_t>& layout() const {
            return _layout;
        }

        bool changed() const {
            return _change;
        }

        void resetInstanceBuffer() {
            RenderBufferID bufferID{ _mesh->renderArray().data().VBuffers[1].id };

            if (_size > _bufferCapacity) {
                _data.reserve(_size * 2);
                RenderAPI::RenderArray::bufferData(bufferID, _data, _size * 10, true);
                _bufferCapacity = 2 * _size;
            }
            else {
                RenderAPI::RenderArray::subBufferData(bufferID, _data, 0);
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
