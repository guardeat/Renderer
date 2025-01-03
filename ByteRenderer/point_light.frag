#version 330 core

layout (location = 0) out vec4 gColor;

uniform sampler2D uNormal;    
uniform sampler2D uAlbedo; 
uniform sampler2D uMaterial;
uniform sampler2D uDepth; 

uniform struct PointLight {
    vec3 position;
    vec3 color;

    float constant;
    float linear;
    float quadratic;
} uPointLight;

uniform vec2 uViewPortSize;
uniform mat4 uInverseView;
uniform mat4 uInverseProjection;

uniform vec3 uViewPos;

const float PI = 3.14159265359;

vec3 WorldPosFromDepth(float depth, vec2 texCoord) {
    float z = depth * 2.0 - 1.0;
    vec4 clipSpacePosition = vec4(texCoord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = uInverseProjection * clipSpacePosition;
    viewSpacePosition /= viewSpacePosition.w;
    vec4 worldSpacePosition = uInverseView * viewSpacePosition;
    return worldSpacePosition.xyz;
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main() {
    vec2 texCoord = gl_FragCoord.xy / uViewPortSize;
    vec3 pos = WorldPosFromDepth(texture(uDepth, texCoord).r, texCoord);
    vec3 normal = normalize(texture(uNormal, texCoord).xyz);
    vec3 albedo = texture(uAlbedo, texCoord).rgb;
    float metallic = texture(uMaterial, texCoord).r;
    float roughness = texture(uMaterial, texCoord).g;

    vec3 V = normalize(uViewPos - pos);
    vec3 L = normalize(uPointLight.position - pos);
    vec3 H = normalize(V + L);
    float distance = length(uPointLight.position - pos);
    float attenuation = 1.0 / (uPointLight.constant + uPointLight.linear * distance + uPointLight.quadratic * (distance * distance));
    vec3 radiance = uPointLight.color * attenuation;

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    float NDF = DistributionGGX(normal, H, roughness);
    float G = GeometrySmith(normal, V, L, roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(normal, L), 0.0);
    vec3 resultColor = (kD * albedo / PI + specular) * radiance * NdotL;

    gColor = vec4(resultColor, 1.0);
}
