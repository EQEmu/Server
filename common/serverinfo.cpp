/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
// Serverinfo.cpp - Server information gathering functions, used in #serverinfo - Windows specific
// I'm not sure quite how to get this exact information in *nix, hopefully someone can fill that in
// -T7g
// Implement preliminary support for *nix variants
// misanthropicfiend

#ifdef _WINDOWS
#include <windows.h>

char Ver_name[100];
DWORD Ver_build, Ver_min, Ver_maj, Ver_pid;

int GetOS()	{

	strcpy(Ver_name, "Unknown operating system");

	OSVERSIONINFO Ver_os;
	Ver_os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if(!(GetVersionEx(&Ver_os))) return 1;

	Ver_build = Ver_os.dwBuildNumber & 0xFFFF;
	Ver_min = Ver_os.dwMinorVersion;
	Ver_maj = Ver_os.dwMajorVersion;
	Ver_pid = Ver_os.dwPlatformId;

	if ((Ver_pid == 1) && (Ver_maj == 4))
	{
		if ((Ver_min < 10) && (Ver_build == 950))
		{
			strcpy(Ver_name, "Microsoft Windows 95");
		}
		else if ((Ver_min < 10) &&
				((Ver_build > 950) && (Ver_build <= 1080)))
		{
			strcpy(Ver_name, "Microsoft Windows 95 SP1");
		}
		else if ((Ver_min < 10) && (Ver_build > 1080))
		{
			strcpy(Ver_name, "Microsoft Windows 95 OSR2");
		}
		else if ((Ver_min == 10) && (Ver_build == 1998))
		{
			strcpy(Ver_name, "Microsoft Windows 98");
		}
		else if ((Ver_min == 10) &&
				((Ver_build > 1998) && (Ver_build < 2183)))
		{
			strcpy(Ver_name, "Microsoft Windows 98, Service Pack 1");
		}
		else if ((Ver_min == 10) && (Ver_build >= 2183))
		{
			strcpy(Ver_name, "Microsoft Windows 98 Second Edition");
		}
		else if (Ver_min == 90)
		{
			strcpy(Ver_name, "Microsoft Windows ME");
		}
	}
	else if (Ver_pid == 2)
	{
		if ((Ver_maj == 3) && (Ver_min == 51))
		{
			strcpy(Ver_name, "Microsoft Windows NT 3.51");
		}
		else if ((Ver_maj == 4) && (Ver_min == 0))
		{
			strcpy(Ver_name, "Microsoft Windows NT 4");
		}
		else if ((Ver_maj == 5) && (Ver_min == 0))
		{
			strcpy(Ver_name, "Microsoft Windows 2000");
		}
		else if ((Ver_maj == 5) && (Ver_min == 1))
		{
			strcpy(Ver_name, "Microsoft Windows XP");
		}
		else if ((Ver_maj == 5) && (Ver_min == 2))
		{
			strcpy(Ver_name, "Microsoft Windows 2003");
		}
	}

	return 0;
}

#else

#include <sys/utsname.h>
#include <stdio.h>
#include <string.h>

char* GetOS(char* os_string) {
	utsname info;

	if(uname(&info)==0) {
		snprintf(os_string, 99, "%s %s %s %s %s", info.sysname, info.nodename, info.release, info.version, info.machine);
	} else {
		strncpy(os_string, "Error determining OS & version!", 25);
	}

	return os_string;

}

#endif
