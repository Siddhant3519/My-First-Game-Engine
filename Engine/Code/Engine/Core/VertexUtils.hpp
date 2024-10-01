#pragma once


//--------------------------------------------------------------------------------------------------
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/ConvexPoly2D.hpp"
#include "Engine/Math/ConvexHull2D.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Plane2D.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/OBB3.hpp"


//--------------------------------------------------------------------------------------------------
#include <vector>


//--------------------------------------------------------------------------------------------------
struct OBB2;
struct Vec3;
struct AABB3;
struct Rgba8;
struct Mat44;


//--------------------------------------------------------------------------------------------------
void TransformVertexArrayXY3D(int numVerts, Vertex_PCU* verts, float scaleXY, float rotationDegreesAboutZ, Vec2 const& tranlationXY);
void TransformVertexArray3D(int numVerts, Vertex_PCU* verts, Mat44 transform);
void TransformVertexArray3D(std::vector<Vertex_PCU>& verts, Mat44 const& transform);
void TransformVertexArray3D(int numVerts, Vertex_PCUTBN* verts, Mat44 const& transform);
void TransformVertexArray3D(std::vector<Vertex_PCUTBN>& verts, Mat44 const& transform);


//--------------------------------------------------------------------------------------------------
void AddVertsForCapsule2D(std::vector<Vertex_PCU>& verts, Vec2 const& boneStart, Vec2 const& boneEnd, float radius, Rgba8 const& color);
void AddVertsForDisc2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, Rgba8 const& color);
void AddVertsForRing2D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec2 const& center, float radius, float thickness, Rgba8 const& color);
void AddVertsForBorderedHexagon2D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec2 const& center, float radius, float thickness, Rgba8 const& color);
void AddVertsForBorderedHexagon2D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec2 const& center, float radius, float thickness, Rgba8 const& color);
void AddVertsForHexagon2D(std::vector <Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec2 const& center, float circumRadius, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForHexagon2D(std::vector <Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec2 const& center, float circumRadius, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForRing2D(std::vector<Vertex_PCU>& verts, Vec2 const& center, float radius, float thickness, Rgba8 const& color);
void AddVertsForSphere3D(std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE, int numLatitudeSlices = 8);
void AddVertsForUVSphereZ3D(std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, float numSlices, float numStacks, Rgba8 const& tint = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForSphereZ3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec3 const& center, float radius, float numSlices, float numStacks, Rgba8 const& tint = Rgba8::WHITE);
void AddVertsForUVSphereZWireframe3D(std::vector<Vertex_PCU>& verts, Vec3 const& center, float radius, int numSlices, int numStacks, float lineThickness, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForSphere3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec3 const& center, float radius, float numSlices, float numStacks, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);		// Mr. Car Version
void AddVertsForSphere3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec3 const& center, float radius, float numSlices, float numStacks, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);		// Mr. Car Version
void AddVertsForUVSphereZ3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, const Vec3& center, float radius, float numSlices, float numStacks, const Rgba8& tint = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);	// Mr. Shi Version
void AddVertsForUnitCylinderX3D(std::vector<Vertex_PCU>& verts, float numSlices, Rgba8 const& tint, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForCylinderZ3D(std::vector<Vertex_PCU>& verts, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, float numSlices, Rgba8 const& tint = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForCylinderZ3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, float numSlices, Rgba8 const& tint = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForCylinderZWireframe3D(std::vector<Vertex_PCU>& verts, Vec2 const& centerXY, FloatRange const& minMaxZ, float radius, float numSlices, float lineThickness, Rgba8 const& tint = Rgba8::WHITE);
void AddVertsForCylinder3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color = Rgba8::WHITE, int numSlices = 8, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, AABB2 const& bounds, Rgba8 const& color, AABB2 const& UVs = AABB2(0.f, 0.f, 1.f, 1.f));
void AddVertsForAABB2D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, AABB2 const& bounds, Rgba8 const& color, AABB2 const& UVs = AABB2(0.f, 0.f, 1.f, 1.f));
void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts, AABB3 const& bounds, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForAABB3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, AABB3 const& bounds, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForAABB3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, AABB3 const& bounds, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForAABBWireframe3D(std::vector<Vertex_PCU>& verts, AABB3 const& bounds, float lineThickness, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForQuad3D(std::vector<Vertex_PCUTBN>& verts, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForQuad3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForQuad3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForRoundedQuad3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, Vec3 const& bottomLeft, Vec3 const& bottomRight, Vec3 const& topRight, Vec3 const& topLeft, Rgba8 const& color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForOBB2D(std::vector<Vertex_PCU>& verts, OBB2 const& box, Rgba8 const& color);
void AddVertsForOBB3D(std::vector<Vertex_PCU>& verts, OBB3 const& box, Rgba8 const& color, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForOBB3D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, OBB3 const& box, Rgba8 color = Rgba8::WHITE, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForOBB3D(std::vector<Vertex_PCUTBN>& verts, std::vector<unsigned int>& indexes, OBB3 const& box, Rgba8 const& color, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForOBBWireframe3d(std::vector<Vertex_PCU>& verts, OBB3 const& box, float lineThickness, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForLineSegment2D(std::vector<Vertex_PCU>& verts, Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color);
void AddVertsForLineSegment2D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Vec2 const& start, Vec2 const& end, float thickness, Rgba8 const& color);
void AddVertsForLineSegment3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float thickness, Rgba8 const& color);
void AddVertsForArrow2D(std::vector<Vertex_PCU>& verts, Vec2 const& tailPos, Vec2 const& tipPos, float arrowSize, float lineThickness, Rgba8 const& color = Rgba8(0, 255, 0));
void AddVertsForCone3D(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color = Rgba8::WHITE, int numSlices = 8, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForCone3D(std::vector<Vertex_PCUTBN>& verts, Vec3 const& start, Vec3 const& end, float radius, Rgba8 const& color = Rgba8::WHITE, int numSlices = 8, AABB2 const& UVs = AABB2::ZERO_TO_ONE);
void AddVertsForLineList(std::vector<Vertex_PCU>& verts, Vec3 const& start, Vec3 const& end, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForConvexPoly2D(std::vector<Vertex_PCU>& verts, ConvexPoly2D const& poly2D, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForConvexPoly2D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, ConvexPoly2D const& poly2D, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForBorderedConvexPoly2D(std::vector<Vertex_PCU>& verts, ConvexPoly2D const& poly2D, float thickness, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForBorderedConvexPoly2D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, ConvexPoly2D const& poly2D, float thickness, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForPlane2D(std::vector<Vertex_PCU>& verts, Plane2D const& plane, float thickness, Vec2 const& camDims, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForPlane2D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, Plane2D const& plane, float thickness, Vec2 const& camDims, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForConvexHull2D(std::vector<Vertex_PCU>& verts, ConvexHull2D const& convexHull2D, float thickness, Vec2 const& camDims, Rgba8 const& color = Rgba8::WHITE);
void AddVertsForConvexHull2D(std::vector<Vertex_PCU>& verts, std::vector<unsigned int>& indexes, ConvexHull2D const& convexHull2D, float thickness, Vec2 const& camDims, Rgba8 const& color = Rgba8::WHITE);

//--------------------------------------------------------------------------------------------------
AABB2 GetVertexBounds2D(std::vector<Vertex_PCU> const& verts);