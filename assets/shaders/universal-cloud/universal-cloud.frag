#version 460 core

in vec3 vecpos;
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 viewPos;

uniform samplerCube cubeMap;
uniform int isSkybox;
uniform float time;

uniform vec3 objPosition;

struct GLSL_MTL_block {
    vec3 Ka;
    float _pad0;

    vec3 Kd;
    float _pad1;

    vec3 Ks;
    float _pad2;

    vec3 Ke;
    float _pad3;

    vec3 F0;
    float _pad4;

    vec2 map_Kd_scale;
    float Ns;
    float Ni;
    float d;
    int illum;
    int usesTexture;

    float _pad5;
};

layout(std430, binding = 5) readonly buffer mtl_blocks_buffer {
    GLSL_MTL_block mtlBlocks[];
};

struct GLSL_GLOBAL_BLOCK {
    vec4 sunColor;
    vec4 sunAmbientColor;
    int numLights;
    float sunIntensity;
    float _pad2[2];
};

layout(std430, binding = 6) readonly buffer global_block_buffer {
    GLSL_GLOBAL_BLOCK globalBlock;
};

struct GLSL_LIGHT_BLOCK {
    vec4 position;
    vec4 color;
    float intensity;
    float power;
    float _pad2[2];
};

layout(std430, binding = 7) readonly buffer light_blocks_buffer {
    GLSL_LIGHT_BLOCK lights[];
};

uniform int currentMTLBlock;

in vec3 scale;

in float angle;

uniform sampler2D ourTexture;

// Very fast noise
// Source: https://www.ronja-tutorials.com/post/024-white-noise/
float rand(vec3 co){
    return fract(sin(dot(co, vec3(12.9898, 78.233, 831.231))) * 43758.5453);
}

in vec2 TexCoord;

void main()
{
    GLSL_MTL_block material = mtlBlocks[currentMTLBlock];
    float distance = length(vecpos);
    float opacityFromDistance = pow(1.0f - distance, 0.8);

    float viewDirectionOpaqueness = max(1.0-2.0*distance, 2.0f * min(1.0f, max(0.0, angle)));

    // color/texture only
    vec4 texData = texture(ourTexture, TexCoord * material.map_Kd_scale);
    FragColor = vec4(texData.rgb, viewDirectionOpaqueness * texData.a * material.d);
    return;
}
