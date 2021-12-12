#ifndef GUILD_BASE_H_
#define GUILD_BASE_H_

#include "guilds.h"
#include <map>
#include <string>
#include <vector>

class Database;

class CharGuildInfo
{
	public:
		//fields from `characer_`
		uint32	char_id;
		std::string	char_name;
		uint8	class_;
		uint16	level;
		uint32	time_last_on;
		uint32	zone_id;

		//fields from `guild_members`
		uint32	guild_id;
		uint8	rank;
		bool	tribute_enable;
		uint32	total_tribute;
		uint32	last_tribute;		//timestamp
		bool	banker;
		bool	alt;
		std::string	public_note;
};

//this object holds guild functionality shared between world and zone.
class BaseGuildManager
{
	public:
		BaseGuildManager();
		virtual ~BaseGuildManager();

		//this must be called before doing anything else with this object
		void SetDatabase(Database *db)
		{
			m_db = db;
		}

		bool LoadGuilds();
		bool RefreshGuild(uint32 guild_id);

		//guild edit actions.
		uint32	CreateGuild(const char* name, uint32 leader_char_id);
		bool	DeleteGuild(uint32 guild_id);
		bool	RenameGuild(uint32 guild_id, const char* name);
		bool	SetGuildMOTD(uint32 guild_id, const char* motd, const char *setter);
		bool	SetGuildURL(uint32 GuildID, const char* URL);
		bool	SetGuildChannel(uint32 GuildID, const char* Channel);

		//character edit actions
		bool	SetGuildLeader(uint32 guild_id, uint32 leader_char_id);
		bool	SetGuild(uint32 charid, uint32 guild_id, uint8 rank);
		bool	SetGuildRank(uint32 charid, uint8 rank);
		bool	SetBankerFlag(uint32 charid, bool is_banker);
		bool	ForceRankUpdate(uint32 charid);
		bool	GetAltFlag(uint32 CharID);
		bool	SetAltFlag(uint32 charid, bool is_alt);
		bool	GetBankerFlag(uint32 CharID);
		bool	SetTributeFlag(uint32 charid, bool enabled);
		bool	SetPublicNote(uint32 charid, const char *note);

		//queries
		bool	GetCharInfo(const char *char_name, CharGuildInfo &into);
		bool	GetCharInfo(uint32 char_id, CharGuildInfo &into);
		bool	GetEntireGuild(uint32 guild_id, std::vector<CharGuildInfo *> &members);	//caller is responsible for deleting each pointer in the resulting vector.
		bool	GuildExists(uint32 guild_id) const;
		bool	GetGuildMOTD(uint32 guild_id, char *motd_buffer, char *setter_buffer) const;
		bool	GetGuildURL(uint32 GuildID, char *URLBuffer) const;
		bool	GetGuildChannel(uint32 GuildID, char *ChannelBuffer) const;
		const char *GetRankName(uint32 guild_id, uint8 rank) const;
		const char *GetGuildName(uint32 guild_id) const;
		std::string GetGuildNameByID(uint32 guild_id) const;
		std::string GetGuildRankName(uint32 guild_id, uint8 rank) const;
		bool IsCharacterInGuild(uint32 character_id, uint32 guild_id = 0);
		bool	GetGuildNameByID(uint32 guild_id, std::string &into) const;
		uint32	GetGuildIDByName(const char *GuildName);
		uint32 GetGuildIDByCharacterID(uint32 character_id);
		bool	IsGuildLeader(uint32 guild_id, uint32 char_id) const;
		uint8	GetDisplayedRank(uint32 guild_id, uint8 rank, uint32 char_id) const;
		bool	CheckGMStatus(uint32 guild_id, uint8 status) const;
		bool	CheckPermission(uint32 guild_id, uint8 rank, GuildAction act) const;
//	uint32	Getguild_id(uint32 eqid);
		uint32	FindGuildByLeader(uint32 leader) const;
//	void	GetGuildMembers(uint32 guild_id,GuildMember_Struct* gms);
		uint32	NumberInGuild(uint32 guild_id);
//	bool	GetGuildRanks(uint32 guildeqid, GuildRanks_Struct* gr);
//	bool	EditGuild(uint32 guild_id, uint8 ranknum, GuildRankLevel_Struct* grl);

		uint8 *MakeGuildList(const char *head_name, uint32 &length) const;	//make a guild list packet, returns ownership of the buffer.

		static const char *const GuildActionNames[_MaxGuildAction];
		uint32	DoesAccountContainAGuildLeader(uint32 AccountID);

	protected:
		//the methods which must be defined by base classes.
		virtual void SendGuildRefresh(uint32 guild_id, bool name, bool motd, bool rank, bool relation) = 0;
		virtual void SendCharRefresh(uint32 old_guild_id, uint32 guild_id, uint32 charid) = 0;
		virtual void SendRankUpdate(uint32 CharID) = 0;
		virtual void SendGuildDelete(uint32 guild_id) = 0;

		uint32	DBCreateGuild(const char* name, uint32 leader_char_id);
		bool	DBDeleteGuild(uint32 guild_id);
		bool	DBRenameGuild(uint32 guild_id, const char* name);
		bool	DBSetGuildLeader(uint32 guild_id, uint32 leader_char_id);
		bool	DBSetGuildMOTD(uint32 guild_id, const char* motd, const char *setter);
		bool	DBSetGuildURL(uint32 GuildID, const char* URL);
		bool	DBSetGuildChannel(uint32 GuildID, const char* Channel);
		bool	DBSetGuild(uint32 charid, uint32 guild_id, uint8 rank);
		bool	DBSetGuildRank(uint32 charid, uint8 rank);
		bool	DBSetBankerFlag(uint32 charid, bool is_banker);
		bool	DBSetAltFlag(uint32 charid, bool is_alt);
		bool	DBSetTributeFlag(uint32 charid, bool enabled);
		bool	DBSetPublicNote(uint32 charid, const char *note);
		bool	QueryWithLogging(std::string query, const char *errmsg);
//	void	DBSetPublicNote(uint32 guild_id,char* charname, char* note);

		bool	LocalDeleteGuild(uint32 guild_id);

		class RankInfo
		{
			public:
				RankInfo();
				std::string name;
				bool permissions[_MaxGuildAction];
		};
		class GuildInfo
		{
			public:
				GuildInfo();
				std::string name;
				std::string motd;
				std::string motd_setter;
				std::string url;
				std::string channel;

				uint32 leader_char_id;
				uint8 minstatus;
				//tribute is not in here on purpose, since it is only valid in world!
				RankInfo ranks[GUILD_MAX_RANK + 1];
		};

		std::map<uint32, GuildInfo *> m_guilds;	//we own the pointers in this map
		void ClearGuilds();	//clears internal structure

		Database *m_db;	//we do not own this

		bool _StoreGuildDB(uint32 guild_id);
		GuildInfo *_CreateGuild(uint32 guild_id, const char *guild_name, uint32 account_id, uint8 minstatus, const char *guild_motd, const char *motd_setter, const char *Channel, const char *URL);
		uint32 _GetFreeGuildID();
};


#endif /*GUILD_BASE_H_*/


