#pragma once


//--------------------------------------------------------------------------------------------------
#include "Engine/Math/Vec3.hpp"


//--------------------------------------------------------------------------------------------------
struct OBB3
{
	OBB3();
	~OBB3();

	explicit OBB3(Vec3 const& center, Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis, Vec3 const& halfDims);
	
	void StretchToIncludePoint(Vec3 const& point);

public:
	Vec3	m_center;
	Vec3	m_iBasis;
	Vec3	m_jBasis;
	Vec3	m_kBasis;
	Vec3	m_halfDims;
};