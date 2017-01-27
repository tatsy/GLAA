#version 410

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;

out vec3 f_posView;
out vec3 f_normView;
out vec2 f_texcoord;
out vec3 f_lightPos;

uniform mat4 u_mvMat;
uniform mat4 u_mvpMat;
uniform mat4 u_normMat;
uniform vec3 u_lightPos;

void main(void) {
    gl_Position = u_mvpMat * vec4(in_position, 1.0);
    f_posView = (u_mvMat * vec4(in_position, 1.0)).xyz;
    f_normView = (u_normMat * vec4(in_normal, 0.0)).xyz;
    f_texcoord = in_texcoord;
    f_lightPos = (u_mvMat * vec4(u_lightPos, 1.0)).xyz;
}
