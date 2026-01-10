#version 460 core

in vec3 vecpos;
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 TexCoords;

out vec4 FragColor;

uniform vec3 viewPos;

// struct Light {
//     vec3 position;
//     vec3 color;
//     float intensity;
//     float power;
// };

// uniform int numLights;
// uniform Light lights[8];
// uniform float sunIntensity;
// uniform vec3 sunColor;
// uniform vec3 sunAmbientColor;
// uniform int usesTexture;

// struct MTL_block {
//     float Ns;
//     vec3 Ka;
//     vec3 Kd;
//     vec3 Ks;
//     vec3 Ke;
//     vec3 F0;
//     float Ni;
//     float d;
//     int illum;
//     vec2 map_Kd_scale;
// };

// uniform MTL_block material;

uniform sampler2D ourTexture;
uniform samplerCube cubeMap;
uniform int isSkybox;


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

void main()
{
    GLSL_MTL_block material = mtlBlocks[currentMTLBlock];
    
    int numLights = globalBlock.numLights;
    float sunIntensity = globalBlock.sunIntensity;
    vec3 sunColor = vec3(globalBlock.sunColor);
    vec3 sunAmbientColor = vec3(globalBlock.sunAmbientColor);
    int usesTexture = material.usesTexture;

    if (isSkybox == 1) {
        FragColor = texture(cubeMap, TexCoords);
        return;
    }

    vec3 sunPos = FragPos + vec3(0, 150, 0);

    // TODO: Compile multiple versions for different illumination models
    if (material.illum == 0) {
        // color/texture only
        vec4 texData = texture(ourTexture, TexCoord * material.map_Kd_scale);
        FragColor = vec4(texData.rgb, texData.a * material.d);
        return;
    } else if (material.illum == 1) {
        // Diffuse only 

        vec3 N = normalize(Normal);
        vec3 color = vec3(0.0);

        for (int i = 0; i < numLights; i++) 
        {
            vec3 L = normalize(vec3(lights[i].position) - FragPos);

            float diff = max(dot(N, L), 0.0);
            vec3 diffuse = material.Kd * diff * vec3(lights[i].color) * lights[i].intensity;

            color += diffuse;
        }

        // include Sun
        color += material.Kd * max(dot(N, vec3(0, -1, 0)), 0.0) * sunColor * sunIntensity;

        vec4 texData = texture(ourTexture, TexCoord * material.map_Kd_scale);
        color *= texData.rgb;

        FragColor = vec4(color, texData.a * material.d);
        return;
    } else if (material.illum >= 2) {
        vec3 N = normalize(Normal);
        vec3 V = normalize(viewPos - FragPos);

        // --- Ambient: tylko raz ---
        vec3 ambient = material.Ka * sunAmbientColor; 

        vec3 color = ambient;

        for (int i = 0; i < numLights; i++)
        {
            float intensity = lights[i].intensity * pow(1 / length(vec3(lights[i].position) - FragPos), lights[i].power);
            vec3 L = normalize(vec3(lights[i].position) - FragPos);

            // Diffuse
            float diff = max(dot(N, L), 0.0);
            vec3 diffuse = material.Kd * diff * vec3(lights[i].color) * intensity;

            // Specular (Blinn-Phong)
            vec3 H = normalize(L + V);
            float spec = pow(max(dot(N, H), 0.0), material.Ns + 0.00001f);
            vec3 specular = material.Ks * spec * vec3(lights[i].color) * intensity;

            color += diffuse + specular;
        }

        // Include Sun
        vec3 L = normalize(sunPos - FragPos);
        // Diffuse
        float diff = max(dot(N, L), 0.0);
        vec3 diffuse = material.Kd * diff * sunColor * sunIntensity;
        // Specular (Blinn-Phong)
        vec3 H = normalize(L + V);
        float spec = pow(max(dot(N, H), 0.0), material.Ns + + 0.00001f);
        vec3 specular = material.Ks * spec * sunColor * sunIntensity;
        // Calculate Ir if reflections enabled
        color += diffuse + specular;

        float effectiveOpacity = material.d;

        if (material.illum >= 3) {
            vec3 I = normalize(FragPos - viewPos);
            vec3 R = normalize(reflect(I, N));
            vec3 reflectionStrength = material.Ks;
            if (material.illum == 5) {
                // use Fresnel with Schlick approximation 
                float cos_theta = dot(N, V);
                float power_term = pow(1.0 - cos_theta, 5.0);
                vec3 R_fresnel = material.F0 + (vec3(1.0) - material.F0) * power_term;
                reflectionStrength *= R_fresnel * material.Ns; // ??
                effectiveOpacity += max(max(reflectionStrength.x, reflectionStrength.y), reflectionStrength.z);
            }
            color = mix(color, texture(cubeMap, R).rgb, reflectionStrength);
        }

        if (usesTexture == 1) {
            vec4 texData = texture(ourTexture, TexCoord * material.map_Kd_scale);
            color *= texData.rgb;
            effectiveOpacity *= texData.a;
        }
         
        FragColor = vec4(color, effectiveOpacity);
        return;
    }

    // Magenta - blad materialu
    FragColor = vec4(1.0, 0.0, 1.0, 1.0);
}
