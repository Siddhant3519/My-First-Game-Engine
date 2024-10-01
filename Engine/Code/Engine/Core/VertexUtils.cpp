#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Mat44.hpp"


//--------------------------------------------------------------------------------------------------
void TransformVertexArrayXY3D(int numVerts, Vertex_PCU* verts, float scaleXY, float rotationDegreesAboutZ, Vec2 const& translationXY)
{
	for (int vertIndex = 0; vertIndex < numVerts; ++vertIndex)
	{
		Vec3& pos = verts[vertIndex].m_position;
		TransformPositionXY3D(pos, scaleXY, rotationDegreesAboutZ, translationXY);
	}
}


//--------------------------------------------------------------------------------------------------
void TransformVertexArray3D(int numVerts, Vertex_PCU* verts, Mat44 transform)
{
	for (int vertIndex = 0; vertIndex < numVerts; ++vertIndex)
	{
		Vec3& pos = verts[vertIndex].m_position;
		pos = transform.TransformPosition3D(pos);
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Vec2 const& boneStart, Vec2 const& boneEnd, float radius, Rgba8 const& color)
{
	Vec2 dispFromBoneStartToBoneEnd = boneEnd - boneStart;
	Vec2 dispFromBoneStartToBoneEndRotated90Degrees = dispFromBoneStartToBoneEnd.GetRotated90Degrees();
	Vec2 dispFromBoneStartToBoneEndRotated90DegreesClamped = dispFromBoneStartToBoneEndRotated90Degrees.GetClamped(radius);


	Vec2 BR = boneStart - dispFromBoneStartToBoneEndRotated90DegreesClamped; // BottomRight
	Vec2 TR = boneEnd - dispFromBoneStartToBoneEndRotated90DegreesClamped;
	Vec2 TL = boneEnd + dispFromBoneStartToBoneEndRotated90DegreesClamped;
	Vec2 BL = boneStart + dispFromBoneStartToBoneEndRotated90DegreesClamped; // BottomLeft

	verts.push_back(Vertex_PCU(Vec3(BR.x, BR.y), color));
	verts.push_back(Vertex_PCU(Vec3(TR.x, TR.y), color));
	verts.push_back(Vertex_PCU(Vec3(TL.x, TL.y), color));

	verts.push_back(Vertex_PCU(Vec3(BR.x, BR.y), color));
	verts.push_back(Vertex_PCU(Vec3(TL.x, TL.y), color));
	verts.push_back(Vertex_PCU(Vec3(BL.x, BL.y), color));

	int numOfTriangles = 18;
	constexpr int vertsPerTriangle = 3;
	float degreesPerSide = 180.f / (float)numOfTriangles;

	float currentOrientation = (TR - boneEnd).GetOrientationDegrees();


	for (int sideNum = 0; sideNum < numOfTriangles; ++sideNum)
	{
		Vec3 sectorTip = Vec3(boneEnd.x, boneEnd.y);
		Vec3 vertex1 = sectorTip + MakeFromPolarDegrees(currentOrientation, radius);
		Vec3 vertex2 = sectorTip + MakeFromPolarDegrees(currentOrientation + degreesPerSide, radius);

		verts.push_back(Vertex_PCU(sectorTip, color));
		verts.push_back(Vertex_PCU(vertex1, color));
		verts.push_back(Vertex_PCU(vertex2, color));

		currentOrientation = currentOrientation + degreesPerSide;
	}

	currentOrientation = (BL - boneStart).GetOrientationDegrees();

	for (int sideNum = 0; sideNum < numOfTriangles; ++sideNum)
	{
		Vec3 sectorTip = Vec3(boneStart.x, boneStart.y);
		Vec3 vertex1 = sectorTip + MakeFromPolarDegrees(currentOrientation, radius);
		Vec3 vertex2 = sectorTip + MakeFromPolarDegrees(currentOrientation + degreesPerSide, radius);

		verts.push_back(Vertex_PCU(sectorTip, color));
		verts.push_back(Vertex_PCU(vertex1, color));
		verts.push_back(Vertex_PCU(vertex2, color));

		currentOrientation = currentOrientation + degreesPerSide;
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForDisc2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, Rgba8 const& color)
{
	int numOfTriangles = 32;
	float degreesPerSide = 360.f / (float)numOfTriangles;
	float currentOrientation = 0.f;

	for (int sideNum = 0; sideNum < numOfTriangles; ++sideNum)
	{
		Vec3 sectorTip = Vec3(center.x, center.y);
		Vec2 sectorTipUV = Vec2(0.5f, 0.5f);
		Vec3 localVertex1 = MakeFromPolarDegrees(currentOrientation, radius);
		Vec3 vertex1 = sectorTip + localVertex1;
		float vertex1U = RangeMap(localVertex1.x, -radius, radius, 0.f, 1.f);
		float vertex1V = RangeMap(localVertex1.y, -radius, radius, 0.f, 1.f);
		Vec3 localVertex2 = MakeFromPolarDegrees(currentOrientation + degreesPerSide, radius);
		Vec3 vertex2 = sectorTip + localVertex2;
		float vertex2U = RangeMap(localVertex2.x, -radius, radius, 0.f, 1.f);
		float vertex2V = RangeMap(localVertex2.y, -radius, radius, 0.f, 1.f);

		verts.push_back(Vertex_PCU(sectorTip, color, sectorTipUV));
		verts.push_back(Vertex_PCU(vertex1, color, Vec2(vertex1U, vertex1V)));
		verts.push_back(Vertex_PCU(vertex2, color, Vec2(vertex2U, vertex2V)));

		currentOrientation = currentOrientation + degreesPerSide;
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForRing2D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec2 const& center, float radius, float thickness, Rgba8 const& color)
{
	int numOfQuads = 32;
	float degreesPerSide = 360.f / (float)numOfQuads;
	float currentOrientation = 0.f;
	float halfThickness = thickness * 0.5f;

	Vec3 sectorTip = Vec3(center.x, center.y);

	Vec3 currentVertexStart = sectorTip + MakeFromPolarDegrees(currentOrientation, radius - halfThickness);
	Vec3 currentVertexEnd = sectorTip + MakeFromPolarDegrees(currentOrientation, radius + halfThickness);
	currentOrientation += degreesPerSide;
	Vec3 nextVertexStart = sectorTip + MakeFromPolarDegrees(currentOrientation, radius - halfThickness);
	Vec3 nextVertexEnd = sectorTip + MakeFromPolarDegrees(currentOrientation, radius + halfThickness);
	AddVertsForQuad3D(verts, indexes, currentVertexStart, currentVertexEnd, nextVertexEnd, nextVertexStart, color);

	currentOrientation += degreesPerSide;
	int currentIndex = (int)verts.size();
	for (int quadNum = 1; quadNum < numOfQuads; ++quadNum)
	{
		nextVertexEnd = sectorTip + MakeFromPolarDegrees(currentOrientation, radius + halfThickness);
		nextVertexStart = sectorTip + MakeFromPolarDegrees(currentOrientation, radius - halfThickness);
		verts.push_back(Vertex_PCU(nextVertexEnd, color, Vec2()));
		verts.push_back(Vertex_PCU(nextVertexStart, color, Vec2()));

		indexes.push_back(currentIndex - 1);
		indexes.push_back(currentIndex - 2);
		indexes.push_back(currentIndex);

		indexes.push_back(currentIndex - 1);
		indexes.push_back(currentIndex);
		indexes.push_back(currentIndex + 1);

		currentOrientation += degreesPerSide;
		currentIndex += 2;
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForBorderedHexagon2D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec2 const& center, float radius, float thickness, Rgba8 const& color)
{
	constexpr int	NUM_OF_HEX_SIDES = 6;
	constexpr float	DEGREES_PER_SIDE = 60.f;
	
	float currentOrientation	= 0.f;
	float halfThickness			= thickness * 0.5f;

	Vec3 sectorTip = Vec3(center.x, center.y);

	Vec3 currentVertexStart		=	sectorTip + MakeFromPolarDegrees(currentOrientation, radius - halfThickness);
	Vec3 currentVertexEnd		=	sectorTip + MakeFromPolarDegrees(currentOrientation, radius + halfThickness);
	currentOrientation			+=	DEGREES_PER_SIDE;
	Vec3 nextVertexStart		=	sectorTip + MakeFromPolarDegrees(currentOrientation, radius - halfThickness);
	Vec3 nextVertexEnd			=	sectorTip + MakeFromPolarDegrees(currentOrientation, radius + halfThickness);
	int firstVertIndex			=	(int)verts.size();
	AddVertsForQuad3D(verts, indexes, currentVertexStart, currentVertexEnd, nextVertexEnd, nextVertexStart, color);

	currentOrientation +=	DEGREES_PER_SIDE;
	int currentIndex	=	(int)verts.size();
	for (int quadNum = 1; quadNum < NUM_OF_HEX_SIDES - 1; ++quadNum)
	{
		nextVertexEnd	= sectorTip + MakeFromPolarDegrees(currentOrientation, radius + halfThickness);
		nextVertexStart = sectorTip + MakeFromPolarDegrees(currentOrientation, radius - halfThickness);
		verts.emplace_back(nextVertexEnd, color);
		verts.emplace_back(nextVertexStart, color);

		indexes.emplace_back(currentIndex - 1);
		indexes.emplace_back(currentIndex - 2);
		indexes.emplace_back(currentIndex);

		indexes.emplace_back(currentIndex - 1);
		indexes.emplace_back(currentIndex);
		indexes.emplace_back(currentIndex + 1);

		currentOrientation += DEGREES_PER_SIDE;
		currentIndex += 2;
	}

	indexes.emplace_back(currentIndex - 1);
	indexes.emplace_back(currentIndex - 2);
	indexes.emplace_back(firstVertIndex + 1);

	indexes.emplace_back(currentIndex - 1);
	indexes.emplace_back(firstVertIndex + 1);
	indexes.emplace_back(firstVertIndex);
}


//--------------------------------------------------------------------------------------------------
void AddVertsForBorderedHexagon2D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec2 const& center, float radius, float thickness, Rgba8 const& color)
{
	constexpr int	NUM_OF_HEX_SIDES = 6;
	constexpr float	DEGREES_PER_SIDE = 60.f;
	
	float currentOrientation	= 0.f;
	float halfThickness			= thickness * 0.5f;

	Vec3 sectorTip = Vec3(center.x, center.y);

	Vec3 currentVertexStart		=	sectorTip + MakeFromPolarDegrees(currentOrientation, radius - halfThickness);
	Vec3 currentVertexEnd		=	sectorTip + MakeFromPolarDegrees(currentOrientation, radius + halfThickness);
	currentOrientation			+=	DEGREES_PER_SIDE;
	Vec3 nextVertexStart		=	sectorTip + MakeFromPolarDegrees(currentOrientation, radius - halfThickness);
	Vec3 nextVertexEnd			=	sectorTip + MakeFromPolarDegrees(currentOrientation, radius + halfThickness);
	int firstVertIndex			=	(int)verts.size();
	AddVertsForQuad3D(verts, indexes, currentVertexStart, currentVertexEnd, nextVertexEnd, nextVertexStart, color);

	currentOrientation +=	DEGREES_PER_SIDE;
	int currentIndex	=	(int)verts.size();
	currentVertexStart	=	nextVertexStart;
	currentVertexEnd	=	nextVertexEnd;
	for (int quadNum = 1; quadNum < NUM_OF_HEX_SIDES - 1; ++quadNum)
	{
		nextVertexEnd	= sectorTip + MakeFromPolarDegrees(currentOrientation, radius + halfThickness);
		nextVertexStart = sectorTip + MakeFromPolarDegrees(currentOrientation, radius - halfThickness);

		Vec3 dispCVSToCVE	=	currentVertexEnd - currentVertexStart;
		Vec3 dispCVEToNVE	=	nextVertexEnd - currentVertexEnd;
		Vec3 normalE		=	CrossProduct3D(dispCVSToCVE, dispCVEToNVE);
		normalE.Normalize();

		Vec3 dispCVSToNVE	=	nextVertexEnd - currentVertexStart;
		Vec3 dispNVEToNVS	=	nextVertexStart - nextVertexEnd;
		Vec3 normalS		=	CrossProduct3D(dispCVSToNVE, dispNVEToNVS);
		normalS.Normalize();

		Vec3 tangents;
		Vec3 binormal;

		verts.emplace_back(nextVertexEnd,	color, Vec2(), tangents, binormal, normalE);
		verts.emplace_back(nextVertexStart, color, Vec2(), tangents, binormal, normalS);

		indexes.emplace_back(currentIndex - 1);
		indexes.emplace_back(currentIndex - 2);
		indexes.emplace_back(currentIndex);

		indexes.emplace_back(currentIndex - 1);
		indexes.emplace_back(currentIndex);
		indexes.emplace_back(currentIndex + 1);

		currentVertexStart	=	nextVertexStart;
		currentVertexEnd	=	nextVertexEnd;
		currentOrientation +=	DEGREES_PER_SIDE;
		currentIndex += 2;
	}

	indexes.emplace_back(currentIndex - 1);
	indexes.emplace_back(currentIndex - 2);
	indexes.emplace_back(firstVertIndex + 1);

	indexes.emplace_back(currentIndex - 1);
	indexes.emplace_back(firstVertIndex + 1);
	indexes.emplace_back(firstVertIndex);

	CalculateTangentSpaceVectors(verts, indexes);
}


//--------------------------------------------------------------------------------------------------
void AddVertsForHexagon2D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec2 const& center, float circumRadius, Rgba8 const& color)
{	
	constexpr int	NUM_OF_HEX_SIDES = 6;
	constexpr float	DEGREES_PER_SIDE = 60.f;

	float	currentOrientation	=	0.f;
	int		indexCount			=	(int)verts.size();
	int		hexCenterIndex		=	indexCount;

	Vec3 hexCenter = Vec3(center.x, center.y);
	verts.emplace_back(hexCenter, color);
	indexes.emplace_back(indexCount);
	++indexCount;

	Vec3 currentVert	=	hexCenter + MakeFromPolarDegrees(currentOrientation, circumRadius);
	currentOrientation +=	DEGREES_PER_SIDE;
	Vec3 nextVert		=	hexCenter + MakeFromPolarDegrees(currentOrientation, circumRadius);
	
	verts.emplace_back(currentVert, color);
	verts.emplace_back(nextVert, color);
	
	indexes.emplace_back(indexCount);
	indexes.emplace_back(indexCount + 1);

	int const firstVertIndex	=	indexCount;
	indexCount					=	(int)verts.size();
	currentOrientation			+=	DEGREES_PER_SIDE;
	for (int triIndex = 1; triIndex < NUM_OF_HEX_SIDES - 1; ++triIndex)
	{
		nextVert = hexCenter + MakeFromPolarDegrees(currentOrientation, circumRadius);
		verts.emplace_back(nextVert, color);

		indexes.emplace_back(hexCenterIndex);
		indexes.emplace_back(indexCount - 1);
		indexes.emplace_back(indexCount);
		
		currentOrientation += DEGREES_PER_SIDE;
		++indexCount;
	}

	indexes.emplace_back(hexCenterIndex);
	indexes.emplace_back(indexCount - 1);
	indexes.emplace_back(firstVertIndex);
}


//--------------------------------------------------------------------------------------------------
void AddVertsForHexagon2D(std::vector <Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec2 const& center, float circumRadius, Rgba8 const& color /*= Rgba8::WHITE*/)
{
	constexpr int	NUM_OF_HEX_SIDES = 6;
	constexpr float	DEGREES_PER_SIDE = 60.f;

	float	currentOrientation	=	0.f;
	int		indexCount			=	(int)verts.size();
	int		hexCenterIndex		=	indexCount;

	Vec3 hexCenter = Vec3(center.x, center.y);

	Vec3 currentVert	=	hexCenter + MakeFromPolarDegrees(currentOrientation, circumRadius);
	currentOrientation +=	DEGREES_PER_SIDE;
	Vec3 nextVert		=	hexCenter + MakeFromPolarDegrees(currentOrientation, circumRadius);
	

	Vec3 dispFromHCToCV = currentVert - hexCenter;
	Vec3 dispFromCVToNV = nextVert - currentVert;
	Vec3 normal			= CrossProduct3D(dispFromHCToCV, dispFromCVToNV);
	normal.Normalize();

	Vec3 tangent;
	Vec3 binormal;

	verts.emplace_back(hexCenter, color, Vec2(), tangent, binormal, normal);
	indexes.emplace_back(indexCount);
	++indexCount;
	verts.emplace_back(currentVert,  color, Vec2(), tangent, binormal, normal);
	verts.emplace_back(nextVert,	 color, Vec2(), tangent, binormal, normal);
	
	indexes.emplace_back(indexCount);
	indexes.emplace_back(indexCount + 1);

	int const firstVertIndex	=	indexCount;
	indexCount					=	(int)verts.size();
	currentOrientation			+=	DEGREES_PER_SIDE;
	currentVert					=	nextVert;
	for (int triIndex = 1; triIndex < NUM_OF_HEX_SIDES - 1; ++triIndex)
	{
		nextVert = hexCenter + MakeFromPolarDegrees(currentOrientation, circumRadius);
		
		dispFromHCToCV =	currentVert - hexCenter;
		dispFromCVToNV =	nextVert - currentVert;
		normal			=	CrossProduct3D(dispFromHCToCV, dispFromCVToNV);
		normal.Normalize();

		verts.emplace_back(nextVert, color, Vec2(), tangent, binormal, normal);

		indexes.emplace_back(hexCenterIndex);
		indexes.emplace_back(indexCount - 1);
		indexes.emplace_back(indexCount);
		
		currentOrientation +=	DEGREES_PER_SIDE;
		currentVert			=	nextVert;
		++indexCount;
	}

	indexes.emplace_back(hexCenterIndex);
	indexes.emplace_back(indexCount - 1);
	indexes.emplace_back(firstVertIndex);

	CalculateTangentSpaceVectors(verts, indexes);
}


//--------------------------------------------------------------------------------------------------
//#ToDo: Optimize
void AddVertsForRing2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, float thickness, Rgba8 const& color)
{
	int numOfQuads = 32;
	float degreesPerSide = 360.f / (float)numOfQuads;
	float currentOrientation = 0.f;
	float halfThickness = thickness * 0.5f;

	Vec3 sectorTip = Vec3(center.x, center.y);

	Vec3 currentVertexStart	= sectorTip + MakeFromPolarDegrees(currentOrientation, radius - halfThickness);
	Vec3 currentVertexEnd	= sectorTip + MakeFromPolarDegrees(currentOrientation, radius + halfThickness);
	currentOrientation		+= degreesPerSide;
	Vec3 nextVertexStart	= sectorTip + MakeFromPolarDegrees(currentOrientation, radius - halfThickness);
	Vec3 nextVertexEnd		= sectorTip + MakeFromPolarDegrees(currentOrientation, radius + halfThickness);
	AddVertsForQuad3D(verts, currentVertexStart, currentVertexEnd, nextVertexEnd, nextVertexStart, color);

	currentVertexStart	= nextVertexStart;
	currentVertexEnd	= nextVertexEnd;
	currentOrientation += degreesPerSide;

	for (int quadNum = 1; quadNum < numOfQuads; ++quadNum)
	{
		nextVertexEnd = sectorTip + MakeFromPolarDegrees(currentOrientation, radius + halfThickness);
		nextVertexStart = sectorTip + MakeFromPolarDegrees(currentOrientation, radius - halfThickness);
		AddVertsForQuad3D(verts, currentVertexStart, currentVertexEnd, nextVertexEnd, nextVertexStart, color);

		currentVertexStart	= nextVertexStart;
		currentVertexEnd	= nextVertexEnd;
		currentOrientation += degreesPerSide;
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForSphere3D(std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, Rgba8 const& color, AABB2 const& UVs, int numLatitudeSlices)
{
	int numLongitudeSlices = 2 * numLatitudeSlices;

	float longitudeSliceAngle = 360.f / numLongitudeSlices;
	float latitudeSliceAngle = 180.f / numLatitudeSlices;
	float currentLongitudeAngle = 0.f;
	float currentLatitudeAngle = -90.f;

	Vec3 BL;
	Vec3 BR;
	Vec3 TR;
	Vec3 TL;

	Vec2 uvBL;
	Vec2 uvBR;
	Vec2 uvTR;
	Vec2 uvTL;

	for (int longitudeSlice = 0; longitudeSlice < numLongitudeSlices; ++longitudeSlice)
	{
		currentLatitudeAngle = -90.0f;
		for (int latitudeSlice = 0; latitudeSlice < numLatitudeSlices; ++latitudeSlice)
		{
			TL = center + Vec3::MakeFromPolarDegrees(currentLatitudeAngle, currentLongitudeAngle, radius);
			TR = center + Vec3::MakeFromPolarDegrees(currentLatitudeAngle, currentLongitudeAngle + (longitudeSliceAngle), radius);
			BR = center + Vec3::MakeFromPolarDegrees(currentLatitudeAngle + (latitudeSliceAngle), currentLongitudeAngle + (longitudeSliceAngle), radius);
			BL = center + Vec3::MakeFromPolarDegrees(currentLatitudeAngle + (latitudeSliceAngle), currentLongitudeAngle, radius);

			uvBL.x = RangeMapClamped(currentLongitudeAngle, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			uvBL.y = RangeMapClamped(currentLatitudeAngle + latitudeSliceAngle, -90.f, 90.f, UVs.m_maxs.y, UVs.m_mins.y);

			uvTR.x = RangeMapClamped(currentLongitudeAngle + longitudeSliceAngle, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			uvTR.y = RangeMapClamped(currentLatitudeAngle, -90.f, 90.f, UVs.m_maxs.y, UVs.m_mins.y);

			AddVertsForQuad3D(verts, BL, BR, TR, TL, color, AABB2(uvBL.x, uvBL.y, uvTR.x, uvTR.y));

			currentLatitudeAngle += latitudeSliceAngle;
		}
		currentLongitudeAngle += longitudeSliceAngle;
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForSphere3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec3 const& center, float radius, float numSlices, float numStacks, Rgba8 const& color, AABB2 const& UVs)
{
	int vertSize = int(verts.size());
	//----------------------------------------------------------------------------------------------------------------------
	// 1. Add all unique vertex positions
	//----------------------------------------------------------------------------------------------------------------------
	// Add bottom vert (bottom of sphere)
	Vec3 bottomVert = center + Vec3::MakeFromSphericalDegrees(0.0f, 90.0f, radius);
	Vec3 bottomNormal = (bottomVert - center).GetNormalized();
	verts.push_back(Vertex_PCUTBN(bottomVert, color, Vec2(), Vec3::ZERO, Vec3::ZERO, bottomNormal));
	// Loop and add all unique verts for stacks and slices EXECEPT top vert (top of sphere)
	float yawDegreesPerSlice = (360.0f / numSlices);
	float pitchDegreesPerStack = (180.0f / numStacks);
	for (int currentSlice = 0; currentSlice < numSlices; currentSlice++)
	{
		float currentYawDegrees = static_cast<float>(currentSlice) * yawDegreesPerSlice;
		for (int currentStack = 1; currentStack < numStacks; currentStack++)
		{
			float currentPitchDegrees = 90.0f - (currentStack * pitchDegreesPerStack);
			//----------------------------------------------------------------------------------------------------------------------
			// Verts			
			Vec3 vertsBottomLeft = center + Vec3::MakeFromSphericalDegrees(currentYawDegrees, currentPitchDegrees, radius);		// then add latitudeOffset to TopRight to add verts for BottomRight		// Yaw to the right then Pitch down (positive)
			//----------------------------------------------------------------------------------------------------------------------
			// UVs
			Vec2 UV_BottomLeft;
			UV_BottomLeft.x = RangeMapClamped(currentYawDegrees, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x);
			UV_BottomLeft.y = RangeMapClamped(currentPitchDegrees, -90.0f, 90.0f, UVs.m_maxs.y, UVs.m_mins.y);
			// Normal
			Vec3 normal = (vertsBottomLeft - center).GetNormalized();
			//----------------------------------------------------------------------------------------------------------------------
			// Pushback verts at currentYaw, with increasing pitch (increasing stack, same slice)
			verts.push_back(Vertex_PCUTBN(vertsBottomLeft, color, UV_BottomLeft, Vec3::ZERO, Vec3::ZERO, normal));
		}
	}
	// Add top vert (top of sphere)
	Vec3 topVert = center + Vec3::MakeFromSphericalDegrees(0.0f, -90.0f, radius);
	Vec3 topNormal = (topVert - center).GetNormalized();
	verts.push_back(Vertex_PCUTBN(topVert, color, Vec2(0.0f, 1.0f), Vec3::ZERO, Vec3::ZERO, topNormal));
	//----------------------------------------------------------------------------------------------------------------------
	// 2. Add index List to form a sphere using unique vertex positions
	//----------------------------------------------------------------------------------------------------------------------
	int vertsPerStack = int(numStacks - 1);		// Excluding bottom and top verts, only quads
	//----------------------------------------------------------------------------------------------------------------------
	// 2a. Tri at bottom of sphere
	//----------------------------------------------------------------------------------------------------------------------
	unsigned int BL = vertSize;
	for (int currentSlice = 0; currentSlice < numSlices; currentSlice++)
	{
		if (currentSlice == (numSlices - 1))
		{
			// Special case for the final tri to reconnect with the starting vert
			unsigned int TL = vertSize + (currentSlice * vertsPerStack) + 1;
			unsigned int TR = BL + 1;
			// Add Tri for bottom of sphere
			indexes.push_back(BL);
			indexes.push_back(TR);
			indexes.push_back(TL);
		}
		else
		{
			unsigned int TL = vertSize + (currentSlice * vertsPerStack) + 1;
			unsigned int TR = TL + vertsPerStack;
			// Add Tri for bottom of sphere
			indexes.push_back(BL);
			indexes.push_back(TR);
			indexes.push_back(TL);
		}
	}
	//----------------------------------------------------------------------------------------------------------------------
	// 2b. Tri at top of sphere
	//----------------------------------------------------------------------------------------------------------------------
	unsigned int topVertIndex = int(verts.size() - 1);
	for (int currentSlice = 0; currentSlice < numSlices; currentSlice++)
	{
		if (currentSlice == (numSlices - 1))
		{
			// Special case for the final tri to reconnect with the starting vert
			BL = vertSize + (currentSlice * vertsPerStack) + vertsPerStack;
			unsigned BR = vertSize + vertsPerStack;
			// Add Tri for bottom of sphere
			indexes.push_back(topVertIndex);
			indexes.push_back(BL);
			indexes.push_back(BR);
		}
		else
		{
			BL = vertSize + (currentSlice * vertsPerStack) + vertsPerStack;
			unsigned int BR = BL + vertsPerStack;
			// Add Tri for bottom of sphere
			indexes.push_back(topVertIndex);
			indexes.push_back(BL);
			indexes.push_back(BR);
		}
	}
	//----------------------------------------------------------------------------------------------------------------------
	// 2c. Quads for faces of sphere
	//----------------------------------------------------------------------------------------------------------------------
	for (int currentSlice = 0; currentSlice < numSlices; currentSlice++)
	{
		for (int currentStack = 0; currentStack < (numStacks - 2); currentStack++)
		{
			if (currentSlice == (numSlices - 1))
			{
				// Special case for the final tri to reconnect with the starting vert
				BL = vertSize + (currentSlice * vertsPerStack) + 1 + currentStack;
				unsigned int BR = vertSize + 1 + currentStack;
				unsigned int TL = BL + 1;
				unsigned int TR = BR + 1;
				// Add quads as I "move" up the stack
				indexes.push_back(BL);	// BL
				indexes.push_back(BR);	// BR
				indexes.push_back(TR);	// TR
				indexes.push_back(BL);	// BL
				indexes.push_back(TR);	// TR
				indexes.push_back(TL);	// TL
			}
			else
			{
				BL = vertSize + (currentSlice * vertsPerStack) + 1 + currentStack;
				unsigned int BR = BL + vertsPerStack;
				unsigned int TL = BL + 1;
				unsigned int TR = BR + 1;
				// Add quads as I "move" up the stack
				indexes.push_back(BL);	// BL
				indexes.push_back(BR);	// BR
				indexes.push_back(TR);	// TR
				indexes.push_back(BL);	// BL
				indexes.push_back(TR);	// TR
				indexes.push_back(TL);	// TL
			}
		}
	}
	CalculateTangentSpaceVectors(verts, indexes);
}


//--------------------------------------------------------------------------------------------------
void AddVertsForSphere3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec3 const& center, float radius, float numSlices, float numStacks, Rgba8 const& color /*= Rgba8::WHITE*/, AABB2 const& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	int vertSize = int(verts.size());
	//----------------------------------------------------------------------------------------------------------------------
	// 1. Add all unique vertex positions
	//----------------------------------------------------------------------------------------------------------------------
	// Add bottom vert (bottom of sphere)
	Vec3 bottomVert = center + Vec3::MakeFromSphericalDegrees(0.0f, 90.0f, radius);
	Vec3 bottomNormal = (bottomVert - center).GetNormalized();
	verts.emplace_back(bottomVert, color, Vec2());
	// Loop and add all unique verts for stacks and slices EXECEPT top vert (top of sphere)
	float yawDegreesPerSlice = (360.0f / numSlices);
	float pitchDegreesPerStack = (180.0f / numStacks);
	for (int currentSlice = 0; currentSlice < numSlices; currentSlice++)
	{
		float currentYawDegrees = static_cast<float>(currentSlice) * yawDegreesPerSlice;
		for (int currentStack = 1; currentStack < numStacks; currentStack++)
		{
			float currentPitchDegrees = 90.0f - (currentStack * pitchDegreesPerStack);
			//----------------------------------------------------------------------------------------------------------------------
			// Verts			
			Vec3 vertsBottomLeft = center + Vec3::MakeFromSphericalDegrees(currentYawDegrees, currentPitchDegrees, radius);		// then add latitudeOffset to TopRight to add verts for BottomRight		// Yaw to the right then Pitch down (positive)
			//----------------------------------------------------------------------------------------------------------------------
			// UVs
			Vec2 UV_BottomLeft;
			UV_BottomLeft.x = RangeMapClamped(currentYawDegrees, 0.0f, 360.0f, UVs.m_mins.x, UVs.m_maxs.x);
			UV_BottomLeft.y = RangeMapClamped(currentPitchDegrees, -90.0f, 90.0f, UVs.m_maxs.y, UVs.m_mins.y);
			// Normal
			Vec3 normal = (vertsBottomLeft - center).GetNormalized();
			//----------------------------------------------------------------------------------------------------------------------
			// Pushback verts at currentYaw, with increasing pitch (increasing stack, same slice)
			verts.emplace_back(vertsBottomLeft, color, UV_BottomLeft);
		}
	}
	// Add top vert (top of sphere)
	Vec3 topVert = center + Vec3::MakeFromSphericalDegrees(0.0f, -90.0f, radius);
	Vec3 topNormal = (topVert - center).GetNormalized();
	verts.emplace_back(topVert, color, Vec2(0.0f, 1.0f));
	//----------------------------------------------------------------------------------------------------------------------
	// 2. Add index List to form a sphere using unique vertex positions
	//----------------------------------------------------------------------------------------------------------------------
	int vertsPerStack = int(numStacks - 1);		// Excluding bottom and top verts, only quads
	//----------------------------------------------------------------------------------------------------------------------
	// 2a. Tri at bottom of sphere
	//----------------------------------------------------------------------------------------------------------------------
	unsigned int BL = vertSize;
	for (int currentSlice = 0; currentSlice < numSlices; currentSlice++)
	{
		if (currentSlice == (numSlices - 1))
		{
			// Special case for the final tri to reconnect with the starting vert
			unsigned int TL = vertSize + (currentSlice * vertsPerStack) + 1;
			unsigned int TR = BL + 1;
			// Add Tri for bottom of sphere
			indexes.emplace_back(BL);
			indexes.emplace_back(TR);
			indexes.emplace_back(TL);
		}
		else
		{
			unsigned int TL = vertSize + (currentSlice * vertsPerStack) + 1;
			unsigned int TR = TL + vertsPerStack;
			// Add Tri for bottom of sphere
			indexes.emplace_back(BL);
			indexes.emplace_back(TR);
			indexes.emplace_back(TL);
		}
	}
	//----------------------------------------------------------------------------------------------------------------------
	// 2b. Tri at top of sphere
	//----------------------------------------------------------------------------------------------------------------------
	unsigned int topVertIndex = int(verts.size() - 1);
	for (int currentSlice = 0; currentSlice < numSlices; currentSlice++)
	{
		if (currentSlice == (numSlices - 1))
		{
			// Special case for the final tri to reconnect with the starting vert
			BL = vertSize + (currentSlice * vertsPerStack) + vertsPerStack;
			unsigned BR = vertSize + vertsPerStack;
			// Add Tri for bottom of sphere
			indexes.emplace_back(topVertIndex);
			indexes.emplace_back(BL);
			indexes.emplace_back(BR);
		}
		else
		{
			BL = vertSize + (currentSlice * vertsPerStack) + vertsPerStack;
			unsigned int BR = BL + vertsPerStack;
			// Add Tri for bottom of sphere
			indexes.emplace_back(topVertIndex);
			indexes.emplace_back(BL);
			indexes.emplace_back(BR);
		}
	}
	//----------------------------------------------------------------------------------------------------------------------
	// 2c. Quads for faces of sphere
	//----------------------------------------------------------------------------------------------------------------------
	for (int currentSlice = 0; currentSlice < numSlices; currentSlice++)
	{
		for (int currentStack = 0; currentStack < (numStacks - 2); currentStack++)
		{
			if (currentSlice == (numSlices - 1))
			{
				// Special case for the final tri to reconnect with the starting vert
				BL = vertSize + (currentSlice * vertsPerStack) + 1 + currentStack;
				unsigned int BR = vertSize + 1 + currentStack;
				unsigned int TL = BL + 1;
				unsigned int TR = BR + 1;
				// Add quads as I "move" up the stack
				indexes.emplace_back(BL);	// BL
				indexes.emplace_back(BR);	// BR
				indexes.emplace_back(TR);	// TR
				indexes.emplace_back(BL);	// BL
				indexes.emplace_back(TR);	// TR
				indexes.emplace_back(TL);	// TL
			}
			else
			{
				BL = vertSize + (currentSlice * vertsPerStack) + 1 + currentStack;
				unsigned int BR = BL + vertsPerStack;
				unsigned int TL = BL + 1;
				unsigned int TR = BR + 1;
				// Add quads as I "move" up the stack
				indexes.emplace_back(BL);	// BL
				indexes.emplace_back(BR);	// BR
				indexes.emplace_back(TR);	// TR
				indexes.emplace_back(BL);	// BL
				indexes.emplace_back(TR);	// TR
				indexes.emplace_back(TL);	// TL
			}
		}
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForUVSphereZ3D(std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, float numSlices, float numStacks, Rgba8 const& tint, AABB2 const& UVs)
{
	float degreesPerSlice = 360.f / numSlices;
	float degreesPerStack = 180.f / numStacks;
	float currentYawDegrees = 0.f;
	float currentPitchDegrees = -90.f;

	Vec3 BL;
	Vec3 BR;
	Vec3 TR;
	Vec3 TL;

	Vec2 uvBL;
	Vec2 uvBR;
	Vec2 uvTR;
	Vec2 uvTL;

	for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	{
		currentPitchDegrees = -90.0f;
		for (int stackNum = 0; stackNum < numStacks; ++stackNum)
		{
			TL = center + Vec3::MakeFromPolarDegrees(currentPitchDegrees, currentYawDegrees, radius);
			TR = center + Vec3::MakeFromPolarDegrees(currentPitchDegrees, currentYawDegrees + (degreesPerSlice), radius);
			BR = center + Vec3::MakeFromPolarDegrees(currentPitchDegrees + (degreesPerStack), currentYawDegrees + (degreesPerSlice), radius);
			BL = center + Vec3::MakeFromPolarDegrees(currentPitchDegrees + (degreesPerStack), currentYawDegrees, radius);

			uvBL.x = RangeMapClamped(currentYawDegrees, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			uvBL.y = RangeMapClamped(currentPitchDegrees + degreesPerStack, -90.f, 90.f, UVs.m_maxs.y, UVs.m_mins.y);

			uvTR.x = RangeMapClamped(currentYawDegrees + degreesPerSlice, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
			uvTR.y = RangeMapClamped(currentPitchDegrees, -90.f, 90.f, UVs.m_maxs.y, UVs.m_mins.y);

			AddVertsForQuad3D(verts, BL, BR, TR, TL, tint, AABB2(uvBL.x, uvBL.y, uvTR.x, uvTR.y));

			currentPitchDegrees += degreesPerStack;
		}
		currentYawDegrees += degreesPerSlice;
	}
}


//--------------------------------------------------------------------------------------------------s
void AddVertsForUVSphereZ3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const Vec3& center, float radius, float numSlices, float numStacks, const Rgba8& tint /*= Rgba8::WHITE*/, AABB2 const& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	int NUM_LAYER = (int)numStacks;
	int NUM_SLICE = (int)numSlices;
	
	int vertsNum = 2 + (NUM_LAYER - 1) * NUM_SLICE;
	int indexNum = 3 * (((NUM_LAYER - 2) * NUM_SLICE * 2) + (2 * NUM_SLICE));
	verts.reserve(verts.size() + vertsNum);
	indexes.reserve(indexes.size() + indexNum);
	int vertStartIndex = (int)verts.size();
	int NUM_VERTS_PER_LAYER = NUM_SLICE + 1;
	float pitchPerStack = 180.f / NUM_LAYER;//Pitch
	float yawPerSlice = 360.f / NUM_SLICE;//Yaw
	for (int layerIndex = 0; layerIndex < 1; layerIndex++)
	{
		for (int sliceIndex = 0; sliceIndex < NUM_SLICE; sliceIndex++)
		{
			float currentPitch = -90.f + layerIndex * pitchPerStack;
			float currentYaw = sliceIndex * yawPerSlice;
			
			Vec3 TR = center + Vec3::MakeFromPolarDegrees(currentYaw + yawPerSlice, currentPitch + pitchPerStack, radius);
			//calculate UV
			float minV = layerIndex * (UVs.GetDimensions().y / (float)NUM_LAYER);
			float maxV = minV + (UVs.GetDimensions().y / (float)NUM_LAYER);
			float minU = 0.f + sliceIndex * (UVs.GetDimensions().x / (float)NUM_SLICE);
			float maxU = minU + (UVs.GetDimensions().x / (float)NUM_SLICE);
			//AABB2 aabb = AABB2(Vec2(minU, minV), Vec2(maxU, maxV));
			//Verts
			if (sliceIndex == 0)
			{
				Vec3 BL = center + Vec3::MakeFromPolarDegrees(currentYaw, currentPitch, radius);
				Vec3 normalBL = Vec3::MakeFromPolarDegrees(currentYaw, currentPitch, radius);
				verts.push_back(Vertex_PCUTBN(BL, tint, Vec2(minU, minV), Vec3::WORLD_ORIGIN, Vec3::WORLD_ORIGIN, normalBL));
				Vec3 TL = center + Vec3::MakeFromPolarDegrees(currentYaw, currentPitch + pitchPerStack, radius);
				Vec3 normalTL = Vec3::MakeFromPolarDegrees(currentYaw, currentPitch + pitchPerStack, radius);
				verts.push_back(Vertex_PCUTBN(TL, tint, Vec2(minU, maxV), Vec3::WORLD_ORIGIN, Vec3::WORLD_ORIGIN, normalTL));
			}
			Vec3 normalTR = Vec3::MakeFromPolarDegrees(currentYaw + yawPerSlice, currentPitch + pitchPerStack, radius);
			verts.push_back(Vertex_PCUTBN(TR, tint, Vec2(maxU, maxV), Vec3::WORLD_ORIGIN, Vec3::WORLD_ORIGIN, normalTR));
		}
		for (int sliceIndex = 0; sliceIndex < NUM_SLICE; sliceIndex++)
		{
			//Calculate Verts No.
			int TRIndex, TLIndex = 0;
			TRIndex = 1 + (sliceIndex + 1);
			TLIndex = 1 + (sliceIndex);
			indexes.push_back(vertStartIndex);
			indexes.push_back(TRIndex);
			indexes.push_back(TLIndex);
		}
	}
	for (int layerIndex = 1; layerIndex < NUM_LAYER - 1; layerIndex++)
	{
		for (int sliceIndex = 0; sliceIndex < NUM_SLICE; sliceIndex++)
		{
			float currentLatitudeDegree = -90.f + layerIndex * pitchPerStack;
			float currentLongtitudeDegree = sliceIndex * yawPerSlice;
			Vec3 BL = center + Vec3::MakeFromPolarDegrees(currentLongtitudeDegree, currentLatitudeDegree, radius);
			Vec3 BR = center + Vec3::MakeFromPolarDegrees(currentLongtitudeDegree + yawPerSlice, currentLatitudeDegree, radius);
			Vec3 TL = center + Vec3::MakeFromPolarDegrees(currentLongtitudeDegree, currentLatitudeDegree + pitchPerStack, radius);
			Vec3 TR = center + Vec3::MakeFromPolarDegrees(currentLongtitudeDegree + yawPerSlice, currentLatitudeDegree + pitchPerStack, radius);
			//calculate UV
			float minV = layerIndex * (UVs.GetDimensions().y / (float)NUM_LAYER);
			float maxV = minV + (UVs.GetDimensions().y / (float)NUM_LAYER);
			float minU = 0.f + sliceIndex * (UVs.GetDimensions().x / (float)NUM_SLICE);
			float maxU = minU + (UVs.GetDimensions().x / (float)NUM_SLICE);
			//TL and TR
			
			if (sliceIndex == 0)
			{
				Vec3 normalTL = Vec3::MakeFromPolarDegrees(currentLongtitudeDegree, currentLatitudeDegree + pitchPerStack, radius);
				verts.push_back(Vertex_PCUTBN(TL, tint, Vec2(minU, maxV), Vec3::WORLD_ORIGIN, Vec3::WORLD_ORIGIN, normalTL));
			}
			Vec3 normalTR = Vec3::MakeFromPolarDegrees(currentLongtitudeDegree + yawPerSlice, currentLatitudeDegree + pitchPerStack, radius);
			verts.push_back(Vertex_PCUTBN(TR, tint, Vec2(maxU, maxV), Vec3::WORLD_ORIGIN, Vec3::WORLD_ORIGIN, normalTR));
		}
		for (int sliceIndex = 0; sliceIndex < NUM_SLICE; sliceIndex++)
		{
			//Calculate Verts No.
			int BLIndex, BRIndex, TRIndex, TLIndex = 0;
			BLIndex = 1 + ((layerIndex - 1) * NUM_VERTS_PER_LAYER) + (sliceIndex);//1+(1-1)*5+(0+1)=2
			TLIndex = 1 + ((layerIndex)*NUM_VERTS_PER_LAYER) + (sliceIndex);//1+(1)*5+(0+1)=8
			BRIndex = 1 + ((layerIndex - 1) * NUM_VERTS_PER_LAYER) + (sliceIndex + 1);//1+(1)*5+(0+1)=8
			TRIndex = 1 + ((layerIndex)*NUM_VERTS_PER_LAYER) + (sliceIndex + 1);//1+(1)*5+(0+1)=8
			indexes.push_back(BLIndex);
			indexes.push_back(BRIndex);
			indexes.push_back(TRIndex);
			indexes.push_back(TRIndex);
			indexes.push_back(TLIndex);
			indexes.push_back(BLIndex);
		}
	}
	for (int layerIndex = NUM_LAYER-1; layerIndex < NUM_LAYER; layerIndex++)
	{
		int EndsliceIndex = 0;
		float currentPitch = -90.f + layerIndex * pitchPerStack;
		float currentYaw = EndsliceIndex * yawPerSlice;
		Vec3 TR = center + Vec3::MakeFromPolarDegrees(currentYaw + yawPerSlice, currentPitch + pitchPerStack, radius);
		//calculate UV
		float minV = layerIndex * (UVs.GetDimensions().y / (float)NUM_LAYER);
		float maxV = minV + (UVs.GetDimensions().y / (float)NUM_LAYER);
		float minU = 0.f + EndsliceIndex * (UVs.GetDimensions().x / (float)NUM_SLICE);
		float maxU = minU + (UVs.GetDimensions().x / (float)NUM_SLICE);
		Vec3 normalTR = Vec3::MakeFromPolarDegrees(currentYaw + yawPerSlice, currentPitch + pitchPerStack, radius);
		verts.push_back(Vertex_PCUTBN(TR, tint, Vec2(maxU, maxV), Vec3::WORLD_ORIGIN, Vec3::WORLD_ORIGIN, normalTR));
		int vertsEndIndex = (int)verts.size() - 1;//21
		for (int sliceIndex = 0; sliceIndex < NUM_SLICE; sliceIndex++)
		{
			//Calculate Verts No.
			int BLIndex, BRIndex = 0;
			BLIndex = 1 + ((layerIndex - 1) * NUM_VERTS_PER_LAYER) + (sliceIndex);//1+(1-1)*5+(0+1)=2
			BRIndex = 1 + ((layerIndex - 1) * NUM_VERTS_PER_LAYER) + (sliceIndex + 1);//1+(1)*5+(0+1)=8
			indexes.push_back(BLIndex);
			indexes.push_back(BRIndex);
			indexes.push_back(vertsEndIndex);
		}
	}
	CalculateTangentSpaceVectors(verts, indexes);
}


//--------------------------------------------------------------------------------------------------
void AddVertsForSphereZ3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec3 const& center, float radius, float numSlices, float numStacks, Rgba8 const& tint)
{
	float degreesPerSlice = 360.f / numSlices;
	float degreesPerStack = 180.f / numStacks;
	float currentYawDegrees = 0.f;
	float currentPitchDegrees = -90.f;
	int startIndex = (int)verts.size();

	Vec3 BL;
	Vec3 BR;
	Vec3 TR;
	Vec3 TL;

	// // // // FIRST PASS: DRAWS THE TOP CONE SEGMENT OF THE SPHERE // // // //
	BL = center + Vec3::MakeFromPolarDegrees(currentPitchDegrees + (degreesPerStack), currentYawDegrees, radius);
	BR = center + Vec3::MakeFromPolarDegrees(currentPitchDegrees + (degreesPerStack), currentYawDegrees + (degreesPerSlice), radius);
	TL = center + Vec3::MakeFromPolarDegrees(currentPitchDegrees, currentYawDegrees, radius);

	verts.push_back(Vertex_PCU(BL, tint, Vec2()));
	verts.push_back(Vertex_PCU(BR, tint, Vec2()));
	verts.push_back(Vertex_PCU(TL, tint, Vec2()));

	indexes.push_back(startIndex);
	indexes.push_back(startIndex + 1);
	indexes.push_back(startIndex + 2);
	int prevMiddle = startIndex + 1;
	int currentIndex = (int)verts.size();

	for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	{
		currentPitchDegrees = -90.f;
		for (int stackNum = 0; stackNum < numStacks; ++stackNum)
		{
			if (stackNum == 0 && sliceNum == 0)
			{
				continue;
			}
			else
			{
				if (stackNum == 0)
				{
					BL = center + Vec3::MakeFromPolarDegrees(currentPitchDegrees + (degreesPerStack), currentYawDegrees + (degreesPerSlice), radius);
					verts.push_back(Vertex_PCU(BL, tint, Vec2()));
					indexes.push_back(prevMiddle);
					indexes.push_back(currentIndex);
					prevMiddle = currentIndex;
					indexes.push_back(startIndex + 2);
					currentIndex += 1;
					TL = BL;

				}
				else
				{

				}
			}
			currentPitchDegrees += degreesPerStack;
		}
		currentYawDegrees += degreesPerSlice;
	}
	// // // // END OF THE FIRST PASS // // // //

	// Vec3 northPole = center + Vec3::MakeFromPolarDegrees(currentPitchDegrees, currentYawDegrees, radius); // Sphere Top, NorthPole
	// verts.push_back(Vertex_PCU(northPole, tint, Vec2()));
	// int startIndex = (int)verts.size();
	// 
	// for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	// {
	// 	currentPitchDegrees = -90.f;
	// 	for (int stackNum = 0; stackNum < numStacks; ++stackNum)
	// 	{
	// 		if (currentPitchDegrees = -90.f)
	// 		{
	// 			indexes.push_back(startIndex);
	// 		}
	// 	}
	// }

}


//--------------------------------------------------------------------------------------------------
void AddVertsForUVSphereZWireframe3D(std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, int numSlices, int numStacks, float lineThickness, Rgba8 const& color)
{
	float degreesPerSlice = 360.f / numSlices;
	float degreesPerStack = 180.f / numStacks;
	float currentYawDegrees = 0.f;
	float currentPitchDegrees = -90.f;

	Vec3 BL;
	Vec3 BR;
	Vec3 TR;
	Vec3 TL;

	for (int longitudeSlice = 0; longitudeSlice < numSlices; ++longitudeSlice)
	{
		currentPitchDegrees = -90.0f;
		for (int latitudeSlice = 0; latitudeSlice < numStacks; ++latitudeSlice)
		{
			TL = center + Vec3::MakeFromPolarDegrees(currentPitchDegrees, currentYawDegrees, radius);
			TR = center + Vec3::MakeFromPolarDegrees(currentPitchDegrees, currentYawDegrees + (degreesPerSlice), radius);
			BR = center + Vec3::MakeFromPolarDegrees(currentPitchDegrees + (degreesPerStack), currentYawDegrees + (degreesPerSlice), radius);
			BL = center + Vec3::MakeFromPolarDegrees(currentPitchDegrees + (degreesPerStack), currentYawDegrees, radius);

			AddVertsForLineSegment3D(verts, TL, TR, lineThickness, color);
			AddVertsForLineSegment3D(verts, TL, BL, lineThickness, color);

			currentPitchDegrees += degreesPerStack;
		}
		currentYawDegrees += degreesPerSlice;
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForUnitCylinderX3D(std::vector<Vertex_PCU>& verts, float numSlices, Rgba8 const& tint, AABB2 const& UVs)
{
	Vec3 sectorMinTip = Vec3(0.f, 0.f, 0.f);
	Vec3 sectorMaxTip = Vec3(1.f, 0.f, 0.f);

	Vec2 sectorTipUV = Vec2(0.5f, 0.5f);
	float currentYawDegrees = 0.f;
	float degreesPerYawSlice = 360.f / numSlices;

	for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	{
		Vec3 currentSectorVert1 = MakeFromPolarDegrees(currentYawDegrees, 1.f);
		currentSectorVert1.z = currentSectorVert1.x;
		currentSectorVert1.x = 0.f;
		Vec3 currentSectorVert2 = MakeFromPolarDegrees(currentYawDegrees + degreesPerYawSlice, 1.f);
		currentSectorVert2.z = currentSectorVert2.x;
		currentSectorVert2.x = 0.f;

		float currentSectorVert1U = RangeMap(currentSectorVert1.z, -1.f, 1.f, UVs.m_mins.x, UVs.m_maxs.x);
		float currentSectorVert1V = RangeMap(currentSectorVert1.y, -1.f, 1.f, UVs.m_mins.y, UVs.m_maxs.y);
		Vec2 currentSectorVert1UV = Vec2(currentSectorVert1U, currentSectorVert1V);

		float currentSectorVert2U = RangeMap(currentSectorVert2.z, -1.f, 1.f, UVs.m_mins.x, UVs.m_maxs.x);
		float currentSectorVert2V = RangeMap(currentSectorVert2.y, -1.f, 1.f, UVs.m_mins.y, UVs.m_maxs.y);
		Vec2 currentSectorVert2UV = Vec2(currentSectorVert2U, currentSectorVert2V);

		Vec3 currentSectorMinVert1 = sectorMinTip + currentSectorVert1;
		Vec3 currentSectorMinVert2 = sectorMinTip + currentSectorVert2;

		verts.push_back(Vertex_PCU(currentSectorMinVert1, tint, Vec2(currentSectorVert2U, 1.f - currentSectorVert2V)));
		verts.push_back(Vertex_PCU(currentSectorMinVert2, tint, Vec2(currentSectorVert2U, 1.f - currentSectorVert2V)));
		verts.push_back(Vertex_PCU(sectorMinTip, tint, sectorTipUV));

		Vec3 currentSectorMaxVert1 = sectorMaxTip + currentSectorVert1;
		Vec3 currentSectorMaxVert2 = sectorMaxTip + currentSectorVert2;

		verts.push_back(Vertex_PCU(currentSectorMaxVert2, tint, currentSectorVert1UV));
		verts.push_back(Vertex_PCU(currentSectorMaxVert1, tint, currentSectorVert2UV));
		verts.push_back(Vertex_PCU(sectorMaxTip, tint, sectorTipUV));

		Vec2 currentCylinderUVBL;
		currentCylinderUVBL.x = RangeMapClamped(currentYawDegrees, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
		currentCylinderUVBL.y = UVs.m_mins.y;

		Vec2 currentCylinderUVTR;
		currentCylinderUVTR.x = RangeMapClamped(currentYawDegrees + degreesPerYawSlice, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
		currentCylinderUVTR.y = UVs.m_maxs.y;

		AddVertsForQuad3D(verts, currentSectorMinVert2, currentSectorMinVert1, currentSectorMaxVert1, currentSectorMaxVert2, tint, AABB2(currentCylinderUVBL, currentCylinderUVTR));
		currentYawDegrees += degreesPerYawSlice;
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForCylinderZ3D(std::vector<Vertex_PCU>& verts, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, float numSlices, Rgba8 const& tint, AABB2 const& UVs)
{
	Vec3 sectorMinTip = Vec3(centerXY, minMaxZ.m_min);
	Vec3 sectorMaxTip = Vec3(centerXY, minMaxZ.m_max);
	Vec2 sectorTipUV = Vec2(0.5f, 0.5f);
	float currentYawDegrees = 0.f;
	float degreesPerYawSlice = 360.f / numSlices;

	for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	{
		Vec3 currentSectorVert1 = MakeFromPolarDegrees(currentYawDegrees, radius);
		float currentSectorVert1U = RangeMap(currentSectorVert1.x, -radius, radius, UVs.m_mins.x, UVs.m_maxs.x);
		float currentSectorVert1V = RangeMap(currentSectorVert1.y, -radius, radius, UVs.m_mins.y, UVs.m_maxs.y);
		Vec2 currentSectorVert1UV = Vec2(currentSectorVert1U, currentSectorVert1V);
		Vec3 currentSectorVert2 = MakeFromPolarDegrees(currentYawDegrees + degreesPerYawSlice, radius);
		float currentSectorVert2U = RangeMap(currentSectorVert2.x, -radius, radius, UVs.m_mins.x, UVs.m_maxs.x);
		float currentSectorVert2V = RangeMap(currentSectorVert2.y, -radius, radius, UVs.m_mins.y, UVs.m_maxs.y);
		Vec2 currentSectorVert2UV = Vec2(currentSectorVert2U, currentSectorVert2V);

		Vec3 currentSectorMinVert1 = sectorMinTip + currentSectorVert1;
		Vec3 currentSectorMinVert2 = sectorMinTip + currentSectorVert2;

		verts.push_back(Vertex_PCU(currentSectorMinVert2, tint, Vec2(currentSectorVert2UV.x, 1.f - currentSectorVert2UV.y)));
		verts.push_back(Vertex_PCU(currentSectorMinVert1, tint, Vec2(currentSectorVert1UV.x, 1.f - currentSectorVert1UV.y)));
		verts.push_back(Vertex_PCU(sectorMinTip, tint, sectorTipUV));

		Vec3 currentSectorMaxVert1 = sectorMaxTip + currentSectorVert1;
		Vec3 currentSectorMaxVert2 = sectorMaxTip + currentSectorVert2;

		verts.push_back(Vertex_PCU(currentSectorMaxVert1, tint, currentSectorVert1UV));
		verts.push_back(Vertex_PCU(currentSectorMaxVert2, tint, currentSectorVert2UV));
		verts.push_back(Vertex_PCU(sectorMaxTip, tint, sectorTipUV));

		Vec2 currentCylinderUVBL;
		currentCylinderUVBL.x = RangeMapClamped(currentYawDegrees, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
		currentCylinderUVBL.y = UVs.m_mins.y;

		Vec2 currentCylinderUVTR;
		currentCylinderUVTR.x = RangeMapClamped(currentYawDegrees + degreesPerYawSlice, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
		currentCylinderUVTR.y = UVs.m_maxs.y;

		AddVertsForQuad3D(verts, currentSectorMinVert1, currentSectorMinVert2, currentSectorMaxVert2, currentSectorMaxVert1, tint, AABB2(currentCylinderUVBL, currentCylinderUVTR));

		currentYawDegrees += degreesPerYawSlice;
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForCylinderZ3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, float numSlices, Rgba8 const& tint, AABB2 const& UVs)
{
	// // // MISC // // //
	Vec3 sectorMinTip = Vec3(centerXY, minMaxZ.m_min);
	Vec3 sectorMaxTip = Vec3(centerXY, minMaxZ.m_max);
	Vec2 sectorTipUV = Vec2(0.5f, 0.5f);
	float currentYawDegrees = 0.f;
	float degreesPerYawSlice = 360.f / numSlices;

	Vec3 prevSectorVert1 = MakeFromPolarDegrees(currentYawDegrees, radius);
	float prevSectorVert1U = RangeMap(prevSectorVert1.x, -radius, radius, UVs.m_mins.x, UVs.m_maxs.x);
	float prevSectorVert1V = RangeMap(prevSectorVert1.y, -radius, radius, UVs.m_mins.y, UVs.m_maxs.y);
	Vec2 prevSectorVert1UV = Vec2(prevSectorVert1U, prevSectorVert1V);

	Vec3 prevSectorMinVert1 = sectorMinTip + prevSectorVert1;
	Vec3 prevSectorMaxVert1 = sectorMaxTip + prevSectorVert1;


	// // // CYLINDER BOTTOM DISC // // //
	int sectorMinTipIndex = (int)verts.size();
	verts.push_back(Vertex_PCU(sectorMinTip, tint, sectorTipUV));
	verts.push_back(Vertex_PCU(prevSectorMinVert1, tint, Vec2(prevSectorVert1UV.x, 1.f - prevSectorVert1UV.y)));

	// // // CYLINDER BODY // // // 
	Vec2 prevCylinderUVBL;
	prevCylinderUVBL.x = UVs.m_mins.x;
	prevCylinderUVBL.y = UVs.m_mins.y;

	verts.push_back(Vertex_PCU(prevSectorMinVert1, tint, prevCylinderUVBL));

	Vec2 prevCylinderUVTL;
	prevCylinderUVTL.x = UVs.m_mins.x;
	prevCylinderUVTL.y = UVs.m_maxs.y;

	verts.push_back(Vertex_PCU(prevSectorMaxVert1, tint, prevCylinderUVTL));

	// // // CYLINDER TOP DISC // // //
	verts.push_back(Vertex_PCU(prevSectorMaxVert1, tint, prevSectorVert1UV));
	int sectorMaxTipIndex = (int)verts.size();
	verts.push_back(Vertex_PCU(sectorMaxTip, tint, sectorTipUV));
	// // //           // // // 

	currentYawDegrees += degreesPerYawSlice;
	int currentIndex = (int)verts.size();
	for (int sliceIndex = 0; sliceIndex < numSlices; ++sliceIndex)
	{
		Vec3 currentSectorVert1 = MakeFromPolarDegrees(currentYawDegrees, radius);
		float currentSectorVert1U = RangeMap(currentSectorVert1.x, -radius, radius, UVs.m_mins.x, UVs.m_maxs.x);
		float currentSectorVert1V = RangeMap(currentSectorVert1.y, -radius, radius, UVs.m_mins.y, UVs.m_maxs.y);
		Vec2 currentSectorVert1UV = Vec2(currentSectorVert1U, currentSectorVert1V);

		Vec3 currentSectorMinVert1 = sectorMinTip + currentSectorVert1;
		Vec3 currentSectorMaxVert1 = sectorMaxTip + currentSectorVert1;

		// // // CYLINDER BOTTOM DISC // // //
		verts.push_back(Vertex_PCU(currentSectorMinVert1, tint, Vec2(currentSectorVert1UV.x, 1.f - currentSectorVert1UV.y)));
		indexes.push_back(sectorMinTipIndex);
		indexes.push_back(currentIndex);
		if (sliceIndex != 0)
		{
			indexes.push_back(currentIndex - 4);
		}
		else
		{
			indexes.push_back(currentIndex - 5);
		}
		currentIndex += 1;

		// // // CYLINDER BODY // // //
		Vec2 currentCylinderUVBL;
		currentCylinderUVBL.x = RangeMapClamped(currentYawDegrees, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
		currentCylinderUVBL.y = UVs.m_mins.y;

		verts.push_back(Vertex_PCU(currentSectorMinVert1, tint, currentCylinderUVBL));

		Vec2 currentCylinderUVTL;
		currentCylinderUVTL.x = currentCylinderUVBL.x;
		currentCylinderUVTL.y = UVs.m_maxs.y;

		verts.push_back(Vertex_PCU(currentSectorMaxVert1, tint, currentCylinderUVTL));
		if (sliceIndex != 0)
		{
			indexes.push_back(currentIndex - 4);
		}
		else
		{
			indexes.push_back(currentIndex - 5);
		}
		indexes.push_back(currentIndex);
		indexes.push_back(currentIndex + 1);

		if (sliceIndex != 0)
		{
			indexes.push_back(currentIndex - 4);
		}
		else
		{
			indexes.push_back(currentIndex - 5);
		}
		indexes.push_back(currentIndex + 1);
		if (sliceIndex != 0)
		{
			indexes.push_back(currentIndex - 3);
		}
		else
		{
			indexes.push_back(currentIndex - 4);
		}
		currentIndex += 2;

		// // // CYLINDER TOP DISC // // //
		verts.push_back(Vertex_PCU(currentSectorMaxVert1, tint, currentSectorVert1UV));
		indexes.push_back(sectorMaxTipIndex);
		if (sliceIndex != 0)
		{
			indexes.push_back(currentIndex - 4);
		}
		else
		{
			indexes.push_back(currentIndex - 5);
		}
		indexes.push_back(currentIndex);
		currentIndex += 1;
		// // //           // // // 

		currentYawDegrees += degreesPerYawSlice;
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForCylinderZWireframe3D(std::vector<Vertex_PCU>& verts, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, float numSlices, float lineThickness, Rgba8 const& tint)
{
	Vec3 sectorMinTip = Vec3(centerXY, minMaxZ.m_min);
	Vec3 sectorMaxTip = Vec3(centerXY, minMaxZ.m_max);
	float currentYawDegrees = 0.f;
	float degreesPerYawSlice = 360.f / numSlices;

	for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	{
		Vec3 currentSectorVert1 = MakeFromPolarDegrees(currentYawDegrees, radius);
		Vec3 currentSectorVert2 = MakeFromPolarDegrees(currentYawDegrees + degreesPerYawSlice, radius);

		Vec3 currentSectorMinVert1 = sectorMinTip + currentSectorVert1;
		Vec3 currentSectorMinVert2 = sectorMinTip + currentSectorVert2;

		AddVertsForLineSegment3D(verts, currentSectorMinVert1, currentSectorMinVert2, lineThickness, tint);

		Vec3 currentSectorMaxVert1 = sectorMaxTip + currentSectorVert1;
		Vec3 currentSectorMaxVert2 = sectorMaxTip + currentSectorVert2;

		AddVertsForLineSegment3D(verts, currentSectorMaxVert1, currentSectorMaxVert2, lineThickness, tint);


		AddVertsForLineSegment3D(verts, currentSectorMaxVert1, currentSectorMinVert1, lineThickness, tint);
		currentYawDegrees += degreesPerYawSlice;
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForCylinder3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& tint, int numSlices, AABB2 const& UVs)
{
	Vec3 dispSE = (end - start);
	Vec3 iForward = dispSE.GetNormalized();

	Vec3 jLeft = CrossProduct3D(Vec3::Z_AXIS, iForward);
	if (jLeft == Vec3::WORLD_ORIGIN)
	{
		jLeft = Vec3::Y_AXIS;
	}
	else
	{
		jLeft.Normalize();
	}
	Vec3 kUp = CrossProduct3D(iForward, jLeft);

	Mat44 transformationMatrix;
	float depth = dispSE.GetLength();
	transformationMatrix.SetIJK3D(iForward * depth, jLeft * radius, kUp * radius);
	transformationMatrix.SetTranslation3D(start);

	std::vector<Vertex_PCU> tempVerts;
	AddVertsForUnitCylinderX3D(tempVerts, (float)numSlices, tint, UVs);
	TransformVertexArray3D(tempVerts, transformationMatrix);

	// how does this differ from for looping?
	verts.insert(verts.end(), tempVerts.begin(), tempVerts.end());

	// transformationMatrix.SetTranslation3D(start);
	// AddVertsForCylinderZ3D(verts, Vec2(0.f, 0.f), FloatRange(0.f, height.GetLength()), radius, (float)numSlices, tint, UVs);
	// TransformVertexArray3D(verts, transformationMatrix);
	// 
	// 
	// // transformationMatrix.SetTranslation3D(start);
	// Mat44 minTransformedMat = transformationMatrix;
	// 
	// 
	// // std::vector<Vertex_PCU> verts;
	// AddVertsForCylinderZ3D(verts, Vec2(start.x, start.y), FloatRange(start.z, start.z + 20.f), radius, (float)numSlices, tint, UVs);
	// for (int vertIndex = 0; vertIndex < verts.size(); ++vertIndex)
	// {
	// 	verts[vertIndex].m_position = minTransformedMat.TransformPosition3D(verts[vertIndex].m_position);
	// }

	// transformationMatrix.SetTranslation3D(end);
	// Mat44 maxTransformedMat = transformationMatrix;
	// 
	// Vec2 sectorTipUV = Vec2(0.5f, 0.5f);
	// float currentYawDegrees = 0.f;
	// float degreesPerYawSlice = 360.f / numSlices;
	// 
	// for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	// {
	// 	Vec3 currentSectorVert1 = MakeFromPolarDegrees(currentYawDegrees, radius);
	// 	currentSectorVert1.z = currentSectorVert1.x;
	// 	currentSectorVert1.x = 0.f;
	// 	float currentSectorVert1U = RangeMap(currentSectorVert1.x, -radius, radius, UVs.m_mins.x, UVs.m_maxs.x);
	// 	float currentSectorVert1V = RangeMap(currentSectorVert1.y, -radius, radius, UVs.m_mins.y, UVs.m_maxs.y);
	// 	Vec2 currentSectorVert1UV = Vec2(currentSectorVert1U, currentSectorVert1V);
	// 	Vec3 currentSectorVert2 = MakeFromPolarDegrees(currentYawDegrees + degreesPerYawSlice, radius);
	// 	currentSectorVert2.z = currentSectorVert2.x;
	// 	currentSectorVert2.x = 0.f;
	// 	float currentSectorVert2U = RangeMap(currentSectorVert2.z, -radius, radius, UVs.m_mins.x, UVs.m_maxs.x);
	// 	float currentSectorVert2V = RangeMap(currentSectorVert2.y, -radius, radius, UVs.m_mins.y, UVs.m_maxs.y);
	// 	Vec2 currentSectorVert2UV = Vec2(currentSectorVert2U, currentSectorVert2V);
	// 	
	// 	Vec3 currentSectorMinVert1 = transformationMatrix.TransformPosition3D(currentSectorVert1);
	// 	Vec3 currentSectorMinVert2 = transformationMatrix.TransformPosition3D(currentSectorVert2);
	// 
	// 	verts.push_back(Vertex_PCU(currentSectorMinVert2, tint, Vec2(currentSectorVert2UV.x, 1.f - currentSectorVert2UV.y)));
	// 	verts.push_back(Vertex_PCU(currentSectorMinVert1, tint, Vec2(currentSectorVert1UV.x, 1.f - currentSectorVert1UV.y)));
	// 	verts.push_back(Vertex_PCU(start, tint, sectorTipUV));
	// 
	// 	Vec3 currentSectorMaxVert1 = translationMatrix.TransformPosition3D(currentSectorVert1);
	// 	Vec3 currentSectorMaxVert2 = translationMatrix.TransformPosition3D(currentSectorVert2);
	// 
	// 	verts.push_back(Vertex_PCU(currentSectorMaxVert1, tint, currentSectorVert1UV));
	// 	verts.push_back(Vertex_PCU(currentSectorMaxVert2, tint, currentSectorVert2UV));
	// 	verts.push_back(Vertex_PCU(end, tint, sectorTipUV));
	// 
	// 	Vec2 currentCylinderUVBL;
	// 	currentCylinderUVBL.x = RangeMapClamped(currentYawDegrees, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
	// 	currentCylinderUVBL.y = UVs.m_mins.y;
	// 	// 
	// 	Vec2 currentCylinderUVTR;
	// 	currentCylinderUVTR.x = RangeMapClamped(currentYawDegrees + degreesPerYawSlice, 0.f, 360.f, UVs.m_mins.x, UVs.m_maxs.x);
	// 	currentCylinderUVTR.y = UVs.m_maxs.y;
	// 
	// 	AddVertsForQuad3D(verts, currentSectorMinVert1, currentSectorMinVert2, currentSectorMaxVert2, currentSectorMaxVert1, tint, AABB2(currentCylinderUVBL, currentCylinderUVTR));
	// 
	// 	currentYawDegrees += degreesPerYawSlice;
	// }
}


//--------------------------------------------------------------------------------------------------
void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color, AABB2 const& UVs)
{
	Vec3 BL = Vec3(bounds.m_mins.x, bounds.m_mins.y, 0);
	Vec3 BR = Vec3(bounds.m_maxs.x, bounds.m_mins.y, 0);
	Vec3 TR = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, 0);
	Vec3 TL = Vec3(bounds.m_mins.x, bounds.m_maxs.y, 0);

	Vec2 uvBL = Vec2(UVs.m_mins.x, UVs.m_mins.y);
	Vec2 uvBR = Vec2(UVs.m_maxs.x, UVs.m_mins.y);
	Vec2 uvTR = Vec2(UVs.m_maxs.x, UVs.m_maxs.y);
	Vec2 uvTL = Vec2(UVs.m_mins.x, UVs.m_maxs.y);

	verts.push_back(Vertex_PCU(BL, color, uvBL));
	verts.push_back(Vertex_PCU(BR, color, uvBR));
	verts.push_back(Vertex_PCU(TR, color, uvTR));

	verts.push_back(Vertex_PCU(BL, color, uvBL));
	verts.push_back(Vertex_PCU(TR, color, uvTR));
	verts.push_back(Vertex_PCU(TL, color, uvTL));
}


//--------------------------------------------------------------------------------------------------
void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, AABB2 const& bounds, Rgba8 const& color, AABB2 const& UVs)
{
	Vec3 BL = Vec3(bounds.m_mins.x, bounds.m_mins.y, 0);
	Vec3 BR = Vec3(bounds.m_maxs.x, bounds.m_mins.y, 0);
	Vec3 TR = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, 0);
	Vec3 TL = Vec3(bounds.m_mins.x, bounds.m_maxs.y, 0);

	Vec2 uvBL = Vec2(UVs.m_mins.x, UVs.m_mins.y);
	Vec2 uvBR = Vec2(UVs.m_maxs.x, UVs.m_mins.y);
	Vec2 uvTR = Vec2(UVs.m_maxs.x, UVs.m_maxs.y);
	Vec2 uvTL = Vec2(UVs.m_mins.x, UVs.m_maxs.y);

	int startIndex = (int)verts.size();
	verts.emplace_back(BL, color, uvBL);
	verts.emplace_back(BR, color, uvBR);
	verts.emplace_back(TR, color, uvTR);
	verts.emplace_back(TL, color, uvTL);

	indexes.emplace_back(startIndex);
	indexes.emplace_back(startIndex + 1);
	indexes.emplace_back(startIndex + 2);
	indexes.emplace_back(startIndex);
	indexes.emplace_back(startIndex + 2);
	indexes.emplace_back(startIndex + 3);
}


//--------------------------------------------------------------------------------------------------
void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts, AABB3 const& bounds, Rgba8 const& color, AABB2 const& UVs)
{
	Vec3 ESB = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 ENB = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 ENT = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z);
	Vec3 EST = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z);

	Vec3 WNB = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 WSB = Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 WST = Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z);
	Vec3 WNT = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z);

	AddVertsForQuad3D(verts, ESB, ENB, ENT, EST, color, UVs); // EAST FACE +X
	AddVertsForQuad3D(verts, WNB, WSB, WST, WNT, color, UVs); // WEST FACE -X
	AddVertsForQuad3D(verts, ENB, WNB, WNT, ENT, color, UVs); // NORTH FACE +Y
	AddVertsForQuad3D(verts, WSB, ESB, EST, WST, color, UVs); // SOUTH FACE -Y
	AddVertsForQuad3D(verts, WST, EST, ENT, WNT, color, UVs); // TOP FACE +Z
	AddVertsForQuad3D(verts, WNB, ENB, ESB, WSB, color, UVs); // BOTTOM FACE -Z
}


//--------------------------------------------------------------------------------------------------
void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, AABB3 const& bounds, Rgba8 const& color /*= Rgba8::WHITE*/, AABB2 const& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	Vec3 ESB = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 ENB = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 ENT = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z);
	Vec3 EST = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z);

	Vec3 WNB = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 WSB = Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 WST = Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z);
	Vec3 WNT = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z);

	AddVertsForQuad3D(verts, indexes, ESB, ENB, ENT, EST, color, UVs); // EAST    FACE  +X
	AddVertsForQuad3D(verts, indexes, WNB, WSB, WST, WNT, color, UVs); // WEST    FACE  -X
	AddVertsForQuad3D(verts, indexes, ENB, WNB, WNT, ENT, color, UVs); // NORTH   FACE  +Y
	AddVertsForQuad3D(verts, indexes, WSB, ESB, EST, WST, color, UVs); // SOUTH   FACE  -Y
	AddVertsForQuad3D(verts, indexes, WST, EST, ENT, WNT, color, UVs); // TOP     FACE  +Z
	AddVertsForQuad3D(verts, indexes, WNB, ENB, ESB, WSB, color, UVs); // BOTTOM  FACE  -Z
}


//--------------------------------------------------------------------------------------------------
void AddVertsForAABB3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, AABB3 const& bounds, Rgba8 const& color /*= Rgba8::WHITE*/, AABB2 const& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	Vec3 ESB = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 ENB = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 ENT = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z);
	Vec3 EST = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z);

	Vec3 WNB = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 WSB = Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 WST = Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z);
	Vec3 WNT = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z);

	AddVertsForQuad3D(verts, indexes, ESB, ENB, ENT, EST, color, UVs); // EAST    FACE  +X
	AddVertsForQuad3D(verts, indexes, WNB, WSB, WST, WNT, color, UVs); // WEST    FACE  -X
	AddVertsForQuad3D(verts, indexes, ENB, WNB, WNT, ENT, color, UVs); // NORTH   FACE  +Y
	AddVertsForQuad3D(verts, indexes, WSB, ESB, EST, WST, color, UVs); // SOUTH   FACE  -Y
	AddVertsForQuad3D(verts, indexes, WST, EST, ENT, WNT, color, UVs); // TOP     FACE  +Z
	AddVertsForQuad3D(verts, indexes, WNB, ENB, ESB, WSB, color, UVs); // BOTTOM  FACE  -Z

	CalculateTangentSpaceVectors(verts, indexes);
}


//--------------------------------------------------------------------------------------------------
void AddVertsForAABBWireframe3D(std::vector<Vertex_PCU>& verts, AABB3 const& bounds, float lineThickness, Rgba8 const& color)
{
	Vec3 ESB = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 ENB = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 ENT = Vec3(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z);
	Vec3 EST = Vec3(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z);

	Vec3 WNB = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z);
	Vec3 WSB = Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z);
	Vec3 WST = Vec3(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z);
	Vec3 WNT = Vec3(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z);

	AddVertsForLineSegment3D(verts, ESB, ENB, lineThickness, color);
	AddVertsForLineSegment3D(verts, ESB, EST, lineThickness, color);
	AddVertsForLineSegment3D(verts, ENB, ENT, lineThickness, color);
	AddVertsForLineSegment3D(verts, EST, ENT, lineThickness, color);

	AddVertsForLineSegment3D(verts, WSB, WNB, lineThickness, color);
	AddVertsForLineSegment3D(verts, WSB, WST, lineThickness, color);
	AddVertsForLineSegment3D(verts, WNB, WNT, lineThickness, color);
	AddVertsForLineSegment3D(verts, WST, WNT, lineThickness, color);

	AddVertsForLineSegment3D(verts, ESB, WSB, lineThickness, color);
	AddVertsForLineSegment3D(verts, ENB, WNB, lineThickness, color);
	AddVertsForLineSegment3D(verts, ENT, WNT, lineThickness, color);
	AddVertsForLineSegment3D(verts, EST, WST, lineThickness, color);
}


//--------------------------------------------------------------------------------------------------
void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& UVs)
{
	Vec2 uvBL = Vec2(UVs.m_mins.x, UVs.m_mins.y);
	Vec2 uvBR = Vec2(UVs.m_maxs.x, UVs.m_mins.y);
	Vec2 uvTR = Vec2(UVs.m_maxs.x, UVs.m_maxs.y);
	Vec2 uvTL = Vec2(UVs.m_mins.x, UVs.m_maxs.y);

	verts.push_back(Vertex_PCU(bottomLeft, color, uvBL));
	verts.push_back(Vertex_PCU(bottomRight, color, uvBR));
	verts.push_back(Vertex_PCU(topRight, color, uvTR));

	verts.push_back(Vertex_PCU(bottomLeft, color, uvBL));
	verts.push_back(Vertex_PCU(topRight, color, uvTR));
	verts.push_back(Vertex_PCU(topLeft, color, uvTL));
}


//--------------------------------------------------------------------------------------------------
void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& UVs)
{
	int startIndex = (int)verts.size();
	Vec2 uvBL = Vec2(UVs.m_mins.x, UVs.m_mins.y);
	Vec2 uvBR = Vec2(UVs.m_maxs.x, UVs.m_mins.y);
	Vec2 uvTR = Vec2(UVs.m_maxs.x, UVs.m_maxs.y);
	Vec2 uvTL = Vec2(UVs.m_mins.x, UVs.m_maxs.y);

	verts.emplace_back(bottomLeft,	color, uvBL);
	verts.emplace_back(bottomRight, color, uvBR);
	verts.emplace_back(topRight,	color, uvTR);
	verts.emplace_back(topLeft,		color, uvTL);

	indexes.emplace_back(startIndex);
	indexes.emplace_back(startIndex + 1);
	indexes.emplace_back(startIndex + 2);

	indexes.emplace_back(startIndex);
	indexes.emplace_back(startIndex + 2);
	indexes.emplace_back(startIndex + 3);
}


//--------------------------------------------------------------------------------------------------
void AddVertsForQuad3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& UVs)
{
	int startIndex	=	(int)verts.size();
	Vec2 uvBL		=	Vec2(UVs.m_mins.x, UVs.m_mins.y);
	Vec2 uvBR		=	Vec2(UVs.m_maxs.x, UVs.m_mins.y);
	Vec2 uvTR		=	Vec2(UVs.m_maxs.x, UVs.m_maxs.y);
	Vec2 uvTL		=	Vec2(UVs.m_mins.x, UVs.m_maxs.y);

	Vec3 dispBLToBR		=	bottomRight - bottomLeft;
	Vec3 dispBRToTR		=	topRight - bottomRight;

	Vec3 normal = CrossProduct3D(dispBLToBR, dispBRToTR);
	normal.Normalize();

	Vec3 tangent	= Vec3::ZERO;
	Vec3 binormal	= Vec3::ZERO;

	verts.emplace_back(bottomLeft,	color, uvBL, tangent, binormal, normal);
	verts.emplace_back(bottomRight, color, uvBR, tangent, binormal, normal);
	verts.emplace_back(topRight,	color, uvTR, tangent, binormal, normal);
	verts.emplace_back(topLeft,		color, uvTL, tangent, binormal, normal);

	indexes.emplace_back(startIndex);
	indexes.emplace_back(startIndex + 1);
	indexes.emplace_back(startIndex + 2);

	indexes.emplace_back(startIndex);
	indexes.emplace_back(startIndex + 2);
	indexes.emplace_back(startIndex + 3);

	CalculateTangentSpaceVectors(verts, indexes);
}


//--------------------------------------------------------------------------------------------------
void AddVertsForRoundedQuad3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color, AABB2 const& UVs)
{
	int startIndex = (int)verts.size();
	Vec2 uvBL = Vec2(UVs.m_mins.x, UVs.m_mins.y);
	Vec2 uvBR = Vec2(UVs.m_maxs.x, UVs.m_mins.y);
	Vec2 uvBMP = Vec2(((UVs.m_maxs.x + UVs.m_mins.x) * 0.5f), UVs.m_mins.y);
	Vec2 uvTR = Vec2(UVs.m_maxs.x, UVs.m_maxs.y);
	Vec2 uvTL = Vec2(UVs.m_mins.x, UVs.m_maxs.y);
	Vec2 uvTMP = Vec2(((UVs.m_maxs.x + UVs.m_mins.x) * 0.5f), UVs.m_maxs.y);

	Vec3 dispBLToBR = bottomRight - bottomLeft;
	Vec3 dispBLToTL = topLeft - bottomLeft;
	Vec3 dirBLBR = dispBLToBR;
	Vec3 dirBLTL = dispBLToTL;
	Vec3 normal = CrossProduct3D(dirBLBR, dirBLTL);
	normal.Normalize();

	Vec3 bottomMidPoint = (bottomRight + bottomLeft) * 0.5f;
	Vec3 topMidPoint = (topRight + topLeft) * 0.5f;

	Vec3 dispBMPToBR = bottomRight - bottomMidPoint;
	Vec3 tangentRight = dispBMPToBR.GetNormalized();
	Vec3 dispBMPToBL = bottomLeft - bottomMidPoint;
	Vec3 tangentLeft = dispBMPToBL.GetNormalized();

	Vec3 tangent;
	Vec3 binormal;

	verts.push_back(Vertex_PCUTBN(bottomLeft,		color, uvBL,  tangent, binormal, tangentLeft));
	verts.push_back(Vertex_PCUTBN(bottomMidPoint,	color, uvBMP, tangent, binormal, normal));
	verts.push_back(Vertex_PCUTBN(bottomRight,		color, uvBR,  tangent, binormal, tangentRight));
	verts.push_back(Vertex_PCUTBN(topRight,			color, uvTR,  tangent, binormal, tangentRight));
	verts.push_back(Vertex_PCUTBN(topMidPoint,		color, uvTMP, tangent, binormal, normal));
	verts.push_back(Vertex_PCUTBN(topLeft,			color, uvTL,  tangent, binormal, tangentLeft));

	indexes.push_back(startIndex);
	indexes.push_back(startIndex + 4);
	indexes.push_back(startIndex + 5);

	indexes.push_back(startIndex);
	indexes.push_back(startIndex + 1);
	indexes.push_back(startIndex + 4);

	indexes.push_back(startIndex + 1);
	indexes.push_back(startIndex + 3);
	indexes.push_back(startIndex + 4);

	indexes.push_back(startIndex + 1);
	indexes.push_back(startIndex + 2);
	indexes.push_back(startIndex + 3);

	CalculateTangentSpaceVectors(verts, indexes);
}


//--------------------------------------------------------------------------------------------------
void AddVertsForOBB2D(std::vector<Vertex_PCU>& verts, OBB2 const& box, Rgba8 const& color)
{
	Vec2 jBasisNormal = box.m_iBasisNormal.GetRotated90Degrees();
	Vec2 scalediBasis = box.m_iBasisNormal * box.m_halfDimensions.x;
	Vec2 scaledjBasis = jBasisNormal * box.m_halfDimensions.y;

	Vec2 BR = box.m_center + scalediBasis - scaledjBasis; // BottomRight
	Vec2 TR = box.m_center + scalediBasis + scaledjBasis;
	Vec2 TL = box.m_center - scalediBasis + scaledjBasis;
	Vec2 BL = box.m_center - scalediBasis - scaledjBasis; // BottomLeft

	Vec2 uvBL = Vec2(0.f, 0.f);
	Vec2 uvBR = Vec2(1.f, 0.f);
	Vec2 uvTR = Vec2(1.f, 1.f);
	Vec2 uvTL = Vec2(0.f, 1.f);

	verts.push_back(Vertex_PCU(Vec3(BR.x, BR.y), color, uvBR));
	verts.push_back(Vertex_PCU(Vec3(TR.x, TR.y), color, uvTR));
	verts.push_back(Vertex_PCU(Vec3(TL.x, TL.y), color, uvTL));

	verts.push_back(Vertex_PCU(Vec3(BR.x, BR.y), color, uvBR));
	verts.push_back(Vertex_PCU(Vec3(TL.x, TL.y), color, uvTL));
	verts.push_back(Vertex_PCU(Vec3(BL.x, BL.y), color, uvBL));
}


//--------------------------------------------------------------------------------------------------
void AddVertsForOBB3D(std::vector<Vertex_PCU>& verts, OBB3 const& box, Rgba8 const& color, AABB2 const& UVs)
{
	Vec3 iForward	=	box.m_iBasis;
	Vec3 jLeft		=	box.m_jBasis;
	Vec3 kUp		=	box.m_kBasis;

	Vec3  const& boxCenter		=	box.m_center;
	float const& boxLength		=	box.m_halfDims.x;
	float const& boxWidth		=	box.m_halfDims.y;
	float const& boxHeight		=	box.m_halfDims.z;

	Vec3 scalediBasis	=  iForward  *	boxLength;
	Vec3 scaledjBasis	=  jLeft	 *	boxWidth;
	Vec3 scaledkBasis	=  kUp		 *	boxHeight;

	Vec3 backBottomLeft		=	boxCenter - scaledkBasis + scaledjBasis - scalediBasis;
	Vec3 backBottomRight	=	boxCenter - scaledkBasis - scaledjBasis - scalediBasis;
	Vec3 frontBottomLeft	=	boxCenter - scaledkBasis + scaledjBasis + scalediBasis;
	Vec3 frontBottomRight	=	boxCenter - scaledkBasis - scaledjBasis + scalediBasis;

	Vec3 backTopLeft	=	boxCenter + scaledkBasis + scaledjBasis - scalediBasis;
	Vec3 backTopRight	=	boxCenter + scaledkBasis - scaledjBasis - scalediBasis;
	Vec3 frontTopLeft	=	boxCenter + scaledkBasis + scaledjBasis + scalediBasis;
	Vec3 frontTopRight	=	boxCenter + scaledkBasis - scaledjBasis + scalediBasis;

	AddVertsForQuad3D(verts,	backBottomRight,	frontBottomRight,	frontTopRight,		backTopRight,		color,	UVs);	// Right Face
	AddVertsForQuad3D(verts,	frontBottomLeft,	backBottomLeft,		backTopLeft,		frontTopLeft,		color,	UVs);	// Left Face
	AddVertsForQuad3D(verts,	frontBottomRight,	frontBottomLeft,	frontTopLeft,		frontTopRight,		color,	UVs);	// Front Face
	AddVertsForQuad3D(verts,	backBottomLeft,		backBottomRight,	backTopRight,		backTopLeft,		color,	UVs);	// Back Face
	AddVertsForQuad3D(verts,	backTopLeft,		backTopRight,		frontTopRight,		frontTopLeft,		color,	UVs);	// Top Face
	AddVertsForQuad3D(verts,	backBottomRight,	backBottomLeft,		frontBottomLeft,	frontBottomRight,	color,	UVs);	// Bottom Face
}


//--------------------------------------------------------------------------------------------------
void AddVertsForOBB3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, OBB3 const& box, Rgba8 const& color, AABB2 const& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	Vec3 iForward	=	box.m_iBasis;
	Vec3 jLeft		=	box.m_jBasis;
	Vec3 kUp		=	box.m_kBasis;

	Vec3  const& boxCenter		=	box.m_center;
	float const& boxLength		=	box.m_halfDims.x;
	float const& boxWidth		=	box.m_halfDims.y;
	float const& boxHeight		=	box.m_halfDims.z;

	Vec3 scalediBasis	=  iForward  *	boxLength;
	Vec3 scaledjBasis	=  jLeft	 *	boxWidth;
	Vec3 scaledkBasis	=  kUp		 *	boxHeight;

	Vec3 backBottomLeft		=	boxCenter - scaledkBasis + scaledjBasis - scalediBasis;
	Vec3 backBottomRight	=	boxCenter - scaledkBasis - scaledjBasis - scalediBasis;
	Vec3 frontBottomLeft	=	boxCenter - scaledkBasis + scaledjBasis + scalediBasis;
	Vec3 frontBottomRight	=	boxCenter - scaledkBasis - scaledjBasis + scalediBasis;

	Vec3 backTopLeft	=	boxCenter + scaledkBasis + scaledjBasis - scalediBasis;
	Vec3 backTopRight	=	boxCenter + scaledkBasis - scaledjBasis - scalediBasis;
	Vec3 frontTopLeft	=	boxCenter + scaledkBasis + scaledjBasis + scalediBasis;
	Vec3 frontTopRight	=	boxCenter + scaledkBasis - scaledjBasis + scalediBasis;

	AddVertsForQuad3D(verts, indexes, backBottomRight,	frontBottomRight, frontTopRight,	backTopRight,	  color, UVs);	// Right Face
	AddVertsForQuad3D(verts, indexes, frontBottomLeft,	backBottomLeft,	  backTopLeft,		frontTopLeft,	  color, UVs);	// Left Face
	AddVertsForQuad3D(verts, indexes, frontBottomRight,	frontBottomLeft,  frontTopLeft,		frontTopRight,	  color, UVs);	// Front Face
	AddVertsForQuad3D(verts, indexes, backBottomLeft,	backBottomRight,  backTopRight,		backTopLeft,	  color, UVs);	// Back Face
	AddVertsForQuad3D(verts, indexes, backTopLeft,		backTopRight,	  frontTopRight,	frontTopLeft,	  color, UVs);	// Top Face
	AddVertsForQuad3D(verts, indexes, backBottomRight,	backBottomLeft,	  frontBottomLeft,	frontBottomRight, color, UVs);	// Bottom Face

	CalculateTangentSpaceVectors(verts, indexes);
}


//--------------------------------------------------------------------------------------------------
void AddVertsForOBB3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, OBB3 const& box, Rgba8 color /*= Rgba8::WHITE*/, AABB2 const& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	Vec3 iForward	=	box.m_iBasis;
	Vec3 jLeft		=	box.m_jBasis;
	Vec3 kUp		=	box.m_kBasis;

	Vec3  const& boxCenter		=	box.m_center;
	float const& boxLength		=	box.m_halfDims.x;
	float const& boxWidth		=	box.m_halfDims.y;
	float const& boxHeight		=	box.m_halfDims.z;

	Vec3 scalediBasis	=  iForward  *	boxLength;
	Vec3 scaledjBasis	=  jLeft	 *	boxWidth;
	Vec3 scaledkBasis	=  kUp		 *	boxHeight;

	Vec3 backBottomLeft		=	boxCenter - scaledkBasis + scaledjBasis - scalediBasis;
	Vec3 backBottomRight	=	boxCenter - scaledkBasis - scaledjBasis - scalediBasis;
	Vec3 frontBottomLeft	=	boxCenter - scaledkBasis + scaledjBasis + scalediBasis;
	Vec3 frontBottomRight	=	boxCenter - scaledkBasis - scaledjBasis + scalediBasis;

	Vec3 backTopLeft	=	boxCenter + scaledkBasis + scaledjBasis - scalediBasis;
	Vec3 backTopRight	=	boxCenter + scaledkBasis - scaledjBasis - scalediBasis;
	Vec3 frontTopLeft	=	boxCenter + scaledkBasis + scaledjBasis + scalediBasis;
	Vec3 frontTopRight	=	boxCenter + scaledkBasis - scaledjBasis + scalediBasis;

	AddVertsForQuad3D(verts, indexes, backBottomRight,	frontBottomRight, frontTopRight,	backTopRight,	  color, UVs);	// Right Face
	AddVertsForQuad3D(verts, indexes, frontBottomLeft,	backBottomLeft,	  backTopLeft,		frontTopLeft,	  color, UVs);	// Left Face
	AddVertsForQuad3D(verts, indexes, frontBottomRight,	frontBottomLeft,  frontTopLeft,		frontTopRight,	  color, UVs);	// Front Face
	AddVertsForQuad3D(verts, indexes, backBottomLeft,	backBottomRight,  backTopRight,		backTopLeft,	  color, UVs);	// Back Face
	AddVertsForQuad3D(verts, indexes, backTopLeft,		backTopRight,	  frontTopRight,	frontTopLeft,	  color, UVs);	// Top Face
	AddVertsForQuad3D(verts, indexes, backBottomRight,	backBottomLeft,	  frontBottomLeft,	frontBottomRight, color, UVs);	// Bottom Face
}


//--------------------------------------------------------------------------------------------------
void AddVertsForOBBWireframe3d(std::vector<Vertex_PCU>& verts, OBB3 const& box, float lineThickness, Rgba8 const& color /*= Rgba8::WHITE*/)
{
	Vec3 iForward	=	box.m_iBasis;
	Vec3 jLeft		=	box.m_jBasis;
	Vec3 kUp		=	box.m_kBasis;

	Vec3  const& boxCenter		=	box.m_center;
	float const& boxLength		=	box.m_halfDims.x;
	float const& boxWidth		=	box.m_halfDims.y;
	float const& boxHeight		=	box.m_halfDims.z;

	Vec3 scalediBasis	=  iForward  *	boxLength;
	Vec3 scaledjBasis	=  jLeft	 *	boxWidth;
	Vec3 scaledkBasis	=  kUp		 *	boxHeight;

	Vec3 backBottomLeft		=	boxCenter - scaledkBasis + scaledjBasis - scalediBasis;
	Vec3 backBottomRight	=	boxCenter - scaledkBasis - scaledjBasis - scalediBasis;
	Vec3 frontBottomLeft	=	boxCenter - scaledkBasis + scaledjBasis + scalediBasis;
	Vec3 frontBottomRight	=	boxCenter - scaledkBasis - scaledjBasis + scalediBasis;

	Vec3 backTopLeft	=	boxCenter + scaledkBasis + scaledjBasis - scalediBasis;
	Vec3 backTopRight	=	boxCenter + scaledkBasis - scaledjBasis - scalediBasis;
	Vec3 frontTopLeft	=	boxCenter + scaledkBasis + scaledjBasis + scalediBasis;
	Vec3 frontTopRight	=	boxCenter + scaledkBasis - scaledjBasis + scalediBasis;

	AddVertsForLineSegment3D(verts, backBottomRight,	frontBottomRight,	lineThickness, color);
	AddVertsForLineSegment3D(verts, backBottomRight,	backTopRight,		lineThickness, color);
	AddVertsForLineSegment3D(verts, frontBottomRight,	frontTopRight,		lineThickness, color);
	AddVertsForLineSegment3D(verts, backTopRight,		frontTopRight,		lineThickness, color);

	AddVertsForLineSegment3D(verts, backBottomLeft,		frontBottomLeft,	lineThickness, color);
	AddVertsForLineSegment3D(verts, backBottomLeft,		backTopLeft,		lineThickness, color);
	AddVertsForLineSegment3D(verts, frontBottomLeft,	frontTopLeft,		lineThickness, color);
	AddVertsForLineSegment3D(verts, backTopLeft,		frontTopLeft,		lineThickness, color);

	AddVertsForLineSegment3D(verts, backBottomRight,	backBottomLeft,		lineThickness, color);
	AddVertsForLineSegment3D(verts, frontBottomRight,	frontBottomLeft,	lineThickness, color);
	AddVertsForLineSegment3D(verts, frontTopRight,		frontTopLeft,		lineThickness, color);
	AddVertsForLineSegment3D(verts, backTopRight,		backTopLeft,	lineThickness, color);
}


//--------------------------------------------------------------------------------------------------
void AddVertsForLineSegment2D(std::vector<Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color)
{
	Vec2 dispFromStartToEnd = end - start;
	Vec2 normalizedDispSE = dispFromStartToEnd.GetNormalized();
	Vec2 perpendicular = normalizedDispSE.GetRotated90Degrees();
	float halfThickness = thickness * 0.5f;
	Vec2 scaledDisp = normalizedDispSE * halfThickness;
	Vec2 scaledPerpendicular = perpendicular * halfThickness;

	Vec2 BR = start - scaledPerpendicular - scaledDisp;
	Vec2 TR = end - scaledPerpendicular + scaledDisp;
	Vec2 TL = end + scaledPerpendicular + scaledDisp;
	Vec2 BL = start + scaledPerpendicular - scaledDisp;

	verts.emplace_back(Vec3(BR.x, BR.y), color);
	verts.emplace_back(Vec3(TR.x, TR.y), color);
	verts.emplace_back(Vec3(TL.x, TL.y), color);

	verts.emplace_back(Vec3(BR.x, BR.y), color);
	verts.emplace_back(Vec3(TL.x, TL.y), color);
	verts.emplace_back(Vec3(BL.x, BL.y), color);
}


//--------------------------------------------------------------------------------------------------
void AddVertsForLineSegment2D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color)
{
	Vec2 dispFromStartToEnd		=	end - start;
	Vec2 normalizedDispSE		=	dispFromStartToEnd.GetNormalized();
	Vec2 perpendicular			=	normalizedDispSE.GetRotated90Degrees();
	float halfThickness			=	thickness * 0.5f;
	Vec2 scaledDisp				=	normalizedDispSE * halfThickness;
	Vec2 scaledPerpendicular	=	perpendicular * halfThickness;

	Vec2 BR = start - scaledPerpendicular - scaledDisp;
	Vec2 TR = end	- scaledPerpendicular + scaledDisp;
	Vec2 TL = end	+ scaledPerpendicular + scaledDisp;
	Vec2 BL = start + scaledPerpendicular - scaledDisp;

	unsigned int index = (unsigned int)verts.size();

	verts.emplace_back(Vec3(BR.x, BR.y), color);
	verts.emplace_back(Vec3(TR.x, TR.y), color);
	verts.emplace_back(Vec3(TL.x, TL.y), color);
	verts.emplace_back(Vec3(BL.x, BL.y), color);

	indexes.emplace_back(index);
	indexes.emplace_back(index + 1);
	indexes.emplace_back(index + 2);

	indexes.emplace_back(index);
	indexes.emplace_back(index + 2);
	indexes.emplace_back(index + 3);
}


//--------------------------------------------------------------------------------------------------
void AddVertsForLineSegment3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float thickness, Rgba8 const& color)
{
	Vec3 iForward = (end - start).GetNormalized();
	Vec3 jLeft = CrossProduct3D(Vec3::Z_AXIS, iForward);
	if (jLeft == Vec3::WORLD_ORIGIN)
	{
		jLeft = Vec3::Y_AXIS;
	}
	jLeft.Normalize();
	Vec3 kUp = CrossProduct3D(iForward, jLeft).GetNormalized();

	float halfThickness = thickness * 0.5f;

	Vec3 scaledForward = iForward * halfThickness;
	Vec3 scaledPerpendicularY = jLeft * halfThickness;
	Vec3 scaledPerpendicularZ = kUp * halfThickness;

	Vec3 ESB = start - scaledForward - scaledPerpendicularY - scaledPerpendicularZ;
	Vec3 ENB = end + scaledForward - scaledPerpendicularY - scaledPerpendicularZ;
	Vec3 ENT = end + scaledForward - scaledPerpendicularY + scaledPerpendicularZ;
	Vec3 EST = start - scaledForward - scaledPerpendicularY + scaledPerpendicularZ;

	Vec3 WNB = end + scaledForward + scaledPerpendicularY - scaledPerpendicularZ;
	Vec3 WSB = start - scaledForward + scaledPerpendicularY - scaledPerpendicularZ;
	Vec3 WST = start - scaledForward + scaledPerpendicularY + scaledPerpendicularZ;
	Vec3 WNT = end + scaledForward + scaledPerpendicularY + scaledPerpendicularZ;

	AddVertsForQuad3D(verts, ESB, ENB, ENT, EST, color); // EAST FACE +X
	AddVertsForQuad3D(verts, WNB, WSB, WST, WNT, color); // WEST FACE -X
	AddVertsForQuad3D(verts, ENB, WNB, WNT, ENT, color); // NORTH FACE +Y
	AddVertsForQuad3D(verts, WSB, ESB, EST, WST, color); // SOUTH FACE -Y
	AddVertsForQuad3D(verts, WST, EST, ENT, WNT, color); // TOP FACE +Z
	AddVertsForQuad3D(verts, WNB, ENB, ESB, WSB, color); // BOTTOM FACE -Z
}


//--------------------------------------------------------------------------------------------------
void AddVertsForArrow2D(std::vector<Vertex_PCU>& verts, Vec2 const& tailPos, Vec2 const& tipPos, float arrowSize, float lineThickness, Rgba8 const& color)
{
	Vec2 dispFromTailToTip = tipPos - tailPos;
	Vec2 directionFromTailToTip = dispFromTailToTip.GetNormalized();
	Vec2 dirFromTip = directionFromTailToTip.GetRotated90Degrees();
	Vec2 arrowBasePos = tipPos - directionFromTailToTip;
	float halfArrowSize = arrowSize * 0.5f;
	Vec2 scaledDirFromTip = dirFromTip * halfArrowSize;
	Vec2 arrowPosRight = arrowBasePos + scaledDirFromTip;
	Vec2 arrowPosLeft = arrowBasePos - scaledDirFromTip;

	AddVertsForLineSegment2D(verts, tailPos, tipPos, lineThickness, color);
	AddVertsForLineSegment2D(verts, tipPos, arrowPosRight, lineThickness, color);
	AddVertsForLineSegment2D(verts, tipPos, arrowPosLeft, lineThickness, color);
}


//--------------------------------------------------------------------------------------------------
void TransformVertexArray3D(std::vector<Vertex_PCU>& verts, Mat44 const& transform)
{
	TransformVertexArray3D((int)verts.size(), verts.data(), transform);
}


//--------------------------------------------------------------------------------------------------
void TransformVertexArray3D(int numVerts, Vertex_PCUTBN* verts, Mat44 const& transform)
{
	for (int vertIndex = 0; vertIndex < numVerts; ++vertIndex)
	{
		Vec3& pos		= verts[vertIndex].m_position;
		Vec3& normal	= verts[vertIndex].m_normal;
		pos				= transform.TransformPosition3D(pos);
		normal			= transform.TransformVectorQuantity3D(normal);
		normal.Normalize();
	}
}


//--------------------------------------------------------------------------------------------------
void TransformVertexArray3D(std::vector<Vertex_PCUTBN>& verts, Mat44 const& transform)
{
	TransformVertexArray3D((int)verts.size(), verts.data(), transform);
}


//--------------------------------------------------------------------------------------------------
// For CCW ordered verts
void AddVertsForConvexPoly2D(std::vector<Vertex_PCU>& verts, ConvexPoly2D const& poly2D, Rgba8 const& color /*= Rgba8::WHITE*/)
{
	std::vector<Vec2> const& pointList	=	poly2D.GetPoints2D();
	size_t numOfTris					=	(unsigned int)(pointList.size() - 2);
	Vec2 const& vert0Pos				=	pointList[0];
	size_t vertIndex					=	1;
	for (size_t triIndex = 1; triIndex <= numOfTris; ++triIndex)
	{
		Vec2 const& vert1Pos = pointList[vertIndex];
		Vec2 const& vert2Pos = pointList[vertIndex + 1];
		verts.emplace_back(Vec3(vert0Pos), color);
		verts.emplace_back(Vec3(vert1Pos), color);
		verts.emplace_back(Vec3(vert2Pos), color);
		vertIndex += 1;
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForConvexPoly2D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, ConvexPoly2D const& poly2D, Rgba8 const& color /*= Rgba8::WHITE*/)
{
	std::vector<Vec2> const& pointList	=	poly2D.GetPoints2D();
	size_t numOfPoints					=	pointList.size();
	size_t numOfTris					=	(unsigned int)(numOfPoints - 2);
	unsigned int const& index0			=	(unsigned int)verts.size();
	unsigned int index					=	index0;
	verts.emplace_back(Vec3(pointList[0]), color);
	verts.emplace_back(Vec3(pointList[1]), color);
	verts.emplace_back(Vec3(pointList[2]), color);
	indexes.emplace_back(index0);
	indexes.emplace_back(index + 1);
	indexes.emplace_back(index + 2);
	index += 2;
	for (size_t triIndex = 2; triIndex <= numOfTris; ++triIndex)
	{
		size_t vertIndex = (size_t)index + 1 - (size_t)index0;
		verts.emplace_back(Vec3(pointList[vertIndex]), color);
		indexes.emplace_back(index0);
		indexes.emplace_back(index);
		indexes.emplace_back(index + 1);
		index += 1;
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForBorderedConvexPoly2D(std::vector<Vertex_PCU>& verts, ConvexPoly2D const& poly2D, float thickness, Rgba8 const& color /*= Rgba8::WHITE*/)
{
	std::vector<Vec2> const& pointList	=	poly2D.GetPoints2D();
	size_t const& numOfPoints			=	pointList.size();
	for (size_t pointIndex = 0; pointIndex < numOfPoints; ++pointIndex)
	{
		Vec2 const& currentPoint = pointList[pointIndex];
		size_t nextPointIndex = pointIndex + 1;
		if (nextPointIndex >= numOfPoints)
		{
			nextPointIndex = 0;
		}
		Vec2 const& nextPoint = pointList[nextPointIndex];
		AddVertsForLineSegment2D(verts, currentPoint, nextPoint, thickness, color);
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForBorderedConvexPoly2D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, ConvexPoly2D const& poly2D, float thickness, Rgba8 const& color /*= Rgba8::WHITE*/)
{
	std::vector<Vec2> const& pointList = poly2D.GetPoints2D();
	size_t const& numOfPoints = pointList.size();
	for (size_t pointIndex = 0; pointIndex < numOfPoints; ++pointIndex)
	{
		Vec2 const& currentPoint = pointList[pointIndex];
		size_t nextPointIndex = pointIndex + 1;
		if (nextPointIndex >= numOfPoints)
		{
			nextPointIndex = 0;
		}
		Vec2 const& nextPoint = pointList[nextPointIndex];
		AddVertsForLineSegment2D(verts, indexes, currentPoint, nextPoint, thickness, color);
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForPlane2D(std::vector<Vertex_PCU>& verts, Plane2D const& plane, float thickness, Vec2 const& camDims /*= Vec2(-1.f, -1.f)*/, Rgba8 const& color /*= Rgba8::WHITE*/)
{
	Vec2  const& planeNormal			=	plane.m_normal;
	float const& planeDistFromOrigin	=	plane.m_distFromOrigin;
	Vec2 nearestPointOnPlane			=	planeNormal * planeDistFromOrigin;
	float scalingFactor					=	camDims.x + camDims.y;
	Vec2 dirAlongThePlane				=	planeNormal.GetRotated90Degrees();
	Vec2 ofscreenVector					=	dirAlongThePlane * scalingFactor;
	Vec2 planeOffscreenStartPoint		=	nearestPointOnPlane - ofscreenVector;
	Vec2 planeOffscreenEndPoint			=	nearestPointOnPlane + ofscreenVector;
	AddVertsForLineSegment2D(verts, planeOffscreenStartPoint, planeOffscreenEndPoint, thickness, color);
}


//--------------------------------------------------------------------------------------------------
void AddVertsForPlane2D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Plane2D const& plane, float thickness, Vec2 const& camDims, Rgba8 const& color)
{
	Vec2  const& planeNormal			=	plane.m_normal;
	float const& planeDistFromOrigin	=	plane.m_distFromOrigin;
	Vec2 nearestPointOnPlane			=	planeNormal * planeDistFromOrigin;
	float scalingFactor					=	camDims.x + camDims.y;
	Vec2 dirAlongThePlane				=	planeNormal.GetRotated90Degrees();
	Vec2 ofscreenVector					=	dirAlongThePlane * scalingFactor;
	Vec2 planeOffscreenStartPoint		=	nearestPointOnPlane - ofscreenVector;
	Vec2 planeOffscreenEndPoint			=	nearestPointOnPlane + ofscreenVector;
	AddVertsForLineSegment2D(verts, indexes, planeOffscreenStartPoint, planeOffscreenEndPoint, thickness, color);
}


//--------------------------------------------------------------------------------------------------
void AddVertsForConvexHull2D(std::vector<Vertex_PCU>& verts, ConvexHull2D const& convexHull2D, float thickness, Vec2 const& camDims, Rgba8 const& color /*= Rgba8::WHITE*/)
{
	std::vector<Plane2D> const& planeList	=	convexHull2D.m_boundingPlanes;
	size_t numOfPlanes						=	planeList.size();
	for (size_t planeIndex = 0; planeIndex < numOfPlanes; ++planeIndex)
	{
		Plane2D const& currentPlane = planeList[planeIndex];
		AddVertsForPlane2D(verts, currentPlane, thickness, camDims, color);
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForConvexHull2D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, ConvexHull2D const& convexHull2D, float thickness, Vec2 const& camDims, Rgba8 const& color)
{
	std::vector<Plane2D> const& planeList	=	convexHull2D.m_boundingPlanes;
	size_t numOfPlanes						=	planeList.size();
	for (size_t planeIndex = 0; planeIndex < numOfPlanes; ++planeIndex)
	{
		Plane2D const& currentPlane = planeList[planeIndex];
		AddVertsForPlane2D(verts, indexes, currentPlane, thickness, camDims, color);
	}
}


//--------------------------------------------------------------------------------------------------
AABB2 GetVertexBounds2D(std::vector<Vertex_PCU> const& verts)
{
	Vec2 firstPos;
	firstPos.x = verts[0].m_position.x;
	firstPos.y = verts[0].m_position.y;
	AABB2 vertexBounds = AABB2(firstPos, firstPos);

	for (int vertIndex = 1; vertIndex < verts.size(); ++vertIndex)
	{
		Vec3 const& currentVertPos = verts[vertIndex].m_position;
		if (vertexBounds.m_mins.x > currentVertPos.x)
		{
			vertexBounds.m_mins.x = currentVertPos.x;
		}
		if (vertexBounds.m_mins.y > currentVertPos.y)
		{
			vertexBounds.m_mins.y = currentVertPos.y;
		}
		if (vertexBounds.m_maxs.x < currentVertPos.x)
		{
			vertexBounds.m_maxs.x = currentVertPos.x;
		}
		if (vertexBounds.m_maxs.y < currentVertPos.y)
		{
			vertexBounds.m_maxs.y = currentVertPos.y;
		}
	}

	return vertexBounds;
}


//--------------------------------------------------------------------------------------------------
void AddVertsForCone3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& tint, int numSlices, AABB2 const& UVs)
{
	Vec3 dispSE = (end - start);
	Vec3 iForward = dispSE.GetNormalized();

	Vec3 jLeft = CrossProduct3D(Vec3::Z_AXIS, iForward);
	if (jLeft == Vec3::WORLD_ORIGIN)
	{
		jLeft = Vec3::Y_AXIS;
	}
	else
	{
		jLeft.Normalize();
	}
	Vec3 kUp = CrossProduct3D(iForward, jLeft);

	Mat44 transformationMatrix;
	transformationMatrix.SetIJK3D(iForward, jLeft, kUp);
	transformationMatrix.SetTranslation3D(start);

	Vec3 sectorMinTip = Vec3(0.f, 0.f, 0.f);
	sectorMinTip = transformationMatrix.TransformPosition3D(sectorMinTip);
	float depth = dispSE.GetLength();
	Vec3 sectorMaxTip = Vec3(depth, 0.f, 0.f);
	sectorMaxTip = transformationMatrix.TransformPosition3D(sectorMaxTip);
	Vec2 sectorTipUV = Vec2(0.5f, 0.5f);

	float currentYawDegrees = 0.f;
	float degreesPerYawSlice = 360.f / numSlices;

	for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	{
		Vec3 currentSectorVert1 = MakeFromPolarDegrees(currentYawDegrees, radius);
		currentSectorVert1.z = currentSectorVert1.x;
		currentSectorVert1.x = 0.f;
		Vec3 currentSectorVert2 = MakeFromPolarDegrees(currentYawDegrees + degreesPerYawSlice, radius);
		currentSectorVert2.z = currentSectorVert2.x;
		currentSectorVert2.x = 0.f;

		float currentSectorVert1U = RangeMap(currentSectorVert1.z, -1.f, 1.f, UVs.m_mins.x, UVs.m_maxs.x);
		float currentSectorVert1V = RangeMap(currentSectorVert1.y, -1.f, 1.f, UVs.m_mins.y, UVs.m_maxs.y);
		Vec2 currentSectorVert1UV = Vec2(currentSectorVert1U, currentSectorVert1V);

		float currentSectorVert2U = RangeMap(currentSectorVert2.z, -1.f, 1.f, UVs.m_mins.x, UVs.m_maxs.x);
		float currentSectorVert2V = RangeMap(currentSectorVert2.y, -1.f, 1.f, UVs.m_mins.y, UVs.m_maxs.y);
		Vec2 currentSectorVert2UV = Vec2(currentSectorVert2U, currentSectorVert2V);

		Vec3 currentSectorMinVert1 = currentSectorVert1;
		Vec3 currentSectorMinVert2 = currentSectorVert2;

		currentSectorMinVert1 = transformationMatrix.TransformPosition3D(currentSectorMinVert1);
		currentSectorMinVert2 = transformationMatrix.TransformPosition3D(currentSectorMinVert2);

		verts.push_back(Vertex_PCU(currentSectorMinVert1, tint, Vec2(currentSectorVert2U, 1.f - currentSectorVert2V)));
		verts.push_back(Vertex_PCU(currentSectorMinVert2, tint, Vec2(currentSectorVert2U, 1.f - currentSectorVert2V)));
		verts.push_back(Vertex_PCU(sectorMinTip, tint, sectorTipUV));

		verts.push_back(Vertex_PCU(sectorMaxTip, tint, sectorTipUV));
		verts.push_back(Vertex_PCU(currentSectorMinVert2, tint, currentSectorVert1UV));
		verts.push_back(Vertex_PCU(currentSectorMinVert1, tint, currentSectorVert2UV));

		currentYawDegrees += degreesPerYawSlice;
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForCone3D(std::vector<Vertex_PCUTBN>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& tint /*= Rgba8::WHITE*/, int numSlices /*= 8*/, AABB2 const& UVs /*= AABB2::ZERO_TO_ONE*/)
{
	Vec3 dispSE = (end - start);
	Vec3 iForward = dispSE.GetNormalized();

	Vec3 jLeft = CrossProduct3D(Vec3::Z_AXIS, iForward);
	if (jLeft == Vec3::WORLD_ORIGIN)
	{
		jLeft = Vec3::Y_AXIS;
	}
	else
	{
		jLeft.Normalize();
	}
	Vec3 kUp = CrossProduct3D(iForward, jLeft);

	Mat44 transformationMatrix;
	transformationMatrix.SetIJK3D(iForward, jLeft, kUp);
	transformationMatrix.SetTranslation3D(start);

	Vec3 sectorMinTip = Vec3(0.f, 0.f, 0.f);
	sectorMinTip = transformationMatrix.TransformPosition3D(sectorMinTip);
	float depth = dispSE.GetLength();
	Vec3 sectorMaxTip = Vec3(depth, 0.f, 0.f);
	sectorMaxTip = transformationMatrix.TransformPosition3D(sectorMaxTip);
	Vec2 sectorTipUV = Vec2(0.5f, 0.5f);

	float currentYawDegrees = 0.f;
	float degreesPerYawSlice = 360.f / numSlices;

	for (int sliceNum = 0; sliceNum < numSlices; ++sliceNum)
	{
		Vec3 currentSectorVert1 = MakeFromPolarDegrees(currentYawDegrees, radius);
		currentSectorVert1.z = currentSectorVert1.x;
		currentSectorVert1.x = 0.f;
		Vec3 currentSectorVert2 = MakeFromPolarDegrees(currentYawDegrees + degreesPerYawSlice, radius);
		currentSectorVert2.z = currentSectorVert2.x;
		currentSectorVert2.x = 0.f;

		float currentSectorVert1U = RangeMap(currentSectorVert1.z, -1.f, 1.f, UVs.m_mins.x, UVs.m_maxs.x);
		float currentSectorVert1V = RangeMap(currentSectorVert1.y, -1.f, 1.f, UVs.m_mins.y, UVs.m_maxs.y);
		Vec2 currentSectorVert1UV = Vec2(currentSectorVert1U, currentSectorVert1V);

		float currentSectorVert2U = RangeMap(currentSectorVert2.z, -1.f, 1.f, UVs.m_mins.x, UVs.m_maxs.x);
		float currentSectorVert2V = RangeMap(currentSectorVert2.y, -1.f, 1.f, UVs.m_mins.y, UVs.m_maxs.y);
		Vec2 currentSectorVert2UV = Vec2(currentSectorVert2U, currentSectorVert2V);

		Vec3 currentSectorMinVert1 = currentSectorVert1;
		Vec3 currentSectorMinVert2 = currentSectorVert2;

		currentSectorMinVert1 = transformationMatrix.TransformPosition3D(currentSectorMinVert1);
		currentSectorMinVert2 = transformationMatrix.TransformPosition3D(currentSectorMinVert2);

		
		Vec3 dispMinVert1ToMinVert2 = currentSectorMinVert2 - currentSectorMinVert1;
		Vec3 dispMinVert2ToMinTip	= sectorMinTip - currentSectorMinVert2;
		Vec3 normal1						= CrossProduct3D(dispMinVert1ToMinVert2, dispMinVert2ToMinTip);
		normal1.Normalize();

		Vec3 dispMaxTipToMinVert2	= currentSectorMinVert2 - sectorMaxTip;
		Vec3 dispMinVert2ToMinVert1 = currentSectorMinVert1 - currentSectorMinVert2;
		Vec3 normal2 = CrossProduct3D(dispMaxTipToMinVert2, dispMinVert2ToMinVert1);
		normal2.Normalize();

		Vec3 tangent	=	Vec3::ZERO;
		Vec3 binormal	=	Vec3::ZERO;

		verts.emplace_back(currentSectorMinVert1, tint, Vec2(currentSectorVert2U, 1.f - currentSectorVert2V), tangent, binormal, normal1);
		verts.emplace_back(currentSectorMinVert2, tint, Vec2(currentSectorVert2U, 1.f - currentSectorVert2V), tangent, binormal, normal1);
		verts.emplace_back(sectorMinTip, tint, sectorTipUV, tangent, binormal, normal1);

		verts.emplace_back(sectorMaxTip, tint, sectorTipUV, tangent, binormal, normal2);
		verts.emplace_back(currentSectorMinVert2, tint, currentSectorVert1UV, tangent, binormal, normal2);
		verts.emplace_back(currentSectorMinVert1, tint, currentSectorVert2UV, tangent, binormal, normal2);

		// verts.push_back(Vertex_PCU(currentSectorMinVert1, tint, Vec2(currentSectorVert2U, 1.f - currentSectorVert2V)));
		// verts.push_back(Vertex_PCU(currentSectorMinVert2, tint, Vec2(currentSectorVert2U, 1.f - currentSectorVert2V)));
		// verts.push_back(Vertex_PCU(sectorMinTip, tint, sectorTipUV));
		
		// verts.push_back(Vertex_PCU(sectorMaxTip, tint, sectorTipUV));
		// verts.push_back(Vertex_PCU(currentSectorMinVert2, tint, currentSectorVert1UV));
		// verts.push_back(Vertex_PCU(currentSectorMinVert1, tint, currentSectorVert2UV));

		currentYawDegrees += degreesPerYawSlice;
	}
}


//--------------------------------------------------------------------------------------------------
void AddVertsForLineList(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, Rgba8 const& color /*= Rgba8::WHITE*/)
{
	verts.emplace_back(start,	color);
	verts.emplace_back(end,		color);
}
