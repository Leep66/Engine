#pragma once
#include <string>
#include <vector>
#include <winsock2.h>
#include "Engine/Core/EventSystem.hpp"

enum class NetworkMode 
{
	NONE = -1,
	SERVER,
	CLIENT,

	COUNT
};

enum class ClientState
{
	CONNECTING,
	CONNECTED,
	DISCONNECTED,
	TERMINATED
};
enum class ServerState
{
	LISTENING,
	CONNECTED,
	DISCONNECTED,
	TERMINATED
};

struct NetworkSystemConfig
{
	std::string m_hostAddressString = "127.0.0.1";
	int m_port = 3100;
	int m_sendBufferSize = 2048;
	int m_recvBufferSize = 2048;
};

class NetworkSystem 
{
public:
	// Constructor/Destructor
	NetworkSystem(NetworkSystemConfig const& config);
	~NetworkSystem();

	// Core functionality
	bool Startup();
	void Shutdown();
	bool StartServer(unsigned short port);
	bool StartClient(const std::string& host, unsigned short port);
	void BeginFrame();
	void EndFrame();
	void Send(const std::string& message);
	bool HasMessages() const;
	std::string GetNextMessage();

	bool WasJustDisconnected() const;

	// Status
	NetworkMode GetMode() const { return m_mode; }
	bool IsRunning() const { return m_mode != NetworkMode::NONE; }
	void CloseConnection();

	void SendCommand(const std::string& commandName, const EventArgs& args);
	void ProcessIncomingMessages();
	void ProcessReceivedMessage(const std::string& message);
	void SendRawCommand(const std::string& commandString);

private:
	// Internal helpers
	bool CheckForConnection();
	bool CheckForMessages();
	void HandleError(const std::string& context);


public:
	NetworkSystemConfig m_config;

	NetworkMode m_mode = NetworkMode::NONE;

	ClientState m_prevClientState;
	ClientState	m_clientState;
	ServerState m_prevServerState;
	ServerState	m_serverState;


	// Server members
	SOCKET m_listenSocket = INVALID_SOCKET;
	std::vector<SOCKET> m_clientSockets;

	// Client members
	SOCKET m_serverSocket = INVALID_SOCKET;

	// Message queue
	std::vector<uint8_t> m_sendBuffer;
	std::vector<uint8_t> m_receiveBuffer;
	std::vector<std::string> m_receivedMessages;
	
	std::string m_localName = "";
	std::string m_remoteName = "";

	bool m_isConnected = false;

	// WinSock data
};