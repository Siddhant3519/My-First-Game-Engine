//------------------------------------------------------------------------------------------------
static const int	NUM_RAY_MARCH_ITERATIONS	=	256;
static const float	MAX_RAY_MARCH_DIST			=	1000.f;
static const float	MIN_DIST			        =	0.00001f;


//------------------------------------------------------------------------------------------------
struct vs_input_t
{
	float3 grassBladeWorldPos	:	INSTANCE_POSITION;
};


//------------------------------------------------------------------------------------------------
struct v2p_t
{
	float4 clipPosition		:	SV_Position;
	float4 color			:	COLOR;
	float4 grassNormal		:	NORMAL;
	float4 grassData		:	GRASS_DATA;
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
cbuffer FoliageConstants : register(b7)
{
    float NumGrassSegments;
    float GrassPatchSize;
    float GrassWidth;
    float GrassHeight;
    //----------------------------------- (16 byte aligned)
};


//--------------------------------------------------------------------------------------------------
Texture2D		StiffnessTexture	:	register(t5);
SamplerState	diffuseSampler		:	register(s0);


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
column_major float4x4 CreateZRotationDegrees(float rotationDegreesAboutZ)
{
    float rotationRadiansAboutZ			=	ConvertDegreesToRadians(rotationDegreesAboutZ);
	float cosRotation					=	cos(rotationRadiansAboutZ);
	float sinRotation					=	sin(rotationRadiansAboutZ);
    column_major float4x4 rotation		=	float4x4(cosRotation,	-sinRotation,	0.f,	0.f,
													 sinRotation,	cosRotation,	0.f,	0.f,
													 0.f,			0.f,			1.f,	0.f, 
													 0.f,			0.f,			0.f,	1.f);
	return rotation;
}


//--------------------------------------------------------------------------------------------------
column_major float4x4 CreateZRotationRadians(float rotationRadiansAboutZ)
{
	float cosRotation					=	cos(rotationRadiansAboutZ);
	float sinRotation					=	sin(rotationRadiansAboutZ);
    column_major float4x4 rotation		=	float4x4(cosRotation,	-sinRotation,	0.f,	0.f,
													 sinRotation,	cosRotation,	0.f,	0.f,
													 0.f,			0.f,			1.f,	0.f, 
													 0.f,			0.f,			0.f,	1.f);
	return rotation;
}


//--------------------------------------------------------------------------------------------------
column_major float4x4 CreateYRotationDegrees(float rotationDegreesAboutZ)
{
    float rotationRadiansAboutZ			=	ConvertDegreesToRadians(rotationDegreesAboutZ);
	float cosRotation					=	cos(rotationRadiansAboutZ);
	float sinRotation					=	sin(rotationRadiansAboutZ);
    column_major float4x4 rotation		=	float4x4(cosRotation,	0.f,	sinRotation,	0.f,
													 0.f,			1.f,	0.f,			0.f,
													 -sinRotation,	0.f,	cosRotation,	0.f,
													 0.f,			0.f,	0.f,			1.f);
	return rotation;
}


//--------------------------------------------------------------------------------------------------
column_major float4x4 CreateNonUniformScale3D(float3 nonUniformScaleXYZ)
{
	column_major float4x4 scale	=	float4x4(nonUniformScaleXYZ.x,	0.f,					0.f,					0.f,
											 0.f,					nonUniformScaleXYZ.y,	0.f,					0.f,
											 0.f,					0.f,					nonUniformScaleXYZ.z,	0.f,
											 0.f,					0.f,					0.f,					1.f);
	return scale;
}


//--------------------------------------------------------------------------------------------------
column_major float4x4 CreateTranslation3D(float3 translationXYZ)
{
    column_major float4x4 translation =	float4x4(1.f,	0.f,	0.f,	translationXYZ.x,
												 0.f,	1.f,	0.f,	translationXYZ.y,
												 0.f,	0.f,	1.f,	translationXYZ.z,
												 0.f,	0.f,	0.f,	1.f);
	return translation;
}


//--------------------------------------------------------------------------------------------------
column_major float4x4 CreateAxisRotationRadians(float3 axis, float rotationRadiansAboutAxis)
{
	float c		=	cos(rotationRadiansAboutAxis);
	float s		=	sin(rotationRadiansAboutAxis);
	float ic	=	1.f - c;
	
    column_major float4x4 rotation		=	float4x4(ic * axis.x * axis.x + c,					ic * axis.x * axis.y - axis.z * s,			ic * axis.x * axis.z + axis.y * s,		0.f,
													 ic * axis.x * axis.y + axis.z * s,			ic * axis.y * axis.y + c,					ic * axis.y * axis.z - axis.x * s,		0.f,
													 ic * axis.x * axis.z - axis.y * s,			ic * axis.y * axis.z + axis.x * s,			ic * axis.z * axis.z + c,				0.f, 
													 0.f,										0.f,										0.f,									1.f);
	return rotation;
}


//--------------------------------------------------------------------------------------------------
float3 ComputeCubicBezier3D(float3 A, float3 B, float3 C, float3 D, float parametricZeroToOne)
{
	// A, B, C, D are control points
	float3 AB			=	lerp(A, B, parametricZeroToOne);
	float3 BC			=	lerp(B, C, parametricZeroToOne);
	float3 CD			=	lerp(C, D, parametricZeroToOne);

	float3 ABBC			=	lerp(AB, BC, parametricZeroToOne);
	float3 BCCD			=	lerp(BC, CD, parametricZeroToOne);

	float3 ABBCBCCD		=	lerp(ABBC, BCCD, parametricZeroToOne);
	return ABBCBCCD;
}


//--------------------------------------------------------------------------------------------------
float3 ComputeCubicBezier3DGrad(float3 A, float3 B, float3 C, float3 D, float parametricZeroToOne)
{
    return	(3.0 * (1.0 - parametricZeroToOne) * (1.0 - parametricZeroToOne) * (B - A) +
			 6.0 * (1.0 - parametricZeroToOne) * parametricZeroToOne * (C - B) +
			 3.0 * parametricZeroToOne * parametricZeroToOne * (D - C));
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
float2 murmurHash(uint index)
{
    const uint m	=	0x5bd1e995u;
    uint2 h			=	uint2(1190494759u, 2147483647u);
    index			*=	m;
    index			^=	index >> 24u;
    index			*=	m;
    h				*=	m;
    h				^=	index;
    h				^=	h >> 13u;
    h				*=	m;
    h				^=	h >> 15u;
	
    const double ONE_OVER_MAX_UINT = (1.0 / (double) 0xFFFFFFFF);
    return float2(float(double(h.x) * ONE_OVER_MAX_UINT), float(double(h.y) * ONE_OVER_MAX_UINT));
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
// static const float3 GRASS_BASE_COLOR	=	float3(0.1f, 0.4f, 0.04f);
// static const float3 GRASS_TIP_COLOR		=	float3(0.5f, 0.7f, 0.3f);

// static const float3 GRASS_BASE_COLOR	=	float3(0.25f, 0.50f, 0.02f);
// static const float3 GRASS_TIP_COLOR		=	float3(0.50f, 0.75f, 0.30f);

static const float3 GRASS_BASE_COLOR	=	float3(0.075f, 0.427f, 0.082f);
static const float3 GRASS_TIP_COLOR		=	float3(0.25f, 0.60f, 0.03f);
// static const float3 GRASS_TIP_COLOR		=	float3(0.15f, 0.55f, 0.02f);

// static const float3 GRASSY_FIELD_BASE_COLOR = float3(0.66f, 0.80f, 0.13f);
// static const float3 GRASSY_FIELD_TIP_COLOR	= float3(0.85f, 0.96f, 0.41f);
static const float3 GRASSY_FIELD_BASE_COLOR = float3(0.83f, 0.93f, 0.39f);
static const float3 GRASSY_FIELD_TIP_COLOR	= float3(0.89f, 0.99f, 0.59f);


//-----------------------------------------------------------------------------------------------
float Get1dNoiseZeroToOne( int index, unsigned int seed )
{
	double ONE_OVER_MAX_UINT = (1.0 / (double) 0xFFFFFFFF);
	return (float)( ONE_OVER_MAX_UINT * (double) Get1dNoiseUint( index, seed ) );
}


//-----------------------------------------------------------------------------------------------
float Compute1dFractalNoise( float position, float scale, unsigned int numOctaves, float octavePersistence, float octaveScale, bool renormalize, unsigned int seed )
{
	const float OCTAVE_OFFSET = 0.636764989593174f; // Translation/bias to add to each octave

	float totalNoise		=	0.f;
	float totalAmplitude	=	0.f;
	float currentAmplitude	=	1.f;
	float currentPosition	=	position * (1.f / scale);

	[unroll]
	for( unsigned int octaveNum = 0; octaveNum < numOctaves; ++ octaveNum )
	{
		// Determine noise values at nearby integer "grid point" positions
		float positionFloor		=	floor( currentPosition );
		int indexWest			=	(int) positionFloor;
		int indexEast			=	indexWest + 1;
		float valueWest			=	Get1dNoiseZeroToOne( indexWest, seed );
		float valueEast			=	Get1dNoiseZeroToOne( indexEast, seed );

		// Do a smoothed (nonlinear) weighted average of nearby grid point values
		float distanceFromWest	=	currentPosition - positionFloor;
		float weightEast		=	SmoothStep3( distanceFromWest ); // Gives rounder (nonlinear) results
		float weightWest		=	1.f - weightEast;
		float noiseZeroToOne	=	(valueWest * weightWest) + (valueEast * weightEast);
		float noiseThisOctave	=	2.f * (noiseZeroToOne - 0.5f); // Map from [0,1] to [-1,1]

		// Accumulate results and prepare for next octave (if any)
		totalNoise			+=	noiseThisOctave * currentAmplitude;
		totalAmplitude		+=	currentAmplitude;
		currentAmplitude	*=	octavePersistence;
		currentPosition		*=	octaveScale;
		currentPosition		+=	OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		++ seed; // Eliminates octaves "echoing" each other (since each octave is uniquely seeded)
	}

	// Re-normalize total noise to within [-1,1] and fix octaves pulling us far away from limits
	if( renormalize && totalAmplitude > 0.f )
	{
		totalNoise /= totalAmplitude;				// Amplitude exceeds 1.0 if octaves are used!
		totalNoise = (totalNoise * 0.5f) + 0.5f;	// Map to [0,1]
		totalNoise = SmoothStep3( totalNoise );		// Push towards extents (octaves pull us away)
		totalNoise = (totalNoise * 2.0f) - 1.f;		// Map back to [-1,1]
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
v2p_t VertexMain(vs_input_t input, uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    const uint	NUM_GRASS_VERTS_PER_FACE	=	(NumGrassSegments + 1) * 2;
    const float	GRASS_HALF_WIDTH			=	GrassWidth * 0.5f;
	
    uint	localVertexIDGrassBlade			=	vertexID % (NUM_GRASS_VERTS_PER_FACE * 2);	// Calculate vertex id local to grass blade
	uint	localVertexIDGrassFace			=	vertexID % NUM_GRASS_VERTS_PER_FACE;		// Calculate vertex id local to grass blade face
	float	facingDirection					=	localVertexIDGrassBlade >= NUM_GRASS_VERTS_PER_FACE ? 1.f : -1.f;
	uint	isOddLocalVertexID				=	localVertexIDGrassFace & 0x01;
	float	heightInterpolatorZeroToOne		=	float(localVertexIDGrassFace - float(isOddLocalVertexID)) / float(NumGrassSegments * 2);
	float	grassWidthScaleFactor			=	SmoothStop3(1.f - heightInterpolatorZeroToOne) * GRASS_HALF_WIDTH;
    float	randomOrientationRadians		=	Get1dNoiseNegOneToOne(instanceID + 1, 7) * PI * 0.75;
    // float	randomOrientationRadians		=	Compute1dFractalNoise(Get1dNoiseUint(instanceID + 1, 6), 1.f, 1, 0.5f, 2.f, true, 7) * PI * 0.75f;
	
	// Stiffness
	float2	localOffsetXY		=	murmurHash(instanceID) * 2.f - 1.f;
	float2  localOffsetAsUVs	=	localOffsetXY * 0.5f - 0.5f;
    float4	stiffnessData		=	StiffnessTexture[localOffsetAsUVs];
    float	stiffness			=	1.f - stiffnessData.r * 0.85f;
	
	// Compute bezier curve to figure out grass blade displacement offset, to curve the grass blades
    float3		curveStartPoint			=	float3(0.f, 0.f, 0.f);
	float3		curveControlPointA		=	float3(0.f, 0.f, 0.33f);
	float3		curveControlPointB		=	float3(0.f, 0.f, 0.66f);
	
	// // debug 
    // randomOrientationRadians = float(instanceID) * 0.2f;
	// Transform vertex position  
	
	float		windStrength			=	Compute3dPerlinNoise(TimeElapsedSeconds, localOffsetXY.x * 0.5f + TimeElapsedSeconds, localOffsetXY.y * 0.5f + TimeElapsedSeconds, 4.f, 2, 0.5f, 2.f, true, 6);
    float		windAngle				=	0.f;
    float3		windAxis				=	float3(sin(windAngle), cos(windAngle), 0.f);
	float		windLeanAngle			=	windStrength * 1.5f * heightInterpolatorZeroToOne * stiffness;
	float		randomLeanAnimation		=	Compute3dPerlinNoise(TimeElapsedSeconds * 4.f, localOffsetXY.x, localOffsetXY.y, 4.f, 2.f, 0.5f, 2.f, true, 4) * (windStrength * 0.5f + 0.125f);
	float		leanOrientationRadians	=	RangeMap(randomOrientationRadians, -PI, PI, -0.5f, 0.5f) + randomLeanAnimation;
	
	float3		curveEndPoint			=	float3(sin(leanOrientationRadians), 0.f, cos(leanOrientationRadians));
	float3		curvedDisplacement		=	ComputeCubicBezier3D(curveStartPoint, curveControlPointA, curveControlPointB, curveEndPoint, heightInterpolatorZeroToOne);
	float3		curveTangent			=	ComputeCubicBezier3DGrad(curveStartPoint, curveControlPointA, curveControlPointB, curveEndPoint, heightInterpolatorZeroToOne);

	
	float		localOffsetZ			=	CalculateTerrainHeight(float3(localOffsetXY  * GrassPatchSize, 0.f)).z;
	// float		grassColorNoise			=	Compute3dPerlinNoise(localOffsetXY.x * 20.f, localOffsetXY.y * 40.5f, localOffsetZ * 20.f, 40.f, 2, 0.5f, 2.f, true, 3);
	// float		grassHeight				=	lerp(0.75f, 1.25f, smoothstep(-1.f, 1.f, grassColorNoise));
	float		emptyPatches			=	localOffsetZ * 0.1f > 0.75f ? 0.f : 1.f; // grassHeight;
	if (localOffsetZ * 0.1f < 0.75f && localOffsetZ * 0.1f > 0.65f)
    {
        emptyPatches = lerp(0.25f, 1.f, 1.f - SmoothStart3(smoothstep(0.65f, 0.75f, localOffsetZ * 0.1f)));
    }
	// else if (localOffsetZ * 0.1f <= 0.65f)
    // {
    //     emptyPatches = lerp(0.5f, 1.f, Get1dNoiseZeroToOne(instanceID + 2, 9));
    // }
	float4x4 	rotationMatrix			=   CreateZRotationRadians(randomOrientationRadians);
	rotationMatrix						=	mul(CreateAxisRotationRadians(windAxis, windLeanAngle), rotationMatrix);
	float4x4	scaleMatrix				=	CreateNonUniformScale3D(float3(GrassHeight * emptyPatches, grassWidthScaleFactor * emptyPatches, GrassHeight * emptyPatches));
	float4x4    translationMatrix		=	CreateTranslation3D(float3(localOffsetXY.xy * GrassPatchSize, localOffsetZ));
	
	
	float3 localVertexPos;
    localVertexPos.x					=	curvedDisplacement.x;
    localVertexPos.y					=	(isOddLocalVertexID ? -1.f : 1.f);
    localVertexPos.z					=	curvedDisplacement.z;
	
	// View space thickening
    float3 grassFaceNormal			=	mul(rotationMatrix, float4(-facingDirection, 0.f, 0.f, 0.f)).xyz;
    float  viewDotNormal			=	saturate(dot(grassFaceNormal, ViewDir.xyz));
    float  viewSpaceThickenFactor	=	SmoothStop4(1.f - viewDotNormal) * smoothstep(0.f, 0.2f, viewDotNormal);
	
	float4 grassLocalNormal			=	float4(-curveTangent.z, 0.f, curveTangent.x, 0.f) * -facingDirection;
	float4 localPosition			=	float4(localVertexPos.xyz, 1.f);
	float4 scaledLocalPos			=	mul(scaleMatrix, localPosition);
	float4 rotatedLocalPos			=	mul(rotationMatrix, scaledLocalPos);
	float4 translatedLocalPos		=	mul(translationMatrix, rotatedLocalPos);

	translatedLocalPos.y			+=	viewSpaceThickenFactor * (float(isOddLocalVertexID) - 0.5f) * grassWidthScaleFactor * -facingDirection;

	float4 rotatedNormal			=	mul(rotationMatrix, grassLocalNormal);
	float4 worldPosition			=	mul(LocalToWorldTransformator, translatedLocalPos);
	float4 viewPosition				=	mul(WorldToViewTransformator, worldPosition);
	float4 clipPosition				=	mul(ViewToClipTransformator, viewPosition);
	
    float3 grassColor1		=	lerp(GRASS_BASE_COLOR, GRASS_TIP_COLOR, heightInterpolatorZeroToOne);
    float3 grassColor2		=	lerp(GRASSY_FIELD_BASE_COLOR, GRASSY_FIELD_TIP_COLOR, heightInterpolatorZeroToOne);
	float  grassColorNoise	=	Compute3dPerlinNoise(worldPosition.x, worldPosition.y, worldPosition.z, 10.f, 2, 0.5f, 2.f, true, 3);
	
	// Blend normal
    float distanceBlend		=	smoothstep(0.f, 10.f, distance(CamPosition, worldPosition.xyz));
    rotatedNormal			=	float4(lerp(rotatedNormal.xyz, float3(0.f, 0.f, 1.f), distanceBlend * 0.5f), 0.f);
	rotatedNormal			=	normalize(rotatedNormal);
	
    v2p_t v2p;
    v2p.clipPosition	=	emptyPatches ? clipPosition : float4(clipPosition.xyz, 0.f);
    v2p.color			=	float4(grassColor1, 1.f); // float4(lerp(grassColor1, grassColor2, smoothstep(-1.f, 1.f, grassColorNoise) * 0.35f), 1.f);
	v2p.grassNormal		=	float4(normalize(rotatedNormal.xyz), 0.f);
	v2p.grassData		=	float4(scaledLocalPos.y, heightInterpolatorZeroToOne, localOffsetAsUVs);
    return v2p;
}


//--------------------------------------------------------------------------------------------------
float RangeMapClamped(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
    float fraction = GetFractionWithinRange(inValue, inStart, inEnd);
    if (fraction < 0.f)
    {
        fraction = 0.f;
    }
    if (fraction > 1.f)
    {
        fraction = 1.f;
    }

    float interPolatedValue = lerp(outStart, outEnd, fraction);
    return interPolatedValue;
}


//-----------------------------------------------------------------------------------------------
float Get2dNoiseFloat(float2 indexXY)
{
    float2 mangledXY = 50.f * frac(indexXY * 0.3183099 + float2(0.71, 0.113));
    return -1.f + 2.f * frac(mangledXY.x * mangledXY.y * (mangledXY.x + mangledXY.y));
}


//-----------------------------------------------------------------------------------------------
float GetCustom2dNoiseFloat(float2 indexXY)
{
    float2 flooredIndex		=	floor(indexXY);
    float  mangled1			=	Get2dNoiseFloat(flooredIndex + float2(0.f, 0.f));
    float  mangled2			=	Get2dNoiseFloat(flooredIndex + float2(1.f, 0.f));
    float  mangled3			=	Get2dNoiseFloat(flooredIndex + float2(0.f, 1.f));
    float  mangled4			=	Get2dNoiseFloat(flooredIndex + float2(1.f, 1.f));
    float2 mangledNumXY		=	smoothstep(0.f, 1.f, frac(indexXY));
	float  mangledX			=	lerp(mangled1, mangled2, mangledNumXY.x);
	float  mangledY			=	lerp(mangled3, mangled4, mangledNumXY.x);
	float  mangledNum		=	lerp(mangledX, mangledY, mangledNumXY.y);
    return mangledNum;
}


//-----------------------------------------------------------------------------------------------
float Smooth2dNoiseFloat(float2 posXY, int numOctaves, float octavePersistance, float octaveScale)
{
    float amplitude = 0.5f;
    float totalNoise = 0.f;
	
	[unroll]
    for (int octaveNum = 0; octaveNum < numOctaves; ++octaveNum)
    {
        float noise		=	GetCustom2dNoiseFloat(posXY);
        totalNoise		+=	noise * amplitude;
        amplitude		*=	octavePersistance;
        posXY			=	posXY * octaveScale;
    }
    return totalNoise;
}


//-----------------------------------------------------------------------------------------------
// Perlin noise is fractal noise with "gradient vector smoothing" applied.
//
// In 2D, gradients are unit-length vectors in various directions with even angular distribution.
//
float Compute2dPerlinNoise( float posX, float posY, float scale, unsigned int numOctaves, float octavePersistence, float octaveScale, bool renormalize, unsigned int seed )
{
	const float OCTAVE_OFFSET = 0.636764989593174f; // Translation/bias to add to each octave
	static const float2 gradients[ 8 ] = // Normalized unit vectors in 8 quarter-cardinal directions
	{
		float2( +0.923879533f, +0.382683432f ),  //  22.5 degrees (ENE)
		float2( +0.382683432f, +0.923879533f ),  //  67.5 degrees (NNE)
		float2( -0.382683432f, +0.923879533f ),  // 112.5 degrees (NNW)
		float2( -0.923879533f, +0.382683432f ),  // 157.5 degrees (WNW)
		float2( -0.923879533f, -0.382683432f ),  // 202.5 degrees (WSW)
		float2( -0.382683432f, -0.923879533f ),  // 247.5 degrees (SSW)
		float2( +0.382683432f, -0.923879533f ),  // 292.5 degrees (SSE)
		float2( +0.923879533f, -0.382683432f )	 // 337.5 degrees (ESE)
	};

	float totalNoise        =   0.f;
	float totalAmplitude    =   0.f;
	float currentAmplitude  =   1.f;
	float invScale          =   (1.f / scale);
	float2 currentPos       =   float2( posX * invScale, posY * invScale );

    [unroll]
	for( unsigned int octaveNum = 0; octaveNum < numOctaves; ++ octaveNum )
	{
		// Determine random unit "gradient vectors" for surrounding corners
		float2 cellMins = float2( floor( currentPos.x ), floor( currentPos.y ) );
		float2 cellMaxs = float2( cellMins.x + 1.f, cellMins.y + 1.f );
		int indexWestX  = (int) cellMins.x;
		int indexSouthY = (int) cellMins.y;
		int indexEastX  = indexWestX  + 1;
		int indexNorthY = indexSouthY + 1;

		unsigned int noiseSW = Get2dNoiseUint( indexWestX, indexSouthY, seed );
		unsigned int noiseSE = Get2dNoiseUint( indexEastX, indexSouthY, seed );
		unsigned int noiseNW = Get2dNoiseUint( indexWestX, indexNorthY, seed );
		unsigned int noiseNE = Get2dNoiseUint( indexEastX, indexNorthY, seed );

		float2 gradientSW = gradients[ noiseSW & 0x00000007 ];
		float2 gradientSE = gradients[ noiseSE & 0x00000007 ];
		float2 gradientNW = gradients[ noiseNW & 0x00000007 ];
		float2 gradientNE = gradients[ noiseNE & 0x00000007 ];

		// Dot each corner's gradient with displacement from corner to position
		float2 displacementFromSW = float2( currentPos.x - cellMins.x, currentPos.y - cellMins.y );
		float2 displacementFromSE = float2( currentPos.x - cellMaxs.x, currentPos.y - cellMins.y );
		float2 displacementFromNW = float2( currentPos.x - cellMins.x, currentPos.y - cellMaxs.y );
		float2 displacementFromNE = float2( currentPos.x - cellMaxs.x, currentPos.y - cellMaxs.y );

		float dotSouthWest = dot( gradientSW, displacementFromSW );
		float dotSouthEast = dot( gradientSE, displacementFromSE );
		float dotNorthWest = dot( gradientNW, displacementFromNW );
		float dotNorthEast = dot( gradientNE, displacementFromNE );

		// Do a smoothed (nonlinear) weighted average of dot results
		float weightEast    =   SmoothStep3( displacementFromSW.x );
		float weightNorth   =   SmoothStep3( displacementFromSW.y );
		float weightWest    =   1.f - weightEast;
		float weightSouth   =   1.f - weightNorth;

		float blendSouth        =   (weightEast * dotSouthEast) + (weightWest * dotSouthWest);
		float blendNorth        =   (weightEast * dotNorthEast) + (weightWest * dotNorthWest);
		float blendTotal        =   (weightSouth * blendSouth) + (weightNorth * blendNorth);
		float noiseThisOctave   =   blendTotal * (1.f / 0.662578106f); // 2D Perlin is in [-.662578106,.662578106]; map to ~[-1,1]
		
		// Accumulate results and prepare for next octave (if any)
		totalNoise          +=  noiseThisOctave * currentAmplitude;
		totalAmplitude      +=  currentAmplitude;
		currentAmplitude    *=  octavePersistence;
		currentPos          *=  octaveScale;
		currentPos.x        +=  OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		currentPos.y        +=  OCTAVE_OFFSET; // Add "irrational" offset to de-align octave grids
		++ seed; // Eliminates octaves "echoing" each other (since each octave is uniquely seeded)
	}
    
    // Re-normalize total noise to within [-1,1] and fix octaves pulling us far away from limits
	if( renormalize && totalAmplitude > 0.f )
	{
		totalNoise /= totalAmplitude;				// Amplitude exceeds 1.0 if octaves are used
		totalNoise = (totalNoise * 0.5f) + 0.5f;	// Map to [0,1]
		totalNoise = SmoothStep3( totalNoise );		// Push towards extents (octaves pull us away)
		totalNoise = (totalNoise * 2.0f) - 1.f;		// Map back to [-1,1]
	}
    
	return totalNoise;
}


//--------------------------------------------------------------------------------------------------
float3 LinearTosRGB(float3 colorToBeConverted)
{
    float3 needsToBeCorrected   =   colorToBeConverted <= float3(0.0031308f, 0.0031308f, 0.0031308f);
    float3 v1                   =   colorToBeConverted * 12.92f;
    float3 v2                   =   pow(colorToBeConverted.xyz, float3(0.41666f, 0.41666f, 0.41666f)) * 1.055f - float3(0.055f, 0.055f, 0.055f);
	
    return lerp(v2, v1, needsToBeCorrected);
}


//--------------------------------------------------------------------------------------------------
static const float3 RED     =   float3(1.f, 0.f, 0.f);
static const float3 GREEN   =   float3(0.f, 1.f, 0.f);
static const float3 BLUE    =   float3(0.f, 0.f, 1.f);
static const float3 GRAY    =   float3(0.5f, 0.5f, 0.5f);
static const float3 WHITE   =   float3(1.f, 1.f, 1.f);


//--------------------------------------------------------------------------------------------------
float3 CalculateHemiLighting(float3 normal, float3 groundColor, float3 skyColor)
{
    return lerp(groundColor, skyColor, 0.5f * normal.z + 0.5f);
}


//--------------------------------------------------------------------------------------------------
float3 CalculateLambertianLighting(float3 normal, float3 viewDir, float3 lightDir, float3 lightColor)
{
    float wrap		=	0.5f; // Poor-mans scattering 
    float dotNL		=	saturate((dot(normal, lightDir) + wrap) / (1.f + wrap));
    float3 lighting =	float3(dotNL.xxx);
	
    float3 backLight	=	saturate((dot(viewDir, -lightDir) + wrap) / (1.f + wrap));
    float3 scatter		=	float3(pow(backLight, 2.f));
    lighting			+=	scatter;
	
    return lightColor * lighting;
}


//--------------------------------------------------------------------------------------------------
float3 CalcualtePhongSpecularLighting(float3 normal, float3 lightDir, float3 viewDir)
{
    float dotNL			=	saturate(dot(normal, lightDir));
    float3 r			=	normalize(reflect(-lightDir, normal));
    float phongValue	=	max(0.f, dot(viewDir, r));
    phongValue			=	pow(phongValue, 32.f);
	
    float3 specular = dotNL * float3(phongValue.xxx);
    return specular;
}


//--------------------------------------------------------------------------------------------------
[earlydepthstencil]
float4 PixelMain(v2p_t input) : SV_Target0
{
    float3 grassBaseColor		=	input.color.rgb;
	float4 grassData			=	input.grassData;
	float3 normal				=	normalize(input.grassNormal.xyz);
	float3 viewDir				=	normalize(ViewDir.xyz);
    float3 color				=	lerp(grassBaseColor * 0.75f, grassBaseColor, smoothstep(0.125f, 0.f, abs(grassData.x)));
	
	// Hemi lighting
    float3 skyColor			=	float3(1.f, 1.f, 0.75f);
	float3 groundColor		=	float3(0.05f, 0.05f, 0.25f);
	
    float3 ambientLighting	=	CalculateHemiLighting(normal, groundColor, skyColor);
	
	// Directional Light 
    float3 lightDir = normalize(float3(-1.f, 1.f, 0.5f));
    float3 lightColor = float3(1.f.xxx);
    float3 diffuseLighting = CalculateLambertianLighting(normal, viewDir, lightDir, lightColor);
	
	// Specular
    float3 specular = CalcualtePhongSpecularLighting(normal, lightDir, viewDir);
	
	// Fake AO
    float ao = RangeMap(pow(grassData.y, 2.f), 0.f, 1.f, 0.125f, 1.f);
	
	float3 lighting			=	diffuseLighting * 0.5f + ambientLighting * 0.5f;
	color					=	color * lighting + specular * 0.25f;
	color					*=	ao;
	
	// color					=	input.color.rgb;
	// // Color correction
	// float  colorCorrectionExp	=	1.f / 2.2f;
    // color						=	pow(color, float3(colorCorrectionExp.xxx));
	return float4(color, 1.f);
}