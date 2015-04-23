#version 410

uniform sampler2D litImage;    // color attachments from the finalPass fbo
uniform sampler2D positions;
uniform vec3 eyePos;            // eye position in world space
uniform vec2 viewport;
uniform bool usingFog;

out vec4 fragColor;

void main(){

    vec2 tCoord = gl_FragCoord.xy/viewport;
    vec4 image = texture(litImage, tCoord);
    vec4 position = texture(positions, tCoord);

    // get the difference between eye and world space pos
    float distance = length(eyePos - position.xyz);
    vec4 fogColor = vec4(.6,.6,.7,1);

    // clear color
    if(!(position.w>.99 && position.w<1.1)){
        fragColor = fogColor;
        return;
    }

    float interpVal;

    if(usingFog){

        // linear interpolation
    //    float fogStart = 2.f;
    //    float fogEnd = 20.f;
    //    interpVal = (fogEnd-distance)/(fogEnd-fogStart);

        // exponential interpolation
        float b = .05;
        interpVal = 1.0/(exp(b*distance));

        clamp(interpVal, 0.0, 1.0);

        fragColor = vec4((image*(interpVal) + fogColor*(1.0-interpVal)).xyz,1);
    }
    else
        fragColor = image;
}
