#include "Engine/Math/OBB2.hpp"

void OBB2::GetCornerPoints(Vec2* out_fourCornerWorldPositions) const
{
	(void)out_fourCornerWorldPositions;
}

Vec2 OBB2::GetLocalPosForWorldPos(Vec2 worldPos) const
{
	return Vec2();
}

Vec2 OBB2::GetWorldPosForLocalPos(Vec2 localPos) const
{
	return Vec2();
}

void OBB2::RotateAboutCenter(float rotationDeltaDegrees)
{
	(void)rotationDeltaDegrees;
}
