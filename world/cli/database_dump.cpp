#include "../../common/database/database_dump_service.h"

void WorldserverCLI::DatabaseDump(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Dumps server database tables";

	std::vector<std::string> arguments = {};
	std::vector<std::string> options   = {
		"--all",
		"--content-tables",
		"--login-tables",
		"--player-tables",
		"--bot-tables",
		"--merc-tables",
		"--state-tables",
		"--system-tables",
		"--query-serv-tables",
		"--table-structure-only",
		"--table-lock",
		"--dump-path=",
		"--dump-output-to-console",
		"--drop-table-syntax-only",
		"--compress"
	};

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	EQEmuCommand::ValidateCmdInput(arguments, options, cmd, argc, argv);

	auto s        = new DatabaseDumpService();
	bool dump_all = cmd[{"-a", "--all"}];

	if (!cmd("--dump-path").str().empty()) {
		s->SetDumpPath(cmd("--dump-path").str());
	}

	/**
	 * Set Option
	 */
	s->SetDumpContentTables(cmd[{"--content-tables"}] || dump_all);
	s->SetDumpLoginServerTables(cmd[{"--login-tables"}] || dump_all);
	s->SetDumpPlayerTables(cmd[{"--player-tables"}] || dump_all);
	s->SetDumpBotTables(cmd[{"--bot-tables"}] || dump_all);
	s->SetDumpMercTables(cmd[{"--merc-tables"}] || dump_all);
	s->SetDumpStateTables(cmd[{"--state-tables"}] || dump_all);
	s->SetDumpSystemTables(cmd[{"--system-tables"}] || dump_all);
	s->SetDumpQueryServerTables(cmd[{"--query-serv-tables"}] || dump_all);
	s->SetDumpAllTables(dump_all);

	s->SetDumpWithNoData(cmd[{"--table-structure-only"}]);
	s->SetDumpTableLock(cmd[{"--table-lock"}]);
	s->SetDumpWithCompression(cmd[{"--compress"}]);
	s->SetDumpOutputToConsole(cmd[{"--dump-output-to-console"}]);
	s->SetDumpDropTableSyntaxOnly(cmd[{"--drop-table-syntax-only"}]);

	/**
	 * Dump
	 */
	s->Dump();
}
