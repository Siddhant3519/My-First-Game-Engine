//--------------------------------------------------------------------------------------------------
static const unsigned int NUM_OF_NODES          =   4;
static const unsigned int MAX_DEFAULT_VALUE     =   255;


//--------------------------------------------------------------------------------------------------
struct vs_input_t
{
	float3 localPosition    : POSITION;
	float4 color            : COLOR;
	float2 uv               : TEXCOORD;	
};


//--------------------------------------------------------------------------------------------------
struct v2p_t
{
	float4 position : SV_Position;
	float4 color    : COLOR;
	float2 uv       : TEXCOORD;
};


//--------------------------------------------------------------------------------------------------
cbuffer CameraConstants : register(b2)
{
    float4x4 viewToClipTransform;
    float4x4 worldToViewTransform;
};


//--------------------------------------------------------------------------------------------------
cbuffer ModelConstants : register(b3)
{
    float4x4 localToWorldTransform;
	float4   modelColor;
};


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
v2p_t VertexMain(vs_input_t input)
{
    v2p_t v2p;
    float4 localPosition    =   float4(input.localPosition, 1);
    float4 worldPosition    =   mul(localToWorldTransform, localPosition);
    float4 viewPosition     =   mul(worldToViewTransform, worldPosition);
    float4 clipPosition     =   mul(viewToClipTransform, viewPosition);
    v2p.position            =   clipPosition;
    v2p.color               =   input.color * modelColor;
    v2p.uv                  =   input.uv;
	return v2p;
}


//--------------------------------------------------------------------------------------------------
Texture2D    diffuseTexture     : register(t0);

SamplerState diffuseSampler     : register(s0);
 

//--------------------------------------------------------------------------------------------------
RasterizerOrderedTexture2D<uint>                    untouchedFragmentMask   :   register(u1);
RasterizerOrderedStructuredBuffer<FragmentArray>    blendingArray           :   register(u2);


//--------------------------------------------------------------------------------------------------
[earlydepthstencil]
void PixelMain(v2p_t input)
{
    float4 premultipliedTexelColor   =   diffuseTexture.Sample(diffuseSampler, input.uv);
    premultipliedTexelColor          *=  input.color;
    premultipliedTexelColor.rgb      =   premultipliedTexelColor.rgb * premultipliedTexelColor.a;
    
    const float currentTexelDepth           =   input.position.z;
    const float currentTexelTransmission    =   1.f - premultipliedTexelColor.a;
    
    Fragment currentTexelBlendingArray[NUM_OF_NODES];
    uint2 texelCoords           =   uint2(input.position.xy);
    uint offsetAddress          =   (SCREEN_WIDTH * texelCoords.y) + texelCoords.x;
    bool isFragmentUntouched    =   untouchedFragmentMask[texelCoords];
    if (isFragmentUntouched)
    {
        currentTexelBlendingArray[0].premultipliedColor     =   premultipliedTexelColor;
        currentTexelBlendingArray[0].transmission           =   currentTexelTransmission;
        currentTexelBlendingArray[0].depth                  =   currentTexelDepth;
        
        [unroll]
        for (uint nodeIndex = 1; nodeIndex < NUM_OF_NODES; ++nodeIndex)
        {
            currentTexelBlendingArray[nodeIndex].premultipliedColor     =   0;
            currentTexelBlendingArray[nodeIndex].transmission           =   MAX_DEFAULT_VALUE;
            currentTexelBlendingArray[nodeIndex].depth                  =   MAX_DEFAULT_VALUE;
        }
        untouchedFragmentMask[texelCoords] = false;
    }
    else
    {
        currentTexelBlendingArray = blendingArray[offsetAddress].fragmentsPerPixel;
        Fragment intermediatecurrentTexelBlendingArray[NUM_OF_NODES + 1];
        for (uint copyingIndex = 0; copyingIndex < NUM_OF_NODES; ++copyingIndex)
        {
            intermediatecurrentTexelBlendingArray[copyingIndex] = currentTexelBlendingArray[copyingIndex];
        }
        
        Fragment incomingFragment;
        incomingFragment.premultipliedColor     =   premultipliedTexelColor;
        incomingFragment.transmission           =   currentTexelTransmission;
        incomingFragment.depth                  =   currentTexelDepth;
        Fragment temp;
        
        [unroll]
        for (uint insertionIndex = 0; insertionIndex < NUM_OF_NODES + 1; ++insertionIndex)
        {
            Fragment currentFragment = intermediatecurrentTexelBlendingArray[insertionIndex];
            if (incomingFragment.depth <= currentFragment.depth)
            {
                temp                                                    =   currentFragment;
                intermediatecurrentTexelBlendingArray[insertionIndex]   =   incomingFragment;
                incomingFragment                                        =   temp;
            }
        }
        
        if (intermediatecurrentTexelBlendingArray[NUM_OF_NODES].depth != float(MAX_DEFAULT_VALUE))
        {
            temp.premultipliedColor =   intermediatecurrentTexelBlendingArray[NUM_OF_NODES - 1].premultipliedColor + (intermediatecurrentTexelBlendingArray[NUM_OF_NODES].premultipliedColor * intermediatecurrentTexelBlendingArray[NUM_OF_NODES - 1].transmission);
            temp.transmission       =   intermediatecurrentTexelBlendingArray[NUM_OF_NODES - 1].transmission * intermediatecurrentTexelBlendingArray[NUM_OF_NODES].transmission;
            temp.depth              =   intermediatecurrentTexelBlendingArray[NUM_OF_NODES - 1].depth;
            
            intermediatecurrentTexelBlendingArray[NUM_OF_NODES - 1] = temp;
        }
        
        [unroll]
        for (uint index = 0; index < NUM_OF_NODES; ++index)
        {
            currentTexelBlendingArray[index] = intermediatecurrentTexelBlendingArray[index];
        }
    }
    
    blendingArray[offsetAddress] = currentTexelBlendingArray;
}