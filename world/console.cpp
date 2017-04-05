#include "console.h"
#include "../common/eqemu_config.h"
#include "../common/util/uuid.h"
#include "../common/eqemu_logsys.h"
#include "../common//net/packet.h"

ConsoleConnection::ConsoleConnection(ConsoleServer *parent, std::shared_ptr<EQ::Net::TCPConnection> connection)
{
	m_parent = parent;
	m_connection = connection;
	m_uuid = EQ::Util::UUID::Generate().ToString();

	m_connection->OnRead(std::bind(&ConsoleConnection::OnRead, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	m_connection->OnDisconnect(std::bind(&ConsoleConnection::OnDisconnect, this, std::placeholders::_1));
	m_connection->Start();
	Clear();
}

ConsoleConnection::~ConsoleConnection()
{
}

void ConsoleConnection::Clear()
{
	EQ::Net::DynamicPacket clear;
	clear.PutUInt8(0, 0);
	m_connection->Write((const char*)clear.Data(), clear.Length());
}

void ConsoleConnection::SendLine(const std::string &line)
{
	m_connection->Write(line.c_str(), line.length());
	SendNewLine();
}

void ConsoleConnection::SendNewLine()
{
	EQ::Net::DynamicPacket newline;
	newline.PutUInt8(0, 10);
	newline.PutUInt8(1, 13);
	m_connection->Write((const char*)newline.Data(), newline.Length());
}

void ConsoleConnection::OnRead(EQ::Net::TCPConnection *c, const unsigned char *data, size_t sz)
{
	m_buffer.insert(m_buffer.end(), (const char*)data, (const char*)data + sz);
	ProcessReadBuffer();
}

void ConsoleConnection::ProcessReadBuffer()
{
	size_t buffer_start = 0;
	for (size_t i = 0; i < m_buffer.size(); ++i) {
		char c = m_buffer[i];

		switch (c) {
		case 0:
			//Clear buffer
			break;
		case 10:
		case 13:
			//New Line
			break;
		case 8:
			//Backspace
			break;
		default:
			break;
		}
	}
}

void ConsoleConnection::ProcessCommand(const std::string &cmd)
{
}

void ConsoleConnection::OnDisconnect(EQ::Net::TCPConnection *c)
{
	m_parent->ConnectionDisconnected(this);
}

ConsoleServer::ConsoleServer()
{
	auto config = EQEmuConfig::get();

	m_server.reset(new EQ::Net::TCPServer());
	m_server->Listen(config->TelnetTCPPort, false, [this](std::shared_ptr<EQ::Net::TCPConnection> connection) {
		ConsoleConnection *c = new ConsoleConnection(this, connection);
		m_connections.insert(std::make_pair(c->GetUUID(), std::unique_ptr<ConsoleConnection>(c)));
	});
}

ConsoleServer::~ConsoleServer()
{
}

void ConsoleServer::ConnectionDisconnected(ConsoleConnection *c)
{
	auto iter = m_connections.find(c->GetUUID());
	if (iter != m_connections.end()) {
		m_connections.erase(iter);
	}
}
