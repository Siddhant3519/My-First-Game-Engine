#pragma once


//--------------------------------------------------------------------------------------------------
struct Vec4
{
	// NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
	float w = 0.f;

	Vec4() {};
	~Vec4() {};
	explicit Vec4(float initialX, float initialY, float initialZ, float initialW);
	
	Vec4 const operator-(Vec4 const& vecToSubtract)		const;		// vec4 - vec4
	Vec4 const operator-()								const;		// -vec4, i.e. "unary negation"
	void	   operator*=(float const uniformScale);				// vec4 *= float
};