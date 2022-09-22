/**
 * EQEmulator: Everquest Server Emulator
 * Copyright (C) 2001-2020 EQEmulator Development Team (https://github.com/EQEmu/Server)
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

#include <fstream>
#include "file.h"

#ifdef _WINDOWS
#include <direct.h>
#include <conio.h>
#include <iostream>
#include <dos.h>
#include <windows.h>
#include <process.h>
#else

#include <unistd.h>
#include <sys/stat.h>

#endif

#include <fmt/format.h>

/**
 * @param name
 * @return
 */
bool File::Exists(const std::string &name)
{
	std::ifstream f(name.c_str());

	return f.good();
}

/**
 * @param directory_name
 */
void File::Makedir(const std::string &directory_name)
{
#ifdef _WINDOWS
	struct _stat st;
	if (_stat(directory_name.c_str(), &st) == 0) // exists
		return;
	_mkdir(directory_name.c_str());
#else
	struct stat st{};
	if (stat(directory_name.c_str(), &st) == 0) { // exists
		return;
	}
	::mkdir(directory_name.c_str(), 0755);
#endif
}

std::string File::FindEqemuConfigPath()
{
	std::string path = fmt::format("{}/eqemu_config.json", File::GetCwd());
	if (File::Exists(path)) {
		return fmt::format("{}/", File::GetCwd());
	}

	path = fmt::format("{}/../eqemu_config.json",File::GetCwd());
	if (File::Exists(path)) {
		return fmt::format("{}/../", File::GetCwd());
	}

	return {};
}

std::string File::GetCwd()
{
	char        buffer[100];
	char        *answer = getcwd(buffer, sizeof(buffer));
	std::string s_cwd;
	if (answer) {
		s_cwd = answer;
	}

	return s_cwd;
}
