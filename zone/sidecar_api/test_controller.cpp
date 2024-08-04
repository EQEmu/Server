#include "sidecar_api.h"

void SidecarApi::TestController(const httplib::Request &req, httplib::Response &res)
{
	nlohmann::json j;

	j["data"]["test"] = "test";

	res.set_content(j.dump(), "application/json");
}
