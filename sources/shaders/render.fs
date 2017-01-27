#version 410

in vec3 f_posView;
in vec3 f_normView;
in vec2 f_texcoord;
in vec3 f_lightPos;

out vec4 out_color;

uniform vec3 u_diffColor;
uniform vec3 u_specColor;

uniform sampler2D u_diffuseMap;
uniform sampler2D u_specularMap;
uniform sampler2D u_bumpMap;
uniform float u_shininess;

uniform bool u_hasDiffuseTex;
uniform bool u_hasSpecularTex;
uniform bool u_hasBumpTex;

float EPS = 1.0e-8;

void main(void) {
    vec3 V = normalize(-f_posView);
    vec3 N = normalize(f_normView);
    vec3 L = normalize(f_lightPos - f_posView);
    vec3 H = normalize(V + L);

    float ndotl = max(0.0, dot(N, L));
    float ndoth = max(0.0, dot(N, H));

    vec3 diffColor = u_diffColor;
    if (u_hasDiffuseTex) {
        diffColor = texture(u_diffuseMap, f_texcoord).rgb;
    }

    vec3 specColor = u_specColor;
    if (u_hasSpecularTex) {
        specColor = texture(u_specularMap, f_texcoord).rgb;
    }

    out_color.rgb = diffColor * ndotl + specColor * pow(ndoth + EPS, u_shininess);
    out_color.a   = 1.0;
}
