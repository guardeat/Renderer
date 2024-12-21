#version 330 core

in vec2 vTexCoord;

out vec4 FragColor;

uniform sampler2D uAlbedoSpecular;

void main() {
    vec3 albedoSpecular = texture(uAlbedoSpecular, vTexCoord).rgb;

    FragColor = vec4(albedoSpecular, 1.0);
}