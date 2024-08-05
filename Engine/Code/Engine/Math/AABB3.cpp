#include "Engine/Math/AABB3.hpp"


//--------------------------------------------------------------------------------------------------
AABB3 const AABB3::ZERO_TO_ONE(0.f, 0.f, 0.f, 1.f, 1.f, 1.f);
AABB3 const AABB3::INVALID(-1.f, -1.f, -1.f, -1.f, -1.f, -1.f);


//--------------------------------------------------------------------------------------------------
AABB3::AABB3(AABB3 const& copyFrom) :
	m_mins(copyFrom.m_mins), 
	m_maxs(copyFrom.m_maxs)
{
}


//--------------------------------------------------------------------------------------------------
AABB3::AABB3(float minX, float minY, float minZ, float maxX, float maxY, float maxZ) :
	m_mins(Vec3(minX, minY, minZ)),
	m_maxs(Vec3(maxX, maxY, maxZ))
{
}


//--------------------------------------------------------------------------------------------------
AABB3::AABB3(Vec3 const& mins, Vec3 const& maxs):
	m_mins(mins), 
	m_maxs(maxs)
{
}


//--------------------------------------------------------------------------------------------------
AABB3::AABB3(Vec3 const& center, float xDepth, float yWidth, float zHeight)
{
	float const& halfDepth		=	xDepth  * 0.5f;
	float const& halfWidth		=	yWidth  * 0.5f;
	float const& halfHeight		=	zHeight * 0.5f;
	
	m_mins.x = center.x - halfDepth;
	m_mins.y = center.y - halfWidth;
	m_mins.z = center.z - halfHeight;

	m_maxs.x = center.x + halfDepth;
	m_maxs.y = center.y + halfWidth;
	m_maxs.z = center.z + halfHeight;
}


//--------------------------------------------------------------------------------------------------
Vec3 AABB3::GetCenter() const
{
	float centerX = (m_maxs.x + m_mins.x) * 0.5f;
	float centerY = (m_maxs.y + m_mins.y) * 0.5f;
	float centerZ = (m_maxs.z + m_mins.z) * 0.5f;

	return Vec3(centerX, centerY, centerZ);
}


//--------------------------------------------------------------------------------------------------
void AABB3::SetDimensions(float width, float height, float depth)
{
	float halfWidth		=	width * 0.5f;
	float halfHeight	=	height * 0.5f;
	float halfDepth		=	depth * 0.5f;

	m_mins.x = -halfDepth;
	m_mins.y = -halfWidth;
	m_mins.z = -halfHeight;

	m_maxs.x = halfDepth;
	m_maxs.y = halfWidth;
	m_maxs.z = halfHeight;
}


//--------------------------------------------------------------------------------------------------
void AABB3::SetCenter(Vec3 const& newCenter)
{
	Vec3 currentCenter			=	GetCenter();
	Vec3 translationToApply		=	newCenter - currentCenter;
	Translate(translationToApply);
}


//--------------------------------------------------------------------------------------------------
void AABB3::SetCenter(float x, float y, float z)
{
	Vec3 currentCenter			=	GetCenter();
	Vec3 newCenter				=	Vec3(x, y, z);
	Vec3 translationToApply		=	newCenter - currentCenter;
	Translate(translationToApply);
}


//--------------------------------------------------------------------------------------------------
void AABB3::Translate(Vec3 const& translationToApply)
{
	m_mins += translationToApply;
	m_maxs += translationToApply;
}