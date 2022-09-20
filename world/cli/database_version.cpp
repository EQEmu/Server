#include "../../common/version.h"
#include "../../common/json/json.h"

void WorldserverCLI::DatabaseVersion(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Shows database version";

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	Json::Value database_version;

	database_version["database_version"]      = CURRENT_BINARY_DATABASE_VERSION;
	database_version["bots_database_version"] = CURRENT_BINARY_BOTS_DATABASE_VERSION;

	std::stringstream payload;
	payload << database_version;

	std::cout << payload.str() << std::endl;
}
