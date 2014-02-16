#version 410

uniform mat4 cameraProjectionMatrix;
uniform mat4 cameraViewMatrix;
uniform mat4 modelMatrix;

uniform mat4 textureMatrix;

layout(location = 0) in vec4 inPosition;
layout(location = 8) in vec4 inTexcoord;
layout(location = 4) in float inAmbientOcclusion;

out vec4 varTexcoord;
out float varAmbientOcclusion;

void main (void)
{
	gl_Position	= cameraProjectionMatrix * cameraViewMatrix * modelMatrix * inPosition;
    varTexcoord = textureMatrix * inTexcoord;
    varAmbientOcclusion = inAmbientOcclusion;
}
