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
struct ps_output_t
{
    float4  accumulationRT      : SV_Target0;
    float   revealageRT         : SV_Target1;
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
Texture2D diffuseTexture : register(t0);

SamplerState diffuseSampler : register(s0);


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
[earlydepthstencil]
ps_output_t PixelMain(v2p_t input)
{
    float4 diffuseColor     =   diffuseTexture.Sample(diffuseSampler, input.uv);
    float4 fragmentColor    =   diffuseColor * input.color;
    float weight            =   clamp(pow(min(1.0, fragmentColor.a * 10.0) + 0.01, 3.0) * 100000000.0 * pow(1.0 - input.position.z * 0.9, 3.0), 0.01, 3000.0);
    // float weight = pow(fragmentColor.a, 5.0f) * clamp(0.3 / (1e-5 + pow(input.position.z / 5, 500.0f)), 1e-2, 3e3);
    
    ps_output_t output;
    output.accumulationRT   =   float4(fragmentColor.rgb * fragmentColor.a, fragmentColor.a) * weight;
    output.revealageRT      =   fragmentColor.a;
    return output;
}