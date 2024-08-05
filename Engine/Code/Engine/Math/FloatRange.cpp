#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/FloatRange.hpp"


//--------------------------------------------------------------------------------------------------
const Vec2 FloatRange::ZERO(0.f, 0.f);
const Vec2 FloatRange::ONE(1.f, 1.f);
const Vec2 FloatRange::ZERO_TO_ONE(0.f, 1.f);


//--------------------------------------------------------------------------------------------------
FloatRange::FloatRange(float min, float max) :
	m_min(min), m_max(max)
{
}


//--------------------------------------------------------------------------------------------------
bool FloatRange::IsOnRange(float numberToCheck) const
{
	if (numberToCheck > m_max || numberToCheck < m_min)
	{
		return false;
	}

	return true;
}


//--------------------------------------------------------------------------------------------------
bool FloatRange::IsOverlappingWith(FloatRange rangeB) const
{
	if (m_min > rangeB.m_max || m_max < rangeB.m_min)
	{
		return false;
	}

	return true;
}


//--------------------------------------------------------------------------------------------------
void FloatRange::SetFromText(char const* text)
{
	Strings delimitedText = SplitStringOnDelimiter(text, '~');
	m_min = float(atof(delimitedText[0].data()));
	m_max = float(atof(delimitedText[1].data()));
}


//--------------------------------------------------------------------------------------------------
void FloatRange::operator=(FloatRange const& copyFrom)
{
	m_min = copyFrom.m_min;
	m_max = copyFrom.m_max;
}


//--------------------------------------------------------------------------------------------------
bool FloatRange::operator==(FloatRange const& compare)
{
	return ((m_min == compare.m_min) && (m_max == compare.m_max));
}


//--------------------------------------------------------------------------------------------------
bool FloatRange::operator!=(FloatRange const& compare)
{
	return ((m_min != compare.m_min) && (m_max != compare.m_max));
}
