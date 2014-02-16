#version 410

uniform sampler2D samplerColor;
uniform sampler2D samplerNormal;

uniform mat4 inverseTransposeModelMatrix;

in vec4 varTexcoord;
in float varAmbientOcclusion;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;

void main (void)
{
    outColor    = texture(samplerColor, varTexcoord.st, 0.0);
    outNormal   = texture(samplerNormal, varTexcoord.st, 0.0);
    vec4 normal = inverseTransposeModelMatrix * (outNormal * 2 - 1);
    float light = dot(normalize(normal.xyz), normalize(vec3(1,3,2)));
    light       = light * 0.67 + 0.33 * varAmbientOcclusion;
	outColor   *= light;
}