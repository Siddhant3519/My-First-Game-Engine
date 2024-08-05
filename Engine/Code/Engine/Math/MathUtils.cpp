#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Plane2D.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"


//--------------------------------------------------------------------------------------------------
#include <math.h>


//--------------------------------------------------------------------------------------------------
float ConvertDegreesToRadians(float degrees)
{
	return (degrees * PI * INVERSE_ONE_EIGHTY);
}


//--------------------------------------------------------------------------------------------------
float ConvertRadiansToDegrees(float radians)
{
	return (radians * ONE_EIGHTY * INVERSE_PI);
}


//--------------------------------------------------------------------------------------------------
float CosDegrees(float degrees)
{
	float radians		=	ConvertDegreesToRadians(degrees);
	float cosRadians	=	cosf(radians);
	
	return cosRadians;
}


//--------------------------------------------------------------------------------------------------
float SinDegrees(float degrees)
{
	float radians		=	ConvertDegreesToRadians(degrees);
	float sinRadians	=	sinf(radians);

	return sinRadians;
}


//--------------------------------------------------------------------------------------------------
float TanDegrees(float degrees)
{
	float radians		=  ConvertDegreesToRadians(degrees);
	float tanRadians	=  tanf(radians);

	return tanRadians;
}


//--------------------------------------------------------------------------------------------------
float Atan2Degrees(float y, float x)
{
	float atan2Radians = atan2f(y, x);
	float atan2Degrees = ConvertRadiansToDegrees(atan2Radians);

	return atan2Degrees;
}


//--------------------------------------------------------------------------------------------------
float ScaleDownTo360(float degreeGreaterThan360)
{
	while (degreeGreaterThan360 > 360.f)
	{
		degreeGreaterThan360 -= 360.f;
	}

	return degreeGreaterThan360;
}


//--------------------------------------------------------------------------------------------------
float ScaleUpToZero(float degreeLesserThanZero)
{
	while (degreeLesserThanZero < 0.f)
	{
		degreeLesserThanZero += 360.f;
	}

	return degreeLesserThanZero;
}


//--------------------------------------------------------------------------------------------------
float GetShortestAngularDispDegrees(float startDegrees, float endDegrees)
{
	// TODO(sid): should I just do a while check here and remove the redundant double check
	//            while (startDegrees > 360.f) {startdeg -= 360.f} and so on instead of a if check then a while check
	if (startDegrees > 360.f)
	{
		startDegrees = ScaleDownTo360(startDegrees);
	}

	if (endDegrees > 360.f)
	{
		endDegrees = ScaleDownTo360(endDegrees);
	}

	// NOTE(sid): what if we were somehow given a -ve start/end degrees smaller than -360
	if (startDegrees < 0.f)
	{
		startDegrees = ScaleUpToZero(startDegrees);
	}

	if (endDegrees < 0.f)
	{
		endDegrees = ScaleUpToZero(endDegrees);
	}

	float angleDispDiff = endDegrees - startDegrees;
	float absoluteAngleDispDiff = ABS_FLOAT(angleDispDiff);
	
	if (absoluteAngleDispDiff <= 180.f)
	{
		return angleDispDiff;
	}

	float remainderAngle = (angleDispDiff < 0.f) ? (360.f + angleDispDiff) : 
		-1.f * (360.f - angleDispDiff);

	return remainderAngle;
}


//--------------------------------------------------------------------------------------------------
float GetTurnedTowardDegrees(float currentDegrees, float goalDegrees, float maxDeltaDegrees)
{
	float shortestAngularDispDegrees = GetShortestAngularDispDegrees(currentDegrees, goalDegrees);
	float direction = (shortestAngularDispDegrees < 0.f) ? -1.f : 1.f;
	
	if (ABS_FLOAT(shortestAngularDispDegrees) > maxDeltaDegrees)
	{

		currentDegrees +=  (direction * maxDeltaDegrees);
		return currentDegrees;
	}

	return goalDegrees;
}


//--------------------------------------------------------------------------------------------------
float GetAngleDegreesBetweenVectors2D(Vec2 const& a, Vec2 const& b)
{
	// TODO(sid): Do one divide instead of 2 divides(normalization)
	Vec2 normalizedA = a.GetNormalized();
	Vec2 normalizedB = b.GetNormalized();
	
	float similarityBetweenTwoVectors = DotProduct2D(normalizedA, normalizedB);
	float similarity = GetClamped(similarityBetweenTwoVectors, -1.f, 1.f);
	float angleRadiansBetweenVectors = acosf(similarity);

	float angleDegreesBetweenVectors = ConvertRadiansToDegrees(angleRadiansBetweenVectors);

	return angleDegreesBetweenVectors;
}


//--------------------------------------------------------------------------------------------------
void SwapFloatValues(float& a, float& b)
{
	float temp = a;
	a = b;
	b = temp;
}


//--------------------------------------------------------------------------------------------------
float DotProduct2D(Vec2 const& a, Vec2 const& b)
{
	float result = (a.x * b.x) + (a.y * b.y);
	
	return result;
}


//--------------------------------------------------------------------------------------------------
float DotProduct3D(Vec3 const& a, Vec3 const& b)
{
	float result = (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
	return result;
}


//--------------------------------------------------------------------------------------------------
float DotProduct4D(Vec4 const& a, Vec4 const& b)
{
	float result = (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
	return result;
}


//--------------------------------------------------------------------------------------------------
float CrossProduct2D(Vec2 const& a, Vec2 const& b)
{
	float result = (a.x * b.y) - (a.y * b.x);
	return result;
}


//--------------------------------------------------------------------------------------------------
Vec3 CrossProduct3D(Vec3 const& a, Vec3 const& b)
{
	Vec3 result;
	result.x = (a.y * b.z) - (a.z * b.y);
	result.y = (a.z * b.x) - (a.x * b.z);
	result.z = (a.x * b.y) - (a.y * b.x);
	
	return result;
}


//--------------------------------------------------------------------------------------------------
float NormalizeByte(unsigned char byteValue)
{
	float floatValue = (float)byteValue / 255.f;
	return floatValue;
}


//--------------------------------------------------------------------------------------------------
unsigned char DenormalizeByte(float zeroToOne)
{
	float floatValue = zeroToOne * 256.f;
	float floorVal = floorf(floatValue);
	float clampedVal = GetClamped(floorVal, 0.f, 255.f);

	return (unsigned char)clampedVal;
}


//--------------------------------------------------------------------------------------------------
float GetDistance2D(Vec2 const& positionA, Vec2 const& positionB)
{
	Vec2 dispVector = positionB - positionA;

	return dispVector.GetLength();
}


//--------------------------------------------------------------------------------------------------
float GetDistanceSquared2D(Vec2 const& positionA, Vec2 const& positionB)
{
	Vec2 dispVector = positionB - positionA;

	return dispVector.GetLengthSquared();
}


//--------------------------------------------------------------------------------------------------
float GetDistance3D(Vec3 const& positionA, Vec3 const& positionB)
{
	Vec3 dispVector = positionB - positionA;

	return dispVector.GetLength();
}


//--------------------------------------------------------------------------------------------------
float GetDistanceSquared3D(Vec3 const& positionA, Vec3 const& positionB)
{
	Vec3 dispVector = positionB - positionA;

	return dispVector.GetLengthSquared();
}


//--------------------------------------------------------------------------------------------------
float GetDistanceXY3D(Vec3 const& positionA, Vec3 const& positionB)
{
	Vec3 dispVectorXY = positionB - positionA;

	return dispVectorXY.GetLengthXY();
}


//--------------------------------------------------------------------------------------------------
float GetDistanceXYSquared3D(Vec3 const& positionA, Vec3 const& positionB)
{
	Vec3 dispVectorXY = positionB - positionA;

	return dispVectorXY.GetLengthXYSquared();
}


//--------------------------------------------------------------------------------------------------
int GetTaxicabDistance2D(IntVec2 const& pointA, IntVec2 const& pointB)
{
	int horizontalDistanceTravelled = ABS_INT(pointA.x - pointB.x);
	int verticalDistanceTravelled = ABS_INT(pointA.y - pointB.y);

	int taxicabDistance = horizontalDistanceTravelled + verticalDistanceTravelled;
	
	return taxicabDistance;
}


//--------------------------------------------------------------------------------------------------
float GetProjectedLength2D(Vec2 const& vectorToProject, Vec2 const& vectorToProjectOnto)
{
	Vec2 normalizedVectorToProjectOnto = vectorToProjectOnto.GetNormalized();

	float projectedLength = DotProduct2D(vectorToProject, normalizedVectorToProjectOnto);
	
	return projectedLength;
}


//--------------------------------------------------------------------------------------------------
Vec2 const GetProjectedOnto2D(Vec2 const& vectroToProject, Vec2 const& vectorToProjectOnto)
{
	Vec2 normalizedVectorToProjectOnto = vectorToProjectOnto.GetNormalized();
	float projectedLength = DotProduct2D(vectroToProject, normalizedVectorToProjectOnto);

	Vec2 projectedVector = normalizedVectorToProjectOnto * projectedLength;
	
	return projectedVector;
}


//--------------------------------------------------------------------------------------------------
float const GetProjectedLength3D(Vec3 const& vectorToProject, Vec3 const& vectorToProjectOnto)
{
	Vec3 normalizedVectorToProjectOnto = vectorToProjectOnto.GetNormalized();
	float projectedLength = DotProduct3D(vectorToProject, normalizedVectorToProjectOnto);
	return projectedLength;
}


//--------------------------------------------------------------------------------------------------
Vec3 const GetProjectedOnto3D(Vec3 const& vectorToProject, Vec3 const& vectorToProjectOnto)
{
	Vec3 normalizedVectorToProjectOnto = vectorToProjectOnto.GetNormalized();
	float projectedLength = DotProduct3D(vectorToProject, normalizedVectorToProjectOnto);
	Vec3 projectedVector = normalizedVectorToProjectOnto * projectedLength;

	return projectedVector;
}


//--------------------------------------------------------------------------------------------------
float GetClamped(float value, float minValue, float maxValue)
{
	if (value < minValue)
	{
		return minValue;
	}
	else if (value > maxValue)
	{
		return maxValue;
	}

	return value;
}


//--------------------------------------------------------------------------------------------------
float GetClampedZeroToOne(float value)
{
	if (value > 1.f)
	{
		return 1.f;
	}
	else if (value < 0.f)
	{
		return 0.f;
	}

	return value;
}


//--------------------------------------------------------------------------------------------------
float Interpolate(float start, float end, float fractionTowardEnd)
{
	float interpolatedVal = ((1 - fractionTowardEnd) * start) + (fractionTowardEnd * end);
	
	return interpolatedVal;
}


//--------------------------------------------------------------------------------------------------
Rgba8 Interpolate(Rgba8 startColor, Rgba8 endColor, float fractionTowardEnd)
{
	Rgba8 interpolatedColor;
	interpolatedColor.r = (char)Interpolate(startColor.r, endColor.r, fractionTowardEnd);
	interpolatedColor.g = (char)Interpolate(startColor.g, endColor.g, fractionTowardEnd);
	interpolatedColor.b = (char)Interpolate(startColor.b, endColor.b, fractionTowardEnd);
	interpolatedColor.a = (char)Interpolate(startColor.a, endColor.a, fractionTowardEnd);

	return interpolatedColor;
}


//--------------------------------------------------------------------------------------------------
float GetFractionWithinRange(float value, float rangeStart, float rangeEnd)
{
	float currentRange = rangeEnd - rangeStart;

	float relativeValPos = (value - rangeStart) / currentRange;

	return relativeValPos;
}


//--------------------------------------------------------------------------------------------------
float RangeMap(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	float relativeInValPos = GetFractionWithinRange(inValue, inStart, inEnd);
	float outRange = outEnd - outStart;

	float rangeMappedVal = (relativeInValPos * outRange) + outStart;

	return rangeMappedVal;
}


//--------------------------------------------------------------------------------------------------
float RangeMapClamped(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	float fraction = GetFractionWithinRange(inValue, inStart, inEnd);
	if (fraction < 0.f)
	{
		fraction = 0.f;
	}
	if (fraction > 1.f)
	{
		fraction = 1.f;
	}

	float interPolatedValue = Interpolate(outStart, outEnd, fraction);
	return interPolatedValue;

	// if (inValue <= inStart)
	// {
	// 	return outStart;
	// }
	// else if (inValue >= inEnd)
	// {
	// 	return outEnd;
	// }
	
	/*float rangeMappedVal = RangeMap(inValue, inStart, inEnd, outStart, outEnd);*/
	/*if (rangeMappedVal >= outStart && rangeMappedVal <= outEnd)
	{
		return rangeMappedVal;
	}*/
	/*if (rangeMappedVal < outStart)
	{
		return outStart;
	}
	else if (rangeMappedVal > outEnd)
	{
		return outEnd;
	}*/

	// return rangeMappedVal;
}


//--------------------------------------------------------------------------------------------------
int RoundDownToInt(float value)
{
	return (int)floorf(value);
}


//--------------------------------------------------------------------------------------------------
int ABS_INT(int value)
{
	int absValue = (value < 0) ? (-value) : (value);

	return absValue;
}


//--------------------------------------------------------------------------------------------------
float ABS_FLOAT(float value)
{
	float absValue = (value < 0) ? (-value) : (value);

	return absValue;
}


//--------------------------------------------------------------------------------------------------
bool IsPointInsideDisc2D(Vec2 const& point, Vec2 const& discCenter, float discRadius)
{
	Vec2 displacementFromCenterToPoint = point - discCenter;
	float distanceFromCenterToPointSquared = displacementFromCenterToPoint.GetLengthSquared();

	float discRadiusSquared = discRadius * discRadius;

	if (distanceFromCenterToPointSquared > discRadiusSquared)
	{
		return false;
	}

	return true;
}


//--------------------------------------------------------------------------------------------------
bool IsPointInsideAABB2D(Vec2 const& point, AABB2 const& box)
{
	Vec2 const& boxMins = box.m_mins;
	Vec2 const& boxMaxs = box.m_maxs;

	// if (boxMins.x > point.x || boxMaxs.x < point.x || boxMins.y > point.y || boxMaxs.y < point.y)
	// {
	// 	return false;
	// }
	// 
	// return true;

	return !(boxMins.x > point.x || boxMaxs.x < point.x || boxMins.y > point.y || boxMaxs.y < point.y);
}


//--------------------------------------------------------------------------------------------------
bool IsPointInsideCapsule2D(Vec2 const& point, Vec2 const& boneStart, Vec2 const& boneEnd, float radius)
{
	Vec2 boneStartToEndDisp = boneEnd - boneStart;
	float radiusSquared = radius * radius;

	// Voronoi region 1: nearest point is boneStart
	Vec2 boneStartToPointDisp = point - boneStart;
	if (DotProduct2D(boneStartToEndDisp, boneStartToPointDisp) < 0)
	{
		float boneStartToPointDistanceSquared = boneStartToPointDisp.GetLengthSquared();
		return boneStartToPointDistanceSquared < radiusSquared;
	}

	// Voronoi region 2: nearest point is boneEnd
	Vec2 boneEndToPointDisp = point - boneEnd;
	if (DotProduct2D(boneStartToEndDisp, boneEndToPointDisp) > 0)
	{
		float boneEndToPointDistanceSquared = boneEndToPointDisp.GetLengthSquared();
		return boneEndToPointDistanceSquared < radiusSquared;
	}

	// Voronoi region 3: nearest point in on the bone
	Vec2 boneStartToEndDispRotated90Degrees = boneStartToEndDisp.GetRotated90Degrees();
	float startToPointProjectionLengthOntoStartToEndDispRotated90Degrees = GetProjectedLength2D(boneStartToPointDisp, boneStartToEndDispRotated90Degrees);
	float projectedLengthSquared = startToPointProjectionLengthOntoStartToEndDispRotated90Degrees * startToPointProjectionLengthOntoStartToEndDispRotated90Degrees;
	return projectedLengthSquared < radiusSquared;
}


//--------------------------------------------------------------------------------------------------
bool IsPointInsideOBB2D(Vec2 const& point, OBB2 const& orientedBox)
{
	Vec2 orientedBoxCenter = orientedBox.m_center;
	Vec2 orientedBoxiBasisNormal = orientedBox.m_iBasisNormal;
	Vec2 orientedBoxjBasisNormal = orientedBoxiBasisNormal.GetRotated90Degrees();
	float orientedBoxHalfWidth = orientedBox.m_halfDimensions.x;
	float orientedBoxHalfHeight = orientedBox.m_halfDimensions.y;

	Vec2 orientedBoxCenterToPointDisp = point - orientedBoxCenter;

	float projectionOfOrientedBoxCenterToPointDispOntoiBasisNormal = DotProduct2D(orientedBoxCenterToPointDisp, orientedBoxiBasisNormal);
	float projectionOfOrientedBoxCetnerToPointDispOntojBasisNormal = DotProduct2D(orientedBoxCenterToPointDisp, orientedBoxjBasisNormal);

	if (ABS_FLOAT(projectionOfOrientedBoxCenterToPointDispOntoiBasisNormal) > orientedBoxHalfWidth)
	{
		return false;
	}

	if (ABS_FLOAT(projectionOfOrientedBoxCetnerToPointDispOntojBasisNormal) > orientedBoxHalfHeight)
	{
		return false;
	}
	
	
	return true;
}


//--------------------------------------------------------------------------------------------------
bool IsPointInsideOrientedSector2D(Vec2 const& point, Vec2 const& sectorTip, float sectorForwardDegree, float sectorApertureDegrees, float sectorRadius)
{
	/*Vec2 displacementFromPointToSectorTip = sectorTip - point;
	float distanceFromPointToSectorTipSquared = displacementFromPointToSectorTip.GetLengthSquared();

	if (distanceFromPointToSectorTipSquared > (sectorRadius * sectorRadius))
	{
		return false;
	}

	float orientationDegreesOfDisplacementFromPointToSectorTip = displacementFromPointToSectorTip.GetOrientationDegrees();
	float halfSectorApertureDegrees = sectorApertureDegrees * 0.5f;

	float sectorMinDegrees = sectorForwardDegree - halfSectorApertureDegrees;
	float sectorMaxDegrees = sectorForwardDegree + halfSectorApertureDegrees;

	
	if (orientationDegreesOfDisplacementFromPointToSectorTip >= sectorMinDegrees &&
		orientationDegreesOfDisplacementFromPointToSectorTip <= sectorMaxDegrees)
	{
		return true;
	}


	return false;*/

	Vec2 sectorForwardNormal = Vec2::MakeFromPolarDegrees(sectorForwardDegree);

	bool isPointInsideDirectedSector = IsPointInsideDirectedSector2D(point, sectorTip, sectorForwardNormal, sectorApertureDegrees, sectorRadius);

	return isPointInsideDirectedSector;
}


//--------------------------------------------------------------------------------------------------
bool IsPointInsideDirectedSector2D(Vec2 const& point, Vec2 const& sectorTip, Vec2 const& sectorForwardNormal, float sectorApertureDegrees, float sectorRadius)
{
	Vec2 displacementFromSectorTipToPoint = point - sectorTip;
	float distanceFromSectorTipToPointSquared = displacementFromSectorTipToPoint.GetLengthSquared();

	if (distanceFromSectorTipToPointSquared > (sectorRadius * sectorRadius))
	{
		return false;
	}

	float halfSectorApertureDegrees = sectorApertureDegrees * 0.5f;
	float angleBetweenSectorForwardNormalAndDisplacementFromSectorTipToPoint = GetAngleDegreesBetweenVectors2D(sectorForwardNormal, displacementFromSectorTipToPoint);
	
	if (angleBetweenSectorForwardNormalAndDisplacementFromSectorTipToPoint <= halfSectorApertureDegrees)
	{
		return true;
	}
	
	return false;
}


//--------------------------------------------------------------------------------------------------
bool IsPointInsideHexagon2D(Vec2 const& point, Vec2 const& hexCenter, float circumRadius)
{
	constexpr int	NUM_OF_HEX_VERTS = 6;
	constexpr float	DEGREES_PER_SIDE = 60.f;

	float currentOrientation	=	0.f;
	Vec2 currentVertexStart		=	hexCenter + Vec2::MakeFromPolarDegrees(currentOrientation, circumRadius);
	currentOrientation	+=	DEGREES_PER_SIDE;
	for (int vertIndex = 0; vertIndex < NUM_OF_HEX_VERTS; ++vertIndex)
	{
		Vec2 nextVertexStart	=	hexCenter + Vec2::MakeFromPolarDegrees(currentOrientation, circumRadius);
		
		Vec2  dispFromPointToCurrentVert	=	currentVertexStart - point;
		Vec2  dispFromCurrentVertToNextVert	=	nextVertexStart - currentVertexStart;
		float normalLength					=	CrossProduct2D(dispFromPointToCurrentVert, dispFromCurrentVertToNextVert);
		
		if (normalLength < 0)
		{
			return false;
		}

		currentOrientation		+=	DEGREES_PER_SIDE;
		currentVertexStart		=	nextVertexStart;
	}

	return true;
}


//--------------------------------------------------------------------------------------------------
bool IsPointInsideConvexHull2D(Vec2 const& point, ConvexHull2D const& convexHull)
{
	std::vector<Plane2D> const& planeList = convexHull.m_boundingPlanes;
	for (size_t planeIndex = 0; planeIndex < planeList.size(); ++planeIndex)
	{
		Plane2D const& currentPlane = planeList[planeIndex];
		if (!IsPointBehindPlane2D(point, currentPlane))
		{
			return false;
		}
	}
	return true;
}


//--------------------------------------------------------------------------------------------------
bool IsPointInsideConvexHull2DIgnoringPlane(Vec2 const& point, ConvexHull2D const& convexHull, Plane2D const* planeToBeIgnored)
{
	std::vector<Plane2D> const& planeList = convexHull.m_boundingPlanes;
	for (size_t planeIndex = 0; planeIndex < planeList.size(); ++planeIndex)
	{
		Plane2D const& currentPlane = planeList[planeIndex];
		if (&currentPlane == planeToBeIgnored)
		{
			continue;
		}
		if (!IsPointBehindPlane2D(point, currentPlane))
		{
			return false;
		}
	}
	return true;
}


//--------------------------------------------------------------------------------------------------
bool IsPointInsideConvexPoly2D(Vec2 const& referencePoint, ConvexPoly2D const& convexPoly)
{
	std::vector<Vec2> const& pointList	=	convexPoly.GetPoints2D();
	Vec2 const& firstPoint				=	pointList[0];
	size_t const& numOfPoints			=	pointList.size();
	for (size_t pointIndex = 0; pointIndex < numOfPoints - 1; ++pointIndex)
	{
		Vec2 const& currentPoint		=	pointList[pointIndex];
		Vec2 const& nextPoint			=	pointList[pointIndex + 1];
		Vec2 dispFromCurrentToNext		=	nextPoint - currentPoint;
		Vec2 dispFromCurrentToRefPoint	=	referencePoint - currentPoint;
		float planeArea					=	CrossProduct2D(dispFromCurrentToRefPoint, dispFromCurrentToNext);
		if (planeArea >= 0)
		{
			return false;
		}
	}

	Vec2 lastPoint					=	pointList[numOfPoints - 1];
	Vec2 dispFromLastToFirst		=	firstPoint - lastPoint;
	Vec2 dispFromLastToRefPoint		=	referencePoint - lastPoint;
	float planeArea					=	CrossProduct2D(dispFromLastToRefPoint, dispFromLastToFirst);
	return (planeArea < 0);
}


//--------------------------------------------------------------------------------------------------
bool IsPointBehindPlane2D(Vec2 const& point, Plane2D const& plane)
{
	Vec2 const& planeNormal					=	plane.m_normal;
	float refPointDistFromOrigin			=	DotProduct2D(point, planeNormal);
	return refPointDistFromOrigin < plane.m_distFromOrigin;
	// Vec2 closestPointOnPlane				=	plane.m_distFromOrigin * planeNormal;
	// Vec2 dispFromClosestPointToRefPoint		=	point - closestPointOnPlane;
	// float planeArea							=	CrossProduct2D(dispFromClosestPointToRefPoint, planeNormal);
	// return (planeArea <  0);
}


//--------------------------------------------------------------------------------------------------
bool DoDiscsOverlap(Vec2 const& centerA, float radiusA, Vec2 const& centerB, float radiusB)
{
	Vec2 distanceBetweenCentersVec = centerB - centerA;
	float distanceBetweenCeters = distanceBetweenCentersVec.GetLength();
	float sumOfRadiis = radiusB + radiusA;

	if (distanceBetweenCeters < sumOfRadiis)
	{
		return true;
	}

	return false;
}


//--------------------------------------------------------------------------------------------------
bool DoSpheresOverlap(Vec3 const& centerA, float radiusA, Vec3 const& centerB, float radiusB)
{
	Vec3 distanceBetweenCentersVec = centerB - centerA;
	float distanceBetweenCeters = distanceBetweenCentersVec.GetLength();
	float sumOfRadiis = radiusB + radiusA;

	if (distanceBetweenCeters < sumOfRadiis)
	{
		return true;
	}

	return false;
}


//--------------------------------------------------------------------------------------------------
Vec2 GetNearestPointOnDisc2D(Vec2 const& referencePosition, Vec2 const& discCenter, float discRadius)
{
	if (IsPointInsideDisc2D(referencePosition, discCenter, discRadius))
	{
		return referencePosition;
	}

	Vec2 displacementFromCenterToReferencePoint = referencePosition - discCenter;
	displacementFromCenterToReferencePoint.ClampLength(discRadius);

	Vec2 nearestPointOnDisc = discCenter + displacementFromCenterToReferencePoint;
	return nearestPointOnDisc;
}


//--------------------------------------------------------------------------------------------------
Vec2 const GetNearestPointOnAABB2D(Vec2 const& referencePos, AABB2 const& box)
{
	Vec2 nearestPoint;
	Vec2 const& boxMins = box.m_mins;
	Vec2 const& boxMaxs = box.m_maxs;
	nearestPoint.x = GetClamped(referencePos.x, boxMins.x, boxMaxs.x);
	nearestPoint.y = GetClamped(referencePos.y, boxMins.y, boxMaxs.y);
	return nearestPoint;
}


//--------------------------------------------------------------------------------------------------
Vec2 const GetNearestPointOnInfiniteLine2D(Vec2 const& referencePos, Vec2 const& pointOnLine, Vec2 const& anotherPointOnLine)
{
	Vec2 nearestPoint;
	Vec2 dispFromLineSegStartToReferencePos = pointOnLine - referencePos;
	Vec2 dispFromLineSegStartToEnd = anotherPointOnLine - pointOnLine;
	Vec2 directionFromLineSegStartToEnd = dispFromLineSegStartToEnd.GetNormalized();
	Vec2 directionFromLineSegStartToReferencePos = dispFromLineSegStartToReferencePos.GetNormalized();
	float projection = DotProduct2D(directionFromLineSegStartToEnd, dispFromLineSegStartToReferencePos);
	Vec2 scaledDirectionFromLineSegStartToReferencePos = directionFromLineSegStartToReferencePos * projection;
	nearestPoint = pointOnLine + scaledDirectionFromLineSegStartToReferencePos;

	return nearestPoint;
}


//--------------------------------------------------------------------------------------------------
Vec2 const GetNearestPointOnLineSegment2D(Vec2 const& referencePos, Vec2 const& lineSegStart, Vec2 const& lineSegEnd)
{
	Vec2 nearestPoint;
	Vec2 dispFromLineSegStartToEnd = lineSegEnd - lineSegStart;

	// Voronoi region 1: Nearest point is lineSegStart
	Vec2 dispFromLineSegStartToReferencePos = referencePos - lineSegStart;
	if (DotProduct2D(dispFromLineSegStartToEnd, dispFromLineSegStartToReferencePos) < 0)
	{
		nearestPoint = lineSegStart;
		return nearestPoint;
	}

	// Voronoi region 2: Nearest point is lineSegEnd
	Vec2 dispFromLineSegEndToReferencePos = referencePos - lineSegEnd;
	if (DotProduct2D(dispFromLineSegStartToEnd, dispFromLineSegEndToReferencePos) > 0)
	{
		nearestPoint = lineSegEnd;
		return nearestPoint;
	}
	
	// Voronoi Region 3: Nearest point is in on the lineSeg
	Vec2 directionFromLineSegStartToEnd = dispFromLineSegStartToEnd.GetNormalized();
	// Vec2 directionFromLineSegStartToReferencePos = dispFromLineSegStartToReferencePos.GetNormalized();
	Vec2 projectionOntoLineSeg = GetProjectedOnto2D(dispFromLineSegStartToReferencePos, directionFromLineSegStartToEnd);
	nearestPoint = lineSegStart + projectionOntoLineSeg;
	
	// float projection = DotProduct2D(directionFromLineSegStartToEnd, dispFromLineSegStartToReferencePos);
	// Vec2 scaledDirectionFromLineSegStartToReferencePos = directionFromLineSegStartToReferencePos * projection;
	// nearestPoint = lineSegStart + scaledDirectionFromLineSegStartToReferencePos;
	
	return nearestPoint;
}


//--------------------------------------------------------------------------------------------------
Vec2 const GetNearestPointOnCapsule2D(Vec2 const& referencePos, Vec2 const& boneStart, Vec2 const& boneEnd, float radius)
{
	if (IsPointInsideCapsule2D(referencePos, boneStart, boneEnd, radius))
	{
		return referencePos;
	}

	// Get nearest point on the line segment from boneStart to boneEnd
	Vec2 nearestPointOnLineSegFromBoneStartToBoneEnd = GetNearestPointOnLineSegment2D(referencePos, boneStart, boneEnd);
	// Scale the vector from the nearest point on line segment to the reference vector by the radius, and add it to the nearest point on the line segment, to get the nearest point the capsule
	Vec2 dispFromNearestPointOnLineSegToReferencePos = referencePos - nearestPointOnLineSegFromBoneStartToBoneEnd;
	Vec2 directionFromNearestPointOnLineSegToRefPos = dispFromNearestPointOnLineSegToReferencePos.GetNormalized();
	Vec2 scaledDirectionFromNearestPointOnLineSegToRefPos = directionFromNearestPointOnLineSegToRefPos * radius;
	Vec2 nearestPoint = nearestPointOnLineSegFromBoneStartToBoneEnd + scaledDirectionFromNearestPointOnLineSegToRefPos;
	return nearestPoint;
}


//--------------------------------------------------------------------------------------------------
Vec2 const GetNearestPointOnOBB2D(Vec2 const& referencePos, OBB2 const& orientedBox)
{
	Vec2 nearestPoint;

	// Get the disp from the obb center to reference pos
	Vec2 dispFromOBBCenterToRefPos = referencePos - orientedBox.m_center;
	// Get the jBasis
	Vec2 jBasisNormal = orientedBox.m_iBasisNormal.GetRotated90Degrees();
	
	// Get the projection of the disp from center to refPos on the i and j basis to convert into local space
	float projectedLenOfDispRefPosOntoiBasis = DotProduct2D(dispFromOBBCenterToRefPos, orientedBox.m_iBasisNormal);
	float projectedLenOfDispRefPosOntojBasis = DotProduct2D(dispFromOBBCenterToRefPos, jBasisNormal);
	// Vec2 projectionOfDispRefOntoiBasis = orientedBox.m_iBasisNormal * projectedLenOfDispRefPosOntoiBasis;
	// Vec2 projectionOfDispRefOntojBasis = jBasisNormal * projectedLenOfDispRefPosOntojBasis;
	// clamp the projection to be within (-halfLength, +halfLength)
	float clampedProjectedLenAlongiBasis = GetClamped(projectedLenOfDispRefPosOntoiBasis, -orientedBox.m_halfDimensions.x, orientedBox.m_halfDimensions.x);
	float clampedProjectedLenAlongjBasis = GetClamped(projectedLenOfDispRefPosOntojBasis, -orientedBox.m_halfDimensions.y, orientedBox.m_halfDimensions.y);
	Vec2 scalediBasisByClampedProjectedLen = orientedBox.m_iBasisNormal * clampedProjectedLenAlongiBasis;
	Vec2 scaledjBasisByClampedProjectedLen = jBasisNormal * clampedProjectedLenAlongjBasis;
	// add the resultant scaled iBasis and jBasis to the center to get the nearest point on the OBB2
	nearestPoint = orientedBox.m_center + scalediBasisByClampedProjectedLen + scaledjBasisByClampedProjectedLen;
	return nearestPoint;
}


//--------------------------------------------------------------------------------------------------
bool PushDiscOutOfFixedPoint2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedPoint)
{
	if (!IsPointInsideDisc2D(fixedPoint, mobileDiscCenter, mobileDiscRadius))
	{
		return false;
	}

	Vec2 displacementFromFixedPointToCenter = mobileDiscCenter - fixedPoint;
	displacementFromFixedPointToCenter.SetLength(mobileDiscRadius);

	mobileDiscCenter = fixedPoint + displacementFromFixedPointToCenter;

	return true;
}


//--------------------------------------------------------------------------------------------------
void BounceDiscOutOfFixedPoint2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2& mobileDiscVelocity, Vec2 const& fixedPoint, float elasticity)
{
	if (!PushDiscOutOfFixedPoint2D(mobileDiscCenter, mobileDiscRadius, fixedPoint))
	{
		return;
	}

	Vec2 surfaceNormal = (mobileDiscCenter - fixedPoint).GetNormalized();
	float projectedLengthAlongNormal = DotProduct2D(mobileDiscVelocity, surfaceNormal);
	Vec2 projectionAlongNormal = surfaceNormal * projectedLengthAlongNormal;
	Vec2 projectionAlongTangent = mobileDiscVelocity - projectionAlongNormal;
	mobileDiscVelocity = projectionAlongTangent + (-projectionAlongNormal * elasticity);
}


//--------------------------------------------------------------------------------------------------
bool PushDiscOutOfFixedDisc2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2 const& fixedDiscCenter, float fixedDiscRadius)
{
	if (!DoDiscsOverlap(mobileDiscCenter, mobileDiscRadius, fixedDiscCenter, fixedDiscRadius))
	{
		return false;
	}

	Vec2 displacementFromMobileToFixedDiscCetner = mobileDiscCenter - fixedDiscCenter;
	displacementFromMobileToFixedDiscCetner.SetLength(mobileDiscRadius + fixedDiscRadius);

	mobileDiscCenter = fixedDiscCenter + displacementFromMobileToFixedDiscCetner;

	return true;
}


//--------------------------------------------------------------------------------------------------
void BounceDiscsOffFixedDisc2D(Vec2& mobileDiscCenter, float mobileDiscRadius, Vec2& mobileDiscVelocity, Vec2 const& fixedDiscCenter, float fixedDiscRadius, float elasticityA, float elasticityB)
{
	if (PushDiscOutOfFixedDisc2D(mobileDiscCenter, mobileDiscRadius, fixedDiscCenter, fixedDiscRadius))
	{
		float elasticity = elasticityA * elasticityB;

		Vec2 normal = (mobileDiscCenter - fixedDiscCenter).GetNormalized();
		float projectedLengthAlongNormal = DotProduct2D(mobileDiscVelocity, normal);
		Vec2 velocityN = normal * projectedLengthAlongNormal;
		Vec2 velocityT = mobileDiscVelocity - velocityN;
		mobileDiscVelocity = velocityT + (-velocityN * elasticity);
	}
}


//--------------------------------------------------------------------------------------------------
bool PushDiscsOutOfEachOther2D(Vec2& centerA, float radiusA, Vec2& centerB, float radiusB)
{
	Vec2 displacementBetweenDiscCentersAToB = centerB - centerA;
	float distBetweenDiscCentersSquared		= displacementBetweenDiscCentersAToB.GetLengthSquared(); 
	float sumOfDiscsRadii					= radiusB + radiusA;

	if (distBetweenDiscCentersSquared > (sumOfDiscsRadii * sumOfDiscsRadii))
	{
		return false;
	}
	
	float distanceBetweenDiscCenters = displacementBetweenDiscCentersAToB.GetLength();
	Vec2 displacementBetweenDiscCentersBToA = centerA - centerB;

	float commonDistanceSharedBetweenDiscs = sumOfDiscsRadii - distanceBetweenDiscCenters;
	float halfCommonDistanceBetweenDiscs = commonDistanceSharedBetweenDiscs * 0.5f;
	float lengthToBeDisplaced = distanceBetweenDiscCenters + halfCommonDistanceBetweenDiscs;

	displacementBetweenDiscCentersAToB.SetLength(lengthToBeDisplaced);
	displacementBetweenDiscCentersBToA.SetLength(lengthToBeDisplaced);

	Vec2 originalCenterA = centerA;

	centerA = centerB + displacementBetweenDiscCentersBToA;
	centerB = originalCenterA + displacementBetweenDiscCentersAToB;

	return true;
}


//--------------------------------------------------------------------------------------------------
void BounceDiscsOffEachOther2D(Vec2& centerA, float radiusA, Vec2& velocityA, Vec2& centerB, float radiusB, Vec2& velocityB, float elasticityA, float elasticityB)
{
	if (PushDiscsOutOfEachOther2D(centerA, radiusA, centerB, radiusB))
	{
		Vec2 normal = centerB - centerA;
		
		float elasticity = elasticityA * elasticityB;

		Vec2 velocityAN = GetProjectedOnto2D(velocityA, normal) * elasticity;
		Vec2 velocityAT = velocityA - velocityAN;

		Vec2 velocityBN = GetProjectedOnto2D(velocityB, normal) * elasticity;
		Vec2 velocityBT = velocityB - velocityBN;

		// Divergence check
		if (DotProduct2D(velocityB, normal) - DotProduct2D(velocityA, normal) > 0.f)
		{
			velocityA = velocityAT + velocityAN;
			velocityB = velocityBT + velocityBN;
			return;
		}

		velocityA = velocityAT + velocityBN;
		velocityB = velocityBT + velocityAN;
	}
}


//--------------------------------------------------------------------------------------------------
bool PushDiscOutOfFixedAABB2D(Vec2& mobileDiscCenter, float mobileDiscRadius, AABB2 const& fixedBox)
{
	if (fixedBox.IsPointInside(mobileDiscCenter))
	{
		return false;
	}

	Vec2 nearestPointOnAABBToDiscCenter = fixedBox.GetNearestPoint(mobileDiscCenter);

	if (PushDiscOutOfFixedPoint2D(mobileDiscCenter, mobileDiscRadius, nearestPointOnAABBToDiscCenter))
	{
		return true;
	}

	return false;
}


//--------------------------------------------------------------------------------------------------
void TransformPosition2D(Vec2& posToTransform, float uniformScale, float rotationDegrees, Vec2 const& translation)
{
	posToTransform *= uniformScale;
	posToTransform.RotateDegrees(rotationDegrees);
	posToTransform += translation;
}


//--------------------------------------------------------------------------------------------------
void TransformPosition2D(Vec2& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation)
{
	posToTransform =  translation + (posToTransform.x * iBasis) + (posToTransform.y * jBasis);
}


//--------------------------------------------------------------------------------------------------
void TransformPositionXY3D(Vec3& posToTransform, float scaleXY, float zRotationDegrees, Vec2 const& translationXY)
{
	posToTransform.x *= scaleXY;
	posToTransform.y *= scaleXY;
	posToTransform.RotateAboutZDegrees(zRotationDegrees);
	posToTransform.x += translationXY.x;
	posToTransform.y += translationXY.y;
}


//--------------------------------------------------------------------------------------------------
void TransformPositionXY3D(Vec3& posToTransform, Vec2 const& iBasis, Vec2 const& jBasis, Vec2 const& translation)
{
	Vec2 transformedPositionXY = translation + (posToTransform.x * iBasis) + (posToTransform.y * jBasis);
	posToTransform.x = transformedPositionXY.x;
	posToTransform.y = transformedPositionXY.y;
}


//--------------------------------------------------------------------------------------------------
Mat44 GetBillboardMatrix(BillboardType billboardType, Mat44 const& cameraMatrix, Vec3 const& billboardPosition, Vec2 const& billboardScale)
{
	Vec3 cameraForward = cameraMatrix.GetIBasis3D();
	Vec3 cameraLeft = cameraMatrix.GetJBasis3D();
	Vec3 cameraUp = cameraMatrix.GetKBasis3D();
	Vec3 cameraPos = cameraMatrix.GetTranslation3D();

	Mat44 billboardMatrix;

	if (BillboardType::FULL_CAMERA_FACING == billboardType)
	{
		Vec3 iForward = (cameraPos - billboardPosition).GetNormalized();
		// Vec3 jLeft;
		// Vec3 kUp;
		// if (ABS_FLOAT(DotProduct3D(iForward, Vec3::Z_AXIS)) < 1.f)
		// {
		// 	jLeft = CrossProduct3D(Vec3::Z_AXIS, iForward);
		// 	jLeft.Normalize();
		// 	kUp = CrossProduct3D(iForward, jLeft);
		// 	kUp.Normalize();
		// }
		// else
		// {
		// 	kUp = CrossProduct3D(iForward, Vec3::Y_AXIS);
		// 	kUp.Normalize();
		// 	jLeft = CrossProduct3D(kUp, iForward);
		// 	jLeft.Normalize();
		// }

		Vec3 jLeft = CrossProduct3D(Vec3::Z_AXIS, iForward);
		if (jLeft == Vec3::WORLD_ORIGIN)
		{
			jLeft = Vec3::Y_AXIS;
		}
		else
		{
			jLeft.Normalize();
		}
		Vec3 kUp = CrossProduct3D(iForward, jLeft);

		billboardMatrix.SetIJKT3D(iForward, jLeft * billboardScale.x, kUp * billboardScale.y, billboardPosition);
	}
	else if (BillboardType::FULL_CAMERA_OPPOSING == billboardType)
	{
		billboardMatrix.SetIJKT3D(-cameraForward, -cameraLeft * billboardScale.x, cameraUp * billboardScale.y, billboardPosition);
	}
	else if (BillboardType::WORLD_UP_CAMERA_FACING == billboardType)
	{
		Vec3 kUp = Vec3::Z_AXIS;
		Vec3 iForward = cameraPos - billboardPosition;
		iForward.z = 0.f;
		iForward.Normalize();
		// Vec3 jLeft;
		// jLeft.x = -iForward.y;
		// jLeft.y = iForward.x;
		Vec3 jLeft = CrossProduct3D(kUp, iForward);
		jLeft.Normalize();

		billboardMatrix.SetIJKT3D(iForward, jLeft * billboardScale.x, kUp * billboardScale.y, billboardPosition);
	}
	else if (BillboardType::WORLD_UP_CAMERA_OPPOSING == billboardType)
	{
		Vec3 kUp = Vec3::Z_AXIS;
		Vec3 iForward = -cameraForward;
		iForward.z = 0.f;
		iForward.Normalize();
		Vec3 jLeft = CrossProduct3D(kUp, iForward);
		jLeft.Normalize();

		billboardMatrix.SetIJKT3D(iForward, jLeft * billboardScale.x, kUp * billboardScale.y, billboardPosition);
	}

	return billboardMatrix;
}


//--------------------------------------------------------------------------------------------------
float ComputeCubicBezier1D(float A, float B, float C, float D, float parametricZeroToOne)
{
	// A, B, C, D are control points
	float AB = Interpolate(A, B, parametricZeroToOne);
	float BC = Interpolate(B, C, parametricZeroToOne);
	float CD = Interpolate(C, D, parametricZeroToOne);

	float ABBC = Interpolate(AB, BC, parametricZeroToOne);
	float BCCD = Interpolate(BC, CD, parametricZeroToOne);

	float ABBCBCCD = Interpolate(ABBC, BCCD, parametricZeroToOne);

	return ABBCBCCD;
}


//--------------------------------------------------------------------------------------------------
float ComputeQuinticBezier1D(float A, float B, float C, float D, float E, float F, float parametricZeroToOne)
{
	// Lerps
	float AB = Interpolate(A, B, parametricZeroToOne);
	float BC = Interpolate(B, C, parametricZeroToOne);
	float CD = Interpolate(C, D, parametricZeroToOne);
	float DE = Interpolate(D, E, parametricZeroToOne);
	float EF = Interpolate(E, F, parametricZeroToOne);

	// Lerp of Lerps
	float ABC = Interpolate(AB, BC, parametricZeroToOne);
	float BCD = Interpolate(BC, CD, parametricZeroToOne);
	float CDE = Interpolate(CD, DE, parametricZeroToOne);
	float DEF = Interpolate(DE, EF, parametricZeroToOne);

	// Lerp of Lerp of Lerps
	float ABCD = Interpolate(ABC, BCD, parametricZeroToOne);
	float BCDE = Interpolate(BCD, CDE, parametricZeroToOne);
	float CDEF = Interpolate(CDE, DEF, parametricZeroToOne);

	// Lerp of Lerp of Lerp of Lerps
	float ABCDE = Interpolate(ABCD, BCDE, parametricZeroToOne);
	float BCDEF = Interpolate(BCDE, CDEF, parametricZeroToOne);

	// Lerp of Lerp of Lerp of Lerp of Lerps
	float ABCDEF = Interpolate(ABCDE, BCDEF, parametricZeroToOne);

	return ABCDEF;
}


//--------------------------------------------------------------------------------------------------
float ComputeQuinticBezier1D(float A, float B, float C, float D, float parametricZeroToOne)
{
	// A, B, C, D are control points
	float AB = Interpolate(A, B, parametricZeroToOne);
	float BC = Interpolate(B, C, parametricZeroToOne);
	float CD = Interpolate(C, D, parametricZeroToOne);

	float ABBC = Interpolate(AB, BC, parametricZeroToOne);
	float BCCD = Interpolate(BC, CD, parametricZeroToOne);

	float ABBCBCCD = Interpolate(ABBC, BCCD, parametricZeroToOne);

	return ABBCBCCD;
}


//--------------------------------------------------------------------------------------------------
// Easing Functions
float Identity(float parametricZeroToOneT)
{
	return parametricZeroToOneT;
}


//--------------------------------------------------------------------------------------------------
float SmoothStart2(float parametricZeroToOneT)
{
	float easingVal = parametricZeroToOneT * parametricZeroToOneT;
	return easingVal;
}


//--------------------------------------------------------------------------------------------------
float SmoothStart3(float parametricZeroToOneT)
{
	float easingVal = parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT;
	return easingVal;
}


//--------------------------------------------------------------------------------------------------
float SmoothStart4(float parametricZeroToOneT)
{
	float easingVal = parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT;
	return easingVal;
}


//--------------------------------------------------------------------------------------------------
float SmoothStart5(float parametricZeroToOneT)
{
	float easingVal = parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT;
	return easingVal;
}


//--------------------------------------------------------------------------------------------------
float SmoothStart6(float parametricZeroToOneT)
{
	float easingVal = parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT;
	return easingVal;
}


//--------------------------------------------------------------------------------------------------
float SmoothStop2(float parametricZeroToOneT)
{
	float mirroredParametricZeroToOne = 1.f - parametricZeroToOneT;
	mirroredParametricZeroToOne *= mirroredParametricZeroToOne;
	float flippedMirrored = 1.f - mirroredParametricZeroToOne;
	return flippedMirrored;
}


//--------------------------------------------------------------------------------------------------
float SmoothStop3(float parametricZeroToOneT)
{
	float mirroredParametricZeroToOne = 1.f - parametricZeroToOneT;
	mirroredParametricZeroToOne *= mirroredParametricZeroToOne * mirroredParametricZeroToOne;
	float flippedMirrored = 1.f - mirroredParametricZeroToOne;
	return flippedMirrored;
}


//--------------------------------------------------------------------------------------------------
float SmoothStop4(float parametricZeroToOneT)
{
	float mirroredParametricZeroToOne = 1.f - parametricZeroToOneT;
	mirroredParametricZeroToOne *= mirroredParametricZeroToOne * mirroredParametricZeroToOne * mirroredParametricZeroToOne;
	float flippedMirrored = 1.f - mirroredParametricZeroToOne;
	return flippedMirrored;
}


//--------------------------------------------------------------------------------------------------
float SmoothStop5(float parametricZeroToOneT)
{
	float mirroredParametricZeroToOne = 1.f - parametricZeroToOneT;
	mirroredParametricZeroToOne *= mirroredParametricZeroToOne * mirroredParametricZeroToOne * mirroredParametricZeroToOne * mirroredParametricZeroToOne;
	float flippedMirrored = 1.f - mirroredParametricZeroToOne;
	return flippedMirrored;
}


//--------------------------------------------------------------------------------------------------
float SmoothStop6(float parametricZeroToOneT)
{
	float mirroredParametricZeroToOne = 1.f - parametricZeroToOneT;
	mirroredParametricZeroToOne *= mirroredParametricZeroToOne * mirroredParametricZeroToOne * mirroredParametricZeroToOne * mirroredParametricZeroToOne * mirroredParametricZeroToOne;
	float flippedMirrored = 1.f - mirroredParametricZeroToOne;
	return flippedMirrored;
}


//--------------------------------------------------------------------------------------------------
float SmoothStep3(float parametricZeroToOneT)
{
	// Interpolate smooth start 3 and smooth stop 3
	return ((3.f * parametricZeroToOneT * parametricZeroToOneT) - (2.f * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT));
}


//--------------------------------------------------------------------------------------------------
float SmoothStep5(float parametricZeroToOneT)
{
	// Interpolate smooth start 5 and smooth stop 5
	// return ((5.f * parametricZeroToOneT * parametricZeroToOneT) - (10.f * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT) + (10.f * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT)
	// 	- (4.f * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT));
	return 6.0f * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT - 15.0f * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT + 10.0f * parametricZeroToOneT * parametricZeroToOneT * parametricZeroToOneT;
}


//--------------------------------------------------------------------------------------------------
float Hesitate3(float parametricZeroToOneT)
{
	return ComputeCubicBezier1D(0.f, 1.f, 0.f, 1.f, parametricZeroToOneT);
}


//--------------------------------------------------------------------------------------------------
float Hesitate5(float parametricZeroToOneT)
{
	return ComputeQuinticBezier1D(0.f, 1.f, 0.f, 1.f, 0.f, 1.f, parametricZeroToOneT);
}


//--------------------------------------------------------------------------------------------------
void CalculateTangentSpaceVectors(std::vector<Vertex_PCUTBN>& vertsToModify, std::vector<unsigned int> const& indexes)
{
	size_t numOfVerts		=	vertsToModify.size();
	size_t numOfIndexes		=	indexes.size();
	// 
	// for (size_t vertIndex = 0; vertIndex < numOfIndexes; vertIndex += 3)
	// {
	// 	size_t const& index0 = indexes[vertIndex + 0];
	// 	size_t const& index1 = indexes[vertIndex + 1];
	// 	size_t const& index2 = indexes[vertIndex + 2];
	// 	
	// 	// Vertexes
	// 	Vertex_PCUTBN& vert0 = vertsToModify[index0];
	// 	Vertex_PCUTBN& vert1 = vertsToModify[index1];
	// 	Vertex_PCUTBN& vert2 = vertsToModify[index2];
	// 
	// 	// Vertex Positions
	// 	Vec3 const& pos0 = vert0.m_position;
	// 	Vec3 const& pos1 = vert1.m_position;
	// 	Vec3 const& pos2 = vert2.m_position;
	// 
	// 	// Vertex Uvs
	// 	Vec2 const& uvs0 = vert0.m_uvTexCoords;
	// 	Vec2 const& uvs1 = vert1.m_uvTexCoords;
	// 	Vec2 const& uvs2 = vert2.m_uvTexCoords;
	// 
	// 	// Vertex Tangents
	// 	Vec3& tangent0 = vert0.m_tangent;
	// 	Vec3& tangent1 = vert1.m_tangent;
	// 	Vec3& tangent2 = vert2.m_tangent;
	// 
	// 	// Vertex Binormals
	// 	Vec3& binormal0 = vert0.m_binormal;
	// 	Vec3& binormal1 = vert1.m_binormal;
	// 	Vec3& binormal2 = vert2.m_binormal;
	// 
	// 	Vec3 edge1		=	pos1 - pos0;
	// 	Vec3 edge2		=	pos2 - pos0;
	// 	Vec2 uvEdge1	=	uvs1 - uvs0;
	// 	Vec2 uvEdge2	=	uvs2 - uvs0;
	// 
	// 	float r			=	1.f / (uvEdge1.x * uvEdge2.y - uvEdge2.x * uvEdge1.y);
	// 	Vec3 tangent	=	(edge1 * uvEdge2.y - edge2 * uvEdge1.y) * r;
	// 	Vec3 binormal	=	(edge2 * uvEdge1.x - edge1 * uvEdge2.x) * r;
	// 
	// 	tangent0 += tangent;
	// 	tangent1 += tangent;
	// 	tangent2 += tangent;
	// 
	// 	binormal0 += binormal;
	// 	binormal1 += binormal;
	// 	binormal2 += binormal;
	// }
	// 
	// for (size_t vertIndex = 0; vertIndex < numOfVerts; ++vertIndex)
	// {
	// 	Vertex_PCUTBN&	currentVert			=	vertsToModify[vertIndex];
	// 	Vec3&			currentTangent		=	currentVert.m_tangent;
	// 	Vec3&			currentBinormal		=	currentVert.m_binormal;
	// 	Vec3&			currentNormal		=	currentVert.m_normal;
	// 
	// 	// currentTangent		=	(currentTangent - currentNormal * DotProduct3D(currentNormal, currentTangent)).GetNormalized();
	// 	// currentBinormal		=	CrossProduct3D(currentNormal, currentTangent);
	// 	Mat44 vectorsToOrthoNormalize(currentNormal, currentTangent, currentBinormal, Vec3());
	// 	vectorsToOrthoNormalize.Orthonormalize_XFwd_YLeft_ZUp();
	// 	
	// 	currentNormal		=	vectorsToOrthoNormalize.GetIBasis3D();
	// 	currentTangent		=	vectorsToOrthoNormalize.GetJBasis3D();
	// 	currentBinormal		=	vectorsToOrthoNormalize.GetKBasis3D();
	// }

	std::vector<Vec3> tan1;
	std::vector<Vec3> tan2;
	tan1.resize(numOfIndexes);
	tan2.resize(numOfIndexes);

	for (size_t vertIndex = 0; vertIndex < numOfIndexes; vertIndex += 3)
	{
		long i1 = indexes[vertIndex + 0];
		long i2 = indexes[vertIndex + 1];
		long i3 = indexes[vertIndex + 2];

		Vertex_PCUTBN& vert1 = vertsToModify[i1];
		Vertex_PCUTBN& vert2 = vertsToModify[i2];
		Vertex_PCUTBN& vert3 = vertsToModify[i3];

		Vec3 const& v1 = vert1.m_position;
		Vec3 const& v2 = vert2.m_position;
		Vec3 const& v3 = vert3.m_position;

		Vec2 const& w1 = vert1.m_uvTexCoords;
		Vec2 const& w2 = vert2.m_uvTexCoords;
		Vec2 const& w3 = vert3.m_uvTexCoords;

		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;

		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;

		float r = 1.0f / (s1 * t2 - s2 * t1);
		Vec3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
			(t2 * z1 - t1 * z2) * r);
		Vec3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
			(s1 * z2 - s2 * z1) * r);

		tan1[i1] += sdir;
		tan1[i2] += sdir;
		tan1[i3] += sdir;

		tan2[i1] += tdir;
		tan2[i2] += tdir;
		tan2[i3] += tdir;
	}

	for (long a = 0; a < numOfVerts; a++)
	{
		Vertex_PCUTBN& verts = vertsToModify[a];
		Vec3 const& n = verts.m_normal;
		Vec3 const& t = tan1[a];

		// Gram-Schmidt orthogonalize
		verts.m_tangent		=	(t - n * DotProduct3D(n, t)).GetNormalized();
		verts.m_binormal	=	CrossProduct3D(n, verts.m_tangent);
	}
}
