#include "Engine/Input/InputSystem.hpp"
#include "Engine/Window/Window.hpp"


//--------------------------------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


//--------------------------------------------------------------------------------------------------
unsigned char const KEYCODE_F1					=	VK_F1;
unsigned char const KEYCODE_F2					=	VK_F2;
unsigned char const KEYCODE_F3					=	VK_F3;
unsigned char const KEYCODE_F4					=	VK_F4;
unsigned char const KEYCODE_F5					=	VK_F5;
unsigned char const KEYCODE_F6					=	VK_F6;
unsigned char const KEYCODE_F7					=	VK_F7;
unsigned char const KEYCODE_F8					=	VK_F8;
unsigned char const KEYCODE_F9					=	VK_F9;
unsigned char const KEYCODE_F10					=	VK_F10;
unsigned char const KEYCODE_F11					=	VK_F11;
unsigned char const KEYCODE_ESC					=	VK_ESCAPE;
unsigned char const KEYCODE_SPACE				=	VK_SPACE;
unsigned char const KEYCODE_ENTER				=	VK_RETURN;
unsigned char const KEYCODE_LMB					=	VK_LBUTTON;
unsigned char const KEYCODE_RMB					=	VK_RBUTTON;
unsigned char const KEYCODE_UP					=	VK_UP;
unsigned char const KEYCODE_LEFT				=	VK_LEFT;
unsigned char const KEYCODE_DOWN				=	VK_DOWN;
unsigned char const KEYCODE_RIGHT				=	VK_RIGHT;
unsigned char const KEYCODE_TILDE				=	0xC0;
unsigned char const KEYCODE_BACKSPACE			=	VK_BACK;
unsigned char const KEYCODE_INSERT				=	VK_INSERT;
unsigned char const KEYCODE_DELETE				=	VK_DELETE;
unsigned char const KEYCODE_HOME				=	VK_HOME;
unsigned char const KEYCODE_END					=	VK_END;
unsigned char const KEYCODE_LSHIFT				=	VK_SHIFT;
unsigned char const KEYCODE_LBRACKET			=	VK_OEM_4;
unsigned char const KEYCODE_RBRACKET			=	VK_OEM_6;
unsigned char const KEYCODE_RCONTROL			=	VK_RCONTROL;
unsigned char const KEYCODE_FORWARD_SLASH		=	VK_OEM_2;
unsigned char const KEYCODE_NUMPAD_DECIMAL		=	VK_DECIMAL;
unsigned char const KEYCODE_NUMPAD_UP			=	VK_NUMPAD8;
unsigned char const KEYCODE_NUMPAD_LEFT			=	VK_NUMPAD4;
unsigned char const KEYCODE_NUMPAD_DOWN			=	VK_NUMPAD2;
unsigned char const KEYCODE_NUMPAD_RIGHT		=	VK_NUMPAD6;
unsigned char const KEYCODE_NUMPAD_ZERO			=	VK_NUMPAD0;
unsigned char const KEYCODE_LEFT_BRACKET		=	0xDB;
unsigned char const KEYCODE_RIGHT_BRACKET		=	0xDD;
unsigned char const KEYCODE_COMMA				=	VK_OEM_COMMA;
unsigned char const KEYCODE_PERIOD				=	VK_OEM_PERIOD;
unsigned char const KEYCODE_SEMI_COLON			=	VK_OEM_1;
unsigned char const KEYCODE_SINGLE_QUOTE		=	VK_OEM_7;


//--------------------------------------------------------------------------------------------------
InputSystem* InputSystem::s_theInput = nullptr;


//--------------------------------------------------------------------------------------------------
InputSystem::InputSystem(InputSystemConfig const& config) : m_config(config)
{
	s_theInput = this;
	for (int controllerIndex = 0; controllerIndex < CONTROLLER_COUNT; ++controllerIndex)
	{
		m_controllers[controllerIndex].m_id = controllerIndex;
	}
}


//--------------------------------------------------------------------------------------------------
void InputSystem::Startup()
{
	g_theEventSystem->SubscribeEventCallbackFunction("KeyPressed", InputSystem::Event_KeyPressed);
	g_theEventSystem->SubscribeEventCallbackFunction("KeyReleased", InputSystem::Event_KeyReleased);
}


//--------------------------------------------------------------------------------------------------
void InputSystem::Shutdown()
{
}


//--------------------------------------------------------------------------------------------------
void InputSystem::BeginFrame()
{
	for (int controllerIndex = 0; controllerIndex < CONTROLLER_COUNT; ++controllerIndex)
	{
		m_controllers[controllerIndex].Update();
	}

	SetMouseModeIfChanged();
}


//--------------------------------------------------------------------------------------------------
void InputSystem::EndFrame()
{
	for (int asciiListIndex = 0; asciiListIndex < NUM_KEYCODES; ++asciiListIndex)
	{
		m_keyStates[asciiListIndex].m_wasKeyDownLastFrame = m_keyStates[asciiListIndex].m_isKeyDownThisFrame;
	}
}


//--------------------------------------------------------------------------------------------------
bool InputSystem::IsKeyDown(unsigned char keyCode)
{
	bool isDown = m_keyStates[keyCode].m_isKeyDownThisFrame;

	return isDown;
}


//--------------------------------------------------------------------------------------------------
bool InputSystem::IsKeyUp(unsigned char keyCode)
{
	bool isKeyUp = !m_keyStates[keyCode].m_isKeyDownThisFrame;

	return isKeyUp;
}


//--------------------------------------------------------------------------------------------------
bool InputSystem::IsKeyDown_WasUp(unsigned char keyCode)
{
	bool isDown = m_keyStates[keyCode].m_isKeyDownThisFrame && !m_keyStates[keyCode].m_wasKeyDownLastFrame;

	return isDown;
}


//--------------------------------------------------------------------------------------------------
bool InputSystem::IsKeyDown_WasDown(unsigned char keyCode)
{
	bool isHeldDown = m_keyStates[keyCode].m_isKeyDownThisFrame && m_keyStates[keyCode].m_wasKeyDownLastFrame;

	return isHeldDown;
}


//--------------------------------------------------------------------------------------------------
bool InputSystem::WasKeyDown_IsUp(unsigned char keyCode)
{
	bool wasDown = m_keyStates[keyCode].m_wasKeyDownLastFrame && !m_keyStates[keyCode].m_isKeyDownThisFrame;

	return wasDown;
}


//--------------------------------------------------------------------------------------------------
bool InputSystem::IsKeyUp_WasUp(unsigned char keyCode)
{
	bool isPressed = !m_keyStates[keyCode].m_isKeyDownThisFrame && !m_keyStates[keyCode].m_wasKeyDownLastFrame;

	return isPressed;
}


//--------------------------------------------------------------------------------------------------
void InputSystem::HandleKeyPressed(unsigned char keyCode)
{
	m_keyStates[keyCode].m_isKeyDownThisFrame = true;
}


//--------------------------------------------------------------------------------------------------
void InputSystem::HandleKeyReleased(unsigned char keyCode)
{
	m_keyStates[keyCode].m_isKeyDownThisFrame = false;
}


//--------------------------------------------------------------------------------------------------
XboxController const& InputSystem::GetController(int controllerID)
{
	return m_controllers[controllerID];
}


//--------------------------------------------------------------------------------------------------
bool InputSystem::Event_KeyPressed(EventArgs& args)
{
	if (!s_theInput)
	{
		return false;
	}
	unsigned char keyCode = (unsigned char)args.GetValue("KeyCode", -1);
	s_theInput->HandleKeyPressed(keyCode);

	return true;
}


//--------------------------------------------------------------------------------------------------
bool InputSystem::Event_KeyReleased(EventArgs& args)
{
	if (!s_theInput)
	{
		return false;
	}
	unsigned char keyCode = (unsigned char)args.GetValue("KeyCode", -1);
	s_theInput->HandleKeyReleased(keyCode);

	return true;
}


//--------------------------------------------------------------------------------------------------
void InputSystem::SetCursorMode(bool hidden, bool relative)
{
	m_mouseState.m_desiredHidden	=	hidden;
	m_mouseState.m_desiredRelative	=	relative;
}


//--------------------------------------------------------------------------------------------------
IntVec2 InputSystem::GetCursorClientDelta() const
{
	return m_mouseState.m_cursorClientDelta;
}


//--------------------------------------------------------------------------------------------------
IntVec2 InputSystem::GetCursorClientPosition() const
{
	return m_mouseState.m_cursorClientPosition;
}


//--------------------------------------------------------------------------------------------------
Vec2 InputSystem::GetCursorNormalizedPosition() const
{
	return Vec2();
}


//--------------------------------------------------------------------------------------------------
void InputSystem::SetMouseModeIfChanged()
{
	if (m_mouseState.m_desiredHidden != m_mouseState.m_currentHidden)
	{
		m_mouseState.m_currentHidden = m_mouseState.m_desiredHidden;
		
		if (m_mouseState.m_currentHidden)
		{
			while (::ShowCursor(false) >= 0) {}
		}
		else
		{
			while (::ShowCursor(true) < 0) {}
		}
	}
	
	POINT cursorPos;
	::GetCursorPos(&cursorPos);
	HWND windowHandle = (HWND)Window::GetWindowContext()->GetHwnd();
	::ScreenToClient(windowHandle, &cursorPos);

	RECT clientRectangle;
	::GetClientRect(windowHandle, &clientRectangle);

	float clientWidth	=	float(clientRectangle.right - clientRectangle.left);
	float clientHeight	=	float(clientRectangle.bottom - clientRectangle.top);
	POINT clientCenter;
	clientCenter.x = (int)(clientWidth * 0.5f);
	clientCenter.y = (int)(clientHeight * 0.5f);

	if (m_mouseState.m_desiredRelative != m_mouseState.m_currentRelative)
	{
		m_mouseState.m_currentRelative = m_mouseState.m_desiredRelative;

		cursorPos.x								=	clientCenter.x;
		cursorPos.y								=	clientCenter.y;
		m_mouseState.m_cursorClientPosition.x	=	clientCenter.x;
		m_mouseState.m_cursorClientPosition.y	=	clientCenter.y;
		m_mouseState.m_cursorClientDelta = IntVec2(0, 0);                                                                                                                                                                                    

		::ClientToScreen(windowHandle, &clientCenter);
		::SetCursorPos(clientCenter.x, clientCenter.y);
		::GetCursorPos(&cursorPos);
		::ScreenToClient(windowHandle, &cursorPos);
	}

	if (m_mouseState.m_currentRelative)
	{
		IntVec2& deltaCursorPos		=	m_mouseState.m_cursorClientDelta;
		IntVec2 previousCursorPos	=	GetCursorClientPosition();
		deltaCursorPos.x			=	cursorPos.x - previousCursorPos.x;
		deltaCursorPos.y			=	cursorPos.y - previousCursorPos.y;

		::ClientToScreen(windowHandle, &clientCenter);
		::SetCursorPos(clientCenter.x, clientCenter.y);
		::GetCursorPos(&cursorPos);
		::ScreenToClient(windowHandle, &cursorPos);

		m_mouseState.m_cursorClientPosition.x = cursorPos.x;
		m_mouseState.m_cursorClientPosition.y = cursorPos.y;
	}
	m_mouseState.m_cursorClientPosition = IntVec2(cursorPos.x, cursorPos.y);
}
