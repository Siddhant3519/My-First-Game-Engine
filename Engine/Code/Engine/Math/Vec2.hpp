#pragma once


//-----------------------------------------------------------------------------------------------
struct Vec2
{
	// NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x = 0.f;
	float y = 0.f;

public:
	Vec2() {}												// default constructor (do nothing)
	~Vec2() {}												// destructor (do nothing)
	Vec2( Vec2 const& copyFrom );							// copy constructor (from another vec2)
	explicit Vec2( float initialX, float initialY );		// explicit constructor (from x, y)

	// static functions
	static Vec2 const MakeFromPolarRadians(float orientationRadians, float length = 1.f);
	static Vec2 const MakeFromPolarDegrees(float orientationDegrees, float length = 1.f);

	float		GetLength()										const;
	float		GetLengthSquared()								const;
	float		GetOrientationRadians()							const;
	float		GetOrientationDegrees()							const;
	Vec2 const	GetRotated90Degrees()							const;
	Vec2 const	GetRotatedMinus90Degrees()						const;
	Vec2 const	GetRotatedRadians(float deltaRadians)			const;
	Vec2 const	GetRotatedDegrees(float deltaDegrees)			const;
	Vec2 const	GetClamped(float maxLength)						const;
	Vec2 const	GetNormalized()									const;
	Vec2 const	GetReflected(Vec2 const& bounceSurfaceNormal)	const;

	void		SetOrientationRadians(float newOrientationRadians);
	void		SetOrientationDegrees(float newOrientationDegrees);
	void		SetPolarRadians(float newOrientationRadians, float newLength);
	void		SetPolarDegrees(float newOrientationDegrees, float newLength);
	void		Rotate90Degrees();
	void		RotateMinus90Degrees();
	void		RotateRadians(float deltaRadians);
	void		RotateDegrees(float deltaDegrees);
	void		SetLength(float newLength);
	void		ClampLength(float maxLength);
	void		Normalize();
	void		Reflect(Vec2 bounceSurfaceNormal);
	float		NormalizeAndGetPreviousLength();
	void		SetFromText(char const* text);

	// Operators (const)
	bool		operator==(Vec2 const& compare)			const;		// vec2 == vec2
	bool		operator!=(Vec2 const& compare)			const;		// vec2 != vec2
	Vec2 const 	operator+(Vec2 const& vecToAdd)			const;		// vec2 + vec2
	Vec2 const 	operator-(Vec2 const& vecToSubtract)	const;		// vec2 - vec2
	Vec2 const 	operator-()								const;		// -vec2, i.e. "unary negation"
	Vec2 const 	operator*(float uniformScale)			const;		// vec2 * float
	Vec2 const 	operator*(Vec2 const& vecToMultiply)	const;		// vec2 * vec2
	Vec2 const 	operator/(float inverseScale)			const;		// vec2 / float

	// Operators (self-mutating / non-const)
	void		operator+=(Vec2 const& vecToAdd );			// vec2 += vec2
	void		operator-=(Vec2 const& vecToSubtract );		// vec2 -= vec2
	void		operator*=(float const uniformScale );		// vec2 *= float
	void		operator/=(float const uniformDivisor );	// vec2 /= float
	void		operator=(Vec2 const& copyFrom );			// vec2 = vec2

	// Standalone "friend" functions that are conceptually, but not actually, part of Vec2::
	friend Vec2 const operator*( float uniformScale, const Vec2& vecToScale );	// float * vec2
};


