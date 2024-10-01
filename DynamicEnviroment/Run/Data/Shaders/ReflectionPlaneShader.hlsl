//------------------------------------------------------------------------------------------------
struct vs_input_t
{
	float3 localPosition	:	POSITION;
	float4 color			:	COLOR;
	float2 uv				:	TEXCOORD;
};


//------------------------------------------------------------------------------------------------
struct v2p_t
{
	float4 clipPosition		:	SV_Position;
	float4 worldPosition	:	POSITION;
	float4 color			:	COLOR;
	float2 uv				:	TEXCOORD;
};


//--------------------------------------------------------------------------------------------------
cbuffer CameraConstants : register(b2)
{
	float4x4 ViewToClipTransformator;
	float4x4 WorldToViewTransformator;
};


//--------------------------------------------------------------------------------------------------
cbuffer ModelConstants : register(b3)
{
	float4x4 LocalToWorldTransformator;
	float4   ModelColor;
};


//--------------------------------------------------------------------------------------------------
cbuffer GameConstants : register(b6)
{
    float	    TimeElapsedSeconds;
	float3	    DirToLight;
    //----------------------------------- (16 byte aligned)
    float4x4    CamRotationMatrix;
    //----------------------------------- (16 byte aligned)
    float3      CamPosition;
    float       padding1;
    //----------------------------------- (16 byte aligned)
	float2	    ScreenRes;
	float2	    padding2;
    //----------------------------------- (16 byte aligned)
	float4		SkyColor;
    //----------------------------------- (16 byte aligned)
};


//--------------------------------------------------------------------------------------------------
TextureCube<float4> CubeMap : register(t0);

SamplerState DiffuseSampler : register(s0);


//--------------------------------------------------------------------------------------------------
float GetFractionWithinRange(float value, float rangeStart, float rangeEnd)
{
    float currentRange = rangeEnd - rangeStart;
    float relativeValPos = (value - rangeStart) / currentRange;
    return relativeValPos;
}


//--------------------------------------------------------------------------------------------------
float RangeMap(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
    float relativeInValPos = GetFractionWithinRange(inValue, inStart, inEnd);
    float outRange = outEnd - outStart;
    float rangeMappedVal = (relativeInValPos * outRange) + outStart;
    return rangeMappedVal;
}


//--------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
	v2p_t v2p;
    float4 localPosition		=	float4(input.localPosition, 1); 
	// float  localPositionDisp	=	sin(localPosition.y + TimeElapsedSeconds * 1.5f);
	// localPositionDisp			=	RangeMap(localPositionDisp, -1.f, 1.f, 0.f, 0.5f);
	// localPosition				=	float4((localPosition.xyz + float3(0.f, 0.f, 1.f) * localPositionDisp), 1.f);
	float4 worldPosition		=	mul(LocalToWorldTransformator, localPosition);
	float4 viewPosition			=	mul(WorldToViewTransformator, worldPosition);
	float4 clipPosition			=	mul(ViewToClipTransformator, viewPosition);
	
    v2p.clipPosition			=	clipPosition;
	v2p.worldPosition			=	worldPosition;
	v2p.uv						=	input.uv;
	v2p.color					=	input.color;
	return v2p;
}


//--------------------------------------------------------------------------------------------------
float3 LinearTosRGB(float3 colorToBeConverted)
{
    float3 needsToBeCorrected = colorToBeConverted <= float3(0.0031308f, 0.0031308f, 0.0031308f);
    float3 v1 = colorToBeConverted * 12.92f;
    float3 v2 = pow(colorToBeConverted.xyz, float3(0.41666f, 0.41666f, 0.41666f)) * 1.055f - float3(0.055f, 0.055f, 0.055f);
	
    return lerp(v2, v1, needsToBeCorrected);
}


//--------------------------------------------------------------------------------------------------
[earlydepthstencil]
float4 PixelMain(v2p_t input) : SV_Target0
{
	// Colors
    float3 color			=	float3(0.f, 0.f, 0.f);
	// float3 fragWorldPos		=	float3(-input.worldPosition.y, input.worldPosition.z, input.worldPosition.x);
	float3 fragWorldPos		=	input.worldPosition.xyz;
	// // Calculate normal using screen space derivatives
	// float3 normal			=	normalize(cross(ddx(fragWorldPos), ddy(fragWorldPos)));
	float3 normal			=	 float3(0.f, 1.f, 0.f);
	
	// Diffuse lighting
    float3 lightDir			=	normalize(float3(1.f, 1.f, -1.f));
    float3 lightColor		=	float3(1.f, 1.f, 0.9f);
	float  lightIntensity	=	max(0.f, dot(lightDir, normal));
	float3 diffuse			=	lightColor * lightIntensity;
	
	// Phong specular
	// Todo, swizzle cam position and world position
	// float3 camPos			=	float3(-CamPosition.y, CamPosition.z, CamPosition.x);
    float3 dirToCam			=	normalize(CamPosition - fragWorldPos);
	float3 worldHalfVec		=	normalize(lightDir + dirToCam);
	float  phongVal			=	max(0.f, dot(dirToCam, worldHalfVec));
	phongVal				=	pow(phongVal, 32.f);
	float3 specular			=	float3(phongVal.xxx);
	
	// IBL specular
    float3 iblReflectionDir		=	normalize(reflect(-dirToCam, normal));
	float3 reflectionColor		=	CubeMap.Sample(DiffuseSampler, -iblReflectionDir).xyz;
    specular					+=	reflectionColor * 0.85f;
	
	// Fresnel 
    float3 fresnel				=	1.f - max(0.f, dot(dirToCam, normal));
	fresnel						=	pow(fresnel, 2.f);
	specular					*=	fresnel;
	
	
	float3 ambient				=	float3(0.5f, 0.5f, 0.5f);
	float3 baseColor			=	input.color.rgb;
	float3 lighitng				=	diffuse * 1.f;
    color						=	baseColor * lighitng + specular;
	color						=	reflectionColor;
	
    // float d1	=	length(input.worldPosition.xyz - float3(10.f, 0.f, 0.f)) - 1.f;
    // float d2	=	length(input.worldPosition.xyz - float3(0.f, 10.f, 0.f)) - 1.f;
    // color		=	lerp(float3(1.f, 0.f, 0.f), float3(0.f.xxx), smoothstep(0.f, 0.1f, d1));
    // color		=	lerp(float3(0.f, 1.f, 0.f), color, smoothstep(0.f, 0.1f, d2));
	
	// Color correction
	// float  colorCorrectionExp	=	1.f / 2.2f;
    // color						=	pow(color, float3(colorCorrectionExp.xxx));
	return float4(color, 1.f);
}