/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

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

#ifndef WORLDCONNECTION_H
#define WORLDCONNECTION_H

#include "../common/emu_tcp_connection.h"
#include <string>

class ServerPacket;

/*
 * This object is an arbitrary connection to world.
 */
class WorldConnection {
public:
	WorldConnection(EmuTCPConnection::ePacketMode mode, const char *password = "");
	virtual ~WorldConnection();

	virtual void Process();
	bool SendPacket(ServerPacket* pack);

	uint32	GetIP()	const	{ return tcpc.GetrIP(); }
	uint16	GetPort() const	{ return tcpc.GetrPort(); }
	bool	Connected() const	{ return (pConnected && tcpc.Connected()); }

	void	SetPassword(const char *password) { m_password = password; }
	bool	Connect();
	void	AsyncConnect();
	void	Disconnect();
	inline bool		TryReconnect() const { return pTryReconnect; }

protected:
	virtual void OnConnected();

	std::string m_password;
	EmuTCPConnection tcpc;
	bool	pTryReconnect;
	bool	pConnected;
};


#endif

