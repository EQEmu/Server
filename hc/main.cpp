#include "../common/event/event_loop.h"
#include "../common/eqemu_logsys.h"
#include "../common/crash.h"
#include "../common/platform.h"
#include <thread>

#include "login.h"
#include "world.h"

EQEmuLogSys Log;

int main() {
	RegisterExecutablePlatform(ExePlatformHC);
	Log.LoadLogSettingsDefaults();
	set_exception_handler();

	Log.OutF(Logs::General, Logs::Headless_Client, "Starting EQEmu Headless Client.");

	std::unique_ptr<LoginConnection> login_connection(new LoginConnection("testuser", "testpass", "127.0.0.1", 5999, "KLS Test"));
	std::unique_ptr<WorldConnection> world_connection;
	login_connection->OnCanLoginToWorld([&](const WorldServer &ws, const std::string &key, uint32_t dbid) {
		Log.OutF(Logs::General, Logs::Headless_Client, "Connect to world server {1} - {0}:9000", ws.address, ws.long_name);
		world_connection.reset(new WorldConnection(key, dbid, ws.address));
	});

	for (;;) {
		EQ::EventLoop::Get().Process();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	
	return 0;
}
