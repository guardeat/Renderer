#version 330 core

in vec2 vTexCoord;

out vec4 FragColor;

uniform sampler2D uAlbedoSpecular;
uniform float uGamma;

void main() {
    vec3 albedo = texture(uAlbedoSpecular, vTexCoord).rgb;

    vec3 mapped = albedo / (albedo + vec3(1.0));

    FragColor = vec4(pow(mapped.rgb, vec3(1.0/uGamma)), 1.0);
}