#include "zone_cli.h"
#include "../common/cli/eqemu_command_handler.h"
#include <string.h>

bool ZoneCLI::RanConsoleCommand(int argc, char **argv)
{
	return argc > 1 && (strstr(argv[1], ":") != nullptr || strstr(argv[1], "--") != nullptr);
}

bool ZoneCLI::RanSidecarCommand(int argc, char **argv)
{
	return argc > 1 && (strstr(argv[1], "sidecar:") != nullptr);
}

void ZoneCLI::CommandHandler(int argc, char **argv)
{
	if (argc == 1) { return; }

	argh::parser cmd;
	cmd.parse(argc, argv, argh::parser::PREFER_PARAM_FOR_UNREG_OPTION);
	EQEmuCommand::DisplayDebug(cmd);

	// Declare command mapping
	auto function_map = EQEmuCommand::function_map;

	// Register commands
	function_map["sidecar:serve-http"] = &ZoneCLI::SidecarServeHttp;

	EQEmuCommand::HandleMenu(function_map, cmd, argc, argv);
}

#include "cli/sidecar_serve_http.cpp"
