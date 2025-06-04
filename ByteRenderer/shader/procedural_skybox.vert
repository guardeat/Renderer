#version 410 core

layout(location = 0) in vec3 aPos;

uniform mat4 uInverseViewProjection;  

out vec3 vRotatedDir;

void main() {
    gl_Position = vec4(aPos, 1.0);
    
    vec4 viewDir = uInverseViewProjection * vec4(aPos.xy, 1.0, 1.0);
    vRotatedDir = normalize(viewDir.xyz / viewDir.w);
}