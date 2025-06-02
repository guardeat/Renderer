#version 410 core

layout (location = 0) out vec3 oNormal;
layout (location = 1) out vec3 oAlbedo;
layout (location = 2) out vec4 oMaterial;

in vec3 vNormal;
in vec2 vTexCoord;

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
    oNormal = normalize(vNormal);

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
        oAlbedo = uAlbedo.rgb;
        oMaterial = vec4(uMetallic, uRoughness, uAO, uEmission);
    } else if (uDataMode == 1) {
        oAlbedo = sampledAlbedo.rgb * uAlbedo.rgb;
        oMaterial = vec4(uMetallic, uRoughness, uAO, uEmission);
    } else if (uDataMode == 2) {
        oAlbedo = uAlbedo.rgb;
        oMaterial = sampledMaterial;
    } else if (uDataMode == 3) {
        oAlbedo = sampledAlbedo.rgb * uAlbedo.rgb;
        oMaterial = sampledMaterial;
    } else {
        oAlbedo = vec3(1.0);
        oMaterial = vec4(1.0);
    }
}
