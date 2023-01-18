#include "iostream"
#include "../common/cli/eqemu_command_handler.h"

#ifndef EQEMU_WORLD_SERVER_COMMAND_HANDLER_H
#define EQEMU_WORLD_SERVER_COMMAND_HANDLER_H

class WorldserverCLI {
public:
	static void CommandHandler(int argc, char **argv);
	static void Version(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void CopyCharacter(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void DatabaseVersion(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void DatabaseSetAccountStatus(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void DatabaseGetSchema(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void DatabaseDump(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void TestCommand(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void TestColors(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void ExpansionTestCommand(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void TestRepository(int argc, char **argv, argh::parser &cmd, std::string &description);
	static void TestRepository2(int argc, char **argv, argh::parser &cmd, std::string &description);
};


#endif //EQEMU_WORLD_SERVER_COMMAND_HANDLER_H
