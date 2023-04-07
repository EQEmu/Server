#include "../../common/database/database_update.h"
#include "../worlddb.h"

void WorldserverCLI::MercsBootstrap(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Bootstraps merc tables";

	std::vector<std::string> arguments = {};
	std::vector<std::string> options   = {};

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	LogInfo("Bootstrapping mercenaries tables");
	database.SourceSqlFromUrl("https://raw.githubusercontent.com/EQEmu/Server/akkadius/db-migration-native/utils/sql/merc_tables_bootstrap.sql");
	RuleManager::Instance()->SetRule("Mercs:AllowMercs", "true", &database, true, true);
	LogInfo("Bootstrapping mercenaries tables... done");
}
