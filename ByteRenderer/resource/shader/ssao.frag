#version 330 core

out float oFragColor;

in vec2 vTexCoord;

uniform sampler2D uNormal;
uniform sampler2D uDepth;
uniform sampler2D uNoise;

uniform mat4 uProjection;
uniform mat4 uInverseProjection;
uniform mat4 uInverseView;
uniform vec2 uScreenSize;
uniform vec3 uSamples[64];

const float radius = 0.5;
const float bias = 0.025;

vec3 getViewPos(vec2 uv, float depth) {
    vec4 ndc = vec4(uv * 2.0 - 1.0, depth, 1.0);
    vec4 viewPos = uInverseProjection * ndc;
    return viewPos.xyz / viewPos.w;
}

void main() {
    vec2 noiseScale = uScreenSize / 4.0;

    vec3 normal = texture(uNormal, vTexCoord).rgb;
    normal = normalize(normal);

    float depth = texture(uDepth, vTexCoord).r;
    if (depth >= 1.0) {
        oFragColor = 1.0;
        return;
    }

    vec3 fragPos = getViewPos(vTexCoord, depth);

    vec3 randomVec = texture(uNoise, vTexCoord * noiseScale).xyz;
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    if (length(tangent) < 0.01)
        tangent = vec3(1.0, 0.0, 0.0);
    vec3 bitangent = normalize(cross(normal, tangent));
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for (int i = 0; i < 64; ++i) {
        vec3 sampleVec = TBN * uSamples[i];
        vec3 samplePos = fragPos + sampleVec * radius;

        vec4 offset = uProjection * vec4(samplePos, 1.0);
        offset.xyz /= offset.w;
        offset.xy = offset.xy * 0.5 + 0.5;

        if (offset.x < 0.0 || offset.x > 1.0 || offset.y < 0.0 || offset.y > 1.0)
            continue;

        float sampleDepth = texture(uDepth, offset.xy).r;
        if (sampleDepth >= 1.0)
            continue;

        vec3 sampleViewPos = getViewPos(offset.xy, sampleDepth);
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleViewPos.z));

        if (sampleViewPos.z <= samplePos.z - bias)
            occlusion += rangeCheck;
    }

    occlusion = 1.0 - (occlusion / 64.0);
    oFragColor = occlusion;
}
