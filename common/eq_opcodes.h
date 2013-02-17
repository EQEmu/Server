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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  04111-1307  USA
*/
#ifndef EQ_OPCODES_H
#define EQ_OPCODES_H


//THIS FILE IS NOT USED ANYMORE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

/*
*
*
*
*
// solar: updated 2/12/04
//
// Invalid opcodes have been \t'd out, confirmed have no \t
// all the ops have the first nibble as 0, so anything that's not 0x0???
// is just an invalid opcode.  if an opcode is known to be wrong set it to
// this to avoid conflicts with real ops
//
// codes preceded by '//' are ones that are known to be right but not used
// by any code yet
//
// codes preceded by '// not used' are ones that are probably wrong and not
// used anywhere
//


//im feeling lazy today and I dont wanna find-replace this include
#include "emu_opcodes.h"



//////////////////////////////////////
// Zone.exe opcodes:
//////////////////////////////////////
//0x029f gives you money
//280 makes you stop receiving and sending chat messages
//260 send spell information
//45 logout
//72 player is in zone blah at dfdsfdsfg
//80 The door says
//89 Your expansion settings are:
//104 scribes a spell to your spellbook
//108,555 kicks player to server select
//123 money split from groups
//11 sense trap response
//296 name approval question
//338 failed to create new player guild
//401 pk question, 0x0191 response
//411 sacrifice question, 0x019b response
//193,213,243,244,431 chat messages
//425,499,505,506,525,526,596 crash
//451 translocate question, 0x01c3 response
//474,537 you escape from combat hiding yourself from view
//492 stat buffs
//496 your petition text is:
//581 you receive money msg with money
//562 you have control of yourself again
//613 CTD
//686 removes the unencumbered
#define LiveOP_Heartbeat		0x0176	// client sends this periodically
#define LiveOP_ReloadUI			0x02d7
#define LiveOP_IncreaseStats	0x01eb
#define LiveOP_ApproveZone	0x0134
#define LiveOP_Dye			0x01d5

// not used	#define LiveOP_ExpansionSetting	0x0203
// not used	#define LiveOP_GainMoney		0x0209
// not used	#define LiveOP_Sacrifice		0x019b
// not used	#define LiveOP_BecomePK			0x0191
#define LiveOP_Stamina		    0x0168
#define LiveOP_ControlBoat		0x014d
#define LiveOP_MobUpdate		0x003e
#define LiveOP_ClientUpdate		0x0027
#define LiveOP_ChannelMessage	0x0024
#define LiveOP_SimpleMessage    0x01d7
#define LiveOP_FormattedMessage 0x01d8
//#define LiveOP_RaidInvite		0x01e4 	// wrong. from seq - not used by eqemu
// #define LiveOP_RaidJoin		0x01e5	// from seq - not used by eqemu
	// not used	#define LiveOP_ApplyPoison		0x00b7
#define LiveOP_TGB				0x01c6 // /targetgroupbuff
// #define LiveOP_CharInfo			0x0012 // /charinfo
// #define LiveOP_Movelog			0x0290 // /movelog
// #define LiveOP_Beta				0x02cb // /beta
#define LiveOP_TestBuff			0x0285 // /testbuffme
// #define LiveOP_Key				0x01e2 // /keys
#define LiveOP_Bind_Wound		0x012d
#define LiveOP_Charm			0x01ab
#define LiveOP_Begging			0x014c
#define LiveOP_MoveCoin			0x0152
#define LiveOP_SpawnDoor		0x0292
#define LiveOP_Sneak			0x009d // Clicked sneak - Doodman 10/10/2003
#define LiveOP_ExpUpdate		0x0079
#define LiveOP_DumpName			0x027d //no idea what this is: just tired of looking at it as unknown; updated by Shawn319
// #define LiveOP_UpdateAA			0x0222
#define LiveOP_RespondAA		0x01ea // AA table
#define LiveOP_SendAAStats		0x01c9
#define LiveOP_SendAATable		0x0366
#define LiveOP_AAAction         0x01e9 // Used for changing percent, buying? and activating skills
#define LiveOP_BoardBoat		0x00bb
#define LiveOP_LeaveBoat		0x00bc

#define LiveOP_AdventureInfoRequest	0x02b8 //Cofruben:received when client click on the recruiter.
#define LiveOP_AdventureInfo	0x02b9 //Cofruben:sent when client right click on the recruiter.
#define LiveOP_AdventureRequest	0x02a6 //Cofruben: received when client press request button.1 normal,2 hard
#define LiveOP_AdventureDetails 0x02a8 //Cofruben: sent when client press request button.
#define LiveOP_LDoNButton		0x02a9 //Cofruben: Received button.(uint8)00 decline,01 accept.
#define LiveOP_AdventureData	0x02ba //Cofruben: Sent when client press accept button.

#define LiveOP_AdventureFinish	0x02c9 //Cofruben:Used when you win/lose a dungeon
#define LiveOP_LeaveAdventure	0x02c6 //Cofruben:received when client press leave adventure button
#define LiveOP_AdventureUpdate	0x02ce


#define LiveOP_SendExpZonein		0x002b	// 0 length packets
#define LiveOP_ZoneInSendName		0x01e4
	//Guild Opcodes
#define LiveOP_GuildLeader		 0x01bf // /guildleader, was 00a7
#define LiveOP_GuildPeace		 0x009a // /guildpeace
#define LiveOP_GuildRemove		 0x0132 // /guildremove
#define LiveOP_GuildMemberList   0x0059 //fixed by Shawn319 (dec 18th patch)
#define LiveOP_GuildMemberUpdate 0x026e
#define LiveOP_GuildInvite		0x0130 // /guildinvite
#define LiveOP_GuildMOTD		0x01c0	// /guildmotd, was 01bf
	// not used	#define LiveOP_GuildManagement	0x005e	// LoY guild mgm't tool
#define LiveOP_GuildPublicNote		0x003c
#define LiveOP_GetGuildMOTD			0x027e	// /getguildmotd
#define LiveOP_GuildDemote			0x0277
#define LiveOP_GuildInviteAccept 0x0131
#define LiveOP_GuildWar			0x00a4 // /guildwar
		#define LiveOP_GuildUpdate		0x0b41
#define LiveOP_GuildDelete			0x0133
#define LiveOP_GuildManageRemove	0x0233
#define LiveOP_GuildManageAdd		0x022d
#define LiveOP_GuildManageStatus	0x0039
	//Bazaar
#define LiveOP_Trader			0x01e8 // /trader
#define LiveOP_Bazaar			0x01e7 // /bazaar search

#define LiveOP_BecomeTrader		0x01c4
#define	LiveOP_BazaarInspect	0x01f4
#define LiveOP_TraderItemUpdate	0x006e
#define	LiveOP_TraderDelItem	0x017c
#define LiveOP_TraderShop		0x01eb	// right-click on a trader in bazaar

#define LiveOP_TraderBuy		0x01ca	// buy from a trader in bazaar
		
#define LiveOP_PetCommands		0x01ac
#define LiveOP_TradeSkillCombine 0x0042
#define LiveOP_AugmentItem 0x02e5
#define LiveOP_ItemName 0x0367



//Shops
#define LiveOP_ShopItem			0x02cd	// Send merchant item data to client (header = 0x64)
#define LiveOP_ShopPlayerBuy	0x0065
	// not used	#define LiveOP_ShopTakeMoney	0x0066
#define LiveOP_ShopPlayerSell	0x006a
#define LiveOP_ShopDelItem		0x006d
#define LiveOP_ShopEndConfirm	0x0f6d
#define LiveOP_ShopRequest		0x00f7	// right-click on merchant
#define LiveOP_ShopEnd			0x006c	// Finished shopping at merchant

#define	LiveOP_AdventureMerchantRequest	0x02d1
#define	LiveOP_AdventureMerchantResponse	0x02d2
#define	LiveOP_AdventureMerchantPurchase	0x02d3
#define	LiveOP_AdventurePointsUpdate	0x02e3

// #define LiveOP_LFPCommand		     0x0272	// Looking for player
// #define LiveOP_LFPGetMatchesRequest  0x0273
// #define LiveOP_LFPGetMatchesResponse 0x0275

// #define LiveOP_LFGGetMatchesRequest  0x0271
// #define LiveOP_LFGGetMatchesResponse 0x0274
#define LiveOP_LFGCommand		     0x0270	// When client issues /LFG command
// #define LiveOP_LFGResponse           0x01b1
#define LiveOP_LFGAppearance	0x01d0	// Some other char in zone turns LFG on/off

#define	LiveOP_MoneyUpdate		0x01b5

#define LiveOP_GroupDelete		0x0721
#define	LiveOP_GroupAcknowledge	0x0272

#define LiveOP_GroupUpdate		0x024a
#define LiveOP_GroupInvite		0x025f
#define LiveOP_GroupDisband		0x00ff
#define LiveOP_GroupInvite2		0x00d5
#define LiveOP_GroupFollow		0x025e
#define LiveOP_GroupFollow2		0x00d7

#define LiveOP_GroupCancelInvite		0x00d6

#define LiveOP_Split			0x0156	// /split
#define LiveOP_Jump				0x00d8	// not used atm but will be when stamina is fixed
#define LiveOP_ConsiderCorpse	0x01d6
#define LiveOP_SkillUpdate		0x0064

#define LiveOP_GMEndTrainingResponse 0x0178
#define LiveOP_GMEndTraining	0x013c
#define LiveOP_GMTrainSkill		0x0175
#define LiveOP_GMTraining		0x013b

#define LiveOP_ConsumeAmmo		0x017b
#define LiveOP_CombatAbility	0x0171

#define LiveOP_TrackUnknown		0x009c
#define LiveOP_TrackTarget		0x0234
#define LiveOP_Track			0x0286
#define	LiveOP_ReadBook			0x0297

#define LiveOP_ItemLinkClick	0x001f
#define LiveOP_ItemLinkResponse	0x01f4
#define LiveOP_ItemLinkText		0x01d9

#define LiveOP_RezzRequest		0x0a41
#define LiveOP_RezzAnswer		0x00e5
#define LiveOP_RezzComplete		0x019b

#define	LiveOP_MoveDoor			0x0128
#define	LiveOP_ClickDoor		0x0127	// Click door
#define LiveOP_SendZonepoints	0x0247	// Coords in a zone that will port you to another zone
#define LiveOP_SetRunMode		0x008c	// Client hit the "run" button (or control+r)
#define LiveOP_InspectRequest	0x0248
#define LiveOP_InspectAnswer	0x0249
#define LiveOP_SenseTraps		0x0187  // Clicked sense traps - @Doodman 10/10/2003
#define LiveOP_DisarmTraps		0x018e  // Clicked disarm traps - @Doodman 10/10/2003
#define LiveOP_Assist			0x01bc
#define LiveOP_PickPocket		0x0240

#define LiveOP_LootRequest		0x0119
#define LiveOP_EndLootRequest	0x011a
#define LiveOP_MoneyOnCorpse	0x011b
#define LiveOP_LootComplete		0x0179
#define LiveOP_LootItem			0x013f
	// solar: there was an LiveOP_PlaceItem synonym for this
#define LiveOP_MoveItem		0x0151	// Client moving an item from one slot to another (user action)

#define LiveOP_WhoAllRequest   	0x0056
#define LiveOP_WhoAllResponse   0x0229
#define LiveOP_Consume			0x0167
#define LiveOP_AutoAttack		0x0172
#define LiveOP_AutoAttack2		0x0186
#define LiveOP_TargetMouse		0x0173	// mouse targetting a person (also: Pressing F* key to target)
#define LiveOP_TargetCommand	0x01ba	// /target user
#define LiveOP_TargetReject		0x01d8	// When /target fails (// solar: untested)
#define LiveOP_Hide				0x009e
#define LiveOP_Forage			0x012e // Clicked forage  - @Doodman 10/10/2003
#define LiveOP_Fishing 			0x0077	
// #define LiveOP_Adventure		0x02d0 // /adventure
// #define LiveOP_Feedback			0x0161	// /feedback
#define LiveOP_Bug				0x0246	// /bug
#define LiveOP_Emote			0x00f2	// /me goes blah
#define LiveOP_EmoteAnim		0x0140 // solar: untested
#define LiveOP_Consider			0x015c
#define LiveOP_FaceChange		0x01cb	// /face
	// not used	#define LiveOP_Report			0x01e0
#define LiveOP_RandomReq		0x0197
#define LiveOP_RandomReply		0x0087
#define LiveOP_Camp				0x01c3
#define LiveOP_YellForHelp		0x0192
#define LiveOP_SafePoint		0x00ef

#define LiveOP_Buff				0x0157
#define LiveOP_BuffFadeMsg		0x00c0
#define LiveOP_MultiLineMsg		0x0440	// is this still good for anything?
#define LiveOP_SpecialMesg		0x021c	// Communicate textual info to client
#define LiveOP_Consent			0x0013	// /consent
#define LiveOP_ConsentResponse	0x029d
#define LiveOP_Deny				0x02d4
#define LiveOP_Stun				0x016c
#define LiveOP_BeginCast		0x0021
#define LiveOP_CastSpell		0x00be
#define LiveOP_InterruptCast	0x01a8
#define LiveOP_Death			0x0105
#define LiveOP_FeignDeath		0x023f
#define LiveOP_Illusion			0x012b
#define LiveOP_LevelUpdate		0x0078
#define LiveOP_LevelAppearance	0x0371
	// not used	#define LiveOP_LocateCorpse     0x00d1  //Sent when a client casts Locate Corpse spells?

#define LiveOP_MemorizeSpell	0x00c2	// Memming a spell from book to spell slot
#define LiveOP_HPUpdate			0x0244	// Update HP % of a PC or NPC
#define LiveOP_SendHPTarget		0x022e
#define LiveOP_Mend				0x007d
	#define LiveOP_MendHPUpdate		0x009b
#define LiveOP_Taunt			0x0160

// #define LiveOP_Summoncorpse		0x02b5 // /summoncorpse
// #define LiveOP_GMSearchCorpse	0x0097	// GM /searchcorpse	- Search all zones for named corpse
// #define LiveOP_SearchCorpse		LiveOP_GMSearchCorpse	// /searchcorpse
#define LiveOP_GMDelCorpse		0x0199 // /delcorpse
#define LiveOP_GMFind			0x0047	// GM /find			- ?
// not used	#define LiveOP_FindResponse		0x02cc
#define LiveOP_GMServers		0x0020	// GM /servers		- ?
#define LiveOP_GMGoto			0x010b	// GM /goto			- Transport to another loc
#define LiveOP_GMSummon			0x028c	// GM /summon		- Summon PC to self
#define	LiveOP_GMKick			0x010a	// GM /kick			- Boot player
#define LiveOP_GMKill			0x0109	// GM /kill			- Insta kill mob/pc
#define LiveOP_GMNameChange		0x0b40 // /name
#define LiveOP_GMLastName		0x00a3	// GM /lastname		- Change user lastname
#define LiveOP_GMToggle			0x01b3	// GM /toggle		- Toggle ability to receive tells from other PC's
#define LiveOP_GMEmoteZone		0x028f	// GM /emotezone	- Send zonewide emote
#define LiveOP_GMBecomeNPC		0x0074	// GM /becomenpc	- Become an NPC
// (TODO: Use opcode 0x012d, which is also sent with LiveOP_GMBecomeNPC to create correct npc
// #define LiveOP_GMApproval		0x01b0	// GM /approval		- Name approval duty?
// not used	#define LiveOP_NameApproval		0x011f //Name approval
#define LiveOP_GMHideMe			0x00de	// GM /hideme		- Remove self from spawn lists and make invis
// #define LiveOP_GMInquire		0x00da	// GM /inquire		- Search soulmark data
// #define	LiveOP_GMSoulmark		0x00dc	// GM /praise /warn	- Add soulmark comment to user file
#define LiveOP_GMZoneRequest	0x0184	// GM /zone			- Transport to another zone
#define LiveOP_GMZoneRequest2	0x0239	// GM /zone 2

#define LiveOP_Petition			0x0068
#define LiveOP_PetitionRefresh	0x0085
#define LiveOP_PDeletePetition	0x01ee
#define LiveOP_PetitionBug		0x0092	// 0094 feedback 0095 guide
// #define LiveOP_PViewPetition	0x01ef
#define LiveOP_PetitionUpdate	0x0069	// Updates the Petitions in the Que
#define LiveOP_PetitionCheckout	0x0076	// Petition Checkout
#define LiveOP_PetitionCheckout2	0x0056 //Also sent when a player checks out a petition Possibly requesting who all
#define LiveOP_PetitionDelete	0x0091	// Client Petition Delete Request
#define LiveOP_PetitionResolve	0x02b4	// Client Petition Resolve Request
#define LiveOP_PetitionCheckIn	0x007e	// Petition Checkin
#define LiveOP_PetitionUnCheckout 0x0090

#define LiveOP_PetitionQue		0x01ec //GM looking at petitions


#define LiveOP_SetServerFilter	0x01bb
	// not used	#define LiveOP_SetServerFilterAck 0x0341

#define LiveOP_NewSpawn			0x0218	// New NPC or PC entering zone
#define LiveOP_Animation		0x0140
// #define LiveOP_MobHealth		0x022e	// health sent when a player clicks on the mob
#define LiveOP_ZoneChange		0x0142	// Client requesting transfer to a different zone
#define LiveOP_DeleteSpawn		0x00f3	// Remove a spawn from the current zone
	// not used	#define LiveOP_ConfirmDelete	0x0178	//Client sends this to server to confirm op_deletespawn
	// not used	#define LiveOP_NewCorpse		0x00da
#define LiveOP_CrashDump		0x0265
	// not used	#define LiveOP_CastOn			0x0119
#define LiveOP_EnvDamage		0x00e8

#define LiveOP_Action			0x0101
#define LiveOP_Damage			0x00e2	// seq calls this Action2
#define LiveOP_ManaChange		0x00bf
#define LiveOP_ClientError		0x027c
// #define LiveOP_LoadSpellSet		0x02a4
#define LiveOP_Save				0x00fb	// Client asking server to save user state
#define LiveOP_LocInfo			0x0316

#define LiveOP_Surname			0x0188
#define LiveOP_SwapSpell		0x018f	// Swapping spell positions within book
#define LiveOP_DeleteSpell			0x01db
#define LiveOP_CloseContainer	0x029f	//Client closing world container (i.e., forge)
#define LiveOP_ClickObjectAck	0x029f	//Client closing world container (i.e., forge)
#define LiveOP_CreateObject		0x00fa	//Zone objects (pok books, objects on ground, etc)
#define	LiveOP_ClickObject		0x00f9	//Client clicking on object
#define LiveOP_ClearObject		0x01c1
#define LiveOP_ZoneUnavail		0x0265
// #define LiveOP_FlashMessage		0x02cd //wierd opcode that flashes message on screen
#define LiveOP_ItemPacket		0x02e0	// Variety of ways for sending out item data
		//0x0283 hmm
#define LiveOP_TradeRequest		0x029a	// Client request trade session
#define LiveOP_TradeRequestAck	0x0037	// Trade request recipient is acknowledging they are able to trade
#define LiveOP_TradeAcceptClick	0x002d
	// not used	#define LiveOP_ItemToTrade		0x0031
#define LiveOP_TradeMoneyUpdate	0x0162
#define LiveOP_TradeCoins		0x0036
#define LiveOP_CancelTrade		0x002e
#define LiveOP_FinishTrade		0x002f
// #define LiveOP_Translocate		0x01c5
	// not used	#define LiveOP_WebUpdate		0x01f2
#define LiveOP_SaveOnZoneReq    0x00a1
#define LiveOP_Logout           0x0185  // Last opcode seny by server when you zone or camp
#define	LiveOP_RequestDuel		0x0298 //Shawn319: Fixed 1/3/04
	// LiveOP_DeclineDuel	0x029c
#define LiveOP_DuelResponse		0x0a5d
#define LiveOP_DuelResponse2	0x016e
#define LiveOP_InstillDoubt		0x007c

#define LiveOP_SafeFallSuccess	0x00ac
#define LiveOP_DisciplineUpdate	0x02fb	

//Tribute Master

//send initial tribute update right before inventory
//send the LiveOP_TributeInfo's in response to LiveOP_ReqClientSpawn
#define LiveOP_TributeUpdate	0x02f2
#define LiveOP_TributeItem		0x02f3
#define LiveOP_TributePointUpdate   0x02f4
#define LiveOP_SendTributes		0x02f5
#define LiveOP_TributeInfo		0x02f6
#define LiveOP_SelectTribute	0x02f7
#define LiveOP_TributeTimer		0x02f8
#define LiveOP_StartTribute		0x02f9
#define LiveOP_TributeNPC		0x02fa		//no idea what this is for
#define LiveOP_TributeMoney		0x02fe
#define LiveOP_TributeToggle	0x0364

//New Tradeskill Interface
#define LiveOP_RecipesFavorite	0x0322
#define LiveOP_RecipesSearch	0x01f9
#define LiveOP_RecipeReply	0x01fa
#define LiveOP_RecipeDetails	0x01fb
#define LiveOP_RecipeAutoCombine 0x01fc

//for the 'find' command
#define LiveOP_FindPersonRequest	0x02db
#define LiveOP_FindPersonReply	0x02dc

#define LiveOP_Shielding		0x01dd		//this is prolly wrong

	//////////////////////////////////////
	// Zone.exe opcodes for login sequence:
	//////////////////////////////////////
#define LiveOP_SetDataRate		0x0198	// Client sending datarate.txt value
#define LiveOP_ZoneEntry		0x023b	// Info about char entering zone..
#define LiveOP_PlayerProfile	0x006b	// Basic player info (no inventory)
#define LiveOP_CharInventory	0x0291	// Full inventory of player
#define LiveOP_ZoneSpawns		0x0170	// All spawns in current zone
#define	LiveOP_TimeOfDay		0x0026	// Notify client of current time
#define LiveOP_Weather			0x015b	// Weather update
#define LiveOP_ReqNewZone		0x00ec	// Client requesting NewZone_Struct
#define LiveOP_NewZone			0x00eb	// Info about zone being loaded (exp multiplier, etc)
#define LiveOP_ReqClientSpawn	0x00fd	// Client requesting spawn data
#define LiveOP_SpawnAppearance	0x012F	// Sets spawnid/animation/equipment
#define LiveOP_ClientReady		0x0086	// Client fully connected, finished loading
#define LiveOP_ZoneComplete			LiveOP_ClientReady //Client sends upon successful zone in

//////////////////////////////////////
// World.exe opcodes:
//////////////////////////////////////
#define LiveOP_LoginComplete		0x02db
#define LiveOP_ApproveWorld			0x0195
#define LiveOP_LogServer			0x035f
#define LiveOP_MOTD					0x01b2	// Server message of the day
#define LiveOP_SendLoginInfo		0x0251
#define LiveOP_DeleteCharacter		0x00ea	// Delete character @ char select
#define LiveOP_SendCharInfo			0x0102	// Send all chars visible @ char select
#define LiveOP_ExpansionInfo		0x00e1	// Which expansions user has
#define LiveOP_CharacterCreate		0x0104	// Create character @ char select
#define LiveOP_RandomNameGenerator	0x02ab	// Returns a random name
#define LiveOP_GuildsList			0x005d	// Server sending client list of guilds
#define LiveOP_ApproveName			0x0125	// Approving new character name @ char creation
#define LiveOP_EnterWorld			0x0261	// Server approval for client to enter world
#define LiveOP_World_Client_CRC1	0x015a	// Contains a snippet of spell data
#define LiveOP_World_Client_CRC2	0x015e	// Second client verification packet
#define LiveOP_SetChatServer		0x0269	// Chatserver? IP,Port,servername.Charname,password(?)
#define LiveOP_ZoneServerInfo		0x0264	// Zone server? IP,0's,uint16?
// not used	#define LiveOP_UserCompInfo		0x02a5	// User submitting computer information

//////////////////////////////////////
// Zone.exe/World.exe shared opcodes:
//////////////////////////////////////
#define LiveOP_AckPacket			0x0017	// Appears to be generic ack at the presentation level
#define LiveOP_WearChange			0x012c	// Client texture/color update

////////////////////////////
// OLD opcodes:
////////////////////////////

// not used	#define LiveOP_SummonedItem		0x0841
// not used	#define LiveOP_HarmTouch		0x0E41
// not used	#define LiveOP_Drink			0x0641
// not used	#define LiveOP_Medding			0x0841
// not used	#define LiveOP_SenseHeading		0x00c3	// Clicked sense heading button - solar: not used anymore

	#define PET_BACKOFF			1
	#define PET_GETLOST			2
	#define PET_HEALTHREPORT	4
	#define PET_GUARDHERE		5
	#define PET_GUARDME			6
	#define PET_ATTACK			7
	#define PET_FOLLOWME		8
	#define PET_SITDOWN			9
	#define PET_STANDUP			10
	#define PET_TAUNT			11
	#define PET_HOLD			12
	#define PET_NOTAUNT			14
	#define PET_LEADER			16
	#define	PET_SLUMBER			17


// Agz: The following is from the old source I used as base
/ENUMERATED PACKET OPCODE
	#define ALL_FINISH                  0x0005
	#define LS_REQUEST_VERSION          0x0059
	#define LS_SEND_VERSION             0x0059
	#define LS_SEND_LOGIN_INFO          0x0001
	#define LS_SEND_SESSION_ID          0x0004
	#define LS_REQUEST_UPDATE           0x0052
	#define LS_SEND_UPDATE              0x0052

	#define LS_REQUEST_SERVERLIST       0x0046
	#define LS_SEND_SERVERLIST          0x0046

	#define LS_REQUEST_SERVERSTATUS     0x0048
	#define LS_SEND_SERVERSTATUS        0x004A
	#define LS_GET_WORLDID              0x0047
	#define LS_SEND_WORLDID             0x0047
	#define WS_SEND_LOGIN_INFO          0x5818
	#define WS_SEND_LOGIN_APPROVED      0x0710
	#define WS_SEND_LOGIN_APPROVED2     0x0180
	#define WS_SEND_CHAR_INFO           0x4720*/
#endif
