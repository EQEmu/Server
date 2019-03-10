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

#include "console.h"
#include "../common/string_util.h"
#include "../common/md5.h"
#include "../common/database.h"
#include "../common/json/json.h"
#include "zone.h"
#include "npc.h"
#include "entity.h"
#include "eqemu_api_zone_data_service.h"

/**
 * @param username
 * @param password
 * @return
 */
struct EQ::Net::ConsoleLoginStatus CheckLogin(const std::string &username, const std::string &password)
{
	struct EQ::Net::ConsoleLoginStatus ret;
	ret.account_id = database.CheckLogin(username.c_str(), password.c_str());
	if (ret.account_id == 0) {
		return ret;
	}

	char account_name[64];
	database.GetAccountName(static_cast<uint32>(ret.account_id), account_name);

	ret.account_name = account_name;
	ret.status       = database.CheckStatus(static_cast<uint32>(ret.account_id));
	return ret;
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleApi(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	Json::Value root;
	Json::Value response;

	BenchTimer timer;
	timer.reset();

	EQEmuApiZoneDataService::get(response, args);

	std::string method = args[0];

	root["execution_time"] = std::to_string(timer.elapsed());
	root["method"]         = method;
	root["data"]           = response;

	std::stringstream payload;
	payload << root;

	connection->SendLine(payload.str());
}

/**
 *
 * @param connection
 * @param command
 * @param args
 */
void ConsoleNull(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleQuit(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	connection->SendLine("Exiting...");
	connection->Close();
}

/**
 * @param console
 */
void RegisterConsoleFunctions(std::unique_ptr<EQ::Net::ConsoleServer> &console)
{
	console->RegisterLogin(std::bind(CheckLogin, std::placeholders::_1, std::placeholders::_2));
	console->RegisterCall("api", 200, "api", std::bind(ConsoleApi, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("ping", 50, "ping", std::bind(ConsoleNull, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("quit", 50, "quit", std::bind(ConsoleQuit, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("exit", 50, "exit", std::bind(ConsoleQuit, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}
