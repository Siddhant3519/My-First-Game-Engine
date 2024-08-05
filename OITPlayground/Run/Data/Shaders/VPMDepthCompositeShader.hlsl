//--------------------------------------------------------------------------------------------------
Texture2D<float4> translucentPassDepths : register(t0);


//--------------------------------------------------------------------------------------------------
RWTexture2D<float4> opaqueAccumulatedDepths : register(u0);


//--------------------------------------------------------------------------------------------------
[numthreads(8, 8, 1)]
void ComputeMain(uint3 DTid : SV_DispatchThreadID)
{
    uint2  screenPos                        =   uint2(DTid.xy);
    float4 currentPassDepth                 =   translucentPassDepths[screenPos];
    float4 accumulatedDepth                 =   opaqueAccumulatedDepths[screenPos];
    float4 updatedCurrentPassDepth          =   currentPassDepth.r > 0.f ? currentPassDepth : float4(1.f, 0.f, 0.f, 0.f);
    opaqueAccumulatedDepths[screenPos]      =   accumulatedDepth.r > updatedCurrentPassDepth.r ? updatedCurrentPassDepth : accumulatedDepth;
}