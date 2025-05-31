#version 410 core

out vec4 oFragColor;

in vec2 vTexCoord;

uniform sampler2D uDepthMap;

void main() {
    float depth = texture(uDepthMap, vTexCoord).r;

    oFragColor = vec4(vec3(depth), 1.0);
}
