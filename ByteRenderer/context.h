#pragma once

#include <tuple>
#include <unordered_map>

#include "typedefs.h"
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
        struct RenderTarget {
            Mesh* mesh;
            Material* material;
            Transform* transform;
        };

        template<typename Type>
        struct RenderItem {
            Type* item;
            Transform* transform;
        };

    private:
        Buffer<RenderTarget> _renderTargets;

        RenderItem<Camera> _camera;
        RenderItem<DirectionalLight> _directionalLight;

        Buffer<RenderItem<PointLight>> _pointLights;

        using InstanceMap = std::unordered_map<InstanceID, RenderInstance>;
        InstanceMap _instances;
        InstanceIDGenerator _generator;

    public:
        void submit(Mesh& mesh, Material& material, Transform& transform) {
            _renderTargets.emplace_back(&mesh, &material, &transform);
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

        RenderTarget& target(size_t index) {
            return _renderTargets[index];
        }

        Mesh& mesh(size_t index) {
            return *_renderTargets[index].mesh;
        }

        Material& material(size_t index) {
            return *_renderTargets[index].material;
        }

        Transform& transform(size_t index) {
            return *_renderTargets[index].transform;
        }

        Buffer<RenderTarget>& targets() {
            return _renderTargets;
        }

        size_t targetCount() const {
            return _renderTargets.size();
        }

        RenderItem<Camera> camera() {
            return _camera;
        }

        RenderItem<DirectionalLight> directionalLight() {
            return _directionalLight;
        }

        RenderItem<PointLight> pointLight(size_t index) {
            return _pointLights[index];
        }

        size_t pointLightCount() const {
            return _pointLights.size();
        }

        void clear() {
            _renderTargets.clear();
            _pointLights.clear();
            _instances.clear();

            _camera.item = nullptr;
            _camera.transform = nullptr;
            _directionalLight.item = nullptr;
            _directionalLight.transform = nullptr;
        }
    };

}
