#version 460 core

in vec3 vecpos;
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 viewPos;

uniform samplerCube cubeMap;
uniform int isSkybox;
uniform float time;

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

    vec3 sunPos = FragPos + vec3(0, 150, 0);

    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);

    // 1. Konfiguracja Wyspy
    float islandRadius = 30.0;  // Promień, gdzie zaczyna się woda (brzeg)
    float dist = length(FragPos.xz); // Odległość pixela od środka świata (0,0)
    vec2 dirFromCenter = normalize(FragPos.xz); // Wektor kierunku OD środka wyspy

    // 2. Tłumienie fal przy brzegu (Shoreline Damping)
    // Fale są zerowe na promieniu wyspy i rosną do 100% siły przez kolejne 20 metrów
    float shoreFactor = clamp((dist - islandRadius) / 20.0, 0.0, 1.0);

    // Jeśli jesteśmy wewnątrz wyspy (pod ziemią), shoreFactor wyzeruje fale.
    
    // 3. Fale Główne (Radial Swell) - Kręgi rozchodzące się od/do wyspy
    // Używamy 'dist' zamiast 'x' czy 'z' w sinusie.
    // '- time' oznacza fale wychodzące od środka. '+ time' oznacza fale uderzające w brzeg.
    float waveRadial = sin(dist * 0.3 - time * 2.0);

    // 4. Fale Szumu (Micro Details) - Żeby woda nie wyglądała jak tarcza strzelnicza
    // Mieszamy zwykłe sinusy w osiach X/Z dla chaosu
    float waveNoise = sin(FragPos.x * 0.9 + time) * cos(FragPos.z * 0.8 + time * 1.2);

    // 5. Złożenie fal
    // Główna fala radialna + szum, wszystko przemnożone przez bliskość brzegu
    float combinedWave = (waveRadial * 1.0 + waveNoise * 0.4) * shoreFactor;

    // 6. Siła zaburzenia normalnej
    float waveStrength = 0.15; // Jak mocno wyginamy światło

    // 7. Aplikacja do normalnej
    // Zamiast dodawać do X i Z losowo, dodajemy zgodnie z wektorem promienia (dirFromCenter)
    // To sprawia, że światło "ślizga się" po kręgach wokół wyspy
    N.x += dirFromCenter.x * combinedWave * waveStrength;
    N.z += dirFromCenter.y * combinedWave * waveStrength;
    
    // Dodatkowy szum, żeby zbić idealną symetrię
    N.x += cos(FragPos.z * 2.0 + time) * 0.05 * shoreFactor;
    N.z += sin(FragPos.x * 2.0 + time) * 0.05 * shoreFactor;

    N = normalize(N);

    vec3 color = material.Ka * sunAmbientColor;

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
    color += diffuse + specular;

    float effectiveOpacity = material.d;

    vec3 I = normalize(FragPos - viewPos);
    vec3 R = normalize(reflect(I, N));
    vec3 reflectionStrength = material.Ks;
    color = mix(color, texture(cubeMap, R).rgb, reflectionStrength);

    FragColor = vec4(color, effectiveOpacity);
}
