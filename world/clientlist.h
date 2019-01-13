#ifndef CLIENTLIST_H_
#define CLIENTLIST_H_

#include "../common/eq_packet_structs.h"
#include "../common/linked_list.h"
#include "../common/timer.h"
#include "../common/rulesys.h"
#include "../common/servertalk.h"
#include "../common/event/timer.h"
#include "../common/net/console_server_connection.h"
#include <vector>
#include <string>

class Client;
class ZoneServer;
class WorldTCPConnection;
class ClientListEntry;
class ServerPacket;
struct ServerClientList_Struct;

class ClientList {
public:
	ClientList();
	~ClientList();

	void Process();

	//from old ClientList
	void	Add(Client* client);
	Client*	Get(uint32 ip, uint16 port);
	Client* FindByAccountID(uint32 account_id);
	Client* FindByName(char* charname);

	void	ZoneBootup(ZoneServer* zs);
	void	RemoveCLEReferances(ClientListEntry* cle);


	//from ZSList

	void	SendWhoAll(uint32 fromid,const char* to, int16 admin, Who_All_Struct* whom, WorldTCPConnection* connection);
	void	SendFriendsWho(ServerFriendsWho_Struct *FriendsWho, WorldTCPConnection* connection);
	void	SendOnlineGuildMembers(uint32 FromID, uint32 GuildID);
	void	SendClientVersionSummary(const char *Name);
	void	SendLFGMatches(ServerLFGMatchesRequest_Struct *LFGMatchesRequest);
	void	ConsoleSendWhoAll(const char* to, int16 admin, Who_All_Struct* whom, WorldTCPConnection* connection);
	void	SendCLEList(const int16& admin, const char* to, WorldTCPConnection* connection, const char* iName = 0);

	bool	SendPacket(const char* to, ServerPacket* pack);
	void	SendGuildPacket(uint32 guild_id, ServerPacket* pack);

	void	ClientUpdate(ZoneServer* zoneserver, ServerClientList_Struct* scl);
	void	CLERemoveZSRef(ZoneServer* iZS);
	void	DisconnectCLE(ClientListEntry* entry);
	ClientListEntry* CheckAuth(uint32 iLSID, const char* iKey);
	ClientListEntry* CheckAuth(const char* iName, const char* iPassword);
	ClientListEntry* CheckAuth(uint32 id, const char* iKey, uint32 ip);
	ClientListEntry* FindCharacter(const char* name);
	ClientListEntry* FindCLEByAccountID(uint32 iAccID);
	ClientListEntry* FindCLEByCharacterID(uint32 iCharID);
	ClientListEntry* GetCLE(uint32 iID);
	void	GetCLEIP(uint32 iIP);
	bool	CheckCLEIP(uint32 iIP);
	uint32	GetCLEIPCount(uint32 iLSAccountID);
	void	DisconnectByIP(uint32 iIP);
	void	EnforceSessionLimit(uint32 iLSAccountID);
	bool	CheckSessionLimit(uint32 iLSAccountID);
	void	CLCheckStale();
	void	CLEKeepAlive(uint32 numupdates, uint32* wid);
	void	CLEAdd(uint32 iLSID, const char* iLoginName, const char* iLoginKey, int16 iWorldAdmin = 0, uint32 ip = 0, uint8 local=0);
	void	UpdateClientGuild(uint32 char_id, uint32 guild_id);

	int GetClientCount();
	void GetClients(const char *zone_name, std::vector<ClientListEntry *> &into);

private:
	void OnTick(EQ::Timer *t);
	inline uint32 GetNextCLEID() { return NextCLEID++; }

	//this is the list of people actively connected to zone
	LinkedList<Client*> list;

	//this is the list of people in any zone, not nescesarily connected to world
	Timer	CLStale_timer;
	uint32 NextCLEID;
	LinkedList<ClientListEntry *> clientlist;

	std::unique_ptr<EQ::Timer> m_tick;
};

#endif /*CLIENTLIST_H_*/

