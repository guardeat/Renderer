#version 410 core

in vec3 vNormal;
in vec2 vTexCoord;
in vec3 vFragPos;

out vec4 oFragColor;

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

    vec4 color;

    if (uDataMode == 0) {
        color = uAlbedo;
    } else if (uDataMode == 1) {
        color = sampledAlbedo * uAlbedo;
    } else if (uDataMode == 2) {
        color = uAlbedo;
    } else if (uDataMode == 3) {
        color = sampledAlbedo * uAlbedo;
    } else {
        color = vec4(1.0);
    }

    oFragColor = color;
}
