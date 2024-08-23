#pragma once

#include <string>

#include "opengl_api.h"

namespace Byte {

    using ShaderTag = std::string;

    struct ShaderPath {
        std::string vertex;
        std::string fragment;
    };

    struct Shader {
    private:
        uint32_t _id{ 0 };

        ShaderPath path;

        friend struct ShaderCompiler;

    public:
        Shader() = default;

        Shader(const std::string& vertexPath, const std::string& fragmentPath)
            :path{ vertexPath,fragmentPath } {
        }

        ~Shader() = default;

        void bind() const {
            OpenglAPI::ShaderAPI::bind(_id);
        }

        void unbind() const {
            OpenglAPI::ShaderAPI::bind(0);
        }

        template<typename Type>
        void uniform(const std::string& name, const Type& value) const {
            OpenglAPI::ShaderAPI::uniform(_id, name, value);
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
            uint32_t vertexShader{ compile(shader.path.vertex, GL_VERTEX_SHADER) };
            uint32_t fragmentShader{ compile(shader.path.fragment, GL_FRAGMENT_SHADER) };

            shader._id = createProgram(vertexShader, fragmentShader);

            OpenglAPI::ShaderAPI::clearShader(vertexShader);
            OpenglAPI::ShaderAPI::clearShader(fragmentShader);
        }

        static uint32_t compile(const std::string& shaderPath, GLenum shaderType) {
            return OpenglAPI::ShaderAPI::compile(shaderPath, shaderType);
        }

        static uint32_t createProgram(uint32_t vertexShader, uint32_t fragmentShader) {
            return OpenglAPI::ShaderAPI::createProgram(vertexShader, fragmentShader);
        }
    };

}
