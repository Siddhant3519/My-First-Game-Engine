#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Plane2D.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/ConvexHull2D.hpp"


//--------------------------------------------------------------------------------------------------
struct Vec3;
struct FloatRange;
struct AABB2;


//--------------------------------------------------------------------------------------------------
struct RaycastResult2D
{
	Vec2 m_impactPos;
	Vec2 m_impactNormal;
	float m_impactDist	=	0.f;
	bool m_didImpact	=	false;
};


//--------------------------------------------------------------------------------------------------
RaycastResult2D RaycastVsDisc2D(Vec2 const& startPos, Vec2 const& fwdNormal, float maxDist, Vec2 const& discCenter, float discRadius);
RaycastResult2D RaycastVsLineSegment2D(Vec2 const& rayStartPos, Vec2 const& fwdNormal, float maxDist, Vec2 const& lineSegmentStartPos, Vec2 const& lineSegmentEndPos);
RaycastResult2D RaycastVsAABB2D(Vec2 const& startPos, Vec2 const& fwdNormal, float maxDist, AABB2 const& bounds);
RaycastResult2D RaycastVsPlane2D(Vec2 const& startPos, Vec2 const& fwdNormal, float maxDist, Plane2D const& plane);
RaycastResult2D RaycastVsConvexHull2D(Vec2 const& startPos, Vec2 const& fwdNormal, float maxDist, ConvexHull2D const& convexHull);


//--------------------------------------------------------------------------------------------------
struct RaycastResult3D
{
	Vec3 m_impactPos;
	Vec3 m_impactNormal;
	float m_impactDist = 0.f;
	bool m_didImpact = false;
};


//--------------------------------------------------------------------------------------------------
RaycastResult3D RaycastVsCylinderZ3D(Vec3 const& start, Vec3 const& fwdNormal, float maxDist, Vec2 const& centerXY, FloatRange minMaxZ, float radius);