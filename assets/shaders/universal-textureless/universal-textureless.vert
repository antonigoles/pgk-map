#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 vecpos;
out vec3 FragPos;
out vec3 Normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform int isCluster;

uniform int isSkybox;

struct SSBO_Transform {
    mat4 model;
};

layout(std430, binding = 0) readonly buffer TransformBuffer {
    SSBO_Transform transformData[];
};

out vec3 TexCoords;

void main()
{
    mat4 _model = mat4(1.0f);
    if (isCluster == 1) {
        _model = transformData[gl_InstanceID].model;
    } else {
        _model = model;
    }
    gl_Position = projection * view * _model * vec4(aPos, 1.0);
    FragPos = vec3(_model * vec4(aPos, 1.0)); 
    vecpos = aPos;
    Normal = mat3(transpose(inverse(mat3(_model)))) * aNormal;
}