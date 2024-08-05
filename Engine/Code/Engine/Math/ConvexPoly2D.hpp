#pragma once


//--------------------------------------------------------------------------------------------------
#include "Engine/Math/Vec2.hpp"


//--------------------------------------------------------------------------------------------------
#include <vector>


//--------------------------------------------------------------------------------------------------
class ConvexPoly2D
{
private:
	std::vector<Vec2> m_points; // Currently only support ordered points (CCW points, CCW with respect to our SD conventions)

public:
	ConvexPoly2D();
	ConvexPoly2D(std::vector<Vec2> const& ccwOrderedPoints);
	~ConvexPoly2D();

	std::vector<Vec2> const& GetPoints2D() const;
};