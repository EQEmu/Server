#ifndef EQEMU_DATABASEMYSQL_H
#define EQEMU_DATABASEMYSQL_H

#include "../common/dbcore.h"
#include "../common/eqemu_logsys.h"

#include <string>
#include <sstream>
#include <stdlib.h>
#include <mysql.h>

class LoginDatabase : public DBcore {
public:

	LoginDatabase() { m_database = nullptr; }
	LoginDatabase(std::string user, std::string pass, std::string host, std::string port, std::string name);
	~LoginDatabase();

	bool GetLoginDataFromAccountInfo(
		const std::string &name,
		const std::string &loginserver,
		std::string &password,
		unsigned int &id
	);

	bool GetLoginTokenDataFromToken(
		const std::string &token,
		const std::string &ip,
		unsigned int &db_account_id,
		std::string &db_loginserver,
		std::string &user
	);

	unsigned int GetFreeID(const std::string &loginserver);

	bool CreateLoginData(
		const std::string &name,
		const std::string &password,
		const std::string &loginserver,
		unsigned int &id
	);

	bool CreateLoginDataWithID(
		const std::string &in_account_name,
		const std::string &in_account_password,
		const std::string &loginserver,
		unsigned int id
	);

	void UpdateLoginserverAccountPasswordHash(
		const std::string &name,
		const std::string &loginserver,
		const std::string &hash
	);

	bool DoesLoginServerAccountExist(
		const std::string &name,
		const std::string &password,
		const std::string &loginserver,
		unsigned int id
	);

	struct DbWorldRegistration {
		bool        loaded            = false;
		int32       server_id         = 0;
		int8        server_list_type  = 3;
		bool        is_server_trusted = false;
		std::string server_description;
		std::string server_list_description;
		std::string server_admin_account_name;
		std::string server_admin_account_password;
		uint32      server_admin_id;
	};

	LoginDatabase::DbWorldRegistration GetWorldRegistration(
		const std::string &short_name,
		const std::string &long_name,
		uint32 login_world_server_admin_id
	);
	void UpdateLSAccountData(unsigned int id, std::string ip_address);
	void UpdateLSAccountInfo(unsigned int id, std::string name, std::string password, std::string email);
	void UpdateWorldRegistration(unsigned int id, std::string long_name, std::string ip_address);
	bool CreateWorldRegistration(
		std::string server_long_name,
		std::string server_short_name,
		std::string server_remote_ip,
		unsigned int &id,
		unsigned int &server_admin_id
	);
	std::string CreateLoginserverApiToken(bool write_mode, bool read_mode);
	MySQLRequestResult GetLoginserverApiTokens();

	uint32 CreateLoginserverWorldAdminAccount(
		const std::string &account_name,
		const std::string &account_password,
		const std::string &first_name,
		const std::string &last_name,
		const std::string &email,
		const std::string &ip_address
	);

	bool DoesLoginserverWorldAdminAccountExist(const std::string &account_name);

	struct DbLoginServerAdmin {
		bool        loaded = false;
		uint32      id;
		std::string account_name;
		std::string account_password;
		std::string first_name;
		std::string last_name;
		std::string email;
		std::string registration_date;
		std::string registration_ip_address;
	};

	LoginDatabase::DbLoginServerAdmin GetLoginServerAdmin(const std::string &account_name);

	struct DbLoginServerAccount {
		bool        loaded = false;
		uint32      id;
		std::string account_name;
		std::string account_password;
		std::string account_email;
		std::string source_loginserver;
		std::string last_login_date;
		std::string last_ip_address;
		std::string created_at;
		std::string updated_at;
	};

	LoginDatabase::DbLoginServerAccount GetLoginServerAccountByAccountName(
		const std::string &account_name,
		const std::string &source_loginserver = "local"
	);

	bool UpdateLoginWorldAdminAccountPassword(
		unsigned int id,
		const std::string &admin_account_password_hash
	);

	bool UpdateLoginWorldAdminAccountPasswordByUsername(
		const std::string &admin_account_username,
		const std::string &admin_account_password_hash
	);

	uint32 CreateLoginAccount(
		const std::string &name,
		const std::string &password,
		const std::string &loginserver = "local",
		const std::string &email = "local_creation"
	);

protected:
	MYSQL *m_database{};
};

#endif

