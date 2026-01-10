#version 330 core

in vec3 vecpos;
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 viewPos;

struct Light {
    vec3 position;
    vec3 color;
    float intensity;
    float power;
};

uniform int numLights;
uniform Light lights[8];

uniform float sunIntensity;
uniform vec3 sunColor;
uniform vec3 sunAmbientColor;

uniform sampler2D ourTexture;

uniform int usesTexture;

struct TextureMap {
    vec2 scale;
};

struct MTL_block {
    float Ns;
    vec3 Ka;
    vec3 Kd;
    vec3 Ks;
    vec3 Ke;
    vec3 F0;
    float Ni;
    float d;
    int illum;
    vec2 map_Kd_scale;
};

uniform MTL_block material;

uniform samplerCube cubeMap;

void main()
{
    vec3 sunPos = FragPos + vec3(0, 150, 0);

    // TODO: Compile multiple versions for different illumination models
    if (material.illum == 1) {
        // Diffuse only 

        vec3 N = normalize(Normal);
        vec3 color = vec3(0.0);

        for (int i = 0; i < numLights; i++) 
        {
            vec3 L = normalize(lights[i].position - FragPos);

            float diff = max(dot(N, L), 0.0);
            vec3 diffuse = material.Kd * diff * lights[i].color * lights[i].intensity;

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
            float intensity = lights[i].intensity * pow(1 / length(lights[i].position - FragPos), lights[i].power);
            vec3 L = normalize(lights[i].position - FragPos);

            // Diffuse
            float diff = max(dot(N, L), 0.0);
            vec3 diffuse = material.Kd * diff * lights[i].color * intensity;

            // Specular (Blinn-Phong)
            vec3 H = normalize(L + V);
            float spec = pow(max(dot(N, H), 0.0), material.Ns + 0.00001f);
            vec3 specular = material.Ks * spec * lights[i].color * intensity;

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
                // use Fresnel with Schlick approximation - hard coded R0 
                float cos_theta = dot(N, V);
                float power_term = pow(1.0 - cos_theta, 5.0);
                vec3 R_fresnel = material.F0 + (vec3(1.0) - material.F0) * power_term;
                reflectionStrength *= R_fresnel * material.Ns; // ??
                effectiveOpacity *= max(max(reflectionStrength.x, reflectionStrength.y), reflectionStrength.z);
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
