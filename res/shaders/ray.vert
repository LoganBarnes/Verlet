#version 410 core

in vec3 position; // Position of the vertex

out vec3 pos;

void main()
{
    pos = vec3(position.xy, -1);
    gl_Position = vec4(position.xy, 0, 1);
}
