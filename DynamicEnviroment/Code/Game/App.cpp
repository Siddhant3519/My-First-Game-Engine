#include "Game/App.hpp"
#include "Game/Game.hpp"


//--------------------------------------------------------------------------------------------------
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Core/Clock.hpp"


//--------------------------------------------------------------------------------------------------
RandomNumberGenerator*		g_rng			= nullptr;
InputSystem*				g_theInput		= nullptr;
BitmapFont*					g_theFont		= nullptr;
Renderer*					g_theRenderer	= nullptr;
Window*						g_theWindow		= nullptr;
App*						g_theApp		= nullptr;


//--------------------------------------------------------------------------------------------------
App::App()
{
	m_devConsoleCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(1920.f, 1080.f));
	m_clock = new Clock();
}


//--------------------------------------------------------------------------------------------------
App::~App()
{
}


//--------------------------------------------------------------------------------------------------
void App::Startup()
{
	XmlDocument gameConfig;
	gameConfig.LoadFile("Data/GameConfig.xml");
	XmlElement& root = *(gameConfig.RootElement());
	g_gameConfigBlackboard.PopulateFromXmlElementAttributes(root);

	m_aspectRatio = g_gameConfigBlackboard.GetValue("windowAspect", 2.f);

	EventSystemConfig eventSystemConfig;
	g_theEventSystem = new EventSystem(eventSystemConfig);

	InputSystemConfig inputSystemConfig;
	g_theInput = new InputSystem(inputSystemConfig);
	
	WindowConfig windowConfig;
	windowConfig.m_windowTitle		=	"Dynamic Environment";
	windowConfig.m_clientAspect		=	m_aspectRatio;
	windowConfig.m_inputSystem		=	g_theInput;
	windowConfig.m_isFullScreen		=	g_gameConfigBlackboard.GetValue("isWindowFullScreen", false);
	g_theWindow = new Window(windowConfig);
	
	RendererConfig renderConfig;
	renderConfig.m_window	=	g_theWindow;
	g_theRenderer			=	new Renderer(renderConfig);

	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_renderer		=	g_theRenderer;
	devConsoleConfig.m_camera		=	&m_devConsoleCamera;
	g_theDevConsole = new DevConsole(devConsoleConfig);

	g_theEventSystem->Startup();
	g_theDevConsole->Startup();
	g_theInput->Startup();
	g_theWindow->Startup();
	g_theRenderer->Startup();
	g_rng = new RandomNumberGenerator();

	g_theFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");

	g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "CONTROLS");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "ESC     - Quit");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "WASD    - To Move");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Q/E     - Elevate");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Space   - Sprint");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "P       - Pause");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "H       - Reset");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "O       - SingleStep");
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "T       - Toggle slow mo");

	g_theEventSystem->SubscribeEventCallbackFunction("quit", Event_Quit);
	g_theEventSystem->SubscribeEventCallbackFunction("debugrenderclear", Command_DebugRenderClear);
	g_theEventSystem->SubscribeEventCallbackFunction("debugrendertoggle", Command_DebugRenderToggle);
	
	m_theGame = new Game(g_theWindow->GetConfig().m_clientAspect);
	
	DebugRenderConfig debugRenderConfig;
	debugRenderConfig.m_renderer		=	g_theRenderer;
	debugRenderConfig.m_startHidden		=	true;
	DebugRenderSystemStartup(debugRenderConfig);
	
	m_theGame->Startup();
}


//--------------------------------------------------------------------------------------------------
void App::Shutdown()
{
	m_theGame->Shutdown();
	delete m_theGame;
	m_theGame = nullptr;

	g_theRenderer->Shutdown();
	g_theWindow->Shutdown();
	g_theInput->Shutdown();
	g_theDevConsole->Shutdown();
	g_theEventSystem->Shutdown();

	delete g_theFont;
	g_theFont = nullptr;

	delete g_rng;
	g_rng = nullptr;

	delete g_theRenderer;
	g_theRenderer = nullptr;

	delete g_theWindow;
	g_theWindow = nullptr;

	delete g_theInput;
	g_theInput = nullptr;

	delete g_theDevConsole;
	g_theDevConsole = nullptr;

	delete g_theEventSystem;
	g_theEventSystem = nullptr;
}


//--------------------------------------------------------------------------------------------------
void App::Run()
{
	// Program main loop; keep running frames until it's time to quit
	while (!IsQuitting())
	{
		RunFrame();
	}
}


//--------------------------------------------------------------------------------------------------
void App::RunFrame()
{
	BeginFrame();
	
	InputHandler();
	Update();
	Render();

	EndFrame();
}


//--------------------------------------------------------------------------------------------------
bool App::IsQuitting() const
{
	return m_isQuitting;
}


//--------------------------------------------------------------------------------------------------
void App::SetQuitting(bool isQuitting)
{
	m_isQuitting = isQuitting;
}


//--------------------------------------------------------------------------------------------------
bool App::HandleQuitRequested()
{
	SetQuitting(true);
	return false;
}


//--------------------------------------------------------------------------------------------------
void App::InputHandler()
{
	if (g_theInput->IsKeyDown(KEYCODE_ESC) && !g_theInput->IsKeyDown_WasDown(KEYCODE_ESC) ||
		g_theInput->GetController(0).WasButtonJustPressed(XboxButtonID::BACK))
	{
		HandleQuitRequested();
		return;
	}

	if (g_theInput->IsKeyDown_WasUp('T'))
	{
		m_isSlowMo = !m_isSlowMo;
		if (m_isSlowMo)
		{
			m_clock->SetTimeScale(0.5f);
		}
		else
		{
			m_clock->SetTimeScale(1.f);
		}
	}

	if (g_theInput->IsKeyDown('P') && !g_theInput->IsKeyDown_WasDown('P')) // 0x50
	{
		m_clock->TogglePause();
	}

	if (g_theInput->IsKeyDown('O') && !g_theInput->IsKeyDown_WasDown('O')) // 0x4F
	{
		m_clock->StepSingleFrame();
	}
}


//--------------------------------------------------------------------------------------------------
bool App::Event_Quit(EventArgs& args)
{
	(void)args;
	g_theApp->HandleQuitRequested();
	return false;
}


//--------------------------------------------------------------------------------------------------
void App::BeginFrame()
{
	Clock::TickSystemClock();
	g_theDevConsole->BeginFrame();
	g_theInput->BeginFrame();
	g_theWindow->BeginFrame();
	g_theRenderer->BeginFrame();
	DebugRenderBeginFrame();
}


//--------------------------------------------------------------------------------------------------
void App::Update()
{
	if (IsQuitting()) return;

	if (g_theWindow->IsWindowInFocus() && !g_theDevConsole->IsOpen())
	{
		g_theInput->SetCursorMode(true, true);
	}
	else
	{
		g_theInput->SetCursorMode(false, false);
	}

	m_theGame->Update();
}


//--------------------------------------------------------------------------------------------------
void App::Render() const
{
	if (IsQuitting()) return;

	m_theGame->Render();

	g_theRenderer->SetModelConstants();
	AABB2 devConsoleBounds = AABB2(Vec2(0.f, 0.f), Vec2(1600.f, 800.f));
	g_theDevConsole->Render(devConsoleBounds);
}


//--------------------------------------------------------------------------------------------------
void App::EndFrame()
{
	g_theInput->EndFrame();
	g_theWindow->EndFrame();
	g_theRenderer->EndFrame();
	DebugRenderEndFrame();
}


//--------------------------------------------------------------------------------------------------
bool Command_DebugRenderClear(EventArgs& args)
{
	UNUSED(args);
	DebugRenderClear();
	return true;
}


//--------------------------------------------------------------------------------------------------
bool Command_DebugRenderToggle(EventArgs& args)
{
	UNUSED(args);
	DebugRenderToggle();
	return true;
}