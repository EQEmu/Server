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
#include "clientlist.h"
#include "login_server.h"
#include "login_server_list.h"
#include "world_config.h"
#include "world_console_connection.h"
#include "worlddb.h"
#include "zonelist.h"
#include "zoneserver.h"
#include "../common/string_util.h"
#include "../common/md5.h"
#include "eqemu_api_world_data_service.h"
#include "world_store.h"
#include <fmt/format.h>

extern ClientList      client_list;
extern ZSList          zoneserver_list;
extern LoginServerList loginserverlist;

/**
 * @param username
 * @param password
 * @return
 */
struct EQ::Net::ConsoleLoginStatus CheckLogin(const std::string &username, const std::string &password)
{
	struct EQ::Net::ConsoleLoginStatus ret;

	std::string prefix   = "eqemu";
	std::string raw_user = "";

	ParseAccountString(username, raw_user, prefix);

	ret.account_id = database.CheckLogin(raw_user.c_str(), password.c_str(), prefix.c_str());

	if (ret.account_id == 0) {
		return ret;
	}

	char account_name[64];
	database.GetAccountName(ret.account_id, account_name);

	ret.account_name = account_name;
	ret.status       = database.CheckStatus(ret.account_id);
	return ret;
}

/**
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

	EQEmuApiWorldDataService::get(response, args);

	std::string method = args[0];

	root["execution_time"] = std::to_string(timer.elapsed());
	root["method"]         = method;
	root["data"]           = response;

	std::stringstream payload;
	payload << root;

	connection->SendLine(payload.str());
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleWhoami(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	connection->SendLine(fmt::format("You are logged in as '{0}'", connection->UserName()));
	connection->SendLine(fmt::format("You are known as '*{0}'", connection->UserName()));
	connection->SendLine(fmt::format("AccessLevel: '{0}'", connection->Admin()));
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleZoneStatus(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	WorldConsoleTCPConnection console_connection(connection);
	zoneserver_list.SendZoneStatus(0, connection->Admin(), &console_connection);
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleWho(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	Who_All_Struct whom;
	memset(&whom, 0, sizeof(whom));
	whom.lvllow   = 0xFFFF;
	whom.lvlhigh  = 0xFFFF;
	whom.wclass   = 0xFFFF;
	whom.wrace    = 0xFFFF;
	whom.gmlookup = 0xFFFF;

	for (auto &arg : args) {
		if (strcasecmp(arg.c_str(), "gm") == 0) {
			whom.gmlookup = 1;
		}
		else if (StringIsNumber(arg)) {
			if (whom.lvllow == 0xFFFF) {
				whom.lvllow  = atoi(arg.c_str());
				whom.lvlhigh = whom.lvllow;
			}
			else if (atoi(arg.c_str()) > int(whom.lvllow)) {
				whom.lvlhigh = atoi(arg.c_str());
			}
			else {
				whom.lvllow = atoi(arg.c_str());
			}
		}
		else {
			strn0cpy(whom.whom, arg.c_str(), sizeof(whom.whom));
		}
	}

	WorldConsoleTCPConnection console_connection(connection);
	client_list.ConsoleSendWhoAll(0, connection->Admin(), &whom, &console_connection);
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleUptime(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 1) {
		return;
	}

	if (StringIsNumber(args[0]) && atoi(args[0].c_str()) > 0) {
		auto                pack = new ServerPacket(ServerOP_Uptime, sizeof(ServerUptime_Struct));
		ServerUptime_Struct *sus = (ServerUptime_Struct *) pack->pBuffer;
		snprintf(sus->adminname, sizeof(sus->adminname), "*%s", connection->UserName().c_str());
		sus->zoneserverid = atoi(args[0].c_str());
		ZoneServer *zs = zoneserver_list.FindByID(sus->zoneserverid);
		if (zs) {
			zs->SendPacket(pack);
		}
		else {
			connection->SendLine("Zoneserver not found.");
		}
		delete pack;
	}
	else {
		WorldConsoleTCPConnection console_connection(connection);
		ZSList::ShowUpTime(&console_connection);
	}
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleMd5(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 1) {
		return;
	}

	uint8 md5[16];
	MD5::Generate((const uchar *) args[0].c_str(), strlen(args[0].c_str()), md5);

	connection->SendLine(
		StringFormat(
			"MD5: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
			md5[0],
			md5[1],
			md5[2],
			md5[3],
			md5[4],
			md5[5],
			md5[6],
			md5[7],
			md5[8],
			md5[9],
			md5[10],
			md5[11],
			md5[12],
			md5[13],
			md5[14],
			md5[15]
		)
	);
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleEmote(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 3) {
		return;
	}

	auto join_args = args;
	join_args.erase(join_args.begin(), join_args.begin() + 2);

	if (strcasecmp(args[0].c_str(), "world") == 0) {
		zoneserver_list.SendEmoteMessageRaw(
			0,
			0,
			AccountStatus::Player,
			atoi(args[1].c_str()),
			JoinString(join_args, " ").c_str()
		);
	}
	else {
		ZoneServer *zs = zoneserver_list.FindByName(args[0].c_str());
		if (zs != 0) {
			zs->SendEmoteMessageRaw(
				0,
				0,
				AccountStatus::Player,
				atoi(args[1].c_str()),
				JoinString(join_args, " ").c_str()
			);
		}
		else {
			zoneserver_list.SendEmoteMessageRaw(
				args[0].c_str(),
				0,
				AccountStatus::Player,
				atoi(args[1].c_str()),
				JoinString(join_args, " ").c_str()
			);
		}
	}
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleAcceptMessages(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 1) {
		connection->SendLine("Usage: acceptmessages [on/off]");
		return;
	}

	if (strcasecmp(args[0].c_str(), "on") == 0) {
		connection->SetAcceptMessages(true);
	}
	else if (strcasecmp(args[0].c_str(), "off") == 0) {
		connection->SetAcceptMessages(false);
	}
	else {
		connection->SendLine("Usage: acceptmessages [on/off]");
	}
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleTell(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 2) {
		return;
	}

	char tmpname[64];
	tmpname[0] = '*';
	strcpy(&tmpname[1], connection->UserName().c_str());
	std::string to = args[0];

	auto join_args = args;
	join_args.erase(join_args.begin(), join_args.begin() + 1);

	zoneserver_list.SendChannelMessage(tmpname, to.c_str(), 7, 0, JoinString(join_args, " ").c_str());
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleBroadcast(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 1) {
		return;
	}

	char tmpname[64];
	tmpname[0] = '*';
	strcpy(&tmpname[1], connection->UserName().c_str());
	zoneserver_list.SendChannelMessage(tmpname, 0, 6, 0, JoinString(args, " ").c_str());
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleGMSay(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 1) {
		return;
	}

	char tmpname[64];
	tmpname[0] = '*';
	strcpy(&tmpname[1], connection->UserName().c_str());
	zoneserver_list.SendChannelMessage(tmpname, 0, 11, 0, JoinString(args, " ").c_str());
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleOOC(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 1) {
		return;
	}

	char tmpname[64];
	tmpname[0] = '*';
	strcpy(&tmpname[1], connection->UserName().c_str());
	zoneserver_list.SendChannelMessage(tmpname, 0, 5, 0, JoinString(args, " ").c_str());
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleAuction(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 1) {
		return;
	}

	char tmpname[64];
	tmpname[0] = '*';
	strcpy(&tmpname[1], connection->UserName().c_str());
	zoneserver_list.SendChannelMessage(tmpname, 0, 4, 0, JoinString(args, " ").c_str());
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleKick(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 1) {
		return;
	}

	char tmpname[64];
	tmpname[0] = '*';
	strcpy(&tmpname[1], connection->UserName().c_str());
	auto pack = new ServerPacket;
	pack->opcode  = ServerOP_KickPlayer;
	pack->size    = sizeof(ServerKickPlayer_Struct);
	pack->pBuffer = new uchar[pack->size];
	ServerKickPlayer_Struct *skp = (ServerKickPlayer_Struct *) pack->pBuffer;
	strcpy(skp->adminname, tmpname);
	strcpy(skp->name, args[0].c_str());
	skp->adminrank = connection->Admin();
	zoneserver_list.SendPacket(pack);
	delete pack;
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleLock(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	WorldConfig::LockWorld();
	if (loginserverlist.Connected()) {
		loginserverlist.SendStatus();
		connection->SendLine("World locked.");
	}
	else {
		connection->SendLine("World locked, but login server not connected.");
	}
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleUnlock(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	WorldConfig::UnlockWorld();
	if (loginserverlist.Connected()) {
		loginserverlist.SendStatus();
		connection->SendLine("World unlocked.");
	}
	else {
		connection->SendLine("World unlocked, but login server not connected.");
	}
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleZoneShutdown(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 1) {
		connection->SendLine("Usage: zoneshutdown zoneshortname");
		return;
	}

	if (args[0].length() == 0) {
		connection->SendLine("Usage: zoneshutdown zoneshortname");
	}
	else {
		char tmpname[64];
		tmpname[0] = '*';
		strcpy(&tmpname[1], connection->UserName().c_str());

		auto pack = new ServerPacket;
		pack->size    = sizeof(ServerZoneStateChange_struct);
		pack->pBuffer = new uchar[pack->size];
		memset(pack->pBuffer, 0, sizeof(ServerZoneStateChange_struct));
		ServerZoneStateChange_struct *s = (ServerZoneStateChange_struct *) pack->pBuffer;
		pack->opcode = ServerOP_ZoneShutdown;
		strcpy(s->adminname, tmpname);
		if (StringIsNumber(args[0])) {
			s->ZoneServerID = atoi(args[0].c_str());
		}
		else {
			s->zoneid = ZoneID(args[0].c_str());
		}

		ZoneServer *zs = 0;
		if (s->ZoneServerID != 0) {
			zs = zoneserver_list.FindByID(s->ZoneServerID);
		}
		else if (s->zoneid != 0) {
			zs = zoneserver_list.FindByName(ZoneName(s->zoneid));
		}
		else {
			connection->SendLine("Error: ZoneShutdown: neither ID nor name specified");
		}

		if (zs == 0) {
			connection->SendLine("Error: ZoneShutdown: zoneserver not found");
		}
		else {
			zs->SendPacket(pack);
		}

		delete pack;
	}
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleZoneBootup(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 2) {
		return;
	}

	if (args[1].length() == 0 || !StringIsNumber(args[0])) {
		connection->SendLine("Usage: zonebootup ZoneServerID# zoneshortname");
	}
	else {
		char tmpname[64];
		tmpname[0] = '*';
		strcpy(&tmpname[1], connection->UserName().c_str());

		Log(Logs::Detail,
			Logs::WorldServer,
			"Console ZoneBootup: %s, %s, %s",
			tmpname,
			args[1].c_str(),
			args[0].c_str());

		if (args.size() > 2) {
			zoneserver_list.SOPZoneBootup(
				tmpname,
				atoi(args[0].c_str()),
				args[1].c_str(),
				(bool) (strcasecmp(args[1].c_str(), "static") == 0));
		}
		else {
			zoneserver_list.SOPZoneBootup(tmpname, atoi(args[0].c_str()), args[1].c_str(), false);
		}
	}
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleZoneLock(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 1) {
		return;
	}

	if (strcasecmp(args[0].c_str(), "list") == 0) {
		WorldConsoleTCPConnection console_connection(connection);
		zoneserver_list.ListLockedZones(0, &console_connection);
	}
	else if (strcasecmp(args[0].c_str(), "lock") == 0 && connection->Admin() >= 101) {
		if (args.size() < 2) {
			return;
		}

		uint16 tmp = ZoneID(args[1].c_str());
		if (tmp) {
			if (zoneserver_list.SetLockedZone(tmp, true)) {
				zoneserver_list.SendEmoteMessage(
					0,
					0,
					AccountStatus::QuestTroupe,
					Chat::Yellow,
					fmt::format(
						"Zone locked: {}",
						ZoneName(tmp)
					).c_str()
				);
			}
			else {
				connection->SendLine("Failed to change lock");
			}
		}
		else {
			connection->SendLine("Usage: #zonelock lock [zonename]");
		}
	}
	else if (strcasecmp(args[0].c_str(), "unlock") == 0 && connection->Admin() >= 101) {
		if (args.size() < 2) {
			return;
		}

		uint16 tmp = ZoneID(args[1].c_str());
		if (tmp) {
			if (zoneserver_list.SetLockedZone(tmp, false)) {
				zoneserver_list.SendEmoteMessage(
					0,
					0,
					AccountStatus::QuestTroupe,
					Chat::Yellow,
					fmt::format(
						"Zone unlocked: {}",
						ZoneName(tmp)
					).c_str()
				);
			}
			else {
				connection->SendLine("Failed to change lock");
			}
		}
		else {
			connection->SendLine("Usage: #zonelock unlock [zonename]");
		}
	}
	else {
		connection->SendLine("#zonelock sub-commands");
		connection->SendLine("  list");
		if (connection->Admin() >= 101) {
			connection->SendLine("  lock [zonename]");
			connection->SendLine("  unlock [zonename]");
		}
	}
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleFlag(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 2) {
		return;
	}

	if (args[1].length() == 0 || !StringIsNumber(args[0])) {
		connection->SendLine("Usage: flag [status] [accountname]");
	}
	else {
		if (atoi(args[0].c_str()) > connection->Admin()) {
			connection->SendLine("You cannot set people's status to higher than your own");
		}
		else if (!database.SetAccountStatus(args[1].c_str(), atoi(args[0].c_str()))) {
			connection->SendLine("Unable to flag account!");
		}
		else {
			connection->SendLine("Account Flaged");
		}
	}
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleSetPass(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() != 2) {
		connection->SendLine("Format: setpass accountname password");
	}
	else {
		std::string prefix = "eqemu";
		std::string raw_user = "";

		ParseAccountString(args[0], raw_user, prefix);

		auto account_id = database.GetAccountIDByName(raw_user, prefix);

		if (!account_id) {
			connection->SendLine("Error: Account not found");
		}
	}
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleVersion(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	connection->SendLine(StringFormat("Current version information."));
	connection->SendLine(StringFormat("  %s", CURRENT_VERSION));
	connection->SendLine(StringFormat("  Compiled on: %s at %s", COMPILE_DATE, COMPILE_TIME));
	connection->SendLine(StringFormat("  Last modified on: %s", LAST_MODIFIED));
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleWorldShutdown(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() == 2) {
		int32 time, interval;
		if (StringIsNumber(args[0]) && StringIsNumber(args[1]) && ((time = atoi(args[0].c_str())) > 0) &&
			((interval = atoi(args[1].c_str())) > 0)) {
			zoneserver_list.WorldShutDown(time, interval);
		}
		else {
			connection->SendLine("Usage: worldshutdown [now] [disable] ([time] [interval])");
		}
	}
	else if (args.size() == 1) {
		if (strcasecmp(args[0].c_str(), "now") == 0) {
			zoneserver_list.WorldShutDown(0, 0);
		}
		else if (strcasecmp(args[0].c_str(), "disable") == 0) {
			connection->SendLine("[SYSTEM] World shutdown has been aborted.");
			zoneserver_list.SendEmoteMessage(
				0,
				0,
				AccountStatus::Player,
				Chat::Yellow,
				"[SYSTEM] World shutdown has been aborted."
			);
			zoneserver_list.shutdowntimer->Disable();
			zoneserver_list.reminder->Disable();
		}
		else {
			connection->SendLine("Usage: worldshutdown [now] [disable] ([time] [interval])");
		}
	}
	else {
		connection->SendLine("Usage: worldshutdown [now] [disable] ([time] [interval])");
	}
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleIpLookup(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (!args.empty()) {
		WorldConsoleTCPConnection console_connection(connection);
		client_list.SendCLEList(connection->Admin(), nullptr, &console_connection, args[0].c_str());
	}
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleSignalCharByName(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.size() < 2) {
		return;
	}

	connection->SendLine(StringFormat("Signal Sent to %s with ID %i", (char *) args[0].c_str(), atoi(args[1].c_str())));
	uint32 message_len = strlen((char *) args[0].c_str()) + 1;
	auto pack = new ServerPacket(ServerOP_CZSignal, sizeof(CZSignal_Struct) + message_len);
	CZSignal_Struct* CZS = (CZSignal_Struct*) pack->pBuffer;
	uint8 update_type = CZUpdateType_ClientName;
	int update_identifier = 0;
	CZS->update_type = update_type;
	CZS->update_identifier = update_identifier;
	CZS->signal = atoi(args[1].c_str());
	strn0cpy(CZS->client_name, (char *) args[0].c_str(), 64);
	zoneserver_list.SendPacket(pack);
	safe_delete(pack);
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleReloadWorld(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	connection->SendLine("Reloading World...");
	auto               pack = new ServerPacket(ServerOP_ReloadWorld, sizeof(ReloadWorld_Struct));
	ReloadWorld_Struct *RW  = (ReloadWorld_Struct *) pack->pBuffer;
	RW->global_repop = ReloadWorld::Repop;
	zoneserver_list.SendPacket(pack);
	safe_delete(pack);
}

/**
 * @param connection
 * @param command
 * @param args
 */
void ConsoleReloadZoneQuests(
	EQ::Net::ConsoleServerConnection *connection,
	const std::string &command,
	const std::vector<std::string> &args
)
{
	if (args.empty()) {
		connection->SendLine("[zone_short_name] required as argument");
		return;
	}

	std::string zone_short_name = args[0];

	connection->SendLine(fmt::format("Reloading Zone [{}]...", zone_short_name));

	auto pack               = new ServerPacket(ServerOP_HotReloadQuests, sizeof(HotReloadQuestsStruct));
	auto *hot_reload_quests = (HotReloadQuestsStruct *) pack->pBuffer;
	strn0cpy(hot_reload_quests->zone_short_name, (char *) zone_short_name.c_str(), 200);

	zoneserver_list.SendPacket(pack);
	safe_delete(pack);
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
void RegisterConsoleFunctions(std::unique_ptr<EQ::Net::ConsoleServer>& console)
{
	console->RegisterLogin(std::bind(CheckLogin, std::placeholders::_1, std::placeholders::_2));
	console->RegisterCall("acceptmessages", 50, "acceptmessages [on/off]", std::bind(ConsoleAcceptMessages, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("api", 200, "api", std::bind(ConsoleApi, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("auction", 50, "auction [message]", std::bind(ConsoleAuction, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("broadcast", 50, "broadcast [message]", std::bind(ConsoleBroadcast, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("echo", 50, "echo [on/off]", std::bind(ConsoleNull, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("emote", 50, "emote [zonename or charname or world] [type] [message]", std::bind(ConsoleEmote, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("flag", 200, "flag [status] [accountname]", std::bind(ConsoleFlag, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("gmsay", 50, "gmsay [message]", std::bind(ConsoleGMSay, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("iplookup", 50, "IPLookup [name]", std::bind(ConsoleIpLookup, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("kick", 150, "kick [charname]", std::bind(ConsoleKick, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("lock", 150, "lock", std::bind(ConsoleLock, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("lsreconnect", 50, "LSReconnect", std::bind(ConsoleNull, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("md5", 50, "md5", std::bind(ConsoleMd5, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("ooc", 50, "ooc [message]", std::bind(ConsoleOOC, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("reloadworld", 200, "reloadworld", std::bind(ConsoleReloadWorld, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("reloadzonequests", 200, "reloadzonequests [zone_short_name]", std::bind(ConsoleReloadZoneQuests, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("setpass", 200, "setpass [account_name] [new_password]", std::bind(ConsoleSetPass, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("signalcharbyname", 50, "signalcharbyname charname ID", std::bind(ConsoleSignalCharByName, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("tell", 50, "tell [name] [message]", std::bind(ConsoleTell, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("unlock", 150, "unlock", std::bind(ConsoleUnlock, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("uptime", 50, "uptime [zone_server_id]", std::bind(ConsoleUptime, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("version", 50, "version", std::bind(ConsoleVersion, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("who", 50, "who", std::bind(ConsoleWho, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("whoami", 50, "whoami", std::bind(ConsoleWhoami, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("worldshutdown", 200, "worldshutdown", std::bind(ConsoleWorldShutdown, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("zonebootup", 150, "zonebootup [zone_server_id] [zone_short_name]", std::bind(ConsoleZoneBootup, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("zonelock", 150, "zonelock [list|lock|unlock] [zone_short_name]", std::bind(ConsoleZoneLock, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("zoneshutdown", 150, "zoneshutdown [zone_short_name or zone_server_id]", std::bind(ConsoleZoneShutdown, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("zonestatus", 50, "zonestatus", std::bind(ConsoleZoneStatus, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));console->RegisterCall("ping", 50, "ping", std::bind(ConsoleNull, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("quit", 50, "quit", std::bind(ConsoleQuit, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	console->RegisterCall("exit", 50, "exit", std::bind(ConsoleQuit, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}
