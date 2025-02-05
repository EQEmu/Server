#include "world_server_cli.h"

/**
 * @param argc
 * @param argv
 */
void WorldserverCLI::CommandHandler(int argc, char **argv)
{
	if (argc == 1) { return; }

	argh::parser cmd;
	cmd.parse(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);
	EQEmuCommand::DisplayDebug(cmd);

	// Declare command mapping
	auto function_map = EQEmuCommand::function_map;

	// Register commands
	function_map["bots:enable"]                 = &WorldserverCLI::BotsEnable;
	function_map["bots:disable"]                = &WorldserverCLI::BotsDisable;
	function_map["mercs:enable"]                = &WorldserverCLI::MercsEnable;
	function_map["mercs:disable"]               = &WorldserverCLI::MercsDisable;
	function_map["world:version"]               = &WorldserverCLI::Version;
	function_map["character:copy-character"]    = &WorldserverCLI::CopyCharacter;
	function_map["database:version"]            = &WorldserverCLI::DatabaseVersion;
	function_map["database:set-account-status"] = &WorldserverCLI::DatabaseSetAccountStatus;
	function_map["database:schema"]             = &WorldserverCLI::DatabaseGetSchema;
	function_map["database:dump"]               = &WorldserverCLI::DatabaseDump;
	function_map["database:updates"]            = &WorldserverCLI::DatabaseUpdates;
	function_map["test:test"]                   = &WorldserverCLI::TestCommand;
	function_map["test:colors"]                 = &WorldserverCLI::TestColors;
	function_map["test:expansion"]              = &WorldserverCLI::ExpansionTestCommand;
	function_map["test:repository"]             = &WorldserverCLI::TestRepository;
	function_map["test:repository2"]            = &WorldserverCLI::TestRepository2;
	function_map["test:db-concurrency"]         = &WorldserverCLI::TestDatabaseConcurrency;
	function_map["test:string-benchmark"]       = &WorldserverCLI::TestStringBenchmarkCommand;
	function_map["etl:settings"]                = &WorldserverCLI::EtlGetSettings;

	EQEmuCommand::HandleMenu(function_map, cmd, argc, argv);
}

#include "cli/database_concurrency.cpp"
#include "cli/bots_enable.cpp"
#include "cli/bots_disable.cpp"
#include "cli/mercs_enable.cpp"
#include "cli/mercs_disable.cpp"
#include "cli/copy_character.cpp"
#include "cli/database_updates.cpp"
#include "cli/database_dump.cpp"
#include "cli/database_get_schema.cpp"
#include "cli/database_set_account_status.cpp"
#include "cli/database_version.cpp"
#include "cli/test.cpp"
#include "cli/test_colors.cpp"
#include "cli/test_expansion.cpp"
#include "cli/test_repository.cpp"
#include "cli/test_repository_2.cpp"
#include "cli/test_string_benchmark.cpp"
#include "cli/version.cpp"
#include "cli/etl_get_settings.cpp"
