#include "world.h"
#include "../common/eqemu_logsys.h"

WorldConnection::WorldConnection(const std::string &key, uint32_t dbid, const std::string &host)
{
	m_connecting = false;
	m_host = host;
	m_key = key;
	m_dbid = dbid;

	m_connection_manager.reset(new EQ::Net::DaybreakConnectionManager());
	m_connection_manager->OnNewConnection(std::bind(&WorldConnection::OnNewConnection, this, std::placeholders::_1));
	m_connection_manager->OnConnectionStateChange(std::bind(&WorldConnection::OnStatusChangeActive, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	m_connection_manager->OnPacketRecv(std::bind(&WorldConnection::OnPacketRecv, this, std::placeholders::_1, std::placeholders::_2));
	m_connection_manager->Connect(host, 9000);
}

WorldConnection::~WorldConnection() {
}

void WorldConnection::OnNewConnection(std::shared_ptr<EQ::Net::DaybreakConnection> connection)
{
	m_connection = connection;
	Log.OutF(Logs::General, Logs::Headless_Client, "Connecting to world...");
}

void WorldConnection::OnStatusChangeActive(std::shared_ptr<EQ::Net::DaybreakConnection> conn, EQ::Net::DbProtocolStatus from, EQ::Net::DbProtocolStatus to)
{
	if (to == EQ::Net::StatusConnected) {
		Log.OutF(Logs::General, Logs::Headless_Client, "World connected.");
		SendClientAuth();
	}

	if (to == EQ::Net::StatusDisconnected) {
		Log.OutF(Logs::General, Logs::Headless_Client, "World connection lost, reconnecting.");
		m_connection.reset();
		m_connection_manager->Connect(m_host, 9000);
	}
}

void WorldConnection::OnStatusChangeInactive(std::shared_ptr<EQ::Net::DaybreakConnection> conn, EQ::Net::DbProtocolStatus from, EQ::Net::DbProtocolStatus to)
{
	if (to == EQ::Net::StatusDisconnected) {
		m_connection.reset();
	}
}

void WorldConnection::OnPacketRecv(std::shared_ptr<EQ::Net::DaybreakConnection> conn, const EQ::Net::Packet &p)
{
	auto opcode = p.GetUInt16(0);
	Log.OutF(Logs::General, Logs::Headless_Client, "Packet in:\n{0}", p.ToString());
}

void WorldConnection::Kill()
{
	m_connection_manager->OnConnectionStateChange(std::bind(&WorldConnection::OnStatusChangeInactive, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	m_connection->Close();
}

void WorldConnection::Start()
{
	m_connection_manager->OnConnectionStateChange(std::bind(&WorldConnection::OnStatusChangeActive, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	m_connection_manager->Connect(m_host, 9000);
}

void WorldConnection::SendClientAuth()
{
	EQ::Net::DynamicPacket p;
	p.Resize(2 + 464);

	p.PutUInt16(0, 0x7a09U);
	std::string dbid_str = std::to_string(m_dbid);

	p.PutCString(2, dbid_str.c_str());
	p.PutCString(2 + dbid_str.length() + 1, m_key.c_str());

	m_connection->QueuePacket(p);
}
