#include "Game/Player.hpp"
#include "Game/Game.hpp"


//--------------------------------------------------------------------------------------------------
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Clock.hpp"


//--------------------------------------------------------------------------------------------------
extern InputSystem* g_theInput;


//--------------------------------------------------------------------------------------------------
Player::Player(Game* owner, Vec3 initialPosition, float clientAspect) : 
	m_owner(owner)
{
	m_clock = new Clock(*owner->m_clock);

	m_camera.SetTransform(initialPosition, EulerAngles(0.f, 0.f, 0.f));

	Vec3 renderIBasis(0.f, 0.f, 1.f);
	Vec3 renderJBasis(-1.f, 0.f, 0.f);
	Vec3 renderKBasis(0.f, 1.f, 0.f);
	m_camera.SetRenderBasis(renderIBasis, renderJBasis, renderKBasis);

	float fovDegrees		=	60.f;
	float zNear				=	0.1f;
	float zFar				=	1000.f;
	m_fovScale				=	TanDegrees(fovDegrees * 0.5f);
	m_camera.SetPerspectiveView(clientAspect, fovDegrees, zNear, zFar);

	// Debug camera
	m_debugCamera.SetTransform(initialPosition, EulerAngles());
	m_debugCamera.SetRenderBasis(renderIBasis, renderJBasis, renderKBasis);
	m_debugCamera.SetPerspectiveView(clientAspect, 60.f, 0.1f, 1000.f);
}


//--------------------------------------------------------------------------------------------------
Player::~Player()
{
	delete m_owner;
	m_owner = nullptr;
}


//--------------------------------------------------------------------------------------------------
void Player::Update()
{
	InputUpdate();
}


//--------------------------------------------------------------------------------------------------
void Player::Render() const
{
}


//--------------------------------------------------------------------------------------------------
void Player::InputUpdate()
{
	Vec3 iForward;
	Vec3 jLeft;
	Vec3 kUp;

	Camera& camera = (!m_owner->m_isDebug || m_owner->m_isDebugCameraLocked) ? m_camera : m_debugCamera;
	EulerAngles currentOrientation = camera.GetCameraOrientation();
	currentOrientation.GetAsVectors_XFwd_YLeft_ZUp(iForward, jLeft, kUp);
	Vec3 moveIntentions;

	if (g_theInput->IsKeyDown('W'))
	{
		moveIntentions += iForward;
	}
	if (g_theInput->IsKeyDown('A'))
	{
		moveIntentions += jLeft;
	}
	if (g_theInput->IsKeyDown('S'))
	{
		moveIntentions -= iForward;
	}
	if (g_theInput->IsKeyDown('D'))
	{
		moveIntentions -= jLeft;
	}
	if (g_theInput->IsKeyDown('Q'))
	{
		moveIntentions -= Vec3::Z_AXIS;
	}
	if (g_theInput->IsKeyDown('E'))
	{
		moveIntentions += Vec3::Z_AXIS;
	}

	float speedScale = 1.f;
	if (g_theInput->IsKeyDown(KEYCODE_SPACE))
	{
		speedScale = 10.f;
	}

	IntVec2 mouseDelta					= g_theInput->GetCursorClientDelta();
	constexpr float MOUSE_SENSITIVITY	= 0.1f;
	currentOrientation.m_yawDegrees		-= (mouseDelta.x * MOUSE_SENSITIVITY);
	currentOrientation.m_pitchDegrees	+= (mouseDelta.y * MOUSE_SENSITIVITY);
	currentOrientation.m_pitchDegrees	= GetClamped(currentOrientation.m_pitchDegrees, -89.9f, 89.9f);

	moveIntentions.Normalize();
	float deltaSeconds		= m_clock->GetDeltaSeconds();
	Vec3 currentPosition	= camera.GetCameraPosition();
	currentPosition			+= moveIntentions * m_speed * speedScale * deltaSeconds;

	camera.SetTransform(currentPosition, currentOrientation);
}