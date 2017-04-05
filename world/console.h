#pragma once

#include "../common/net/tcp_server.h"
#include "../common/event/timer.h"
#include <map>
#include <vector>

class ConsoleServer;
class ConsoleConnection
{
public:
	ConsoleConnection(ConsoleServer *parent, std::shared_ptr<EQ::Net::TCPConnection> connection);
	~ConsoleConnection();

	std::string GetUUID() const { return m_uuid; }

	void Clear();
	void SendLine(const std::string &line);
	void SendNewLine();
private:
	void OnRead(EQ::Net::TCPConnection* c, const unsigned char* data, size_t sz);
	void ProcessReadBuffer();
	void ProcessCommand(const std::string& cmd);
	void OnDisconnect(EQ::Net::TCPConnection* c);

	ConsoleServer *m_parent;
	std::shared_ptr<EQ::Net::TCPConnection> m_connection;
	std::string m_uuid;

	std::vector<char> m_buffer;
};

class ConsoleServer
{
public:
	ConsoleServer();
	~ConsoleServer();

	void ConnectionDisconnected(ConsoleConnection *c);
private:

	std::unique_ptr<EQ::Net::TCPServer> m_server;
	std::map<std::string, std::unique_ptr<ConsoleConnection>> m_connections;
	friend class ConsoleConnection;
};