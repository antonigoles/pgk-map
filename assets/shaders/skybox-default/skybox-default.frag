#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

in vec3 camPos;

uniform samplerCube skybox;

void main()
{   
    FragColor = mix(vec4(0.5, 0.8, 1.0, 1.0), texture(skybox, TexCoords), (camPos.y - 100000) / 1000.0);
}

