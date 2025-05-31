#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform vec3 uPosition;
uniform vec3 uScale;
uniform vec4 uRotation;

uniform mat4 uProjection;
uniform mat4 uView;

out vec3 vNormal;
out vec2 vTexCoord;
out vec3 vFragPos;

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

    vec3 translated = translate(aPos,uPosition,uScale,uRotation);
    gl_Position = uProjection * uView * vec4(translated.xyz, 1.0);

    vNormal = normalize(rotateVertex(aNormal,uRotation));
    vTexCoord = aTexCoord;
    vFragPos = translated;
}
