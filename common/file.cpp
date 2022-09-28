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
#include <filesystem>

namespace fs = std::filesystem;

/**
 * @param name
 * @return
 */
bool File::Exists(const std::string &name)
{
	return fs::exists(fs::path{name});
}

/**
 * @param directory_name
 */
void File::Makedir(const std::string &directory_name)
{
	fs::create_directory(directory_name);
	fs::permissions(directory_name, fs::perms::owner_all);
}

std::string File::FindEqemuConfigPath()
{
	if (File::Exists(fs::path{File::GetCwd() + "/eqemu_config.json"}.string())) {
		return File::GetCwd();
	}
	else if (File::Exists(fs::path{File::GetCwd() + "/../eqemu_config.json"}.string())) {
		return canonical(fs::path{File::GetCwd() + "/../"}).string();
	}
	else if (File::Exists(fs::path{File::GetCwd() + "/login.json"}.string())) {
		return File::GetCwd();
	}
	else if (File::Exists(fs::path{File::GetCwd() + "/../login.json"}.string())) {
		return canonical(fs::path{File::GetCwd() + "/../"}).string();
	}

	return {};
}

std::string File::GetCwd()
{
	return fs::current_path().string();
}
