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

#include "../common/debug.h"
#include "../common/opcodemgr.h"
#include "../common/EQStreamFactory.h"
#include "../common/rulesys.h"
#include "../common/servertalk.h"
#include "../common/platform.h"
#include "../common/crash.h"
#include "database.h"
#include "queryservconfig.h"
#include "worldserver.h"
#include "lfguild.h"
#include <list>
#include <signal.h>

volatile bool RunLoops = true;

uint32 MailMessagesSent = 0;
uint32 ChatMessagesSent = 0;

TimeoutManager timeout_manager;

Database database;
LFGuildManager lfguildmanager;
std::string WorldShortName;

const queryservconfig *Config;

WorldServer *worldserver = 0;


void CatchSignal(int sig_num) {

	RunLoops = false;

	if(worldserver)
		worldserver->Disconnect();
}

int main() {
	RegisterExecutablePlatform(ExePlatformQueryServ);
	set_exception_handler();

	Timer LFGuildExpireTimer(60000);

	Timer InterserverTimer(INTERSERVER_TIMER); // does auto-reconnect

	_log(QUERYSERV__INIT, "Starting EQEmu QueryServ.");

	if (!queryservconfig::LoadConfig()) {

		_log(QUERYSERV__INIT, "Loading server configuration failed.");

		return(1);
	}

	Config = queryservconfig::get();

	if(!load_log_settings(Config->LogSettingsFile.c_str()))
		_log(QUERYSERV__INIT, "Warning: Unable to read %s", Config->LogSettingsFile.c_str());
	else
		_log(QUERYSERV__INIT, "Log settings loaded from %s", Config->LogSettingsFile.c_str());

	WorldShortName = Config->ShortName;

	_log(QUERYSERV__INIT, "Connecting to MySQL...");

	if (!database.Connect(
		Config->QSDatabaseHost.c_str(),
		Config->QSDatabaseUsername.c_str(),
		Config->QSDatabasePassword.c_str(),
		Config->QSDatabaseDB.c_str(),
		Config->QSDatabasePort)) {
		_log(WORLD__INIT_ERR, "Cannot continue without a database connection.");
		return(1);
	}

	if (signal(SIGINT, CatchSignal) == SIG_ERR)	{
		_log(QUERYSERV__ERROR, "Could not set signal handler");
		return 0;
	}
	if (signal(SIGTERM, CatchSignal) == SIG_ERR)	{
		_log(QUERYSERV__ERROR, "Could not set signal handler");
		return 0;
	}

	worldserver = new WorldServer;

	worldserver->Connect();

	lfguildmanager.LoadDatabase();

	while(RunLoops) {

		Timer::SetCurrentTime();

		if(LFGuildExpireTimer.Check())
			lfguildmanager.ExpireEntries();

		if (InterserverTimer.Check()) {
			if (worldserver->TryReconnect() && (!worldserver->Connected()))
				worldserver->AsyncConnect();
		}
		worldserver->Process();

		timeout_manager.CheckTimeouts();

		Sleep(100);
	}
}

void UpdateWindowTitle(char* iNewTitle) {
#ifdef _WINDOWS
	char tmp[500];
	if (iNewTitle) {
		snprintf(tmp, sizeof(tmp), "QueryServ: %s", iNewTitle);
	}
	else {
		snprintf(tmp, sizeof(tmp), "QueryServ");
	}
	SetConsoleTitle(tmp);
#endif
}
