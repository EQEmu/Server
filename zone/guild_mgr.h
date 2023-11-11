#ifndef GUILD_MGR_H_
#define GUILD_MGR_H_

#include "../common/types.h"
#include "../common/guild_base.h"
#include <map>
#include <list>
#include "../zone/petitions.h"

extern PetitionList petition_list;
//extern GuildRanks_Struct guilds[512];
//extern ZoneDatabase database;

#define PBUFFER 50
#define MBUFFER 50

#define GUILD_BANK_MAIN_AREA_SIZE	200
#define GUILD_BANK_DEPOSIT_AREA_SIZE	20
class Client;
class ServerPacket;

struct GuildBankItem
{
	uint32	ItemID;
	uint32	Quantity;
	char	Donator[64];
	uint8	Permissions;
	char	WhoFor[64];
};

struct GuildBankItems
{
	GuildBankItem	MainArea[GUILD_BANK_MAIN_AREA_SIZE];
	GuildBankItem	DepositArea[GUILD_BANK_DEPOSIT_AREA_SIZE];
};

struct GuildBank
{
	uint32	GuildID;
	GuildBankItems	Items;
};

enum {	GuildBankBulkItems = 0, GuildBankItemUpdate = 1, GuildBankPromote = 3, GuildBankViewItem = 4, GuildBankDeposit = 5,
	GuildBankPermissions = 6, GuildBankWithdraw = 7, GuildBankSplitStacks = 8, GuildBankMergeStacks = 9, GuildBankAcknowledge = 10 };

enum {	GuildBankDepositArea = 0, GuildBankMainArea = 1 };

enum {	GuildBankBankerOnly = 0, GuildBankSingleMember = 1, GuildBankPublicIfUsable = 2, GuildBankPublic = 3 };

class ZoneGuildManager : public BaseGuildManager {
public:
	~ZoneGuildManager(void);

	//called by worldserver when it receives a message from world.
	void ProcessWorldPacket(ServerPacket *pack);

	void ListGuilds(Client *c, std::string search_criteria = std::string()) const;
	void ListGuilds(Client *c, uint32 guild_id = 0) const;
	void DescribeGuild(Client *c, uint32 guild_id) const;


//	bool	DonateTribute(uint32 charid, uint32 guild_id, uint32 tribute_amount);

	uint8 *MakeGuildMembers(uint32 guild_id, const char *prefix_name, uint32 &length);	//make a guild member list packet, returns ownership of the buffer.

	void RecordInvite(uint32 char_id, uint32 guild_id, uint8 rank);
	bool VerifyAndClearInvite(uint32 char_id, uint32 guild_id, uint8 rank);
	void SendGuildMemberUpdateToWorld(const char *MemberName, uint32 GuildID, uint16 ZoneID, uint32 LastSeen);
	void RequestOnlineGuildMembers(uint32 FromID, uint32 GuildID);
	void UpdateRankPermission(uint32 gid, uint32 charid, uint32 fid, uint32 rank, uint32 value);
	void SendPermissionUpdate(uint32 guild_id, uint32 rank, uint32 function_id, uint32 value);
	void UpdateRankName(uint32 gid, uint32 rank, std::string rank_name);
	void SendRankName(uint32 guild_id, uint32 rank, std::string& rank_name);
	void SendAllRankNames(uint32 guild_id, uint32 char_id);
	BaseGuildManager::GuildInfo* GetGuildByGuildID(uint32 guild_id);

protected:
	virtual void SendGuildRefresh(uint32 guild_id, bool name, bool motd, bool rank, bool relation);
	virtual void SendCharRefresh(uint32 old_guild_id, uint32 guild_id, uint32 charid);
	virtual void SendRankUpdate(uint32 CharID);
	virtual void SendGuildDelete(uint32 guild_id);

	std::map<uint32, std::pair<uint32, uint8> > m_inviteQueue;	//map from char ID to guild,rank

private:
	uint32 id;
};


class GuildBankManager
{

public:
	~GuildBankManager();
	void SendGuildBank(Client *c);
	bool AddItem(uint32 GuildID, uint8 Area, uint32 ItemID, int32 QtyOrCharges, const char *Donator, uint8 Permissions, const char *WhoFor);
	int Promote(uint32 GuildID, int SlotID);
	void SetPermissions(uint32 GuildID, uint16 SlotID, uint32 Permissions, const char *MemberName);
	EQ::ItemInstance* GetItem(uint32 GuildID, uint16 Area, uint16 SlotID, uint32 Quantity);
	bool DeleteItem(uint32 GuildID, uint16 Area, uint16 SlotID, uint32 Quantity);
	bool HasItem(uint32 GuildID, uint32 ItemID);
	bool IsAreaFull(uint32 GuildID, uint16 Area);
	bool MergeStacks(uint32 GuildID, uint16 SlotID);
	bool SplitStack(uint32 GuildID, uint16 SlotID, uint32 Quantity);
	bool AllowedToWithdraw(uint32 GuildID, uint16 Area, uint16 SlotID, const char *Name);

private:
	bool IsLoaded(uint32 GuildID);
	bool Load(uint32 GuildID);
	std::list<GuildBank*>::iterator GetGuildBank(uint32 GuildID);
	void UpdateItemQuantity(uint32 GuildID, uint16 Area, uint16 SlotID, uint32 Quantity);

	std::list<GuildBank*> Banks;

};

extern ZoneGuildManager guild_mgr;
extern GuildBankManager *GuildBanks;


#endif /*GUILD_MGR_H_*/

