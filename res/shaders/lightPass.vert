#version 410 core

in vec3 position; // Position of the vertex
in vec3 normal;   // Normal of the vertex
in vec2 texCoord; // UV texture coordinates

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform bool inLight;

//layout (location = 0) out vec2 uv;
//layout (location = 0) out vec3 camPos;

void main()
{
    uv = texCoord;
    worldPos = position;

    if(inLight)
        gl_Position =  vec4(position,1.0);          //render as full screen quad
    else
        gl_Position = projection*view*model*vec4(position,1.0);
}
