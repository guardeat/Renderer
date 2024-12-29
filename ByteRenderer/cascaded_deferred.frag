#version 330 core

layout (location = 0) out vec3 gFragPosition;  
layout (location = 1) out vec4 gFragNormal;   
layout (location = 2) out vec4 gAlbedoSpecular; 

in vec3 vNormal;   
in vec2 vTexCoord; 
in vec3 vFragPos;  

uniform vec4 uAlbedo;   

uniform mat4 uView;

uniform sampler2D uDepthMaps[4];  
uniform float uCascadeFars[4];     
uniform mat4 uLightSpaces[4];   
uniform int uCascadeCount;

float calculateShadow(vec3 fragWorldPos)
{
    vec4 fragPosViewSpace = uView * vec4(fragWorldPos, 1.0);
    float depthValue = abs(fragPosViewSpace.z);

    float shadow = 0.0;

    int layer = 0;
    for (int i = uCascadeCount - 1; i > 0; --i)
    {
        if (depthValue < uCascadeFars[i])
        {
            layer = i;
            break;
        }
    }
  
    vec4 fragLightSpace = uLightSpaces[layer] * vec4(fragWorldPos, 1.0);

    vec3 projCoords = fragLightSpace.xyz / fragLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; 

    if (projCoords.x < 0.0 || projCoords.x > 1.0 || 
        projCoords.y < 0.0 || projCoords.y > 1.0 || 
        projCoords.z < 0.0 || projCoords.z > 1.0) {
        return 0.0; 
    }

    float closestDepth = texture(uDepthMaps[layer], projCoords.xy).r;
    float currentDepth = projCoords.z;

    float shadowValue = 0.0;
    vec2 texelSize = 1.0 / textureSize(uDepthMaps[layer], 0);
    int pcfRadius = 1;
    
    for(int x = -pcfRadius; x <= pcfRadius; ++x)
    {
        for(int y = -pcfRadius; y <= pcfRadius; ++y)
        {
            vec2 offset = vec2(x, y) * texelSize;
            float closestDepth = texture(uDepthMaps[layer], projCoords.xy + offset).r;
            shadowValue += currentDepth > closestDepth + 0.001 ? 1.0 : 0.0;
        }
    }
    
    int samples = (pcfRadius * 2 + 1) * (pcfRadius * 2 + 1);
    shadow = shadowValue / float(samples);
    
    return shadow;
}

void main()
{    
    gFragPosition = vFragPos;

    gFragNormal.xyz = normalize(vNormal);
    gFragNormal.w = calculateShadow(vFragPos);

    gAlbedoSpecular = uAlbedo;
}
