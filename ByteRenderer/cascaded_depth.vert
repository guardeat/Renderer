#version 330 core

layout (location = 0) in vec3 aPos;

uniform vec3 uPosition;
uniform vec3 uScale;
uniform vec4 uRotation;

vec3 rotateVertex( vec3 v, vec4 q ) {
    return v + 2.*cross( q.xyz, cross( q.xyz, v ) + q.w*v ); 
}

vec3 translateVertex(vec3 point, vec3 translation) {
    return point + translation;
}

vec3 scaleVertex(vec3 point, vec3 scaleFactor) {
    return point * scaleFactor;
}

vec3 translate(vec3 aPos, vec3 position, vec3 scale, vec4 rotation) {
    vec3 translatedPos = scaleVertex(aPos, scale);

    translatedPos = rotateVertex(translatedPos, rotation);

    translatedPos = translateVertex(translatedPos, position);

    return translatedPos;
}

void main() {
    gl_Position = vec4(translate(aPos,uPosition,uScale,uRotation), 1.0);
}