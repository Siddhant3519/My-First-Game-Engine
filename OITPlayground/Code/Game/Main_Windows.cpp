#include "Game/App.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

// NOTE(sid): WIN32_LEAN_AND_MEAN -> Excludes certain type of APIs (Shell, sockets etc) Used to reduce the size of Windows Header files
#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
// NOTE(sid): Windows specefic "functions"
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include <math.h>
#include <cassert>
#include <crtdbg.h>

extern App* g_theApp;

//-----------------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int )
{
	Sleep(0);
	(void) commandLineString;
	(void) applicationInstanceHandle;
	
	
	g_theApp = new App();
	g_theApp->Startup();
	g_theApp->Run();
	g_theApp->Shutdown();
	delete g_theApp;
	g_theApp = nullptr;

	return 0;
}


