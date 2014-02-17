#version 410

uniform sampler2D samplerColor;
uniform sampler2D samplerNormal;
uniform sampler2D samplerShadow;

uniform mat4 modelMatrix;
uniform mat4 spotlightProjectionMatrix;
uniform mat4 spotlightViewMatrix;

in vec4 varTexcoord;
in float varAmbientOcclusion;
in vec4 varPosition;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;

void main (void)
{
    outColor    = texture(samplerColor, varTexcoord.st, 0.0);

    vec4 modelNormal   = vec4(texture(samplerNormal, varTexcoord.st, 0.0).xyz * 2 - 1, 0);
    vec4 worldNormal = modelMatrix * modelNormal;
    outNormal = worldNormal * 0.5 + 0.5;
    vec4 spotlightEyeNormal = spotlightViewMatrix * worldNormal;

    vec4 spotlightEyePosition = spotlightViewMatrix * varPosition;
    vec4 spotlightClipPosition = spotlightProjectionMatrix * spotlightEyePosition;
    vec4 spotlightNDCPosition = spotlightClipPosition / spotlightClipPosition.w;
    vec4 spotlightWindowPosition = spotlightNDCPosition * 0.5 + 0.5;
    
    bool lit = spotlightWindowPosition.z <= texture(samplerShadow, spotlightWindowPosition.st, 0.0).r;
    
    float incidence = clamp(dot(normalize(spotlightEyeNormal), normalize(-spotlightEyePosition)), 0, 1);
    
    outColor = lit ? vec4(1,1,1,1) * incidence : vec4(0,0,0,0);
    
    /*
    float light = clamp(dot(normalize(normal.xyz), normalize(vec3(1,3,2))), 0, 1);
    light       = light * 0.67 + 0.33 * varAmbientOcclusion;
	outColor   *= light;
    outColor *= texture(samplerShadow, )*/
}