#version 410 core

in vec2 vTexCoord;

out vec4 FragColor;

uniform sampler2D uDepthMap;

void main() {
    float depth = texture(uDepthMap, vTexCoord).r;

    FragColor = vec4(vec3(depth), 1.0);
}
