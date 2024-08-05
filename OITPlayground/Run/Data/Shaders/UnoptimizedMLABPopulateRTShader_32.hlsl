//--------------------------------------------------------------------------------------------------
static const unsigned int NUM_OF_NODES      =   32;
static const unsigned int MAX_DEFAULT_VALUE =   255;


//--------------------------------------------------------------------------------------------------
cbuffer ScreenConstants : register(b5)
{
    float SCREEN_WIDTH;
}


//--------------------------------------------------------------------------------------------------
struct Fragment
{
    float4 premultipliedColor;
    float  transmission;
    float  depth;
};


//--------------------------------------------------------------------------------------------------
struct FragmentArray
{
    Fragment fragmentsPerPixel[NUM_OF_NODES];
};


//--------------------------------------------------------------------------------------------------
StructuredBuffer<FragmentArray> blendingArray : register(t0);


//--------------------------------------------------------------------------------------------------
RWTexture2D<float4> mainRT                  : register(u0);
RWTexture2D<uint>   untouchedFragmentMask   : register(u1);


//--------------------------------------------------------------------------------------------------
[numthreads(8, 8, 1)]
void ComputeMain(uint3 DTid : SV_DispatchThreadID)
{
    uint2  texelCoords          =   uint2(DTid.xy);
    bool   isFragmentUntouched  =   untouchedFragmentMask[texelCoords];
    float3 backgroundColor      =   mainRT[texelCoords].rgb;
    float3 color                =   backgroundColor;
    
    if (!isFragmentUntouched)
    {
        uint            offsetAddress               =   (SCREEN_WIDTH * texelCoords.y + texelCoords.x);
        FragmentArray   currentTexelBlendingArray   =   blendingArray[offsetAddress];

        for (int fragIndex = NUM_OF_NODES - 1; fragIndex >= 0; --fragIndex)
        {
            Fragment currentFrag = currentTexelBlendingArray.fragmentsPerPixel[fragIndex];
            if (currentFrag.depth != float(MAX_DEFAULT_VALUE))
            {
                color = currentFrag.premultipliedColor.rgb + (color * currentFrag.transmission);
            }
        }
        
        untouchedFragmentMask[texelCoords] = true;
    }
    
    mainRT[texelCoords] = float4(color, 1.f);
}