#pragma once
#include "Engine/Math/Vec2.hpp"

struct FloatRange
{
	FloatRange() {};
	explicit FloatRange(float min, float max);

	bool IsOnRange(float numberToCheck)			const;
	bool IsOverlappingWith(FloatRange rangeB)	const;

	void SetFromText(char const* text);

	void operator=(FloatRange const& copyFrom);
	bool operator==(FloatRange const& compare);
	bool operator!=(FloatRange const& compare);

	static const Vec2 ZERO;
	static const Vec2 ONE;
	static const Vec2 ZERO_TO_ONE;

	float m_min = 0.f;
	float m_max = 0.f;
};