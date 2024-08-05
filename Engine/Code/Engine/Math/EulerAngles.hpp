#pragma once


//--------------------------------------------------------------------------------------------------
struct Vec3;
struct Mat44;


//--------------------------------------------------------------------------------------------------
struct EulerAngles
{
public:
	EulerAngles() = default;
	EulerAngles(float yawDegrees, float pitchDegrees, float rollDegrees);

	void	GetAsVectors_XFwd_YLeft_ZUp(Vec3& out_forwardIBasis, Vec3& out_leftJBasis, Vec3& out_upKBasis)	const;
	Mat44	GetAsMatrix_XFwd_YLeft_ZUp()																	const;
	Vec3	GetXForward()																					const;
	Vec3	GetXForward(Vec3& out_forwardIBasis)															const;
	Vec3	GetYLeft()																						const;
	void	GetYLeft(Vec3& out_leftJBasis)																	const;

	void SetFromText(char const* text);

	// Operators (const)
	bool operator==(EulerAngles const& compare) const;

public:
	float m_yawDegrees = 0.f;
	float m_pitchDegrees = 0.f;
	float m_rollDegrees = 0.f;
};