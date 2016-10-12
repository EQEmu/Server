#include "relay_link.h"
#include "dns.h"
#include "../eqemu_logsys.h"
#include <algorithm>

EQ::Net::RelayLink::RelayLink(const std::string &addr, int port, const std::string &identifier)
	: m_timer(std::unique_ptr<EQ::Timer>(new EQ::Timer(250, true, std::bind(&EQ::Net::RelayLink::Connect, this))))
{
	m_established = false;
	m_connecting = false;
	m_port = port;
	m_identifier = identifier;
	DNSLookup(addr, port, false, [this](const std::string &address) {
		m_addr = address;
	});
}

EQ::Net::RelayLink::~RelayLink()
{
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
			//process raw packet
		}
		else {
			auto msg = fmt::format("**PACKETMODE{0}**", m_identifier);
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
			}
		}
	}
	catch (std::exception &ex) {
		Log.OutF(Logs::General, Logs::Debug, "Error parsing relay link packet: {0}", ex.what());
	}
}

void EQ::Net::RelayLink::SendIdentifier()
{
	auto msg = fmt::format("**PACKETMODE{0}**\r", m_identifier);
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
