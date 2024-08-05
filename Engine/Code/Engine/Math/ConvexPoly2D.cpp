#include "Engine/Math/ConvexPoly2D.hpp"


//--------------------------------------------------------------------------------------------------
ConvexPoly2D::ConvexPoly2D()
{

}


//--------------------------------------------------------------------------------------------------
// CCW according to our SD engine conventions
ConvexPoly2D::ConvexPoly2D(std::vector<Vec2> const& ccwOrderedPoints) :
	m_points(ccwOrderedPoints)
{

}


//--------------------------------------------------------------------------------------------------
ConvexPoly2D::~ConvexPoly2D()
{

}


//--------------------------------------------------------------------------------------------------
std::vector<Vec2> const& ConvexPoly2D::GetPoints2D() const
{
	return m_points;
}
