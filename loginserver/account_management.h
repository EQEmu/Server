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
	static uint64 CreateLoginServerAccount(LoginAccountContext c);
	static uint64 CheckLoginserverUserCredentials(LoginAccountContext c);
	static bool UpdateLoginserverUserCredentials(LoginAccountContext c);
	static uint64 CheckExternalLoginserverUserCredentials(LoginAccountContext c);
	static bool UpdateLoginserverWorldAdminAccountPasswordByName(LoginAccountContext c);
	static uint64 HealthCheckUserLogin();

	static bool CreateLoginserverWorldAdminAccount(
		const std::string &username,
		const std::string &password,
		const std::string &email,
		const std::string &first_name = "",
		const std::string &last_name = "",
		const std::string &ip_address = ""
	);
};


#endif //EQEMU_ACCOUNT_MANAGEMENT_H
