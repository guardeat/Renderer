#pragma once

#include "mesh.h"
#include "material.h"
#include "transform.h"

namespace Byte {

    using InstanceID = uint32_t;

    class RenderInstance {
    private:
        InstanceID instanceID{ 0 };

        Mesh* _mesh;
        Material* _material;

        Buffer<Transform*> _transforms;

    public:
        RenderInstance() = default;

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

        Buffer<Transform*>& transforms() {
            return _transforms;
        }

        const Buffer<Transform*>& transforms() const {
            return _transforms;
        }

        InstanceID id() const {
            return instanceID;
        }

        void id(InstanceID newID) {
            instanceID = newID;
        }
    };

    struct InstanceIDGenerator {
    private:
        InstanceID current{ 1 };

    public:
        InstanceID generate() {
            return current++;
        }
    };

}
