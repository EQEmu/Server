/*  EQEMu:  Everquest Server Emulator
	Copyright (C) 2001-2003  EQEMu Development Team (http://eqemulator.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef EQ_PACKET_STRUCTS_H
#define EQ_PACKET_STRUCTS_H

#include "types.h"
#include <string.h>
#include <string>
#include <time.h>
//#include "../common/version.h"
//#include "../common/item_struct.h"

static const uint32 BUFF_COUNT = 25;

//#include "eq_constants.h"

/*
** Compiler override to ensure
** byte aligned structures
*/
#pragma pack(1)

struct LoginInfo_Struct {
/*000*/	char	login_info[64];
/*064*/	uint8	unknown064[124];
/*188*/	uint8	zoning;			// 01 if zoning, 00 if not
/*189*/	uint8	unknown189[275];
/*488*/
};

struct EnterWorld_Struct {
/*000*/	char	name[64];
/*064*/	uint32	tutorial;		// 01 on "Enter Tutorial", 00 if not
/*068*/	uint32	return_home;		// 01 on "Return Home", 00 if not
};

struct ExpansionInfo_Struct {
/*0000*/	uint32	Expansions;
};

/* Name Approval Struct */
/* Len: */
/* Opcode: 0x8B20*/
struct NameApproval
{
	char name[64];
	int32 race;
	int32 class_;
	int32 deity;
};

/*
** Entity identification struct
** Size: 4 bytes
** OPCodes: OP_DeleteSpawn, OP_Assist
*/
struct EntityId_Struct
{
/*00*/	uint32	entity_id;
/*04*/
};

struct Duel_Struct
{
	int32 duel_initiator;
	int32 duel_target;
};

struct DuelResponse_Struct
{
	int32 target_id;
	int32 entity_id;
	int32 unknown;
};

//adventure stuff
enum AdventureObjective
{
	Adventure_Random = 0,
	Adventure_Assassinate = 1,
	Adventure_Kill = 2,
	Adventure_Collect = 3,
	Adventure_Rescue = 4
};

struct AdventureInfo
{
	uint32 id;
	std::string zone_name;
	uint16 zone_version;
	uint16 is_hard;
	uint8 is_raid;
	uint16 min_level;
	uint16 max_level;
	AdventureObjective type;
	uint32 type_data;
	uint16 type_count;
	std::string text;
	uint32 duration;
	uint32 zone_in_time;
	uint8 theme;
	uint16 win_points;
	uint16 lose_points;
	uint16 zone_in_zone_id;
	double zone_in_x; //loc of zone in object
	double zone_in_y; //loc of zone in object
	sint16 zone_in_object_id; //actually a global door id
	double dest_x; //x we zone into
	double dest_y; //y we zone into
	double dest_z; //z we zone into
	double dest_h; //h we zone into
};

struct AdventureDetails
{
	uint32 id;
	AdventureInfo *ai;
	sint32 instance_id;
	int32 count;
	int8 status;
	int32 assassinate_count;
	int32 time_created;
	int32 time_zoned;
	int32 time_completed;
};
///////////////////////////////////////////////////////////////////////////////


/*
** Color_Struct
** Size: 4 bytes
** Used for convenience
** Merth: Gave struct a name so gcc 2.96 would compile
**
*/
struct Color_Struct
{
	union
	{
		struct
		{
			int8	blue;
			int8	green;
			int8	red;
			uint8	use_tint;	// if there's a tint this is FF
		} rgb;
		uint32 color;
	};
};

/*
** Character Selection Struct
** Length: 1704 Bytes
**
*/
struct CharacterSelect_Struct {
/*0000*/	int32	race[10];				// Characters Race
/*0040*/	Color_Struct	cs_colors[10][9];	// Characters Equipment Colors
/*0400*/	int8	beardcolor[10];			// Characters beard Color
/*0410*/	int8	hairstyle[10];			// Characters hair style
/*0420*/	int32	equip[10][9];			// 0=helm, 1=chest, 2=arm, 3=bracer, 4=hand, 5=leg, 6=boot, 7=melee1, 8=melee2  (Might not be)
/*0780*/	int32	secondary[10];			// Characters secondary IDFile number
/*0820*/	int32	drakkin_heritage[10];		// added for SoF
/*0860*/	int32	drakkin_tattoo[10];			// added for SoF
/*0900*/	int32	drakkin_details[10];		// added for SoF
/*0940*/	int32	deity[10];				// Characters Deity
/*0980*/	int8	gohome[10];				// 1=Go Home available, 0=not
/*0990*/	int8	tutorial[10];			// 1=Tutorial available, 0=not
/*1000*/	int8	beard[10];				// Characters Beard Type
/*1010*/	int8	unknown902[10];			// 10x ff
/*1020*/	int32	primary[10];			// Characters primary IDFile number
/*1060*/	int8	haircolor[10];			// Characters Hair Color
/*1070*/	int8	unknown0962[2];			// 2x 00
/*1072*/	int32	zone[10];				// Characters Current Zone
/*1112*/	int8	class_[10];				// Characters Classes
/*1022*/	int8	face[10];				// Characters Face Type
/*1032*/	char	name[10][64];			// Characters Names
/*1672*/	int8	gender[10];				// Characters Gender
/*1682*/	int8	eyecolor1[10];			// Characters Eye Color
/*1692*/	int8	eyecolor2[10];			// Characters Eye 2 Color
/*1702*/	int8	level[10];				// Characters Levels
/*1712*/
};

/*
** Generic Spawn Struct
** Length: 257 Bytes
** Fields from old struct not yet found:
**	float	size;
**	float	walkspeed;	// probably one of the ff 33 33 33 3f
**	float	runspeed;	// probably one of the ff 33 33 33 3f
**	int8	traptype;	// 65 is disarmable trap, 66 and 67 are invis triggers/traps
**	int8	npc_armor_graphic;	// 0xFF=Player, 0=none, 1=leather, 2=chain, 3=steelplate
**	int8	npc_helm_graphic;	// 0xFF=Player, 0=none, 1=leather, 2=chain, 3=steelplate
**
*/

/*
** Generic Spawn Struct
** Length: 383 Octets
** Used in:
**   spawnZoneStruct
**   dbSpawnStruct
**   petStruct
**   newSpawnStruct
*/
/*
showeq -> eqemu
sed -e 's/_t//g' -e 's/seto_0xFF/set_to_0xFF/g'
*/
struct Spawn_Struct {
/*0000*/ uint8 unknown0000;
/*0001*/ uint8  gm;             // 0=no, 1=gm
/*0002*/ uint8 unknown0003;
/*0003*/ int8   aaitle;       // 0=none, 1=general, 2=archtype, 3=class
/*0004*/ uint8 unknown0004;
/*0005*/ uint8  anon;           // 0=normal, 1=anon, 2=roleplay
/*0006*/ uint8  face;	          // Face id for players
/*0007*/ char     name[64];       // Player's Name
/*0071*/ int16  deity;          // Player's Deity
/*0073*/ uint16 unknown0073;
/*0075*/ float    size;           // Model size
/*0079*/ uint32 unknown0079;
/*0083*/ uint8  NPC;            // 0=player,1=npc,2=pc corpse,3=npc corpse,a
/*0084*/ uint8  invis;          // Invis (0=not, 1=invis)
/*0085*/ uint8  haircolor;      // Hair color
/*0086*/ uint8  curHp;          // Current hp %%% wrong
/*0087*/ uint8  max_hp;         // (name prolly wrong)takes on the value 100 for players, 100 or 110 for NPCs and 120 for PC corpses...
/*0088*/ uint8  findable;       // 0=can't be found, 1=can be found
/*0089*/ uint8 unknown0089[5];
/*0094*/ signed   deltaHeading:10;// change in heading
         signed   x:19;           // x coord
         signed   padding0054:3;  // ***Placeholder
/*0098*/ signed   y:19;           // y coord
         signed   animation:10;   // animation
         signed   padding0058:3;  // ***Placeholder
/*0102*/ signed   z:19;           // z coord
         signed   deltaY:13;      // change in y
/*0106*/ signed   deltaX:13;      // change in x
         unsigned heading:12;     // heading
         signed   padding0066:7;  // ***Placeholder
/*0110*/ signed   deltaZ:13;      // change in z
         signed   padding0070:19; // ***Placeholder
/*0114*/ uint8  eyecolor1;      // Player's left eye color
/*0115*/ uint8 unknown0115[12];	// Was [24]
/*0127*/ uint32 drakkin_heritage;	// Added for SoF
/*0131*/ uint32 drakkin_tattoo;		// Added for SoF
/*0135*/ uint32 drakkin_details;	// Added for SoF
/*0139*/ uint8  showhelm;       // 0=no, 1=yes
/*0140*/ uint8 unknown0140[4];
/*0144*/ uint8  is_npc;         // 0=no, 1=yes
/*0145*/ uint8  hairstyle;      // Hair style
/*0146*/ uint8  beard;          // Beard style (not totally, sure but maybe!)
/*0147*/ uint8 unknown0147[4];
/*0151*/ uint8  level;          // Spawn Level
/*0152*/ uint8  unknown0259[4]; // ***Placeholder
/*0156*/ uint8  beardcolor;     // Beard color
/*0157*/ char     suffix[32];     // Player's suffix (of Veeshan, etc.)
/*0189*/ uint32 petOwnerId;     // If this is a pet, the spawn id of owner
/*0193*/ int8   guildrank;      // 0=normal, 1=officer, 2=leader
/*0194*/ uint8 unknown0194[3];
/*0197*/ union
         {
             struct
             {
               /*0197*/ uint32 equip_helmet;    // Equipment: Helmet Visual
               /*0201*/ uint32 equip_chest;     // Equipment: Chest Visual
               /*0205*/ uint32 equip_arms;      // Equipment: Arms Visual
               /*0209*/ uint32 equip_bracers;   // Equipment: Bracers Visual
               /*0213*/ uint32 equip_hands;     // Equipment: Hands Visual
               /*0217*/ uint32 equip_legs;      // Equipment: Legs Visual
               /*0221*/ uint32 equip_feet;      // Equipment: Feet Visual
               /*0225*/ uint32 equip_primary;   // Equipment: Primary Visual
               /*0229*/ uint32 equip_secondary; // Equipment: Secondary Visual
             } equip;
             /*0197*/ uint32 equipment[9];  // Array elements correspond to struct equipment above
         };
/*0233*/ float    runspeed;       // Speed when running
/*0036*/ uint8  afk;            // 0=no, 1=afk
/*0238*/ uint32 guildID;        // Current guild
/*0242*/ char     title[32];      // Title
/*0274*/ uint8 unknown0274;
/*0275*/ uint8  set_to_0xFF[8]; // ***Placeholder (all ff)
/*0283*/ uint8  helm;           // Helm texture
/*0284*/ uint32 race;           // Spawn race
/*0288*/ uint32 unknown0288;
/*0292*/ char     lastName[32];   // Player's Lastname
/*0324*/ float    walkspeed;      // Speed when walking
/*0328*/ uint8 unknown0328;
/*0329*/ uint8  is_pet;         // 0=no, 1=yes
/*0330*/ uint8  light;          // Spawn's lightsource %%% wrong
/*0331*/ uint8  class_;         // Player's class
/*0332*/ uint8  eyecolor2;      // Left eye color
/*0333*/ uint8 flymode;
/*0334*/ uint8  gender;         // Gender (0=male, 1=female)
/*0335*/ uint8  bodytype;       // Bodytype
/*0336*/ uint8 unknown0336[3];
union
{
/*0339*/ int8 equip_chest2;     // Second place in packet for chest texture (usually 0xFF in live packets)
                                  // Not sure why there are 2 of them, but it effects chest texture!
/*0339*/ int8 mount_color;      // drogmor: 0=white, 1=black, 2=green, 3=red
                                  // horse: 0=brown, 1=white, 2=black, 3=tan
};
/*0340*/ uint32 spawnId;        // Spawn Id
/*0344*/ uint8 unknown0344[4];
/*0348*/ union
         {
             struct
             {
                 /*0348*/ Color_Struct color_helmet;    // Color of helmet item
                 /*0352*/ Color_Struct color_chest;     // Color of chest item
                 /*0356*/ Color_Struct color_arms;      // Color of arms item
                 /*0360*/ Color_Struct color_bracers;   // Color of bracers item
                 /*0364*/ Color_Struct color_hands;     // Color of hands item
                 /*0368*/ Color_Struct color_legs;      // Color of legs item
                 /*0372*/ Color_Struct color_feet;      // Color of feet item
                 /*0376*/ Color_Struct color_primary;   // Color of primary item
                 /*0380*/ Color_Struct color_secondary; // Color of secondary item
             } equipment_colors;
             /*0348*/ Color_Struct colors[9]; // Array elements correspond to struct equipment_colors above
         };
/*0384*/ uint8  lfg;            // 0=off, 1=lfg on
/*0385*/

};

/*
** New Spawn
** Length: 176 Bytes
** OpCode: 4921
*/
struct NewSpawn_Struct
{
	struct Spawn_Struct spawn;	// Spawn Information
};

struct ClientZoneEntry_Struct {
/*0000*/	uint32	unknown00;
/*0004*/	char	char_name[64];			// Character Name
};

/*
** Server Zone Entry Struct
** OPCodes: OP_ServerZoneEntry
**
*/
struct ServerZoneEntry_Struct
{
	struct NewSpawn_Struct player;
};

struct NewZone_Struct {
/*0000*/	char	char_name[64];			// Character Name
/*0064*/	char	zone_short_name[32];	// Zone Short Name
/*0096*/	char	zone_long_name[278];	// Zone Long Name
/*0374*/	uint8	ztype;					// Zone type (usually FF)
/*0375*/	uint8	fog_red[4];				// Zone fog (red)
/*0379*/	uint8	fog_green[4];			// Zone fog (green)
/*0383*/	uint8	fog_blue[4];			// Zone fog (blue)
/*0387*/	uint8	unknown323;
/*0388*/	float	fog_minclip[4];
/*0404*/	float	fog_maxclip[4];
/*0420*/	float	gravity;
/*0424*/	int8	time_type;
/*0425*/	uint8	unknown360[49];
/*0474*/	uint8	sky;					// Sky Type
/*0475*/	uint8	unknown331[13];			// ***Placeholder
/*0488*/	float	zone_exp_multiplier;	// Experience Multiplier
/*0492*/	float	safe_y;					// Zone Safe Y
/*0496*/	float	safe_x;					// Zone Safe X
/*0500*/	float	safe_z;					// Zone Safe Z
/*0504*/	float	max_z;					// Guessed
/*0508*/	float	underworld;				// Underworld, min z (Not Sure?)
/*0512*/	float	minclip;				// Minimum View Distance
/*0516*/	float	maxclip;				// Maximum View DIstance
/*0520*/	int8	unknown_end[84];		// ***Placeholder
/*0604*/	char	zone_short_name2[68];
/*0672*/	char	unknown672[12];
/*0684*/	uint16	zone_id;
/*0686*/	uint16	zone_instance;
/*0688*/	uint32	unknown688;
/*0692*/	uint8	unknown692[8];
/*0700*/
};

/*
** Memorize Spell Struct
** Length: 12 Bytes
**
*/
struct MemorizeSpell_Struct {
int32 slot;     // Spot in the spell book/memorized slot
int32 spell_id; // Spell id (200 or c8 is minor healing, etc)
int32 scribing; // 1 if memorizing a spell, set to 0 if scribing to book, 2 if un-memming
int32 unknown12;
};

/*
** Make Charmed Pet
** Length: 12 Bytes
**
*/
struct Charm_Struct {
/*00*/	int32	owner_id;
/*04*/	int32	pet_id;
/*08*/	int32	command;    // 1: make pet, 0: release pet
/*12*/
};

struct InterruptCast_Struct
{
	int32 spawnid;
	int32 messageid;
	char*	message;
};

struct DeleteSpell_Struct
{
/*000*/sint16	spell_slot;
/*002*/int8	unknowndss002[2];
/*004*/int8	success;
/*005*/int8	unknowndss006[3];
/*008*/
};

struct ManaChange_Struct
{
	int32	new_mana;                  // New Mana AMount
	int32	stamina;
	int32	spell_id;
	int32	unknown12;
};

struct SwapSpell_Struct
{
	int32 from_slot;
	int32 to_slot;


};

struct BeginCast_Struct
{
	// len = 8
/*000*/	int16	caster_id;
/*002*/	int16	spell_id;
/*004*/	int32	cast_time;		// in miliseconds
};

struct CastSpell_Struct
{
	int32	slot;
	int32	spell_id;
	int32	inventoryslot;  // slot for clicky item, 0xFFFF = normal cast
	int32	target_id;
	int8    cs_unknown[4];
};

/*
** SpawnAppearance_Struct
** Changes client appearance for all other clients in zone
** Size: 8 bytes
** Used in: OP_SpawnAppearance
**
*/
struct SpawnAppearance_Struct
{
/*0000*/ uint16 spawn_id;          // ID of the spawn
/*0002*/ uint16 type;              // Values associated with the type
/*0004*/ uint32 parameter;         // Type of data sent
/*0008*/
};


// solar: this is used inside profile
struct SpellBuff_Struct
{
/*000*/	int8	slotid;		//badly named... seems to be 2 for a real buff, 0 otherwise
/*001*/ int8	level;
/*002*/	int8	bard_modifier;
/*003*/	int8	effect;			//not real
/*004*/	int32	spellid;
/*008*/ int32	duration;
/*012*/	int16	dmg_shield_remaining;
//these last four bytes are really the caster's global player ID for wearoff
/*013*/ int8	persistant_buff;	//prolly not real, used for perm illusions
/*014*/ int8	reserved;		//proll not real, reserved will use for something else later
/*012*/	int32	player_id;	//'global' ID of the caster, for wearoff messages
};

struct SpellBuffFade_Struct {
/*000*/	uint32 entityid;
/*004*/	int8 slot;
/*005*/	int8 level;
/*006*/	int8 effect;
/*007*/	int8 unknown7;
/*008*/	uint32 spellid;
/*012*/	uint32 duration;
/*016*/	uint32 unknown016;
/*020*/	uint32 unknown020;	//prolly global player ID
/*024*/	uint32 slotid;
/*028*/	uint32 bufffade;
/*032*/
};

struct PetBuff_Struct {
/*000*/ uint32 petid;
/*004*/ uint32 spellid[BUFF_COUNT];
/*104*/ uint32 unknown700;
/*108*/ uint32 unknown701;
/*112*/ uint32 unknown702;
/*116*/ uint32 unknown703;
/*120*/ uint32 unknown704;
/*124*/ uint32 ticsremaining[BUFF_COUNT];
/*224*/ uchar unknown705[20];
/*244*/ uint32 buffcount;
};

struct ItemNamePacket_Struct {
/*000*/	uint32 item_id;
/*004*/	uint32 unkown004;
/*008*/ char name[64];
/*072*/
};

// Length: 10
struct ItemProperties_Struct {

int8	unknown01[2];
int8	charges;
int8	unknown02[13];
};

struct GMTrainee_Struct
{
	/*000*/ uint32 npcid;
	/*004*/ uint32 playerid;
	/*008*/ uint32 skills[73];
	/*300*/ int8 unknown300[148];
	/*448*/
};

struct GMTrainEnd_Struct
{
	/*000*/ uint32 npcid;
	/*004*/ uint32 playerid;
	/*008*/
};

struct GMSkillChange_Struct {
/*000*/	int16		npcid;
/*002*/ int8		unknown1[2];    // something like PC_ID, but not really. stays the same thru the session though
/*002*/ int16       skillbank;      // 0 if normal skills, 1 if languages
/*002*/ int8		unknown2[2];
/*008*/ uint16		skill_id;
/*010*/ int8		unknown3[2];
};

struct GMTrainSkillConfirm_Struct {	// SoF only
/*000*/	uint32	SkillID;
/*004*/	uint32	Cost;
/*008*/	uint8	NewSkill;	// Set to 1 for 'You have learned the basics' message.
/*009*/	char	TrainerName[64];
/*073*/
};

struct ConsentResponse_Struct {
	char grantname[64];
	char ownername[64];
	int8 permission;
	char zonename[32];
};

/*
** Name Generator Struct
** Length: 72 bytes
** OpCode: 0x0290
*/
struct NameGeneration_Struct
{
/*0000*/	int32	race;
/*0004*/	int32	gender;
/*0008*/	char	name[64];
/*0072*/
};

/*
** Character Creation struct
** Length: 140 Bytes
** OpCode: 0x0113
*/
struct CharCreate_Struct
{
/*0000*/	int32	class_;
/*0004*/	int32	haircolor;	// Might be hairstyle
/*0008*/	int32	beardcolor;	// Might be beard
/*0012*/	int32	beard;		// Might be beardcolor
/*0016*/	int32	gender;
/*0020*/	int32	race;
/*0024*/	int32	start_zone;
	// 0 = odus
	// 1 = qeynos
	// 2 = halas
	// 3 = rivervale
	// 4 = freeport
	// 5 = neriak
	// 6 = gukta/grobb
	// 7 = ogguk
	// 8 = kaladim
	// 9 = gfay
	// 10 = felwithe
	// 11 = akanon
	// 12 = cabalis
	// 13 = shar vahl
/*0028*/	int32	hairstyle;	// Might be haircolor
/*0032*/	int32	deity;
/*0036*/	int32	STR;
/*0040*/	int32	STA;
/*0044*/	int32	AGI;
/*0048*/	int32	DEX;
/*0052*/	int32	WIS;
/*0056*/	int32	INT;
/*0060*/	int32	CHA;
/*0064*/	int32	face;		// Could be unknown0076
/*0068*/	int32	eyecolor1;	//its possiable we could have these switched
/*0073*/	int32	eyecolor2;	//since setting one sets the other we really can't check
/*0076*/	int32	drakkin_heritage;	// added for SoF
/*0080*/	int32	drakkin_tattoo;		// added for SoF
/*0084*/	int32	drakkin_details;	// added for SoF
/*0088*/
};

/*
 *Used in PlayerProfile
 */
struct AA_Array
{
	int32 AA;
	int32 value;
};


static const uint32 MAX_PP_DISCIPLINES = 100;
static const uint32 MAX_DISCIPLINE_TIMERS = 20;

struct Disciplines_Struct {
	uint32 values[MAX_PP_DISCIPLINES];
};

static const uint32 TRIBUTE_SLOT_START = 400;
static const uint32 MAX_PLAYER_TRIBUTES = 5;
static const uint32 MAX_PLAYER_BANDOLIER = 4;
static const uint32 MAX_PLAYER_BANDOLIER_ITEMS = 4;
static const uint32 MAX_POTIONS_IN_BELT = 4;
static const uint32 TRIBUTE_NONE = 0xFFFFFFFF;
struct Tribute_Struct {
	uint32 tribute;
	uint32 tier;
};

//len = 72
struct BandolierItem_Struct {
	uint32 item_id;
	uint32 icon;
	char item_name[64];
};

//len = 320
enum { //bandolier item positions
	bandolierMainHand = 0,
	bandolierOffHand,
	bandolierRange,
	bandolierAmmo
};
struct Bandolier_Struct {
	char name[32];
	BandolierItem_Struct items[MAX_PLAYER_BANDOLIER_ITEMS];
};
struct PotionBelt_Struct {
	BandolierItem_Struct items[MAX_POTIONS_IN_BELT];
};

struct MovePotionToBelt_Struct {
	uint32	Action;
	uint32	SlotNumber;
	uint32	ItemID;
};

static const uint32 MAX_GROUP_LEADERSHIP_AA_ARRAY = 16;
static const uint32 MAX_RAID_LEADERSHIP_AA_ARRAY = 16;
static const uint32 MAX_LEADERSHIP_AA_ARRAY = (MAX_GROUP_LEADERSHIP_AA_ARRAY+MAX_RAID_LEADERSHIP_AA_ARRAY);
struct LeadershipAA_Struct {
	uint32 ranks[MAX_LEADERSHIP_AA_ARRAY];
};
struct GroupLeadershipAA_Struct {
	uint32 ranks[MAX_GROUP_LEADERSHIP_AA_ARRAY];
};
struct RaidLeadershipAA_Struct {
	uint32 ranks[MAX_RAID_LEADERSHIP_AA_ARRAY];
};

 /**
* A bind point.
* Size: 20 Octets
*/
struct BindStruct {
   /*000*/ uint32 zone_id;
   /*004*/ float x;
   /*008*/ float y;
   /*012*/ float z;
   /*016*/ float heading;
   /*020*/
};


/*
** Player Profile
**
** Length: 4308 bytes
** OpCode: 0x006a
 */
static const uint32 MAX_PP_LANGUAGE		= 28;
static const uint32 MAX_PP_SPELLBOOK	= 400;
static const uint32 MAX_PP_MEMSPELL		= 9;
static const uint32 MAX_PP_SKILL		= 100;
static const uint32 MAX_PP_AA_ARRAY		= 240;
static const uint32 MAX_GROUP_MEMBERS	= 6;

/*
showeq -> eqemu
sed -e 's/_t//g' -e 's/MAX_AA/MAX_PP_AA_ARRAY/g' \
    -e 's/MAX_SPELL_SLOTS/MAX_PP_MEMSPELL/g' \
    -e 's/MAX_KNOWN_SKILLS/MAX_PP_SKILL/g' \
    -e 's/MAXRIBUTES/MAX_PLAYER_TRIBUTES/g' \
    -e 's/MAX_BUFFS/BUFF_COUNT/g' \
    -e 's/MAX_KNOWN_LANGS/MAX_PP_LANGUAGE/g' \
    -e 's/MAX_RECASTYPES/MAX_RECAST_TYPES/g' \
    -e 's/spellBuff/SpellBuff_Struct/g' \
    -e 's/lastName/last_name/g' \
    -e 's/guildID/guildid/g' \
    -e 's/itemint/item_tint/g' \
    -e 's/MANA/mana/g' \
    -e 's/curHp/cur_hp/g' \
    -e 's/sSpellBook/spell_book/g' \
    -e 's/sMemSpells/mem_spells/g' \
    -e 's/uint32[ \t]*disciplines\[MAX_DISCIPLINES\]/Disciplines_Struct  disciplines/g' \
    -e 's/aa_unspent/aapoints/g' \
    -e 's/aa_spent/aapoints_spent/g' \
    -e 's/InlineItem[ \t]*potionBelt\[MAX_POTIONS_IN_BELT\]/PotionBelt_Struct  potionbelt/g' \
    -e 's/ldon_guk_points/ldon_points_guk/g' \
    -e 's/ldon_mir_points/ldon_points_mir/g' \
    -e 's/ldon_mmc_points/ldon_points_mmc/g' \
    -e 's/ldon_ruj_points/ldon_points_ruj/g' \
    -e 's/ldonak_points/ldon_points_tak/g' \
    -e 's/ldon_avail_points/ldon_points_available/g' \
    -e 's/tributeTime/tribute_time_remaining/g' \
    -e 's/careerTribute/career_tribute_points/g' \
    -e 's/currentTribute/tribute_points/g' \
    -e 's/tributeActive/tribute_active/g' \
    -e 's/TributeStruct/Tribute_Struct/g' \
    -e 's/expGroupLeadAA/group_leadership_exp/g' \
    -e 's/expRaidLeadAA/raid_leadership_exp/g' \
    -e 's/groupLeadAAUnspent/group_leadership_points/g' \
    -e 's/raidLeadAAUnspent/raid_leadership_points/g' \
    -e 's/uint32[ \t]*leadershipAAs\[MAX_LEAD_AA\]/LeadershipAA_Struct leader_abilities/g' \
    -e 's/BandolierStruct/Bandolier_Struct/g' \
    -e 's/MAX_BANDOLIERS/MAX_PLAYER_BANDOLIER/g' \
    -e 's/birthdayTime/birthday/g' \
    -e 's/lastSaveTime/lastlogin/g' \
    -e 's/zoneId/zone_id/g' \
    -e 's/hunger/hunger_level/g' \
    -e 's/thirst/thirst_level/g' \
    -e 's/guildstatus/guildrank/g' \
    -e 's/airRemaining/air_remaining/g' \



 */
struct PlayerProfile_Struct
{
/*0000*/	uint32				checksum;			// Checksum from CRC32::SetEQChecksum
/*0004*/	char				name[64];			// Name of player sizes not right
/*0068*/	char				last_name[32];		// Last name of player sizes not right
/*0100*/	uint32				gender;				// Player Gender - 0 Male, 1 Female
/*0104*/	uint32				race;				// Player race
/*0108*/	uint32				class_;				// Player class
/*0112*/	uint32				unknown0112;		//
/*0116*/	uint32				level;				// Level of player (might be one byte)
/*0120*/	BindStruct			binds[5];           // Bind points (primary is first)
/*0220*/	uint32				deity;				// deity
/*0224*/	uint32				guild_id;
/*0228*/	uint32				birthday;			// characters bday
/*0232*/	uint32				lastlogin;			// last login or zone time
/*0236*/	uint32				timePlayedMin;			// in minutes
/*0240*/	uint8				pvp;
/*0241*/	uint8				level2; //no idea why this is here, but thats how it is on live
/*0242*/	uint8				anon;		// 2=roleplay, 1=anon, 0=not anon
/*0243*/	uint8				gm;
/*0244*/	uint8				guildrank;
/*0245*/	uint8				unknown0245[7];	//
/*0252*/	uint32				intoxication;
/*0256*/	uint32				spellSlotRefresh[MAX_PP_MEMSPELL];	//in ms
/*0292*/	uint32				abilitySlotRefresh;
/*0296*/	uint8				haircolor;			// Player hair color
/*0297*/	uint8				beardcolor;			// Player beard color
/*0298*/	uint8				eyecolor1;			// Player left eye color
/*0299*/	uint8				eyecolor2;			// Player right eye color
/*0300*/	uint8				hairstyle;			// Player hair style
/*0301*/	uint8				beard;				// Beard type
/*0302*/	uint8				ability_time_seconds; //The following four spots are unknown right now.....
/*0303*/	uint8				ability_number; //ability used
/*0304*/	uint8				ability_time_minutes;
/*0305*/	uint8				ability_time_hours;//place holder
/*0306*/	uint8				unknown0306[6];		// @bp Spacer/Flag?
/*0312*/	uint32				item_material[9];	// Item texture/material of worn/held items
/*0348*/	uint8				unknown0256[44];
/*0396*/	Color_Struct		item_tint[9];
/*0432*/	AA_Array			aa_array[MAX_PP_AA_ARRAY];
/*2348*/	float				unknown2348;		//seen ~128, ~47
/*2352*/	char				servername[32];		// length probably not right
/*2384*/	char				title[32];			//length might be wrong
/*2416*/	char				suffix[32];			//length might be wrong
/*2448*/	uint32				guildid2;		//
/*2452*/	uint32				exp;				// Current Experience
/*2456*/	uint32				unknown1496;
/*2460*/	uint32				points;				// Unspent Practice points
/*2464*/	uint32				mana;				// current mana
/*2468*/	uint32				cur_hp;				// current hp
/*2472*/	uint32				unknown1512;		// 0x05
/*2476*/	uint32				STR;				// Strength
/*2480*/	uint32				STA;				// Stamina
/*2484*/	uint32				CHA;				// Charisma
/*2488*/	uint32				DEX;				// Dexterity
/*2492*/	uint32				INT;				// Intelligence
/*2496*/	uint32				AGI;				// Agility
/*2500*/	uint32				WIS;				// Wisdom
/*2504*/	uint8				face;				// Player face
/*2505*/	uint8				unknown1545[47];	// ?
/*2552*/	uint8				languages[MAX_PP_LANGUAGE];
/*2580*/	uint8				unknown1620[4];
/*2584*/	int32				spell_book[MAX_PP_SPELLBOOK];
/*4184*/	uint8				unknown3224[448];	// all 0xff
/*4632*/	int32				mem_spells[MAX_PP_MEMSPELL];
/*4668*/	uint8				unknown3704[32];	//
/*4700*/	float				y;					// Player y position
/*4704*/	float				x;					// Player x position
/*4708*/	float				z;					// Player z position
/*4712*/	float				heading;			// Direction player is facing
/*4716*/	uint8				unknown3756[4];		//
/*4720*/	sint32				platinum;			// Platinum Pieces on player
/*4724*/	sint32				gold;				// Gold Pieces on player
/*4728*/	sint32				silver;				// Silver Pieces on player
/*4732*/	sint32				copper;				// Copper Pieces on player
/*4736*/	sint32				platinum_bank;		// Platinum Pieces in Bank
/*4740*/	sint32				gold_bank;			// Gold Pieces in Bank
/*4744*/	sint32				silver_bank;		// Silver Pieces in Bank
/*4748*/	sint32				copper_bank;		// Copper Pieces in Bank
/*4752*/	sint32				platinum_cursor;	// Platinum on cursor
/*4756*/	sint32				gold_cursor;		// Gold on cursor
/*4760*/	sint32				silver_cursor;		// Silver on cursor
/*4764*/	sint32				copper_cursor;		// Copper on cursor
/*4768*/	sint32				platinum_shared;        // Platinum shared between characters
/*4772*/	uint8				unknown3812[24];        // @bp unknown skills?
/*4796*/	uint32				skills[MAX_PP_SKILL];
/*5196*/	uint8				unknown5096[184];     // @bp unknown skills?
/*5380*/	int32				pvp2;	//
/*5384*/	int32				unknown4420;	//
/*5388*/	int32				pvptype;	//
/*5392*/	int32				unknown4428;	//
/*5396*/	uint32				ability_down;			// Doodman - Guessing
/*5400*/	uint8				unknown4436[8];	//
/*5408*/	uint32				autosplit;			//not used right now
/*5412*/	uint8				unknown4448[8];
/*5420*/	int32				zone_change_count;      // Number of times user has zoned in their career (guessing)
/*5424*/	uint8				unknown4460[16];	//
/*5440*/	uint32				drakkin_heritage;	//
/*5444*/	uint32				drakkin_tattoo;		//
/*5448*/	uint32				drakkin_details;	//
/*5452*/	int32				expansions;		// expansion setting, bit field of expansions avaliable
/*5456*/	sint32				toxicity;	//from drinking potions, seems to increase by 3 each time you drink
/*5460*/	char				unknown4496[16];	//
/*5476*/	sint32				hunger_level;
/*5480*/	sint32				thirst_level;
/*5484*/	int32				ability_up;
/*5488*/	char				unknown4524[16];
/*5504*/	uint16				zone_id;			// Current zone of the player
/*5506*/	uint16				zoneInstance;			// Instance ID
/*5508*/	SpellBuff_Struct	buffs[BUFF_COUNT];			// Buffs currently on the player
/*6008*/	char 				groupMembers[6][64];		//
/*6392*/	char				unknown6392[656];
/*7048*/	uint32				entityid;
/*7052*/	uint32				leadAAActive;
/*7056*/	uint32				unknown7056;
/*7060*/	sint32				ldon_points_guk;		//client uses these as signed
/*7064*/	sint32				ldon_points_mir;
/*7068*/	sint32				ldon_points_mmc;
/*7072*/	sint32				ldon_points_ruj;
/*7076*/	sint32				ldon_points_tak;
/*7080*/	sint32				ldon_points_available;
/*7084*/	uint8				unknown5940[112];
/*7196*/	uint32				tribute_time_remaining;	//in miliseconds
/*7200*/	uint32				showhelm;
/*7204*/	uint32				career_tribute_points;
/*7208*/	uint32				unknown6056;
/*7212*/	uint32				tribute_points;
/*7216*/	uint32				unknown6064;
/*7220*/	uint32				tribute_active;		//1=active
/*7224*/	Tribute_Struct		tributes[MAX_PLAYER_TRIBUTES];
/*7264*/	Disciplines_Struct	disciplines;			//fathernitwit: 10-06-04
/*7664*/	char				unknown7464[240];
/*7904*/	uint32				endurance;
/*7908*/	uint32				group_leadership_exp;	//0-1000
/*7912*/	uint32				raid_leadership_exp;	//0-2000
/*7916*/	uint32				group_leadership_points;
/*7920*/	uint32				raid_leadership_points;
/*7924*/	LeadershipAA_Struct	leader_abilities;
/*8052*/	uint8				unknown8052[132];
/*8184*/	uint32				air_remaining;
/*8188*/	uint8				unknown8188[4608]; //probably raid members 4608 = 64 * 72
/*12796*/	uint32				aapoints_spent;
/*12800*/	uint32				expAA;
/*12804*/	uint32				aapoints;	//avaliable, unspent
/*12808*/	uint8				unknown12808[36];
/*12844*/	Bandolier_Struct	bandoliers[MAX_PLAYER_BANDOLIER];
/*14124*/	uint8				unknown14124[5116];
/*19240*/	uint32				timeentitledonaccount;
/*19244*/	PotionBelt_Struct	potionbelt;	//there should be 3 more of these
/*19532*/	uint8				unknown19532[8];
/*19540*/	uint32				currentRadCrystals; 	// Current count of radiant crystals
/*19544*/	uint32				careerRadCrystals; // Total count of radiant crystals ever
/*19548*/	uint32				currentEbonCrystals;		// Current count of ebon crystals
/*19552*/	uint32				careerEbonCrystals;	// Total count of ebon crystals ever
/*19556*/	uint8				groupAutoconsent;   // 0=off, 1=on
/*19557*/	uint8				raidAutoconsent;    // 0=off, 1=on
/*19558*/	uint8				guildAutoconsent;   // 0=off, 1=on
/*19559*/	uint8				unknown19559[5];    // ***Placeholder (6/29/2005)
/*19564*/	uint32				RestTimer;
/*19568*/
};




/*
** Client Target Struct
** Length: 2 Bytes
** OpCode: 6221
*/
struct ClientTarget_Struct {
/*000*/	int32	new_target;			// Target ID
};

/*
** Target Rejection Struct
** Length: 12 Bytes
** OpCode: OP_TargetReject
*/
struct TargetReject_Struct {
/*00*/	uint8	unknown00[12];
};

struct PetCommand_Struct {
/*000*/ int32	command;
/*004*/ int32	unknown;
};

/*
** Delete Spawn
** Length: 4 Bytes
** OpCode: OP_DeleteSpawn
*/
struct DeleteSpawn_Struct
{
/*00*/ int32 spawn_id;             // Spawn ID to delete
/*04*/
};

/*
** Channel Message received or sent
** Length: 144 Bytes + Variable Length + 1
** OpCode: OP_ChannelMessage
**
*/
struct ChannelMessage_Struct
{
/*000*/	char	targetname[64];		// Tell recipient
/*064*/	char	sender[64];			// The senders name (len might be wrong)
/*128*/	int32	language;			// Language
/*132*/	int32	chan_num;			// Channel
/*136*/	int32	cm_unknown4[2];		// ***Placeholder
/*144*/	int32	skill_in_language;	// The players skill in this language? might be wrong
/*148*/	char	*message;			// Variable length message
};

/*
** Special Message
** Length: 4 Bytes + Variable Text Length + 1
** OpCode: OP_SpecialMesg
**
*/
/*
	Theres something wrong with this... example live packet:
Server->Client: [ Opcode: OP_SpecialMesg (0x0fab) Size: 244 ]
   0: 01 02 00 0A 00 00 00 09 - 05 00 00 42 61 72 73 74  | ...........Barst
  16: 72 65 20 53 6F 6E 67 77 - 65 61 76 65 72 00 7C F9  | re Songweaver.|.
  32: FF FF 84 FF FF FF 03 00 - 00 00 47 72 65 65 74 69  | ..........Greeti

*/
struct SpecialMesg_Struct
{
/*00*/	char	header[3];				// 04 04 00 <-- for #emote style msg
/*03*/	uint32	msg_type;				// Color of text (see MT_*** below)
/*07*/	uint32	target_spawn_id;		// Who is it being said to?
/*11*/	char	sayer[1];				// Who is the source of the info
/*12*/	uint8	unknown12[12];
/*24*/	char	message[1];				// What is being said?
};

/*
** When somebody changes what they're wearing
**      or give a pet a weapon (model changes)
** Length: 16 Bytes
** Opcode: 9220
*/
struct WearChange_Struct{
/*000*/ int16 spawn_id;
/*002*/ int16 material;
/*004*/ Color_Struct color;
/*009*/ int8 wear_slot_id;
};

/*
** Type:   Bind Wound Structure
** Length: 8 Bytes
*/
//Fixed for 7-14-04 patch
struct BindWound_Struct
{
/*002*/	int16	to;			// TargetID
/*004*/	int16	unknown2;		// ***Placeholder
/*006*/	int16	type;
/*008*/	int16	unknown6;
};


/*
** Type:   Zone Change Request (before hand)
** Length: 88 bytes
** OpCode: a320
*/

struct ZoneChange_Struct {
/*000*/	char	char_name[64];     // Character Name
/*064*/	uint16	zoneID;
/*066*/	uint16	instanceID;
/*068*/	float	y;
/*072*/	float	x;
/*076*/	float	z;
/*080*/	int32	zone_reason;	//0x0A == death, I think
/*084*/	sint32	success;		// =0 client->server, =1 server->client, -X=specific error
/*088*/
};

// Whatever you send to the client in RequestClientZoneChange_Struct.type, the client will send back
// to the server in ZoneChange_Struct.zone_reason. My guess is this is a memo field of sorts.
// WildcardX 27 January 2008

struct RequestClientZoneChange_Struct {
/*00*/	uint16	zone_id;
/*02*/	uint16	instance_id;
/*04*/	float	y;
/*08*/	float	x;
/*12*/	float	z;
/*16*/	float	heading;
/*20*/	uint32	type;	//unknown... values
};

struct Animation_Struct {
/*00*/	int16 spawnid;
/*02*/	int8 action;
/*03*/	int8 value;
/*04*/
};

// solar: this is what causes the caster to animate and the target to
// get the particle effects around them when a spell is cast
// also causes a buff icon
struct Action_Struct
{
 /* 00 */	int16 target;	// id of target
 /* 02 */	int16 source;	// id of caster
 /* 04 */	uint16 level; // level of caster
 /* 06 */	uint16 instrument_mod;
 /* 08 */	uint32 unknown08;
 /* 12 */	uint16 unknown16;
// some kind of sequence that's the same in both actions
// as well as the combat damage, to tie em together?
 /* 14 */	int32 sequence;
 /* 18 */	uint32 unknown18;
 /* 22 */	int8 type;		// 231 (0xE7) for spells
 /* 23 */	uint32 unknown23;
 /* 27 */	int16 spell;	// spell id being cast
 /* 29 */	int8 unknown29;
// this field seems to be some sort of success flag, if it's 4
 /* 30 */	int8 buff_unknown;	// if this is 4, a buff icon is made
 /* 31 */
};

// solar: this is what prints the You have been struck. and the regular
// melee messages like You try to pierce, etc.  It's basically the melee
// and spell damage message
struct CombatDamage_Struct
{
/* 00 */	int16	target;
/* 02 */	int16	source;
/* 04 */	int8	type; //slashing, etc.  231 (0xE7) for spells
/* 05 */	int16	spellid;
/* 07 */	int32	damage;
/* 11 */	int32 unknown11;
/* 15 */	int32 sequence;	// see above notes in Action_Struct
/* 19 */	int32	unknown19;
/* 23 */
};

/*
** Consider Struct
*/
struct Consider_Struct{
/*000*/ uint32	playerid;               // PlayerID
/*004*/ uint32	targetid;               // TargetID
/*008*/ int32	faction;                // Faction
/*0012*/ int32	level;                  // Level
/*016*/ sint32	cur_hp;                  // Current Hitpoints
/*020*/ sint32	max_hp;                  // Maximum Hitpoints
/*024*/ int8 pvpcon;     // Pvp con flag 0/1
/*025*/ int8	unknown3[3];
};

/*
** Spawn Death Blow
** Length: 32 Bytes
** OpCode: 0114
*/
struct Death_Struct
{
/*000*/	int32	spawn_id;
/*004*/	int32	killer_id;
/*008*/	int32	corpseid;	// was corpseid
/*012*/	int32	bindzoneid;
/*016*/	int32	spell_id;
/*020*/  int32	attack_skill;
/*024*/	int32	damage;
/*028*/	int32	unknown028;
};

struct BecomeCorpse_Struct {
	int32	spawn_id;
	float y;
	float x;
	float z;
};

/*
** Spawn position update
**	Struct sent from server->client to update position of
**	another spawn's position update in zone (whether NPC or PC)
**
*/
struct PlayerPositionUpdateServer_Struct
{
/*0000*/ uint16		spawn_id;
/*0002*/ sint32		delta_heading:10,  // change in heading
         			x_pos:19,             // x coord
         			padding0002:3;    // ***Placeholder
/*0006*/ sint32		y_pos:19,             // y coord
         			animation:10,     // animation
         			padding0006:3;    // ***Placeholder
/*0010*/ sint32		z_pos:19,             // z coord
         			delta_y:13;        // change in y
/*0014*/ sint32		delta_x:13,        // change in x
        			heading:12,       // heading
         			padding0014:7;    // ***Placeholder
/*0018*/ sint32		delta_z:13,        // change in z
         			padding0018:19;   // ***Placeholder
/*0022*/
};

/*
** Player position update
**	Struct sent from client->server to update
**	player position on server
**
*/
struct PlayerPositionUpdateClient_Struct
{
/*0000*/ uint16	spawn_id;
/*0022*/ uint16	sequence;	//increments one each packet
/*0004*/ float y_pos;                 // y coord
/*0008*/ float delta_z;            // Change in z
/*0016*/ float delta_x;            // Change in x
/*0012*/ float delta_y;            // Change in y
/*0020*/ sint32 animation:10,     // animation
         		delta_heading:10,  // change in heading
         		padding0020:12;   // ***Placeholder (mostly 1)
/*0024*/ float x_pos;                 // x coord
/*0028*/ float z_pos;                 // z coord
/*0034*/ uint16 heading:12,     // Directional heading
         		padding0004:4;  // ***Placeholder
/*0032*/ uint8 unknown0006[2];  // ***Placeholder
/*0036*/
};

/*
** Spawn HP Update
** Length: 10 Bytes
** OpCode: OP_HPUpdate
*/
struct SpawnHPUpdate_Struct
{
/*00*/ uint32	cur_hp;               // Id of spawn to update
/*04*/ sint32	max_hp;                 // Maximum hp of spawn
/*08*/ sint16	spawn_id;                 // Current hp of spawn
/*10*/
};
struct SpawnHPUpdate_Struct2
{
/*01*/ sint16	spawn_id;
/*00*/ int8	hp;
};
/*
** Stamina
** Length: 8 Bytes
** OpCode: 5721
*/
struct Stamina_Struct {
/*00*/ int32 food;                     // (low more hungry 127-0)
/*02*/ int32 water;                    // (low more thirsty 127-0)
};

/*
** Level Update
** Length: 12 Bytes
*/
struct LevelUpdate_Struct
{
/*00*/ uint32 level;                  // New level
/*04*/ uint32 level_old;              // Old level
/*08*/ uint32 exp;                    // Current Experience
};

/*
** Experience Update
** Length: 14 Bytes
** OpCode: 9921
*/
struct ExpUpdate_Struct
{
/*0000*/ uint32 exp;                    // Current experience ratio from 0 to 330
/*0004*/ uint32 aaxp; // @BP ??
};

/*
** Item Packet Struct - Works on a variety of opcodes
** Packet Types: See ItemPacketType enum
**
*/
enum ItemPacketType
{
	ItemPacketViewLink			= 0x00,
	ItemPacketTradeView			= 0x65,
	ItemPacketLoot				= 0x66,
	ItemPacketTrade				= 0x67,
	ItemPacketCharInventory		= 0x69,
	ItemPacketSummonItem		= 0x6A,
	ItemPacketTributeItem		= 0x6C,
	ItemPacketMerchant			= 0x64,
	ItemPacketWorldContainer	= 0x6B
};
struct ItemPacket_Struct
{
/*00*/	ItemPacketType	PacketType;
/*04*/	char			SerializedItem[1];
/*xx*/
};

struct BulkItemPacket_Struct
{
/*00*/	char			*SerializedItem;
/*xx*/
};

struct Consume_Struct
{
/*0000*/ int32 slot;
/*0004*/ int32 auto_consumed; // 0xffffffff when auto eating e7030000 when right click
/*0008*/ int8  c_unknown1[4];
/*0012*/ int8  type; // 0x01=Food 0x02=Water
/*0013*/ int8  unknown13[3];
};


struct MoveItem_Struct
{
/*0000*/ uint32 from_slot;
/*0004*/ uint32 to_slot;
/*0008*/ uint32 number_in_stack;
};

//
// from_slot/to_slot
// -1 - destroy
//  0 - cursor
//  1 - inventory
//  2 - bank
//  3 - trade
//  4 - shared bank
//
// cointype
// 0 - copeer
// 1 - silver
// 2 - gold
// 3 - platinum
//
static const uint32 COINTYPE_PP = 3;
static const uint32 COINTYPE_GP = 2;
static const uint32 COINTYPE_SP = 1;
static const uint32 COINTYPE_CP = 0;

struct MoveCoin_Struct
{
		 sint32 from_slot;
		 sint32 to_slot;
		 uint32 cointype1;
		 uint32 cointype2;
		 sint32	amount;
};
struct TradeCoin_Struct{
	int32	trader;
	int8	slot;
	int16	unknown5;
	int8	unknown7;
	int32	amount;
};
struct TradeMoneyUpdate_Struct{
	int32	trader;
	int32	type;
	int32	amount;
};
/*
** Surname struct
** Size: 100 bytes
*/
struct Surname_Struct
{
/*0000*/	char name[64];
/*0064*/	uint32 unknown0064;
/*0068*/	char lastname[32];
/*0100*/
};

struct GuildsListEntry_Struct {
	char name[64];
};

static const uint32 MAX_NUMBER_GUILDS = 1500;
struct GuildsList_Struct {
	int8 head[64]; // First on guild list seems to be empty...
	GuildsListEntry_Struct Guilds[MAX_NUMBER_GUILDS];
};

struct GuildUpdate_Struct {
	int32	guildID;
	GuildsListEntry_Struct entry;
};

/*
** Money Loot
** Length: 22 Bytes
** OpCode: 5020
*/
struct moneyOnCorpseStruct {
/*0000*/ uint8	response;		// 0 = someone else is, 1 = OK, 2 = not at this time
/*0001*/ uint8	unknown1;		// = 0x5a
/*0002*/ uint8	unknown2;		// = 0x40
/*0003*/ uint8	unknown3;		// = 0
/*0004*/ uint32	platinum;		// Platinum Pieces
/*0008*/ uint32	gold;			// Gold Pieces

/*0012*/ uint32	silver;			// Silver Pieces
/*0016*/ uint32	copper;			// Copper Pieces
};

//opcode = 0x5220
// size 292


struct LootingItem_Struct {
/*000*/	int32	lootee;
/*002*/	int32	looter;
/*004*/	int16	slot_id;
/*006*/	int8	unknown3[2];
/*008*/	int32	auto_loot;
};

struct GuildManageStatus_Struct{
	int32	guild_id;
	int32	oldrank;
	int32	newrank;
	char	name[64];
};
// Guild invite, remove
struct GuildJoin_Struct{
/*000*/	int32	guild_id;
/*004*/	int32	unknown04;
/*008*/	int32	level;
/*012*/	int32	class_;
/*016*/	int32	rank;//0 member, 1 officer, 2 leader
/*020*/	int32	zoneid;
/*024*/	int32	unknown24;
/*028*/	char	name[64];
/*092*/
};
struct GuildInviteAccept_Struct {
	char inviter[64];
	char newmember[64];
	int32 response;
	int32 guildeqid;
};
struct GuildManageRemove_Struct {
	int32 guildeqid;
	char member[64];
};
struct GuildCommand_Struct {
	char othername[64];
	char myname[64];
	int16 guildeqid;
	int8 unknown[2]; // for guildinvite all 0's, for remove 0=0x56, 2=0x02
	int32 officer;
};

// 4244 bytes. Is not really an 'OnLevelMessage', it causes a popup box to display in the client
// Text looks like HTML.
struct OnLevelMessage_Struct {
/*0000*/       char    Title[128];
/*0128*/       char    Text[4096];
/*4224*/       uint32  Buttons;
/*4228*/       uint32  unknown4228;
/*4232*/       uint32  PopupID;
/*4236*/       uint32  unknown4236;
/*4240*/       uint32  unknown4240;
};

struct PopupResponse_Struct {
/*0000*/	int32	unknown0000;
/*0004*/	int32	popupid;
};

struct GuildManageBanker_Struct {
	uint32 unknown0;
	char myname[64];
	char member[64];
	uint32	enabled;
};

// Opcode OP_GMZoneRequest
// Size = 88 bytes
struct GMZoneRequest_Struct {
/*0000*/	char	charname[64];
/*0064*/	int32	zone_id;
/*0068*/	float	x;
/*0072*/	float	y;
/*0076*/	float	z;
/*0080*/	float	heading;
/*0084*/	int32	success;		// 0 if command failed, 1 if succeeded?
/*0088*/
//	/*072*/	sint8	success;		// =0 client->server, =1 server->client, -X=specific error
//	/*073*/	int8	unknown0073[3]; // =0 ok, =ffffff error
};

struct GMSummon_Struct {
/*  0*/	char    charname[64];
/* 30*/	char    gmname[64];
/* 60*/ int32	success;
/* 61*/	int32	zoneID;
/*92*/	float  y;
/*96*/	float  x;
/*100*/ float  z;
/*104*/	int32 unknown2; // E0 E0 56 00
};

struct GMGoto_Struct { // x,y is swapped as compared to summon and makes sense as own packet
/*  0*/	char    charname[64];

/* 64*/	char    gmname[64];
/* 128*/ int32	success;
/* 132*/	int32	zoneID;

/*136*/	sint32  y;
/*140*/	sint32  x;
/*144*/ sint32  z;
/*148*/	int32 unknown2; // E0 E0 56 00
};

struct GMLastName_Struct {
	char name[64];
	char gmname[64];
	char lastname[64];
	int16 unknown[4];	// 0x00, 0x00
					    // 0x01, 0x00 = Update the clients
};

//Combat Abilities
struct CombatAbility_Struct {
	int32 m_target;		//the ID of the target mob
	int32 m_atk;
	int32 m_skill;
};

struct DeleteItem_Struct {
/*0000*/ uint32 from_slot;
/*0004*/ uint32 to_slot;
/*0008*/ uint32 number_in_stack;
};

//Instill Doubt
struct Instill_Doubt_Struct {
	int8 i_id;
	int8 ia_unknown;
	int8 ib_unknown;
	int8 ic_unknown;
	int8 i_atk;

	int8 id_unknown;
	int8 ie_unknown;
	int8 if_unknown;
	int8 i_type;
	int8 ig_unknown;
	int8 ih_unknown;
	int8 ii_unknown;
};

struct GiveItem_Struct {
	uint16 to_entity;
	sint16 to_equipSlot;
	uint16 from_entity;
	sint16 from_equipSlot;
};

struct RandomReq_Struct {
	int32 low;
	int32 high;
};

/* solar: 9/23/03 reply to /random command; struct from Zaphod */
struct RandomReply_Struct {
/* 00 */	int32 low;
/* 04 */	int32 high;
/* 08 */	int32 result;
/* 12 */	char name[64];
/* 76 */
};

struct LFG_Struct {
/*
Wrong size on OP_LFG. Got: 80, Expected: 68
   0: 00 00 00 00 01 00 00 00 - 00 00 00 00 64 00 00 00  | ............d...
  16: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
  32: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
  48: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
  64: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
Wrong size on OP_LFG. Got: 80, Expected: 68
   0: 00 00 00 00 01 00 00 00 - 3F 00 00 00 41 00 00 00  | ........?...A...
  16: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
  32: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
  48: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
  64: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
Wrong size on OP_LFG. Got: 80, Expected: 68
   0: 00 00 00 00 01 00 00 00 - 3F 00 00 00 41 00 00 00  | ........?...A...
  16: 46 72 75 62 20 66 72 75 - 62 20 66 72 75 62 00 00  | Frub frub frub..
  32: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
  48: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
  64: 00 00 00 00 00 00 00 00 - 00 00 00 00 00 00 00 00  | ................
*/
/*000*/	uint32 unknown000;
/*004*/	uint8 value; // 0x00 = off 0x01 = on
/*005*/	uint8  MatchFilter;
/*006*/	uint16 Unknown006;
/*008*/	uint32 FromLevel;
/*012*/	uint32 ToLevel;
/*016*/	char Comments[64];
};

struct LFGGetMatchesRequest_Struct {
/*000*/	uint32	Unknown000;
/*004*/	uint32	FromLevel;
/*008*/	uint32	ToLevel;
/*012*/	uint32	Classes;
};

enum {	LFPOff = 0,
	LFPOn = 1,
	LFPMemberUpdate = 255 // Internal use, not sent by client
};

struct LFP_Struct {
/*000*/	uint32	Unknown000;
/*004*/	uint8	Action;
/*005*/	uint8	MatchFilter;
/*006*/	uint16	Unknown006;
/*008*/	uint32	FromLevel;
/*012*/	uint32	ToLevel;
/*016*/	uint32	Classes;
/*020*/	char	Comments[64];
};

struct LFPGetMatchesRequest_Struct {
/*000*/	uint32	Unknown000;
/*004*/	uint32	FromLevel;
/*008*/	uint32	ToLevel;
};

/*
** LFG_Appearance_Struct
** Packet sent to clients to notify when someone in zone toggles LFG flag
** Size: 8 bytes
** Used in: OP_LFGAppearance
**
*/
struct LFG_Appearance_Struct
{
/*0000*/ uint32 spawn_id;		// ID of the client
/*0004*/ uint8 lfg;				// 1=LFG, 0=Not LFG
/*0005*/ char unknown0005[3];	//
/*0008*/
};


// EverQuest Time Information:
// 72 minutes per EQ Day
// 3 minutes per EQ Hour
// 6 seconds per EQ Tick (2 minutes EQ Time)
// 3 seconds per EQ Minute

struct TimeOfDay_Struct {
	int8	hour;
	int8	minute;
	int8	day;
	int8	month;
	int32	year;
};

// Darvik: shopkeeper structs
struct Merchant_Click_Struct {
/*000*/ int32	npcid;			// Merchant NPC's entity id
/*004*/ int32	playerid;
/*008*/ int32	command;		//1=open, 0=cancel/close
/*012*/ float	rate;			//cost multiplier, dosent work anymore
};
/*
Unknowns:
0 is e7 from 01 to // MAYBE SLOT IN PURCHASE
1 is 03
2 is 00
3 is 00
4 is ??
5 is ??
6 is 00 from a0 to
7 is 00 from 3f to */
/*
0 is F6 to 01
1 is CE CE
4A 4A
00 00
00 E0
00 CB
00 90
00 3F
*/



struct Merchant_Sell_Struct {
/*000*/	int32	npcid;			// Merchant NPC's entity id
/*004*/	int32	playerid;		// Player's entity id
/*008*/	int32	itemslot;
		int32	unknown12;
/*016*/	int8	quantity;		// Already sold
/*017*/ int8	Unknown016[3];
/*020*/ int32	price;
};
struct Merchant_Purchase_Struct {
/*000*/	int32	npcid;			// Merchant NPC's entity id
/*004*/	int32	itemslot;		// Player's entity id
/*008*/	int32	quantity;
/*012*/	int32	price;
};
struct Merchant_DelItem_Struct{
/*000*/	int32	npcid;			// Merchant NPC's entity id
/*004*/	int32	playerid;		// Player's entity id
/*008*/	int32	itemslot;
/*012*/	uint32	unknown012;
};
struct Adventure_Purchase_Struct {
/*000*/	int32	some_flag;	//set to 1 generally...
/*000*/	int32	npcid;
/*004*/	int32	itemid;
/*008*/	int32	variable;
};

struct Adventure_Sell_Struct {
/*000*/	int32	unknown000;	//0x01
/*004*/	int32	npcid;
/*008*/	int32	slot;
/*012*/	int32	charges;
/*016*/	int32	sell_price;
};


struct AdventurePoints_Update_Struct {
/*000*/	uint32	ldon_available_points;		// Total available points
/*004*/	uint8	unknown004[36];
/*040*/	uint8	unknown040[16];
/*056*/	uint32	ldon_guk_points;		// Earned Deepest Guk points
/*060*/	uint8	unknown060[16];
/*076*/	uint32	ldon_mirugal_points;		// Earned Mirugal' Menagerie points
/*080*/	uint8	unknown080[16];
/*096*/	uint32	ldon_mistmoore_points;		// Earned Mismoore Catacombs Points
/*100*/	uint8	unknown100[16];
/*116*/	uint32	ldon_rujarkian_points;		// Earned Rujarkian Hills points
/*120*/	uint8	unknown120[16];
/*136*/	uint32	ldon_takish_points;		// Earned Takish points
/*140*/
};

struct AdventureFinish_Struct{
/*000*/ uint32 win_lose;//Cofruben: 00 is a lose,01 is win.
/*004*/ uint32 points;
/*008*/
};
//OP_AdventureRequest
struct AdventureRequest_Struct{
/*000*/	int32 risk;//1 normal,2 hard.
/*004*/	int32 entity_id;
/*008*/	int32 type;
/*012*/
};
struct AdventureRequestResponse_Struct{
/*0000*/ int32 unknown000;
/*0004*/ char text[2048];
/*2052*/ int32 timetoenter;
/*2056*/ int32 timeleft;
/*2060*/ int32 risk;
/*2064*/ float x;
/*2068*/ float y;
/*2072*/ float z;
/*2076*/ int32 showcompass;
/*2080*/ int32 unknown2080;
/*2084*/
};

struct AdventureCountUpdate_Struct
{
/*000*/ int32 current;
/*004*/	int32 total;
/*008*/
};

struct AdventureStatsColumn_Struct
{
/*000*/ int32 total;
/*004*/	int32 guk;
/*008*/	int32 mir;
/*012*/	int32 mmc;
/*016*/	int32 ruj;
/*020*/	int32 tak;
/*024*/
};

struct AdventureStats_Struct
{
/*000*/ AdventureStatsColumn_Struct success;
/*024*/ AdventureStatsColumn_Struct failure;
/*048*/	AdventureStatsColumn_Struct rank;
/*072*/	AdventureStatsColumn_Struct rank2;
/*096*/
};

//this is mostly right but something is off that causes the client to crash sometimes
//I don't really care enough about the feature to work on it anymore though.
struct AdventureLeaderboardEntry_Struct
{
/*004*/ char name[64];
/*008*/ int32 success;
/*012*/ int32 failure;
/*016*/
};

struct AdventureLeaderboard_Struct
{
/*000*/ int32 unknown000;
/*004*/ int32 unknown004;
/*008*/ int32 success;
/*012*/ int32 failure;
/*016*/ int32 our_rank;
/*020*/
};

/*struct Item_Shop_Struct {
	uint16 merchantid;
	int8 itemtype;
	Item_Struct item;
	int8 iss_unknown001[6];
};*/

struct Illusion_Struct {  //size: 256 - SoF
/*000*/	uint32	spawnid;
/*004*/	char charname[64];		//
/*068*/	uint16	race;			// race
/*070*/	char	unknown006[2];
/*072*/	uint8	gender;
/*073*/	uint8	texture;
/*074*/	uint8	unknown008;		//
/*075*/	uint8	unknown009;		//
/*076*/	uint8	helmtexture;	// Verified
/*077*/	uint8	unknown010;		//
/*078*/	uint8	unknown011;		//
/*079*/	uint8	unknown012;		//
/*080*/	uint32	face;			// Verified
/*084*/	uint8	hairstyle;		// Verified
/*085*/	uint8	haircolor;		// Verified
/*085*/	uint8	beard;			// Verified
/*087*/	uint8	beardcolor;		// Verified
/*088*/	uint32	drakkin_heritage;	// Temp Placeholder until field is identified in SoF
/*092*/	uint32	drakkin_tattoo;		// Temp Placeholder until field is identified in SoF
/*096*/	uint32	drakkin_details;	// Temp Placeholder until field is identified in SoF
/*100*/	uint32	armor_tint;		// Temp Placeholder until field is identified in SoF
/*104*/	uint8	eyecolor1;		// Temp Placeholder until field is identified in SoF
/*105*/	uint8	eyecolor2;		// Temp Placeholder until field is identified in SoF
/*106*/	uint8	unknown106[150];	//was uint8	unknown021[168];
/*256*/
};

struct Illusion_Struct_Old {
/*000*/	uint32	spawnid;
		char charname[64];
/**/	uint16	race;
/**/	char	unknown006[2];
/**/	uint8	gender;
/**/	uint8	texture;
/**/	uint8	helmtexture;
/**/	uint8	unknown011;
/**/	uint32	face;
/**/	char	unknown020[88];
/**/
};

struct ZonePoint_Entry {
/*0000*/	int32	iterator;
/*0004*/	float	y;
/*0008*/	float	x;
/*0012*/	float	z;
/*0016*/	float	heading;
/*0020*/	int16	zoneid;
/*0022*/	int16	zoneinstance; // LDoN instance
};

struct ZonePoints {
/*0000*/	int32	count;
/*0004*/	struct	ZonePoint_Entry *zpe; // Always add one extra to the end after all zonepoints
};

struct SkillUpdate_Struct {
/*00*/	uint32 skillId;
/*04*/	uint32 value;
/*08*/
};

struct ZoneUnavail_Struct {
	//This actually varies, but...
	char zonename[16];
	sint16 unknown[4];
};

struct GroupGeneric_Struct {
	char name1[64];
	char name2[64];
};

struct GroupCancel_Struct {
	char	name1[64];
	char	name2[64];
	uint8	toggle;
};

struct GroupUpdate_Struct {
/*0000*/	int32	action;
/*0004*/	char	yourname[64];
/*0068*/	char	membername[5][64];
/*0388*/	char	leadersname[64];
};

struct GroupUpdate2_Struct {
/*0000*/	int32	action;
/*0004*/	char	yourname[64];
/*0068*/	char	membername[5][64];
/*0388*/	char	leadersname[64];
/*0452*/	GroupLeadershipAA_Struct leader_aas;
/*0580*/	int8	unknown580[196];
/*0766*/	uint32	NPCMarkerID;	// EntityID of player delegated MarkNPC ability
/*0780*/	int8	unknown780[56];
/*0836*/
};
struct GroupJoin_Struct {
/*0000*/	int32	action;
/*0004*/	char	yourname[64];
/*0068*/	char	membername[64];
/*0132*/	GroupLeadershipAA_Struct leader_aas;
/*0196*/	int8	unknown196[196];
/*0392*/	uint32	NPCMarkerID;	// EntityID of player delegated MarkNPC ability
/*0396*/	int8	unknown396[56];
/*0452*/
};

struct GroupFollow_Struct { // SoF Follow Struct
/*0000*/	char	name1[64];	// inviter
/*0064*/	char	name2[64];	// invitee
/*0128*/	int32	unknown0128;
/*0132*/
};

struct FaceChange_Struct {
/*000*/	int8	haircolor;
/*001*/	int8	beardcolor;
/*002*/	int8	eyecolor1; // the eyecolors always seem to be the same, maybe left and right eye?
/*003*/	int8	eyecolor2;
/*004*/	int8	hairstyle;
/*005*/	int8	beard;
/*006*/	int8	face;
/*007*/ int32	drakkin_heritage;
/*011*/ int32	drakkin_tattoo;
/*015*/ int32	drakkin_details;
//there are only 10 faces for barbs changing woad just
//increase the face value by ten so if there were 8 woad
//designs then there would be 80 barb faces
};

/*
** Trade request from one client to another
** Used to initiate a trade
** Size: 8 bytes
** Used in: OP_TradeRequest
*/
struct TradeRequest_Struct {
/*00*/	uint32 to_mob_id;
/*04*/	uint32 from_mob_id;
/*08*/
};

struct TradeAccept_Struct {
/*00*/	uint32 from_mob_id;
/*04*/	uint32 unknown4;		//seems to be garbage
/*08*/
};

/*
** Cancel Trade struct
** Sent when a player cancels a trade
** Size: 8 bytes
** Used In: OP_CancelTrade
**
*/
struct CancelTrade_Struct {
/*00*/	int32 fromid;
/*04*/	int32 action;
/*08*/
};

struct PetitionUpdate_Struct {
	int32 petnumber;    // Petition Number
	int32 color;		// 0x00 = green, 0x01 = yellow, 0x02 = red
	int32 status;
	time_t senttime;    // 4 has to be 0x1F
	char accountid[32];
	char gmsenttoo[64];
	sint32 quetotal;
	char charname[64];
};

struct Petition_Struct {
	int32 petnumber;
	int32 urgency;
	char accountid[32];
	char lastgm[32];
	int32	zone;
	//char zone[32];
	char charname[64];
	int32 charlevel;
	int32 charclass;
	int32 charrace;
	int32 unknown;
	//time_t senttime; // Time?
	int32 checkouts;
	int32 unavail;
	//int8 unknown5[4];
	time_t senttime;
	int32 unknown2;
	char petitiontext[1024];
	char gmtext[1024];
};


struct Who_All_Struct { // 76 length total
/*000*/	char	whom[64];
/*064*/	int32	wrace;		// FF FF = no race

/*068*/	int32	wclass;		// FF FF = no class
/*072*/	int32	lvllow;		// FF FF = no numbers
/*076*/	int32	lvlhigh;	// FF FF = no numbers
/*080*/	int32	gmlookup;	// FF FF = not doing /who all gm
/*084*/	int32	guildid;
/*088*/	int8	unknown076[64];
/*152*/	int32	type;		// New for SoF. 0 = /who 3 = /who all
/*156*/
};

struct Stun_Struct { // 4 bytes total
	int32 duration; // Duration of stun
};

struct AugmentItem_Struct {
/*00*/	sint16	container_slot;
/*02*/	char	unknown02[2];
/*04*/	sint32	augment_slot;
/*08*/
};

// OP_Emote
struct Emote_Struct {
/*0000*/	int32 unknown01;
/*0004*/	char message[1024];
/*1028*/
};

// Inspect
struct Inspect_Struct {
	int16 TargetID;
	int16 PlayerID;
};
//OP_InspectAnswer
struct InspectResponse_Struct{//Cofruben:need to send two of this for the inspect response.
/*000*/	int32 TargetID;
/*004*/	int32 playerid;
/*008*/	char itemnames[21][64];
/*1352*/char unknown_zero[64];//fill with zero's.
/*1416*/int32 itemicons[21];
/*1500*/int32 unknown_zero2;
/*1504*/char text[288];
};

//OP_SetDataRate
struct SetDataRate_Struct {
	float newdatarate;
};

//OP_SetServerFilter
struct SetServerFilter_Struct {
	uint32 filters[29];		//see enum eqFilterType
};

//Op_SetServerFilterAck
struct SetServerFilterAck_Struct {
	int8 blank[8];
};
struct IncreaseStat_Struct{
	/*0000*/	int8	unknown0;
	/*0001*/	int8	str;
	/*0002*/	int8	sta;
	/*0003*/	int8	agi;
	/*0004*/	int8	dex;
	/*0005*/	int8	int_;
	/*0006*/	int8	wis;
	/*0007*/	int8	cha;
	/*0008*/	int8	fire;
	/*0009*/	int8	cold;
	/*0010*/	int8	magic;
	/*0011*/	int8	poison;
	/*0012*/	int8	disease;
	/*0013*/	char	unknown13[116];
	/*0129*/	int8	str2;
	/*0130*/	int8	sta2;
	/*0131*/	int8	agi2;
	/*0132*/	int8	dex2;
	/*0133*/	int8	int_2;
	/*0134*/	int8	wis2;
	/*0135*/	int8	cha2;
	/*0136*/	int8	fire2;
	/*0137*/	int8	cold2;
	/*0138*/	int8	magic2;
	/*0139*/	int8	poison2;
	/*0140*/	int8	disease2;
};

struct GMName_Struct {
	char oldname[64];
	char gmname[64];
	char newname[64];
	int8 badname;
	int8 unknown[3];
};

struct GMDelCorpse_Struct {
	char corpsename[64];
	char gmname[64];
	int8 unknown;
};

struct GMKick_Struct {
	char name[64];
	char gmname[64];
	int8 unknown;
};


struct GMKill_Struct {
	char name[64];
	char gmname[64];
	int8 unknown;
};


struct GMEmoteZone_Struct {
	char text[512];
};

// This is where the Text is sent to the client.
// Use ` as a newline character in the text.
// Variable length.
struct BookText_Struct {
	uint8 window;	// where to display the text (0xFF means new window)
	uint8 type;             //type: 0=scroll, 1=book, 2=item info.. prolly others.
	uint32 invslot;	// Only used in SoF and later clients.
	char booktext[1]; // Variable Length
};
// This is the request to read a book.
// This is just a "text file" on the server
// or in our case, the 'name' column in our books table.
struct BookRequest_Struct {
	uint8 window;	// where to display the text (0xFF means new window)
	uint8 type;             //type: 0=scroll, 1=book, 2=item info.. prolly others.
	uint32 invslot;	// Only used in Sof and later clients;
	char txtfile[20];
};

/*
** Object/Ground Spawn struct
** Used for Forges, Ovens, ground spawns, items dropped to ground, etc
** Size: 92 bytes
** OpCodes: OP_CreateObject
** Last Updated: Oct-17-2003
**
*/
struct Object_Struct {
/*00*/	uint32	linked_list_addr[2];// <Zaphod> They are, get this, prev and next, ala linked list
/*08*/	uint16	unknown008[2];		//
/*12*/	uint32	drop_id;			// Unique object id for zone
/*16*/	uint16	zone_id;			// Redudant, but: Zone the object appears in
/*18*/	uint16	zone_instance;		//
/*20*/	uint32	unknown020;			//
/*24*/	uint32	unknown024;			//
/*28*/	float	heading;			// heading
/*32*/	float	z;					// z coord
/*36*/	float	x;					// x coord
/*40*/	float	y;					// y coord
/*44*/	char	object_name[20];	// Name of object, usually something like IT63_ACTORDEF
/*64*/	float	unknown064;			// seems like coords, not always valid, all 0 on most world objects
/*68*/	float	unknown068;			// seems like coords, not always valid, all 0 on most world objects
/*72*/	float	unknown072;			// seems like coords, not always valid, all 0 on most world objects
/*76*/	uint32	unknown076;			//
/*80*/	uint32	object_type;		// Type of object, not directly translated to OP_OpenObject
/*84*/	uint32	unknown084;			//set to 0xFF
/*88*/	uint32	spawn_id;			// Spawn Id of client interacting with object
/*92*/
};
//<Zaphod> 01 = generic drop, 02 = armor, 19 = weapon
//[13:40] <Zaphod> and 0xff seems to be indicative of the tradeskill/openable items that end up returning the old style item type in the OP_OpenObject

/*
** Click Object Struct
** Client clicking on zone object (forge, groundspawn, etc)
** Size: 8 bytes
** Last Updated: Oct-17-2003
**
*/
struct ClickObject_Struct {
/*00*/	uint32 drop_id;
/*04*/	uint32 player_id;
/*08*/
};

struct Shielding_Struct {
	uint32 target_id;
};

/*
** Click Object Action Struct
** Response to client clicking on a World Container (ie, forge)
*  also sent by the client when they close the container.
**
*/
struct ClickObjectAction_Struct {
/*00*/	uint32	player_id;	// Entity Id of player who clicked object
/*04*/	uint32	drop_id;	// Zone-specified unique object identifier
/*08*/	uint32	open;		// 1=opening, 0=closing
/*12*/	uint32	type;		// See object.h, "Object Types"
/*16*/	uint32	unknown16;	// set to 0xA
/*20*/	uint32	icon;		// Icon to display for tradeskill containers
/*24*/	uint32	unknown24;	//
/*28*/	char	object_name[64]; // Object name to display
/*92*/
};

/*
** This is different now, mostly unknown
**
*/
struct CloseContainer_Struct {
/*00*/	uint32	player_id;	// Entity Id of player who clicked object
/*04*/	uint32	drop_id;	// Zone-specified unique object identifier
/*08*/	uint32	open;		// 1=opening, 0=closing
/*12*/	uint32	unknown12[12];
};

/*
** Generic Door Struct
** Length: 52 Octets
** Used in:
**    cDoorSpawnsStruct(f721)
**
*/
struct Door_Struct
{
/*0000*/ char    name[32];            // Filename of Door // Was 10char long before... added the 6 in the next unknown to it: Daeken M. BlackBlade //changed both to 32: Trevius
/*0032*/ float   yPos;               // y loc
/*0036*/ float   xPos;               // x loc
/*0040*/ float   zPos;               // z loc
/*0044*/ float	 heading;
/*0048*/ int32   incline;	// rotates the whole door
/*0052*/ int16   size;			// 100 is normal, smaller number = smaller model
/*0054*/ int8    unknown0038[6];
/*0060*/ uint8   doorId;             // door's id #
/*0061*/ uint8   opentype;
/*
 *  Open types:
 * 66 = PORT1414 (Qeynos)
 * 55 = BBBOARD (Qeynos)
 * 100 = QEYLAMP (Qeynos)
 * 56 = CHEST1 (Qeynos)
 * 5 = DOOR1 (Qeynos)
 */
/*0062*/ uint8  state_at_spawn;
/*0063*/ uint8  invert_state;	// if this is 1, the door is normally open
/*0064*/ int32  door_param;
/*0068*/ uint8  unknown0052[12]; // mostly 0s, the last 3 bytes are something tho
/*0080*/
};



struct DoorSpawns_Struct {
	struct Door_Struct *doors;
};

/*
 OP Code: 	Op_ClickDoor
 Size:		16

 10/10/2003-Doodman	Filled in struct
*/
struct ClickDoor_Struct {
/*000*/	int32	doorid;
/*004*/	int8	picklockskill;
/*005*/	int8	unknown005[3];
/*008*/ int32	item_id;
/*012*/ int16	player_id;
/*014*/ int8	unknown014[2];
};

struct MoveDoor_Struct {
	int8	doorid;
	int8	action;
};


struct BecomeNPC_Struct {
	int32 id;
	sint32 maxlevel;
};

struct Underworld_Struct {
	float speed;
	float y;
	float x;
	float z;
};

struct Resurrect_Struct	{
/*000*/	int32	unknown000;
/*004*/	int16	zone_id;
/*006*/	int16	instance_id;
/*008*/	float	y;
/*012*/	float	x;
/*016*/	float	z;
/*020*/	int32   unknown020;
/*024*/	char	your_name[64];
/*088*/	int32	unknown088;
/*092*/	char	rezzer_name[64];
/*156*/	int32	spellid;
/*160*/	char	corpse_name[64];
/*224*/	int32	action;
/* 228 */
};

struct Translocate_Struct {
/*000*/	int32	ZoneID;
/*004*/	int32	SpellID;
/*008*/	int32	unknown008;  //Heading ?
/*012*/	char	Caster[64];
/*076*/	float	y;
/*080*/	float   x;
/*084*/	float	z;
/*088*/	uint32  Complete;
};

struct Sacrifice_Struct {
/*000*/	uint32	CasterID;
/*004*/	uint32	TargetID;
/*008*/	uint32	Confirm;
};

struct SetRunMode_Struct {
	int8 mode;
	int8 unknown[3];
};

//EnvDamage is EnvDamage2 without a few bytes at the end.

struct EnvDamage2_Struct {
/*0000*/	int32 id;
/*0004*/	int16 unknown4;
/*0006*/	int32 damage;
/*0010*/	int8 unknown10[12];
/*0022*/	int8 dmgtype; //FA = Lava; FC = Falling
/*0023*/	int8 unknown2[4];
/*0027*/	int16 constant; //Always FFFF
/*0029*/	int16 unknown29;
};

//Bazaar Stuff =D
//

enum {
	BazaarTrader_StartTraderMode = 1,
	BazaarTrader_EndTraderMode = 2,
	BazaarTrader_UpdatePrice = 3,
	BazaarTrader_EndTransaction = 4,
	BazaarSearchResults = 7,
	BazaarWelcome = 9,
	BazaarBuyItem = 10,
	BazaarTrader_ShowItems = 11,
	BazaarSearchDone = 12,
	BazaarTrader_CustomerBrowsing = 13
};

enum {
	BazaarPriceChange_Fail = 0,
	BazaarPriceChange_UpdatePrice = 1,
	BazaarPriceChange_RemoveItem = 2,
	BazaarPriceChange_AddItem = 3
};

struct BazaarWindowStart_Struct {
	int8   Action;
	int8   Unknown001;
	int16  Unknown002;
};


struct BazaarWelcome_Struct {
	BazaarWindowStart_Struct Beginning;
	int32  Traders;
	int32  Items;
	int8   Unknown012[8];
};

struct BazaarSearch_Struct {
	BazaarWindowStart_Struct Beginning;
	int32	TraderID;
	int32	Class_;
	int32	Race;
	int32	ItemStat;
	int32	Slot;
	int32	Type;
	char	Name[64];
	int32	MinPrice;
	int32	MaxPrice;
	int32	Minlevel;
	int32	MaxLlevel;
};
struct BazaarInspect_Struct{
	int32 ItemID;
	int32 Unknown004;
	char Name[64];
};

struct NewBazaarInspect_Struct {
/*000*/	BazaarWindowStart_Struct Beginning;
/*004*/	char Name[64];
/*068*/	int32 Unknown068;
/*072*/	int32 Unknown072;
/*076*/	int32 Unknown076;
/*080*/	sint32 SerialNumber;
/*084*/	int32 Unknown084;
};

struct BazaarReturnDone_Struct{
	int32 Type;
	int32 TraderID;
	int32 Unknown008;
	int32 Unknown012;
	int32 Unknown016;
};
struct BazaarSearchResults_Struct {
	BazaarWindowStart_Struct Beginning;
	int32	NumItems;
	int32	ItemID;
	int32	SellerID;
	int32	Cost;
	int32	ItemStat;
	char	Name[64];
};

// Barter/Buyer
//
//
enum {
	Barter_BuyerSearch = 0,
	Barter_SellerSearch = 1,
	Barter_BuyerModeOn = 2,
	Barter_BuyerModeOff = 3,
	Barter_BuyerItemUpdate = 5,
	Barter_BuyerItemRemove = 6,
	Barter_SellItem = 7,
	Barter_SellerTransactionComplete = 8,
	Barter_BuyerTransactionComplete = 9,
	Barter_BuyerInspectBegin = 10,
	Barter_BuyerInspectEnd = 11,
	Barter_BuyerAppearance = 12,
	Barter_BuyerInspectWindow = 13,
	Barter_BarterItemInspect = 14,
	Barter_SellerBrowsing = 15,
	Barter_BuyerSearchResults = 16,
	Barter_Welcome = 17,
	Barter_WelcomeMessageUpdate = 19,
	Barter_BuyerItemInspect = 21
};

struct BuyerWelcomeMessageUpdate_Struct {
/*000*/	uint32	Action;
/*004*/	char	WelcomeMessage[256];
};

struct BuyerItemSearch_Struct {
/*000*/	uint32	Unknown000;
/*004*/	char	SearchString[64];
};

struct	BuyerItemSearchResultEntry_Struct {
/*000*/	char	ItemName[64];
/*064*/	uint32	ItemID;
/*068*/	uint32	Unknown068;
/*072*/	uint32	Unknown072;
};

#define MAX_BUYER_ITEMSEARCH_RESULTS 200

struct	BuyerItemSearchResults_Struct {
	uint32	Action;
	uint32	ResultCount;
	BuyerItemSearchResultEntry_Struct	Results[MAX_BUYER_ITEMSEARCH_RESULTS];
};

struct BarterSearchRequest_Struct {
	uint32	Action;
	char	SearchString[64];
	uint32	SearchID;
};

struct BuyerItemSearchLinkRequest_Struct {
/*000*/	uint32	Action;	// 0x00000015
/*004*/	uint32	ItemID;
/*008*/	uint32	Unknown008;
/*012*/	uint32	Unknown012;
};

struct BarterItemSearchLinkRequest_Struct {
/*000*/	uint32	Action;	// 0x0000000E
/*004*/	uint32	SearcherID;
/*008*/	uint32	Unknown008;
/*012*/	uint32	Unknown012;
/*016*/	uint32	ItemID;
/*020*/	uint32	Unknown020;
};

struct BuyerInspectRequest_Struct {
	uint32	Action;
	uint32	BuyerID;
	uint32	Approval;
};

struct BuyerBrowsing_Struct {
	uint32	Action;
	char	PlayerName[64];
};

struct BuyerRemoveItem_Struct {
	uint32	Action;
	uint32	BuySlot;
};

struct ServerSideFilters_Struct {
int8	clientattackfilters; // 0) No, 1) All (players) but self, 2) All (players) but group
int8	npcattackfilters;	 // 0) No, 1) Ignore NPC misses (all), 2) Ignore NPC Misses + Attacks (all but self), 3) Ignores NPC Misses + Attacks (all but group)
int8	clientcastfilters;	 // 0) No, 1) Ignore PC Casts (all), 2) Ignore PC Casts (not directed towards self)
int8	npccastfilters;		 // 0) No, 1) Ignore NPC Casts (all), 2) Ignore NPC Casts (not directed towards self)
};

/*
** Client requesting item statistics
** Size: 48 bytes
** Used In: OP_ItemLinkClick
** Last Updated: 2/15/2009
**
*/
struct	ItemViewRequest_Struct {
/*000*/	uint32	item_id;
/*004*/	uint32	augments[5];
/*024*/ uint32	link_hash;
/*028*/	uint32	unknown028;
/*032*/	char	unknown032[12];	//probably includes loregroup & evolving info. see Client::MakeItemLink() in zone/inventory.cpp:469
/*044*/	uint16	icon;
/*046*/	char	unknown046[2];
};

struct	LDONItemViewRequest_Struct {
	uint32	item_id;
	uint8	unknown004[4];
	char	item_name[64];
};

/*
 *  Client to server packet
 */
struct PickPocket_Struct {
// Size 18
    uint32 to;
    uint32 from;
    uint16 myskill;
    uint8 type; // -1 you are being picked, 0 failed , 1 = plat, 2 = gold, 3 = silver, 4 = copper, 5 = item
    uint8 unknown1; // 0 for response, unknown for input
    uint32 coin;
    uint8 lastsix[2];
};
/*
 * Server to client packet
 */

enum  {
	PickPocketFailed = 0,
	PickPocketPlatinum = 1,
	PickPocketGold = 2,
	PickPocketSilver = 3,
	PickPocketCopper = 4,
	PickPocketItem = 5
};


struct sPickPocket_Struct {
	// Size 28 = coin/fail
	uint32 to;
	uint32 from;
	uint32 myskill;
	uint32 type;
	uint32 coin;
	char itemname[64];
};

struct LogServer_Struct {
// Op_Code OP_LOGSERVER
/*000*/	uint32	unknown000;
/*004*/	uint8	enable_pvp;
/*005*/	uint8	unknown005;
/*006*/	uint8	unknown006;
/*007*/	uint8	unknown007;
/*008*/	uint8	enable_FV;
/*009*/	uint8	unknown009;
/*010*/	uint8	unknown010;
/*011*/	uint8	unknown011;
/*012*/	uint32	unknown012;	// htonl(1) on live
/*016*/	uint32	unknown016;	// htonl(1) on live
/*020*/	uint8	unknown020[12];
/*032*/	char	worldshortname[32];
/*064*/	uint8	unknown064[32];
/*096*/	char	unknown096[16];	// 'pacman' on live
/*112*/	char	unknown112[16];	// '64.37,148,36' on live
/*126*/	uint8	unknown128[48];
/*176*/	uint32	unknown176;	// htonl(0x00002695)
/*180*/	char	unknown180[80];	// 'eqdataexceptions@mail.station.sony.com' on live
/*260*/	uint8	enable_petition_wnd;
/*261*/	uint8	enablevoicemacros;
/*262*/	uint8	enablemail;
/*263*/	uint8	disable_tutorial_go_home;
/*264*/
};

struct ApproveWorld_Struct {
// Size 544
// Op_Code OP_ApproveWorld
    uint8 unknown544[544];
};

struct ClientError_Struct
{
/*00001*/	char	type;
/*00001*/	char	unknown0001[69];
/*00069*/	char	character_name[64];
/*00134*/	char	unknown134[192];
/*00133*/	char	message[31994];
/*32136*/
};

struct MobHealth
{
	/*0000*/	int8	hp; //health percent
	/*0001*/	int16	id;//mobs id
};

struct Track_Struct {
	int16 entityid;
	int16 padding002;
	float distance;
};

struct Tracking_Struct {
	Track_Struct *Entrys;
};

/*
** ZoneServerInfo_Struct
** Zone server information
** Size: 130 bytes
** Used In: OP_ZoneServerInfo
**
*/
struct ZoneServerInfo_Struct
{
/*0000*/	char	ip[128];
/*0128*/	uint16	port;
};

struct WhoAllPlayer{
	int32	formatstring;
	int32	pidstring;
	char*	name;
	int32	rankstring;
	char*	guild;
	int32	unknown80[2];
	int32	zonestring;
	int32	zone;
	int32	class_;
	int32	level;
	int32	race;
	char*	account;
	int32	unknown100;
};

// The following four structs are the WhoAllPlayer struct above broken down
// for use in World ClientList::SendFriendsWho to accomodate the user of variable
// length strings within the struct above.

struct	WhoAllPlayerPart1 {
	int32	FormatMSGID;
	int32	PIDMSGID;
	char	Name[1];;
};

struct	WhoAllPlayerPart2 {
	int32	RankMSGID;
	char	Guild[1];
};

struct	WhoAllPlayerPart3 {
	int32	Unknown80[2];
	int32	ZoneMSGID;
	int32	Zone;
	int32	Class_;
	int32	Level;
	int32	Race;
	char	Account[1];
};

struct	WhoAllPlayerPart4 {
	int32	Unknown100;
};

struct WhoAllReturnStruct {
/*000*/	int32	id;
/*004*/	int32	playerineqstring;
/*008*/	char	line[27];
/*035*/	int8	unknown35; //0A
/*036*/	int32	unknown36;//0s
/*040*/	int32	playersinzonestring;
/*044*/	int32	unknown44[2]; //0s
/*052*/	int32	unknown52;//1
/*056*/	int32	unknown56;//1
/*060*/	int32	playercount;//1
	struct WhoAllPlayer *player;
};

struct Trader_Struct {
/*000*/	int32	Code;
/*004*/	int32	Unknown004;
/*008*/	uint64	Items[80];
/*648*/	int32	ItemCost[80];
};

struct ClickTrader_Struct {
/*000*/	int32	Code;
/*004*/	int32	Unknown004;
/*008*/	sint64	SerialNumber[80];
/*648*/	int32	ItemCost[80];
};

struct GetItems_Struct{
	int32	Items[80];
	sint32	SerialNumber[80];
	sint32	Charges[80];
};

struct BecomeTrader_Struct{
	int32 ID;
	int32 Code;
};

struct Trader_ShowItems_Struct{
/*000*/	int32 Code;
/*004*/	int32 TraderID;
/*008*/	int32 Unknown08[3];
};

struct TraderBuy_Struct{
/*000*/	int32 Action;
/*004*/	int32 TraderID;
/*008*/	int32 ItemID;
/*012*/	int32 AlreadySold;
/*016*/	int32 Price;
/*020*/	int32 Quantity;
/*024*/	char  ItemName[64];
};

struct TraderItemUpdate_Struct{
	int32 Unknown000;
	int32 TraderID;
	int8  FromSlot;
	int   ToSlot; //7?
	int16 Charges;
};

struct TraderPriceUpdate_Struct {
/*000*/	int32	Action;
/*004*/	int32	SubAction;
/*008*/	sint32	SerialNumber;
/*012*/	int32	Unknown012;
/*016*/	int32	NewPrice;
/*020*/	int32	Unknown016;
};

struct MoneyUpdate_Struct{
	sint32 platinum;
	sint32 gold;
	sint32 silver;
	sint32 copper;
};

struct TraderDelItem_Struct{
	int32 Unknown000;
	int32 TraderID;
	int32 ItemID;
	int32 Unknown012;
};

struct TraderClick_Struct{
/*000*/	int32 TraderID;
/*004*/	int32 Unknown004;
/*008*/	int32 Unknown008;
/*012*/	int32 Approval;
};

struct FormattedMessage_Struct{
	int32	unknown0;
	int32	string_id;
	int32	type;
	char	*message;
};
struct SimpleMessage_Struct{
	int32	string_id;
	int32	color;
	int32	unknown8;
};

struct GuildMemberUpdate_Struct {
/*00*/	int32	guild_id;
/*04*/	char	member_name[64];
/*68*/	int16	zone_id;
/*70*/	int16	instance_id;	//speculated
/*72*/	int32	some_timestamp;	//unix timestamp
/*76*/
};

struct GuildMemberLevelUpdate_Struct {
/*00*/	uint32 guild_id;
/*04*/	char	member_name[64];
/*68*/	uint32	level;	//not sure
};

struct Internal_GuildMemberEntry_Struct {
//	char	name[64];					//variable length
	int32	level;						//network byte order
	int32	banker;						//1=yes, 0=no, network byte order
	int32	class_;						//network byte order
	int32	rank;						//network byte order
	int32	time_last_on;				//network byte order
	int32	tribute_enable;				//network byte order
	int32	total_tribute;				//total guild tribute donated, network byte order
	int32	last_tribute;				//unix timestamp
//	char	public_note[1];				//variable length.
	int16	zoneinstance;				//network byte order
	int16	zone_id;					//network byte order
};

struct Internal_GuildMembers_Struct {	//just for display purposes, this is not actually used in the message encoding.
	char	player_name[64];		//variable length.
	int32	count;				//network byte order
	int32	name_length;	//total length of all of the char names, excluding terminators
	int32	note_length;	//total length of all the public notes, excluding terminators
	Internal_GuildMemberEntry_Struct *member;
	/*
	 * followed by a set of `count` null terminated name strings
	 * and then a set of `count` null terminated public note strings
	 */
};

struct GuildMOTD_Struct{
/*0000*/	int32	unknown0;
/*0004*/	char	name[64];
/*0068*/	char	setby_name[64];
/*0132*/	int32	unknown132;
/*0136*/	char	motd[512];
};

struct GuildUpdate_PublicNote{
	int32	unknown0;
	char	name[64];
	char	target[64];
	char	note[1]; //variable length.
};

struct GuildDemoteStruct{
	char	name[64];
	char	target[64];
};

struct GuildRemoveStruct{
	char	target[64];
	char	name[64];
	int32	unknown128;
	int32	leaderstatus; //?
};

struct GuildMakeLeader{
	char	name[64];
	char	target[64];
};

struct BugStruct{
/*0000*/	char	chartype[64];
/*0064*/	char	name[96];
/*0160*/	char	ui[128];
/*0288*/	float	x;
/*0292*/	float	y;
/*0296*/	float	z;
/*0300*/	float	heading;
/*0304*/	int32	unknown304;
/*0308*/	char	unknown308[160];
/*0468*/	char	target_name[64];
/*0532*/	int32	type;
/*0536*/	char	unknown536[2052];
/*2584*/	char	bug[2048];
/*4632*/	char	unknown4632[6];
/*4638*/	char	system_info[4094];
};
struct Make_Pet_Struct { //Simple struct for getting pet info
	int8 level;
	int8 class_;
	int16 race;
	int8 texture;
	int8 pettype;
	float size;
	int8 type;
	int32 min_dmg;
	int32 max_dmg;
};
struct Ground_Spawn{
	float max_x;
	float max_y;
	float min_x;
	float min_y;
	float max_z;
	float heading;
	char name[16];
	int32 item;
	int32 max_allowed;
	int32 respawntimer;
};
struct Ground_Spawns {
	struct Ground_Spawn spawn[50]; //Assigned max number to allow
};
struct PetitionBug_Struct{
	int32	petition_number;
	int32	unknown4;
	char	accountname[64];
	int32	zoneid;
	char	name[64];
	int32	level;
	int32	class_;
	int32	race;
	int32	unknown152[3];
	int32	time;
	int32	unknown168;
	char	text[1028];
};

struct DyeStruct
{
	union
	{
		struct
		{
			struct Color_Struct head;
			struct Color_Struct chest;
			struct Color_Struct arms;
			struct Color_Struct wrists;
			struct Color_Struct hands;
			struct Color_Struct legs;
			struct Color_Struct feet;
			struct Color_Struct primary;	// you can't actually dye this
			struct Color_Struct secondary;	// or this
		}
		dyes;
		struct Color_Struct dye[9];
	};
};

struct ApproveZone_Struct {
	char	name[64];
	int32	zoneid;
	int32	approve;
};
struct ZoneInSendName_Struct {
	int32	unknown0;
	char	name[64];
	char	name2[64];
	int32	unknown132;
};
struct ZoneInSendName_Struct2 {
	int32	unknown0;
	char	name[64];
	int32	unknown68[145];
};

static const uint32 MAX_TRIBUTE_TIERS = 10;

struct StartTribute_Struct {
   int32	client_id;
   int32	tribute_master_id;
   int32	response;
};

struct TributeLevel_Struct {
   uint32	level;	//backwards byte order!
   int32	tribute_item_id;	//backwards byte order!
   int32	cost;	//backwards byte order!
};

struct TributeAbility_Struct {
	int32	tribute_id;	//backwards byte order!
	int32	tier_count;	//backwards byte order!
	TributeLevel_Struct tiers[MAX_TRIBUTE_TIERS];
	char	*name;
};

struct GuildTributeAbility_Struct {
	uint32	guild_id;
	TributeAbility_Struct ability;
};

struct SelectTributeReq_Struct {
   int32	client_id;	//? maybe action ID?
   uint32	tribute_id;
   int32	unknown8;	//seen E3 00 00 00
};

struct SelectTributeReply_Struct {
   int32	client_id;	//echoed from request.
   uint32	tribute_id;
   char	*desc;
};

struct TributeInfo_Struct {
	int32	active;		//0 == inactive, 1 == active
	uint32	tributes[MAX_PLAYER_TRIBUTES];	//-1 == NONE
	int32	tiers[MAX_PLAYER_TRIBUTES];		//all 00's
	int32	tribute_master_id;
};

struct TributeItem_Struct {
	int32   slot;
	int32   quantity;
	int32   tribute_master_id;
	sint32  tribute_points;
};

struct TributePoint_Struct {
	sint32   tribute_points;
	int32   unknown04;
	sint32   career_tribute_points;
	int32   unknown12;
};

struct TributeMoney_Struct {
	int32   platinum;
	int32   tribute_master_id;
	sint32   tribute_points;
};


struct Split_Struct
{
	uint32	platinum;
	uint32	gold;
	uint32	silver;
	uint32	copper;
};


/*
** New Combine Struct
** Client requesting to perform a tradeskill combine
** Size: 4 bytes
** Used In: OP_TradeSkillCombine
** Last Updated: Oct-15-2003
**
*/
struct NewCombine_Struct {
/*00*/	sint16	container_slot;
/*02*/	char	unknown02[2];
/*04*/
};


//client requesting favorite recipies
struct TradeskillFavorites_Struct {
	uint32 object_type;
	uint32 some_id;
	uint32 favorite_recipes[500];
};

//search request
struct RecipesSearch_Struct {
	uint32 object_type;	//same as in favorites
	uint32 some_id;			//same as in favorites
	uint32 mintrivial;
	uint32 maxtrivial;
	char query[56];
	uint32 unknown4;	//is set to 00 03 00 00
	uint32 unknown5; //is set to 4C DD 12 00
/*80*/
};

//one sent for each item, from server in reply to favorites or search
struct RecipeReply_Struct {
	uint32 object_type;
	uint32 some_id;	 //same as in favorites
	uint32 component_count;
	uint32 recipe_id;
	uint32 trivial;
	char recipe_name[64];
/*84*/
};

//received and sent back as an ACK with different reply_code
struct RecipeAutoCombine_Struct {
	uint32 object_type;
	uint32 some_id;
	uint32 unknown1;		//echoed in reply
	uint32 recipe_id;
	uint32 reply_code;		// 93 64 e1 00 (junk) in request
								// 00 00 00 00 in successful reply
								// f5 ff ff ff in 'you dont have all the stuff' reply
};

struct LevelAppearance_Struct { //Sends a little graphic on level up
	int32	spawn_id;
	int32	parm1;
	int32	value1a;
	int32	value1b;
	int32	parm2;
	int32	value2a;
	int32	value2b;
	int32	parm3;
	int32	value3a;
	int32	value3b;
	int32	parm4;
	int32	value4a;
	int32	value4b;
	int32	parm5;
	int32	value5a;
	int32	value5b;
/*64*/
};
struct MerchantList{
	uint32	id;
	uint32	slot;
	uint32	item;
};
struct TempMerchantList{
	uint32	npcid;
	uint32	slot;
	uint32	item;
	uint32	charges; //charges/quantity
	uint32	origslot;
};


struct FindPerson_Point {
	float y;
	float x;
	float z;
};

struct FindPersonRequest_Struct {
	uint32	npc_id;
	FindPerson_Point client_pos;
};

//variable length packet of points
struct FindPersonResult_Struct {
	FindPerson_Point dest;
	FindPerson_Point *path;	//last element must be the same as dest
};

struct MobRename_Struct {
/*000*/	char	old_name[64];
/*064*/	char	old_name_again[64];	//not sure what the difference is
/*128*/	char	new_name[64];
/*192*/	uint32	unknown192;		//set to 0
/*196*/	uint32	unknown196;		//set to 1
/*200*/
};

struct PlayMP3_Struct {
	char filename[128];
};

//this is for custom title display in the skill window
struct TitleEntry_Struct {
	uint32	skill_id;
	uint32	skill_value;
	char	title[1];
};

struct Titles_Struct {
	uint32	title_count;
	TitleEntry_Struct *titles;
};

//this is for title selection by the client
struct TitleListEntry_Struct {
	uint32	unknown0;	//title ID
	char prefix[1];		//variable length, null terminated
	char postfix[1];		//variable length, null terminated
};

struct TitleList_Struct {
	uint32 title_count;
	TitleListEntry_Struct *titles;	//list of title structs
	//uint32 unknown_ending; seen 0x7265, 0
};

struct SetTitle_Struct {
	uint32	is_suffix;	//guessed: 0 = prefix, 1 = suffix
	uint32	title_id;
};

struct SetTitleReply_Struct {
	uint32	is_suffix;	//guessed: 0 = prefix, 1 = suffix
	char	title[32];
	uint32	entity_id;
};

#if 0
// Old struct not used by Task System implementation but left for reference
struct TaskDescription_Struct {
/*000*/	uint32	activity_count;		//not right.
/*004*/	uint32	taskid;
/*008*/	uint8	unk;
/*009*/	uint32	id3;
/*013*/	uint32	unknown13;
/*017*/	char	name[1];	//variable length, 0 terminated
/*018*/	uint32	unknown18;
/*022*/	uint32	unknown22;
/*026*/	char	desc[1];	//variable length, 0 terminated
/*027*/	uint32	reward_count;   //not sure
/*031*/	uint32	unknown31;
/*035*/	uint32	unknown35;
/*039*/	uint16	unknown39;
/*041*/	char	reward_link[1];	//variable length, 0 terminated
/*042*/	uint32	unknown43;  //maybe crystal count?
};
#endif

struct TaskMemberList_Struct {
/*00*/  uint32  gopher_id;
/*04*/  uint32  unknown04;
/*08*/  uint32  member_count;   //1 less than the number of members
/*12*/  char	*list_pointer;
/*	list is of the form:
	char member_name[1]	//null terminated string
	uint8   task_leader	//boolean flag
*/
};

#if 0
// Old structs not used by Task System implentation but left for reference
struct TaskActivity_Struct {
/*000*/	uint32	activity_count;		//not right
/*004*/	uint32	id3;
/*008*/	uint32	taskid;
/*012*/	uint32	activity_id;
/*016*/	uint32	unknown016;
/*020*/	uint32	activity_type;
/*024*/	uint32	unknown024;
/*028*/	uint32	unknown28;
/*032*/	char mob_name[1];	//variable length, 0 terminated
/*033*/	char item_name[1];	//variable length, 0 terminated
/*034*/	uint32	goal_count;
/*038*/	uint32	unknown38;	//0xFFFFFFFF
/*042*/	uint32	unknown42;	//0xFFFFFFFF
/*046*/	uint32	unknown46;  //saw 0x151,0x156
/*050*/	uint32	unknown50;  //saw 0x404,0
/*054*/	char activity_name[1];  //variable length, 0 terminated... commonly empty
/*055*/	uint32	done_count;
/*059*/	uint32	unknown59;	//=1 except on unknown and terminal activities?
/*063*/
};

struct TaskHistoryEntry_Struct {
	uint32	task_id;
	char	name[1];
	uint32	completed_time;
};
struct TaskHistory_Struct {
	uint32 completed_count;
	TaskHistoryEntry_Struct entries[0];
};
#endif

struct AcceptNewTask_Struct {
	uint32  unknown00;
	uint32	task_id;		//set to 0 for 'decline'
	uint32	task_master_id;	//entity ID
};

//was all 0's from client, server replied with same op, all 0's
struct CancelTask_Struct {
	uint32 SequenceNumber;
	uint32 unknown4; // Only seen 0x00000002
};

#if 0
// old struct, not used by Task System implementation but left for reference.
struct AvaliableTask_Struct {
	uint32	task_index;		//no idea, seen 0x1
	uint32	task_master_id;	//entity ID
	uint32	task_id;
	uint32	unknown012;
	uint32	activity_count;	//not sure, seen 2
	char desc[1];	//variable length, 0 terminated
	uint32	reward_platinum;//not sure on these
	uint32	reward_gold;
	uint32	reward_silver;
	uint32	reward_copper;
	char some_name[1];	//variable length, 0 terminated
	uint8	unknown1;
	uint32	unknown2;	//0xFFFFFFFF
	uint32	unknown3;	//0xFFFFFFFF
	uint32	unknown4;	//seen 0x16
	uint8	unknown5;
};
#endif


// Many of the Task System packets contain variable length strings, as well as variable numbers
// of records, hence splitting them into multiple structs (header, middle, trailer) etc.
//
struct AvailableTaskHeader_Struct {
	uint32	TaskCount;
	uint32	unknown1;
	uint32	TaskGiver;
};

struct AvailableTaskData1_Struct {
	uint32	TaskID;
	uint32	TimeLimit;
	uint32	unknown2;
};

struct AvailableTaskData2_Struct {
	uint32	unknown1,unknown2,unknown3,unknown4;
};

struct AvailableTaskTrailer_Struct {
	uint32	ItemCount;
	uint32	unknown1, unknown2;
	uint32	StartZone;
};

struct TaskDescriptionHeader_Struct {
	uint32	SequenceNumber; // The order the tasks appear in the journal. 0 for first task, 1 for second, etc.
	uint32	TaskID;
	uint32	unknown2;
	uint32	unknown3;
	uint8	unknown4;
};

struct TaskDescriptionData1_Struct {
	uint32	Duration;
	uint32	unknown2;
	uint32	StartTime;
};

struct TaskDescriptionData2_Struct {
	uint32	RewardCount; // ??
	uint32	unknown1;
	uint32	unknown2;
	uint16	unknown3;
	//uint8	unknown4;
};

struct TaskDescriptionTrailer_Struct {
	//uint16  unknown1; // 0x0012
	uint32	Points;
};

struct TaskActivityHeader_Struct {
	uint32	TaskSequenceNumber;
	uint32	unknown2; // Seen 0x00000002
	uint32	TaskID;
	uint32	ActivityID;
	uint32	unknown3;
	uint32	ActivityType;
	uint32	Optional;
	uint32	unknown5;
};

struct TaskActivityData1_Struct {
	uint32	GoalCount;
	uint32	unknown1; // 0xffffffff
	uint32	unknown2; // 0xffffffff
	uint32	ZoneID; // seen 0x36
	uint32  unknown3;
};

struct TaskActivityTrailer_Struct {
	uint32	DoneCount;
	uint32	unknown1; // Seen 1
};

// The Short_Struct is sent for tasks that are hidden and act as a placeholder
struct TaskActivityShort_Struct {
	uint32	TaskSequenceNumber;
	uint32	unknown2; // Seen 0x00000002
	uint32	TaskID;
	uint32	ActivityID;
	uint32	unknown3;
	uint32	ActivityType; // 0xffffffff for the short packet
	uint32  unknown4;
};

struct TaskActivityComplete_Struct {
	uint32	TaskIndex;
	uint32	unknown2; // 0x00000002
	uint32	unknown3;
	uint32	ActivityID;
	uint32	unknown4; // 0x00000001
	uint32	unknown5; // 0x00000001
};

#if 0
// This is a dupe of the CancelTask struct
struct TaskComplete_Struct {
	uint32	unknown00; // 0x00000000
	uint32	unknown04; // 0x00000002
};
#endif

struct TaskHistoryRequest_Struct {
	uint32	TaskIndex; // This is the sequence the task was sent in the Completed Tasks packet.
};

struct TaskHistoryReplyHeader_Struct {
	uint32	TaskID;
	uint32	ActivityCount;
};

struct TaskHistoryReplyData1_Struct {
	uint32	ActivityType;
};

struct TaskHistoryReplyData2_Struct {
	uint32  GoalCount;
	uint32	unknown04; // 0xffffffff
	uint32	unknown08; // 0xffffffff
	uint32	ZoneID;
	uint32	unknown16;
};

enum { VoiceMacroTell = 1, VoiceMacroGroup = 2, VoiceMacroRaid = 3 };

struct VoiceMacroIn_Struct {
/*000*/	char	Unknown000[64];
/*064*/	uint32	Type;	// 1 = Tell, 2 = Group, 3 = Raid
/*068*/	char	Target[64];
/*132*/	uint32	Unknown132;	// Seems to be 0x0000000c always
/*136*/	uint32	MacroNumber;
};

struct VoiceMacroOut_Struct {
/*000*/	char	From[64];
/*064*/	uint32	Type;	// 1 = Tell, 2 = Group, 3 = Raid
/*068*/	uint32	Unknown068;
/*072*/	uint32	Voice;
/*076*/	uint32	MacroNumber;
/*080*/	char	Unknown080[60];
};

struct BankerChange_Struct {
	uint32	platinum;
	uint32	gold;
	uint32	silver;
	uint32	copper;
	uint32	platinum_bank;
	uint32	gold_bank;
	uint32	silver_bank;
	uint32	copper_bank;
};

struct LeadershipExpUpdate_Struct {
   /*0000*/ uint32 unknown0000;          // All zeroes?
   /*0004*/ uint32 group_leadership_exp;         // Group leadership exp value
   /*0008*/ uint32 group_leadership_points;   // Unspent group points
   /*0012*/ uint32 unknown0012;          // Type?
   /*0016*/ uint32 unknown0016;          // All zeroes?
   /*0020*/ uint32 raid_leadership_exp;          // Raid leadership exp value
   /*0024*/ uint32 raid_leadership_points;    // Unspent raid points
   /*0028*/ uint32 unknown0028;
};

struct UpdateLeadershipAA_Struct {
/*00*/	uint32	ability_id;
/*04*/	uint32	new_rank;
/*08*/	uint32	pointsleft;
/*12*/
};

enum
{
	GroupLeadershipAbility_MarkNPC = 0
};

struct DoGroupLeadershipAbility_Struct
{
/*000*/	uint32	Ability;
/*000*/	uint32	Parameter;
};

struct DelegateAbility_Struct
{
/*000*/	uint32	DelegateAbility;
/*004*/	uint32	MemberNumber;
/*008*/	uint32	Action;
/*012*/	uint32	Unknown012;
/*016*/	uint32	Unknown016;
/*020*/	uint32	EntityID;
/*024*/	uint32	Unknown024;
/*028*/	char	Name[64];
};

struct GroupUpdateLeaderAA_Struct
{
/*000*/	char	Unknown000[64];
/*064*/	GroupLeadershipAA_Struct LeaderAAs;
/*128*/	char	unknown128[128];
};

struct MarkNPC_Struct
{
/*00*/	uint32	TargetID;	// Target EntityID
/*04*/	uint32	Number;		// Number to mark them with (1, 2 or 3)
};

struct RaidGeneral_Struct {
/*00*/	uint32		action;	//=10
/*04*/	char		player_name[64];	//should both be the player's name
/*04*/	char		leader_name[64];
/*132*/	uint32		parameter;
};

struct RaidAddMember_Struct {
/*000*/ RaidGeneral_Struct raidGen; //param = (group num-1); 0xFFFFFFFF = no group
/*136*/ uint8 _class;
/*137*/	uint8 level;
/*138*/	uint8 isGroupLeader;
/*139*/	uint8 flags[5]; //no idea if these are needed...
};


struct RaidAdd_Struct {
/*000*/	uint32		action;	//=0
/*004*/	char		player_name[64];	//should both be the player's name
/*068*/	char		leader_name[64];
/*132*/	uint8		_class;
/*133*/	uint8		level;
/*134*/	uint8		has_group;
/*135*/	uint8		unknown135;	//seems to be 0x42 or 0
};

struct RaidCreate_Struct {
/*00*/	uint32		action;	//=8
/*04*/	char		leader_name[64];
/*68*/	uint32		leader_id;
};

struct RaidMemberInfo_Struct {
/*00*/	uint8		group_number;
/*01*/	char		member_name[1];		//dyanmic length, null terminated '\0'
/*00*/	uint8		unknown00;
/*01*/	uint8		_class;
/*02*/	uint8		level;
/*03*/	uint8		is_raid_leader;
/*04*/	uint8		is_group_leader;
/*05*/	uint8		main_tank;		//not sure
/*06*/	uint8		unknown06[5];	//prolly more flags
};

struct RaidDetails_Struct {
/*000*/	uint32		action;	//=6,20
/*004*/	char		leader_name[64];
/*068*/	uint32		unknown68[4];
/*084*/	LeadershipAA_Struct abilities;	//ranks in backwards byte order
/*128*/	uint8		unknown128[142];
/*354*/	uint32		leader_id;
};

struct RaidMembers_Struct {
/*000*/	RaidDetails_Struct		details;
/*358*/	uint32					member_count;		//including leader
/*362*/	RaidMemberInfo_Struct	members[1];
/*...*/	RaidMemberInfo_Struct	empty;	//seem to have an extra member with a 0 length name on the end
};

struct DynamicWall_Struct {
/*00*/  char	name[32];
/*32*/  float   y;
/*36*/  float   x;
/*40*/  float   z;
/*44*/  uint32  something;
/*48*/  uint32  unknown48;  //0
/*52*/  uint32  one_hundred;	//0x64
/*56*/  uint32  unknown56;  //0
/*60*/  uint32  something2;
/*64*/  sint32  unknown64;  //-1
/*68*/  uint32  unknown68;  //0
/*72*/  uint32  unknown72;  //0
/*76*/  uint32  unknown76;  //0x100
/*80*/
};

enum {	//bandolier actions
	BandolierCreate = 0,
	BandolierRemove = 1,
	BandolierSet = 2
};

struct BandolierCreate_Struct {
/*00*/	uint32	action;	//0 for create
/*04*/	uint8	number;
/*05*/	char	name[32];
/*37*/	uint16	unknown37;	//seen 0x93FD
/*39*/	uint8	unknown39;	//0
};

struct BandolierDelete_Struct {
/*00*/	uint32	action;
/*04*/	uint8	number;
/*05*/	int8	unknown05[35];
};

struct BandolierSet_Struct {
/*00*/	uint32	action;
/*04*/	uint8	number;
/*05*/	int8	unknown05[35];
};

struct Arrow_Struct {
/*000*/	uint32	type;		//unsure on name, seems to be 0x1, dosent matter
/*005*/	int8	unknown004[12];
/*016*/	float	src_y;
/*020*/	float	src_x;
/*024*/	float	src_z;
/*028*/	int8	unknown028[12];
/*040*/	float	velocity;		//4 is normal, 20 is quite fast
/*044*/	float	launch_angle;	//0-450ish, not sure the units, 140ish is straight
/*048*/	float	tilt;		//on the order of 125
/*052*/	int8	unknown052[8];
/*060*/	float	arc;
/*064*/	int8	unknown064[12];
/*076*/	uint32	source_id;
/*080*/ uint32	target_id;	//entity ID
/*084*/	uint32	item_id;	//1 to about 150ish
/*088*/	uint32	unknown088;	//seen 125, dosent seem to change anything..
/*092*/ uint32	unknown092;	//seen 16, dosent seem to change anything
/*096*/	uint8	unknown096[2];
/*098*/	uint8	skill;
/*099*/	uint8	item_type;
/*100*/	uint8	unknown100;
/*101*/	char	model_name[16];
/*117*/	int8	unknown117[19];
};

//made a bunch of trivial structs for stuff for opcode finder to use
struct Consent_Struct {
	char name[1];	//always at least a null
};

struct AdventureMerchant_Struct {
	uint32	unknown_flag;		//seems to be 1
	uint32	entity_id;
};

struct Save_Struct {
	int8	unknown00[192];
};

struct GMToggle_Struct {
	int8 unknown0[64];
	uint32 toggle;
};

struct GroupInvite_Struct {
	char invitee_name[64];
	char inviter_name[64];
//	int8	unknown128[65];
};

struct BuffFadeMsg_Struct {
	uint32 color;
	char msg[1];
};

struct UseAA_Struct {
	int32 begin;
	int32 ability;
	int32 end;
};

struct AA_Ability {
/*00*/	int32 skill_id;
/*04*/	int32 base1;
/*08*/	int32 base2;
/*12*/	int32 slot;
};

struct SendAA_Struct {
/* EMU additions for internal use */
	char name[128];
	sint16 cost_inc;

/*0000*/	int32 id;
/*0004*/	int32 unknown004;
/*0008*/	int32 hotkey_sid;
/*0012*/	int32 hotkey_sid2;
/*0016*/	int32 title_sid;
/*0020*/	int32 desc_sid;
/*0024*/	int32 class_type;
/*0028*/	int32 cost;
/*0032*/	int32 seq;
/*0036*/	int32 current_level; //1s, MQ2 calls this AARankRequired
/*0040*/	int32 prereq_skill;		//is < 0, abs() is category #
/*0044*/	int32 prereq_minpoints; //min points in the prereq
/*0048*/	int32 type;
/*0052*/	int32 spellid;
/*0056*/	int32 spell_type;
/*0060*/	int32 spell_refresh;
/*0064*/	int16 classes;
/*0066*/	int16 berserker; //seems to be 1 if its a berserker ability
/*0068*/	int32 max_level;
/*0072*/	int32 last_id;
/*0076*/	int32 next_id;
/*0080*/	int32 cost2;
/*0084*/	int32 unknown80[2]; //0s
/*0088*/	int32 total_abilities;
/*0092*/	AA_Ability *abilities;
};

struct AA_Action {
/*00*/	int32	action;
/*04*/	int32	ability;
/*08*/	int32	unknown08;
/*12*/	int32	exp_value;
};

struct AA_Skills {		//this should be removed and changed to AA_Array
/*00*/	int32	aa_skill;
/*04*/	int32	aa_value;
};

struct AAExpUpdate_Struct {
/*00*/	int32 unknown00;	//seems to be a value from AA_Action.ability
/*04*/	int32 aapoints_unspent;
/*08*/	int8 aaxp_percent;	//% of exp that goes to AAs
/*09*/	int8 unknown09[3];	//live dosent always zero these, so they arnt part of aaxp_percent
};


struct AltAdvStats_Struct {
/*000*/  uint32 experience;
/*004*/  uint16 unspent;
/*006*/  uint16	unknown006;
/*008*/  int8	percentage;
/*009*/  int8	unknown009[3];
};

struct PlayerAA_Struct {
	AA_Skills aa_list[MAX_PP_AA_ARRAY];
};

struct AATable_Struct {
	AA_Skills aa_list[MAX_PP_AA_ARRAY];
};

struct Weather_Struct {
	uint32	val1;	//generall 0x000000FF
	uint32	type;	//0x31=rain, 0x02=snow(i think), 0 = normal
	uint32	mode;
};

struct ZoneInUnknown_Struct {
	uint32	val1;
	uint32	val2;
	uint32	val3;
};

struct MobHealth_Struct {
	uint16 entity_id;
	uint8 hp;
};

struct AnnoyingZoneUnknown_Struct {
	uint32	entity_id;
	uint32	value;		//always 4
};

struct LoadSpellSet_Struct {
      uint32  spell[MAX_PP_MEMSPELL];      // 0xFFFFFFFF if no action, slot number if to unmem starting at 0
      uint32 unknown;	//there seems to be an extra field in this packet...
};

// This is the structure for OP_ZonePlayerToBind opcode. Discovered on Feb 9 2007 by FNW from packet logs for titanium client
// This field "zone_name" is text the Titanium client will display on player death
// it appears to be a variable length, null-terminated string
// In logs it has "Bind Location" text which shows up on Titanium client as ....
// "Return to Bind Location, please wait..."
// This can be used to send zone name instead.. On 6.2 client, this is ignored.
struct ZonePlayerToBind_Struct {
/*000*/	uint32 bind_zone_id;
/*004*/	float x;
/*008*/	float y;
/*012*/	float z;
/*016*/	float heading;
/*020*/	char zone_name[1];
};

typedef struct {
/*000*/	uint32	bind_number;		// Number of this bind in the iteration
/*004*/	uint32	bind_zone_id;		// ID of the zone for this bind point or resurect point
/*008*/	float	x;					// X loc for this bind point
/*012*/	float	y;					// Y loc for this bind point
/*016*/	float	z;					// Z loc for this bind point
/*020*/	float	heading;			// Heading for this bind point
/*024*/	char	bind_zone_name[1];	// Or "Bind Location" or "Resurrect"
/*000*/	uint8	validity;		// 0 = valid choice, 1 = not a valid choice at this time (resurrection)
} RespawnOptions_Struct;

struct RespawnWindow_Struct {
/*000*/	uint32	unknown000;		// Seen 0
/*004*/	uint32	time_remaining;	// Total time before respawn in milliseconds
/*008*/	uint32	unknown008;		// Seen 0
/*012*/	uint32	total_binds;	// Total Bind Point Options? - Seen 2
/*016*/ RespawnOptions_Struct bind_points;
// First bind point is "Bind Location" and the last one is "Ressurect"
};

/**
 * Shroud spawn. For others shrouding, this has their spawnId and
 * spawnStruct.
 *
 * Length: 586
 * OpCode: OP_Shroud
 */
struct spawnShroudOther
{
/*0000*/ uint32 spawnId;          // Spawn Id of the shrouded player
/*0004*/ Spawn_Struct spawn;         // Updated spawn struct for the player
/*0586*/
};

struct ApplyPoison_Struct {
	uint32 inventorySlot;
	uint32 success;
};

struct ItemVerifyRequest_Struct {
/*000*/	sint32	slot;		// Slot being Right Clicked
/*004*/	uint32	target;		// Target Entity ID
/*008*/
};

struct ItemVerifyReply_Struct {
/*000*/	sint32	slot;		// Slot being Right Clicked
/*004*/	uint32	spell;		// Spell ID to cast if different than item effect
/*008*/	uint32	target;		// Target Entity ID
/*012*/
};

/**
 * Shroud yourself. For yourself shrouding, this has your spawnId, spawnStruct,
 * bits of your charProfileStruct (no checksum, then charProfile up till
 * but not including name), and an itemPlayerPacket for only items on the player
 * and not the bank.
 *
 * Length: Variable
 * OpCode: OP_Shroud
 */
#if 0
struct spawnShroudSelf
{
/*00000*/ uint32_t spawnId;            // Spawn Id of you
/*00004*/ Spawn_Struct spawn;           // Updated spawnStruct for you
//this is a sub-struct of PlayerProfile, which we havent broken out yet.
/*00586*/ playerProfileStruct profile; // Character profile for shrouded char
/*13522*/ uint8 items;               // Items on the player
/*xxxxx*/
};
#endif


typedef struct {
	char	Name[64];
	uint16	Class;
	uint16	Level;
	uint16	Zone;
	uint16	GuildID;
} GroupLFPMemberEntry;

struct ControlBoat_Struct {
/*000*/	uint32	boatId;		// entitylist id of the boat
/*004*/	bool	TakeControl;	// 01 if taking control, 00 if releasing it
/*007*/ 				// no idea what these last three bytes represent
};

struct AugmentInfo_Struct
{
/*000*/ uint32	itemid;		// id of the solvent needed
/*004*/ uint8	window;		// window to display the information in
/*005*/	uint8	unknown005[67];	// total packet length 72, all the rest were always 00
/*072*/
};

struct ClearObject_Struct
{
/*000*/	uint8	Clear;	// If this is not set to non-zero there is a random chance of a client crash.
/*001*/	uint8	Unknown001[7];
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

struct PVPStats_Struct
{
/*0000*/ uint32 Kills;
/*0004*/ uint32 Deaths;
/*0008*/ uint32 PVPPointsAvailable;
/*0012*/ uint32 TotalPVPPoints;
/*0016*/ uint32 BestKillStreak;
/*0020*/ uint32 WorstDeathStreak;
/*0024*/ uint32 CurrentKillStreak;
/*0028*/ uint32 Infamy;
/*0032*/ uint32 Vitality;
/*0036*/ PVPStatsEntry_Struct LastDeath;
/*0124*/ PVPStatsEntry_Struct LastKill;
/*0212*/ PVPStatsEntry_Struct KillsLast24Hours[50];
/*4612*/
};

typedef struct
{
/*000*/	char Name[64];
/*064*/	uint32 Kills;
/*068*/	uint32 Deaths;
/*072*/	uint32 TotalPoints;
/*076*/	uint32 Infamy;
/*080*/
} PVPLeaderBoardEntry_Struct;

enum { PVPSortByKills = 0, PVPSortByPoints, PVPSortByInfamy };

struct PVPLeaderBoardRequest_Struct
{
/*00*/ uint32 SortType;
/*04*/
};

struct PVPLeaderBoard_Struct
{
/*0000*/ uint32 Unknown0000;
/*0004*/ uint32 MyKills;
/*0008*/ uint32 MyTotalPoints;
/*0012*/ uint32 MyRank;
/*0016*/ uint32 MyDeaths;
/*0020*/ uint32 MyInfamy;
/*0024*/ PVPLeaderBoardEntry_Struct Entries[100];
/*8024*/
};

struct PVPLeaderBoardDetailsRequest_Struct
{
/*00*/ uint32 Unknown00;
/*04*/ char Name[64];
/*68*/
};

struct PVPLeaderBoardDetailsReply_Struct
{
/*000*/ char Name[64];
/*064*/ int8 Unknown064[64];
/*128*/ uint32 Level;
/*132*/ uint32 Race;
/*136*/ uint32 Class;
/*140*/ uint32 GuildID;
/*144*/ uint32 TotalAA;
/*148*/ uint32 Unknown148;
/*152*/ uint32 Kills;
/*156*/ uint32 Deaths;
/*160*/ uint32 Infamy;
/*164*/ uint32 Points;
/*168*/
};

struct DisciplineTimer_Struct
{
/*00*/ uint32  TimerID;
/*04*/ uint32  Duration;
/*08*/ uint32  Unknown08;
};

//old structures live here:
//#include "eq_old_structs.h"

// Restore structure packing to default
#pragma pack()

#endif
