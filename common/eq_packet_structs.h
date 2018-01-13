/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

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

#ifndef EQ_PACKET_STRUCTS_H
#define EQ_PACKET_STRUCTS_H

#include "types.h"
#include <string.h>
#include <string>
#include <list>
#include <time.h>
#include "../common/version.h"
#include "emu_constants.h"
#include "textures.h"


static const uint32 BUFF_COUNT = 25;
static const uint32 PET_BUFF_COUNT = 30;
static const uint32 MAX_MERC = 100;
static const uint32 MAX_MERC_GRADES = 10;
static const uint32 MAX_MERC_STANCES = 10;
static const uint32 BLOCKED_BUFF_COUNT = 20;


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
	uint32 race;
	uint32 class_;
	uint32 deity;
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
	uint32 duel_initiator;
	uint32 duel_target;
};

struct DuelResponse_Struct
{
	uint32 target_id;
	uint32 entity_id;
	uint32 unknown;
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

typedef enum
{
	LDoNTypeMechanical = 1,
	LDoNTypeMagical = 2,
	LDoNTypeCursed = 3,
} LDoNChestTypes;

struct LDoNTrapTemplate
{
	uint32 id;
	LDoNChestTypes type;
	uint32 spell_id;
	uint16 skill;
	uint8 locked;
};

///////////////////////////////////////////////////////////////////////////////


// All clients translate the character select information to some degree

struct CharSelectEquip : EQEmu::textures::Texture_Struct, EQEmu::textures::Tint_Struct {};

// RoF2-based hybrid struct
struct CharacterSelectEntry_Struct
{
	char Name[64];
	uint8 Class;
	uint32 Race;
	uint8 Level;
	uint8 ShroudClass;
	uint32 ShroudRace;
	uint16 Zone;
	uint16 Instance;
	uint8 Gender;
	uint8 Face;
	CharSelectEquip	Equip[EQEmu::textures::materialCount];
	uint8 Unknown15;			// Seen FF
	uint8 Unknown19;			// Seen FF
	uint32 DrakkinTattoo;
	uint32 DrakkinDetails;
	uint32 Deity;
	uint32 PrimaryIDFile;
	uint32 SecondaryIDFile;
	uint8 HairColor;
	uint8 BeardColor;
	uint8 EyeColor1;
	uint8 EyeColor2;
	uint8 HairStyle;
	uint8 Beard;
	uint8 GoHome;				// Seen 0 for new char and 1 for existing
	uint8 Tutorial;				// Seen 1 for new char or 0 for existing
	uint32 DrakkinHeritage;
	uint8 Unknown1;				// Seen 0
	uint8 Enabled;				// Originally labeled as 'CharEnabled' - unknown purpose and setting
	uint32 LastLogin;
	uint8 Unknown2;				// Seen 0
};

struct CharacterSelect_Struct
{
	uint32 CharCount;	//number of chars in this packet
	uint32 TotalChars;	//total number of chars allowed?
	CharacterSelectEntry_Struct Entries[0];
};

/*
** Generic Spawn Struct
** Length: 257 Bytes
** Fields from old struct not yet found:
**	float	size;
**	float	walkspeed;	// probably one of the ff 33 33 33 3f
**	float	runspeed;	// probably one of the ff 33 33 33 3f
**	uint8	traptype;	// 65 is disarmable trap, 66 and 67 are invis triggers/traps
**	uint8	npc_armor_graphic;	// 0xFF=Player, 0=none, 1=leather, 2=chain, 3=steelplate
**	uint8	npc_helm_graphic;	// 0xFF=Player, 0=none, 1=leather, 2=chain, 3=steelplate
**
*/

/*
** Generic Spawn Struct
** Length: 383 Octets
** Used in:
** spawnZoneStruct
** dbSpawnStruct
** petStruct
** newSpawnStruct
*/
/*
showeq -> eqemu
sed -e 's/_t//g' -e 's/seto_0xFF/set_to_0xFF/g'
*/
struct Spawn_Struct {
/*0000*/ uint8 unknown0000;
/*0001*/ uint8	gm;					// 0=no, 1=gm
/*0002*/ uint8	unknown0003;
/*0003*/ uint8	aaitle;				// 0=none, 1=general, 2=archtype, 3=class
/*0004*/ uint8	unknown0004;
/*0005*/ uint8	anon;				// 0=normal, 1=anon, 2=roleplay
/*0006*/ uint8	face;				// Face id for players
/*0007*/ char	name[64];			// Player's Name
/*0071*/ uint16	deity;				// Player's Deity
/*0073*/ uint16 unknown0073;
/*0075*/ float	size;				// Model size
/*0079*/ uint32	unknown0079;
/*0083*/ uint8	NPC;				// 0=player,1=npc,2=pc corpse,3=npc corpse,a
/*0084*/ uint8	invis;				// Invis (0=not, 1=invis)
/*0085*/ uint8	haircolor;			// Hair color
/*0086*/ uint8	curHp;				// Current hp %%% wrong
/*0087*/ uint8	max_hp;				// (name prolly wrong)takes on the value 100 for players, 100 or 110 for NPCs and 120 for PC corpses...
/*0088*/ uint8	findable;			// 0=can't be found, 1=can be found
/*0089*/ uint8	unknown0089[5];
/*0094*/ signed	deltaHeading:10;	// change in heading
/*????*/ signed	x:19;				// x coord
/*????*/ signed	padding0054:3;		// ***Placeholder
/*0098*/ signed	y:19;				// y coord
/*????*/ signed	animation:10;		// animation
/*????*/ signed	padding0058:3;		// ***Placeholder
/*0102*/ signed	z:19;				// z coord
/*????*/ signed	deltaY:13;			// change in y
/*0106*/ signed	deltaX:13;			// change in x
/*????*/ unsigned	heading:12;		// heading
/*????*/ signed	padding0066:7;		// ***Placeholder
/*0110*/ signed	deltaZ:13;			// change in z
/*????*/ signed	padding0070:19;		// ***Placeholder
/*0114*/ uint8	eyecolor1;			// Player's left eye color
/*0115*/ uint8	unknown0115[11];	// Was [24]
/*0126*/ uint8	StandState;	// stand state for SoF+ 0x64 for normal animation
/*0127*/ uint32	drakkin_heritage;	// Added for SoF
/*0131*/ uint32	drakkin_tattoo;		// Added for SoF
/*0135*/ uint32	drakkin_details;	// Added for SoF
/*0139*/ uint8	showhelm;			// 0=no, 1=yes
/*0140*/ uint8	unknown0140[4];
/*0144*/ uint8	is_npc;				// 0=no, 1=yes
/*0145*/ uint8	hairstyle;			// Hair style
/*0146*/ uint8	beard;				// Beard style (not totally, sure but maybe!)
/*0147*/ uint8	unknown0147[4];
/*0151*/ uint8	level;				// Spawn Level
// None = 0, Open = 1, WeaponSheathed = 2, Aggressive = 4, ForcedAggressive = 8, InstrumentEquipped = 16, Stunned = 32, PrimaryWeaponEquipped = 64, SecondaryWeaponEquipped = 128
/*0152*/ uint32 PlayerState;           // Controls animation stuff
/*0156*/ uint8	beardcolor;			// Beard color
/*0157*/ char	suffix[32];			// Player's suffix (of Veeshan, etc.)
/*0189*/ uint32	petOwnerId;			// If this is a pet, the spawn id of owner
/*0193*/ uint8	guildrank;			// 0=normal, 1=officer, 2=leader
/*0194*/ uint8	unknown0194[3];
/*0197*/ EQEmu::TextureProfile equipment;
/*0233*/ float	runspeed;		// Speed when running
/*0036*/ uint8	afk;			// 0=no, 1=afk
/*0238*/ uint32	guildID;		// Current guild
/*0242*/ char	title[32];		// Title
/*0274*/ uint8	unknown0274;	// non-zero prefixes name with '!'
/*0275*/ uint8	set_to_0xFF[8];	// ***Placeholder (all ff)
/*0283*/ uint8	helm;			// Helm texture
/*0284*/ uint32	race;			// Spawn race
/*0288*/ uint32	unknown0288;
/*0292*/ char	lastName[32];	// Player's Lastname
/*0324*/ float	walkspeed;		// Speed when walking
/*0328*/ uint8	unknown0328;
/*0329*/ uint8	is_pet;			// 0=no, 1=yes
/*0330*/ uint8	light;			// Spawn's lightsource %%% wrong
/*0331*/ uint8	class_;			// Player's class
/*0332*/ uint8	eyecolor2;		// Left eye color
/*0333*/ uint8	flymode;
/*0334*/ uint8	gender;			// Gender (0=male, 1=female)
/*0335*/ uint8	bodytype;		// Bodytype
/*0336*/ uint8 unknown0336[3];
union
{
/*0339*/ uint8 equip_chest2;	// Second place in packet for chest texture (usually 0xFF in live packets)
								// Not sure why there are 2 of them, but it effects chest texture!
/*0339*/ uint8 mount_color;		// drogmor: 0=white, 1=black, 2=green, 3=red
								// horse: 0=brown, 1=white, 2=black, 3=tan
};
/*0340*/ uint32 spawnId;		// Spawn Id
/*0344*/ uint8 unknown0344[3];
/*0347*/ uint8 IsMercenary;
/*0348*/ EQEmu::TintProfile equipment_tint;
/*0384*/ uint8	lfg;			// 0=off, 1=lfg on
/*0385*/

	bool DestructibleObject;	// Only used to flag as a destrible object
	char DestructibleModel[64];	// Model of the Destructible Object - Required - Seen "DEST_TNT_G"
	char DestructibleName2[64];	// Secondary name - Not Required - Seen "a_tent"
	char DestructibleString[64];	// Unknown - Not Required - Seen "ZoneActor_01186"
	uint32 DestructibleAppearance;	// Damage Appearance
	uint32 DestructibleUnk1;
	uint32 DestructibleID1;
	uint32 DestructibleID2;
	uint32 DestructibleID3;
	uint32 DestructibleID4;
	uint32 DestructibleUnk2;
	uint32 DestructibleUnk3;
	uint32 DestructibleUnk4;
	uint32 DestructibleUnk5;
	uint32 DestructibleUnk6;
	uint32 DestructibleUnk7;
	uint8 DestructibleUnk8;
	uint32 DestructibleUnk9;
	bool targetable_with_hotkey;
	bool show_name;

};

struct PlayerState_Struct {
/*00*/	uint32 spawn_id;
/*04*/	uint32 state;
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
/*0424*/	uint8	time_type;
/*0425*/    uint8   rain_chance[4];
/*0429*/    uint8   rain_duration[4];
/*0433*/    uint8   snow_chance[4];
/*0437*/    uint8   snow_duration[4];
/*0441*/	uint8	unknown360[33];
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
/*0520*/	uint8	unknown_end[84];		// ***Placeholder
/*0604*/	char	zone_short_name2[68];
/*0672*/	char	unknown672[12];
/*0684*/	uint16	zone_id;
/*0686*/	uint16	zone_instance;
/*0688*/	uint32	unknown688;
/*0692*/	uint8	unknown692[8];
/*0700*/	float	fog_density;
/*0704*/	uint32	SuspendBuffs;
/*0704*/
};

/*
** Memorize Spell Struct
** Length: 12 Bytes
**
*/
struct MemorizeSpell_Struct {
uint32 slot;		// Spot in the spell book/memorized slot
uint32 spell_id;	// Spell id (200 or c8 is minor healing, etc)
uint32 scribing;	// 1 if memorizing a spell, set to 0 if scribing to book, 2 if un-memming
uint32 reduction;	// lower reuse
};

/*
** Linked Spell Reuse Timer
** Length: 12
** Comes before the OP_Memorize
** Live (maybe TDS steam) has an extra DWORD after timer_id
*/
struct LinkedSpellReuseTimer_Struct {
	uint32 timer_id; // Timer ID of the spell
	uint32 end_time; // timestamp of when it will be ready
	uint32 start_time; // timestamp of when it started
};

/*
** Make Charmed Pet
** Length: 12 Bytes
**
*/
struct Charm_Struct {
/*00*/	uint32	owner_id;
/*04*/	uint32	pet_id;
/*08*/	uint32	command; // 1: make pet, 0: release pet
/*12*/
};

struct InterruptCast_Struct
{
	uint32 spawnid;
	uint32 messageid;
	char	message[0];
};

struct DeleteSpell_Struct
{
/*000*/int16	spell_slot;
/*002*/uint8	unknowndss002[2];
/*004*/uint8	success;
/*005*/uint8	unknowndss006[3];
/*008*/
};

struct ManaChange_Struct
{
/*00*/	uint32	new_mana;		// New Mana AMount
/*04*/	uint32	stamina;
/*08*/	uint32	spell_id;
/*12*/	uint8	keepcasting;	// won't stop the cast. Change mana while casting?
/*13*/	uint8	padding[3];		// client doesn't read it, garbage data seems like
};

struct SwapSpell_Struct
{
	uint32 from_slot;
	uint32 to_slot;


};

struct BeginCast_Struct
{
	// len = 8
/*000*/	uint16	caster_id;
/*002*/	uint16	spell_id;
/*004*/	uint32	cast_time;		// in miliseconds
};

struct CastSpell_Struct
{
	uint32	slot;
	uint32	spell_id;
	uint32	inventoryslot; // slot for clicky item, 0xFFFF = normal cast
	uint32	target_id;
	uint32  cs_unknown1;
	uint32  cs_unknown2;
 	float   y_pos;
 	float   x_pos;
	float   z_pos;
};

struct SpellEffect_Struct
{
/*000*/	uint32 EffectID;
/*004*/	uint32 EntityID;
/*008*/	uint32 EntityID2;	// EntityID again
/*012*/	uint32 Duration;		// In Milliseconds
/*016*/	uint32 FinishDelay;	// In Milliseconds - delay for final part of spell effect
/*020*/	uint32 Unknown020;	// Seen 3000
/*024*/ uint8 Unknown024;	// Seen 1 for SoD
/*025*/ uint8 Unknown025;	// Seen 1 for Live
/*026*/ uint16 Unknown026;	// Seen 1157 and 1177 - varies per char
/*028*/
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
/*0000*/ uint16 spawn_id;		// ID of the spawn
/*0002*/ uint16 type;			// Values associated with the type
/*0004*/ uint32 parameter;		// Type of data sent
/*0008*/
};


// this is used inside profile
struct SpellBuff_Struct
{
/*000*/	uint8	effect_type;	// 0 = no buff, 2 = buff, 4 = inverse affects of buff
/*001*/	uint8	level;
/*002*/	uint8	bard_modifier;
/*003*/	uint8	unknown003;		// MQ2 used to call this "damage shield" -- don't see client referencing it, so maybe server side DS type tracking? -- OSX client calls this "activated"
/*004*/	uint32	spellid;
/*008*/	int32	duration;
/*012*/	uint32	counters;	// single book keeping value (counters, rune/vie)
/*016*/	uint32	player_id;	// caster ID, pretty sure just zone ID
// extra stuff for newer packets
/*020*/	uint32	num_hits;
/*024*/	float	y;				// referenced by SPA 441
/*028*/	float	x;				// unsure if all buffs get them
/*032*/	float	z;				// as valid data
/*036*/
};

struct SpellBuffPacket_Struct {
/*000*/	uint32 entityid;
/*004*/	SpellBuff_Struct buff;
/*040*/	uint32 slotid;
/*044*/	uint32 bufffade;
/*048*/
};

// Underfoot & later struct.
struct BuffRemoveRequest_Struct
{
/*00*/ uint32 SlotID;
/*04*/ uint32 EntityID;
/*08*/
 };

struct PetBuff_Struct {
/*000*/ uint32 petid;
/*004*/ uint32 spellid[PET_BUFF_COUNT];
/*124*/ int32 ticsremaining[PET_BUFF_COUNT];
/*244*/ uint32 buffcount;
};

struct BlockedBuffs_Struct
{
/*00*/ int32 SpellID[BLOCKED_BUFF_COUNT];
/*80*/ uint32 Count;
/*84*/ uint8 Pet;
/*85*/ uint8 Initialise;
/*86*/ uint16 Flags;
};

// same for adding
struct RemoveNimbusEffect_Struct
{
/*00*/ uint32 spawnid;			// Spawn ID
/*04*/ int32 nimbus_effect;	// Nimbus Effect Number
/*08*/
};

struct ItemNamePacket_Struct {
/*000*/	uint32 item_id;
/*004*/	uint32 unkown004;
/*008*/ char name[64];
/*072*/
};

// Length: 10
struct ItemProperties_Struct {

uint8	unknown01[2];
uint8	charges;
uint8	unknown02[13];
};

struct GMTrainee_Struct
{
	/*000*/ uint32 npcid;
	/*004*/ uint32 playerid;
	/*008*/ uint32 skills[PACKET_SKILL_ARRAY_SIZE];
	/*408*/ uint8 unknown408[40];
	/*448*/
};

struct GMTrainEnd_Struct
{
	/*000*/ uint32 npcid;
	/*004*/ uint32 playerid;
	/*008*/
};

struct GMSkillChange_Struct {
/*000*/	uint16		npcid;
/*002*/ uint8		unknown1[2];	// something like PC_ID, but not really. stays the same thru the session though
/*002*/ uint16		skillbank;		// 0 if normal skills, 1 if languages
/*002*/ uint8		unknown2[2];
/*008*/ uint16		skill_id;
/*010*/ uint8		unknown3[2];
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
	uint8 permission;
	char zonename[32];
};

/*
** Name Generator Struct
** Length: 72 bytes
** OpCode: 0x0290
*/
struct NameGeneration_Struct
{
/*0000*/	uint32	race;
/*0004*/	uint32	gender;
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
/*0000*/	uint32	class_;
/*0004*/	uint32	haircolor;	// Might be hairstyle
/*0008*/	uint32	beardcolor;	// Might be beard
/*0012*/	uint32	beard;		// Might be beardcolor
/*0016*/	uint32	gender;
/*0020*/	uint32	race;
/*0024*/	uint32	start_zone;
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
/*0028*/	uint32	hairstyle;	// Might be haircolor
/*0032*/	uint32	deity;
/*0036*/	uint32	STR;
/*0040*/	uint32	STA;
/*0044*/	uint32	AGI;
/*0048*/	uint32	DEX;
/*0052*/	uint32	WIS;
/*0056*/	uint32	INT;
/*0060*/	uint32	CHA;
/*0064*/	uint32	face;		// Could be unknown0076
/*0068*/	uint32	eyecolor1;	//its possiable we could have these switched
/*0073*/	uint32	eyecolor2;	//since setting one sets the other we really can't check
/*0076*/	uint32	drakkin_heritage;	// added for SoF
/*0080*/	uint32	drakkin_tattoo;		// added for SoF
/*0084*/	uint32	drakkin_details;	// added for SoF
/*0088*/	uint32	tutorial;
};

/*
 *Used in PlayerProfile
 */
struct AA_Array
{
	uint32 AA;
	uint32 value;
	uint32 charges;
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

static const uint32 MAX_PP_DISCIPLINES = 100;
static const uint32 MAX_DISCIPLINE_TIMERS = 20;

struct Disciplines_Struct {
	uint32 values[MAX_PP_DISCIPLINES];
};

static const uint32 TRIBUTE_NONE = 0xFFFFFFFF;

struct Tribute_Struct {
	uint32 tribute;
	uint32 tier;
};

// Bandolier item positions
enum
{
	bandolierPrimary = 0,
	bandolierSecondary,
	bandolierRange,
	bandolierAmmo
};

//len = 72
struct BandolierItem_Struct
{
	uint32 ID;
	uint32 Icon;
	char Name[64];
};

//len = 320
struct Bandolier_Struct
{
	char Name[32];
	BandolierItem_Struct Items[EQEmu::legacy::BANDOLIER_ITEM_COUNT];
};

//len = 72
struct PotionBeltItem_Struct
{
	uint32 ID;
	uint32 Icon;
	char Name[64];
};

//len = 288
struct PotionBelt_Struct
{
	PotionBeltItem_Struct Items[EQEmu::legacy::POTION_BELT_ITEM_COUNT];
};

struct MovePotionToBelt_Struct
{
	uint32	Action;
	uint32	SlotNumber;
	uint32	ItemID;
};

static const uint32 MAX_GROUP_LEADERSHIP_AA_ARRAY = 16;
static const uint32 MAX_RAID_LEADERSHIP_AA_ARRAY = 16;
static const uint32 MAX_LEADERSHIP_AA_ARRAY = (MAX_GROUP_LEADERSHIP_AA_ARRAY+MAX_RAID_LEADERSHIP_AA_ARRAY);
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
			GroupLeadershipAA_Struct group;
			RaidLeadershipAA_Struct raid;
		};
		uint32 ranks[MAX_LEADERSHIP_AA_ARRAY];
	};
};

 /**
* A bind point.
* Size: 20 Octets
*/
struct BindStruct {
	/*000*/ uint32 zoneId;
	/*004*/ float x;
	/*008*/ float y;
	/*012*/ float z;
	/*016*/ float heading;
	/*020*/ uint32 instance_id;
	/*024*/
};

struct SuspendedMinion_Struct
{
	/*000*/	uint16 SpellID;
	/*002*/	uint32 HP;
	/*006*/	uint32 Mana;
	/*010*/	SpellBuff_Struct Buffs[BUFF_COUNT];
	/*510*/	EQEmu::TextureMaterialProfile Items;
	/*546*/	char Name[64];
	/*610*/
};


/*
** Player Profile
**
** Length: 4308 bytes
** OpCode: 0x006a
 */
static const uint32 MAX_PP_LANGUAGE = 28;
static const uint32 MAX_PP_SPELLBOOK = 480;	// Set for all functions
static const uint32 MAX_PP_MEMSPELL = static_cast<uint32>(EQEmu::CastingSlot::MaxGems); // Set to latest client so functions can work right -- 12
static const uint32 MAX_PP_REF_SPELLBOOK = 480;	// Set for Player Profile size retain
static const uint32 MAX_PP_REF_MEMSPELL = 9; // Set for Player Profile size retain

static const uint32 MAX_PP_SKILL		= PACKET_SKILL_ARRAY_SIZE;	// 100 - actual skills buffer size
static const uint32 MAX_PP_INNATE_SKILL	= 25;
static const uint32 MAX_PP_AA_ARRAY		= 240;
static const uint32 MAX_GROUP_MEMBERS	= 6;
static const uint32 MAX_RECAST_TYPES	= 20;

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
	-e 's/uint32[ \t]*disciplines\[MAX_DISCIPLINES\]/Disciplines_Struct disciplines/g' \
	-e 's/aa_unspent/aapoints/g' \
	-e 's/aa_spent/aapoints_spent/g' \
	-e 's/InlineItem[ \t]*potionBelt\[MAX_POTIONS_IN_BELT\]/PotionBelt_Struct potionbelt/g' \
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
/*0120*/	BindStruct			binds[5];			// Bind points (primary is first, home city is fifth)
/*0220*/	uint32				deity;				// deity
/*0224*/	uint32				guild_id;
/*0228*/	uint32				birthday;			// characters bday
/*0232*/	uint32				lastlogin;			// last login or zone time
/*0236*/	uint32				timePlayedMin;		// in minutes
/*0240*/	uint8				pvp;
/*0241*/	uint8				level2;				//no idea why this is here, but thats how it is on live
/*0242*/	uint8				anon;				// 2=roleplay, 1=anon, 0=not anon
/*0243*/	uint8				gm;
/*0244*/	uint8				guildrank;
/*0245*/	uint8				guildbanker;
/*0246*/	uint8				unknown0246[6];		//
/*0252*/	uint32				intoxication;
/*0256*/	uint32				spellSlotRefresh[MAX_PP_MEMSPELL];	//in ms
/*0292*/	uint32				abilitySlotRefresh;
/*0296*/	uint8				haircolor;			// Player hair color
/*0297*/	uint8				beardcolor;			// Player beard color
/*0298*/	uint8				eyecolor1;			// Player left eye color
/*0299*/	uint8				eyecolor2;			// Player right eye color
/*0300*/	uint8				hairstyle;			// Player hair style
/*0301*/	uint8				beard;				// Beard type
/*0302*/	uint8				ability_time_seconds;	//The following four spots are unknown right now.....
/*0303*/	uint8				ability_number;		//ability used
/*0304*/	uint8				ability_time_minutes;
/*0305*/	uint8				ability_time_hours;	//place holder
/*0306*/	uint8				unknown0306[6];		// @bp Spacer/Flag?
/*0312*/	EQEmu::TextureMaterialProfile	item_material;	// Item texture/material of worn/held items
/*0348*/	uint8				unknown0348[44];
/*0392*/	EQEmu::TintProfile	item_tint;
/*0428*/	AA_Array			aa_array[MAX_PP_AA_ARRAY];
/*2348*/	float				unknown2384;		//seen ~128, ~47
/*2352*/	char				servername[32];		// length probably not right
/*2384*/	char				title[32];			// length might be wrong
/*2416*/	char				suffix[32];			// length might be wrong
/*2448*/	uint32				guildid2;			//
/*2452*/	uint32				exp;				// Current Experience
/*2456*/	uint32				unknown2492;
/*2460*/	uint32				points;				// Unspent Practice points
/*2464*/	uint32				mana;				// current mana
/*2468*/	uint32				cur_hp;				// current hp
/*2472*/	uint32				unknown2508;		// 0x05
/*2476*/	uint32				STR;				// Strength
/*2480*/	uint32				STA;				// Stamina
/*2484*/	uint32				CHA;				// Charisma
/*2488*/	uint32				DEX;				// Dexterity
/*2492*/	uint32				INT;				// Intelligence
/*2496*/	uint32				AGI;				// Agility
/*2500*/	uint32				WIS;				// Wisdom
/*2504*/	uint8				face;				// Player face
/*2505*/	uint8				unknown2541[47];	// ?
/*2552*/	uint8				languages[MAX_PP_LANGUAGE];
/*2580*/	uint8				unknown2616[4];
/*2584*/	uint32				spell_book[MAX_PP_REF_SPELLBOOK];
/*4504*/	uint8				unknown4540[128];	// Was [428] all 0xff
/*4632*/	uint32				mem_spells[MAX_PP_MEMSPELL];
/*4668*/	uint8				unknown4704[32];	//
/*4700*/	float				y;					// Player y position
/*4704*/	float				x;					// Player x position
/*4708*/	float				z;					// Player z position
/*4712*/	float				heading;			// Direction player is facing
/*4716*/	uint8				unknown4752[4];		//
/*4720*/	int32				platinum;			// Platinum Pieces on player
/*4724*/	int32				gold;				// Gold Pieces on player
/*4728*/	int32				silver;				// Silver Pieces on player
/*4732*/	int32				copper;				// Copper Pieces on player
/*4736*/	int32				platinum_bank;		// Platinum Pieces in Bank
/*4740*/	int32				gold_bank;			// Gold Pieces in Bank
/*4744*/	int32				silver_bank;		// Silver Pieces in Bank
/*4748*/	int32				copper_bank;		// Copper Pieces in Bank
/*4752*/	int32				platinum_cursor;	// Platinum on cursor
/*4756*/	int32				gold_cursor;		// Gold on cursor
/*4760*/	int32				silver_cursor;		// Silver on cursor
/*4764*/	int32				copper_cursor;		// Copper on cursor
/*4768*/	int32				platinum_shared;	// Platinum shared between characters
/*4772*/	uint8				unknown4808[24];
/*4796*/	uint32				skills[MAX_PP_SKILL];	// [400] List of skills	// 100 dword buffer
/*5196*/	uint32				InnateSkills[MAX_PP_INNATE_SKILL];
/*5296*/	uint8				unknown5132[84];
/*5380*/	uint32				pvp2;				//
/*5384*/	uint32				unknown5420;		//
/*5388*/	uint32				pvptype;			//
/*5392*/	uint32				unknown5428;		//
/*5396*/	uint32				ability_down;		// Guessing
/*5400*/	uint8				unknown5436[8];		//
/*5408*/	uint32				autosplit;			//not used right now
/*5412*/	uint8				unknown5448[8];
/*5420*/	uint32				zone_change_count;	// Number of times user has zoned in their career (guessing)
/*5424*/	uint8				unknown5460[16];	//
/*5440*/	uint32				drakkin_heritage;	//
/*5444*/	uint32				drakkin_tattoo;		//
/*5448*/	uint32				drakkin_details;	//
/*5452*/	uint32				expansions;			// expansion setting, bit field of expansions avaliable
/*5456*/	int32				toxicity;			//from drinking potions, seems to increase by 3 each time you drink
/*5460*/	char				unknown5496[16];	//
/*5476*/	int32				hunger_level;
/*5480*/	int32				thirst_level;
/*5484*/	uint32				ability_up;
/*5488*/	char				unknown5524[16];
/*5504*/	uint16				zone_id;			// Current zone of the player
/*5506*/	uint16				zoneInstance;		// Instance ID
/*5508*/	SpellBuff_Struct	buffs[BUFF_COUNT];	// Buffs currently on the player
/*6008*/	char				groupMembers[6][64];//
/*6392*/	char				unknown6428[656];
/*7048*/	uint32				entityid;
/*7052*/	uint32				leadAAActive;
/*7056*/	uint32				unknown7092;
/*7060*/	int32				ldon_points_guk;	//client uses these as signed
/*7064*/	int32				ldon_points_mir;
/*7068*/	int32				ldon_points_mmc;
/*7072*/	int32				ldon_points_ruj;
/*7076*/	int32				ldon_points_tak;
/*7080*/	int32				ldon_points_available;
/*7084*/	int32				ldon_wins_guk;
/*7088*/	int32				ldon_wins_mir;
/*7092*/	int32				ldon_wins_mmc;
/*7096*/	int32				ldon_wins_ruj;
/*7100*/	int32				ldon_wins_tak;
/*7104*/	int32				ldon_losses_guk;
/*7108*/	int32				ldon_losses_mir;
/*7112*/	int32				ldon_losses_mmc;
/*7116*/	int32				ldon_losses_ruj;
/*7120*/	int32				ldon_losses_tak;
/*7124*/	uint8				unknown7160[72];
/*7196*/	uint32				tribute_time_remaining;	//in miliseconds
/*7200*/	uint32				showhelm;
/*7204*/	uint32				career_tribute_points;
/*7208*/	uint32				unknown7244;
/*7212*/	uint32				tribute_points;
/*7216*/	uint32				unknown7252;
/*7220*/	uint32				tribute_active;		//1=active
/*7224*/	Tribute_Struct		tributes[EQEmu::legacy::TRIBUTE_SIZE];
/*7264*/	Disciplines_Struct	disciplines;
/*7664*/	uint32				recastTimers[MAX_RECAST_TYPES];	// Timers (GMT of last use)
/*7744*/	char				unknown7780[160];
/*7904*/	uint32				endurance;
/*7908*/	uint32				group_leadership_exp;	//0-1000
/*7912*/	uint32				raid_leadership_exp;	//0-2000
/*7916*/	uint32				group_leadership_points;
/*7920*/	uint32				raid_leadership_points;
/*7924*/	LeadershipAA_Struct	leader_abilities;
/*8052*/	uint8				unknown8088[132];
/*8184*/	uint32				air_remaining;
/*8188*/	uint32				PVPKills;
/*8192*/	uint32				PVPDeaths;
/*8196*/	uint32				PVPCurrentPoints;
/*8200*/	uint32				PVPCareerPoints;
/*8204*/	uint32				PVPBestKillStreak;
/*8208*/	uint32				PVPWorstDeathStreak;
/*8212*/	uint32				PVPCurrentKillStreak;
/*8216*/	PVPStatsEntry_Struct	PVPLastKill;
/*8304*/	PVPStatsEntry_Struct	PVPLastDeath;
/*8392*/	uint32				PVPNumberOfKillsInLast24Hours;
/*8396*/	PVPStatsEntry_Struct	PVPRecentKills[50];
/*12796*/	uint32				aapoints_spent;
/*12800*/	uint32				expAA;
/*12804*/	uint32				aapoints;			//avaliable, unspent
/*12808*/	uint8				unknown12844[36];
/*12844*/	Bandolier_Struct	bandoliers[EQEmu::legacy::BANDOLIERS_SIZE];
/*14124*/	uint8				unknown14160[4506];
/*18630*/	SuspendedMinion_Struct	SuspendedMinion; // No longer in use
/*19240*/	uint32				timeentitledonaccount;
/*19244*/	PotionBelt_Struct	potionbelt;			//there should be 3 more of these
/*19532*/	uint8				unknown19568[8];
/*19540*/	uint32				currentRadCrystals; // Current count of radiant crystals
/*19544*/	uint32				careerRadCrystals;	// Total count of radiant crystals ever
/*19548*/	uint32				currentEbonCrystals;// Current count of ebon crystals
/*19552*/	uint32				careerEbonCrystals;	// Total count of ebon crystals ever
/*19556*/	uint8				groupAutoconsent;	// 0=off, 1=on
/*19557*/	uint8				raidAutoconsent;	// 0=off, 1=on
/*19558*/	uint8				guildAutoconsent;	// 0=off, 1=on
/*19559*/	uint8				unknown19595[5];	// ***Placeholder (6/29/2005)
/*19564*/	uint32				RestTimer;
/*19568*/
};




/*
** Client Target Struct
** Length: 2 Bytes
** OpCode: 6221
*/
struct ClientTarget_Struct {
/*000*/	uint32	new_target;			// Target ID
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
/*000*/ uint32	command;
/*004*/ uint32	target;
};

struct PetCommandState_Struct {
/*00*/	uint32	button_id;
/*04*/	uint32	state;
};

/*
** Delete Spawn
** Length: 4 Bytes
** OpCode: OP_DeleteSpawn
*/
struct DeleteSpawn_Struct
{
/*00*/ uint32 spawn_id;		// Spawn ID to delete
/*04*/ uint8 Decay;			// 0 = vanish immediately, 1 = 'Decay' sparklies for corpses.
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
/*128*/	uint32	language;			// Language
/*132*/	uint32	chan_num;			// Channel
/*136*/	uint32	cm_unknown4[2];		// ***Placeholder
/*144*/	uint32	skill_in_language;	// The players skill in this language? might be wrong
/*148*/	char	message[0];			// Variable length message
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
** When somebody changes what they're wearing or give a pet a weapon (model changes)
** Length: 19 Bytes
*/
struct WearChange_Struct{
/*000*/ uint16 spawn_id;
/*002*/ uint32 material;
/*006*/ uint32 unknown06;
/*010*/ uint32 elite_material;	// 1 for Drakkin Elite Material
/*014*/ uint32 hero_forge_model; // New to VoA
/*018*/ uint32 unknown18; // New to RoF
/*022*/ EQEmu::textures::Tint_Struct color;
/*026*/ uint8 wear_slot_id;
/*027*/
};

/*
** Type: Bind Wound Structure
** Length: 8 Bytes
*/
//Fixed for 7-14-04 patch
struct BindWound_Struct
{
/*002*/	uint16	to;			// TargetID
/*004*/	uint16	unknown2;		// ***Placeholder
/*006*/	uint16	type;
/*008*/	uint16	unknown6;
};


/*
** Type: Zone Change Request (before hand)
** Length: 88 bytes
** OpCode: a320
*/

struct ZoneChange_Struct {
/*000*/	char	char_name[64];	// Character Name
/*064*/	uint16	zoneID;
/*066*/	uint16	instanceID;
/*068*/	float	y;
/*072*/	float	x;
/*076*/	float	z;
/*080*/	uint32	zone_reason;	//0x0A == death, I think
/*084*/	int32	success;		// =0 client->server, =1 server->client, -X=specific error
/*088*/
};

// Whatever you send to the client in RequestClientZoneChange_Struct.type, the client will send back
// to the server in ZoneChange_Struct.zone_reason. My guess is this is a memo field of sorts.
// 27 January 2008

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
/*00*/	uint16 spawnid;
/*02*/	uint8 speed;
/*03*/	uint8 action;
/*04*/
};

// this is what causes the caster to animate and the target to
// get the particle effects around them when a spell is cast
// also causes a buff icon
struct Action_Struct
{
 /* 00 */	uint16 target;	// id of target
 /* 02 */	uint16 source;	// id of caster
 /* 04 */	uint16 level; // level of caster
 /* 06 */	uint16 instrument_mod;
 /* 08 */	uint32 bard_focus_id;
 /* 12 */	uint16 unknown16;
// some kind of sequence that's the same in both actions
// as well as the combat damage, to tie em together?
 /* 14 */	uint32 sequence;
 /* 18 */	uint32 unknown18;
 /* 22 */	uint8 type;		// 231 (0xE7) for spells
 /* 23 */	uint32 unknown23;
 /* 27 */	uint16 spell;	// spell id being cast
 /* 29 */	uint8 unknown29;
// this field seems to be some sort of success flag, if it's 4
 /* 30 */	uint8 buff_unknown;	// if this is 4, a buff icon is made
 /* 31 */
};

// this is what prints the You have been struck. and the regular
// melee messages like You try to pierce, etc. It's basically the melee
// and spell damage message
struct CombatDamage_Struct
{
/* 00 */	uint16	target;
/* 02 */	uint16	source;
/* 04 */	uint8	type; //slashing, etc. 231 (0xE7) for spells
/* 05 */	uint16	spellid;
/* 07 */	uint32	damage;
/* 11 */	float force;
/* 15 */	float meleepush_xy;	// see above notes in Action_Struct
/* 19 */	float meleepush_z;
/* 23 */	uint32 special; // 2 = Rampage, 1 = Wild Rampage
};

/*
** Consider Struct
*/
struct Consider_Struct{
/*000*/ uint32	playerid;		// PlayerID
/*004*/ uint32	targetid;		// TargetID
/*008*/ uint32	faction;		// Faction
/*0012*/ uint32	level;			// Level
/*016*/ int32	cur_hp;			// Current Hitpoints
/*020*/ int32	max_hp;			// Maximum Hitpoints
/*024*/ uint8 pvpcon;			// Pvp con flag 0/1
/*025*/ uint8	unknown3[3];
};

/*
** Spawn Death Blow
** Length: 32 Bytes
** OpCode: 0114
*/
struct Death_Struct
{
/*000*/	uint32	spawn_id;
/*004*/	uint32	killer_id;
/*008*/	uint32	corpseid;	// was corpseid
/*012*/	uint32	bindzoneid;
/*016*/	uint32	spell_id;
/*020*/	uint32	attack_skill;
/*024*/	uint32	damage;
/*028*/	uint32	unknown028;
};

struct BecomeCorpse_Struct {
	uint32	spawn_id;
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
/*0002*/ int32		delta_heading:10,	// change in heading
					x_pos:19,			// x coord
					padding0002:3;		// ***Placeholder
/*0006*/ int32		y_pos:19,			// y coord
					animation:10,		// animation
					padding0006:3;		// ***Placeholder
/*0010*/ int32		z_pos:19,			// z coord
					delta_y:13;			// change in y
/*0014*/ int32		delta_x:13,			// change in x
					heading:12,			// heading
					padding0014:7;		// ***Placeholder
/*0018*/ int32		delta_z:13,			// change in z
					padding0018:19;		// ***Placeholder
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
/*0002*/ uint16	sequence;			//increments one each packet
/*0004*/ float	y_pos;				// y coord
/*0008*/ float	delta_z;			// Change in z
/*0012*/ float	delta_x;			// Change in x
/*0016*/ float	delta_y;			// Change in y
/*0020*/ int32	animation:10,		// animation
				delta_heading:10,	// change in heading
				padding0020:12;		// ***Placeholder (mostly 1)
/*0024*/ float	x_pos;				// x coord
/*0028*/ float	z_pos;				// z coord
/*0032*/ uint16	heading:12,			// Directional heading
				padding0004:4;		// ***Placeholder
/*0034*/ uint8	unknown0006[2];		// ***Placeholder
/*0036*/
};

struct SpawnPositionUpdate_Struct
{
/*0000*/ uint16		spawn_id;
/*0002*/ uint64		y_pos:19, z_pos:19, x_pos:19, padding002:7;
/*0010*/ unsigned	heading:12;
/*0012*/ signed		padding010:4;
};

/*
** Spawn HP Update
** Length: 10 Bytes
** OpCode: OP_HPUpdate
*/
struct SpawnHPUpdate_Struct
{
/*00*/ uint32	cur_hp;		// Id of spawn to update
/*04*/ int32	max_hp;		// Maximum hp of spawn
/*08*/ int16	spawn_id;	// Current hp of spawn
/*10*/
};

struct ManaUpdate_Struct
{
/*00*/ uint32	cur_mana;
/*04*/ uint32	max_mana;
/*08*/ uint16	spawn_id;
/*10*/
};

struct EnduranceUpdate_Struct
{
/*00*/ uint32	cur_end;
/*04*/ uint32	max_end;
/*08*/ uint16	spawn_id;
/*10*/
};

struct SpawnHPUpdate_Struct2
{
/*00*/ int16	spawn_id;
/*02*/ uint8		hp;			//HP Percentage
/*03*/
};

struct MobManaUpdate_Struct
{
/*00*/ uint16	spawn_id;
/*02*/ uint8		mana;		//Mana Percentage
/*03*/
};

struct MobEnduranceUpdate_Struct
{
/*00*/ uint16	spawn_id;
/*02*/ uint8		endurance;	//Endurance Percentage
/*03*/
};

// Is this even used?
struct MobHealth
{
	/*0000*/	uint8	hp;	//health percent
	/*0001*/	uint16	id;	//mobs id
};

/*
** Stamina
** Length: 8 Bytes
** OpCode: 5721
*/
struct Stamina_Struct {
/*00*/ uint32 food;		// (low more hungry 127-0)
/*02*/ uint32 water;	// (low more thirsty 127-0)
};

/*
** Level Update
** Length: 12 Bytes
*/
struct LevelUpdate_Struct
{
/*00*/ uint32 level;		// New level
/*04*/ uint32 level_old;	// Old level
/*08*/ uint32 exp;			// Current Experience
};

/*
** Experience Update
** Length: 14 Bytes
** OpCode: 9921
*/
struct ExpUpdate_Struct
{
/*0000*/ uint32 exp;	// Current experience ratio from 0 to 330
/*0004*/ uint32 aaxp;	// @BP ??
};

/*
** Item Packet Struct - Works on a variety of opcodes
** Packet Types: See ItemPacketType enum
**
*/
enum ItemPacketType
{
	ItemPacketViewLink			= 0x00,
	ItemPacketMerchant			= 0x64,
	ItemPacketTradeView			= 0x65,
	ItemPacketLoot				= 0x66,
	ItemPacketTrade				= 0x67,
	ItemPacketCharInventory		= 0x69,
	ItemPacketLimbo				= 0x6A,
	ItemPacketWorldContainer	= 0x6B,
	ItemPacketTributeItem		= 0x6C,
	ItemPacketGuildTribute		= 0x6D,
	ItemPacketCharmUpdate		= 0x6E, // noted as incorrect
	ItemPacketInvalid			= 0xFF
};

//enum ItemPacketType
//{
//	ItemPacketMerchant			= /*100*/ 0x64, // Titanium+
//	ItemPacketTradeView			= /*101*/ 0x65,
//	ItemPacketLoot				= /*102*/ 0x66,
//	ItemPacketTrade				= /*103*/ 0x67,
//	ItemPacketCharInventory		= /*105*/ 0x69, // 105 looks like raw item packet (no appearance update) thru shared bank..110, possibly possessions with appearance update
//	ItemPacketLimbo				= /*106*/ 0x6A,
//	ItemPacketWorldContainer	= /*107*/ 0x6B,
//	ItemPacketTributeItem		= /*108*/ 0x6C,
//	ItemPacketGuildTribute		= /*109*/ 0x6D, // missing from EQEmu
//	ItemPacket10				= /*110*/ 0x6E,
//	ItemPacket11				= /*111*/ 0x6F, // UF+ (equipment slots only) (RoF+ checks '(WORD*)slot + 4 != -1' [(WORD*)]slot + 2 would be bag index - if used) (guess) (appearance (over-level) items?)
//	ItemPacket12				= /*112*/ 0x70, // RoF+ (causes stat update) (could be TrophyTribute and GuildTrophyTribute together - two case methodology - is it checking for GuildID?)
//	ItemPacketMerchantRecovery	= /*113*/ 0x71,
//	ItemPacket14				= /*115*/ 0x73, (real estate/moving crate?)
//	ItemPacket__				= /*xxx*/ 0xXX // switch 'default' - all clients
//};

struct ItemPacket_Struct
{
/*00*/	ItemPacketType	PacketType;
/*04*/	char			SerializedItem[1];
/*xx*/
};

struct BulkItemPacket_Struct
{
/*00*/	char			SerializedItem[0];
/*xx*/
};

struct Consume_Struct
{
/*0000*/ uint32	slot;
/*0004*/ uint32	auto_consumed; // 0xffffffff when auto eating e7030000 when right click
/*0008*/ uint8	c_unknown1[4];
/*0012*/ uint8	type; // 0x01=Food 0x02=Water
/*0013*/ uint8	unknown13[3];
};

struct DeleteItem_Struct {
/*0000*/ uint32	from_slot;
/*0004*/ uint32	to_slot;
/*0008*/ uint32	number_in_stack;
/*0012*/
};

struct MoveItem_Struct
{
/*0000*/ uint32 from_slot;
/*0004*/ uint32 to_slot;
/*0008*/ uint32 number_in_stack;
/*0012*/
};

// both MoveItem_Struct/DeleteItem_Struct server structures will be changing to a structure-based slot format..this will
// be used for handling SoF/SoD/etc... time stamps sent using the MoveItem_Struct format. (nothing will be done with this
// info at the moment..but, it is forwarded on to the server for handling/future use)
struct ClientTimeStamp_Struct
{
/*0000*/ uint32	from_slot;
/*0004*/ uint32	to_slot;
/*0008*/ uint32	number_in_stack;
/*0012*/
};

//
// from_slot/to_slot
// -1 - destroy
// 0 - cursor
// 1 - inventory
// 2 - bank
// 3 - trade
// 4 - shared bank
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
	int32 from_slot;
	int32 to_slot;
	uint32 cointype1;
	uint32 cointype2;
	int32	amount;
};
struct TradeCoin_Struct{
	uint32	trader;
	uint8	slot;
	uint16	unknown5;
	uint8	unknown7;
	uint32	amount;
};
struct TradeMoneyUpdate_Struct{
	uint32	trader;
	uint32	type;
	uint32	amount;
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
	uint8 head[64]; // First on guild list seems to be empty...
	GuildsListEntry_Struct Guilds[MAX_NUMBER_GUILDS];
};

struct GuildUpdate_Struct {
	uint32	guildID;
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
/*000*/	uint32	lootee;
/*002*/	uint32	looter;
/*004*/	uint16	slot_id;
/*006*/	uint8	unknown3[2];
/*008*/	int32	auto_loot;
};

struct GuildManageStatus_Struct{
	uint32	guild_id;
	uint32	oldrank;
	uint32	newrank;
	char	name[64];
};
// Guild invite, remove
struct GuildJoin_Struct{
/*000*/	uint32	guild_id;
/*004*/	uint32	unknown04;
/*008*/	uint32	level;
/*012*/	uint32	class_;
/*016*/	uint32	rank;//0 member, 1 officer, 2 leader
/*020*/	uint32	zoneid;
/*024*/	uint32	unknown24;
/*028*/	char	name[64];
/*092*/
};
struct GuildInviteAccept_Struct {
	char inviter[64];
	char newmember[64];
	uint32 response;
	uint32 guildeqid;
};
struct GuildManageRemove_Struct {
	uint32 guildeqid;
	char member[64];
};
struct GuildCommand_Struct {
	char othername[64];
	char myname[64];
	uint16 guildeqid;
	uint8 unknown[2]; // for guildinvite all 0's, for remove 0=0x56, 2=0x02
	uint32 officer;
};

struct OnLevelMessage_Struct
{
	char	Title[128];
	char	Text[4096];
	uint32	Buttons;
	uint32	Duration;
	uint32	PopupID;
	uint32	NegativeID;
	uint32  SoundControls;
	char	ButtonName0[25];
	char	ButtonName1[25];
};

struct PopupResponse_Struct {
/*0000*/	uint32	unknown0000;
/*0004*/	uint32	popupid;
};

struct GuildManageBanker_Struct {
	uint32 unknown0;
	char myname[64];
	char member[64];
	uint32	enabled;
};

// Server -> Client
// Update a guild members rank and banker status
struct GuildSetRank_Struct
{
/*00*/	uint32	Unknown00;
/*04*/	uint32	Unknown04;
/*08*/	uint32	Rank;
/*12*/	char	MemberName[64];
/*76*/	uint32	Banker;
/*80*/
};

// Opcode OP_GMZoneRequest
// Size = 88 bytes
struct GMZoneRequest_Struct {
/*0000*/	char	charname[64];
/*0064*/	uint32	zone_id;
/*0068*/	float	x;
/*0072*/	float	y;
/*0076*/	float	z;
/*0080*/	char	unknown0080[4];
/*0084*/	uint32	success;		// 0 if command failed, 1 if succeeded?
/*0088*/
//	/*072*/	int8	success;		// =0 client->server, =1 server->client, -X=specific error
//	/*073*/	uint8	unknown0073[3]; // =0 ok, =ffffff error
};

struct GMSummon_Struct {
/*  0*/	char	charname[64];
/* 30*/	char	gmname[64];
/* 60*/	uint32	success;
/* 61*/	uint32	zoneID;
/*92*/	float	y;
/*96*/	float	x;
/*100*/	float	z;
/*104*/	uint32	unknown2; // E0 E0 56 00
};

struct GMGoto_Struct { // x,y is swapped as compared to summon and makes sense as own packet
/*  0*/ char	charname[64];

/* 64*/ char	gmname[64];
/* 128*/uint32	success;
/* 132*/ uint32	zoneID;

/*136*/ int32	y;
/*140*/ int32	x;
/*144*/ int32	z;
/*148*/ uint32	unknown2; // E0 E0 56 00
};

struct GMLastName_Struct {
	char name[64];
	char gmname[64];
	char lastname[64];
	uint16 unknown[4];	// 0x00, 0x00
						// 0x01, 0x00 = Update the clients
};

//Combat Abilities
struct CombatAbility_Struct {
	uint32 m_target;		//the ID of the target mob
	uint32 m_atk;
	uint32 m_skill;
};

//Instill Doubt
struct Instill_Doubt_Struct {
	uint8 i_id;
	uint8 ia_unknown;
	uint8 ib_unknown;
	uint8 ic_unknown;
	uint8 i_atk;

	uint8 id_unknown;
	uint8 ie_unknown;
	uint8 if_unknown;
	uint8 i_type;
	uint8 ig_unknown;
	uint8 ih_unknown;
	uint8 ii_unknown;
};

struct GiveItem_Struct {
	uint16 to_entity;
	int16 to_equipSlot;
	uint16 from_entity;
	int16 from_equipSlot;
};

struct RandomReq_Struct {
	uint32 low;
	uint32 high;
};

/* 9/23/03 reply to /random command */
struct RandomReply_Struct {
/* 00 */	uint32 low;
/* 04 */	uint32 high;
/* 08 */	uint32 result;
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
/*005*/	uint8 MatchFilter;
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
	uint8	hour;
	uint8	minute;
	uint8	day;
	uint8	month;
	uint32	year;
};

// Darvik: shopkeeper structs
struct Merchant_Click_Struct {
/*000*/ uint32	npcid;			// Merchant NPC's entity id
/*004*/ uint32	playerid;
/*008*/ uint32	command;		//1=open, 0=cancel/close
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
/*000*/	uint32	npcid;			// Merchant NPC's entity id
/*004*/	uint32	playerid;		// Player's entity id
/*008*/	uint32	itemslot;
		uint32	unknown12;
/*016*/	uint32	quantity;
/*020*/ uint32	price;
};
struct Merchant_Purchase_Struct {
/*000*/	uint32	npcid;			// Merchant NPC's entity id
/*004*/	uint32	itemslot;		// Player's entity id
/*008*/	uint32	quantity;
/*012*/	uint32	price;
};
struct Merchant_DelItem_Struct{
/*000*/	uint32	npcid;			// Merchant NPC's entity id
/*004*/	uint32	playerid;		// Player's entity id
/*008*/	uint32	itemslot;
/*012*/	uint32	unknown012;
};
struct Adventure_Purchase_Struct {
/*000*/	uint32	Type;	// 1 = LDoN, 2 = Discord, 4 = Norrath's Keepers, 5 = Dark Reign
/*000*/	uint32	npcid;
/*004*/	uint32	itemid;
/*008*/	uint32	variable;
};

struct Adventure_Sell_Struct {
/*000*/	uint32	unknown000;	//0x01
/*004*/	uint32	npcid;
/*008*/	uint32	slot;
/*012*/	uint32	charges;
/*016*/	uint32	sell_price;
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

struct Weblink_Struct{
/*000*/ char weblink[1];
/*004*/
};

//OP_AdventureRequest
struct AdventureRequest_Struct{
/*000*/	uint32 risk;//1 normal,2 hard.
/*004*/	uint32 entity_id;
/*008*/	uint32 type;
/*012*/
};
struct AdventureRequestResponse_Struct{
/*0000*/ uint32 unknown000;
/*0004*/ char text[2048];
/*2052*/ uint32 timetoenter;
/*2056*/ uint32 timeleft;
/*2060*/ uint32 risk;
/*2064*/ float x;
/*2068*/ float y;
/*2072*/ float z;
/*2076*/ uint32 showcompass;
/*2080*/ uint32 unknown2080;
/*2084*/
};

struct AdventureCountUpdate_Struct
{
/*000*/ uint32 current;
/*004*/	uint32 total;
/*008*/
};

struct AdventureStatsColumn_Struct
{
/*000*/ uint32 total;
/*004*/	uint32 guk;
/*008*/	uint32 mir;
/*012*/	uint32 mmc;
/*016*/	uint32 ruj;
/*020*/	uint32 tak;
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
/*000*/ char name[64];
/*064*/ uint32 success;
/*068*/ uint32 failure;
/*072*/
};

struct AdventureLeaderboardRequest_Struct
{
/*000*/ uint32 type;
/*004*/ uint32 theme;
/*008*/ uint32 risk;
/*012*/
};

struct AdventureLeaderboard_Struct
{
/*000*/ uint32 unknown000;
/*004*/ uint32 unknown004;
/*008*/ uint32 success;
/*012*/ uint32 failure;
/*016*/ uint32 our_rank;
/*020*/	AdventureLeaderboardEntry_Struct entries[100];
};

/*struct Item_Shop_Struct {
	uint16 merchantid;
	uint8 itemtype;
	EQEmu::ItemData item;
	uint8 iss_unknown001[6];
};*/

struct Illusion_Struct { //size: 256 - SoF
/*000*/	uint32	spawnid;
/*004*/	char charname[64];		//
/*068*/	uint16	race;			//
/*070*/	char	unknown006[2];
/*072*/	uint8	gender;
/*073*/	uint8	texture;
/*074*/	uint8	unknown008;		//
/*075*/	uint8	unknown009;		//
/*076*/	uint8	helmtexture;	//
/*077*/	uint8	unknown010;		//
/*078*/	uint8	unknown011;		//
/*079*/	uint8	unknown012;		//
/*080*/	uint32	face;			//
/*084*/	uint8	hairstyle;		//
/*085*/	uint8	haircolor;		//
/*086*/	uint8	beard;			//
/*087*/	uint8	beardcolor;		//
/*088*/	float	size;			//
/*092*/	uint32	drakkin_heritage;	//
/*096*/	uint32	drakkin_tattoo;		//
/*100*/	uint32	drakkin_details;	//
/*104*/	EQEmu::TintProfile	armor_tint;	//
/*140*/	uint8	eyecolor1;		// Field Not Identified in any Illusion Struct
/*141*/	uint8	eyecolor2;		// Field Not Identified in any Illusion Struct
/*142*/	uint8	unknown138[114];	//
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

// OP_Sound - Size: 68
struct QuestReward_Struct
{
	/*000*/ uint32	mob_id;	// ID of mob awarding the client
	/*004*/ uint32	target_id;
	/*008*/ uint32	exp_reward;
	/*012*/ uint32	faction;
	/*016*/ int32	faction_mod;
	/*020*/ uint32	copper;		// Gives copper to the client
	/*024*/ uint32	silver;		// Gives silver to the client
	/*028*/ uint32	gold;		// Gives gold to the client
	/*032*/ uint32	platinum;	// Gives platinum to the client
	/*036*/ uint32	item_id;
	/*040*/ uint32	unknown040;
	/*044*/ uint32	unknown044;
	/*048*/ uint32	unknown048;
	/*052*/ uint32	unknown052;
	/*056*/ uint32	unknown056;
	/*060*/ uint32	unknown060;
	/*064*/ uint32	unknown064;
	/*068*/
};

// Size: 8
struct Camera_Struct
{
	uint32	duration;	// Duration in ms
	uint32	intensity;	// Between 1023410176 and 1090519040
};

struct ZonePoint_Entry {
/*0000*/	uint32	iterator;
/*0004*/	float	y;
/*0008*/	float	x;
/*0012*/	float	z;
/*0016*/	float	heading;
/*0020*/	uint16	zoneid;
/*0022*/	uint16	zoneinstance; // LDoN instance
};

struct ZonePoints {
/*0000*/	uint32	count;
/*0004*/	struct	ZonePoint_Entry zpe[0]; // Always add one extra to the end after all zonepoints
};

struct SkillUpdate_Struct {
/*00*/	uint32 skillId;
/*04*/	uint32 value;
/*08*/
};

struct ZoneUnavail_Struct {
	//This actually varies, but...
	char zonename[16];
	int16 unknown[4];
};

enum {	//Group action fields
	groupActJoin = 0,
	groupActLeave = 1,
	groupActDisband = 6,
	groupActUpdate = 7,
	groupActMakeLeader = 8,
	groupActInviteInitial = 9,
	groupActAAUpdate = 10
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
/*0000*/	uint32	action;
/*0004*/	char	yourname[64];
/*0068*/	char	membername[5][64];
/*0388*/	char	leadersname[64];
};

struct GroupUpdate2_Struct {
/*0000*/	uint32	action;
/*0004*/	char	yourname[64];
/*0068*/	char	membername[5][64];
/*0388*/	char	leadersname[64];
/*0452*/	GroupLeadershipAA_Struct leader_aas;
/*0580*/	uint8	unknown580[196];
/*0766*/	uint32	NPCMarkerID;	// EntityID of player delegated MarkNPC ability
/*0780*/	uint8	unknown780[56];
/*0836*/
};
struct GroupJoin_Struct {
/*0000*/	uint32	action;
/*0004*/	char	yourname[64];
/*0068*/	char	membername[64];
/*0132*/	GroupLeadershipAA_Struct leader_aas;
/*0196*/	uint8	unknown196[196];
/*0392*/	uint32	NPCMarkerID;	// EntityID of player delegated MarkNPC ability
/*0396*/	uint8	unknown396[56];
/*0452*/
};

// SoD+ Struct
struct GroupLeadershipAAUpdate_Struct
{
/*000*/	uint32	Unknown000;	// GroupID or Leader EntityID ?
/*004*/	GroupLeadershipAA_Struct LeaderAAs;
/*068*/	uint32	Unknown068[49];	// Was 63
/*264*/	uint32	NPCMarkerID;
/*268*/	uint32	Unknown268[13];
/*320*/
};

struct GroupFollow_Struct { // SoF Follow Struct
/*0000*/	char	name1[64];	// inviter
/*0064*/	char	name2[64];	// invitee
/*0128*/	uint32	unknown0128;
/*0132*/
};

// this is generic struct
struct GroupLeaderChange_Struct
{
/*000*/		char	Unknown000[64];
/*064*/		char	LeaderName[64];
/*128*/		char	Unknown128[20];
};

struct GroupMentor_Struct {
/*000*/	int percent;
/*004*/	char name[64];
/*068*/
};

struct FaceChange_Struct {
/*000*/	uint8	haircolor;
/*001*/	uint8	beardcolor;
/*002*/	uint8	eyecolor1; // the eyecolors always seem to be the same, maybe left and right eye?
/*003*/	uint8	eyecolor2;
/*004*/	uint8	hairstyle;
/*005*/	uint8	beard;
/*006*/	uint8	face;
/*007*/ uint32	drakkin_heritage;
/*011*/ uint32	drakkin_tattoo;
/*015*/ uint32	drakkin_details;
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
/*00*/	uint32 fromid;
/*04*/	uint32 action;
/*08*/
};

struct TradeBusy_Struct {
/*00*/	uint32 to_mob_id;
/*04*/	uint32 from_mob_id;
/*08*/	uint8 type;			// Seen 01
/*09*/	uint8 unknown09;	// Seen EF (239)
/*10*/	uint8 unknown10;	// Seen FF (255)
/*11*/	uint8 unknown11;	// Seen FF (255)
/*12*/
};

struct PetitionUpdate_Struct {
	uint32 petnumber;	// Petition Number
	uint32 color;		// 0x00 = green, 0x01 = yellow, 0x02 = red
	uint32 status;
	time_t senttime;	// 4 has to be 0x1F
	char accountid[32];
	char gmsenttoo[64];
	int32 quetotal;
	char charname[64];
};

struct Petition_Struct {
	uint32 petnumber;
	uint32 urgency;
	char accountid[32];
	char lastgm[32];
	uint32	zone;
	//char zone[32];
	char charname[64];
	uint32 charlevel;
	uint32 charclass;
	uint32 charrace;
	uint32 unknown;
	//time_t senttime; // Time?
	uint32 checkouts;
	uint32 unavail;
	//uint8 unknown5[4];
	time_t senttime;
	uint32 unknown2;
	char petitiontext[1024];
	char gmtext[1024];
};


struct Who_All_Struct { // 76 length total
/*000*/	char	whom[64];
/*064*/	uint32	wrace;		// FF FF = no race

/*068*/	uint32	wclass;		// FF FF = no class
/*072*/	uint32	lvllow;		// FF FF = no numbers
/*076*/	uint32	lvlhigh;	// FF FF = no numbers
/*080*/	uint32	gmlookup;	// FF FF = not doing /who all gm
/*084*/	uint32	guildid;
/*088*/	uint8	unknown076[64];
/*152*/	uint32	type;		// New for SoF. 0 = /who 3 = /who all
/*156*/
};

struct Stun_Struct { // 4 bytes total
	uint32 duration; // Duration of stun
};

struct AugmentItem_Struct {
/*00*/	uint32	container_index;
/*04*/	int32	container_slot;
/*08*/	uint32  augment_index;
/*12*/	int32	augment_slot;
/*16*/	uint32	dest_inst_id;			// The unique serial number for the item instance that is being augmented
/*20*/	int32	augment_action;			// Guessed - 0 = augment, 1 = remove with distiller, 3 = delete aug
/*24*/
};

// OP_Emote
struct Emote_Struct {
/*0000*/	uint32 type;			// 0 - custom, 0xffffffff - command (/dance, /flip, etc...)
/*0004*/	char message[1024];
/*1028*/
};

// Inspect
struct Inspect_Struct {
	uint16 TargetID;
	uint16 PlayerID;
};

//OP_InspectAnswer - Size: 1860
struct InspectResponse_Struct {
/*000*/	uint32 TargetID;
/*004*/	uint32 playerid;
/*008*/	char itemnames[23][64];
/*1480*/uint32 itemicons[23];
/*1572*/char text[288];	// Max number of chars in Inspect Window appears to be 254 // Msg struct property is 256 (254 + '\0' is my guess)
/*1860*/
};

//OP_InspectMessageUpdate - Size: 256 (SoF+ clients after self-inspect window is closed)
struct InspectMessage_Struct {
/*000*/ char text[256];
/*256*/
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
	uint8 blank[8];
};
struct IncreaseStat_Struct{
	/*0000*/	uint8	unknown0;
	/*0001*/	uint8	str;
	/*0002*/	uint8	sta;
	/*0003*/	uint8	agi;
	/*0004*/	uint8	dex;
	/*0005*/	uint8	int_;
	/*0006*/	uint8	wis;
	/*0007*/	uint8	cha;
	/*0008*/	uint8	fire;
	/*0009*/	uint8	cold;
	/*0010*/	uint8	magic;
	/*0011*/	uint8	poison;
	/*0012*/	uint8	disease;
	/*0013*/	char	unknown13[116];
	/*0129*/	uint8	str2;
	/*0130*/	uint8	sta2;
	/*0131*/	uint8	agi2;
	/*0132*/	uint8	dex2;
	/*0133*/	uint8	int_2;
	/*0134*/	uint8	wis2;
	/*0135*/	uint8	cha2;
	/*0136*/	uint8	fire2;
	/*0137*/	uint8	cold2;
	/*0138*/	uint8	magic2;
	/*0139*/	uint8	poison2;
	/*0140*/	uint8	disease2;
};

struct GMName_Struct {
	char oldname[64];
	char gmname[64];
	char newname[64];
	uint8 badname;
	uint8 unknown[3];
};

struct GMDelCorpse_Struct {
	char corpsename[64];
	char gmname[64];
	uint8 unknown;
};

struct GMKick_Struct {
	char name[64];
	char gmname[64];
	uint8 unknown;
};


struct GMKill_Struct {
	char name[64];
	char gmname[64];
	uint8 unknown;
};


struct GMEmoteZone_Struct {
	char text[512];
};

// This is where the Text is sent to the client.
// Use ` as a newline character in the text.
// Variable length.
struct BookText_Struct {
	uint8 window;	// where to display the text (0xFF means new window)
	uint8 type;		//type: 0=scroll, 1=book, 2=item info.. prolly others.
	uint32 invslot;	// Only used in SoF and later clients.
	char booktext[1]; // Variable Length
};
// This is the request to read a book.
// This is just a "text file" on the server
// or in our case, the 'name' column in our books table.
struct BookRequest_Struct {
	uint8 window;	// where to display the text (0xFF means new window)
	uint8 type;		//type: 0=scroll, 1=book, 2=item info.. prolly others.
	uint32 invslot;	// Only used in Sof and later clients;
	int16 subslot; // The subslot inside of a bag if it is inside one.
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
/*00*/	uint32	linked_list_addr[2];// They are, get this, prev and next, ala linked list
/*08*/	float	size;				//
/*10*/	uint16	solidtype;			//
/*12*/	uint32	drop_id;			// Unique object id for zone
/*16*/	uint16	zone_id;			// Redudant, but: Zone the object appears in
/*18*/	uint16	zone_instance;		//
/*20*/	uint32	unknown020;			//
/*24*/	uint32	unknown024;			//
/*28*/	float	tilt_x;
/*32*/	float	tilt_y;
/*36*/	float	heading;			// heading
/*40*/	float	z;					// z coord
/*44*/	float	x;					// x coord
/*76*/	float	y;					// y coord
/*80*/	char	object_name[32];	// Name of object, usually something like IT63_ACTORDEF
/*84*/	uint32	unknown076;			//
/*88*/	uint32	object_type;		// Type of object, not directly translated to OP_OpenObject
/*92*/	uint32	unknown084;			//set to 0xFF
		uint32	spawn_id;			// Spawn Id of client interacting with object

};
// 01 = generic drop, 02 = armor, 19 = weapon
//[13:40] and 0xff seems to be indicative of the tradeskill/openable items that end up returning the old style item type in the OP_OpenObject

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
** also sent by the client when they close the container.
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
** cDoorSpawnsStruct(f721)
**
*/
struct Door_Struct
{
/*0000*/ char	name[32];		// Filename of Door // Was 10char long before... added the 6 in the next unknown to it //changed both to 32
/*0032*/ float	yPos;			// y loc
/*0036*/ float	xPos;			// x loc
/*0040*/ float	zPos;			// z loc
/*0044*/ float	heading;
/*0048*/ uint32	incline;		// rotates the whole door
/*0052*/ uint16	size;			// 100 is normal, smaller number = smaller model
/*0054*/ uint8	unknown0038[6];
/*0060*/ uint8	doorId;			// door's id #
/*0061*/ uint8	opentype;
/*
 * Open types:
 * 66 = PORT1414 (Qeynos)
 * 55 = BBBOARD (Qeynos)
 * 100 = QEYLAMP (Qeynos)
 * 56 = CHEST1 (Qeynos)
 * 5 = DOOR1 (Qeynos)
 */
/*0062*/ uint8 state_at_spawn;
/*0063*/ uint8 invert_state;	// if this is 1, the door is normally open
/*0064*/ uint32 door_param;
/*0068*/ uint8 unknown0052[12]; // mostly 0s, the last 3 bytes are something tho
/*0080*/
};



struct DoorSpawns_Struct {
	struct Door_Struct doors[0];
};

/*
 OP Code: Op_ClickDoor
 Size:		16
*/
struct ClickDoor_Struct {
/*000*/	uint8	doorid;
/*001*/	uint8	unknown001;		// This may be some type of action setting
/*002*/	uint8	unknown002;		// This is sometimes set after a lever is closed
/*003*/	uint8	unknown003;		// Seen 0
/*004*/	uint8	picklockskill;
/*005*/	uint8	unknown005[3];
/*008*/ uint32	item_id;
/*012*/ uint16	player_id;
/*014*/ uint8	unknown014[2];
/*016*/
};

struct MoveDoor_Struct {
	uint8	doorid;
	uint8	action;
};


struct BecomeNPC_Struct {
	uint32 id;
	int32 maxlevel;
};

struct Underworld_Struct {
	float speed;
	float y;
	float x;
	float z;
};

struct Resurrect_Struct	{
/*000*/	uint32	unknown000;
/*004*/	uint16	zone_id;
/*006*/	uint16	instance_id;
/*008*/	float	y;
/*012*/	float	x;
/*016*/	float	z;
/*020*/	uint32	unknown020;
/*024*/	char	your_name[64];
/*088*/	uint32	unknown088;
/*092*/	char	rezzer_name[64];
/*156*/	uint32	spellid;
/*160*/	char	corpse_name[64];
/*224*/	uint32	action;
/* 228 */
};

struct Translocate_Struct {
/*000*/	uint32	ZoneID;
/*004*/	uint32	SpellID;
/*008*/	uint32	unknown008; //Heading ?
/*012*/	char	Caster[64];
/*076*/	float	y;
/*080*/	float	x;
/*084*/	float	z;
/*088*/	uint32	Complete;
};

struct PendingTranslocate_Struct
{
	uint32 zone_id;
	uint16 instance_id;
	float heading;
	float x;
	float y;
	float z;
	uint32 spell_id;
};

struct Sacrifice_Struct {
/*000*/	uint32	CasterID;
/*004*/	uint32	TargetID;
/*008*/	uint32	Confirm;
};

struct SetRunMode_Struct {
	uint8 mode;
	uint8 unknown[3];
};

//EnvDamage is EnvDamage2 without a few bytes at the end.

struct EnvDamage2_Struct {
/*0000*/	uint32 id;
/*0004*/	uint16 unknown4;
/*0006*/	uint32 damage;
/*0010*/	uint8 unknown10[12];
/*0022*/	uint8 dmgtype; //FA = Lava; FC = Falling
/*0023*/	uint8 unknown2[4];
/*0027*/	uint16 constant; //Always FFFF
/*0029*/	uint16 unknown29;
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
	BazaarTrader_CustomerBrowsing = 13,
	BazaarInspectItem = 18,
	BazaarSearchDone2 = 19,
	BazaarTrader_StartTraderMode2 = 22
};

enum {
	BazaarPriceChange_Fail = 0,
	BazaarPriceChange_UpdatePrice = 1,
	BazaarPriceChange_RemoveItem = 2,
	BazaarPriceChange_AddItem = 3
};

struct BazaarWindowStart_Struct {
	uint8	Action;
	uint8	Unknown001;
	uint16	Unknown002;
};


struct BazaarWelcome_Struct {
	BazaarWindowStart_Struct Beginning;
	uint32	Traders;
	uint32	Items;
	uint32	Unknown012;
	uint32	Unknown016;
};

struct BazaarSearch_Struct {
	BazaarWindowStart_Struct Beginning;
	uint32	TraderID;
	uint32	Class_;
	uint32	Race;
	uint32	ItemStat;
	uint32	Slot;
	uint32	Type;
	char	Name[64];
	uint32	MinPrice;
	uint32	MaxPrice;
	uint32	Minlevel;
	uint32	MaxLlevel;
};
struct BazaarInspect_Struct{
	uint32 ItemID;
	uint32 Unknown004;
	char Name[64];
};

struct NewBazaarInspect_Struct {
/*000*/	BazaarWindowStart_Struct Beginning;
/*004*/	char Name[64];
/*068*/	uint32 Unknown068;
/*072*/	uint32 Unknown072;
/*076*/	uint32 Unknown076;
/*080*/	int32 SerialNumber;
/*084*/	uint32 Unknown084;
};

struct BazaarReturnDone_Struct{
	uint32 Type;
	uint32 TraderID;
	uint32 Unknown008;
	uint32 Unknown012;
	uint32 Unknown016;
};
struct BazaarSearchResults_Struct {
/*000*/	BazaarWindowStart_Struct Beginning;
/*004*/	uint32	NumItems;
/*008*/	uint32	SerialNumber;
/*012*/	uint32	SellerID;
/*016*/	uint32	Cost;
/*020*/	uint32	ItemStat;
/*024*/	char	ItemName[64];
/*088*/
	// New fields for SoD+, stripped off when encoding for older clients.
	char	SellerName[64];
	uint32	ItemID;
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
	Barter_BuyerItemInspect = 21,
	Barter_Unknown23 = 23
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
uint8	clientattackfilters;	// 0) No, 1) All (players) but self, 2) All (players) but group
uint8	npcattackfilters;		// 0) No, 1) Ignore NPC misses (all), 2) Ignore NPC Misses + Attacks (all but self), 3) Ignores NPC Misses + Attacks (all but group)
uint8	clientcastfilters;		// 0) No, 1) Ignore PC Casts (all), 2) Ignore PC Casts (not directed towards self)
uint8	npccastfilters;			// 0) No, 1) Ignore NPC Casts (all), 2) Ignore NPC Casts (not directed towards self)
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
/*004*/	uint32	augments[6];
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
 * Client to server packet
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

enum {
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
/*263*/	uint8	disable_tutorial_go_home; // This appears valid on Ti and RoF..other clients need verification
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

struct Track_Struct {
	uint32 entityid;
	float distance;
	// Fields for SoD and later
	uint8 level;
	uint8 is_npc;
	char name[64];
	uint8 is_pet;
	uint8 is_merc;
};

struct Tracking_Struct {
	Track_Struct Entrys[0];
};

struct TrackTarget_Struct
{
	uint32	EntityID;
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
	uint32	formatstring;
	uint32	pidstring;
	char*	name;
	uint32	rankstring;
	char*	guild;
	uint32	unknown80[2];
	uint32	zonestring;
	uint32	zone;
	uint32	class_;
	uint32	level;
	uint32	race;
	char*	account;
	uint32	unknown100;
};

// The following four structs are the WhoAllPlayer struct above broken down
// for use in World ClientList::SendFriendsWho to accomodate the user of variable
// length strings within the struct above.

struct	WhoAllPlayerPart1 {
	uint32	FormatMSGID;
	uint32	PIDMSGID;
	char	Name[1];;
};

struct	WhoAllPlayerPart2 {
	uint32	RankMSGID;
	char	Guild[1];
};

struct	WhoAllPlayerPart3 {
	uint32	Unknown80[2];
	uint32	ZoneMSGID;
	uint32	Zone;
	uint32	Class_;
	uint32	Level;
	uint32	Race;
	char	Account[1];
};

struct	WhoAllPlayerPart4 {
	uint32	Unknown100;
};

struct WhoAllReturnStruct {
/*000*/	uint32	id;
/*004*/	uint32	playerineqstring;
/*008*/	char	line[27];
/*035*/	uint8	unknown35; //0A
/*036*/	uint32	unknown36;//0s
/*040*/	uint32	playersinzonestring;
/*044*/	uint32	unknown44[2]; //0s
/*052*/	uint32	unknown52;//1
/*056*/	uint32	unknown56;//1
/*060*/	uint32	playercount;//1
	struct WhoAllPlayer player[0];
};

struct Trader_Struct {
/*000*/	uint32	Code;
/*004*/	uint32	Unknown004;
/*008*/	uint64	Items[80];
/*648*/	uint32	ItemCost[80];
};

struct ClickTrader_Struct {
/*000*/	uint32	Code;
/*004*/	uint32	Unknown004;
/*008*/	int64	SerialNumber[80];
/*648*/	uint32	ItemCost[80];
};

struct GetItems_Struct{
	uint32	Items[80];
	int32	SerialNumber[80];
	int32	Charges[80];
};

struct BecomeTrader_Struct
{
/*000*/	uint32 ID;
/*004*/	uint32 Code;
/*008*/	char Name[64];
/*072*/	uint32 Unknown072;	// Observed 0x33,0x91 etc on zone-in, 0x00 when sent for a new trader after zone-in
/*076*/
};

struct TraderStatus_Struct{
	uint32 Code;
	uint32 Uknown04;
	uint32 Uknown08;
};

struct Trader_ShowItems_Struct{
/*000*/	uint32 Code;
/*004*/	uint32 TraderID;
/*008*/	uint32 Unknown08[3];
/*020*/
};

struct TraderBuy_Struct{
/*000*/	uint32 Action;
/*004*/	uint32 TraderID;
/*008*/	uint32 ItemID;
/*012*/	uint32 AlreadySold;
/*016*/	uint32 Price;
/*020*/	uint32 Quantity;
/*024*/	char ItemName[64];
};

struct TraderItemUpdate_Struct{
	uint32	Unknown000;
	uint32	TraderID;
	uint8	FromSlot;
	int		ToSlot; //7?
	uint16	Charges;
};

struct TraderPriceUpdate_Struct {
/*000*/	uint32	Action;
/*004*/	uint32	SubAction;
/*008*/	int32	SerialNumber;
/*012*/	uint32	Unknown012;
/*016*/	uint32	NewPrice;
/*020*/	uint32	Unknown016;
};

struct MoneyUpdate_Struct{
	int32 platinum;
	int32 gold;
	int32 silver;
	int32 copper;
};

struct TraderDelItem_Struct{
	uint32 Unknown000;
	uint32 TraderID;
	uint32 ItemID;
	uint32 Unknown012;
};

struct TraderClick_Struct{
/*000*/	uint32 TraderID;
/*004*/	uint32 Code;
/*008*/	uint32 Unknown008;
/*012*/	uint32 Approval;
/*016*/
};

struct FormattedMessage_Struct{
	uint32	unknown0;
	uint32	string_id;
	uint32	type;
	char	message[0];
};
struct SimpleMessage_Struct{
	uint32	string_id;
	uint32	color;
	uint32	unknown8;
};

struct GuildMemberUpdate_Struct {
/*00*/	uint32	GuildID;
/*04*/	char	MemberName[64];
/*68*/	uint16	ZoneID;
/*70*/	uint16	InstanceID;	//speculated
/*72*/	uint32	LastSeen;	//unix timestamp
/*76*/
};

struct GuildMemberLevelUpdate_Struct {
/*00*/	uint32 guild_id;
/*04*/	char	member_name[64];
/*68*/	uint32	level;	//not sure
};

struct Internal_GuildMemberEntry_Struct {
//	char	name[64];					//variable length
	uint32	level;						//network byte order
	uint32	banker;						//1=yes, 0=no, network byte order
	uint32	class_;						//network byte order
	uint32	rank;						//network byte order
	uint32	time_last_on;				//network byte order
	uint32	tribute_enable;				//network byte order
	uint32	total_tribute;				//total guild tribute donated, network byte order
	uint32	last_tribute;				//unix timestamp
//	char	public_note[1];				//variable length.
	uint16	zoneinstance;				//network byte order
	uint16	zone_id;					//network byte order
};

struct Internal_GuildMembers_Struct {	//just for display purposes, this is not actually used in the message encoding.
	char	player_name[64];		//variable length.
	uint32	count;				//network byte order
	uint32	name_length;	//total length of all of the char names, excluding terminators
	uint32	note_length;	//total length of all the public notes, excluding terminators
	Internal_GuildMemberEntry_Struct member[0];
	/*
	* followed by a set of `count` null terminated name strings
	* and then a set of `count` null terminated public note strings
	*/
};

struct GuildMOTD_Struct{
/*0000*/	uint32	unknown0;
/*0004*/	char	name[64];
/*0068*/	char	setby_name[64];
/*0132*/	uint32	unknown132;
/*0136*/	char	motd[512];
};

struct GuildUpdate_PublicNote{
	uint32	unknown0;
	char	name[64];
	char	target[64];
	char	note[1]; //variable length.
};

struct GuildUpdateURLAndChannel_Struct
{
/*0000*/	uint32	Action;	// 0 = Update URL, 1 = Update Channel
/*0004*/	char	Unknown0004[76];
/*0080*/	char	Text[512];	// URL can be up to 512, SoD client caps the Channel at 128;
/*0592*/	char	Unknown0592[3584];
/*4176*/
};

//Case 5 in Rain of Fear and higher clients for guild permissions.
//RankID is the internal guild rank. There are 8 in Rain of Fear as opposed to the 3 in Titanium.
//PermissionID is the type of permission. There are 32 total, with some unused. Live May 2014 sends and uses 26 of them. Varies through client version.
//Permission value is a char that is either 0 or 1. Enabled for that rank/disabled for that rank.
//The client sends this struct on changing a guild rank. The server sends each rank in 32 or less packets upon zonein if you are in a guild.
struct GuildUpdateRanks_Struct
{
/*0000*/	uint32	Action;	// 0 = Update URL, 1 = Update Channel, 5 = RoF Ranks
/*0004*/	uint32	Unknown0004; //Seen 00 00 00 00
/*0008*/	uint32	Unknown0008; //Seen 96 29 00 00
/*0008*/	char	Unknown0012[64]; //Seen "CharacterName"
/*0076*/	uint32	GuildID; //Guild ID of "CharacterName"
/*0080*/	uint32	RankID;
/*0084*/	uint32	PermissionID;
/*0088*/	char	PermissionVal;
/*0089*/	char	Unknown0089[3]; //Seen 2c 01 00 ?
/*0092*/
};


struct GuildStatus_Struct
{
/*000*/	char	Name[64];
/*064*/	uint8	Unknown064[72];
};

struct GuildDemoteStruct{
	char	name[64];
	char	target[64];
};

struct GuildRemoveStruct{
	char	target[64];
	char	name[64];
	uint32	unknown128;
	uint32	leaderstatus; //?
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
/*0304*/	uint32	unknown304;
/*0308*/	char	unknown308[160];
/*0468*/	char	target_name[64];
/*0532*/	uint32	type;
/*0536*/	char	unknown536[2052];
/*2584*/	char	bug[2048];
/*4632*/	char	unknown4632[6];
/*4638*/	char	system_info[4094];
};
struct Make_Pet_Struct { //Simple struct for getting pet info
	uint8 level;
	uint8 class_;
	uint16 race;
	uint8 texture;
	uint8 pettype;
	float size;
	uint8 type;
	uint32 min_dmg;
	uint32 max_dmg;
};
struct Ground_Spawn{
	float max_x;
	float max_y;
	float min_x;
	float min_y;
	float max_z;
	float heading;
	char name[20];
	uint32 item;
	uint32 max_allowed;
	uint32 respawntimer;
};
struct Ground_Spawns {
	struct Ground_Spawn spawn[50]; //Assigned max number to allow
};
struct PetitionBug_Struct{
	uint32	petition_number;
	uint32	unknown4;
	char	accountname[64];
	uint32	zoneid;
	char	name[64];
	uint32	level;
	uint32	class_;
	uint32	race;
	uint32	unknown152[3];
	uint32	time;
	uint32	unknown168;
	char	text[1028];
};

struct ApproveZone_Struct {
	char	name[64];
	uint32	zoneid;
	uint32	approve;
};
struct ZoneInSendName_Struct {
	uint32	unknown0;
	char	name[64];
	char	name2[64];
	uint32	unknown132;
};
struct ZoneInSendName_Struct2 {
	uint32	unknown0;
	char	name[64];
	uint32	unknown68[145];
};

static const uint32 MAX_TRIBUTE_TIERS = 10;

struct StartTribute_Struct {
	uint32	client_id;
	uint32	tribute_master_id;
	uint32	response;
};

struct TributeLevel_Struct {
	uint32	level;	//backwards byte order!
	uint32	tribute_item_id;	//backwards byte order!
	uint32	cost;	//backwards byte order!
};

struct TributeAbility_Struct {
	uint32	tribute_id;	//backwards byte order!
	uint32	tier_count;	//backwards byte order!
	TributeLevel_Struct tiers[MAX_TRIBUTE_TIERS];
	char	name[0];
};

struct GuildTributeAbility_Struct {
	uint32	guild_id;
	TributeAbility_Struct ability;
};

struct SelectTributeReq_Struct {
	uint32	client_id;	//? maybe action ID?
	uint32	tribute_id;
	uint32	unknown8;	//seen E3 00 00 00
};

struct SelectTributeReply_Struct {
	uint32	client_id;	//echoed from request.
	uint32	tribute_id;
	char		desc[0];
};

struct TributeInfo_Struct {
	uint32	active;		//0 == inactive, 1 == active
	uint32	tributes[EQEmu::legacy::TRIBUTE_SIZE];	//-1 == NONE
	uint32	tiers[EQEmu::legacy::TRIBUTE_SIZE];		//all 00's
	uint32	tribute_master_id;
};

struct TributeItem_Struct {
	uint32	slot;
	uint32	quantity;
	uint32	tribute_master_id;
	int32	tribute_points;
};

struct TributePoint_Struct {
	int32	tribute_points;
	uint32	unknown04;
	int32	career_tribute_points;
	uint32	unknown12;
};

struct TributeMoney_Struct {
	uint32	platinum;
	uint32	tribute_master_id;
	int32	tribute_points;
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
/*00*/	int16	container_slot;
/*02*/	int16	guildtribute_slot;
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
	uint32 some_id; //same as in favorites
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

// this is the "value#a" data
enum EParticlePoint {
	eDefault,
	eChest,
	eHead,
	eLeftHand,
	eRigthHand,
	eLeftFoot,
	eRightFood,
	eLeftEye,
	eRightEye,
	eMouth
};

struct LevelAppearance_Struct { //Sends a little graphic on level up
	uint32	spawn_id;
	uint32	parm1;
	uint32	value1a;
	uint32	value1b;
	uint32	parm2;
	uint32	value2a;
	uint32	value2b;
	uint32	parm3;
	uint32	value3a;
	uint32	value3b;
	uint32	parm4;
	uint32	value4a;
	uint32	value4b;
	uint32	parm5;
	uint32	value5a;
	uint32	value5b;
/*64*/
};

struct MerchantList {
	uint32	id;
	uint32	slot;
	uint32	item;
	int16	faction_required;
	int8	level_required;
	uint16	alt_currency_cost;
	uint32	classes_required;
	uint8	probability;
};

struct TempMerchantList {
	uint32	npcid;
	uint32	slot;
	uint32	item;
	uint32	charges; //charges/quantity
	uint32	origslot;
};

struct AltCurrencyDefinition_Struct {
	uint32 id;
	uint32 item_id;
};

struct NPC_Emote_Struct {
	uint32	emoteid;
	uint8	event_;
	uint8	type;
	char	text[515];
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
	FindPerson_Point path[0];	//last element must be the same as dest
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
	char filename[0];
};

//this is for custom title display in the skill window
struct TitleEntry_Struct {
	uint32	title_id;
	char	title[1];
	char	suffix[1];
};

struct Titles_Struct {
	uint32	title_count;
	TitleEntry_Struct titles[0];
};

//this is for title selection by the client
struct TitleListEntry_Struct {
	uint32	unknown0;	//title ID
	char prefix[1];		//variable length, null terminated
	char postfix[1];		//variable length, null terminated
};

struct TitleList_Struct {
	uint32 title_count;
	TitleListEntry_Struct titles[0];	//list of title structs
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

struct TaskMemberList_Struct {
/*00*/ uint32	gopher_id;
/*04*/ uint32	unknown04;
/*08*/ uint32	member_count;	//1 less than the number of members
/*12*/ char	list_pointer[0];
/*	list is of the form:
	char member_name[1]	//null terminated string
	uint8	task_leader	//boolean flag
*/
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
/*027*/	uint32	reward_count;	//not sure
/*031*/	uint32	unknown31;
/*035*/	uint32	unknown35;
/*039*/	uint16	unknown39;
/*041*/	char	reward_link[1];	//variable length, 0 terminated
/*042*/	uint32	unknown43; //maybe crystal count?
};

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
/*046*/	uint32	unknown46; //saw 0x151,0x156
/*050*/	uint32	unknown50; //saw 0x404,0
/*054*/	char activity_name[1]; //variable length, 0 terminated... commonly empty
/*055*/	uint32	done_count;
/*059*/	uint32	unknown59;	//=1 except on unknown and terminal activities?
/*063*/
};

struct TaskSelectorWindowHeader_Struct {
/*000*/	uint32	TaskCount;			// Guessed - Seen 1
/*000*/	uint32	Unknown2;			// Seen 2
/*000*/	uint32	TaskGiver;			// Guessed - Seen 1517
/*000*/	TaskSelectorData_Struct Tasks[1];
};

struct TaskSelectorData_Struct {
/*000*/	uint32	TaskID;				// Seen 208
/*000*/	float	Unknown5;			// Seen 1.0
/*000*/	uint32	Unknown6;			// Seen 0
/*000*/	uint32	Unknown7;			// Seen 0
/*000*/	char	TaskName[1];		// Null Terminated
/*000*/	char	TaskDescription[1];	// Null Terminated
/*000*/	uint8	Unknown10;			// Possibly another Null Terminated String? Seen 0
/*000*/	uint32	ActivityCount;		// Seen 5
/*000*/	TaskSelectorActivities_Struct Activities[1];
};

struct TaskSelectorActivities_Struct {
/*000*/	uint32	ActivityNumber;			// Seen 0 to 4
/*000*/	uint32	ActivityType;			// Guessed - 1 = Give? 2 = Kill? 3 = Loot? 5 = Speak?
/*000*/	uint32	Unknown14;				// Seen 0
/*000*/	char	Text1;					// Seen 0
/*000*/	uint32	Text2Len;				// Size of the following string
/*000*/	char	Text2;					// If the previous field is 0, this field is not send (item name?)
/*000*/	uint32	GoalCount;				// Seen 1
/*000*/	uint32	NumString1Len;			// Seen 2
/*000*/	char	NumString1;				// Seen "-1"
/*000*/	uint32	NumString2Len;			// Seen 2
/*000*/	char	NumString2;				// Seen "-1"
/*000*/	char	ZoneIDString;			// Seen "188" tutoriala
/*000*/	char	Text3[1];				// Null Terminated
/*000*/	char	NumString3;				// Null Terminated - Seen "1546"
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
	uint32	unknown00;
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
	//uint16	unknown1; // 0x0012
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
	uint32	unknown3;
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
	uint32	unknown4;
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
	uint32	GoalCount;
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
/*0000*/ uint32 unknown0000;				// All zeroes?
/*0004*/ uint32 group_leadership_exp;		// Group leadership exp value
/*0008*/ uint32 group_leadership_points;	// Unspent group points
/*0012*/ uint32 unknown0012;				// Type?
/*0016*/ uint32 unknown0016;				// All zeroes?
/*0020*/ uint32 raid_leadership_exp;		// Raid leadership exp value
/*0024*/ uint32 raid_leadership_points;		// Unspent raid points
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
	// The following field is for SoD+
/*08**/	char	Name[64];
};

struct InspectBuffs_Struct {
/*000*/ uint32 spell_id[BUFF_COUNT];
/*100*/ int32 tics_remaining[BUFF_COUNT];
};

struct RaidGeneral_Struct {
/*00*/	uint32		action;	//=10
/*04*/	char		player_name[64];	//should both be the player's name
/*64*/	char		leader_name[64];
/*132*/	uint32		parameter;
};

struct RaidAddMember_Struct {
/*000*/ RaidGeneral_Struct raidGen; //param = (group num-1); 0xFFFFFFFF = no group
/*136*/ uint8 _class;
/*137*/	uint8 level;
/*138*/	uint8 isGroupLeader;
/*139*/	uint8 flags[5]; //no idea if these are needed...
};

struct RaidMOTD_Struct {
/*000*/ RaidGeneral_Struct general; // leader_name and action only used
/*136*/ char motd[0]; // max size is 1024, but reply is variable
};

struct RaidLeadershipUpdate_Struct {
/*000*/	uint32 action;
/*004*/	char player_name[64];
/*068*/	char leader_name[64];
/*132*/	GroupLeadershipAA_Struct group; //unneeded
/*196*/	RaidLeadershipAA_Struct raid;
/*260*/	char Unknown260[128]; //unverified
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
/*00*/ char		name[32];
/*32*/ float	y;
/*36*/ float	x;
/*40*/ float	z;
/*44*/ uint32	something;
/*48*/ uint32	unknown48;		//0
/*52*/ uint32	one_hundred;	//0x64
/*56*/ uint32	unknown56;		//0
/*60*/ uint32	something2;
/*64*/ int32	unknown64;		//-1
/*68*/ uint32	unknown68;		//0
/*72*/ uint32	unknown72;		//0
/*76*/ uint32	unknown76;		//0x100
/*80*/
};

// Bandolier actions
enum
{
	bandolierCreate = 0,
	bandolierRemove,
	bandolierSet
};

struct BandolierCreate_Struct
{
/*00*/	uint32 Action;		//0 for create
/*04*/	uint8 Number;
/*05*/	char Name[32];
/*37*/	uint16 Unknown37;	//seen 0x93FD
/*39*/	uint8 Unknown39;	//0
};

struct BandolierDelete_Struct
{
/*00*/	uint32 Action;
/*04*/	uint8 Number;
/*05*/	uint8 Unknown05[35];
};

struct BandolierSet_Struct
{
/*00*/	uint32 Action;
/*04*/	uint8 Number;
/*05*/	uint8 Unknown05[35];
};

struct Arrow_Struct {
/*000*/	uint32	type;		//unsure on name, seems to be 0x1, dosent matter
/*005*/	uint8	unknown004[12];
/*016*/	float	src_y;
/*020*/	float	src_x;
/*024*/	float	src_z;
/*028*/	uint8	unknown028[12];
/*040*/	float	velocity;		//4 is normal, 20 is quite fast
/*044*/	float	launch_angle;	//0-450ish, not sure the units, 140ish is straight
/*048*/	float	tilt;		//on the order of 125
/*052*/	uint8	unknown052[8];
/*060*/	float	arc;
/*064*/	uint8	unknown064[12];
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
/*117*/	uint8	unknown117[19];
};

//made a bunch of trivial structs for stuff for opcode finder to use
struct Consent_Struct {
	char name[1];	//always at least a null
};

enum { LDoNMerchant = 1, DiscordMerchant = 2, NorrathsKeepersMerchant = 4, DarkReignMerchant = 5 };

struct AdventureMerchant_Struct {
	uint32	Type;	// 1 = LDoN, 2 = Discord, 4 = Norrath's Keepers, 5 = Dark Reign
	uint32	entity_id;
};

struct Save_Struct {
	uint8	unknown00[192];
};

struct GMToggle_Struct {
	uint8 unknown0[64];
	uint32 toggle;
};

struct GroupInvite_Struct {
	char invitee_name[64];
	char inviter_name[64];
//	uint8	unknown128[65];
};

struct ColoredText_Struct {
	uint32 color;
	char msg[1];
};

struct UseAA_Struct {
	uint32 begin;
	uint32 ability;
	uint32 end;
};

//new AA stuff
//reference only
struct AARankInfo_Struct
{
	uint32 id;
	int32 upper_hotkey_sid;
	int32 lower_hotkey_sid;
	int32 title_sid;
	int32 desc_sid;
	int32 level_req;
	int32 cost;
	uint32 seq;
	uint32 current_level;
	uint32 type;
	int32 spell;
	int32 spell_type;
	int32 spell_refresh;
	int32 classes;
	int32 max_level;
	int32 prev_id;
	int32 next_id;
	int32 total_cost;
	int32 expansion;
	int32 category;
	uint32 charges;
	uint8 grant_only;
	uint32 total_effects;
	uint32 total_prereqs;
};

struct AARankPrereq_Struct
{
	int32 aa_id;
	int32 points;
};

struct AARankEffect_Struct
{
	int32 effect_id;
	int32 base1;
	int32 base2;
	int32 slot;
};

//old AA stuff

struct AA_Ability {
/*00*/	uint32 skill_id;
/*04*/	uint32 base1;
/*08*/	uint32 base2;
/*12*/	uint32 slot;
};

struct SendAA_Struct {
/* EMU additions for internal use */
	char name[128];
	int16 cost_inc;
	uint32 sof_current_level;
	uint32 sof_next_id;
	uint8 level_inc;

/*0000*/	uint32 id;
/*0004*/	uint32 unknown004;
/*0008*/	uint32 hotkey_sid;
/*0012*/	uint32 hotkey_sid2;
/*0016*/	uint32 title_sid;
/*0020*/	uint32 desc_sid;
/*0024*/	uint32 class_type;
/*0028*/	uint32 cost;
/*0032*/	uint32 seq;
/*0036*/	uint32 current_level; //1s, MQ2 calls this AARankRequired
/*0040*/	uint32 prereq_skill;		//is < 0, abs() is category #
/*0044*/	uint32 prereq_minpoints; //min points in the prereq
/*0048*/	uint32 type;
/*0052*/	uint32 spellid;
/*0056*/	uint32 spell_type;
/*0060*/	uint32 spell_refresh;
/*0064*/	uint16 classes;
/*0066*/	uint16 berserker; //seems to be 1 if its a berserker ability
/*0068*/	uint32 max_level;
/*0072*/	uint32 last_id;
/*0076*/	uint32 next_id;
/*0080*/	uint32 cost2;
/*0084*/	uint32 unknown80[2]; //0s
// Begin SoF Specific/Adjusted AA Fields
/*0088*/	uint32 aa_expansion;
/*0092*/	uint32 special_category;
/*0096*/	uint32 sof_type;
/*0100*/	uint32 sof_cost_inc;
/*0104*/	uint32 sof_max_level;
/*0108*/	uint32 sof_next_skill;
/*0112*/	uint32 clientver;
/*0016*/	uint32 account_time_required;
/*0120*/	uint32 total_abilities;
/*0124*/	AA_Ability abilities[0];
};

struct AA_Action {
/*00*/	uint32	action;
/*04*/	uint32	ability;
/*08*/	uint32	target_id;
/*12*/	uint32	exp_value;
};

struct AAExpUpdate_Struct {
/*00*/	uint32 unknown00;	//seems to be a value from AA_Action.ability
/*04*/	uint32 aapoints_unspent;
/*08*/	uint8 aaxp_percent;	//% of exp that goes to AAs
/*09*/	uint8 unknown09[3];	//live dosent always zero these, so they arnt part of aaxp_percent
};


struct AltAdvStats_Struct {
/*000*/	uint32 experience;
/*004*/	uint16 unspent;
/*006*/	uint16	unknown006;
/*008*/	uint8	percentage;
/*009*/	uint8	unknown009[3];
};

struct PlayerAA_Struct {						// Is this still used?
	AA_Array aa_list[MAX_PP_AA_ARRAY];
};

struct AATable_Struct {
/*00*/ int32		aa_spent;					// Total AAs Spent
/*04*/ AA_Array	aa_list[MAX_PP_AA_ARRAY];
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
	uint32 spell[MAX_PP_MEMSPELL];	// 0xFFFFFFFF if no action, slot number if to unmem starting at 0
	uint32 unknown;					//there seems to be an extra field in this packet...
};

// This is the structure for OP_ZonePlayerToBind opcode. Discovered on Feb 9 2007 by FNW from packet logs for titanium client
// This field "zone_name" is text the Titanium client will display on player death
// it appears to be a variable length, null-terminated string
// In logs it has "Bind Location" text which shows up on Titanium client as ....
// "Return to Bind Location, please wait..."
// This can be used to send zone name instead.. On 6.2 client, this is ignored.
struct ZonePlayerToBind_Struct {
/*000*/	uint16 bind_zone_id;
/*002*/	uint16 bind_instance_id;
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
/*0000*/ uint32 spawnId;		// Spawn Id of the shrouded player
/*0004*/ Spawn_Struct spawn;	// Updated spawn struct for the player
/*0586*/
};

struct ApplyPoison_Struct {
	uint32 inventorySlot;
	uint32 success;
};

struct ItemVerifyRequest_Struct {
/*000*/	int32	slot;		// Slot being Right Clicked
/*004*/	uint32	target;		// Target Entity ID
/*008*/
};

struct ItemVerifyReply_Struct {
/*000*/	int32	slot;		// Slot being Right Clicked
/*004*/	uint32	spell;		// Spell ID to cast if different than item effect
/*008*/	uint32	target;		// Target Entity ID
/*012*/
};

struct ItemRecastDelay_Struct {
/*000*/	uint32	recast_delay;	// in seconds
/*004*/	uint32	recast_type;
/*008*/	uint32	unknown008;
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
/*00000*/ uint32 spawnId;				// Spawn Id of you
/*00004*/ Spawn_Struct spawn;			// Updated spawnStruct for you
//this is a sub-struct of PlayerProfile, which we havent broken out yet.
/*00586*/ playerProfileStruct profile;	// Character profile for shrouded char
/*13522*/ uint8 items;					// Items on the player
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
/*000*/	uint32	boatId;			// entitylist id of the boat
/*004*/	bool	TakeControl;	// 01 if taking control, 00 if releasing it
/*007*/	char	unknown[3];		// no idea what these last three bytes represent
};

struct AugmentInfo_Struct
{
/*000*/ uint32	itemid;			// id of the solvent needed
/*004*/ uint32	window;			// window to display the information in
/*008*/ char	augment_info[64];	// the reply has the text here
/*072*/
};

struct ClearObject_Struct
{
/*000*/	uint8	Clear;	// If this is not set to non-zero there is a random chance of a client crash.
/*001*/	uint8	Unknown001[7];
};

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
/*064*/ uint8 Unknown064[64];
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
/*00*/ uint32	TimerID;
/*04*/ uint32	Duration;
/*08*/ uint32	Unknown08;
};

struct InternalVeteranRewardItem
{
/*000*/	uint32 item_id;
/*004*/ uint32 charges;
/*008*/	char item_name[64];
};

struct InternalVeteranReward
{
/*000*/	uint32 claim_id;
/*004*/ uint32 number_available;
/*008*/	uint32 claim_count;
/*012*/	InternalVeteranRewardItem items[8];
};

struct VeteranClaim
{
/*000*/	char name[64]; //name + other data
/*064*/	uint32 claim_id;
/*068*/	uint32 unknown068;
/*072*/	uint32 action;
};

struct GMSearchCorpse_Struct
{
/*000*/	char Unknown000[64];
/*064*/	char Name[64];
/*128*/	uint32 Unknown128;
};

struct CrystalCountUpdate_Struct
{
/*000*/	uint32	CurrentRadiantCrystals;
/*004*/	uint32	CurrentEbonCrystals;
/*008*/	uint32	CareerRadiantCrystals;
/*012*/	uint32	CareerEbonCrystals;
};

struct BeggingResponse_Struct
{
/*00*/	uint32	Unknown00;
/*04*/	uint32	Unknown04;
/*08*/	uint32	Unknown08;
/*12*/	uint32	Result;	// 0 = Fail, 1 = Plat, 2 = Gold, 3 = Silver, 4 = Copper
/*16*/	uint32	Amount;
};

struct GuildBankAck_Struct
{
/*00*/	uint32	Action;	//	10
/*04*/	uint32	Unknown04;
};

struct GuildBankDepositAck_Struct
{
/*00*/	uint32	Action;	//	10
/*04*/	uint32	Unknown04;
/*08*/	uint32	Fail;	//1 = Fail, 0 = Success
};

struct GuildBankPromote_Struct
{
/*00*/	uint32	Action;	// 3
/*04*/	uint32	Unknown04;
/*08*/	uint32	Slot;
/*12*/	uint32	Slot2;	// Always appears to be the same as Slot for Action code 3
};

struct GuildPromoteStruct {
/*000*/	char	target[64];
/*064*/	char	name[64];
/*128*/	uint32	rank;
/*132*/	uint32	myrank;
/*136*/
};

struct GuildBankPermissions_Struct
{
/*00*/	uint32	Action;	// 6
/*04*/	uint32	Unknown04;
/*08*/	uint16	SlotID;
/*10*/	uint16	Unknown10; // Saw 1, probably indicating it is the main area rather than deposits
/*12*/	uint32	ItemID;
/*16*/	uint32	Permissions;
/*20*/	char	MemberName[64];
};

struct GuildBankViewItem_Struct
{
/*00*/	uint32	Action;
/*04*/	uint32	Unknown04;
/*08*/	uint16	SlotID;	// 0 = Deposit area, 1 = Main area
/*10*/	uint16	Area;
/*12*/	uint32	Unknown12;
/*16*/	uint32	Unknown16;
};

struct GuildBankWithdrawItem_Struct
{
/*00*/	uint32	Action;
/*04*/	uint32	Unknown04;
/*08*/	uint16	SlotID;
/*10*/	uint16	Area;
/*12*/	uint32	Unknown12;
/*16*/	uint32	Quantity;
/*20*/
};

struct GuildBankItemUpdate_Struct
{
	void Init(uint32 inAction, uint32 inUnknown004, uint16 inSlotID, uint16 inArea, uint16 inUnknown012, uint32 inItemID, uint32 inIcon, uint32 inQuantity,
			uint32 inPermissions, uint32 inAllowMerge, bool inUseable)
	{
		Action = inAction;
		Unknown004 = inUnknown004;
		SlotID = inSlotID;
		Area = inArea;
		Unknown012 = inUnknown012;
		ItemID = inItemID;
		Icon = inIcon;
		Quantity = inQuantity;
		Permissions = inPermissions;
		AllowMerge = inAllowMerge;
		Useable = inUseable;
		ItemName[0] = '\0';
		Donator[0] = '\0';
		WhoFor[0] = '\0';
	};

/*000*/	uint32	Action;
/*004*/	uint32	Unknown004;
/*008*/	uint16	SlotID;
/*010*/	uint16	Area;
/*012*/	uint32	Unknown012;
/*016*/	uint32	ItemID;
/*020*/	uint32	Icon;
/*024*/	uint32	Quantity;
/*028*/	uint32	Permissions;
/*032*/	uint8	AllowMerge;
/*033*/	uint8	Useable;	// Used in conjunction with the Public-if-useable permission.
/*034*/	char	ItemName[64];
/*098*/	char	Donator[64];
/*162*/ char	WhoFor[64];
/*226*/	uint16	Unknown226;
};

// newer clients (RoF+) send a list that contains 240 entries
// The packets don't actually use all 64 chars in the strings, but we'll just overallocate for these
struct GuildBankItemListEntry_Struct
{
	uint8 vaild;
	uint32 permissions;
	char whofor[64];
	char donator[64];
	uint32 item_id;
	uint32 item_icon;
	uint32 quantity;
	uint8 allow_merge; // 1 here for non-full stacks
	uint8 usable;
	char item_name[64];
};

struct GuildBankClear_Struct
{
/*00*/	uint32	Action;
/*04*/	uint32	Unknown04;
/*08*/	uint32	DepositAreaCount;
/*12*/	uint32	MainAreaCount;
};

struct FindableNPC_Struct
{
/*000*/	uint32	Action;		// 0 = Add, 1 = Remove
/*004*/	uint32	EntityID;
/*008*/	char	Name[64];
/*072*/	char	LastName[32];
/*104*/	uint32	Race;
/*108*/	uint8	Class;
/*109*/	uint8	Unknown109;	// Observed 0x16
/*110*/	uint8	Unknown110;	// Observed 0x06
/*111*/	uint8	Unknown111;	// Observed 0x24
/*112*/
};

struct GroupRole_Struct
{
/*000*/	char	Name1[64];
/*064*/	char	Name2[64];
/*128*/	uint32	Unknown128;
/*132*/	uint32	Unknown132;
/*136*/	uint32	Unknown136;
/*140*/	uint32	RoleNumber;
/*144*/	uint8	Toggle;
/*145*/	uint8	Unknown145[3];
/*148*/
};

struct HideCorpse_Struct
{
/*00*/	uint32	Action;
/*04*/	uint32	Unknown04;
/*08*/
};

struct BuffIconEntry_Struct
{
	uint32 buff_slot;
	uint32 spell_id;
	int32 tics_remaining;
	uint32 num_hits;
	char caster[64];
};

struct BuffIcon_Struct
{
	uint32 entity_id;
	uint8  all_buffs;
	uint16 count;
	uint8 type; // 0 = self buff window, 1 = self target window, 4 = group, 5 = PC, 7 = NPC
	int32 tic_timer;
	int32 name_lengths; // so ahh we kind of do these packets hacky, this is the total length of all the names to make creating the real packets in the translators easier
	BuffIconEntry_Struct entries[0];
};

struct ExpeditionInfo_Struct
{
/*000*/ uint32 max_players;
/*004*/ char expedition_name[128];
/*132*/ char leader_name[64];
};

struct ExpeditionJoinPrompt_Struct
{
/*000*/ char player_name[64];
/*064*/ char expedition_name[64];
};

struct ExpeditionExpireWarning
{
/*008*/ uint32 minutes_remaining;
};

struct ExpeditionCompassEntry_Struct
{
/*000*/ uint32 enabled; //guess
/*004*/ float y;
/*008*/ float x;
/*012*/ float z;
};

struct ExpeditionCompass_Struct
{
/*000*/ uint32 count;
/*004*/ ExpeditionCompassEntry_Struct entries[0];
};

struct ExpeditionMemberEntry_Struct
{
	char name[64];
	char status;
};

struct ExpeditionMemberList_Struct
{
/*000*/ uint32 count;
/*004*/ ExpeditionMemberEntry_Struct entries[0];
};

struct ExpeditionLockoutEntry_Struct
{
/*000*/ uint32 time_left;
/*004*/ char expedition[128];
/*132*/ char expedition_event[128];
};

struct ExpeditionLockoutList_Struct
{
/*000*/ uint32 count;
/*004*/ ExpeditionLockoutEntry_Struct entries[0];
};

struct ExpeditionLeaderSet_Struct
{
/*000*/ char leader_name[64];
};

struct CorpseDrag_Struct
{
/*000*/ char CorpseName[64];
/*064*/ char DraggerName[64];
/*128*/ uint8 Unknown128[24];
/*152*/
};

struct ChangeSize_Struct
{
/*00*/ uint32 EntityID;
/*04*/ float Size;
/*08*/ uint32 Unknown08;	// Observed 0
/*12*/ float Unknown12;		// Observed 1.0f
/*16*/
};

// New OpCode/Struct for SoD+
struct GroupMakeLeader_Struct
{
/*000*/ uint32 Unknown000;
/*004*/ char CurrentLeader[64];
/*068*/ char NewLeader[64];
/*132*/ char Unknown072[324];
/*456*/
};

//One packet i didn't include here is the alt currency merchant window.
//it works much like the adventure merchant window
//it is formated like: dbstringid|1|dbstringid|count
//ex for a blank crowns window you would send:
//999999|1|999999|0
//any items come after in much the same way adventure merchant items do except there is no theme included
#define ALT_CURRENCY_OP_POPULATE 8
#define ALT_CURRENCY_OP_UPDATE 7

//Server -> Client
//Populates the initial Alternate Currency Window
struct AltCurrencyPopulateEntry_Struct
{
/*000*/ uint32 currency_number; //corresponds to a dbstr id as well, the string matches what shows up in the "alternate currency" tab.
/*004*/ uint32 unknown00; //always 1
/*008*/ uint32 currency_number2; //always same as currency number
/*012*/ uint32 item_id; //appears to be the item id
/*016*/ uint32 item_icon; //actual icon
/*020*/ uint32 stack_size; //most are set to 1000, the stack size for the item; should match db i think or there will be problems.
};

struct AltCurrencyPopulate_Struct {
/*000*/ uint32 opcode; //8 for populate
/*004*/ uint32 count; //number of entries
/*008*/ AltCurrencyPopulateEntry_Struct entries[0];
};

//Server -> Client
//Updates the value of a specific Alternate Currency
struct AltCurrencyUpdate_Struct {
/*000*/ uint32 opcode; //7 for update
/*004*/ char name[64]; //name of client (who knows why just do it)
/*068*/ uint32 currency_number; //matches currency_number from populate entry
/*072*/ uint32 unknown072; //always 1
/*076*/ uint32 amount; //new amount
/*080*/ uint32 unknown080; //seen 0
/*084*/ uint32 unknown084; //seen 0
};

//Client -> Server
//When an item is selected while the alt currency merchant window is open
struct AltCurrencySelectItem_Struct {
/*000*/ uint32 merchant_entity_id;
/*004*/ uint32 slot_id;
/*008*/ uint32 unknown008;
/*012*/ uint32 unknown012;
/*016*/ uint32 unknown016;
/*020*/ uint32 unknown020;
/*024*/ uint32 unknown024;
/*028*/ uint32 unknown028;
/*032*/ uint32 unknown032;
/*036*/ uint32 unknown036;
/*040*/ uint32 unknown040;
/*044*/ uint32 unknown044;
/*048*/ uint32 unknown048;
/*052*/ uint32 unknown052;
/*056*/ uint32 unknown056;
/*060*/ uint32 unknown060;
/*064*/ uint32 unknown064;
/*068*/ uint32 unknown068;
/*072*/ uint32 unknown072;
/*076*/ uint32 unknown076;
};

//Server -> Client
//As setup it makes it so that item can't be sold to the merchant.
//eg: "I will give you no doubloons for a cloth cap."
//Probably also sends amounts somewhere
struct AltCurrencySelectItemReply_Struct {
/*000*/ uint32	unknown000;
/*004*/ uint8	unknown004; //0xff
/*005*/ uint8	unknown005; //0xff
/*006*/ uint8	unknown006; //0xff
/*007*/ uint8	unknown007; //0xff
/*008*/ char	item_name[64];
/*072*/ uint32	unknown074;
/*076*/ uint32	cost;
/*080*/ uint32	unknown080;
/*084*/ uint32	unknown084;
};

//Client -> Server
//Requests purchase of a specific item from the vendor
struct AltCurrencyPurchaseItem_Struct {
/*000*/ uint32 merchant_entity_id;
/*004*/ uint32 item_id;
/*008*/ uint32 unknown008; //1
};

//Client -> Server
//Reclaims / Create currency button pushed.
struct AltCurrencyReclaim_Struct {
/*000*/ uint32 currency_id;
/*004*/ uint32 unknown004;
/*008*/ uint32 count;
/*012*/ uint32 reclaim_flag; //1 = this is reclaim
};

struct AltCurrencySellItem_Struct {
/*000*/ uint32 merchant_entity_id;
/*004*/ uint32 slot_id;
/*008*/ uint32 charges;
/*012*/ uint32 cost;
};

struct Untargetable_Struct {
/*000*/	uint32 id;
/*004*/	uint32 targetable_flag; //0 = not targetable, 1 or higher = targetable
/*008*/
};

struct CrystalReclaim_Struct {
/**/ uint32 type;
/**/ uint32 amount;
};

struct LFGuild_SearchPlayer_Struct
{
/*00*/	uint32	Command;
/*04*/	uint32	Unknown04;
/*08*/	uint32	FromLevel;
/*12*/	uint32	ToLevel;
/*16*/	uint32	MinAA;
/*20*/	uint32	TimeZone;
/*24*/	uint32	Classes;
};

struct LFGuild_SearchGuild_Struct
{
/*00*/	uint32	Command;
/*04*/	uint32	Unknown04;
/*08*/	uint32	Level;
/*12*/	uint32	AAPoints;
/*16*/	uint32	TimeZone;
/*20*/	uint32	Class;
/*24*/
};

struct LFGuild_PlayerToggle_Struct
{
/*000*/ uint32	Command;
/*004*/ uint8	Unknown004[68];
/*072*/ char	Comment[256];
/*328*/ uint8	Unknown328[268];
/*596*/ uint32	TimeZone;
/*600*/ uint8	Toggle;
/*601*/ uint8	Unknown601[7];
/*608*/ uint32	TimePosted;
/*612*/ uint8	Unknown612[12];
/*624*/
};

struct LFGuild_GuildToggle_Struct
{
/*000*/ uint32	Command;
/*004*/ uint8	Unknown004[8];
/*012*/ char	Comment[256];
/*268*/ uint8	Unknown268[256];
/*524*/ uint32	FromLevel;
/*528*/ uint32	ToLevel;
/*532*/ uint32	Classes;
/*536*/ uint32	AACount;
/*540*/ uint32	TimeZone;
/*544*/ uint8	Toggle;
/*545*/ uint8	Unknown545[3];
/*548*/ uint32	TimePosted;
/*552*/ char	Name[64];
/*616*/
};

struct MaxCharacters_Struct
{
/*000*/ uint32 max_chars;	// Seen 4 on Silver Account (4 characters max)
/*004*/ uint32 unknown004;	// Seen 0
/*008*/ uint32 unknown008;	// Seen 0
};

struct Membership_Struct
{
/*000*/ uint32 membership;	// Seen 2 on Gold Account
/*004*/ uint32 races;	// Seen ff ff 01 00
/*008*/ uint32 classes;	// Seen ff ff 01 01
/*012*/ uint32 entrysize; // Seen 15 00 00 00
/*016*/ int32 entries[21]; //Varies. Seen ff ff ff ff, and 01 00 00 00
/*104*/ uint32 exit_url_length;	// Length of the exit_url string (0 for none)
/*108*/ // char exit_url[0];	// URL that will open when EQ is exited
};
// Used by MercenaryListEntry_Struct
struct MercenaryStance_Struct {
/*0000*/	uint32	StanceIndex;	// Index of this stance (sometimes reverse reverse order - 3, 2, 1, 0 for 4 stances etc)
/*0004*/	uint32	Stance;			// From dbstr_us.txt - 1^24^Passive^0, 2^24^Balanced^0, etc (1 to 9 as of April 2012)
};

struct Membership_Entry_Struct
{
/*000*/ uint32 purchase_id;		// Seen 1, then increments 90287 to 90300
/*004*/ uint32 bitwise_entry;	// Seen 16 to 65536 - Skips 4096
/*008*/
};

struct Membership_Setting_Struct
{
/*000*/ uint32 setting_index;	// 0, 1, or 2
/*004*/ uint32 setting_id;		// 0 to 21
/*008*/ int32 setting_value;	// All can be 0, 1, or -1
/*012*/
};

struct Membership_Details_Struct
{
/*0000*/ uint32 membership_setting_count;	// Seen 66
/*0016*/ Membership_Setting_Struct settings[66];
/*0012*/ uint32 race_entry_count;	// Seen 15
/*1044*/ Membership_Entry_Struct membership_races[15];
/*0012*/ uint32 class_entry_count;	// Seen 15
/*1044*/ Membership_Entry_Struct membership_classes[15];
/*1044*/ uint32 exit_url_length;	// Length of the exit_url string (0 for none)
/*1048*/ //char exit_url[42];		// Upgrade to Silver or Gold Membership URL
/*1048*/ uint32 exit_url_length2;	// Length of the exit_url2 string (0 for none)
/*0000*/ //char exit_url2[49];		// Upgrade to Gold Membership URL
};

struct ItemPreview_Struct
{
/*000*/	uint32	itemid;
/*004*/	uint32	unknown004[3];
/*016*/	uint32	slot;
/*020*/	uint32	unknown020;
/*024*/	uint16	slot2;
/*026*/	uint8	unknown026[54];
};

// Used by specific packets
struct MercenaryList_Struct {
/*0000*/	uint32	MercID;				// ID unique to each type of mercenary (probably a DB id)
/*0004*/	uint32	MercType;			// From dbstr_us.txt - Apprentice (330000100), Journeyman (330000200), Master (330000300)
/*0008*/	uint32	MercSubType;		// From dbstr_us.txt - 330020105^23^Race: Guktan<br>Type: Healer<br>Confidence: High<br>Proficiency: Apprentice, Tier V...
/*0012*/	uint32	PurchaseCost;		// Purchase Cost (in gold)
/*0016*/	uint32	UpkeepCost;			// Upkeep Cost (in gold)
/*0020*/	uint32	Status;				// Required Account Status (Free = 0, Silver = 1, Gold = 2) at merchants - Seen 0 (suspended) or 1 (unsuspended) on hired mercs ?
/*0024*/	uint32	AltCurrencyCost;	// Alternate Currency Purchase Cost? (all seen costs show N/A Bayle Mark) - Seen 0
/*0028*/	uint32	AltCurrencyUpkeep;	// Alternate Currency Upkeep Cost? (all seen costs show 1 Bayle Mark) - Seen 1
/*0032*/	uint32	AltCurrencyType;	// Alternate Currency Type? - 19^17^Bayle Mark^0 - Seen 19
/*0036*/	uint8	MercUnk01;			// Unknown (always see 0)
/*0037*/	int32	TimeLeft;			// Unknown (always see -1 at merchant) - Seen 900000 (15 minutes in ms for newly hired merc)
/*0041*/	uint32	MerchantSlot;		// Merchant Slot? Increments, but not always by 1 - May be for Merc Window Options (Seen 5, 36, 1 for active mercs)?
/*0045*/	uint32	MercUnk02;			// Unknown (normally see 1, but sometimes 2 or 0)
/*0049*/	uint32	StanceCount;		// Iterations of MercenaryStance_Struct - Normally 2 to 4 seen
/*0053*/	int32	MercUnk03;			// Unknown (always 0 at merchant) - Seen on active merc: 93 a4 03 77, b8 ed 2f 26, 88 d5 8b c3, and 93 a4 ad 77
/*0057*/	uint8	MercUnk04;			// Seen 1
/*0058*/	char	MercName[1];		// Null Terminated Mercenary Name (00 at merchants)
};


// Used by MercenaryMerchantList_Struct
struct MercenaryListEntry_Struct {
/*0000*/	uint32	MercID;				// ID unique to each type of mercenary (probably a DB id)
/*0004*/	uint32	MercType;			// From dbstr_us.txt - Apprentice (330000100), Journeyman (330000200), Master (330000300)
/*0008*/	uint32	MercSubType;		// From dbstr_us.txt - 330020105^23^Race: Guktan<br>Type: Healer<br>Confidence: High<br>Proficiency: Apprentice, Tier V...
/*0012*/	uint32	PurchaseCost;		// Purchase Cost (in gold)
/*0016*/	uint32	UpkeepCost;			// Upkeep Cost (in gold)
/*0020*/	uint32	Status;				// Required Account Status (Free = 0, Silver = 1, Gold = 2) at merchants - Seen 0 (suspended) or 1 (unsuspended) on hired mercs ?
/*0024*/	uint32	AltCurrencyCost;	// Alternate Currency Purchase Cost? (all seen costs show N/A Bayle Mark) - Seen 0
/*0028*/	uint32	AltCurrencyUpkeep;	// Alternate Currency Upkeep Cost? (all seen costs show 1 Bayle Mark) - Seen 1
/*0032*/	uint32	AltCurrencyType;	// Alternate Currency Type? - 19^17^Bayle Mark^0 - Seen 19
/*0036*/	uint8	MercUnk01;			// Unknown (always see 0)
/*0037*/	int32	TimeLeft;			// Unknown (always see -1 at merchant) - Seen 900000 (15 minutes in ms for newly hired merc)
/*0041*/	uint32	MerchantSlot;		// Merchant Slot? Increments, but not always by 1 - May be for Merc Window Options (Seen 5, 36, 1 for active mercs)?
/*0045*/	uint32	MercUnk02;			// Unknown (normally see 1, but sometimes 2 or 0)
/*0049*/	uint32	StanceCount;		// Iterations of MercenaryStance_Struct - Normally 2 to 4 seen
/*0053*/	int32	MercUnk03;			// Unknown (always 0 at merchant) - Seen on active merc: 93 a4 03 77, b8 ed 2f 26, 88 d5 8b c3, and 93 a4 ad 77
/*0057*/	uint8	MercUnk04;			// Seen 1
/*0058*/	char	MercName[1];		// Null Terminated Mercenary Name (00 at merchants)
/*0000*/	MercenaryStance_Struct Stances[MAX_MERC_STANCES];	// Count Varies, but hard set to 5 max for now - From dbstr_us.txt - 1^24^Passive^0, 2^24^Balanced^0, etc (1 to 9 as of April 2012)
};

// [OPCode: 0x27ac OP_MercenaryDataResponse] On Live as of April 2 2012 [Server->Client]
// Opcode should be renamed to something like OP_MercenaryMerchantShopResponse since the Data Response packet is different
// Sent by the server when browsing the Mercenary Merchant
struct MercenaryMerchantList_Struct {
/*0000*/	uint32	MercTypeCount;			// Number of Merc Types to follow
/*0004*/	uint32	MercGrades[MAX_MERC_GRADES];	// Count varies, but hard set to 3 max for now - From dbstr_us.txt - Apprentice (330000100), Journeyman (330000200), Master (330000300)
/*0016*/	uint32	MercCount;				// Number of MercenaryInfo_Struct to follow
/*0020*/	MercenaryListEntry_Struct Mercs[MAX_MERC];	// Data for individual mercenaries in the Merchant List
};

// [OPCode: 0x4dd9 OP_MercenaryDataRequest] On Live as of April 2 2012 [Client->Server]
// Opcode should be renamed to something like OP_MercenaryMerchantShopRequest since the Data Request packet is different
// Right clicking merchant - shop request
struct MercenaryMerchantShopRequest_Struct {
/*0000*/	uint32	MercMerchantID;			// Entity ID of the Mercenary Merchant
/*0004*/
};

// Used by MercenaryDataUpdate_Struct
struct MercenaryData_Struct {
/*0000*/	uint32	MercID;				// ID unique to each type of mercenary (probably a DB id) - (if 1, do not send MercenaryData_Struct - No merc hired)
/*0004*/	uint32	MercType;			// From dbstr_us.txt - Apprentice (330000100), Journeyman (330000200), Master (330000300)
/*0008*/	uint32	MercSubType;		// From dbstr_us.txt - 330020105^23^Race: Guktan<br>Type: Healer<br>Confidence: High<br>Proficiency: Apprentice, Tier V...
/*0012*/	uint32	PurchaseCost;		// Purchase Cost (in gold)
/*0016*/	uint32	UpkeepCost;			// Upkeep Cost (in gold)
/*0020*/	uint32	Status;				// Required Account Status (Free = 0, Silver = 1, Gold = 2) at merchants - Seen 0 (suspended) or 1 (unsuspended) on hired mercs ?
/*0024*/	uint32	AltCurrencyCost;	// Alternate Currency Purchase Cost? (all seen costs show N/A Bayle Mark) - Seen 0
/*0028*/	uint32	AltCurrencyUpkeep;	// Alternate Currency Upkeep Cost? (all seen costs show 1 Bayle Mark) - Seen 1
/*0032*/	uint32	AltCurrencyType;	// Alternate Currency Type? - 19^17^Bayle Mark^0 - Seen 19
/*0036*/	uint8	MercUnk01;			// Unknown (always see 0)
/*0037*/	int32	TimeLeft;			// Unknown (always see -1 at merchant) - Seen 900000 (15 minutes in ms for newly hired merc)
/*0041*/	uint32	MerchantSlot;		// Merchant Slot? Increments, but not always by 1 - May be for Merc Window Options (Seen 5, 36, 1 for active mercs)?
/*0045*/	uint32	MercUnk02;			// Unknown (normally see 1, but sometimes 2 or 0)
/*0049*/	uint32	StanceCount;		// Iterations of MercenaryStance_Struct - Normally 2 to 4 seen
/*0053*/	int32	MercUnk03;			// Unknown (always 0 at merchant) - Seen on active merc: 93 a4 03 77, b8 ed 2f 26, 88 d5 8b c3, and 93 a4 ad 77
/*0057*/	uint8	MercUnk04;			// Seen 1
/*0058*/	char	MercName[64];		// Null Terminated Mercenary Name (00 at merchants)
/*0000*/	MercenaryStance_Struct Stances[MAX_MERC_STANCES];	// Count Varies, but hard set to 2 for now - From dbstr_us.txt - 1^24^Passive^0, 2^24^Balanced^0, etc (1 to 9 as of April 2012)
/*0000*/	uint32	MercUnk05;			// Seen 1 - Extra Merc Data field that differs from MercenaryListEntry_Struct
// MercUnk05 may be a field that is at the end of the packet only, even if multiple mercs are listed (haven't seen examples of multiple mercs owned at once)
};

// [OPCode: 0x6537] On Live as of April 2 2012 [Server->Client]
// Should be named OP_MercenaryDataResponse, but the current opcode using that name should be renamed first
// Size varies if mercenary is hired or if browsing Mercenary Merchant
// This may also be the response for Client->Server 0x0327 (size 0) packet On Live as of April 2 2012
struct MercenaryDataUpdate_Struct {
/*0000*/	int32	MercStatus;					// Seen 0 with merc and -1 with no merc hired
/*0004*/	uint32	MercCount;					// Seen 1 with 1 merc hired and 0 with no merc hired
/*0008*/	MercenaryData_Struct MercData[MAX_MERC];	// Data for individual mercenaries in the Merchant List
};

// [OPCode: 0x6537] On Live as of April 2 2012 [Server->Client]
// Size 12 and sent on Zone-In if no mercenary is currently hired and when merc is dismissed
// (Same packet as MercAssign_Struct?)
struct NoMercenaryHired_Struct {
/*0000*/	int32	MercStatus;			// Seen -1 with no merc hired
/*0004*/	uint32	MercCount;			// Seen 0 with no merc hired
/*0008*/	uint32	MercID;				// Seen 1 when no merc is hired - ID unique to each type of mercenary
/*0012*/
};

// OP_MercenaryAssign (Same packet as NoMercenaryHired_Struct?)
struct MercenaryAssign_Struct {
/*0000*/	uint32	MercEntityID;	// Seen 0 (no merc spawned) or 615843841 and 22779137
/*0004*/	uint32	MercUnk01;		//
/*0008*/	uint32	MercUnk02;		//
/*0012*/
};

// [OPCode: 0x495d OP_MercenaryTimer] On Live as of April 2 2012 [Server->Client] [Size: 20]
// Sent on Zone-In, or after Dismissing, Suspending, or Unsuspending Mercs
struct MercenaryStatus_Struct {
/*0000*/	uint32	MercEntityID;	// Seen 0 (no merc spawned) or 615843841 and 22779137
/*0004*/	uint32	UpdateInterval;	// Seen 900000 - Matches from 0x6537 packet (15 minutes in ms?)
/*0008*/	uint32	MercUnk01;		// Seen 180000 - 3 minutes in milleseconds? Maybe next update interval?
/*0012*/	uint32	MercState;		// Seen 5 (normal) or 1 (suspended)
/*0016*/	uint32	SuspendedTime;	// Seen 0 (not suspended) or c9 c2 64 4f (suspended on Sat Mar 17 11:58:49 2012) - Unix Timestamp
/*0020*/
};

// [OPCode: 0x4c6c] On Live as of April 2 2012 [Client->Server] [Size: 8]
// Sent from the client when using the Mercenary Window
struct MercenaryCommand_Struct {
/*0000*/	uint32	MercCommand;	// Seen 0 (zone in with no merc or suspended), 1 (dismiss merc), 5 (normal state), 36 (zone in with merc)
/*0004*/	int32	Option;			// Seen -1 (zone in with no merc), 0 (setting to passive stance), 1 (normal or setting to balanced stance)
/*0008*/
};

// [OPCode: 0x1a79] On Live as of April 2 2012 [Client->Server] [Size: 1]
// Requesting to suspend or unsuspend merc
struct SuspendMercenary_Struct {
/*0000*/	uint8	SuspendMerc;	// Seen 30 (48) for suspending or unsuspending
/*0001*/
};

// [OPCode: 0x2528] On Live as of April 2 2012 [Server->Client] [Size: 4]
// Response to suspend merc with timestamp
struct SuspendMercenaryResponse_Struct {
/*0000*/	uint32	SuspendTime;	// Unix Timestamp - Seen a9 11 78 4f
/*0004*/
};

// [OPCode: 0x5e78 (OP_MercenaryHire?)] On Live as of April 2 2012
// Sent by client when requesting to view Mercenary info or Hire a Mercenary
struct MercenaryMerchantRequest_Struct {
/*0000*/	uint32	MercID;			// Seen 399 and 400 for merc ID
/*0004*/	uint32	MercUnk01;		// Seen 1
/*0008*/	uint32	MercMerchantID;	// Entity ID for Mercenary Merchant
/*0012*/	uint32	MercUnk02;		// Seen 65302016 (00 6e e4 03) - (probably actually individual uint8 fields), but seen as DWORD in Seeds client.
/*0016*/
};

// [OPCode: 0x5e78 (OP_MercenaryHire?)] On Live as of April 2 2012
/*
Valid response IDs:

0 - Hire me! (Assign Merc after sending this.)
1 - Insufficient money message.
2 - Mercenary-To-Hire does not exist in the server's DB.
3 - Mercenary failed to spawn. (this actually tells us the mercenary should spawn BEFORE recieving this packet.)
4 - Mercenaries not allowed in raids.
5 - You already have a mercenary request pending
6 - You must dismiss the mercenary before hiring a new one.
7 - You must dismiss your suspended one before hiring a new one.
8 - Group is full.
9 - Error creating mercenary
10 - Replacing mercenary(?!)
11 - Your mercenary has quit! You ran out of money to pay for your mercenary!
12 - Your mercenary waived an upkeep cost of %d plat, and %d gold and your mercenary upkeep cost timer has been reset to %s. <-- these values are for GM resets of mercenaries and are generated from the client's
mercenary info. NOT from the packet.
13 - Your mercenary is about to be quit due to insufficient funds! <--- Sent before the mercenary quits, unsure of time sent before.
14 - There is no mercenary liason nearby! <-- hacking attempt check if no mercenary merchant is in the zone!
15 - You are too far away from the liason! <-- Liason exists as type in the zone, but client is too far away. (position update happened)
16 - You do not meet the requirements for that mercenary! <-- For example, if a mercenary is 'unlocked' in some way, send this if they do not have the mercenary unlock.
*/
// Sent by Server in response to requesting to view Mercenary info or Hire a Mercenary
struct MercenaryMerchantResponse_Struct {
/*0000*/	uint32	ResponseType;
/*0004*/
};

struct ServerLootItem_Struct {
	uint32	item_id;	  // uint32	item_id;
	int16	equip_slot;	  // int16	equip_slot;
	uint16	charges;	  // uint8	charges; 
	uint16	lootslot;	  // uint16	lootslot;
	uint32	aug_1;		  // uint32	aug_1;
	uint32	aug_2;		  // uint32	aug_2;
	uint32	aug_3;		  // uint32	aug_3;
	uint32	aug_4;		  // uint32	aug_4;
	uint32	aug_5;		  // uint32	aug_5;
	uint32	aug_6;		  // uint32	aug_5;
	uint8	attuned;
	uint8	min_level;
	uint8	max_level;
};

//Found in client near a ref to the string:
//"Got a broadcast message for ... %s ...\n"
struct ClientMarqueeMessage_Struct {
	uint32 type;
	uint32 unk04; // no idea, didn't notice a change when altering it.
	//According to asm the following are hard coded values: 2, 4, 5, 6, 7, 10, 12, 13, 14, 15, 16, 18, 20
	//There is also a non-hardcoded fall through but to be honest i don't know enough about what it does yet
	uint32 priority; //needs a better name but it does:
	//opacity = (priority / 255) - floor(priority / 255)
	//# of fade in/out blinks = (int)((priority - 1) / 255)
	//so 510 would have 100% opacity and 1 extra blink at end
	uint32 fade_in_time; //The fade in time, in ms
	uint32 fade_out_time; //The fade out time, in ms
	uint32 duration; //in ms
	char msg[1]; //message plus null terminator
	
};

typedef std::list<ServerLootItem_Struct*> ItemList;


struct fling_struct {
/* 00 */ uint32 collision; // 0 collision is off, anything else it's on
/* 04 */ int32 travel_time; // ms -- UF we need to calc this, RoF+ -1 auto calcs
/* 08 */ uint8 unk3; // bool, set to 1 has something to do with z-axis or something weird things happen if the new Z is above or equal to yours
/* 09 */ uint8 disable_fall_damage; // 1 you take no fall damage, 0 you take fall damage
/* 10 */ uint8 padding[2];
/* 12 */ float speed_z;
/* 16 */ float new_y;
/* 20 */ float new_x;
/* 24 */ float new_z;
/* 28 */
};

// used when action == 0
struct AuraCreate_Struct {
/* 00 */	uint32 action; // 0 = add, 1 = delete, 2 = reset
/* 04 */	uint32 type; // unsure -- normal auras show 1 clicky (ex. Circle of Power) show 0
/* 08 */	char aura_name[64];
/* 72 */	uint32 entity_id;
/* 76 */	uint32 icon;
/* 80 */
};

// used when action == 1
struct AuraDestory_Struct {
/* 00 */	uint32 action; // 0 = add, 1 = delete, 2 = reset
/* 04 */	uint32 entity_id;
/* 08 */
};
// I think we can assume it's just action for 2, client doesn't seem to do anything with the rest of the data in that case

// Restore structure packing to default
#pragma pack()

#endif

