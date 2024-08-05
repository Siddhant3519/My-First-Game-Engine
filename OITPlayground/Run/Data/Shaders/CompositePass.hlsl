//-----------------------------------------------------------------------------------------------------
struct vs_input_t
{
    float3 localPosition    : POSITION;
    float4 color            : COLOR;
    float2 uv               : TEXCOORD;
};


//-----------------------------------------------------------------------------------------------------
struct v2p_t
{
    float4 position     : SV_Position;
    float4 color        : COLOR;
    float2 uv           : TEXCOORD;
};


//-----------------------------------------------------------------------------------------------------
cbuffer CameraConstants : register(b2)
{
    float4x4 projectionMatrix;
    float4x4 viewMatrix;
};


//-----------------------------------------------------------------------------------------------------
cbuffer ModelConstants : register(b3)
{
    float4x4    modelMatrix;
    float4      modelColor;
};


//-----------------------------------------------------------------------------------------------------
Texture2D accumulationTex   : register(t1);
Texture2D revealageTex      : register(t2);


//-----------------------------------------------------------------------------------------------------
SamplerState diffuseSampler : register(s0);


//-----------------------------------------------------------------------------------------------------
float EPSILON = 0.00001f;


//-----------------------------------------------------------------------------------------------------
bool IsApproximatelyEqual(float a, float b)
{
    return (abs(a - b) <= (abs(a) < abs(b) ? abs(b) : abs(a)) * EPSILON);
}


//-----------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
    v2p_t v2p;
    float4 localPosition    =   float4(input.localPosition, 1);
    float4 worldPosition    =   mul(modelMatrix, float4(input.localPosition, 1));
    float4 viewPosition     =   mul(viewMatrix, worldPosition);
    float4 clipPosition     =   mul(projectionMatrix, viewPosition);
    v2p.position            =   clipPosition;
    v2p.color               =   input.color * modelColor;
    v2p.uv                  =   input.uv;
    return v2p;
}


//-----------------------------------------------------------------------------------------------------
float4 PixelMain(v2p_t input) : SV_Target0
{
    float revealage = revealageTex.Sample(diffuseSampler, input.uv).r;
    if (IsApproximatelyEqual(revealage, 1.f))
    {
        discard;
    }
    
    float4 accumulation = accumulationTex.Sample(diffuseSampler, input.uv);
    if (isinf(max(max(accumulation.r, accumulation.g), accumulation.b)))
    {
        accumulation.rgb = float3(accumulation.a, accumulation.a, accumulation.a);
    }
    
    float3 compositedColor = accumulation.rgb / max(accumulation.a, EPSILON);
    
    float4 finalColor = float4(compositedColor, 1.f - revealage);
    return finalColor;
}