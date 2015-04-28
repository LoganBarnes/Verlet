#version 410
in vec3 position; // Position of the vertex
in vec3 normal;
in vec2 texC;

void main()
{
    gl_Position =  vec4(position, 1.0);          //render as full screen quad
}

