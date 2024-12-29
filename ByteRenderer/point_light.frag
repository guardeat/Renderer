#version 330 core

layout (location = 0) out vec4 gColor;

uniform sampler2D uNormal;    
uniform sampler2D uAlbedo; 
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

vec3 WorldPosFromDepth(float depth, vec2 texCoord) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(texCoord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = uInverseProjection * clipSpacePosition;

    viewSpacePosition /= viewSpacePosition.w;

    vec4 worldSpacePosition = uInverseView * viewSpacePosition;

    return worldSpacePosition.xyz;
}

void main() {
    vec2 texCoord = gl_FragCoord.xy / uViewPortSize;

    vec3 pos = WorldPosFromDepth(texture(uDepth, texCoord).r, texCoord);
    vec3 normal = normalize(texture(uNormal, texCoord).xyz);
    vec3 albedo = texture(uAlbedo, texCoord).rgb;

    vec3 lightDir = normalize(uPointLight.position - pos);
    float lightDistance = length(uPointLight.position - pos);

    float attenuation = 1.0 / (uPointLight.constant + 
                               uPointLight.linear * lightDistance + 
                               uPointLight.quadratic * (lightDistance * lightDistance));

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * uPointLight.color;

    vec3 resultColor = albedo * diffuse * attenuation;

    gColor = vec4(resultColor, 1.0);
}
