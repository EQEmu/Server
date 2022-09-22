#include "../../common/http/httplib.h"
#include "../../common/eqemu_logsys.h"
#include "../sidecar_api/sidecar_api.h"

void ZoneCLI::SidecarServeHttp(int argc, char **argv, argh::parser &cmd, std::string &description)
{
//	std::thread web_api_thread(start_web_server);
//	web_api_thread.detach();

	int port = 0;
	if (!cmd("--port").str().empty()) {
		port = strtoll(cmd("--port").str().c_str(), nullptr, 10);
	}

	SidecarApi::BootWebserver(port);
}
