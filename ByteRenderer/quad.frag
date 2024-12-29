#version 330 core

in vec2 vTexCoord;

out vec4 FragColor;

uniform sampler2D uAlbedoSpecular;
uniform float uGamma;

void main() {
    vec3 albedoSpecular = texture(uAlbedoSpecular, vTexCoord).rgb;

    FragColor = vec4(pow(albedoSpecular.rgb, vec3(1.0/uGamma)), 1.0);
}