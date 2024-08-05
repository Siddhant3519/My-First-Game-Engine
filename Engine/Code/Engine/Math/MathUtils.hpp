#pragma once


//--------------------------------------------------------------------------------------------------
#include "Engine/Math/ConvexHull2D.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Vec4.hpp"



//--------------------------------------------------------------------------------------------------
struct IntVec2;
struct Rgba8;
struct Mat44;
struct Vec2;
struct Vec3;


//--------------------------------------------------------------------------------------------------
enum class BillboardType
{
	NONE = -1,

	WORLD_UP_CAMERA_FACING,
	WORLD_UP_CAMERA_OPPOSING,
	FULL_CAMERA_FACING,
	FULL_CAMERA_OPPOSING,
	
	COUNT,
};


//--------------------------------------------------------------------------------------------------
float ConvertDegreesToRadians(float degrees);
float ConvertRadiansToDegrees(float radians);
float CosDegrees(float degrees);
float SinDegrees(float degrees);
float TanDegrees(float degrees);
float Atan2Degrees(float y, float x);
float ScaleDownTo360(float degreeGreaterThan360);
float ScaleUpToZero(float degreeLesserThanZero);
float GetShortestAngularDispDegrees(float startDegrees, float endDegrees);
float GetTurnedTowardDegrees(float currentDegrees, float goalDegrees, float maxDeltaDegrees);
float GetAngleDegreesBetweenVectors2D(Vec2 const& a, Vec2 const& b);

void SwapFloatValues(float& a, float& b);

float	DotProduct2D(Vec2 const& a, Vec2 const& b);
float	DotProduct3D(Vec3 const& a, Vec3 const& b);
float	DotProduct4D(Vec4 const& a, Vec4 const& b);
float	CrossProduct2D(Vec2 const& a, Vec2 const& b);
Vec3	CrossProduct3D(Vec3 const& a, Vec3 const& b);
float	NormalizeByte(unsigned char byteValue);
unsigned char DenormalizeByte(float zeroToOne);

float			GetDistance2D(Vec2 const& positionA, Vec2 const& positionB);
float			GetDistanceSquared2D(Vec2 const& positoinA, Vec2 const& positionB);
float			GetDistance3D(Vec3 const& positionA, Vec3 const& positionB);
float			GetDistanceSquared3D(Vec3 const& positionA, Vec3 const& positionB);
float			GetDistanceXY3D(Vec3 const& positionA, Vec3 const& positionB);
float			GetDistanceXYSquared3D(Vec3 const& positionA, Vec3 const& positionB);
int				GetTaxicabDistance2D(IntVec2 const& pointA, IntVec2 const& pointB);
float			GetProjectedLength2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto);
Vec2	const	GetProjectedOnto2D(Vec2 const& vectroToProject, Vec2 const& vectorToProjectOnto);
float	const	GetProjectedLength3D(Vec3 const& vectorToProject, Vec3 const& vectorToProjectOnto);
Vec3	const	GetProjectedOnto3D(Vec3 const& vectorToProject, Vec3 const& vectorToProjectOnto);

float GetClamped(float value, float minValue, float maxValue);
float GetClampedZeroToOne(float value);
float Interpolate(float start, float end, float fractionTowardEnd);
Rgba8 Interpolate(Rgba8 startColor, Rgba8 endColor, float fractionTowardEnd);
float GetFractionWithinRange(float value, float rangeStart, float rangeEnd);
float RangeMap(float inValue, float inStart, float inEnd, float outStart, float outEnd);
float RangeMapClamped(float inValue, float inStart, float inEnd, float outStart, float outEnd);
int	  RoundDownToInt(float value);
int	  ABS_INT(int value);
float ABS_FLOAT(float value);

bool IsPointInsideDisc2D(Vec2 const& point, Vec2 const& discCenter, float discRadius);
bool IsPointInsideAABB2D(Vec2 const& point, AABB2 const& box);
bool IsPointInsideCapsule2D(Vec2 const& point, Vec2 const& boneStart, Vec2 const& boneEnd, float radius);
bool IsPointInsideOBB2D(Vec2 const& point, OBB2 const& orientedBox);
bool IsPointInsideOrientedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegree, float sectorApertureDegrees, float sectorRadius);
bool IsPointInsideDirectedSector2D(Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius);
bool IsPointInsideHexagon2D(Vec2 const& point, Vec2 const& hexCenter, float circumRadius);
bool IsPointInsideConvexHull2D(Vec2 const& point, ConvexHull2D const& convexHull);
bool IsPointInsideConvexHull2DIgnoringPlane(Vec2 const& point, ConvexHull2D const& convexHull, Plane2D const* planeToBeIgnored);
bool IsPointInsideConvexPoly2D(Vec2 const& referencePoint, ConvexPoly2D const& convexPoly);
bool IsPointBehindPlane2D(Vec2 const& point, Plane2D const& plane);

bool DoDiscsOverlap(Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB);
bool DoSpheresOverlap(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB);

Vec2 GetNearestPointOnDisc2D(Vec2 const& referencePosition, Vec2 const& discCenter, float discRadius);
Vec2 const GetNearestPointOnAABB2D(Vec2 const& referencePos, AABB2 const& box);
Vec2 const GetNearestPointOnInfiniteLine2D(Vec2 const& referencePos, Vec2 const& pointOnLine, Vec2 const& anotherPointOnLine);
Vec2 const GetNearestPointOnLineSegment2D(Vec2 const& referencePos, Vec2 const& lineSegStart, Vec2 const& lineSegEnd);
Vec2 const GetNearestPointOnCapsule2D(Vec2 const& referencePos, Vec2 const& boneStart, Vec2 const& boneEnd, float radius);
Vec2 const GetNearestPointOnOBB2D(Vec2 const& referencePos, OBB2 const& orientedBox);

bool PushDiscOutOfFixedPoint2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedPoint);
void BounceDiscOutOfFixedPoint2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2& mobileDiscVelocity, Vec2 const& fixedPoint, float elasticity = 1.0f);
bool PushDiscOutOfFixedDisc2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedDiscCenter, float fixedDiscRadius);
void BounceDiscsOffFixedDisc2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2& mobileDiscVelocity, Vec2 const& fixedDiscCenter, float fixedDiscRadius, float elasticityA = 1.0f, float elasticityB = 1.0f);
bool PushDiscsOutOfEachOther2D(Vec2& centerA, float radiusA, Vec2& centerB, float radiusB);
void BounceDiscsOffEachOther2D(Vec2& centerA, float radiusA, Vec2& velocityA, Vec2& centerB, float radiusB, Vec2& velocityB, float elasticityA = 1.0f, float elasticityB = 1.0f);
bool PushDiscOutOfFixedAABB2D(Vec2& mobileDiscCenter, float mobileDiscRadius, AABB2 const& fixedBox);

void TransformPosition2D(Vec2& posToTransform, float uniformScale, float rotationDegrees, Vec2 const& translation);
void TransformPosition2D(Vec2& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation);
void TransformPositionXY3D(Vec3& posToTransform, float scaleXY, float zRotationDegrees, Vec2 const& translationXY);
void TransformPositionXY3D(Vec3& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation);

Mat44 GetBillboardMatrix(BillboardType billboardType, Mat44 const& cameraMatrix, Vec3 const& billboardPosition, Vec2 const& billboardScale = Vec2(1.f, 1.f));
//--------------------------------------------------------------------------------------------------
// 1D Standalone Bezier Curve
// A, B, C, D are control points
float ComputeCubicBezier1D(float A, float B, float C, float D, float parametricZeroToOne);
float ComputeQuinticBezier1D(float A, float B, float C, float D, float E, float F, float parametricZeroToOne);
//--------------------------------------------------------------------------------------------------
// Easing Function
float Identity(float parametricZeroToOneT);
// Smooth Start (EaseIn)
float SmoothStart2(float parametricZeroToOneT);
float SmoothStart3(float parametricZeroToOneT);
float SmoothStart4(float parametricZeroToOneT);
float SmoothStart5(float parametricZeroToOneT);
float SmoothStart6(float parametricZeroToOneT);
// Smooth Start (EaseOut)
float SmoothStop2(float parametricZeroToOneT);
float SmoothStop3(float parametricZeroToOneT);
float SmoothStop4(float parametricZeroToOneT);
float SmoothStop5(float parametricZeroToOneT);
float SmoothStop6(float parametricZeroToOneT);
// Smooth Step
float SmoothStep3(float parametricZeroToOneT);
float SmoothStep5(float parametricZeroToOneT);
// Hesitate
float Hesitate3(float parametricZeroToOneT);
float Hesitate5(float parametricZeroToOneT);

//--------------------------------------------------------------------------------------------------
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include <vector>
void CalculateTangentSpaceVectors(std::vector<Vertex_PCUTBN>& vertsToModify, std::vector<unsigned int> const& indexes);