#pragma once

#include "../common/net/daybreak_connection.h"
#include "../common/event/timer.h"
#include <map>

struct WorldServer
{
	std::string long_name;
	std::string address;
	int type;
	std::string lang;
	std::string region;
	int status;
	int players;
};

class LoginConnection
{
public:
	LoginConnection(const std::string &username, const std::string &password, const std::string &host, int host_port, const std::string &server);
	void OnCanLoginToWorld(std::function<void(const WorldServer&, const std::string&, uint32_t)> cb) { m_on_can_login_world = cb; }

	~LoginConnection();
private:
	void OnNewConnection(std::shared_ptr<EQ::Net::DaybreakConnection> connection);
	void OnStatusChangeActive(std::shared_ptr<EQ::Net::DaybreakConnection> conn, EQ::Net::DbProtocolStatus from, EQ::Net::DbProtocolStatus to);
	void OnStatusChangeInactive(std::shared_ptr<EQ::Net::DaybreakConnection> conn, EQ::Net::DbProtocolStatus from, EQ::Net::DbProtocolStatus to);
	void OnPacketRecv(std::shared_ptr<EQ::Net::DaybreakConnection> conn, const EQ::Net::Packet &p);
	void Kill();
	void Start();

	void SendSessionReady();
	void SendLogin();
	void SendServerRequest();
	void SendPlayRequest(uint32_t id);
	void ProcessLoginResponse(const EQ::Net::Packet &p);
	void ProcessServerPacketList(const EQ::Net::Packet &p);
	void ProcessServerPlayResponse(const EQ::Net::Packet &p);

	std::unique_ptr<EQ::Net::DaybreakConnectionManager> m_connection_manager;
	std::shared_ptr<EQ::Net::DaybreakConnection> m_connection;
	bool m_connecting;
	std::unique_ptr<EQ::Timer> m_connect_timer;

	std::string m_username;
	std::string m_password;
	std::string m_host;
	int m_host_port;
	std::string m_server;

	std::string m_key;
	uint32_t m_dbid;
	std::map<uint32_t, WorldServer> m_world_servers;
	std::function<void(const WorldServer&, const std::string&, uint32_t)> m_on_can_login_world;
};