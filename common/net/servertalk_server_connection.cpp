#include "servertalk_server_connection.h"
#include "servertalk_server.h"
#include "../eqemu_logsys.h"
#include "../util/uuid.h"

EQ::Net::ServertalkServerConnection::ServertalkServerConnection(std::shared_ptr<EQ::Net::TCPConnection> c, EQ::Net::ServertalkServer *parent)
{
	m_connection = c;
	m_parent = parent;
	m_uuid = EQ::Util::UUID::Generate().ToString();
	m_connection->OnRead(std::bind(&ServertalkServerConnection::OnRead, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	m_connection->OnDisconnect(std::bind(&ServertalkServerConnection::OnDisconnect, this, std::placeholders::_1));
	m_connection->Start();
}

EQ::Net::ServertalkServerConnection::~ServertalkServerConnection()
{
}

void EQ::Net::ServertalkServerConnection::Send(uint16_t opcode, EQ::Net::Packet & p)
{
	// pad zero size packets
	if (p.Length() == 0) {
		p.PutUInt8(0, 0);
	}

	EQ::Net::DynamicPacket out;
	out.PutUInt32(0, p.Length());
	out.PutUInt16(4, opcode);
	out.PutPacket(6, p);

	InternalSend(ServertalkMessage, out);
}

void EQ::Net::ServertalkServerConnection::SendPacket(ServerPacket *p)
{
	EQ::Net::DynamicPacket pout;
	if (p->pBuffer) {
		pout.PutData(0, p->pBuffer, p->size);
	}
	Send(p->opcode, pout);
}

void EQ::Net::ServertalkServerConnection::OnMessage(uint16_t opcode, std::function<void(uint16_t, EQ::Net::Packet&)> cb)
{
	m_message_callbacks.insert(std::make_pair(opcode, cb));
}

void EQ::Net::ServertalkServerConnection::OnMessage(std::function<void(uint16_t, EQ::Net::Packet&)> cb)
{
	m_message_callback = cb;
}

void EQ::Net::ServertalkServerConnection::OnRead(TCPConnection *c, const unsigned char *data, size_t sz)
{
	m_buffer.insert(m_buffer.end(), (const char*)data, (const char*)data + sz);
	ProcessReadBuffer();
}

void EQ::Net::ServertalkServerConnection::ProcessReadBuffer()
{
	size_t current = 0;
	size_t total = m_buffer.size();

	while (current < total) {
		auto left = total - current;

		/*
		//header:
		//uint32 length;
		//uint8 type;
		*/
		size_t length = 0;
		uint8_t type = 0;
		if (left < 5) {
			break;
		}

		length = *(uint32_t*)&m_buffer[current];
		type = *(uint8_t*)&m_buffer[current + 4];

		if (current + 5 + length > total) {
			break;
		}

		if (length == 0) {
			EQ::Net::DynamicPacket p;
			switch (type) {
			case ServertalkClientHello:
			{
				SendHello();
			}
			break;
			case ServertalkClientHandshake:
			case ServertalkClientDowngradeSecurityHandshake:
				ProcessHandshake(p);
				break;
			case ServertalkMessage:
				ProcessMessage(p);
				break;
			}
		}
		else {
			EQ::Net::StaticPacket p(&m_buffer[current + 5], length);
			switch (type) {
			case ServertalkClientHello:
			{
				SendHello();
			}
			break;
			case ServertalkClientHandshake:
			case ServertalkClientDowngradeSecurityHandshake:
				ProcessHandshake(p);
				break;
			case ServertalkMessage:
				ProcessMessage(p);
				break;
			}
		}

		current += length + 5;
	}

	if (current == total) {
		m_buffer.clear();
	}
	else {
		m_buffer.erase(m_buffer.begin(), m_buffer.begin() + current);
	}
}

void EQ::Net::ServertalkServerConnection::OnDisconnect(TCPConnection *c)
{
	m_parent->ConnectionDisconnected(this);
}

void EQ::Net::ServertalkServerConnection::SendHello()
{
	EQ::Net::DynamicPacket hello;
	hello.PutInt8(0, 0);

	InternalSend(ServertalkServerHello, hello);
}

void EQ::Net::ServertalkServerConnection::InternalSend(ServertalkPacketType type, EQ::Net::Packet &p)
{
	if (!m_connection)
		return;

	EQ::Net::DynamicPacket out;
	out.PutUInt32(0, (uint32_t)p.Length());
	out.PutUInt8(4, (uint8_t)type);
	if (p.Length() > 0) {
		out.PutPacket(5, p);
	}

	m_connection->Write((const char*)out.Data(), out.Length());
}

void EQ::Net::ServertalkServerConnection::ProcessHandshake(EQ::Net::Packet &p)
{
	try {
		m_identifier = p.GetCString(0);
		auto credentials = p.GetCString(m_identifier.length() + 1);

		if (!m_parent->CheckCredentials(credentials)) {
			LogError("Got incoming connection with invalid credentials during handshake, dropping connection.");
			m_connection->Disconnect();
			return;
		}

		m_parent->ConnectionIdentified(this);
	}
	catch (std::exception &ex) {
		LogError("Error parsing handshake from client: {0}", ex.what());
		m_connection->Disconnect();
	}
}

void EQ::Net::ServertalkServerConnection::ProcessMessage(EQ::Net::Packet &p)
{
	try {
		auto length = p.GetUInt32(0);
		auto opcode = p.GetUInt16(4);
		if (length > 0) {
			auto data = p.GetString(6, length);
			size_t message_len = length;
			EQ::Net::StaticPacket packet(&data[0], message_len);

			auto cb = m_message_callbacks.find(opcode);
			if (cb != m_message_callbacks.end()) {
				cb->second(opcode, packet);
			}

			if (m_message_callback) {
				m_message_callback(opcode, packet);
			}
		}
	}
	catch (std::exception &ex) {
		LogError("Error parsing message from client: {0}", ex.what());
	}
}
