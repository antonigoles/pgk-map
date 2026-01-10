#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 vecpos;
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 projection;
uniform mat4 view;

struct SSBO_Transform {
    mat4 model;
};

layout(std430, binding = 0) readonly buffer TransformBuffer {
    SSBO_Transform transformData[];
};

void main()
{
    mat4 model = transformData[gl_InstanceID].model;
    // mat4 model = mat4(1.0f);

    // model[0][0] = _model[0][0];
    // model[1][1] = _model[1][1];
    // model[2][2] = _model[2][2];
    // model[3][3] = _model[3][3];

    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPos = vec3(model * vec4(aPos, 1.0)); 
    vecpos = aPos;
    Normal = mat3(transpose(inverse(mat3(model)))) * aNormal;
    TexCoord = aTexCoord;
}