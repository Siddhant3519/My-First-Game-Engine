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
    opaqueAccumulatedDepths[screenPos]      =   accumulatedDepth.r < currentPassDepth.r ? currentPassDepth : accumulatedDepth;
}