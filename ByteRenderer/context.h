#pragma once

#include <tuple>

#include "typedefs.h"
#include "mesh.h"
#include "material.h"
#include "transform.h"
#include "mat.h"
#include "framebuffer.h"
#include "light.h"
#include "camera.h"

namespace Byte {

    using RenderTarget = std::tuple<Mesh&, Material&, Transform&>;

    template<typename Type>
    using RenderItem = std::tuple<Type&, Transform&>;

    class RenderContext {
    private:
        Buffer<Mesh*> _meshes;
        Buffer<Material*> _materials;
        Buffer<Transform*> _transforms;

        Camera* _mainCamera{};
        Transform* _mainCameraTransform{};

        DirectionalLight* _directionalLight{};
        Transform* _directionalLightTransform{};

        Buffer<PointLight*> _pointLights;
        Buffer<Transform*> _pointLightTransforms;

    public:
        void submit(Mesh& mesh, Material& material, Transform& transform) {
            _meshes.push_back(&mesh);
            _materials.push_back(&material);
            _transforms.push_back(&transform);
        }

        void submit(Camera& camera, Transform& cameraTransform) {
            _mainCamera = &camera;
            _mainCameraTransform = &cameraTransform;
        }

        void submit(DirectionalLight& light, Transform& lightTransform) {
            _directionalLight = &light;
            _directionalLightTransform = &lightTransform;
        }

        void submit(PointLight& light, Transform& lightTransform) {
            _pointLights.push_back(&light);
            _pointLightTransforms.push_back(&lightTransform);
        }

        RenderTarget item(size_t index) {
            return std::tie(*_meshes[index], *_materials[index], *_transforms[index]);
        }

        Buffer<Mesh*>& meshes() {
            return _meshes;
        }

        size_t itemCount() const {
            return _meshes.size();
        }

        RenderItem<Camera> camera() {
            return std::tie(*_mainCamera, *_mainCameraTransform);
        }

        RenderItem<DirectionalLight> directionalLight() {
            return std::tie(*_directionalLight, *_directionalLightTransform);
        }

        RenderItem<PointLight> pointLight(size_t index) {
            return std::tie(*_pointLights[index], *_pointLightTransforms[index]);
        }

        size_t pointLightCount() const {
            return _pointLights.size();
        }

        void clear() {
            _meshes.clear();
            _materials.clear();
            _transforms.clear();

            _pointLights.clear();
            _pointLightTransforms.clear();

            _mainCamera = nullptr;
            _mainCameraTransform = nullptr;
            _directionalLight = nullptr;
            _directionalLightTransform = nullptr;
        }
    };

}
