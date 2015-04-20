#version 410

uniform sampler2D litImage;    // color attachments from the finalPass fbo
uniform sampler2D positions;
uniform vec3 eyePos;            // eye position in world space
uniform vec2 viewport;

out vec4 fragColor;

void main(){

    vec2 tCoord = gl_FragCoord.xy/viewport;
    vec4 image = texture(litImage,tCoord);
    vec4 position = texture(positions, tCoord);

//    fragColor = image;

            // get the difference between eye and world space pos
            float distance = length(eyePos - position.xyz);
            vec4 fogColor = vec4(.8,.8,1,1);

            //use distance as interpolator
            //greater distance should have more fog

            if(!(position.w>.99 && position.w<1.1)){
                fragColor = vec4(0,0,0,0);
                return;
            }

            float interpVal = pow(.95,distance);
            fragColor = vec4((image*(interpVal) + fogColor*(1.0-interpVal)).xyz,1);
//            fragColor = image;
}
