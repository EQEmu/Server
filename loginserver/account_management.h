#ifndef EQEMU_ACCOUNT_MANAGEMENT_H
#define EQEMU_ACCOUNT_MANAGEMENT_H

#include "iostream"
#include "../common/types.h"
#include "login_types.h"
#include "encryption.h"
#include "login_server.h"

extern LoginServer server;
extern Database database;

class AccountManagement {
public:
	struct EncryptionResult {
		std::string password;
		int         mode = 0;
		std::string mode_name;
	};

	static EncryptionResult EncryptPasswordFromContext(LoginAccountContext c)
	{
		EncryptionResult r;
		r.password  = eqcrypt_hash(
			c.username,
			c.password,
			server.options.GetEncryptionMode()
		);
		r.mode      = server.options.GetEncryptionMode();
		r.mode_name = GetEncryptionByModeId(r.mode);
		return r;
	}

	static int32 CreateLoginServerAccount(LoginAccountContext c);
	static bool CreateLoginserverWorldAdminAccount(
		const std::string &username,
		const std::string &password,
		const std::string &email,
		const std::string &first_name = "",
		const std::string &last_name = "",
		const std::string &ip_address = ""
	);

	static uint32 CheckLoginserverUserCredentials(LoginAccountContext c);
	static bool UpdateLoginserverUserCredentials(LoginAccountContext c);
	static uint32 CheckExternalLoginserverUserCredentials(LoginAccountContext c);
	static bool UpdateLoginserverWorldAdminAccountPasswordByName(LoginAccountContext c);
	static uint32 HealthCheckUserLogin();
};


#endif //EQEMU_ACCOUNT_MANAGEMENT_H
