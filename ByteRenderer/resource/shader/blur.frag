#version 410 core

out float FragColor;

in vec2 vTexCoord;

uniform sampler2D uSrcTexture;
uniform sampler2D uDepth;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(uSrcTexture, 0));

    float centerDepth = texture(uDepth, vTexCoord).r;
    float result = 0.0;
    float totalWeight = 0.0;

    float sigmaDepth = 0.1;

    for (int x = -2; x < 2; ++x) {
        for (int y = -2; y < 2; ++y) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            vec2 sampleCoord = vTexCoord + offset;

            float sampleDepth = texture(uDepth, sampleCoord).r;
            float sampleValue = texture(uSrcTexture, sampleCoord).r;

            float depthDiff = abs(centerDepth - sampleDepth);
            float weight = exp(- (depthDiff * depthDiff) / (2.0 * sigmaDepth * sigmaDepth));

            result += sampleValue * weight;
            totalWeight += weight;
        }
    }

    FragColor = result / totalWeight;
}
