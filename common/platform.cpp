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

#include "platform.h"

EQEmuExePlatform exe_platform = ExePlatformNone;

void RegisterExecutablePlatform(EQEmuExePlatform p)
{
	exe_platform = p;
}

const EQEmuExePlatform &GetExecutablePlatform()
{
	return exe_platform;
}

int GetExecutablePlatformInt()
{
	return exe_platform;
}

bool IsWorld()
{
	return exe_platform == EQEmuExePlatform::ExePlatformWorld;
}

bool IsQueryServ()
{
	return exe_platform == EQEmuExePlatform::ExePlatformQueryServ;
}

std::string GetPlatformName()
{
	switch (GetExecutablePlatformInt()) {
		case EQEmuExePlatform::ExePlatformWorld:
			return "World";
		case EQEmuExePlatform::ExePlatformQueryServ:
			return "QS";
		case EQEmuExePlatform::ExePlatformZone:
			return "Zone";
		case EQEmuExePlatform::ExePlatformUCS:
			return "UCS";
		case EQEmuExePlatform::ExePlatformLogin:
			return "Login";
		case EQEmuExePlatform::ExePlatformSharedMemory:
			return "SharedMem";
		case EQEmuExePlatform::ExePlatformClientImport:
			return "Import";
		case EQEmuExePlatform::ExePlatformClientExport:
			return "Export";
		case EQEmuExePlatform::ExePlatformLaunch:
			return "Launch";
		case EQEmuExePlatform::ExePlatformHC:
			return "HC";
		case EQEmuExePlatform::ExePlatformTests:
			return "Tests";
		case EQEmuExePlatform::ExePlatformZoneSidecar:
			return "ZoneSidecar";
		default:
			return "";
	}
}
