[[FX]]

// Supported Flags
/* ---------------
	_F01_Skinning
	_F02_NormalMapping
	_F03_ParallaxMapping
	_F04_EnvMapping
	_F05_AlphaTest
*/


// Samplers
sampler2D normalMap = sampler_state
{
	Texture = "textures/common/defnorm.tga";
};

samplerCube ambientMap = sampler_state
{
	Address = Clamp;
	Filter = Bilinear;
	MaxAnisotropy = 1;
};

float4 myColor;

context TRANSLUCENT
{
	VertexShader = compile GLSL VS_GENERAL;
	PixelShader = compile GLSL FS_TRANSLUCENT;

	BlendMode = Blend;
	CullMode = None;
}	

context SOLID
{
	VertexShader = compile GLSL VS_GENERAL;
	PixelShader = compile GLSL FS_SOLID;
}

context DYNAMIC
{
	VertexShader = compile GLSL VS_GENERAL;
	PixelShader = compile GLSL FS_DYNAMIC;
}

[[VS_GENERAL]]
// =================================================================================================

#ifdef _F03_ParallaxMapping
	#define _F02_NormalMapping
#endif

#include "shaders/utilityLib/vertCommon.glsl"

#ifdef _F01_Skinning
	#include "shaders/utilityLib/vertSkinning.glsl"
#endif

uniform mat4 viewProjMat;
uniform vec3 viewerPos;
attribute vec3 vertPos;
attribute vec2 texCoords0;
attribute vec3 normal;

#ifdef _F02_NormalMapping
	attribute vec4 tangent;
#endif

varying vec4 pos, vsPos;
varying vec2 texCoords;

#ifdef _F02_NormalMapping
	varying mat3 tsbMat;
#else
	varying vec3 tsbNormal;
#endif
#ifdef _F03_ParallaxMapping
	varying vec3 eyeTS;
#endif


void main( void )
{
#ifdef _F01_Skinning
	mat4 skinningMat = calcSkinningMat();
	mat3 skinningMatVec = getSkinningMatVec( skinningMat );
#endif
	
	// Calculate normal
#ifdef _F01_Skinning
	vec3 _normal = normalize( calcWorldVec( skinVec( normal, skinningMatVec ) ) );
#else
	vec3 _normal = normalize( calcWorldVec( normal ) );
#endif

	// Calculate tangent and bitangent
#ifdef _F02_NormalMapping
	#ifdef _F01_Skinning
		vec3 _tangent = normalize( calcWorldVec( skinVec( tangent.xyz, skinningMatVec ) ) );
	#else
		vec3 _tangent = normalize( calcWorldVec( tangent.xyz ) );
	#endif
	
	vec3 _bitangent = cross( _normal, _tangent ) * tangent.w;
	tsbMat = calcTanToWorldMat( _tangent, _bitangent, _normal );
#else
	tsbNormal = _normal;
#endif

	// Calculate world space position
#ifdef _F01_Skinning	
	pos = calcWorldPos( skinPos( vec4( vertPos, 1.0 ), skinningMat ) );
#else
	pos = calcWorldPos( vec4( vertPos, 1.0 ) );
#endif

	vsPos = calcViewPos( pos );

	// Calculate tangent space eye vector
#ifdef _F03_ParallaxMapping
	eyeTS = calcTanVec( viewerPos - pos.xyz, _tangent, _bitangent, _normal );
#endif
	
	// Calculate texture coordinates and clip space position
	texCoords = texCoords0;
	gl_Position = viewProjMat * pos;
}


[[FS_SOLID]]	
// =================================================================================================


#include "shaders/utilityLib/fragLighting.glsl" 

uniform sampler2D albedoMap;
uniform samplerCube ambientMap;
uniform vec4 myColor;

varying vec4 pos;
varying vec2 texCoords;
varying vec3 tsbNormal;
uniform vec4 customInstData[4];


void main( void )
{
	vec3 normal = tsbNormal;

	gl_FragColor.rgb = myColor.xyz * textureCube( ambientMap, normal ).rgb;
	
}

[[FS_DYNAMIC]]	
// =================================================================================================


#include "shaders/utilityLib/fragLighting.glsl" 

uniform sampler2D albedoMap;
uniform samplerCube ambientMap;

varying vec4 pos;
varying vec2 texCoords;
varying vec3 tsbNormal;
uniform vec4 customInstData[4];


void main( void )
{
	vec3 normal = tsbNormal;

	gl_FragColor.rgb = customInstData[0].xyz * textureCube( ambientMap, normal ).rgb;
	
}

[[FS_TRANSLUCENT]]	
// =================================================================================================

uniform sampler2D albedoMap;
uniform samplerCube ambientMap;
uniform vec4 customInstData[4];

varying vec4 pos;
varying vec2 texCoords;

varying vec3 tsbNormal;

void main( void )
{
	gl_FragColor = customInstData[0];
}
