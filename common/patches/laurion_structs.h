#ifndef LAURION_STRUCTS_H_
#define LAURION_STRUCTS_H_

namespace Laurion {
	namespace structs {
		// constants
		static const uint32 MAX_PP_AA_ARRAY = 300;
		static const uint32 MAX_PP_SKILL = PACKET_SKILL_ARRAY_SIZE;
		static const uint32 MAX_PP_INNATE_SKILL = 25;
		static const uint32 MAX_PP_DISCIPLINES = 300;
		static const uint32 MAX_PP_COMBAT_ABILITY_TIMERS = 25;
		static const uint32 MAX_PP_UNKNOWN_ABILITIES = 25;
		static const uint32 MAX_RECAST_TYPES = 25;
		static const uint32 MAX_ITEM_RECAST_TYPES = 100;
		static const uint32 BUFF_COUNT = 62;
		static const uint32 MAX_PP_LANGUAGE = 32;
#pragma pack(1)
		
		struct LoginInfo_Struct {
			/*000*/	char	login_info[64];
			/*064*/	uint8	unknown064[124];
			/*188*/	uint8	zoning;			// 01 if zoning, 00 if not
			/*189*/	uint8	unknown189[275];
			/*488*/
		};

		struct ClientZoneEntry_Struct {
			/*00*/ uint32	unknown00;	// ***Placeholder
			/*04*/ char	char_name[64];	// Player firstname [32]
			/*68*/ uint32	unknown68;
			/*72*/ uint32	unknown72;
			/*76*/ uint32	unknown76;
			/*80*/ uint32	unknown80;
			/*84*/ uint32	unknown84;
			/*88*/ uint32	unknown88;
			/*92*/
		};

		struct Membership_Struct
		{
			/*000*/ uint8 membership; //0 not gold, 2 gold
			/*001*/ uint32 races;	// Seen ff ff 01 00
			/*005*/ uint32 classes;	// Seen ff ff 01 00
			/*009*/ uint32 entrysize; // Seen 33
			/*013*/ int32 entries[33]; // Most -1, 1, and 0 for Gold Status
			/*145*/
		};

		struct Membership_Entry_Struct
		{
			/*000*/ uint32 purchase_id;		// Seen 1, then increments 90287 to 90300
			/*004*/ uint32 bitwise_entry;	// Seen 16 to 65536 - Skips 4096
			/*008*/
		};

		struct Membership_Setting_Struct
		{
			/*000*/ int8 setting_index;	// 0, 1, 2 or 3: f2p, silver, gold, platinum?
			/*001*/ int32 setting_id;		// 0 to 23 actually seen but the OP_Membership packet has up to 32
			/*005*/ int32 setting_value;	
			/*009*/
		};

		struct Membership_Details_Struct
		{
			/*000*/ uint32 membership_setting_count;	// Seen 96
			/*004*/ Membership_Setting_Struct settings[96]; // 864 Bytes
			/*364*/ uint32 race_entry_count;	// Seen 17
			/*368*/ Membership_Entry_Struct membership_races[17]; // 136 Bytes
			/*3f0*/ uint32 class_entry_count;	// Seen 15
			/*3f4*/ Membership_Entry_Struct membership_classes[17]; // 136 Bytes
			/*47c*/ uint32 exit_url_length;	// Length of the exit_url string (0 for none)
			/*480*/ //char exit_url[42];		// Upgrade to Silver or Gold Membership URL
		};

		struct MaxCharacters_Struct {
			/*000*/ uint32 max_chars;
			/*004*/ uint32 marketplace_chars;
			/*008*/ int32 unknown008; //some of these probably deal with heroic characters or something
			/*00c*/ int32 unknown00c;
			/*010*/ int32 unknown010;
			/*014*/ int32 unknown014;
			/*018*/ int32 unknown018;
			/*01c*/ int32 unknown01c;
			/*020*/ int32 unknown020;
			/*024*/ int32 unknown024;
			/*028*/ int32 unknown028;
			/*02c*/ int32 unknown02c;
			/*030*/ int32 unknown030;
			/*034*/ int32 unknown034;
			/*038*/
		};

		struct ExpansionInfo_Struct {
			/*000*/	char	Unknown000[64];
			/*064*/	uint32	Expansions;
		};

		/*
		* Visible equiptment.
		* Size: 20 Octets
		*/
		struct Texture_Struct
		{
			uint32 Material;
			uint32 Unknown1;
			uint32 EliteMaterial;
			uint32 HeroForgeModel;
			uint32 Material2;	// Same as material?
		};

		/*
		** Color_Struct
		** Size: 4 bytes
		** Used for convenience
		** Merth: Gave struct a name so gcc 2.96 would compile
		**
		*/
		struct Tint_Struct
		{
			union {
				struct {
					uint8 Blue;
					uint8 Green;
					uint8 Red;
					uint8 UseTint;	// if there's a tint this is FF
				};
				uint32 Color;
			};
		};

		struct CharSelectEquip : Texture_Struct, Tint_Struct {};

		struct CharacterSelectEntry_Struct
		{
			char Name[1];
			uint32 Class;
			uint32 Race;
			uint8 Level;
			uint32 ShroudClass;
			uint32 ShroudRace;
			uint16 Zone;
			uint16 Instance;
			uint8 Gender;
			uint8 Face;
			CharSelectEquip Equip[9];
			uint8 Unknown1; //Seen 256
			uint8 Unknown2; //Seen 0
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
			uint8 Enabled;
			uint8 Tutorial;
			uint32 DrakkinHeritage;
			uint8 Unknown3;
			uint8 GoHome;
			uint32 LastLogin;
			uint8 Unknown4; // Seen 0
			uint8 Unknown5; // Seen 0
			uint8 Unknown6; // Seen 0
			uint8 Unknown7; // Seen 0
			uint32 CharacterId; //A Guess, Character I made a little bit after has a number a few hundred after the first
			uint32 Unknown8; // Seen 1
		};

		/*
		** Character Selection Struct
		**
		*/
		struct CharacterSelect_Struct
		{
			/*000*/	uint32 CharCount;	//number of chars in this packet
		};

		struct SpawnAppearance_Struct
		{
			/*0000*/ uint32 spawn_id;		// ID of the spawn
			/*0004*/ uint32 type;			// Values associated with the type
			/*0008*/ uint32 parameter;		// Type of data sent
			/*0012*/ uint32 unknown012;
			/*0016*/ uint32 unknown016;
			/*0020*/ uint32 unknown020;
			/*0024*/
		};

		struct Spawn_Struct_Bitfields
		{
			union {
				struct {
					// byte 1
					/*00*/	unsigned   gender : 2;		// Gender (0=male, 1=female, 2=monster)
					/*02*/	unsigned   ispet : 1;			// Guessed based on observing live spawns
					/*03*/	unsigned   afk : 1;			// 0=no, 1=afk
					/*04*/	unsigned   anon : 2;			// 0=normal, 1=anon, 2=roleplay
					/*06*/	unsigned   gm : 1;
					/*07*/	unsigned   sneak : 1;
					// byte 2
					/*08*/	unsigned   lfg : 1;
					/*09*/	unsigned   unk9 : 1;
					/*10*/	unsigned   invis : 12;		// there are 3000 different (non-GM) invis levels
					/*22*/	unsigned   linkdead : 1;		// 1 Toggles LD on or off after name. Correct for RoF2
					/*23*/	unsigned   showhelm : 1;
					// byte 4
					/*24*/	unsigned   betabuffed : 1;		// Prefixes name with !
					/*25*/	unsigned   trader : 1;
					/*26*/	unsigned   animationonpop : 1;
					/*27*/	unsigned   targetable : 1;
					/*28*/	unsigned   targetable_with_hotkey : 1;
					/*29*/	unsigned   showname : 1;
					/*30*/	unsigned   idleanimationsoff : 1; // what we called statue?
					/*31*/	unsigned   untargetable : 1;	// bClickThrough
					// byte 5
					/*32*/	unsigned   buyer : 1;
					/*33*/	unsigned   offline : 1;
					/*34*/	unsigned   interactiveobject : 1;
					/*35*/	unsigned   missile : 1;
					/*36*/	unsigned   title : 1;
					/*37*/	unsigned   suffix : 1;
					/*38*/	unsigned   unk38 : 1;
					/*39*/	unsigned   unk39 : 1;
				};
				uint8 raw[5];
			};
		};

		struct Spawn_Struct_Position
		{
			union {
				struct {
					signed y : 19;
					signed deltaX : 13;

					unsigned heading : 12;
					signed z : 19;
					unsigned pad1 : 1;

					unsigned pitch : 12;
					signed animation : 10;    //these might be swapped
					signed deltaHeading : 10; //these might be swapped

					signed deltaY : 13;
					signed deltaZ : 13;
					unsigned pad3 : 6;

					signed x : 19;
					unsigned pad4 : 13;
				};
				uint32_t raw[5];
			};
		};

		struct Client_Position
		{
			/*00*/ float delta_x;
			/*04*/ float x;
			/*08*/ float z;
			/*12*/ signed animation : 10;
			       unsigned pitch : 12;
			       signed padding1 : 10;
			/*16*/ float delta_y;
			/*20*/ float y;
			/*24*/ signed delta_heading : 10;
				   signed heading : 12;
				   signed padding2 : 10;
			/*28*/ float delta_z;
			/*32*/ 
		};

		struct PlayerPositionUpdateServer_Struct
		{
			/*00*/ uint16 spawn_id;
			/*02*/ uint16 vehicle_id;
			/*04*/ Spawn_Struct_Position position;
			/*24*/
		};

		struct PlayerPositionUpdateClient_Struct {
			/*00*/ uint16 sequence;
			/*02*/ uint16 spawn_id;
			/*04*/ uint16 vehicle_id;
			/*06*/ Client_Position position;
			/*38*/
		};

		struct Door_Struct
		{
			/*000*/ char name[32];
			/*032*/ float DefaultY;
			/*036*/ float DefaultX;
			/*040*/ float DefaultZ;
			/*044*/ float DefaultHeading;
			/*048*/ uint32 DefaultDoorAngle; //rof2's incline
			/*052*/ float Y; //most (all I've seen?) doors match the defaults here
			/*056*/ float X;
			/*060*/ float Z;
			/*064*/ float Heading;
			/*068*/ float DoorAngle; //not sure if this is actually a float; it might be a uint32 like DefaultDoorAngle
			/*072*/ uint32 ScaleFactor; //rof2's size
			/*076*/ uint32 Unknown76; //client doesn't seem to read this
			/*080*/ uint8 Id; //doorid
			/*081*/ uint8 Type; //opentype
			/*082*/ uint8 State; //state_at_spawn
			/*083*/ uint8 DefaultState; //invert_state
			/*084*/ int32 Param; //door_param
			/*088*/ uint32 AdventureDoorId;
			/*092*/ uint32 DynDoorID;
			/*096*/ uint32 RealEstateDoorID;
			/*100*/ uint8 bHasScript;
			/*101*/ uint8 bUsable; //1 if clickable
			/*102*/ uint8 bRemainOpen;
			/*103*/ uint8 bVisible; //1 is visible
			/*104*/ uint8 bHeadingChanged;
			/*105*/ uint8 padding1[3];
			/*108*/ float TopSpeed1;
			/*112*/ float TopSpeed2;
			/*116*/ uint8 bNeedsTimeStampSet;
			/*117*/ uint8 padding2[3];
			/*120*/ float unknownFloat1;
			/*124*/ float unknownFloat2;
			/*128*/ uint8 unknownByte1;
			/*129*/ uint8 padding3[3];
			/*132*/
		};

		struct ZonePoint_Entry {
			/*00*/ uint32 iterator;
			/*04*/ float y;
			/*08*/ float x;
			/*12*/ float z;
			/*16*/ float heading;
			/*20*/ uint16 zoneid;
			/*22*/ uint16 zoneinstance;
			/*24*/ uint32 unknown024;
			/*28*/ uint32 unknown028;
			/*32*/
		};

		struct ZonePoints {
			/*00*/ uint32 count;
			/*04*/ struct ZonePoint_Entry zpe[0]; // Always add one extra to the end after all zonepoints
		};

		struct EnterWorld_Struct {
			/*000*/	char	name[64];
			/*064*/	int32	unknown1;
			/*068*/	int32	unknown2; //laurion handles these differently so for now im just going to ignore them till i figure it out
		};

		struct ZoneChange_Struct {
			/*000*/	char	char_name[64];     // Character Name
			/*064*/	uint16	zoneID;
			/*066*/	uint16	instanceID;
			/*068*/	uint32  Unknown068;
			/*072*/	uint32  Unknown072;
			/*076*/	float	y;
			/*080*/	float	x;
			/*084*/	float	z;
			/*088*/	uint32	zone_reason;	//0x0A == death, I think
			/*092*/	int32	success;		// =0 client->server, =1 server->client, -X=specific error
			/*096*/ uint32	Unknown096;	// Not sure the extra 4 bytes goes here or earlier in the struct.
			/*100*/
		};

		struct RequestClientZoneChange_Struct {
			/*000*/	uint16	zone_id;
			/*002*/	uint16	instance_id;
			/*004*/	uint32	unknown004;
			/*008*/	float	y;
			/*012*/	float	x;
			/*016*/	float	z;
			/*020*/	float	heading;
			/*024*/	uint32	type;	//unknown... values
			/*032*/	uint8	unknown032[144];
			/*172*/	uint32	unknown172;
			/*176*/
		};

		struct WearChange_Struct {
			/*000*/ uint32 spawn_id;
			/*004*/ uint32 wear_slot_id;
			/*008*/ uint32 armor_id;
			/*012*/ uint32 variation;
			/*016*/ uint32 material;
			/*020*/ uint32 new_armor_id;
			/*024*/ uint32 new_armor_type;
			/*028*/ uint32 color;
			/*032*/
		};

		struct ExpUpdate_Struct
		{
			/*000*/ uint64 exp; //This is exp % / 1000 now; eg 69250 = 69.25%
			/*008*/ uint64 unknown; //unclear, I didn't see the client actually read this value but i might have missed it
		};

		struct DeleteSpawn_Struct
		{
			/*00*/ uint32 spawn_id;		// Spawn ID to delete
			/*04*/ uint8 unknown04;		// Seen 1
			/*05*/
		};

		//OP_SetServerFilter
		struct SetServerFilter_Struct {
			uint32 filters[68];
		};

		// Was new to RoF2, doesn't look changed
		// The padding is because these structs are padded to the default 4 bytes
		struct InventorySlot_Struct
		{
			/*000*/	int16 Type;
			/*002*/	int16 Padding1;
			/*004*/	int16 Slot;
			/*006*/	int16 SubIndex;
			/*008*/	int16 AugIndex;
			/*010*/	int16 Padding2;
			/*012*/
		};

		// Was new for RoF2 - Used for Merchant_Purchase_Struct, doesn't look changed
		// Can't sellfrom other than main inventory so Slot Type is not needed.
		// The padding is because these structs are padded to the default 4 bytes
		struct TypelessInventorySlot_Struct
		{
			/*000*/	int16 Slot;
			/*002*/	int16 SubIndex;
			/*004*/	int16 AugIndex;
			/*006*/	int16 Padding;
			/*008*/
		};

		struct Consider_Struct {
			/*000*/ uint32	playerid;               // PlayerID
			/*004*/ uint32	targetid;               // TargetID
			/*008*/ uint32	faction;                // Faction
			/*012*/ uint32	level;					// Level
			/*016*/ uint32	report_mode;			// 0 normally, 4 will do a more detailed report that only works if you have GM flag set
			/*020*/ uint8	rare_creature;			// Will do the rare creature string
			/*021*/ uint8	loot_locked;			// Will list the target as (loot locked)
			/*022*/ uint8	unknown022;				// Padding probably
			/*023*/ uint8	unknown023;				// Padding probably
			/*024*/
		};

		struct ChangeSize_Struct
		{
			/*00*/ uint32 EntityID;
			/*04*/ float Size;
			/*08*/ uint32 Unknown08;	// Observed 0
			/*12*/ float Unknown12;		// Observed 1.0f
			/*16*/
		};

		struct SpawnHPUpdate_Struct
		{
			/*00*/ int16	spawn_id;
			/*02*/ int64	cur_hp;
			/*10*/ int64	max_hp;
			/*18*/
		};

#pragma pack()

	};	//end namespace structs
};	//end namespace laurion

#endif /*LAURION_STRUCTS_H_*/
