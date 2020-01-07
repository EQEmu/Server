#include "../common/event/event_loop.h"
#include "../common/eqemu_logsys.h"
#include "../common/crash.h"
#include "../common/json_config.h"
#include <thread>

#include "eq.h"

int main() {
	EQEmuLogSys::Get()->LoadLogSettingsDefaults("headless_client");
	set_exception_handler();

	LogInfo("Starting EQEmu Headless Client.");

	auto config = EQ::JsonConfigFile::Load("hc.json");
	auto config_handle = config.RawHandle();

	std::vector<std::unique_ptr<EverQuest>> eq_list;

	try {
		for (int i = 0; i < config_handle.size(); ++i) {
			auto c = config_handle[i];

			auto host = c["host"].asString();
			auto port = c["port"].asInt();
			auto user = c["user"].asString();
			auto pass = c["pass"].asString();
			auto server = c["server"].asString();
			auto character = c["character"].asString();
			
			LogInfo("Connecting to {0}:{1} as Account '{2}' to Server '{3}' under Character '{4}'", host, port, user, server, character);

			eq_list.push_back(std::unique_ptr<EverQuest>(new EverQuest(host, port, user, pass, server, character)));
		}
	}
	catch (std::exception &ex) {
		LogError("Error parsing config file: {0}", ex.what());
		return 0;
	}

	for (;;) {
		EQ::EventLoop::Get().Process();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	
	return 0;
}
