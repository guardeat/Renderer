#pragma once

#include <cstdint>
#include <fstream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "stb_image.h"

#include "window.h"
#include "texture_data.h"
#include "mesh_geometry.h"
#include "render_array_data.h"
#include "g_buffer_data.h"
#include "mat.h"
#include "vec.h"
#include "quaternion.h"

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

        static void clearBuffer() {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
            glViewport(0, 0, width, height);
        }

        static void bindDefaultBuffer() {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        static void drawElements(size_t size) {
            glDrawElements(GL_TRIANGLES, static_cast<GLint>(size), GL_UNSIGNED_INT, 0);
        }
            
        static void drawQuad() {
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            glBindVertexArray(0);
        }
      
        static RArrayData buildRenderArray(MeshGeometry& geometry, bool isStatic) {
            uint32_t VAO, VBO, NBO, UVBO, UV2BO, EBO;

            auto draw = isStatic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;

            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);

            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, geometry.position.size() * sizeof(float), geometry.position.data(), draw);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            glGenBuffers(1, &NBO);
            glBindBuffer(GL_ARRAY_BUFFER, NBO);
            glBufferData(GL_ARRAY_BUFFER, geometry.normal.size() * sizeof(float), geometry.normal.data(), draw);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);

            glGenBuffers(1, &UVBO);
            glBindBuffer(GL_ARRAY_BUFFER, UVBO);
            glBufferData(GL_ARRAY_BUFFER, geometry.uv1.size() * sizeof(float), geometry.uv1.data(), draw);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(2);

            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, geometry.index.size() * sizeof(uint32_t), geometry.index.data(), draw);

            if (!geometry.uv2.empty()) {
                glGenBuffers(1, &UV2BO);
                glBindBuffer(GL_ARRAY_BUFFER, UV2BO);
                glBufferData(GL_ARRAY_BUFFER, geometry.uv2.size() * sizeof(float), geometry.uv2.data(), draw);
                glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(3);
            }
            else {
                UV2BO = 0;
            }

            glBindVertexArray(0);

            return RArrayData{ VAO, VBO, NBO, UVBO, EBO, UV2BO };
        }

        static RArrayData buildQuad(const MeshGeometry& geometry) {
            uint32_t VAO, VBO, UVBO, EBO;

            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);

            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(
                GL_ARRAY_BUFFER,
                geometry.position.size() * sizeof(float),
                geometry.position.data(),
                GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            glGenBuffers(1, &UVBO);
            glBindBuffer(GL_ARRAY_BUFFER, UVBO);
            glBufferData(
                GL_ARRAY_BUFFER,
                geometry.uv1.size() * sizeof(float),
                geometry.uv1.data(),
                GL_STATIC_DRAW);

            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);

            glGenBuffers(1, &EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(
                GL_ELEMENT_ARRAY_BUFFER, 
                geometry.index.size() * sizeof(uint32_t), 
                geometry.index.data(), 
                GL_STATIC_DRAW);

            glBindVertexArray(0);

            return RArrayData{ VAO, VBO, 0, UVBO, EBO, 0 };
        }

        static void deleteRenderArray(RArrayData& renderArraydata) {
            glDeleteVertexArrays(1, &renderArraydata.VAO);
            glDeleteBuffers(1, &renderArraydata.VBO);
            glDeleteBuffers(1, &renderArraydata.NBO);
            glDeleteBuffers(1, &renderArraydata.UVBO);
            glDeleteBuffers(1, &renderArraydata.EBO);
            glDeleteBuffers(1, &renderArraydata.UV2BO);
        }

        static void bindRenderArray(RArrayID id) {
            glBindVertexArray(id);
        }

        static void unbindRenderArray() {
            glBindVertexArray(0);
        }
        

        static void deleteProgram(uint32_t id) {
            glDeleteProgram(id);
        }

        static void deleteShader(uint32_t id) {
            glDeleteShader(id);
        }

        static void bindProgram(uint32_t id) {
            glUseProgram(id);
        }

        static void unbindProgram() {
            glUseProgram(0);
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
               
            const char* sCode = shaderCode.c_str();

            uint32_t id;

            id = glCreateShader(shaderType);
            glShaderSource(id, 1, &sCode, NULL);
            glCompileShader(id);
            checkShaderErrors(id);

            return id;
        }

        static uint32_t createProgram(uint32_t vertexShader, uint32_t fragmentShader) {
            uint32_t id;

            id = glCreateProgram();
            glAttachShader(id, vertexShader);
            glAttachShader(id, fragmentShader);
            glLinkProgram(id);
            checkProgramErrors(id);

            return id;
        }

        static void checkShaderErrors(uint32_t shader) {
            int success;
            char infoLog[1024];

            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                throw std::exception{ infoLog };
            }
        }

        static void checkProgramErrors(uint32_t program) {
            int success;
            char infoLog[1024];

            glGetProgramiv(program, GL_LINK_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(program, 1024, NULL, infoLog);
                throw std::exception{ infoLog };
            }

        }

        static GBufferData buildGbuffer(size_t width, size_t height) {
            unsigned int gBuffer;
            glGenFramebuffers(1, &gBuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
            unsigned int gPosition, gNormal, gAlbedoSpec;

            GLint glWidth = static_cast<GLint>(width);
            GLint glHeight = static_cast<GLint>(height);

            glGenTextures(1, &gPosition);
            glBindTexture(GL_TEXTURE_2D, gPosition);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, glWidth, glHeight, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

            glGenTextures(1, &gNormal);
            glBindTexture(GL_TEXTURE_2D, gNormal);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, glWidth, glHeight, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

            glGenTextures(1, &gAlbedoSpec);
            glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glWidth, glHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

            unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
            glDrawBuffers(3, attachments);

            unsigned int rboDepth;
            glGenRenderbuffers(1, &rboDepth);
            glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, glWidth, glHeight);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
  
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                std::cout << "Framebuffer not complete!" << std::endl;

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            return GBufferData{ gBuffer,gPosition, gNormal, gAlbedoSpec };
        }

        static void deleteGbuffer(GBufferData& gBufferData) {
            glDeleteFramebuffers(1, &gBufferData.id);
            glDeleteTextures(1, &gBufferData.position);
            glDeleteTextures(1, &gBufferData.normal);
            glDeleteTextures(1, &gBufferData.albedoSpecular);
        }

        static void clearBuffer(GBufferData& gBufferData) {
            bindGBuffer(gBufferData);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        static void bindGBuffer(GBufferData& gBufferData) {
            glBindFramebuffer(GL_FRAMEBUFFER, gBufferData.id);
        }

        static void unbindGBuffer() {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        static void updateDepth(const GBufferData& data, size_t width, size_t height) {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, data.id);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

            GLint glHeight{ static_cast<GLint>(height) };
            GLint glWidth{ static_cast<GLint>(width) };
            glBlitFramebuffer(0, 0, glWidth, glHeight, 0, 0, glWidth, glHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
     
        static void bindTexture(unsigned int textureID, GLenum textureUnit = GL_TEXTURE0) {
            glActiveTexture(textureUnit);
            glBindTexture(GL_TEXTURE_2D, textureID);
        }

        static void unbindTexture() {
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        static void deleteTexture(unsigned int textureID) {
            glDeleteTextures(1, &textureID);
        }
        
	};

}