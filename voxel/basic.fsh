#version 410

uniform sampler2D diffuseTexture;

in vec4  varNormal;
in vec2  varTexcoord;

out vec4 outColor;
out vec4 outNormal;

void main (void)
{
	outColor    = texture(diffuseTexture, varTexcoord.st, 0.0);
    outNormal   = normalize(vec4(varNormal.xyz, 0)) * 0.5 + 0.5;
}