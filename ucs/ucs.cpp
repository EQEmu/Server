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
#include "../common/debug.h"
#include "clientlist.h"
#include "../common/opcodemgr.h"
#include "../common/eq_stream_factory.h"
#include "../common/rulesys.h"
#include "../common/servertalk.h"
#include "../common/platform.h"
#include "../common/crash.h"
#include "database.h"
#include "ucsconfig.h"
#include "chatchannel.h"
#include "worldserver.h"
#include <list>
#include <signal.h>

volatile bool RunLoops = true;

uint32 MailMessagesSent = 0;
uint32 ChatMessagesSent = 0;

TimeoutManager timeout_manager;

Clientlist *CL;

ChatChannelList *ChannelList;

Database database;

std::string WorldShortName;

const ucsconfig *Config;

WorldServer *worldserver = nullptr;
EQEmuLogSys logger;

void CatchSignal(int sig_num) {

	RunLoops = false;

	if(worldserver)
		worldserver->Disconnect();
}

std::string GetMailPrefix() {

	return "SOE.EQ." + WorldShortName + ".";

}

int main() {
	RegisterExecutablePlatform(ExePlatformUCS);
	logger.LoadLogSettingsDefaults();
	set_exception_handler();

	// Check every minute for unused channels we can delete
	//
	Timer ChannelListProcessTimer(60000);

	Timer InterserverTimer(INTERSERVER_TIMER); // does auto-reconnect

	logger.LogDebugType(EQEmuLogSys::Detail, EQEmuLogSys::UCS_Server, "Starting EQEmu Universal Chat Server.");

	if (!ucsconfig::LoadConfig()) {

		logger.LogDebugType(EQEmuLogSys::Detail, EQEmuLogSys::UCS_Server, "Loading server configuration failed.");

		return 1;
	}

	Config = ucsconfig::get();

	if(!load_log_settings(Config->LogSettingsFile.c_str()))
		logger.LogDebugType(EQEmuLogSys::Detail, EQEmuLogSys::UCS_Server, "Warning: Unable to read %s", Config->LogSettingsFile.c_str());
	else
		logger.LogDebugType(EQEmuLogSys::Detail, EQEmuLogSys::UCS_Server, "Log settings loaded from %s", Config->LogSettingsFile.c_str());

	WorldShortName = Config->ShortName;

	logger.LogDebugType(EQEmuLogSys::Detail, EQEmuLogSys::UCS_Server, "Connecting to MySQL...");

	if (!database.Connect(
		Config->DatabaseHost.c_str(),
		Config->DatabaseUsername.c_str(),
		Config->DatabasePassword.c_str(),
		Config->DatabaseDB.c_str(),
		Config->DatabasePort)) {
		logger.LogDebugType(EQEmuLogSys::Detail, EQEmuLogSys::World_Server, "Cannot continue without a database connection.");
		return 1;
	}

	char tmp[64];

	if (database.GetVariable("RuleSet", tmp, sizeof(tmp)-1)) {
		logger.LogDebugType(EQEmuLogSys::Detail, EQEmuLogSys::World_Server, "Loading rule set '%s'", tmp);
		if(!RuleManager::Instance()->LoadRules(&database, tmp)) {
			logger.LogDebugType(EQEmuLogSys::Detail, EQEmuLogSys::UCS_Server, "Failed to load ruleset '%s', falling back to defaults.", tmp);
		}
	} else {
		if(!RuleManager::Instance()->LoadRules(&database, "default")) {
			logger.LogDebugType(EQEmuLogSys::Detail, EQEmuLogSys::UCS_Server, "No rule set configured, using default rules");
		} else {
			logger.LogDebugType(EQEmuLogSys::Detail, EQEmuLogSys::UCS_Server, "Loaded default rule set 'default'", tmp);
		}
	}

	database.ExpireMail();

	if(Config->ChatPort != Config->MailPort)
	{
		logger.LogDebugType(EQEmuLogSys::Detail, EQEmuLogSys::UCS_Server, "MailPort and CharPort must be the same in eqemu_config.xml for UCS.");
		exit(1);
	}

	CL = new Clientlist(Config->ChatPort);

	ChannelList = new ChatChannelList();

	database.LoadChatChannels();

	if (signal(SIGINT, CatchSignal) == SIG_ERR)	{
		logger.LogDebugType(EQEmuLogSys::Detail, EQEmuLogSys::UCS_Server, "Could not set signal handler");
		return 1;
	}
	if (signal(SIGTERM, CatchSignal) == SIG_ERR)	{
		logger.LogDebugType(EQEmuLogSys::Detail, EQEmuLogSys::UCS_Server, "Could not set signal handler");
		return 1;
	}

	worldserver = new WorldServer;

	worldserver->Connect();

	while(RunLoops) {

		Timer::SetCurrentTime();

		CL->Process();

		if(ChannelListProcessTimer.Check())
			ChannelList->Process();

		if (InterserverTimer.Check()) {
			if (worldserver->TryReconnect() && (!worldserver->Connected()))
				worldserver->AsyncConnect();
		}
		worldserver->Process();

		timeout_manager.CheckTimeouts();

		Sleep(100);
	}

	ChannelList->RemoveAllChannels();

	CL->CloseAllConnections();

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
