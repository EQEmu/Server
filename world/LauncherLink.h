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
#ifndef LAUNCHERLINK_H_
#define LAUNCHERLINK_H_

#include "../common/EmuTCPConnection.h"
#include "../common/timer.h"
#include <string>
#include <vector>
#include <map>

class ServerPacket;

class LauncherLink {
public:
	LauncherLink(int id, EmuTCPConnection *tcpc);
	~LauncherLink();

	bool		Process();
	bool		SendPacket(ServerPacket* pack) { return tcpc->SendPacket(pack); }
//	bool		SendPacket(TCPConnection::TCPNetPacket_Struct* tnps) { return tcpc->SendPacket(tnps); }

	int GetID() const { return(ID); }
	void Disconnect() { tcpc->Disconnect(); }

	inline bool			HasName() const		{ return(m_name.length() > 0); }
	inline uint32		GetIP() const		{ return tcpc->GetrIP(); }
	inline uint16		GetPort() const		{ return tcpc->GetrPort(); }
	inline const char * GetName() const		{ return(m_name.c_str()); }
	inline int			CountZones() const	{ return(m_states.size()); }

	bool ContainsZone(const char *short_name) const;

	//commands
	void Shutdown();
	void BootZone(const char *short_name, uint16 port);
	void StartZone(const char *short_name);
	void RestartZone(const char *short_name);
	void StopZone(const char *short_name);
	void BootDynamics(uint8 new_total);

	void GetZoneList(std::vector<std::string> &list);
	void GetZoneDetails(const char *short_name, std::map<std::string,std::string> &result);

protected:
	const int			ID;
	EmuTCPConnection*const tcpc;
	bool				authenticated;
	std::string			m_name;
	Timer				m_bootTimer;

	uint8 m_dynamicCount;

	typedef struct {
		bool up;
		uint32 starts;	//number of times this zone has started
		uint16 port;	//the port this zone wants to use (0=pick one)
	} ZoneState;
	std::map<std::string, ZoneState> m_states;
};

#endif /*LAUNCHERLINK_H_*/

