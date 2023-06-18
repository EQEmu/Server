#include "../worlddb.h"
#include "../../common/database_schema.h"

void WorldserverCLI::MercsEnable(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Enables mercenaries";

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	// bootstrap merc tables if they don't exist
	if (!database.DoesTableExist("merc_types")) {
		LogInfo("Bootstrapping merc tables");
		database.SourceSqlFromUrl(
			"https://raw.githubusercontent.com/EQEmu/Server/master/utils/sql/merc_tables_bootstrap.sql"
		);
	}
	else {
		LogInfo("Merc tables already exist, skipping bootstrap");
	}

	LogInfo("Enabling mercs");
	LogInfo("Setting rule Mercs:AllowMercs to true");
	RuleManager::Instance()->SetRule("Mercs:AllowMercs", "true", &database, true, true);
	LogInfo("Mercs enabled");
}
