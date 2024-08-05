static const unsigned int NUM_OF_NODES = 2;

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
RWByteAddressBuffer                     linkedListRootBuffer : register(u1);
RWStructuredBuffer<FragmentLinkedList>  fragLinkedListBuffer : register(u2);


//--------------------------------------------------------------------------------------------------
[numthreads(8, 8, 1)]
void ComputeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 screenPos = uint2(dispatchThreadID.xy);
    uint indexIntoRootBuffer    =   4 * (SCREEN_WIDTH * screenPos.y + screenPos.x);
    uint valueAtRootNode        =   linkedListRootBuffer.Load(indexIntoRootBuffer);
    
    FragmentLinkedList clearValues;
    clearValues.fragmentColor           =   UINT_MAX;
    clearValues.fragmentTransmission    =   UINT_MAX;
    clearValues.fragmentDepth           =   UINT_MAX;
    clearValues.nextFragmentNode        =   UINT_MAX;
    
    // Should this thread safe? Or is this good enough for now?
    uint fragCount = 0;
    [loop]
    while (valueAtRootNode != UINT_MAX && fragCount < NUM_OF_NODES)
    {
        uint nextNode = fragLinkedListBuffer[valueAtRootNode].nextFragmentNode;
        fragLinkedListBuffer[valueAtRootNode] = clearValues;
        valueAtRootNode = nextNode;
        ++fragCount;
    }
    
    linkedListRootBuffer.Store(indexIntoRootBuffer, UINT_MAX);
}