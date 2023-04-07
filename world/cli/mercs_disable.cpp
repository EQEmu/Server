#include "../worlddb.h"
#include "../../common/database_schema.h"

void WorldserverCLI::MercsDisable(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Disable mercenaries";

	std::vector<std::string> arguments = {};
	std::vector<std::string> options   = {};

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	std::string sql = "SET FOREIGN_KEY_CHECKS = 0;";
	for (auto &t: DatabaseSchema::GetMercTables()) {
		sql += fmt::format("DROP TABLE IF EXISTS {};", t);
	}

	sql += "SET FOREIGN_KEY_CHECKS = 1;";

	database.QueryDatabaseMulti(sql);
	RuleManager::Instance()->SetRule("Mercs:AllowMercs", "false", &database, true, true);
	LogInfo("Disabled mercs and dropped tables");
}
