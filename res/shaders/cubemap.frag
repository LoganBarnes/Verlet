#version 410 core

in vec3 texc;

uniform samplerCube envMap;

out vec4 fragColor;

void main () {
        fragColor = texture(envMap, texc);
}
