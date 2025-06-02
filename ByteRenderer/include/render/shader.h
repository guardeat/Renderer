#pragma once

#include <string>
#include <type_traits>
#include <variant>
#include <cstdint>

#include "render_type.h"
#include "render_api.h"
#include "texture.h"

namespace Byte {

    struct Shader {
    private:
        uint32_t _id{ 0 };

        ShaderPath _path;

        using UniformVector = std::vector<Uniform>;
        UniformVector _uniforms;

        struct Binding {
            TextureTag tag;
            UniformTag uniformTag;
            TextureUnit unit;
        };

        using TextureBindingVector = std::vector<Binding>;
        TextureBindingVector _bindings;

        friend struct ShaderCompiler;

    public:
        Shader() = default;

        Shader(
            const Path& vertex, 
            const Path& fragment, 
            const Path& geometry = "",
            const Path& tessC = "", 
            const Path& tessE = "")
            :_path{ vertex,fragment,geometry,tessC,tessE } {
        }

        ~Shader() = default;

        void bind() const {
            RenderAPI::Shader::bind(_id);
        }

        void unbind() const {
            RenderAPI::Shader::unbind();
        }

        template<typename Type>
        void uniform(const std::string& name, const Type& value) const {
            RenderAPI::Shader::uniform(_id, name, value);
        }

        template<typename Type>
        void uniform(const std::string& name, const Buffer<Type>& values) const {
            for (size_t i{}; i < values.size(); ++i) {
                RenderAPI::Shader::uniform(_id, name + "[" + std::to_string(i) + "]", values[i]);
            }
        }

        void uniform(const ShaderInputMap& inputs) {
            for (const auto& [tag, input] : inputs) {
                std::visit([this, &tag](const auto& inputValue) {
                    RenderAPI::Shader::uniform(_id, tag, inputValue.value);
                }, input); 
            }
        }

        void uniform(const Material& material) {
            if (!material.hasTexture("albedo") && !material.hasTexture("material")) {
                uniform<int>("uDataMode", 0);
                uniform<float>("uMetallic", material.metallic());
                uniform<float>("uRoughness", material.roughness());
                uniform<float>("uAO", material.ambientOcclusion());
                uniform<float>("uEmission", material.emission());
            }

            else if (material.hasTexture("albedo") && !material.hasTexture("material")) {
                uniform<int>("uDataMode", 1);
                uniform<int>("uAlbedoTexture", 0);

                material.texture("albedo").bind();
                uniform<float>("uMetallic", material.metallic());
                uniform<float>("uRoughness", material.roughness());
                uniform<float>("uAO", material.ambientOcclusion());
                uniform<float>("uEmission", material.emission());
            }

            else if (!material.hasTexture("albedo") && material.hasTexture("material")) {
                uniform<int>("uDataMode", 2);
                uniform<int>("uMaterialTexture", 0);

                material.texture("material").bind();
            }

            else {
                uniform<int>("uDataMode", 3);
                uniform<int>("uAlbedoTexture", 0);
                uniform<int>("uMaterialTexture", 1);

                material.texture("albedo").bind();
                material.texture("material").bind(TextureUnit::T1);
            }

            uniform<Vec4>("uAlbedo", material.albedo());

            for (auto& [tag, uniformTag, unit] : _bindings) {
                material.texture(tag).bind();
                uniform<int>(uniformTag, static_cast<int>(unit));
            }
        }

        uint32_t id() const {
            return _id;
        }

        void include(Uniform&& input) {
            _uniforms.emplace_back(std::forward<Uniform>(input));
        }

        UniformVector& uniformVector() {
            return _uniforms;
        }

        const UniformVector& uniformVector() const {
            return _uniforms;
        }

        bool compiled() const {
            return _id != 0;
        }
    };

    struct ShaderCompiler {
        static void compile(Shader& shader) {
            uint32_t vertexShader{ compile(shader._path.vertex, ShaderType::VERTEX) };
            uint32_t fragmentShader{ compile(shader._path.fragment, ShaderType::FRAGMENT) };

            uint32_t geometryShader{ 0 };
            uint32_t tessCShader{ 0 };
            uint32_t tessEShader{ 0 };

            if (!shader._path.geometry.empty()) {
                geometryShader = compile(shader._path.geometry, ShaderType::GEOMETRY);
            }

            if (!shader._path.tessellationControl.empty()) {
                tessCShader = compile(shader._path.tessellationControl, ShaderType::TESSELLATION_CONTROL);
            }

            if (!shader._path.tessellationEvaluation.empty()) {
                tessEShader = compile(shader._path.tessellationEvaluation, ShaderType::TESSELLATION_EVALUATION);
            }

            shader._id = createProgram(vertexShader, fragmentShader, geometryShader, tessCShader, tessEShader);

            RenderAPI::Shader::release(vertexShader);
            RenderAPI::Shader::release(fragmentShader);
            if (geometryShader) {
                RenderAPI::Shader::release(geometryShader);
            }
            if (tessCShader) {
                RenderAPI::Shader::release(tessCShader);
            }
            if (tessEShader) {
                RenderAPI::Shader::release(tessEShader);
            }
        }

        static uint32_t compile(const Path& shaderPath, ShaderType shaderType) {
            return RenderAPI::Shader::compile(shaderPath, shaderType);
        }

        static uint32_t createProgram(
            uint32_t vertex, 
            uint32_t fragment, 
            uint32_t geometry = 0,
            uint32_t tessC = 0,
            uint32_t tessE = 0) {
            return RenderAPI::Program::build(vertex, fragment, geometry, tessC, tessE);
        }
    };

}
