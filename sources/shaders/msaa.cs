#version 450

#define AA_TYPE_NONE 0
#define AA_TYPE_SSAA 1
#define AA_TYPE_MSAA 2

uniform mat4 u_mvMat;
uniform mat4 u_normMat;
uniform vec3 u_lightPos;
uniform int u_aaType;
uniform int u_subsample;

layout(rgba32f, binding = 0) readonly uniform image2D positionMap;
layout(rgba32f, binding = 1) readonly uniform image2D normalMap;
layout(rgba32f, binding = 2) readonly uniform image2D diffuseMap;
layout(rgba32f, binding = 3) readonly uniform image2D specularMap;

layout(rgba8_snorm, binding = 4) writeonly uniform image2D renderTarget;

layout(local_size_x = 32, local_size_y = 32) in;

float EPS = 1.0e-8;

vec3 shading(ivec2 pixelPos) {
    vec3 position = imageLoad(positionMap, pixelPos).xyz;
    vec3 normal = imageLoad(normalMap, pixelPos).xyz;
    vec3 diffuse = imageLoad(diffuseMap, pixelPos).xyz;
    vec4 specular = imageLoad(specularMap, pixelPos);

    vec3 posView = (u_mvMat * vec4(position, 1.0)).xyz;
    vec3 normView = (u_normMat * vec4(normal, 0.0)).xyz;
    vec3 lightPosView = (u_mvMat * vec4(u_lightPos, 1.0)).xyz;

    vec3 V = normalize(-posView);
    vec3 N = normalize(normView);
    vec3 L = normalize(lightPosView - posView);
    vec3 H = normalize(V + L);

    float ndotl = max(0.0, dot(N, L));
    float ndoth = max(0.0, dot(N, H));

    vec3 rgb = diffuse * ndotl + specular.rgb * pow(ndoth + EPS, specular.a);
    return rgb;
}

vec3 shadingSSAA(ivec2 pixelPos) {
    vec3 rgb = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < u_subsample; i++) {
        for (int j = 0; j < u_subsample; j++) {
            ivec2 subpixel = pixelPos * u_subsample + ivec2(i, j);
            rgb += shading(subpixel);
        }
    }
    rgb /= (u_subsample * u_subsample);

    return rgb;
}

vec3 shadingMSAA(ivec2 pixelPos) {
    // Edge test
    float zValue = imageLoad(positionMap, pixelPos * u_subsample).w;
    bool isEdge = false;
    for (int i = 0; i < u_subsample; i++) {
        for (int j = 0; j < u_subsample; j++) {
            ivec2 subpixel = pixelPos * u_subsample + ivec2(i, j);
            float zSub = imageLoad(positionMap, subpixel).w;
            if (abs(zValue - zSub) > 1.0e-4) {
                isEdge = true;
                break;
            }
        }

        if (isEdge) {
            break;
        }
    }

    // Shading
    if (isEdge) {
        return shadingSSAA(pixelPos);
    } else {
        return shading(pixelPos * u_subsample);
    }
}

void main(void) {
    ivec2 pixelPos = ivec2(gl_GlobalInvocationID.xy);

    // Visibility test
    vec3 rgb;
    if (u_aaType == AA_TYPE_NONE) {
        rgb = shading(pixelPos * u_subsample);
    } else if (u_aaType == AA_TYPE_SSAA) {
        rgb = shadingSSAA(pixelPos);
    } else if (u_aaType == AA_TYPE_MSAA) {
        rgb = shadingMSAA(pixelPos);
    }
    imageStore(renderTarget, pixelPos, vec4(rgb, 1.0));
}
