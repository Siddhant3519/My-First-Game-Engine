static const unsigned int NUM_OF_NODES = 4;


//--------------------------------------------------------------------------------------------------
cbuffer ScreenConstants : register(b8)
{
    float   SCREEN_WIDTH;
    float   pad0;
    uint    UINT_MAX;
    float   CAMERA_FAR;
};


//--------------------------------------------------------------------------------------------------
struct FragmentLinkedList
{
    unsigned int fragmentColor;
    unsigned int fragmentTransmission;
    unsigned int fragmentDepth;
    unsigned int nextFragmentNode;
};


//--------------------------------------------------------------------------------------------------
float4 GetRGBAFromRGBE(float4 rgbeColor)
{
    float4 rgbaColor = rgbeColor;
    rgbaColor.rgb    = float3(rgbeColor.rgb * exp2(rgbeColor.a - 127));
    return rgbaColor;
}

//--------------------------------------------------------------------------------------------------
float4 UnpackUintToFloatRgba(unsigned int packedUintColor)
{
    float4 unpackedFloat4Rgba;
    
    unsigned int a = (packedUintColor >> 24UL) & 0xFFUL;
    unsigned int b = (packedUintColor >> 16UL) & 0xFFUL;
    unsigned int g = (packedUintColor >> 8UL) & 0xFFUL;
    unsigned int r = (packedUintColor) & 0xFFUL;

    unpackedFloat4Rgba[0] = float(r) / 255.f;
    unpackedFloat4Rgba[1] = float(g) / 255.f;
    unpackedFloat4Rgba[2] = float(b) / 255.f;
    unpackedFloat4Rgba[3] = float(a) / 1.f;
    
    return unpackedFloat4Rgba;
}

// //--------------------------------------------------------------------------------------------------
// float4 UnpackUintToFloatRgba(uint packedInput)
// {
//     float4 unpackedColor;
//     uint4 packedColorAsUint4 = uint4((packedInput & 0xFFUL), (packedInput >> 8UL) & 0xFFUL, (packedInput >> 16UL) & 0xFFUL, (packedInput >> 24UL));
//     unpackedColor = ((float4) packedColorAsUint4) / 255;
//     return unpackedColor;
// }

//--------------------------------------------------------------------------------------------------
ByteAddressBuffer linkedListRootBuffer : register(t1);
StructuredBuffer<FragmentLinkedList> fragLinkedListBuffer : register(t2);


//--------------------------------------------------------------------------------------------------
RWTexture2D<float4> testRT : register(u1);


//--------------------------------------------------------------------------------------------------
[numthreads(8, 8, 1)]
void ComputeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2   screenPosXY     =   uint2(dispatchThreadID.xy);
    FragmentLinkedList fragments[NUM_OF_NODES];
    
    uint fragCount          =   0;
    uint offsetAddress      =   4 * (SCREEN_WIDTH * screenPosXY.y + screenPosXY.x);
    uint rootNodeIndex      =   linkedListRootBuffer.Load(offsetAddress);
    
    // Copy the linked list for this pixel into the temp array
    [loop]
    while(rootNodeIndex != UINT_MAX && fragCount < NUM_OF_NODES)
    {
        fragments[fragCount] = fragLinkedListBuffer[rootNodeIndex];
        ++fragCount;
        rootNodeIndex = fragLinkedListBuffer[rootNodeIndex].nextFragmentNode;
    }
    
    // Sort the array by depth
    [loop]
    for (uint i = 1; i < fragCount; i++)
    {
        FragmentLinkedList toInsert = fragments[i];
        uint j = i;
        while (j > 0 && toInsert.fragmentDepth > fragments[j - 1].fragmentDepth)
        {
            fragments[j] = fragments[j - 1];
            j--;
        }
        fragments[j] = toInsert;
    }
    
    float3 color = testRT[dispatchThreadID.xy].rgb;
    
    // combine the colors
    for (i = 0; i < fragCount; i++)
    {
        float4 fragColor = GetRGBAFromRGBE(UnpackUintToFloatRgba(fragments[i].fragmentColor));
        // float4 fragColor = UnpackUintToFloatRgba(fragments[i].fragmentColor);
        color = (color.rgb * fragments[i].fragmentTransmission) / 255 + fragColor.rgb;
    }
 
    testRT[screenPosXY] = float4(color.rgb, 1);
    // float4 fragColor = UnpackUintToFloatRgba(fragments[0].fragmentColor);
    // float normalizedInverseAlpha = float(fragments[0].fragmentTransmission * 1.f / 255.f);
    // float3 destColorBlend = color.rgb * normalizedInverseAlpha;
    // testRT[screenPosXY] = float4(fragColor.rgb + destColorBlend, 1);
}