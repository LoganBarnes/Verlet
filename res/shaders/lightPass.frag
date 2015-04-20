#version 410 core

// Responsible for the lighting calculations on diffuse and specular light

in vec2 uv;

uniform sampler2D positions;    // color attachments from the geometry FBO
uniform sampler2D normals;

uniform vec3 eyePos;            // eye position in world space

// Light data
const int MAX_LIGHTS = 10;
uniform int lightTypes[MAX_LIGHTS];         // 0 for point, 1 for directional
uniform vec3 lightPositions[MAX_LIGHTS];    // pos for point lights dir for direction
uniform vec3 lightAttenuations[MAX_LIGHTS]; // Constant, linear, and quadratic term
uniform vec3 lightColors[MAX_LIGHTS];

uniform vec2 viewport;

void main(){

    vec2 tCoord = gl_FragCoord.xy/viewport;

    vec4 position_worldSpace = texture(positions,tCoord);
    vec4 normal_worldSpace = texture(normals,tCoord);
    float shininess = normal_worldSpace.w;

    vec3 diffLight = vec3(0,0,0);
    vec3 specLight = vec3(0,0,0);

    for (int i = 0; i < MAX_LIGHTS; i++) {

        vec4 vertexToLight;
        // Point Light
        if (lightTypes[i] == 0)
            vertexToLight = normalize(vec4(lightPositions[i],1) - position_worldSpace);
        // Directional Light
        else if (lightTypes[i] == 1)
            vertexToLight = normalize(vec4(-lightPositions[i], 0));
        else
            continue;

        float diffuseIntensity = max(0.0, dot(vertexToLight, normal_worldSpace));
        diffLight += max(vec3(0), lightColors[i] * diffuseIntensity);

        // specular componenent
        if (abs(shininess) > 0.001)
        {
            vec4 lightReflection = normalize(-reflect(vertexToLight, vec4(normal_worldSpace.xyz,0)));
            vec4 eyeDirection = normalize(vec4(eyePos,1) - position_worldSpace);
            float specIntensity = pow(max(0.0, dot(eyeDirection, lightReflection)), shininess);
            specLight += max (vec3(0), lightColors[i] * specIntensity);
        }
    }

    gl_FragData[0] = vec4(diffLight,1);
    gl_FragData[1] = vec4(specLight,1);

}
