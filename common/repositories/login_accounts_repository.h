#ifndef EQEMU_LOGIN_ACCOUNTS_REPOSITORY_H
#define EQEMU_LOGIN_ACCOUNTS_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_login_accounts_repository.h"
#include "../../loginserver/login_types.h"

class LoginAccountsRepository : public BaseLoginAccountsRepository {
public:
	static int64 GetFreeID(Database &db, const std::string &loginserver)
	{
		auto query = fmt::format(
			"SELECT IFNULL(MAX(id), 0) + 1 FROM login_accounts WHERE source_loginserver = '{}'",
			Strings::Escape(loginserver)
		);

		auto results = db.QueryDatabase(query);
		if (!results.Success() || results.RowCount() != 1) {
			return 0;
		}

		auto row = results.begin();

		return Strings::ToUnsignedInt(row[0]);
	}

	static LoginAccountsRepository::LoginAccounts SaveAccountFromContext(
		Database &db,
		LoginAccountContext c
	)
	{
		auto a = LoginAccountsRepository::NewEntity();
		a.id				 = GetFreeID(db, c.source_loginserver);
		a.account_name       = c.username;
		a.account_password   = c.encrypted_password;
		a.account_email      = !c.email.empty() ? c.email : "local_creation";
		a.source_loginserver = c.source_loginserver;
		a.last_ip_address    = "127.0.0.1";
		a.last_login_date    = std::time(nullptr);
		a.created_at         = std::time(nullptr);
		LoginAccountsRepository::InsertOne(db, a);

		return GetAccountFromContext(db, c).id > 0 ? a : NewEntity();
	}

	static LoginAccountsRepository::LoginAccounts GetAccountFromContext(
		Database &db,
		LoginAccountContext c
	)
	{
		auto results = LoginAccountsRepository::GetWhere(
			db,
			fmt::format(
				"account_name = '{}' AND source_loginserver = '{}'",
				c.username,
				c.source_loginserver
			)
		);

		auto e = LoginAccountsRepository::NewEntity();
		if (results.size() == 1) {
			e = results.front();
		}

		return e;
	}
};

#endif //EQEMU_LOGIN_ACCOUNTS_REPOSITORY_H
