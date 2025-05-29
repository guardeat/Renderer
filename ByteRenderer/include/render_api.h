#pragma once

#include <cstdint>
#include <fstream>
#include <algorithm>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "window.h"
#include "mat.h"
#include "vec.h"
#include "quaternion.h"
#include "render_type.h"

namespace Byte {

    class RenderAPI {
    public:
        static void initialize(Window& window) {
            static bool initialized{ false };

            glfwMakeContextCurrent(window.glfwWindow);
            glfwSetFramebufferSizeCallback(window.glfwWindow, RenderAPI::framebufferSizeCallback);

            if (!initialized) {
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

                if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
                    throw std::exception{ "GLAD cannot be loaded" };
                }

                glEnable(GL_DEPTH_TEST);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                initialized = true;
            }
        }

        static void update(Window& window) {
            glfwSwapBuffers(window.glfwWindow);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
            glViewport(0, 0, width, height);
        }

        static void enableDepth() {
            glEnable(GL_DEPTH_TEST);
        }

        static void disableDepth() {
            glDisable(GL_DEPTH_TEST);
        }

        static void enableBlend() {
            glEnable(GL_BLEND);
        }

        static void disableBlend() {
            glDisable(GL_BLEND);
        }

        static void setBlend(float sFactor, float dFactor) {
            float total = sFactor + dFactor;
            if (total == 0.0f) {
                total = 1.0f;
            }

            float sNorm{ sFactor / total };
            float dNorm{ dFactor / total };

            glBlendColor(sNorm, sNorm, sNorm, dNorm);
            glBlendFunc(GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR);
        }

        static void setBlend() {
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            glBlendEquation(GL_FUNC_ADD);
        }

        static void cullFront() {
            glCullFace(GL_FRONT);
        }

        static void cullBack() {
            glCullFace(GL_BACK);
        }

        static void enableCulling() {
            glEnable(GL_CULL_FACE);
        }

        static void disableCulling() {
            glDisable(GL_CULL_FACE);
        }

        static void patchParameter(size_t value) {
            glPatchParameteri(GL_PATCH_VERTICES, static_cast<GLint>(value));
        }

        static void viewPort(size_t width, size_t height, size_t x = 0, size_t y = 0) {
            GLint glWidth{ static_cast<GLint>(width) };
            GLint glHeight{ static_cast<GLint>(height) };
            GLint glX{ static_cast<GLint>(x) };
            GLint glY{ static_cast<GLint>(y) };

            glViewport(glX, glY, glWidth, glHeight);
        }

        static void clearColor(Vec3 color) {
            clearColor(Vec4{ color.x,color.y,color.z, 1.0f });
        }

        static void clearColor(Vec4 color) {
            glClearColor(color.x, color.y, color.z, color.w);
        }

        struct Framebuffer {
            static void build(FramebufferData& data) {
                glGenFramebuffers(1, &data.id);
                glBindFramebuffer(GL_FRAMEBUFFER, data.id);

                GLint glWidth{ static_cast<GLint>(data.width) };
                GLint glHeight{ static_cast<GLint>(data.height) };

                bool hasDepth{ false };

                for (auto& [tag,att] : data.textures) {
                    size_t width{ att.width ? att.width : data.width };
                    size_t height{ att.height ? att.height : data.height };

                    att.width = width;
                    att.height = height;

                    if (att.type == TextureType::TEXTURE_2D) {
                        Texture::build(att);
                        glFramebufferTexture2D(
                            GL_FRAMEBUFFER, TypeCast::convert(att.attachment), 
                            GL_TEXTURE_2D, att.id, 0);
                    }
                    else {
                        Texture::buildArray(att);
                        glFramebufferTexture(
                            GL_FRAMEBUFFER, TypeCast::convert(att.attachment),
                            att.id, 0);
                    }
 
                    if (att.attachment == AttachmentType::DEPTH) {
                        hasDepth = true;
                    }
                    else {
                        data.attachments.push_back(att.attachment);
                    }
                }

                std::sort(data.attachments.begin(), data.attachments.end(), [](AttachmentType a, AttachmentType b) {
                    return static_cast<uint8_t>(a) < static_cast<uint8_t>(b);
                });

                if (!hasDepth) {
                    unsigned int rboDepth;
                    glGenRenderbuffers(1, &rboDepth);
                    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
                    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, glWidth, glHeight);
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
                }

                if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                    release(data);
                    throw std::exception("Framebuffer not complete");
                }

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }

            static void clear(FramebufferID id) {
                glBindFramebuffer(GL_FRAMEBUFFER, id);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            }

            static void clearDepth(FramebufferID id) {
                glBindFramebuffer(GL_FRAMEBUFFER, id);
                glClear(GL_DEPTH_BUFFER_BIT);
            }

            static void bind(FramebufferData& data) {
                glBindFramebuffer(GL_FRAMEBUFFER, data.id);

                if (!data.attachments.empty()) {
                    Buffer<uint32_t> attachments;
                    for (auto& att : data.attachments) {
                        attachments.push_back(TypeCast::convert(att));
                    }
                    glDrawBuffers(static_cast<GLsizei>(attachments.size()), attachments.data());
                }

                glViewport(0,0,static_cast<GLsizei>(data.width),static_cast<GLsizei>(data.height));
            }

            static void unbind() {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }

            static void release(FramebufferData& data) {
                glDeleteBuffers(1, &data.id);

                data.id = 0;

                for (auto& pair : data.textures) {
                    if (pair.second.id) {
                        Texture::release(pair.second.id);
                        pair.second.id = 0;
                    }
                }
            }

        };

        struct Draw {
            static void elements(size_t size) {
                glDrawElements(GL_TRIANGLES, static_cast<GLint>(size), GL_UNSIGNED_INT, 0);
            }

            static void instancedElements(size_t size, size_t instanceCount) {
                if (instanceCount) {
                    GLint glSize{ static_cast<GLint>(size) };
                    GLsizei glCount{ static_cast<GLsizei>(instanceCount) };
                    glDrawElementsInstanced(GL_TRIANGLES, glSize, GL_UNSIGNED_INT, 0, glCount);
                }
            }

            static void quad() {
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

                glBindVertexArray(0);
            }
        };

        struct RenderArray {
            static RenderArrayData build(
                const Buffer<float>& vertices,
                const Buffer<uint32_t>& indices,
                Buffer<VertexAttribute>& attributes, 
                bool isStatic) {
                uint32_t VAO, VBO, EBO;

                auto draw{ isStatic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW };

                glGenVertexArrays(1, &VAO);
                glBindVertexArray(VAO);

                glGenBuffers(1, &VBO);
                glBindBuffer(GL_ARRAY_BUFFER, VBO);

                glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), draw);

                uint32_t vertexStride{};

                for (auto& attribute : attributes) {
                    vertexStride += attribute.size * attribute.stride;
                }

                for (auto& attribute : attributes) {
                    attribute.bufferID = VBO;
                    glVertexAttribPointer(
                        attribute.index,
                        attribute.stride,
                        attribute.type, 
                        attribute.normalized, 
                        vertexStride, 
                        (void*)attribute.offset);
                    glEnableVertexAttribArray(attribute.index);
                }

                glGenBuffers(1, &EBO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), draw);

                glBindVertexArray(0);

                Buffer<RBufferData> buffers{ RBufferData{VBO,attributes} };

                return RenderArrayData{ VAO, buffers, EBO, indices.size() };
            }

            static RenderArrayData build(
                const Buffer<float>& vertices,
                const Buffer<uint32_t>& indices,
                Buffer<VertexAttribute>& attributes,
                Buffer<VertexAttribute>& instanceAttributes,
                bool isStatic) {

                uint32_t VAO, VBO, EBO, iVBO;

                auto draw{ isStatic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW };

                glGenVertexArrays(1, &VAO);
                glBindVertexArray(VAO);

                glGenBuffers(1, &VBO);
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), draw);

                uint32_t vertexStride{};
                for (auto& attribute : attributes) {
                    vertexStride += attribute.size * attribute.stride;
                }

                for (auto& attribute : attributes) {
                    attribute.bufferID = VBO;
                    glVertexAttribPointer(
                        attribute.index,
                        attribute.stride,
                        attribute.type,
                        attribute.normalized,
                        vertexStride,
                        (void*)attribute.offset);
                    glEnableVertexAttribArray(attribute.index);
                }

                glGenBuffers(1, &EBO);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), draw);

                glGenBuffers(1, &iVBO);
                glBindBuffer(GL_ARRAY_BUFFER, iVBO);

                uint32_t instanceStride{};
                for (const auto& attribute : instanceAttributes) {
                    instanceStride += attribute.size * attribute.stride;
                }

                for (const auto& attribute : instanceAttributes) {
                    glVertexAttribPointer(
                        attribute.index,
                        attribute.stride,
                        attribute.type,
                        attribute.normalized,
                        instanceStride,
                        (void*)attribute.offset);
                    glEnableVertexAttribArray(attribute.index);
                    glVertexAttribDivisor(attribute.index, 1);
                }
                glBindVertexArray(0);

                Buffer<RBufferData> buffers{ RBufferData{VBO,attributes}, RBufferData{iVBO,attributes} };

                return RenderArrayData{ VAO, buffers, EBO, indices.size() };
            }

            static Buffer<VertexAttribute> buildAttributes(const Buffer<uint8_t>& layout, uint8_t indexOffset = 0) {
                uint16_t stride{ 0 };

                Buffer<VertexAttribute> atts;

                for (uint8_t index{ 0 }; index < layout.size(); ++index) {
                    uint16_t aLayout{ layout[index] };
                    uint16_t strideSize{ static_cast<uint16_t>(stride * sizeof(float)) };
                    uint8_t i{ static_cast<uint8_t>(index + indexOffset) };

                    atts.push_back(VertexAttribute{ 0, sizeof(float), GL_FLOAT,strideSize,aLayout, i,false });

                    stride += aLayout;
                }

                return atts;
            }

            static void fillArray(Buffer<float>& data, bool isStatic) {
                auto draw{ isStatic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW };
                glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), draw);
            }

            static void release(RenderArrayData& renderArrayData) {
                if (renderArrayData.VAO != 0) {
                    glDeleteVertexArrays(1, &renderArrayData.VAO);

                    for (const auto& VBuffer : renderArrayData.VBuffers) {
                        glDeleteBuffers(1, &VBuffer.id);
                    }

                    glDeleteBuffers(1, &renderArrayData.EBO);
                }
            }

            static void bind(RenderArrayID id) {
                glBindVertexArray(id);
            }

            static void unbind() {
                glBindVertexArray(0);
            }

            static void bufferData(RenderBufferID id, Buffer<float>& data, size_t size, bool isStatic) {
                auto draw{ isStatic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW };

                glBindBuffer(GL_ARRAY_BUFFER, id);
                glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), data.data(), draw);
            }

            static void subBufferData(RenderBufferID id, Buffer<float>& data, size_t offset = 0) {
                glBindBuffer(GL_ARRAY_BUFFER, id);
                glBufferSubData(GL_ARRAY_BUFFER, offset, data.size() * sizeof(float), data.data());
            }
        };

        struct Program {
            static void release(uint32_t id) {
                glDeleteProgram(id);
            }

            static uint32_t build(uint32_t vertex, uint32_t fragment, uint32_t geometry = 0) {
                uint32_t id{ glCreateProgram() };

                glAttachShader(id, vertex);
                glAttachShader(id, fragment);

                if (geometry) {
                    glAttachShader(id, geometry);
                }

                glLinkProgram(id);
                check(id);

                return id;
            }

            static void check(uint32_t program) {
                int success;
                char infoLog[1024];

                glGetProgramiv(program, GL_LINK_STATUS, &success);
                if (!success) {
                    glGetShaderInfoLog(program, 1024, NULL, infoLog);
                    throw std::exception{ infoLog };
                }

            }
        };

        struct Shader {
            static void bind(uint32_t id) {
                glUseProgram(id);
            }

            static void unbind() {
                glUseProgram(0);
            }

            static void release(uint32_t id) {
                glDeleteShader(id);
            }

            template<typename Type>
            static void uniform(uint32_t id, const std::string& name, const Type& value) {
                throw std::exception("No such uniform type");
            }

            template<>
            static void uniform<bool>(uint32_t id, const std::string& name, const bool& value) {
                glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
            }

            template<>
            static void uniform<int>(uint32_t id, const std::string& name, const int& value) {
                glUniform1i(glGetUniformLocation(id, name.c_str()), value);
            }

            template<>
            static void uniform<size_t>(uint32_t id, const std::string& name, const size_t& value) {
                glUniform1i(glGetUniformLocation(id, name.c_str()), static_cast<GLint>(value));
            }

            template<>
            static void uniform<float>(uint32_t id, const std::string& name, const float& value) {
                glUniform1f(glGetUniformLocation(id, name.c_str()), value);
            }

            template<>
            static void uniform<Vec2>(uint32_t id, const std::string& name, const Vec2& value) {
                glUniform2f(glGetUniformLocation(id, name.c_str()), value.x, value.y);
            }

            template<>
            static void uniform<Vec3>(uint32_t id, const std::string& name, const Vec3& value) {
                auto loc{ glGetUniformLocation(id, name.c_str()) };
                glUniform3f(glGetUniformLocation(id, name.c_str()), value.x, value.y, value.z);
            }

            template<>
            static void uniform<Vec4>(uint32_t id, const std::string& name, const Vec4& value) {
                glUniform4f(glGetUniformLocation(id, name.c_str()), value.x, value.y, value.z, value.w);
            }

            template<>
            static void uniform<Quaternion>(uint32_t id, const std::string& name, const Quaternion& value) {
                glUniform4f(glGetUniformLocation(id, name.c_str()), value.x, value.y, value.z, value.w);
            }

            template<>
            static void uniform<Mat2>(uint32_t id, const std::string& name, const Mat2& value) {
                glUniformMatrix2fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, value.data);
            }

            template<>
            static void uniform<Mat3>(uint32_t id, const std::string& name, const Mat3& value) {
                glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, value.data);
            }

            template<>
            static void uniform<Mat4>(uint32_t id, const std::string& name, const Mat4& value) {
                glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, value.data);
            }

            static uint32_t compile(const Path& shaderPath, ShaderType shaderType) {
                std::string shaderCode;
                std::ifstream shaderFile;

                shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

                shaderFile.open(shaderPath);
                std::stringstream shaderStream;

                shaderStream << shaderFile.rdbuf();

                shaderFile.close();

                shaderCode = shaderStream.str();

                const char* sCode{ shaderCode.c_str() };

                uint32_t id{ glCreateShader(TypeCast::convert(shaderType)) };
                glShaderSource(id, 1, &sCode, NULL);
                glCompileShader(id);
                check(id);

                return id;
            }

            static void check(uint32_t shader) {
                int success;
                char infoLog[1024];

                glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
                if (!success) {
                    glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                    throw std::exception{ infoLog };
                }
            }

        };

        struct Texture {
            static void build(TextureData& data) {
                TextureID textureID;

                GLint glWidth{ static_cast<GLint>(data.width) };
                GLint glHeight{ static_cast<GLint>(data.height) };

                glGenTextures(1, &textureID);

                glBindTexture(GL_TEXTURE_2D, textureID);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TypeCast::convert(data.wrapS));
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TypeCast::convert(data.wrapT));
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TypeCast::convert(data.minFilter));
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TypeCast::convert(data.magFilter));

                uint8_t* textureData{data.data.empty() ? nullptr : data.data.data()};

                glTexImage2D(
                    GL_TEXTURE_2D, 0,
                    TypeCast::convert(data.internalFormat),
                    glWidth, glHeight, 0,
                    TypeCast::convert(data.format),
                    TypeCast::convert(data.dataType), textureData);

                glGenerateMipmap(GL_TEXTURE_2D);

                glBindTexture(GL_TEXTURE_2D, 0);

                data.id = textureID;
            }

            static TextureID buildArray(TextureData& data) {
                TextureID textureID;

                GLint glWidth{ static_cast<GLint>(data.width) };
                GLint glHeight{ static_cast<GLint>(data.height) };
                GLint glLayerCount{ static_cast<GLint>(data.layerCount) };

                glGenTextures(1, &textureID);

                glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);

                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, TypeCast::convert(data.wrapS));
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, TypeCast::convert(data.wrapT));
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, TypeCast::convert(data.minFilter));
                glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, TypeCast::convert(data.magFilter));

                uint8_t* textureData{ data.data.empty() ? nullptr : data.data.data() };

                glTexImage3D(
                    GL_TEXTURE_2D_ARRAY, 0,
                    TypeCast::convert(data.internalFormat),
                    glWidth, glHeight, glLayerCount, 0,
                    TypeCast::convert(data.format),
                    TypeCast::convert(data.dataType), textureData);

                glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

                return textureID;
            }

            static void bind(
                TextureID textureID, 
                TextureUnit unit = TextureUnit::T0,
                TextureType type = TextureType::TEXTURE_2D) {
                glActiveTexture(TypeCast::convert(unit));
                glBindTexture(TypeCast::convert(type), textureID);
            }

            static void unbind() {
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            static void release(TextureID textureID) {
                glDeleteTextures(1, &textureID);
            }
        };

        struct TypeCast {
            static GLenum convert(TextureUnit unit) {
                return static_cast<GLenum>(unit) + GL_TEXTURE0;
            }

            static GLenum convert(ShaderType type) {
                return static_cast<GLenum>(type);
            }

            static GLenum convert(DataType type) {
                return static_cast<GLenum>(type) + GL_BYTE;
            }

            static GLenum convert(ColorFormat format) {
                return static_cast<GLenum>(format);
            }

            static GLenum convert(AttachmentType type) {
                return static_cast<GLenum>(type) + GL_COLOR_ATTACHMENT0;
            }

            static GLenum convert(TextureType type) {
                return static_cast<GLenum>(type);
            }

            static GLenum convert(TextureFilter type) {
                return static_cast<GLenum>(type);
            }

            static GLenum convert(TextureWrap type) {
                return static_cast<GLenum>(type);
            }
        };

	};

}