#ifndef EQEMU_ACCOUNT_MANAGEMENT_H
#define EQEMU_ACCOUNT_MANAGEMENT_H

#include "iostream"
#include "../common/types.h"

class AccountManagement {
public:

	/**
	 * @param username
	 * @param password
	 * @param email
	 * @param source_loginserver
	 * @param login_account_id
	 * @return
	 */
	static int32 CreateLoginServerAccount(
		std::string username,
		std::string password,
		std::string email = "",
		const std::string &source_loginserver = "local",
		uint32 login_account_id = 0
	);

	/**
	 * @param username
	 * @param password
	 * @param email
	 * @return
	 */
	static bool CreateLoginserverWorldAdminAccount(
		const std::string &username,
		const std::string &password,
		const std::string &email,
		const std::string &first_name = "",
		const std::string &last_name = "",
		const std::string &ip_address = ""
	);

	/**
	 * @param in_account_username
	 * @param in_account_password
	 * @return
	 */
	static uint32 CheckLoginserverUserCredentials(
		const std::string &in_account_username,
		const std::string &in_account_password,
		const std::string &source_loginserver = "local"
	);

	/**
	 * @param in_account_username
	 * @param in_account_password
	 * @return
	 */
	static bool UpdateLoginserverUserCredentials(
		const std::string &in_account_username,
		const std::string &in_account_password,
		const std::string &source_loginserver = "local"
	);

	/**
	 * @param in_account_username
	 * @param in_account_password
	 * @return
	 */
	static uint32 CheckExternalLoginserverUserCredentials(
		const std::string &in_account_username,
		const std::string &in_account_password
	);

	/**
	 * @param in_account_username
	 * @param in_account_password
	 * @return
	 */
	static bool UpdateLoginserverWorldAdminAccountPasswordByName(
		const std::string &in_account_username,
		const std::string &in_account_password
	);

	/**
	 * @param in_account_id
	 * @param in_account_password_hash
	 * @return
	 */
	static bool UpdateLoginserverWorldAdminAccountPasswordById(
		uint32 in_account_id,
		const std::string &in_account_password_hash
	);

	static uint32 HealthCheckUserLogin();
};


#endif //EQEMU_ACCOUNT_MANAGEMENT_H
