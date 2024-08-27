#pragma once

#include <unordered_map>

#include "texture.h"
#include "g_buffer.h"
#include "shader.h"
#include "mesh.h"

namespace Byte {

    struct RenderData {
        size_t height{ 0 };
        size_t width{ 0 };

        GBuffer gBuffer{};
        Texture shadowMap;

        using ShaderMap = std::unordered_map<ShaderTag, Shader>;
        ShaderMap shaderMap;

        Mesh quad{
        Buffer<float>{
            -1.0f,  1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
             1.0f,  1.0f, 0.0f,
             1.0f, -1.0f, 0.0f
            },
        Buffer<float>{},    
        Buffer<float>{
            0.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 1.0f,
            1.0f, 0.0f
            },          
        Buffer<uint32_t>{
            0, 1, 2,  
            1, 3, 2   
            }
        };
    };

}