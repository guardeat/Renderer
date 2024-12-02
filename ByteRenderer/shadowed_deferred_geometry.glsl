#version 330 core

layout (location = 0) out vec3 gFragPosition;
layout (location = 1) out vec4 gFragNormal;
layout (location = 2) out vec4 gAlbedoSpecular;

in vec3 vNormal;
in vec2 vTexCoord;
in vec3 vFragPos;
in vec4 vFragLightSpacePos;

uniform vec4 uAlbedo;
uniform vec3 uLightDir;

uniform sampler2D uDepthMap;

float calculateShadow(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.x < 0.0 || projCoords.x > 1.0 || 
        projCoords.y < 0.0 || projCoords.y > 1.0 || 
        projCoords.z < 0.0 || projCoords.z > 1.0) {
        return 0.0; 
    }

    float closestDepth = texture(uDepthMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;

    float bias = 0.005;
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}

void main()
{    
    gFragPosition = vFragPos;
    gFragNormal.xyz = normalize(vNormal);
    gFragNormal.w = calculateShadow(vFragLightSpacePos);

    gAlbedoSpecular = uAlbedo;
}