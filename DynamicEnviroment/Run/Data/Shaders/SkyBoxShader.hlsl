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
	float3 directionVec		:	TEXCOORD;
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
TextureCube<float4> CubeMap	: register(t0);

SamplerState diffuseSampler : register(s0);


//--------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
	v2p_t v2p;
    float4 localPosition	=	float4(input.localPosition, 1);
	float4 worldPosition	=	mul(LocalToWorldTransformator, localPosition);
	float4 viewPosition		=	mul(WorldToViewTransformator, worldPosition);
	float4 clipPosition		=	mul(ViewToClipTransformator, viewPosition);
	
    v2p.clipPosition		=	clipPosition.xyww;
	v2p.directionVec		=	input.localPosition;
	return v2p;
}


//--------------------------------------------------------------------------------------------------
[earlydepthstencil]
float4 PixelMain(v2p_t input) : SV_Target0
{
	// Sample cubemap
	// // Swizzle directionVec for future proof
    float4 color = CubeMap.Sample(diffuseSampler, normalize(input.directionVec));
    return color;
}