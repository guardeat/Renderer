#version 410 core

in vec3 vNormal;
in vec3 vFragPos;
in vec2 vTexCoord;

out vec4 oFragColor;

void main() {
    oFragColor = vec4(1.0f,0.0f,0.0f,0.0f);
}
