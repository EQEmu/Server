#ifndef EQEMU_CLIENT_H
#define EQEMU_CLIENT_H

#include "../common/global_define.h"
#include "../common/opcodemgr.h"
#include "../common/random.h"
#include "../common/eq_stream_intf.h"
#include "../common/net/dns.h"
#include "../common/net/daybreak_connection.h"
#include "login_types.h"
#include <memory>

class Client {
public:
	Client(std::shared_ptr<EQStreamInterface> c, LSClientVersion v);
	~Client() {}
	bool Process();
	void HandleSessionReady(const char *data, unsigned int size);
	void HandleLogin(const char *data, unsigned int size);

	// Sends the expansion data packet
	// Titanium uses the encrypted data block to contact the expansion (You own xxx:) and the max expansions (of yyy)
	// Rof uses a separate data packet specifically for the expansion data
	// Live, as of July 2021 uses a similar but slightly different seperate data packet
	void SendExpansionPacketData(PlayerLoginReply &plrs);
	void SendPlayToWorld(const char *data);
	void SendServerListPacket(uint32 seq);
	void SendPlayResponse(EQApplicationPacket *outapp);
	void GenerateRandomLoginKey();
	unsigned int GetAccountID() const { return m_account_id; }
	std::string GetLoginServerName() const { return m_loginserver_name; }
	std::string GetAccountName() const { return m_account_name; }
	std::string GetClientLoggingDescription();
	std::string GetLoginKey() const { return m_key; }
	unsigned int GetSelectedPlayServerID() const { return m_selected_play_server_id; }
	unsigned int GetCurrentPlaySequence() const { return m_play_sequence_id; }
	LSClientVersion GetClientVersion() const { return m_client_version; }
	std::shared_ptr<EQStreamInterface> GetConnection() { return m_connection; }

	void AttemptLoginAccountCreation(const std::string &user, const std::string &pass, const std::string &loginserver);

	void SendFailedLogin();

	bool VerifyAndUpdateLoginHash(
		const std::string &account_username,
		const std::string &source_loginserver,
		const std::string &account_password,
		const std::string &password_hash
	);

	void DoSuccessfulLogin(const std::string &in_account_name, int db_account_id, const std::string &db_loginserver);

private:
	EQ::Random                                          m_random;
	std::shared_ptr<EQStreamInterface>                  m_connection;
	LSClientVersion                                     m_client_version;
	LSClientStatus                                      m_client_status;
	std::string                                         m_account_name;
	unsigned int                                        m_account_id;
	std::string                                         m_loginserver_name;
	unsigned int                                        m_selected_play_server_id;
	unsigned int                                        m_play_sequence_id;
	std::string                                         m_key;
	std::unique_ptr<EQ::Net::DaybreakConnectionManager> m_login_connection_manager;
	std::shared_ptr<EQ::Net::DaybreakConnection>        m_login_connection;
	LoginBaseMessage                                    m_login_base_message;
	std::string                                         m_stored_username;
	std::string                                         m_stored_password;
	static bool ProcessHealthCheck(std::string username) {
		return username == "healthcheckuser";
	}
};

#endif

