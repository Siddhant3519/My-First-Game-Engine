#pragma once

//--------------------------------------------------------------------------------------------------
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Rgba8.hpp"


//--------------------------------------------------------------------------------------------------
#include <string>
#include <vector>
#include <atomic>
#include <mutex>


//--------------------------------------------------------------------------------------------------
class	BitmapFont;
class	Stopwatch;
class	Renderer;
class	Camera;
struct	AABB2;


//--------------------------------------------------------------------------------------------------
#if defined (ERROR)
#undef ERROR
#endif


//--------------------------------------------------------------------------------------------------
class DevConsole;
extern DevConsole* g_theDevConsole;


//--------------------------------------------------------------------------------------------------
struct DevConsoleLine
{
	Rgba8 m_color;
	std::string m_text;
};


//--------------------------------------------------------------------------------------------------
struct DevConsoleConfig
{
	Renderer* m_renderer = nullptr;
	Camera* m_camera = nullptr;
	std::string m_fontName = "SquirrelFixedFont";
	float m_fontAspect = 0.7f;
	float m_lineOnScreen = 21.f;
	int m_maxCommandHistory = 128;
};


//--------------------------------------------------------------------------------------------------
class DevConsole
{
public:
	DevConsole(DevConsoleConfig const& config);
	~DevConsole();

	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void Execute(std::string const& consoleCommandText);
	void AddLine(Rgba8 const& color, std::string const& text);
	void Render(AABB2 const& bounds);
	void ToggleOpen();

	bool IsOpen();
	 

public:
	static Rgba8 const ERROR;
	static Rgba8 const WARNING;
	static Rgba8 const INFO_MAJOR;
	static Rgba8 const INFO_MINOR;
	static Rgba8 const COMMAND_ECHO;
	static Rgba8 const INPUT_TEXT;
	static Rgba8 const INPUT_CARET;

public:
	static bool Event_KeyPressed(EventArgs& args);
	static bool Event_KeyReleased(EventArgs& args);
	static bool Event_CharInput(EventArgs& args);
	static bool Event_Echo(EventArgs& args);
	static bool Command_Clear(EventArgs& args);
	static bool Command_Help(EventArgs& args);

protected:
	std::mutex m_devConsoleLinesMutex;
	std::mutex m_devConsoleRenderMutex;

	DevConsoleConfig m_config;
	std::atomic<bool> m_isOpen = false;
	std::vector<DevConsoleLine> m_lines;
	std::string m_inputText;
	std::string m_fontFilePath;
	int m_caretPosition = 0;
	bool m_caretVisible = true;
	Stopwatch* m_caretStopwatch = nullptr;
	std::vector<std::string> m_commandHistory;
	int m_historyIndex = -1;
	float m_cellHeight = -1;
};