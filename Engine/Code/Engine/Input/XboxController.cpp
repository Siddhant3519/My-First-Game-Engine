#include "Engine/Input/XboxController.hpp"
#include "Engine/Core/EngineCommon.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>
#pragma comment(lib, "xinput9_1_0")

XboxController::XboxController()
{
	m_leftStick.SetDeadZoneThresholds(NORMALIZED_INNER_DEAD_ZONE_THRESHOLD, NORMALIZED_OUTER_DEAD_ZONE_THRESHOLD);
	// TODO(sid): Do I change the thresholds for the aiming stick
	m_rightStick.SetDeadZoneThresholds(NORMALIZED_INNER_DEAD_ZONE_THRESHOLD, NORMALIZED_OUTER_DEAD_ZONE_THRESHOLD);
}

bool XboxController::IsConnected() const
{
	return m_isConnected;
}

int XboxController::GetControllerID() const
{
	return m_id;
}

AnalogJoystick const& XboxController::GetLeftStick() const
{
	return m_leftStick;
}

AnalogJoystick const& XboxController::GetRightStick() const
{
	return m_rightStick;
}

float XboxController::GetLeftTrigger() const
{
	return m_leftTrigger;
}

float XboxController::GetRightTrigger() const
{
	return m_rightTrigger;
}

KeyButtonState const& XboxController::GetButton(XboxButtonID buttonID) const
{
	return m_buttons[(int)buttonID];
}

bool XboxController::IsButtonDown(XboxButtonID buttonID) const
{
	KeyButtonState currentButton = GetButton(buttonID);

	return currentButton.m_isKeyDownThisFrame;
}

bool XboxController::WasButtonJustPressed(XboxButtonID buttonID) const
{
	KeyButtonState currentButton = GetButton(buttonID);

	return (currentButton.m_isKeyDownThisFrame) && (!currentButton.m_wasKeyDownLastFrame);
}

bool XboxController::WasButtonJustReleased(XboxButtonID buttonID) const
{
	KeyButtonState currentButton = GetButton(buttonID);

	return (!currentButton.m_isKeyDownThisFrame) && (currentButton.m_wasKeyDownLastFrame);
}

void XboxController::Update()
{
	for (int buttonIndex = 0; buttonIndex < (int)XboxButtonID::NUMOFBUTTONS; ++buttonIndex)
	{
		m_buttons[buttonIndex].m_wasKeyDownLastFrame = m_buttons[buttonIndex].m_isKeyDownThisFrame;
	}

	XINPUT_STATE controllerState;
	if (XInputGetState(m_id, &controllerState) == ERROR_SUCCESS)
	{
		m_isConnected = true;
		XINPUT_GAMEPAD& gamePad = controllerState.Gamepad;

		UpdateJoystick(m_leftStick, gamePad.sThumbLX, gamePad.sThumbLY);
		UpdateJoystick(m_rightStick, gamePad.sThumbRX, gamePad.sThumbRY);

		UpdateTrigger(m_leftTrigger, gamePad.bLeftTrigger);
		UpdateTrigger(m_rightTrigger, gamePad.bRightTrigger);

		UpdateButton(XboxButtonID::DPAD_UP, gamePad.wButtons, XINPUT_GAMEPAD_DPAD_UP);
		UpdateButton(XboxButtonID::DPAD_DOWN, gamePad.wButtons, XINPUT_GAMEPAD_DPAD_DOWN);
		UpdateButton(XboxButtonID::DPAD_LEFT, gamePad.wButtons, XINPUT_GAMEPAD_DPAD_LEFT);
		UpdateButton(XboxButtonID::DPAD_RIGHT, gamePad.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT);

		UpdateButton(XboxButtonID::START, gamePad.wButtons, XINPUT_GAMEPAD_START);
		UpdateButton(XboxButtonID::BACK, gamePad.wButtons, XINPUT_GAMEPAD_BACK);

		UpdateButton(XboxButtonID::LEFT_THUMB, gamePad.wButtons, XINPUT_GAMEPAD_LEFT_THUMB);
		UpdateButton(XboxButtonID::RIGHT_THUMB, gamePad.wButtons, XINPUT_GAMEPAD_RIGHT_THUMB);

		UpdateButton(XboxButtonID::A_BUTTON, gamePad.wButtons, XINPUT_GAMEPAD_A);
		UpdateButton(XboxButtonID::B_BUTTON, gamePad.wButtons, XINPUT_GAMEPAD_B);
		UpdateButton(XboxButtonID::X_BUTTON, gamePad.wButtons, XINPUT_GAMEPAD_X);
		UpdateButton(XboxButtonID::Y_BUTTON, gamePad.wButtons, XINPUT_GAMEPAD_Y);

	}
	else
	{
		Reset();
	}
}

void XboxController::Reset()
{
	m_leftTrigger = 0.f;
	m_rightTrigger = 0.f;
	m_isConnected = false;

	for (int buttonIndex = 0; buttonIndex < (int)XboxButtonID::NUMOFBUTTONS; ++buttonIndex)
	{
		m_buttons[buttonIndex].m_isKeyDownThisFrame = false;
		m_buttons[buttonIndex].m_isKeyDownThisFrame = false;
	}

	m_leftStick.Reset();
	m_rightStick.Reset();
}

void XboxController::UpdateJoystick(AnalogJoystick& out_joystick, short rawX, short rawY)
{
	//Vec2 normalizedVec = Vec2(rawX, rawY).GetNormalized();
	Vec2 normalizedVec = Vec2(rawX / (float)MAXSHORT, rawY / (float)MAXSHORT);
	out_joystick.UpdatePosition(normalizedVec.x, normalizedVec.y);
}

void XboxController::UpdateTrigger(float& out_triggerValue, unsigned char rawValue)
{
	out_triggerValue = rawValue / (float)(MAXBYTE);
}

void XboxController::UpdateButton(XboxButtonID buttonID, unsigned short controllerButtonFlag, unsigned short desiredButtonFlag)
{
	m_buttons[(int)buttonID].m_isKeyDownThisFrame = ((controllerButtonFlag & desiredButtonFlag) == desiredButtonFlag);
}