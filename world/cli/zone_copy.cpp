#include "../../common/eqemu_logsys_log_aliases.h"
#include "../../common/zone_copy.h"
#include "../../common/database.h"

void WorldserverCLI::ZoneCopyCmd(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Copies a character into a destination account";

	auto c = ZoneCopier();
	c.SetDatabase(&database);
	c.SetContentDatabase(&content_db);
	c.SetSourceShortName("soldungb");
	c.SetSourceVersion(0);
	c.SetDestinationVersion(1);
	c.ZoneCopy();
}
