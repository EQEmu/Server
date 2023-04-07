#include "../worlddb.h"
#include "../../common/rulesys.h"

void WorldserverCLI::BotsBootstrap(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Bootstraps bot tables and enables bots";

	std::vector<std::string> arguments = {};
	std::vector<std::string> options   = {};

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	LogInfo("Bootstrapping bots");
	database.SourceSqlFromUrl("https://raw.githubusercontent.com/EQEmu/Server/akkadius/db-migration-native/utils/sql/bot_tables_bootstrap.sql");

	RuleManager::Instance()->SetRule("Bots:Enabled", "true", &database, true, true);
	LogInfo("Bootstrapping bots... done");
}
