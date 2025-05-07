#version 330 core

layout (location = 0) out vec3 downsample;

uniform sampler2D uSrcTexture;
uniform vec2 uSrcResolution;

in vec2 vTexCoord;

void main()
{
    vec2 srcTexelSize = 1.0 / uSrcResolution;
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;

    vec3 a = texture(uSrcTexture, vec2(vTexCoord.x - 2*x, vTexCoord.y + 2*y)).rgb;
    vec3 b = texture(uSrcTexture, vec2(vTexCoord.x,       vTexCoord.y + 2*y)).rgb;
    vec3 c = texture(uSrcTexture, vec2(vTexCoord.x + 2*x, vTexCoord.y + 2*y)).rgb;
                     
    vec3 d = texture(uSrcTexture, vec2(vTexCoord.x - 2*x, vTexCoord.y)).rgb;
    vec3 e = texture(uSrcTexture, vec2(vTexCoord.x,       vTexCoord.y)).rgb;
    vec3 f = texture(uSrcTexture, vec2(vTexCoord.x + 2*x, vTexCoord.y)).rgb;
                     
    vec3 g = texture(uSrcTexture, vec2(vTexCoord.x - 2*x, vTexCoord.y - 2*y)).rgb;
    vec3 h = texture(uSrcTexture, vec2(vTexCoord.x,       vTexCoord.y - 2*y)).rgb;
    vec3 i = texture(uSrcTexture, vec2(vTexCoord.x + 2*x, vTexCoord.y - 2*y)).rgb;
                     
    vec3 j = texture(uSrcTexture, vec2(vTexCoord.x - x, vTexCoord.y + y)).rgb;
    vec3 k = texture(uSrcTexture, vec2(vTexCoord.x + x, vTexCoord.y + y)).rgb;
    vec3 l = texture(uSrcTexture, vec2(vTexCoord.x - x, vTexCoord.y - y)).rgb;
    vec3 m = texture(uSrcTexture, vec2(vTexCoord.x + x, vTexCoord.y - y)).rgb;

    downsample = e * 0.125;
    downsample += (a + c + g + i) * 0.03125;
    downsample += (b + d + f + h) * 0.0625;
    downsample += (j + k + l + m) * 0.125;
}