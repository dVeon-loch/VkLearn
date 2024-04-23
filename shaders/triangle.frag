#version 450

layout(location = 0) out vec4 outColor;

// The input variable does not necessarily have to use the same name, 
// they will be linked together using the indexes specified by the location directives
layout(location = 0) in vec3 fragColor;

void main() 
{
    outColor = vec4(fragColor, 1.0);
}