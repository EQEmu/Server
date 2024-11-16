#ifndef LARION_STRUCTS_H_
#define LARION_STRUCTS_H_

namespace Larion {
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
				uint8_t data[5];
			};
		};
#pragma pack()

	};	//end namespace structs
};	//end namespace larion

#endif /*LARION_STRUCTS_H_*/
