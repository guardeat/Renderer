#version 330 core

in vec2 vTexCoord; 

out vec4 FragColor; 

uniform sampler2D uAlbedoSpecular;          
uniform sampler2D uDirectionalLightAlbedo;  
uniform sampler2D uPointLightAlbedo;      

void main() {
    vec3 albedoSpecular = texture(uAlbedoSpecular, vTexCoord).rgb;
    vec3 directionalLight = texture(uDirectionalLightAlbedo, vTexCoord).rgb;
    vec3 pointLight = texture(uPointLightAlbedo, vTexCoord).rgb;

    vec3 finalColor = (directionalLight + pointLight);

    FragColor = vec4(finalColor, 1.0);
}