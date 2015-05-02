#version 410 core

//in vec3 color;
in mat4 viewMat;
in vec4 worldPos;
in vec4 worldNormal;

in vec4 position_cameraSpace;
in vec4 normal_cameraSpace;
in vec2 texc;

out vec4 fragColor;


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

uniform sampler2D tex;
uniform int useTexture = 0;

void main()
{
    vec4 norm_camSpace = normal_cameraSpace;
    if (!gl_FrontFacing)
        norm_camSpace = -normal_cameraSpace;

    vec3 diffCol = diffuse_color + norm_camSpace.xyz * .1;
//    vec3 diffCol = diffuse_color;
    vec3 color = world_color * diffCol; // Add ambient component

    for (int i = 0; i < MAX_LIGHTS; i++) {
        vec4 vertexToLight;
        // Point Light
        if (lightTypes[i] == 0)
            vertexToLight = normalize(viewMat * vec4(lightPositions[i], 1) - position_cameraSpace);
        // Directional Light
        else if (lightTypes[i] == 1)
            vertexToLight = normalize(viewMat * vec4(-lightPositions[i], 0));
        else
            continue;

        if (transparency < 1.0)
            color = diffuse_color;
        else
        {
            // Add diffuse component
            float diffuseIntensity = max(0.0, dot(vertexToLight, norm_camSpace));
            color += max(vec3(0), lightColors[i] * diffCol * diffuseIntensity);

            // Add specular component
            if (abs(shininess) > 0.001)
            {
                vec3 lightReflection = normalize(-reflect(vertexToLight, norm_camSpace).xyz);
                vec3 eyeDirection = normalize(-position_cameraSpace.xyz);
                float specIntensity = pow(max(0.0, dot(eyeDirection, lightReflection)), shininess);
                color += max (vec3(0), lightColors[i] * specular_color * specIntensity);
            }
        }
    }
    color = clamp(color + allWhite, 0.0, 1.0) * allBlack;

    vec3 texColor = texture(tex, texc).rgb;
    texColor = clamp(texColor + vec3(1-useTexture), vec3(0.0), vec3(1.0));
    fragColor = vec4(color * texColor, transparency);

}
