#ifndef ZONELIST_H_
#define ZONELIST_H_

#include "../common/types.h"
#include "../common/eqtime.h"
#include "../common/timer.h"
#include "../common/linked_list.h"
#include <vector>

class WorldTCPConnection;
class ServerPacket;
class ZoneServer;

class ZSList
{
public:
	enum { MaxLockedZones = 10 };

	static void ShowUpTime(WorldTCPConnection* con, const char* adminname = 0);

	ZSList();
	~ZSList();
	ZoneServer* FindByName(const char* zonename);
	ZoneServer* FindByID(uint32 ZoneID);
	ZoneServer* FindByZoneID(uint32 ZoneID);
	ZoneServer*	FindByPort(uint16 port);
	ZoneServer* FindByInstanceID(uint32 InstanceID);

	void	SendChannelMessage(const char* from, const char* to, uint8 chan_num, uint8 language, const char* message, ...);
	void	SendChannelMessageRaw(const char* from, const char* to, uint8 chan_num, uint8 language, const char* message);
	void	SendEmoteMessage(const char* to, uint32 to_guilddbid, int16 to_minstatus, uint32 type, const char* message, ...);
	void	SendEmoteMessageRaw(const char* to, uint32 to_guilddbid, int16 to_minstatus, uint32 type, const char* message);

	void	SendZoneStatus(const char* to, int16 admin, WorldTCPConnection* connection);

	void	SendTimeSync();
	void	Add(ZoneServer* zoneserver);
	void	Process();
	void	KillAll();
	bool	SendPacket(ServerPacket* pack);
	bool	SendPacket(uint32 zoneid, ServerPacket* pack);
	bool	SendPacket(uint32 zoneid, uint16 instanceid, ServerPacket* pack);
	inline uint32	GetNextID()		{ return NextID++; }
	void	RebootZone(const char* ip1,uint16 port, const char* ip2, uint32 skipid, uint32 zoneid = 0);
	uint32	TriggerBootup(uint32 iZoneID, uint32 iInstanceID = 0);
	void	SOPZoneBootup(const char* adminname, uint32 ZoneServerID, const char* zonename, bool iMakeStatic = false);
	EQTime	worldclock;
	bool	SetLockedZone(uint16 iZoneID, bool iLock);
	bool	IsZoneLocked(uint16 iZoneID);
	void	ListLockedZones(const char* to, WorldTCPConnection* connection);
	Timer*	shutdowntimer;
	Timer*	reminder;
	void	NextGroupIDs(uint32 &start, uint32 &end);
	void	SendLSZones();
	uint16 GetAvailableZonePort();

	int GetZoneCount();
	void GetZoneIDList(std::vector<uint32> &zones);
	void WorldShutDown(uint32 time, uint32 interval);

protected:
	uint32 NextID;
	LinkedList<ZoneServer*> list;
	uint16	pLockedZones[MaxLockedZones];
	uint32 CurGroupID;
	uint16 LastAllocatedPort;


};

#endif /*ZONELIST_H_*/

