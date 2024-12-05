#version 330 core

layout(location = 2) out vec4 gAlbedoSpecular;

uniform vec3 uDirection;

in vec3 vFragPos;
in vec3 vRotatedPos;

void main() {
    float bottomBlend = smoothstep(-1.0, 0.5, vFragPos.y); 
    float middleBlend = smoothstep(0.0, 0.3, vFragPos.y);  

    vec4 colorBelow = vec4(0.0, 0.0, 0.2, 1.0);     
    vec4 colorMiddle = vec4(1.0, 0.8, 0.7, 1.0);   
    vec4 colorAbove = vec4(0.2, 0.5, 0.8, 1.0);   

    vec4 blendedBelowMiddle = mix(colorBelow, colorMiddle, bottomBlend);

    vec4 finalColor = mix(blendedBelowMiddle, colorAbove, middleBlend);

    vec3 normal = normalize(vRotatedPos - vec3(0.0));

    float intensity = pow(max(dot(normal, uDirection), 0.0), 32.0);
    vec4 sunColor = vec4(1.0, 0.8, 0.0,1.0);

    if(intensity > 0.9){
        gAlbedoSpecular = sunColor * intensity;
    }
    else{
        gAlbedoSpecular =  mix(finalColor,sunColor * intensity,intensity);
    }
}