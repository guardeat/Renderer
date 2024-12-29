#version 330 core

layout(location = 0) out vec4 gColor;

in vec2 vTexCoord; 
   
uniform sampler2D uNormal;      
uniform sampler2D uAlbedo; 
uniform sampler2D uDepth;

uniform vec3 uViewPos; 

uniform struct {
    vec3 direction; 
    vec3 color;      
    float intensity; 
} uDirectionalLight;

uniform mat4 uView;
uniform mat4 uInverseView;
uniform mat4 uInverseProjection;

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

vec3 WorldPosFromDepth(float depth) {
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(vTexCoord * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = uInverseProjection * clipSpacePosition;

    viewSpacePosition /= viewSpacePosition.w;

    vec4 worldSpacePosition = uInverseView * viewSpacePosition;

    return worldSpacePosition.xyz;
}

void main()
{
    vec3 albedo = texture(uAlbedo, vTexCoord).rgb;

    if(texture(uDepth, vTexCoord).r == 1.0) {
        gColor = vec4(albedo,1.0);
    }
    else {
        vec3 fragPos = WorldPosFromDepth(texture(uDepth, vTexCoord).r);
        vec3 normal = normalize(texture(uNormal, vTexCoord).rgb);
        float shadow = calculateShadow(fragPos);
        float specularStrength = texture(uAlbedo, vTexCoord).a;

        vec3 ambient = 0.3 * albedo * uDirectionalLight.color * uDirectionalLight.intensity;

        vec3 lightDir = normalize(-uDirectionalLight.direction);
        float diff = max(dot(normal, lightDir), 0.0);           
        vec3 diffuse = diff * albedo * uDirectionalLight.color * uDirectionalLight.intensity;

        vec3 viewDir = normalize(uViewPos - fragPos);
        vec3 reflectDir = reflect(-lightDir, normal); 
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
        vec3 specular = spec * specularStrength * uDirectionalLight.color * uDirectionalLight.intensity;

        vec3 finalColor = (ambient + (1.0 - shadow) * (diffuse + specular));

        gColor = vec4(finalColor, 1.0);
    }
}

