#version 410 core

layout (quads, fractional_odd_spacing, ccw) in;

uniform sampler2D uHeightMap;

uniform vec3 uPosition;           
uniform vec3 uScale;
uniform vec4 uRotation;

uniform mat4 uProjection;
uniform mat4 uView;

in vec2 vTexCoord_[];

out vec3 vFragPos;
out vec3 vNormal;
out vec2 vTexCoord;

vec3 rotateVertex(vec3 v, vec4 q) {
    return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v); 
}

vec3 translateVertex(vec3 point, vec3 translation) {
    return point + translation;
}

vec3 scaleVertex(vec3 point, vec3 scaleFactor) {
    return point * scaleFactor;
}

vec3 applyTransform(vec3 aPos, vec3 position, vec3 scale, vec4 rotation) {
    vec3 transformedPos = scaleVertex(aPos, scale);
    transformedPos = rotateVertex(transformedPos, rotation);
    transformedPos = translateVertex(transformedPos, position);
    return transformedPos;
}

float getHeight(vec2 uv) {
    return texture(uHeightMap, uv).y * 64.0 - 16.0;
}

vec3 interpolatePosition(vec4 p00, vec4 p01, vec4 p10, vec4 p11, float u, float v) {
    vec4 p0 = mix(p00, p01, u);
    vec4 p1 = mix(p10, p11, u);
    return mix(p0, p1, v).xyz;
}

void main()
{
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    vec2 t00 = vTexCoord_[0];
    vec2 t01 = vTexCoord_[1];
    vec2 t10 = vTexCoord_[2];
    vec2 t11 = vTexCoord_[3];

    vec2 uv = mix(mix(t00, t01, u), mix(t10, t11, u), v);
    vTexCoord = uv;

    vec4 p00 = gl_in[0].gl_Position;
    vec4 p01 = gl_in[1].gl_Position;
    vec4 p10 = gl_in[2].gl_Position;
    vec4 p11 = gl_in[3].gl_Position;

    float heightCenter = getHeight(uv);

    float offset = 0.001;

    vec3 posCenter = interpolatePosition(p00, p01, p10, p11, u, v) + vec3(0, heightCenter, 0);

    float heightU = getHeight(uv + vec2(offset, 0.0));
    float heightV = getHeight(uv + vec2(0.0, offset));

    vec3 posU = interpolatePosition(p00, p01, p10, p11, u + offset, v) + vec3(0, heightU, 0);
    vec3 posV = interpolatePosition(p00, p01, p10, p11, u, v + offset) + vec3(0, heightV, 0);

    vec3 tangentU = posU - posCenter;
    vec3 tangentV = posV - posCenter;

    vec3 normal = normalize(cross(tangentV, tangentU));

    vec3 worldPos = applyTransform(posCenter, uPosition, uScale, uRotation);
    vFragPos = worldPos;

    vNormal = rotateVertex(normal, uRotation);

    gl_Position = uProjection * uView * vec4(worldPos, 1.0);
}
