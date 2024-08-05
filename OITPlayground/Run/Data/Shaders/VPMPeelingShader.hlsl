//--------------------------------------------------------------------------------------------------
struct vs_input_t
{
	float3 localPosition	:	POSITION;
	float4 color			:	COLOR;
	float2 uv				:	TEXCOORD;	
};


//--------------------------------------------------------------------------------------------------
struct v2p_t
{
	float4 position		:	SV_Position;
	float4 color		:	COLOR;
	float2 uv			:	TEXCOORD;
};


//--------------------------------------------------------------------------------------------------
cbuffer CameraConstants : register(b2)
{
	float4x4 viewToClipTransformator;
	float4x4 worldToViewTransformator;
};


//--------------------------------------------------------------------------------------------------
cbuffer ModelConstants : register(b3)
{
	float4x4 localToWorldTransformator;
	float4   modelColor;
};


//--------------------------------------------------------------------------------------------------
Texture2D diffuseTexture	: register(t0);
Texture2D depthTexture		: register(t1);

SamplerState diffuseSampler : register(s0);


//--------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
	v2p_t v2p;
    float4 localPosition	=	float4(input.localPosition, 1);
	float4 worldPosition	=	mul(localToWorldTransformator, localPosition);
	float4 viewPosition		=	mul(worldToViewTransformator, worldPosition);
	float4 clipPosition		=	mul(viewToClipTransformator, viewPosition);
	v2p.position			=	clipPosition;
	v2p.color				=	input.color * modelColor;
	v2p.uv					=	input.uv;
	return v2p;
}


//--------------------------------------------------------------------------------------------------
float4 PixelMain(v2p_t input) : SV_Target0
{
	// Calculate texel color
	float4 color		=	diffuseTexture.Sample(diffuseSampler, input.uv);
	color				*=	input.color;
	color				=   float4(color.rgb * color.a, color.a);
    
	// Discard the pixel if its further away than desired max depth
	uint2 texelCoords	=	uint2(input.position.xy);
	float maxDepth		=	depthTexture[texelCoords].r;
	bool isClipped		=	input.position.z >= maxDepth;
    if (isClipped)
    {
        discard;
    }
	
	return color;
}