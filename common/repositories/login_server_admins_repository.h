#ifndef EQEMU_LOGIN_SERVER_ADMINS_REPOSITORY_H
#define EQEMU_LOGIN_SERVER_ADMINS_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_login_server_admins_repository.h"

class LoginServerAdminsRepository : public BaseLoginServerAdminsRepository {
public:
	static LoginServerAdmins GetByName(Database &db, std::string account_name)
	{
		auto admins = GetWhere(
			db,
			fmt::format(
				"account_name = '{}' LIMIT 1",
				Strings::Escape(account_name)
			)
		);

		if (admins.size() == 1) {
			return admins.front();
		}

		return NewEntity();
	}
};

#endif //EQEMU_LOGIN_SERVER_ADMINS_REPOSITORY_H
