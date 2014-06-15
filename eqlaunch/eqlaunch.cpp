/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

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
#include "../common/ProcLauncher.h"
#include "../common/EQEmuConfig.h"
#include "../common/servertalk.h"
#include "../common/platform.h"
#include "../common/crash.h"
#include "worldserver.h"
#include "ZoneLaunch.h"
#include <vector>
#include <map>
#include <set>
#include <signal.h>
#include <time.h>

bool RunLoops = false;

void CatchSignal(int sig_num);

int main(int argc, char *argv[]) {
	RegisterExecutablePlatform(ExePlatformLaunch);
	set_exception_handler();
	std::string launcher_name;
	if(argc == 2)
		launcher_name = argv[1];
	if(launcher_name.length() < 1) {
		_log(LAUNCHER__ERROR, "You must specfify a launcher name as the first argument to this program.");
		return 1;
	}

	_log(LAUNCHER__INIT, "Loading server configuration..");
	if (!EQEmuConfig::LoadConfig()) {
		_log(LAUNCHER__ERROR, "Loading server configuration failed.");
		return 1;
	}
	const EQEmuConfig *Config = EQEmuConfig::get();
	if (signal(SIGINT, CatchSignal) == SIG_ERR)	{
		_log(LAUNCHER__ERROR, "Could not set signal handler");
		return 1;
	}
	if (signal(SIGTERM, CatchSignal) == SIG_ERR)	{
		_log(LAUNCHER__ERROR, "Could not set signal handler");
		return 1;
	}
	#ifndef WIN32
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)	{
		_log(LAUNCHER__ERROR, "Could not set signal handler");
		return 1;
	}
	
	const char *pv = getenv("LD_LIBRARY_PATH");
	if(pv == nullptr)
		putenv(strdup("LD_LIBRARY_PATH=."));
	else {
		char *v = (char *) malloc(strlen(pv) + 19);
		sprintf(v, "LD_LIBRARY_PATH=.:%s", pv);
		putenv(v);
	}
	#endif
	std::map<std::string, ZoneLaunch *> zones;
	WorldServer world(zones, launcher_name.c_str(), Config);
	if (!world.Connect())
		_log(LAUNCHER__ERROR, "worldserver.Connect() FAILED! Will retry.");
	std::map<std::string, ZoneLaunch *>::iterator zone, zend;
	std::set<std::string> to_remove;
	Timer InterserverTimer(INTERSERVER_TIMER); // does auto-reconnect
	_log(LAUNCHER__INIT, "Starting main loop...");
	ProcLauncher *launch = ProcLauncher::get();
	RunLoops = true;
	while(RunLoops) {
		Timer::SetCurrentTime();
		world.Process();
		launch->Process();
		zone = zones.begin();
		zend = zones.end();
		for(; zone != zend; ++zone) {
			if(!zone->second->Process())
				to_remove.insert(zone->first);
		}
		while(!to_remove.empty()) {
			std::string rem = *to_remove.begin();
			to_remove.erase(rem);
			zone = zones.find(rem);
			if(zone == zones.end())
				continue;
			delete zone->second;
			zones.erase(rem);
		}
		if (InterserverTimer.Check()) {
			if (world.TryReconnect() && (!world.Connected()))
				world.AsyncConnect();
		}
		if(zones.empty())
			Sleep(5000);
		else
			Sleep(2000);
	}
	zone = zones.begin();
	zend = zones.end();
	for(; zone != zend; ++zone)
		zone->second->Stop();
	Sleep(1);
	launch->Process();
	launch->TerminateAll(false);
	Sleep(1);
	launch->Process();
	//kill anybody left
	launch->TerminateAll(true);
	for(; zone != zend; ++zone)
		delete zone->second;
	return 0;
}

void CatchSignal(int sig_num) {
	_log(LAUNCHER__STATUS, "Caught signal %d", sig_num);
	RunLoops = false;
}