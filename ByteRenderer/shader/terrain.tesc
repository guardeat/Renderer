#version 410 core

layout (vertices=4) out;

in vec2 vTexCoord[];

out vec2 vTexCoord_[];

uniform mat4 uView;

uniform vec3 uPosition;           
uniform vec3 uScale;
uniform vec4 uRotation;

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
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    vTexCoord_[gl_InvocationID] = vTexCoord[gl_InvocationID];

    if(gl_InvocationID == 0)
    {
        const int MIN_TESS_LEVEL = 4;
        const int MAX_TESS_LEVEL = 64;
        const float MIN_DISTANCE = 20;
        const float MAX_DISTANCE = 800;

        vec4 eyeSpacePos00 = uView * vec4(translate(gl_in[0].gl_Position.xyz, uPosition, uScale, uRotation), 1.0);
        vec4 eyeSpacePos01 = uView * vec4(translate(gl_in[1].gl_Position.xyz, uPosition, uScale, uRotation), 1.0);
        vec4 eyeSpacePos10 = uView * vec4(translate(gl_in[2].gl_Position.xyz, uPosition, uScale, uRotation), 1.0);
        vec4 eyeSpacePos11 = uView * vec4(translate(gl_in[3].gl_Position.xyz, uPosition, uScale, uRotation), 1.0);

        float distance00 = clamp((abs(eyeSpacePos00.z)-MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0);
        float distance01 = clamp((abs(eyeSpacePos01.z)-MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0);
        float distance10 = clamp((abs(eyeSpacePos10.z)-MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0);
        float distance11 = clamp((abs(eyeSpacePos11.z)-MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0);

        float tessLevel0 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance10, distance00) );
        float tessLevel1 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance00, distance01) );
        float tessLevel2 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance01, distance11) );
        float tessLevel3 = mix( MAX_TESS_LEVEL, MIN_TESS_LEVEL, min(distance11, distance10) );

        gl_TessLevelOuter[0] = tessLevel0;
        gl_TessLevelOuter[1] = tessLevel1;
        gl_TessLevelOuter[2] = tessLevel2;
        gl_TessLevelOuter[3] = tessLevel3;

        gl_TessLevelInner[0] = max(tessLevel1, tessLevel3);
        gl_TessLevelInner[1] = max(tessLevel0, tessLevel2);
    }
}