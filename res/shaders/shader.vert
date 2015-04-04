#version 410 core

in vec3 position; // Position of the vertex
in vec3 normal;   // Normal of the vertex
in vec2 texCoord; // UV texture coordinates

out vec3 color; // Computed color for this vertex
out vec2 texc;

// Transformation matrices
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

// Light data
const int MAX_LIGHTS = 10;
uniform int lightTypes[MAX_LIGHTS];         // 0 for point, 1 for directional
uniform vec3 lightPositions[MAX_LIGHTS];    // pos for point lights dir for direction
uniform vec3 lightAttenuations[MAX_LIGHTS]; // Constant, linear, and quadratic term
uniform vec3 lightColors[MAX_LIGHTS];

uniform vec3 diffuse_color;
uniform vec3 world_color = vec3(0.2);
uniform vec3 specular_color = vec3(1.0);
uniform float shininess = 1.0;
uniform float transparency = 1.0;

uniform vec2 subImages = vec2(1.);
uniform vec2 subPos = vec2(0.);
uniform vec2 repeatUV = vec2(1.0);

uniform vec3 allBlack = vec3(1.0);
uniform vec3 allWhite = vec3(0.0);

void main(){

    texc = texCoord * repeatUV;

    vec4 position_cameraSpace = view * model * vec4(position, 1.0);
    vec4 normal_cameraSpace = vec4(normalize(mat3(transpose(inverse(view * model))) * normal), 0.0);

    gl_Position = projection * position_cameraSpace;

    color = world_color * diffuse_color; // Add ambient component

    for (int i = 0; i < MAX_LIGHTS; i++) {
        vec4 vertexToLight;
        // Point Light
        if (lightTypes[i] == 0)
            vertexToLight = normalize(view * vec4(lightPositions[i], 1) - position_cameraSpace);
        // Directional Light
        else if (lightTypes[i] == 1)
            vertexToLight = normalize(view * vec4(-lightPositions[i], 0));
        else
            continue;

        if (transparency < 1.0)
            color = diffuse_color;
        else
        {
            // Add diffuse component
            float diffuseIntensity = max(0.0, dot(vertexToLight, normal_cameraSpace));
            color += max(vec3(0), lightColors[i] * diffuse_color * diffuseIntensity);

            // Add specular component
            if (abs(shininess) > 0.001)
            {
                vec4 lightReflection = normalize(-reflect(vertexToLight, normal_cameraSpace));
                vec4 eyeDirection = normalize(vec4(0,0,0,1) - position_cameraSpace);
                float specIntensity = pow(max(0.0, dot(eyeDirection, lightReflection)), shininess);
                color += max (vec3(0), lightColors[i] * specular_color * specIntensity);
            }
        }
    }
    color = clamp(color + allWhite, 0.0, 1.0) * allBlack;
}
