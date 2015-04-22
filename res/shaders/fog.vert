#version 410
in vec3 position; // Position of the vertex

void main()
{
    gl_Position =  vec4(position,1.0);          //render as full screen quad
}

