#version 410 core

layout(location = 0) out vec3 oColor;

in vec2 vTexCoord;

uniform sampler2D uNormal;
uniform sampler2D uAlbedo;
uniform sampler2D uMaterial;
uniform sampler2D uDepth;
uniform sampler2D uSSAO;

uniform vec3 uViewPos;

uniform struct {
    vec3 direction;
    vec3 color;
    float intensity;
} uDirectionalLight;

uniform mat4 uView;
uniform mat4 uInverseView;
uniform mat4 uInverseProjection;

uniform sampler2D uDepthMaps[4];
uniform float uCascadeFars[4];
uniform mat4 uLightSpaces[4];
uniform int uCascadeCount;

uniform bool uUseSSAO;

const float PI = 3.14159265359;

float calculateShadow(vec3 fragWorldPos, vec3 normal)
{
    vec4 fragPosViewSpace = uView * vec4(fragWorldPos, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    float shadow = 0.0;

    int layer = 0;
    for (int i = uCascadeCount - 1; i > 0; --i)
    {
        if (depthValue < uCascadeFars[i])
        {
            layer = i;
            break;
        }
    }

    vec4 fragLightSpace = uLightSpaces[layer] * vec4(fragWorldPos, 1.0);
    vec3 projCoords = fragLightSpace.xyz / fragLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.x < 0.0 || projCoords.x > 1.0 || 
        projCoords.y < 0.0 || projCoords.y > 1.0 || 
        projCoords.z < 0.0 || projCoords.z > 1.0) {
        return 0.0;
    }

    float currentDepth = projCoords.z;
    vec2 texelSize = 1.0 / textureSize(uDepthMaps[layer], 0);
    int pcfRadius = 1;
    float shadowValue = 0.0;

    for(int x = -pcfRadius; x <= pcfRadius; ++x)
    {
        for(int y = -pcfRadius; y <= pcfRadius; ++y)
        {
            vec2 offset = vec2(x, y) * texelSize;
            float closestDepth = texture(uDepthMaps[layer], projCoords.xy + offset).r;
            float bias = 0.0002;
            shadowValue += currentDepth - bias > closestDepth ? 1.0 : 0.0;
        }
    }

    int samples = (pcfRadius * 2 + 1) * (pcfRadius * 2 + 1);
    shadow = shadowValue / float(samples);

    return shadow;
}

vec3 worldPosFromDepth(float depth) {
    float z = depth * 2.0 - 1.0;
    vec4 clipSpacePosition = vec4(vTexCoord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = uInverseProjection * clipSpacePosition;
    viewSpacePosition /= viewSpacePosition.w;
    vec4 worldSpacePosition = uInverseView * viewSpacePosition;
    return worldSpacePosition.xyz;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return nom / denom;
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

void main()
{
    vec3 albedo = texture(uAlbedo, vTexCoord).rgb;
    float metallic = texture(uMaterial, vTexCoord).r;
    float roughness = texture(uMaterial, vTexCoord).g;
    float materialAO = texture(uMaterial, vTexCoord).b;

    float ssaoFactor = 1.0f;
    if(uUseSSAO){
        ssaoFactor = texture(uSSAO, vTexCoord).r;
    }

    float emission = texture(uMaterial, vTexCoord).a;

    if(texture(uDepth, vTexCoord).r >= 1.0) {
        oColor = albedo;
    }
    else {
        vec3 fragPos = worldPosFromDepth(texture(uDepth, vTexCoord).r);
        vec3 normal = normalize(texture(uNormal, vTexCoord).rgb);
        float shadow = calculateShadow(fragPos, normal);

        vec3 F0 = vec3(0.04);
        F0 = mix(F0, albedo, metallic);

        vec3 lightDir = normalize(-uDirectionalLight.direction);
        vec3 viewDir = normalize(uViewPos - fragPos);
        vec3 halfwayDir = normalize(lightDir + viewDir);

        float NDF = distributionGGX(normal, halfwayDir, roughness);
        float G = geometrySmith(normal, viewDir, lightDir, roughness);
        vec3 F = fresnelSchlick(max(dot(halfwayDir, viewDir), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(normal, lightDir), 0.0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * NdotL + 0.001;
        vec3 specular = numerator / denominator;

        vec3 Lo = (kD * albedo / PI + specular) * NdotL * uDirectionalLight.color * uDirectionalLight.intensity * (1.0 - shadow);
        vec3 ambient = materialAO * albedo * uDirectionalLight.color * uDirectionalLight.intensity * ssaoFactor;
        vec3 finalColor = ambient + Lo + emission * albedo;
        oColor = finalColor;
    }
}
