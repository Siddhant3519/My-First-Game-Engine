//--------------------------------------------------------------------------------------------------
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"


//--------------------------------------------------------------------------------------------------
#include <math.h>


//--------------------------------------------------------------------------------------------------
Vec3 const Vec3::WORLD_ORIGIN(0.f, 0.f, 0.f);
Vec3 const Vec3::X_AXIS(1.f, 0.f, 0.f);
Vec3 const Vec3::Y_AXIS(0.f, 1.f, 0.f);
Vec3 const Vec3::Z_AXIS(0.f, 0.f, 1.f);
Vec3 const Vec3::ZERO(0.f, 0.f, 0.f);


//--------------------------------------------------------------------------------------------------
Vec3::Vec3(const Vec3& copyFrom) : x(copyFrom.x), y(copyFrom.y), z(copyFrom.z)
{
}


//--------------------------------------------------------------------------------------------------
Vec3::Vec3(float initialX, float initialY) : x(initialX), y(initialY)
{
}


//--------------------------------------------------------------------------------------------------
Vec3::Vec3(float initialX, float initialY, float initialZ) : x (initialX), y (initialY), z (initialZ)
{
}


//--------------------------------------------------------------------------------------------------
Vec3::Vec3(Vec2 initialXY):
	x(initialXY.x), y(initialXY.y), z(0.f)
{
}


//--------------------------------------------------------------------------------------------------
Vec3::Vec3(Vec2 const& copyFrom, float initialZ) : x(copyFrom.x), y(copyFrom.y), z(initialZ)
{
}


//--------------------------------------------------------------------------------------------------
Vec3::Vec3(IntVec3 const& fromIntVec3)
	: x((float)fromIntVec3.x),
	  y((float)fromIntVec3.y),
	  z((float)fromIntVec3.z)
{
}


//--------------------------------------------------------------------------------------------------
float Vec3::GetLength() const
{
	return sqrtf((x * x) + (y * y) + (z * z));
}


//--------------------------------------------------------------------------------------------------
float Vec3::GetLengthXY() const
{
	return sqrtf((x * x) + (y * y));
}


//--------------------------------------------------------------------------------------------------
float Vec3::GetLengthSquared() const
{
	return ((x * x) + (y * y) + (z * z));
}


//--------------------------------------------------------------------------------------------------
float Vec3::GetLengthXYSquared() const
{
	return ((x * x) + (y * y));
}


//--------------------------------------------------------------------------------------------------
float Vec3::GetAngleAboutZRadians() const
{
	return (atan2f(y, x));
}


//--------------------------------------------------------------------------------------------------
float Vec3::GetAngleAboutZDegrees() const
{
	float angleInRadians = GetAngleAboutZRadians();
	return (angleInRadians * 180 * INVERSE_PI);
}


//--------------------------------------------------------------------------------------------------
Vec3 const Vec3::GetRotatedAboutZRadians(float deltaRadians) const
{
	// Calculate current length, orientation (radians) and final desired orientation (radians)
	float currentLength			=	GetLengthXY();
	float angleAboutZRadians	=	GetAngleAboutZRadians();
	angleAboutZRadians			+=	deltaRadians;

	return Vec3 ((currentLength * cosf(angleAboutZRadians)), (currentLength * sinf(angleAboutZRadians)), (z));
}


//--------------------------------------------------------------------------------------------------
Vec3 const Vec3::GetRotatedAboutZDegrees(float deltaDegrees) const
{
	float deltaRadians = deltaDegrees * PI * INVERSE_ONE_EIGHTY;
	return GetRotatedAboutZRadians(deltaRadians);
}


//--------------------------------------------------------------------------------------------------
Vec3 const Vec3::GetClamped(float maxLength) const
{
	float currentLengthSquared = GetLengthSquared();

	if (currentLengthSquared > (maxLength * maxLength))
	{
		Vec3 normalizedVec	=	GetNormalized();
		Vec3 clampedVec		=	normalizedVec * maxLength;
		return clampedVec;
	}
	return Vec3(x, y, z);
}


//--------------------------------------------------------------------------------------------------
Vec3 const Vec3::GetNormalized() const
{
	float magnitude = GetLength();
	if (magnitude == 0.f)
	{
		return Vec3::ZERO;
	}

	float inverseMagMultiplier	=	1.f / magnitude;
	Vec3 normalizedVec			=	Vec3((x * inverseMagMultiplier), (y * inverseMagMultiplier), (z * inverseMagMultiplier));
	return normalizedVec;
}


//--------------------------------------------------------------------------------------------------
void Vec3::RotateAboutZRadians(float deltaRadians)
{
	// Calculate current length, orientation (radians), and desired orientation (radians)
	float currentLength		=	GetLengthXY();
	float angleInRadians	=	atan2f(y, x);
	angleInRadians			+=	deltaRadians;
	SetPolarRadians(angleInRadians, currentLength);
}


//--------------------------------------------------------------------------------------------------
void Vec3::RotateAboutZDegrees(float deltaDegrees)
{
	float angleInRadians = ConvertDegreesToRadians(deltaDegrees);
	RotateAboutZRadians(angleInRadians);
}


//--------------------------------------------------------------------------------------------------
void Vec3::SetPolarRadians(float newOrientationRadians, float newLength)
{
	x = newLength * cosf(newOrientationRadians);
	y = newLength * sinf(newOrientationRadians);
	z = z;
}


//--------------------------------------------------------------------------------------------------
void Vec3::SetPolarDegrees(float newOrientationDegrees, float newLength)
{
	float newOrientationRadians = ConvertDegreesToRadians(newOrientationDegrees);
	SetPolarRadians(newOrientationRadians, newLength);
}


//--------------------------------------------------------------------------------------------------
void Vec3::Normalize()
{
	float magnitude = GetLength();
	if (magnitude == 0.f) return;
	float inverseMagMultiplier = 1.f / magnitude;

	x *= inverseMagMultiplier;
	y *= inverseMagMultiplier;
	z *= inverseMagMultiplier;
}


//--------------------------------------------------------------------------------------------------
Vec3 const Vec3::MakeFromPolarRadians(float latitudeRadians, float longitudeRadians, float length)
{
	float cy = cosf(longitudeRadians);
	float cp = cosf(latitudeRadians);
	float sy = sinf(longitudeRadians);
	float sp = sinf(latitudeRadians);

	return Vec3(length * cy * cp, length * sy * cp, length * -sp);
}


//--------------------------------------------------------------------------------------------------
Vec3 const Vec3::MakeFromPolarDegrees(float latitudeDegrees, float longitudeDegrees, float length)
{
	float cy = CosDegrees(longitudeDegrees);
	float cp = CosDegrees(latitudeDegrees);
	float sy = SinDegrees(longitudeDegrees);
	float sp = SinDegrees(latitudeDegrees);

	return Vec3(length * cy * cp, length * sy * cp, length * -sp);
}


//--------------------------------------------------------------------------------------------------
Vec3 const Vec3::MakeFromSphericalRadians(float yawRadians, float pitchRadians, float length /*= 1.f*/)
{
	Vec3 vectorInCartesianCoords;
	vectorInCartesianCoords.x		=		cosf( yawRadians ) * cosf( pitchRadians );
	vectorInCartesianCoords.y		=		sinf( yawRadians ) * cosf( pitchRadians );
	vectorInCartesianCoords.z		=		-sinf( pitchRadians );
	vectorInCartesianCoords			*=		length;
	return vectorInCartesianCoords;
}


//--------------------------------------------------------------------------------------------------
Vec3 const Vec3::MakeFromSphericalDegrees(float yawDegrees, float pitchDegrees, float length /*= 1.f*/)
{
	Vec3 vectorInCartesianCoords;
	float pitchRadians			=	ConvertDegreesToRadians( pitchDegrees );
	float yawRadians			=	ConvertDegreesToRadians( yawDegrees );
	vectorInCartesianCoords		=	MakeFromSphericalRadians( yawRadians, pitchRadians, length );
	return vectorInCartesianCoords;
}


//--------------------------------------------------------------------------------------------------
void Vec3::SetFromText(char const* text)
{
	Strings delimitedText = SplitStringOnDelimiter(text, ',');
	x = float(atof(delimitedText[0].data()));
	y = float(atof(delimitedText[1].data()));

	if (delimitedText.size() > 2)
	{
		z = float(atof(delimitedText[2].data()));
	}
}


//--------------------------------------------------------------------------------------------------
bool Vec3::operator==(Vec3 const& compare) const
{
	return (x == compare.x && y == compare.y && z == compare.z);
}


//--------------------------------------------------------------------------------------------------
bool Vec3::operator!=(Vec3 const& compare) const
{
	return (x != compare.x || y != compare.y || z != compare.z);
}


//--------------------------------------------------------------------------------------------------
Vec3 const Vec3::operator+(Vec3 const& vecToAdd) const
{
	return Vec3(x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z);
}


//--------------------------------------------------------------------------------------------------
Vec3 const Vec3::operator-(Vec3 const& vecToSubtract) const
{
	return Vec3(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z);
}


//--------------------------------------------------------------------------------------------------
Vec3 const Vec3::operator-() const
{
	return Vec3(-x, -y, -z);
}


//--------------------------------------------------------------------------------------------------
Vec3 const Vec3::operator*(float uniformScale) const
{
	return Vec3(x * uniformScale, y * uniformScale, z * uniformScale);
}


//--------------------------------------------------------------------------------------------------
Vec3 const Vec3::operator*(Vec3 const& vecToMultiply) const
{
	return Vec3(x * vecToMultiply.x, y * vecToMultiply.y, z * vecToMultiply.z);
}


//--------------------------------------------------------------------------------------------------
Vec3 const Vec3::operator/(float inverseScale) const
{
	float multiplier = 1.f / inverseScale;
	return Vec3(x * multiplier, y * multiplier, z * multiplier);
}


//--------------------------------------------------------------------------------------------------
void Vec3::operator+=(Vec3 const& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}


//--------------------------------------------------------------------------------------------------
void Vec3::operator-=(Vec3 const& vecToSubtract)
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}


//--------------------------------------------------------------------------------------------------
void Vec3::operator*=(float const uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
}


//--------------------------------------------------------------------------------------------------
void Vec3::operator/=(float const uniformDivisor)
{
	float multiplier = 1.f / uniformDivisor;

	x *= multiplier;
	y *= multiplier;
	z *= multiplier;
}


//--------------------------------------------------------------------------------------------------
void Vec3::operator=(Vec3 const& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}


//--------------------------------------------------------------------------------------------------
Vec3 const operator*(float uniformScale, Vec3 const& vecToScale)
{
	return Vec3(uniformScale * vecToScale.x, uniformScale * vecToScale.y, uniformScale * vecToScale.z);
}


//--------------------------------------------------------------------------------------------------
Vec3 const MakeFromPolarRadians(float orientationRadians, float length)
{
	return Vec3(length * cosf(orientationRadians), length * sinf(orientationRadians), 0);
}


//--------------------------------------------------------------------------------------------------
Vec3 const MakeFromPolarDegrees(float orientationDegrees, float length)
{
	float orientationInRadians = ConvertDegreesToRadians(orientationDegrees);

	return Vec3(length * cosf(orientationInRadians), length * sinf(orientationInRadians), 0);
}