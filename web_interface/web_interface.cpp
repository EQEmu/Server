#include "../common/debug.h"
#include "../common/opcodemgr.h"
#include "../common/EQStreamFactory.h"
#include "../common/rulesys.h"
#include "../common/servertalk.h"
#include "../common/platform.h"
#include "../common/crash.h"
#include "../common/EQEmuConfig.h"
#include "worldserver.h"
#include <signal.h>

volatile bool run = true;
TimeoutManager timeout_manager;
const EQEmuConfig *config = nullptr;
WorldServer *worldserver = nullptr;

void CatchSignal(int sig_num) {
	run = false;
	if(worldserver)
		worldserver->Disconnect();
}

int main() {
	RegisterExecutablePlatform(ExePlatformWebInterface);
	set_exception_handler();
	Timer InterserverTimer(INTERSERVER_TIMER); // does auto-reconnect
	_log(WEB_INTERFACE__INIT, "Starting EQEmu Socket Server.");
	
	if (signal(SIGINT, CatchSignal) == SIG_ERR)	{
		_log(WEB_INTERFACE__ERROR, "Could not set signal handler");
		return 1;
	}
	
	if (signal(SIGTERM, CatchSignal) == SIG_ERR)	{
		_log(WEB_INTERFACE__ERROR, "Could not set signal handler");
		return 1;
	}

	config = EQEmuConfig::get();

	worldserver = new WorldServer(config->SharedKey); 
	worldserver->Connect();

		while(run) { 
		Timer::SetCurrentTime(); 
		if (InterserverTimer.Check()) {
			if (worldserver->TryReconnect() && (!worldserver->Connected()))
				worldserver->AsyncConnect();
		}
		worldserver->Process(); 
		timeout_manager.CheckTimeouts();
		Sleep(1);
	}

	return 0;
}

