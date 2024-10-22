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

		struct LogServer_Struct {
			//As far as I can tell the client does not read this field
			/*000*/	uint32	unknown000;
			//client sets this to 140e32edc, confirmed it deals with pvp via disassembly aka hasn't moved
			/*004*/	uint8	enable_pvp;
			/*005*/	uint8	unknown005;
			/*006*/	uint8	unknown006;
			/*007*/	uint8	unknown007;
			//client sets this to 140e32ee0, this was set to 1 on special ruleset servers but 0 on non-special ruleset servers
			/*008*/	uint8	special_ruleset;
			//client sets this to 140e3bedd
			//I've seen 0 on normal servers
			//it deals with char select and # of char slots somehow
			/*009*/	uint8	unknown009;
			//client sets this to 140e32ee1, confirmed via disassembly to be enable_FV
			/*00a*/	uint8	enable_FV;
			//client sets this to 140e3bf0, seems to affect the made by label on items
			//always has been 0 for me
			/*00b*/	uint8	enable_made_by;
			//client sets this to 140e3bf09
			//I see it being written to but not read in code
			//I've only ever seen it passed as 1
			/*00c*/	uint8	unknown00c;
			//client sets this to 140e3bf0a
			//I've only ever seen it passed as 1
			//0 seems to disable something about languages?
			/*00d*/	uint8	languages_enabled;
			/*00e*/	uint8	unknown00e;
			/*00f*/	uint8	unknown00f;
			/*010*/	uint8	unknown010;
			/*011*/	uint8	unknown011;
			/*012*/	uint8	unknown012;
			/*013*/	uint8	unknown013;
			/*014*/	uint8	unknown014;
			/*015*/	char	worldshortname[32];
			/*035*/ uint8   unknown035[11];
			/*040*/ uint8   unknown040[16];
			/*050*/ uint8   unknown050[16];
			/*060*/ uint8   unknown060[16];
			/*070*/ uint8   unknown070[16];
			/*080*/ uint8   unknown080[16];
			/*090*/ uint8   unknown090[16];
			/*0a0*/ uint8   unknown0a0[16];
			/*0b0*/ uint8   unknown0b0[16];
			/*0c0*/ uint8   unknown0c0[16];
			/*0d0*/ uint8   unknown0d0[16];
			/*0e0*/ uint8   unknown0e0[16];
			/*0f0*/ uint8   unknown0f0[16];
			/*100*/ uint8   unknown100[16];
			/*110*/ uint8   unknown110[16];
			/*120*/ uint8   unknown120[16];
			/*130*/ uint8   unknown130[16];
			/*140*/ uint8   unknown140[16];
			/*140*/ uint8   unknown140[16];
			/*150*/ uint8   unknown150[16];
			/*160*/ uint8   unknown160[16];
			/*170*/ uint8   unknown170[16];
			/*180*/ uint8   unknown180[16];
			/*190*/ uint8   unknown190[16];
			/*1a0*/ uint8   unknown1a0[16];
			/*1b0*/ uint8   unknown1b0[16];
			/*1c0*/ uint8   unknown1c0[16];
			/*1d0*/ uint8   unknown1d0[16];
			/*1e0*/ uint8   unknown1e0[16];
			/*1f0*/ uint8   unknown1f0[16];
			/*200*/ uint8   unknown200[16];
			/*210*/ uint8   unknown210[16];
			/*220*/ uint8   unknown220[16];
			/*230*/ uint8   unknown230[16];
			/*240*/ uint8   unknown240[16];
			/*240*/ uint8   unknown240[16];
			/*250*/ uint8   unknown250[16];
			/*260*/ uint8   unknown260[16];
			/*270*/ uint8   unknown270[16];
			/*280*/ uint8   unknown280[16];
			/*290*/ uint8   unknown290[16];
			/*2a0*/ uint8   unknown2a0[16];
			/*2b0*/ uint8   unknown2b0[16];
			/*2c0*/ uint8   unknown2c0[16];
			/*2d0*/ uint8   unknown2d0[16];
			/*2e0*/ uint8   unknown2e0[16];
			/*2f0*/ uint8   unknown2f0[16];
			/*300*/ uint8   unknown300[16];
			/*310*/ uint8   unknown310[16];
			/*320*/ uint8   unknown320[16];
			/*330*/ uint8   unknown330[16];
			/*340*/ uint8   unknown340[16];
			/*340*/ uint8   unknown340[16];
			/*350*/ uint8   unknown350[16];
			/*360*/ uint8   unknown360[16];
			/*370*/ uint8   unknown370[16];
			/*380*/ uint8   unknown380[16];
			/*390*/ uint8   unknown390[16];
			/*3a0*/ uint8   unknown3a0[16];
			/*3b0*/ uint8   unknown3b0[16];
			/*3c0*/ uint8   unknown3c0[16];
			/*3d0*/ uint8   unknown3d0[16];
			/*3e0*/ uint8   unknown3e0[16];
			/*3f0*/ uint8   unknown3f0[16];
			/*400*/ uint8   unknown400[16];
			/*410*/ uint8   unknown410[16];
			/*420*/ uint8   unknown420[16];
			/*430*/ uint8   unknown430[16];
			/*440*/ uint8   unknown440[16];
			/*440*/ uint8   unknown440[16];
			/*450*/ uint8   unknown450[16];
			/*460*/ uint8   unknown460[16];
			/*470*/ uint8   unknown470[16];
			/*480*/ uint8   unknown480[16];
			/*490*/ uint8   unknown490[16];
			/*4a0*/ uint8   unknown4a0[16];
			/*4b0*/ uint8   unknown4b0[16];
			/*4c0*/ uint8   unknown4c0[16];
			/*4d0*/ uint8   unknown4d0[16];
			/*4e0*/ uint8   unknown4e0[16];
			/*4f0*/ uint8   unknown4f0[16];
			/*500*/ uint8   unknown500[16];
			/*510*/ uint8   unknown510[16];
			/*520*/ uint8   unknown520[16];
			/*530*/ uint8   unknown530[16];
			/*540*/ uint8   unknown540[16];
			/*540*/ uint8   unknown540[16];
			/*550*/ uint8   unknown550[16];
			/*560*/ uint8   unknown560[16];
			/*570*/ uint8   unknown570[16];
			/*580*/ uint8   unknown580[16];
			/*590*/ uint8   unknown590[16];
			/*5a0*/ uint8   unknown5a0[12];
			//Client reads this and sets it to 140e32ee2
			//I've only ever seen 0, it affects some calculation in the client
			/*5ac*/ uint8   unknown5ac;
			/*5ad*/ uint8   unknown5ad[3];
			/*5b0*/ uint8   unknown5b0[3];
			//Client reads this and sets it to 140e3bf18
			//seems to enable the voice system
			/*5b4*/ uint8   enable_voice_system;
			//Client reads this and sets it to 140e3bef0
			//Seems to enable the mail system
			/*5b5*/ uint8   enable_mail;
			//Client reads this and sets it to 140e3bef1
			//Not entirely sure what this does, it seems to be related to character create
			//I've only ever seen it set to 0
			/*5b6*/ uint8   unknown5b6;
			/*5b7*/ uint8   unknown5b7;
			//Client reads this and sets it to 140e3bef2
			//Not entirely sure what this does, it seems to be related to character create
			//Seen set to 1
			/*5b8*/ uint8   unknown5b8;
			/*5b9*/ uint8   unknown5b9[8];
			//Client sets this to 140e3beec
			//Seems to help enable beta buff command
			/*5c0*/ uint8   beta_buff_enabled;
			//Client sets this to 140e3beed
			//Guessing this is for marking as a beta server
			/*5c1*/ uint8   is_beta_server;
			//Client sets this to 140e3beee
			//Test sets this to one and all 3 of these need to be 1 to use /betabuff
			/*5c2*/ uint8   is_test_server;
			//Client sets this to 140e3beef
			//Client writes but doesn't read this anywhere as far as I can tell
			/*5c3*/ uint8   unknown5c3;
			/*5c4*/ uint8   unknown5c4[12];
			/*5d0*/ uint8   unknown5d0[16];
			/*5e0*/ uint8   unknown5e0[16];
			/*5f0*/ uint8   unknown5f0[12];
			//Client sets this to 140e3d5f4
			//Not sure what this affects
			//I've seen ff ff ff ff passed
			/*5fc*/ int32   unknown5fc;
			//Client sets this to 140e3d5f4
			//not sure what this does
			//Ive seen 0 on normal servers but 1 on test
			/*600*/ int32   unknown600;
			/*604*/ uint8   unknown604[12];
			/*610*/ uint8   unknown610[16];
			/*620*/ uint8   unknown620[16];
			/*630*/ uint8   unknown630[16];
			/*640*/ uint8   unknown640[16];
			/*640*/ uint8   unknown640[16];
			/*650*/ uint8   unknown650[16];
			/*660*/ uint8   unknown660[16];
			/*670*/ uint8   unknown670[16];
			/*680*/ uint8   unknown680[16];
			/*690*/ uint8   unknown690[16];
			/*6a0*/ uint8   unknown6a0[16];
			/*6b0*/ uint8   unknown6b0[16];
			/*6c0*/ uint8   unknown6c0[16];
			/*6d0*/ uint8   unknown6d0[16];
			/*6e0*/ uint8   unknown6e0[16];
			/*6f0*/ uint8   unknown6f0[16];
			/*700*/ uint8   unknown700[16];
			/*710*/ uint8   unknown710[14];
			//Client reads this and sets it to 140e32ee2
			//based on strings near related disassembly im guessing this controls
			//which kind of items are allowed (there are strings for heirloom, prestige, etc)
			//every server ive logged in so far has had this to 0 but I haven't tried a TLP yet
			/*71d*/ uint8   heirlooms_disabled;
			/*71e*/ uint8   unknown71e;
			/*720*/ uint8   unknown720[16];
		};

		struct ClientZoneEntry_Struct {
			/*00*/ uint32	unknown00;	// ***Placeholder
			/*04*/ char	char_name[64];	// Player firstname [32]
			/*68*/ uint32	unknown68;
			/*72*/ uint32	unknown72;
		};
	};	//end namespace structs
};	//end namespace larion

#endif /*LARION_STRUCTS_H_*/










