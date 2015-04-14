#version 410 core

in vec3 position; // Position of the vertex
in vec3 normal;   // Normal of the vertex
in vec2 texCoord; // UV texture coordinates

//out vec3 color; // Computed color for this vertex
out vec4 position_cameraSpace;
out vec4 normal_cameraSpace;
out vec2 texc;

// Transformation matrices
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec2 repeatUV = vec2(1.0);

void main(){

    texc = texCoord * repeatUV;

    position_cameraSpace = view * model * vec4(position, 1.0);
    normal_cameraSpace = vec4(normalize(mat3(transpose(inverse(view * model))) * normal), 0.0);

    gl_Position = projection * position_cameraSpace;

}
