#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2TCPIP.h>
#include <algorithm>
#include <iostream> 
#include "Engine/Network/NetworkSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EventSystem.hpp"

#pragma comment(lib, "Ws2_32.lib")

extern DevConsole* g_theDevConsole;
extern NetworkSystem* g_theNetwork;

// Constants
constexpr int MIN_BUFFER_SIZE = 128;
constexpr int MAX_BUFFER_SIZE = 65535; // 64KB
constexpr unsigned short DEFAULT_PORT = 3100;

NetworkSystem::NetworkSystem(NetworkSystemConfig const& config)
	: m_config(config)
	, m_clientState(ClientState::TERMINATED)
	, m_serverState(ServerState::TERMINATED)
{
	// m_mode = config.m_mode;
	// Validate buffer sizes
	m_config.m_sendBufferSize = std::clamp(
		m_config.m_sendBufferSize,
		MIN_BUFFER_SIZE,
		MAX_BUFFER_SIZE
	);

	m_config.m_recvBufferSize = std::clamp(
		m_config.m_recvBufferSize,
		MIN_BUFFER_SIZE,
		MAX_BUFFER_SIZE
	);

	// Pre-allocate buffers
	m_sendBuffer.reserve(m_config.m_sendBufferSize);
	m_receiveBuffer.reserve(m_config.m_recvBufferSize);
}

NetworkSystem::~NetworkSystem()
{
	Shutdown();
}

bool NetworkSystem::Startup()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		HandleError("WSAStartup failed");
		return false;
	}

	return true;
}

void NetworkSystem::Shutdown()
{
	CloseConnection();
	WSACleanup();

	m_mode = NetworkMode::NONE;
	m_clientState = ClientState::TERMINATED;
	m_serverState = ServerState::TERMINATED;
}

bool NetworkSystem::StartServer(unsigned short port)
{
	if (m_mode != NetworkMode::NONE)
	{
		HandleError("Network system already initialized");
		return false;
	}

	m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_listenSocket == INVALID_SOCKET)
	{
		HandleError("Server socket creation failed");
		return false;
	}

	// Set non-blocking
	u_long blockingMode = 1;
	if (ioctlsocket(m_listenSocket, FIONBIO, &blockingMode) != 0)
	{
		HandleError("Failed to set non-blocking mode");
		closesocket(m_listenSocket);
		return false;
	}

	// Bind to port
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if (bind(m_listenSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		HandleError("Bind failed");
		closesocket(m_listenSocket);
		return false;
	}

	if (listen(m_listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		HandleError("Listen failed");
		closesocket(m_listenSocket);
		return false;
	}

	m_mode = NetworkMode::SERVER;
	m_serverState = ServerState::LISTENING;
	return true;
}

bool NetworkSystem::StartClient(const std::string& host, unsigned short port)
{
	if (m_mode != NetworkMode::NONE)
	{
		HandleError("Network system already initialized");
		return false;
	}

	m_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_serverSocket == INVALID_SOCKET)
	{
		HandleError("Client socket creation failed");
		return false;
	}

	// Connect in blocking mode first
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, host.c_str(), &addr.sin_addr);
	addr.sin_port = htons(port);

	if (connect(m_serverSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		HandleError("Connect failed");
		closesocket(m_serverSocket);
		return false;
	}

	// Now set non-blocking
	u_long blockingMode = 1;
	if (ioctlsocket(m_serverSocket, FIONBIO, &blockingMode) != 0)
	{
		HandleError("Failed to set non-blocking mode");
		closesocket(m_serverSocket);
		return false;
	}

	m_mode = NetworkMode::CLIENT;
	m_clientState = ClientState::CONNECTED;
	return true;

}

void NetworkSystem::BeginFrame()
{
	ProcessIncomingMessages();

	switch (m_mode)
	{
	case NetworkMode::SERVER:
	{
		CheckForConnection();
		CheckForMessages();

		break;
	}

	case NetworkMode::CLIENT:
	{
		CheckForConnection();
		CheckForMessages();

		break;
	}

	case NetworkMode::NONE:
	default:
		break;
	}

	
}

void NetworkSystem::EndFrame()
{
	m_prevServerState = m_serverState;
	m_prevClientState = m_clientState;
}

bool NetworkSystem::CheckForConnection()
{
	if (m_mode == NetworkMode::SERVER)
	{
		SOCKET clientSocket = accept(m_listenSocket, nullptr, nullptr);
		if (clientSocket != INVALID_SOCKET)
		{
			// Set non-blocking
			u_long blockingMode = 1;
			ioctlsocket(clientSocket, FIONBIO, &blockingMode);

			m_clientSockets.push_back(clientSocket);
			m_serverState = ServerState::CONNECTED;
			m_isConnected = true;

			return true;
		}
		else
		{
			// No new client, but keep current m_isConnected status
		}
	}
	else if (m_mode == NetworkMode::CLIENT)
	{
		fd_set writeSet, exceptSet;
		FD_ZERO(&writeSet);
		FD_ZERO(&exceptSet);
		FD_SET(m_serverSocket, &writeSet);
		FD_SET(m_serverSocket, &exceptSet);

		timeval timeout = { 0 };
		int result = select(0, nullptr, &writeSet, &exceptSet, &timeout);

		if (result == SOCKET_ERROR)
		{
			HandleError("select() failed");
			m_clientState = ClientState::DISCONNECTED;
			m_isConnected = false;
			CloseConnection();
			return false;
		}

		if (FD_ISSET(m_serverSocket, &exceptSet))
		{
			m_clientState = ClientState::DISCONNECTED;
			m_isConnected = false;
			g_theDevConsole->AddLine(DevConsole::ERROR_COLOR, "Server connection error");
			CloseConnection();
			return false;
		}

		if (FD_ISSET(m_serverSocket, &writeSet) && m_clientState == ClientState::CONNECTING)
		{
			m_clientState = ClientState::CONNECTED;
			m_isConnected = true;
			g_theDevConsole->AddLine(DevConsole::INFO_MAJOR, "Connected to server");
			return true;
		}

		// Check for disconnection if already connected
		if (m_clientState == ClientState::CONNECTED)
		{
			char temp;
			int recvResult = recv(m_serverSocket, &temp, 1, MSG_PEEK);
			if (recvResult == 0)
			{
				// Server closed connection gracefully
				m_clientState = ClientState::DISCONNECTED;
				m_isConnected = false;
				g_theDevConsole->AddLine(DevConsole::ERROR_COLOR, "Server disconnected");
				CloseConnection();
				return false;
			}
			else if (recvResult == SOCKET_ERROR)
			{
				int err = WSAGetLastError();
				if (err != WSAEWOULDBLOCK)
				{
					// Connection reset or fatal error
					m_clientState = ClientState::DISCONNECTED;
					m_isConnected = false;
					g_theDevConsole->AddLine(DevConsole::ERROR_COLOR, Stringf("Server disconnected (error %d)", err));
					CloseConnection();
					return false;
				}
			}
		}
	}

	return false;
}


bool NetworkSystem::CheckForMessages()
{
	bool receivedData = false;

	if (m_mode == NetworkMode::SERVER)
	{
		for (auto it = m_clientSockets.begin(); it != m_clientSockets.end(); )
		{
			char buffer[1024];
			int bytesReceived = recv(*it, buffer, sizeof(buffer), 0);

			if (bytesReceived > 0)
			{
				m_receiveBuffer.insert(m_receiveBuffer.end(), buffer, buffer + bytesReceived);
				receivedData = true;
				++it;
			}
			else if (bytesReceived == 0)
			{
				// Connection closed
				closesocket(*it);
				it = m_clientSockets.erase(it);
				if (m_clientSockets.empty())
				{
					m_serverState = ServerState::DISCONNECTED;
				}
			}
			else
			{
				int error = WSAGetLastError();
				if (error != WSAEWOULDBLOCK)
				{
					closesocket(*it);
					it = m_clientSockets.erase(it);
					if (m_clientSockets.empty())
					{
						m_serverState = ServerState::DISCONNECTED;
					}
				}
				else
				{
					++it;
				}
			}
		}
	}
	else if (m_mode == NetworkMode::CLIENT && m_clientState == ClientState::CONNECTED)
	{
		char buffer[1024];
		int bytesReceived = recv(m_serverSocket, buffer, sizeof(buffer), 0);

		if (bytesReceived > 0)
		{
			m_receiveBuffer.insert(m_receiveBuffer.end(), buffer, buffer + bytesReceived);
			receivedData = true;
		}
		else if (bytesReceived == 0)
		{
			// Connection closed by server
			m_clientState = ClientState::DISCONNECTED;
			CloseConnection();
		}
		else
		{
			int error = WSAGetLastError();
			if (error != WSAEWOULDBLOCK)
			{
				m_clientState = ClientState::DISCONNECTED;
				CloseConnection();
			}
		}
	}

	auto findNull = [](const std::vector<uint8_t>& buf) -> size_t {
		auto it = std::find(buf.begin(), buf.end(), '\0');
		return (it != buf.end()) ? std::distance(buf.begin(), it) : std::string::npos;
		};

	size_t nullPos;
	while ((nullPos = findNull(m_receiveBuffer)) != std::string::npos)
	{
		std::string message(m_receiveBuffer.begin(), m_receiveBuffer.begin() + nullPos);
		m_receivedMessages.push_back(message);
		m_receiveBuffer.erase(m_receiveBuffer.begin(), m_receiveBuffer.begin() + nullPos + 1);
	}

	return receivedData;
}

void NetworkSystem::Send(const std::string& message)
{
	if (m_mode == NetworkMode::NONE)
	{
		return;
	}

	// Ensure null-terminated
	std::vector<uint8_t> packet(message.begin(), message.end());
	packet.push_back('\0');

	if (m_mode == NetworkMode::SERVER)
	{
		for (SOCKET client : m_clientSockets)
		{
			send(client, (const char*)packet.data(), (int)packet.size(), 0);
		}
	}
	else if (m_mode == NetworkMode::CLIENT && m_clientState == ClientState::CONNECTED)
	{
		send(m_serverSocket, (const char*)packet.data(), (int)packet.size(), 0);
	}
}

bool NetworkSystem::HasMessages() const
{
	return !m_receivedMessages.empty();
}

std::string NetworkSystem::GetNextMessage()
{
	if (m_receivedMessages.empty())
	{
		return "";
	}

	std::string message = m_receivedMessages.front();
	m_receivedMessages.erase(m_receivedMessages.begin());
	return message;
}



bool NetworkSystem::WasJustDisconnected() const
{
	if (m_mode == NetworkMode::SERVER)
	{
		return m_prevServerState == ServerState::CONNECTED
			&& m_serverState == ServerState::DISCONNECTED;
	}
	else if (m_mode == NetworkMode::CLIENT)
	{
		return m_prevClientState == ClientState::CONNECTED
			&& m_clientState == ClientState::DISCONNECTED;
	}
	return false;
}


void NetworkSystem::CloseConnection()
{
	if (m_mode == NetworkMode::SERVER)
	{
		for (SOCKET client : m_clientSockets)
		{
			closesocket(client);
		}
		m_clientSockets.clear();

		if (m_listenSocket != INVALID_SOCKET)
		{
			closesocket(m_listenSocket);
			m_listenSocket = INVALID_SOCKET;
		}

		m_serverState = ServerState::TERMINATED;
	}
	else if (m_mode == NetworkMode::CLIENT)
	{
		if (m_serverSocket != INVALID_SOCKET)
		{
			closesocket(m_serverSocket);
			m_serverSocket = INVALID_SOCKET;
		}

		m_clientState = ClientState::TERMINATED;
	}

	m_mode = NetworkMode::NONE;
	m_receivedMessages.clear();
	m_receiveBuffer.clear();
}

void NetworkSystem::SendCommand(const std::string& commandName, const EventArgs& args)
{
	if (m_mode == NetworkMode::NONE)
	{
		g_theDevConsole->AddLine(DevConsole::ERROR_COLOR, "[Network] Cannot send command: not connected");
		return;
	}

	std::string message = commandName;

	for (const auto& pair : args.m_keyValuePairs)
	{
		message += Stringf(" %s=%s", pair.first.c_str(), pair.second.c_str());
	}

	message += "\n";
	Send(message);
}



void NetworkSystem::ProcessIncomingMessages()
{
	char buffer[1024];

	if (m_mode == NetworkMode::SERVER)
	{
		for (SOCKET clientSocket : m_clientSockets)
		{
			int recvSize = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
			if (recvSize > 0)
			{
				buffer[recvSize] = '\0';
				std::string message(buffer);
				ProcessReceivedMessage(message);
			}
		}
	}
	else if (m_mode == NetworkMode::CLIENT)
	{
		int recvSize = recv(m_serverSocket, buffer, sizeof(buffer) - 1, 0);
		if (recvSize > 0)
		{
			buffer[recvSize] = '\0';
			std::string message(buffer);
			ProcessReceivedMessage(message);
		}
	}
}

void NetworkSystem::ProcessReceivedMessage(const std::string& message)
{
	std::vector<std::string> tokens = SplitStringOnDelimiter(message, ' ');
	if (tokens.empty())
		return;

	std::string commandName = CleanupString(tokens[0]);

	EventArgs args;
	for (size_t i = 1; i < tokens.size(); ++i)
	{
		size_t equalPos = tokens[i].find('=');
		if (equalPos != std::string::npos)
		{
			std::string key = CleanupString(tokens[i].substr(0, equalPos));
			std::string value = CleanupString(tokens[i].substr(equalPos + 1));
			args.SetValue(key, value);
		}
	}

	args.SetValue("remote", "true");
	FireEvent(commandName, args);
}

void NetworkSystem::SendRawCommand(const std::string& commandString)
{
	if (m_mode == NetworkMode::CLIENT)
	{
		send(m_serverSocket, commandString.c_str(), (int)commandString.size(), 0);
	}
	else if (m_mode == NetworkMode::SERVER)
	{
		for (SOCKET clientSocket : m_clientSockets)
		{
			send(clientSocket, commandString.c_str(), (int)commandString.size(), 0);
		}
	}
}

void NetworkSystem::HandleError(const std::string& context)
{
	int error = WSAGetLastError();
	std::cerr << "[NetworkError] " << context << " (Code: " << error << ")" << std::endl;
}