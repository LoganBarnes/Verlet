#version 410 core

// Responsible for the lighting calculations on diffuse and specular light
in vec2 uv;
in vec3 worldPos;

layout(location=0) out vec4 out0;

uniform sampler2D positions;    // color attachments from the geometry FBO
uniform sampler2D normals;
uniform vec3 playerPos;            // player position in world space
uniform vec3 eyePos;               // eye position in world space

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
uniform vec3 worldColor = vec3(0.2);

uniform vec2 viewport;


float intersectPlayer(vec3 p, vec3 d){

    float t1=-1;
    float t2=-1;

    float playerRad = .750;
    //vec3 playerPos = vec3(0,3,0);

    //quadratic terms in world space
    float a = d.x*d.x + d.y*d.y + d.z*d.z;
    float b = 2.0*(p.x-playerPos.x)*d.x + 2.0*(p.y-playerPos.y)*d.y + 2.0*(p.z-playerPos.z)*d.z;
    float c = playerPos.x*playerPos.x + playerPos.y*playerPos.y + playerPos.z*playerPos.z + p.x*p.x + p.y*p.y + p.z*p.z + -2.0*(playerPos.x*p.x + playerPos.y*p.y + playerPos.z*p.z) - (playerRad*playerRad);

    if((b*b - 4.0*a*c)>=0){
        t1 = (-b + sqrt(b*b - 4.0*a*c))/(2.0*a);
        t2 = (-b - sqrt(b*b - 4.0*a*c))/(2.0*a);
    }

    if (t1*t2 > 0)
        return min(t1,t2);
    else
        return max(t1,t2);
}



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


        // shadows block

        if (i == 0){        // shadows only enabled on character light

            vec3 p = position_worldSpace.xyz;
            vec3 dir = vertexToLight.xyz;

//            float tValToLight;
//            if (dir.x != 0)
//                tValToLight = (lightPositions[0].x - p.x)/dir.x;
//            else
//                tValToLight = (lightPositions[0].y - p.y)/dir.y;
//            if (tVal > 0 && shininess != 3.0 && tVal < tValToLight){
//                continue;
//            }

            float tVal = intersectPlayer(p+dir*.001, dir);

            if (tVal > 0 && shininess != 3.0){
                // if tval accurate, get the point that it goes to and the distance between
                vec3 endPt = p + tVal*dir;
                float distToPlayer = length(endPt - p);
                if (distToPlayer < d){      //if player closer than the light, ignore this light
                    continue;
                }
            }
        }
        // end shadows block



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

    //ka, kd, and ks removed
    // ambient
    vec3 ambient = worldColor;

    // diffuse
    vec3 diffuse = diffLight.xyz*diffColor;

    // specular
    vec3 specular = specLight.xyz*specColor;

    // final
    vec3 finalColor = ambient+diffuse+specular;
    finalColor = clamp(finalColor + vec3(0.0), 0.0, 1.0) * vec3(1.0);

    out0 = vec4(finalColor,1);

}
