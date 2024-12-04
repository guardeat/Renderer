#version 330 core

layout(location = 2) out vec4 gAlbedoSpecular;

uniform vec3 uDirection;

in vec3 vFragPos;

void main() {
    float bottomBlend = smoothstep(-1.0, 0.5, vFragPos.y); 
    float middleBlend = smoothstep(0.0, 0.3, vFragPos.y);  

    vec4 colorBelow = vec4(0.0, 0.0, 0.2, 1.0);     
    vec4 colorMiddle = vec4(1.0, 0.8, 0.7, 1.0);   
    vec4 colorAbove = vec4(0.2, 0.5, 0.8, 1.0);   

    vec4 blendedBelowMiddle = mix(colorBelow, colorMiddle, bottomBlend);

    vec4 finalColor = mix(blendedBelowMiddle, colorAbove, middleBlend);

    gAlbedoSpecular = finalColor;
}