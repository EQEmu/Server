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

#include "account_management.h"
#include "login_server.h"

extern LoginServer server;

/**
 * @param username
 * @param password
 * @param email
 * @return
 */
uint32 AccountManagement::CreateLocalLoginServerAccount(
	std::string username,
	std::string password,
	std::string email
)
{
	auto mode = server.options.GetEncryptionMode();
	auto hash = eqcrypt_hash(username, password, mode);

	LogInfo(
		"Attempting to create local login account for user [{0}] encryption algorithm [{1}] ({2})",
		username,
		GetEncryptionByModeId(mode),
		mode
	);

	unsigned int db_id          = 0;
	std::string  db_loginserver = server.options.GetDefaultLoginServerName();
	if (server.db->DoesLoginServerAccountExist(username, hash, db_loginserver, 1)) {
		LogWarning(
			"Attempting to create local login account for user [{0}] login [{1}] db_id [{2}] but already exists!",
			username,
			db_loginserver,
			db_id
		);

		return 0;
	}

	uint32 created_account_id = server.db->CreateLoginAccount(username, hash, db_loginserver, email);
	if (created_account_id > 0) {
		LogInfo(
			"Account creation success for user [{0}] encryption algorithm [{1}] ({2}) id: [{3}]",
			username,
			GetEncryptionByModeId(mode),
			mode,
			created_account_id
		);

		return created_account_id;
	}

	LogError("Failed to create local login account for user [{0}]!", username);

	return 0;
}

/**
 * @param username
 * @param password
 * @param email
 * @return
 */
bool AccountManagement::CreateLoginserverWorldAdminAccount(
	const std::string &username,
	const std::string &password,
	const std::string &email,
	const std::string &first_name,
	const std::string &last_name,
	const std::string &ip_address
)
{
	auto mode = server.options.GetEncryptionMode();
	auto hash = eqcrypt_hash(username, password, mode);

	LogInfo(
		"Attempting to create world admin account | username [{0}] encryption algorithm [{1}] ({2})",
		username,
		GetEncryptionByModeId(mode),
		mode
	);

	if (server.db->DoesLoginserverWorldAdminAccountExist(username)) {
		LogWarning(
			"Attempting to create world admin account for user [{0}] but already exists!",
			username
		);

		return false;
	}

	uint32 created_world_admin_id = server.db->CreateLoginserverWorldAdminAccount(
		username,
		hash,
		first_name,
		last_name,
		email,
		ip_address
	);

	if (created_world_admin_id > 0) {
		LogInfo(
			"Account creation success for user [{0}] encryption algorithm [{1}] ({2}) new admin id [{3}]",
			username,
			GetEncryptionByModeId(mode),
			mode,
			created_world_admin_id
		);
		return true;
	}

	LogError("Failed to create world admin account account for user [{0}]!", username);

	return false;
}
