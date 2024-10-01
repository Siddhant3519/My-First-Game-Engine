//--------------------------------------------------------------------------------------------------
struct vs_input_t
{
	float3 localPosition	:	POSITION;
	float4 color			:	COLOR;
	float2 uv				:	TEXCOORD;	
};


//--------------------------------------------------------------------------------------------------
struct v2p_t
{
	float4 position		:	SV_Position;
	float4 color		:	COLOR;
	float2 uv			:	TEXCOORD;
};


//--------------------------------------------------------------------------------------------------
cbuffer CameraConstants : register(b2)
{
	float4x4 ViewToClipTransformator;
	float4x4 WorldToViewTransformator;
};


//--------------------------------------------------------------------------------------------------
cbuffer ModelConstants : register(b3)
{
	float4x4 LocalToWorldTransformator;
	float4   ModelColor;
};


//--------------------------------------------------------------------------------------------------
cbuffer GameConstants : register(b6)
{
    float	    TimeElapsedSeconds;
	float3	    LightDir;
    //----------------------------------- (16 byte aligned)
    float4x4    CamRotationMatrix;
    //----------------------------------- (16 byte aligned)
    float3      CamPosition;
    float       padding1;
    //----------------------------------- (16 byte aligned)
	float2	    ScreenRes;
	float2	    padding2;
    //----------------------------------- (16 byte aligned)
	float4		SkyColor;
	float4		ViewDir;
    //----------------------------------- (16 byte aligned)
};


//--------------------------------------------------------------------------------------------------
Texture2D		DiffuseTexture		:	register(t0);
SamplerState	DiffuseSampler		:	register(s0);


//-----------------------------------------------------------------------------------------------
// Fast hash of an int32 into a different (unrecognizable) uint32.
//
// Returns an unsigned integer containing 32 reasonably-well-scrambled bits, based on the hash
//	of a given (signed) integer input parameter (position/index) and [optional] seed.  Kind of
//	like looking up a value in an infinitely large table of previously generated random numbers.
//
unsigned int Get1dNoiseUint( int positionX, unsigned int seed )
{
	const unsigned int BIT_NOISE1 = 0xd2a80a23;
	const unsigned int BIT_NOISE2 = 0xa884f197;
	const unsigned int BIT_NOISE3 = 0x1b56c4e9;

	unsigned int mangledBits = (unsigned int) positionX;
	mangledBits *= BIT_NOISE1;
	mangledBits += seed;
	mangledBits ^= (mangledBits >> 7);
	mangledBits += BIT_NOISE2;
	mangledBits ^= (mangledBits >> 8);
	mangledBits *= BIT_NOISE3;
	mangledBits ^= (mangledBits >> 11);
	return mangledBits;
}


//-----------------------------------------------------------------------------------------------
unsigned int Get2dNoiseUint(int indexX, int indexY, unsigned int seed)
{
    int PRIME_NUMBER = 198491317; // Large prime number with non-boring bits
    return Get1dNoiseUint(indexX + (PRIME_NUMBER * indexY), seed);
}

//-----------------------------------------------------------------------------------------------
unsigned int Get3dNoiseUint(int indexX, int indexY, int indexZ, unsigned int seed)
{
    const int PRIME1 = 198491317; // Large prime number with non-boring bits
    const int PRIME2 = 6542989; // Large prime number with distinct and non-boring bits
    return Get1dNoiseUint(indexX + (PRIME1 * indexY) + (PRIME2 * indexZ), seed);
}


//-----------------------------------------------------------------------------------------------
float Get1dNoiseNegOneToOne( int index, unsigned int seed )
{
    const float		ONE_OVER_MAX_INT		=	4.65661287e-10;
	unsigned int	noiseAsUint				=	Get1dNoiseUint(index, seed);
	int				noiseAsInt				=	int(noiseAsUint);
	float			noiseAsFloat			=	float(noiseAsInt);
	float			noiseNegOneToOneFloat	=	ONE_OVER_MAX_INT * noiseAsFloat;
	return noiseNegOneToOneFloat;
}


//-----------------------------------------------------------------------------------------------
float Get3dNoiseNegOneToOne(int indexX, int indexY, int indexZ, unsigned int seed)
{
    const float		ONE_OVER_MAX_INT		=	4.65661287e-10;
	unsigned int	noiseAsUint				=	Get3dNoiseUint(indexX, indexY, indexZ, seed);
	int				noiseAsInt				=	int(noiseAsUint);
	float			noiseAsFloat			=	float(noiseAsInt);
	float			noiseNegOneToOneFloat	=	ONE_OVER_MAX_INT * noiseAsFloat;
	return noiseNegOneToOneFloat;
}


//-----------------------------------------------------------------------------------------------
float Get2dNoiseZeroToOne(int indexX, int indexY, unsigned int seed)
{
    const double ONE_OVER_MAX_UINT = (1.0 / (double) 0xFFFFFFFF);
    return (float) (ONE_OVER_MAX_UINT * (double) Get2dNoiseUint(indexX, indexY, seed));
}


//-----------------------------------------------------------------------------------------------
float Get3dNoiseZeroToOne(int indexX, int indexY, int indexZ, unsigned int seed)
{
    const double ONE_OVER_MAX_UINT = (1.0 / (double) 0xFFFFFFFF);
    return (float) (ONE_OVER_MAX_UINT * (double) Get3dNoiseUint(indexX, indexY, indexZ, seed));
}


//--------------------------------------------------------------------------------------------------
float SmoothStart2(float parametricZeroToOneT)
{
	float easingVal = parametricZeroToOneT * parametricZeroToOneT;
	return easingVal;
}


//--------------------------------------------------------------------------------------------------
float SmoothStart3(float parametricZeroToOneT)
{
	float easingVal = parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT;
	return easingVal;
}


//--------------------------------------------------------------------------------------------------
float SmoothStart4(float parametricZeroToOneT)
{
	float easingVal = parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT;
	return easingVal;
}


//--------------------------------------------------------------------------------------------------
float SmoothStart5(float parametricZeroToOneT)
{
	float easingVal = parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT;
	return easingVal;
}


//--------------------------------------------------------------------------------------------------
float SmoothStart6(float parametricZeroToOneT)
{
	float easingVal = parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT;
	return easingVal;
}


//--------------------------------------------------------------------------------------------------
float SmoothStop2(float parametricZeroToOneT)
{
    float mirroredParametricZeroToOne = 1.f - parametricZeroToOneT;
    mirroredParametricZeroToOne *= mirroredParametricZeroToOne;
    float flippedMirrored = 1.f - mirroredParametricZeroToOne;
    return flippedMirrored;
}


//--------------------------------------------------------------------------------------------------
float SmoothStop3(float parametricZeroToOneT)
{
    float mirroredParametricZeroToOne = 1.f - parametricZeroToOneT;
    mirroredParametricZeroToOne *= mirroredParametricZeroToOne * mirroredParametricZeroToOne;
    float flippedMirrored = 1.f - mirroredParametricZeroToOne;
    return flippedMirrored;
}


//--------------------------------------------------------------------------------------------------
float SmoothStop4(float parametricZeroToOneT)
{
    float mirroredParametricZeroToOne = 1.f - parametricZeroToOneT;
    mirroredParametricZeroToOne *= mirroredParametricZeroToOne * mirroredParametricZeroToOne * mirroredParametricZeroToOne;
    float flippedMirrored = 1.f - mirroredParametricZeroToOne;
    return flippedMirrored;
}


//--------------------------------------------------------------------------------------------------
float SmoothStop5(float parametricZeroToOneT)
{
    float mirroredParametricZeroToOne = 1.f - parametricZeroToOneT;
    mirroredParametricZeroToOne *= mirroredParametricZeroToOne * mirroredParametricZeroToOne * mirroredParametricZeroToOne * mirroredParametricZeroToOne;
    float flippedMirrored = 1.f - mirroredParametricZeroToOne;
    return flippedMirrored;
}


//--------------------------------------------------------------------------------------------------
float SmoothStop6(float parametricZeroToOneT)
{
    float mirroredParametricZeroToOne = 1.f - parametricZeroToOneT;
    mirroredParametricZeroToOne *= mirroredParametricZeroToOne * mirroredParametricZeroToOne * mirroredParametricZeroToOne * mirroredParametricZeroToOne * mirroredParametricZeroToOne;
    float flippedMirrored = 1.f - mirroredParametricZeroToOne;
    return flippedMirrored;
}


//--------------------------------------------------------------------------------------------------
static const float PI					=	3.141592653f;
static const float ONE_EIGHTY			=	180.f;
static const float INVERSE_ONE_EIGHTY	=	1 / ONE_EIGHTY;


//--------------------------------------------------------------------------------------------------
float ConvertDegreesToRadians(float degrees)
{
    return (degrees * PI * INVERSE_ONE_EIGHTY);
}


//--------------------------------------------------------------------------------------------------
float GetFractionWithinRange(float value, float rangeStart, float rangeEnd)
{
    float currentRange		=	rangeEnd - rangeStart;
    float relativeValPos	=	(value - rangeStart) / currentRange;
    return relativeValPos;
}


//--------------------------------------------------------------------------------------------------
float RangeMap(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
    float relativeInValPos	=	GetFractionWithinRange(inValue, inStart, inEnd);
    float outRange			=	outEnd - outStart;
    float rangeMappedVal	=	(relativeInValPos * outRange) + outStart;
    return rangeMappedVal;
}


//--------------------------------------------------------------------------------------------------
float2 Get1dNoiseFloat2( int positionX, unsigned int seed )
{
	const unsigned int BIT_NOISE1 = 0xd2a80a23;
	const unsigned int BIT_NOISE2 = 0xa884f197;
	const unsigned int BIT_NOISE3 = 0x1b56c4e9;

    uint2 mangledBits	=	uint2(1190494759u, 2147483647u);
	mangledBits			*=	BIT_NOISE1;
	mangledBits			+=	seed;
	mangledBits			^=	(mangledBits >> 7);
	mangledBits			+=	BIT_NOISE2;
	mangledBits			^=	(mangledBits >> 8);
	mangledBits			*=	BIT_NOISE3;
	mangledBits			^=	(mangledBits >> 11);
	
    const double ONE_OVER_MAX_UINT = (1.0 / (double) 0xFFFFFFFF);
    return float2(float(double(mangledBits.x) * ONE_OVER_MAX_UINT), float(double(mangledBits.y) * ONE_OVER_MAX_UINT));
	
	return mangledBits;
}


//------------------------------------------------------------------------------------------------
static const float fSQRT_3_OVER_3 = 0.57735026918962576450914878050196f;


//--------------------------------------------------------------------------------------------------
float SmoothStep3(float parametricZeroToOneT)
{
	// Interpolate smooth start 3 and smooth stop 3
    return ((3.f * parametricZeroToOneT * parametricZeroToOneT) - (2.f * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT));
}


//--------------------------------------------------------------------------------------------------
float SmoothStep5(float parametricZeroToOneT)
{
	// Interpolate smooth start 5 and smooth stop 5
	// return ((5.f * parametricZeroToOneT * parametricZeroToOneT) - (10.f * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT) + (10.f * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT)
	// 	- (4.f * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT));
    return 6.0f * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT - 15.0f * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT + 10.0f * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT;
}


//------------------------------------------------------------------------------------------------
float Compute3dPerlinNoise( float posX, float posY, float posZ, float scale, unsigned int numOctaves, float octavePersistence, float octaveScale, bool renormalize, unsigned int seed )
{
	const float OCTAVE_OFFSET = 0.636764989593174f; // Translation/bias to add to each octave

	static const float3 gradients[ 8 ] = // Traditional "12 edges" requires modulus and isn't any better.
	{
		float3( +fSQRT_3_OVER_3, +fSQRT_3_OVER_3, +fSQRT_3_OVER_3 ), // Normalized unit 3D vectors
		float3( -fSQRT_3_OVER_3, +fSQRT_3_OVER_3, +fSQRT_3_OVER_3 ), //  pointing toward cube
		float3( +fSQRT_3_OVER_3, -fSQRT_3_OVER_3, +fSQRT_3_OVER_3 ), //  corners, so components
		float3( -fSQRT_3_OVER_3, -fSQRT_3_OVER_3, +fSQRT_3_OVER_3 ), //  are all sqrt(3)/3, i.e.
		float3( +fSQRT_3_OVER_3, +fSQRT_3_OVER_3, -fSQRT_3_OVER_3 ), // 0.5773502691896257645091f.
		float3( -fSQRT_3_OVER_3, +fSQRT_3_OVER_3, -fSQRT_3_OVER_3 ), // These are slightly better
		float3( +fSQRT_3_OVER_3, -fSQRT_3_OVER_3, -fSQRT_3_OVER_3 ), // than axes (1,0,0) and much
		float3( -fSQRT_3_OVER_3, -fSQRT_3_OVER_3, -fSQRT_3_OVER_3 )  // faster than edges (1,1,0).
	};

	float totalNoise		=	0.f;
	float totalAmplitude	=	0.f;
	float currentAmplitude	=	1.f;
	float invScale			=	(1.f / scale);
	float3 currentPos		=	float3( posX * invScale, posY * invScale, posZ * invScale );

	[unroll]
	for( unsigned int octaveNum = 0; octaveNum < numOctaves; ++ octaveNum )
	{
		// Determine random unit "gradient vectors" for surrounding corners
		float3 cellMins		=	float3( floor( currentPos.x ), floor( currentPos.y ), floor( currentPos.z ) );
		float3 cellMaxs		=	float3( cellMins.x + 1.f, cellMins.y + 1.f, cellMins.z + 1.f );
		int indexWestX		=	(int) cellMins.x;
		int indexSouthY		=	(int) cellMins.y;
		int indexBelowZ		=	(int) cellMins.z;
		int indexEastX		=	indexWestX  + 1;
		int indexNorthY		=	indexSouthY + 1;
		int indexAboveZ		=	indexBelowZ + 1;

		unsigned int noiseBelowSW	=	Get3dNoiseUint( indexWestX, indexSouthY, indexBelowZ, seed );
		unsigned int noiseBelowSE	=	Get3dNoiseUint( indexEastX, indexSouthY, indexBelowZ, seed );
		unsigned int noiseBelowNW	=	Get3dNoiseUint( indexWestX, indexNorthY, indexBelowZ, seed );
		unsigned int noiseBelowNE	=	Get3dNoiseUint( indexEastX, indexNorthY, indexBelowZ, seed );
		unsigned int noiseAboveSW	=	Get3dNoiseUint( indexWestX, indexSouthY, indexAboveZ, seed );
		unsigned int noiseAboveSE	=	Get3dNoiseUint( indexEastX, indexSouthY, indexAboveZ, seed );
		unsigned int noiseAboveNW	=	Get3dNoiseUint( indexWestX, indexNorthY, indexAboveZ, seed );
		unsigned int noiseAboveNE	=	Get3dNoiseUint( indexEastX, indexNorthY, indexAboveZ, seed );

		float3 gradientBelowSW	=	gradients[ noiseBelowSW & 0x00000007 ];
		float3 gradientBelowSE	=	gradients[ noiseBelowSE & 0x00000007 ];
		float3 gradientBelowNW	=	gradients[ noiseBelowNW & 0x00000007 ];
		float3 gradientBelowNE	=	gradients[ noiseBelowNE & 0x00000007 ];
		float3 gradientAboveSW	=	gradients[ noiseAboveSW & 0x00000007 ];
		float3 gradientAboveSE	=	gradients[ noiseAboveSE & 0x00000007 ];
		float3 gradientAboveNW	=	gradients[ noiseAboveNW & 0x00000007 ];
		float3 gradientAboveNE	=	gradients[ noiseAboveNE & 0x00000007 ];

		// Dot each corner's gradient with displacement from corner to position
		float3 displacementFromBelowSW	=	float3( currentPos.x - cellMins.x, currentPos.y - cellMins.y, currentPos.z - cellMins.z );
		float3 displacementFromBelowSE	=	float3( currentPos.x - cellMaxs.x, currentPos.y - cellMins.y, currentPos.z - cellMins.z );
		float3 displacementFromBelowNW	=	float3( currentPos.x - cellMins.x, currentPos.y - cellMaxs.y, currentPos.z - cellMins.z );
		float3 displacementFromBelowNE	=	float3( currentPos.x - cellMaxs.x, currentPos.y - cellMaxs.y, currentPos.z - cellMins.z );
		float3 displacementFromAboveSW	=	float3( currentPos.x - cellMins.x, currentPos.y - cellMins.y, currentPos.z - cellMaxs.z );
		float3 displacementFromAboveSE	=	float3( currentPos.x - cellMaxs.x, currentPos.y - cellMins.y, currentPos.z - cellMaxs.z );
		float3 displacementFromAboveNW	=	float3( currentPos.x - cellMins.x, currentPos.y - cellMaxs.y, currentPos.z - cellMaxs.z );
		float3 displacementFromAboveNE	=	float3( currentPos.x - cellMaxs.x, currentPos.y - cellMaxs.y, currentPos.z - cellMaxs.z );

		float dotBelowSW	=	dot( gradientBelowSW, displacementFromBelowSW );
		float dotBelowSE	=	dot( gradientBelowSE, displacementFromBelowSE );
		float dotBelowNW	=	dot( gradientBelowNW, displacementFromBelowNW );
		float dotBelowNE	=	dot( gradientBelowNE, displacementFromBelowNE );
		float dotAboveSW	=	dot( gradientAboveSW, displacementFromAboveSW );
		float dotAboveSE	=	dot( gradientAboveSE, displacementFromAboveSE );
		float dotAboveNW	=	dot( gradientAboveNW, displacementFromAboveNW );
		float dotAboveNE	=	dot( gradientAboveNE, displacementFromAboveNE );

		// Do a smoothed (nonlinear) weighted average of dot results
		float weightEast	=	SmoothStep3( displacementFromBelowSW.x );
		float weightNorth	=	SmoothStep3( displacementFromBelowSW.y );
		float weightAbove	=	SmoothStep3( displacementFromBelowSW.z );
		float weightWest	=	1.f - weightEast;
		float weightSouth	=	1.f - weightNorth;
		float weightBelow	=	1.f - weightAbove;

		// 8-way blend (8 -> 4 -> 2 -> 1)
		float blendBelowSouth	=	(weightEast * dotBelowSE) + (weightWest * dotBelowSW);
		float blendBelowNorth	=	(weightEast * dotBelowNE) + (weightWest * dotBelowNW);
		float blendAboveSouth	=	(weightEast * dotAboveSE) + (weightWest * dotAboveSW);
		float blendAboveNorth	=	(weightEast * dotAboveNE) + (weightWest * dotAboveNW);
		float blendBelow		=	(weightSouth * blendBelowSouth) + (weightNorth * blendBelowNorth);
		float blendAbove		=	(weightSouth * blendAboveSouth) + (weightNorth * blendAboveNorth);
		float blendTotal		=	(weightBelow * blendBelow) + (weightAbove * blendAbove);
		float noiseThisOctave	=	blendTotal * (1.f / 0.793856621f); // 3D Perlin is in [-.793856621,.793856621]; map to ~[-1,1]

		// Accumulate results and prepare for next octave (if any)
		totalNoise			+=	noiseThisOctave * currentAmplitude;
		totalAmplitude		+=	currentAmplitude;
		currentAmplitude	*=	octavePersistence;
		currentPos			*=	octaveScale;
		currentPos.x		+=	OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		currentPos.y		+=	OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		currentPos.z		+=	OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		++ seed; // Eliminates octaves "echoing" each other (since each octave is uniquely seeded)
	}

	// Re-normalize total noise to within [-1,1] and fix octaves pulling us far away from limits
	if( renormalize && totalAmplitude > 0.f )
	{
		totalNoise	/=	totalAmplitude;					// Amplitude exceeds 1.0 if octaves are used
		totalNoise	=	(totalNoise * 0.5f) + 0.5f;		// Map to [0,1]
		totalNoise	=	SmoothStep3( totalNoise );		// Push towards extents (octaves pull us away)
		totalNoise	=	(totalNoise * 2.0f) - 1.f;		// Map back to [-1,1]
	}

	return totalNoise;
}


//--------------------------------------------------------------------------------------------------
float3 CalculateTerrainHeight(float3 vertWorldPos)
{
    float3 scaledVertWorldPos	=	vertWorldPos;
	float vertZDist				=	Compute3dPerlinNoise(scaledVertWorldPos.x, scaledVertWorldPos.y, scaledVertWorldPos.z, 20.f, 1, 0.5f, 2.f, true, 0) * 0.5f + 0.5f;
	vertZDist					*=	10.f;
    return float3(vertWorldPos.xy, vertZDist);
}


//--------------------------------------------------------------------------------------------------
static const float3 GRASS_BASE_COLOR			=	float3(0.075f, 0.427f, 0.082f);
static const float3 GRASS_TIP_COLOR				=	float3(0.25f, 0.60f, 0.03f);
static const float3 GRASSY_FIELD_BASE_COLOR		=	float3(0.83f, 0.93f, 0.39f);
static const float3 GRASSY_FIELD_TIP_COLOR		=	float3(0.89f, 0.99f, 0.59f);


//--------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input, uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    v2p_t v2p;
    float4 localPosition	=	float4(input.localPosition, 1);
	float4 worldPosition	=	mul(LocalToWorldTransformator, localPosition);
	worldPosition			=	float4(CalculateTerrainHeight(worldPosition.xyz), 1.f);
	float4 viewPosition		=	mul(WorldToViewTransformator, worldPosition);
	float4 clipPosition		=	mul(ViewToClipTransformator, viewPosition);
	v2p.position			=	clipPosition;
	v2p.color				=	input.color * ModelColor;
	v2p.uv					=	input.uv;
	return v2p;
}


//--------------------------------------------------------------------------------------------------
static const float3 RED     =   float3(1.f, 0.f, 0.f);
static const float3 GREEN   =   float3(0.f, 1.f, 0.f);
static const float3 BLUE    =   float3(0.f, 0.f, 1.f);
static const float3 GRAY    =   float3(0.5f, 0.5f, 0.5f);
static const float3 WHITE   =   float3(1.f, 1.f, 1.f);


//--------------------------------------------------------------------------------------------------
[earlydepthstencil]
float4 PixelMain(v2p_t input) : SV_Target0
{
	float4 color	=	DiffuseTexture.Sample(DiffuseSampler, input.uv);
	color			*=	input.color;
	return color;
}