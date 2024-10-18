#version 330 core

out vec4 FragColor;

uniform sampler2D uSPosition;
uniform sampler2D uSNormal;
uniform sampler2D uSAlbedoSpec;

uniform struct PointLight {
	vec3 position;
	vec3 color;
	
	float intensity;

} uPointLight;

uniform vec2 uViewPortSize;

void main() {
	vec2 texCoord = gl_FragCoord.xy / uViewPortSize; 

	vec3 pos = texture(uSPosition,texCoord).xyz;
	vec3 normal = texture(uSNormal,texCoord).xyz;
	vec3 baseColor = texture(uSAlbedoSpec,texCoord).xyz;

	vec3 lightDir = normalize(uPointLight.position - pos);
    float lightDistance = length(uPointLight.position - pos);
    float attenuation = uPointLight.intensity / (lightDistance * lightDistance);
	
	vec3 color = baseColor + (baseColor * attenuation * uPointLight.color) / 2;

	FragColor = vec4(color,1.0f);
}