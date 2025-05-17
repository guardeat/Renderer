#version 410 core

layout (location = 0) out vec3 upsample;

uniform sampler2D uSrcTexture;
uniform float uFilterRadius;

in vec2 vTexCoord;

void main()
{
    float x = uFilterRadius;
    float y = uFilterRadius;

    vec3 a = texture(uSrcTexture, vec2(vTexCoord.x - x, vTexCoord.y + y)).rgb;
    vec3 b = texture(uSrcTexture, vec2(vTexCoord.x,     vTexCoord.y + y)).rgb;
    vec3 c = texture(uSrcTexture, vec2(vTexCoord.x + x, vTexCoord.y + y)).rgb;

    vec3 d = texture(uSrcTexture, vec2(vTexCoord.x - x, vTexCoord.y)).rgb;
    vec3 e = texture(uSrcTexture, vec2(vTexCoord.x,     vTexCoord.y)).rgb;
    vec3 f = texture(uSrcTexture, vec2(vTexCoord.x + x, vTexCoord.y)).rgb;

    vec3 g = texture(uSrcTexture, vec2(vTexCoord.x - x, vTexCoord.y - y)).rgb;
    vec3 h = texture(uSrcTexture, vec2(vTexCoord.x,     vTexCoord.y - y)).rgb;
    vec3 i = texture(uSrcTexture, vec2(vTexCoord.x + x, vTexCoord.y - y)).rgb;

    upsample = e * 4.0;
    upsample += (b + d + f + h) * 2.0;
    upsample += (a + c + g + i);
    upsample *= 1.0 / 16.0;
}
