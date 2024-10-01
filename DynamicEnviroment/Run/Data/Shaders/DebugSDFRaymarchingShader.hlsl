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
    float       CamAspect;
    //----------------------------------- (16 byte aligned)
	float2	    ScreenRes;
	float		FOVScale;
	float	    NearPlaneDist;
    //----------------------------------- (16 byte aligned)
	float4		SkyColor;
	float4		ViewDir;
    //----------------------------------- (16 byte aligned)
};


//--------------------------------------------------------------------------------------------------
Texture2D diffuseTexture	: register(t0);

SamplerState diffuseSampler : register(s0);


//--------------------------------------------------------------------------------------------------
v2p_t VertexMain(vs_input_t input)
{
	v2p_t v2p;
    float4 localPosition	=	float4(input.localPosition, 1);
	float4 worldPosition	=	mul(LocalToWorldTransformator, localPosition);
	float4 viewPosition		=	mul(WorldToViewTransformator, worldPosition);
	float4 clipPosition		=	mul(ViewToClipTransformator, viewPosition);
	v2p.position			=	clipPosition;
	v2p.color				=	input.color * ModelColor;
	v2p.uv					=	input.uv;
	return v2p;
}


//--------------------------------------------------------------------------------------------------
// Convert from Right handed +X forward coordinate system to Left handed +Z forward coordinate system
float3 ConvertToDXConventionZForward(float3 engineConventionXForward)
{
    return float3(-engineConventionXForward.y, engineConventionXForward.z, engineConventionXForward.x);
}


//--------------------------------------------------------------------------------------------------
float GetSDFSphere(float3 sphereCenter, float3 playerRefPos, float sphereRadius)
{
    float signedDistance = length(playerRefPos - sphereCenter) - sphereRadius;
    return signedDistance;
}


//--------------------------------------------------------------------------------------------------
float GetSDFFunkyBall(float3 ballCenter, float3 refPos, float defaultBallRadius)
{
	// Get the fractional part of the time, this basically makes it so that the we always get a between of 0-1
	float fractionalTime	=	frac(TimeElapsedSeconds * 0.5f);
	// 
	// Parabola Note:
	// Vertex is the minimum/maximum point in a parabola
	// Goal is to have a parabola with a vertex of 1, and the x intercept of 0, and 1
	// Using the intercepts we get the equation of the parabola to be x * (1 - x), (1 - x) as we want a parabola going downwards
	// Now to find the vertex we can subsitute the x value at the mid point of the parabola 0.5f, we get a y value of 0.25f
	// 1/0.25f * x * (1 - x) ==> 4 * x * (1.f - x)
	float ballHeight		=	4.f * fractionalTime * (1.f - fractionalTime);
	float3 ballHeightDisp	=	float3(0.f, ballHeight, 0.f);
    float signedDistance	=	length(refPos - ballCenter - ballHeightDisp) - defaultBallRadius;
    return signedDistance;
}


//--------------------------------------------------------------------------------------------------
float GetSDFPlane(float3 planeCenter, float3 refPos)
{
    float signedDistance = (refPos - planeCenter).y;
    return signedDistance;
}


//--------------------------------------------------------------------------------------------------
float CalculateSceneSDF(float3 scenePos)
{
    float signedDistance1 =	GetSDFFunkyBall(float3(0.f, 0.f, 0.f), scenePos, 0.25f);
    float signedDistance2 =	GetSDFPlane(float3(0.f, -0.25f, 0.f), scenePos);
	
    return min(signedDistance1, signedDistance2); // Min: combine objects
}


//--------------------------------------------------------------------------------------------------
float3 CalculateNormal(float3 scenePos)
{
    float2 deltaDisp = float2(0.0001f, 0.f);
	// Calculate the gradient, which in this case is the surface normal
    return normalize(float3( CalculateSceneSDF(scenePos + deltaDisp.xyy) - CalculateSceneSDF(scenePos - deltaDisp.xyy), 
							 CalculateSceneSDF(scenePos + deltaDisp.yxy) - CalculateSceneSDF(scenePos - deltaDisp.yxy), 
							 CalculateSceneSDF(scenePos + deltaDisp.yyx) - CalculateSceneSDF(scenePos - deltaDisp.yyx) ));
}


//--------------------------------------------------------------------------------------------------
float RaySphereCast(float3 rayOrigin, float3 rayDir)
{
	// Raymarch through the scene
    float rayMarchStepSize = NearPlaneDist;
    for (int rayStepCount = 0; rayStepCount < 100; ++rayStepCount)
    {
		// Calculate pos to sample sdf
        float3 scenePos = rayOrigin + rayMarchStepSize * rayDir;
		// Sample SDF to get the closest object's signed distance
        float sceneSD = CalculateSceneSDF(scenePos);
		
		// We hit something
		if (sceneSD < 0.001f)
        {
            break;
        }
		
		// If no hit then step again
        rayMarchStepSize += sceneSD;
		
		// Clip if we are beyond the far clip plane
		if (rayMarchStepSize > 20.f)
        {
            break;
        }
    }
	
	if (rayMarchStepSize > 20.f)
    {
        rayMarchStepSize = -1.f;
    }
	
    return rayMarchStepSize;
}


//--------------------------------------------------------------------------------------------------
[earlydepthstencil]
float4 PixelMain(v2p_t input) : SV_Target0
{
	// float4 color	=	diffuseTexture.Sample(diffuseSampler, input.uv);
	
	// fragment coordinates centered at the origin
    float2 fragCoords	=	input.uv * 2.f - 1.f;
	
	// DirectX coordinate system left handed Z forward
	float3 rayOrigin	=	ConvertToDXConventionZForward(CamPosition); 
	
	// Scale by aspect ratio and fov to ensure no distortions
	float3 rayDir		=	normalize(float3(1.f, -fragCoords.x * CamAspect * FOVScale, fragCoords.y * FOVScale)); // Engine convention: Right handed +X Forward
	rayDir				=	mul(CamRotationMatrix, float4(rayDir, 0.f)).xyz;
	rayDir				=	ConvertToDXConventionZForward(rayDir);
	
    
	// Defualt fragment color
	float3 fragColor	=	float3(0.4f, 0.75f, 1.f) - 0.7f * rayDir.y;
	fragColor			=	lerp(fragColor, float3(0.7f, 0.75f, 0.8f), exp(-10.f * rayDir.y));	
	
	// Raymarch through the scene
    float rayMarchStepSize = RaySphereCast(rayOrigin, rayDir);
	
	if (rayMarchStepSize > 0.f)
    {
        float3 intersectPos			 =	rayOrigin + rayMarchStepSize * rayDir; // calculate where intersection happened
		float3 objectNormal			 =	CalculateNormal(intersectPos);
		
		const float3 sunColor		 =	float3(7.f, 4.5f, 3.f);
		const float3 skyColor		 =	float3(0.5f, 0.8f, 0.9f);
		const float3 bounceColor	 =	float3(0.7f, 0.3f, 0.2f); // color used to eliminate pure blacks
		const float3 matte			 =  float3(0.2f, 0.2f, 0.2f); // default color, like for example the grass in reality will not have a green of 1.f, but 0.15f-0.2f
		
		float3 sunDir				 =	normalize(float3(0.8f, 0.4f, -0.2f));
		float  sunDiffuseStrength	 =	clamp(dot(objectNormal, sunDir), 0.f, 1.f);	// Light direction is inverted and sent via constant buffer 
		// Adding a bias to intersect pos to prevent self collisions
		float  sunShadow			 =	step(RaySphereCast(intersectPos + objectNormal * 0.001f, sunDir), 0.f);	// If sphere cast hit nothing then, is not in shadow return 1, else return 0
		float  skyDiffuseStrength	 =	clamp(0.5f + 0.5f * dot(objectNormal, float3(0.f, 1.f, 0.f)), 0.f, 1.f);
		float  bounceDiffuseStrength =	clamp(0.5f + 0.5f * dot(objectNormal, float3(0.f, -1.f, 0.f)), 0.f, 1.f); // Useful for any intersected surfaces found at the bottom that have trouble getting direct light
		
        fragColor					 =	matte * sunColor * sunDiffuseStrength * sunShadow; // float3(1.f, 1.f, 1.f);
		fragColor					+=	matte * skyColor * skyDiffuseStrength;
		fragColor					+=	matte * bounceColor * bounceDiffuseStrength;
    }
	
	
	// Gamma correction
    fragColor = pow(fragColor, float3(0.4545f.xxx));
	
    return float4(fragColor, 1.f);
}