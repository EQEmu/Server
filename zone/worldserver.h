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
#ifndef WORLDSERVER_H
#define WORLDSERVER_H

#include "../common/eq_packet_structs.h"
#include "../common/net/servertalk_client_connection.h"
#include "zone_event_scheduler.h"

class ServerPacket;
class EQApplicationPacket;
class Client;

class WorldServer {
public:
	WorldServer();
	~WorldServer();

	void Connect();
	bool SendPacket(ServerPacket* pack);
	std::string GetIP() const;
	uint16 GetPort() const;
	bool Connected() const;

	void HandleMessage(uint16 opcode, const EQ::Net::Packet &p);

	bool SendChannelMessage(Client* from, const char* to, uint8 chan_num, uint32 guilddbid, uint8 language, uint8 lang_skill, const char* message, ...);
	bool SendEmoteMessage(const char* to, uint32 to_guilddbid, uint32 type, const char* message, ...);
	bool SendEmoteMessage(const char* to, uint32 to_guilddbid, int16 to_minstatus, uint32 type, const char* message, ...);
	bool SendVoiceMacro(Client* From, uint32 Type, char* Target, uint32 MacroNumber, uint32 GroupOrRaidID = 0);
	void SetZoneData(uint32 iZoneID, uint32 iInstanceID = 0);
	bool RezzPlayer(EQApplicationPacket* rpack, uint32 rezzexp, uint32 dbid, uint16 opcode);
	bool IsOOCMuted() const { return(oocmuted); }

	uint32 NextGroupID();

	void SetLaunchedName(const char *n) { m_launchedName = n; }
	void SetLauncherName(const char *n) { m_launcherName = n; }
	void SendReloadTasks(uint8 reload_type, uint32 task_id = 0);
	void HandleReloadTasks(ServerPacket *pack);
	void UpdateLFP(uint32 LeaderID, uint8 Action, uint8 MatchFilter, uint32 FromLevel, uint32 ToLevel, uint32 Classes, const char *Comments,
				GroupLFPMemberEntry *LFPMembers);
	void UpdateLFP(uint32 LeaderID, GroupLFPMemberEntry *LFPMembers);
	void StopLFP(uint32 LeaderID);
	void HandleLFGMatches(ServerPacket *pack);
	void HandleLFPMatches(ServerPacket *pack);

	void RequestTellQueue(const char *who);

private:
	virtual void OnConnected();

	std::string m_launchedName;
	std::string m_launcherName;

	bool oocmuted;

	uint32 cur_groupid;
	uint32 last_groupid;

	void OnKeepAlive(EQ::Timer *t);

	std::unique_ptr<EQ::Net::ServertalkClient> m_connection;
	std::unique_ptr<EQ::Timer> m_keepalive;

	ZoneEventScheduler *m_zone_scheduler;
public:
	ZoneEventScheduler *GetScheduler() const;
	void SetScheduler(ZoneEventScheduler *scheduler);
};
#endif

