#include "Engine/Core/DevConsole.hpp"
#include "Engine/Network/NetSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


//--------------------------------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")


//--------------------------------------------------------------------------------------------------
#include "Game/EngineBuildPreferences.hpp"
#if !defined( ENGINE_DISABLE_NETWORK )
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
NetSystem* g_theNetSystem = nullptr;


//--------------------------------------------------------------------------------------------------
NetSystem::NetSystem(NetSystemConfig const& config) :
	m_config(config)
{
	m_sendBuffer = new char[config.m_sendBufferSize];
	m_recvBuffer = new char[config.m_sendBufferSize];
}


//--------------------------------------------------------------------------------------------------
NetSystem::~NetSystem()
{
	delete[] m_sendBuffer;
	delete[] m_recvBuffer;
}


//--------------------------------------------------------------------------------------------------
void NetSystem::Startup()
{
	g_theEventSystem->SubscribeEventCallbackFunction("RemoteCommand", Event_RemoteCommand);
	g_theEventSystem->SubscribeEventCallbackFunction("BurstTest", Event_BurstTest);

	WSADATA data;
	int result = WSAStartup(MAKEWORD(2, 2), &data);
	if (result == SOCKET_ERROR)
	{
		// int errorCode = WSAGetLastError();
		DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
		DebuggerPrintf("\nSomething went wrong WSAStartup function!!!");
		DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
	}

	NetSystemMode const& currentMode = m_config.m_mode;
	if (currentMode == NET_SYSTEM_MODE_SERVER)
	{
		ServerStartup();
	}
	else if (currentMode == NET_SYSTEM_MODE_CLIENT)
	{
		ClientStartup();
	}
}


//--------------------------------------------------------------------------------------------------
void NetSystem::Shutdown()
{
	if (m_config.m_mode == NET_SYSTEM_MODE_SERVER)
	{
		closesocket(m_clientSocket);
		closesocket(m_listenSocket);
		WSACleanup();
	}
	else if (m_config.m_mode == NET_SYSTEM_MODE_CLIENT)
	{
		closesocket(m_clientSocket);
		WSACleanup();
	}
}


//--------------------------------------------------------------------------------------------------
void NetSystem::BeginFrame()
{
	NetSystemMode const& currentMode = m_config.m_mode;
	if (currentMode == NET_SYSTEM_MODE_SERVER)
	{
		ServerBeginFrame();
	}
	else if (currentMode == NET_SYSTEM_MODE_CLIENT)
	{
		ClientBeginFrame();
	}
}


//--------------------------------------------------------------------------------------------------
void NetSystem::SendMessages(std::string const& messageToSend)
{
	m_sendQueue.emplace_back(messageToSend);
}


//--------------------------------------------------------------------------------------------------
void NetSystem::ServerStartup()
{
	m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	unsigned long	blockingMode	= 1;
	int				result			= ioctlsocket(m_listenSocket, FIONBIO, &blockingMode);
	if (result == SOCKET_ERROR)
	{
		int errorCode = WSAGetLastError();
		if (errorCode == WSANOTINITIALISED)
		{
			DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
			DebuggerPrintf("\nSuccessful WSAStartup not yet performed!!!");
			DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
		}
	}

	m_hostAddress					= INADDR_ANY;
	Strings colonDelimitedList		= SplitStringOnDelimiterWithQuotes(m_config.m_hostAddressAsString, ':');
	size_t	colonDelimitedListSize	= colonDelimitedList.size();
	m_hostPort						= (unsigned short)(atoi(colonDelimitedList[colonDelimitedListSize - 1].c_str()));
	
	sockaddr_in addr;
	addr.sin_family				= AF_INET;
	addr.sin_addr.S_un.S_addr	= htonl(m_hostAddress);
	addr.sin_port				= htons(m_hostPort);
	result						= bind(m_listenSocket, (sockaddr*)&addr, (int)sizeof(addr));
	if (result == SOCKET_ERROR)
	{
		// int errorCode = WSAGetLastError();
		DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
		DebuggerPrintf("\nSomething went wrong while trying to bind the listen socket to a port!!!");
		DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
	}

	result = listen(m_listenSocket, SOMAXCONN);
	if (result == SOCKET_ERROR)
	{
		// int errorCode = WSAGetLastError();
		DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
		DebuggerPrintf("\nSomething went wrong while trying to listen for a connection!!!");
		DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
		m_serverState = ServerState::INVALID;
	}
	else
	{
		m_serverState = ServerState::LISTENING;
	}
}


//--------------------------------------------------------------------------------------------------
void NetSystem::ClientStartup()
{
	m_clientSocket				= socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	unsigned long blockingMode	= 1;
	int result					= ioctlsocket(m_clientSocket, FIONBIO, &blockingMode);
	if (result == SOCKET_ERROR)
	{
		//int errorCode = WSAGetLastError();
		// if (errorCode == WSANOTINITIALISED)
		// {
			DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
			DebuggerPrintf("\nSomething went wrong while setting the blocking mode!!!");
			DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
		// }
	}
	else
	{
		m_clientState = ClientState::READY_TO_CONNECT;
	}

	IN_ADDR addr;
	result = inet_pton(AF_INET, m_config.m_hostAddressAsString.c_str(), &addr);
	if (result == SOCKET_ERROR)
	{
		// int errorCode = WSAGetLastError();
		// if (errorCode == WSANOTINITIALISED)
		// {
		DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
		DebuggerPrintf("\nSomething went wrong while converting from IPv4 to IPV6 network addresses!!!");
		DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
		// }
	}
	m_hostAddress = ntohl(addr.S_un.S_addr);

	Strings colonDelimitedList = SplitStringOnDelimiterWithQuotes(m_config.m_hostAddressAsString, ':');
	size_t	colonDelimitedListSize = colonDelimitedList.size();
	m_hostPort = (unsigned short)(atoi(colonDelimitedList[colonDelimitedListSize - 1].c_str()));
}


//--------------------------------------------------------------------------------------------------
void NetSystem::ServerBeginFrame()
{
	switch (m_serverState)
	{
	case NetSystem::ServerState::INVALID:
	{
		DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
		DebuggerPrintf("\nInvalid Server State!!!");
		DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
		break;
	}
	case NetSystem::ServerState::LISTENING:
	{
		m_clientSocket = accept(m_listenSocket, NULL, NULL);
		if (m_clientSocket == INVALID_SOCKET)
		{
			int errorCode = WSAGetLastError();
			if (errorCode == WSAEWOULDBLOCK)
			{
				break;
			}
			else
			{
				// DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
				// DebuggerPrintf("\nSomething went wrong while accepting the incoming connection attempt!!!");
				// DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
				m_serverState = ServerState::INVALID;
				// break;
				ERROR_AND_DIE("\nSomething went wrong while accepting the incoming connection attempt!!!");
			}
		}

		// DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
		// DebuggerPrintf("\nConnection Accepted!!!");
		// DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
		// std::string connectionAcceptedString = "Echo Message=\"ConnectionAccepted\"";
		// g_theDevConsole->Execute(connectionAcceptedString);
		// m_sendQueue.emplace_back("Echo Message=\"Connection Accepted\"");

		m_serverState = ServerState::CONNECTED;
		unsigned long blockingMode = 1;
		int result = ioctlsocket(m_clientSocket, FIONBIO, &blockingMode);
		if (result == SOCKET_ERROR)
		{
			//int errorCode = WSAGetLastError();
			// if (errorCode == WSANOTINITIALISED)
			// {
			// DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
			// DebuggerPrintf("\nSomething went wrong while setting the blocking mode!!!");
			// DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
			ERROR_AND_DIE("\nSomething went wrong while setting the blocking mode!!!");
			// }
		}
		break;
	}
	case NetSystem::ServerState::CONNECTED:
	{
		// DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
		// DebuggerPrintf("\nConnection Already Exists!!!");
		// DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");

		// int result = recv(m_clientSocket, m_recvBuffer, m_config.m_recvBufferSize, 0);
		// if (result == SOCKET_ERROR)
		// {
		// 	int errorCode = WSAGetLastError();
		// 	if (errorCode == WSAECONNRESET)
		// 	{
		// 		m_serverState = ServerState::LISTENING;
		// 		Shutdown();
		// 		Startup();
		// 		return;
		// 	}
		// 	else if (errorCode != WSAEWOULDBLOCK)
		// 	{
		// 		DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
		// 		DebuggerPrintf("\nSomething went wrong while recv the message!!!");
		// 		DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
		// 	}
		// }
		// else if (result == 0)
		// {
		// 	m_serverState = ServerState::LISTENING;
		// 	Shutdown();
		// 	Startup();
		// 	return;
		// }

		bool isSuccess = ReceiveMessagesAndCheckForFailure();
		if (!isSuccess)
		{
			return;
		}

		SendMessages();
		
		// for (size_t messageIndex = 0; messageIndex < m_sendQueue.size(); ++messageIndex)
		// {
		// 	std::string currentMessage = m_sendQueue[messageIndex];
		// 	size_t sizeOfCurrentMessage = currentMessage.size();
		// 	GUARANTEE_OR_DIE(sizeOfCurrentMessage <= m_config.m_sendBufferSize, "ERROR: Tried to send a message greater than what the send buffer is capable of sending!!!");
		// 
		// 	size_t lenOfNullTerminatedString = strlen(currentMessage.c_str()) + 1;
		// 	errno_t stringCopyError = strncpy_s(m_sendBuffer, m_config.m_sendBufferSize, currentMessage.c_str(), lenOfNullTerminatedString);
		// 	GUARANTEE_OR_DIE(stringCopyError == 0, "ERROR: Something went wrong while trying to copy the contents of the string to the send buffer!!!");
		// 
		// 	result = send(m_clientSocket, m_sendBuffer, (int)lenOfNullTerminatedString, 0);
		// 	if (result == SOCKET_ERROR)
		// 	{
		// 		int errorCode = WSAGetLastError();
		// 		if (errorCode != WSAEWOULDBLOCK)
		// 		{
		// 			DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
		// 			DebuggerPrintf("\nSomething Went Wrong While Trying To Send A Message!!!");
		// 			DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
		// 		}
		// 	}
		// }
		// m_sendQueue.clear();

		// result = send(m_clientSocket, m_sendBuffer, 3, 0);
		// if (result == SOCKET_ERROR)
		// {
		// 	int errorCode = WSAGetLastError();
		// 	if (errorCode != WSAEWOULDBLOCK)
		// 	{
		// 		DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
		// 		DebuggerPrintf("\nSomething Went Wrong While Trying To Send A Message!!!");
		// 		DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
		// 	}
		// }
		break;
	}
	default:
	{
		// DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
		// DebuggerPrintf("\nSomething Went Terribly Wrong in Server Begin Frame!!!");
		// DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
		ERROR_AND_DIE("\nSomething Went Terribly Wrong in Server Begin Frame!!!");
		break;
	}
	}
}


//--------------------------------------------------------------------------------------------------
void NetSystem::ClientBeginFrame()
{
	switch (m_clientState)
	{
	case NetSystem::ClientState::INVALID:
	{
		DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
		DebuggerPrintf("\nInvalid Client State!!!");
		DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
		break;
	}
	case NetSystem::ClientState::READY_TO_CONNECT:
	{
		sockaddr_in addr;
		addr.sin_family				= AF_INET;
		addr.sin_addr.S_un.S_addr	= htonl(m_hostAddress);
		addr.sin_port				= htons(m_hostPort);
		int result					= connect(m_clientSocket, (sockaddr*)(&addr), (int)sizeof(addr));
		if (result == SOCKET_ERROR)
		{
			int errorCode = WSAGetLastError();
			if (errorCode == WSAEWOULDBLOCK)
			{
				m_clientState = ClientState::CONNECTING;
			}
			else
			{
				// DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
				// DebuggerPrintf("\nSomething Went Wrong While Trying To Connect To The Server!!!");
				// DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
				m_clientState = ClientState::INVALID;
				ERROR_AND_DIE("\nSomething Went Wrong While Trying To Connect To The Server!!!");
			}
		}
		else
		{
			m_clientState = ClientState::CONNECTING;
		}
		break;
	}
	case NetSystem::ClientState::CONNECTING:
	{
		fd_set failedSocket;
		FD_ZERO(&failedSocket);
		FD_SET(m_clientSocket, &failedSocket);
		timeval failedWaitTime = {};
		int failedResult = select(0, NULL, NULL, &failedSocket, &failedWaitTime);
		if (failedResult == SOCKET_ERROR)
		{
			// DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
			// DebuggerPrintf("\nSomething Went Wrong While Trying To Check For Failure!!!");
			// DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
			m_clientState = ClientState::INVALID;
			// break;
			ERROR_AND_DIE("\nSomething Went Wrong While Trying To Check For Failure!!!");
		}
		if (failedResult > 0 && FD_ISSET(m_clientSocket, &failedSocket))
		{
			m_clientState = ClientState::READY_TO_CONNECT;
			break;
		}


		fd_set sockets;
		FD_ZERO(&sockets);
		FD_SET(m_clientSocket, &sockets);
		timeval waitTime = { };
		int result = select(0, NULL, &sockets, NULL, &waitTime);
		if (result == SOCKET_ERROR)
		{
			// DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
			// DebuggerPrintf("\nSomething Went Wrong While Trying To Connect To The Server!!!");
			// DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
			m_clientState = ClientState::INVALID;
			// break;
			ERROR_AND_DIE("\nSomething Went Wrong While Trying To Connect To The Server!!!");
		}
		if (result > 0 && FD_ISSET(m_clientSocket, &sockets))
		{
			m_clientState = ClientState::CONNECTED;
			// m_sendQueue.emplace_back("Echo Message=\"Connection Accepted\"");
		}
		break;
	}
	case NetSystem::ClientState::CONNECTED:
	{
		ReceiveMessagesAndCheckForFailure();
		SendMessages();
		// result = send(m_clientSocket, m_sendBuffer, 3, 0);
		// if (result == SOCKET_ERROR)
		// {
		// 	int errorCode = WSAGetLastError();
		// 	if (errorCode != WSAEWOULDBLOCK)
		// 	{
		// 		DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
		// 		DebuggerPrintf("\nSomething Went Wrong While Trying To Send A Message!!!");
		// 		DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
		// 	}
		// }

		break;
	}
	default:
		ERROR_AND_DIE("\nSomething Went Terribly Wrong in Server Begin Frame!!!");
		break;
	}
}


//--------------------------------------------------------------------------------------------------
void NetSystem::SendMessages()
{
	for (size_t messageIndex = 0; messageIndex < m_sendQueue.size(); ++messageIndex)
	{
		std::string currentMessage = m_sendQueue[messageIndex];
		size_t sizeOfCurrentMessage = currentMessage.size();
		GUARANTEE_OR_DIE(sizeOfCurrentMessage < m_config.m_sendBufferSize, "ERROR: Tried to send a message greater than what the send buffer is capable of sending!!!");

		size_t lenOfNullTerminatedString = strlen(currentMessage.c_str()) + 1;
		errno_t stringCopyError = strncpy_s(m_sendBuffer, m_config.m_sendBufferSize, currentMessage.c_str(), lenOfNullTerminatedString);
		GUARANTEE_OR_DIE(stringCopyError == 0, "ERROR: Something went wrong while trying to copy the contents of the string to the send buffer!!!");

		int result = send(m_clientSocket, m_sendBuffer, (int)lenOfNullTerminatedString, 0);
		if (result == SOCKET_ERROR)
		{
			int errorCode = WSAGetLastError();
			if (errorCode != WSAEWOULDBLOCK)
			{
				// DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
				// DebuggerPrintf("\nSomething Went Wrong While Trying To Send A Message!!!");
				// DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
				ERROR_AND_DIE("\nSomething Went Wrong While Trying To Send A Message!!!");
			}
		}
	}

	m_sendQueue.clear();
}


//--------------------------------------------------------------------------------------------------
bool NetSystem::ReceiveMessagesAndCheckForFailure()
{
	int result = recv(m_clientSocket, m_recvBuffer, m_config.m_recvBufferSize, 0);
	if (result == SOCKET_ERROR)
	{
		int errorCode = WSAGetLastError();
		if (errorCode == WSAECONNRESET)
		{
			if (m_config.m_mode == NET_SYSTEM_MODE_CLIENT)
			{
				m_clientState = ClientState::READY_TO_CONNECT;
			}
			else if (m_config.m_mode == NET_SYSTEM_MODE_SERVER)
			{
				m_serverState = ServerState::LISTENING;
			}

			Shutdown();
			Startup();
			return false;
		}
		if (errorCode != WSAEWOULDBLOCK)
		{
			// DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
			// DebuggerPrintf("\nSomething went wrong while recv the message!!!");
			// DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
			ERROR_AND_DIE("\nSomething went wrong while recv the message!!!");
			// return false;
		}
		else
		{
			// DebuggerPrintf("\n--------------------------------------------------------------------------------------------------");
			// DebuggerPrintf("\nWaiting on receiving a message!!!");
			// DebuggerPrintf("\n--------------------------------------------------------------------------------------------------\n");
			return true;
		}
	}
	else if (result == 0)
	{
		if (m_config.m_mode == NET_SYSTEM_MODE_CLIENT)
		{
			m_clientState = ClientState::READY_TO_CONNECT;
		}
		else if (m_config.m_mode == NET_SYSTEM_MODE_SERVER)
		{
			m_serverState = ServerState::LISTENING;
		}
		// m_clientState = ClientState::READY_TO_CONNECT;
		Shutdown();
		Startup();
		return false;
	}

	for (size_t charIndex = 0; charIndex < result; ++charIndex)
	{
		char const currentChar = m_recvBuffer[charIndex];
		if (currentChar != '\0')
		{ 
			m_recvRemaining += currentChar;
		}
		else
		{
			g_theDevConsole->Execute(m_recvRemaining);
			if (m_config.m_mode == NET_SYSTEM_MODE_SERVER)
			{
				// TrimString(m_recvRemaining, '\"');
				// // std::string messageToEchoToTheClient = std::string("Server Executed Remote Command: " + m_recvRemaining);
				// std::string echoAppendedMessage = std::string("Echo Message=\"Server Executed Remote Command: " + m_recvRemaining + '\"');
				// // std::string echoAppendedMessage = std::string("Echo Message\" + messageToEcho + \"");
				// // m_sendQueue.emplace_back(messageToEchoToTheClient + '\"');
				// m_sendQueue.emplace_back(echoAppendedMessage);
			}
			m_recvRemaining.clear();
		}
	}

	return true;
}


//--------------------------------------------------------------------------------------------------
bool NetSystem::Event_RemoteCommand(EventArgs& args)
{
	if (!g_theDevConsole)
	{
		return false;
	}

	std::string command = args.GetValue("Command", "INVALID_COMMAND");
	if (command == "INVALID_COMMAND")
	{
		return false;
	}

	TrimString(command, '\"');
	g_theNetSystem->m_sendQueue.emplace_back(command);

	return true;
}


//--------------------------------------------------------------------------------------------------
bool NetSystem::Event_BurstTest(EventArgs& args)
{
	if (g_theNetSystem->m_config.m_mode == NET_SYSTEM_MODE_NONE)
	{
		return false;
	}
	(void)args;

	// g_theNetSystem->m_sendQueue.emplace_back("Echo Message=1");
	// g_theNetSystem->m_sendQueue.emplace_back("Echo Message=2");
	// g_theNetSystem->m_sendQueue.emplace_back("Echo Message=3");
	int count = 1;
	while (count <= 20)
	{
		g_theNetSystem->m_sendQueue.emplace_back(Stringf("Echo Message=%d", count));
		++count;
	}
	return true;
}


//--------------------------------------------------------------------------------------------------
#endif
//--------------------------------------------------------------------------------------------------
