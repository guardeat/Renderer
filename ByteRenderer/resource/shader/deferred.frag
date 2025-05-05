#version 330 core

layout (location = 0) out vec3 gNormal;
layout (location = 1) out vec3 gAlbedo;
layout (location = 2) out vec4 gMaterial;

in vec3 vNormal;
in vec2 vTexCoord;
in vec3 vFragPos;

uniform vec3 uAlbedo;
uniform float uMetallic;
uniform float uRoughness;
uniform float uAO;
uniform float uEmission;

uniform sampler2D uAlbedoTexture;
uniform sampler2D uMaterialTexture;

uniform int uDataMode;

void main()
{    
    gNormal = normalize(vNormal);

    switch(uDataMode) {
        case 0: 
            gAlbedo = uAlbedo;
            gMaterial = vec4(uMetallic,uRoughness,uAO,uEmission);
            break;
        case 1:
            gAlbedo = texture(uAlbedoTexture,vTexCoord).rgb * uAlbedo;
            gMaterial = vec4(uMetallic,uRoughness,uAO,uEmission);
            break;
        case 2:
             gAlbedo = uAlbedo;
             gMaterial = texture(uMaterialTexture,vTexCoord);
             break;
        case 3:
            gAlbedo = texture(uAlbedoTexture,vTexCoord).rgb * uAlbedo;
            gMaterial = texture(uMaterialTexture,vTexCoord);
            break;
        default:
            gAlbedo = vec3(1.0f);
            gMaterial = vec4(1.0f);
    }
}