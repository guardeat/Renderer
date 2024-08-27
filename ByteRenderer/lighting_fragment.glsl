#version 330 core

in vec2 vTexCoords;

out vec4 FragColor;

uniform sampler2D uPosition;
uniform sampler2D uNormal;
uniform sampler2D uAlbedoSpec;

uniform vec3 uViewPos;

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

uniform DirectionalLight uDirectionalLight;

void main()
{
    vec3 fragPos = texture(uPosition, vTexCoords).rgb;
    vec3 normal = normalize(texture(uNormal, vTexCoords).rgb);
    vec3 albedo = texture(uAlbedoSpec, vTexCoords).rgb;
    float specularStrength = texture(uAlbedoSpec, vTexCoords).a;

    vec3 ambient = uDirectionalLight.intensity * albedo;

    vec3 lightDir = normalize(-uDirectionalLight.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * uDirectionalLight.color * albedo;

    vec3 viewDir = normalize(uViewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * uDirectionalLight.color;

    vec3 finalColor = ambient + diffuse + specular;

    FragColor = vec4(finalColor, 1.0);
}