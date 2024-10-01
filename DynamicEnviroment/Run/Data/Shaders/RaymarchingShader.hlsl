//------------------------------------------------------------------------------------------------
static const int	NUM_RAY_MARCH_ITERATIONS	=	256;
static const float	MAX_RAY_MARCH_DIST			=	1000.f;
static const float	MIN_DIST			        =	0.00001f;


//------------------------------------------------------------------------------------------------
struct vs_input_t
{
	float3 localPosition	:	POSITION;
	float4 color			:	COLOR;
	float2 uv				:	TEXCOORD;
};


//------------------------------------------------------------------------------------------------
struct v2p_t
{
	float4 clipPosition		:	SV_Position;
	float4 color			:	COLOR;
	float2 uv				:	TEXCOORD;
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
    //----------------------------------- (16 byte aligned)
};


//--------------------------------------------------------------------------------------------------
SamplerState diffuseSampler : register(s0);


//--------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
	v2p_t v2p;
    float4 localPosition	=	float4(input.localPosition, 1);
	float4 worldPosition	=	mul(LocalToWorldTransformator, localPosition);
	float4 viewPosition		=	mul(WorldToViewTransformator, worldPosition);
	float4 clipPosition		=	mul(ViewToClipTransformator, viewPosition);
	
    v2p.clipPosition		=	clipPosition;
	v2p.uv					=	input.uv;
    v2p.color				=	input.color;
	return v2p;
}


//--------------------------------------------------------------------------------------------------
float GetFractionWithinRange(float value, float rangeStart, float rangeEnd)
{
    float currentRange = rangeEnd - rangeStart;
    float relativeValPos = (value - rangeStart) / currentRange;
    return relativeValPos;
}


//--------------------------------------------------------------------------------------------------
float RangeMap(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
    float relativeInValPos = GetFractionWithinRange(inValue, inStart, inEnd);
    float outRange = outEnd - outStart;
    float rangeMappedVal = (relativeInValPos * outRange) + outStart;
    return rangeMappedVal;
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


//------------------------------------------------------------------------------------------------
static const float fSQRT_3_OVER_3 = 0.57735026918962576450914878050196f;


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


//-----------------------------------------------------------------------------------------------
// Fast hash of an int32 into a different (unrecognizable) uint32.
//
// Returns an unsigned integer containing 32 reasonably-well-scrambled bits, based on the hash
//	of a given (signed) integer input parameter (position/index) and [optional] seed.  Kind of
//	like looking up a value in an infinitely large table of previously generated random numbers.
//
unsigned int Get1dNoiseUint( int positionX, unsigned int seed )
{
	static const unsigned int BIT_NOISE1 = 0xd2a80a23;
	static const unsigned int BIT_NOISE2 = 0xa884f197;
	static const unsigned int BIT_NOISE3 = 0x1b56c4e9;

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
unsigned int Get2dNoiseUint( int indexX, int indexY, unsigned int seed )
{
	static const int PRIME_NUMBER = 198491317; // Large prime number with non-boring bits
	return Get1dNoiseUint( indexX + (PRIME_NUMBER * indexY), seed );
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
struct Material
{
    float3  color;
    float   signedDist;
};


//--------------------------------------------------------------------------------------------------
float SDFSphere3D(float3 refPos, float3 sphereCenter, float radius)
{
    float signedDist = length(refPos - sphereCenter) - radius;
    return signedDist;
}


//--------------------------------------------------------------------------------------------------
float SDFBox3D(float3 refPos, float3 boxCenter, float3 boxDims)
{
    float3 absDiff = abs(refPos - boxCenter) - boxDims;
    return length(max(absDiff, 0.f)) + min(max(absDiff.x, max(absDiff.y, absDiff.z)), 0.f);
}


//--------------------------------------------------------------------------------------------------
float SDFPlane(float3 refPos, float3 planeCenter)
{
    return (refPos - planeCenter).y;
}


//--------------------------------------------------------------------------------------------------
Material UnionOP(Material a, Material b)
{
	if (a.signedDist < b.signedDist)
    {
        return a;
    }
	else
    {
        return b;
    }
    // Material result = ((a.signedDist < b.signedDist) ? a : b);
    // return result;
}


//--------------------------------------------------------------------------------------------------
Material CalculateSceneSDF(float3 refPos)
{
	static const float SEA_LEVEL			=	0.4f;
	static const float SEA_DEPTH			=	3.f;
	static const float MOUNTAIN_MAX_HEIGHT	=	5.f;
    // // Compute terrain noise values
    // float terrainHeightNoise		=	Compute2dPerlinNoise(refPos.x * 0.25f, refPos.z * 0.35f, 5.f, 5, 0.5f, 2.f, true, 0);
    // // float hilliness					=   0.5f + (0.5f * Compute2dPerlinNoise(refPos.x, refPos.z, 60.f, 3, 0.4f, 2.f, true, 2));
    // // float oceanness					=   0.5f + (0.5f * Compute2dPerlinNoise(refPos.x, refPos.z, 30.f, 5, 0.05f, 2.f, true, 3));
    // // float mountainMaxHeight			=	MOUNTAIN_MAX_HEIGHT - SEA_LEVEL + SEA_DEPTH;
	// // float mountainHeight				=	hilliness * float(mountainMaxHeight);
	// // float terrainHeight				=	SEA_LEVEL - SEA_DEPTH + (MOUNTAIN_MAX_HEIGHT * terrainHeightNoise);
	// float terrainHeight				=	terrainHeightNoise * MOUNTAIN_MAX_HEIGHT;
	
	// oceanness						=	SmoothStart6(oceanness);
	// hilliness						=	SmoothStop2(hilliness);
	// float terrainVarianceLow		=	(1.f - hilliness) * 0.25f;
	// float terrainVarianceHigh		=	hilliness;
	// float terrainVariance			=	RangeMapClamped(terrainHeightNoise, 0.f, 1.f, terrainVarianceLow, terrainVarianceHigh);
	// int   terrainHeightWithHills	=	SEALEVEL - SEADEPTH + (int)(60.f * SmoothStep3(terrainVariance));
	
	// Calculate terrain noise
    float terrainHeightNoise	=	Smooth2dNoiseFloat(refPos.xz * 0.5f, 1, 0.5f, 2.f);
	terrainHeightNoise			=	abs(terrainHeightNoise) * 1.5f;
	terrainHeightNoise			+=	0.1f * Smooth2dNoiseFloat(refPos.xz * 4.f, 6, 0.5f, 2.f);
	
	// Calculate land material
    float3 landColor		=	float3(0.5f, 0.4, 0.4);
	landColor				=	lerp(landColor, landColor * 0.25f, smoothstep(SEA_LEVEL - 0.1f, SEA_LEVEL, terrainHeightNoise));
    Material landMat; 
    landMat.color			=   landColor;
    landMat.signedDist		=   refPos.y + terrainHeightNoise;
	
	// Calculate water material
	float3 shallowWaterColor	=	float3(0.25f, 0.25f, 0.75f);
	float3 deepWaterColor		=	float3(0.025f, 0.025f, 0.15f);
	float3 waterColor			=	lerp(shallowWaterColor, deepWaterColor, smoothstep(SEA_LEVEL, SEA_LEVEL + 0.1f, terrainHeightNoise));
	waterColor					=	lerp(waterColor, WHITE, smoothstep(SEA_LEVEL + 0.0125f, SEA_LEVEL, terrainHeightNoise));
    Material waterMat;
    waterMat.color				=   waterColor;
    waterMat.signedDist			=   refPos.y + SEA_LEVEL;
	
	// Blend water and land
    Material result = UnionOP(landMat, waterMat);
    return result;
}


//--------------------------------------------------------------------------------------------------
float3 CalculateNormal(float3 refPos)
{
    const float  EPSILON    =   0.0001f;
	// Engine coordinate system naming convention (Right-handed XForward), but is used as Dx coordiate convention (Left-handed ZForward)
    const float3 NORTH      =   float3(0.f, EPSILON, 0.f);  
    const float3 WEST       =   float3(-EPSILON, 0.f, 0.f);  
    const float3 SOUTH      =   float3(0.f, -EPSILON, 0.f);  
    const float3 EAST       =   float3(EPSILON, 0.f, 0.f);  
    const float3 FORWARD    =   float3(0.f, 0.f, EPSILON);  
    const float3 BACKWARD   =   float3(0.f, 0.f, -EPSILON);
    
    // Calculate the gradiant change using finite diferencing
    float3 normal;
    normal.x = CalculateSceneSDF(refPos + EAST).signedDist    - CalculateSceneSDF(refPos + WEST).signedDist;
    normal.y = CalculateSceneSDF(refPos + NORTH).signedDist   - CalculateSceneSDF(refPos + SOUTH).signedDist;
    normal.z = CalculateSceneSDF(refPos + FORWARD).signedDist - CalculateSceneSDF(refPos + BACKWARD).signedDist;
    return normalize(normal);
}


//--------------------------------------------------------------------------------------------------
float3 CalculateLighting(float3 normalToSurfaceBeingLit, float3 lightDir, float3 lightColor)
{
    float lightStrength = saturate(dot(lightDir, normalToSurfaceBeingLit));
    return lightColor * lightStrength;
}


//--------------------------------------------------------------------------------------------------
Material RayCast(float3 rayStart, float3 rayDir, int numSteps, float startDist, float maxDist)
{
    Material finalMaterial      =   { float3(0.f.xxx), startDist };
    Material defaultMaterial    =   { float3(0.f.xxx), -1.f };
    
    for (int rayStepIndex = 0; rayStepIndex < numSteps; ++rayStepIndex)
    {
		// Calculate new position to sample scene sdf
        float3 scenePos		=	rayStart + rayDir * finalMaterial.signedDist;
		// Sample scene sdf and fetch the closest objects signed distance
        Material result	    =	CalculateSceneSDF(scenePos);
		// Break when intersection found
        if (abs(result.signedDist)  < MIN_DIST * finalMaterial.signedDist)
        {
            break;
        }
		
		// Update distance marched, and ensure the range marched is within bounds
        finalMaterial.signedDist	+=  result.signedDist;
        finalMaterial.color			=   result.color;
		
		// Travelled too far, no intersection
		if (finalMaterial.signedDist > maxDist)
        {
            return defaultMaterial;
        }
    }
    
    return finalMaterial;
}


//--------------------------------------------------------------------------------------------------
float CalculateShadows(float3 refPos, float3 lightDir)
{
    Material result = RayCast(refPos, lightDir, 64.f, 0.01f, 10.f);
    if (result.signedDist >= 0.f)
    {
        return 0.f;
    }
    else
    {
        return 1.f;
    }
}


//--------------------------------------------------------------------------------------------------
float CalculateAO(float3 refPos, float3 normal)
{
    float ao		=	0.f;
    float stepSize	=	0.1f;
    
    for (float iterations = 0.f; iterations < 5.f; ++iterations)
    {
        float distFactor = 1.f / pow(2.f, iterations);
        ao += distFactor * (iterations * stepSize - CalculateSceneSDF(refPos + normal * iterations * stepSize).signedDist);
    }
    return 1.f - ao;
}


//--------------------------------------------------------------------------------------------------
// Perform sphere tracing for the scene, and fetch color for the fragment
float3 RayMarch(float3 rayStart, float3 rayDir)
{
    Material material		=   RayCast(rayStart, rayDir, NUM_RAY_MARCH_ITERATIONS, 1.f, MAX_RAY_MARCH_DIST);
    // float3 skyColor		=   float3(0.55f, 0.6f, 1.f);
    float  skyLerpFactor	=   exp(saturate(rayDir.y) * -40.f);
	float  sunFactor		=	pow(saturate(dot(LightDir, rayDir)), 8.f);
    float3 horizonColor		=   lerp(float3(0.025f, 0.065f, 0.5f), SkyColor.xyz, skyLerpFactor);
	float3 fogColor			=	lerp(horizonColor, float3(1.f, 0.9f, 0.65f), sunFactor);
    if (material.signedDist < 0.f)
    {
        return fogColor;
    }
    else
    {
		float3 scenePos     =   rayStart + rayDir * material.signedDist;
		// float3 lightDir     =   normalize(float3(1.f, 2.f, -1.f));
		float3 lightColor   =   WHITE;
		float3 normal       =   CalculateNormal(scenePos);
		float  shadowed     =   CalculateShadows(scenePos, LightDir);
		float3 lighting     =   CalculateLighting(normal, LightDir, lightColor);
		lighting            *=  shadowed;
		float3 color        =   material.color * lighting;
		// float ao            =   CalculateAO(scenePos, normal);
		float fogDist		=	distance(rayStart, scenePos);
		float inScatter     =   1.f - exp(-fogDist * fogDist * lerp(0.0005f, 0.001f, sunFactor));	// Scattering light into the camera as distance increases
		float extinction    =   exp(-fogDist * fogDist * 0.01f);									// Scattering/absorbing light before it reaches the viewer/camera
		color               =   color * extinction + fogColor * inScatter;
		return color;
    }
    // return float3(ao.xxx);
}


//--------------------------------------------------------------------------------------------------
[earlydepthstencil]
float4 PixelMain(v2p_t input) : SV_Target0
{
    float2 screenCenteredUVs	=	input.uv - 0.5f;
	float2 pixelCoords		    =	screenCenteredUVs * ScreenRes;

    float3 rayDir               =   normalize(float3(pixelCoords * 2.0 / ScreenRes.y, 1.0));
    float3 rayOrigin            =   float3(-CamPosition.y, CamPosition.z, CamPosition.x);
    rayDir                      =   float3(rayDir.z, -rayDir.x, rayDir.y);
    rayDir                      =   mul(CamRotationMatrix, float4(rayDir, 0.f)).xyz;
    rayDir                      =   float3(-rayDir.y, rayDir.z, rayDir.x);
    float3 color                =   RayMarch(rayOrigin, rayDir);
    
	// Color correction
	float  colorCorrectionExp	=	1.f / 2.2f;
    color						=	pow(color, float3(colorCorrectionExp.xxx));
	return float4(color, 1.f);
}