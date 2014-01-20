#ifndef CLIENT62_STRUCTS_H_
#define CLIENT62_STRUCTS_H_

namespace Client62 {
	namespace structs {


static const uint32 BUFF_COUNT = 25;
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
/*
	Cofruben:
	Adventure stuff,not a net one,just one for our use
*/
static const uint32 ADVENTURE_COLLECT		= 0;
static const uint32 ADVENTURE_MASSKILL		= 1;
static const uint32 ADVENTURE_NAMED			= 2;
static const uint32 ADVENTURE_RESCUE		= 3;

struct AdventureInfo {
	uint32 QuestID;
	uint32 NPCID;
	bool in_use;
	uint32 status;
	bool ShowCompass;
	uint32 Objetive;// can be item to collect,mobs to kill,boss to kill and someone to rescue.
	uint32 ObjetiveValue;// number of items,or number of needed mob kills.
	char text[512];
	uint8 type;
	uint32 minutes;
	uint32 points;
	float x;
	float y;
	uint32 zoneid;
	uint32 zonedungeonid;
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
			uint8	blue;
			uint8	green;
			uint8	red;
			uint8	use_tint;	// if there's a tint this is FF
		} rgb;
		uint32 color;
	};
};

/*
** Character Selection Struct
** Length: 1676 Bytes
**
*/
struct CharacterSelect_Struct {
/*0000*/	uint32	zone[10];			// Characters Current Zone
/*0040*/	uint8	eyecolor1[10];			// Characters Eye Color
/*0050*/	uint8	eyecolor2[10];			// Characters Eye 2 Color
/*0060*/	uint8	hairstyle[10];			// Characters hair style
/*0070*/	uint8	unknown0070[2];
/*0072*/	uint32	primary[10];			// Characters primary IDFile number
/*0112*/	uint32	race[10];			// Characters Race
/*0152*/	uint8	class_[10];			// Characters Classes
/*0162*/	char	name[10][64];			// Characters Names
/*0802*/	uint8	gender[10];			// Characters Gender
/*0812*/	uint8	level[10];			// Characters Levels
/*0822*/	uint8	unknown0822[2];
/*0824*/	uint32	secondary[10];			// Characters secondary IDFile number
/*0864*/	uint8	face[10];			// Characters Face Type
/*0874*/	uint8	beard[10];			// Characters Beard Type
/*0884*/	uint32	equip[10][9];			// 0=helm, 1=chest, 2=arm, 3=bracer, 4=hand, 5=leg, 6=boot, 7=melee1, 8=melee2  (Might not be)
/*1244*/	uint8	haircolor[10];			// Characters Hair Color
/*1254*/	uint8	gohome[10];			// 1=Go Home available, 0=not
/*1264*/	Color_Struct	cs_colors[10][9];	// Characters Equipment Colors
/*1624*/	uint32	deity[10];			// Characters Deity
/*1664*/	uint8	beardcolor[10];			// Characters beard Color
/*1674*/	uint8	unknown1674[2];
/*1676*/
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
**   spawnZoneStruct
**   dbSpawnStruct
**   petStruct
**   newSpawnStruct
*/
struct Spawn_Struct
{
/*0000*/ uint8  NPC;            // 0=player,1=npc,2=pc corpse,3=npc corpse,a
/*0001*/ uint8 set_to_0xFF[8];  // ***Placeholder (all ff)
/*0009*/ uint8  curHp;          // Current hp
/*0010*/ char    lastName[32];    // Player's Lastname
/*0042*/ uint8  bodytype;       // Bodytype
/*0043*/ uint8 unknown0042[7];
/*0050*/ uint32 petOwnerId;     // If this is a pet, the spawn id of owner
/*0054*/ signed   deltaHeading:10;// change in heading
         signed   x:19;           // x coord
         signed   padding0054:3;  // ***Placeholder
/*0058*/ signed   y:19;           // y coord
         signed   animation:10;   // ***Placeholder (seems like speed)
         signed   padding0058:3;  // animation
/*0062*/ signed   z:19;           // z coord
         signed   deltaY:13;      // change in y
/*0066*/ signed   deltaX:13;      // change in x
         unsigned heading:12;     // heading
         signed   padding0066:7;  // ***Placeholder
/*0070*/ signed   deltaZ:13;      // change in z
         signed   padding0070:19; // ***Placeholder
/*0074*/ uint16  deity;          // Player's Deity
/*0076*/ uint8 unknown[2];
/*0078*/ union
         {
             struct
             {
               /*0078*/ uint32 equip_helmet;    // Equipment: Helmet Visual
               /*0082*/ uint32 equip_chest;     // Equipment: Chest Visual
               /*0086*/ uint32 equip_arms;      // Equipment: Arms Visual
               /*0090*/ uint32 equip_bracers;   // Equipment: Bracers Visual
               /*0094*/ uint32 equip_hands;     // Equipment: Hands Visual
               /*0098*/ uint32 equip_legs;      // Equipment: Legs Visual
               /*0102*/ uint32 equip_feet;      // Equipment: Feet Visual
               /*0106*/ uint32 equip_primary;   // Equipment: Primary Visual
               /*0110*/ uint32 equip_secondary; // Equipment: Secondary Visual
             } equip;
             /*0078*/ uint32 equipment[9];  // Array elements correspond to struct equipment above
         };
/*0114*/ uint8 unknown0114[5];
/*0119*/ uint8  afk;            // 0=no, 1=afk
/*0120*/ uint32 spawnId;        // Spawn Id
/*0124*/ uint8 is_pet;         // 0=no, 1=yes
/*0125*/ uint8  gm;             // 0=no, 1=gm
/*0126*/ uint8 unknown0126[4];
/*0130*/ uint8   aaitle;       // 0=none, 1=general, 2=archtype, 3=class
/*0131*/ uint8 unknown0132[24];
/*0155*/ uint8  anon;           // 0=normal, 1=anon, 2=roleplay
/*0156*/ uint8 unknown0156;
/*0157*/ uint8  face;	          // Face id for players
/*0159*/ uint8 helm;
/*0159*/ uint8 unknown0159;
/*0160*/ float    runspeed;       // Speed when running
/*0164*/ uint8  eyecolor1;      // Player's left eye color
/*0165*/ uint8	beard;			//not 100% verified, but slightly verified
/*0166*/ uint8  is_npc;         // 0=no, 1=yes
/*0167*/ uint8  flymode;	//seems to be 3 for 'useable' npc classes
/*0168*/ uint32 guildID;        // Current guild
/*0172*/ union
         {
             struct
             {
                 /*0172*/ Color_Struct color_helmet;    // Color of helmet item
                 /*0176*/ Color_Struct color_chest;     // Color of chest item
                 /*0180*/ Color_Struct color_arms;      // Color of arms item
                 /*0184*/ Color_Struct color_bracers;   // Color of bracers item
                 /*0188*/ Color_Struct color_hands;     // Color of hands item
                 /*0192*/ Color_Struct color_legs;      // Color of legs item
                 /*0196*/ Color_Struct color_feet;      // Color of feet item
                 /*0200*/ Color_Struct color_primary;   // Color of primary item
                 /*0204*/ Color_Struct color_secondary; // Color of secondary item
             } equipment_colors;
             /*0172*/ Color_Struct colors[9]; // Array elements correspond to struct equipment_colors above
         };
/*0208*/ uint8 hairstyle;
/*0209*/ float    walkspeed;      // Speed when walking
/*0213*/ uint8 unknown0213[3];
/*0216*/ uint8  class_;         // Player's class
/*0217*/ uint8  beardcolor;     // Beard color
/*0218*/ float    size;           // Model size
/*0222*/ uint8  findable;       // 0=can't be found, 1=can be found
/*0223*/ float	unknown_float;
/*0227*/ char     suffix[32];     // Player's suffix (of Veeshan, etc.)
/*0259*/ uint8 unknown0259[4];	//[0] is sometimes 100
/*0263*/ uint8   guildrank;      // 0=normal, 1=officer, 2=leader
/*0264*/ uint8 unknown0264[3];
union
{
/*0267*/ uint8 equip_chest2;     // Second place in packet for chest texture (usually 0xFF in live packets)
                                  // Not sure why there are 2 of them, but it effects chest texture!
/*0267*/ uint8 mount_color;      // drogmor: 0=white, 1=black, 2=green, 3=red
                                  // horse: 0=brown, 1=white, 2=black, 3=tan
};
/*0268*/ uint32 race;            // Spawn race
/*0272*/ uint8  invis;          // Invis (0=not, 1=invis)
/*0273*/ uint8 unknown0276[5];
/*0278*/ uint8  lfg;            // 0=off, 1=lfg on
/*0279*/ uint8  level;          // Spawn Level
/*0280*/ uint8  haircolor;      // Hair color
/*0281*/ uint8 max_hp;	//(name prolly wrong)takes on the value 100 for players, 100 or 110 for NPCs and 120 for PC corpses...
/*0282*/ uint8  light;          // Spawn's lightsource
/*0283*/ uint8  gender;         // Gender (0=male, 1=female)
/*0284*/ char     name[64];       // Player's Name
/*0348*/ uint8  eyecolor2;      // Left eye color
/*0349*/ char     title[32];      // Title
/*0381*/ uint8 unknown0381[2];
}; /*0383*/

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
** Length: 452 Bytes
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
/*0682*/	uint8	unknown692[4];
};

/*
** Memorize Spell Struct
** Length: 12 Bytes
**
*/
struct MemorizeSpell_Struct {
uint32 slot;     // Spot in the spell book/memorized slot
uint32 spell_id; // Spell id (200 or c8 is minor healing, etc)
uint32 scribing; // 1 if memorizing a spell, set to 0 if scribing to book, 2 if un-memming
uint32 unknown12;
};

/*
** Make Charmed Pet
** Length: 12 Bytes
**
*/
struct Charm_Struct {
/*00*/	uint32	owner_id;
/*04*/	uint32	pet_id;
/*08*/	uint32	command;    // 1: make pet, 0: release pet
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
	uint32	new_mana;                  // New Mana AMount
	uint32	stamina;
	uint32	spell_id;
	uint32	unknown12;
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
	uint32	inventoryslot;  // slot for clicky item, 0xFFFF = normal cast
	uint32	target_id;
	uint8    cs_unknown[4];
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
/*000*/	uint8	slotid;		//badly named... seems to be 2 for a real buff, 0 otherwise
/*001*/ uint8	level;
/*002*/	uint8	bard_modifier;
/*003*/	uint8	effect;			//not real
/*004*/	uint32	spellid;
/*008*/ uint32	duration;
/*012*/	uint32	counters;
/*014*/	uint8	Unknown012[4];
};

struct SpellBuffFade_Struct {
/*000*/	uint32 entityid;
/*004*/	uint8 slot;
/*005*/	uint8 level;
/*006*/	uint8 effect;
/*007*/	uint8 unknown7;
/*008*/	uint32 spellid;
/*012*/	uint32 duration;
/*016*/	uint32 unknown016;
/*020*/	uint32 unknown020;	//prolly global player ID
/*024*/	uint32 slotid;
/*028*/	uint32 bufffade;
/*032*/
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
	/*008*/ uint32 skills[73];
	/*300*/ uint8 unknown300[148];
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
/*002*/ uint8		unknown1[2];    // something like PC_ID, but not really. stays the same thru the session though
/*002*/ uint16       skillbank;      // 0 if normal skills, 1 if languages
/*002*/ uint8		unknown2[2];
/*008*/ uint16		skill_id;
/*010*/ uint8		unknown3[2];
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
	/*0000*/	uint32	class_; //guess
	/*0004*/	char	name[64];
	/*0068*/	uint32	beardcolor;	// credit goes to vesuvias for appearance stuff
	/*0072*/	uint32	beard;
	/*0076*/	uint32	haircolor;
	/*0080*/	int32	gender;
	/*0084*/	int32	race;
	/*0088*/	int32	start_zone;
	/*0092*/	int32	hairstyle;
	/*0096*/	uint32	deity;
	///*0072*/	int32	deity;


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


/*0100*/	int32	STR;
/*0104*/	int32	STA;
/*0108*/	int32	AGI;
/*0112*/	int32	DEX;
/*0116*/	int32	WIS;
/*0120*/	int32	INT;
/*0124*/	int32	CHA;
/*0128*/	uint32	face;
/*0132*/	uint32	eyecolor1;	//its possiable we could have these switched
/*0136*/	uint32	eyecolor2;	//since setting one sets the other we really can't check
/*0140*/	uint32	unknown140;
};

/*
 *Used in PlayerProfile
 */
struct AA_Array
{
	uint32 AA;
	uint32 value;
};


static const uint32 MAX_PP_DISCIPLINES = 100;

struct Disciplines_Struct {
	uint32 values[MAX_PP_DISCIPLINES];
};

static const uint32 MAX_PLAYER_TRIBUTES = 5;
static const uint32 MAX_PLAYER_BANDOLIER = 4;
static const uint32 MAX_PLAYER_BANDOLIER_ITEMS = 4;
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
	BandolierItem_Struct items[MAX_PLAYER_BANDOLIER_ITEMS];
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

/*
** Player Profile
**
** Length: 4308 bytes
** OpCode: 0x006a
 */
static const uint32 MAX_PP_LANGUAGE		= 28;
static const uint32 MAX_PP_SPELLBOOK	= 400;
static const uint32 MAX_PP_MEMSPELL		= 9;
static const uint32 MAX_PP_SKILL		= _SkillPacketArraySize;	// 100 - actual skills buffer size
static const uint32 MAX_PP_AA_ARRAY		= 240;
static const uint32 MAX_GROUP_MEMBERS	= 6;
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
/*0120*/	uint32				bind_zone_id;		// Zone player is bound in
/*0124*/	uint32				unknown0124[4];
/*0140*/	float				bind_x[4];				// Bind loc x coord
/*0156*/	float				zone_safe_x;
/*0160*/	float				bind_y[4];				// Bind loc y coord
/*0176*/	float				zone_safe_y;
/*0180*/	float				bind_z[4];				// Bind loc z coord
/*0196*/	float				zone_safe_z;
/*0200*/	float				bind_heading[4];		//
/*0216*/	float				zone_safe_heading;
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
/*2584*/	uint32				spell_book[MAX_PP_SPELLBOOK];
/*4184*/	uint8				unknown3224[448];	// all 0xff
/*4632*/	uint32				mem_spells[MAX_PP_MEMSPELL];
/*4668*/	uint8				unknown3704[32];	//
/*4700*/	float				y;					// Player y position
/*4704*/	float				x;					// Player x position
/*4708*/	float				z;					// Player z position
/*4712*/	float				heading;			// Direction player is facing
/*4716*/	uint8				unknown3756[4];		//
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
/*4768*/	int32				platinum_shared;        // Platinum shared between characters
/*4772*/	uint8				unknown3812[24];
/*4796*/	uint32				skills[MAX_PP_SKILL];	// 100 dword buffer
/*5196*/	uint8				unknown5096[184];
/*5380*/	uint32				pvp2;	//
/*5384*/	uint32				unknown4420;	//
/*5388*/	uint32				pvptype;	//
/*5392*/	uint32				unknown4428;	//
/*5396*/	uint32				ability_down;			// Doodman - Guessing
/*5400*/	uint8				unknown4436[8];	//
/*5408*/	uint32				autosplit;			//not used right now
/*5412*/	uint8				unknown4448[8];
/*5420*/	uint32				zone_change_count;      // Number of times user has zoned in their career (guessing)
/*5424*/	uint8				unknown4460[28];	//
/*5452*/	uint32				expansions;		// expansion setting, bit field of expansions avaliable
/*5456*/	int32				toxicity;	//from drinking potions, seems to increase by 3 each time you drink
/*5460*/	char				unknown4496[16];	//
/*5476*/	int32				hunger_level;
/*5480*/	int32				thirst_level;
/*5484*/	uint32				ability_up;
/*5488*/	char				unknown4524[16];
/*5504*/	uint16				zone_id;			// Current zone of the player
/*5506*/	uint16				zoneInstance;			// Instance ID
/*5508*/	SpellBuff_Struct	buffs[BUFF_COUNT];			// Buffs currently on the player
/*6008*/	char 				groupMembers[6][64];		//
/*6392*/	char				unknown6392[656];
/*7048*/	uint32				entityid;
/*7052*/	uint32				leadAAActive;
/*7056*/	uint32				unknown7056;
/*7060*/	int32				ldon_points_guk;		//client uses these as signed
/*7064*/	int32				ldon_points_mir;
/*7068*/	int32				ldon_points_mmc;
/*7072*/	int32				ldon_points_ruj;
/*7076*/	int32				ldon_points_tak;
/*7080*/	int32				ldon_points_available;
/*7084*/	uint8				unknown5940[112];
/*7196*/	uint32				tribute_time_remaining;	//in miliseconds
/*7200*/	uint32				unknown6048;
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
/*8188*/	uint8				unknown8188[4608];
/*12796*/	uint32				aapoints_spent;
/*12800*/	uint32				expAA;
/*12804*/	uint32				aapoints;	//avaliable, unspent
/*12808*/	uint8				unknown12808[36];
/*12844*/	Bandolier_Struct	bandoliers[MAX_PLAYER_BANDOLIER];
/*14124*/	uint8				unknown14124[5120];
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
/*19564*/	uint32				unknown15964;
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
/*004*/ uint32	unknown;
};

/*
** Delete Spawn
** Length: 4 Bytes
** OpCode: OP_DeleteSpawn
*/
struct DeleteSpawn_Struct
{
/*00*/ uint32 spawn_id;             // Spawn ID to delete
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
** When somebody changes what they're wearing
**      or give a pet a weapon (model changes)
** Length: 16 Bytes
** Opcode: 9220
*/
struct WearChange_Struct{
/*000*/ uint16 spawn_id;
/*002*/ uint16 material;
/*004*/ Color_Struct color;
/*009*/ uint8 wear_slot_id;
};

/*
** Type:   Bind Wound Structure
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
/*080*/	uint32	zone_reason;	//0x0A == death, I think
/*084*/	int32	success;		// =0 client->server, =1 server->client, -X=specific error
/*088*/
};

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
/*02*/	uint8 action;
/*03*/	uint8 value;
/*04*/
};

// solar: this is what causes the caster to animate and the target to
// get the particle effects around them when a spell is cast
// also causes a buff icon
struct Action_Struct
{
 /* 00 */	uint16 target;	// id of target
 /* 02 */	uint16 source;	// id of caster
 /* 04 */	uint16 level; // level of caster
 /* 06 */	uint16 instrument_mod;
 /* 08 */	uint32 unknown08;
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

// solar: this is what prints the You have been struck. and the regular
// melee messages like You try to pierce, etc.  It's basically the melee
// and spell damage message
struct CombatDamage_Struct
{
/* 00 */	uint16	target;
/* 02 */	uint16	source;
/* 04 */	uint8	type; //slashing, etc.  231 (0xE7) for spells
/* 05 */	uint16	spellid;
/* 07 */	uint32	damage;
/* 11 */	uint32 unknown11;
/* 15 */	uint32 sequence;	// see above notes in Action_Struct
/* 19 */	uint32	unknown19;
/* 23 */
};

/*
** Consider Struct
** Length: 24 Bytes
** OpCode: 3721
*/
struct Consider_Struct{
/*000*/ uint32	playerid;               // PlayerID
/*004*/ uint32	targetid;               // TargetID
/*008*/ uint32	faction;                // Faction
/*0012*/ uint32	level;                  // Level
/*016*/ int32	cur_hp;                  // Current Hitpoints
/*020*/ int32	max_hp;                  // Maximum Hitpoints
/*024*/ uint8 pvpcon;     // Pvp con flag 0/1
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
/*012*/	uint32	attack_skill;	// was type
/*016*/	uint32	spell_id;
/*020*/ uint32	bindzoneid;	//bindzoneid?
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
/*0002*/ int32		delta_heading:10,  // change in heading
         			x_pos:19,             // x coord
         			padding0002:3;    // ***Placeholder
/*0006*/ int32		y_pos:19,             // y coord
         			animation:10,     // animation
         			padding0006:3;    // ***Placeholder
/*0010*/ int32		z_pos:19,             // z coord
         			delta_y:13;        // change in y
/*0014*/ int32		delta_x:13,        // change in x
        			heading:12,       // heading
         			padding0014:7;    // ***Placeholder
/*0018*/ int32		delta_z:13,        // change in z
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
/*0020*/ int32 animation:10,     // animation
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
/*04*/ int32	max_hp;                 // Maximum hp of spawn
/*08*/ int16	spawn_id;                 // Current hp of spawn
/*10*/
};
struct SpawnHPUpdate_Struct2
{
/*01*/ int16	spawn_id;
/*00*/ uint8	hp;
};
/*
** Stamina
** Length: 8 Bytes
** OpCode: 5721
*/
struct Stamina_Struct {
/*00*/ uint32 food;                     // (low more hungry 127-0)
/*02*/ uint32 water;                    // (low more thirsty 127-0)
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
/*00*/	char			SerializedItem[0];
/*xx*/
};

struct Consume_Struct
{
/*0000*/ uint32 slot;
/*0004*/ uint32 auto_consumed; // 0xffffffff when auto eating e7030000 when right click
/*0008*/ uint8  c_unknown1[4];
/*0012*/ uint8  type; // 0x01=Food 0x02=Water
/*0013*/ uint8  unknown13[3];
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
		 int32 from_slot;
		 int32 to_slot;
		 int32 cointype1;
		 int32 cointype2;
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
/*008*/	uint32	auto_loot;
};

struct GuildManageStatus_Struct{
	uint32	guildid;
	uint32	oldrank;
	uint32	newrank;
	char	name[64];
};
// Guild invite, remove
struct GuildJoin_Struct{
/*000*/	uint32	guildid;
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

// 4244 bytes. Is not really an 'OnLevelMessage', it causes a popup box to display in the client
// Text looks like HTML.
struct OnLevelMessage_Struct {
/*0000*/       char    Title[128];
/*0128*/       char    Text[4096];
/*4224*/       uint32  Buttons;
/*4228*/       uint32  Duration;
/*4232*/       uint32  PopupID;
/*4236*/       uint32  unknown4236;
/*4240*/       uint32  unknown4240;
/*4244*/
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
/*  0*/	char    charname[64];
/* 30*/	char    gmname[64];
/* 60*/ uint32	success;
/* 61*/	uint32	zoneID;
/*92*/	int32  y;
/*96*/	int32  x;
/*100*/ int32  z;
/*104*/	uint32 unknown2; // E0 E0 56 00
};

struct GMGoto_Struct { // x,y is swapped as compared to summon and makes sense as own packet
/*  0*/	char    charname[64];

/* 64*/	char    gmname[64];
/* 128*/ uint32	success;
/* 132*/	uint32	zoneID;

/*136*/	int32  y;
/*140*/	int32  x;
/*144*/ int32  z;
/*148*/	uint32 unknown2; // E0 E0 56 00
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

struct DeleteItem_Struct {
/*0000*/ uint32 from_slot;
/*0004*/ uint32 to_slot;
/*0008*/ uint32 number_in_stack;
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

/* solar: 9/23/03 reply to /random command; struct from Zaphod */
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
/*004*/	uint32 value; // 0x00 = off 0x01 = on
/*008*/	uint32 unknown008;
/*012*/	uint32 unknown012;
/*016*/	char	name[64];
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
/*016*/	uint8	quantity;		// Already sold
/*017*/ uint8	Unknown016[3];
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
};
struct Adventure_Purchase_Struct {
/*000*/	uint32	some_flag;	//set to 1 generally...
/*000*/	uint32	npcid;
/*004*/	uint32	itemid;
/*008*/	uint32	variable;
};

struct AdventurePoints_Update_Struct {
/*000*/	uint32				ldon_available_points;		// Total available points
/*004*/ uint8				unkown_apu004[20];
/*024*/	uint32				ldon_guk_points;		// Earned Deepest Guk points
/*028*/	uint32				ldon_mirugal_points;		// Earned Mirugal' Mebagerie points
/*032*/	uint32				ldon_mistmoore_points;		// Earned Mismoore Catacombs Points
/*036*/	uint32				ldon_rujarkian_points;		// Earned Rujarkian Hills points
/*040*/	uint32				ldon_takish_points;		// Earned Takish points
/*044*/	uint8				unknown_apu042[216];
};


struct AdventureFinish_Struct{
	uint32 win_lose;//Cofruben: 00 is a lose,01 is win.
	uint32 points;
};
//OP_AdventureRequest
struct AdventureRequest_Struct{
	uint32 risk;//1 normal,2 hard.
	uint32 entity_id;
};
struct AdventureRequestResponse_Struct{
	uint32 unknown000;
	char text[2048];
	uint32 timetoenter;
	uint32 timeleft;
	uint32 risk;
	float x;
	float y;
	float z;
	uint32 showcompass;
	uint32 unknown2080;
};



/*struct Item_Shop_Struct {
	uint16 merchantid;
	uint8 itemtype;
	Item_Struct item;
	uint8 iss_unknown001[6];
};*/

/*
** Illusion_Struct
** Changes client visible features
** Size: 168 bytes
** Used In: OP_Illusion, #face, Mob::SendIllusionPacket()
** Fields from the deprecated struct:
**	uint8	unknown_26; //Always 26
**	uint8	haircolor;
**	uint8	beardcolor;
**	uint8	eyecolor1; // the eyecolors always seem to be the same, maybe left and right eye?
**	uint8	eyecolor2;
**	uint8	hairstyle;
**	uint8	aa_title;
**	uint8	luclinface; // and beard
** Updated by Father Nitwit for 7-14-04 patch
**
*/

struct Illusion_Struct {
/*000*/	uint32	spawnid;
/*004*/	char	charname[64];
/*068*/	uint16	race;
/*070*/	char	unknown070[2];
/*072*/	uint8	gender;
/*073*/	uint8	texture;
/*074*/	uint8	helmtexture;
/*075*/	uint8	unknown075;
/*076*/	uint32	face;
/*080*/	uint8	hairstyle;
/*081*/	uint8	haircolor;
/*082*/	uint8	beard;
/*083*/	uint8	beardcolor;
/*084*/	float	size;
/*088*/	char	unknown084[80];
/*168*/
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
/*0580*/	uint8	unknown[188];
/*0768*/
};
struct GroupJoin_Struct {
/*0000*/	uint32	action;
/*0004*/	char	yourname[64];
/*0068*/	char	membername[64];
/*0132*/	uint8	unknown[84];
};

struct FaceChange_Struct {
/*000*/	uint8	haircolor;
/*001*/	uint8	beardcolor;
/*002*/	uint8	eyecolor1; // the eyecolors always seem to be the same, maybe left and right eye?
/*003*/	uint8	eyecolor2;
/*004*/	uint8	hairstyle;
/*005*/	uint8	beard;	// vesuvias
/*006*/	uint8	face;
//vesuvias:
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

struct PetitionUpdate_Struct {
	uint32 petnumber;    // Petition Number
	uint32 color;		// 0x00 = green, 0x01 = yellow, 0x02 = red
	uint32 status;
	uint32 senttime;    // 4 has to be 0x1F
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
	uint32 senttime;
	uint32 unknown2;
	char petitiontext[1024];
	char gmtext[1024];
};


struct Who_All_Struct { // 76 length total
/*000*/	char	whom[64];
/*064*/	uint32	wrace;		// FF FF = no race

/*066*/	uint32	wclass;		// FF FF = no class
/*068*/	uint32	lvllow;		// FF FF = no numbers
/*070*/	uint32	lvlhigh;	// FF FF = no numbers
/*072*/	uint32	gmlookup;	// FF FF = not doing /who all gm
/*074*/	uint32	unknown074;
/*076*/	uint8	unknown076[64];
/*140*/
};

struct Stun_Struct { // 4 bytes total
	uint32 duration; // Duration of stun
};

struct AugmentItem_Struct {
/*00*/	int16	container_slot;
/*02*/	char	unknown02[2];
/*04*/	int32	augment_slot;
/*08*/
};

// OP_Emote
struct Emote_Struct {
/*0000*/	uint32 unknown01;
/*0004*/	char message[1024];
/*1028*/
};

// Inspect
struct Inspect_Struct {
	uint16 TargetID;
	uint16 PlayerID;
};
//OP_InspectAnswer
struct InspectResponse_Struct{//Cofruben:need to send two of this for the inspect response.
/*000*/	uint32 TargetID;
/*004*/	uint32 playerid;
/*008*/	char itemnames[21][64];
/*1352*/char unknown_zero[64];//fill with zero's.
/*1416*/uint32 itemicons[21];
/*1500*/uint32 unknown_zero2;
/*1504*/char text[288];
};

//OP_SetDataRate
struct SetDataRate_Struct {
	float newdatarate;
};

//OP_SetServerFilter
struct SetServerFilter_Struct {
	uint32 filters[25];		//see enum eqFilterType
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
	uint8 type;             //type: 0=scroll, 1=book, 2=item info.. prolly others.
	char booktext[1]; // Variable Length
};
// This is the request to read a book.
// This is just a "text file" on the server
// or in our case, the 'name' column in our books table.
struct BookRequest_Struct {
	uint8 window;	// where to display the text (0xFF means new window)
	uint8 type;             //type: 0=scroll, 1=book, 2=item info.. prolly others.
	char txtfile[1]; // Variable Length
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
** Click Object Acknowledgement Struct
** Response to client clicking on a World Container (ie, forge)
**
*/
struct ClickObjectAck_Struct {
/*00*/	uint32	player_id;	// Entity Id of player who clicked object
/*04*/	uint32	drop_id;	// Zone-specified unique object identifier
/*08*/	uint32	open;		// 1=opening, 0=closing
/*12*/	uint32	type;		// See object.h, "Object Types"
/*16*/	uint32	unknown16;	//
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
/*0000*/ char    name[32];            // Filename of Door // Was 10char long before... added the 6 in the next unknown to it: Daeken M. BlackBlade
/*0032*/ float   yPos;               // y loc
/*0036*/ float   xPos;               // x loc
/*0040*/ float   zPos;               // z loc
/*0044*/ float	 heading;
/*0048*/ uint32   incline;	// rotates the whole door
/*0052*/ uint16   size;			// 100 is normal, smaller number = smaller model
/*0054*/ uint8    unknown0038[6];
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
/*0064*/ uint32  door_param;		//this may index zone_points, representing the destination
/*0068*/ uint8  unknown0052[12]; // mostly 0s, the last 3 bytes are something tho
/*0080*/
};



struct DoorSpawns_Struct {
	struct Door_Struct doors[0];
};

/*
 OP Code: 	Op_ClickDoor
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
	uint32	unknown00;
	uint16	zone_id;
	uint16	instance_id;
	float	y;
	float	x;
	float	z;
	char	your_name[64];
	uint32	unknown88;
	char	rezzer_name[64];
	uint32	spellid;
	char	corpse_name[64];
	uint32	action;
/* 228 */
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

struct BazaarWindowStart_Struct {
	uint8   Action;
	uint8   Unknown001;
	uint16  Unknown002;
};


struct BazaarWelcome_Struct {
	BazaarWindowStart_Struct beginning;
	uint32  traders;
	uint32  items;
	uint8   unknown1[8];
};

struct BazaarSearch_Struct {
	BazaarWindowStart_Struct beginning;
	uint32	traderid;
	uint32  class_;
	uint32  race;
	uint32  stat;
	uint32  slot;
	uint32  type;
	char   name[64];
	uint32	minprice;
	uint32	maxprice;
};
struct BazaarInspect_Struct{
	uint32 item_id;
	uint32 unknown;
	char name[64];
};
struct BazaarReturnDone_Struct{
	uint32 type;
	uint32 traderid;
	uint32 unknown8;
	uint32 unknown12;
	uint32 unknown16;
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
};

struct ServerSideFilters_Struct {
uint8	clientattackfilters; // 0) No, 1) All (players) but self, 2) All (players) but group
uint8	npcattackfilters;	 // 0) No, 1) Ignore NPC misses (all), 2) Ignore NPC Misses + Attacks (all but self), 3) Ignores NPC Misses + Attacks (all but group)
uint8	clientcastfilters;	 // 0) No, 1) Ignore PC Casts (all), 2) Ignore PC Casts (not directed towards self)
uint8	npccastfilters;		 // 0) No, 1) Ignore NPC Casts (all), 2) Ignore NPC Casts (not directed towards self)
};

/*
** Client requesting item statistics
** Size: 32 bytes
** Used In: OP_ItemLinkClick
** Last Updated: 2/15/2009
**
*/
struct	ItemViewRequest_Struct {
/*000*/	uint32	item_id;
/*004*/	uint32	augments[5];
/*024*/ uint32	link_hash;
/*028*/	char	unknown028[4];
/*032*/
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
/*004*/	uint32	unknown004;
/*008*/	uint32	unknown008;
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
/*260*/	uint8	unknown260;	// 0x01 on live
/*261*/	uint8	unknown261;	// 0x01 on live
/*262*/	uint8	unknown262[2];
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
	/*0000*/	uint8	hp; //health percent
	/*0001*/	uint16	id;//mobs id
};

struct Track_Struct {
	uint16 entityid;
	uint16 padding002;
	float distance;
};

struct Tracking_Struct {
	Track_Struct Entrys[0];
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

struct WhoAllReturnStruct {
	uint32	id;
	uint32	playerineqstring;
	char	line[27];
	uint8	unknown35; //0A
	uint32	unknown36;//0s
	uint32	playersinzonestring;
	uint32	unknown44[2]; //0s
	uint32	unknown52;//1
	uint32	unknown56;//1
	uint32	playercount;//1
	struct WhoAllPlayer player[0];
};

struct Trader_Struct {
	uint32	code;
	uint32	itemid[160];
	uint32	unknown;
	uint32	itemcost[80];
};

struct ClickTrader_Struct {
	uint32	code;
	uint32	unknown[161];//damn soe this is totally pointless :/ but at least your finally using memset! Good job :) -LE
	uint32	itemcost[80];
};

struct GetItems_Struct{
	uint32	items[80];
};

struct BecomeTrader_Struct
{
	uint32 ID;
	uint32 Code;
};

struct Trader_ShowItems_Struct{
	uint32 code;
	uint32 traderid;
	uint32 unknown08[3];
};

struct TraderBuy_Struct{
	uint32 unknown0;
	uint32 traderid;
	uint32 itemid;
	uint32 unknown8;
	uint32 price;
	uint32 quantity;
	uint32 slot_num;
	char  itemname[60];
};

struct TraderItemUpdate_Struct{
	uint32 unknown0;
	uint32 traderid;
	uint8  fromslot;
	uint8  toslot; //7?
	uint16 charges;
};

struct MoneyUpdate_Struct{
	int32 platinum;
	int32 gold;
	int32 silver;
	int32 copper;
};

struct TraderDelItem_Struct{
	uint32 slotid;
	uint32 quantity;
	uint32 unknown;
};

struct TraderClick_Struct{
	uint32 traderid;
	uint32 unknown4[2];
	uint32 approval;
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

struct GuildMemberEntry_Struct {
	char	name[1];					//variable length
	uint32	level;						//network byte order
	uint32	banker;						//1=yes, 0=no, network byte order
	uint32	class_;						//network byte order
	uint32	rank;						//network byte order
	uint32	time_last_on;				//network byte order
	uint32	tribute_enable;				//network byte order
	uint32	total_tribute;				//total guild tribute donated, network byte order
	uint32	last_tribute;				//unix timestamp
	char	public_note[1];				//variable length.
	uint16	zoneinstance;				//network byte order
	uint16	zone_id;					//network byte order
/* 38 + strings */
};

struct GuildMembers_Struct {	//just for display purposes, this is not actually used in the message encoding.
	char	player_name[1];		//variable length.
	uint32	count;				//network byte order
	GuildMemberEntry_Struct member[0];
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
	char	note[100]; //we are cutting this off at 100, actually around 252
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
/*0308*/	uint32	type;
/*0312*/	char	unknown312[2144];
/*2456*/	char	bug[1024];
/*3480*/	char	placeholder[2];
/*3482*/	char	system_info[4098];
};

struct Ground_Spawn{
	float max_x;
	float max_y;
	float min_x;
	float min_y;
	float max_z;
	float heading;
	char name[16];
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
   char	desc[0];
};

struct TributeInfo_Struct {
	uint32	active;		//0 == inactive, 1 == active
	uint32	tributes[MAX_PLAYER_TRIBUTES];	//-1 == NONE
	uint32	tiers[MAX_PLAYER_TRIBUTES];		//all 00's
	uint32	tribute_master_id;
};

struct TributeItem_Struct {
	uint32   slot;
	uint32   quantity;
	uint32   tribute_master_id;
	int32  tribute_points;
};

struct TributePoint_Struct {
	int32   tribute_points;
	uint32   unknown04;
	int32   career_tribute_points;
	uint32   unknown12;
};

struct TributeMoney_Struct {
	uint32   platinum;
	uint32   tribute_master_id;
	int32   tribute_points;
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
	uint32	skill_id;
	uint32	skill_value;
	char	title[1];
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

struct TaskMemberList_Struct {
/*00*/  uint32  gopher_id;
/*04*/  uint32  unknown04;
/*08*/  uint32  member_count;   //1 less than the number of members
/*12*/  char	list_pointer[0];
/*	list is of the form:
	char member_name[1]	//null terminated string
	uint8   task_leader	//boolean flag
*/
};


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

struct AcceptNewTask_Struct {
	uint32	task_id;		//set to 0 for 'decline'
	uint32	task_master_id;	//entity ID
};

//was all 0's from client, server replied with same op, all 0's
struct CancelTask_Struct {
	uint32 unknown0;
	uint32 unknown4;
};

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
/*00*/	uint32	group_leadership_exp;
/*04*/	uint32	group_leadership_points;
/*08*/	uint32	raid_leadership_exp;
/*12*/	uint32	raid_leadership_points;
};

struct UpdateLeadershipAA_Struct {
/*00*/	uint32	ability_id;
/*04*/	uint32	new_rank;
/*08*/	uint32	pointsleft;
/*12*/
};


struct RaidGeneral_Struct {
/*00*/	uint32		action;	//=10
/*04*/	char		player_name[64];	//should both be the player's name
/*04*/	char		leader_name[64];
/*132*/	uint32		parameter;
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
/*64*/  int32  unknown64;  //-1
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
/*05*/	uint8	unknown05[35];
};

struct BandolierSet_Struct {
/*00*/	uint32	action;
/*04*/	uint8	number;
/*05*/	uint8	unknown05[35];
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
/*096*/	uint8	unknown096[5];
/*101*/	char	model_name[16];
/*117*/	uint8	unknown117[19];
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

struct BuffFadeMsg_Struct {
	uint32 color;
	char msg[1];
};

struct UseAA_Struct {
	uint32 begin;
	uint32 ability;
	uint32 end;
};

struct AA_Ability {
/*00*/	uint32 skill_id;
/*04*/	uint32 base1;
/*08*/	uint32 base2;
/*12*/	uint32 slot;
};

struct SendAA_Struct {
/*0000*/	uint32 id;
/*0004*/	uint32 hotkey_sid;
/*0008*/	uint32 hotkey_sid2;
/*0012*/	uint32 title_sid;
/*0016*/	uint32 desc_sid;
/*0020*/	uint32 class_type;
/*0024*/	uint32 cost;
/*0028*/	uint32 seq;
/*0032*/	uint32 current_level; //1s, MQ2 calls this AARankRequired
/*0036*/	uint32 prereq_skill;		//is < 0, abs() is category #
/*0040*/	uint32 prereq_minpoints; //min points in the prereq
/*0044*/	uint32 type;
/*0048*/	uint32 spellid;
/*0052*/	uint32 spell_type;
/*0056*/	uint32 spell_refresh;
/*0060*/	uint16 classes;
/*0062*/	uint16 berserker; //seems to be 1 if its a berserker ability
/*0064*/	uint32 max_level;
/*0068*/	uint32 last_id;
/*0072*/	uint32 next_id;
/*0076*/	uint32 cost2;
/*0080*/	uint32 unknown80[2]; //0s
/*0084*/	uint32 total_abilities;
/*0088*/	AA_Ability abilities[0];
};

struct AA_List {
	SendAA_Struct* aa[0];
};

struct AA_Action {
/*00*/	uint32	action;
/*04*/	uint32	ability;
/*08*/	uint32	unknown08;
/*12*/	uint32	exp_value;
};

struct AA_Skills {		//this should be removed and changed to AA_Array
/*00*/	uint32	aa_skill;
/*04*/	uint32	aa_value;
};

struct AAExpUpdate_Struct {
/*00*/	uint32 unknown00;	//seems to be a value from AA_Action.ability
/*04*/	uint32 aapoints_unspent;
/*08*/	uint8 aaxp_percent;	//% of exp that goes to AAs
/*09*/	uint8 unknown09[3];	//live dosent always zero these, so they arnt part of aaxp_percent
};


struct AltAdvStats_Struct {
/*000*/  uint32 experience;
/*004*/  uint16 unspent;
/*006*/  uint16	unknown006;
/*008*/  uint8	percentage;
/*009*/  uint8	unknown009[3];
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

struct GuildMemberUpdate_Struct {
/*00*/	uint32 guild_id;	//not sure
/*04*/	char	member_name[64];
/*68*/	uint16	zone_id;
/*70*/	uint16	instance_id;
/*72*/	uint32	unknown072;
};






	};	//end namespace structs
};	//end namespace Client62

#endif /*CLIENT62_STRUCTS_H_*/










