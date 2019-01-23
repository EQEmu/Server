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
#include "../common/net/servertalk_client_connection.h"
#include "../common/net/servertalk_legacy_client_connection.h"
#include "../common/event/timer.h"
#include <memory>

class LoginServer{
public:
	LoginServer(const char*, uint16, const char*, const char*, bool legacy);
	~LoginServer();

	bool Connect();

	void SendInfo();
	void SendNewInfo();
	void SendStatus();

	void SendPacket(ServerPacket* pack);
	void SendAccountUpdate(ServerPacket* pack);
	bool Connected();
	bool MiniLogin() { return minilogin; }
	bool CanUpdate() { return CanAccountUpdate; }

private:
	void ProcessUsertoWorldReq(uint16_t opcode, EQ::Net::Packet &p);
	void ProcessLSClientAuth(uint16_t opcode, EQ::Net::Packet &p);
	void ProcessLSFatalError(uint16_t opcode, EQ::Net::Packet &p);
	void ProcessSystemwideMessage(uint16_t opcode, EQ::Net::Packet &p);
	void ProcessLSRemoteAddr(uint16_t opcode, EQ::Net::Packet &p);
	void ProcessLSAccountUpdate(uint16_t opcode, EQ::Net::Packet &p);

	bool minilogin;
	std::unique_ptr<EQ::Net::ServertalkClient> client;
	std::unique_ptr<EQ::Net::ServertalkLegacyClient> legacy_client;
	std::unique_ptr<EQ::Timer> statusupdate_timer;
	char	LoginServerAddress[256];
	uint32	LoginServerIP;
	uint16	LoginServerPort;
	char	LoginAccount[32];
	char	LoginPassword[32];
	bool	CanAccountUpdate;
	bool    IsLegacy;
};
#endif
