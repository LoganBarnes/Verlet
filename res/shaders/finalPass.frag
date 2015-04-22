#version 410 core

// Read in diffuse and spec light and combine with material properties/global settings to complete lighting model
in vec2 uv;

uniform sampler2D diffuseLights;    // color attachments from the geometry FBO
uniform sampler2D specularLights;

uniform vec2 viewport;

// global coefficients
uniform vec3 globalConstants;       // ka,kd,ks
uniform vec3 worldColor = vec3(0.2);

// material properties
uniform vec3 cDiffuse, cSpec;
uniform sampler2D tex;

uniform bool usingFog;

out vec4 fragColor;

void main(){

    vec2 tCoord = gl_FragCoord.xy/viewport;

    vec4 diffLight = texture(diffuseLights,tCoord);
    vec4 specLight = texture(specularLights,tCoord);

    // ambient term:
    vec3 ambient = worldColor*globalConstants.x;

    // diffuse term:
    vec3 diffuse = diffLight.xyz*cDiffuse*globalConstants.y;

    // specular term:
    vec3 specular = specLight.xyz*cSpec*globalConstants.z;

    // final
    vec3 finalColor = ambient + diffuse + specular;
    finalColor = clamp(finalColor + vec3(0.0), 0.0, 1.0) * vec3(1.0);

//    if(usingFog)
//        gl_FragData[0] = vec4(finalColor,1);
//    else
        fragColor = vec4(finalColor,1);
}
