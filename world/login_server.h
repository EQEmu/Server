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
#ifndef LOGINSERVER_H
#define LOGINSERVER_H

#include "../common/servertalk.h"
#include "../common/linked_list.h"
#include "../common/timer.h"
#include "../common/queue.h"
#include "../common/eq_packet_structs.h"
#include "../common/mutex.h"
#include "../common/emu_tcp_connection.h"

class LoginServer{
public:
	LoginServer(const char*, uint16, const char*, const char*);
	~LoginServer();

	bool InitLoginServer();

	bool Process();
	bool Connect();

	void SendInfo();
	void SendNewInfo();
	void SendStatus();

	void SendPacket(ServerPacket* pack) { tcpc->SendPacket(pack); }
	void SendAccountUpdate(ServerPacket* pack);
	bool ConnectReady() { return tcpc->ConnectReady(); }
	bool Connected() { return tcpc->Connected(); }
	bool MiniLogin() { return minilogin; }
	bool CanUpdate() { return CanAccountUpdate; }

private:
	bool minilogin;
	EmuTCPConnection* tcpc;
	char	LoginServerAddress[256];
	uint32	LoginServerIP;
	uint16	LoginServerPort;
	char	LoginAccount[32];
	char	LoginPassword[32];
	bool	CanAccountUpdate;

	Timer statusupdate_timer;
};
#endif
