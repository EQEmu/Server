/*#include "login.h"
#include "../common/eqemu_logsys.h"
#include <openssl/des.h>

const char* eqcrypt_block(const char *buffer_in, size_t buffer_in_sz, char* buffer_out, bool enc) {
	DES_key_schedule k;
	DES_cblock v;

	memset(&k, 0, sizeof(DES_key_schedule));
	memset(&v, 0, sizeof(DES_cblock));

	if (!enc && buffer_in_sz && buffer_in_sz % 8 != 0) {
		return nullptr;
	}

	DES_ncbc_encrypt((const unsigned char*)buffer_in, (unsigned char*)buffer_out, (long)buffer_in_sz, &k, &v, enc);
	return buffer_out;
}

LoginConnection::LoginConnection(const std::string &username, const std::string &password, const std::string &host, int host_port, const std::string &server)
{
	m_connecting = false;
	m_username = username;
	m_password = password;
	m_host = host;
	m_host_port = host_port;
	m_server = server;

	m_connection_manager.reset(new EQ::Net::DaybreakConnectionManager());

	m_connection_manager->OnNewConnection(std::bind(&LoginConnection::OnNewConnection, this, std::placeholders::_1));
	m_connection_manager->OnConnectionStateChange(std::bind(&LoginConnection::OnStatusChangeActive, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	m_connection_manager->OnPacketRecv(std::bind(&LoginConnection::OnPacketRecv, this, std::placeholders::_1, std::placeholders::_2));

	m_connection_manager->Connect(host, host_port);
}

LoginConnection::~LoginConnection()
{
}

void LoginConnection::OnNewConnection(std::shared_ptr<EQ::Net::DaybreakConnection> connection)
{
	m_connection = connection;
	Log.OutF(Logs::General, Logs::Headless_Client, "Connecting...");
}

void LoginConnection::OnStatusChangeActive(std::shared_ptr<EQ::Net::DaybreakConnection> conn, EQ::Net::DbProtocolStatus from, EQ::Net::DbProtocolStatus to)
{
	if (to == EQ::Net::StatusConnected) {
		Log.OutF(Logs::General, Logs::Headless_Client, "Login connected.");
		SendSessionReady();
	}

	if (to == EQ::Net::StatusDisconnected) {
		Log.OutF(Logs::General, Logs::Headless_Client, "Login connection lost, reconnecting.");
		m_key.clear();
		m_dbid = 0;
		m_connection.reset();
		m_connection_manager->Connect(m_host, m_host_port);
	}
}

void LoginConnection::OnStatusChangeInactive(std::shared_ptr<EQ::Net::DaybreakConnection> conn, EQ::Net::DbProtocolStatus from, EQ::Net::DbProtocolStatus to)
{
	if (to == EQ::Net::StatusDisconnected) {
		m_key.clear();
		m_dbid = 0;
		m_connection.reset();
	}
}

void LoginConnection::OnPacketRecv(std::shared_ptr<EQ::Net::DaybreakConnection> conn, const EQ::Net::Packet &p)
{
	auto opcode = p.GetUInt16(0);
	switch (opcode) {
	case 0x0017: //OP_ChatMessage
		SendLogin();
		break;
	case 0x0018:
		ProcessLoginResponse(p);
		break;
	case 0x0019:
		ProcessServerPacketList(p);
		break;
	case 0x0022:
		ProcessServerPlayResponse(p);
		break;
	}
}

void LoginConnection::Kill()
{
	m_connection_manager->OnConnectionStateChange(std::bind(&LoginConnection::OnStatusChangeInactive, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	m_key.clear();
	m_dbid = 0;
	m_connection->Close();
}

void LoginConnection::Start()
{
	m_connection_manager->OnConnectionStateChange(std::bind(&LoginConnection::OnStatusChangeActive, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	m_connection_manager->Connect(m_host, m_host_port);
}

void LoginConnection::SendSessionReady()
{
	EQ::Net::DynamicPacket p;
	p.PutUInt16(0, 1); //OP_SessionReady
	p.PutUInt32(2, 2);

	m_connection->QueuePacket(p);
}

void LoginConnection::SendLogin()
{
	size_t buffer_len = m_username.length() + m_password.length() + 2;
	std::unique_ptr<char[]> buffer(new char[buffer_len]);

	strcpy(&buffer[0], m_username.c_str());
	strcpy(&buffer[m_username.length() + 1], m_password.c_str());

	size_t encrypted_len = buffer_len;

	if (encrypted_len % 8 > 0) {
		encrypted_len = ((encrypted_len / 8) + 1) * 8;
	}

	EQ::Net::DynamicPacket p;
	p.Resize(12 + encrypted_len);
	p.PutUInt16(0, 2); //OP_Login
	p.PutUInt32(2, 3);

	eqcrypt_block(&buffer[0], buffer_len, (char*)p.Data() + 12, true);

	m_connection->QueuePacket(p);
}

void LoginConnection::SendServerRequest()
{
	EQ::Net::DynamicPacket p;
	p.PutUInt16(0, 4); //OP_ServerListRequest
	p.PutUInt32(2, 4);

	m_connection->QueuePacket(p);
}

void LoginConnection::SendPlayRequest(uint32_t id)
{
	EQ::Net::DynamicPacket p;
	p.PutUInt16(0, 0x000d);
	p.PutUInt16(2, 5);
	p.PutUInt32(4, 0);
	p.PutUInt32(8, 0);
	p.PutUInt32(12, id);

	m_connection->QueuePacket(p);
}

void LoginConnection::ProcessLoginResponse(const EQ::Net::Packet &p)
{
	auto encrypt_size = p.Length() - 12;
	if (encrypt_size % 8 > 0) {
		encrypt_size = (encrypt_size / 8) * 8;
	}

	std::unique_ptr<char[]> decrypted(new char[encrypt_size]);

	eqcrypt_block((char*)p.Data() + 12, encrypt_size, &decrypted[0], false);

	EQ::Net::StaticPacket sp(&decrypted[0], encrypt_size);
	auto response_error = sp.GetUInt16(1);

	if (response_error > 101) {
		Log.OutF(Logs::General, Logs::Headless_Client, "Error logging in response code: {0}", response_error);
		Kill();
	}
	else {
		m_key = sp.GetCString(12);
		m_dbid = sp.GetUInt32(8);

		Log.OutF(Logs::General, Logs::Headless_Client, "Logged in successfully with dbid {0} and key {1}", m_dbid, m_key);
		SendServerRequest();
	}
}

void LoginConnection::ProcessServerPacketList(const EQ::Net::Packet &p)
{
	m_world_servers.clear();
	auto number_of_servers = p.GetUInt32(18);
	size_t idx = 22;

	for (auto i = 0U; i < number_of_servers; ++i) {
		WorldServer ws;
		ws.address = p.GetCString(idx);
		idx += (ws.address.length() + 1);

		ws.type = p.GetInt32(idx);
		idx += 4;

		auto id = p.GetUInt32(idx);
		idx += 4;

		ws.long_name = p.GetCString(idx);
		idx += (ws.long_name.length() + 1);

		ws.lang = p.GetCString(idx);
		idx += (ws.lang.length() + 1);

		ws.region = p.GetCString(idx);
		idx += (ws.region.length() + 1);

		ws.status = p.GetInt32(idx);
		idx += 4;

		ws.players = p.GetInt32(idx);
		idx += 4;

		m_world_servers[id] = ws;
	}

	for (auto server : m_world_servers) {
		if (server.second.long_name.compare(m_server) == 0) {
			Log.OutF(Logs::General, Logs::Headless_Client, "Found world server {0}, attempting to login.", m_server);
			SendPlayRequest(server.first);
			return;
		}
	}

	Log.OutF(Logs::General, Logs::Headless_Client, "Got response from login server but could not find world server {0} disconnecting.", m_server);
	Kill();
}

void LoginConnection::ProcessServerPlayResponse(const EQ::Net::Packet &p)
{
	auto allowed = p.GetUInt8(12);

	if (allowed) {
		auto server = p.GetUInt32(18);
		auto ws = m_world_servers.find(server);
		if (ws != m_world_servers.end()) {
			if (m_on_can_login_world) {
				m_on_can_login_world(ws->second, m_key, m_dbid);
			}

			Kill();
		}
	}
	else {
		auto message = p.GetUInt16(13);
		Log.OutF(Logs::General, Logs::Headless_Client, "Failed to login to server with message {0}");
		Kill();
	}
}
*/