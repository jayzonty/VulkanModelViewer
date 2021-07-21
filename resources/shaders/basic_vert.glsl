#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;

layout(set = 0, binding = 0) uniform FrameUBO
{
    mat4 view;
    mat4 proj;
} frameUBO;

struct ObjectData
{
    mat4 model;
};

layout(std140, set = 1, binding = 0) readonly buffer ObjectBuffer
{
    ObjectData data[];
} objectBuffer;

void main()
{
    mat4 modelMatrix = objectBuffer.data[gl_BaseInstance].model;
    gl_Position = frameUBO.proj * frameUBO.view * modelMatrix * vec4(position, 1.0);
    fragColor = color;
    fragUV = uv;
}

