#include "../../common/http/httplib.h"
#include "../../common/eqemu_logsys.h"
#include "../sidecar_api/sidecar_api.h"
#include "../../common/platform.h"

void ZoneCLI::PurgeExpiredInstances(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	if (cmd[{"-h", "--help"}]) {
		return;
	}

	database.PurgeExpiredInstances();
}
