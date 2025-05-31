#version 410 core

layout (location = 0) in vec3 aPos;

layout (location = 3) in vec3 aPosition;
layout (location = 4) in vec3 aScale;
layout (location = 5) in vec4 aRotation;

uniform mat4 uLightSpace;

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
    vec3 translated = translate(aPos,aPosition,aScale,aRotation);
    gl_Position = uLightSpace * vec4(translated.xyz, 1.0);
}
