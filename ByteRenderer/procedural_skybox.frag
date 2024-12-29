#version 330 core

layout(location = 2) out vec4 gAlbedoSpecular;

struct DirectionalLight {
    vec3 direction; 
    vec3 color;      
    float intensity; 
};

uniform DirectionalLight uDirectionalLight;

in vec3 vFragPos;
in vec3 vRotatedPos;

void main() {
    float bottomBlend = smoothstep(-1.0, 1.0, vFragPos.y); 
    float middleBlend = smoothstep(0.0, 0.3, vFragPos.y);  

    vec4 colorBelow = vec4(0.0, 0.0, 0.2, 1.0);     
    vec4 colorMiddle = vec4(0.7, 0.8, 0.9, 1.0);   
    vec4 colorAbove = vec4(0.2, 0.5, 0.8, 1.0);   

    vec4 blendedBelowMiddle = mix(colorBelow, colorMiddle, bottomBlend);
    vec4 finalColor = mix(blendedBelowMiddle, colorAbove, middleBlend);

    vec3 normal = normalize(vec3(0.0) - vRotatedPos);
    float intensity = pow(max(dot(normal, normalize(uDirectionalLight.direction)), 0.0), 64.0) * uDirectionalLight.intensity;

    vec4 sunColor = vec4(1.0, 0.8, 0.4, 1.0);
    vec4 centerColor = vec4(1.0, 0.9, 0.7, 1.0);

    vec4 blendedSunCircle = mix(sunColor, centerColor, smoothstep(0.9, 0.95, intensity));

    float timeIntensity = clamp(dot(vec3(0, -1, 0), normalize(uDirectionalLight.direction)), 0.2, 1.0);

    sunColor.rgb *= uDirectionalLight.color;
    centerColor.rgb *= uDirectionalLight.color;
    finalColor.rgb *= uDirectionalLight.intensity * timeIntensity;

    if (intensity > 0.9) {
        finalColor = blendedSunCircle;
    } else {
        finalColor = mix(finalColor, sunColor * intensity, intensity);
    }

    finalColor.rgb *= uDirectionalLight.intensity;
    gAlbedoSpecular = finalColor; 
}
