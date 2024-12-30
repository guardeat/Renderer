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

void main()
{    
    gNormal = normalize(vNormal);
    gAlbedo = uAlbedo;
    gMaterial = vec4(uMetallic,uRoughness,uAO,uEmission);
}