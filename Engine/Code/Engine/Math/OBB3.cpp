#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/MathUtils.hpp"


//--------------------------------------------------------------------------------------------------
OBB3::OBB3()
{
}


//--------------------------------------------------------------------------------------------------
OBB3::~OBB3()
{
}


//--------------------------------------------------------------------------------------------------
OBB3::OBB3(Vec3 const& center, Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis, Vec3 const& halfDims) : 
	m_center(center),
	m_iBasis(iBasis),
	m_jBasis(jBasis),
	m_kBasis(kBasis),
	m_halfDims(halfDims)
{
}


//--------------------------------------------------------------------------------------------------
void OBB3::StretchToIncludePoint(Vec3 const& point)
{
	Vec3 dispFromCenterToPoint = point - m_center;
	float localX	=	DotProduct3D(dispFromCenterToPoint, m_iBasis);
	float localY	=	DotProduct3D(dispFromCenterToPoint, m_jBasis);
	float localZ	=	DotProduct3D(dispFromCenterToPoint, m_kBasis);
	float absLocalX		=	ABS_FLOAT(localX);
	float absLocalY		=	ABS_FLOAT(localY);
	float absLocalZ		=	ABS_FLOAT(localZ);
	
	if (m_halfDims.x < absLocalX)
	{
		float growDist		=	absLocalX - m_halfDims.x;
		float halfGrowDist	=	growDist * 0.5f;
		m_halfDims.x		+=	halfGrowDist;
		float sign			=	localX < 0.f ? -1.f : 1.f;
		m_center			+=	(m_iBasis * sign * halfGrowDist);
	}

	if (m_halfDims.y < absLocalY)
	{
		float growDist		=	absLocalY - m_halfDims.y;
		float halfGrowDist	=	growDist * 0.5f;
		m_halfDims.y		+=	halfGrowDist;
		float sign			=	localY < 0.f ? -1.f : 1.f;
		m_center			+=	(m_jBasis * sign * halfGrowDist);
	}

	if (m_halfDims.z < absLocalZ)
	{
		float growDist		=	absLocalZ - m_halfDims.z;
		float halfGrowDist	=	growDist * 0.5f;
		m_halfDims.z		+=	halfGrowDist;
		float sign			=	localZ < 0.f ? -1.f : 1.f;
		m_center			+=	(m_kBasis * sign * halfGrowDist);
	}
}