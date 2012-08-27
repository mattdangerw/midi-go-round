[[FX]]

// Samplers
samplerCube albedoMap = sampler_state
{
	Address = Clamp;
};

context SOLID
{
	VertexShader = compile GLSL VS_GENERAL;
	PixelShader = compile GLSL FS_SOLID;
}


[[VS_GENERAL]]
// =================================================================================================

#include "shaders/utilityLib/vertCommon.glsl"

uniform mat4 viewProjMat;
uniform vec3 viewerPos;
attribute vec3 vertPos;
varying vec3 viewVec;

void main(void)
{
	vec4 pos = calcWorldPos( vec4( vertPos, 1.0 ) );
	viewVec = pos.xyz - viewerPos;
	
	gl_Position = viewProjMat * pos;
}

[[FS_SOLID]]
// =================================================================================================

uniform samplerCube albedoMap;
varying vec3 viewVec;

void main( void )
{
	vec3 sampleVec = vec3(viewVec.y, -1 * viewVec.z, -1 * viewVec.x);
	vec3 albedo = textureCube( albedoMap, sampleVec ).rgb;
	
	gl_FragColor.rgb = albedo;
}
