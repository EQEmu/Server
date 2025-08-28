#pragma once

#include "../common/net/reliable_stream_connection.h"
#include "../common/event/timer.h"
#include <map>

class WorldConnection
{
public:
	WorldConnection(const std::string &key, uint32_t dbid, const std::string &host);
	~WorldConnection();
private:
	void OnNewConnection(std::shared_ptr<EQ::Net::ReliableStreamConnection> connection);
	void OnStatusChangeActive(std::shared_ptr<EQ::Net::ReliableStreamConnection> conn, EQ::Net::DbProtocolStatus from, EQ::Net::DbProtocolStatus to);
	void OnStatusChangeInactive(std::shared_ptr<EQ::Net::ReliableStreamConnection> conn, EQ::Net::DbProtocolStatus from, EQ::Net::DbProtocolStatus to);
	void OnPacketRecv(std::shared_ptr<EQ::Net::ReliableStreamConnection> conn, const EQ::Net::Packet &p);
	void Kill();
	void Start();

	void SendClientAuth();

	std::unique_ptr<EQ::Net::ReliableStreamConnectionManager> m_connection_manager;
	std::shared_ptr<EQ::Net::ReliableStreamConnection> m_connection;
	bool m_connecting;
	std::unique_ptr<EQ::Timer> m_connect_timer;

	std::string m_host;

	std::string m_key;
	uint32_t m_dbid;
};