#pragma once
#include "Engine/Math/IntVec2.hpp"

struct IntRange
{
	IntRange() {};
	explicit IntRange(int min, int max);

	bool IsOnRange(int numberToCheck);
	bool IsOverlappingWith(IntRange rangeB);

	void operator=(IntRange const& copyFrom);
	bool operator==(IntRange const& compare);
	bool operator!=(IntRange const& compare);

	static const IntRange ZERO;
	static const IntRange ONE;
	static const IntRange ZERO_TO_ONE;

	int m_min = 0;
	int m_max = 0;
};