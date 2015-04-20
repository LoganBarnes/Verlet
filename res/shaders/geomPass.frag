#version 410 core

in vec4 worldPos; 
in vec4 worldNormal; 

//take in the world normal and position and save into the color attachments 

void main(){

    gl_FragData[0] = worldPos;                                      //color attachment 0 has pos
    gl_FragData[1] = worldNormal;                                   //color attachment 1 has normal & shininess
}

