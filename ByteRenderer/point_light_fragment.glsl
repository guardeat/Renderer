#version 330 core

layout (location = 1) out vec4 gAlbedoSpecular;

uniform sampler2D uSPosition;   
uniform sampler2D uSNormal;    
uniform sampler2D uSAlbedoSpec; 

struct PointLight {
    vec3 position;
    vec3 color;

    float constant;
    float linear;
    float quadratic;
};

uniform PointLight uPointLight;
uniform vec2 uViewPortSize;

void main() {
    vec2 texCoord = gl_FragCoord.xy / uViewPortSize;

    vec3 pos = texture(uSPosition, texCoord).xyz;
    vec3 normal = normalize(texture(uSNormal, texCoord).xyz);
    vec3 albedo = texture(uSAlbedoSpec, texCoord).rgb;

    vec3 lightDir = normalize(uPointLight.position - pos);
    float lightDistance = length(uPointLight.position - pos);

    float attenuation = 1.0 / (uPointLight.constant + 
                               uPointLight.linear * lightDistance + 
                               uPointLight.quadratic * (lightDistance * lightDistance));

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * uPointLight.color;

    vec3 resultColor = albedo * diffuse * attenuation;

    gAlbedoSpecular = vec4(resultColor, 1.0);
}
