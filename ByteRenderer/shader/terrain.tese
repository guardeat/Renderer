#version 410 core

layout (quads, fractional_odd_spacing, ccw) in;

uniform sampler2D uHeightMap;

uniform vec3 uPosition;           
uniform vec3 uScale;
uniform vec4 uRotation;

uniform mat4 uProjection;
uniform mat4 uView;

in vec2 vTexCoord_[];

out float vHeight;

vec3 rotateVertex( vec3 v, vec4 q ) {
    return v + 2.*cross( q.xyz, cross( q.xyz, v ) + q.w*v ); 
}

vec3 translateVertex(vec3 point, vec3 translation) {
    return point + translation;
}

vec3 scaleVertex(vec3 point, vec3 scaleFactor) {
    return point * scaleFactor;
}

vec3 translate(vec3 aPos, vec3 position, vec3 scale, vec4 rotation) {
    vec3 translatedPos = scaleVertex(aPos, scale);

    translatedPos = rotateVertex(translatedPos, rotation);

    translatedPos = translateVertex(translatedPos, position);

    return translatedPos;
}

void main()
{
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    vec2 t00 = vTexCoord_[0];
    vec2 t01 = vTexCoord_[1];
    vec2 t10 = vTexCoord_[2];
    vec2 t11 = vTexCoord_[3];

    vec2 t0 = (t01 - t00) * u + t00;
    vec2 t1 = (t11 - t10) * u + t10;
    vec2 texCoord = (t1 - t0) * v + t0;

    vHeight = texture(uHeightMap, texCoord).y * 64.0 - 16.0;

    vec4 p00 = gl_in[0].gl_Position;
    vec4 p01 = gl_in[1].gl_Position;
    vec4 p10 = gl_in[2].gl_Position;
    vec4 p11 = gl_in[3].gl_Position;

    vec4 uVec = p01 - p00;
    vec4 vVec = p10 - p00;
    vec4 normal = normalize( vec4(cross(vVec.xyz, uVec.xyz), 0) );

    vec4 p0 = (p01 - p00) * u + p00;
    vec4 p1 = (p11 - p10) * u + p10;
    vec4 p = (p1 - p0) * v + p0;

    p += normal * vHeight;

    gl_Position = uProjection * uView * vec4(p.xyz, 1.0);
}