//--------------------------------------------------------------------------------------------------
Texture2D<float4> translucentRT : register(t0);


//--------------------------------------------------------------------------------------------------
RWTexture2D<float4> backgroundRT : register(u0);


//--------------------------------------------------------------------------------------------------
[numthreads(8, 8, 1)]
void ComputeMain(uint3 DTid : SV_DispatchThreadID)
{
    uint2  screenPos            =   uint2(DTid.xy);
    float4 sourceColor          =   backgroundRT[screenPos];
    float4 destColor            =   translucentRT[screenPos];
    float4 compositedColor;
    compositedColor.rgb         =   destColor.rgb + (destColor.a * sourceColor.rgb);
    compositedColor.a           =   1.f;
    backgroundRT[screenPos]     =   compositedColor;
}