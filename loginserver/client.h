#ifndef EQEMU_CLIENT_H
#define EQEMU_CLIENT_H

#include "../common/global_define.h"
#include "../common/opcodemgr.h"
#include "../common/random.h"
#include "../common/eq_stream_intf.h"
#include "../common/net/dns.h"
#include "../common/net/daybreak_connection.h"
#include "login_structures.h"
#include <memory>

enum LSClientVersion {
	cv_titanium,
	cv_sod
};

enum LSClientStatus {
	cs_not_sent_session_ready,
	cs_waiting_for_login,
	cs_creating_account,
	cs_failed_to_login,
	cs_logged_in
};

namespace LS {
	namespace ServerStatusFlags {
		enum eServerStatusFlags {
			Up     = 0, // default
			Down   = 1,
			Unused = 2,
			Locked = 4  // can be combined with Down to show "Locked (Down)"
		};
	}

	namespace ServerTypeFlags {
		enum eServerTypeFlags {
			None      = 0,
			Standard  = 1,
			Unknown2  = 2,
			Unknown4  = 4,
			Preferred = 8,
			Legends   = 16 // can be combined with Preferred flag to override color in Legends section with Preferred color (green)
		};
	}
}

/**
 * Client class, controls a single client and it's connection to the login server
 */
class Client {
public:

	/**
	 * Constructor, sets our connection to c and version to v
	 *
	 * @param c
	 * @param v
	 */
	Client(std::shared_ptr<EQStreamInterface> c, LSClientVersion v);

	/**
	 * Destructor
	 */
	~Client() {}

	/**
	 * Processes the client's connection and does various actions
	 *
	 * @return
	 */
	bool Process();

	/**
	 * Sends our reply to session ready packet
	 *
	 * @param data
	 * @param size
	 */
	void Handle_SessionReady(const char *data, unsigned int size);

	/**
	 * Verifies login and send a reply
	 *
	 * @param data
	 * @param size
	 */
	void Handle_Login(const char *data, unsigned int size);

	/**
	 * Sends a packet to the requested server to see if the client is allowed or not
	 *
	 * @param data
	 */
	void Handle_Play(const char *data);

	/**
	 * Sends a server list packet to the client
	 *
	 * @param seq
	 */
	void SendServerListPacket(uint32 seq);

	/**
	 * Sends the input packet to the client and clears our play response states
	 *
	 * @param outapp
	 */
	void SendPlayResponse(EQApplicationPacket *outapp);

	/**
	 * Generates a random login key for the client during login
	 */
	void GenerateKey();

	/**
	 * Gets the account id of this client
	 *
	 * @return
	 */
	unsigned int GetAccountID() const { return m_account_id; }

	/**
	 * Gets the loginserver name of this client
	 *
	 * @return
	 */
	std::string GetLoginServerName() const { return m_loginserver_name; }

	/**
	 * Gets the account name of this client
	 *
	 * @return
	 */
	std::string GetAccountName() const { return m_account_name; }

	/**
	 * Gets the key generated at login for this client
	 *
	 * @return
	 */
	std::string GetKey() const { return m_key; }

	/**
	 * Gets the server selected to be played on for this client
	 *
	 * @return
	 */
	unsigned int GetPlayServerID() const { return m_play_server_id; }

	/**
	 * Gets the play sequence state for this client
	 *
	 * @return
	 */
	unsigned int GetPlaySequence() const { return m_play_sequence_id; }

	/**
	 * Gets the connection for this client
	 *
	 * @return
	 */
	std::shared_ptr<EQStreamInterface> GetConnection() { return m_connection; }

	/**
	 * Attempts to create a login account
	 *
	 * @param user
	 * @param pass
	 * @param loginserver
	 */
	void AttemptLoginAccountCreation(const std::string &user, const std::string &pass, const std::string &loginserver);

	/**
	 * Does a failed login
	 */
	void DoFailedLogin();

	/**
	 * Verifies a login hash, will also attempt to update a login hash if needed
	 *
	 * @param account_username
	 * @param source_loginserver
	 * @param account_password
	 * @param password_hash
	 * @return
	 */
	bool VerifyLoginHash(
		const std::string &account_username,
		const std::string &source_loginserver,
		const std::string &account_password,
		const std::string &password_hash
	);

	void DoSuccessfulLogin(const std::string in_account_name, int db_account_id, const std::string &db_loginserver);
	void CreateLocalAccount(const std::string &username, const std::string &password);
	void CreateEQEmuAccount(const std::string &in_account_name, const std::string &in_account_password, unsigned int loginserver_account_id);

private:
	EQ::Random                         m_random;
	std::shared_ptr<EQStreamInterface> m_connection;
	LSClientVersion                    m_client_version;
	LSClientStatus                     m_client_status;

	std::string  m_account_name;
	unsigned int m_account_id;
	std::string  m_loginserver_name;
	unsigned int m_play_server_id;
	unsigned int m_play_sequence_id;
	std::string  m_key;

	std::unique_ptr<EQ::Net::DaybreakConnectionManager> m_login_connection_manager;
	std::shared_ptr<EQ::Net::DaybreakConnection>        m_login_connection;
	LoginBaseMessage_Struct                             m_llrs;

	std::string m_stored_user;
	std::string m_stored_pass;
	void LoginOnNewConnection(std::shared_ptr<EQ::Net::DaybreakConnection> connection);
	void LoginOnStatusChange(
		std::shared_ptr<EQ::Net::DaybreakConnection> conn,
		EQ::Net::DbProtocolStatus from,
		EQ::Net::DbProtocolStatus to
	);
	void LoginOnStatusChangeIgnored(
		std::shared_ptr<EQ::Net::DaybreakConnection> conn,
		EQ::Net::DbProtocolStatus from,
		EQ::Net::DbProtocolStatus to
	);
	void LoginOnPacketRecv(std::shared_ptr<EQ::Net::DaybreakConnection> conn, const EQ::Net::Packet &p);
	void LoginSendSessionReady();
	void LoginSendLogin();
	void LoginProcessLoginResponse(const EQ::Net::Packet &p);
	static bool ProcessHealthCheck(std::string username);
};

#endif

