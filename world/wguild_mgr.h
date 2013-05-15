#ifndef GUILD_MGR_H_
#define GUILD_MGR_H_

#include "../common/types.h"
#include "../common/guild_base.h"

class Client;
class ServerPacket;

class WorldGuildManager : public BaseGuildManager {
public:

	//called by zoneserver when it receives a guild message from zone.
	void ProcessZonePacket(ServerPacket *pack);

	uint8 *MakeGuildMembers(uint32 guild_id, const char *prefix_name, uint32 &length);	//make a guild member list packet, returns ownership of the buffer.

protected:
	virtual void SendGuildRefresh(uint32 guild_id, bool name, bool motd, bool rank, bool relation);
	virtual void SendCharRefresh(uint32 old_guild_id, uint32 guild_id, uint32 charid);
	virtual void SendRankUpdate(uint32 CharID) { return; }
	virtual void SendGuildDelete(uint32 guild_id);

	//map<uint32, uint32> m_tribute;	//map from guild ID to current tribute ammount
};

extern WorldGuildManager guild_mgr;


#endif /*GUILD_MGR_H_*/

