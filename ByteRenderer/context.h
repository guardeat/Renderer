#pragma once

#include <tuple>
#include <unordered_map>

#include "render_type.h"
#include "mesh.h"
#include "material.h"
#include "transform.h"
#include "mat.h"
#include "framebuffer.h"
#include "light.h"
#include "camera.h"
#include "instancing.h"

namespace Byte {

    class RenderContext {
    public:
        struct RenderEntity {
            Mesh* mesh;
            Material* material;
            Transform* transform;
        };

        template<typename Type>
        struct RenderItem {
            Type* item{};
            Transform* transform{};
        };

    private:
        Buffer<RenderEntity> _renderEntities;

        RenderItem<Camera> _camera{};
        RenderItem<DirectionalLight> _directionalLight{};

        Buffer<RenderItem<PointLight>> _pointLights;

        using InstanceMap = std::unordered_map<InstanceTag, RenderInstance>;
        InstanceMap _instances;

    public:
        void submit(Mesh& mesh, Material& material, Transform& transform) {
            _renderEntities.emplace_back(&mesh, &material, &transform);
        }

        void submit(Camera& camera, Transform& cameraTransform) {
            _camera = { &camera, &cameraTransform };
        }

        void submit(DirectionalLight& light, Transform& lightTransform) {
            _directionalLight = { &light, &lightTransform };
        }

        void submit(PointLight& light, Transform& lightTransform) {
            _pointLights.emplace_back(&light, &lightTransform);
        }

        RenderEntity& entity(size_t index) {
            return _renderEntities[index];
        }

        const RenderEntity& entity(size_t index) const { 
            return _renderEntities[index];
        }

        Mesh& mesh(size_t index) {
            return *_renderEntities[index].mesh;
        }

        const Mesh& mesh(size_t index) const { 
            return *_renderEntities[index].mesh;
        }

        Material& material(size_t index) {
            return *_renderEntities[index].material;
        }

        const Material& material(size_t index) const {
            return *_renderEntities[index].material;
        }

        Transform& transform(size_t index) {
            return *_renderEntities[index].transform;
        }

        const Transform& transform(size_t index) const { 
            return *_renderEntities[index].transform;
        }

        Buffer<RenderEntity>& entities() {
            return _renderEntities;
        }

        const Buffer<RenderEntity>& entities() const { 
            return _renderEntities;
        }

        size_t entityCount() const {
            return _renderEntities.size();
        }

        RenderItem<Camera> camera() {
            return _camera;
        }

        const RenderItem<Camera> camera() const { 
            return _camera;
        }

        RenderItem<DirectionalLight> directionalLight() {
            return _directionalLight;
        }

        const RenderItem<DirectionalLight> directionalLight() const { 
            return _directionalLight;
        }

        RenderItem<PointLight> pointLight(size_t index) {
            return _pointLights[index];
        }

        const RenderItem<PointLight> pointLight(size_t index) const { 
            return _pointLights[index];
        }

        size_t pointLightCount() const {
            return _pointLights.size();
        }

        RenderInstance& instance(const InstanceTag& tag) {
            return _instances.at(tag);
        }

        const RenderInstance& instance(const InstanceTag& tag) const { 
            return _instances.at(tag);
        }

        InstanceMap& instances() {
            return _instances;
        }

        const InstanceMap& instances() const { 
            return _instances;
        }

        void clear() {
            _renderEntities.clear();
            _pointLights.clear();
            _instances.clear();

            _camera.item = nullptr;
            _camera.transform = nullptr;
            _directionalLight.item = nullptr;
            _directionalLight.transform = nullptr;
        }
    };

}
