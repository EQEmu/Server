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
#ifdef _WINDOWS
	#include <winsock2.h>
	#include <windows.h>
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <netdb.h>
#endif

#include "../common/types.h"
#include "../common/timer.h"
void CatchSignal(int);
void UpdateWindowTitle(char* iNewTitle = 0);

class NetConnection
{
public:
	~NetConnection();
	NetConnection();

	uint32	GetIP();
	uint32	GetIP(char* name);
	Timer object_timer;
	Timer door_timer;
	Timer corpse_timer;
	Timer group_timer;
	Timer raid_timer;
	Timer trap_timer;
};
