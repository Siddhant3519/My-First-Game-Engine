#pragma once
#include "Engine/Math/Vec2.hpp"


//--------------------------------------------------------------------------------------------------
struct AABB2
{
public:
	Vec2 m_mins;
	Vec2 m_maxs;

	static AABB2 const ZERO_TO_ONE;
	static AABB2 const INVALID;
public:
	AABB2() {};
	~AABB2() {};
	
	// Operator (const)
	bool operator==(AABB2 const& compare) const;

	explicit AABB2(float minX, float minY, float maxX, float maxY);
	explicit AABB2(Vec2 const& mins, Vec2 const& maxs);
	explicit AABB2(Vec2 const& center, float width, float height);
	
	bool			IsPointInside(Vec2 const& point)									const;
	Vec2	const	GetCenter()															const;
	Vec2	const	GetDimensions()														const;
	Vec2	const	GetNearestPoint(Vec2 const& referencePosition)						const;
	Vec2	const	GetPointAtUV(Vec2 const& uv)										const;
	Vec2	const	GetUVForPoint(Vec2 const& point)									const;
	AABB2	const	GetBoxAtUVs(AABB2 const& uvBounds)									const;
	AABB2	const	GetBoxAtUVs(float uvMinX, float uvMinY, float uvMaxX, float uvMaxY) const;
	void AddPadding(float paddingX, float paddingY);

	void Translate(Vec2 const& translationToApply);
	void SetCenter(Vec2 const& newCenter);
	void SetCenter(float centerX, float centerY);
	void SetDimensions(Vec2 const& newDimensions);
	void SetDimensions(float width, float height);
	void StretchToIncludePoint(Vec2 const& point);
};