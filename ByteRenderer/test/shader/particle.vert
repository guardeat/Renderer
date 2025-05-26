#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

layout (location = 3) in vec3 aPosition;
layout (location = 4) in vec3 aScale;
layout (location = 5) in vec4 aRotation;

uniform mat4 uProjection;
uniform mat4 uView;

out vec3 vNormal;
out vec2 vTexCoord;
out vec3 vFragPos;

vec3 scaleVertex(vec3 point, vec3 scaleFactor) {
    return point * scaleFactor;
}

vec3 translateVertex(vec3 point, vec3 translation) {
    return point + translation;
}

vec3 getCameraPosition(mat4 view) {
    mat4 invView = inverse(view);
    return vec3(invView[3]);
}

void quatToAxisAngle(vec4 q, out vec3 axis, out float angle) {
    angle = 2.0 * acos(q.w);
    float s = sqrt(1.0 - q.w * q.w);
    if (s < 0.001) {
        axis = vec3(1.0, 0.0, 0.0);
    } else {
        axis = q.xyz / s;
    }
}

mat3 rotationAroundAxis(vec3 axis, float angle) {
    float c = cos(angle);
    float s = sin(angle);
    float t = 1.0 - c;

    axis = normalize(axis);
    float x = axis.x;
    float y = axis.y;
    float z = axis.z;

    return mat3(
        t*x*x + c,      t*x*y - s*z,    t*x*z + s*y,
        t*x*y + s*z,    t*y*y + c,      t*y*z - s*x,
        t*x*z - s*y,    t*y*z + s*x,    t*z*z + c
    );
}

mat3 getBaseBillboardRotation(vec3 objPos, vec3 camPos) {
    vec3 up = normalize(camPos - objPos);

    vec3 forward = vec3(0.0, 0.0, 1.0);

    vec3 right = normalize(cross(up, forward));

    vec3 normal = cross(right, up);

    return mat3(right, up, normal);
}

void main() {
    vec3 scaled = scaleVertex(aPos, aScale);

    vec3 camPos = getCameraPosition(uView);

    mat3 baseRot = getBaseBillboardRotation(aPosition, camPos);

    vec3 quatAxis;
    float quatAngle;
    quatToAxisAngle(aRotation, quatAxis, quatAngle);

    float projection = dot(quatAxis, baseRot[1]);
    vec3 axisAroundUp = baseRot[1] * projection;

    mat3 rotAroundUp;
    if(length(axisAroundUp) < 0.001) {
        rotAroundUp = mat3(1.0);
    } else {
        rotAroundUp = rotationAroundAxis(axisAroundUp, quatAngle * sign(projection));
    }

    mat3 finalRot = rotAroundUp * baseRot;

    vec3 rotated = finalRot * scaled;

    vec3 translated = translateVertex(rotated, aPosition);

    gl_Position = uProjection * uView * vec4(translated, 1.0);

    vNormal = normalize(finalRot * aNormal);
    vTexCoord = aTexCoord;
    vFragPos = translated;
}
