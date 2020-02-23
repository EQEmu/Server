/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2019 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */
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
};


#endif //EQEMU_ACCOUNT_MANAGEMENT_H
