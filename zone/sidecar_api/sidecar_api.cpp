#include "sidecar_api.h"
#include "../../common/http/httplib.h"
#include "../../common/eqemu_logsys.h"
#include "../zonedb.h"
#include "../../shared_memory/loot.h"
#include "../../common/process.h"
#include "../common.h"
#include "../zone.h"
#include "../client.h"
#include "../../common/json/json.hpp"

void CatchSidecarSignal(int sig_num)
{
	LogInfo("[SidecarAPI] Caught signal [{}]", sig_num);
	LogInfo("Forcefully exiting for now");
	std::exit(0);
}

#include "log_handler.cpp"
#include "test_controller.cpp"
#include "loot_simulator_controller.cpp"
#include "../../common/file_util.h"

void SidecarApi::BootWebserver(int port)
{
	LogInfo("Booting zone sidecar API");

	std::signal(SIGINT, CatchSidecarSignal);
	std::signal(SIGTERM, CatchSidecarSignal);
	std::signal(SIGKILL, CatchSidecarSignal);

	int         web_api_port = port > 0 ? port : 9080;
	std::string hotfix_name  = "zonesidecar_api_";

	// bake shared memory if it doesn't exist
	if (!file_exists("shared/zonesidecar_api_loot_drop")) {
		LogInfo("Creating shared memory for prefix [{}]", hotfix_name);

		std::string output = Process::execute(
			fmt::format(
				"./bin/shared_memory -hotfix={} loot items",
				hotfix_name
			)
		);
		std::cout << output << "\n";
	}

	LogInfo("Loading loot tables");
	if (!database.LoadLoot(hotfix_name)) {
		LogError("Loading loot failed!");
	}

	// bootup a fake zone
	Zone::Bootup(ZoneID("qrg"), 0, false);
	zone->StopShutdownTimer();

	httplib::Server api;

	api.set_logger(SidecarApi::LogHandler);
	api.Get("/v1/test-controller", SidecarApi::TestController);
	api.Get("/v1/loot-simulate", SidecarApi::LootSimulatorController);

	LogInfo("Webserver API now listening on port [{0}]", web_api_port);
	api.listen("0.0.0.0", web_api_port);
}
