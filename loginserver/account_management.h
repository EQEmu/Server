#ifndef EQEMU_ACCOUNT_MANAGEMENT_H
#define EQEMU_ACCOUNT_MANAGEMENT_H

#include "iostream"
#include "../common/types.h"
#include "login_types.h"

class AccountManagement {
public:
	static int32 CreateLoginServerAccount(LoginAccountContext c);

	static bool CreateLoginserverWorldAdminAccount(
		const std::string &username,
		const std::string &password,
		const std::string &email,
		const std::string &first_name = "",
		const std::string &last_name = "",
		const std::string &ip_address = ""
	);

	static uint32 CheckLoginserverUserCredentials(
		const std::string &in_account_username,
		const std::string &in_account_password,
		const std::string &source_loginserver = "local"
	);

	static bool UpdateLoginserverUserCredentials(
		const std::string &in_account_username,
		const std::string &in_account_password,
		const std::string &source_loginserver = "local"
	);

	static uint32 CheckExternalLoginserverUserCredentials(
		const std::string &in_account_username,
		const std::string &in_account_password
	);

	static bool UpdateLoginserverWorldAdminAccountPasswordByName(
		const std::string &in_account_username,
		const std::string &in_account_password
	);

	static uint32 HealthCheckUserLogin();
};


#endif //EQEMU_ACCOUNT_MANAGEMENT_H
