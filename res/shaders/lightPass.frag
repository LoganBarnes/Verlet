#version 410 core

// Responsible for the lighting calculations on diffuse and specular light

in vec2 uv;

layout(location=0) out vec4 out0;
layout(location=1) out vec4 out1;

uniform sampler2D positions;    // color attachments from the geometry FBO
uniform sampler2D normals;

uniform vec3 eyePos;            // eye position in world space

// Light data
uniform int lightType;         // 0 for point, 1 for directional
uniform vec3 lightPosition;    // pos for point lights dir for direction
uniform vec3 lightAttenuation; // Constant, linear, and quadratic term
uniform vec3 lightColor;

uniform vec2 viewport;

void main(){

    vec2 tCoord = gl_FragCoord.xy/viewport;

    vec4 position_worldSpace = texture(positions,tCoord);
    vec4 normal_worldSpace = texture(normals,tCoord);
    float shininess = normal_worldSpace.w;

    vec3 diffLight = vec3(0,0,0);
    vec3 specLight = vec3(0,0,0);

    vec4 vertexToLight;
    // Point Light
    if (lightType == 0)
        vertexToLight = normalize(vec4(lightPosition,1) - position_worldSpace);
    // Directional Light
    else if (lightType == 1)
        vertexToLight = normalize(vec4(-lightPosition, 0));

    float diffuseIntensity = max(0.0, dot(vertexToLight, normal_worldSpace));
    diffLight += max(vec3(0), lightColor * diffuseIntensity);

    // specular componenent
    if (abs(shininess) > 0.001)
    {
        vec4 lightReflection = normalize(-reflect(vertexToLight, vec4(normal_worldSpace.xyz,0)));
        vec4 eyeDirection = normalize(vec4(eyePos,1) - position_worldSpace);
        float specIntensity = pow(max(0.0, dot(eyeDirection, lightReflection)), shininess);
        specLight += max (vec3(0), lightColor * specIntensity);
    }

    diffLight = clamp(diffLight, 0.0, 1.0);
    specLight = clamp(specLight, 0.0, 1.0);

//    gl_FragData[0] = vec4(diffLight,1);
//    gl_FragData[1] = vec4(specLight,1);
    out0 = vec4(diffLight, 1);
    out1 = vec4(diffLight, 1);

}
