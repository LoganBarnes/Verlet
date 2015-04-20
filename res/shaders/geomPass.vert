#version 410 core

in vec3 position; 	//position of vertex in obj space
in vec3 normal; 	//normal of vertex in obj space
in vec2 texCoord;       // UV texture coordinates

// Transformation matrices
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform float shininess; 		//alpha channel in normal

out vec4 worldPos; 
out vec4 worldNormal; 


void main()
{
    //save and output position and normals in world space for lighting 
    worldPos = model * vec4(position, 1.0); 
    worldNormal = vec4(normalize(mat3(transpose(inverse(model))) * normal),shininess);
    
    gl_Position = projection * view * worldPos;
}
