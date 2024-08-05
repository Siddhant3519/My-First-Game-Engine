//--------------------------------------------------------------------------------------------------
#include "Engine/Math/Vec4.hpp"


//--------------------------------------------------------------------------------------------------
Vec4::Vec4(float initialX, float initialY, float initialZ, float initialW) :
	x(initialX), 
	y(initialY),
	z(initialZ),
	w(initialW)
{
}


//--------------------------------------------------------------------------------------------------
const Vec4 Vec4::operator-(Vec4 const& vecToSubtract) const
{
	return Vec4(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z, w - vecToSubtract.w);
}


//--------------------------------------------------------------------------------------------------
const Vec4 Vec4::operator-() const
{
	return Vec4(-x, -y, -z, -w);
}


//--------------------------------------------------------------------------------------------------
void Vec4::operator*=(float const uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
	w *= uniformScale;
}