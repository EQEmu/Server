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
#include "../common/event/task_scheduler.h"
#include "../common/event/event_loop.h"
#include "../common/net/dns.h"

extern LoginServer       server;
EQ::Event::TaskScheduler task_runner;

/**
 * @param username
 * @param password
 * @param email
 * @param source_loginserver
 * @param login_account_id
 * @return
 */
int32 AccountManagement::CreateLoginServerAccount(
	std::string username,
	std::string password,
	std::string email,
	const std::string &source_loginserver,
	uint32 login_account_id
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

	unsigned int db_id = 0;
	if (server.db->DoesLoginServerAccountExist(username, hash, source_loginserver, 1)) {
		LogWarning(
			"Attempting to create local login account for user [{0}] login [{1}] but already exists!",
			username,
			source_loginserver
		);

		return -1;
	}

	uint32 created_account_id = 0;
	if (login_account_id > 0) {
		created_account_id = server.db->CreateLoginDataWithID(username, hash, source_loginserver, login_account_id);
	}
	else {
		created_account_id = server.db->CreateLoginAccount(username, hash, source_loginserver, email);
	}

	if (created_account_id > 0) {
		LogInfo(
			"Account creation success for user [{0}] encryption algorithm [{1}] ({2}) id: [{3}]",
			username,
			GetEncryptionByModeId(mode),
			mode,
			created_account_id
		);

		return (int32) created_account_id;
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

/**
 * @param in_account_username
 * @param in_account_password
 * @return
 */
uint32 AccountManagement::CheckLoginserverUserCredentials(
	const std::string &in_account_username,
	const std::string &in_account_password,
	const std::string &source_loginserver
)
{
	auto mode = server.options.GetEncryptionMode();

	Database::DbLoginServerAccount
		login_server_admin = server.db->GetLoginServerAccountByAccountName(
		in_account_username,
		source_loginserver
	);

	if (!login_server_admin.loaded) {
		LogError(
			"CheckLoginUserCredentials account [{0}] source_loginserver [{1}] not found!",
			in_account_username,
			source_loginserver
		);

		return false;
	}

	bool validated_credentials = eqcrypt_verify_hash(
		in_account_username,
		in_account_password,
		login_server_admin.account_password,
		mode
	);

	if (!validated_credentials) {
		LogError(
			"CheckLoginUserCredentials account [{0}] source_loginserver [{1}] invalid credentials!",
			in_account_username,
			source_loginserver
		);

		return 0;
	}

	LogInfo(
		"CheckLoginUserCredentials account [{0}] source_loginserver [{1}] credentials validated success!",
		in_account_username,
		source_loginserver
	);

	return login_server_admin.id;
}


/**
 * @param in_account_username
 * @param in_account_password
 * @return
 */
bool AccountManagement::UpdateLoginserverUserCredentials(
	const std::string &in_account_username,
	const std::string &in_account_password,
	const std::string &source_loginserver
)
{
	auto mode = server.options.GetEncryptionMode();

	Database::DbLoginServerAccount
		login_server_account = server.db->GetLoginServerAccountByAccountName(
		in_account_username,
		source_loginserver
	);

	if (!login_server_account.loaded) {
		LogError(
			"ChangeLoginserverUserCredentials account [{0}] source_loginserver [{1}] not found!",
			in_account_username,
			source_loginserver
		);

		return false;
	}

	server.db->UpdateLoginserverAccountPasswordHash(
		in_account_username,
		source_loginserver,
		eqcrypt_hash(
			in_account_username,
			in_account_password,
			mode
		)
	);

	LogInfo(
		"ChangeLoginserverUserCredentials account [{0}] source_loginserver [{1}] credentials updated!",
		in_account_username,
		source_loginserver
	);

	return true;
}

/**
 * @param in_account_username
 * @param in_account_password
 */
bool AccountManagement::UpdateLoginserverWorldAdminAccountPasswordByName(
	const std::string &in_account_username,
	const std::string &in_account_password
)
{
	auto mode = server.options.GetEncryptionMode();
	auto hash = eqcrypt_hash(in_account_username, in_account_password, mode);

	bool updated_account = server.db->UpdateLoginWorldAdminAccountPasswordByUsername(
		in_account_username,
		hash
	);

	LogInfo(
		"[{}] account_name [{}] status [{}]",
		__func__,
		in_account_username,
		(updated_account ? "success" : "failed")
	);

	return updated_account;
}

/**
 * @param in_account_id
 * @param in_account_password_hash
 */
bool AccountManagement::UpdateLoginserverWorldAdminAccountPasswordById(
	uint32 in_account_id,
	const std::string &in_account_password_hash
)
{
	bool updated_account = server.db->UpdateLoginWorldAdminAccountPassword(in_account_id, in_account_password_hash);

	LogInfo(
		"[{}] account_name [{}] status [{}]",
		__func__,
		in_account_id,
		(updated_account ? "success" : "failed")
	);

	return updated_account;
}

/**
 * @param in_account_username
 * @param in_account_password
 * @return
 */
uint32 AccountManagement::CheckExternalLoginserverUserCredentials(
	const std::string &in_account_username,
	const std::string &in_account_password
)
{
	auto res = task_runner.Enqueue(
		[&]() -> uint32 {
			bool   running = true;
			uint32 ret     = 0;

			EQ::Net::DaybreakConnectionManager           mgr;
			std::shared_ptr<EQ::Net::DaybreakConnection> c;

			mgr.OnNewConnection(
				[&](std::shared_ptr<EQ::Net::DaybreakConnection> connection) {
					c = connection;
				}
			);

			mgr.OnConnectionStateChange(
				[&](
					std::shared_ptr<EQ::Net::DaybreakConnection> conn,
					EQ::Net::DbProtocolStatus from,
					EQ::Net::DbProtocolStatus to
				) {
					if (EQ::Net::StatusConnected == to) {
						EQ::Net::DynamicPacket p;
						p.PutUInt16(0, 1); //OP_SessionReady
						p.PutUInt32(2, 2);
						c->QueuePacket(p);
					}
					else if (EQ::Net::StatusDisconnected == to) {
						running = false;
					}
				}
			);

			mgr.OnPacketRecv(
				[&](std::shared_ptr<EQ::Net::DaybreakConnection> conn, const EQ::Net::Packet &p) {
					auto opcode = p.GetUInt16(0);
					switch (opcode) {
						case 0x0017: //OP_ChatMessage
						{
							size_t buffer_len =
									   in_account_username.length() + in_account_password.length() + 2;

							std::unique_ptr<char[]> buffer(new char[buffer_len]);

							strcpy(&buffer[0], in_account_username.c_str());
							strcpy(&buffer[in_account_username.length() + 1], in_account_password.c_str());

							size_t encrypted_len = buffer_len;

							if (encrypted_len % 8 > 0) {
								encrypted_len = ((encrypted_len / 8) + 1) * 8;
							}

							EQ::Net::DynamicPacket p;
							p.Resize(12 + encrypted_len);
							p.PutUInt16(0, 2); //OP_Login
							p.PutUInt32(2, 3);

							eqcrypt_block(&buffer[0], buffer_len, (char *) p.Data() + 12, true);
							c->QueuePacket(p);
							break;
						}
						case 0x0018: {
							auto encrypt_size                    = p.Length() - 12;
							if (encrypt_size % 8 > 0) {
								encrypt_size = (encrypt_size / 8) * 8;
							}

							std::unique_ptr<char[]> decrypted(new char[encrypt_size]);

							eqcrypt_block((char *) p.Data() + 12, encrypt_size, &decrypted[0], false);

							EQ::Net::StaticPacket sp(&decrypted[0], encrypt_size);
							auto                  response_error = sp.GetUInt16(1);
							auto                  m_dbid         = sp.GetUInt32(8);

							{
								ret     = (response_error <= 101 ? m_dbid : 0);
								running = false;
							}
							break;
						}
					}
				}
			);

			EQ::Net::DNSLookup(
				"login.eqemulator.net", 5999, false, [&](const std::string &addr) {
					if (addr.empty()) {
						ret     = 0;
						running = false;
					}

					mgr.Connect(addr, 5999);
				}
			);

			auto &loop = EQ::EventLoop::Get();
			while (running) {
				loop.Process();
			}

			return ret;
		}
	);

	return res.get();
}