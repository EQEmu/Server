#include "relay_link.h"
#include "dns.h"
#include "../eqemu_logsys.h"
#include "../md5.h"
#include "../servertalk.h"

EQ::Net::RelayLink::RelayLink(const std::string &addr, int port, const std::string &identifier, const std::string &password)
	: m_timer(std::unique_ptr<EQ::Timer>(new EQ::Timer(250, true, std::bind(&EQ::Net::RelayLink::Connect, this))))
{
	m_established = false;
	m_connecting = false;
	m_port = port;
	m_identifier = identifier;
	m_password = password;
	DNSLookup(addr, port, false, [this](const std::string &address) {
		m_addr = address;
	});

	m_opcode_dispatch.insert(std::make_pair(ServerOP_ZAAuthFailed, std::bind(&RelayLink::OnAuthFailed, this, std::placeholders::_1)));
}

EQ::Net::RelayLink::~RelayLink()
{
}

void EQ::Net::RelayLink::OnMessageType(uint16 opcode, std::function<void(const EQ::Net::Packet&p)> cb)
{
	if (opcode != ServerOP_ZAAuthFailed) {
		m_opcode_dispatch.insert(std::make_pair(opcode, cb));
	}
}

void EQ::Net::RelayLink::SendPacket(uint16 opcode, const EQ::Net::Packet &p)
{
	EQ::Net::WritablePacket packet;
	packet.PutUInt32(0, p.Length() + 7);
	packet.PutInt8(4, 0);
	packet.PutUInt16(5, opcode);
	if(p.Length() > 0)
		packet.PutPacket(7, p);

	if (m_connection) {
		m_connection->Write((const char*)packet.Data(), packet.Length());
	}
	else {
		m_packet_queue.push(packet);
	}
}

void EQ::Net::RelayLink::Connect()
{
	if (m_addr.length() == 0 || m_port == 0 || m_connection || m_connecting) {
		return;
	}

	m_connecting = true;
	EQ::Net::TCPConnection::Connect(m_addr, m_port, false, [this](std::shared_ptr<EQ::Net::TCPConnection> connection) {
		if (connection == nullptr) {
			Log.OutF(Logs::General, Logs::Debug, "Error connecting to {0}:{1}, attempting to reconnect...", m_addr, m_port);
			m_connecting = false;
			return;
		}

		Log.OutF(Logs::General, Logs::Debug, "Connected to {0}:{1}", m_addr, m_port);
		m_connection = connection;
		m_connection->OnDisconnect([this](EQ::Net::TCPConnection *c) {
			Log.OutF(Logs::General, Logs::Debug, "Connection lost to {0}:{1}, attempting to reconnect...", m_addr, m_port);
			m_established = false;
			m_connection.reset();
		});

		m_connection->OnRead(std::bind(&EQ::Net::RelayLink::ProcessData, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		m_connection->Start();

		SendIdentifier();
		m_connecting = false;
	});
}

void EQ::Net::RelayLink::ProcessData(EQ::Net::TCPConnection *c, const unsigned char *data, size_t length)
{
	EQ::Net::ReadOnlyPacket p((void*)data, length);
	try {
		Log.OutF(Logs::General, Logs::Debug, "Process data:\n{0}", p.ToString());

		if (m_established) {
			ProcessPacket(p);
		}
		else {
			std::string msg;
			if (m_identifier.compare("LOGIN") == 0) {
				msg = fmt::format("**PACKETMODE**\r");
			}
			else {
				msg = fmt::format("**PACKETMODE{0}**\r", m_identifier);
			}

			std::string cmp_msg;
			if (p.GetInt8(0) == '*') {
				cmp_msg = p.GetString(0, msg.length());
			}
			else if (p.GetInt8(1) == '*') {
				cmp_msg = p.GetString(1, msg.length());
			}
			else {
				return;
			}

			if (cmp_msg.compare(msg) == 0) {
				m_established = true;
				Log.OutF(Logs::General, Logs::Debug, "Established connection of type {0}", m_identifier);
				SendPassword();
			}
		}
	}
	catch (std::exception &ex) {
		Log.OutF(Logs::General, Logs::Debug, "Error parsing relay link packet: {0}", ex.what());
	}
}

void EQ::Net::RelayLink::ProcessPacket(const EQ::Net::Packet &p)
{
	char *buffer = (char*)p.Data();
	m_data_buffer.insert(m_data_buffer.begin() + m_data_buffer.size(), buffer, buffer + p.Length());

	ProcessBuffer();
}

void EQ::Net::RelayLink::ProcessBuffer()
{
	size_t size = 7;
	size_t base = 0;
	size_t used = m_data_buffer.size();
	while ((used - base) >= size) {
		uint32 packet_size = *(uint32*)&m_data_buffer[base];
		uint8 packet_flags = *(uint8*)&m_data_buffer[base + 4];
		uint16 packet_opcode = *(uint16*)&m_data_buffer[base + 5];

		if ((used - base) >= packet_size) {
			EQ::Net::ReadOnlyPacket p(&m_data_buffer[base], packet_size);

			if (m_opcode_dispatch.count(packet_opcode) > 0) {
				auto &cb = m_opcode_dispatch[(int)packet_opcode];
				cb(p);
			}
			else {
				Log.OutF(Logs::General, Logs::Debug, "Unhandled packet of type {0:x}", packet_opcode);
			}

			base += packet_size;
		}
		else {
			EQ::Net::WritablePacket p;

			if (m_opcode_dispatch.count(packet_opcode) > 0) {
				auto &cb = m_opcode_dispatch[(int)packet_opcode];
				cb(p);
			}
			else {
				Log.OutF(Logs::General, Logs::Debug, "Unhandled packet of type {0:x}", packet_opcode);
			}
		}
	}

	if (used == base) {
		m_data_buffer.clear();
	}
	else {
		m_data_buffer.erase(m_data_buffer.begin(), m_data_buffer.begin() + base);
	}
}

void EQ::Net::RelayLink::ProcessQueue()
{
	if (!m_connection)
		return;

	while (!m_packet_queue.empty()) {
		auto &p = m_packet_queue.front();
		m_connection->Write((const char*)p.Data(), p.Length());
		m_packet_queue.pop();
	}
}

void EQ::Net::RelayLink::SendIdentifier()
{
	std::string msg;
	if (m_identifier.compare("LOGIN") == 0) {
		msg = fmt::format("**PACKETMODE**\r");
	}
	else {
		msg = fmt::format("**PACKETMODE{0}**\r", m_identifier);
	}
	EQ::Net::WritablePacket packet;
	packet.PutData(0, (void*)msg.c_str(), msg.length());
	SendInternal(packet);
}

void EQ::Net::RelayLink::SendInternal(const EQ::Net::Packet &p)
{
	if (m_connection == nullptr) {
		return;
	}

	m_connection->Write((const char*)p.Data(), p.Length());
}

void EQ::Net::RelayLink::SendPassword()
{
	if (m_password.length() > 0) {
		char hash[16] = { 0 };
		MD5::Generate((const uchar*)m_password.c_str(), m_password.length(), (uchar*)&hash[0]);

		EQ::Net::WritablePacket p;
		p.PutData(0, &hash[0], 16);
		SendPacket(ServerOP_ZAAuth, p);
	}
}

void EQ::Net::RelayLink::OnAuthFailed(const EQ::Net::Packet &p)
{
	if (m_connection) {
		Log.OutF(Logs::General, Logs::Debug, "Authorization failed for server type {0}", m_identifier);
		m_connection->Disconnect();
	}
}
