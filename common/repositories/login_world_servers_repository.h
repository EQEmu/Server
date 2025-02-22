#ifndef EQEMU_LOGIN_WORLD_SERVERS_REPOSITORY_H
#define EQEMU_LOGIN_WORLD_SERVERS_REPOSITORY_H

#include "../database.h"
#include "../strings.h"
#include "base/base_login_world_servers_repository.h"

class LoginWorldServersRepository: public BaseLoginWorldServersRepository {
public:
	static LoginWorldServers GetFromWorldContext(Database &db, LoginWorldContext c) {
		std::string where = fmt::format(
			"short_name = '{}' AND long_name = '{}'",
			Strings::Escape(c.short_name),
			Strings::Escape(c.long_name)
		);

		if (c.admin_id > 0) {
			where += fmt::format(" AND login_server_admin_id = {}", c.admin_id);
		}

		where += " LIMIT 1";

		auto results = GetWhere(db, where);
		auto e = NewEntity();
		if (results.size() == 1) {
			e = results.front();
		}

		return e;
	}
};

#endif //EQEMU_LOGIN_WORLD_SERVERS_REPOSITORY_H
