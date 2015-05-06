#version 410

uniform sampler2D litImage;    // color attachments from the finalPass fbo
uniform sampler2D positions;
uniform vec3 eyePos;            // eye position in world space
uniform vec3 playerPos;            // player position in world space
uniform vec2 viewport;
uniform bool usingFog;

out vec4 fragColor;

void main(){

    vec2 tCoord = gl_FragCoord.xy/viewport;
    vec4 image = texture(litImage, tCoord);
    vec4 position = texture(positions, tCoord);

    float distance = length(eyePos - position.xyz);
    float camToPlayer = length(eyePos - playerPos);
//    vec4 fogColor = vec4(.7,.7,.98,1);
    vec4 fogColor = vec4(.27,.27,.32,1);

    float interpVal;

    if(usingFog){
        // clear color
        if(!(position.w>.99 && position.w<1.1)){
            fragColor = fogColor;
            return;
        }

        // exponential interpolation
        float b = .05;
        interpVal = 1.0/(exp(b*distance)) + .2;
        clamp(interpVal, 0.0, 1.0);

        fragColor = vec4((image*(interpVal) + fogColor*(1.0-interpVal)).xyz,1);
    }
    else
        fragColor = image;
}
