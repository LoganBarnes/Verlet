#version 410 core

in vec3 position; // Position of the vertex
in vec3 normal;   // Normal of the vertex
in vec2 texCoord; // UV texture coordinates

out vec2 uv;

void main()
{
    uv = texCoord;
    gl_Position = vec4(position,1.0);
}

