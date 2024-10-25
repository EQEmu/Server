#ifndef LARION_STRUCTS_H_
#define LARION_STRUCTS_H_

namespace Larion {
	namespace structs {
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
			/*000*/ uint8 setting_index;	// 0, 1, 2 or 3: f2p, silver, gold, platinum?
			/*001*/ uint32 setting_id;		// 0 to 23 actually seen but the OP_Membership packet has up to 32
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

	};	//end namespace structs
};	//end namespace larion

#endif /*LARION_STRUCTS_H_*/










