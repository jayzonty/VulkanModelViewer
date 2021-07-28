#version 460
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;

layout(location = 0) out vec4 finalFragColor;

layout(set = 2, binding = 0) uniform sampler2D emissiveMap;
layout(set = 3, binding = 0) uniform sampler2D diffuseMap;

void main()
{
    vec4 emission = texture(emissiveMap, fragUV);
    finalFragColor = emission + vec4(fragColor, 1.0) * texture(diffuseMap, fragUV);
}
