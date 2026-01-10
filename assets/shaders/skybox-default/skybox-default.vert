#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;
out vec3 camPos;

uniform mat4 projection;
uniform mat4 view;

uniform vec3 cameraPosition;

void main()
{
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = vec4(pos.x, pos.y, 0.0, pos.w);
    camPos = cameraPosition;
}