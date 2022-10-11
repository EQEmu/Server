#include "world_api.h"

constexpr static int HTTP_RESPONSE_OK           = 200;
constexpr static int HTTP_RESPONSE_BAD_REQUEST  = 400;
constexpr static int HTTP_RESPONSE_UNAUTHORIZED = 401;

#include "../../common/eqemu_logsys.h"
#include "../../common/strings.h"
#include "log_handler.cpp"
#include "../../common/json/json.hpp"

std::string authorization_key;

// routes have to be defined here in order to be proxied to the zone sidecar
std::vector<std::string> zone_sidecar_get_routes = {
	"/api/v1/test-controller",
	"/api/v1/loot-simulate"
};

httplib::Client client("localhost:9099");

void WorldApi::BootWebserver(int port, const std::string &sidecar_key)
{
	LogInfo("Booting zone sidecar API");

	if (!sidecar_key.empty()) {
		authorization_key = sidecar_key;
		LogInfo("Booting with sidecar key [{}]", authorization_key);
	}

	int web_api_port = port > 0 ? port : 9081;

	httplib::Server api;

	client.set_connection_timeout(1, 0);
	client.set_read_timeout(1, 0);
	client.set_write_timeout(1, 0);
	client.set_bearer_token_auth(authorization_key);

	api.set_logger(WorldApi::LogHandler);
	api.set_pre_routing_handler(
		[](const httplib::Request &req, httplib::Response &res) {
			for (const std::string &route: zone_sidecar_get_routes) {
				if (route == req.path) {
					httplib::Headers headers = {
						{"Content-Type",  "application/json"},
						{"Authorization", "Bearer test"}
					};

					auto r = client.Get(
						req.path, req.params, req.headers, [](uint64_t len, uint64_t total) {
							return true;
						}
					);
					res.status = r->status;
					res.set_content(r->body, "application/json");

					LogHTTPDetail("Proxy body content is [{}]", r->body);

					return httplib::Server::HandlerResponse::Unhandled;
				}
			}

			nlohmann::json j;
			j["error"] = "Invalid route";
			res.set_content(j.dump(), "application/json");
			res.status = HTTP_RESPONSE_UNAUTHORIZED;

			return httplib::Server::HandlerResponse::Handled;
		}
	);

	for (const std::string &r: zone_sidecar_get_routes) {
		LogHTTP("Registering GET route [{}]", r);
		api.Get(
			r, [](const httplib::Request &req, httplib::Response &res) {
				LogHTTPDetail("Matching zone sidecar route handler [{}]", req.path);
			}
		);
	}

	LogInfo("Webserver API now listening on port [{0}]", web_api_port);
	api.listen("0.0.0.0", web_api_port);
}
