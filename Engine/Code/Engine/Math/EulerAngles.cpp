#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec3.hpp"


//--------------------------------------------------------------------------------------------------
EulerAngles::EulerAngles(float yawDegrees, float pitchDegrees, float rollDegrees) :
	m_yawDegrees(yawDegrees), m_pitchDegrees(pitchDegrees), m_rollDegrees(rollDegrees)
{
}


//--------------------------------------------------------------------------------------------------
void EulerAngles::GetAsVectors_XFwd_YLeft_ZUp(Vec3& out_forwardIBasis, Vec3& out_leftJBasis, Vec3& out_upKBasis) const
{
	float cy = CosDegrees(m_yawDegrees);
	float cp = CosDegrees(m_pitchDegrees);
	float cr = CosDegrees(m_rollDegrees);
	float sy = SinDegrees(m_yawDegrees);
	float sp = SinDegrees(m_pitchDegrees);
	float sr = SinDegrees(m_rollDegrees);

	out_forwardIBasis.x = cy * cp;
	out_forwardIBasis.y = sy * cp;
	out_forwardIBasis.z = -sp;

	out_leftJBasis.x = (-sy * cr) + (cy * sp * sr);
	out_leftJBasis.y = (cy * cr) + (sy * sp * sr);
	out_leftJBasis.z = cp * sr;

	out_upKBasis.x = (sy * sr) + (cy * sp * cr);
	out_upKBasis.y = (-cy * sr) + (sy * sp * cr);
	out_upKBasis.z = cp * cr;
}


//--------------------------------------------------------------------------------------------------
Mat44 EulerAngles::GetAsMatrix_XFwd_YLeft_ZUp() const
{
	Mat44 transformedMatrix; 
	float cy = CosDegrees(m_yawDegrees);
	float cp = CosDegrees(m_pitchDegrees);
	float cr = CosDegrees(m_rollDegrees);
	float sy = SinDegrees(m_yawDegrees);
	float sp = SinDegrees(m_pitchDegrees);
	float sr = SinDegrees(m_rollDegrees);
	
	transformedMatrix.m_values[0] = cy * cp;
	transformedMatrix.m_values[1] = sy * cp;
	transformedMatrix.m_values[2] = -sp;

	transformedMatrix.m_values[4] = (-sy * cr) + (cy * sp * sr);
	transformedMatrix.m_values[5] = (cy * cr) + (sy * sp * sr);
	transformedMatrix.m_values[6] = cp * sr;

	transformedMatrix.m_values[8] = (sy * sr) + (cy * sp * cr);
	transformedMatrix.m_values[9] = (-cy * sr) + (sy * sp * cr);
	transformedMatrix.m_values[10] = cp * cr;

	return transformedMatrix;
}


//--------------------------------------------------------------------------------------------------
Vec3 EulerAngles::GetXForward() const
{
	Vec3 forwardIBasis;

	float cy = CosDegrees(m_yawDegrees);
	float cp = CosDegrees(m_pitchDegrees);
	float sy = SinDegrees(m_yawDegrees);
	float sp = SinDegrees(m_pitchDegrees);

	forwardIBasis.x = cy * cp;
	forwardIBasis.y = sy * cp;
	forwardIBasis.z = -sp;

	return forwardIBasis;
}


//--------------------------------------------------------------------------------------------------
Vec3 EulerAngles::GetXForward(Vec3& out_forwardIBasis) const
{
	float cy = CosDegrees(m_yawDegrees);
	float cp = CosDegrees(m_pitchDegrees);
	float sy = SinDegrees(m_yawDegrees);
	float sp = SinDegrees(m_pitchDegrees);

	out_forwardIBasis.x = cy * cp;
	out_forwardIBasis.y = sy * cp;
	out_forwardIBasis.z = -sp;

	return out_forwardIBasis;
}


//--------------------------------------------------------------------------------------------------
Vec3 EulerAngles::GetYLeft() const
{
	float cy = CosDegrees(m_yawDegrees);
	float cp = CosDegrees(m_pitchDegrees);
	float cr = CosDegrees(m_rollDegrees);
	float sy = SinDegrees(m_yawDegrees);
	float sp = SinDegrees(m_pitchDegrees);
	float sr = SinDegrees(m_rollDegrees);

	Vec3 leftJBasis;
	leftJBasis.x = (-sy * cr) + (cy * sp * sr);
	leftJBasis.y = (cy * cr) + (sy * sp * sr);
	leftJBasis.z = cp * sr;

	return leftJBasis;
}


//--------------------------------------------------------------------------------------------------
void EulerAngles::GetYLeft(Vec3& out_leftJBasis) const
{
	float cy = CosDegrees(m_yawDegrees);
	float cp = CosDegrees(m_pitchDegrees);
	float cr = CosDegrees(m_rollDegrees);
	float sy = SinDegrees(m_yawDegrees);
	float sp = SinDegrees(m_pitchDegrees);
	float sr = SinDegrees(m_rollDegrees);

	out_leftJBasis.x = (-sy * cr) + (cy * sp * sr);
	out_leftJBasis.y = (cy * cr) + (sy * sp * sr);
	out_leftJBasis.z = cp * sr;
}


//--------------------------------------------------------------------------------------------------
void EulerAngles::SetFromText(char const* text)
{
	Strings delimitedText = SplitStringOnDelimiter(text, ',');
	m_yawDegrees = float(atof(delimitedText[0].data()));
	m_pitchDegrees = float(atof(delimitedText[1].data()));

	if (delimitedText.size() > 2)
	{
		m_rollDegrees = float(atof(delimitedText[2].data()));
	}
}


//--------------------------------------------------------------------------------------------------
bool EulerAngles::operator==(EulerAngles const& compare) const
{
	return (m_yawDegrees == compare.m_yawDegrees && m_pitchDegrees == compare.m_pitchDegrees && m_rollDegrees == compare.m_rollDegrees);
}
