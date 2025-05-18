#version 410 core

layout (location = 0) out vec3 gNormal;
layout (location = 1) out vec3 gAlbedo;
layout (location = 2) out vec4 gMaterial;

in vec3 vNormal;
in vec2 vTexCoord;
in vec3 vFragPos;

uniform vec4 uAlbedo;
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

    vec4 sampledAlbedo = vec4(1.0);
    vec4 sampledMaterial = vec4(1.0);

    bool useAlbedoTexture = (uDataMode == 1 || uDataMode == 3);

    if (useAlbedoTexture) {
        sampledAlbedo = texture(uAlbedoTexture, vTexCoord);
        if (sampledAlbedo.a == 0.0) {
            discard;
        }
    } else {
        if (uAlbedo.a == 0.0) {
            discard;
        }
    }

    if (uDataMode == 0) {
        gAlbedo = uAlbedo.rgb;
        gMaterial = vec4(uMetallic, uRoughness, uAO, uEmission);
    } else if (uDataMode == 1) {
        gAlbedo = sampledAlbedo.rgb * uAlbedo.rgb;
        gMaterial = vec4(uMetallic, uRoughness, uAO, uEmission);
    } else if (uDataMode == 2) {
        gAlbedo = uAlbedo.rgb;
        gMaterial = sampledMaterial;
    } else if (uDataMode == 3) {
        gAlbedo = sampledAlbedo.rgb * uAlbedo.rgb;
        gMaterial = sampledMaterial;
    } else {
        gAlbedo = vec3(1.0);
        gMaterial = vec4(1.0);
    }
}
