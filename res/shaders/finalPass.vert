#version 410 core

in vec3 position; // Position of the vertex
in vec3 normal;   // Normal of the vertex
in vec2 texCoord; // UV texture coordinates

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 uv;

void main()
{
    uv = texCoord;
    gl_Position = projection*view*model*vec4(position,1.0);
}
