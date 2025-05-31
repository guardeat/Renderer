#pragma once

#include <tuple>
#include <unordered_map>
#include <limits>
#include <variant>

#include "render_type.h"
#include "mesh.h"
#include "material.h"
#include "transform.h"
#include "mat.h"
#include "framebuffer.h"
#include "light.h"
#include "camera.h"
#include "instance_group.h"
#include "shader.h"

namespace Byte {

    class RenderContext {
    public:
        struct RenderEntity {
            Mesh* mesh;
            Material* material;
            Transform* transform;
            RenderMode mode{ RenderMode::ENABLED };
        };

        template<typename Type>
        struct RenderItem {
            Type* item{};
            Transform* transform{};
            RenderMode mode{ RenderMode::ENABLED };
        };

    private:
        using EntityMap = std::unordered_map<RenderID, RenderEntity>;
        EntityMap _renderEntities;

        RenderID _cameraID{};
        RenderItem<Camera> _camera{};

        RenderID _directionalLightID{};
        RenderItem<DirectionalLight> _directionalLight{};

        using PointLightMap = std::unordered_map<RenderID, RenderItem<PointLight>>;
        PointLightMap _pointLights;

        using InstanceMap = std::unordered_map<InstanceTag, InstanceGroup>;
        InstanceMap _instances;

        ShaderInputMap _inputMap;

    public:
        RenderID submit(Mesh& mesh, Material& material, Transform& transform) {
            RenderID id{ RenderIDGenerator::generate() };
            _renderEntities.emplace(id, RenderEntity{ &mesh, &material, &transform });
            return id;
        }

        RenderID submit(const InstanceTag& tag, Mesh& mesh, Material& material, Transform& transform) {
            _instances.emplace(tag, InstanceGroup{ mesh,material });
            RenderID id{ RenderIDGenerator::generate() };
            _instances.at(tag).add(transform,id);
            return id;
        }

        RenderID submit(const InstanceTag& tag, Transform& transform) {
            RenderID id{ RenderIDGenerator::generate() };
            _instances.at(tag).add(transform, id);
            return id;
        }

        RenderID submit(Camera& camera, Transform& cameraTransform) {
            RenderID id{ RenderIDGenerator::generate() };
            _cameraID = id;
            _camera = { &camera, &cameraTransform };
            return id;
        }

        RenderID submit(DirectionalLight& light, Transform& lightTransform) {
            RenderID id{ RenderIDGenerator::generate() };
            _directionalLightID = id;
            _directionalLight = { &light, &lightTransform };
            return id;
        }

        RenderID submit(PointLight& light, Transform& lightTransform) {
            RenderID id{ RenderIDGenerator::generate() };
            _pointLights.emplace(id, RenderItem<PointLight>{ &light, &lightTransform });
            return id;
        }

        template<typename Type>
        Type& input(const UniformTag& tag) {
            return std::get<ShaderInput<Type>>(_inputMap.at(tag)).value;
        }

        template<typename Type>
        const Type& input(const UniformTag& tag) const {
            return std::get<ShaderInput<Type>>(_inputMap.at(tag)).value;
        }

        template<typename Type>
        void input(const UniformTag& tag, ShaderInput<Type>&& shaderInput) {
            _inputMap.emplace(tag, std::forward<ShaderInput<Type>>(shaderInput));
        }

        ShaderInputMap& shaderInputMap() {
            return _inputMap;
        }

        const ShaderInputMap& shaderInputMap() const {
            return _inputMap;
        }

        void eraseEntity(RenderID id) {
            _renderEntities.erase(id);
        }

        void eraseEntity(const InstanceTag& tag, RenderID id) {
            _instances.at(tag).erase(id);
        }

        void eraseInstance(const InstanceTag& tag) {
            _instances.erase(tag);
        }

        void eraseItem(RenderID id) {
            _pointLights.erase(id);
        }

        void createInstance(const InstanceTag& tag, Mesh& mesh, Material& material) {
            _instances.emplace(tag, InstanceGroup{ mesh,material });
        }

        RenderEntity& entity(RenderID id) {
            return _renderEntities.at(id);
        }

        const RenderEntity& entity(RenderID id) const {
            return _renderEntities.at(id);
        }

        RenderItem<Camera>& camera() {
            return _camera;
        }

        const RenderItem<Camera>& camera() const {
            return _camera;
        }

        RenderItem<DirectionalLight>& directionalLight() {
            return _directionalLight;
        }

        const RenderItem<PointLight>& pointLight(RenderID id) const {
            return _pointLights.at(id);
        }

        EntityMap& renderEntities() {
            return _renderEntities;
        }

        const EntityMap& renderEntities() const {
            return _renderEntities;
        }

        PointLightMap& pointLights() {
            return _pointLights;
        }

        const PointLightMap& pointLights() const {
            return _pointLights;
        }

        InstanceGroup& instance(const InstanceTag& tag) {
            return _instances.at(tag);
        }

        const InstanceGroup& instance(const InstanceTag& tag) const {
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
