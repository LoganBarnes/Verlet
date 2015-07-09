#version 410 core

// Responsible for the lighting calculations on diffuse and specular light

in vec2 uv;
in vec3 worldPos;

layout(location=0) out vec4 out0;
//layout(location=1) out vec4 out1;

//out vec4 fragColor;

uniform sampler2D positions;    // color attachments from the geometry FBO
uniform sampler2D normals;

uniform vec3 eyePos;            // eye position in world space

// Light data
uniform int numLights;
const int MAX_LIGHTS = 50;
uniform int lightTypes[MAX_LIGHTS];         // 0 for point, 1 for directional
uniform vec3 lightPositions[MAX_LIGHTS];    // pos for point lights dir for direction
uniform vec3 lightAttenuations[MAX_LIGHTS]; // Constant, linear, and quadratic term
uniform vec3 lightColors[MAX_LIGHTS];

// Material data
uniform sampler2D materialColors;   // diffuse color + monochromatic spec
// global coefficients
uniform vec3 globalConstants;       // ka,kd,ks
uniform vec3 worldColor = vec3(0.2);

uniform vec2 viewport;
uniform int mode;

// fog stuff:
uniform vec3 playerPos;            // player position in world space
uniform bool usingFog;

void main(){

    vec2 tCoord = gl_FragCoord.xy/viewport;

    vec4 position_worldSpace = texture(positions,tCoord);
    vec4 normal_worldSpace = texture(normals,tCoord);
    vec4 materialColor = texture(materialColors, tCoord);

    float shininess = normal_worldSpace.w;
    vec3 diffColor = materialColor.xyz;
    vec3 specColor = vec3(materialColor.w);

    vec3 diffLight = vec3(0,0,0);
    vec3 specLight = vec3(0,0,0);


    for(int i=0; i< numLights; i++){

        float atten = 0.0;
        float d;
        vec4 vertexToLight;
        // Point Light
        if (lightTypes[i] == 0){
            vertexToLight = vec4(lightPositions[i],1.0) - vec4(position_worldSpace.xyz,1.0);
            d = length(vertexToLight);
            vertexToLight = normalize(vertexToLight);
             atten = 1.0/(lightAttenuations[i].x + lightAttenuations[i].y*d + lightAttenuations[i].z*d*d);
        }
        // Directional Light
        else if (lightTypes[i] == 1){
            vertexToLight = normalize(vec4(-lightPositions[i], 0));
            atten = 1.0;
        }

        float diffuseIntensity = max(0.0, dot(vertexToLight, normal_worldSpace));
        diffLight += max(vec3(0), atten*((normal_worldSpace.xyz * .2 + lightColors[i]) * diffuseIntensity)); // colors influenced by normals

        // specular componenent
        if (abs(shininess) > 0.001)
        {
            vec4 lightReflection = normalize(-reflect(vertexToLight, vec4(normal_worldSpace.xyz,0)));
            vec4 eyeDirection = normalize(vec4(eyePos,1) - position_worldSpace);
            float specIntensity = clamp(pow(max(0.0, dot(eyeDirection, lightReflection)), shininess), 0.0,1);

            specLight += max (vec3(0), atten*(lightColors[i] * specIntensity));
        }

    }
    //done with light

    // Material properties:

    // ambient
    vec3 ambient = worldColor*globalConstants.x;

    // diffuse
    vec3 diffuse = diffLight.xyz*diffColor*globalConstants.y;

    // specular
    vec3 specular = specLight.xyz*specColor*globalConstants.z;

    // final
    vec3 finalColor = ambient+diffuse+specular;
    finalColor = clamp(finalColor + vec3(0.0), 0.0, 1.0) * vec3(1.0);

//    fragColor = vec4(finalColor,1);

    out0 = vec4(finalColor,1);

}
