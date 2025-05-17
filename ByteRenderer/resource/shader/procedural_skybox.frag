#version 410 core

layout(location = 1) out vec3 gAlbedo;

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

    vec3 colorBelow = vec3(0.0, 0.0, 0.2);     
    vec3 colorMiddle = vec3(0.7, 0.8, 0.9);   
    vec3 colorAbove = vec3(0.2, 0.5, 0.8);   

    vec3 blendedBelowMiddle = mix(colorBelow, colorMiddle, bottomBlend);
    vec3 finalColor = mix(blendedBelowMiddle, colorAbove, middleBlend);

    vec3 normal = normalize(vec3(0.0) - vRotatedPos);
    float intensity = pow(max(dot(normal, normalize(uDirectionalLight.direction)), 0.0), 64.0) * uDirectionalLight.intensity;

    vec3 sunColor = vec3(1.0, 0.8, 0.4);
    vec3 centerColor = vec3(1.0, 0.9, 0.7);

    vec3 blendedSunCircle = mix(sunColor, centerColor, smoothstep(0.9, 0.95, intensity));

    float timeIntensity = clamp(dot(vec3(0, -1, 0), normalize(uDirectionalLight.direction)), 0.2, 1.0);

    sunColor *= uDirectionalLight.color;
    centerColor *= uDirectionalLight.color;
    finalColor *= uDirectionalLight.intensity * timeIntensity;

    if (intensity > 0.9) {
        finalColor = blendedSunCircle;
    } else {
        finalColor = mix(finalColor, sunColor * intensity, intensity);
    }

    finalColor *= uDirectionalLight.intensity;
    gAlbedo = finalColor; 
}
