#pragma once	
#include "Engine/Input/KeyButtonState.hpp"
#include "Engine/Input/AnalogJoystick.hpp"

// class AnalogJoystick;

constexpr float NORMALIZED_INNER_DEAD_ZONE_THRESHOLD = 0.50f;
constexpr float NORMALIZED_OUTER_DEAD_ZONE_THRESHOLD = 0.75f;

enum class XboxButtonID
{
	INVALIDBUTTON = -1, 
	
	DPAD_UP,
	DPAD_DOWN,
	DPAD_LEFT,
	DPAD_RIGHT,

	START,
	BACK,

	LEFT_THUMB,
	RIGHT_THUMB,

	LEFT_SHOULDER,
	RIGHT_SHOULDER,

	A_BUTTON,
	B_BUTTON,
	X_BUTTON,
	Y_BUTTON,

	NUMOFBUTTONS
};

class XboxController
{
	friend class InputSystem;

public:
	XboxController();
	~XboxController() {};

	bool IsConnected() const;
	int GetControllerID() const;
	AnalogJoystick const& GetLeftStick() const;
	AnalogJoystick const& GetRightStick() const;
	float GetLeftTrigger() const;
	float GetRightTrigger() const;
	KeyButtonState const& GetButton(XboxButtonID buttonID) const;
	bool IsButtonDown(XboxButtonID buttonID) const;
	bool WasButtonJustPressed(XboxButtonID buttonID) const;
	bool WasButtonJustReleased(XboxButtonID buttonID) const;

private:
	void Update();
	void Reset();
	void UpdateJoystick(AnalogJoystick& out_joystick, short rawX, short rawY);
	void UpdateTrigger(float& out_triggerValue, unsigned char rawValue);
	void UpdateButton(XboxButtonID buttonID, unsigned short controllerButtonFlag, unsigned short desiredButtonFlag);

private:
	int m_id = -1;
	bool m_isConnected = false;
	float m_leftTrigger = 0.f;
	float m_rightTrigger = 0.f;
	KeyButtonState m_buttons[(int)XboxButtonID::NUMOFBUTTONS];
	AnalogJoystick m_leftStick;
	AnalogJoystick m_rightStick;
};