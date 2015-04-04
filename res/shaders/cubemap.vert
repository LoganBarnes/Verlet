#version 410 core

in vec3 position;

uniform mat4 projection;
uniform mat4 view;

out vec3 texc;

#define PI 3.14159265358979

mat4 rotationMatrix(in vec3 axis, in float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    return mat4(oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s, 0.0,
                oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s, 0.0,
                oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c, 0.0,
                0.0, 0.0, 0.0, 1.0);
}

void main () {
//        texc = vec3(rotationMatrix(vec3(0, 1, 0), -PI / 2.0) * vec4(position, 1.0));
        texc = position.xyz;
	mat3 v = mat3(view);

	gl_Position = projection * mat4(v) * vec4(position, 1.0);
	//gl_Position = projection * view * vec4(position, 1.0);
}
