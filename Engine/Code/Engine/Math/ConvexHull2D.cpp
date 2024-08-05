#include "Engine/Math/ConvexHull2D.hpp"
#include "Engine/Math/MathUtils.hpp"

//--------------------------------------------------------------------------------------------------
ConvexHull2D::ConvexHull2D()
{
}


//--------------------------------------------------------------------------------------------------
ConvexHull2D::ConvexHull2D(ConvexPoly2D const& convexPoly2D)
{
	std::vector<Vec2> const& pointList	=	convexPoly2D.GetPoints2D();
	Vec2 const& firstPoint				=	pointList[0];
	size_t const& numOfPoints			=	pointList.size();

	for (size_t pointIndex = 0; pointIndex < numOfPoints - 1; ++pointIndex)
	{
		Plane2D currentPlane;
		Vec2 const& currentPoint		=	pointList[pointIndex];
		Vec2 const& nextPoint			=	pointList[pointIndex + 1];
		Vec2 dispFromCurrentToNext		=	nextPoint - currentPoint;
		Vec2 planeNormal				=	dispFromCurrentToNext.GetRotatedMinus90Degrees();
		planeNormal.Normalize();
		currentPlane.m_distFromOrigin	=	DotProduct2D(currentPoint, planeNormal);
		currentPlane.m_normal			=	planeNormal;
		m_boundingPlanes.emplace_back(currentPlane);
	}
	Plane2D lastPlane;
	Vec2 lastPoint				=	pointList[numOfPoints - 1];
	Vec2 dispFromLastToFirst	=	firstPoint - lastPoint;
	Vec2 planeNormal			=	dispFromLastToFirst.GetRotatedMinus90Degrees();
	planeNormal.Normalize();
	lastPlane.m_distFromOrigin	=	DotProduct2D(lastPoint, planeNormal);
	lastPlane.m_normal			=	planeNormal;
	m_boundingPlanes.emplace_back(lastPlane);
}


//--------------------------------------------------------------------------------------------------
ConvexHull2D::~ConvexHull2D()
{
}