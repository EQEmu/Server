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

#include "../common/global_define.h"
#include "../common/eqemu_logsys.h"
#include "../common/opcodemgr.h"
#include "../common/rulesys.h"
#include "../common/servertalk.h"
#include "../common/platform.h"
#include "../common/crash.h"
#include "../common/string_util.h"
#include "../common/event/event_loop.h"
#include "../common/timer.h"
#include "database.h"
#include "queryservconfig.h"
#include "lfguild.h"
#include "worldserver.h"
#include <list>
#include <signal.h>
#include <thread>

volatile bool RunLoops = true;

Database              database;
LFGuildManager        lfguildmanager;
std::string           WorldShortName;
const queryservconfig *Config;
WorldServer           *worldserver = 0;
EQEmuLogSys           LogSys;

void CatchSignal(int sig_num)
{
	RunLoops = false;
}

int main()
{
	RegisterExecutablePlatform(ExePlatformQueryServ);
	LogSys.LoadLogSettingsDefaults();
	set_exception_handler();
	Timer LFGuildExpireTimer(60000);

	LogInfo("Starting EQEmu QueryServ");
	if (!queryservconfig::LoadConfig()) {
		LogInfo("Loading server configuration failed");
		return 1;
	}

	Config         = queryservconfig::get();
	WorldShortName = Config->ShortName;

	LogInfo("Connecting to MySQL");

	/* MySQL Connection */
	if (!database.Connect(
		Config->QSDatabaseHost.c_str(),
		Config->QSDatabaseUsername.c_str(),
		Config->QSDatabasePassword.c_str(),
		Config->QSDatabaseDB.c_str(),
		Config->QSDatabasePort
	)) {
		LogInfo("Cannot continue without a database connection");
		return 1;
	}

	LogSys.SetDatabase(&database)
		->LoadLogDatabaseSettings()
		->StartFileLogs();

	if (signal(SIGINT, CatchSignal) == SIG_ERR) {
		LogInfo("Could not set signal handler");
		return 1;
	}
	if (signal(SIGTERM, CatchSignal) == SIG_ERR) {
		LogInfo("Could not set signal handler");
		return 1;
	}

	/* Initial Connection to Worldserver */
	worldserver = new WorldServer;
	worldserver->Connect();

	/* Load Looking For Guild Manager */
	lfguildmanager.LoadDatabase();

	while (RunLoops) {
		Timer::SetCurrentTime();
		if (LFGuildExpireTimer.Check()) {
			lfguildmanager.ExpireEntries();
		}

		EQ::EventLoop::Get().Process();
		Sleep(5);
	}
	LogSys.CloseFileLogs();
}

void UpdateWindowTitle(char *iNewTitle)
{
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
