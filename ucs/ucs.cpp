/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2008 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

*/

#include "../common/eqemu_logsys.h"
#include "../common/global_define.h"
#include "clientlist.h"
#include "../common/opcodemgr.h"
#include "../common/rulesys.h"
#include "../common/servertalk.h"
#include "../common/platform.h"
#include "../common/crash.h"
#include "../common/event/event_loop.h"
#include "database.h"
#include "ucsconfig.h"
#include "chatchannel.h"
#include "worldserver.h"
#include <list>
#include <signal.h>

#include "../common/net/tcp_server.h"
#include "../common/net/servertalk_client_connection.h"

ChatChannelList *ChannelList;
Clientlist *g_Clientlist;
EQEmuLogSys LogSys;
Database database;
WorldServer *worldserver = nullptr;

const ucsconfig *Config;

std::string WorldShortName;

uint32 ChatMessagesSent = 0;
uint32 MailMessagesSent = 0;

volatile bool RunLoops = true;

void CatchSignal(int sig_num) {

	RunLoops = false;
}

std::string GetMailPrefix() {

	return "SOE.EQ." + WorldShortName + ".";

}

int main() {
	RegisterExecutablePlatform(ExePlatformUCS);
	LogSys.LoadLogSettingsDefaults();
	set_exception_handler();

	// Check every minute for unused channels we can delete
	//
	Timer ChannelListProcessTimer(60000);
	Timer ClientConnectionPruneTimer(60000);

	Timer InterserverTimer(INTERSERVER_TIMER); // does auto-reconnect

	LogInfo("Starting EQEmu Universal Chat Server");

	if (!ucsconfig::LoadConfig()) {
		LogInfo("Loading server configuration failed");
		return 1;
	}

	Config = ucsconfig::get();

	WorldShortName = Config->ShortName;

	LogInfo("Connecting to MySQL");

	if (!database.Connect(
		Config->DatabaseHost.c_str(),
		Config->DatabaseUsername.c_str(),
		Config->DatabasePassword.c_str(),
		Config->DatabaseDB.c_str(),
		Config->DatabasePort)) {
		LogInfo("Cannot continue without a database connection");
		return 1;
	}

	LogSys.SetDatabase(&database)
		->LoadLogDatabaseSettings()
		->StartFileLogs();

	char tmp[64];

	// ucs has no 'reload rules' handler
	if (database.GetVariable("RuleSet", tmp, sizeof(tmp)-1)) {
		LogInfo("Loading rule set [{}]", tmp);
		if(!RuleManager::Instance()->LoadRules(&database, tmp, false)) {
			LogInfo("Failed to load ruleset [{}], falling back to defaults", tmp);
		}
	} else {
		if(!RuleManager::Instance()->LoadRules(&database, "default", false)) {
			LogInfo("No rule set configured, using default rules");
		} else {
			LogInfo("Loaded default rule set 'default'", tmp);
		}
	}

	EQ::InitializeDynamicLookups();
	LogInfo("Initialized dynamic dictionary entries");

	database.ExpireMail();

	if(Config->ChatPort != Config->MailPort)
	{
		LogInfo("MailPort and CharPort must be the same in eqemu_config.json for UCS");
		exit(1);
	}

	g_Clientlist = new Clientlist(Config->ChatPort);

	ChannelList = new ChatChannelList();

	database.LoadChatChannels();

	if (signal(SIGINT, CatchSignal) == SIG_ERR)	{
		LogInfo("Could not set signal handler");
		return 1;
	}
	if (signal(SIGTERM, CatchSignal) == SIG_ERR)	{
		LogInfo("Could not set signal handler");
		return 1;
	}

	worldserver = new WorldServer;

	auto loop_fn = [&](EQ::Timer* t) {

		Timer::SetCurrentTime();

		g_Clientlist->Process();

		if (ChannelListProcessTimer.Check()) {
			ChannelList->Process();
		}

		if (ClientConnectionPruneTimer.Check()) {
			g_Clientlist->CheckForStaleConnectionsAll();
		}

	};

	EQ::Timer process_timer(loop_fn);
	process_timer.Start(32, true);

	EQ::EventLoop::Get().Run();

	ChannelList->RemoveAllChannels();

	g_Clientlist->CloseAllConnections();

	LogSys.CloseFileLogs();

}

void UpdateWindowTitle(char* iNewTitle) {
#ifdef _WINDOWS
		char tmp[500];
		if (iNewTitle) {
				snprintf(tmp, sizeof(tmp), "UCS: %s", iNewTitle);
		}
		else {
				snprintf(tmp, sizeof(tmp), "UCS");
		}
		SetConsoleTitle(tmp);
#endif
}
