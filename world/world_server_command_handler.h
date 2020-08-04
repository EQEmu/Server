/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2019 EQEmulator Development Team (https://github.com/EQEmu/Server)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY except by those people which sell it, which
 * are required to give you total support for your newly bought product;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include "iostream"
#include "../common/cli/eqemu_command_handler.h"

#ifndef EQEMU_WORLD_SERVER_COMMAND_HANDLER_H
#define EQEMU_WORLD_SERVER_COMMAND_HANDLER_H

namespace WorldserverCommandHandler {
	void CommandHandler(int argc, char **argv);
	void Version(int argc, char **argv, argh::parser &cmd, std::string &description);
	void CopyCharacter(int argc, char **argv, argh::parser &cmd, std::string &description);
	void DatabaseVersion(int argc, char **argv, argh::parser &cmd, std::string &description);
	void DatabaseSetAccountStatus(int argc, char **argv, argh::parser &cmd, std::string &description);
	void DatabaseGetSchema(int argc, char **argv, argh::parser &cmd, std::string &description);
	void DatabaseDump(int argc, char **argv, argh::parser &cmd, std::string &description);
	void TestCommand(int argc, char **argv, argh::parser &cmd, std::string &description);
	void ExpansionTestCommand(int argc, char **argv, argh::parser &cmd, std::string &description);
	void TestRepository(int argc, char **argv, argh::parser &cmd, std::string &description);
	void TestRepository2(int argc, char **argv, argh::parser &cmd, std::string &description);
};


#endif //EQEMU_WORLD_SERVER_COMMAND_HANDLER_H
