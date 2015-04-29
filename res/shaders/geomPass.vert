#version 410 core

layout (location = 0) in vec3 position; //position of vertex in obj space
layout (location = 1) in vec3 normal; 	//normal of vertex in obj space
layout (location = 2) in vec2 texCoord; // UV texture coordinates

// Transformation matrices
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 invModel = mat3(1,0,0,
                             0,1,0,
                             0,0,1);

uniform float shininess; 		//alpha channel in normal

out mat4 viewMat;
out vec4 worldPos;
out vec4 worldNormal;

out vec4 position_cameraSpace;
out vec4 normal_cameraSpace;
out vec2 texc;

uniform vec2 repeatUV = vec2(1.0);

void main()
{
    texc = texCoord * repeatUV;

    viewMat = view;

    position_cameraSpace = view * model * vec4(position, 1.0);
    normal_cameraSpace = vec4(normalize(mat3(transpose(inverse(view * model))) * normal), 0.0);

    //save and output position and normals in world space for lighting 
    worldPos = model * vec4(position, 1.0); 
    worldNormal = vec4(normalize(invModel * normal),shininess);
    
    gl_Position = projection * view * worldPos;
}
