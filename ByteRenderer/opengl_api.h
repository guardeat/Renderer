#pragma once

#include <cstdint>
#include <fstream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "stb_image.h"

#include "window.h"
#include "mat.h"
#include "vec.h"
#include "quaternion.h"
#include "typedefs.h"

namespace Byte {

    class OpenglAPI {
    public:
        static void initialize(Window& window) {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

            glfwMakeContextCurrent(window.glfwWindow);
            glfwSetFramebufferSizeCallback(window.glfwWindow, OpenglAPI::framebufferSizeCallback);

            if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
                throw std::exception{ "GLAD cannot be loaded" };
            }

            glEnable(GL_DEPTH_TEST);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_set_flip_vertically_on_load(true);
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

        struct Framebuffer {
            static FramebufferData build(const FramebufferConfig& config) {
                FramebufferID frameBufferID;
                glGenFramebuffers(1, &frameBufferID);
                glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);

                GLint glWidth{ static_cast<GLint>(config.width) };
                GLint glHeight{ static_cast<GLint>(config.height) };

                FramebufferData::TextureMap textures;

                Buffer<uint32_t> attachments;

                for (auto& att : config.attachments) {
                    TextureID id{Texture::build(config.width,config.height,nullptr, att.internalFormat,att.format,att.type)};
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + att.index, GL_TEXTURE_2D, id, 0);
                    textures[att.tag] = id;
                    attachments.push_back(GL_COLOR_ATTACHMENT0 + att.index);
                }

                if (attachments.size() > 1) {
                    glDrawBuffers(static_cast<GLsizei>(attachments.size()), attachments.data());
                }

                unsigned int rboDepth;
                glGenRenderbuffers(1, &rboDepth);
                glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, glWidth, glHeight);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

                if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                    throw std::exception("Framebuffer not complete");
                }

                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                return FramebufferData{ frameBufferID, textures, attachments};
            }

            static void clear(FramebufferID id) {
                glBindFramebuffer(GL_FRAMEBUFFER, id);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            }

            static void bind(FramebufferData& data) {
                glBindFramebuffer(GL_FRAMEBUFFER, data.id);
                glDrawBuffers(static_cast<GLsizei>(data.attachments.size()), data.attachments.data());
            }

            static void unbind() {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }

            static void release(FramebufferData& data) {
                glDeleteBuffers(1, &data.id);

                for (auto& pair : data.textures) {
                    if (pair.second) {
                        Texture::release(pair.second);
                    }
                }
            }

            static void blitDepth(FramebufferID source, FramebufferID dest, size_t width, size_t height) {
                glBindFramebuffer(GL_READ_FRAMEBUFFER, dest);
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, source);

                GLint glHeight{ static_cast<GLint>(height) };
                GLint glWidth{ static_cast<GLint>(width) };
                glBlitFramebuffer(0, 0, glWidth, glHeight, 0, 0, glWidth, glHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }

            static void blit(
                FramebufferID src,
                FramebufferID dest, 
                size_t width, 
                size_t height, 
                GLenum srcAtt, 
                GLenum destAtt) {
                GLint glWidth{ static_cast<GLint>(width) };
                GLint glHeight{ static_cast<GLint>(height) };

                glBindFramebuffer(GL_READ_FRAMEBUFFER, src);
                glReadBuffer(srcAtt);

                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest);
                glDrawBuffer(destAtt);

                glBlitFramebuffer(
                    0, 0, glWidth, glHeight,  
                    0, 0, glWidth, glHeight, 
                    GL_COLOR_BUFFER_BIT,       
                    GL_NEAREST                
                );

                glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            }

        };

        struct Draw {
            static void elements(size_t size) {
                glDrawElements(GL_TRIANGLES, static_cast<GLint>(size), GL_UNSIGNED_INT, 0);
            }

            static void instancedElements(size_t size, size_t instanceCount) {
                GLint glSize{ static_cast<GLint>(size) };
                GLsizei glCount{ static_cast<GLsizei>(instanceCount) };
                glDrawElementsInstanced(GL_TRIANGLES, glSize, GL_UNSIGNED_INT, 0, glCount);
            }

            static void quad() {
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

                glBindVertexArray(0);
            }
        };

        struct RArray {
            static RArrayData build(
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

                return RArrayData{ VAO, attributes,{VBO}, EBO, indices.size(), isStatic };
            }

            static RArrayData build(
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

                return RArrayData{ VAO, attributes,{VBO,iVBO}, EBO, indices.size(), isStatic };
            }

            static void fillArray(Buffer<float>& data, bool isStatic) {
                auto draw{ isStatic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW };
                glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), draw);
            }

            static void release(RArrayData& renderArrayData) {
                glDeleteVertexArrays(1, &renderArrayData.VAO);

                for (const auto& id : renderArrayData.vertexBuffers) {
                    glDeleteBuffers(1, &id);
                }

                glDeleteBuffers(1, &renderArrayData.EBO);
            }

            static void bind(RArrayID id) {
                glBindVertexArray(id);
            }

            static void unbind() {
                glBindVertexArray(0);
            }

            static void bufferData(RBufferID id, Buffer<float>& data, size_t size, bool isStatic) {
                auto draw{ isStatic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW };

                glBindBuffer(GL_ARRAY_BUFFER, id);
                glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), data.data(), draw);
            }

            static void subBufferData(RBufferID id, Buffer<float>& data, size_t offset = 0) {
                glBindBuffer(GL_ARRAY_BUFFER, id);
                glBufferSubData(GL_ARRAY_BUFFER, offset, data.size() * sizeof(float), data.data());
            }
        };

        struct Program {
            static void release(uint32_t id) {
                glDeleteProgram(id);
            }

            static uint32_t build(uint32_t vertexShader, uint32_t fragmentShader) {
                uint32_t id;

                id = glCreateProgram();
                glAttachShader(id, vertexShader);
                glAttachShader(id, fragmentShader);
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

            static uint32_t compile(const std::string& shaderPath, GLenum shaderType) {
                std::string shaderCode;
                std::ifstream shaderFile;

                shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

                shaderFile.open(shaderPath);
                std::stringstream shaderStream;

                shaderStream << shaderFile.rdbuf();

                shaderFile.close();

                shaderCode = shaderStream.str();

                const char* sCode{ shaderCode.c_str() };

                uint32_t id;

                id = glCreateShader(shaderType);
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
            static TextureID build(
                size_t width, 
                size_t height, 
                const uint8_t* data = nullptr, 
                GLenum internalFormat = GL_RGBA, 
                GLenum format = GL_RGBA, 
                GLenum type = GL_UNSIGNED_BYTE) {
                TextureID textureID;

                GLint glWidth{ static_cast<GLint>(width) };
                GLint glHeight{ static_cast<GLint>(height) };

                glGenTextures(1, &textureID);

                glBindTexture(GL_TEXTURE_2D, textureID);
 
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);     
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);     

                glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, glWidth, glHeight, 0, format, type, data);

                glGenerateMipmap(GL_TEXTURE_2D);

                glBindTexture(GL_TEXTURE_2D, 0);

                return textureID;
            }

            static void bind(TextureID textureID, GLenum textureUnit = GL_TEXTURE0) {
                glActiveTexture(textureUnit);
                glBindTexture(GL_TEXTURE_2D, textureID);
            }

            static void unbind() {
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            static void release(TextureID textureID) {
                glDeleteTextures(1, &textureID);
            }
        };
        
	};

}