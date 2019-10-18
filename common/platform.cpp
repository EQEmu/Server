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

void RegisterExecutablePlatform(EQEmuExePlatform p) {
	exe_platform = p;
}

const EQEmuExePlatform& GetExecutablePlatform() {
	return exe_platform;
}

/**
 * @return
 */
int GetExecutablePlatformInt(){
	return exe_platform;
}

/**
 * Returns platform name by string
 *
 * @return
 */
std::string GetPlatformName()
{
	switch (GetExecutablePlatformInt()) {
		case EQEmuExePlatform::ExePlatformWorld:
			return "WorldServer";
		case EQEmuExePlatform::ExePlatformQueryServ:
			return "QueryServer";
		case EQEmuExePlatform::ExePlatformZone:
			return "ZoneServer";
		case EQEmuExePlatform::ExePlatformUCS:
			return "UCS";
		case EQEmuExePlatform::ExePlatformLogin:
			return "LoginServer";
		case EQEmuExePlatform::ExePlatformSocket_Server:
			return "SocketServer";
		case EQEmuExePlatform::ExePlatformSharedMemory:
			return "SharedMemory";
		case EQEmuExePlatform::ExePlatformClientImport:
			return "ClientImport";
		case EQEmuExePlatform::ExePlatformClientExport:
			return "ClientExport";
		case EQEmuExePlatform::ExePlatformLaunch:
			return "Launch";
		case EQEmuExePlatform::ExePlatformHC:
			return "HC";
		default:
			return "";
	}
}