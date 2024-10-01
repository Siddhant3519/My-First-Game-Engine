#pragma once


//--------------------------------------------------------------------------------------------------
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EventSystem.hpp"


//--------------------------------------------------------------------------------------------------
class Game;
class Clock;


//--------------------------------------------------------------------------------------------------
class App
{
public:
	App();
	~App();
	void Startup();
	void Shutdown();
	void Run();
	void RunFrame();

	bool IsQuitting() const;
	void SetQuitting(bool isQuitting);

	bool HandleQuitRequested();
	void InputHandler();
	static bool Event_Quit(EventArgs& args);

private:
	void BeginFrame();
	void Update();
	void Render() const;
	void EndFrame();

public:
	Clock* m_clock = nullptr;

private:
	Game* m_theGame				=	nullptr;
	Camera m_devConsoleCamera	=	{};
	float m_aspectRatio			=	2.f;
	bool m_isQuitting			=	false;
	bool m_isSlowMo				=	false;
};