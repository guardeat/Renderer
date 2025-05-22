#version 410 core

in vec2 vTexCoord;

out vec4 oFragColor;

uniform sampler2D uAlbedo;
uniform float uGamma;

void main() {
    vec3 albedo = texture(uAlbedo, vTexCoord).rgb;

    vec3 mapped = albedo / (albedo + vec3(1.0));

    vec3 gammaCorrected = pow(mapped, vec3(1.0 / uGamma));

    oFragColor = vec4(gammaCorrected, 1.0);
}
