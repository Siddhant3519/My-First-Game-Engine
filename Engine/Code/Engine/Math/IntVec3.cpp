//--------------------------------------------------------------------------------------------------
#include "Engine/Math/IntVec3.hpp"


//--------------------------------------------------------------------------------------------------
IntVec3::IntVec3(IntVec3 const& copyFrom)
	: x(copyFrom.x), y(copyFrom.y), z(copyFrom.z)
{
}


//--------------------------------------------------------------------------------------------------
IntVec3::IntVec3(int initialX, int initialY, int initialZ)
	: x(initialX), y(initialY), z(initialZ)
{
}


//--------------------------------------------------------------------------------------------------
IntVec3 const IntVec3::operator+(IntVec3 const& vecToAdd) const
{
	IntVec3 result;
	result.x = x + vecToAdd.x;
	result.y = y + vecToAdd.y;
	result.z = z + vecToAdd.z;
	return result;
}


//--------------------------------------------------------------------------------------------------
IntVec3 const IntVec3::operator-(IntVec3 const& vecToSubtract) const
{
	IntVec3 result;
	result.x = x - vecToSubtract.x;
	result.y = y - vecToSubtract.y;
	result.z = z - vecToSubtract.z;
	return result;

}


//--------------------------------------------------------------------------------------------------
void IntVec3::operator=(IntVec3 const& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}


//--------------------------------------------------------------------------------------------------
void IntVec3::operator+=(IntVec3 const& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}


//--------------------------------------------------------------------------------------------------
void IntVec3::operator-=(IntVec3 const& vecToSubtract)
{
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
	x -= vecToSubtract.x;
}


//--------------------------------------------------------------------------------------------------
void IntVec3::operator*=(int uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
}


//--------------------------------------------------------------------------------------------------
bool IntVec3::operator==(IntVec3 const& compare) const
{
	return ((x == compare.x) && (y == compare.y) && (z == compare.z));
}


//--------------------------------------------------------------------------------------------------
bool IntVec3::operator!=(IntVec3 const& compare) const
{
	return ((x != compare.x) || (y != compare.y) || (z != compare.z));
}