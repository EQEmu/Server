/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2003 EQEMu Development Team (http://eqemulator.net)

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
#ifndef EQEMU_DATABASE_H
#define EQEMU_DATABASE_H

#define AUTHENTICATION_TIMEOUT	60
#define INVALID_ID				0xFFFFFFFF

#include "debug.h"
#include "types.h"
#include "dbcore.h"
#include "linked_list.h"
#include "eq_packet_structs.h"
#include <string>
#include <vector>
#include <map>

//atoi is not uint32 or uint32 safe!!!!
#define atoul(str) strtoul(str, nullptr, 10)

//class Spawn;
class Corpse;
class Spawn2;
class NPC;
class SpawnGroupList;
class Petition;
class Client;
class Merc;
class MySQLRequestResult;
struct Combine_Struct;
//struct Faction;
//struct FactionMods;
//struct FactionValue;
struct ZonePoint;
struct NPCType;
class Inventory;
class ItemInst;

struct EventLogDetails_Struct {
	uint32	id;
	char	accountname[64];
	uint32	account_id;
	int16	status;
	char	charactername[64];
	char	targetname[64];
	char	timestamp[64];
	char	descriptiontype[64];
	char	details[128];
};

struct CharacterEventLog_Struct {
uint32	count;
uint8	eventid;
EventLogDetails_Struct eld[255];
};

struct npcDecayTimes_Struct {
	uint16 minlvl;
	uint16 maxlvl;
	uint32 seconds;
};


struct VarCache_Struct {
	char varname[26];	
	char value[0];
};

struct PlayerProfile_Struct;
struct GuildRankLevel_Struct;
struct GuildRanks_Struct;
struct ExtendedProfile_Struct;
struct GuildMember_Struct;
class PTimerList;

#pragma pack(1)

/* Conversion Structs */

namespace Convert {
	struct BindStruct {
		/*000*/ uint32 zoneId;
		/*004*/ float x;
		/*008*/ float y;
		/*012*/ float z;
		/*016*/ float heading;
	};
	struct Color_Struct
	{
		union
		{
			struct
			{
				uint8	blue;
				uint8	green;
				uint8	red;
				uint8	use_tint;	// if there's a tint this is FF
			} rgb;
			uint32 color;
		};
	};
	struct AA_Array
	{
		uint32 AA;
		uint32 value;
	};
	struct SpellBuff_Struct
	{
		/*000*/	uint8	slotid;		//badly named... seems to be 2 for a real buff, 0 otherwise
		/*001*/ uint8	level;
		/*002*/	uint8	bard_modifier;
		/*003*/	uint8	effect;			//not real
		/*004*/	uint32	spellid;
		/*008*/ uint32	duration;
		/*012*/	uint32	counters;
		/*016*/	uint32	player_id;	//'global' ID of the caster, for wearoff messages
		/*020*/
	};
	struct Tribute_Struct {
		uint32 tribute;
		uint32 tier;
	};
	struct Disciplines_Struct {
		uint32 values[MAX_PP_DISCIPLINES];
	};
	struct GroupLeadershipAA_Struct {
		union {
			struct {
				uint32 groupAAMarkNPC;
				uint32 groupAANPCHealth;
				uint32 groupAADelegateMainAssist;
				uint32 groupAADelegateMarkNPC;
				uint32 groupAA4;
				uint32 groupAA5;
				uint32 groupAAInspectBuffs;
				uint32 groupAA7;
				uint32 groupAASpellAwareness;
				uint32 groupAAOffenseEnhancement;
				uint32 groupAAManaEnhancement;
				uint32 groupAAHealthEnhancement;
				uint32 groupAAHealthRegeneration;
				uint32 groupAAFindPathToPC;
				uint32 groupAAHealthOfTargetsTarget;
				uint32 groupAA15;
			};
			uint32 ranks[MAX_GROUP_LEADERSHIP_AA_ARRAY];
		};
	};

	struct RaidLeadershipAA_Struct {
		union {
			struct {
				uint32 raidAAMarkNPC;
				uint32 raidAANPCHealth;
				uint32 raidAADelegateMainAssist;
				uint32 raidAADelegateMarkNPC;
				uint32 raidAA4;
				uint32 raidAA5;
				uint32 raidAA6;
				uint32 raidAASpellAwareness;
				uint32 raidAAOffenseEnhancement;
				uint32 raidAAManaEnhancement;
				uint32 raidAAHealthEnhancement;
				uint32 raidAAHealthRegeneration;
				uint32 raidAAFindPathToPC;
				uint32 raidAAHealthOfTargetsTarget;
				uint32 raidAA14;
				uint32 raidAA15;
			};
			uint32 ranks[MAX_RAID_LEADERSHIP_AA_ARRAY];
		};
	};

	struct LeadershipAA_Struct {
		union {
			struct {
				Convert::GroupLeadershipAA_Struct group;
				Convert::RaidLeadershipAA_Struct raid;
			};
			uint32 ranks[MAX_LEADERSHIP_AA_ARRAY];
		};
	};
	typedef struct
	{
		/*00*/ char Name[64];
		/*64*/ uint32 Level;
		/*68*/ uint32 Race;
		/*72*/ uint32 Class;
		/*76*/ uint32 Zone;
		/*80*/ uint32 Time;
		/*84*/ uint32 Points;
		/*88*/
	} PVPStatsEntry_Struct;
	struct BandolierItem_Struct {
		uint32 item_id;
		uint32 icon;
		char item_name[64];
	};
	struct Bandolier_Struct {
		char name[32];
		Convert::BandolierItem_Struct items[EmuConstants::BANDOLIER_SIZE];
	};
	struct PotionBelt_Struct {
		Convert::BandolierItem_Struct items[EmuConstants::POTION_BELT_SIZE];
	};
	struct SuspendedMinion_Struct
	{
		/*000*/	uint16 SpellID;
		/*002*/	uint32 HP;
		/*006*/	uint32 Mana;
		/*010*/	Convert::SpellBuff_Struct Buffs[BUFF_COUNT];
		/*510*/	uint32 Items[_MaterialCount];
		/*546*/	char Name[64];
		/*610*/
	};

	struct PlayerProfile_Struct {
		/*0000*/	uint32							checksum;			// Checksum from CRC32::SetEQChecksum
		/*0004*/	char							name[64];			// Name of player sizes not right
		/*0068*/	char							last_name[32];		// Last name of player sizes not right
		/*0100*/	uint32							gender;				// Player Gender - 0 Male, 1 Female
		/*0104*/	uint32							race;				// Player race
		/*0108*/	uint32							class_;				// Player class
		/*0112*/	uint32							unknown0112;		//
		/*0116*/	uint32							level;				// Level of player (might be one byte)
		/*0120*/	Convert::BindStruct				binds[5];			// Bind points (primary is first, home city is fifth)
		/*0220*/	uint32							deity;				// deity
		/*0224*/	uint32							guild_id;
		/*0228*/	uint32							birthday;			// characters bday
		/*0232*/	uint32							lastlogin;			// last login or zone time
		/*0236*/	uint32							timePlayedMin;		// in minutes
		/*0240*/	uint8							pvp;
		/*0241*/	uint8							level2;				//no idea why this is here, but thats how it is on live
		/*0242*/	uint8							anon;				// 2=roleplay, 1=anon, 0=not anon
		/*0243*/	uint8							gm;
		/*0244*/	uint8							guildrank;
		/*0245*/	uint8							guildbanker;
		/*0246*/	uint8							unknown0246[6];		//
		/*0252*/	uint32							intoxication;
		/*0256*/	uint32							spellSlotRefresh[MAX_PP_REF_MEMSPELL];	//in ms
		/*0292*/	uint32							abilitySlotRefresh;
		/*0296*/	uint8							haircolor;			// Player hair color
		/*0297*/	uint8							beardcolor;			// Player beard color
		/*0298*/	uint8							eyecolor1;			// Player left eye color
		/*0299*/	uint8							eyecolor2;			// Player right eye color
		/*0300*/	uint8							hairstyle;			// Player hair style
		/*0301*/	uint8							beard;				// Beard type
		/*0302*/	uint8							ability_time_seconds;	//The following four spots are unknown right now.....
		/*0303*/	uint8							ability_number;		//ability used
		/*0304*/	uint8							ability_time_minutes;
		/*0305*/	uint8							ability_time_hours;	//place holder
		/*0306*/	uint8							unknown0306[6];		// @bp Spacer/Flag?
		/*0312*/	uint32							item_material[_MaterialCount];	// Item texture/material of worn/held items
		/*0348*/	uint8							unknown0348[44];
		/*0392*/	Convert::Color_Struct			item_tint[_MaterialCount];
		/*0428*/	Convert::AA_Array				aa_array[MAX_PP_AA_ARRAY];
		/*2348*/	float							unknown2384;		//seen ~128, ~47
		/*2352*/	char							servername[32];		// length probably not right
		/*2384*/	char							title[32];			// length might be wrong
		/*2416*/	char							suffix[32];			// length might be wrong
		/*2448*/	uint32							guildid2;			//
		/*2452*/	uint32							exp;				// Current Experience
		/*2456*/	uint32							unknown2492;
		/*2460*/	uint32							points;				// Unspent Practice points
		/*2464*/	uint32							mana;				// current mana
		/*2468*/	uint32							cur_hp;				// current hp
		/*2472*/	uint32							unknown2508;		// 0x05
		/*2476*/	uint32							STR;				// Strength
		/*2480*/	uint32							STA;				// Stamina
		/*2484*/	uint32							CHA;				// Charisma
		/*2488*/	uint32							DEX;				// Dexterity
		/*2492*/	uint32							INT;				// Intelligence
		/*2496*/	uint32							AGI;				// Agility
		/*2500*/	uint32							WIS;				// Wisdom
		/*2504*/	uint8							face;				// Player face
		/*2505*/	uint8							unknown2541[47];	// ?
		/*2552*/	uint8							languages[MAX_PP_LANGUAGE];
		/*2580*/	uint8							unknown2616[4];
		/*2584*/	uint32							spell_book[MAX_PP_REF_SPELLBOOK];
		/*4504*/	uint8							unknown4540[128];	// Was [428] all 0xff
		/*4632*/	uint32							mem_spells[MAX_PP_REF_MEMSPELL];
		/*4668*/	uint8							unknown4704[32];	//
		/*4700*/	float							y;					// Player y position
		/*4704*/	float							x;					// Player x position
		/*4708*/	float							z;					// Player z position
		/*4712*/	float							heading;			// Direction player is facing
		/*4716*/	uint8							unknown4752[4];		//
		/*4720*/	int32							platinum;			// Platinum Pieces on player
		/*4724*/	int32							gold;				// Gold Pieces on player
		/*4728*/	int32							silver;				// Silver Pieces on player
		/*4732*/	int32							copper;				// Copper Pieces on player
		/*4736*/	int32							platinum_bank;		// Platinum Pieces in Bank
		/*4740*/	int32							gold_bank;			// Gold Pieces in Bank
		/*4744*/	int32							silver_bank;		// Silver Pieces in Bank
		/*4748*/	int32							copper_bank;		// Copper Pieces in Bank
		/*4752*/	int32							platinum_cursor;	// Platinum on cursor
		/*4756*/	int32							gold_cursor;		// Gold on cursor
		/*4760*/	int32							silver_cursor;		// Silver on cursor
		/*4764*/	int32							copper_cursor;		// Copper on cursor
		/*4768*/	int32							platinum_shared;	// Platinum shared between characters
		/*4772*/	uint8							unknown4808[24];
		/*4796*/	uint32							skills[MAX_PP_SKILL];	// [400] List of skills	// 100 dword buffer
		/*5196*/	uint8							unknown5132[184];
		/*5380*/	uint32							pvp2;				//
		/*5384*/	uint32							unknown5420;		//
		/*5388*/	uint32							pvptype;			//
		/*5392*/	uint32							unknown5428;		//
		/*5396*/	uint32							ability_down;		// Guessing
		/*5400*/	uint8							unknown5436[8];		//
		/*5408*/	uint32							autosplit;			//not used right now
		/*5412*/	uint8							unknown5448[8];
		/*5420*/	uint32							zone_change_count;	// Number of times user has zoned in their career (guessing)
		/*5424*/	uint8							unknown5460[16];	//
		/*5440*/	uint32							drakkin_heritage;	//
		/*5444*/	uint32							drakkin_tattoo;		//
		/*5448*/	uint32							drakkin_details;	//
		/*5452*/	uint32							expansions;			// expansion setting, bit field of expansions avaliable
		/*5456*/	int32							toxicity;			//from drinking potions, seems to increase by 3 each time you drink
		/*5460*/	char							unknown5496[16];	//
		/*5476*/	int32							hunger_level;
		/*5480*/	int32							thirst_level;
		/*5484*/	uint32							ability_up;
		/*5488*/	char							unknown5524[16];
		/*5504*/	uint16							zone_id;			// Current zone of the player
		/*5506*/	uint16							zoneInstance;		// Instance ID
		/*5508*/	Convert::SpellBuff_Struct		buffs[BUFF_COUNT];	// Buffs currently on the player
		/*6008*/	char							groupMembers[6][64];//
		/*6392*/	char							unknown6428[656];
		/*7048*/	uint32							entityid;
		/*7052*/	uint32							leadAAActive;
		/*7056*/	uint32							unknown7092;
		/*7060*/	int32							ldon_points_guk;	//client uses these as signed
		/*7064*/	int32							ldon_points_mir;
		/*7068*/	int32							ldon_points_mmc;
		/*7072*/	int32							ldon_points_ruj;
		/*7076*/	int32							ldon_points_tak;
		/*7080*/	int32							ldon_points_available;
		/*7084*/	int32							ldon_wins_guk;
		/*7088*/	int32							ldon_wins_mir;
		/*7092*/	int32							ldon_wins_mmc;
		/*7096*/	int32							ldon_wins_ruj;
		/*7100*/	int32							ldon_wins_tak;
		/*7104*/	int32							ldon_losses_guk;
		/*7108*/	int32							ldon_losses_mir;
		/*7112*/	int32							ldon_losses_mmc;
		/*7116*/	int32							ldon_losses_ruj;
		/*7120*/	int32							ldon_losses_tak;
		/*7124*/	uint8							unknown7160[72];
		/*7196*/	uint32							tribute_time_remaining;	//in miliseconds
		/*7200*/	uint32							showhelm;
		/*7204*/	uint32							career_tribute_points;
		/*7208*/	uint32							unknown7244;
		/*7212*/	uint32							tribute_points;
		/*7216*/	uint32							unknown7252;
		/*7220*/	uint32							tribute_active;		//1=active
		/*7224*/	Convert::Tribute_Struct			tributes[EmuConstants::TRIBUTE_SIZE];
		/*7264*/	Convert::Disciplines_Struct		disciplines;
		/*7664*/	uint32							recastTimers[MAX_RECAST_TYPES];	// Timers (GMT of last use)
		/*7744*/	char							unknown7780[160];
		/*7904*/	uint32							endurance;
		/*7908*/	uint32							group_leadership_exp;	//0-1000
		/*7912*/	uint32							raid_leadership_exp;	//0-2000
		/*7916*/	uint32							group_leadership_points;
		/*7920*/	uint32							raid_leadership_points;
		/*7924*/	Convert::LeadershipAA_Struct	leader_abilities;
		/*8052*/	uint8							unknown8088[132];
		/*8184*/	uint32							air_remaining;
		/*8188*/	uint32							PVPKills;
		/*8192*/	uint32							PVPDeaths;
		/*8196*/	uint32							PVPCurrentPoints;
		/*8200*/	uint32							PVPCareerPoints;
		/*8204*/	uint32							PVPBestKillStreak;
		/*8208*/	uint32							PVPWorstDeathStreak;
		/*8212*/	uint32							PVPCurrentKillStreak;
		/*8216*/	Convert::PVPStatsEntry_Struct	PVPLastKill;
		/*8304*/	Convert::PVPStatsEntry_Struct	PVPLastDeath;
		/*8392*/	uint32							PVPNumberOfKillsInLast24Hours;
		/*8396*/	Convert::PVPStatsEntry_Struct	PVPRecentKills[50];
		/*12796*/	uint32							aapoints_spent;
		/*12800*/	uint32							expAA;
		/*12804*/	uint32							aapoints;			//avaliable, unspent
		/*12808*/	uint8							unknown12844[36];
		/*12844*/	Convert::Bandolier_Struct		bandoliers[EmuConstants::BANDOLIERS_COUNT];
		/*14124*/	uint8							unknown14160[4506];
		/*18630*/	Convert::SuspendedMinion_Struct	SuspendedMinion; // No longer in use
		/*19240*/	uint32							timeentitledonaccount;
		/*19244*/	Convert::PotionBelt_Struct		potionbelt;			//there should be 3 more of these
		/*19532*/	uint8							unknown19568[8];
		/*19540*/	uint32							currentRadCrystals; // Current count of radiant crystals
		/*19544*/	uint32							careerRadCrystals;	// Total count of radiant crystals ever
		/*19548*/	uint32							currentEbonCrystals;// Current count of ebon crystals
		/*19552*/	uint32							careerEbonCrystals;	// Total count of ebon crystals ever
		/*19556*/	uint8							groupAutoconsent;	// 0=off, 1=on
		/*19557*/	uint8							raidAutoconsent;	// 0=off, 1=on
		/*19558*/	uint8							guildAutoconsent;	// 0=off, 1=on
		/*19559*/	uint8							unknown19595[5];	// ***Placeholder (6/29/2005)
		/*19564*/	uint32							RestTimer;
		/*19568*/
	};
	

	namespace player_lootitem_temp
	{
		struct ServerLootItem_Struct_temp {
			uint32	item_id;
			int16	equipSlot;
			uint8	charges;
			uint16	lootslot;
			uint32 aug1;
			uint32 aug2;
			uint32 aug3;
			uint32 aug4;
			uint32 aug5;
		};
	}

	struct DBPlayerCorpse_Struct_temp {
		uint32	crc;
		bool	locked;
		uint32	itemcount;
		uint32	exp;
		float	size;
		uint8	level;
		uint8	race;
		uint8	gender;
		uint8	class_;
		uint8	deity;
		uint8	texture;
		uint8	helmtexture;
		uint32	copper;
		uint32	silver;
		uint32	gold;
		uint32	plat;
		Color_Struct item_tint[9];
		uint8 haircolor;
		uint8 beardcolor;
		uint8 eyecolor1;
		uint8 eyecolor2;
		uint8 hairstyle;
		uint8 face;
		uint8 beard;
		uint32 drakkin_heritage;
		uint32 drakkin_tattoo;
		uint32 drakkin_details;
		player_lootitem_temp::ServerLootItem_Struct_temp	items[0];
	};

	namespace classic_db_temp {
		struct DBPlayerCorpse_Struct_temp {
			uint32	crc;
			bool	locked;
			uint32	itemcount;
			uint32	exp;
			float	size;
			uint8	level;
			uint8	race;
			uint8	gender;
			uint8	class_;
			uint8	deity;
			uint8	texture;
			uint8	helmtexture;
			uint32	copper;
			uint32	silver;
			uint32	gold;
			uint32	plat;
			Color_Struct item_tint[9];
			uint8 haircolor;
			uint8 beardcolor;
			uint8 eyecolor1;
			uint8 eyecolor2;
			uint8 hairstyle;
			uint8 face;
			uint8 beard;
			player_lootitem_temp::ServerLootItem_Struct_temp	items[0];
		};
	}
}

#pragma pack()

class Database : public DBcore {
public:
	Database();
	Database(const char* host, const char* user, const char* passwd, const char* database,uint32 port);
	bool Connect(const char* host, const char* user, const char* passwd, const char* database,uint32 port);
	~Database();


	/*
	* General Character Related Stuff
	*/

	/* Character Creation */
	bool	SaveCharacterCreate(uint32 character_id, uint32 account_id, PlayerProfile_Struct* pp);

	bool	MoveCharacterToZone(const char* charname, const char* zonename);
	bool	MoveCharacterToZone(const char* charname, const char* zonename,uint32 zoneid);
	bool	MoveCharacterToZone(uint32 iCharID, const char* iZonename);
	bool	UpdateName(const char* oldname, const char* newname);
	bool	SetHackerFlag(const char* accountname, const char* charactername, const char* hacked);
	bool	SetMQDetectionFlag(const char* accountname, const char* charactername, const char* hacked, const char* zone);
	bool	AddToNameFilter(const char* name);
	bool	ReserveName(uint32 account_id, char* name);
	bool	CreateCharacter(uint32 account_id, char* name, uint16 gender, uint16 race, uint16 class_, uint8 str, uint8 sta, uint8 cha, uint8 dex, uint8 int_, uint8 agi, uint8 wis, uint8 face);
	bool	StoreCharacter(uint32 account_id, PlayerProfile_Struct* pp, Inventory* inv);
	bool	DeleteCharacter(char* name);

	/*
	* General Information Getting Queries
	*/
	bool	CheckNameFilter(const char* name, bool surname = false);
	bool	CheckUsedName(const char* name);
	uint32	GetAccountIDByChar(const char* charname, uint32* oCharID = 0);
	uint32	GetAccountIDByChar(uint32 char_id);
	uint32	GetAccountIDByName(const char* accname, int16* status = 0, uint32* lsid = 0);
	uint32	GetGuildIDByCharID(uint32 char_id);
	void	GetAccountName(uint32 accountid, char* name, uint32* oLSAccountID = 0);
	void	GetCharName(uint32 char_id, char* name);
	uint32	GetCharacterInfo(const char* iName, uint32* oAccID = 0, uint32* oZoneID = 0, uint32* oInstanceID = 0,float* oX = 0, float* oY = 0, float* oZ = 0);
	uint32	GetCharacterID(const char *name);
	bool	CheckBannedIPs(const char* loginIP); //Lieka Edit: Check incomming connection against banned IP table.
	bool	AddBannedIP(char* bannedIP, const char* notes); //Lieka Edit: Add IP address to the Banned_IPs table.
	bool	CheckGMIPs(const char* loginIP, uint32 account_id);
	bool	AddGMIP(char* ip_address, char* name);
	void	LoginIP(uint32 AccountID, const char* LoginIP);

	/*
	* Instancing Stuff
	*/
	bool VerifyZoneInstance(uint32 zone_id, uint16 instance_id);
	bool VerifyInstanceAlive(uint16 instance_id, uint32 char_id);
	bool CharacterInInstanceGroup(uint16 instance_id, uint32 char_id);
	void DeleteInstance(uint16 instance_id);
	bool CheckInstanceExpired(uint16 instance_id);
	uint32 ZoneIDFromInstanceID(uint16 instance_id);
	uint32 VersionFromInstanceID(uint16 instance_id);
	uint32 GetTimeRemainingInstance(uint16 instance_id, bool &is_perma);
	bool GetUnusedInstanceID(uint16 &instance_id);
	bool CreateInstance(uint16 instance_id, uint32 zone_id, uint32 version, uint32 duration);
	void PurgeExpiredInstances();
	bool AddClientToInstance(uint16 instance_id, uint32 char_id);
	bool RemoveClientFromInstance(uint16 instance_id, uint32 char_id);
	bool RemoveClientsFromInstance(uint16 instance_id);
	bool CheckInstanceExists(uint16 instance_id);
	void BuryCorpsesInInstance(uint16 instance_id);
	uint16 GetInstanceVersion(uint16 instance_id);
	uint16 GetInstanceID(const char* zone, uint32 charid, int16 version);
	uint16 GetInstanceID(uint32 zone, uint32 charid, int16 version);
	void GetCharactersInInstance(uint16 instance_id, std::list<uint32> &charid_list);
	void AssignGroupToInstance(uint32 gid, uint32 instance_id);
	void AssignRaidToInstance(uint32 rid, uint32 instance_id);
	void FlagInstanceByGroupLeader(uint32 zone, int16 version, uint32 charid, uint32 gid);
	void FlagInstanceByRaidLeader(uint32 zone, int16 version, uint32 charid, uint32 rid);
	void SetInstanceDuration(uint16 instance_id, uint32 new_duration);
	bool GlobalInstance(uint16 instance_id);

	/*
	* Adventure related.
	*/
	void UpdateAdventureStatsEntry(uint32 char_id, uint8 theme, bool win);
	bool GetAdventureStats(uint32 char_id, AdventureStats_Struct *as);

	/*
	* Account Related
	*/
	uint32	GetMiniLoginAccount(char* ip);
	void	GetAccountFromID(uint32 id, char* oAccountName, int16* oStatus);
	uint32	CheckLogin(const char* name, const char* password, int16* oStatus = 0);
	int16	CheckStatus(uint32 account_id);
	uint32	CreateAccount(const char* name, const char* password, int16 status, uint32 lsaccount_id = 0);
	bool	DeleteAccount(const char* name);
	bool	SetAccountStatus(const char* name, int16 status);
	bool	SetLocalPassword(uint32 accid, const char* password);
	uint32	GetAccountIDFromLSID(uint32 iLSID, char* oAccountName = 0, int16* oStatus = 0);
	bool	UpdateLiveChar(char* charname,uint32 lsaccount_id);
	bool	GetLiveChar(uint32 account_id, char* cname);
	uint8	GetAgreementFlag(uint32 acctid);
	void	SetAgreementFlag(uint32 acctid);

	/*
	* Groups
	*/
	uint32	GetGroupID(const char* name);
	void	SetGroupID(const char* name, uint32 id, uint32 charid, uint32 ismerc = false);
	void	ClearGroup(uint32 gid = 0);
	char*	GetGroupLeaderForLogin(const char* name,char* leaderbuf);

	void	SetGroupLeaderName(uint32 gid, const char* name);
	char*	GetGroupLeadershipInfo(uint32 gid, char* leaderbuf, char* maintank = nullptr, char* assist = nullptr, char* puller = nullptr, char *marknpc = nullptr,
						char *mentoree = nullptr, int *mentor_percent = nullptr, GroupLeadershipAA_Struct* GLAA = nullptr);
	void	ClearGroupLeader(uint32 gid = 0);
	

	/*
	* Raids
	*/
	void	ClearRaid(uint32 rid = 0);
	void	ClearRaidDetails(uint32 rid = 0);
	uint32	GetRaidID(const char* name);
	const char *GetRaidLeaderName(uint32 rid);
	void	GetGroupLeadershipInfo(uint32 gid, uint32 rid, char* maintank = nullptr, char* assist = nullptr, char* puller = nullptr, char *marknpc = nullptr,
			char *mentoree = nullptr, int *mentor_percent = nullptr, GroupLeadershipAA_Struct* GLAA = nullptr);
	void	GetRaidLeadershipInfo(uint32 rid, char* maintank = nullptr, char* assist = nullptr, char* puller = nullptr, char *marknpc = nullptr,
			RaidLeadershipAA_Struct* RLAA = nullptr);
	void	SetRaidGroupLeaderInfo(uint32 gid, uint32 rid);
	void	ClearRaidLeader(uint32 gid = 0xFFFFFFFF, uint32 rid = 0);

	/* Database Conversions*/
	bool	CheckDatabaseConversions();
	bool	CheckDatabaseConvertPPDeblob();
	bool	CheckDatabaseConvertCorpseDeblob();
	bool	CheckDatabaseConvertBotsPostPPDeblob();

	/*
	* Database Variables
	*/
	bool	GetVariable(const char* varname, char* varvalue, uint16 varvalue_len);
	bool	SetVariable(const char* varname, const char* varvalue);
	bool	LoadVariables();
	uint32	LoadVariables_MQ(char** query);
	bool	LoadVariables_result(MySQLRequestResult results);

	/*
	* General Queries
	*/
	bool	LoadZoneNames();
	bool	GetZoneLongName(const char* short_name, char** long_name, char* file_name = 0, float* safe_x = 0, float* safe_y = 0, float* safe_z = 0, uint32* graveyard_id = 0, uint32* maxclients = 0);
	bool	GetZoneGraveyard(const uint32 graveyard_id, uint32* graveyard_zoneid = 0, float* graveyard_x = 0, float* graveyard_y = 0, float* graveyard_z = 0, float* graveyard_heading = 0);
	uint32	GetZoneGraveyardID(uint32 zone_id, uint32 version);
	uint32	GetZoneID(const char* zonename);
	uint8	GetPEQZone(uint32 zoneID, uint32 version);
	const char*	GetZoneName(uint32 zoneID, bool ErrorUnknown = false);
	uint8	GetServerType();
	bool	GetSafePoints(const char* short_name, uint32 version, float* safe_x = 0, float* safe_y = 0, float* safe_z = 0, int16* minstatus = 0, uint8* minlevel = 0, char *flag_needed = nullptr);
	bool	GetSafePoints(uint32 zoneID, uint32 version, float* safe_x = 0, float* safe_y = 0, float* safe_z = 0, int16* minstatus = 0, uint8* minlevel = 0, char *flag_needed = nullptr) { return GetSafePoints(GetZoneName(zoneID), version, safe_x, safe_y, safe_z, minstatus, minlevel, flag_needed); }
	uint8	GetSkillCap(uint8 skillid, uint8 in_race, uint8 in_class, uint16 in_level);
	uint8	GetRaceSkill(uint8 skillid, uint8 in_race);
	bool	LoadPTimers(uint32 charid, PTimerList &into);
	void	ClearPTimers(uint32 charid);
	void	ClearMerchantTemp();
	void	SetLFP(uint32 CharID, bool LFP);
	void	SetLFG(uint32 CharID, bool LFG);
	void	SetFirstLogon(uint32 CharID, uint8 firstlogon);
	void	SetLoginFlags(uint32 CharID, bool LFP, bool LFG, uint8 firstlogon);
	void	AddReport(std::string who, std::string against, std::string lines);

private:
	void DBInitVars();

	std::map<uint32,std::string>	zonename_array;

	Mutex				Mvarcache;
	uint32				varcache_max;
	VarCache_Struct**	varcache_array;
	uint32				varcache_lastupdate;


	/*
	* Groups, utility methods.
	*/
	void    ClearAllGroupLeaders();
	void    ClearAllGroups();


	/*
	* Raid, utility methods.
	*/
	void ClearAllRaids();
	void ClearAllRaidDetails();
	void ClearAllRaidLeaders();
};

#endif
