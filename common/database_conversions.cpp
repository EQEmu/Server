/*	EQEMu: Everquest Server Emulator
Copyright (C) 2001-2015 EQEMu Development Team (http://eqemulator.net)

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

#include "../common/global_define.h"
#include "../common/rulesys.h"
#include "../common/string_util.h"
#include "../common/timer.h"

#include "database.h"
#include "extprofile.h"

#include <iomanip>
#include <iostream>

// Disgrace: for windows compile
#ifdef _WINDOWS
#include <windows.h>
#define snprintf	_snprintf
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#else
#include "unix.h"
#include <netinet/in.h>
#include <sys/time.h>
#endif

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

	static const size_t BANDOLIERS_SIZE = 4;
	static const size_t BANDOLIER_ITEM_COUNT = 4;
	struct BandolierItem_Struct {
		uint32 ID;
		uint32 Icon;
		char Name[64];
	};
	struct Bandolier_Struct {
		char Name[32];
		Convert::BandolierItem_Struct Items[Convert::BANDOLIER_ITEM_COUNT];
	};

	static const size_t POTION_BELT_ITEM_COUNT = 4;
	struct PotionBeltItem_Struct {
		uint32 ID;
		uint32 Icon;
		char Name[64];
	};
	struct PotionBelt_Struct {
		Convert::PotionBeltItem_Struct Items[Convert::POTION_BELT_ITEM_COUNT];
	};

	struct SuspendedMinion_Struct
	{
		/*000*/	uint16 SpellID;
		/*002*/	uint32 HP;
		/*006*/	uint32 Mana;
		/*010*/	Convert::SpellBuff_Struct Buffs[BUFF_COUNT];
		/*510*/	uint32 Items[EQEmu::textures::materialCount];
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
		/*0312*/	uint32							item_material[EQEmu::textures::materialCount];	// Item texture/material of worn/held items
		/*0348*/	uint8							unknown0348[44];
		/*0392*/	Convert::Color_Struct			item_tint[EQEmu::textures::materialCount];
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
		/*7224*/	Convert::Tribute_Struct			tributes[EQEmu::legacy::TRIBUTE_SIZE];
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
		/*12844*/	Convert::Bandolier_Struct		bandoliers[Convert::BANDOLIERS_SIZE];
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
			uint32 aug6;
			uint8 attuned;
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

static inline void loadbar(unsigned int x, unsigned int n, unsigned int w = 50) {
	if ((x != n) && (x % (n / 100 + 1) != 0)) return;
	float ratio = x / (float)n;
	int   c = ratio * w;
	std::cout << std::setw(3) << (int)(ratio * 100) << "% [";
	for (int x = 0; x < c; x++) std::cout << "=";
	for (int x = c; x < w; x++) std::cout << " ";
	std::cout << "]\r" << std::flush;
}


bool Database::CheckDatabaseConversions() {
	CheckDatabaseConvertPPDeblob();
	CheckDatabaseConvertCorpseDeblob();

	/* Run EQEmu Server script (Checks for database updates) */
	system("perl eqemu_server.pl ran_from_world");

	return true;
}

bool Database::CheckDatabaseConvertPPDeblob(){
	unsigned int lengths;
	unsigned int lengths_e;
	std::string squery;
	Convert::PlayerProfile_Struct* pp;
	ExtendedProfile_Struct* e_pp;
	uint32 pplen = 0;
	uint32 i;
	int character_id = 0;
	int account_id = 0;
	int number_of_characters = 0;
	int printppdebug = 0; /* Prints Player Profile */
	int runconvert = 0;

	/* Check For Legacy Storage Method */
	std::string rquery = StringFormat("SHOW TABLES LIKE 'character_'");
	auto results = QueryDatabase(rquery);
	if (results.RowCount() == 1){
		runconvert = 1;
		printf("\n\n::: Legacy Character Data Binary Blob Storage Detected... \n");
		printf("----------------------------------------------------------\n\n");
		printf(" Database currently has character data being stored via \n");
		printf("  the legacy character storage method and will proceed with converting...\n\n");
		printf(" It is recommended that you backup your database \n");
		printf("  before continuing the automatic conversion process...\n\n");
		printf("----------------------------------------------------------\n\n");
		std::cout << "Press ENTER to continue....." << std::endl << std::endl;
		std::cin.ignore(1);
	}

	// runconvert = 0;
	// printppdebug = 1;

	if (runconvert == 1){
		printf("Running character binary blob to database conversion... \n");
		/* Get the number of characters */
		rquery = StringFormat("SELECT COUNT(`id`) FROM `character_`");
		results = QueryDatabase(rquery);
		for (auto row = results.begin(); row != results.end(); ++row) {
			number_of_characters = atoi(row[0]);
			printf("Number of Characters in Database: %i \n", number_of_characters);
		}

		/* Check for table `character_data` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_data'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_data` doesn't exist... creating...");
			rquery = StringFormat(
				"CREATE TABLE `character_data` (									"
				"`id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT,						"
				"`account_id` int(11) NOT NULL DEFAULT '0',							"
				"`name` varchar(64) NOT NULL DEFAULT '',							"
				"`last_name` varchar(64) NOT NULL DEFAULT '',						"
				"`title` varchar(32) NOT NULL DEFAULT '',							"
				"`suffix` varchar(32) NOT NULL DEFAULT '',							"
				"`zone_id` int(11) UNSIGNED NOT NULL DEFAULT 0,						"
				"`zone_instance` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`y` float NOT NULL DEFAULT '0',									"
				"`x` float NOT NULL DEFAULT '0',									"
				"`z` float NOT NULL DEFAULT '0',									"
				"`heading` float NOT NULL DEFAULT '0',								"
				"`gender` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`race` smallint(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`class` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`level` int(11) UNSIGNED NOT NULL DEFAULT 0,						"
				"`deity` int(11) UNSIGNED NOT NULL DEFAULT 0,						"
				"`birthday` int(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`last_login` int(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`time_played` int(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`level2` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`anon` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`gm` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,						"
				"`face` int(11) UNSIGNED NOT NULL DEFAULT 0,						"
				"`hair_color` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`hair_style` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`beard` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`beard_color` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`eye_color_1` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`eye_color_2` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`drakkin_heritage` int(11) UNSIGNED NOT NULL DEFAULT 0,			"
				"`drakkin_tattoo` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`drakkin_details` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`ability_time_seconds` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,	"
				"`ability_number` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,			"
				"`ability_time_minutes` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,	"
				"`ability_time_hours` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`exp` int(11) UNSIGNED NOT NULL DEFAULT 0,							"
				"`aa_points_spent` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`aa_exp` int(11) UNSIGNED NOT NULL DEFAULT 0,						"
				"`aa_points` int(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`group_leadership_exp` int(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`raid_leadership_exp` int(11) UNSIGNED NOT NULL DEFAULT 0,			"
				"`group_leadership_points` int(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`raid_leadership_points` int(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`points` int(11) UNSIGNED NOT NULL DEFAULT 0,						"
				"`cur_hp` int(11) UNSIGNED NOT NULL DEFAULT 0,						"
				"`mana` int(11) UNSIGNED NOT NULL DEFAULT 0,						"
				"`endurance` int(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`intoxication` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`str` int(11) UNSIGNED NOT NULL DEFAULT 0,							"
				"`sta` int(11) UNSIGNED NOT NULL DEFAULT 0,							"
				"`cha` int(11) UNSIGNED NOT NULL DEFAULT 0,							"
				"`dex` int(11) UNSIGNED NOT NULL DEFAULT 0,							"
				"`int` int(11) UNSIGNED NOT NULL DEFAULT 0,							"
				"`agi` int(11) UNSIGNED NOT NULL DEFAULT 0,							"
				"`wis` int(11) UNSIGNED NOT NULL DEFAULT 0,							"
				"`zone_change_count` int(11) UNSIGNED NOT NULL DEFAULT 0,			"
				"`toxicity` int(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`hunger_level` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`thirst_level` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`ability_up` int(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`ldon_points_guk` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`ldon_points_mir` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`ldon_points_mmc` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`ldon_points_ruj` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`ldon_points_tak` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`ldon_points_available` int(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`tribute_time_remaining` int(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`career_tribute_points` int(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`tribute_points` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`tribute_active` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`pvp_status` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`pvp_kills` int(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`pvp_deaths` int(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`pvp_current_points` int(11) UNSIGNED NOT NULL DEFAULT 0,			"
				"`pvp_career_points` int(11) UNSIGNED NOT NULL DEFAULT 0,			"
				"`pvp_best_kill_streak` int(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`pvp_worst_death_streak` int(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`pvp_current_kill_streak` int(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`pvp2` int(11) UNSIGNED NOT NULL DEFAULT 0,						"
				"`pvp_type` int(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`show_helm` int(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`group_auto_consent` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`raid_auto_consent` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`guild_auto_consent` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`leadership_exp_on` tinyint(11) UNSIGNED NOT NULL DEFAULT 0,		"
				"`RestTimer` int(11) UNSIGNED NOT NULL DEFAULT 0,					"
				"`air_remaining` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`autosplit_enabled` int(11) UNSIGNED NOT NULL DEFAULT 0,			"
				"`lfp` tinyint(1) unsigned NOT NULL DEFAULT '0',					"
				"`lfg` tinyint(1) unsigned NOT NULL DEFAULT '0',					"
				"`mailkey` char(16) NOT NULL DEFAULT '',							"
				"`xtargets` tinyint(3) unsigned NOT NULL DEFAULT '5',				"
				"`firstlogon` tinyint(3) NOT NULL DEFAULT '0',						"
				"`e_aa_effects` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`e_percent_to_aa` int(11) UNSIGNED NOT NULL DEFAULT 0,				"
				"`e_expended_aa_spent` int(11) UNSIGNED NOT NULL DEFAULT 0,			"
				"PRIMARY KEY(`id`),													"
				"UNIQUE KEY `name` (`name`),										"
				"KEY `account_id` (`account_id`)									"
				") ENGINE = InnoDB AUTO_INCREMENT = 1 DEFAULT CHARSET = latin1;		"
				);
			auto results = QueryDatabase(rquery);
			printf(" done...\n");
		}
		/* Check for table `character_currency` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_currency'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_currency` doesn't exist... creating...");
			rquery = StringFormat(
				" CREATE TABLE `character_currency` (                                  "
				" 	`id` int(11) UNSIGNED NOT NULL DEFAULT 0,                              "
				" 	`platinum` int(11) UNSIGNED NOT NULL DEFAULT 0,                    "
				" 	`gold` int(11) UNSIGNED NOT NULL DEFAULT 0,                        "
				" 	`silver` int(11) UNSIGNED NOT NULL DEFAULT 0,                      "
				" 	`copper` int(11) UNSIGNED NOT NULL DEFAULT 0,                      "
				" 	`platinum_bank` int(11) UNSIGNED NOT NULL DEFAULT 0,               "
				" 	`gold_bank` int(11) UNSIGNED NOT NULL DEFAULT 0,                   "
				" 	`silver_bank` int(11) UNSIGNED NOT NULL DEFAULT 0,                 "
				" 	`copper_bank` int(11) UNSIGNED NOT NULL DEFAULT 0,                 "
				" 	`platinum_cursor` int(11) UNSIGNED NOT NULL DEFAULT 0,             "
				" 	`gold_cursor` int(11) UNSIGNED NOT NULL DEFAULT 0,                 "
				" 	`silver_cursor` int(11) UNSIGNED NOT NULL DEFAULT 0,               "
				" 	`copper_cursor` int(11) UNSIGNED NOT NULL DEFAULT 0,               "
				" 	`radiant_crystals` int(11) UNSIGNED NOT NULL DEFAULT 0,            "
				" 	`career_radiant_crystals` int(11) UNSIGNED NOT NULL DEFAULT 0,     "
				" 	`ebon_crystals` int(11) UNSIGNED NOT NULL DEFAULT 0,               "
				" 	`career_ebon_crystals` int(11) UNSIGNED NOT NULL DEFAULT 0,        "
				" 	PRIMARY KEY (`id`),                                                "
				"   KEY `id` (`id`)                                                    "
				" ) ENGINE=InnoDB DEFAULT CHARSET=latin1;             "
				);
			auto results = QueryDatabase(rquery);
			printf(" done...\n");
		}
		/* Check for table `character_alternate_abilities` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_alternate_abilities'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_alternate_abilities` doesn't exist... creating...");
			rquery = StringFormat(
				" CREATE TABLE `character_alternate_abilities` (						"
				" `id` int(11) UNSIGNED NOT NULL DEFAULT 0,									"
				" `slot` smallint(11) UNSIGNED NOT NULL DEFAULT 0,						"
				" `aa_id` smallint(11) UNSIGNED NOT NULL DEFAULT 0,						"
				" `aa_value` smallint(11) UNSIGNED NOT NULL DEFAULT 0,					"
				" PRIMARY KEY(`id`,`slot`),												"
				" KEY `id` (`id`)														"
				" ) ENGINE = InnoDB DEFAULT CHARSET = latin1;		"
				);
			auto results = QueryDatabase(rquery);
			printf(" done...\n");
		}
		/* Check for table `character_bind` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_bind'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_bind` doesn't exist... creating...");
			rquery = StringFormat(
				"CREATE TABLE `character_bind` (							   "
				"`id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT,				   "
				"`is_home` tinyint(11) UNSIGNED NOT NULL DEFAULT '0',		   "
				"`zone_id` smallint(11) UNSIGNED NOT NULL DEFAULT '0',		   "
				"`instance_id` mediumint(11) UNSIGNED NOT NULL DEFAULT '0',	   "
				"`x` float NOT NULL DEFAULT '0',							   "
				"`y` float NOT NULL DEFAULT '0',							   "
				"`z` float NOT NULL DEFAULT '0',							   "
				"`heading` float NOT NULL DEFAULT '0',						   "
				"PRIMARY KEY(`id`, `is_home`),								   "
				"KEY `id` (`id`)											   "
				") ENGINE = InnoDB DEFAULT CHARSET = latin1;"
				);
			auto results = QueryDatabase(rquery);
			printf(" done...\n");
		}
		/* Check for table `character_languages` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_languages'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_languages` doesn't exist... creating...");
			rquery = StringFormat(
				"CREATE TABLE `character_languages` (						   "
				"`id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT,				   "
				"`lang_id` smallint(11) UNSIGNED NOT NULL DEFAULT '0',		   "
				"`value` smallint(11) UNSIGNED NOT NULL DEFAULT '0',		   "
				"PRIMARY KEY(`id`, `lang_id`),								   "
				"KEY `id` (`id`)											   "
				") ENGINE = InnoDB DEFAULT CHARSET = latin1;"
				);
			auto results = QueryDatabase(rquery);
			printf(" done...\n");
		}
		/* Check for table `character_skills` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_skills'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_skills` doesn't exist... creating...");
			rquery = StringFormat(
				"CREATE TABLE `character_skills` (							   "
				"`id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT,				   "
				"`skill_id` smallint(11) UNSIGNED NOT NULL DEFAULT '0',		   "
				"`value` smallint(11) UNSIGNED NOT NULL DEFAULT '0',		   "
				"PRIMARY KEY(`id`, `skill_id`),								   "
				"KEY `id` (`id`)											   "
				") ENGINE = InnoDB DEFAULT CHARSET = latin1;"
				);
			auto results = QueryDatabase(rquery);
			printf(" done...\n");
		}
		/* Check for table `character_spells` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_spells'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_spells` doesn't exist... creating...");
			rquery = StringFormat(
				"CREATE TABLE `character_spells` (							   "
				"`id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT,				   "
				"`slot_id` smallint(11) UNSIGNED NOT NULL DEFAULT '0',		   "
				"`spell_id` smallint(11) UNSIGNED NOT NULL DEFAULT '0',		   "
				"PRIMARY KEY(`id`, `slot_id`),								   "
				"KEY `id` (`id`)											   "
				") ENGINE = InnoDB DEFAULT CHARSET = latin1;"
				);
			auto results = QueryDatabase(rquery);
			printf(" done...\n");
		}
		/* Check for table `character_memmed_spells` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_memmed_spells'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_memmed_spells` doesn't exist... creating...");
			rquery = StringFormat(
				"CREATE TABLE `character_memmed_spells` (							   "
				"`id` int(11) UNSIGNED NOT NULL DEFAULT 0,				   "
				"`slot_id` smallint(11) UNSIGNED NOT NULL DEFAULT '0',		   "
				"`spell_id` smallint(11) UNSIGNED NOT NULL DEFAULT '0',		   "
				"PRIMARY KEY(`id`, `slot_id`),								   "
				"KEY `id` (`id`)											   "
				") ENGINE = InnoDB DEFAULT CHARSET = latin1;"
				);
			auto results = QueryDatabase(rquery);
			printf(" done...\n");
		}
		/* Check for table `character_disciplines` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_disciplines'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_disciplines` doesn't exist... creating...");
			rquery = StringFormat(
				" CREATE TABLE `character_disciplines` (						  "
				" `id` int(11) UNSIGNED NOT NULL DEFAULT 0,				  "
				" `slot_id` smallint(11) UNSIGNED NOT NULL DEFAULT '0',			  "
				" `disc_id` smallint(11) UNSIGNED NOT NULL DEFAULT '0',			  "
				" PRIMARY KEY(`id`, `slot_id`),									  "
				" KEY `id` (`id`)												  "
				" ) ENGINE = InnoDB DEFAULT CHARSET = latin1;  "
				);
			auto results = QueryDatabase(rquery);
			printf(" done...\n");
		}
		/* Check for table `character_material` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_material'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_material` doesn't exist... creating...");
			rquery = StringFormat(
				"CREATE TABLE `character_material` ( "
				"`id` int(11) UNSIGNED NOT NULL AUTO_INCREMENT,"
				"`slot` tinyint(11) UNSIGNED NOT NULL DEFAULT '0',"
				"`blue` tinyint(11) UNSIGNED NOT NULL DEFAULT '0',"
				"`green` tinyint(11) UNSIGNED NOT NULL DEFAULT '0',"
				"`red` tinyint(11) UNSIGNED NOT NULL DEFAULT '0',"
				"`use_tint` tinyint(11) UNSIGNED NOT NULL DEFAULT '0',"
				"`color` int(11) UNSIGNED NOT NULL DEFAULT '0',"
				"PRIMARY KEY(`id`, `slot`),"
				"KEY `id` (`id`)"
				") ENGINE = InnoDB AUTO_INCREMENT = 1 DEFAULT CHARSET = latin1;"
				);
			auto results = QueryDatabase(rquery);
			printf(" done...\n");
		}
		/* Check for table `character_tribute` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_tribute'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_tribute` doesn't exist... creating...");
			rquery = StringFormat(
				"CREATE TABLE `character_tribute` (							   "
				"`id` int(11) unsigned NOT NULL DEFAULT 0,				   "
				"`tier` tinyint(11) unsigned NOT NULL DEFAULT '0',			   "
				"`tribute` int(11) UNSIGNED NOT NULL DEFAULT '0',			   "
				"KEY `id` (`id`)											   "
				") ENGINE = InnoDB DEFAULT CHARSET = latin1;"
				);
			auto results = QueryDatabase(rquery);
			printf(" done...\n");
		}
		/* Check for table `character_bandolier` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_bandolier'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_bandolier` doesn't exist... creating...");
			rquery = StringFormat(
				"CREATE TABLE `character_bandolier` (							"
				"`id` int(11) unsigned NOT NULL DEFAULT 0,					"
				"`bandolier_id` tinyint(11) unsigned NOT NULL DEFAULT '0',		"
				"`bandolier_slot` tinyint(11) unsigned NOT NULL DEFAULT '0',	"
				"`item_id` int(11) UNSIGNED NOT NULL DEFAULT '0',				"
				"`icon` int(11) UNSIGNED NOT NULL DEFAULT '0',				"
				"`bandolier_name` varchar(32) NOT NULL DEFAULT '0',				"
				"PRIMARY KEY(`id`,`bandolier_id`, `bandolier_slot`),			"
				"KEY `id` (`id`)												"
				") ENGINE = InnoDB DEFAULT CHARSET = latin1;	"
				);
			auto results = QueryDatabase(rquery);
			printf(" done...\n");
		}
		/* Check for table `character_potionbelt` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_potionbelt'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_potionbelt` doesn't exist... creating...");
			rquery = StringFormat(
				"CREATE TABLE `character_potionbelt` (						  "
				"`id` int(11) unsigned NOT NULL DEFAULT 0,				  "
				"`potion_id` tinyint(11) unsigned NOT NULL DEFAULT '0',		  "
				"`item_id` int(11) UNSIGNED NOT NULL DEFAULT '0',			  "
				"`icon` int(11) UNSIGNED NOT NULL DEFAULT '0',				  "
				"PRIMARY KEY(`id`,`potion_id`),								  "
				"KEY `id` (`id`)												  "
				") ENGINE = InnoDB DEFAULT CHARSET = latin1;"
				);
			auto results = QueryDatabase(rquery);
			printf(" done...\n");
		}
		/* Check for table `character_potionbelt` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_inspect_messages'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_inspect_messages` doesn't exist... creating...");
			rquery = StringFormat(
				"CREATE TABLE `character_inspect_messages` (					  "
				"`id` int(11) unsigned NOT NULL DEFAULT 0,				  "
				"`inspect_message` varchar(255) NOT NULL DEFAULT '',			  "
				"PRIMARY KEY(`id`),											  "
				"KEY `id` (`id`)												  "
				") ENGINE = InnoDB DEFAULT CHARSET = latin1;"
				);
			auto results = QueryDatabase(rquery);
			printf(" done...\n");
		}
		/* Check for table `character_leadership_abilities` */
		rquery = StringFormat("SHOW TABLES LIKE 'character_leadership_abilities'");
		results = QueryDatabase(rquery);
		if (results.RowCount() == 0){
			printf("Table: `character_leadership_abilities` doesn't exist... creating...");
			rquery = StringFormat(
				"CREATE TABLE `character_leadership_abilities` ("
				"`id` int(11) UNSIGNED NOT NULL DEFAULT 0, "
				"`slot` smallint(11) UNSIGNED NOT NULL DEFAULT 0, "
				"`rank` smallint(11) UNSIGNED NOT NULL DEFAULT 0, "
				"PRIMARY KEY(`id`,`slot`), "
				"KEY `id` (`id`)												  "
				") ENGINE = InnoDB DEFAULT CHARSET = latin1; "
				);
			auto results = QueryDatabase(rquery);
			printf(" done...\n");
		}

		/* Done */
		printf("Starting conversion...\n\n");


		int char_iter_count = 0;
		rquery = StringFormat("SELECT `id` FROM `character_`");
		results = QueryDatabase(rquery);

		uint8 firstlogon = 0;
		uint8 lfg = 0;
		uint8 lfp = 0;
		std::string mailkey;
		uint8 xtargets = 0;
		std::string inspectmessage;

		for (auto row = results.begin(); row != results.end(); ++row) {
			char_iter_count++;
			squery = StringFormat("SELECT `id`, `profile`, `name`, `level`, `account_id`, `firstlogon`, `lfg`, `lfp`, `mailkey`, `xtargets`, `inspectmessage`, `extprofile` FROM `character_` WHERE `id` = %i", atoi(row[0]));
			auto results2 = QueryDatabase(squery);
			auto row2 = results2.begin();
			pp = (Convert::PlayerProfile_Struct*)row2[1];
			e_pp = (ExtendedProfile_Struct*)row2[11];
			character_id = atoi(row[0]);
			account_id = atoi(row2[4]);
			/* Convert some data from the character_ table that is still relevant */
			firstlogon = atoi(row2[5]);
			lfg = atoi(row2[6]);
			lfp = atoi(row2[7]);
			mailkey = row2[8];
			xtargets = atoi(row2[9]);
			inspectmessage = row2[10];

			/* Verify PP Integrity */
			lengths = results2.LengthOfColumn(1);
			if (lengths == sizeof(Convert::PlayerProfile_Struct)) { /* If PP is the size it is expected to be */
				memcpy(pp, row2[1], sizeof(Convert::PlayerProfile_Struct));
			}
			/* Continue of PP Size does not match (Usually a created character never logged in) */
			else {
				std::cout << (row2[2] ? row2[2] : "Unknown") << " ID: " << character_id << " size mismatch. Expected Size: " << sizeof(Convert::PlayerProfile_Struct) << " Seen: " << lengths << std::endl;
				continue;
			}

			lengths_e = results2.LengthOfColumn(11);
			if (lengths_e == sizeof(ExtendedProfile_Struct)) {
				memcpy(e_pp, row2[11], sizeof(ExtendedProfile_Struct));
			}
			if (e_pp->expended_aa > 4000000){ e_pp->expended_aa = 0; }

			/* Loading Status on conversion */
			if (runconvert == 1){
				std::cout << "\r" << char_iter_count << "/" << number_of_characters << " " << std::flush;
				loadbar(char_iter_count, number_of_characters, 50);

				/* Run inspect message convert  */
				if (inspectmessage != ""){
					std::string rquery = StringFormat("REPLACE INTO `character_inspect_messages` (id, inspect_message)"
						"VALUES (%u, '%s')",
						character_id,
						EscapeString(inspectmessage).c_str()
						);
					auto results = QueryDatabase(rquery);
				}

				/* Run Currency Convert */
				std::string rquery = StringFormat("REPLACE INTO `character_currency` (id, platinum, gold, silver, copper,"
					"platinum_bank, gold_bank, silver_bank, copper_bank,"
					"platinum_cursor, gold_cursor, silver_cursor, copper_cursor, "
					"radiant_crystals, career_radiant_crystals, ebon_crystals, career_ebon_crystals)"
					"VALUES (%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u)",
					character_id,
					pp->platinum,
					pp->gold,
					pp->silver,
					pp->copper,
					pp->platinum_bank,
					pp->gold_bank,
					pp->silver_bank,
					pp->copper_bank,
					pp->platinum_cursor,
					pp->gold_cursor,
					pp->silver_cursor,
					pp->copper_cursor,
					pp->currentRadCrystals,
					pp->careerRadCrystals,
					pp->currentEbonCrystals,
					pp->careerEbonCrystals
					);
				auto results = QueryDatabase(rquery);

				if (pp->tribute_time_remaining < 0 || pp->tribute_time_remaining == 4294967295){ pp->tribute_time_remaining = 0; }

				/* Run Character Data Convert */
				rquery = StringFormat(
					"REPLACE INTO `character_data` ("
					"id,"
					"account_id,"
					"`name`,"
					"last_name,"
					"gender,"
					"race,"
					"class,"
					"`level`,"
					"deity,"
					"birthday,"
					"last_login,"
					"time_played,"
					"pvp_status,"
					"level2,"
					"anon,"
					"gm,"
					"intoxication,"
					"hair_color,"
					"beard_color,"
					"eye_color_1,"
					"eye_color_2,"
					"hair_style,"
					"beard,"
					"ability_time_seconds,"
					"ability_number,"
					"ability_time_minutes,"
					"ability_time_hours,"
					"title,"
					"suffix,"
					"exp,"
					"points,"
					"mana,"
					"cur_hp,"
					"str,"
					"sta,"
					"cha,"
					"dex,"
					"`int`,"
					"agi,"
					"wis,"
					"face,"
					"y,"
					"x,"
					"z,"
					"heading,"
					"pvp2,"
					"pvp_type,"
					"autosplit_enabled,"
					"zone_change_count,"
					"drakkin_heritage,"
					"drakkin_tattoo,"
					"drakkin_details,"
					"toxicity,"
					"hunger_level,"
					"thirst_level,"
					"ability_up,"
					"zone_id,"
					"zone_instance,"
					"leadership_exp_on,"
					"ldon_points_guk,"
					"ldon_points_mir,"
					"ldon_points_mmc,"
					"ldon_points_ruj,"
					"ldon_points_tak,"
					"ldon_points_available,"
					"tribute_time_remaining,"
					"show_helm,"
					"career_tribute_points,"
					"tribute_points,"
					"tribute_active,"
					"endurance,"
					"group_leadership_exp,"
					"raid_leadership_exp,"
					"group_leadership_points,"
					"raid_leadership_points,"
					"air_remaining,"
					"pvp_kills,"
					"pvp_deaths,"
					"pvp_current_points,"
					"pvp_career_points,"
					"pvp_best_kill_streak,"
					"pvp_worst_death_streak,"
					"pvp_current_kill_streak,"
					"aa_points_spent,"
					"aa_exp,"
					"aa_points,"
					"group_auto_consent,"
					"raid_auto_consent,"
					"guild_auto_consent,"
					"RestTimer,"
					"firstlogon,"
					"lfg,"
					"lfp,"
					"mailkey,"
					"xtargets,"
					"e_aa_effects,"
					"e_percent_to_aa,"
					"e_expended_aa_spent"
					")"
					"VALUES ("
					"%u,"		// id														
					"%u,"		// account_id												
					"'%s',"		// `name`					  
					"'%s',"		// last_name					
					"%u,"		// gender					  
					"%u,"		// race						  
					"%u,"		// class							
					"%u,"		// `level`					  
					"%u,"		// deity							
					"%u,"		// birthday					  
					"%u,"		// last_login				  
					"%u,"		// time_played				  
					"%u,"		// pvp_status				  
					"%u,"		// level2					  
					"%u,"		// anon						  
					"%u,"		// gm						  
					"%u,"		// intoxication				  
					"%u,"		// hair_color				  
					"%u,"		// beard_color				  
					"%u,"		// eye_color_1				  
					"%u,"		// eye_color_2				  
					"%u,"		// hair_style				  
					"%u,"		// beard							
					"%u,"		// ability_time_seconds		  
					"%u,"		// ability_number			  
					"%u,"		// ability_time_minutes		  
					"%u,"		// ability_time_hours		  
					"'%s',"		// title						
					"'%s',"		// suffix					  
					"%u,"		// exp						  
					"%u,"		// points					  
					"%u,"		// mana						  
					"%u,"		// cur_hp					  
					"%u,"		// str						  
					"%u,"		// sta						  
					"%u,"		// cha						  
					"%u,"		// dex						  
					"%u,"		// `int`							
					"%u,"		// agi						  
					"%u,"		// wis						  
					"%u,"		// face						  
					"%f,"		// y								
					"%f,"		// x								
					"%f,"		// z								
					"%f,"		// heading					  
					"%u,"		// pvp2						  
					"%u,"		// pvp_type					  
					"%u,"		// autosplit_enabled				
					"%u,"		// zone_change_count				
					"%u,"		// drakkin_heritage			  
					"%u,"		// drakkin_tattoo			  
					"%u,"		// drakkin_details			  
					"%i,"		// toxicity	 				  
					"%u,"		// hunger_level				  
					"%u,"		// thirst_level				  
					"%u,"		// ability_up				  
					"%u,"		// zone_id					  
					"%u,"		// zone_instance					
					"%u,"		// leadership_exp_on				
					"%u,"		// ldon_points_guk			  
					"%u,"		// ldon_points_mir			  
					"%u,"		// ldon_points_mmc			  
					"%u,"		// ldon_points_ruj			  
					"%u,"		// ldon_points_tak			  
					"%u,"		// ldon_points_available			
					"%u,"		// tribute_time_remaining	  
					"%u,"		// show_helm						
					"%u,"		// career_tribute_points			
					"%u,"		// tribute_points			  
					"%u,"		// tribute_active			  
					"%u,"		// endurance						
					"%u,"		// group_leadership_exp		  
					"%u,"		// raid_leadership_exp		  
					"%u,"		// group_leadership_points	  
					"%u,"		// raid_leadership_points	  
					"%u,"		// air_remaining					
					"%u,"		// pvp_kills						
					"%u,"		// pvp_deaths				  
					"%u,"		// pvp_current_points		  
					"%u,"		// pvp_career_points				
					"%u,"		// pvp_best_kill_streak		  
					"%u,"		// pvp_worst_death_streak	  
					"%u,"		// pvp_current_kill_streak	  
					"%u,"		// aa_points_spent			  
					"%u,"		// aa_exp					  
					"%u,"		// aa_points						
					"%u,"		// group_auto_consent		  
					"%u,"		// raid_auto_consent				
					"%u,"		// guild_auto_consent		  
					"%u," 		// RestTimer
					"%u,"		// First Logon - References online status for EVENT_CONNECT/EVENT_DISCONNECt
					"%u,"		// Looking for Group
					"%u,"		// Looking for P?
					"'%s',"		// Mailkey
					"%u,"		// X Targets
					"%u,"		// AA Effects
					"%u,"		// Percent to AA
					"%u"		// e_expended_aa_spent
					")",
					character_id,
					account_id,
					EscapeString(pp->name).c_str(),
					EscapeString(pp->last_name).c_str(),
					pp->gender,
					pp->race,
					pp->class_,
					pp->level,
					pp->deity,
					pp->birthday,
					pp->lastlogin,
					pp->timePlayedMin,
					pp->pvp,
					pp->level2,
					pp->anon,
					pp->gm,
					pp->intoxication,
					pp->haircolor,
					pp->beardcolor,
					pp->eyecolor1,
					pp->eyecolor2,
					pp->hairstyle,
					pp->beard,
					pp->ability_time_seconds,
					pp->ability_number,
					pp->ability_time_minutes,
					pp->ability_time_hours,
					EscapeString(pp->title).c_str(),
					EscapeString(pp->suffix).c_str(),
					pp->exp,
					pp->points,
					pp->mana,
					pp->cur_hp,
					pp->STR,
					pp->STA,
					pp->CHA,
					pp->DEX,
					pp->INT,
					pp->AGI,
					pp->WIS,
					pp->face,
					pp->y,
					pp->x,
					pp->z,
					pp->heading,
					pp->pvp2,
					pp->pvptype,
					pp->autosplit,
					pp->zone_change_count,
					pp->drakkin_heritage,
					pp->drakkin_tattoo,
					pp->drakkin_details,
					pp->toxicity,
					pp->hunger_level,
					pp->thirst_level,
					pp->ability_up,
					pp->zone_id,
					pp->zoneInstance,
					pp->leadAAActive == 0 ? 0 : 1,
					pp->ldon_points_guk,
					pp->ldon_points_mir,
					pp->ldon_points_mmc,
					pp->ldon_points_ruj,
					pp->ldon_points_tak,
					pp->ldon_points_available,
					pp->tribute_time_remaining,
					pp->showhelm,
					pp->career_tribute_points,
					pp->tribute_points,
					pp->tribute_active,
					pp->endurance,
					pp->group_leadership_exp,
					pp->raid_leadership_exp,
					pp->group_leadership_points,
					pp->raid_leadership_points,
					pp->air_remaining,
					pp->PVPKills,
					pp->PVPDeaths,
					pp->PVPCurrentPoints,
					pp->PVPCareerPoints,
					pp->PVPBestKillStreak,
					pp->PVPWorstDeathStreak,
					pp->PVPCurrentKillStreak,
					pp->aapoints_spent,
					pp->expAA,
					pp->aapoints,
					pp->groupAutoconsent,
					pp->raidAutoconsent,
					pp->guildAutoconsent,
					pp->RestTimer,
					firstlogon,
					lfg,
					lfp,
					mailkey.c_str(),
					xtargets,
					e_pp->aa_effects,
					e_pp->perAA,
					e_pp->expended_aa
					);
				results = QueryDatabase(rquery);


				/*
				We set a first entry variable because we need the first initial piece of the query to be declared
				This is to speed up the INSERTS and trim down the amount of individual sends during the process.
				The speed difference is dramatic
				*/
				/* Run AA Convert */
				int first_entry = 0; rquery = "";
				for (i = 0; i < MAX_PP_AA_ARRAY; i++){
					if (pp->aa_array[i].AA > 0 && pp->aa_array[i].value > 0){
						if (first_entry != 1){
							rquery = StringFormat("REPLACE INTO `character_alternate_abilities` (id, slot, aa_id, aa_value)"
								" VALUES (%u, %u, %u, %u)", character_id, i, pp->aa_array[i].AA, pp->aa_array[i].value);
							first_entry = 1;
						}
						else {
							rquery = rquery + StringFormat(", (%u, %u, %u, %u)", character_id, i, pp->aa_array[i].AA, pp->aa_array[i].value);
						}
					}
				}
				if (rquery != ""){ results = QueryDatabase(rquery); }

				/* Run Bind Home Convert */
				if (pp->binds[4].zoneId < 999 && !_ISNAN_(pp->binds[4].x) && !_ISNAN_(pp->binds[4].y) && !_ISNAN_(pp->binds[4].z) && !_ISNAN_(pp->binds[4].heading)) {
					rquery = StringFormat("REPLACE INTO `character_bind` (id, zone_id, instance_id, x, y, z, heading, is_home)"
						" VALUES (%u, %u, %u, %f, %f, %f, %f, 1)",
						character_id, pp->binds[4].zoneId, 0, pp->binds[4].x, pp->binds[4].y, pp->binds[4].z, pp->binds[4].heading);
					if (rquery != ""){ results = QueryDatabase(rquery); }
				}

				/* Run Bind Convert */
				if (pp->binds[0].zoneId < 999 && !_ISNAN_(pp->binds[0].x) && !_ISNAN_(pp->binds[0].y) && !_ISNAN_(pp->binds[0].z) && !_ISNAN_(pp->binds[0].heading)) {
					rquery = StringFormat("REPLACE INTO `character_bind` (id, zone_id, instance_id, x, y, z, heading, is_home)"
						" VALUES (%u, %u, %u, %f, %f, %f, %f, 0)",
						character_id, pp->binds[0].zoneId, 0, pp->binds[0].x, pp->binds[0].y, pp->binds[0].z, pp->binds[0].heading);
					if (rquery != ""){ results = QueryDatabase(rquery); }
				}
				/* Run Language Convert */
				first_entry = 0; rquery = "";
				for (i = 0; i < MAX_PP_LANGUAGE; i++){
					if (pp->languages[i] > 0){
						if (first_entry != 1){
							rquery = StringFormat("REPLACE INTO `character_languages` (id, lang_id, value) VALUES (%u, %u, %u)", character_id, i, pp->languages[i]);
							first_entry = 1;
						}
						rquery = rquery + StringFormat(", (%u, %u, %u)", character_id, i, pp->languages[i]);
					}
				}
				if (rquery != ""){ results = QueryDatabase(rquery); }
				/* Run Skill Convert */
				first_entry = 0; rquery = "";
				for (i = 0; i < MAX_PP_SKILL; i++){
					if (pp->skills[i] > 0){
						if (first_entry != 1){
							rquery = StringFormat("REPLACE INTO `character_skills` (id, skill_id, value) VALUES (%u, %u, %u)", character_id, i, pp->skills[i]);
							first_entry = 1;
						}
						rquery = rquery + StringFormat(", (%u, %u, %u)", character_id, i, pp->skills[i]);
					}
				}
				if (rquery != ""){ results = QueryDatabase(rquery); }
				/* Run Spell Convert */
				first_entry = 0; rquery = "";
				for (i = 0; i < MAX_PP_REF_SPELLBOOK; i++){
					if (pp->spell_book[i] > 0 && pp->spell_book[i] != 4294967295 && pp->spell_book[i] < 40000 && pp->spell_book[i] != 1){
						if (first_entry != 1){
							rquery = StringFormat("REPLACE INTO `character_spells` (id, slot_id, spell_id) VALUES (%u, %u, %u)", character_id, i, pp->spell_book[i]);
							first_entry = 1;
						}
						rquery = rquery + StringFormat(", (%u, %u, %u)", character_id, i, pp->spell_book[i]);
					}
				}
				if (rquery != ""){ results = QueryDatabase(rquery); }
				/* Run Max Memmed Spell Convert */
				first_entry = 0; rquery = "";
				for (i = 0; i < MAX_PP_REF_MEMSPELL; i++){
					if (pp->mem_spells[i] > 0 && pp->mem_spells[i] != 65535 && pp->mem_spells[i] != 4294967295){
						if (first_entry != 1){
							rquery = StringFormat("REPLACE INTO `character_memmed_spells` (id, slot_id, spell_id) VALUES (%u, %u, %u)", character_id, i, pp->mem_spells[i]);
							first_entry = 1;
						}
						rquery = rquery + StringFormat(", (%u, %u, %u)", character_id, i, pp->mem_spells[i]);
					}
				}
				if (rquery != ""){ results = QueryDatabase(rquery); }
				/* Run Discipline Convert */
				first_entry = 0; rquery = "";
				for (i = 0; i < MAX_PP_DISCIPLINES; i++){
					if (pp->disciplines.values[i] > 0 && pp->disciplines.values[i] < 60000){
						if (first_entry != 1){
							rquery = StringFormat("REPLACE INTO `character_disciplines` (id, slot_id, disc_id) VALUES (%u, %u, %u)", character_id, i, pp->disciplines.values[i]);
							first_entry = 1;
						}
						rquery = rquery + StringFormat(", (%u, %u, %u)", character_id, i, pp->disciplines.values[i]);
					}
				}
				if (rquery != ""){ results = QueryDatabase(rquery); }
				/* Run Material Color Convert */
				first_entry = 0; rquery = "";
				for (i = EQEmu::textures::textureBegin; i < EQEmu::textures::materialCount; i++){
					if (pp->item_tint[i].color > 0){
						if (first_entry != 1){
							rquery = StringFormat("REPLACE INTO `character_material` (id, slot, blue, green, red, use_tint, color) VALUES (%u, %u, %u, %u, %u, %u, %u)", character_id, i, pp->item_tint[i].rgb.blue, pp->item_tint[i].rgb.green, pp->item_tint[i].rgb.red, pp->item_tint[i].rgb.use_tint, pp->item_tint[i].color);
							first_entry = 1;
						}
						rquery = rquery + StringFormat(", (%u, %u, %u, %u, %u, %u, %u)", character_id, i, pp->item_tint[i].rgb.blue, pp->item_tint[i].rgb.green, pp->item_tint[i].rgb.red, pp->item_tint[i].rgb.use_tint, pp->item_tint[i].color);
					}
				}
				if (rquery != ""){ results = QueryDatabase(rquery); }
				/* Run Tribute Convert */
				first_entry = 0; rquery = "";
				for (i = 0; i < EQEmu::legacy::TRIBUTE_SIZE; i++){
					if (pp->tributes[i].tribute > 0 && pp->tributes[i].tribute != 4294967295){
						if (first_entry != 1){
							rquery = StringFormat("REPLACE INTO `character_tribute` (id, tier, tribute) VALUES (%u, %u, %u)", character_id, pp->tributes[i].tier, pp->tributes[i].tribute);
							first_entry = 1;
						}
						rquery = rquery + StringFormat(", (%u, %u, %u)", character_id, pp->tributes[i].tier, pp->tributes[i].tribute);
					}
				}
				if (rquery != ""){ results = QueryDatabase(rquery); }
				/* Run Bandolier Convert */
				first_entry = 0; rquery = "";
				for (i = 0; i < Convert::BANDOLIERS_SIZE; i++){
					if (strlen(pp->bandoliers[i].Name) < 32) {
						for (int si = 0; si < Convert::BANDOLIER_ITEM_COUNT; si++){
							if (pp->bandoliers[i].Items[si].ID > 0){
								if (first_entry != 1) {
									rquery = StringFormat("REPLACE INTO `character_bandolier` (id, bandolier_id, bandolier_slot, item_id, icon, bandolier_name) VALUES (%i, %u, %i, %u, %u, '%s')", character_id, i, si, pp->bandoliers[i].Items[si].ID, pp->bandoliers[i].Items[si].Icon, pp->bandoliers[i].Name);
									first_entry = 1;
								}
								rquery = rquery + StringFormat(", (%i, %u, %i, %u, %u, '%s')", character_id, i, si, pp->bandoliers[i].Items[si].ID, pp->bandoliers[i].Items[si].Icon, pp->bandoliers[i].Name);
							}
						}
					}
				}
				if (rquery != ""){ results = QueryDatabase(rquery); }
				/* Run Potion Belt Convert */
				first_entry = 0; rquery = "";
				for (i = 0; i < Convert::POTION_BELT_ITEM_COUNT; i++){
					if (pp->potionbelt.Items[i].ID > 0){
						if (first_entry != 1){
							rquery = StringFormat("REPLACE INTO `character_potionbelt` (id, potion_id, item_id, icon) VALUES (%i, %u, %u, %u)", character_id, i, pp->potionbelt.Items[i].ID, pp->potionbelt.Items[i].Icon);
							first_entry = 1;
						}
						rquery = rquery + StringFormat(", (%i, %u, %u, %u)", character_id, i, pp->potionbelt.Items[i].ID, pp->potionbelt.Items[i].Icon);

					}
				}
				if (rquery != ""){ results = QueryDatabase(rquery); }
				/* Run Leadership AA Convert */
				first_entry = 0; rquery = "";
				for (i = 0; i < MAX_LEADERSHIP_AA_ARRAY; i++){
					if (pp->leader_abilities.ranks[i] > 0 && pp->leader_abilities.ranks[i] < 6){
						if (first_entry != 1){
							rquery = StringFormat("REPLACE INTO `character_leadership_abilities` (id, slot, rank) VALUES (%i, %u, %u)", character_id, i, pp->leader_abilities.ranks[i]);
							first_entry = 1;
						}
						rquery = rquery + StringFormat(", (%i, %u, %u)", character_id, i, pp->leader_abilities.ranks[i]);
					}
				}
				if (rquery != ""){ results = QueryDatabase(rquery); }
			}
		}
		if (runconvert == 1){
			std::string rquery = StringFormat("RENAME TABLE `character_` TO `character_old`"); QueryDatabase(rquery);
			printf("\n\nRenaming `character_` table to `character_old`, this is a LARGE table so when you don't need it anymore, I would suggest deleting it yourself...\n");
			printf("\n\nCharacter blob conversion complete, continuing world bootup...\n");
		}
	}
	return true;
}

bool Database::CheckDatabaseConvertCorpseDeblob(){
	Convert::DBPlayerCorpse_Struct_temp* dbpc;
	Convert::classic_db_temp::DBPlayerCorpse_Struct_temp* dbpc_c;
	uint32 in_datasize;
	bool is_sof = false;
	std::string c_type;
	std::string scquery;
	int8 first_entry = 0;

	std::string query = StringFormat("SHOW TABLES LIKE 'player_corpses'");
	auto results = QueryDatabase(query);
	if (results.RowCount() != 0){
		query = StringFormat(
			"CREATE TABLE `character_corpse_items` (	  "
			"`corpse_id` int(11) unsigned NOT NULL,		  "
			"`equip_slot` int(11) unsigned NOT NULL,	  "
			"`item_id` int(11) unsigned DEFAULT NULL,	  "
			"`charges` int(11) unsigned DEFAULT NULL,	  "
			"`aug_1` int(11) unsigned DEFAULT '0',		  "
			"`aug_2` int(11) unsigned DEFAULT '0',		  "
			"`aug_3` int(11) unsigned DEFAULT '0',		  "
			"`aug_4` int(11) unsigned DEFAULT '0',		  "
			"`aug_5` int(11) unsigned DEFAULT '0',		  "
			"`aug_6` int(11) unsigned DEFAULT '0',		  "
			"`attuned` smallint(5) NOT NULL DEFAULT '0',  "
			"PRIMARY KEY(`corpse_id`, `equip_slot`)		  "
			") ENGINE = InnoDB DEFAULT CHARSET = latin1;  "
			);
		results = QueryDatabase(query);
		query = StringFormat("RENAME TABLE `player_corpses` TO `character_corpses`");
		results = QueryDatabase(query);
		query = StringFormat(
			" ALTER TABLE `character_corpses`																				\n"
			" ADD COLUMN `is_locked`  		   tinyint(11) NULL DEFAULT 0 AFTER `WasAtGraveyard`,							\n"
			" ADD COLUMN `exp`  			       int(11) UNSIGNED NULL DEFAULT 0,											\n"
			" ADD COLUMN `size`                  int(11) UNSIGNED NULL DEFAULT 0,											\n"
			" ADD COLUMN `level`                 int(11) UNSIGNED NULL DEFAULT 0,											\n"
			" ADD COLUMN `race`                  int(11) UNSIGNED NULL DEFAULT 0,											\n"
			" ADD COLUMN `gender`                int(11) UNSIGNED NULL DEFAULT 0,											\n"
			" ADD COLUMN `class`                 int(11) UNSIGNED NULL DEFAULT 0,											\n"
			" ADD COLUMN `deity`                 int(11) UNSIGNED NULL DEFAULT 0,											\n"
			" ADD COLUMN `texture`               int(11) UNSIGNED NULL DEFAULT 0,											\n"
			" ADD COLUMN `helm_texture`          int(11) UNSIGNED NULL DEFAULT 0,											\n"
			" ADD COLUMN `copper`                int(11) UNSIGNED NULL DEFAULT 0,											\n"
			" ADD COLUMN `silver`                int(11) UNSIGNED NULL DEFAULT 0,											\n"
			" ADD COLUMN `gold`                  int(11) UNSIGNED NULL DEFAULT 0,											\n"
			" ADD COLUMN `platinum`              int(11) UNSIGNED NULL DEFAULT 0,											\n"
			" ADD COLUMN `hair_color`            int(11) UNSIGNED NULL DEFAULT 0,											\n"
			" ADD COLUMN `beard_color`           int(11) UNSIGNED NULL DEFAULT 0,											\n"
			" ADD COLUMN `eye_color_1`           int(11) UNSIGNED NULL DEFAULT 0,											\n"
			" ADD COLUMN `eye_color_2`           int(11) UNSIGNED NULL DEFAULT 0,											\n"
			" ADD COLUMN `hair_style`            int(11) UNSIGNED NULL DEFAULT 0,											\n"
			" ADD COLUMN `face`                  int(11) UNSIGNED NULL DEFAULT 0,											\n"
			" ADD COLUMN `beard`                 int(11) UNSIGNED NULL DEFAULT 0,											\n"
			" ADD COLUMN `drakkin_heritage`      int(11) UNSIGNED NULL DEFAULT 0,											\n"
			" ADD COLUMN `drakkin_tattoo`        int(11) UNSIGNED NULL DEFAULT 0,											\n"
			" ADD COLUMN `drakkin_details`       int(11) UNSIGNED NULL DEFAULT 0,											\n"
			" ADD COLUMN `wc_1`       		   int(11) UNSIGNED NULL DEFAULT 0,												\n"
			" ADD COLUMN `wc_2`       		   int(11) UNSIGNED NULL DEFAULT 0,												\n"
			" ADD COLUMN `wc_3`       		   int(11) UNSIGNED NULL DEFAULT 0,												\n"
			" ADD COLUMN `wc_4`       		   int(11) UNSIGNED NULL DEFAULT 0,												\n"
			" ADD COLUMN `wc_5`       		   int(11) UNSIGNED NULL DEFAULT 0,												\n"
			" ADD COLUMN `wc_6`       		   int(11) UNSIGNED NULL DEFAULT 0,												\n"
			" ADD COLUMN `wc_7`       		   int(11) UNSIGNED NULL DEFAULT 0,												\n"
			" ADD COLUMN `wc_8`       		   int(11) UNSIGNED NULL DEFAULT 0,												\n"
			" ADD COLUMN `wc_9`       		   int(11) UNSIGNED NULL DEFAULT 0,												\n"
			" CHANGE COLUMN `zoneid` `zone_id`  smallint(5) NOT NULL DEFAULT 0 AFTER `charname`,							\n"
			" CHANGE COLUMN `instanceid` `instance_id`  smallint(5) UNSIGNED NOT NULL DEFAULT 0 AFTER `zone_id`,			\n"
			" CHANGE COLUMN `timeofdeath` `time_of_death`  datetime NOT NULL DEFAULT '0000-00-00 00:00:00' AFTER `data`,	\n"
			" CHANGE COLUMN `rezzed` `is_rezzed`  tinyint(3) UNSIGNED NULL DEFAULT 0 AFTER `time_of_death`,					\n"
			" CHANGE COLUMN `IsBurried` `is_buried`  tinyint(3) NOT NULL DEFAULT 0 AFTER `is_rezzed`;						\n"

			);
		results = QueryDatabase(query);
		query = StringFormat(
			" ALTER TABLE `character_corpses`																	 \n"
			" CHANGE COLUMN `WasAtGraveyard` `was_at_graveyard`  tinyint(3) NOT NULL DEFAULT 0 AFTER `is_buried` \n"
			);
		results = QueryDatabase(query);
	}

	std::string rquery = StringFormat("SHOW COLUMNS FROM `character_corpses` LIKE 'data'");
	results = QueryDatabase(rquery);
	if (results.RowCount() != 0){
		rquery = StringFormat("SELECT DISTINCT charid FROM character_corpses");
		results = QueryDatabase(rquery);
		for (auto row = results.begin(); row != results.end(); ++row) {
			std::string squery = StringFormat("SELECT id, charname, data, time_of_death, is_rezzed FROM character_corpses WHERE `charid` = %i", atoi(row[0]));
			auto results2 = QueryDatabase(squery);
			for (auto row2 = results2.begin(); row2 != results2.end(); ++row2) {
				in_datasize = results2.LengthOfColumn(2);
				dbpc = (Convert::DBPlayerCorpse_Struct_temp*)row2[2];
				dbpc_c = (Convert::classic_db_temp::DBPlayerCorpse_Struct_temp*)row2[2];

				if (dbpc == nullptr)
					continue;
				if (dbpc_c == nullptr)
					continue;


				/* SoF+ */
				uint32 esize1 = (sizeof(Convert::DBPlayerCorpse_Struct_temp) + (dbpc->itemcount * sizeof(Convert::player_lootitem_temp::ServerLootItem_Struct_temp)));
				uint32 esize2 = (sizeof(Convert::classic_db_temp::DBPlayerCorpse_Struct_temp) + (dbpc_c->itemcount * sizeof(Convert::player_lootitem_temp::ServerLootItem_Struct_temp)));

				/* SoF */
				if (in_datasize == esize1) {
					is_sof = true;
					c_type = "SOF";
				}
				/* Classic */
				if (in_datasize == esize2) {
					is_sof = false;
					c_type = "Legacy";
				}
				if (in_datasize != esize2 && in_datasize != esize1) {
					is_sof = false;
					c_type = "NULL";
					continue;
				}
				std::cout << "Converting Corpse: [OK] [" << c_type << "]: " << "ID: " << atoi(row2[0]) << std::endl;

				if (is_sof){
					scquery = StringFormat("UPDATE `character_corpses` SET \n"
						"`is_locked` =          %d,\n"
						"`exp` =                 %u,\n"
						"`size` =               %f,\n"
						"`level` =              %u,\n"
						"`race` =               %u,\n"
						"`gender` =             %u,\n"
						"`class` =              %u,\n"
						"`deity` =              %u,\n"
						"`texture` =            %u,\n"
						"`helm_texture` =       %u,\n"
						"`copper` =             %u,\n"
						"`silver` =             %u,\n"
						"`gold` =               %u,\n"
						"`platinum` =           %u,\n"
						"`hair_color`  =        %u,\n"
						"`beard_color` =        %u,\n"
						"`eye_color_1` =        %u,\n"
						"`eye_color_2` =        %u,\n"
						"`hair_style`  =        %u,\n"
						"`face` =               %u,\n"
						"`beard` =              %u,\n"
						"`drakkin_heritage` =    %u,\n"
						"`drakkin_tattoo`  =    %u,\n"
						"`drakkin_details` =    %u,\n"
						"`wc_1` =               %u,\n"
						"`wc_2` =               %u,\n"
						"`wc_3` =               %u,\n"
						"`wc_4` =               %u,\n"
						"`wc_5` =               %u,\n"
						"`wc_6` =               %u,\n"
						"`wc_7` =               %u,\n"
						"`wc_8` =               %u,\n"
						"`wc_9`	=                %u \n"
						"WHERE `id` = %u		   \n",
						dbpc->locked,
						dbpc->exp,
						dbpc->size,
						dbpc->level,
						dbpc->race,
						dbpc->gender,
						dbpc->class_,
						dbpc->deity,
						dbpc->texture,
						dbpc->helmtexture,
						dbpc->copper,
						dbpc->silver,
						dbpc->gold,
						dbpc->plat,
						dbpc->haircolor,
						dbpc->beardcolor,
						dbpc->eyecolor1,
						dbpc->eyecolor2,
						dbpc->hairstyle,
						dbpc->face,
						dbpc->beard,
						dbpc->drakkin_heritage,
						dbpc->drakkin_tattoo,
						dbpc->drakkin_details,
						dbpc->item_tint[0].color,
						dbpc->item_tint[1].color,
						dbpc->item_tint[2].color,
						dbpc->item_tint[3].color,
						dbpc->item_tint[4].color,
						dbpc->item_tint[5].color,
						dbpc->item_tint[6].color,
						dbpc->item_tint[7].color,
						dbpc->item_tint[8].color,
						atoi(row2[0])
						);
					if (scquery != ""){ auto sc_results = QueryDatabase(scquery); }

					first_entry = 0;
					scquery = "";
					/* Print Items */
					for (unsigned int i = 0; i < dbpc->itemcount; i++) {
						if (first_entry != 1){
							scquery = StringFormat("REPLACE INTO `character_corpse_items` \n"
								" (corpse_id, equip_slot, item_id, charges, aug_1, aug_2, aug_3, aug_4, aug_5, aug_6, attuned) \n"
								" VALUES (%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u) \n",
								atoi(row2[0]),
								dbpc->items[i].equipSlot,
								dbpc->items[i].item_id,
								dbpc->items[i].charges,
								dbpc->items[i].aug1,
								dbpc->items[i].aug2,
								dbpc->items[i].aug3,
								dbpc->items[i].aug4,
								dbpc->items[i].aug5,
								dbpc->items[i].aug6,
								dbpc->items[i].attuned
								);
							first_entry = 1;
						}
						else{
							scquery = scquery + StringFormat(", (%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u) \n",
								atoi(row2[0]),
								dbpc->items[i].equipSlot,
								dbpc->items[i].item_id,
								dbpc->items[i].charges,
								dbpc->items[i].aug1,
								dbpc->items[i].aug2,
								dbpc->items[i].aug3,
								dbpc->items[i].aug4,
								dbpc->items[i].aug5,
								dbpc->items[i].aug6,
								dbpc->items[i].attuned
								);
						}
					}
					if (scquery != ""){ auto sc_results = QueryDatabase(scquery); }
				}
				else{
					/* Classic Converter */
					scquery = StringFormat("UPDATE `character_corpses` SET \n"
						"`is_locked` =          %d,\n"
						"`exp` =                %u,\n"
						"`size` =               %f,\n"
						"`level` =              %u,\n"
						"`race` =               %u,\n"
						"`gender` =             %u,\n"
						"`class` =              %u,\n"
						"`deity` =              %u,\n"
						"`texture` =            %u,\n"
						"`helm_texture` =       %u,\n"
						"`copper` =             %u,\n"
						"`silver` =             %u,\n"
						"`gold` =               %u,\n"
						"`platinum` =           %u,\n"
						"`hair_color`  =        %u,\n"
						"`beard_color` =        %u,\n"
						"`eye_color_1` =        %u,\n"
						"`eye_color_2` =        %u,\n"
						"`hair_style`  =        %u,\n"
						"`face` =               %u,\n"
						"`beard` =              %u,\n"
						"`wc_1` =               %u,\n"
						"`wc_2` =               %u,\n"
						"`wc_3` =               %u,\n"
						"`wc_4` =               %u,\n"
						"`wc_5` =               %u,\n"
						"`wc_6` =               %u,\n"
						"`wc_7` =               %u,\n"
						"`wc_8` =               %u,\n"
						"`wc_9`	=               %u \n"
						"WHERE `id` = %u		   \n",
						dbpc_c->locked,
						dbpc_c->exp,
						dbpc_c->size,
						dbpc_c->level,
						dbpc_c->race,
						dbpc_c->gender,
						dbpc_c->class_,
						dbpc_c->deity,
						dbpc_c->texture,
						dbpc_c->helmtexture,
						dbpc_c->copper,
						dbpc_c->silver,
						dbpc_c->gold,
						dbpc_c->plat,
						dbpc_c->haircolor,
						dbpc_c->beardcolor,
						dbpc_c->eyecolor1,
						dbpc_c->eyecolor2,
						dbpc_c->hairstyle,
						dbpc_c->face,
						dbpc_c->beard,
						dbpc_c->item_tint[0].color,
						dbpc_c->item_tint[1].color,
						dbpc_c->item_tint[2].color,
						dbpc_c->item_tint[3].color,
						dbpc_c->item_tint[4].color,
						dbpc_c->item_tint[5].color,
						dbpc_c->item_tint[6].color,
						dbpc_c->item_tint[7].color,
						dbpc_c->item_tint[8].color,
						atoi(row2[0])
						);
					if (scquery != ""){ auto sc_results = QueryDatabase(scquery); }

					first_entry = 0;
					scquery = "";

					/* Print Items */
					for (unsigned int i = 0; i < dbpc_c->itemcount; i++) {
						if (first_entry != 1){
							scquery = StringFormat("REPLACE INTO `character_corpse_items` \n"
								" (corpse_id, equip_slot, item_id, charges, aug_1, aug_2, aug_3, aug_4, aug_5, aug_6, attuned) \n"
								" VALUES (%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u) \n",
								atoi(row2[0]),
								dbpc_c->items[i].equipSlot,
								dbpc_c->items[i].item_id,
								dbpc_c->items[i].charges,
								dbpc_c->items[i].aug1,
								dbpc_c->items[i].aug2,
								dbpc_c->items[i].aug3,
								dbpc_c->items[i].aug4,
								dbpc_c->items[i].aug5,
								dbpc_c->items[i].aug6,
								dbpc_c->items[i].attuned
								);
							first_entry = 1;
						}
						else{
							scquery = scquery + StringFormat(", (%u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u) \n",
								atoi(row2[0]),
								dbpc_c->items[i].equipSlot,
								dbpc_c->items[i].item_id,
								dbpc_c->items[i].charges,
								dbpc_c->items[i].aug1,
								dbpc_c->items[i].aug2,
								dbpc_c->items[i].aug3,
								dbpc_c->items[i].aug4,
								dbpc_c->items[i].aug5,
								dbpc_c->items[i].aug6,
								dbpc_c->items[i].attuned
								);
						}
					}
					if (scquery != ""){ auto sc_results = QueryDatabase(scquery); }
				}
			}
		}
		QueryDatabase(StringFormat("ALTER TABLE `character_corpses` DROP COLUMN `data`"));
	}
	return true;
}
