#version 410

uniform mat4 cameraProjectionMatrix;
uniform mat4 cameraViewMatrix;
uniform mat4 modelMatrix;

uniform mat4 inverseTransposeModelMatrix;

in vec4  inPosition;
in vec4  inNormal;
in vec2  inTexcoord;

out vec4 varNormal;
out vec2 varTexcoord;

void main (void)
{
	gl_Position	= cameraProjectionMatrix * cameraViewMatrix * modelMatrix * inPosition;
    varNormal = inverseTransposeModelMatrix * inNormal;
    varTexcoord = inTexcoord;
}
