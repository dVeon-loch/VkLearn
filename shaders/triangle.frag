#version 450

layout(location = 0) out vec4 outColor;

// The input variable does not necessarily have to use the same name, 
// they will be linked together using the indexes specified by the location directives
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(binding = 1) uniform sampler2D texSampler;

void main() 
{
    outColor = texture(texSampler, fragTexCoord * 2.0);
}