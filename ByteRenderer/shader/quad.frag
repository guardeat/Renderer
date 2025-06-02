#version 410 core

in vec2 vTexCoord;

out vec4 oFragColor;

uniform sampler2D uAlbedo;
uniform sampler2D uDepth;

uniform float uGamma;
uniform float uNear;
uniform float uFar;
uniform vec3 uFogColor;
uniform float uFogNear;
uniform float uFogFar;

float linearizeDepth(float depth, float near, float far) {
    float z = depth * 2.0 - 1.0; 
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main() {
    vec3 albedo = texture(uAlbedo, vTexCoord).rgb;
    vec3 mapped = albedo / (albedo + vec3(1.0));
    vec3 gammaCorrected = pow(mapped, vec3(1.0 / uGamma));

    float depthSample = texture(uDepth, vTexCoord).r;

    if (depthSample == 1) {
        oFragColor = vec4(gammaCorrected, 1.0);
        return;
    }

    float depth = linearizeDepth(depthSample, uNear, uFar);

    float fogFactor = clamp((uFogFar - depth) / (uFogFar - uFogNear), 0.0, 1.0);

    vec3 finalColor = mix(uFogColor, gammaCorrected, fogFactor);

    oFragColor = vec4(finalColor, 1.0);
}
