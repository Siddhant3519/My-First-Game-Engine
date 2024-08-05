#pragma once


//--------------------------------------------------------------------------------------------------
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec3.hpp"


//--------------------------------------------------------------------------------------------------
struct Vec3
{
	// NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;

	// Helper static variables
	static Vec3 const X_AXIS;
	static Vec3 const Y_AXIS;
	static Vec3 const Z_AXIS;
	static Vec3 const WORLD_ORIGIN;
	static Vec3 const ZERO;

public:
	// Construction/Destruction
	~Vec3() {}														// destructor (do nothing)
	Vec3() {}														// default constructor (do nothing)
	Vec3(Vec3 const& copyFrom);										// copy constructor (from another vec3) 
	
	explicit Vec3(float initialX, float initialY);					// explicit constructor (from x, y, z)
	explicit Vec3(float initialX, float initialY, float initialZ);
	explicit Vec3(Vec2 initialXY);
	explicit Vec3(Vec2 const& copyFrom, float initialZ);
	explicit Vec3(IntVec3 const& fromIntVec3);

	
	float		GetLength()										const;
	float		GetLengthXY()									const;
	float		GetLengthSquared()								const;
	float		GetLengthXYSquared()							const;
	float		GetAngleAboutZRadians()							const;
	float		GetAngleAboutZDegrees()							const;
	Vec3 const	GetRotatedAboutZRadians(float deltaRadians)		const;
	Vec3 const	GetRotatedAboutZDegrees(float deltaDegrees)		const;
	Vec3 const	GetClamped(float maxLength)						const;
	Vec3 const	GetNormalized()									const;


	void RotateAboutZRadians(float deltaRadians);
	void RotateAboutZDegrees(float deltaRadians);
	void SetPolarRadians(float newOrientationRadians, float newLength);
	void SetPolarDegrees(float newOrientationDegrees, float newLength);
	void Normalize();

	// static methods
	static Vec3 const MakeFromPolarRadians(float latitudeRadians, float longitudeRadians, float length = 1.f);
	static Vec3 const MakeFromPolarDegrees(float latitudeDegrees, float longitudeDegrees, float length = 1.f);
	static Vec3 const MakeFromSphericalRadians(float yawRadians, float pitchRadians, float length = 1.f);
	static Vec3 const MakeFromSphericalDegrees(float yawDegrees, float pitchDegrees, float length = 1.f);

	void SetFromText(char const* text);

	// Operators (const)
	bool		operator==(const Vec3& compare) const;		
	bool		operator!=(const Vec3& compare) const;		
	const Vec3	operator+(const Vec3& vecToAdd) const;		
	const Vec3	operator-(const Vec3& vecToSubtract) const;	
	const Vec3	operator-() const;							
	const Vec3	operator*(float uniformScale) const;		
	const Vec3	operator*(const Vec3& vecToMultiply) const;	
	const Vec3	operator/(float inverseScale) const;		

	// Operators (self-mutating / non-const)
	void		operator+=(const Vec3& vecToAdd);			
	void		operator-=(const Vec3& vecToSubtract);		
	void		operator*=(const float uniformScale);		
	void		operator/=(const float uniformDivisor);		
	void		operator=(const Vec3& copyFrom);			

	// Standalone "friend" functions that are conceptually, but not actually, part of Vec2::
	friend const Vec3 operator*(float uniformScale, const Vec3& vecToScale);
};


//--------------------------------------------------------------------------------------------------
// Construct a Vec3 with a z component of 0
Vec3 const MakeFromPolarRadians(float orientationRadians, float length = 1.f);
Vec3 const MakeFromPolarDegrees(float orientationDegrees, float length = 1.f);
