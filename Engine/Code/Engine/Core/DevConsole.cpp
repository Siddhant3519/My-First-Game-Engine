#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Network/NetSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Stopwatch.hpp"


//--------------------------------------------------------------------------------------------------
extern DevConsole* g_theDevConsole = nullptr;


//--------------------------------------------------------------------------------------------------
#if defined (ERROR)
#undef ERROR
#endif


//--------------------------------------------------------------------------------------------------
Rgba8 const DevConsole::ERROR(255, 10, 10);
Rgba8 const DevConsole::WARNING(255, 255, 0);
Rgba8 const DevConsole::INFO_MAJOR(0, 255, 255);
Rgba8 const DevConsole::INFO_MINOR(0, 139, 139);
Rgba8 const DevConsole::COMMAND_ECHO(255, 0, 255);
Rgba8 const DevConsole::INPUT_TEXT(200, 200, 200);
Rgba8 const DevConsole::INPUT_CARET(255, 255, 255);


//--------------------------------------------------------------------------------------------------
DevConsole::DevConsole(DevConsoleConfig const& config) :
	m_config(config)
{
	m_caretStopwatch = new Stopwatch(0.5f);
}


//--------------------------------------------------------------------------------------------------
DevConsole::~DevConsole()
{
}


//--------------------------------------------------------------------------------------------------
void DevConsole::Startup()
{
	SubscribeEventCallbackFunction("CharInput",		Event_CharInput);
	SubscribeEventCallbackFunction("KeyPressed",	Event_KeyPressed);
	SubscribeEventCallbackFunction("KeyReleased",	Event_KeyReleased);
	SubscribeEventCallbackFunction("Echo",			Event_Echo);
	SubscribeEventCallbackFunction("clear",			Command_Clear);
	SubscribeEventCallbackFunction("help",			Command_Help);
	
	AddLine(DevConsole::INFO_MINOR, "Type help for a list of commands");

	m_cellHeight = 1.f / (m_config.m_lineOnScreen + 1);

	m_fontFilePath = std::string("Data/Fonts/" + m_config.m_fontName);
	m_caretStopwatch->Start();
}


//--------------------------------------------------------------------------------------------------
void DevConsole::Shutdown()
{
}


//--------------------------------------------------------------------------------------------------
void DevConsole::BeginFrame()
{
	if (!m_isOpen)
	{
		return;
	}
	if (m_caretStopwatch->IsStopped())
	{
		m_caretStopwatch->Start();
	}
	if (m_caretStopwatch->DecrementDurationIfElapsed())
	{
		m_caretVisible = !m_caretVisible;
	}
}


//--------------------------------------------------------------------------------------------------
void DevConsole::EndFrame()
{
}


//--------------------------------------------------------------------------------------------------
void DevConsole::Execute(std::string const& consoleCommandText)
{
	Strings spaceDelimitedText = SplitStringOnDelimiterWithQuotes(consoleCommandText, ' ');
	if (spaceDelimitedText.size() == 1 && spaceDelimitedText[0].size() == 0)
	{
		ToggleOpen();
		return;
	}

	if (spaceDelimitedText[0] != "Echo")
	{
		AddLine(COMMAND_ECHO, consoleCommandText);
	}
	// if (spaceDelimitedText[0] == "Echo" || (spaceDelimitedText[0] == "RemoteCommand" && g_theNetSystem->m_config.m_mode == NET_SYSTEM_MODE_SERVER))
	// {
	// 	// AddLine(COMMAND_ECHO, consoleCommandText);
	// }
	// else
	// {
	// 	AddLine(COMMAND_ECHO, consoleCommandText);
	// }

	std::vector<std::string> out_registeredCommands;
	g_theEventSystem->GetAllRegisteredCommands(out_registeredCommands);
	bool wasConsumed = false;
	for (int commandIndex = (int)out_registeredCommands.size() - 1; commandIndex >= 0; --commandIndex)
	{
		if (spaceDelimitedText[0] == "Server" || spaceDelimitedText[0] == "Connection")
		{
			return;
		}
		if (spaceDelimitedText[0] == out_registeredCommands[commandIndex])
		{
			EventArgs args;
			// Strings argumentKeyValPairsT = SplitStringOnDelimiterWithQuotes(spaceDelimitedText[1], '=');
			if (spaceDelimitedText.size() == 2)
			{
				Strings argumentKeyValPairs = SplitStringOnDelimiterWithQuotes(spaceDelimitedText[1], '=');
				if (argumentKeyValPairs.size() < 2)
				{
					AddLine(ERROR, "Invalid format");
					wasConsumed = true;
					break;
				}
				args.SetValue(argumentKeyValPairs[0], argumentKeyValPairs[1]);
			}
			else if (spaceDelimitedText.size() > 2)
			{
				AddLine(ERROR, "If you need to use White spaces put them in quotes");
			}
			FireEvent(spaceDelimitedText[0], args);
			g_theDevConsole->m_commandHistory.push_back(consoleCommandText);
			g_theDevConsole->m_historyIndex = (int)g_theDevConsole->m_commandHistory.size();
			wasConsumed = true;
			break;
		}
	}
	// if (spaceDelimitedText.size() > 2)
	// {
	// 	wasConsumed = true;
	// }

	if (!wasConsumed)
	{
		std::string errorString = "Unknown command: " + spaceDelimitedText[0];
		AddLine(ERROR, errorString);
		g_theDevConsole->m_commandHistory.push_back(consoleCommandText);
		g_theDevConsole->m_historyIndex = (int)g_theDevConsole->m_commandHistory.size();
	}

	// if (spaceDelimitedText.size() < 2)
	// {
	// 	g_theDevConsole->m_commandHistory.push_back(spaceDelimitedText[0]);
	// 	g_theDevConsole->m_historyIndex = (int)g_theDevConsole->m_commandHistory.size();
	// 	// g_theDevConsole->m_commandHistory.push_back(consoleCommandText);
	// 	// g_theDevConsole->m_historyIndex = (int)g_theDevConsole->m_commandHistory.size();
	// 	if (spaceDelimitedText[0] == "Help" || spaceDelimitedText[0] == "help")
	// 	{
	// 		FireEvent("Help");
	// 		AddLine(COMMAND_ECHO, spaceDelimitedText[0]);
	// 	}
	// 	else if (spaceDelimitedText[0] == "Clear" || spaceDelimitedText[0] == "clear")
	// 	{
	// 		FireEvent("Clear");
	// 		AddLine(COMMAND_ECHO, spaceDelimitedText[0]);
	// 	}
	// 	else if (spaceDelimitedText[0].empty())
	// 	{
	// 		m_isOpen = false;
	// 	}
	// 	else if (spaceDelimitedText[0] == "Quit" || spaceDelimitedText[0] == "quit")
	// 	{
	// 		FireEvent("Quit");
	// 	}
	// 	else
	// 	{
	// 		std::string errorString = "Unknown command: " + spaceDelimitedText[0];
	// 		AddLine(ERROR, errorString);
	// 	}
	// }
	// else if (spaceDelimitedText.size() == 2)
	// {
	// 	Strings equalDelimitedText = SplitStringOnDelimiter(spaceDelimitedText[1], '=');
	// 	if (equalDelimitedText.size() == 2)
	// 	{
	// 		std::vector<std::string> out_registeredCommands;
	// 		g_theEventSystem->GetAllRegisteredCommands(out_registeredCommands);
	// 		for (int commandIndex = (int)out_registeredCommands.size() - 1; commandIndex >= 0; --commandIndex)
	// 		{
	// 			g_theDevConsole->m_commandHistory.push_back(consoleCommandText);
	// 			g_theDevConsole->m_historyIndex = (int)g_theDevConsole->m_commandHistory.size();
	// 			if (spaceDelimitedText[0] == out_registeredCommands[commandIndex])
	// 			{
	// 				FireEvent(spaceDelimitedText[0]);
	// 			}
	// 			else
	// 			{
	// 				std::string errorString = "Unknown command: " + spaceDelimitedText[0];
	// 				AddLine(ERROR, errorString);
	// 			}
	// 		}
	// 	}
	// }
}


//--------------------------------------------------------------------------------------------------
void DevConsole::AddLine(Rgba8 const& color, std::string const& text)
{

	DevConsoleLine currentLine;
	currentLine.m_color = color;
	currentLine.m_text = text;

	m_devConsoleLinesMutex.lock();
	m_lines.push_back(currentLine);
	m_devConsoleLinesMutex.unlock();
}


//--------------------------------------------------------------------------------------------------
void DevConsole::Render(AABB2 const& bounds)
{
	if (!m_isOpen)
	{
		return;
	}
	
	m_devConsoleRenderMutex.lock();
	Camera const& devConsoleCamera = *m_config.m_camera;

	m_config.m_renderer->BeginCamera(devConsoleCamera);

	std::vector<Vertex_PCU> devConsoleOverlay;
	AddVertsForAABB2D(devConsoleOverlay, bounds, Rgba8(0, 0, 0, 175));
	
	BitmapFont* font = m_config.m_renderer->CreateOrGetBitmapFont(m_fontFilePath.c_str());
	
	float cellHeight = m_cellHeight * bounds.m_maxs.y;
	AABB2 localBounds = AABB2(bounds.m_mins.x, bounds.m_mins.y + cellHeight, bounds.m_maxs.x, bounds.m_mins.y + cellHeight + cellHeight);
	
	std::vector<Vertex_PCU> textInBoxVerts;
	float fontAspect = m_config.m_fontAspect;
	
	int linesToBePrinted = 0;

	for (int lineIndex = ((int)m_lines.size()) - 1; lineIndex >= 0; --lineIndex)
	{
		if (linesToBePrinted > m_config.m_lineOnScreen - 1)
		{
			break;
		}

		font->AddVertsForTextInBox2D(textInBoxVerts, localBounds, cellHeight, m_lines[lineIndex].m_text, m_lines[lineIndex].m_color, fontAspect, Vec2(0.f, 0.f));
		
		localBounds.m_mins.y += cellHeight;
		localBounds.m_maxs.y += cellHeight;
		++linesToBePrinted;
	}

	localBounds.m_mins.y = bounds.m_mins.y;
	localBounds.m_maxs.y = cellHeight;
	font->AddVertsForTextInBox2D(textInBoxVerts, localBounds, cellHeight, m_inputText, INPUT_TEXT, fontAspect, Vec2(0.f, 0.f));

	float cellWidth = cellHeight * fontAspect;
	AABB2 carretBounds(0.f, 0.f, cellWidth * 0.1f, cellHeight);
	float caretPosition = cellWidth * m_caretPosition;
	caretPosition = GetClamped(caretPosition, bounds.m_mins.x, bounds.m_maxs.x);
	carretBounds.SetCenter(Vec2(caretPosition, cellHeight * 0.5f));
	std::vector<Vertex_PCU> carretVerts;
	AddVertsForAABB2D(carretVerts, carretBounds, INPUT_CARET);

	m_config.m_renderer->SetBlendMode(BlendMode::ALPHA);
	m_config.m_renderer->BindTexture(nullptr);
	m_config.m_renderer->DrawVertexArray((int)devConsoleOverlay.size(), devConsoleOverlay.data());
	
	if (m_caretVisible)
	{
		m_config.m_renderer->SetBlendMode(BlendMode::ALPHA);
		m_config.m_renderer->BindTexture(nullptr);
		m_config.m_renderer->DrawVertexArray((int)carretVerts.size(), carretVerts.data());
	}
	
	m_config.m_renderer->SetBlendMode(BlendMode::ALPHA);
	m_config.m_renderer->BindTexture(&font->GetTexture());
	m_config.m_renderer->DrawVertexArray((int)textInBoxVerts.size(), textInBoxVerts.data());
	m_config.m_renderer->EndCamera(devConsoleCamera);
	m_devConsoleRenderMutex.unlock();
}


//--------------------------------------------------------------------------------------------------
void DevConsole::ToggleOpen()
{
	m_isOpen = !m_isOpen;
}


//--------------------------------------------------------------------------------------------------
bool DevConsole::IsOpen()
{
	return m_isOpen;
}


//--------------------------------------------------------------------------------------------------
bool DevConsole::Event_KeyPressed(EventArgs& args)
{	
	if (!g_theDevConsole)
	{
		return false;
	}
	unsigned char keyCode = (unsigned char)args.GetValue("KeyCode", -1);
	
	if (keyCode == KEYCODE_TILDE)
	{
		g_theDevConsole->ToggleOpen();
		return true;
	}
	if (g_theDevConsole->m_isOpen)
	{
		if (keyCode == KEYCODE_ENTER)
		{
			g_theDevConsole->Execute(g_theDevConsole->m_inputText);
			g_theDevConsole->m_inputText.clear();
			g_theDevConsole->m_caretPosition = 0;
			return true;
		}
		if (keyCode == KEYCODE_ESC)
		{
			if (!g_theDevConsole->m_inputText.empty())
			{
				g_theDevConsole->m_inputText.clear();
				g_theDevConsole->m_caretPosition = 0;
			}
			else
			{
				g_theDevConsole->m_isOpen = false;
			}
			return true;
		}
		if (keyCode == KEYCODE_LEFT)
		{
			g_theDevConsole->m_caretStopwatch->Restart();
			g_theDevConsole->m_caretVisible = true;
			if (g_theDevConsole->m_caretPosition == 0)
			{
				return true;
			}
			--g_theDevConsole->m_caretPosition;

			return true;
		}
		if (keyCode == KEYCODE_RIGHT)
		{
			g_theDevConsole->m_caretStopwatch->Restart();
			g_theDevConsole->m_caretVisible = true;
			if (g_theDevConsole->m_caretPosition == g_theDevConsole->m_inputText.size())
			{
				return true;
			}
			++g_theDevConsole->m_caretPosition;

			return true;
		}
		if (keyCode == KEYCODE_UP)
		{
			if (g_theDevConsole->m_historyIndex <= 0)
			{
				return true;
			}
			--g_theDevConsole->m_historyIndex;
			g_theDevConsole->m_inputText = g_theDevConsole->m_commandHistory[g_theDevConsole->m_historyIndex];
			g_theDevConsole->m_caretPosition = (int)g_theDevConsole->m_inputText.size();
			return true;
		}
		if (keyCode == KEYCODE_DOWN)
		{
			if (g_theDevConsole->m_historyIndex >= g_theDevConsole->m_commandHistory.size() - 1)
			{
				return true;
			}
			++g_theDevConsole->m_historyIndex;
			g_theDevConsole->m_inputText = g_theDevConsole->m_commandHistory[g_theDevConsole->m_historyIndex];
			g_theDevConsole->m_caretPosition = (int)g_theDevConsole->m_inputText.size();
			return true;
		}
		if (keyCode == KEYCODE_BACKSPACE)
		{
			g_theDevConsole->m_caretStopwatch->Restart();
			g_theDevConsole->m_caretVisible = true;
			if (g_theDevConsole->m_caretPosition == 0 || g_theDevConsole->m_caretPosition == g_theDevConsole->m_inputText.size() + 1)
			{
				return true;
			}
			std::string subString_first = std::string(g_theDevConsole->m_inputText, 0, g_theDevConsole->m_caretPosition - (size_t)1);
			std::string subString_second = std::string(g_theDevConsole->m_inputText, g_theDevConsole->m_caretPosition, g_theDevConsole->m_inputText.size() - 1);
			--g_theDevConsole->m_caretPosition;
	
			g_theDevConsole->m_inputText = subString_first + subString_second;
			return true;
		}
		if (keyCode == KEYCODE_DELETE)
		{
			g_theDevConsole->m_caretStopwatch->Restart();
			g_theDevConsole->m_caretVisible = true;
			if (g_theDevConsole->m_caretPosition == g_theDevConsole->m_inputText.size())
			{
				return true;
			}

			std::string subString_first = std::string(g_theDevConsole->m_inputText, 0, g_theDevConsole->m_caretPosition);
			std::string subString_second = std::string(g_theDevConsole->m_inputText, g_theDevConsole->m_caretPosition + (size_t)1, g_theDevConsole->m_inputText.size() - 1);
			
			g_theDevConsole->m_inputText = subString_first + subString_second;
			return true;
		}
		if (keyCode == KEYCODE_HOME)
		{
			g_theDevConsole->m_caretStopwatch->Restart();
			g_theDevConsole->m_caretVisible = true;
			g_theDevConsole->m_caretPosition = 0;
			return true;
		}
		if (keyCode == KEYCODE_END)
		{
			g_theDevConsole->m_caretStopwatch->Restart();
			g_theDevConsole->m_caretVisible = true;
			g_theDevConsole->m_caretPosition = (int)g_theDevConsole->m_inputText.size();
			return true;
		}
		return true;
	}
	
	return false;
}


//--------------------------------------------------------------------------------------------------
bool DevConsole::Event_KeyReleased(EventArgs& args)
{
	args;
	return false;
}


//--------------------------------------------------------------------------------------------------
bool DevConsole::Event_CharInput(EventArgs& args)
{
	if (!g_theDevConsole)
	{
		return false;
	}
	if (g_theDevConsole->m_isOpen)
	{
		unsigned char keyCode = (unsigned char)args.GetValue("KeyCode", -1);

		if ((keyCode >= 32 && keyCode <= 126) && (keyCode != '`' && keyCode != KEYCODE_TILDE))
		{
			g_theDevConsole->m_caretStopwatch->Restart();
			g_theDevConsole->m_caretVisible = true;
			std::string subString_first = std::string(g_theDevConsole->m_inputText, 0, g_theDevConsole->m_caretPosition);
			std::string subString_second = std::string(g_theDevConsole->m_inputText, g_theDevConsole->m_caretPosition, g_theDevConsole->m_inputText.size());
			subString_first += keyCode;
			g_theDevConsole->m_inputText = subString_first + subString_second;
			g_theDevConsole->m_caretPosition++;
			return true;
		}
	}

	return false;
}


//--------------------------------------------------------------------------------------------------
bool DevConsole::Event_Echo(EventArgs& args)
{
	if (!g_theDevConsole)
	{
		return false;
	}

	std::string echoText = args.GetValue("Message", "INVALID_TEXT");
	if (echoText == "INVALID_TEXT")
	{
		g_theDevConsole->AddLine(Rgba8::CRIMSON, "INVALID PARAMETERS/SYNTAX");
		return false;
	}

	TrimString(echoText, '"');
	g_theDevConsole->AddLine(Rgba8::WHITE, echoText);
	return true;
}


//--------------------------------------------------------------------------------------------------
bool DevConsole::Command_Clear(EventArgs& args)
{
	(void)args;
	if (g_theDevConsole)
	{
		g_theDevConsole->m_lines.clear();
		return true;
	}
	return false;
}


//--------------------------------------------------------------------------------------------------
bool DevConsole::Command_Help(EventArgs& args)
{
	if (!g_theDevConsole)
	{
		return false;
	}
	g_theDevConsole->AddLine(INFO_MAJOR, "Registered Commands");
	std::vector<std::string> out_registeredCommands;
	g_theEventSystem->GetAllRegisteredCommands(out_registeredCommands);
	for (int commandIndex = (int)out_registeredCommands.size() - 1; commandIndex >= 0; --commandIndex)
	{
		g_theDevConsole->AddLine(INFO_MINOR, out_registeredCommands[commandIndex]);
	}
	(void)args;
	return true;
}
