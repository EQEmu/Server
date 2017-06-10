#include "eq.h"
#include "../common/net/dns.h"

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

EverQuest::EverQuest(const std::string &host, int port, const std::string &user, const std::string &pass, const std::string &server, const std::string &character)
{
	m_host = host;
	m_port = port;
	m_user = user;
	m_pass = pass;
	m_server = server;
	m_character = character;
	m_dbid = 0;

	EQ::Net::DNSLookup(m_host, port, false, [&](const std::string &addr) {
		if (addr.empty()) {
			Log.OutF(Logs::General, Logs::Headless_Client, "Could not resolve address: {0}", m_host);
			return;
		}
		else {
			m_host = addr;
			m_login_connection_manager.reset(new EQ::Net::DaybreakConnectionManager());

			m_login_connection_manager->OnNewConnection(std::bind(&EverQuest::LoginOnNewConnection, this, std::placeholders::_1));
			m_login_connection_manager->OnConnectionStateChange(std::bind(&EverQuest::LoginOnStatusChangeReconnectEnabled, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
			m_login_connection_manager->OnPacketRecv(std::bind(&EverQuest::LoginOnPacketRecv, this, std::placeholders::_1, std::placeholders::_2));

			m_login_connection_manager->Connect(m_host, m_port);
		}
	});
}

EverQuest::~EverQuest()
{
}

void EverQuest::LoginOnNewConnection(std::shared_ptr<EQ::Net::DaybreakConnection> connection)
{
	m_login_connection = connection;
	Log.OutF(Logs::General, Logs::Headless_Client, "Connecting...");
}

void EverQuest::LoginOnStatusChangeReconnectEnabled(std::shared_ptr<EQ::Net::DaybreakConnection> conn, EQ::Net::DbProtocolStatus from, EQ::Net::DbProtocolStatus to)
{
	if (to == EQ::Net::StatusConnected) {
		Log.OutF(Logs::General, Logs::Headless_Client, "Login connected.");
		LoginSendSessionReady();
	}

	if (to == EQ::Net::StatusDisconnected) {
		Log.OutF(Logs::General, Logs::Headless_Client, "Login connection lost before we got to world, reconnecting.");
		m_key.clear();
		m_dbid = 0;
		m_login_connection.reset();
		m_login_connection_manager->Connect(m_host, m_port);
	}
}

void EverQuest::LoginOnStatusChangeReconnectDisabled(std::shared_ptr<EQ::Net::DaybreakConnection> conn, EQ::Net::DbProtocolStatus from, EQ::Net::DbProtocolStatus to)
{
	if (to == EQ::Net::StatusDisconnected) {
		m_login_connection.reset();
	}
}

void EverQuest::LoginOnPacketRecv(std::shared_ptr<EQ::Net::DaybreakConnection> conn, const EQ::Net::Packet & p)
{
	auto opcode = p.GetUInt16(0);
	switch (opcode) {
	case 0x0017: //OP_ChatMessage
		LoginSendLogin();
		break;
	case 0x0018:
		LoginProcessLoginResponse(p);
		break;
	case 0x0019:
		LoginProcessServerPacketList(p);
		break;
	case 0x0022:
		LoginProcessServerPlayResponse(p);
		break;
	}
}

void EverQuest::LoginSendSessionReady()
{
	EQ::Net::DynamicPacket p;
	p.PutUInt16(0, 1); //OP_SessionReady
	p.PutUInt32(2, 2);

	m_login_connection->QueuePacket(p);
}

void EverQuest::LoginSendLogin()
{
	size_t buffer_len = m_user.length() + m_pass.length() + 2;
	std::unique_ptr<char[]> buffer(new char[buffer_len]);

	strcpy(&buffer[0], m_user.c_str());
	strcpy(&buffer[m_user.length() + 1], m_pass.c_str());

	size_t encrypted_len = buffer_len;

	if (encrypted_len % 8 > 0) {
		encrypted_len = ((encrypted_len / 8) + 1) * 8;
	}

	EQ::Net::DynamicPacket p;
	p.Resize(12 + encrypted_len);
	p.PutUInt16(0, 2); //OP_Login
	p.PutUInt32(2, 3);

	eqcrypt_block(&buffer[0], buffer_len, (char*)p.Data() + 12, true);

	m_login_connection->QueuePacket(p);
}

void EverQuest::LoginSendServerRequest()
{
	EQ::Net::DynamicPacket p;
	p.PutUInt16(0, 4); //OP_ServerListRequest
	p.PutUInt32(2, 4);

	m_login_connection->QueuePacket(p);
}

void EverQuest::LoginSendPlayRequest(uint32_t id)
{
	EQ::Net::DynamicPacket p;
	p.PutUInt16(0, 0x000d);
	p.PutUInt16(2, 5);
	p.PutUInt32(4, 0);
	p.PutUInt32(8, 0);
	p.PutUInt32(12, id);

	m_login_connection->QueuePacket(p);
}

void EverQuest::LoginProcessLoginResponse(const EQ::Net::Packet & p)
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
		LoginDisableReconnect();
	}
	else {
		m_key = sp.GetCString(12);
		m_dbid = sp.GetUInt32(8);

		Log.OutF(Logs::General, Logs::Headless_Client, "Logged in successfully with dbid {0} and key {1}", m_dbid, m_key);
		LoginSendServerRequest();
	}
}

void EverQuest::LoginProcessServerPacketList(const EQ::Net::Packet & p)
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
			LoginSendPlayRequest(server.first);
			return;
		}
	}

	Log.OutF(Logs::General, Logs::Headless_Client, "Got response from login server but could not find world server {0} disconnecting.", m_server);
	LoginDisableReconnect();
}

void EverQuest::LoginProcessServerPlayResponse(const EQ::Net::Packet &p)
{
	auto allowed = p.GetUInt8(12);

	if (allowed) {
		auto server = p.GetUInt32(18);
		auto ws = m_world_servers.find(server);
		if (ws != m_world_servers.end()) {
			ConnectToWorld();
			LoginDisableReconnect();
		}
	}
	else {
		auto message = p.GetUInt16(13);
		Log.OutF(Logs::General, Logs::Headless_Client, "Failed to login to server with message {0}");
		LoginDisableReconnect();
	}
}

void EverQuest::LoginDisableReconnect()
{
	m_login_connection_manager->OnConnectionStateChange(std::bind(&EverQuest::LoginOnStatusChangeReconnectDisabled, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	m_login_connection->Close();
}

void EverQuest::ConnectToWorld()
{
	m_world_connection_manager.reset(new EQ::Net::DaybreakConnectionManager());
	m_world_connection_manager->OnNewConnection(std::bind(&EverQuest::WorldOnNewConnection, this, std::placeholders::_1));
	m_world_connection_manager->OnConnectionStateChange(std::bind(&EverQuest::WorldOnStatusChangeReconnectEnabled, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	m_world_connection_manager->OnPacketRecv(std::bind(&EverQuest::WorldOnPacketRecv, this, std::placeholders::_1, std::placeholders::_2));
	m_world_connection_manager->Connect(m_host, 9000);
}

void EverQuest::WorldOnNewConnection(std::shared_ptr<EQ::Net::DaybreakConnection> connection)
{
	m_world_connection = connection;
	Log.OutF(Logs::General, Logs::Headless_Client, "Connecting to world...");
}

void EverQuest::WorldOnStatusChangeReconnectEnabled(std::shared_ptr<EQ::Net::DaybreakConnection> conn, EQ::Net::DbProtocolStatus from, EQ::Net::DbProtocolStatus to)
{
	if (to == EQ::Net::StatusConnected) {
		Log.OutF(Logs::General, Logs::Headless_Client, "World connected.");
		WorldSendClientAuth();
	}

	if (to == EQ::Net::StatusDisconnected) {
		Log.OutF(Logs::General, Logs::Headless_Client, "World connection lost, reconnecting.");
		m_world_connection.reset();
		m_world_connection_manager->Connect(m_host, 9000);
	}
}

void EverQuest::WorldOnStatusChangeReconnectDisabled(std::shared_ptr<EQ::Net::DaybreakConnection> conn, EQ::Net::DbProtocolStatus from, EQ::Net::DbProtocolStatus to)
{
	if (to == EQ::Net::StatusDisconnected) {
		m_world_connection.reset();
	}
}

void EverQuest::WorldOnPacketRecv(std::shared_ptr<EQ::Net::DaybreakConnection> conn, const EQ::Net::Packet & p)
{
	auto opcode = p.GetUInt16(0);
	switch (opcode) {
	case 0x00d2:
		WorldProcessCharacterSelect(p);
		break;
	default:
		Log.OutF(Logs::General, Logs::Headless_Client, "Unhandled opcode: {0:#x}", opcode);
		break;
	}
}

void EverQuest::WorldSendClientAuth()
{
	EQ::Net::DynamicPacket p;
	p.Resize(2 + 464);

	p.PutUInt16(0, 0x7a09U);
	std::string dbid_str = std::to_string(m_dbid);

	p.PutCString(2, dbid_str.c_str());
	p.PutCString(2 + dbid_str.length() + 1, m_key.c_str());

	m_world_connection->QueuePacket(p);
}

void EverQuest::WorldSendEnterWorld(const std::string &character)
{
	EQ::Net::DynamicPacket p;
	p.PutUInt16(0, 0x578f);
	p.PutString(2, character);
	p.PutUInt32(66, 0);
	p.PutUInt32(70, 0);

	m_world_connection->QueuePacket(p);
}

void EverQuest::WorldProcessCharacterSelect(const EQ::Net::Packet &p)
{
	auto char_count = p.GetUInt32(2);
	size_t idx = 6;

	//Log.OutF(Logs::General, Logs::Headless_Client, "{0} characters", char_count);
	for (uint32_t i = 0; i < char_count; ++i) {
		auto name = p.GetCString(idx);
		idx += name.length() + 1;

		auto pclass = p.GetUInt8(idx);
		auto prace = p.GetUInt32(idx + 1);
		auto plevel = p.GetUInt8(idx + 5);

		idx += 274;
		if (m_character.compare(name) == 0) {
			Log.OutF(Logs::General, Logs::Headless_Client, "Found {0}, would attempt to login here.", m_character);
			WorldSendEnterWorld(m_character);
			return;
		}
	}

	Log.OutF(Logs::General, Logs::Headless_Client, "Could not find {0}, cannot continue to login.", m_character);
}
 