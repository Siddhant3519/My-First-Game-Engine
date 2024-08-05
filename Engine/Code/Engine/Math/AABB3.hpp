#pragma once

//--------------------------------------------------------------------------------------------------
#include "Engine/Math/Vec3.hpp"


//--------------------------------------------------------------------------------------------------
struct AABB3
{
	AABB3() {};
	~AABB3() {};

	AABB3(AABB3 const& copyFrom);
	explicit AABB3(float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
	explicit AABB3(Vec3 const& mins, Vec3 const& maxs);
	explicit AABB3(Vec3 const& center, float xDepth, float yWidth, float zHeight);

	Vec3 GetCenter() const;
	void SetDimensions(float width, float height, float depth);
	void SetCenter(Vec3 const& newCenter);
	void SetCenter(float x, float y, float z);
	void Translate(Vec3 const& translationToApply);

	static AABB3 const ZERO_TO_ONE;
	static AABB3 const INVALID;

/*public:*/
	Vec3 m_mins;
	Vec3 m_maxs;
};