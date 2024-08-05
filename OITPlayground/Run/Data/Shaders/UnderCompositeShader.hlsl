//--------------------------------------------------------------------------------------------------
Texture2D<float4> sourceRT : register(t0);


//--------------------------------------------------------------------------------------------------
RWTexture2D<float4> destRT : register(u1);


//--------------------------------------------------------------------------------------------------
[numthreads(8, 8, 1)]
void ComputeMain(uint3 DTid : SV_DispatchThreadID)
{
    uint2 screenPos = uint2(DTid.xy);
    
    float4 sourceColor  =   sourceRT[screenPos];
    float4 destColor    =   destRT[screenPos];

    float3 compositedColorRGB       =   float3(destColor.a * (sourceColor.rgb)) + destColor.rgb;
    float  compositedColorAlpha     =   1.f;
    destRT[screenPos]               =   float4(compositedColorRGB, compositedColorAlpha);
}