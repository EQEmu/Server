#include <stdio.h>
#include <string.h>
#include <openssl/des.h>
#include <memory>

#include "../common/eqemu_logsys.h"
#include "../common/json_config.h"
#include "../common/net/daybreak_connection.h"
#include "../common/net/dns.h"
#include "../common/event/event_loop.h"
#include "../common/event/timer.h"

EQEmuLogSys LogSys;

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

bool connected = false;
bool running = true;
std::unique_ptr<EQ::Net::DaybreakConnectionManager> m_login_connection_manager;
std::shared_ptr<EQ::Net::DaybreakConnection> m_login_connection;
std::string m_user;
std::string m_pass;

void LoginSendSessionReady()
{
	EQ::Net::DynamicPacket p;
	p.PutUInt16(0, 1); //OP_SessionReady
	p.PutUInt32(2, 2);

	m_login_connection->QueuePacket(p);
}

void LoginSendLogin()
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

void LoginProcessLoginResponse(const EQ::Net::Packet & p)
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
		printf("Error logging in response code: %u\n", response_error);
		m_login_connection->Close();
	}
	else {
		auto m_dbid = sp.GetUInt32(8);

		printf("Logged in successfully with dbid %u", m_dbid);
		m_login_connection->Close();
	}
}

void LoginOnNewConnection(std::shared_ptr<EQ::Net::DaybreakConnection> connection)
{
	m_login_connection = connection;
}

void LoginOnStatusChange(std::shared_ptr<EQ::Net::DaybreakConnection> conn, EQ::Net::DbProtocolStatus from, EQ::Net::DbProtocolStatus to)
{
	if (to == EQ::Net::StatusConnected) {
		connected = true;
		LoginSendSessionReady();
	}

	if (to == EQ::Net::StatusDisconnected) {
		running = false;
	}
}

void LoginOnPacketRecv(std::shared_ptr<EQ::Net::DaybreakConnection> conn, const EQ::Net::Packet &p) {
	auto opcode = p.GetUInt16(0);
	switch (opcode) {
	case 0x0017: //OP_ChatMessage
		LoginSendLogin();
		break;
	case 0x0018:
		LoginProcessLoginResponse(p);
		break;
	}
}

int main(int argc, char **argv) {
	LogSys.LoadLogSettingsDefaults();

	if (argc != 3) {
		printf("Usage: lp [username] [password]\n");
		return -1;
	}

	m_user = argv[1];
	m_pass = argv[2];

	std::string address;
	int port;
	EQ::Timer t(5000U, false, [&](EQ::Timer *timer) {
		if (!connected) {
			printf("Connection failure.\n");
			running = false;
		}
	});

	try {
		auto config = EQ::JsonConfigFile::Load("eqhost.json");
		address = config.GetVariableString("login", "address", "login.eqemulator.net");
		port = config.GetVariableInt("login", "port", 5999);
	}
	catch (std::exception) {
		printf("Unable to parse eqhost.json\n");
		return -1;
	}

	EQ::Net::DNSLookup(address, port, false, [&](const std::string &addr) {
		if (addr.empty()) {
			printf("DNS Error for %s\n", address.c_str());
			running = false;
			return;
		}

		m_login_connection_manager.reset(new EQ::Net::DaybreakConnectionManager());

		m_login_connection_manager->OnNewConnection(std::bind(&LoginOnNewConnection, std::placeholders::_1));
		m_login_connection_manager->OnConnectionStateChange(std::bind(&LoginOnStatusChange, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		m_login_connection_manager->OnPacketRecv(std::bind(&LoginOnPacketRecv, std::placeholders::_1, std::placeholders::_2));

		m_login_connection_manager->Connect(addr, port);
	});
	
	while (running) {
		EQ::EventLoop::Get().Process();
	}

	return 0;
}