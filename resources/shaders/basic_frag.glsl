#version 460
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;

layout(location = 0) out vec4 finalFragColor;

layout(set = 2, binding = 0) uniform sampler2D texSampler;

void main()
{
    finalFragColor = vec4(fragColor, 1.0) * texture(texSampler, fragUV);
}

