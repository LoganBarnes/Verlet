#version 410 core

// Read in diffuse and spec light and combine with material properties/global settings to complete lighting model
in vec2 uv;

out vec4 fragColor;

uniform sampler2D diffuseLights;    // color attachments from the geometry FBO
uniform sampler2D specularLights;
uniform sampler2D materialColors;   // diffuse color + monochromatic spec

uniform vec2 viewport;

// global coefficients
uniform vec3 globalConstants;       // ka,kd,ks
uniform vec3 worldColor = vec3(0.2);

void main(){

    vec2 tCoord = gl_FragCoord.xy/viewport;

    vec4 diffLight = texture(diffuseLights,tCoord);
    vec4 specLight = texture(specularLights,tCoord);
    vec4 materialColor = texture(materialColors, tCoord);

//    // allow for diffuse lighting of non spaces
//    if(materialColor.x==0 && materialColor.y==0 && materialColor.z==0)
//        materialColor = vec4(1,1,1,1);

    vec3 diffColor = materialColor.xyz;
    vec3 specColor = vec3(materialColor.w);

    // ambient term:
    vec3 ambient = worldColor*globalConstants.x;

    // diffuse term:
    vec3 diffuse = diffLight.xyz*diffColor*globalConstants.y;

    // specular term:
    vec3 specular = specLight.xyz*specColor*globalConstants.z;

    // final
    vec3 finalColor = ambient+diffuse+specular;

    finalColor = clamp(finalColor + vec3(0.0), 0.0, 1.0) * vec3(1.0);

    fragColor = vec4(finalColor,1);
}
