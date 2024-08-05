#include "Engine/Math/IntRange.hpp"

IntRange const IntRange::ZERO(0, 0);
IntRange const IntRange::ONE(1, 1);
IntRange const IntRange::ZERO_TO_ONE(0, 1);


IntRange::IntRange(int min, int max) :
	m_min(min), m_max(max)
{
}

bool IntRange::IsOnRange(int numberToCheck)
{
	if (numberToCheck > m_max || numberToCheck < m_min)
	{
		return false;
	}

	return true;
}

bool IntRange::IsOverlappingWith(IntRange rangeB)
{
	if (m_min > rangeB.m_max || m_max < rangeB.m_min)
	{
		return false;
	}

	return true;
}

void IntRange::operator=(IntRange const& copyFrom)
{
	m_min = copyFrom.m_min;
	m_max = copyFrom.m_max;
}

bool IntRange::operator==(IntRange const& compare)
{
	return ((m_min == compare.m_min) && (m_max == compare.m_max));
}

bool IntRange::operator!=(IntRange const& compare)
{
	return ((m_min != compare.m_min) && (m_max != compare.m_max));
}
