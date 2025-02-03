#include "../common/global_define.h"

#include "database.h"
#include "login_server.h"
#include "../common/eqemu_logsys.h"
#include "../common/strings.h"
#include "../common/util/uuid.h"

extern LoginServer server;

LoginDatabase::LoginDatabase(
	std::string user,
	std::string pass,
	std::string host,
	std::string port,
	std::string name
)
{
	uint32 errnum = 0;
	char   errbuf[MYSQL_ERRMSG_SIZE];
	if (!Open(
		host.c_str(),
		user.c_str(),
		pass.c_str(),
		name.c_str(),
		Strings::ToUnsignedInt(port),
		&errnum,
		errbuf
	)
		) {
		LogError("Failed to connect to database: Error: [{}]", errbuf);
		exit(1);
	}
	else {
		LogInfo("Using database [{}] at [{}:{}]", name, host, port);
	}
}

LoginDatabase::~LoginDatabase()
{
	if (m_database) {
		mysql_close(m_database);
	}
}

bool LoginDatabase::GetLoginTokenDataFromToken(
	const std::string &token,
	const std::string &ip,
	unsigned int &db_account_id,
	std::string &db_loginserver,
	std::string &user
)
{
	auto query = fmt::format("SELECT login_server, username, account_id FROM login_tickets WHERE expires > NOW()"
		" AND id='{}' AND ip_address='{}' LIMIT 1",
		Strings::Escape(token),
		Strings::Escape(ip));

	auto results = QueryDatabase(query);
	if (results.RowCount() == 0 || !results.Success()) {
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		db_loginserver = row[0];
		user = row[1];
		db_account_id = Strings::ToUnsignedInt(row[2]);

		return true;
	}

	return false;
}

LoginDatabase::DbWorldRegistration LoginDatabase::GetWorldRegistration(
	const std::string &short_name,
	const std::string &long_name,
	uint32 login_world_server_admin_id
)
{
	auto query = fmt::format(
		"SELECT\n"
		"  WSR.id,\n"
		"  WSR.tag_description,\n"
		"  WSR.is_server_trusted,\n"
		"  SLT.id,\n"
		"  SLT.description,\n"
		"  ifnull(WSR.login_server_admin_id, 0) AS login_server_admin_id\n"
		"FROM\n"
		"  login_world_servers AS WSR\n"
		"  JOIN login_server_list_types AS SLT ON WSR.login_server_list_type_id = SLT.id\n"
		"WHERE\n"
		"  WSR.short_name = '{}' AND WSR.long_name = '{}' AND WSR.login_server_admin_id = {} LIMIT 1",
		Strings::Escape(short_name),
		Strings::Escape(long_name),
		login_world_server_admin_id
	);

	LoginDatabase::DbWorldRegistration r{};

	auto results = QueryDatabase(query);
	if (!results.Success() || results.RowCount() != 1) {
		return r;
	}

	auto row = results.begin();

	r.loaded                  = true;
	r.server_id               = Strings::ToInt(row[0]);
	r.server_description      = row[1];
	r.server_list_type        = Strings::ToInt(row[3]);
	r.is_server_trusted       = Strings::ToInt(row[2]) > 0;
	r.server_list_description = row[4];
	r.server_admin_id         = Strings::ToUnsignedInt(row[5]);

	if (r.server_admin_id <= 0) {
		return r;
	}

	auto world_registration_query = fmt::format(
		"SELECT account_name, account_password FROM login_server_admins WHERE id = {} LIMIT 1",
		r.server_admin_id
	);

	auto world_registration_results = QueryDatabase(world_registration_query);
	if (world_registration_results.Success() && world_registration_results.RowCount() == 1) {
		auto world_registration_row = world_registration_results.begin();
		r.server_admin_account_name     = world_registration_row[0];
		r.server_admin_account_password = world_registration_row[1];
	}

	return r;
}

void LoginDatabase::UpdateWorldRegistration(unsigned int id, std::string long_name, std::string ip_address)
{
	auto query = fmt::format(
		"UPDATE login_world_servers SET last_login_date = NOW(), last_ip_address = '{}', long_name = '{}' WHERE id = {}",
		ip_address,
		Strings::Escape(long_name),
		id
	);

	QueryDatabase(query);
}

bool LoginDatabase::UpdateLoginWorldAdminAccountPassword(
	unsigned int id,
	const std::string &admin_account_password_hash
)
{
	auto results = QueryDatabase(
		fmt::format(
			"UPDATE login_server_admins SET account_password = '{}' WHERE id = {}",
			Strings::Escape(admin_account_password_hash),
			id
		)
	);

	return results.Success();
}

bool LoginDatabase::UpdateLoginWorldAdminAccountPasswordByUsername(
	const std::string &admin_account_username,
	const std::string &admin_account_password_hash
)
{
	auto results = QueryDatabase(
		fmt::format(
			"UPDATE login_server_admins SET account_password = '{}' WHERE account_name = '{}'",
			Strings::Escape(admin_account_password_hash),
			Strings::Escape(admin_account_username)
		)
	);

	return results.Success();
}

bool LoginDatabase::CreateWorldRegistration(
	std::string server_long_name,
	std::string server_short_name,
	std::string server_remote_ip,
	unsigned int &id,
	unsigned int &server_admin_id
)
{
	auto results = QueryDatabase("SELECT IFNULL(max(id), 0) + 1 FROM login_world_servers");
	if (!results.Success() || results.RowCount() != 1) {
		return false;
	}

	auto row = results.begin();

	id = Strings::ToUnsignedInt(row[0]);
	auto insert_query = fmt::format(
		"INSERT INTO login_world_servers SET id = {}, long_name = '{}', short_name = '{}', last_ip_address = '{}', \n"
		"login_server_list_type_id = 3, login_server_admin_id = {}, is_server_trusted = 0, tag_description = ''",
		id,
		Strings::Escape(server_long_name),
		Strings::Escape(server_short_name),
		server_remote_ip,
		server_admin_id
	);

	auto insert_results = QueryDatabase(insert_query);
	if (!insert_results.Success()) {
		LogError(
			"Failed to register world server {} - {}",
			server_long_name,
			server_short_name
		);

		return false;
	}

	return true;
}

uint32 LoginDatabase::CreateLoginserverWorldAdminAccount(
	const std::string &account_name,
	const std::string &account_password,
	const std::string &first_name,
	const std::string &last_name,
	const std::string &email,
	const std::string &ip_address
)
{
	auto query = fmt::format(
		"INSERT INTO login_server_admins (account_name, account_password, first_name, last_name, email, registration_date, "
		"registration_ip_address) "
		"VALUES ('{}', '{}', '{}', '{}', '{}', NOW(), '{}')",
		Strings::Escape(account_name),
		Strings::Escape(account_password),
		Strings::Escape(first_name),
		Strings::Escape(last_name),
		Strings::Escape(email),
		ip_address
	);

	auto results = QueryDatabase(query);

	return (results.Success() ? results.LastInsertedID() : 0);
}

bool LoginDatabase::DoesLoginserverWorldAdminAccountExist(
	const std::string &account_name
)
{
	auto query = fmt::format(
		"SELECT account_name FROM login_server_admins WHERE account_name = '{}' LIMIT 1",
		Strings::Escape(account_name)
	);

	auto results = QueryDatabase(query);

	return (results.RowCount() == 1);
}

LoginDatabase::DbLoginServerAdmin LoginDatabase::GetLoginServerAdmin(const std::string &account_name)
{
	auto query = fmt::format(
		"SELECT id, account_name, account_password, first_name, last_name, email, registration_date, registration_ip_address"
		" FROM login_server_admins WHERE account_name = '{}' LIMIT 1",
		Strings::Escape(account_name)
	);

	auto results = QueryDatabase(query);

	LoginDatabase::DbLoginServerAdmin r{};
	if (results.RowCount() == 1) {
		auto row = results.begin();
		r.loaded                  = true;
		r.id                      = Strings::ToUnsignedInt(row[0]);
		r.account_name            = row[1];
		r.account_password        = row[2];
		r.first_name              = row[3];
		r.last_name               = row[4];
		r.email                   = row[5];
		r.registration_date       = row[7];
		r.registration_ip_address = row[8];
	}

	return r;
}
