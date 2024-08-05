#pragma once


//--------------------------------------------------------------------------------------------------
#include "Engine/Math/Plane2D.hpp"
#include "Engine/Math/ConvexPoly2D.hpp"

//--------------------------------------------------------------------------------------------------
#include <vector>


//--------------------------------------------------------------------------------------------------
class ConvexHull2D
{
public:
	ConvexHull2D();
	ConvexHull2D(ConvexPoly2D const& convexPoly2D);
	~ConvexHull2D();
	
public:
	std::vector<Plane2D> m_boundingPlanes;
};