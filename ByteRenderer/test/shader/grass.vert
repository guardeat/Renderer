#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aPosition;
layout (location = 4) in vec3 aScale;

uniform mat4 uProjection;
uniform mat4 uView;

uniform float uTime;
uniform vec3 uWind;

out vec3 vNormal;
out vec2 vTexCoord;
out vec3 vFragPos;

vec3 rotateAroundY(vec3 v, float angle) {
    float s = sin(angle);
    float c = cos(angle);
    return vec3(
        c * v.x + s * v.z,
        v.y,
        -s * v.x + c * v.z
    );
}

vec3 getCameraPosition() {
    return vec3(inverse(uView)[3]);
}

vec3 getYBillboardDirection(vec3 instancePos) {
    vec3 toCamera = getCameraPosition() - instancePos;
    toCamera.y = 0.0;
    return normalize(toCamera);
}

float getYRotationAngle(vec3 dir) {
    return atan(dir.x, dir.z);
}

float rand(vec3 seed) {
    return fract(sin(dot(seed, vec3(12.9898, 78.233, 37.719))) * 43758.5453);
}

void main() {
    vec3 toCamDir = getYBillboardDirection(aPosition);
    float baseAngle = getYRotationAngle(toCamDir);
    float randomOffset = (rand(aPosition) - 0.5) * radians(20.0);
    float angle = baseAngle + randomOffset;

    vec3 scaledPos = aPos * aScale;

    float posRandomFactor = rand(aPosition * 0.1); 
    float swayStrength = (0.1 + length(uWind) * 0.3) * (0.5 + posRandomFactor * 0.5);
    float swaySpeed = 2.0 + length(uWind) * 3.0;
    vec2 windDir = normalize(uWind.xz + vec2(0.001));
    float phaseOffset = dot(windDir, aPosition.xz);
    float sway = sin(uTime * swaySpeed + phaseOffset + posRandomFactor * 2.0) * swayStrength;

    scaledPos.x += sway * aPos.y;

    vec3 rotatedPos = rotateAroundY(scaledPos, angle);
    vec3 worldPos = rotatedPos + aPosition;

    gl_Position = uProjection * uView * vec4(worldPos, 1.0);

    vNormal = normalize(rotateAroundY(aNormal, angle));
    vTexCoord = aTexCoord;
    vFragPos = worldPos;
}
