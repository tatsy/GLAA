#version 450

layout(location = 0) in vec3 f_position;
layout(location = 1) in vec3 f_normal;
layout(location = 2) in vec2 f_texcoord;
layout(location = 3) in float f_depth;

layout(location = 0) out vec4 out_position;
layout(location = 1) out vec4 out_normal;
layout(location = 2) out vec4 out_diffuse;
layout(location = 3) out vec4 out_specular;

uniform vec3 u_diffColor;
uniform vec3 u_specColor;

uniform sampler2D u_diffuseMap;
uniform sampler2D u_specularMap;
uniform sampler2D u_bumpMap;
uniform float u_shininess;

uniform bool u_hasDiffuseTex;
uniform bool u_hasSpecularTex;
uniform bool u_hasBumpTex;

void main(void) {
    out_position = vec4(f_position, f_depth);
    out_normal = vec4(f_normal, 1.0);

    if (u_hasDiffuseTex) {
        out_diffuse = texture(u_diffuseMap, f_texcoord);
    } else {
        out_diffuse = vec4(u_diffColor, 1.0);
    }

    if (u_hasSpecularTex) {
        out_specular.rgb = texture(u_specularMap, f_texcoord).rgb;
    } else {
        out_specular.rgb = u_specColor;
    }
    out_specular.a = u_shininess;
}
