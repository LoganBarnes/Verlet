#version 410 core

in vec2 texc;
in vec3 tnt;

out vec4 fragColor;

uniform mat4 view;
uniform vec4 color;

uniform sampler2D tex;
uniform int useTexture = 0;
uniform float transparency = 1.0;

void main(){

    if (useTexture == 1)
    {
        vec3 texColor = texture(tex, texc).rgb;
        texColor = clamp(texColor + vec3(1-useTexture), vec3(0.0), vec3(1.0));
        fragColor = vec4(texColor * tnt, transparency);
    }
    else if (useTexture == 0)
        fragColor = color;
    else
        fragColor = vec4(1.);
//    fragColor = vec4(1, 0, 0, 1);
}
