#version 330 core

layout (location = 0) out vec3 gFragPosition;
layout (location = 1) out vec3 gFragNormal;
layout (location = 2) out vec4 gAlbedoSpecular;

in vec3 vNormal;
in vec2 vTexCoord;
in vec3 vFragPos;

uniform vec4 uAlbedo;

uniform sampler2D uDiffuseTexture;
uniform sampler2D uSpecularTexture;

void main()
{    
    gFragPosition = vFragPos;
    gFragNormal = normalize(vNormal);
    //gAlbedoSpecular.rgb = texture(uDiffuseTexture, vTexCoord).rgb;
    //gAlbedoSpecular.a = texture(uSpecularTexture, vTexCoord).r;

    gAlbedoSpecular = uAlbedo;
}