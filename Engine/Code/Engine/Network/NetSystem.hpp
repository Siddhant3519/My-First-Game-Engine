#pragma once


//--------------------------------------------------------------------------------------------------
#include "Engine/Core/EventSystem.hpp"


//--------------------------------------------------------------------------------------------------
#include <string>
#include <vector>


//--------------------------------------------------------------------------------------------------
class NetSystem;
extern NetSystem* g_theNetSystem;


//--------------------------------------------------------------------------------------------------
enum NetSystemMode
{
	NET_SYSTEM_MODE_NONE = 0,
	NET_SYSTEM_MODE_CLIENT,
	NET_SYSTEM_MODE_SERVER,
};

//--------------------------------------------------------------------------------------------------
struct NetSystemConfig
{
	NetSystemMode	m_mode					= NET_SYSTEM_MODE_NONE;
	std::string		m_hostAddressAsString   = "";
	int				m_sendBufferSize		= 2048;
	int				m_recvBufferSize		= 2048;
};


//--------------------------------------------------------------------------------------------------
class NetSystem
{
public:

	enum class ClientState
	{
		INVALID,
		READY_TO_CONNECT,
		CONNECTING,
		CONNECTED,
	};

	enum class ServerState
	{
		INVALID,
		LISTENING,
		CONNECTED,
	};

public:
	NetSystem(NetSystemConfig const& config);
	~NetSystem();

	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void SendMessages(std::string const& messageToSend);

private:
	void ServerStartup();
	void ClientStartup();

	void ServerBeginFrame();
	void ClientBeginFrame();


	void SendMessages();
	bool ReceiveMessagesAndCheckForFailure();

public:
	static bool Event_RemoteCommand(EventArgs& args);
	static bool Event_BurstTest(EventArgs& args);

public:
	NetSystemConfig				m_config;
	ClientState					m_clientState = ClientState::INVALID;
	ServerState					m_serverState = ServerState::INVALID;

	uintptr_t					m_clientSocket	= ~0ull;
	uintptr_t					m_listenSocket	= ~0ull;

	unsigned long				m_hostAddress	= 0;
	unsigned short				m_hostPort		= 0;

	char*						m_sendBuffer	= nullptr;
	char*						m_recvBuffer	= nullptr;

	std::vector<std::string>	m_sendQueue;
	std::string					m_recvRemaining;
};