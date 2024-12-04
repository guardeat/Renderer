#version 330 core

layout(location = 0) in vec3 aPos;

uniform mat4 uProjection;
uniform vec4 uRotation;

out vec3 vFragPos;

vec3 rotateVertex( vec3 v, vec4 q ) {
    return v + 2.*cross( q.xyz, cross( q.xyz, v ) + q.w*v ); 
}

void main() {
    vec3 rotatedPos = rotateVertex(aPos,-uRotation);

    vFragPos = (uProjection * vec4(rotatedPos,1.0)).xyz;

    gl_Position = uProjection * vec4(aPos,1.0);
}
