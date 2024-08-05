#pragma once


//--------------------------------------------------------------------------------------------------
#include "Engine/Input/KeyButtonState.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/IntVec2.hpp"


//--------------------------------------------------------------------------------------------------
struct IntVec2;


//--------------------------------------------------------------------------------------------------
constexpr int CONTROLLER_COUNT = 4;
constexpr int NUM_KEYCODES = 256;


//--------------------------------------------------------------------------------------------------
extern unsigned char const KEYCODE_F1;
extern unsigned char const KEYCODE_F2;
extern unsigned char const KEYCODE_F3;
extern unsigned char const KEYCODE_F4;
extern unsigned char const KEYCODE_F5;
extern unsigned char const KEYCODE_F6;
extern unsigned char const KEYCODE_F7;
extern unsigned char const KEYCODE_F8;
extern unsigned char const KEYCODE_F9;
extern unsigned char const KEYCODE_F10;
extern unsigned char const KEYCODE_F11;
extern unsigned char const KEYCODE_ESC;
extern unsigned char const KEYCODE_SPACE;
extern unsigned char const KEYCODE_ENTER;
extern unsigned char const KEYCODE_LMB;
extern unsigned char const KEYCODE_RMB;
extern unsigned char const KEYCODE_UP;
extern unsigned char const KEYCODE_LEFT;
extern unsigned char const KEYCODE_DOWN;
extern unsigned char const KEYCODE_RIGHT;
extern unsigned char const KEYCODE_TILDE;
extern unsigned char const KEYCODE_BACKSPACE;
extern unsigned char const KEYCODE_INSERT;
extern unsigned char const KEYCODE_DELETE;
extern unsigned char const KEYCODE_HOME;
extern unsigned char const KEYCODE_END;
extern unsigned char const KEYCODE_LSHIFT;
extern unsigned char const KEYCODE_LBRACKET;
extern unsigned char const KEYCODE_RBRACKET;
extern unsigned char const KEYCODE_RCONTROL;
extern unsigned char const KEYCODE_FORWARD_SLASH;
extern unsigned char const KEYCODE_NUMPAD_DECIMAL;
extern unsigned char const KEYCODE_NUMPAD_UP;
extern unsigned char const KEYCODE_NUMPAD_LEFT;
extern unsigned char const KEYCODE_NUMPAD_DOWN;
extern unsigned char const KEYCODE_NUMPAD_RIGHT;
extern unsigned char const KEYCODE_NUMPAD_ZERO;
extern unsigned char const KEYCODE_LEFT_BRACKET;
extern unsigned char const KEYCODE_RIGHT_BRACKET;
extern unsigned char const KEYCODE_COMMA;
extern unsigned char const KEYCODE_PERIOD;
extern unsigned char const KEYCODE_SEMI_COLON;
extern unsigned char const KEYCODE_SINGLE_QUOTE;


//--------------------------------------------------------------------------------------------------
struct MouseState
{
	IntVec2 m_cursorClientPosition;
	IntVec2 m_cursorClientDelta;

	bool m_currentHidden = false;
	bool m_desiredHidden = false;

	bool m_currentRelative = false;
	bool m_desiredRelative = false;
};


//--------------------------------------------------------------------------------------------------
struct InputSystemConfig
{

};


//--------------------------------------------------------------------------------------------------
class InputSystem
{
public:
	InputSystem(InputSystemConfig const& config);
	~InputSystem() {};
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();
	bool IsKeyDown(unsigned char keycode);
	bool IsKeyUp(unsigned char keyCode);
	bool IsKeyDown_WasUp(unsigned char keycode);
	bool IsKeyDown_WasDown(unsigned char keyCode);
	bool WasKeyDown_IsUp(unsigned char keycode);
	bool IsKeyUp_WasUp(unsigned char keycode);
	void HandleKeyPressed(unsigned char keyCode);
	void HandleKeyReleased(unsigned char keyCode);
	XboxController const& GetController(int controllerID);

	void SetCursorMode(bool hidden, bool relative);
	IntVec2 GetCursorClientDelta() const;
	IntVec2 GetCursorClientPosition() const;
	Vec2 GetCursorNormalizedPosition() const;


	void SetMouseModeIfChanged();

	static bool Event_KeyPressed(EventArgs& args);
	static bool Event_KeyReleased(EventArgs& args);

protected:
	static InputSystem* s_theInput;
	InputSystemConfig m_config;
	KeyButtonState m_keyStates[NUM_KEYCODES];
	XboxController m_controllers[CONTROLLER_COUNT];
	MouseState m_mouseState;
};