#version 410 core

layout (location = 0) out vec3 downsample;

uniform sampler2D uSrcTexture;
uniform vec2 uSrcResolution;
uniform float uInvGamma;
uniform bool uKarisAvarage;

in vec2 vTexCoord;

vec3 PowVec3(vec3 v, float p)
{
    return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

vec3 toSRGB(vec3 v)   {
    return PowVec3(v, uInvGamma); 
}

float sRGBToLuma(vec3 col)
{
	return dot(col, vec3(0.299f, 0.587f, 0.114f));
}

float karisAverage(vec3 col)
{
	float luma = sRGBToLuma(toSRGB(col)) * 0.25f;
	return 1.0f / (1.0f + luma);
}

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

    if(uKarisAvarage){
        vec3 groups[5];
        groups[0] = (a+b+d+e) * (0.125f/4.0f);
	    groups[1] = (b+c+e+f) * (0.125f/4.0f);
	    groups[2] = (d+e+g+h) * (0.125f/4.0f);
	    groups[3] = (e+f+h+i) * (0.125f/4.0f);
	    groups[4] = (j+k+l+m) * (0.5f/4.0f);
	    groups[0] *= karisAverage(groups[0]);
	    groups[1] *= karisAverage(groups[1]);
	    groups[2] *= karisAverage(groups[2]);
	    groups[3] *= karisAverage(groups[3]);
	    groups[4] *= karisAverage(groups[4]);
	    downsample = groups[0]+groups[1]+groups[2]+groups[3]+groups[4];
        downsample = max(downsample, 0.0001f);
    }

    else{
        downsample = e * 0.125;
        downsample += (a + c + g + i) * 0.03125;
        downsample += (b + d + f + h) * 0.0625;
        downsample += (j + k + l + m) * 0.125;
        downsample = max(downsample, 0.0001f);
    }
}