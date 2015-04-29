#version 410 core

layout(location=0) in vec4 camPos;
layout(location=1) in vec4 camNormal;

//in vec4 position_cameraSpace;
//in vec4 normal_cameraSpace;
//in vec2 texc;
//in mat4 viewMat;

layout(location=0) out vec4 out0;
layout(location=1) out vec4 out1;
layout(location=2) out vec4 out2;

//take in the world normal and position and save into the color attachments

uniform vec4 materialColor;         // keeps diffuse color and monochromatic spec color

void main(){

    out0 = camPos;
    out1 = camNormal;
    out2 = materialColor;
}
