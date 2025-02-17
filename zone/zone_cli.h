#ifndef EQEMU_ZONE_CLI_H
#define EQEMU_ZONE_CLI_H

#include "../common/cli/argh.h"

class ZoneCLI {
public:
	static void CommandHandler(int argc, char **argv);
	static void BenchmarkDatabuckets(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void SidecarServeHttp(int argc, char **argv, argh::parser &cmd, std::string &description);
	static bool RanConsoleCommand(int argc, char **argv);
	static bool RanSidecarCommand(int argc, char **argv);
	static bool RanTestCommand(int argc, char **argv);
	static void NpcHandins(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void NpcHandinsMultiQuest(int argc, char **argv, argh::parser &cmd, std::string &description);
};


#endif //EQEMU_ZONE_CLI_H
