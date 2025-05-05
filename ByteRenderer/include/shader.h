#pragma once

#include "render_type.h"
#include "opengl_api.h"

namespace Byte {

    struct ShaderPath {
        Path vertex;
        Path fragment;
        Path geometry;
    };

    struct Shader {
    private:
        uint32_t _id{ 0 };

        ShaderPath _path;

        friend struct ShaderCompiler;

    public:
        Shader() = default;

        Shader(const Path& vertex, const Path& fragment, const Path& geometry = "")
            :_path{ vertex,fragment,geometry } {
        }

        ~Shader() = default;

        void bind() const {
            OpenGLAPI::Shader::bind(_id);
        }

        void unbind() const {
            OpenGLAPI::Shader::unbind();
        }

        template<typename Type>
        void uniform(const std::string& name, const Type& value) const {
            OpenGLAPI::Shader::uniform(_id, name, value);
        }

        template<typename Type>
        void uniform(const std::string& name, const Buffer<Type>& values) const {
            for (size_t i{}; i < values.size(); ++i) {
                OpenGLAPI::Shader::uniform(_id, name + "[" + std::to_string(i) + "]", values[i]);
            }
        }

        uint32_t id() const {
            return _id;
        }

        bool compiled() const {
            return _id != 0;
        }
    };

    struct ShaderCompiler {
        static void compile(Shader& shader) {
            uint32_t vertexShader{ compile(shader._path.vertex, ShaderType::VERTEX ) };
            uint32_t fragmentShader{ compile(shader._path.fragment, ShaderType::FRAGMENT ) };

            if (!shader._path.geometry.empty()) {
                uint32_t geometryShader{ compile(shader._path.geometry, ShaderType::GEOMETRY) };
                shader._id = createProgram(vertexShader, fragmentShader, geometryShader);
                OpenGLAPI::Shader::release(geometryShader);
            }
            else {
                shader._id = createProgram(vertexShader, fragmentShader);
            }

            OpenGLAPI::Shader::release(vertexShader);
            OpenGLAPI::Shader::release(fragmentShader);
        }

        static uint32_t compile(const Path& shaderPath, ShaderType shaderType) {
            return OpenGLAPI::Shader::compile(shaderPath, shaderType);
        }

        static uint32_t createProgram(
            uint32_t vertex, 
            uint32_t fragment, 
            uint32_t geometry = 0) {
            return OpenGLAPI::Program::build(vertex, fragment, geometry);
        }
    };

}
