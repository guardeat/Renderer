#version 410 core

out vec4 oFragColor;

in vec2 vTexCoord;

uniform sampler2D uAlbedo;
uniform vec2 uScreenSize;
uniform float uGamma;

float luminance(vec3 color) {
    return dot(color, vec3(0.299, 0.587, 0.114));
}

vec3 tonemap(vec3 color) {
    return color / (color + vec3(1.0));
}

vec3 gammaCorrect(vec3 color, float gamma) {
    return pow(color, vec3(1.0 / gamma));
}

void main() {
    vec2 texel = 1.0 / uScreenSize;

    vec3 rgbNW = gammaCorrect(tonemap(texture(uAlbedo, vTexCoord + vec2(-texel.x, -texel.y)).rgb), uGamma);
    vec3 rgbNE = gammaCorrect(tonemap(texture(uAlbedo, vTexCoord + vec2( texel.x, -texel.y)).rgb), uGamma);
    vec3 rgbSW = gammaCorrect(tonemap(texture(uAlbedo, vTexCoord + vec2(-texel.x,  texel.y)).rgb), uGamma);
    vec3 rgbSE = gammaCorrect(tonemap(texture(uAlbedo, vTexCoord + vec2( texel.x,  texel.y)).rgb), uGamma);
    vec3 rgbM  = gammaCorrect(tonemap(texture(uAlbedo, vTexCoord).rgb), uGamma);

    float lumaNW = luminance(rgbNW);
    float lumaNE = luminance(rgbNE);
    float lumaSW = luminance(rgbSW);
    float lumaSE = luminance(rgbSE);
    float lumaM  = luminance(rgbM);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
    float range = lumaMax - lumaMin;

    if (range < 0.05) {
        oFragColor = vec4(rgbM, 1.0);
        return;
    }

    float dirX = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    float dirY =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    vec2 dir = normalize(vec2(dirX, dirY));
    dir = clamp(dir, -2.0, 2.0) * texel;

    vec3 rgbA = 0.5 * (
        gammaCorrect(tonemap(texture(uAlbedo, vTexCoord + dir * (1.0 / 3.0 - 0.5)).rgb), uGamma) +
        gammaCorrect(tonemap(texture(uAlbedo, vTexCoord + dir * (2.0 / 3.0 - 0.5)).rgb), uGamma)
    );

    vec3 rgbB = 0.25 * (
        gammaCorrect(tonemap(texture(uAlbedo, vTexCoord + dir * -0.5).rgb), uGamma) +
        gammaCorrect(tonemap(texture(uAlbedo, vTexCoord + dir *  0.5).rgb), uGamma)
    ) + 0.5 * rgbA;

    float lumaB = luminance(rgbB);
    if (lumaB < lumaMin || lumaB > lumaMax)
        oFragColor = vec4(rgbA, 1.0);
    else
        oFragColor = vec4(rgbB, 1.0);
}
