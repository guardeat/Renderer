#version 410 core

layout(location = 0) in vec3 aPos;

uniform mat4 uInverseProjection;  

out vec3 vRotatedDir;

void main() {
    gl_Position = vec4(aPos, 1.0);
    
    vec4 viewDir = uInverseProjection * vec4(aPos.xy, 1.0, 1.0);
    vRotatedDir = normalize(viewDir.xyz);
}