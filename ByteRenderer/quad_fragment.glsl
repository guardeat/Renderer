#version 330 core

in vec2 vTexCoord;

out vec4 FragColor;

uniform sampler2D uAlbedoSpecular;

void main() {
    FragColor = vec4(texture(uAlbedoSpecular, vTexCoord).rgb,1.0f);
}