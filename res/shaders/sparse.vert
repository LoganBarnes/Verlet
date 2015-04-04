#version 410 core

in vec3 position; // Position of the vertex
in vec2 texCoord; // UV texture coordinates

out vec2 texc;
out vec3 tnt;


// Transformation matrices
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 tint = vec3(1.0);
uniform vec3 player;
uniform int playerMode = 0;

uniform vec2 subImages = vec2(1.);
uniform vec2 subPos = vec2(0.);
uniform vec2 repeatUV = vec2(1.0);

float rand(in vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main() {
    texc = texCoord * repeatUV;

    vec3 glow = vec3(0.0);
    vec4 posWorld = model * vec4(position, 1.0);
    float dist = distance(vec3(posWorld), player);

    if (playerMode != 0)
    {
        float intensity = 1.25;
        if (playerMode == 2)
            intensity = 2.5;
        float attenGlow = clamp(1.0 / (dist * .3), 0.0, 1.0);
        glow = (vec3(1, .5, 0) - tint * .5) * attenGlow * intensity;
    }
    float atten = clamp(1.0 / (dist * .02), 0.0, 1.0);
    tnt = tint * atten + glow;

    gl_Position = projection * view * model * vec4(position, 1.0);
}
