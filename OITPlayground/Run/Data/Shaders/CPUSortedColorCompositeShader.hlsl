//--------------------------------------------------------------------------------------------------
Texture2D<float4> sourceRT : register(t0);


//--------------------------------------------------------------------------------------------------
RWTexture2D<float4> destRT : register(u0);


//--------------------------------------------------------------------------------------------------
[numthreads(8, 8, 1)]
void ComputeMain(uint3 DTid : SV_DispatchThreadID)
{
    uint2  screenPos            =   uint2(DTid.xy);
    float4 sourceColor          =   sourceRT[screenPos];
    if (sourceColor.a == 1)
    {
        return;
    }
    float4 destColor            =   destRT[screenPos];
    float4 compositedColor      =   float4(sourceColor.rgb + ((1.f - sourceColor.a) * destColor.rgb), 1.f);
    destRT[screenPos]           =   compositedColor;
}