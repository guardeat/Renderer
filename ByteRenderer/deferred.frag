#version 330 core

layout (location = 0) out vec3 gNormal;
layout (location = 1) out vec4 gAlbedo;

in vec3 vNormal;
in vec2 vTexCoord;
in vec3 vFragPos;

uniform vec4 uAlbedo;

void main()
{    
    gNormal = normalize(vNormal);
    gAlbedo = uAlbedo;
}