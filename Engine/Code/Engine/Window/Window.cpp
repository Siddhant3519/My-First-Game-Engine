#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Math/IntVec2.hpp"


//--------------------------------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>


//--------------------------------------------------------------------------------------------------
Window* Window::s_mainWindow = nullptr;


//-----------------------------------------------------------------------------------------------
// Handles Windows (Win32) messages/events; i.e. the OS is trying to tell us something happened.
// This function is called by Windows whenever we ask it for notifications
//
// #SD1ToDo: We will move this function to a more appropriate place later on...
//
// NOTE(sid): LRESULT (signed result that program returns to windows, it contains the programs response to a particular message)=> typedef LONG_PTR LRESULT 
// NOTE(sid): LONG_PTR (casting pointer to long -> pointer arithmetic) => typedef __int64 LONG_PTR (if win64) else, long
// NOTE(sid): CALLBACK is the calling convention of the function
// .......... Calling convention determines how data is read and provided (compliant to the ABI), in case of x64 the default convention uses a four register fast call calling convention
// .......... by default. 
// .......... fastcall -> stored in registers, then pushed on stack
// .......... stdcall -> pushes parameters on the stack, in reverse order (right to left) stack cleanup is handled by the callee
// .......... cdecl -> pushes parameters on the stack, in reverse order (right to left) stack cleanup is handled by the caller
// NOTE(sid): Here, WPARAM & LPARAM are additional message info, depending on the wmMessageCode
// NOTE(sid): This is a user defined callback function that processes messeges sent to window.
LRESULT CALLBACK WindowsMessageHandlingProcedure(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam)
{
	Window* window = Window::GetWindowContext();
	GUARANTEE_OR_DIE(window != nullptr, "Window::GetWindow() returned null");

	InputSystem* input = window->GetConfig().m_inputSystem;
	GUARANTEE_OR_DIE(input != nullptr, "Window::GetConfig().m_inputSystem returned null");

	switch (wmMessageCode)
	{
	case WM_CLOSE:
	{
		FireEvent("quit");
		/*g_theApp->HandleQuitRequested();*/
		// ERROR_RECOVERABLE("WM_CLOSE is not yet implemented!");
		return 0;
	}

	case WM_LBUTTONDOWN:
	{
		unsigned char keyCode = (unsigned char)KEYCODE_LMB;

		if (input)
		{
			input->HandleKeyPressed(keyCode);
			return 0;
		}

		break;
	}

	case WM_RBUTTONDOWN:
	{
		unsigned char keyCode = (unsigned char)KEYCODE_RMB;

		if (input)
		{
			input->HandleKeyPressed(keyCode);
			return 0;
		}

		break;
	}

	// Raw physical keyboard "key-was-just-depressed" event (case-insensitive, not translated)
	case WM_KEYDOWN:
	{
		EventArgs args;
		args.SetValue("KeyCode", Stringf("%d", (unsigned char)wParam));
		FireEvent("KeyPressed", args);

		break;
	}

	case WM_LBUTTONUP:
	{
		unsigned char keyCode = KEYCODE_LMB;

		if (input)
		{
			input->HandleKeyReleased(keyCode);
			return 0;
		}

		break;
	}

	case WM_RBUTTONUP:
	{
		unsigned char keyCode = KEYCODE_RMB;

		if (input)
		{
			input->HandleKeyReleased(keyCode);
			return 0;
		}

		break;
	}

	case WM_MOUSEWHEEL:
	{
		float wheelDelta = (float)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
		UNUSED(wheelDelta);
		break;
	}

	// Raw physical keyboard "key-was-just-released" event (case-insensitive, not translated)
	case WM_KEYUP:
	{
		EventArgs args;
		args.SetValue("KeyCode", Stringf("%d", (unsigned char)wParam));
		FireEvent("KeyReleased", args);

		break;
	}

	case WM_CHAR:
	{
		EventArgs args;
		args.SetValue("KeyCode", Stringf("%d", (unsigned char)wParam));
		FireEvent("CharInput", args);

		break;
	}
	}

	// Send back to Windows any unhandled/unconsumed messages we want other apps to see (e.g. play/pause in music apps, etc.)
	return DefWindowProc(windowHandle, wmMessageCode, wParam, lParam);
}


//--------------------------------------------------------------------------------------------------
Window::Window(WindowConfig const& config)
	: m_config(config)
{
	s_mainWindow = this;
}


//--------------------------------------------------------------------------------------------------
Window::~Window()
{
}


//--------------------------------------------------------------------------------------------------
void Window::Startup()
{
	// CreateOSWindow(applicationInstanceHandle);
	CreateOSWindow();
}


//--------------------------------------------------------------------------------------------------
void Window::Shutdown()
{
}


//--------------------------------------------------------------------------------------------------
void Window::BeginFrame()
{
	// Process OS messages (keyboard/mouse button clicked, application lost/gained focus, etc.)
	RunMessagePump(); // calls our own WindowsMessageHandlingProcedure() function for us!

}


//--------------------------------------------------------------------------------------------------
void Window::EndFrame()
{
}


//--------------------------------------------------------------------------------------------------
bool Window::IsWindowInFocus()
{
	if (GetHwnd() != ::GetActiveWindow())
	{
		return false;
	}
	else
	{
		return true;
	}
}


//--------------------------------------------------------------------------------------------------
void* Window::GetHwnd() const
{
	return m_windowHandle;
}


//--------------------------------------------------------------------------------------------------
IntVec2 Window::GetClientDimensions() const
{
	int clientWidth		= (int)m_clientWidth;
	int clientHeight	= (int)m_clientHeight;
	return IntVec2(clientWidth, clientHeight);
}


//--------------------------------------------------------------------------------------------------
Vec2 Window::GetNormalizedCursorPos() const
{
	POINT cursorPos;
	::GetCursorPos(&cursorPos);

	HWND windowHandle = ::GetActiveWindow();
	::ScreenToClient(windowHandle, &cursorPos);

	RECT clientRectangle;
	::GetClientRect(windowHandle, &clientRectangle);

	float clientWidth = float(clientRectangle.right - clientRectangle.left);
	float clientHeight = float(clientRectangle.bottom - clientRectangle.top);
	float clientMouseX = (float)cursorPos.x;
	float clientMouseY = (float)cursorPos.y;

	// ::ShowCursor(false);

	float mouseNormalizedU = clientMouseX / clientWidth;
	float mouseNormalizedV = clientMouseY / clientHeight;
	return Vec2(mouseNormalizedU, 1.f - mouseNormalizedV);
}


//--------------------------------------------------------------------------------------------------
std::string Window::GetXMLFileName(std::string const& directoryPath)
{
	(void)directoryPath;
	// SetCurrentDirectoryA(directoryPath.c_str());
	constexpr int maxfileNameLength = 256;
	char filePath[MAX_PATH] = {};
	OPENFILENAMEA infoAboutUserFileSelection		=	{};
	infoAboutUserFileSelection.lStructSize			=	sizeof(OPENFILENAMEA);
	infoAboutUserFileSelection.lpstrFile			=	filePath;
	infoAboutUserFileSelection.nMaxFile				=	maxfileNameLength;
	infoAboutUserFileSelection.lpstrInitialDir		=	directoryPath.c_str();
	infoAboutUserFileSelection.lpstrDefExt			=	"xml";
	infoAboutUserFileSelection.lpstrFilter			=	"XML Files\0*.xml\0\0";

	char currentDirectory[MAX_PATH] = {};
	DWORD status = GetCurrentDirectoryA(MAX_PATH, currentDirectory);
	if (status == 0)
	{
		return "INVALID";
	}
	BOOL returnInfo = GetOpenFileNameA(&infoAboutUserFileSelection);
	if (!returnInfo)
	{
		return "INVALID";
	}
	SetCurrentDirectoryA(currentDirectory);

	Strings			backSlashSplittedText	=	SplitStringOnDelimiter(filePath, '\\');
	size_t			listLength				=	backSlashSplittedText.size();
	std::string 	fileName				=	backSlashSplittedText[listLength - 1];
	return fileName;
}


//--------------------------------------------------------------------------------------------------
// void Window::CreateOSWindow(void* applicationInstanceHandle, float clientAspect)
void Window::CreateOSWindow()
{
	float clientAspect = m_config.m_clientAspect;

	// Define a window style/class

	WNDCLASSEX windowClassDescription;
	// NOTE(sid): memset stets a buffer to a specified character, in this case every byte from &windowClassDescription to size ( windowClassDescription ) is initialized to zero
	// .......... sizeof ( windowCalssDescription ) is 
	memset(&windowClassDescription, 0, sizeof(windowClassDescription));
	windowClassDescription.cbSize = sizeof(windowClassDescription); // NOTE(sid): size (bytes) of this structure
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context // NOTE(sid): class styles (https://docs.microsoft.com/en-us/windows/desktop/winmsg/about-window-classes)
	windowClassDescription.lpfnWndProc = static_cast<WNDPROC>(WindowsMessageHandlingProcedure); // Register our Windows message-handling function
	windowClassDescription.hInstance = GetModuleHandle(NULL); // NOTE(sid): handle to the instance that contains winproc for the class
	windowClassDescription.hIcon = NULL; // NOTE(sid): if null, system provided a default icon
	windowClassDescription.hCursor = NULL; // NOTE(sid): if null, application must explicitly set the cursor shape whenever the mouse moves into the application's window.
	windowClassDescription.lpszClassName = TEXT("Simple Window Class"); // NOTE(sid): Maximum length for lpszClassName is 256. Any greater then RegisterCalssEx () will fail.
	RegisterClassEx(&windowClassDescription); // NOTE(sid): registers a window for subsequent calls to CreateWindow

	// Get desktop rect, dimensions, aspect
	RECT desktopRect; // NOTE(sid): top-left, and lower-right
	HWND desktopWindowHandle = GetDesktopWindow(); // NOTE(sid): handle to the desktop window (area on which other windows are drawn)
	GetClientRect(desktopWindowHandle, &desktopRect); // NOTE(sid): Retrieves coordinates of a window's client area.
	float desktopWidth = (float)(desktopRect.right - desktopRect.left);
	float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);
	float desktopAspect = desktopWidth / desktopHeight;

	// #SD1ToDo: Add support for fullscreen mode (requires different window style flags than windowed mode)
	DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_SYSMENU | WS_OVERLAPPED;
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;
	if (m_config.m_isFullScreen)
	{
		windowStyleFlags = WS_POPUP | WS_SYSMENU;
		m_clientWidth = desktopWidth;
		m_clientHeight = desktopHeight;
		m_config.m_clientAspect = desktopAspect;
	}
	else
	{
		IntVec2 const& windowDims = m_config.m_windowDims;
		if (windowDims.x == -1 || windowDims.y == -1)
		{
			// Calculate maximum client size (as some % of desktop size)
			constexpr float maxClientFractionOfDesktop = 0.90f;
			m_clientWidth = desktopWidth * maxClientFractionOfDesktop;
			m_clientHeight = desktopHeight * maxClientFractionOfDesktop;
			if (clientAspect > desktopAspect)
			{
				// Client window has a wider aspect than desktop; shrink client height to match its width
				m_clientHeight = m_clientWidth / clientAspect;
			}
			else
			{
				// Client window has a taller aspect than desktop; shrink client width to match its height
				m_clientWidth = m_clientHeight * clientAspect;
			}
		}
		else
		{
			m_clientWidth	= (float)windowDims.x;
			m_clientHeight	= (float)windowDims.y;
		}
	}

	// Calculate client rect bounds by centering the client area
	float clientMarginX = 0.5f * (desktopWidth - m_clientWidth);
	float clientMarginY = 0.5f * (desktopHeight - m_clientHeight);
	RECT clientRect;
	IntVec2 const& clientTopLeft = m_config.m_windowStartPos;
	if (clientTopLeft.x == -1 || clientTopLeft.y == -1 || m_config.m_isFullScreen)
	{
		clientRect.left	= (int)clientMarginX;
		clientRect.top	= (int)clientMarginY;
	}
	else
	{
		clientRect.left	= clientTopLeft.x;
		clientRect.top	= clientTopLeft.y;
	}

	clientRect.right	= clientRect.left + (int)m_clientWidth;
	clientRect.bottom	= clientRect.top  + (int)m_clientHeight;
	// NOTE(sid): clientRect is our "window"
	// 
	// Calculate the outer dimensions of the physical window, including frame et. al.
	RECT windowRect = clientRect;
	//NOTE(sid): calculates the required size of the window including the desired client window size
	AdjustWindowRectEx(&windowRect, windowStyleFlags, FALSE, windowStyleExFlags);

	WCHAR windowTitle[1024]; // NOTE(sid): WCHAR 16bit unicode character
	// NOTE(sid): multibyte character is composed of one or more bytes. Each byte sequence represents a single character.
	// NOTE(sid): maps character string (need not be multibyte) to wide character (UTF-16)
	MultiByteToWideChar(GetACP(), 0, m_config.m_windowTitle.c_str(), -1, windowTitle, sizeof(windowTitle) / sizeof(windowTitle[0]));

	// NOTE(sid): if the function succeeds, the return value is a handle to the new window. else NULL
	HWND hwnd = CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		GetModuleHandle(NULL), // NOTE(sid): handle to the instance of the module to be associated. 
		NULL);

	// NOTE(sid): Activates window and displays it in its current size and position
	ShowWindow(hwnd, SW_SHOW);
	// NOTE(sid): Brings the thread that created the window to the foreground and activates the window. Keyboard input is directed towards this window, and this thread gets relatively higher priority
	SetForegroundWindow(hwnd);
	// NOTE(sid): Keyboard focus is set to the current window
	SetFocus(hwnd);

	m_windowHandle = (void*)hwnd;

	// NOTE(sid): Loads specified cursor, IDC_ARROW signifies the standard arrow, HINSTANCE is NULL we won't be loading a cursor from some other executable
	// .......... If the function succeeds, the return value is the handle to the new loaded cursor
	HCURSOR cursor = LoadCursor(NULL, IDC_ARROW);
	// NOTE(sid): sets cursor shape (if NULL the cursor is removed from the screen)
	SetCursor(cursor);
}


//-----------------------------------------------------------------------------------------------
// Processes all Windows messages (WM_xxx) for this app that have queued up since last frame.
// For each message in the queue, our WindowsMessageHandlingProcedure (or "WinProc") function
//	is called, telling us what happened (key up/down, minimized/restored, gained/lost focus, etc.)
//
void Window::RunMessagePump()
{
	// NOTE(sid): will soon contain message information from a thread's message queue
	MSG queuedMessage;
	for (;; )
	{
		// NOTE(sid): dispatches incoming nonqueued messages, checks the thread message queue for a posted message and retireves the message
		// .......... It takes a pointer to the MSG structure that receives message info, if the handle to window whose messages are to retireved is null then both window messages and thread messages are processed
		// .......... if both filtermin and filter max are 0 then PeekMessage returns all available messages, Specifies how messages are to be handled in this case messages are removed from the queue after processing by PeekMessage
		const BOOL wasMessagePresent = PeekMessage(&queuedMessage, NULL, 0, 0, PM_REMOVE);
		if (!wasMessagePresent)
		{
			break;
		}

		// NOTE(sid): translates virtual key messages into character messages, which are then posted to the calling thread's message queue, to be read the next time the thread calls the PeekMessage ()
		TranslateMessage(&queuedMessage);
		// NOTE(sid): Sends messages to windowproc in this case it's WindowsMessageHandlingProcedure
		DispatchMessage(&queuedMessage); // This tells Windows to call our "WindowsMessageHandlingProcedure" (a.k.a. "WinProc") function
	}
}