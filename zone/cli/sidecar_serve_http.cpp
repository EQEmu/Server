#include "../../common/http/httplib.h"
#include "../../common/eqemu_logsys.h"
#include "../sidecar_api/sidecar_api.h"
#include "../../common/platform.h"

void ZoneCLI::SidecarServeHttp(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	RegisterExecutablePlatform(EQEmuExePlatform::ExePlatformZoneSidecar);

	int port = 0;
	if (!cmd("--port").str().empty()) {
		port = strtoll(cmd("--port").str().c_str(), nullptr, 10);
	}

	SidecarApi::BootWebserver(port);
}
