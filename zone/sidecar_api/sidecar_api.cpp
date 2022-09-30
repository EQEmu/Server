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
#include "map_best_z_controller.cpp"
#include "loot_simulator_controller.cpp"
#include "../../common/file.h"

constexpr static int HTTP_RESPONSE_OK           = 200;
constexpr static int HTTP_RESPONSE_BAD_REQUEST  = 400;
constexpr static int HTTP_RESPONSE_UNAUTHORIZED = 401;

std::string authorization_key;

void SidecarApi::BootWebserver(int port, const std::string &key)
{
	LogInfo("Booting zone sidecar API");

	std::signal(SIGINT, CatchSidecarSignal);
	std::signal(SIGTERM, CatchSidecarSignal);
	std::signal(SIGKILL, CatchSidecarSignal);

	if (!key.empty()) {
		authorization_key = key;
		LogInfo("Booting with authorization key [{}]", authorization_key);
	}

	int         web_api_port = port > 0 ? port : 9080;
	std::string hotfix_name  = "zonesidecar_api_";

	// bake shared memory if it doesn't exist
	// TODO: Windows
	if (!File::Exists("shared/zonesidecar_api_loot_drop")) {
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
	api.set_pre_routing_handler(
		[](const auto &req, auto &res) {
			for (const auto &header: req.headers) {
				auto header_key   = header.first;
				auto header_value = header.second;
				if (header_key == "Authorization") {
					std::string auth_key = header_value;
					Strings::FindReplace(auth_key, "Bearer ", "");

					// authorization key matches, pass the request on to the route handler
					if (auth_key == authorization_key) {
						return httplib::Server::HandlerResponse::Unhandled;
					}
					else {
						LogInfo("[API] Authorization key [{}] not authorized", auth_key);
					}
				}
			}

			nlohmann::json j;
			j["error"] = "Authorization key not authorized!";
			res.set_content(j.dump(), "application/json");
			res.status = HTTP_RESPONSE_UNAUTHORIZED;

			return httplib::Server::HandlerResponse::Handled;
		}
	);
	api.Get("/api/v1/test-controller", SidecarApi::TestController);
	api.Get("/api/v1/loot-simulate", SidecarApi::LootSimulatorController);

	LogInfo("Webserver API now listening on port [{0}]", web_api_port);
	api.listen("0.0.0.0", web_api_port);
}
