#version 410 core

in vec4 worldPos;
in vec4 worldNormal;

layout(location=0) out vec4 out0;
layout(location=1) out vec4 out1;
layout(location=2) out vec4 out2;

//take in the world normal and position and save into the color attachments

uniform vec4 materialColor;         // keeps diffuse color and monochromatic spec color

void main(){

    out0 = worldPos;
    out1 = worldNormal;
    out2 = materialColor;
}
