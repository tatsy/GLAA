#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;

uniform mat4 u_mvpMat;

layout(location = 0) out vec3 f_position;
layout(location = 1) out vec3 f_normal;
layout(location = 2) out vec2 f_texcoord;
layout(location = 3) out float f_depth;

void main(void) {
    gl_Position = u_mvpMat * vec4(in_position, 1.0);

    f_position = in_position;
    f_normal = in_normal;
    f_texcoord = in_texcoord;
    f_depth = gl_Position.z / gl_Position.w;
}
