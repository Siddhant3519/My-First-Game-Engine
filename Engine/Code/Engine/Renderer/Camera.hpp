#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/EulerAngles.hpp"

class Camera
{
	friend class Renderer;

public:
	enum Mode
	{
		Mode_Orthographic,
		Mode_Perspective,

		Mode_count
	};
	void SetOrthoView(Vec2 const& bottomLeft, Vec2 const& topRight, float near = 0.f, float far = 1.f);
	void SetPerspectiveView(float aspect, float fov, float near, float far);
	void SetPerspectiveFOV(float fov);
	void SetPerspectiveAspect(float aspect);
	void SetCameraCenter(Vec2 const& newCenter);
	void SetCameraViewport(float minX, float minY, float maxX, float maxY);
	void SetCameraViewport(Vec2 mins, Vec2 maxs);
	void SetCameraViewport(AABB2 viewport);
	void SetRenderBasis(Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis);
	void SetTransform(Vec3 const& position, EulerAngles const& orientation);
	void SetPositionOnly(Vec3 const& position);
	void SetOrientationOnly(EulerAngles const& orientation);

	Mat44 GetOrthographicMatrix()			const;
	Mat44 GetPerspectiveMatrix()			const;
	float GetCameraFarClipDist()			const;
	float GetCameraNearClipDist()			const;
	float GetPerspectiveCameraFov()			const;
	float GetPerspectiveCameraAspect()		const;
	Mat44 GetProjectionMatrix()				const;
	Mat44 GetRenderMatrix()					const;
	Mat44 GetViewMatrix()					const;
	Mat44 GetModelMatrix()					const;

	Vec2 GetCameraCenter() const;
	Vec2 GetOrthoBottomLeft() const;
	Vec2 GetOrthoTopRight() const;
	AABB2 GetCameraAABB() const;
	AABB2 GetCameraViewport() const;

	Vec3 GetCameraPosition() const;
	EulerAngles GetCameraOrientation() const;

	void Translate2D(Vec2 const& translation);
	Vec2 CameraShake(float randomTheta,float shakeAmount);

	Vec3 GetWorldMouseRayNormalized(Vec2 const& cursorPosInScreenSpace, Vec2 const& screenDims) const;

private:
	Mode m_mode = Mode_Orthographic;
	Vec2 m_orthographicBotttomLeft;
	Vec2 m_orthographicTopRight;
	AABB2 m_viewport = AABB2::INVALID;
	float m_orthographicNear;
	float m_orthographicFar;

	float m_perspectiveAspect;
	float m_perspectiveFOV;
	float m_perspectiveNear;
	float m_perspectiveFar;

	Vec3 m_renderIBasis = Vec3(1.f, 0.f, 0.f);
	Vec3 m_renderJBasis = Vec3(0.f, 1.f, 0.f);
	Vec3 m_renderKBasis = Vec3(0.f, 0.f, 1.f);

	Vec3 m_position;
	EulerAngles m_orientation;
};