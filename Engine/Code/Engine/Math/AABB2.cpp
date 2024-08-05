#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"


//--------------------------------------------------------------------------------------------------
AABB2 const AABB2::ZERO_TO_ONE(0.f, 0.f, 1.f, 1.f);
AABB2 const AABB2::INVALID(-1.f, -1.f, -1.f, -1.f);


//--------------------------------------------------------------------------------------------------
bool AABB2::operator==(AABB2 const& compare) const
{
	return (m_mins == compare.m_mins && m_maxs == compare.m_maxs);
}


//--------------------------------------------------------------------------------------------------
AABB2::AABB2(float minX, float minY, float maxX, float maxY)
	: m_mins(Vec2(minX, minY)),
	m_maxs(Vec2 (maxX, maxY))
{
}


//--------------------------------------------------------------------------------------------------
AABB2::AABB2(Vec2 const& mins, Vec2 const& maxs) 
	: m_mins(mins), m_maxs(maxs)
{
}


//--------------------------------------------------------------------------------------------------
AABB2::AABB2(Vec2 const& center, float width, float height)
{
	Vec2 mins;
	Vec2 maxs;
	Vec2 halfDimensions;
	halfDimensions.x = width * 0.5f;
	halfDimensions.y = height * 0.5f;

	mins.x = center.x - halfDimensions.x;
	mins.y = center.y - halfDimensions.y;

	maxs.x = center.x + halfDimensions.x;
	maxs.y = center.y + halfDimensions.y;

	m_mins = mins;
	m_maxs = maxs;
}


//--------------------------------------------------------------------------------------------------
bool AABB2::IsPointInside(Vec2 const& point) const
{
	if (m_mins.x < point.x && m_maxs.x > point.x && m_mins.y < point.y && m_maxs.y > point.y)
	{
		return true;
	}

	return false;
}


//--------------------------------------------------------------------------------------------------
Vec2 const AABB2::GetCenter() const
{
	float centerX = (m_maxs.x + m_mins.x) * 0.5f;
	float centerY = (m_maxs.y + m_mins.y) * 0.5f;

	return Vec2(centerX, centerY);
}


//--------------------------------------------------------------------------------------------------
Vec2 const AABB2::GetDimensions() const
{
	float width = m_maxs.x - m_mins.x;
	float height = m_maxs.y - m_mins.y;
	return Vec2 (width, height);
}


//--------------------------------------------------------------------------------------------------
Vec2 const AABB2::GetNearestPoint(Vec2 const& referencePosition) const
{
	float nearestX = 0.f;
	float nearestY = 0.f;
	

	// Clamp (px, minX, maxX);
	// Clamp (py, minY, maxY);

	if (referencePosition.x <= m_maxs.x && referencePosition.x >= m_mins.x)
	{
		nearestX = referencePosition.x;
	}
	else if (referencePosition.x < m_mins.x)
	{
		nearestX = m_mins.x;
	}
	else if (referencePosition.x > m_maxs.x)
	{
		nearestX = m_maxs.x;
	}

	if (referencePosition.y <= m_maxs.y && referencePosition.y >= m_mins.y)
	{
		nearestY = referencePosition.y;
	}
	else if (referencePosition.y < m_mins.y)
	{
		nearestY = m_mins.y;
	}
	else if (referencePosition.y > m_maxs.y)
	{
		nearestY = m_maxs.y;
	}

	return Vec2 (nearestX, nearestY);
}


//--------------------------------------------------------------------------------------------------
Vec2 const AABB2::GetPointAtUV(Vec2 const& uv) const
{
	float pointAtUV_X = Interpolate(m_mins.x, m_maxs.x, uv.x);
	float pointAtUV_Y = Interpolate(m_mins.y, m_maxs.y, uv.y);
	// float pointAtUV_X = RangeMap(uv.x, 0.f, 1.f, m_mins.x, m_maxs.x);
	// float pointAtUV_Y = RangeMap(uv.y, 0.f, 1.f, m_mins.y, m_maxs.y);

	return Vec2(pointAtUV_X, pointAtUV_Y);
}


//--------------------------------------------------------------------------------------------------
Vec2 const AABB2::GetUVForPoint(Vec2 const& point) const
{
	float uvForPoint_X = GetFractionWithinRange(point.x, m_mins.x, m_maxs.x);
	float uvForPoint_Y = GetFractionWithinRange(point.y, m_mins.y, m_maxs.y);

	return Vec2(uvForPoint_X, uvForPoint_Y);
}


//--------------------------------------------------------------------------------------------------
AABB2 const AABB2::GetBoxAtUVs(AABB2 const& uvBounds) const
{
	Vec2 mins = GetPointAtUV(uvBounds.m_mins);
	Vec2 maxs = GetPointAtUV(uvBounds.m_maxs);

	return AABB2(mins, maxs);
}


//--------------------------------------------------------------------------------------------------
AABB2 const AABB2::GetBoxAtUVs(float uvMinX, float uvMinY, float uvMaxX, float uvMaxY) const
{
	Vec2 mins = GetPointAtUV(Vec2(uvMinX, uvMinY));
	Vec2 maxs = GetPointAtUV(Vec2(uvMaxX, uvMaxY));

	return AABB2(mins, maxs);
}


//--------------------------------------------------------------------------------------------------
void AABB2::AddPadding(float paddingX, float paddingY)
{
	m_mins.x -= paddingX;
	m_maxs.x += paddingX;
	m_mins.y -= paddingY;
	m_maxs.y += paddingY;
}


//--------------------------------------------------------------------------------------------------
void AABB2::Translate(Vec2 const& translationToApply)
{
	m_mins += translationToApply;
	m_maxs += translationToApply;
}


//--------------------------------------------------------------------------------------------------
void AABB2::SetCenter(Vec2 const& newCenter)
{
	Vec2 currentCenter = GetCenter();

	Vec2 translationToApply = newCenter - currentCenter;

	Translate(translationToApply);
}


//--------------------------------------------------------------------------------------------------
void AABB2::SetCenter(float centerX, float centerY)
{
	Vec2 currentCenter		= GetCenter();
	Vec2 newCenter			= Vec2(centerX, centerY);
	
	Vec2 translationToApply = newCenter - currentCenter;
	Translate(translationToApply);
}


//--------------------------------------------------------------------------------------------------
void AABB2::SetDimensions(Vec2 const& newDimensions)
{
	Vec2 currentCenter = GetCenter();
	Vec2 newHalfDimensions = newDimensions * 0.5f;
	
	float minX_Origin = -newHalfDimensions.x;
	float maxX_Origin = newHalfDimensions.x;

	float minY_Origin = -newHalfDimensions.y;
	float maxY_Origin = newHalfDimensions.y;

	AABB2 newDimensionsAroundOrigin (minX_Origin, minY_Origin, maxX_Origin, maxY_Origin);
	
	m_mins = newDimensionsAroundOrigin.m_mins + currentCenter;
	m_maxs = newDimensionsAroundOrigin.m_maxs + currentCenter;
}


//--------------------------------------------------------------------------------------------------
void AABB2::SetDimensions(float width, float height)
{
	Vec2 currentCenter (GetCenter());
	Vec2 newHalfDims (width * 0.5f, height * 0.5f);

	Vec2 minsAroundOrigin;
	minsAroundOrigin.x = -newHalfDims.x;
	minsAroundOrigin.y = -newHalfDims.y;

	Vec2 maxsAroundOrigin;
	maxsAroundOrigin.x = newHalfDims.x;
	maxsAroundOrigin.y = newHalfDims.y;

	m_mins = minsAroundOrigin + currentCenter;
	m_maxs = maxsAroundOrigin + currentCenter;
}


//--------------------------------------------------------------------------------------------------
void AABB2::StretchToIncludePoint(Vec2 const& point)
{
	if (IsPointInside(point))
	{
		return;
	}

	if (m_mins.x > point.x)
	{
		m_mins.x = point.x;
	}
	else if (m_maxs.x < point.x)
	{
		m_maxs.x = point.x;
	}

	if (m_mins.y > point.y)
	{
		m_mins.y = point.y;
	}
	else if (m_maxs.y < point.y)
	{
		m_maxs.y = point.y;
	}
}