#version 330 core

out vec4 FragColor;

in vec2 vTexCoords;

uniform sampler2D vPosition;
uniform sampler2D vNormal;
uniform sampler2D vAlbedoSpec;

uniform vec3 uViewPos;

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};

uniform DirectionalLight uDirectionalLight;

void main()
{
    vec3 fragPos = texture(vPosition, vTexCoords).rgb;
    vec3 normal = normalize(texture(vNormal, vTexCoords).rgb);
    vec3 albedo = texture(vAlbedoSpec, vTexCoords).rgb;
    float specularStrength = texture(vAlbedoSpec, vTexCoords).a;

    vec3 ambient = 0.1 * albedo;

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