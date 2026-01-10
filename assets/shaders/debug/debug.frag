#version 330 core

in vec3 vecpos;
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 viewPos;
uniform vec3 lightPos;

void main()
{
    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
