//--------------------------------------------------------------------------------------------------
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"


//--------------------------------------------------------------------------------------------------
#include <math.h>


//--------------------------------------------------------------------------------------------------
IntVec2::IntVec2(IntVec2 const& copyFrom) :
	x(copyFrom.x), y(copyFrom.y)
{
}


//--------------------------------------------------------------------------------------------------
IntVec2::IntVec2(int initialX, int initialY) :
	x (initialX), y (initialY)
{
}


//--------------------------------------------------------------------------------------------------
float IntVec2::GetLength() const
{
	int distanceSquared = (x * x) + (y * y);
	return sqrtf((float)distanceSquared);
}


//--------------------------------------------------------------------------------------------------
int IntVec2::GetTaxicabLength() const
{
	int taxiCabLength = ABS_INT(x) + ABS_INT(y);
	return taxiCabLength;
}


//--------------------------------------------------------------------------------------------------
int IntVec2::GetLengthSquared() const
{
	int distanceSquared = (x * x) + (y * y);	
	return distanceSquared;
}


//--------------------------------------------------------------------------------------------------
float IntVec2::GetOrientationRadians() const
{
	float orientationInRadians = atan2f((float)y, (float)x);	
	return orientationInRadians;
}


//--------------------------------------------------------------------------------------------------
float IntVec2::GetOrientationDegrees() const
{
	float orientationInRadians = GetOrientationRadians();
	return (orientationInRadians * 180.f * INVERSE_PI);
}


//--------------------------------------------------------------------------------------------------
IntVec2 const IntVec2::GetRotated90Degrees() const
{
	return IntVec2(-y, x);
}


//--------------------------------------------------------------------------------------------------
IntVec2 const IntVec2::GetRotatedMinus90Degrees() const
{
	return IntVec2(y, -x);
}


//--------------------------------------------------------------------------------------------------
void IntVec2::Rotate90Degrees()
{
	int oldY	=	y;
	y			=	x;
	x			=	-oldY;
}


//--------------------------------------------------------------------------------------------------
void IntVec2::RotateMinus90Degrees()
{
	int oldY	=	y;
	y			=	-x;
	x			=	oldY;
}


//--------------------------------------------------------------------------------------------------
void IntVec2::SetFromText(char const* text)
{
	Strings delimitedText	=	SplitStringOnDelimiter(text, ',');
	x						=	int(atoi(delimitedText[0].data()));
	y						=	int(atoi(delimitedText[1].data()));
}


//--------------------------------------------------------------------------------------------------
IntVec2 const IntVec2::operator+(IntVec2 const& vecToAdd) const
{
	return IntVec2(x + vecToAdd.x, y + vecToAdd.y);
}


//--------------------------------------------------------------------------------------------------
IntVec2 const IntVec2::operator-(IntVec2 const& vecToSubtract) const
{
	return IntVec2(x - vecToSubtract.x, y - vecToSubtract.y);
}


//--------------------------------------------------------------------------------------------------
void IntVec2::operator=(IntVec2 const& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
}


//--------------------------------------------------------------------------------------------------
bool IntVec2::operator==(const IntVec2& compare) const
{
	return ((x == compare.x) && (y == compare.y));
}


//--------------------------------------------------------------------------------------------------
bool IntVec2::operator!=(const IntVec2& compare) const
{
	return ((x != compare.x) || (y != compare.y));
}