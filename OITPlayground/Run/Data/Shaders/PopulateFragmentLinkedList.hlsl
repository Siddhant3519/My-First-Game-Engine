struct vs_input_t
{
    float3 localPosition    :   POSITION;
    float4 color            :   COLOR;
    float2 uv               :   TEXCOORD;
};


//--------------------------------------------------------------------------------------------------
struct v2p_t
{
    float4 position         :   SV_Position;
    float4 color            :   COLOR;
    float2 uv               :   TEXCOORD;
    float3 viewPosition     :   VIEW_POSITION;
};


//--------------------------------------------------------------------------------------------------
cbuffer CameraConstants : register(b2)
{
    float4x4 projectionMatrix;
    float4x4 viewMatrix;
};


//--------------------------------------------------------------------------------------------------
cbuffer ModelConstants : register(b3)
{
    float4x4    modelMatrix;
    float4      modelColor;
};


//--------------------------------------------------------------------------------------------------
cbuffer ScreenConstants : register(b8)
{
    float   SCREEN_WIDTH;
    float   pad0;
    uint    UINT_MAX;
    float   CAMERA_FAR;
};


//--------------------------------------------------------------------------------------------------
Texture2D diffuseTexture : register(t0);


//--------------------------------------------------------------------------------------------------
SamplerState diffuseSampler : register(s0);


//--------------------------------------------------------------------------------------------------
struct FragmentLinkedList
{
    unsigned int    fragmentColor;
    unsigned int    fragmentTransmission;
    unsigned int    fragmentDepth;
    unsigned int    nextFragmentNode;
};


//--------------------------------------------------------------------------------------------------
RWByteAddressBuffer                     linkedListRootBuffer : register(u1);
RWStructuredBuffer<FragmentLinkedList>  fragLinkedListBuffer : register(u2);


//--------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
    v2p_t v2p;
    float4 localPosition    =   float4(input.localPosition, 1);
    float4 worldPosition    =   mul(modelMatrix, localPosition);
    float4 viewPosition     =   mul(viewMatrix, worldPosition);
    float4 clipPosition     =   mul(projectionMatrix, viewPosition);
    v2p.position            =   clipPosition;
    v2p.color               =   input.color * modelColor;
    v2p.uv                  =   input.uv;
    v2p.viewPosition        =   viewPosition.xyz;
    return v2p;
}


//--------------------------------------------------------------------------------------------------
float4 GetRGBEFromRGBA(float4 rgbaColor)
{
    float base          =   max (rgbaColor.r, max (rgbaColor.g, rgbaColor.b));
    int exponent        =   0;
    float mantissa      =   frexp(base, exponent);
    float4 rgbeColor    =   float4(saturate(rgbaColor.rgb / exp2(exponent)), exponent + 127);
    return rgbeColor;
}


//--------------------------------------------------------------------------------------------------
uint PackFloat4RGBAToUint(float4 unpackedColor)
{
    uint packedColor;
    uint r  = uint(unpackedColor.r * 255.f);
    uint g  = uint(unpackedColor.g * 255.f);
    uint b  = uint(unpackedColor.b * 255.f);
    uint a  = uint(unpackedColor.a * 1.f);
    
    packedColor = a << 24UL | b << 16UL | g << 8UL | r;
    return packedColor;
}


// //--------------------------------------------------------------------------------------------------
// uint PackFloat4RGBAToUint(float4 unpackedColor)
// {
//     uint4 unpackedColorAsUint   =   (uint4)(saturate(unpackedColor) * 255 + 0.5);
//     uint packedColor            =   (unpackedColorAsUint.a << 24UL) | (unpackedColorAsUint.b << 16UL) | (unpackedColorAsUint.g << 8UL) | (unpackedColorAsUint.r);
//     return packedColor;
// }


//--------------------------------------------------------------------------------------------------
[earlydepthstencil]
void PixelMain(v2p_t input)
{
    float4 diffuseColor     =  diffuseTexture.Sample(diffuseSampler, input.uv);
    float4 color            =  diffuseColor * input.color;
    
    uint2   screenPos               =   uint2(input.position.xy);
    float   fragmentDepthZeroToOne  =   input.viewPosition.x / CAMERA_FAR;
    float4  premultipliedColor      =   float4(color.rgb * color.a, 1);
    uint    packedFragColor         =   PackFloat4RGBAToUint(GetRGBEFromRGBA(premultipliedColor));
    // uint    packedFragColor         =   PackFloat4RGBAToUint(premultipliedColor);
    
    unsigned int indexIntoRootBuffer    =   4 * (SCREEN_WIDTH * screenPos.y + screenPos.x);
    uint         oldFragCount           =   UINT_MAX;
    uint         fragCount              =   fragLinkedListBuffer.IncrementCounter();
    linkedListRootBuffer.InterlockedExchange(indexIntoRootBuffer, fragCount, oldFragCount);

    FragmentLinkedList currentFrag;
    currentFrag.fragmentColor           =   packedFragColor;
    currentFrag.fragmentDepth           =   uint(fragmentDepthZeroToOne * 0xFFFFFFFF);
    float fragTransmission              =   1.f - color.a;
    currentFrag.fragmentTransmission    =   uint(fragTransmission * 0xFF);
    currentFrag.nextFragmentNode        =   oldFragCount;
    
    fragLinkedListBuffer[fragCount] = currentFrag;
}