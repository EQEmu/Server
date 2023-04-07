#include "../worlddb.h"
#include "../../common/database_schema.h"

void WorldserverCLI::BotsDisable(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Disables bots and drops tables";

	std::vector<std::string> arguments = {};
	std::vector<std::string> options   = {};

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	std::string sql = "SET FOREIGN_KEY_CHECKS = 0;";
	for (auto &t: DatabaseSchema::GetBotTables()) {
		sql += fmt::format("DROP TABLE IF EXISTS {};", t);
	}

	sql += "SET FOREIGN_KEY_CHECKS = 1;";

	database.QueryDatabaseMulti(sql);

	RuleManager::Instance()->SetRule("Bots:Enabled", "false", &database, true, true);

	LogInfo("Disabled bots and dropped tables");
}
