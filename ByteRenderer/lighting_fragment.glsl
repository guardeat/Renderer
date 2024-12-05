#version 330 core

layout(location = 0) out vec4 gAlbedoSpecular;

in vec2 vTexCoord; 

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
    vec3 fragPos = texture(uPosition, vTexCoord).rgb;
    vec3 normal = normalize(texture(uNormal, vTexCoord).rgb);
    float shadow = texture(uNormal, vTexCoord).w;
    vec3 albedo = texture(uAlbedoSpec, vTexCoord).rgb;
    float specularStrength = texture(uAlbedoSpec, vTexCoord).a;

    vec3 ambient = 0.3 * albedo * uDirectionalLight.color * uDirectionalLight.intensity;

    vec3 lightDir = normalize(-uDirectionalLight.direction);
    float diff = max(dot(normal, lightDir), 0.0);           
    vec3 diffuse = diff * albedo * uDirectionalLight.color * uDirectionalLight.intensity;

    vec3 viewDir = normalize(uViewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal); 
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = spec * specularStrength * uDirectionalLight.color * uDirectionalLight.intensity;

    vec3 finalColor = (ambient + (1.0 - shadow) * (diffuse + specular));

    gAlbedoSpecular = vec4(finalColor, 1.0);

    if(fragPos == vec3(0.0)){
        gAlbedoSpecular.xyz = albedo;
    }
}

