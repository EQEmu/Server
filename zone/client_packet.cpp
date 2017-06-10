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

#include "../common/global_define.h"
#include "../common/eqemu_logsys.h"
#include "../common/opcodemgr.h"
#include <iomanip>
#include <iostream>
#include <math.h>
#include <set>
#include <stdio.h>
#include <string.h>
#include <zlib.h>

#ifdef _WINDOWS
#define snprintf	_snprintf
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#else
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#include "../common/crc32.h"
#include "../common/data_verification.h"
#include "../common/faction.h"
#include "../common/guilds.h"
#include "../common/rdtsc.h"
#include "../common/rulesys.h"
#include "../common/skills.h"
#include "../common/spdat.h"
#include "../common/string_util.h"
#include "../common/zone_numbers.h"
#include "event_codes.h"
#include "guild_mgr.h"
#include "merc.h"
#include "petitions.h"
#include "pets.h"
#include "queryserv.h"
#include "quest_parser_collection.h"
#include "string_ids.h"
#include "titles.h"
#include "water_map.h"
#include "worldserver.h"
#include "zone.h"

#ifdef BOTS
#include "bot.h"
#endif

extern QueryServ* QServ;
extern Zone* zone;
extern volatile bool is_zone_loaded;
extern WorldServer worldserver;
extern PetitionList petition_list;
extern EntityList entity_list;
typedef void (Client::*ClientPacketProc)(const EQApplicationPacket *app);

//Use a map for connecting opcodes since it dosent get used a lot and is sparse
std::map<uint32, ClientPacketProc> ConnectingOpcodes;
//Use a static array for connected, for speed
ClientPacketProc ConnectedOpcodes[_maxEmuOpcode];

void MapOpcodes()
{
	ConnectingOpcodes.clear();
	memset(ConnectedOpcodes, 0, sizeof(ConnectedOpcodes));

	// Now put all the opcodes into their home...
	// connecting opcode handler assignments:
	ConnectingOpcodes[OP_ApproveZone] = &Client::Handle_Connect_OP_ApproveZone;
	ConnectingOpcodes[OP_BlockedBuffs] = &Client::Handle_OP_BlockedBuffs;
	ConnectingOpcodes[OP_ClientError] = &Client::Handle_Connect_OP_ClientError;
	ConnectingOpcodes[OP_ClientReady] = &Client::Handle_Connect_OP_ClientReady;
	ConnectingOpcodes[OP_ClientUpdate] = &Client::Handle_Connect_OP_ClientUpdate;
	ConnectingOpcodes[OP_GetGuildsList] = &Client::Handle_OP_GetGuildsList; // temporary hack
	ConnectingOpcodes[OP_ReqClientSpawn] = &Client::Handle_Connect_OP_ReqClientSpawn;
	ConnectingOpcodes[OP_ReqNewZone] = &Client::Handle_Connect_OP_ReqNewZone;
	ConnectingOpcodes[OP_SendAAStats] = &Client::Handle_Connect_OP_SendAAStats;
	ConnectingOpcodes[OP_SendAATable] = &Client::Handle_Connect_OP_SendAATable;
	ConnectingOpcodes[OP_SendExpZonein] = &Client::Handle_Connect_OP_SendExpZonein;
	ConnectingOpcodes[OP_SendGuildTributes] = &Client::Handle_Connect_OP_SendGuildTributes;
	ConnectingOpcodes[OP_SendGuildTributes] = &Client::Handle_Connect_OP_SendGuildTributes; // I guess it didn't believe us with the first assignment?
	ConnectingOpcodes[OP_SendTributes] = &Client::Handle_Connect_OP_SendTributes;
	ConnectingOpcodes[OP_SetServerFilter] = &Client::Handle_Connect_OP_SetServerFilter;
	ConnectingOpcodes[OP_SpawnAppearance] = &Client::Handle_Connect_OP_SpawnAppearance;
	ConnectingOpcodes[OP_TGB] = &Client::Handle_Connect_OP_TGB;
	ConnectingOpcodes[OP_UpdateAA] = &Client::Handle_Connect_OP_UpdateAA;
	ConnectingOpcodes[OP_WearChange] = &Client::Handle_Connect_OP_WearChange;
	ConnectingOpcodes[OP_WorldObjectsSent] = &Client::Handle_Connect_OP_WorldObjectsSent;
	ConnectingOpcodes[OP_XTargetAutoAddHaters] = &Client::Handle_OP_XTargetAutoAddHaters;
	ConnectingOpcodes[OP_XTargetRequest] = &Client::Handle_OP_XTargetRequest;
	ConnectingOpcodes[OP_ZoneComplete] = &Client::Handle_Connect_OP_ZoneComplete;
	ConnectingOpcodes[OP_ZoneEntry] = &Client::Handle_Connect_OP_ZoneEntry;

	// connected opcode handler assignments:
	ConnectedOpcodes[OP_0x0193] = &Client::Handle_0x0193;
	ConnectedOpcodes[OP_AAAction] = &Client::Handle_OP_AAAction;
	ConnectedOpcodes[OP_AcceptNewTask] = &Client::Handle_OP_AcceptNewTask;
	ConnectedOpcodes[OP_AdventureInfoRequest] = &Client::Handle_OP_AdventureInfoRequest;
	ConnectedOpcodes[OP_AdventureLeaderboardRequest] = &Client::Handle_OP_AdventureLeaderboardRequest;
	ConnectedOpcodes[OP_AdventureMerchantPurchase] = &Client::Handle_OP_AdventureMerchantPurchase;
	ConnectedOpcodes[OP_AdventureMerchantRequest] = &Client::Handle_OP_AdventureMerchantRequest;
	ConnectedOpcodes[OP_AdventureMerchantSell] = &Client::Handle_OP_AdventureMerchantSell;
	ConnectedOpcodes[OP_AdventureRequest] = &Client::Handle_OP_AdventureRequest;
	ConnectedOpcodes[OP_AdventureStatsRequest] = &Client::Handle_OP_AdventureStatsRequest;
	ConnectedOpcodes[OP_AggroMeterLockTarget] = &Client::Handle_OP_AggroMeterLockTarget;
	ConnectedOpcodes[OP_AltCurrencyMerchantRequest] = &Client::Handle_OP_AltCurrencyMerchantRequest;
	ConnectedOpcodes[OP_AltCurrencyPurchase] = &Client::Handle_OP_AltCurrencyPurchase;
	ConnectedOpcodes[OP_AltCurrencyReclaim] = &Client::Handle_OP_AltCurrencyReclaim;
	ConnectedOpcodes[OP_AltCurrencySell] = &Client::Handle_OP_AltCurrencySell;
	ConnectedOpcodes[OP_AltCurrencySellSelection] = &Client::Handle_OP_AltCurrencySellSelection;
	ConnectedOpcodes[OP_Animation] = &Client::Handle_OP_Animation;
	ConnectedOpcodes[OP_ApplyPoison] = &Client::Handle_OP_ApplyPoison;
	ConnectedOpcodes[OP_Assist] = &Client::Handle_OP_Assist;
	ConnectedOpcodes[OP_AssistGroup] = &Client::Handle_OP_AssistGroup;
	ConnectedOpcodes[OP_AugmentInfo] = &Client::Handle_OP_AugmentInfo;
	ConnectedOpcodes[OP_AugmentItem] = &Client::Handle_OP_AugmentItem;
	ConnectedOpcodes[OP_AutoAttack] = &Client::Handle_OP_AutoAttack;
	ConnectedOpcodes[OP_AutoAttack2] = &Client::Handle_OP_AutoAttack2;
	ConnectedOpcodes[OP_AutoFire] = &Client::Handle_OP_AutoFire;
	ConnectedOpcodes[OP_Bandolier] = &Client::Handle_OP_Bandolier;
	ConnectedOpcodes[OP_BankerChange] = &Client::Handle_OP_BankerChange;
	ConnectedOpcodes[OP_Barter] = &Client::Handle_OP_Barter;
	ConnectedOpcodes[OP_BazaarInspect] = &Client::Handle_OP_BazaarInspect;
	ConnectedOpcodes[OP_BazaarSearch] = &Client::Handle_OP_BazaarSearch;
	ConnectedOpcodes[OP_Begging] = &Client::Handle_OP_Begging;
	ConnectedOpcodes[OP_Bind_Wound] = &Client::Handle_OP_Bind_Wound;
	ConnectedOpcodes[OP_BlockedBuffs] = &Client::Handle_OP_BlockedBuffs;
	ConnectedOpcodes[OP_BoardBoat] = &Client::Handle_OP_BoardBoat;
	ConnectedOpcodes[OP_Buff] = &Client::Handle_OP_Buff;
	ConnectedOpcodes[OP_BuffRemoveRequest] = &Client::Handle_OP_BuffRemoveRequest;
	ConnectedOpcodes[OP_Bug] = &Client::Handle_OP_Bug;
	ConnectedOpcodes[OP_Camp] = &Client::Handle_OP_Camp;
	ConnectedOpcodes[OP_CancelTask] = &Client::Handle_OP_CancelTask;
	ConnectedOpcodes[OP_CancelTrade] = &Client::Handle_OP_CancelTrade;
	ConnectedOpcodes[OP_CastSpell] = &Client::Handle_OP_CastSpell;
	ConnectedOpcodes[OP_ChannelMessage] = &Client::Handle_OP_ChannelMessage;
	ConnectedOpcodes[OP_ClearBlockedBuffs] = &Client::Handle_OP_ClearBlockedBuffs;
	ConnectedOpcodes[OP_ClearNPCMarks] = &Client::Handle_OP_ClearNPCMarks;
	ConnectedOpcodes[OP_ClearSurname] = &Client::Handle_OP_ClearSurname;
	ConnectedOpcodes[OP_ClickDoor] = &Client::Handle_OP_ClickDoor;
	ConnectedOpcodes[OP_ClickObject] = &Client::Handle_OP_ClickObject;
	ConnectedOpcodes[OP_ClickObjectAction] = &Client::Handle_OP_ClickObjectAction;
	ConnectedOpcodes[OP_ClientError] = &Client::Handle_OP_ClientError;
	ConnectedOpcodes[OP_ClientTimeStamp] = &Client::Handle_OP_ClientTimeStamp;
	ConnectedOpcodes[OP_ClientUpdate] = &Client::Handle_OP_ClientUpdate;
	ConnectedOpcodes[OP_CombatAbility] = &Client::Handle_OP_CombatAbility;
	ConnectedOpcodes[OP_ConfirmDelete] = &Client::Handle_OP_ConfirmDelete;
	ConnectedOpcodes[OP_Consent] = &Client::Handle_OP_Consent;
	ConnectedOpcodes[OP_ConsentDeny] = &Client::Handle_OP_ConsentDeny;
	ConnectedOpcodes[OP_Consider] = &Client::Handle_OP_Consider;
	ConnectedOpcodes[OP_ConsiderCorpse] = &Client::Handle_OP_ConsiderCorpse;
	ConnectedOpcodes[OP_Consume] = &Client::Handle_OP_Consume;
	ConnectedOpcodes[OP_ControlBoat] = &Client::Handle_OP_ControlBoat;
	ConnectedOpcodes[OP_CorpseDrag] = &Client::Handle_OP_CorpseDrag;
	ConnectedOpcodes[OP_CorpseDrop] = &Client::Handle_OP_CorpseDrop;
	ConnectedOpcodes[OP_CrashDump] = &Client::Handle_OP_CrashDump;
	ConnectedOpcodes[OP_CrystalCreate] = &Client::Handle_OP_CrystalCreate;
	ConnectedOpcodes[OP_CrystalReclaim] = &Client::Handle_OP_CrystalReclaim;
	ConnectedOpcodes[OP_Damage] = &Client::Handle_OP_Damage;
	ConnectedOpcodes[OP_Death] = &Client::Handle_OP_Death;
	ConnectedOpcodes[OP_DelegateAbility] = &Client::Handle_OP_DelegateAbility;
	ConnectedOpcodes[OP_DeleteItem] = &Client::Handle_OP_DeleteItem;
	ConnectedOpcodes[OP_DeleteSpawn] = &Client::Handle_OP_DeleteSpawn;
	ConnectedOpcodes[OP_DeleteSpell] = &Client::Handle_OP_DeleteSpell;
	ConnectedOpcodes[OP_DisarmTraps] = &Client::Handle_OP_DisarmTraps;
	ConnectedOpcodes[OP_DoGroupLeadershipAbility] = &Client::Handle_OP_DoGroupLeadershipAbility;
	ConnectedOpcodes[OP_DuelResponse] = &Client::Handle_OP_DuelResponse;
	ConnectedOpcodes[OP_DuelResponse2] = &Client::Handle_OP_DuelResponse2;
	ConnectedOpcodes[OP_DumpName] = &Client::Handle_OP_DumpName;
	ConnectedOpcodes[OP_Dye] = &Client::Handle_OP_Dye;
	ConnectedOpcodes[OP_Emote] = &Client::Handle_OP_Emote;
	ConnectedOpcodes[OP_EndLootRequest] = &Client::Handle_OP_EndLootRequest;
	ConnectedOpcodes[OP_EnvDamage] = &Client::Handle_OP_EnvDamage;
	ConnectedOpcodes[OP_FaceChange] = &Client::Handle_OP_FaceChange;
	ConnectedOpcodes[OP_FeignDeath] = &Client::Handle_OP_FeignDeath;
	ConnectedOpcodes[OP_FindPersonRequest] = &Client::Handle_OP_FindPersonRequest;
	ConnectedOpcodes[OP_Fishing] = &Client::Handle_OP_Fishing;
	ConnectedOpcodes[OP_FloatListThing] = &Client::Handle_OP_Ignore;
	ConnectedOpcodes[OP_Forage] = &Client::Handle_OP_Forage;
	ConnectedOpcodes[OP_FriendsWho] = &Client::Handle_OP_FriendsWho;
	ConnectedOpcodes[OP_GetGuildMOTD] = &Client::Handle_OP_GetGuildMOTD;
	ConnectedOpcodes[OP_GetGuildsList] = &Client::Handle_OP_GetGuildsList;
	ConnectedOpcodes[OP_GMBecomeNPC] = &Client::Handle_OP_GMBecomeNPC;
	ConnectedOpcodes[OP_GMDelCorpse] = &Client::Handle_OP_GMDelCorpse;
	ConnectedOpcodes[OP_GMEmoteZone] = &Client::Handle_OP_GMEmoteZone;
	ConnectedOpcodes[OP_GMEndTraining] = &Client::Handle_OP_GMEndTraining;
	ConnectedOpcodes[OP_GMFind] = &Client::Handle_OP_GMFind;
	ConnectedOpcodes[OP_GMGoto] = &Client::Handle_OP_GMGoto;
	ConnectedOpcodes[OP_GMHideMe] = &Client::Handle_OP_GMHideMe;
	ConnectedOpcodes[OP_GMKick] = &Client::Handle_OP_GMKick;
	ConnectedOpcodes[OP_GMKill] = &Client::Handle_OP_GMKill;
	ConnectedOpcodes[OP_GMLastName] = &Client::Handle_OP_GMLastName;
	ConnectedOpcodes[OP_GMNameChange] = &Client::Handle_OP_GMNameChange;
	ConnectedOpcodes[OP_GMSearchCorpse] = &Client::Handle_OP_GMSearchCorpse;
	ConnectedOpcodes[OP_GMServers] = &Client::Handle_OP_GMServers;
	ConnectedOpcodes[OP_GMSummon] = &Client::Handle_OP_GMSummon;
	ConnectedOpcodes[OP_GMToggle] = &Client::Handle_OP_GMToggle;
	ConnectedOpcodes[OP_GMTraining] = &Client::Handle_OP_GMTraining;
	ConnectedOpcodes[OP_GMTrainSkill] = &Client::Handle_OP_GMTrainSkill;
	ConnectedOpcodes[OP_GMZoneRequest] = &Client::Handle_OP_GMZoneRequest;
	ConnectedOpcodes[OP_GMZoneRequest2] = &Client::Handle_OP_GMZoneRequest2;
	ConnectedOpcodes[OP_GroundSpawn] = &Client::Handle_OP_CreateObject;
	ConnectedOpcodes[OP_GroupAcknowledge] = &Client::Handle_OP_GroupAcknowledge;
	ConnectedOpcodes[OP_GroupCancelInvite] = &Client::Handle_OP_GroupCancelInvite;
	ConnectedOpcodes[OP_GroupDelete] = &Client::Handle_OP_GroupDelete;
	ConnectedOpcodes[OP_GroupDisband] = &Client::Handle_OP_GroupDisband;
	ConnectedOpcodes[OP_GroupFollow] = &Client::Handle_OP_GroupFollow;
	ConnectedOpcodes[OP_GroupFollow2] = &Client::Handle_OP_GroupFollow2;
	ConnectedOpcodes[OP_GroupInvite] = &Client::Handle_OP_GroupInvite;
	ConnectedOpcodes[OP_GroupInvite2] = &Client::Handle_OP_GroupInvite2;
	ConnectedOpcodes[OP_GroupMakeLeader] = &Client::Handle_OP_GroupMakeLeader;
	ConnectedOpcodes[OP_GroupMentor] = &Client::Handle_OP_GroupMentor;
	ConnectedOpcodes[OP_GroupRoles] = &Client::Handle_OP_GroupRoles;
	ConnectedOpcodes[OP_GroupUpdate] = &Client::Handle_OP_GroupUpdate;
	ConnectedOpcodes[OP_GuildBank] = &Client::Handle_OP_GuildBank;
	ConnectedOpcodes[OP_GuildCreate] = &Client::Handle_OP_GuildCreate;
	ConnectedOpcodes[OP_GuildDelete] = &Client::Handle_OP_GuildDelete;
	ConnectedOpcodes[OP_GuildDemote] = &Client::Handle_OP_GuildDemote;
	ConnectedOpcodes[OP_GuildInvite] = &Client::Handle_OP_GuildInvite;
	ConnectedOpcodes[OP_GuildInviteAccept] = &Client::Handle_OP_GuildInviteAccept;
	ConnectedOpcodes[OP_GuildLeader] = &Client::Handle_OP_GuildLeader;
	ConnectedOpcodes[OP_GuildManageBanker] = &Client::Handle_OP_GuildManageBanker;
	ConnectedOpcodes[OP_GuildPeace] = &Client::Handle_OP_GuildPeace;
	ConnectedOpcodes[OP_GuildPromote] = &Client::Handle_OP_GuildPromote;
	ConnectedOpcodes[OP_GuildPublicNote] = &Client::Handle_OP_GuildPublicNote;
	ConnectedOpcodes[OP_GuildRemove] = &Client::Handle_OP_GuildRemove;
	ConnectedOpcodes[OP_GuildStatus] = &Client::Handle_OP_GuildStatus;
	ConnectedOpcodes[OP_GuildUpdateURLAndChannel] = &Client::Handle_OP_GuildUpdateURLAndChannel;
	ConnectedOpcodes[OP_GuildWar] = &Client::Handle_OP_GuildWar;
	ConnectedOpcodes[OP_Heartbeat] = &Client::Handle_OP_Heartbeat;
	ConnectedOpcodes[OP_Hide] = &Client::Handle_OP_Hide;
	ConnectedOpcodes[OP_HideCorpse] = &Client::Handle_OP_HideCorpse;
	ConnectedOpcodes[OP_Illusion] = &Client::Handle_OP_Illusion;
	ConnectedOpcodes[OP_InspectAnswer] = &Client::Handle_OP_InspectAnswer;
	ConnectedOpcodes[OP_InspectMessageUpdate] = &Client::Handle_OP_InspectMessageUpdate;
	ConnectedOpcodes[OP_InspectRequest] = &Client::Handle_OP_InspectRequest;
	ConnectedOpcodes[OP_InstillDoubt] = &Client::Handle_OP_InstillDoubt;
	ConnectedOpcodes[OP_ItemLinkClick] = &Client::Handle_OP_ItemLinkClick;
	ConnectedOpcodes[OP_ItemLinkResponse] = &Client::Handle_OP_ItemLinkResponse;
	ConnectedOpcodes[OP_ItemName] = &Client::Handle_OP_ItemName;
	ConnectedOpcodes[OP_ItemPreview] = &Client::Handle_OP_ItemPreview;
	ConnectedOpcodes[OP_ItemVerifyRequest] = &Client::Handle_OP_ItemVerifyRequest;
	ConnectedOpcodes[OP_ItemViewUnknown] = &Client::Handle_OP_Ignore;
	ConnectedOpcodes[OP_Jump] = &Client::Handle_OP_Jump;
	ConnectedOpcodes[OP_KeyRing] = &Client::Handle_OP_KeyRing;
	ConnectedOpcodes[OP_LDoNButton] = &Client::Handle_OP_LDoNButton;
	ConnectedOpcodes[OP_LDoNDisarmTraps] = &Client::Handle_OP_LDoNDisarmTraps;
	ConnectedOpcodes[OP_LDoNInspect] = &Client::Handle_OP_LDoNInspect;
	ConnectedOpcodes[OP_LDoNOpen] = &Client::Handle_OP_LDoNOpen;
	ConnectedOpcodes[OP_LDoNPickLock] = &Client::Handle_OP_LDoNPickLock;
	ConnectedOpcodes[OP_LDoNSenseTraps] = &Client::Handle_OP_LDoNSenseTraps;
	ConnectedOpcodes[OP_LeadershipExpToggle] = &Client::Handle_OP_LeadershipExpToggle;
	ConnectedOpcodes[OP_LeaveAdventure] = &Client::Handle_OP_LeaveAdventure;
	ConnectedOpcodes[OP_LeaveBoat] = &Client::Handle_OP_LeaveBoat;
	ConnectedOpcodes[OP_LFGCommand] = &Client::Handle_OP_LFGCommand;
	ConnectedOpcodes[OP_LFGGetMatchesRequest] = &Client::Handle_OP_LFGGetMatchesRequest;
	ConnectedOpcodes[OP_LFGuild] = &Client::Handle_OP_LFGuild;
	ConnectedOpcodes[OP_LFPCommand] = &Client::Handle_OP_LFPCommand;
	ConnectedOpcodes[OP_LFPGetMatchesRequest] = &Client::Handle_OP_LFPGetMatchesRequest;
	ConnectedOpcodes[OP_LoadSpellSet] = &Client::Handle_OP_LoadSpellSet;
	ConnectedOpcodes[OP_Logout] = &Client::Handle_OP_Logout;
	ConnectedOpcodes[OP_LootItem] = &Client::Handle_OP_LootItem;
	ConnectedOpcodes[OP_LootRequest] = &Client::Handle_OP_LootRequest;
	ConnectedOpcodes[OP_ManaChange] = &Client::Handle_OP_ManaChange;
	ConnectedOpcodes[OP_MemorizeSpell] = &Client::Handle_OP_MemorizeSpell;
	ConnectedOpcodes[OP_Mend] = &Client::Handle_OP_Mend;
	ConnectedOpcodes[OP_MercenaryCommand] = &Client::Handle_OP_MercenaryCommand;
	ConnectedOpcodes[OP_MercenaryDataRequest] = &Client::Handle_OP_MercenaryDataRequest;
	ConnectedOpcodes[OP_MercenaryDataUpdateRequest] = &Client::Handle_OP_MercenaryDataUpdateRequest;
	ConnectedOpcodes[OP_MercenaryDismiss] = &Client::Handle_OP_MercenaryDismiss;
	ConnectedOpcodes[OP_MercenaryHire] = &Client::Handle_OP_MercenaryHire;
	ConnectedOpcodes[OP_MercenarySuspendRequest] = &Client::Handle_OP_MercenarySuspendRequest;
	ConnectedOpcodes[OP_MercenaryTimerRequest] = &Client::Handle_OP_MercenaryTimerRequest;
	ConnectedOpcodes[OP_MoveCoin] = &Client::Handle_OP_MoveCoin;
	ConnectedOpcodes[OP_MoveItem] = &Client::Handle_OP_MoveItem;
	ConnectedOpcodes[OP_OpenContainer] = &Client::Handle_OP_OpenContainer;
	ConnectedOpcodes[OP_OpenGuildTributeMaster] = &Client::Handle_OP_OpenGuildTributeMaster;
	ConnectedOpcodes[OP_OpenInventory] = &Client::Handle_OP_OpenInventory;
	ConnectedOpcodes[OP_OpenTributeMaster] = &Client::Handle_OP_OpenTributeMaster;
	ConnectedOpcodes[OP_PDeletePetition] = &Client::Handle_OP_PDeletePetition;
	ConnectedOpcodes[OP_PetCommands] = &Client::Handle_OP_PetCommands;
	ConnectedOpcodes[OP_Petition] = &Client::Handle_OP_Petition;
	ConnectedOpcodes[OP_PetitionBug] = &Client::Handle_OP_PetitionBug;
	ConnectedOpcodes[OP_PetitionCheckIn] = &Client::Handle_OP_PetitionCheckIn;
	ConnectedOpcodes[OP_PetitionCheckout] = &Client::Handle_OP_PetitionCheckout;
	ConnectedOpcodes[OP_PetitionDelete] = &Client::Handle_OP_PetitionDelete;
	ConnectedOpcodes[OP_PetitionQue] = &Client::Handle_OP_PetitionQue;
	ConnectedOpcodes[OP_PetitionRefresh] = &Client::Handle_OP_PetitionRefresh;
	ConnectedOpcodes[OP_PetitionResolve] = &Client::Handle_OP_PetitionResolve;
	ConnectedOpcodes[OP_PetitionUnCheckout] = &Client::Handle_OP_PetitionUnCheckout;
	ConnectedOpcodes[OP_PlayerStateAdd] = &Client::Handle_OP_PlayerStateAdd;
	ConnectedOpcodes[OP_PlayerStateRemove] = &Client::Handle_OP_PlayerStateRemove;
	ConnectedOpcodes[OP_PickPocket] = &Client::Handle_OP_PickPocket;
	ConnectedOpcodes[OP_PopupResponse] = &Client::Handle_OP_PopupResponse;
	ConnectedOpcodes[OP_PotionBelt] = &Client::Handle_OP_PotionBelt;
	ConnectedOpcodes[OP_PurchaseLeadershipAA] = &Client::Handle_OP_PurchaseLeadershipAA;
	ConnectedOpcodes[OP_PVPLeaderBoardDetailsRequest] = &Client::Handle_OP_PVPLeaderBoardDetailsRequest;
	ConnectedOpcodes[OP_PVPLeaderBoardRequest] = &Client::Handle_OP_PVPLeaderBoardRequest;
	ConnectedOpcodes[OP_RaidInvite] = &Client::Handle_OP_RaidCommand;
	ConnectedOpcodes[OP_RandomReq] = &Client::Handle_OP_RandomReq;
	ConnectedOpcodes[OP_ReadBook] = &Client::Handle_OP_ReadBook;
	ConnectedOpcodes[OP_RecipeAutoCombine] = &Client::Handle_OP_RecipeAutoCombine;
	ConnectedOpcodes[OP_RecipeDetails] = &Client::Handle_OP_RecipeDetails;
	ConnectedOpcodes[OP_RecipesFavorite] = &Client::Handle_OP_RecipesFavorite;
	ConnectedOpcodes[OP_RecipesSearch] = &Client::Handle_OP_RecipesSearch;
	ConnectedOpcodes[OP_ReloadUI] = &Client::Handle_OP_ReloadUI;
	ConnectedOpcodes[OP_RemoveBlockedBuffs] = &Client::Handle_OP_RemoveBlockedBuffs;
	ConnectedOpcodes[OP_Report] = &Client::Handle_OP_Report;
	ConnectedOpcodes[OP_RequestDuel] = &Client::Handle_OP_RequestDuel;
	ConnectedOpcodes[OP_RequestTitles] = &Client::Handle_OP_RequestTitles;
	ConnectedOpcodes[OP_RespawnWindow] = &Client::Handle_OP_RespawnWindow;
	ConnectedOpcodes[OP_Rewind] = &Client::Handle_OP_Rewind;
	ConnectedOpcodes[OP_RezzAnswer] = &Client::Handle_OP_RezzAnswer;
	ConnectedOpcodes[OP_Sacrifice] = &Client::Handle_OP_Sacrifice;
	ConnectedOpcodes[OP_SafeFallSuccess] = &Client::Handle_OP_SafeFallSuccess;
	ConnectedOpcodes[OP_SafePoint] = &Client::Handle_OP_SafePoint;
	ConnectedOpcodes[OP_Save] = &Client::Handle_OP_Save;
	ConnectedOpcodes[OP_SaveOnZoneReq] = &Client::Handle_OP_SaveOnZoneReq;
	ConnectedOpcodes[OP_SelectTribute] = &Client::Handle_OP_SelectTribute;

	// Use or Ignore sense heading based on rule.
	bool train = RuleB(Skills, TrainSenseHeading);

	ConnectedOpcodes[OP_SenseHeading] =
		(train) ? &Client::Handle_OP_SenseHeading : &Client::Handle_OP_Ignore;

	ConnectedOpcodes[OP_SenseTraps] = &Client::Handle_OP_SenseTraps;
	ConnectedOpcodes[OP_SetGuildMOTD] = &Client::Handle_OP_SetGuildMOTD;
	ConnectedOpcodes[OP_SetRunMode] = &Client::Handle_OP_SetRunMode;
	ConnectedOpcodes[OP_SetServerFilter] = &Client::Handle_OP_SetServerFilter;
	ConnectedOpcodes[OP_SetStartCity] = &Client::Handle_OP_SetStartCity;
	ConnectedOpcodes[OP_SetTitle] = &Client::Handle_OP_SetTitle;
	ConnectedOpcodes[OP_Shielding] = &Client::Handle_OP_Shielding;
	ConnectedOpcodes[OP_ShopEnd] = &Client::Handle_OP_ShopEnd;
	ConnectedOpcodes[OP_ShopPlayerBuy] = &Client::Handle_OP_ShopPlayerBuy;
	ConnectedOpcodes[OP_ShopPlayerSell] = &Client::Handle_OP_ShopPlayerSell;
	ConnectedOpcodes[OP_ShopRequest] = &Client::Handle_OP_ShopRequest;
	ConnectedOpcodes[OP_Sneak] = &Client::Handle_OP_Sneak;
	ConnectedOpcodes[OP_SpawnAppearance] = &Client::Handle_OP_SpawnAppearance;
	ConnectedOpcodes[OP_Split] = &Client::Handle_OP_Split;
	ConnectedOpcodes[OP_Surname] = &Client::Handle_OP_Surname;
	ConnectedOpcodes[OP_SwapSpell] = &Client::Handle_OP_SwapSpell;
	ConnectedOpcodes[OP_TargetCommand] = &Client::Handle_OP_TargetCommand;
	ConnectedOpcodes[OP_TargetMouse] = &Client::Handle_OP_TargetMouse;
	ConnectedOpcodes[OP_TaskHistoryRequest] = &Client::Handle_OP_TaskHistoryRequest;
	ConnectedOpcodes[OP_Taunt] = &Client::Handle_OP_Taunt;
	ConnectedOpcodes[OP_TestBuff] = &Client::Handle_OP_TestBuff;
	ConnectedOpcodes[OP_TGB] = &Client::Handle_OP_TGB;
	ConnectedOpcodes[OP_Track] = &Client::Handle_OP_Track;
	ConnectedOpcodes[OP_TrackTarget] = &Client::Handle_OP_TrackTarget;
	ConnectedOpcodes[OP_TrackUnknown] = &Client::Handle_OP_TrackUnknown;
	ConnectedOpcodes[OP_TradeAcceptClick] = &Client::Handle_OP_TradeAcceptClick;
	ConnectedOpcodes[OP_TradeBusy] = &Client::Handle_OP_TradeBusy;
	ConnectedOpcodes[OP_Trader] = &Client::Handle_OP_Trader;
	ConnectedOpcodes[OP_TraderBuy] = &Client::Handle_OP_TraderBuy;
	ConnectedOpcodes[OP_TradeRequest] = &Client::Handle_OP_TradeRequest;
	ConnectedOpcodes[OP_TradeRequestAck] = &Client::Handle_OP_TradeRequestAck;
	ConnectedOpcodes[OP_TraderShop] = &Client::Handle_OP_TraderShop;
	ConnectedOpcodes[OP_TradeSkillCombine] = &Client::Handle_OP_TradeSkillCombine;
	ConnectedOpcodes[OP_Translocate] = &Client::Handle_OP_Translocate;
	ConnectedOpcodes[OP_TributeItem] = &Client::Handle_OP_TributeItem;
	ConnectedOpcodes[OP_TributeMoney] = &Client::Handle_OP_TributeMoney;
	ConnectedOpcodes[OP_TributeNPC] = &Client::Handle_OP_TributeNPC;
	ConnectedOpcodes[OP_TributeToggle] = &Client::Handle_OP_TributeToggle;
	ConnectedOpcodes[OP_TributeUpdate] = &Client::Handle_OP_TributeUpdate;
	ConnectedOpcodes[OP_VetClaimRequest] = &Client::Handle_OP_VetClaimRequest;
	ConnectedOpcodes[OP_VoiceMacroIn] = &Client::Handle_OP_VoiceMacroIn;
	ConnectedOpcodes[OP_WearChange] = &Client::Handle_OP_WearChange;
	ConnectedOpcodes[OP_WhoAllRequest] = &Client::Handle_OP_WhoAllRequest;
	ConnectedOpcodes[OP_WorldUnknown001] = &Client::Handle_OP_Ignore;
	ConnectedOpcodes[OP_XTargetAutoAddHaters] = &Client::Handle_OP_XTargetAutoAddHaters;
	ConnectedOpcodes[OP_XTargetOpen] = &Client::Handle_OP_XTargetOpen;
	ConnectedOpcodes[OP_XTargetRequest] = &Client::Handle_OP_XTargetRequest;
	ConnectedOpcodes[OP_YellForHelp] = &Client::Handle_OP_YellForHelp;
	ConnectedOpcodes[OP_ZoneChange] = &Client::Handle_OP_ZoneChange;
	ConnectedOpcodes[OP_ResetAA] = &Client::Handle_OP_ResetAA;
}

void ClearMappedOpcode(EmuOpcode op)
{
	if (op >= _maxEmuOpcode)
		return;

	ConnectedOpcodes[op] = nullptr;
	auto iter = ConnectingOpcodes.find(op);
	if (iter != ConnectingOpcodes.end()) {
		ConnectingOpcodes.erase(iter);
	}
}

// client methods
int Client::HandlePacket(const EQApplicationPacket *app)
{
	if (LogSys.log_settings[Logs::LogCategory::Netcode].is_category_enabled == 1) {
		char buffer[64];
		app->build_header_dump(buffer);
		Log(Logs::Detail, Logs::Client_Server_Packet, "Dispatch opcode: %s", buffer);
	}

	if (LogSys.log_settings[Logs::Client_Server_Packet].is_category_enabled == 1)
		Log(Logs::General, Logs::Client_Server_Packet, "[%s - 0x%04x] [Size: %u]", OpcodeManager::EmuToName(app->GetOpcode()), app->GetOpcode(), app->Size());

	if (LogSys.log_settings[Logs::Client_Server_Packet_With_Dump].is_category_enabled == 1)
		Log(Logs::General, Logs::Client_Server_Packet_With_Dump, "[%s - 0x%04x] [Size: %u] %s", OpcodeManager::EmuToName(app->GetOpcode()), app->GetOpcode(), app->Size(), DumpPacketToString(app).c_str());

	EmuOpcode opcode = app->GetOpcode();
	if (opcode == OP_AckPacket) {
		return true;
	}

#if EQDEBUG >= 9
	std::cout << "Received 0x" << std::hex << std::setw(4) << std::setfill('0') << opcode << ", size=" << std::dec << app->size << std::endl;
#endif

	switch (client_state) {
	case CLIENT_CONNECTING: {
		if (ConnectingOpcodes.count(opcode) != 1) {
			//Hate const cast but everything in lua needs to be non-const even if i make it non-mutable
			std::vector<EQEmu::Any> args;
			args.push_back(const_cast<EQApplicationPacket*>(app));
			parse->EventPlayer(EVENT_UNHANDLED_OPCODE, this, "", 1, &args);

#if EQDEBUG >= 10
			Log(Logs::General, Logs::Error, "HandlePacket() Opcode error: Unexpected packet during CLIENT_CONNECTING: opcode:"
				" %s (#%d eq=0x%04x), size: %i", OpcodeNames[opcode], opcode, 0, app->size);
			DumpPacket(app);
#endif
			break;
		}

		ClientPacketProc p;
		p = ConnectingOpcodes[opcode];

		//call the processing routine
		(this->*p)(app);

		//special case where connecting code needs to boot client...
		if (client_state == CLIENT_KICKED) {
			return(false);
		}

		break;
	}
	case CLIENT_CONNECTED: {
		ClientPacketProc p;
		p = ConnectedOpcodes[opcode];
		if (p == nullptr) {
			std::vector<EQEmu::Any> args;
			args.push_back(const_cast<EQApplicationPacket*>(app));
			parse->EventPlayer(EVENT_UNHANDLED_OPCODE, this, "", 0, &args);

			if (LogSys.log_settings[Logs::Client_Server_Packet_Unhandled].is_category_enabled == 1) {
				char buffer[64];
				app->build_header_dump(buffer);
				Log(Logs::General, Logs::Client_Server_Packet_Unhandled, "%s %s", buffer, DumpPacketToString(app).c_str());
			}
			break;
		}

		//call the processing routine
		(this->*p)(app);
		break;
	}
	case CLIENT_KICKED:
	case DISCONNECTED:
	case CLIENT_LINKDEAD:
		break;
	default:
		Log(Logs::General, Logs::None, "Unknown client_state: %d\n", client_state);
		break;
	}

	return(true);
}

// Finish client connecting state
void Client::CompleteConnect()
{

	UpdateWho();
	client_state = CLIENT_CONNECTED;
	SendAllPackets();
	hpupdate_timer.Start();
	position_timer.Start();
	autosave_timer.Start();
	SetDuelTarget(0);
	SetDueling(false);

	EnteringMessages(this);
	LoadZoneFlags();

	/* Sets GM Flag if needed & Sends Petition Queue */
	UpdateAdmin(false);

	if (IsInAGuild()) {
		uint8 rank = GuildRank();
		if (ClientVersion() >= EQEmu::versions::ClientVersion::RoF)
		{
			switch (rank) {
			case 0: { rank = 5; break; }	// GUILD_MEMBER	0
			case 1: { rank = 3; break; }	// GUILD_OFFICER 1
			case 2: { rank = 1; break; }	// GUILD_LEADER	2
			default: { break; }				// GUILD_NONE
			}
		}
		SendAppearancePacket(AT_GuildID, GuildID(), false);
		SendAppearancePacket(AT_GuildRank, rank, false);
	}
	for (uint32 spellInt = 0; spellInt < MAX_PP_REF_SPELLBOOK; spellInt++) {
		if (m_pp.spell_book[spellInt] < 3 || m_pp.spell_book[spellInt] > 50000)
			m_pp.spell_book[spellInt] = 0xFFFFFFFF;
	}
	//SendAATable();

	if (GetHideMe()) Message(13, "[GM] You are currently hidden to all clients");

	uint32 raidid = database.GetRaidID(GetName());
	Raid *raid = nullptr;
	if (raidid > 0) {
		raid = entity_list.GetRaidByID(raidid);
		if (!raid) {
			raid = new Raid(raidid);
			if (raid->GetID() != 0) {
				entity_list.AddRaid(raid, raidid);
				raid->LoadLeadership(); // Recreating raid in new zone, get leadership from DB
			}
			else
				raid = nullptr;
		}
		if (raid) {
			SetRaidGrouped(true);
			raid->LearnMembers();
			raid->VerifyRaid();
			raid->GetRaidDetails();
			/*
			Only leader should get this; send to all for now till
			I figure out correct creation; can probably also send a no longer leader packet for non leaders
			but not important for now.
			*/
			raid->SendRaidCreate(this);
			raid->SendMakeLeaderPacketTo(raid->leadername, this);
			raid->SendRaidAdd(GetName(), this);
			raid->SendBulkRaid(this);
			raid->SendGroupUpdate(this);
			raid->SendRaidMOTD(this);
			if (raid->IsLeader(this)) { // We're a raid leader, lets update just in case!
				raid->UpdateRaidAAs();
				raid->SendAllRaidLeadershipAA();
			}
			uint32 grpID = raid->GetGroup(GetName());
			if (grpID < 12) {
				raid->SendRaidGroupRemove(GetName(), grpID);
				raid->SendRaidGroupAdd(GetName(), grpID);
				raid->CheckGroupMentor(grpID, this);
				if (raid->IsGroupLeader(GetName())) { // group leader same thing!
					raid->UpdateGroupAAs(raid->GetGroup(this));
					raid->GroupUpdate(grpID, false);
				}
			}
			raid->SendGroupLeadershipAA(this, grpID); // this may get sent an extra time ...

			SetXTargetAutoMgr(raid->GetXTargetAutoMgr());
			if (!GetXTargetAutoMgr()->empty())
				SetDirtyAutoHaters();

			if (raid->IsLocked())
				raid->SendRaidLockTo(this);
		}
	}

	//bulk raid send in here eventually

	//reapply some buffs
	uint32 buff_count = GetMaxTotalSlots();
	for (uint32 j1 = 0; j1 < buff_count; j1++) {
		if (!IsValidSpell(buffs[j1].spellid))
			continue;

		const SPDat_Spell_Struct &spell = spells[buffs[j1].spellid];

		int NimbusEffect = GetNimbusEffect(buffs[j1].spellid);
		if (NimbusEffect) {
			if (!IsNimbusEffectActive(NimbusEffect))
				SendSpellEffect(NimbusEffect, 500, 0, 1, 3000, true);
		}

		for (int x1 = 0; x1 < EFFECT_COUNT; x1++) {
			switch (spell.effectid[x1]) {
			case SE_IllusionCopy:
			case SE_Illusion: {
				if (spell.base[x1] == -1) {
					if (gender == 1)
						gender = 0;
					else if (gender == 0)
						gender = 1;
					SendIllusionPacket(GetRace(), gender, 0xFF, 0xFF);
				}
				else if (spell.base[x1] == -2) // WTF IS THIS
				{
					if (GetRace() == 128 || GetRace() == 130 || GetRace() <= 12)
						SendIllusionPacket(GetRace(), GetGender(), spell.base2[x1], spell.max[x1]);
				}
				else if (spell.max[x1] > 0)
				{
					SendIllusionPacket(spell.base[x1], 0xFF, spell.base2[x1], spell.max[x1]);
				}
				else
				{
					SendIllusionPacket(spell.base[x1], 0xFF, 0xFF, 0xFF);
				}
				switch (spell.base[x1]) {
				case OGRE:
					SendAppearancePacket(AT_Size, 9);
					break;
				case TROLL:
					SendAppearancePacket(AT_Size, 8);
					break;
				case VAHSHIR:
				case BARBARIAN:
					SendAppearancePacket(AT_Size, 7);
					break;
				case HALF_ELF:
				case WOOD_ELF:
				case DARK_ELF:
				case FROGLOK:
					SendAppearancePacket(AT_Size, 5);
					break;
				case DWARF:
					SendAppearancePacket(AT_Size, 4);
					break;
				case HALFLING:
				case GNOME:
					SendAppearancePacket(AT_Size, 3);
					break;
				default:
					SendAppearancePacket(AT_Size, 6);
					break;
				}
				break;
			}
			case SE_SummonHorse: {
				SummonHorse(buffs[j1].spellid);
				//hasmount = true;	//this was false, is that the correct thing?
				break;
			}
			case SE_Silence:
			{
				Silence(true);
				break;
			}
			case SE_Amnesia:
			{
				Amnesia(true);
				break;
			}
			case SE_DivineAura:
			{
				invulnerable = true;
				break;
			}
			case SE_Invisibility2:
			case SE_Invisibility:
			{
				invisible = true;
				SendAppearancePacket(AT_Invis, 1);
				break;
			}
			case SE_Levitate:
			{
				if (!zone->CanLevitate())
				{
					if (!GetGM())
					{
						SendAppearancePacket(AT_Levitate, 0);
						BuffFadeByEffect(SE_Levitate);
						Message(13, "You can't levitate in this zone.");
					}
				}
				else {
					SendAppearancePacket(AT_Levitate, 2);
				}
				break;
			}
			case SE_InvisVsUndead2:
			case SE_InvisVsUndead:
			{
				invisible_undead = true;
				break;
			}
			case SE_InvisVsAnimals:
			{
				invisible_animals = true;
				break;
			}
			case SE_AddMeleeProc:
			case SE_WeaponProc:
			{
				AddProcToWeapon(GetProcID(buffs[j1].spellid, x1), false, 100 + spells[buffs[j1].spellid].base2[x1], buffs[j1].spellid, buffs[j1].casterlevel);
				break;
			}
			case SE_DefensiveProc:
			{
				AddDefensiveProc(GetProcID(buffs[j1].spellid, x1), 100 + spells[buffs[j1].spellid].base2[x1], buffs[j1].spellid);
				break;
			}
			case SE_RangedProc:
			{
				AddRangedProc(GetProcID(buffs[j1].spellid, x1), 100 + spells[buffs[j1].spellid].base2[x1], buffs[j1].spellid);
				break;
			}
			}
		}
	}

	/* Sends appearances for all mobs not doing anim_stand aka sitting, looting, playing dead */
	entity_list.SendZoneAppearance(this);
	/* Sends the Nimbus particle effects (up to 3) for any mob using them */
	entity_list.SendNimbusEffects(this);

	entity_list.SendUntargetable(this);

	int x;
	for (x = 0; x < 8; x++) {
		SendWearChange(x);
	}
	// added due to wear change above
	UpdateActiveLight();
	SendAppearancePacket(AT_Light, GetActiveLightType());

	Mob *pet = GetPet();
	if (pet != nullptr) {
		for (x = 0; x < 8; x++) {
			pet->SendWearChange(x);
		}
		// added due to wear change above
		pet->UpdateActiveLight();
		pet->SendAppearancePacket(AT_Light, pet->GetActiveLightType());
	}

	entity_list.SendTraders(this);

	if (GetPet()) {
		GetPet()->SendPetBuffsToClient();
	}

	if (GetGroup())
		database.RefreshGroupFromDB(this);

	if (RuleB(TaskSystem, EnableTaskSystem))
		TaskPeriodic_Timer.Start();
	else
		TaskPeriodic_Timer.Disable();

	conn_state = ClientConnectFinished;

	//enforce some rules..
	if (!CanBeInZone()) {
		Log(Logs::Detail, Logs::None, "[CLIENT] Kicking char from zone, not allowed here");
		GoToSafeCoords(database.GetZoneID("arena"), 0);
		return;
	}

	if (zone)
		zone->weatherSend();

	TotalKarma = database.GetKarma(AccountID());
	SendDisciplineTimers();

	parse->EventPlayer(EVENT_ENTER_ZONE, this, "", 0);

	/* This sub event is for if a player logs in for the first time since entering world. */
	if (firstlogon == 1) {
		parse->EventPlayer(EVENT_CONNECT, this, "", 0);
		/* QS: PlayerLogConnectDisconnect */
		if (RuleB(QueryServ, PlayerLogConnectDisconnect)) {
			std::string event_desc = StringFormat("Connect :: Logged into zoneid:%i instid:%i", this->GetZoneID(), this->GetInstanceID());
			QServ->PlayerLogEvent(Player_Log_Connect_State, this->CharacterID(), event_desc);
		}
	}

	if (zone) {
		if (zone->GetInstanceTimer()) {
			uint32 ttime = zone->GetInstanceTimer()->GetRemainingTime();
			uint32 day = (ttime / 86400000);
			uint32 hour = (ttime / 3600000) % 24;
			uint32 minute = (ttime / 60000) % 60;
			uint32 second = (ttime / 1000) % 60;
			if (day) {
				Message(15, "%s(%u) will expire in %u days, %u hours, %u minutes, and %u seconds.",
					zone->GetLongName(), zone->GetInstanceID(), day, hour, minute, second);
			}
			else if (hour) {
				Message(15, "%s(%u) will expire in %u hours, %u minutes, and %u seconds.",
					zone->GetLongName(), zone->GetInstanceID(), hour, minute, second);
			}
			else if (minute) {
				Message(15, "%s(%u) will expire in %u minutes, and %u seconds.",
					zone->GetLongName(), zone->GetInstanceID(), minute, second);
			}
			else {
				Message(15, "%s(%u) will expire in in %u seconds.",
					zone->GetLongName(), zone->GetInstanceID(), second);
			}
		}
	}

	SendRewards();
	SendAltCurrencies();
	database.LoadAltCurrencyValues(CharacterID(), alternate_currency);
	SendAlternateCurrencyValues();
	alternate_currency_loaded = true;
	ProcessAlternateCurrencyQueue();

	/* This needs to be set, this determines whether or not data was loaded properly before a save */
	client_data_loaded = true;

	CalcItemScale();
	DoItemEnterZone();

	if (zone->GetZoneID() == RuleI(World, GuildBankZoneID) && GuildBanks)
		GuildBanks->SendGuildBank(this);

	if (ClientVersion() >= EQEmu::versions::ClientVersion::SoD)
		entity_list.SendFindableNPCList(this);

	if (IsInAGuild()) {
		SendGuildRanks();
		guild_mgr.SendGuildMemberUpdateToWorld(GetName(), GuildID(), zone->GetZoneID(), time(nullptr));
		guild_mgr.RequestOnlineGuildMembers(this->CharacterID(), this->GuildID());
	}

	/** Request adventure info **/
	auto pack = new ServerPacket(ServerOP_AdventureDataRequest, 64);
	strcpy((char*)pack->pBuffer, GetName());
	worldserver.SendPacket(pack);
	delete pack;

	if (IsClient() && CastToClient()->ClientVersionBit() & EQEmu::versions::bit_UFAndLater) {
		EQApplicationPacket *outapp = MakeBuffsPacket(false);
		CastToClient()->FastQueuePacket(&outapp);
	}

	// TODO: load these states
	// We at least will set them to the correct state for now
	if (m_ClientVersionBit & EQEmu::versions::bit_UFAndLater && GetPet()) {
		SetPetCommandState(PET_BUTTON_SIT, 0);
		SetPetCommandState(PET_BUTTON_STOP, 0);
		SetPetCommandState(PET_BUTTON_REGROUP, 0);
		SetPetCommandState(PET_BUTTON_FOLLOW, 1);
		SetPetCommandState(PET_BUTTON_GUARD, 0);
		SetPetCommandState(PET_BUTTON_TAUNT, 1);
		SetPetCommandState(PET_BUTTON_HOLD, 0);
		SetPetCommandState(PET_BUTTON_GHOLD, 0);
		SetPetCommandState(PET_BUTTON_FOCUS, 0);
		SetPetCommandState(PET_BUTTON_SPELLHOLD, 0);
	}

	entity_list.RefreshClientXTargets(this);

	worldserver.RequestTellQueue(GetName());
}

void Client::CheatDetected(CheatTypes CheatType, float x, float y, float z)
{
	//ToDo: Break warp down for special zones. Some zones have special teleportation pads or bad .map files which can trigger the detector without a legit zone request.

	switch (CheatType)
	{
	case MQWarp: //Some zones may still have issues. Database updates will eliminate most if not all problems.
		if (RuleB(Zone, EnableMQWarpDetector)
			&& ((this->Admin() < RuleI(Zone, MQWarpExemptStatus)
				|| (RuleI(Zone, MQWarpExemptStatus)) == -1)))
		{
			Message(13, "Large warp detected.");
			char hString[250];
			sprintf(hString, "/MQWarp with location %.2f, %.2f, %.2f", GetX(), GetY(), GetZ());
			database.SetMQDetectionFlag(this->account_name, this->name, hString, zone->GetShortName());
		}
		break;
	case MQWarpShadowStep:
		if (RuleB(Zone, EnableMQWarpDetector)
			&& ((this->Admin() < RuleI(Zone, MQWarpExemptStatus)
				|| (RuleI(Zone, MQWarpExemptStatus)) == -1)))
		{
			char *hString = nullptr;
			MakeAnyLenString(&hString, "/MQWarp(SS) with location %.2f, %.2f, %.2f, the target was shadow step exempt but we still found this suspicious.", GetX(), GetY(), GetZ());
			database.SetMQDetectionFlag(this->account_name, this->name, hString, zone->GetShortName());
			safe_delete_array(hString);
		}
		break;
	case MQWarpKnockBack:
		if (RuleB(Zone, EnableMQWarpDetector)
			&& ((this->Admin() < RuleI(Zone, MQWarpExemptStatus)
				|| (RuleI(Zone, MQWarpExemptStatus)) == -1)))
		{
			char *hString = nullptr;
			MakeAnyLenString(&hString, "/MQWarp(KB) with location %.2f, %.2f, %.2f, the target was Knock Back exempt but we still found this suspicious.", GetX(), GetY(), GetZ());
			database.SetMQDetectionFlag(this->account_name, this->name, hString, zone->GetShortName());
			safe_delete_array(hString);
		}
		break;

	case MQWarpLight:
		if (RuleB(Zone, EnableMQWarpDetector)
			&& ((this->Admin() < RuleI(Zone, MQWarpExemptStatus)
				|| (RuleI(Zone, MQWarpExemptStatus)) == -1)))
		{
			if (RuleB(Zone, MarkMQWarpLT))
			{
				char *hString = nullptr;
				MakeAnyLenString(&hString, "/MQWarp(LT) with location %.2f, %.2f, %.2f, running fast but not fast enough to get killed, possibly: small warp, speed hack, excessive lag, marked as suspicious.", GetX(), GetY(), GetZ());
				database.SetMQDetectionFlag(this->account_name, this->name, hString, zone->GetShortName());
				safe_delete_array(hString);
			}
		}
		break;

	case MQZone:
		if (RuleB(Zone, EnableMQZoneDetector) && ((this->Admin() < RuleI(Zone, MQZoneExemptStatus) || (RuleI(Zone, MQZoneExemptStatus)) == -1)))
		{
			char hString[250];
			sprintf(hString, "/MQZone used at %.2f, %.2f, %.2f to %.2f %.2f %.2f", GetX(), GetY(), GetZ(), x, y, z);
			database.SetMQDetectionFlag(this->account_name, this->name, hString, zone->GetShortName());
		}
		break;
	case MQZoneUnknownDest:
		if (RuleB(Zone, EnableMQZoneDetector) && ((this->Admin() < RuleI(Zone, MQZoneExemptStatus) || (RuleI(Zone, MQZoneExemptStatus)) == -1)))
		{
			char hString[250];
			sprintf(hString, "/MQZone used at %.2f, %.2f, %.2f", GetX(), GetY(), GetZ());
			database.SetMQDetectionFlag(this->account_name, this->name, hString, zone->GetShortName());
		}
		break;
	case MQGate:
		if (RuleB(Zone, EnableMQGateDetector) && ((this->Admin() < RuleI(Zone, MQGateExemptStatus) || (RuleI(Zone, MQGateExemptStatus)) == -1))) {
			Message(13, "Illegal gate request.");
			char hString[250];
			sprintf(hString, "/MQGate used at %.2f, %.2f, %.2f", GetX(), GetY(), GetZ());
			database.SetMQDetectionFlag(this->account_name, this->name, hString, zone->GetShortName());
			if (zone)
			{
				this->SetZone(this->GetZoneID(), zone->GetInstanceID()); //Prevent the player from zoning, place him back in the zone where he tried to originally /gate.
			}
			else
			{
				this->SetZone(this->GetZoneID(), 0); //Prevent the player from zoning, place him back in the zone where he tried to originally /gate.

			}
		}
		break;
	case MQGhost: //Not currently implemented, but the framework is in place - just needs detection scenarios identified
		if (RuleB(Zone, EnableMQGhostDetector) && ((this->Admin() < RuleI(Zone, MQGhostExemptStatus) || (RuleI(Zone, MQGhostExemptStatus)) == -1))) {
			database.SetMQDetectionFlag(this->account_name, this->name, "/MQGhost", zone->GetShortName());
		}
		break;
	default:
		char *hString = nullptr;
		MakeAnyLenString(&hString, "Unhandled HackerDetection flag with location %.2f, %.2f, %.2f.", GetX(), GetY(), GetZ());
		database.SetMQDetectionFlag(this->account_name, this->name, hString, zone->GetShortName());
		safe_delete_array(hString);
		break;
	}
}

// connecting opcode handlers
/*
void Client::Handle_Connect_0x3e33(const EQApplicationPacket *app)
{
//OP_0x0380 = 0x642c
EQApplicationPacket* outapp = new EQApplicationPacket(OP_0x0380, sizeof(uint32)); // Dunno
QueuePacket(outapp);
safe_delete(outapp);
return;
}
*/

void Client::Handle_Connect_OP_ApproveZone(const EQApplicationPacket *app)
{
	if (app->size != sizeof(ApproveZone_Struct)) {
		Log(Logs::General, Logs::Error, "Invalid size on OP_ApproveZone: Expected %i, Got %i",
			sizeof(ApproveZone_Struct), app->size);
		return;
	}
	ApproveZone_Struct* azone = (ApproveZone_Struct*)app->pBuffer;
	azone->approve = 1;
	QueuePacket(app);
	return;
}

void Client::Handle_Connect_OP_ClientError(const EQApplicationPacket *app)
{
	if (app->size != sizeof(ClientError_Struct)) {
		Log(Logs::General, Logs::Error, "Invalid size on OP_ClientError: Expected %i, Got %i",
			sizeof(ClientError_Struct), app->size);
		return;
	}
	// Client reporting error to server
	ClientError_Struct* error = (ClientError_Struct*)app->pBuffer;
	Log(Logs::General, Logs::Error, "Client error: %s", error->character_name);
	Log(Logs::General, Logs::Error, "Error message: %s", error->message);
	Message(13, error->message);
#if (EQDEBUG>=5)
	DumpPacket(app);
#endif
	return;
}

void Client::Handle_Connect_OP_ClientReady(const EQApplicationPacket *app)
{
	conn_state = ClientReadyReceived;
	if (!Spawned())
		SendZoneInPackets();
	CompleteConnect();
	SendHPUpdate();
}

void Client::Handle_Connect_OP_ClientUpdate(const EQApplicationPacket *app)
{
	//Once we get this, the client thinks it is connected
	//So give it the benefit of the doubt and move to connected

	Handle_Connect_OP_ClientReady(app);
}

void Client::Handle_Connect_OP_ReqClientSpawn(const EQApplicationPacket *app)
{
	conn_state = ClientSpawnRequested;

	auto outapp = new EQApplicationPacket;

	// Send Zone Doors
	if (entity_list.MakeDoorSpawnPacket(outapp, this))
	{
		QueuePacket(outapp);
	}
	safe_delete(outapp);

	// Send Zone Objects
	entity_list.SendZoneObjects(this);
	SendZonePoints();
	// Live does this
	outapp = new EQApplicationPacket(OP_SendAAStats, 0);
	FastQueuePacket(&outapp);

	// Tell client they can continue we're done
	outapp = new EQApplicationPacket(OP_ZoneServerReady, 0);
	FastQueuePacket(&outapp);
	outapp = new EQApplicationPacket(OP_SendExpZonein, 0);
	FastQueuePacket(&outapp);

	if (ClientVersion() >= EQEmu::versions::ClientVersion::RoF)
	{
		outapp = new EQApplicationPacket(OP_ClientReady, 0);
		FastQueuePacket(&outapp);
	}

	// New for Secrets of Faydwer - Used in Place of OP_SendExpZonein
	outapp = new EQApplicationPacket(OP_WorldObjectsSent, 0);
	QueuePacket(outapp);
	safe_delete(outapp);

	if (strncasecmp(zone->GetShortName(), "bazaar", 6) == 0)
		SendBazaarWelcome();

	conn_state = ZoneContentsSent;

	return;
}

void Client::Handle_Connect_OP_ReqNewZone(const EQApplicationPacket *app)
{
	conn_state = NewZoneRequested;

	EQApplicationPacket* outapp = nullptr;

	/////////////////////////////////////
	// New Zone Packet
	outapp = new EQApplicationPacket(OP_NewZone, sizeof(NewZone_Struct));
	NewZone_Struct* nz = (NewZone_Struct*)outapp->pBuffer;
	memcpy(outapp->pBuffer, &zone->newzone_data, sizeof(NewZone_Struct));
	strcpy(nz->char_name, m_pp.name);

	FastQueuePacket(&outapp);

	return;
}

void Client::Handle_Connect_OP_SendAAStats(const EQApplicationPacket *app)
{
	SendAlternateAdvancementTimers();
	auto outapp = new EQApplicationPacket(OP_SendAAStats, 0);
	QueuePacket(outapp);
	safe_delete(outapp);
	return;
}

void Client::Handle_Connect_OP_SendAATable(const EQApplicationPacket *app)
{
	SendAlternateAdvancementTable();
	return;
}

void Client::Handle_Connect_OP_SendExpZonein(const EQApplicationPacket *app)
{
	auto outapp = new EQApplicationPacket(OP_SendExpZonein, 0);
	QueuePacket(outapp);
	safe_delete(outapp);

	// SoF+ Gets Zone-In packets after sending OP_WorldObjectsSent
	if (ClientVersion() < EQEmu::versions::ClientVersion::SoF)
	{
		SendZoneInPackets();
	}

	return;
}

void Client::Handle_Connect_OP_SendGuildTributes(const EQApplicationPacket *app)
{
	SendGuildTributes();
	return;
}

void Client::Handle_Connect_OP_SendTributes(const EQApplicationPacket *app)
{
	SendTributes();
	return;
}

void Client::Handle_Connect_OP_SetServerFilter(const EQApplicationPacket *app)
{
	if (app->size != sizeof(SetServerFilter_Struct)) {
		Log(Logs::General, Logs::Error, "Received invalid sized OP_SetServerFilter");
		DumpPacket(app);
		return;
	}
	SetServerFilter_Struct* filter = (SetServerFilter_Struct*)app->pBuffer;
	ServerFilter(filter);
	return;
}

void Client::Handle_Connect_OP_SpawnAppearance(const EQApplicationPacket *app)
{
	return;
}

void Client::Handle_Connect_OP_TGB(const EQApplicationPacket *app)
{
	if (app->size != sizeof(uint32)) {
		Log(Logs::General, Logs::Error, "Invalid size on OP_TGB: Expected %i, Got %i",
			sizeof(uint32), app->size);
		return;
	}
	OPTGB(app);
	return;
}

void Client::Handle_Connect_OP_UpdateAA(const EQApplicationPacket *app)
{
	SendAlternateAdvancementPoints();
}

void Client::Handle_Connect_OP_WearChange(const EQApplicationPacket *app)
{
	//not sure what these are supposed to mean to us.
	return;
}

void Client::Handle_Connect_OP_WorldObjectsSent(const EQApplicationPacket *app)
{
	// New for SoF+
	auto outapp = new EQApplicationPacket(OP_WorldObjectsSent, 0);
	QueuePacket(outapp);
	safe_delete(outapp);

	// Packet order changed for SoF+, so below is sent here instead of OP_SendExpLogin
	SendZoneInPackets();

	if (RuleB(Mercs, AllowMercs))
	{
		SpawnMercOnZone();
	}

	return;
}

void Client::Handle_Connect_OP_ZoneComplete(const EQApplicationPacket *app)
{
	auto outapp = new EQApplicationPacket(OP_0x0347, 0);
	QueuePacket(outapp);
	safe_delete(outapp);
	return;
}

void Client::Handle_Connect_OP_ZoneEntry(const EQApplicationPacket *app)
{
	if (app->size != sizeof(ClientZoneEntry_Struct))
		return;
	ClientZoneEntry_Struct *cze = (ClientZoneEntry_Struct *)app->pBuffer;

	if (strlen(cze->char_name) > 63)
		return;

	conn_state = ReceivedZoneEntry;

	SetClientVersion(Connection()->ClientVersion());
	m_ClientVersionBit = EQEmu::versions::ConvertClientVersionToClientVersionBit(Connection()->ClientVersion());

	bool siv = m_inv.SetInventoryVersion(m_ClientVersion);
	Log(Logs::General, Logs::None, "%s inventory version to %s(%i)", (siv ? "Succeeded in setting" : "Failed to set"), ClientVersionName(m_ClientVersion), m_ClientVersion);

	/* Antighost code
	tmp var is so the search doesnt find this object
	*/
	Client* client = entity_list.GetClientByName(cze->char_name);
	if (!zone->GetAuth(ip, cze->char_name, &WID, &account_id, &character_id, &admin, lskey, &tellsoff)) {
		Log(Logs::General, Logs::Error, "GetAuth() returned false kicking client");
		if (client != 0) {
			client->Save();
			client->Kick();
		}
		//ret = false; // TODO: Can we tell the client to get lost in a good way
		client_state = CLIENT_KICKED;
		return;
	}

	strcpy(name, cze->char_name);
	/* Check for Client Spoofing */
	if (client != 0) {
		struct in_addr ghost_addr;
		ghost_addr.s_addr = eqs->GetRemoteIP();

		Log(Logs::General, Logs::Error, "Ghosting client: Account ID:%i Name:%s Character:%s IP:%s",
			client->AccountID(), client->AccountName(), client->GetName(), inet_ntoa(ghost_addr));
		client->Save();
		client->Disconnect();
	}

	uint32 pplen = 0;
	EQApplicationPacket* outapp = nullptr;
	MYSQL_RES* result = nullptr;
	bool loaditems = 0;
	uint32 i;
	std::string query;
	unsigned long* lengths = nullptr;

	uint32 cid = CharacterID();
	character_id = cid; /* Global character_id reference */

						/* Flush and reload factions */
	database.RemoveTempFactions(this);
	database.LoadCharacterFactionValues(cid, factionvalues);

	/* Load Character Account Data: Temp until I move */
	query = StringFormat("SELECT `status`, `name`, `lsaccount_id`, `gmspeed`, `revoked`, `hideme`, `time_creation` FROM `account` WHERE `id` = %u", this->AccountID());
	auto results = database.QueryDatabase(query);
	for (auto row = results.begin(); row != results.end(); ++row) {
		admin = atoi(row[0]);
		strncpy(account_name, row[1], 30);
		lsaccountid = atoi(row[2]);
		gmspeed = atoi(row[3]);
		revoked = atoi(row[4]);
		gmhideme = atoi(row[5]);
		account_creation = atoul(row[6]);
	}

	/* Load Character Data */
	query = StringFormat("SELECT `lfp`, `lfg`, `xtargets`, `firstlogon`, `guild_id`, `rank` FROM `character_data` LEFT JOIN `guild_members` ON `id` = `char_id` WHERE `id` = %i", cid);
	results = database.QueryDatabase(query);
	for (auto row = results.begin(); row != results.end(); ++row) {
		if (row[4] && atoi(row[4]) > 0) {
			guild_id = atoi(row[4]);
			if (row[5] != nullptr) { guildrank = atoi(row[5]); }
			else { guildrank = GUILD_RANK_NONE; }
		}

		if (LFP) { LFP = atoi(row[0]); }
		if (LFG) { LFG = atoi(row[1]); }
		if (row[3])
			firstlogon = atoi(row[3]);
	}

	if (RuleB(Character, SharedBankPlat))
		m_pp.platinum_shared = database.GetSharedPlatinum(this->AccountID());

	database.ClearOldRecastTimestamps(cid); /* Clear out our old recast timestamps to keep the DB clean */
	loaditems = database.GetInventory(cid, &m_inv); /* Load Character Inventory */
	database.LoadCharacterBandolier(cid, &m_pp); /* Load Character Bandolier */
	database.LoadCharacterBindPoint(cid, &m_pp); /* Load Character Bind */
	database.LoadCharacterMaterialColor(cid, &m_pp); /* Load Character Material */
	database.LoadCharacterPotions(cid, &m_pp); /* Load Character Potion Belt */
	database.LoadCharacterCurrency(cid, &m_pp); /* Load Character Currency into PP */
	database.LoadCharacterData(cid, &m_pp, &m_epp); /* Load Character Data from DB into PP as well as E_PP */
	database.LoadCharacterSkills(cid, &m_pp); /* Load Character Skills */
	database.LoadCharacterInspectMessage(cid, &m_inspect_message); /* Load Character Inspect Message */
	database.LoadCharacterSpellBook(cid, &m_pp); /* Load Character Spell Book */
	database.LoadCharacterMemmedSpells(cid, &m_pp);  /* Load Character Memorized Spells */
	database.LoadCharacterDisciplines(cid, &m_pp); /* Load Character Disciplines */
	database.LoadCharacterLanguages(cid, &m_pp); /* Load Character Languages */
	database.LoadCharacterLeadershipAA(cid, &m_pp); /* Load Character Leadership AA's */
	database.LoadCharacterTribute(cid, &m_pp); /* Load CharacterTribute */

											   /* Load AdventureStats */
	AdventureStats_Struct as;
	if (database.GetAdventureStats(cid, &as))
	{
		m_pp.ldon_wins_guk = as.success.guk;
		m_pp.ldon_wins_mir = as.success.mir;
		m_pp.ldon_wins_mmc = as.success.mmc;
		m_pp.ldon_wins_ruj = as.success.ruj;
		m_pp.ldon_wins_tak = as.success.tak;
		m_pp.ldon_losses_guk = as.failure.guk;
		m_pp.ldon_losses_mir = as.failure.mir;
		m_pp.ldon_losses_mmc = as.failure.mmc;
		m_pp.ldon_losses_ruj = as.failure.ruj;
		m_pp.ldon_losses_tak = as.failure.tak;
	}

	/* Set item material tint */
	for (int i = EQEmu::textures::textureBegin; i <= EQEmu::textures::LastTexture; i++)
	{
		if (m_pp.item_tint.Slot[i].UseTint == 1 || m_pp.item_tint.Slot[i].UseTint == 255)
		{
			m_pp.item_tint.Slot[i].UseTint = 0xFF;
		}
	}

	if (level) { level = m_pp.level; }

	/* If GM, not trackable */
	if (gmhideme) { trackable = false; }
	/* Set Con State for Reporting */
	conn_state = PlayerProfileLoaded;

	m_pp.zone_id = zone->GetZoneID();
	m_pp.zoneInstance = zone->GetInstanceID();

	/* Set Total Seconds Played */
	TotalSecondsPlayed = m_pp.timePlayedMin * 60;
	/* Set Max AA XP */
	max_AAXP = RuleI(AA, ExpPerPoint);
	/* If we can maintain intoxication across zones, check for it */
	if (!RuleB(Character, MaintainIntoxicationAcrossZones))
		m_pp.intoxication = 0;

	strcpy(name, m_pp.name);
	strcpy(lastname, m_pp.last_name);
	/* If PP is set to weird coordinates */
	if ((m_pp.x == -1 && m_pp.y == -1 && m_pp.z == -1) || (m_pp.x == -2 && m_pp.y == -2 && m_pp.z == -2)) {
		auto safePoint = zone->GetSafePoint();
		m_pp.x = safePoint.x;
		m_pp.y = safePoint.y;
		m_pp.z = safePoint.z;
	}
	/* If too far below ground, then fix */
	// float ground_z = GetGroundZ(m_pp.x, m_pp.y, m_pp.z);
	// if (m_pp.z < (ground_z - 500))
	// 	m_pp.z = ground_z;

	/* Set Mob variables for spawn */
	class_ = m_pp.class_;
	level = m_pp.level;
	m_Position.x = m_pp.x;
	m_Position.y = m_pp.y;
	m_Position.z = m_pp.z;
	m_Position.w = m_pp.heading;
	race = m_pp.race;
	base_race = m_pp.race;
	gender = m_pp.gender;
	base_gender = m_pp.gender;
	deity = m_pp.deity;
	haircolor = m_pp.haircolor;
	beardcolor = m_pp.beardcolor;
	eyecolor1 = m_pp.eyecolor1;
	eyecolor2 = m_pp.eyecolor2;
	hairstyle = m_pp.hairstyle;
	luclinface = m_pp.face;
	beard = m_pp.beard;
	drakkin_heritage = m_pp.drakkin_heritage;
	drakkin_tattoo = m_pp.drakkin_tattoo;
	drakkin_details = m_pp.drakkin_details;

	/* If GM not set in DB, and does not meet min status to be GM, reset */
	if (m_pp.gm && admin < minStatusToBeGM)
		m_pp.gm = 0;

	/* Load Guild */
	if (!IsInAGuild()) {
		m_pp.guild_id = GUILD_NONE;
	}
	else {
		m_pp.guild_id = GuildID();
		uint8 rank = guild_mgr.GetDisplayedRank(GuildID(), GuildRank(), CharacterID());
		// FIXME: RoF guild rank
		if (ClientVersion() >= EQEmu::versions::ClientVersion::RoF) {
			switch (rank) {
			case 0:
				rank = 5;
				break;
			case 1:
				rank = 3;
				break;
			case 2:
				rank = 1;
				break;
			default:
				break;
			}
		}
		m_pp.guildrank = rank;
		if (zone->GetZoneID() == RuleI(World, GuildBankZoneID))
			GuildBanker = (guild_mgr.IsGuildLeader(GuildID(), CharacterID()) || guild_mgr.GetBankerFlag(CharacterID()));
	}
	m_pp.guildbanker = GuildBanker;

	switch (race)
	{
	case OGRE:
		size = 9; break;
	case TROLL:
		size = 8; break;
	case VAHSHIR: case BARBARIAN:
		size = 7; break;
	case HUMAN: case HIGH_ELF: case ERUDITE: case IKSAR: case DRAKKIN:
		size = 6; break;
	case HALF_ELF:
		size = 5.5; break;
	case WOOD_ELF: case DARK_ELF: case FROGLOK:
		size = 5; break;
	case DWARF:
		size = 4; break;
	case HALFLING:
		size = 3.5; break;
	case GNOME:
		size = 3; break;
	default:
		size = 0;
	}

	/* Check for Invalid points */
	if (m_pp.ldon_points_guk < 0 || m_pp.ldon_points_guk > 2000000000) { m_pp.ldon_points_guk = 0; }
	if (m_pp.ldon_points_mir < 0 || m_pp.ldon_points_mir > 2000000000) { m_pp.ldon_points_mir = 0; }
	if (m_pp.ldon_points_mmc < 0 || m_pp.ldon_points_mmc > 2000000000) { m_pp.ldon_points_mmc = 0; }
	if (m_pp.ldon_points_ruj < 0 || m_pp.ldon_points_ruj > 2000000000) { m_pp.ldon_points_ruj = 0; }
	if (m_pp.ldon_points_tak < 0 || m_pp.ldon_points_tak > 2000000000) { m_pp.ldon_points_tak = 0; }
	if (m_pp.ldon_points_available < 0 || m_pp.ldon_points_available > 2000000000) { m_pp.ldon_points_available = 0; }

	if (RuleB(World, UseClientBasedExpansionSettings)) {
		m_pp.expansions = EQEmu::versions::ConvertClientVersionToExpansion(ClientVersion());
	}
	else {
		m_pp.expansions = RuleI(World, ExpansionSettings);
	}

	if (!database.LoadAlternateAdvancement(this)) {
		Log(Logs::General, Logs::Error, "Error loading AA points for %s", GetName());
	}

	if (SPDAT_RECORDS > 0) {
		for (uint32 z = 0; z<MAX_PP_MEMSPELL; z++) {
			if (m_pp.mem_spells[z] >= (uint32)SPDAT_RECORDS)
				UnmemSpell(z, false);
		}

		database.LoadBuffs(this);
		uint32 max_slots = GetMaxBuffSlots();
		for (int i = 0; i < BUFF_COUNT; i++) {
			if (buffs[i].spellid != SPELL_UNKNOWN) {
				m_pp.buffs[i].spellid = buffs[i].spellid;
				m_pp.buffs[i].bard_modifier = buffs[i].instrument_mod;
				m_pp.buffs[i].effect_type = 2;
				m_pp.buffs[i].player_id = 0x2211;
				m_pp.buffs[i].level = buffs[i].casterlevel;
				m_pp.buffs[i].unknown003 = 0;
				m_pp.buffs[i].duration = buffs[i].ticsremaining;
				m_pp.buffs[i].counters = buffs[i].counters;
				m_pp.buffs[i].num_hits = buffs[i].numhits;
			}
			else {
				m_pp.buffs[i].spellid = SPELLBOOK_UNKNOWN;
				m_pp.buffs[i].bard_modifier = 10;
				m_pp.buffs[i].effect_type = 0;
				m_pp.buffs[i].player_id = 0;
				m_pp.buffs[i].level = 0;
				m_pp.buffs[i].unknown003 = 0;
				m_pp.buffs[i].duration = 0;
				m_pp.buffs[i].counters = 0;
				m_pp.buffs[i].num_hits = 0;
			}
		}
	}

	/* Load Character Key Ring */
	KeyRingLoad();

	/* Send Group Members via PP */
	uint32 groupid = database.GetGroupID(GetName());
	Group* group = nullptr;
	if (groupid > 0) {
		group = entity_list.GetGroupByID(groupid);
		if (!group) {	//nobody from our is here... start a new group
			group = new Group(groupid);
			if (group->GetID() != 0)
				entity_list.AddGroup(group, groupid);
			else	//error loading group members...
			{
				delete group;
				group = nullptr;
			}
		}	//else, somebody from our group is already here...

		if (!group)
			database.SetGroupID(GetName(), 0, CharacterID(), false);	//cannot re-establish group, kill it

	}
	else {	//no group id
			//clear out the group junk in our PP
		uint32 xy = 0;
		for (xy = 0; xy < MAX_GROUP_MEMBERS; xy++)
			memset(m_pp.groupMembers[xy], 0, 64);
	}

	if (group) {
		// If the group leader is not set, pull the group leader infomrmation from the database.
		if (!group->GetLeader()) {
			char ln[64];
			char MainTankName[64];
			char AssistName[64];
			char PullerName[64];
			char NPCMarkerName[64];
			char mentoree_name[64];
			int mentor_percent;
			GroupLeadershipAA_Struct GLAA;
			memset(ln, 0, 64);
			strcpy(ln, database.GetGroupLeadershipInfo(group->GetID(), ln, MainTankName, AssistName, PullerName, NPCMarkerName, mentoree_name, &mentor_percent, &GLAA));
			Client *c = entity_list.GetClientByName(ln);
			if (c)
				group->SetLeader(c);

			group->SetMainTank(MainTankName);
			group->SetMainAssist(AssistName);
			group->SetPuller(PullerName);
			group->SetNPCMarker(NPCMarkerName);
			group->SetGroupAAs(&GLAA);
			group->SetGroupMentor(mentor_percent, mentoree_name);

			//group->NotifyMainTank(this, 1);
			//group->NotifyMainAssist(this, 1);
			//group->NotifyPuller(this, 1);

			// If we are the leader, force an update of our group AAs to other members in the zone, in case
			// we purchased a new one while out-of-zone.
			if (group->IsLeader(this))
				group->SendLeadershipAAUpdate();
		}
		JoinGroupXTargets(group);
		group->UpdatePlayer(this);
		LFG = false;
	}

#ifdef BOTS
	Bot::LoadAndSpawnAllZonedBots(this);
#endif

	CalcBonuses();
	if (RuleB(Zone, EnableLoggedOffReplenishments) &&
		time(nullptr) - m_pp.lastlogin >= RuleI(Zone, MinOfflineTimeToReplenishments)) {
		m_pp.cur_hp = GetMaxHP();
		m_pp.mana = GetMaxMana();
		m_pp.endurance = GetMaxEndurance();
	}

	if (m_pp.cur_hp <= 0)
		m_pp.cur_hp = GetMaxHP();

	SetHP(m_pp.cur_hp);
	Mob::SetMana(m_pp.mana); // mob function doesn't send the packet
	SetEndurance(m_pp.endurance);

	/* Update LFP in case any (or all) of our group disbanded while we were zoning. */
	if (IsLFP()) { UpdateLFP(); }

	p_timers.SetCharID(CharacterID());
	if (!p_timers.Load(&database)) {
		Log(Logs::General, Logs::Error, "Unable to load ability timers from the database for %s (%i)!", GetCleanName(), CharacterID());
	}

	/* Load Spell Slot Refresh from Currently Memoried Spells */
	for (unsigned int i = 0; i < MAX_PP_MEMSPELL; ++i)
		if (IsValidSpell(m_pp.mem_spells[i]))
			m_pp.spellSlotRefresh[i] = p_timers.GetRemainingTime(pTimerSpellStart + m_pp.mem_spells[i]) * 1000;

	/* Ability slot refresh send SK/PAL */
	if (m_pp.class_ == SHADOWKNIGHT || m_pp.class_ == PALADIN) {
		uint32 abilitynum = 0;
		if (m_pp.class_ == SHADOWKNIGHT) { abilitynum = pTimerHarmTouch; }
		else { abilitynum = pTimerLayHands; }

		uint32 remaining = p_timers.GetRemainingTime(abilitynum);
		if (remaining > 0 && remaining < 15300)
			m_pp.abilitySlotRefresh = remaining * 1000;
		else
			m_pp.abilitySlotRefresh = 0;
	}

#ifdef _EQDEBUG
	printf("Dumping inventory on load:\n");
	m_inv.dumpEntireInventory();
#endif

	/* Reset to max so they dont drown on zone in if its underwater */
	m_pp.air_remaining = 60;
	/* Check for PVP Zone status*/
	if (zone->IsPVPZone())
		m_pp.pvp = 1;
	/* Time entitled on Account: Move to account */
	m_pp.timeentitledonaccount = database.GetTotalTimeEntitledOnAccount(AccountID()) / 1440;
	/* Reset rest timer if the durations have been lowered in the database */
	if ((m_pp.RestTimer > RuleI(Character, RestRegenTimeToActivate)) && (m_pp.RestTimer > RuleI(Character, RestRegenRaidTimeToActivate)))
		m_pp.RestTimer = 0;

	/* This checksum should disappear once dynamic structs are in... each struct strategy will do it */
	CRC32::SetEQChecksum((unsigned char*)&m_pp, sizeof(PlayerProfile_Struct) - 4);

	outapp = new EQApplicationPacket(OP_PlayerProfile, sizeof(PlayerProfile_Struct));

	/* The entityid field in the Player Profile is used by the Client in relation to Group Leadership AA */
	m_pp.entityid = GetID();
	memcpy(outapp->pBuffer, &m_pp, outapp->size);
	outapp->priority = 6;
	FastQueuePacket(&outapp);

	if (m_pp.RestTimer)
		rest_timer.Start(m_pp.RestTimer * 1000);

	/* Load Pet */
	database.LoadPetInfo(this);
	if (m_petinfo.SpellID > 1 && !GetPet() && m_petinfo.SpellID <= SPDAT_RECORDS) {
		MakePoweredPet(m_petinfo.SpellID, spells[m_petinfo.SpellID].teleport_zone, m_petinfo.petpower, m_petinfo.Name, m_petinfo.size);
		if (GetPet() && GetPet()->IsNPC()) {
			NPC *pet = GetPet()->CastToNPC();
			pet->SetPetState(m_petinfo.Buffs, m_petinfo.Items);
			pet->CalcBonuses();
			pet->SetHP(m_petinfo.HP);
			pet->SetMana(m_petinfo.Mana);
		}
		m_petinfo.SpellID = 0;
	}
	/* Moved here so it's after where we load the pet data. */
	if (!GetAA(aaPersistentMinion))
		memset(&m_suspendedminion, 0, sizeof(PetInfo));

	/* Server Zone Entry Packet */
	outapp = new EQApplicationPacket(OP_ZoneEntry, sizeof(ServerZoneEntry_Struct));
	ServerZoneEntry_Struct* sze = (ServerZoneEntry_Struct*)outapp->pBuffer;

	FillSpawnStruct(&sze->player, CastToMob());
	sze->player.spawn.curHp = 1;
	sze->player.spawn.NPC = 0;
	sze->player.spawn.z += 6;	//arbitrary lift, seems to help spawning under zone.
	outapp->priority = 6;
	FastQueuePacket(&outapp);

	/* Zone Spawns Packet */
	entity_list.SendZoneSpawnsBulk(this);
	entity_list.SendZoneCorpsesBulk(this);
	entity_list.SendZonePVPUpdates(this);	//hack until spawn struct is fixed.

											/* Time of Day packet */
	outapp = new EQApplicationPacket(OP_TimeOfDay, sizeof(TimeOfDay_Struct));
	TimeOfDay_Struct* tod = (TimeOfDay_Struct*)outapp->pBuffer;
	zone->zone_time.GetCurrentEQTimeOfDay(time(0), tod);
	outapp->priority = 6;
	FastQueuePacket(&outapp);

	/* Tribute Packets */
	DoTributeUpdate();
	if (m_pp.tribute_active) {
		//restart the tribute timer where we left off
		tribute_timer.Start(m_pp.tribute_time_remaining);
	}

	/*
	Character Inventory Packet
	this is not quite where live sends inventory, they do it after tribute
	*/
	if (loaditems) { /* Dont load if a length error occurs */
		BulkSendInventoryItems();
		/* Send stuff on the cursor which isnt sent in bulk */
		for (auto iter = m_inv.cursor_cbegin(); iter != m_inv.cursor_cend(); ++iter) {
			/* First item cursor is sent in bulk inventory packet */
			if (iter == m_inv.cursor_cbegin())
				continue;
			const EQEmu::ItemInstance *inst = *iter;
			SendItemPacket(EQEmu::inventory::slotCursor, inst, ItemPacketLimbo);
		}
	}

	/* Task Packets */
	LoadClientTaskState();

	if (ClientVersion() >= EQEmu::versions::ClientVersion::RoF) {
		outapp = new EQApplicationPacket(OP_ReqNewZone, 0);
		Handle_Connect_OP_ReqNewZone(outapp);
		safe_delete(outapp);
	}

	if (m_ClientVersionBit & EQEmu::versions::bit_UFAndLater) {
		outapp = new EQApplicationPacket(OP_XTargetResponse, 8);
		outapp->WriteUInt32(GetMaxXTargets());
		outapp->WriteUInt32(0);
		FastQueuePacket(&outapp);
	}

	/*
	Weather Packet
	This shouldent be moved, this seems to be what the client
	uses to advance to the next state (sending ReqNewZone)
	*/
	outapp = new EQApplicationPacket(OP_Weather, 12);
	Weather_Struct *ws = (Weather_Struct *)outapp->pBuffer;
	ws->val1 = 0x000000FF;
	if (zone->zone_weather == 1) { ws->type = 0x31; } // Rain
	if (zone->zone_weather == 2) {
		outapp->pBuffer[8] = 0x01;
		ws->type = 0x02;
	}
	outapp->priority = 6;
	QueuePacket(outapp);
	safe_delete(outapp);

	SetAttackTimer();
	conn_state = ZoneInfoSent;
	zoneinpacket_timer.Start();
	return;
}

// connected opcode handlers
void Client::Handle_0x0193(const EQApplicationPacket *app)
{
	// Not sure what this opcode does. It started being sent when OP_ClientUpdate was
	// changed to pump OP_ClientUpdate back out instead of OP_MobUpdate
	// 2 bytes: 00 00

	return;
}

void Client::Handle_OP_AAAction(const EQApplicationPacket *app)
{
	Log(Logs::Detail, Logs::AA, "Received OP_AAAction");

	if (app->size != sizeof(AA_Action)) {
		Log(Logs::General, Logs::AA, "Error! OP_AAAction size didnt match!");
		return;
	}
	AA_Action* action = (AA_Action*)app->pBuffer;

	if (action->action == aaActionActivate) {//AA Hotkey
		Log(Logs::Detail, Logs::AA, "Activating AA %d", action->ability);
		ActivateAlternateAdvancementAbility(action->ability, action->target_id);
	}
	else if (action->action == aaActionBuy) {
		PurchaseAlternateAdvancementRank(action->ability);
	}
	else if (action->action == aaActionDisableEXP) { //Turn Off AA Exp
		if (m_epp.perAA > 0)
			Message_StringID(0, AA_OFF);

		m_epp.perAA = 0;
		SendAlternateAdvancementStats();
	}
	else if (action->action == aaActionSetEXP) {
		if (m_epp.perAA == 0)
			Message_StringID(0, AA_ON);
		m_epp.perAA = action->exp_value;
		if (m_epp.perAA < 0 || m_epp.perAA > 100)
			m_epp.perAA = 0;	// stop exploit with sanity check

								// send an update
		SendAlternateAdvancementStats();
		SendAlternateAdvancementTable();
	}
	else {
		Log(Logs::General, Logs::AA, "Unknown AA action : %u %u %u %d", action->action, action->ability, action->target_id, action->exp_value);
	}
}

void Client::Handle_OP_AcceptNewTask(const EQApplicationPacket *app)
{

	if (app->size != sizeof(AcceptNewTask_Struct)) {
		Log(Logs::General, Logs::None, "Size mismatch in OP_AcceptNewTask expected %i got %i",
			sizeof(AcceptNewTask_Struct), app->size);
		DumpPacket(app);
		return;
	}
	AcceptNewTask_Struct *ant = (AcceptNewTask_Struct*)app->pBuffer;

	if (ant->task_id > 0 && RuleB(TaskSystem, EnableTaskSystem) && taskstate)
		taskstate->AcceptNewTask(this, ant->task_id, ant->task_master_id);
}

void Client::Handle_OP_AdventureInfoRequest(const EQApplicationPacket *app)
{
	if (app->size < sizeof(EntityId_Struct))
	{
		Log(Logs::General, Logs::Error, "Handle_OP_AdventureInfoRequest had a packet that was too small.");
		return;
	}
	EntityId_Struct* ent = (EntityId_Struct*)app->pBuffer;
	Mob * m = entity_list.GetMob(ent->entity_id);
	if (m && m->IsNPC())
	{
		std::map<uint32, std::string>::iterator it;
		it = zone->adventure_entry_list_flavor.find(m->CastToNPC()->GetAdventureTemplate());
		if (it != zone->adventure_entry_list_flavor.end())
		{
			auto outapp = new EQApplicationPacket(OP_AdventureInfo, (it->second.size() + 2));
			strn0cpy((char*)outapp->pBuffer, it->second.c_str(), it->second.size());
			FastQueuePacket(&outapp);
		}
		else
		{
			if (m->CastToNPC()->GetAdventureTemplate() != 0)
			{
				std::string text = "Choose your difficulty and preferred adventure type.";
				auto outapp = new EQApplicationPacket(OP_AdventureInfo, (text.size() + 2));
				strn0cpy((char*)outapp->pBuffer, text.c_str(), text.size());
				FastQueuePacket(&outapp);
			}
		}
	}
}

void Client::Handle_OP_AdventureLeaderboardRequest(const EQApplicationPacket *app)
{
	if (app->size < sizeof(AdventureLeaderboardRequest_Struct))
	{
		return;
	}

	if (adventure_leaderboard_timer)
	{
		return;
	}

	adventure_leaderboard_timer = new Timer(4000);
	auto pack = new ServerPacket(ServerOP_AdventureLeaderboard, sizeof(ServerLeaderboardRequest_Struct));
	ServerLeaderboardRequest_Struct *lr = (ServerLeaderboardRequest_Struct*)pack->pBuffer;
	strcpy(lr->player, GetName());

	AdventureLeaderboardRequest_Struct *lrs = (AdventureLeaderboardRequest_Struct*)app->pBuffer;
	lr->type = 1 + (lrs->theme * 2) + lrs->type;
	worldserver.SendPacket(pack);
	delete pack;
}

void Client::Handle_OP_AdventureMerchantPurchase(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Adventure_Purchase_Struct))
	{
		Log(Logs::General, Logs::Error, "OP size error: OP_AdventureMerchantPurchase expected:%i got:%i", sizeof(Adventure_Purchase_Struct), app->size);
		return;
	}

	Adventure_Purchase_Struct* aps = (Adventure_Purchase_Struct*)app->pBuffer;
	/*
	Get item apc->itemid (can check NPC if thats necessary), ldon point theme check only if theme is not 0 (I am not sure what 1-5 are though for themes)
	if(ldon_points_available >= item ldonpointcost)
	{
	give item (67 00 00 00 for the packettype using opcode 0x02c5)
	ldon_points_available -= ldonpointcost;
	}
	*/
	uint32 merchantid = 0;
	Mob* tmp = entity_list.GetMob(aps->npcid);
	if (tmp == 0 || !tmp->IsNPC() || ((tmp->GetClass() != ADVENTUREMERCHANT) &&
		(tmp->GetClass() != DISCORD_MERCHANT) && (tmp->GetClass() != NORRATHS_KEEPERS_MERCHANT) && (tmp->GetClass() != DARK_REIGN_MERCHANT)))
		return;

	//you have to be somewhat close to them to be properly using them
	if (DistanceSquared(m_Position, tmp->GetPosition()) > USE_NPC_RANGE2)
		return;

	merchantid = tmp->CastToNPC()->MerchantType;

	const EQEmu::ItemData* item = nullptr;
	bool found = false;
	std::list<MerchantList> merlist = zone->merchanttable[merchantid];
	std::list<MerchantList>::const_iterator itr;

	for (itr = merlist.begin(); itr != merlist.end(); ++itr) {
		MerchantList ml = *itr;
		if (GetLevel() < ml.level_required) {
			continue;
		}

		int32 fac = tmp->GetPrimaryFaction();
		if (fac != 0 && GetModCharacterFactionLevel(fac) < ml.faction_required) {
			continue;
		}

		item = database.GetItem(ml.item);
		if (!item)
			continue;
		if (item->ID == aps->itemid) { //This check to make sure that the item is actually on the NPC, people attempt to inject packets to get items summoned...
			found = true;
			break;
		}
	}
	if (!item || !found) {
		Message(13, "Error: The item you purchased does not exist!");
		return;
	}

	if (aps->Type == LDoNMerchant)
	{
		if (m_pp.ldon_points_available < int32(item->LDoNPrice)) {
			Message(13, "You cannot afford that item.");
			return;
		}

		if (item->LDoNTheme <= 16)
		{
			if (item->LDoNTheme & 16)
			{
				if (m_pp.ldon_points_tak < int32(item->LDoNPrice))
				{
					Message(13, "You need at least %u points in tak to purchase this item.", int32(item->LDoNPrice));
					return;
				}
			}
			else if (item->LDoNTheme & 8)
			{
				if (m_pp.ldon_points_ruj < int32(item->LDoNPrice))
				{
					Message(13, "You need at least %u points in ruj to purchase this item.", int32(item->LDoNPrice));
					return;
				}
			}
			else if (item->LDoNTheme & 4)
			{
				if (m_pp.ldon_points_mmc < int32(item->LDoNPrice))
				{
					Message(13, "You need at least %u points in mmc to purchase this item.", int32(item->LDoNPrice));
					return;
				}
			}
			else if (item->LDoNTheme & 2)
			{
				if (m_pp.ldon_points_mir < int32(item->LDoNPrice))
				{
					Message(13, "You need at least %u points in mir to purchase this item.", int32(item->LDoNPrice));
					return;
				}
			}
			else if (item->LDoNTheme & 1)
			{
				if (m_pp.ldon_points_guk < int32(item->LDoNPrice))
				{
					Message(13, "You need at least %u points in guk to purchase this item.", int32(item->LDoNPrice));
					return;
				}
			}
		}
	}
	else if (aps->Type == DiscordMerchant)
	{
		if (GetPVPPoints() < item->LDoNPrice)
		{
			Message(13, "You need at least %u PVP points to purchase this item.", int32(item->LDoNPrice));
			return;
		}
	}
	else if (aps->Type == NorrathsKeepersMerchant)
	{
		if (GetRadiantCrystals() < item->LDoNPrice)
		{
			Message(13, "You need at least %u Radiant Crystals to purchase this item.", int32(item->LDoNPrice));
			return;
		}
	}
	else if (aps->Type == DarkReignMerchant)
	{
		if (GetEbonCrystals() < item->LDoNPrice)
		{
			Message(13, "You need at least %u Ebon Crystals to purchase this item.", int32(item->LDoNPrice));
			return;
		}
	}
	else
	{
		Message(13, "Unknown Adventure Merchant type.");
		return;
	}


	if (CheckLoreConflict(item))
	{
		Message(15, "You can only have one of a lore item.");
		return;
	}

	if (aps->Type == LDoNMerchant)
	{
		int32 requiredpts = (int32)item->LDoNPrice*-1;

		if (!UpdateLDoNPoints(requiredpts, 6))
			return;
	}
	else if (aps->Type == DiscordMerchant)
	{
		SetPVPPoints(GetPVPPoints() - (int32)item->LDoNPrice);
		SendPVPStats();
	}
	else if (aps->Type == NorrathsKeepersMerchant)
	{
		SetRadiantCrystals(GetRadiantCrystals() - (int32)item->LDoNPrice);
		SendCrystalCounts();
	}
	else if (aps->Type == DarkReignMerchant)
	{
		SetEbonCrystals(GetEbonCrystals() - (int32)item->LDoNPrice);
		SendCrystalCounts();
	}
	int16 charges = 1;
	if (item->MaxCharges != 0)
		charges = item->MaxCharges;

	EQEmu::ItemInstance *inst = database.CreateItem(item, charges);
	if (!AutoPutLootInInventory(*inst, true, true))
	{
		PutLootInInventory(EQEmu::inventory::slotCursor, *inst);
	}
	Save(1);
}

void Client::Handle_OP_AdventureMerchantRequest(const EQApplicationPacket *app)
{
	if (app->size != sizeof(AdventureMerchant_Struct))
	{
		Log(Logs::General, Logs::Error, "OP size error: OP_AdventureMerchantRequest expected:%i got:%i", sizeof(AdventureMerchant_Struct), app->size);
		return;
	}
	std::stringstream ss(std::stringstream::in | std::stringstream::out);

	uint8 count = 0;
	AdventureMerchant_Struct* eid = (AdventureMerchant_Struct*)app->pBuffer;
	uint32 merchantid = 0;

	Mob* tmp = entity_list.GetMob(eid->entity_id);
	if (tmp == 0 || !tmp->IsNPC() || ((tmp->GetClass() != ADVENTUREMERCHANT) &&
		(tmp->GetClass() != DISCORD_MERCHANT) && (tmp->GetClass() != NORRATHS_KEEPERS_MERCHANT) && (tmp->GetClass() != DARK_REIGN_MERCHANT)))
		return;

	//you have to be somewhat close to them to be properly using them
	if (DistanceSquared(m_Position, tmp->GetPosition()) > USE_NPC_RANGE2)
		return;

	merchantid = tmp->CastToNPC()->MerchantType;
	tmp->CastToNPC()->FaceTarget(this->CastToMob());

	const EQEmu::ItemData *item = nullptr;
	std::list<MerchantList> merlist = zone->merchanttable[merchantid];
	std::list<MerchantList>::const_iterator itr;
	for (itr = merlist.begin(); itr != merlist.end() && count<255; ++itr) {
		const MerchantList &ml = *itr;
		if (GetLevel() < ml.level_required) {
			continue;
		}

		int32 fac = tmp->GetPrimaryFaction();
		if (fac != 0 && GetModCharacterFactionLevel(fac) < ml.faction_required) {
			continue;
		}

		item = database.GetItem(ml.item);
		if (item)
		{
			uint32 theme;
			if (item->LDoNTheme > 16)
			{
				theme = 0;
			}
			else if (item->LDoNTheme & 16)
			{
				theme = 5;
			}
			else if (item->LDoNTheme & 8)
			{
				theme = 4;
			}
			else if (item->LDoNTheme & 4)
			{
				theme = 3;
			}
			else if (item->LDoNTheme & 2)
			{
				theme = 2;
			}
			else if (item->LDoNTheme & 1)
			{
				theme = 1;
			}
			else
			{
				theme = 0;
			}
			ss << "^" << item->Name << "|";
			ss << item->ID << "|";
			ss << item->LDoNPrice << "|";
			ss << theme << "|";
			ss << "0|";
			ss << "1|";
			ss << item->Races << "|";
			ss << item->Classes;
			count++;
		}
	}
	//Count
	//^Item Name,Item ID,Cost in Points,Theme (0=none),0,1,races bit map,classes bitmap
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_AdventureMerchantResponse, ss.str().size() + 2);
	outapp->pBuffer[0] = count;
	strn0cpy((char*)&outapp->pBuffer[1], ss.str().c_str(), ss.str().size());
	FastQueuePacket(&outapp);
}

void Client::Handle_OP_AdventureMerchantSell(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Adventure_Sell_Struct))
	{
		Log(Logs::General, Logs::None, "Size mismatch on OP_AdventureMerchantSell: got %u expected %u",
			app->size, sizeof(Adventure_Sell_Struct));
		DumpPacket(app);
		return;
	}

	Adventure_Sell_Struct *ams_in = (Adventure_Sell_Struct*)app->pBuffer;

	Mob* vendor = entity_list.GetMob(ams_in->npcid);
	if (vendor == 0 || !vendor->IsNPC() || ((vendor->GetClass() != ADVENTUREMERCHANT) &&
		(vendor->GetClass() != NORRATHS_KEEPERS_MERCHANT) && (vendor->GetClass() != DARK_REIGN_MERCHANT)))
	{
		Message(13, "Vendor was not found.");
		return;
	}

	if (DistanceSquared(m_Position, vendor->GetPosition())  > USE_NPC_RANGE2)
	{
		Message(13, "Vendor is out of range.");
		return;
	}

	uint32 itemid = GetItemIDAt(ams_in->slot);

	if (itemid == 0)
	{
		Message(13, "Found no item at that slot.");
		return;
	}

	const EQEmu::ItemData* item = database.GetItem(itemid);
	EQEmu::ItemInstance* inst = GetInv().GetItem(ams_in->slot);
	if (!item || !inst) {
		Message(13, "You seemed to have misplaced that item...");
		return;
	}

	// Note that Lucy has ldonsold values of 4 and 5 for items sold by Norrath's Keepers and Dark Reign, whereas 13th Floor
	// has ldonsold = 0 for these items, so some manual editing of the items DB will be required to support sell back of the
	// items.
	//
	// The Merchant seems to have some other way of knowing whether he will accept the item, other than the ldonsold field,
	// e.g. if you summon items 76036 and 76053 (good and evil versions of Spell: Ward Of Vengeance), if you are interacting
	// with a Norrath's Keeper merchant and click on 76036 in your inventory, he says he will give you radiant crystals for
	// it, but he will refuse for item 76053.
	//
	// Similarly, just giving a cloth cap an ldonsold value of 4 will not make the Merchant buy it.
	//
	// Note that the the Client will not allow you to sell anything back to a Discord merchant, so there is no need to handle
	// that case here.
	if (item->LDoNSold == 0)
	{
		Message(13, "The merchant does not want that item.");
		return;
	}

	if (item->LDoNPrice == 0)
	{
		Message(13, "The merchant does not want that item.");
		return;
	}

	// 06/11/2016 This formula matches RoF2 client side calculation.
	int32 price = (item->LDoNPrice + 1) * item->LDoNSellBackRate / 100;

	if (price == 0)
	{
		Message(13, "The merchant does not want that item.");
		return;
	}

	if (RuleB(EventLog, RecordSellToMerchant))
		LogMerchant(this, vendor, ams_in->charges, price, item, false);

	if (!inst->IsStackable())
	{
		DeleteItemInInventory(ams_in->slot, 0, false);
	}
	else
	{
		if (inst->GetCharges() < ams_in->charges)
		{
			ams_in->charges = inst->GetCharges();
		}

		if (ams_in->charges == 0)
		{
			Message(13, "Charge mismatch error.");
			return;
		}

		DeleteItemInInventory(ams_in->slot, ams_in->charges, false);
		price *= ams_in->charges;
	}

	auto outapp = new EQApplicationPacket(OP_AdventureMerchantSell, sizeof(Adventure_Sell_Struct));
	Adventure_Sell_Struct *ams = (Adventure_Sell_Struct*)outapp->pBuffer;
	ams->slot = ams_in->slot;
	ams->unknown000 = 1;
	ams->npcid = ams->npcid;
	ams->charges = ams_in->charges;
	ams->sell_price = price;
	FastQueuePacket(&outapp);

	switch (vendor->GetClass())
	{
	case ADVENTUREMERCHANT:
	{
		UpdateLDoNPoints(price, 6);
		break;
	}
	case NORRATHS_KEEPERS_MERCHANT:
	{
		SetRadiantCrystals(GetRadiantCrystals() + price);
		break;
	}
	case DARK_REIGN_MERCHANT:
	{
		SetEbonCrystals(GetEbonCrystals() + price);
		break;
	}

	default:
		break;
	}

	Save(1);
}

void Client::Handle_OP_AdventureRequest(const EQApplicationPacket *app)
{
	if (app->size < sizeof(AdventureRequest_Struct))
	{
		Log(Logs::General, Logs::Error, "Handle_OP_AdventureRequest had a packet that was too small.");
		return;
	}

	if (IsOnAdventure())
	{
		return;
	}

	if (!p_timers.Expired(&database, pTimerStartAdventureTimer, false))
	{
		return;
	}

	if (GetPendingAdventureRequest())
	{
		return;
	}

	AdventureRequest_Struct* ars = (AdventureRequest_Struct*)app->pBuffer;
	uint8 group_members = 0;
	Raid *r = nullptr;
	Group *g = nullptr;

	if (IsRaidGrouped())
	{
		r = GetRaid();
		group_members = r->RaidCount();
	}
	else if (IsGrouped())
	{
		g = GetGroup();
		group_members = g->GroupCount();
	}
	else
	{
		return;
	}

	if (group_members < RuleI(Adventure, MinNumberForGroup) || group_members > RuleI(Adventure, MaxNumberForGroup))
	{
		return;
	}

	Mob* m = entity_list.GetMob(ars->entity_id);
	uint32 template_id = 0;
	if (m && m->IsNPC())
	{
		template_id = m->CastToNPC()->GetAdventureTemplate();
	}
	else
	{
		return;
	}

	auto packet =
		new ServerPacket(ServerOP_AdventureRequest, sizeof(ServerAdventureRequest_Struct) + (64 * group_members));
	ServerAdventureRequest_Struct *sar = (ServerAdventureRequest_Struct*)packet->pBuffer;
	sar->member_count = group_members;
	sar->risk = ars->risk;
	sar->type = ars->type;
	sar->template_id = template_id;
	strcpy(sar->leader, GetName());

	if (IsRaidGrouped())
	{
		int i = 0;
		for (int x = 0; x < 72; ++x)
		{
			if (i == group_members)
			{
				break;
			}

			const char *c_name = nullptr;
			c_name = r->GetClientNameByIndex(x);
			if (c_name)
			{
				memcpy((packet->pBuffer + sizeof(ServerAdventureRequest_Struct) + (64 * i)), c_name, strlen(c_name));
				++i;
			}
		}
	}
	else
	{
		int i = 0;
		for (int x = 0; x < 6; ++x)
		{
			if (i == group_members)
			{
				break;
			}

			const char *c_name = nullptr;
			c_name = g->GetClientNameByIndex(x);
			if (c_name)
			{
				memcpy((packet->pBuffer + sizeof(ServerAdventureRequest_Struct) + (64 * i)), c_name, strlen(c_name));
				++i;
			}
		}
	}

	worldserver.SendPacket(packet);
	delete packet;
	p_timers.Start(pTimerStartAdventureTimer, 5);
}

void Client::Handle_OP_AdventureStatsRequest(const EQApplicationPacket *app)
{
	if (adventure_stats_timer)
	{
		return;
	}

	adventure_stats_timer = new Timer(8000);
	auto outapp = new EQApplicationPacket(OP_AdventureStatsReply, sizeof(AdventureStats_Struct));
	AdventureStats_Struct *as = (AdventureStats_Struct*)outapp->pBuffer;

	if (database.GetAdventureStats(CharacterID(), as))
	{
		m_pp.ldon_wins_guk = as->success.guk;
		m_pp.ldon_wins_mir = as->success.mir;
		m_pp.ldon_wins_mmc = as->success.mmc;
		m_pp.ldon_wins_ruj = as->success.ruj;
		m_pp.ldon_wins_tak = as->success.tak;
		m_pp.ldon_losses_guk = as->failure.guk;
		m_pp.ldon_losses_mir = as->failure.mir;
		m_pp.ldon_losses_mmc = as->failure.mmc;
		m_pp.ldon_losses_ruj = as->failure.ruj;
		m_pp.ldon_losses_tak = as->failure.tak;
	}

	FastQueuePacket(&outapp);
}

void Client::Handle_OP_AggroMeterLockTarget(const EQApplicationPacket *app)
{
	if (app->size < sizeof(uint32)) {
		Log(Logs::General, Logs::Error, "Handle_OP_AggroMeterLockTarget had a packet that was too small.");
		return;
	}

	SetAggroMeterLock(app->ReadUInt32(0));
	ProcessAggroMeter();
}

void Client::Handle_OP_AltCurrencyMerchantRequest(const EQApplicationPacket *app)
{
	VERIFY_PACKET_LENGTH(OP_AltCurrencyMerchantRequest, app, uint32);

	NPC* tar = entity_list.GetNPCByID(*((uint32*)app->pBuffer));
	if (tar) {
		if (DistanceSquared(m_Position, tar->GetPosition())  > USE_NPC_RANGE2)
			return;

		if (tar->GetClass() != ALT_CURRENCY_MERCHANT) {
			return;
		}

		uint32 alt_cur_id = tar->GetAltCurrencyType();
		if (alt_cur_id == 0) {
			return;
		}

		auto altc_iter = zone->AlternateCurrencies.begin();
		bool found = false;
		while (altc_iter != zone->AlternateCurrencies.end()) {
			if ((*altc_iter).id == alt_cur_id) {
				found = true;
				break;
			}
			++altc_iter;
		}

		if (!found) {
			return;
		}

		std::stringstream ss(std::stringstream::in | std::stringstream::out);
		std::stringstream item_ss(std::stringstream::in | std::stringstream::out);
		ss << alt_cur_id << "|1|" << alt_cur_id;
		uint32 count = 0;
		uint32 merchant_id = tar->MerchantType;
		const EQEmu::ItemData *item = nullptr;

		std::list<MerchantList> merlist = zone->merchanttable[merchant_id];
		std::list<MerchantList>::const_iterator itr;
		for (itr = merlist.begin(); itr != merlist.end() && count < 255; ++itr) {
			const MerchantList &ml = *itr;
			if (GetLevel() < ml.level_required) {
				continue;
			}

			int32 fac = tar->GetPrimaryFaction();
			if (fac != 0 && GetModCharacterFactionLevel(fac) < ml.faction_required) {
				continue;
			}

			item = database.GetItem(ml.item);
			if (item)
			{
				item_ss << "^" << item->Name << "|";
				item_ss << item->ID << "|";
				item_ss << ml.alt_currency_cost << "|";
				item_ss << "0|";
				item_ss << "1|";
				item_ss << item->Races << "|";
				item_ss << item->Classes;
				count++;
			}
		}

		if (count > 0) {
			ss << "|" << count << item_ss.str();
		}
		else {
			ss << "|0";
		}

		EQApplicationPacket* outapp = new EQApplicationPacket(OP_AltCurrencyMerchantReply, ss.str().length() + 1);
		memcpy(outapp->pBuffer, ss.str().c_str(), ss.str().length());
		FastQueuePacket(&outapp);
	}
}

void Client::Handle_OP_AltCurrencyPurchase(const EQApplicationPacket *app)
{
	VERIFY_PACKET_LENGTH(OP_AltCurrencyPurchase, app, AltCurrencyPurchaseItem_Struct);
	AltCurrencyPurchaseItem_Struct *purchase = (AltCurrencyPurchaseItem_Struct*)app->pBuffer;
	NPC* tar = entity_list.GetNPCByID(purchase->merchant_entity_id);
	if (tar) {
		if (DistanceSquared(m_Position, tar->GetPosition())> USE_NPC_RANGE2)
			return;

		if (tar->GetClass() != ALT_CURRENCY_MERCHANT) {
			return;
		}

		uint32 alt_cur_id = tar->GetAltCurrencyType();
		if (alt_cur_id == 0) {
			return;
		}

		const EQEmu::ItemData* item = nullptr;
		uint32 cost = 0;
		uint32 current_currency = GetAlternateCurrencyValue(alt_cur_id);
		uint32 merchant_id = tar->MerchantType;
		bool found = false;
		std::list<MerchantList> merlist = zone->merchanttable[merchant_id];
		std::list<MerchantList>::const_iterator itr;
		for (itr = merlist.begin(); itr != merlist.end(); ++itr) {
			MerchantList ml = *itr;
			if (GetLevel() < ml.level_required) {
				continue;
			}

			int32 fac = tar->GetPrimaryFaction();
			if (fac != 0 && GetModCharacterFactionLevel(fac) < ml.faction_required) {
				continue;
			}

			item = database.GetItem(ml.item);
			if (!item)
				continue;

			if (item->ID == purchase->item_id) { //This check to make sure that the item is actually on the NPC, people attempt to inject packets to get items summoned...
				cost = ml.alt_currency_cost;
				found = true;
				break;
			}
		}

		if (!item || !found) {
			Message(13, "Error: The item you purchased does not exist!");
			return;
		}

		if (cost > current_currency) {
			Message(13, "You cannot afford that item right now.");
			return;
		}

		if (CheckLoreConflict(item))
		{
			Message(15, "You can only have one of a lore item.");
			return;
		}

		/* QS: PlayerLogAlternateCurrencyTransactions :: Merchant Purchase */
		if (RuleB(QueryServ, PlayerLogAlternateCurrencyTransactions)) {
			std::string event_desc = StringFormat("Merchant Purchase :: Spent alt_currency_id:%i cost:%i for itemid:%i in zoneid:%i instid:%i", alt_cur_id, cost, item->ID, this->GetZoneID(), this->GetInstanceID());
			QServ->PlayerLogEvent(Player_Log_Alternate_Currency_Transactions, this->CharacterID(), event_desc);
		}

		AddAlternateCurrencyValue(alt_cur_id, -((int32)cost));
		int16 charges = 1;
		if (item->MaxCharges != 0)
			charges = item->MaxCharges;

		EQEmu::ItemInstance *inst = database.CreateItem(item, charges);
		if (!AutoPutLootInInventory(*inst, true, true))
		{
			PutLootInInventory(EQEmu::inventory::slotCursor, *inst);
		}

		Save(1);
	}
}

void Client::Handle_OP_AltCurrencyReclaim(const EQApplicationPacket *app)
{
	VERIFY_PACKET_LENGTH(OP_AltCurrencyReclaim, app, AltCurrencyReclaim_Struct);
	AltCurrencyReclaim_Struct *reclaim = (AltCurrencyReclaim_Struct*)app->pBuffer;
	uint32 item_id = 0;
	auto iter = zone->AlternateCurrencies.begin();
	while (iter != zone->AlternateCurrencies.end()) {
		if ((*iter).id == reclaim->currency_id) {
			item_id = (*iter).item_id;
		}
		++iter;
	}

	if (item_id == 0) {
		return;
	}

	/* Item to Currency Storage */
	if (reclaim->reclaim_flag == 1) {
		uint32 removed = NukeItem(item_id, invWhereWorn | invWherePersonal | invWhereCursor);
		if (removed > 0) {
			AddAlternateCurrencyValue(reclaim->currency_id, removed);

			/* QS: PlayerLogAlternateCurrencyTransactions :: Item to Currency */
			if (RuleB(QueryServ, PlayerLogAlternateCurrencyTransactions)) {
				std::string event_desc = StringFormat("Reclaim :: Item to Currency :: alt_currency_id:%i amount:%i to currency tab in zoneid:%i instid:%i", reclaim->currency_id, removed, this->GetZoneID(), this->GetInstanceID());
				QServ->PlayerLogEvent(Player_Log_Alternate_Currency_Transactions, this->CharacterID(), event_desc);
			}
		}
	}
	/* Cursor to Item storage */
	else {
		uint32 max_currency = GetAlternateCurrencyValue(reclaim->currency_id);

		if (max_currency == 0 || reclaim->count == 0)
			return;

		/* If you input more than you have currency wise, just give the max of the currency you currently have */
		if (reclaim->count > max_currency) {
			SummonItem(item_id, max_currency);
			SetAlternateCurrencyValue(reclaim->currency_id, 0);
		}
		else {
			SummonItem(item_id, reclaim->count, 0, 0, 0, 0, 0, 0, false, EQEmu::inventory::slotCursor);
			AddAlternateCurrencyValue(reclaim->currency_id, -((int32)reclaim->count));
		}
		/* QS: PlayerLogAlternateCurrencyTransactions :: Cursor to Item Storage */
		if (RuleB(QueryServ, PlayerLogAlternateCurrencyTransactions)) {
			std::string event_desc = StringFormat("Reclaim :: Cursor to Item :: alt_currency_id:%i amount:-%i in zoneid:%i instid:%i", reclaim->currency_id, reclaim->count, this->GetZoneID(), this->GetInstanceID());
			QServ->PlayerLogEvent(Player_Log_Alternate_Currency_Transactions, this->CharacterID(), event_desc);
		}
	}
}

void Client::Handle_OP_AltCurrencySell(const EQApplicationPacket *app)
{
	VERIFY_PACKET_LENGTH(OP_AltCurrencySell, app, AltCurrencySellItem_Struct);
	EQApplicationPacket *outapp = app->Copy();
	AltCurrencySellItem_Struct *sell = (AltCurrencySellItem_Struct*)outapp->pBuffer;

	NPC* tar = entity_list.GetNPCByID(sell->merchant_entity_id);
	if (tar) {
		if (DistanceSquared(m_Position, tar->GetPosition()) > USE_NPC_RANGE2)
			return;

		if (tar->GetClass() != ALT_CURRENCY_MERCHANT) {
			return;
		}

		uint32 alt_cur_id = tar->GetAltCurrencyType();
		if (alt_cur_id == 0) {
			return;
		}

		EQEmu::ItemInstance* inst = GetInv().GetItem(sell->slot_id);
		if (!inst) {
			return;
		}

		if (!RuleB(Merchant, EnableAltCurrencySell)) {
			return;
		}

		const EQEmu::ItemData* item = nullptr;
		uint32 cost = 0;
		uint32 current_currency = GetAlternateCurrencyValue(alt_cur_id);
		uint32 merchant_id = tar->MerchantType;
		uint32 npc_id = tar->GetNPCTypeID();
		bool found = false;
		std::list<MerchantList> merlist = zone->merchanttable[merchant_id];
		std::list<MerchantList>::const_iterator itr;
		for (itr = merlist.begin(); itr != merlist.end(); ++itr) {
			MerchantList ml = *itr;
			if (GetLevel() < ml.level_required) {
				continue;
			}

			int32 fac = tar->GetPrimaryFaction();
			if (fac != 0 && GetModCharacterFactionLevel(fac) < ml.faction_required) {
				continue;
			}

			item = database.GetItem(ml.item);
			if (!item)
				continue;

			if (item->ID == inst->GetItem()->ID) {
				cost = ml.alt_currency_cost;
				found = true;
				break;
			}
		}

		if (!found) {
			return;
		}

		if (!inst->IsStackable())
		{
			DeleteItemInInventory(sell->slot_id, 0, false);
		}
		else
		{
			if (inst->GetCharges() < sell->charges)
			{
				sell->charges = inst->GetCharges();
			}

			if (sell->charges == 0)
			{
				Message(13, "Charge mismatch error.");
				return;
			}

			DeleteItemInInventory(sell->slot_id, sell->charges, false);
			cost *= sell->charges;
		}

		sell->cost = cost;

		/* QS: PlayerLogAlternateCurrencyTransactions :: Sold to Merchant*/
		if (RuleB(QueryServ, PlayerLogAlternateCurrencyTransactions)) {
			std::string event_desc = StringFormat("Sold to Merchant :: itemid:%u npcid:%u alt_currency_id:%u cost:%u in zoneid:%u instid:%i", item->ID, npc_id, alt_cur_id, cost, this->GetZoneID(), this->GetInstanceID());
			QServ->PlayerLogEvent(Player_Log_Alternate_Currency_Transactions, this->CharacterID(), event_desc);
		}

		FastQueuePacket(&outapp);
		AddAlternateCurrencyValue(alt_cur_id, cost);
		Save(1);
	}
}

void Client::Handle_OP_AltCurrencySellSelection(const EQApplicationPacket *app)
{
	VERIFY_PACKET_LENGTH(OP_AltCurrencySellSelection, app, AltCurrencySelectItem_Struct);

	AltCurrencySelectItem_Struct *select = (AltCurrencySelectItem_Struct*)app->pBuffer;
	NPC* tar = entity_list.GetNPCByID(select->merchant_entity_id);
	if (tar) {
		if (DistanceSquared(m_Position, tar->GetPosition()) > USE_NPC_RANGE2)
			return;

		if (tar->GetClass() != ALT_CURRENCY_MERCHANT) {
			return;
		}

		uint32 alt_cur_id = tar->GetAltCurrencyType();
		if (alt_cur_id == 0) {
			return;
		}

		EQEmu::ItemInstance *inst = m_inv.GetItem(select->slot_id);
		if (!inst) {
			return;
		}

		const EQEmu::ItemData* item = nullptr;
		uint32 cost = 0;
		uint32 current_currency = GetAlternateCurrencyValue(alt_cur_id);
		uint32 merchant_id = tar->MerchantType;

		if (RuleB(Merchant, EnableAltCurrencySell)) {
			bool found = false;
			std::list<MerchantList> merlist = zone->merchanttable[merchant_id];
			std::list<MerchantList>::const_iterator itr;
			for (itr = merlist.begin(); itr != merlist.end(); ++itr) {
				MerchantList ml = *itr;
				if (GetLevel() < ml.level_required) {
					continue;
				}

				int32 fac = tar->GetPrimaryFaction();
				if (fac != 0 && GetModCharacterFactionLevel(fac) < ml.faction_required) {
					continue;
				}

				item = database.GetItem(ml.item);
				if (!item)
					continue;

				if (item->ID == inst->GetItem()->ID) {
					cost = ml.alt_currency_cost;
					found = true;
					break;
				}
			}

			if (!found) {
				cost = 0;
			}
		}
		else {
			cost = 0;
		}

		auto outapp =
			new EQApplicationPacket(OP_AltCurrencySellSelection, sizeof(AltCurrencySelectItemReply_Struct));
		AltCurrencySelectItemReply_Struct *reply = (AltCurrencySelectItemReply_Struct*)outapp->pBuffer;
		reply->unknown004 = 0xFF;
		reply->unknown005 = 0xFF;
		reply->unknown006 = 0xFF;
		reply->unknown007 = 0xFF;
		strcpy(reply->item_name, inst->GetItem()->Name);
		reply->cost = cost;
		FastQueuePacket(&outapp);
	}
}

void Client::Handle_OP_Animation(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Animation_Struct)) {
		Log(Logs::General, Logs::Error, "Received invalid sized "
			"OP_Animation: got %d, expected %d", app->size,
			sizeof(Animation_Struct));
		DumpPacket(app);
		return;
	}

	Animation_Struct *s = (Animation_Struct *)app->pBuffer;

	//might verify spawn ID, but it wouldent affect anything
	DoAnim(s->action, s->speed);

	return;
}

void Client::Handle_OP_ApplyPoison(const EQApplicationPacket *app)
{
	if (app->size != sizeof(ApplyPoison_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_ApplyPoison, size=%i, expected %i", app->size, sizeof(ApplyPoison_Struct));
		DumpPacket(app);
		return;
	}
	uint32 ApplyPoisonSuccessResult = 0;
	ApplyPoison_Struct* ApplyPoisonData = (ApplyPoison_Struct*)app->pBuffer;
	const EQEmu::ItemInstance* PrimaryWeapon = GetInv().GetItem(EQEmu::inventory::slotPrimary);
	const EQEmu::ItemInstance* SecondaryWeapon = GetInv().GetItem(EQEmu::inventory::slotSecondary);
	const EQEmu::ItemInstance* PoisonItemInstance = GetInv()[ApplyPoisonData->inventorySlot];

	bool IsPoison = PoisonItemInstance && (PoisonItemInstance->GetItem()->ItemType == EQEmu::item::ItemTypePoison);

	if (!IsPoison)
	{
		Log(Logs::Detail, Logs::Spells, "Item used to cast spell effect from a poison item was missing from inventory slot %d "
			"after casting, or is not a poison!", ApplyPoisonData->inventorySlot);

		Message(0, "Error: item not found for inventory slot #%i or is not a poison", ApplyPoisonData->inventorySlot);
	}
	else if (GetClass() == ROGUE)
	{
		if ((PrimaryWeapon && PrimaryWeapon->GetItem()->ItemType == EQEmu::item::ItemType1HPiercing) ||
			(SecondaryWeapon && SecondaryWeapon->GetItem()->ItemType == EQEmu::item::ItemType1HPiercing)) {
			float SuccessChance = (GetSkill(EQEmu::skills::SkillApplyPoison) + GetLevel()) / 400.0f;
			double ChanceRoll = zone->random.Real(0, 1);

			CheckIncreaseSkill(EQEmu::skills::SkillApplyPoison, nullptr, 10);

			if (ChanceRoll < SuccessChance) {
				ApplyPoisonSuccessResult = 1;
				// NOTE: Someone may want to tweak the chance to proc the poison effect that is added to the weapon here.
				// My thinking was that DEX should be apart of the calculation.
				AddProcToWeapon(PoisonItemInstance->GetItem()->Proc.Effect, false, (GetDEX() / 100) + 103);
			}

			DeleteItemInInventory(ApplyPoisonData->inventorySlot, 1, true);

			Log(Logs::General, Logs::None, "Chance to Apply Poison was %f. Roll was %f. Result is %u.", SuccessChance, ChanceRoll, ApplyPoisonSuccessResult);
		}
	}

	auto outapp = new EQApplicationPacket(OP_ApplyPoison, nullptr, sizeof(ApplyPoison_Struct));
	ApplyPoison_Struct* ApplyPoisonResult = (ApplyPoison_Struct*)outapp->pBuffer;
	ApplyPoisonResult->success = ApplyPoisonSuccessResult;
	ApplyPoisonResult->inventorySlot = ApplyPoisonData->inventorySlot;

	FastQueuePacket(&outapp);
}

void Client::Handle_OP_Assist(const EQApplicationPacket *app)
{
	if (app->size != sizeof(EntityId_Struct)) {
		Log(Logs::General, Logs::None, "Size mismatch in OP_Assist expected %i got %i", sizeof(EntityId_Struct), app->size);
		return;
	}

	EntityId_Struct* eid = (EntityId_Struct*)app->pBuffer;
	Entity* entity = entity_list.GetID(eid->entity_id);

	EQApplicationPacket* outapp = app->Copy();
	eid = (EntityId_Struct*)outapp->pBuffer;
	if (RuleB(Combat, AssistNoTargetSelf))
		eid->entity_id = GetID();
	if (entity && entity->IsMob()) {
		Mob *assistee = entity->CastToMob();
		if (assistee->GetTarget()) {
			Mob *new_target = assistee->GetTarget();
			if (new_target && (GetGM() ||
				Distance(m_Position, assistee->GetPosition()) <= TARGETING_RANGE)) {
				SetAssistExemption(true);
				eid->entity_id = new_target->GetID();
			}
		}
	}

	FastQueuePacket(&outapp);
	return;
}

void Client::Handle_OP_AssistGroup(const EQApplicationPacket *app)
{
	if (app->size != sizeof(EntityId_Struct)) {
		Log(Logs::General, Logs::None, "Size mismatch in OP_AssistGroup expected %i got %i", sizeof(EntityId_Struct), app->size);
		return;
	}
	QueuePacket(app);
	return;
}

void Client::Handle_OP_AugmentInfo(const EQApplicationPacket *app)
{
	// This packet is sent by the client when an Augment item information window is opened.
	// Some clients this seems to nuke the charm text (ex. Adventurer's Stone)

	if (app->size != sizeof(AugmentInfo_Struct)) {
		Log(Logs::General, Logs::None, "Size mismatch in OP_AugmentInfo expected %i got %i",
			sizeof(AugmentInfo_Struct), app->size);
		DumpPacket(app);
		return;
	}

	AugmentInfo_Struct* AugInfo = (AugmentInfo_Struct*)app->pBuffer;
	const EQEmu::ItemData * item = database.GetItem(AugInfo->itemid);

	if (item) {
		strn0cpy(AugInfo->augment_info, item->Name, 64);
		AugInfo->itemid = 0;
		QueuePacket(app);
	}
}

void Client::Handle_OP_AugmentItem(const EQApplicationPacket *app)
{
	if (app->size != sizeof(AugmentItem_Struct)) {
		Log(Logs::General, Logs::Error, "Invalid size for AugmentItem_Struct: Expected: %i, Got: %i",
			sizeof(AugmentItem_Struct), app->size);
		return;
	}

	AugmentItem_Struct* in_augment = (AugmentItem_Struct*)app->pBuffer;
	bool deleteItems = false;
	if (ClientVersion() >= EQEmu::versions::ClientVersion::RoF)
	{
		if ((in_augment->container_slot < 0 || in_augment->container_slot >= EQEmu::legacy::SLOT_CURSOR) &&
			in_augment->container_slot != EQEmu::legacy::SLOT_POWER_SOURCE &&
			(in_augment->container_slot < EQEmu::legacy::SLOT_PERSONAL_BAGS_BEGIN || in_augment->container_slot > EQEmu::legacy::SLOT_PERSONAL_BAGS_END))
		{
			Message(13, "The server does not allow augmentation actions from this slot.");
			auto cursor_item = m_inv[EQEmu::legacy::SLOT_CURSOR];
			auto augmented_item = m_inv[in_augment->container_slot];
			SendItemPacket(EQEmu::legacy::SLOT_CURSOR, cursor_item, ItemPacketCharInventory);
			// this may crash clients on certain slots
			SendItemPacket(in_augment->container_slot, augmented_item, ItemPacketCharInventory);
			return;
		}

		EQEmu::ItemInstance *itemOneToPush = nullptr, *itemTwoToPush = nullptr;

		//Log(Logs::DebugLevel::Moderate, Logs::Debug, "cslot: %i aslot: %i cidx: %i aidx: %i act: %i dest: %i",
		//	in_augment->container_slot, in_augment->augment_slot, in_augment->container_index, in_augment->augment_index, in_augment->augment_action, in_augment->dest_inst_id);

		EQEmu::ItemInstance *tobe_auged = nullptr, *old_aug = nullptr, *new_aug = nullptr, *aug = nullptr, *solvent = nullptr;
		EQEmu::InventoryProfile& user_inv = GetInv();

		uint16 item_slot = in_augment->container_slot;
		uint16 solvent_slot = in_augment->augment_slot;
		uint8 mat = EQEmu::InventoryProfile::CalcMaterialFromSlot(item_slot); // for when player is augging a piece of equipment while they're wearing it

		if (item_slot == INVALID_INDEX || solvent_slot == INVALID_INDEX)
		{
			Message(13, "Error: Invalid Aug Index.");
			return;
		}

		tobe_auged = user_inv.GetItem(item_slot);
		solvent = user_inv.GetItem(solvent_slot);

		if (!tobe_auged)
		{
			Message(13, "Error: Invalid item passed for augmenting.");
			return;
		}

		if ((in_augment->augment_action == 1) || (in_augment->augment_action == 2))
		{
			// Check for valid distiller if safely removing / swapping an augmentation

			if (!solvent)
			{
				old_aug = tobe_auged->GetAugment(in_augment->augment_index);
				if (!old_aug || old_aug->GetItem()->AugDistiller != 0) {
					Log(Logs::General, Logs::Error, "Player tried to safely remove an augment without a distiller.");
					Message(13, "Error: Missing an augmentation distiller for safely removing this augment.");
					return;
				}
			}
			else if (solvent->GetItem()->ItemType == EQEmu::item::ItemTypeAugmentationDistiller)
			{
				old_aug = tobe_auged->GetAugment(in_augment->augment_index);

				if (!old_aug)
				{
					Log(Logs::General, Logs::Error, "Player tried to safely remove a nonexistent augment.");
					Message(13, "Error: No augment found in slot %i for safely removing.", in_augment->augment_index);
					return;
				}
				else if (solvent->GetItem()->ID != old_aug->GetItem()->AugDistiller)
				{
					Log(Logs::General, Logs::Error, "Player tried to safely remove an augment with the wrong distiller (item %u vs expected %u).", solvent->GetItem()->ID, old_aug->GetItem()->AugDistiller);
					Message(13, "Error: Wrong augmentation distiller for safely removing this augment.");
					return;
				}
			}
			else if (solvent->GetItem()->ItemType != EQEmu::item::ItemTypePerfectedAugmentationDistiller)
			{
				Log(Logs::General, Logs::Error, "Player tried to safely remove an augment with a non-distiller item.");
				Message(13, "Error: Invalid augmentation distiller for safely removing this augment.");
				return;
			}
		}

		switch (in_augment->augment_action)
		{
		case 0: // Adding an augment
		case 2: // Swapping augment
			new_aug = user_inv.GetItem(EQEmu::inventory::slotCursor);

			if (!new_aug) // Shouldn't get the OP code without the augment on the user's cursor, but maybe it's h4x.
			{
				Log(Logs::General, Logs::Error, "AugmentItem OpCode with 'Insert' or 'Swap' action received, but no augment on client's cursor.");
				Message(13, "Error: No augment found on cursor for inserting.");
				return;
			}
			else
			{
				if (((tobe_auged->IsAugmentSlotAvailable(new_aug->GetAugmentType(), in_augment->augment_index)) != -1) &&
					(tobe_auged->AvailableWearSlot(new_aug->GetItem()->Slots)))
				{
					old_aug = tobe_auged->RemoveAugment(in_augment->augment_index);
					if (old_aug)
					{
						// An old augment was removed in order to be replaced with the new one (augment_action 2)

						CalcBonuses();

						std::vector<EQEmu::Any> args;
						args.push_back(old_aug);
						parse->EventItem(EVENT_UNAUGMENT_ITEM, this, tobe_auged, nullptr, "", in_augment->augment_index, &args);

						args.assign(1, tobe_auged);
						args.push_back(false);
						parse->EventItem(EVENT_AUGMENT_REMOVE, this, old_aug, nullptr, "", in_augment->augment_index, &args);
					}

					tobe_auged->PutAugment(in_augment->augment_index, *new_aug);
					tobe_auged->UpdateOrnamentationInfo();

					aug = tobe_auged->GetAugment(in_augment->augment_index);
					if (aug)
					{
						std::vector<EQEmu::Any> args;
						args.push_back(aug);
						parse->EventItem(EVENT_AUGMENT_ITEM, this, tobe_auged, nullptr, "", in_augment->augment_index, &args);

						args.assign(1, tobe_auged);
						parse->EventItem(EVENT_AUGMENT_INSERT, this, aug, nullptr, "", in_augment->augment_index, &args);
					}
					else
					{
						Message(13, "Error: Could not properly insert augmentation into augment slot %i. Aborting.", in_augment->augment_index);
						return;
					}

					itemOneToPush = tobe_auged->Clone();
					if (old_aug)
					{
						itemTwoToPush = old_aug->Clone();
					}

					// Must push items after the items in inventory are deleted - necessary due to lore items...
					if (itemOneToPush)
					{
						DeleteItemInInventory(item_slot, 0, true);
						DeleteItemInInventory(EQEmu::inventory::slotCursor, new_aug->IsStackable() ? 1 : 0, true);

						if (solvent)
						{
							// Consume the augment distiller
							DeleteItemInInventory(solvent_slot, solvent->IsStackable() ? 1 : 0, true);
						}

						if (itemTwoToPush)
						{
							// This is a swap. Return the old aug to the player's cursor.
							if (!PutItemInInventory(EQEmu::inventory::slotCursor, *itemTwoToPush, true))
							{
								Log(Logs::General, Logs::Error, "Problem returning old augment to player's cursor after augmentation swap.");
								Message(15, "Error: Failed to retrieve old augment after augmentation swap!");
							}
						}

						if (PutItemInInventory(item_slot, *itemOneToPush, true))
						{
							// Successfully added an augment to the item

							CalcBonuses();

							if (mat != EQEmu::textures::materialInvalid)
							{
								SendWearChange(mat); // Visible item augged while equipped. Send WC in case ornamentation changed.
							}
						}
						else
						{
							Message(13, "Error: No available slot for end result. Please free up the augment slot.");
						}
					}
					else
					{
						Message(13, "Error in cloning item for augment. Aborted.");
					}
				}
				else
				{
					Message(13, "Error: No available slot for augment in that item.");
				}
			}
			break;
		case 1: // Removing augment safely (distiller)
			aug = tobe_auged->GetAugment(in_augment->augment_index);
			if (aug)
			{
				std::vector<EQEmu::Any> args;
				args.push_back(aug);
				parse->EventItem(EVENT_UNAUGMENT_ITEM, this, tobe_auged, nullptr, "", in_augment->augment_index, &args);

				args.assign(1, tobe_auged);

				args.push_back(false);

				parse->EventItem(EVENT_AUGMENT_REMOVE, this, aug, nullptr, "", in_augment->augment_index, &args);
			}
			else
			{
				Message(13, "Error: Could not find augmentation to remove at index %i. Aborting.", in_augment->augment_index);
				return;
			}

			old_aug = tobe_auged->RemoveAugment(in_augment->augment_index);
			tobe_auged->UpdateOrnamentationInfo();

			itemOneToPush = tobe_auged->Clone();
			if (old_aug)
				itemTwoToPush = old_aug->Clone();

			if (itemOneToPush && itemTwoToPush)
			{
				// Consume the augment distiller
				if (solvent)
					DeleteItemInInventory(solvent_slot, solvent->IsStackable() ? 1 : 0, true);

				// Remove the augmented item
				DeleteItemInInventory(item_slot, 0, true);

				// Replace it with the unaugmented item
				if (!PutItemInInventory(item_slot, *itemOneToPush, true))
				{
					Log(Logs::General, Logs::Error, "Problem returning equipment item to player's inventory after safe augment removal.");
					Message(15, "Error: Failed to return item after de-augmentation!");
				}

				CalcBonuses();

				if (mat != EQEmu::textures::materialInvalid)
				{
					SendWearChange(mat); // Visible item augged while equipped. Send WC in case ornamentation changed.
				}

				// Drop the removed augment on the player's cursor
				if (!PutItemInInventory(EQEmu::inventory::slotCursor, *itemTwoToPush, true))
				{
					Log(Logs::General, Logs::Error, "Problem returning augment to player's cursor after safe removal.");
					Message(15, "Error: Failed to return augment after removal from item!");
					return;
				}
			}
			break;
		case 3: // Destroying augment (formerly done in birdbath/sealer with a solvent)

				// RoF client does not require an augmentation solvent for destroying an augmentation in an item.
				// Augments can be destroyed with a right click -> Destroy at any time.

			aug = tobe_auged->GetAugment(in_augment->augment_index);
			if (aug)
			{
				std::vector<EQEmu::Any> args;
				args.push_back(aug);
				parse->EventItem(EVENT_UNAUGMENT_ITEM, this, tobe_auged, nullptr, "", in_augment->augment_index, &args);

				args.assign(1, tobe_auged);

				args.push_back(true);

				parse->EventItem(EVENT_AUGMENT_REMOVE, this, aug, nullptr, "", in_augment->augment_index, &args);
			}
			else
			{
				Message(13, "Error: Could not find augmentation to remove at index %i. Aborting.");
				return;
			}

			tobe_auged->DeleteAugment(in_augment->augment_index);
			tobe_auged->UpdateOrnamentationInfo();

			itemOneToPush = tobe_auged->Clone();
			if (itemOneToPush)
			{
				DeleteItemInInventory(item_slot, 0, true);

				if (!PutItemInInventory(item_slot, *itemOneToPush, true))
				{
					Log(Logs::General, Logs::Error, "Problem returning equipment item to player's inventory after augment deletion.");
					Message(15, "Error: Failed to return item after destroying augment!");
				}
			}

			CalcBonuses();

			if (mat != EQEmu::textures::materialInvalid)
			{
				SendWearChange(mat);
			}
			break;
		default: // Unknown
			Log(Logs::General, Logs::Inventory, "Unrecognized augmentation action - cslot: %i aslot: %i cidx: %i aidx: %i act: %i dest: %i",
				in_augment->container_slot, in_augment->augment_slot, in_augment->container_index, in_augment->augment_index, in_augment->augment_action, in_augment->dest_inst_id);
			break;
		}
	}
	else
	{
		// Delegate to tradeskill object to perform combine
		Object::HandleAugmentation(this, in_augment, m_tradeskill_object);
	}
	return;
}

void Client::Handle_OP_AutoAttack(const EQApplicationPacket *app)
{
	if (app->size != 4) {
		Log(Logs::General, Logs::Error, "OP size error: OP_AutoAttack expected:4 got:%i", app->size);
		return;
	}

	if (app->pBuffer[0] == 0)
	{
		auto_attack = false;
		if (IsAIControlled())
			return;
		attack_timer.Disable();
		ranged_timer.Disable();
		attack_dw_timer.Disable();

		m_AutoAttackPosition = glm::vec4();
		m_AutoAttackTargetLocation = glm::vec3();
		aa_los_them_mob = nullptr;
	}
	else if (app->pBuffer[0] == 1)
	{
		auto_attack = true;
		auto_fire = false;
		if (IsAIControlled())
			return;
		SetAttackTimer();

		if (GetTarget())
		{
			aa_los_them_mob = GetTarget();
			m_AutoAttackPosition = GetPosition();
			m_AutoAttackTargetLocation = glm::vec3(aa_los_them_mob->GetPosition());
			los_status = CheckLosFN(aa_los_them_mob);
			los_status_facing = IsFacingMob(aa_los_them_mob);
		}
		else
		{
			m_AutoAttackPosition = GetPosition();
			m_AutoAttackTargetLocation = glm::vec3();
			aa_los_them_mob = nullptr;
			los_status = false;
			los_status_facing = false;
		}
	}
}

void Client::Handle_OP_AutoAttack2(const EQApplicationPacket *app)
{
	return;
}

void Client::Handle_OP_AutoFire(const EQApplicationPacket *app)
{
	if (app->size != sizeof(bool)) {
		Log(Logs::General, Logs::None, "Size mismatch in OP_AutoFire expected %i got %i", sizeof(bool), app->size);
		DumpPacket(app);
		return;
	}
	bool *af = (bool*)app->pBuffer;
	auto_fire = *af;
	auto_attack = false;
	SetAttackTimer();
}

void Client::Handle_OP_Bandolier(const EQApplicationPacket *app)
{
	// Although there are three different structs for OP_Bandolier, they are all the same size.
	//
	if (app->size != sizeof(BandolierCreate_Struct)) {
		Log(Logs::General, Logs::None, "Size mismatch in OP_Bandolier expected %i got %i",
			sizeof(BandolierCreate_Struct), app->size);
		DumpPacket(app);
		return;
	}

	BandolierCreate_Struct *bs = (BandolierCreate_Struct*)app->pBuffer;

	switch (bs->Action)
	{
	case bandolierCreate:
		CreateBandolier(app);
		break;
	case bandolierRemove:
		RemoveBandolier(app);
		break;
	case bandolierSet:
		SetBandolier(app);
		break;
	default:
		Log(Logs::General, Logs::None, "Unknown Bandolier action %i", bs->Action);
		break;
	}
}

void Client::Handle_OP_BankerChange(const EQApplicationPacket *app)
{
	if (app->size != sizeof(BankerChange_Struct) && app->size != 4) //Titanium only sends 4 Bytes for this
	{
		Log(Logs::General, Logs::None, "Size mismatch in OP_BankerChange expected %i got %i", sizeof(BankerChange_Struct), app->size);
		DumpPacket(app);
		return;
	}

	uint32 distance = 0;
	NPC *banker = entity_list.GetClosestBanker(this, distance);

	if (!banker || distance > USE_NPC_RANGE2)
	{
		char *hacked_string = nullptr;
		MakeAnyLenString(&hacked_string, "Player tried to make use of a banker(money) but %s is non-existant or too far away (%u units).",
			banker ? banker->GetName() : "UNKNOWN NPC", distance);
		database.SetMQDetectionFlag(AccountName(), GetName(), hacked_string, zone->GetShortName());
		safe_delete_array(hacked_string);
		return;
	}

	auto outapp = new EQApplicationPacket(OP_BankerChange, nullptr, sizeof(BankerChange_Struct));
	BankerChange_Struct *bc = (BankerChange_Struct *)outapp->pBuffer;

	if (m_pp.platinum < 0)
		m_pp.platinum = 0;
	if (m_pp.gold < 0)
		m_pp.gold = 0;
	if (m_pp.silver < 0)
		m_pp.silver = 0;
	if (m_pp.copper < 0)
		m_pp.copper = 0;

	if (m_pp.platinum_bank < 0)
		m_pp.platinum_bank = 0;
	if (m_pp.gold_bank < 0)
		m_pp.gold_bank = 0;
	if (m_pp.silver_bank < 0)
		m_pp.silver_bank = 0;
	if (m_pp.copper_bank < 0)
		m_pp.copper_bank = 0;

	uint64 cp = static_cast<uint64>(m_pp.copper) +
		(static_cast<uint64>(m_pp.silver) * 10) +
		(static_cast<uint64>(m_pp.gold) * 100) +
		(static_cast<uint64>(m_pp.platinum) * 1000);

	m_pp.copper = cp % 10;
	cp /= 10;
	m_pp.silver = cp % 10;
	cp /= 10;
	m_pp.gold = cp % 10;
	cp /= 10;
	m_pp.platinum = cp;

	cp = static_cast<uint64>(m_pp.copper_bank) +
		(static_cast<uint64>(m_pp.silver_bank) * 10) +
		(static_cast<uint64>(m_pp.gold_bank) * 100) +
		(static_cast<uint64>(m_pp.platinum_bank) * 1000);

	m_pp.copper_bank = cp % 10;
	cp /= 10;
	m_pp.silver_bank = cp % 10;
	cp /= 10;
	m_pp.gold_bank = cp % 10;
	cp /= 10;
	m_pp.platinum_bank = cp;

	bc->copper = m_pp.copper;
	bc->silver = m_pp.silver;
	bc->gold = m_pp.gold;
	bc->platinum = m_pp.platinum;

	bc->copper_bank = m_pp.copper_bank;
	bc->silver_bank = m_pp.silver_bank;
	bc->gold_bank = m_pp.gold_bank;
	bc->platinum_bank = m_pp.platinum_bank;

	FastQueuePacket(&outapp);

	return;
}

void Client::Handle_OP_Barter(const EQApplicationPacket *app)
{

	if (app->size < 4)
	{
		Log(Logs::General, Logs::None, "OP_Barter packet below minimum expected size. The packet was %i bytes.", app->size);
		DumpPacket(app);
		return;
	}

	char* Buf = (char *)app->pBuffer;

	// The first 4 bytes of the packet determine the action. A lot of Barter packets require the
	// packet the client sent, sent back to it as an acknowledgement.
	//
	uint32 Action = VARSTRUCT_DECODE_TYPE(uint32, Buf);


	switch (Action)
	{

	case Barter_BuyerSearch:
	{
		BuyerItemSearch(app);
		break;
	}

	case Barter_SellerSearch:
	{
		BarterSearchRequest_Struct *bsr = (BarterSearchRequest_Struct*)app->pBuffer;
		SendBuyerResults(bsr->SearchString, bsr->SearchID);
		break;
	}

	case Barter_BuyerModeOn:
	{
		if (!Trader) {
			ToggleBuyerMode(true);
		}
		else {
			Buf = (char *)app->pBuffer;
			VARSTRUCT_ENCODE_TYPE(uint32, Buf, Barter_BuyerModeOff);
			Message(13, "You cannot be a Trader and Buyer at the same time.");
		}
		QueuePacket(app);
		break;
	}

	case Barter_BuyerModeOff:
	{
		QueuePacket(app);
		ToggleBuyerMode(false);
		break;
	}

	case Barter_BuyerItemUpdate:
	{
		UpdateBuyLine(app);
		break;
	}

	case Barter_BuyerItemRemove:
	{
		BuyerRemoveItem_Struct* bris = (BuyerRemoveItem_Struct*)app->pBuffer;
		database.RemoveBuyLine(CharacterID(), bris->BuySlot);
		QueuePacket(app);
		break;
	}

	case Barter_SellItem:
	{
		SellToBuyer(app);
		break;
	}

	case Barter_BuyerInspectBegin:
	{
		ShowBuyLines(app);
		break;
	}

	case Barter_BuyerInspectEnd:
	{
		BuyerInspectRequest_Struct* bir = (BuyerInspectRequest_Struct*)app->pBuffer;
		Client *Buyer = entity_list.GetClientByID(bir->BuyerID);
		if (Buyer)
			Buyer->WithCustomer(0);

		break;
	}

	case Barter_BarterItemInspect:
	{
		BarterItemSearchLinkRequest_Struct* bislr = (BarterItemSearchLinkRequest_Struct*)app->pBuffer;

		const EQEmu::ItemData* item = database.GetItem(bislr->ItemID);

		if (!item)
			Message(13, "Error: This item does not exist!");
		else
		{
			EQEmu::ItemInstance* inst = database.CreateItem(item);
			if (inst)
			{
				SendItemPacket(0, inst, ItemPacketViewLink);
				safe_delete(inst);
			}
		}
		break;
	}

	case Barter_Welcome:
	{
		SendBazaarWelcome();
		break;
	}

	case Barter_WelcomeMessageUpdate:
	{
		BuyerWelcomeMessageUpdate_Struct* bwmu = (BuyerWelcomeMessageUpdate_Struct*)app->pBuffer;
		SetBuyerWelcomeMessage(bwmu->WelcomeMessage);
		break;
	}

	case Barter_BuyerItemInspect:
	{
		BuyerItemSearchLinkRequest_Struct* bislr = (BuyerItemSearchLinkRequest_Struct*)app->pBuffer;

		const EQEmu::ItemData* item = database.GetItem(bislr->ItemID);

		if (!item)
			Message(13, "Error: This item does not exist!");
		else
		{
			EQEmu::ItemInstance* inst = database.CreateItem(item);
			if (inst)
			{
				SendItemPacket(0, inst, ItemPacketViewLink);
				safe_delete(inst);
			}
		}
		break;
	}

	case Barter_Unknown23:
	{
		// Sent by SoD client for no discernible reason.
		break;
	}

	default:
		Message(13, "Unrecognised Barter action.");
		Log(Logs::Detail, Logs::Trading, "Unrecognised Barter Action %i", Action);

	}
}

void Client::Handle_OP_BazaarInspect(const EQApplicationPacket *app)
{
	if (app->size != sizeof(BazaarInspect_Struct)) {
		Log(Logs::General, Logs::Error, "Invalid size for BazaarInspect_Struct: Expected %i, Got %i",
			sizeof(BazaarInspect_Struct), app->size);
		return;
	}

	BazaarInspect_Struct* bis = (BazaarInspect_Struct*)app->pBuffer;

	const EQEmu::ItemData* item = database.GetItem(bis->ItemID);

	if (!item) {
		Message(13, "Error: This item does not exist!");
		return;
	}

	EQEmu::ItemInstance* inst = database.CreateItem(item);

	if (inst) {
		SendItemPacket(0, inst, ItemPacketViewLink);
		safe_delete(inst);
	}

	return;
}

void Client::Handle_OP_BazaarSearch(const EQApplicationPacket *app)
{

	if (app->size == sizeof(BazaarSearch_Struct)) {

		BazaarSearch_Struct* bss = (BazaarSearch_Struct*)app->pBuffer;

		this->SendBazaarResults(bss->TraderID, bss->Class_, bss->Race, bss->ItemStat, bss->Slot, bss->Type,
			bss->Name, bss->MinPrice * 1000, bss->MaxPrice * 1000);
	}
	else if (app->size == sizeof(BazaarWelcome_Struct)) {

		BazaarWelcome_Struct* bws = (BazaarWelcome_Struct*)app->pBuffer;

		if (bws->Beginning.Action == BazaarWelcome)
			SendBazaarWelcome();
	}
	else if (app->size == sizeof(NewBazaarInspect_Struct)) {

		NewBazaarInspect_Struct *nbis = (NewBazaarInspect_Struct*)app->pBuffer;

		Client *c = entity_list.GetClientByName(nbis->Name);
		if (c) {
			EQEmu::ItemInstance* inst = c->FindTraderItemBySerialNumber(nbis->SerialNumber);
			if (inst)
				SendItemPacket(0, inst, ItemPacketViewLink);
		}
		return;
	}
	else {
		Log(Logs::Detail, Logs::Trading, "Malformed BazaarSearch_Struct packe, Action %it received, ignoring...");
		Log(Logs::General, Logs::Error, "Malformed BazaarSearch_Struct packet received, ignoring...\n");
	}

	return;
}

void Client::Handle_OP_Begging(const EQApplicationPacket *app)
{
	if (!p_timers.Expired(&database, pTimerBeggingPickPocket, false))
	{
		Message(13, "Ability recovery time not yet met.");

		auto outapp = new EQApplicationPacket(OP_Begging, sizeof(BeggingResponse_Struct));
		BeggingResponse_Struct *brs = (BeggingResponse_Struct*)outapp->pBuffer;
		brs->Result = 0;
		FastQueuePacket(&outapp);
		return;
	}

	if (!HasSkill(EQEmu::skills::SkillBegging) || !GetTarget())
		return;

	if (GetTarget()->GetClass() == LDON_TREASURE)
		return;

	p_timers.Start(pTimerBeggingPickPocket, 8);

	auto outapp = new EQApplicationPacket(OP_Begging, sizeof(BeggingResponse_Struct));
	BeggingResponse_Struct *brs = (BeggingResponse_Struct*)outapp->pBuffer;

	brs->Result = 0; // Default, Fail.
	if (GetTarget() == this)
	{
		FastQueuePacket(&outapp);
		return;
	}

	int RandomChance = zone->random.Int(0, 100);

	int ChanceToAttack = 0;

	if (GetLevel() > GetTarget()->GetLevel())
		ChanceToAttack = zone->random.Int(0, 15);
	else
		ChanceToAttack = zone->random.Int(((this->GetTarget()->GetLevel() - this->GetLevel()) * 10) - 5, ((this->GetTarget()->GetLevel() - this->GetLevel()) * 10));

	if (ChanceToAttack < 0)
		ChanceToAttack = -ChanceToAttack;

	if (RandomChance < ChanceToAttack)
	{
		GetTarget()->Attack(this);
		QueuePacket(outapp);
		safe_delete(outapp);
		return;
	}

	uint16 CurrentSkill = GetSkill(EQEmu::skills::SkillBegging);

	float ChanceToBeg = ((float)(CurrentSkill / 700.0f) + 0.15f) * 100;

	if (RandomChance < ChanceToBeg)
	{
		brs->Amount = zone->random.Int(1, 10);
		// This needs some work to determine how much money they can beg, based on skill level etc.
		if (CurrentSkill < 50)
		{
			brs->Result = 4;	// Copper
			AddMoneyToPP(brs->Amount, false);
		}
		else
		{
			brs->Result = 3;	// Silver
			AddMoneyToPP(brs->Amount * 10, false);
		}

	}
	QueuePacket(outapp);
	safe_delete(outapp);
	CheckIncreaseSkill(EQEmu::skills::SkillBegging, nullptr, -10);
}

void Client::Handle_OP_Bind_Wound(const EQApplicationPacket *app)
{
	if (app->size != sizeof(BindWound_Struct)) {
		Log(Logs::General, Logs::Error, "Size mismatch for Bind wound packet");
		DumpPacket(app);
	}
	BindWound_Struct* bind_in = (BindWound_Struct*)app->pBuffer;
	Mob* bindmob = entity_list.GetMob(bind_in->to);
	if (!bindmob) {
		Log(Logs::General, Logs::Error, "Bindwound on non-exsistant mob from %s", this->GetName());
	}
	else {
		Log(Logs::General, Logs::None, "BindWound in: to:\'%s\' from=\'%s\'", bindmob->GetName(), GetName());
		BindWound(bindmob, true);
	}
	return;
}

void Client::Handle_OP_BlockedBuffs(const EQApplicationPacket *app)
{
	if (!RuleB(Spells, EnableBlockedBuffs))
		return;

	if (app->size != sizeof(BlockedBuffs_Struct))
	{
		Log(Logs::General, Logs::None, "Size mismatch in OP_BlockedBuffs expected %i got %i",
			sizeof(BlockedBuffs_Struct), app->size);

		DumpPacket(app);

		return;
	}

	std::set<uint32>::iterator Iterator;

	BlockedBuffs_Struct *bbs = (BlockedBuffs_Struct*)app->pBuffer;

	std::set<uint32> *BlockedBuffs = bbs->Pet ? &PetBlockedBuffs : &PlayerBlockedBuffs;

	if (bbs->Initialise == 1)
	{
		BlockedBuffs->clear();

		for (unsigned int i = 0; i < BLOCKED_BUFF_COUNT; ++i)
		{
			if ((IsValidSpell(bbs->SpellID[i])) && IsBeneficialSpell(bbs->SpellID[i]) && !spells[bbs->SpellID[i]].no_block)
			{
				if (BlockedBuffs->find(bbs->SpellID[i]) == BlockedBuffs->end())
					BlockedBuffs->insert(bbs->SpellID[i]);
			}
		}

		auto outapp = new EQApplicationPacket(OP_BlockedBuffs, sizeof(BlockedBuffs_Struct));

		BlockedBuffs_Struct *obbs = (BlockedBuffs_Struct*)outapp->pBuffer;

		for (unsigned int i = 0; i < BLOCKED_BUFF_COUNT; ++i)
			obbs->SpellID[i] = -1;

		obbs->Pet = bbs->Pet;
		obbs->Initialise = 1;
		obbs->Flags = 0x54;
		obbs->Count = BlockedBuffs->size();

		unsigned int Element = 0;

		Iterator = BlockedBuffs->begin();

		while (Iterator != BlockedBuffs->end())
		{
			obbs->SpellID[Element++] = (*Iterator);
			++Iterator;
		}

		FastQueuePacket(&outapp);
		return;
	}

	if ((bbs->Initialise == 0) && (bbs->Count > 0))
	{
		auto outapp = new EQApplicationPacket(OP_BlockedBuffs, sizeof(BlockedBuffs_Struct));

		BlockedBuffs_Struct *obbs = (BlockedBuffs_Struct*)outapp->pBuffer;

		for (unsigned int i = 0; i < BLOCKED_BUFF_COUNT; ++i)
			obbs->SpellID[i] = -1;

		obbs->Pet = bbs->Pet;
		obbs->Initialise = 0;
		obbs->Flags = 0x54;

		for (unsigned int i = 0; i < BLOCKED_BUFF_COUNT; ++i)
		{
			if (!IsValidSpell(bbs->SpellID[i]) || !IsBeneficialSpell(bbs->SpellID[i]) || spells[bbs->SpellID[i]].no_block)
				continue;

			if ((BlockedBuffs->size() < BLOCKED_BUFF_COUNT) && (BlockedBuffs->find(bbs->SpellID[i]) == BlockedBuffs->end()))
				BlockedBuffs->insert(bbs->SpellID[i]);
		}
		obbs->Count = BlockedBuffs->size();

		Iterator = BlockedBuffs->begin();

		unsigned int Element = 0;

		while (Iterator != BlockedBuffs->end())
		{
			obbs->SpellID[Element++] = (*Iterator);
			++Iterator;
		}

		FastQueuePacket(&outapp);
	}
}

void Client::Handle_OP_BoardBoat(const EQApplicationPacket *app)
{
	// this sends unclean mob name, so capped at 64
	// a_boat006
	if (app->size <= 5 || app->size > 64) {
		Log(Logs::General, Logs::Error, "Size mismatch in OP_BoardBoad. Expected greater than 5 less than 64, got %i", app->size);
		DumpPacket(app);
		return;
	}

	char boatname[64];
	memcpy(boatname, app->pBuffer, app->size);
	boatname[63] = '\0';

	Mob* boat = entity_list.GetMob(boatname);
	if (!boat || (boat->GetRace() != CONTROLLED_BOAT && boat->GetRace() != 502))
		return;
	BoatID = boat->GetID();	// set the client's BoatID to show that it's on this boat

	return;
}

void Client::Handle_OP_Buff(const EQApplicationPacket *app)
{
	if (app->size != sizeof(SpellBuffPacket_Struct))
	{
		Log(Logs::General, Logs::Error, "Size mismatch in OP_Buff. expected %i got %i", sizeof(SpellBuffPacket_Struct), app->size);
		DumpPacket(app);
		return;
	}

	SpellBuffPacket_Struct* sbf = (SpellBuffPacket_Struct*)app->pBuffer;
	uint32 spid = sbf->buff.spellid;
	Log(Logs::Detail, Logs::Spells, "Client requested that buff with spell id %d be canceled.", spid);

	//something about IsDetrimentalSpell() crashes this portion of code..
	//tbh we shouldn't use it anyway since this is a simple red vs blue buff check and
	//isdetrimentalspell() is much more complex
	if (spid == 0xFFFF || (IsValidSpell(spid) && (spells[spid].goodEffect == 0)))
		QueuePacket(app);
	else
		BuffFadeBySpellID(spid);

	return;
}

void Client::Handle_OP_BuffRemoveRequest(const EQApplicationPacket *app)
{
	// In SoD, this is used for clicking off Pet Buffs only. In Underfoot, it is used both for Client and Pets
	// The payload contains buffslot and EntityID only, so we must check if the EntityID is ours or our pets.
	//
	VERIFY_PACKET_LENGTH(OP_BuffRemoveRequest, app, BuffRemoveRequest_Struct);

	BuffRemoveRequest_Struct *brrs = (BuffRemoveRequest_Struct*)app->pBuffer;

	Mob *m = nullptr;

	if (brrs->EntityID == GetID()) {
		m = this;
	}
	else if (brrs->EntityID == GetPetID()) {
		m = GetPet();
	}
#ifdef BOTS
	else {
		Mob* bot_test = entity_list.GetMob(brrs->EntityID);
		if (bot_test && bot_test->IsBot() && bot_test->GetOwner() == this)
			m = bot_test;
	}
#endif

	if (!m)
		return;

	if (brrs->SlotID > (uint32)m->GetMaxTotalSlots())
		return;

	uint16 SpellID = m->GetSpellIDFromSlot(brrs->SlotID);

	if (SpellID && IsBeneficialSpell(SpellID) && !spells[SpellID].no_remove)
		m->BuffFadeBySlot(brrs->SlotID, true);
}

void Client::Handle_OP_Bug(const EQApplicationPacket *app)
{
	if (app->size != sizeof(BugStruct))
		printf("Wrong size of BugStruct got %d expected %zu!\n", app->size, sizeof(BugStruct));
	else {
		BugStruct* bug = (BugStruct*)app->pBuffer;
		database.UpdateBug(bug);
	}
	return;
}

void Client::Handle_OP_Camp(const EQApplicationPacket *app)
{
#ifdef BOTS
	// This block is necessary to clean up any bot objects owned by a Client
	Bot::BotOrderCampAll(this);
#endif
	if (IsLFP())
		worldserver.StopLFP(CharacterID());

	if (GetGM())
	{
		OnDisconnect(true);
		return;
	}
	camp_timer.Start(29000, true);
	return;
}

void Client::Handle_OP_CancelTask(const EQApplicationPacket *app)
{

	if (app->size != sizeof(CancelTask_Struct)) {
		Log(Logs::General, Logs::None, "Size mismatch in OP_CancelTask expected %i got %i",
			sizeof(CancelTask_Struct), app->size);
		DumpPacket(app);
		return;
	}
	CancelTask_Struct *cts = (CancelTask_Struct*)app->pBuffer;

	if (RuleB(TaskSystem, EnableTaskSystem) && taskstate)
		taskstate->CancelTask(this, cts->SequenceNumber);
}

void Client::Handle_OP_CancelTrade(const EQApplicationPacket *app)
{
	if (app->size != sizeof(CancelTrade_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_CancelTrade, size=%i, expected %i", app->size, sizeof(CancelTrade_Struct));
		return;
	}
	Mob* with = trade->With();
	if (with && with->IsClient()) {
		CancelTrade_Struct* msg = (CancelTrade_Struct*)app->pBuffer;

		// Forward cancel packet to other client
		msg->fromid = with->GetID();
		//msg->action = 1;

		with->CastToClient()->QueuePacket(app);

		// Put trade items/cash back into inventory
		FinishTrade(this);
		trade->Reset();
	}
	else if (with) {
		CancelTrade_Struct* msg = (CancelTrade_Struct*)app->pBuffer;
		msg->fromid = with->GetID();
		QueuePacket(app);
		FinishTrade(this);
		trade->Reset();
	}
	EQApplicationPacket end_trade1(OP_FinishWindow, 0);
	QueuePacket(&end_trade1);

	EQApplicationPacket end_trade2(OP_FinishWindow2, 0);
	QueuePacket(&end_trade2);
	return;
}

void Client::Handle_OP_CastSpell(const EQApplicationPacket *app)
{
	using EQEmu::CastingSlot;
	if (app->size != sizeof(CastSpell_Struct)) {
		std::cout << "Wrong size: OP_CastSpell, size=" << app->size << ", expected " << sizeof(CastSpell_Struct) << std::endl;
		return;
	}
	if (IsAIControlled()) {
		this->Message_StringID(13, NOT_IN_CONTROL);
		//Message(13, "You cant cast right now, you arent in control of yourself!");
		return;
	}

	CastSpell_Struct* castspell = (CastSpell_Struct*)app->pBuffer;

	m_TargetRing = glm::vec3(castspell->x_pos, castspell->y_pos, castspell->z_pos);

	Log(Logs::General, Logs::Spells, "OP CastSpell: slot=%d, spell=%d, target=%d, inv=%lx", castspell->slot, castspell->spell_id, castspell->target_id, (unsigned long)castspell->inventoryslot);
	CastingSlot slot = static_cast<CastingSlot>(castspell->slot);

	/* Memorized Spell */
	if (m_pp.mem_spells[castspell->slot] && m_pp.mem_spells[castspell->slot] == castspell->spell_id) {
		uint16 spell_to_cast = 0;
		if (castspell->slot < MAX_PP_MEMSPELL) {
			spell_to_cast = m_pp.mem_spells[castspell->slot];
			if (spell_to_cast != castspell->spell_id) {
				InterruptSpell(castspell->spell_id); //CHEATER!!!
				return;
			}
		}
		else if (castspell->slot >= MAX_PP_MEMSPELL) {
			InterruptSpell();
			return;
		}

		CastSpell(spell_to_cast, castspell->target_id, slot);
	}
	/* Spell Slot or Potion Belt Slot */
	else if (slot == CastingSlot::Item || slot == CastingSlot::PotionBelt)	// ITEM or POTION cast
	{
		if (m_inv.SupportsClickCasting(castspell->inventoryslot) || slot == CastingSlot::PotionBelt)	// sanity check
		{
			// packet field types will be reviewed as packet transistions occur
			const EQEmu::ItemInstance* inst = m_inv[castspell->inventoryslot]; //slot values are int16, need to check packet on this field
																			   //bool cancast = true;
			if (inst && inst->IsClassCommon())
			{
				const EQEmu::ItemData* item = inst->GetItem();
				if (item->Click.Effect != (uint32)castspell->spell_id)
				{
					database.SetMQDetectionFlag(account_name, name, "OP_CastSpell with item, tried to cast a different spell.", zone->GetShortName());
					InterruptSpell(castspell->spell_id);	//CHEATER!!
					return;
				}

				if ((item->Click.Type == EQEmu::item::ItemEffectClick) || (item->Click.Type == EQEmu::item::ItemEffectExpendable) || (item->Click.Type == EQEmu::item::ItemEffectEquipClick) || (item->Click.Type == EQEmu::item::ItemEffectClick2))
				{
					if (item->Click.Level2 > 0)
					{
						if (GetLevel() >= item->Click.Level2)
						{
							EQEmu::ItemInstance* p_inst = (EQEmu::ItemInstance*)inst;
							int i = parse->EventItem(EVENT_ITEM_CLICK_CAST, this, p_inst, nullptr, "", castspell->inventoryslot);

							if (i == 0) {
								CastSpell(item->Click.Effect, castspell->target_id, slot, item->CastTime, 0, 0, castspell->inventoryslot);
							}
							else {
								InterruptSpell(castspell->spell_id);
								return;
							}
						}
						else
						{
							database.SetMQDetectionFlag(account_name, name, "OP_CastSpell with item, did not meet req level.", zone->GetShortName());
							Message(0, "Error: level not high enough.", castspell->inventoryslot);
							InterruptSpell(castspell->spell_id);
						}
					}
					else
					{
						EQEmu::ItemInstance* p_inst = (EQEmu::ItemInstance*)inst;
						int i = parse->EventItem(EVENT_ITEM_CLICK_CAST, this, p_inst, nullptr, "", castspell->inventoryslot);

						if (i == 0) {
							CastSpell(item->Click.Effect, castspell->target_id, slot, item->CastTime, 0, 0, castspell->inventoryslot);
						}
						else {
							InterruptSpell(castspell->spell_id);
							return;
						}
					}
				}
				else
				{
					Message(0, "Error: unknown item->Click.Type (0x%02x)", item->Click.Type);
				}
			}
			else
			{
				Message(0, "Error: item not found in inventory slot #%i", castspell->inventoryslot);
				InterruptSpell(castspell->spell_id);
			}
		}
		else
		{
			Message(0, "Error: castspell->inventoryslot >= %i (0x%04x)", EQEmu::inventory::slotCursor, castspell->inventoryslot);
			InterruptSpell(castspell->spell_id);
		}
	}
	/* Discipline -- older clients use the same slot as items, but we translate to it's own */
	else if (slot == CastingSlot::Discipline) {
		if (!UseDiscipline(castspell->spell_id, castspell->target_id)) {
			Log(Logs::General, Logs::Spells, "Unknown ability being used by %s, spell being cast is: %i\n", GetName(), castspell->spell_id);
			InterruptSpell(castspell->spell_id);
			return;
		}
	}
	/* ABILITY cast (LoH and Harm Touch) */
	else if (slot == CastingSlot::Ability) {
		uint16 spell_to_cast = 0;

		if (castspell->spell_id == SPELL_LAY_ON_HANDS && GetClass() == PALADIN) {
			if (!p_timers.Expired(&database, pTimerLayHands)) {
				Message(13, "Ability recovery time not yet met.");
				InterruptSpell(castspell->spell_id);
				return;
			}
			spell_to_cast = SPELL_LAY_ON_HANDS;
			p_timers.Start(pTimerLayHands, LayOnHandsReuseTime);
		}
		else if ((castspell->spell_id == SPELL_HARM_TOUCH
			|| castspell->spell_id == SPELL_HARM_TOUCH2) && GetClass() == SHADOWKNIGHT) {
			if (!p_timers.Expired(&database, pTimerHarmTouch)) {
				Message(13, "Ability recovery time not yet met.");
				InterruptSpell(castspell->spell_id);
				return;
			}

			// determine which version of HT we are casting based on level
			if (GetLevel() < 40)
				spell_to_cast = SPELL_HARM_TOUCH;
			else
				spell_to_cast = SPELL_HARM_TOUCH2;

			p_timers.Start(pTimerHarmTouch, HarmTouchReuseTime);
		}

		if (spell_to_cast > 0)	// if we've matched LoH or HT, cast now
			CastSpell(spell_to_cast, castspell->target_id, slot);
	}
	return;
}

void Client::Handle_OP_ChannelMessage(const EQApplicationPacket *app)
{
	ChannelMessage_Struct* cm = (ChannelMessage_Struct*)app->pBuffer;

	if (app->size < sizeof(ChannelMessage_Struct)) {
		std::cout << "Wrong size " << app->size << ", should be " << sizeof(ChannelMessage_Struct) << "+ on 0x" << std::hex << std::setfill('0') << std::setw(4) << app->GetOpcode() << std::dec << std::endl;
		return;
	}
	if (IsAIControlled()) {
		Message(13, "You try to speak but cant move your mouth!");
		return;
	}

	ChannelMessageReceived(cm->chan_num, cm->language, cm->skill_in_language, cm->message, cm->targetname);
	return;
}

void Client::Handle_OP_ClearBlockedBuffs(const EQApplicationPacket *app)
{
	if (!RuleB(Spells, EnableBlockedBuffs))
		return;

	if (app->size != 1)
	{
		Log(Logs::General, Logs::None, "Size mismatch in OP_ClearBlockedBuffs expected 1 got %i", app->size);

		DumpPacket(app);

		return;
	}

	bool Pet = app->pBuffer[0];

	if (Pet)
		PetBlockedBuffs.clear();
	else
		PlayerBlockedBuffs.clear();

	QueuePacket(app);
}

void Client::Handle_OP_ClearNPCMarks(const EQApplicationPacket *app)
{

	if (app->size != 0)
	{
		Log(Logs::General, Logs::None, "Size mismatch in OP_ClearNPCMarks expected 0 got %i",
			app->size);

		DumpPacket(app);

		return;
	}

	Group *g = GetGroup();

	if (g)
		g->ClearAllNPCMarks();
}

void Client::Handle_OP_ClearSurname(const EQApplicationPacket *app)
{
	ChangeLastName("");
}

void Client::Handle_OP_ClickDoor(const EQApplicationPacket *app)
{
	if (app->size != sizeof(ClickDoor_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_ClickDoor, size=%i, expected %i", app->size, sizeof(ClickDoor_Struct));
		return;
	}
	ClickDoor_Struct* cd = (ClickDoor_Struct*)app->pBuffer;
	Doors* currentdoor = entity_list.FindDoor(cd->doorid);
	if (!currentdoor)
	{
		Message(0, "Unable to find door, please notify a GM (DoorID: %i).", cd->doorid);
		return;
	}

	char buf[20];
	snprintf(buf, 19, "%u", cd->doorid);
	buf[19] = '\0';
	std::vector<EQEmu::Any> args;
	args.push_back(currentdoor);
	parse->EventPlayer(EVENT_CLICK_DOOR, this, buf, 0, &args);

	currentdoor->HandleClick(this, 0);
	return;
}

void Client::Handle_OP_ClickObject(const EQApplicationPacket *app)
{
	if (app->size != sizeof(ClickObject_Struct)) {
		Log(Logs::General, Logs::Error, "Invalid size on ClickObject_Struct: Expected %i, Got %i",
			sizeof(ClickObject_Struct), app->size);
		return;
	}

	ClickObject_Struct* click_object = (ClickObject_Struct*)app->pBuffer;
	Entity* entity = entity_list.GetID(click_object->drop_id);
	if (entity && entity->IsObject()) {
		Object* object = entity->CastToObject();

		object->HandleClick(this, click_object);

		std::vector<EQEmu::Any> args;
		args.push_back(object);

		char buf[10];
		snprintf(buf, 9, "%u", click_object->drop_id);
		buf[9] = '\0';
		parse->EventPlayer(EVENT_CLICK_OBJECT, this, buf, GetID(), &args);
	}

	// Observed in RoF after OP_ClickObjectAction:
	//EQApplicationPacket end_trade2(OP_FinishWindow2, 0);
	//QueuePacket(&end_trade2);
	return;
}

void Client::Handle_OP_ClickObjectAction(const EQApplicationPacket *app)
{
	if (app->size == 0) {
		// RoF sends this packet 0 sized when switching from auto-combine to experiment windows.
		// Not completely sure if 0 sized is for this or for closing objects as commented out below
		EQApplicationPacket end_trade1(OP_FinishWindow, 0);
		QueuePacket(&end_trade1);

		EQApplicationPacket end_trade2(OP_FinishWindow2, 0);
		QueuePacket(&end_trade2);

		// RoF sends a 0 sized packet for closing objects
		if (GetTradeskillObject() && ClientVersion() >= EQEmu::versions::ClientVersion::RoF)
			GetTradeskillObject()->CastToObject()->Close();

		return;
	}
	else
	{
		if (app->size != sizeof(ClickObjectAction_Struct)) {
			Log(Logs::General, Logs::Error, "Invalid size on OP_ClickObjectAction: Expected %i, Got %i",
				sizeof(ClickObjectAction_Struct), app->size);
			return;
		}

		ClickObjectAction_Struct* oos = (ClickObjectAction_Struct*)app->pBuffer;
		Entity* entity = entity_list.GetEntityObject(oos->drop_id);
		if (entity && entity->IsObject()) {
			Object* object = entity->CastToObject();
			if (oos->open == 0) {
				object->Close();
			}
			else {
				Log(Logs::General, Logs::Error, "Unsupported action %d in OP_ClickObjectAction", oos->open);
			}
		}
		else {
			Log(Logs::General, Logs::Error, "Invalid object %d in OP_ClickObjectAction", oos->drop_id);
		}
	}

	SetTradeskillObject(nullptr);

	EQApplicationPacket end_trade1(OP_FinishWindow, 0);
	QueuePacket(&end_trade1);

	EQApplicationPacket end_trade2(OP_FinishWindow2, 0);
	QueuePacket(&end_trade2);
	return;
}

void Client::Handle_OP_ClientError(const EQApplicationPacket *app)
{
	ClientError_Struct* error = (ClientError_Struct*)app->pBuffer;
	Log(Logs::General, Logs::Error, "Client error: %s", error->character_name);
	Log(Logs::General, Logs::Error, "Error message:%s", error->message);
	return;
}

void Client::Handle_OP_ClientTimeStamp(const EQApplicationPacket *app)
{
	return;
}

void Client::Handle_OP_ClientUpdate(const EQApplicationPacket *app)
{
	if (IsAIControlled())
		return;

	if (dead)
		return;

	//currently accepting two sizes, one has an extra byte on the end
	if (app->size != sizeof(PlayerPositionUpdateClient_Struct)
		&& app->size != (sizeof(PlayerPositionUpdateClient_Struct) + 1)
		) {
		Log(Logs::General, Logs::Error, "OP size error: OP_ClientUpdate expected:%i got:%i", sizeof(PlayerPositionUpdateClient_Struct), app->size);
		return;
	}
	PlayerPositionUpdateClient_Struct* ppu = (PlayerPositionUpdateClient_Struct*)app->pBuffer;

	if (ppu->spawn_id != GetID()) {
		// check if the id is for a boat the player is controlling
		if (ppu->spawn_id == BoatID) {
			Mob* boat = entity_list.GetMob(BoatID);
			if (boat == 0) {	// if the boat ID is invalid, reset the id and abort
				BoatID = 0;
				return;
			}

			// set the boat's position deltas
			auto boatDelta = glm::vec4(ppu->delta_x, ppu->delta_y, ppu->delta_z, ppu->delta_heading);
			boat->SetDelta(boatDelta);
			// send an update to everyone nearby except the client controlling the boat
			auto outapp =
				new EQApplicationPacket(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
			PlayerPositionUpdateServer_Struct* ppus = (PlayerPositionUpdateServer_Struct*)outapp->pBuffer;
			boat->MakeSpawnUpdate(ppus);
			entity_list.QueueCloseClients(boat, outapp, true, 300, this, false);
			safe_delete(outapp);
			// update the boat's position on the server, without sending an update
			boat->GMMove(ppu->x_pos, ppu->y_pos, ppu->z_pos, EQ19toFloat(ppu->heading), false);
			return;
		}
		else return;	// if not a boat, do nothing
	}

	float dist = 0;
	float tmp;
	tmp = m_Position.x - ppu->x_pos;
	dist += tmp*tmp;
	tmp = m_Position.y - ppu->y_pos;
	dist += tmp*tmp;
	dist = sqrt(dist);

	//the purpose of this first block may not be readily apparent
	//basically it's so people don't do a moderate warp every 2.5 seconds
	//letting it even out and basically getting the job done without triggering
	if (dist == 0)
	{
		if (m_DistanceSinceLastPositionCheck > 0.0)
		{
			uint32 cur_time = Timer::GetCurrentTime();
			if ((cur_time - m_TimeSinceLastPositionCheck) > 0)
			{
				float speed = (m_DistanceSinceLastPositionCheck * 100) / (float)(cur_time - m_TimeSinceLastPositionCheck);
				int runs = GetRunspeed();
				if (speed > (runs * RuleR(Zone, MQWarpDetectionDistanceFactor)))
				{
					if (!GetGMSpeed() && (runs >= GetBaseRunspeed() || (speed > (GetBaseRunspeed() * RuleR(Zone, MQWarpDetectionDistanceFactor)))))
					{
						if (IsShadowStepExempted())
						{
							if (m_DistanceSinceLastPositionCheck > 800)
							{
								CheatDetected(MQWarpShadowStep, ppu->x_pos, ppu->y_pos, ppu->z_pos);
							}
						}
						else if (IsKnockBackExempted())
						{
							//still potential to trigger this if you're knocked back off a
							//HUGE fall that takes > 2.5 seconds
							if (speed > 30.0f)
							{
								CheatDetected(MQWarpKnockBack, ppu->x_pos, ppu->y_pos, ppu->z_pos);
							}
						}
						else if (!IsPortExempted())
						{
							if (!IsMQExemptedArea(zone->GetZoneID(), ppu->x_pos, ppu->y_pos, ppu->z_pos))
							{
								if (speed > (runs * 2 * RuleR(Zone, MQWarpDetectionDistanceFactor)))
								{
									m_TimeSinceLastPositionCheck = cur_time;
									m_DistanceSinceLastPositionCheck = 0.0f;
									CheatDetected(MQWarp, ppu->x_pos, ppu->y_pos, ppu->z_pos);
									//Death(this, 10000000, SPELL_UNKNOWN, _1H_BLUNT);
								}
								else
								{
									CheatDetected(MQWarpLight, ppu->x_pos, ppu->y_pos, ppu->z_pos);
								}
							}
						}
					}
				}
				SetShadowStepExemption(false);
				SetKnockBackExemption(false);
				SetPortExemption(false);
				m_TimeSinceLastPositionCheck = cur_time;
				m_DistanceSinceLastPositionCheck = 0.0f;
				m_CheatDetectMoved = false;
			}
		}
		else
		{
			m_TimeSinceLastPositionCheck = Timer::GetCurrentTime();
			m_CheatDetectMoved = false;
		}
	}
	else
	{
		m_DistanceSinceLastPositionCheck += dist;
		m_CheatDetectMoved = true;
		if (m_TimeSinceLastPositionCheck == 0)
		{
			m_TimeSinceLastPositionCheck = Timer::GetCurrentTime();
		}
		else
		{
			uint32 cur_time = Timer::GetCurrentTime();
			if ((cur_time - m_TimeSinceLastPositionCheck) > 2500)
			{
				float speed = (m_DistanceSinceLastPositionCheck * 100) / (float)(cur_time - m_TimeSinceLastPositionCheck);
				int runs = GetRunspeed();
				if (speed > (runs * RuleR(Zone, MQWarpDetectionDistanceFactor)))
				{
					if (!GetGMSpeed() && (runs >= GetBaseRunspeed() || (speed > (GetBaseRunspeed() * RuleR(Zone, MQWarpDetectionDistanceFactor)))))
					{
						if (IsShadowStepExempted())
						{
							if (m_DistanceSinceLastPositionCheck > 800)
							{
								//if(!IsMQExemptedArea(zone->GetZoneID(), ppu->x_pos, ppu->y_pos, ppu->z_pos))
								//{
								CheatDetected(MQWarpShadowStep, ppu->x_pos, ppu->y_pos, ppu->z_pos);
								//Death(this, 10000000, SPELL_UNKNOWN, _1H_BLUNT);
								//}
							}
						}
						else if (IsKnockBackExempted())
						{
							//still potential to trigger this if you're knocked back off a
							//HUGE fall that takes > 2.5 seconds
							if (speed > 30.0f)
							{
								CheatDetected(MQWarpKnockBack, ppu->x_pos, ppu->y_pos, ppu->z_pos);
							}
						}
						else if (!IsPortExempted())
						{
							if (!IsMQExemptedArea(zone->GetZoneID(), ppu->x_pos, ppu->y_pos, ppu->z_pos))
							{
								if (speed > (runs * 2 * RuleR(Zone, MQWarpDetectionDistanceFactor)))
								{
									m_TimeSinceLastPositionCheck = cur_time;
									m_DistanceSinceLastPositionCheck = 0.0f;
									CheatDetected(MQWarp, ppu->x_pos, ppu->y_pos, ppu->z_pos);
									//Death(this, 10000000, SPELL_UNKNOWN, _1H_BLUNT);
								}
								else
								{
									CheatDetected(MQWarpLight, ppu->x_pos, ppu->y_pos, ppu->z_pos);
								}
							}
						}
					}
				}
				SetShadowStepExemption(false);
				SetKnockBackExemption(false);
				SetPortExemption(false);
				m_TimeSinceLastPositionCheck = cur_time;
				m_DistanceSinceLastPositionCheck = 0.0f;
			}
		}

		if (IsDraggingCorpse())
			DragCorpses();
	}

	//Check to see if PPU should trigger an update to the rewind position.
	float rewind_x_diff = 0;
	float rewind_y_diff = 0;

	rewind_x_diff = ppu->x_pos - m_RewindLocation.x;
	rewind_x_diff *= rewind_x_diff;
	rewind_y_diff = ppu->y_pos - m_RewindLocation.y;
	rewind_y_diff *= rewind_y_diff;

	//We only need to store updated values if the player has moved.
	//If the player has moved more than units for x or y, then we'll store
	//his pre-PPU x and y for /rewind, in case he gets stuck.
	if ((rewind_x_diff > 750) || (rewind_y_diff > 750))
		m_RewindLocation = glm::vec3(m_Position);

	//If the PPU was a large jump, such as a cross zone gate or Call of Hero,
	//just update rewind coords to the new ppu coords. This will prevent exploitation.

	if ((rewind_x_diff > 5000) || (rewind_y_diff > 5000))
		m_RewindLocation = glm::vec3(ppu->x_pos, ppu->y_pos, ppu->z_pos);

	if (proximity_timer.Check()) {
		entity_list.ProcessMove(this, glm::vec3(ppu->x_pos, ppu->y_pos, ppu->z_pos));
		if (RuleB(TaskSystem, EnableTaskSystem) && RuleB(TaskSystem, EnableTaskProximity))
			ProcessTaskProximities(ppu->x_pos, ppu->y_pos, ppu->z_pos);

		m_Proximity = glm::vec3(ppu->x_pos, ppu->y_pos, ppu->z_pos);
	}

	// Update internal state
	m_Delta = glm::vec4(ppu->delta_x, ppu->delta_y, ppu->delta_z, ppu->delta_heading);

	if (IsTracking() && ((m_Position.x != ppu->x_pos) || (m_Position.y != ppu->y_pos))) {
		if (zone->random.Real(0, 100) < 70)//should be good
			CheckIncreaseSkill(EQEmu::skills::SkillTracking, nullptr, -20);
	}

	// Break Hide if moving without sneaking and set rewind timer if moved
	if (ppu->y_pos != m_Position.y || ppu->x_pos != m_Position.x) {
		if ((hidden || improved_hidden) && !sneaking) {
			hidden = false;
			improved_hidden = false;
			if (!invisible) {
				auto outapp =
					new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
				SpawnAppearance_Struct* sa_out = (SpawnAppearance_Struct*)outapp->pBuffer;
				sa_out->spawn_id = GetID();
				sa_out->type = 0x03;
				sa_out->parameter = 0;
				entity_list.QueueClients(this, outapp, true);
				safe_delete(outapp);
			}
		}
		rewind_timer.Start(30000, true);
	}

	/* Handle client aggro scanning timers NPCs */
	is_client_moving = (ppu->y_pos == m_Position.y && ppu->x_pos == m_Position.x) ? false : true;

	if (is_client_moving) {
		Log(Logs::Detail, Logs::Normal, "ClientUpdate: Client is moving - scan timer is: %u", client_scan_npc_aggro_timer.GetDuration());
		if (client_scan_npc_aggro_timer.GetDuration() > 1000) {

			npc_close_scan_timer.Disable();
			npc_close_scan_timer.Start(500);

			client_scan_npc_aggro_timer.Disable();
			client_scan_npc_aggro_timer.Start(500);

		}
	}
	else {
		Log(Logs::Detail, Logs::Normal, "ClientUpdate: Client is NOT moving - scan timer is: %u", client_scan_npc_aggro_timer.GetDuration());
		if (client_scan_npc_aggro_timer.GetDuration() < 1000) {

			npc_close_scan_timer.Disable();
			npc_close_scan_timer.Start(6000);

			client_scan_npc_aggro_timer.Disable();
			client_scan_npc_aggro_timer.Start(3000);
		}
	}

	// Outgoing client packet
	float tmpheading = EQ19toFloat(ppu->heading);
	/* The clients send an update at best every 1.3 seconds
	* We want to avoid reflecting these updates to other clients as much as possible
	* The client also sends an update every 280 ms while turning, if we prevent
	* sending these by checking if the location is the same too aggressively, clients end up spinning
	* so keep a count of how many packets are the same within a tolerance and stop when we get there */

	bool pos_same = FCMP(ppu->y_pos, m_Position.y) && FCMP(ppu->x_pos, m_Position.x) && FCMP(tmpheading, m_Position.w) && ppu->animation == animation;
	if (!pos_same || (pos_same && position_update_same_count < 6))
	{
		if (pos_same)
			position_update_same_count++;
		else
			position_update_same_count = 0;

		m_Position.x = ppu->x_pos;
		m_Position.y = ppu->y_pos;
		m_Position.z = ppu->z_pos;
		m_Position.w = tmpheading;
		animation = ppu->animation;

		auto outapp = new EQApplicationPacket(OP_ClientUpdate, sizeof(PlayerPositionUpdateServer_Struct));
		PlayerPositionUpdateServer_Struct* ppu = (PlayerPositionUpdateServer_Struct*)outapp->pBuffer;
		MakeSpawnUpdate(ppu);
		if (gmhideme)
			entity_list.QueueClientsStatus(this, outapp, true, Admin(), 250);
		else
			entity_list.QueueCloseClients(this, outapp, true, 300, nullptr, false);
		safe_delete(outapp);
	}

	if (zone->watermap) {
		if (zone->watermap->InLiquid(glm::vec3(m_Position)))
			CheckIncreaseSkill(EQEmu::skills::SkillSwimming, nullptr, -17);
		CheckRegionTypeChanges();
	}

	return;
}

/*
void Client::Handle_OP_CloseContainer(const EQApplicationPacket *app)
{
if (app->size != sizeof(CloseContainer_Struct)) {
LogFile->write(EQEMuLog::Error, "Invalid size on CloseContainer_Struct: Expected %i, Got %i",
sizeof(CloseContainer_Struct), app->size);
return;
}

SetTradeskillObject(nullptr);

ClickObjectAck_Struct* oos = (ClickObjectAck_Struct*)app->pBuffer;
Entity* entity = entity_list.GetEntityObject(oos->drop_id);
if (entity && entity->IsObject()) {
Object* object = entity->CastToObject();
object->Close();
}
return;
}
*/

void Client::Handle_OP_CombatAbility(const EQApplicationPacket *app)
{
	if (app->size != sizeof(CombatAbility_Struct)) {
		std::cout << "Wrong size on OP_CombatAbility. Got: " << app->size << ", Expected: " << sizeof(CombatAbility_Struct) << std::endl;
		return;
	}
	auto ca_atk = (CombatAbility_Struct *)app->pBuffer;
	OPCombatAbility(ca_atk);
	return;
}

void Client::Handle_OP_ConfirmDelete(const EQApplicationPacket* app)
{
	return;
}

void Client::Handle_OP_Consent(const EQApplicationPacket *app)
{
	if (app->size<64) {
		Consent_Struct* c = (Consent_Struct*)app->pBuffer;
		if (strcmp(c->name, GetName()) != 0) {
			auto pack = new ServerPacket(ServerOP_Consent, sizeof(ServerOP_Consent_Struct));
			ServerOP_Consent_Struct* scs = (ServerOP_Consent_Struct*)pack->pBuffer;
			strcpy(scs->grantname, c->name);
			strcpy(scs->ownername, GetName());
			scs->message_string_id = 0;
			scs->permission = 1;
			scs->zone_id = zone->GetZoneID();
			scs->instance_id = zone->GetInstanceID();
			//consent_list.push_back(scs->grantname);
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
		else {
			Message_StringID(0, CONSENT_YOURSELF);
		}
	}
	return;
}

void Client::Handle_OP_ConsentDeny(const EQApplicationPacket *app)
{
	if (app->size<64) {
		Consent_Struct* c = (Consent_Struct*)app->pBuffer;
		auto pack = new ServerPacket(ServerOP_Consent, sizeof(ServerOP_Consent_Struct));
		ServerOP_Consent_Struct* scs = (ServerOP_Consent_Struct*)pack->pBuffer;
		strcpy(scs->grantname, c->name);
		strcpy(scs->ownername, GetName());
		scs->message_string_id = 0;
		scs->permission = 0;
		scs->zone_id = zone->GetZoneID();
		scs->instance_id = zone->GetInstanceID();
		//consent_list.remove(scs->grantname);
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
	return;
}

void Client::Handle_OP_Consider(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Consider_Struct))
	{
		Log(Logs::General, Logs::None, "Size mismatch in Consider expected %i got %i", sizeof(Consider_Struct), app->size);
		return;
	}
	Consider_Struct* conin = (Consider_Struct*)app->pBuffer;
	Mob* tmob = entity_list.GetMob(conin->targetid);
	if (tmob == 0)
		return;

	if (tmob->GetClass() == LDON_TREASURE)
	{
		Message(15, "%s", tmob->GetCleanName());
		return;
	}

	auto outapp = new EQApplicationPacket(OP_Consider, sizeof(Consider_Struct));
	Consider_Struct* con = (Consider_Struct*)outapp->pBuffer;
	con->playerid = GetID();
	con->targetid = conin->targetid;
	if (tmob->IsNPC())
		con->faction = GetFactionLevel(character_id, tmob->GetNPCTypeID(), race, class_, deity, (tmob->IsNPC()) ? tmob->CastToNPC()->GetPrimaryFaction() : 0, tmob); // Dec. 20, 2001; TODO: Send the players proper deity
	else
		con->faction = 1;
	con->level = GetLevelCon(tmob->GetLevel());

	if (ClientVersion() <= EQEmu::versions::ClientVersion::Titanium) {
		if (con->level == CON_GRAY)	{
			con->level = CON_GREEN;
		}
		if (con->level == CON_WHITE) {
			con->level = CON_WHITE_TITANIUM;
		}
	}

	if (zone->IsPVPZone()) {
		if (!tmob->IsNPC())
			con->pvpcon = tmob->CastToClient()->GetPVP();
	}

	// If we're feigned show NPC as indifferent
	if (tmob->IsNPC())
	{
		if (GetFeigned())
			con->faction = FACTION_INDIFFERENT;
	}

	if (!(con->faction == FACTION_SCOWLS))
	{
		if (tmob->IsNPC())
		{
			if (tmob->CastToNPC()->IsOnHatelist(this))
				con->faction = FACTION_THREATENLY;
		}
	}

	if (con->faction == FACTION_APPREHENSIVE) {
		con->faction = FACTION_SCOWLS;
	}
	else if (con->faction == FACTION_DUBIOUS) {
		con->faction = FACTION_THREATENLY;
	}
	else if (con->faction == FACTION_SCOWLS) {
		con->faction = FACTION_APPREHENSIVE;
	}
	else if (con->faction == FACTION_THREATENLY) {
		con->faction = FACTION_DUBIOUS;
	}

	mod_consider(tmob, con);

	QueuePacket(outapp);
	safe_delete(outapp);
	// only wanted to check raid target once
	// and need con to still be around so, do it here!
	if (tmob->IsRaidTarget()) {
		uint32 color = 0;
		switch (con->level) {
		case CON_GREEN:
			color = 2;
			break;
		case CON_LIGHTBLUE:
			color = 10;
			break;
		case CON_BLUE:
			color = 4;
			break;
		case CON_WHITE_TITANIUM:
		case CON_WHITE:
			color = 10;
			break;
		case CON_YELLOW:
			color = 15;
			break;
		case CON_RED:
			color = 13;
			break;
		case CON_GRAY:
			color = 6;
			break;
		}

		if (ClientVersion() <= EQEmu::versions::ClientVersion::Titanium) {
			if (color == 6)	{
				color = 2;
			}
		}

		SendColoredText(color, std::string("This creature would take an army to defeat!"));
	}

	// this could be done better, but this is only called when you con so w/e
	// Shroud of Stealth has a special message
	if (improved_hidden && (!tmob->see_improved_hide && (tmob->see_invis || tmob->see_hide)))
		Message_StringID(10, SOS_KEEPS_HIDDEN);
	// we are trying to hide but they can see us
	else if ((invisible || invisible_undead || hidden || invisible_animals) && !IsInvisible(tmob))
		Message_StringID(10, SUSPECT_SEES_YOU);

	return;
}

void Client::Handle_OP_ConsiderCorpse(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Consider_Struct))
	{
		Log(Logs::General, Logs::None, "Size mismatch in Consider corpse expected %i got %i", sizeof(Consider_Struct), app->size);
		return;
	}
	Consider_Struct* conin = (Consider_Struct*)app->pBuffer;
	Corpse* tcorpse = entity_list.GetCorpseByID(conin->targetid);
	if (tcorpse && tcorpse->IsNPCCorpse()) {
		uint32 min; uint32 sec; uint32 ttime;
		if ((ttime = tcorpse->GetDecayTime()) != 0) {
			sec = (ttime / 1000) % 60; // Total seconds
			min = (ttime / 60000) % 60; // Total seconds / 60 drop .00
			char val1[20] = { 0 };
			char val2[20] = { 0 };
			Message_StringID(10, CORPSE_DECAY1, ConvertArray(min, val1), ConvertArray(sec, val2));
		}
		else {
			Message_StringID(10, CORPSE_DECAY_NOW);
		}
	}
	else if (tcorpse && tcorpse->IsPlayerCorpse()) {
		uint32 day, hour, min, sec, ttime;
		if ((ttime = tcorpse->GetDecayTime()) != 0) {
			sec = (ttime / 1000) % 60; // Total seconds
			min = (ttime / 60000) % 60; // Total seconds
			hour = (ttime / 3600000) % 24; // Total hours
			day = ttime / 86400000; // Total Days
			if (day)
				Message(0, "This corpse will decay in %i days, %i hours, %i minutes and %i seconds.", day, hour, min, sec);
			else if (hour)
				Message(0, "This corpse will decay in %i hours, %i minutes and %i seconds.", hour, min, sec);
			else
				Message(0, "This corpse will decay in %i minutes and %i seconds.", min, sec);

			Message(0, "This corpse %s be resurrected.", tcorpse->IsRezzed() ? "cannot" : "can");
			/*
			hour = 0;

			if((ttime = tcorpse->GetResTime()) != 0) {
			sec = (ttime/1000)%60; // Total seconds
			min = (ttime/60000)%60; // Total seconds
			hour = (ttime/3600000)%24; // Total hours
			if(hour)
			Message(0, "This corpse can be resurrected for %i hours, %i minutes and %i seconds.", hour, min, sec);
			else
			Message(0, "This corpse can be resurrected for %i minutes and %i seconds.", min, sec);
			}
			else {
			Message_StringID(0, CORPSE_TOO_OLD);
			}
			*/
		}
		else {
			Message_StringID(10, CORPSE_DECAY_NOW);
		}
	}
}

void Client::Handle_OP_Consume(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Consume_Struct))
	{
		Log(Logs::General, Logs::Error, "OP size error: OP_Consume expected:%i got:%i", sizeof(Consume_Struct), app->size);
		return;
	}
	Consume_Struct* pcs = (Consume_Struct*)app->pBuffer;
	if (pcs->type == 0x01)
	{
		if (m_pp.hunger_level > 6000)
		{
			EQApplicationPacket *outapp = nullptr;
			outapp = new EQApplicationPacket(OP_Stamina, sizeof(Stamina_Struct));
			Stamina_Struct* sta = (Stamina_Struct*)outapp->pBuffer;
			sta->food = m_pp.hunger_level > 6000 ? 6000 : m_pp.hunger_level;
			sta->water = m_pp.thirst_level > 6000 ? 6000 : m_pp.thirst_level;

			QueuePacket(outapp);
			safe_delete(outapp);
			return;
		}
	}
	else if (pcs->type == 0x02)
	{
		if (m_pp.thirst_level > 6000)
		{
			EQApplicationPacket *outapp = nullptr;
			outapp = new EQApplicationPacket(OP_Stamina, sizeof(Stamina_Struct));
			Stamina_Struct* sta = (Stamina_Struct*)outapp->pBuffer;
			sta->food = m_pp.hunger_level > 6000 ? 6000 : m_pp.hunger_level;
			sta->water = m_pp.thirst_level > 6000 ? 6000 : m_pp.thirst_level;

			QueuePacket(outapp);
			safe_delete(outapp);
			return;
		}
	}

	EQEmu::ItemInstance *myitem = GetInv().GetItem(pcs->slot);
	if (myitem == nullptr) {
		Log(Logs::General, Logs::Error, "Consuming from empty slot %d", pcs->slot);
		return;
	}

	const EQEmu::ItemData* eat_item = myitem->GetItem();
	if (pcs->type == 0x01) {
		Consume(eat_item, EQEmu::item::ItemTypeFood, pcs->slot, (pcs->auto_consumed == 0xffffffff));
	}
	else if (pcs->type == 0x02) {
		Consume(eat_item, EQEmu::item::ItemTypeDrink, pcs->slot, (pcs->auto_consumed == 0xffffffff));
	}
	else {
		Log(Logs::General, Logs::Error, "OP_Consume: unknown type, type:%i", (int)pcs->type);
		return;
	}
	if (m_pp.hunger_level > 50000)
		m_pp.hunger_level = 50000;
	if (m_pp.thirst_level > 50000)
		m_pp.thirst_level = 50000;
	EQApplicationPacket *outapp = nullptr;
	outapp = new EQApplicationPacket(OP_Stamina, sizeof(Stamina_Struct));
	Stamina_Struct* sta = (Stamina_Struct*)outapp->pBuffer;
	sta->food = m_pp.hunger_level > 6000 ? 6000 : m_pp.hunger_level;
	sta->water = m_pp.thirst_level > 6000 ? 6000 : m_pp.thirst_level;

	QueuePacket(outapp);
	safe_delete(outapp);
	return;
}

void Client::Handle_OP_ControlBoat(const EQApplicationPacket *app)
{
	if (app->size != sizeof(ControlBoat_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_ControlBoat, size=%i, expected %i", app->size, sizeof(ControlBoat_Struct));
		return;
	}
	ControlBoat_Struct* cbs = (ControlBoat_Struct*)app->pBuffer;
	Mob* boat = entity_list.GetMob(cbs->boatId);
	if (boat == 0)
		return;	// do nothing if the boat isn't valid

	if (!boat->IsNPC() || (boat->GetRace() != CONTROLLED_BOAT && boat->GetRace() != 502))
	{
		char *hacked_string = nullptr;
		MakeAnyLenString(&hacked_string, "OP_Control Boat was sent against %s which is of race %u", boat->GetName(), boat->GetRace());
		database.SetMQDetectionFlag(this->AccountName(), this->GetName(), hacked_string, zone->GetShortName());
		safe_delete_array(hacked_string);
		return;
	}

	if (cbs->TakeControl) {
		// this uses the boat's target to indicate who has control of it. It has to check hate to make sure the boat isn't actually attacking anyone.
		if ((boat->GetTarget() == 0) || (boat->GetTarget() == this && boat->GetHateAmount(this) == 0)) {
			boat->SetTarget(this);
		}
		else {
			this->Message_StringID(13, IN_USE);
			return;
		}
	}
	else
		boat->SetTarget(0);

	auto outapp = new EQApplicationPacket(OP_ControlBoat, 0);
	FastQueuePacket(&outapp);
	safe_delete(outapp);
	// have the boat signal itself, so quests can be triggered by boat use
	boat->CastToNPC()->SignalNPC(0);
}

void Client::Handle_OP_CorpseDrag(const EQApplicationPacket *app)
{
	if (DraggedCorpses.size() >= (unsigned int)RuleI(Character, MaxDraggedCorpses))
	{
		Message_StringID(13, CORPSEDRAG_LIMIT);
		return;
	}

	VERIFY_PACKET_LENGTH(OP_CorpseDrag, app, CorpseDrag_Struct);

	CorpseDrag_Struct *cds = (CorpseDrag_Struct*)app->pBuffer;

	Mob* corpse = entity_list.GetMob(cds->CorpseName);

	if (!corpse || !corpse->IsPlayerCorpse() || corpse->CastToCorpse()->IsBeingLooted())
		return;

	Client *c = entity_list.FindCorpseDragger(corpse->GetID());

	if (c)
	{
		if (c == this)
			Message_StringID(MT_DefaultText, CORPSEDRAG_ALREADY, corpse->GetCleanName());
		else
			Message_StringID(MT_DefaultText, CORPSEDRAG_SOMEONE_ELSE, corpse->GetCleanName());

		return;
	}

	if (!corpse->CastToCorpse()->Summon(this, false, true))
		return;

	DraggedCorpses.push_back(std::pair<std::string, uint16>(cds->CorpseName, corpse->GetID()));

	Message_StringID(MT_DefaultText, CORPSEDRAG_BEGIN, cds->CorpseName);
}

void Client::Handle_OP_CorpseDrop(const EQApplicationPacket *app)
{
	if (app->size == 1)
	{
		Message_StringID(MT_DefaultText, CORPSEDRAG_STOPALL);
		ClearDraggedCorpses();
		return;
	}

	for (auto Iterator = DraggedCorpses.begin(); Iterator != DraggedCorpses.end(); ++Iterator)
	{
		if (!strcasecmp(Iterator->first.c_str(), (const char *)app->pBuffer))
		{
			Message_StringID(MT_DefaultText, CORPSEDRAG_STOP);
			Iterator = DraggedCorpses.erase(Iterator);
			return;
		}
	}
}

void Client::Handle_OP_CrashDump(const EQApplicationPacket *app)
{
	return;
}

void Client::Handle_OP_CreateObject(const EQApplicationPacket *app)
{
	if (LogSys.log_settings[Logs::Inventory].is_category_enabled)
		Log(Logs::Detail, Logs::Inventory, "Handle_OP_CreateObject() [psize: %u] %s", app->size, DumpPacketToString(app).c_str());

	DropItem(EQEmu::inventory::slotCursor);
	return;
}

void Client::Handle_OP_CrystalCreate(const EQApplicationPacket *app)
{
	VERIFY_PACKET_LENGTH(OP_CrystalCreate, app, CrystalReclaim_Struct);
	CrystalReclaim_Struct *cr = (CrystalReclaim_Struct*)app->pBuffer;

	const uint32 requestQty = cr->amount;
	const bool isRadiant = cr->type == 4;
	const bool isEbon = cr->type == 5;

	// Check: Valid type requested.
	if (!isRadiant && !isEbon) {
		return;
	}
	// Check: Valid quantity requested.
	if (requestQty < 1) {
		return;
	}

	// Check: Valid client state to make request.
	// In this situation the client is either desynced or attempting an exploit.
	const uint32 currentQty = isRadiant ? GetRadiantCrystals() : GetEbonCrystals();
	if (currentQty == 0) {
		return;
	}

	// Prevent the client from creating more than they have.
	const uint32 amount = EQEmu::ClampUpper(requestQty, currentQty);
	const uint32 itemID = isRadiant ? RuleI(Zone, RadiantCrystalItemID) : RuleI(Zone, EbonCrystalItemID);

	// Summon crystals for player.
	const bool success = SummonItem(itemID, amount);

	if (!success) {
		return;
	}

	// Deduct crystals from client and update them.
	if (isRadiant) {
		m_pp.currentRadCrystals -= amount;
		m_pp.careerRadCrystals -= amount;
	}
	else if (isEbon) {
		m_pp.currentEbonCrystals -= amount;
		m_pp.careerEbonCrystals -= amount;
	}

	SaveCurrency();
	SendCrystalCounts();
}

void Client::Handle_OP_CrystalReclaim(const EQApplicationPacket *app)
{
	uint32 ebon = NukeItem(RuleI(Zone, EbonCrystalItemID), invWhereWorn | invWherePersonal | invWhereCursor);
	uint32 radiant = NukeItem(RuleI(Zone, RadiantCrystalItemID), invWhereWorn | invWherePersonal | invWhereCursor);
	if ((ebon + radiant) > 0) {
		AddCrystals(radiant, ebon);
	}
}

void Client::Handle_OP_Damage(const EQApplicationPacket *app)
{
	if (app->size != sizeof(CombatDamage_Struct)) {
		Log(Logs::General, Logs::Error, "Received invalid sized OP_Damage: got %d, expected %d", app->size,
			sizeof(CombatDamage_Struct));
		DumpPacket(app);
		return;
	}

	// Broadcast to other clients
	CombatDamage_Struct* damage = (CombatDamage_Struct*)app->pBuffer;
	//dont send to originator of falling damage packets
	entity_list.QueueClients(this, app, (damage->type == DamageTypeFalling));
	return;
}

void Client::Handle_OP_Death(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Death_Struct))
		return;

	Death_Struct* ds = (Death_Struct*)app->pBuffer;

	//I think this attack_skill value is really a value from SkillDamageTypes...
	if (ds->attack_skill > EQEmu::skills::HIGHEST_SKILL) {
		return;
	}

	if (GetHP() > 0)
		return;

	Mob* killer = entity_list.GetMob(ds->killer_id);
	Death(killer, ds->damage, ds->spell_id, (EQEmu::skills::SkillType)ds->attack_skill);
	return;
}

void Client::Handle_OP_DelegateAbility(const EQApplicationPacket *app)
{

	if (app->size != sizeof(DelegateAbility_Struct))
	{
		Log(Logs::General, Logs::None, "Size mismatch in OP_DelegateAbility expected %i got %i",
			sizeof(DelegateAbility_Struct), app->size);

		DumpPacket(app);

		return;
	}

	DelegateAbility_Struct* das = (DelegateAbility_Struct*)app->pBuffer;

	Group *g = GetGroup();

	if (!g) return;

	switch (das->DelegateAbility)
	{
	case 0:
	{
		g->DelegateMainAssist(das->Name);
		break;
	}
	case 1:
	{
		g->DelegateMarkNPC(das->Name);
		break;
	}
	case 2:
	{
		g->DelegateMainTank(das->Name);
		break;
	}
	case 3:
	{
		g->DelegatePuller(das->Name);
		break;
	}
	default:
		break;
	}
}

void Client::Handle_OP_DeleteItem(const EQApplicationPacket *app)
{
	if (app->size != sizeof(DeleteItem_Struct)) {
		std::cout << "Wrong size on OP_DeleteItem. Got: " << app->size << ", Expected: " << sizeof(DeleteItem_Struct) << std::endl;
		return;
	}

	DeleteItem_Struct* alc = (DeleteItem_Struct*)app->pBuffer;
	const EQEmu::ItemInstance *inst = GetInv().GetItem(alc->from_slot);
	if (inst && inst->GetItem()->ItemType == EQEmu::item::ItemTypeAlcohol) {
		entity_list.MessageClose_StringID(this, true, 50, 0, DRINKING_MESSAGE, GetName(), inst->GetItem()->Name);
		CheckIncreaseSkill(EQEmu::skills::SkillAlcoholTolerance, nullptr, 25);

		int16 AlcoholTolerance = GetSkill(EQEmu::skills::SkillAlcoholTolerance);
		int16 IntoxicationIncrease;

		if (ClientVersion() < EQEmu::versions::ClientVersion::SoD)
			IntoxicationIncrease = (200 - AlcoholTolerance) * 30 / 200 + 10;
		else
			IntoxicationIncrease = (270 - AlcoholTolerance) * 0.111111108 + 10;

		if (IntoxicationIncrease < 0)
			IntoxicationIncrease = 1;

		m_pp.intoxication += IntoxicationIncrease;

		if (m_pp.intoxication > 200)
			m_pp.intoxication = 200;
	}
	DeleteItemInInventory(alc->from_slot, 1);

	return;
}

void Client::Handle_OP_DeleteSpawn(const EQApplicationPacket *app)
{
	// The client will send this with his id when he zones, maybe when he disconnects too?
	//eqs->RemoveData(); // Flushing the queue of packet data to allow for proper zoning

	//just make sure this gets out
	auto outapp = new EQApplicationPacket(OP_LogoutReply);
	FastQueuePacket(&outapp);

	outapp = new EQApplicationPacket(OP_DeleteSpawn, sizeof(EntityId_Struct));
	EntityId_Struct* eid = (EntityId_Struct*)outapp->pBuffer;
	eid->entity_id = GetID();

	entity_list.QueueClients(this, outapp, false);
	safe_delete(outapp);

	hate_list.RemoveEntFromHateList(this->CastToMob());

	Disconnect();
	return;
}

void Client::Handle_OP_DeleteSpell(const EQApplicationPacket *app)
{
	if (app->size != sizeof(DeleteSpell_Struct))
		return;

	EQApplicationPacket* outapp = app->Copy();
	DeleteSpell_Struct* dss = (DeleteSpell_Struct*)outapp->pBuffer;

	if (dss->spell_slot < 0 || dss->spell_slot > int(MAX_PP_SPELLBOOK))
		return;

	if (m_pp.spell_book[dss->spell_slot] != SPELLBOOK_UNKNOWN) {
		m_pp.spell_book[dss->spell_slot] = SPELLBOOK_UNKNOWN;
		database.DeleteCharacterSpell(this->CharacterID(), m_pp.spell_book[dss->spell_slot], dss->spell_slot);
		dss->success = 1;
	}
	else
		dss->success = 0;

	FastQueuePacket(&outapp);
	return;
}

void Client::Handle_OP_DisarmTraps(const EQApplicationPacket *app)
{
	if (!HasSkill(EQEmu::skills::SkillDisarmTraps))
		return;

	if (!p_timers.Expired(&database, pTimerDisarmTraps, false)) {
		Message(13, "Ability recovery time not yet met.");
		return;
	}

	int reuse = DisarmTrapsReuseTime - GetSkillReuseTime(EQEmu::skills::SkillDisarmTraps);

	if (reuse < 1)
		reuse = 1;

	p_timers.Start(pTimerDisarmTraps, reuse - 1);

	Trap* trap = entity_list.FindNearbyTrap(this, 60);
	if (trap && trap->detected)
	{
		int uskill = GetSkill(EQEmu::skills::SkillDisarmTraps);
		if ((zone->random.Int(0, 49) + uskill) >= (zone->random.Int(0, 49) + trap->skill))
		{
			Message(MT_Skills, "You disarm a trap.");
			trap->disarmed = true;
			trap->chkarea_timer.Disable();
			trap->respawn_timer.Start((trap->respawn_time + zone->random.Int(0, trap->respawn_var)) * 1000);
		}
		else
		{
			if (zone->random.Int(0, 99) < 25) {
				Message(MT_Skills, "You set off the trap while trying to disarm it!");
				trap->Trigger(this);
			}
			else {
				Message(MT_Skills, "You failed to disarm a trap.");
			}
		}
		CheckIncreaseSkill(EQEmu::skills::SkillDisarmTraps, nullptr);
		return;
	}
	Message(MT_Skills, "You did not find any traps close enough to disarm.");
	return;
}

void Client::Handle_OP_DoGroupLeadershipAbility(const EQApplicationPacket *app)
{

	if (app->size != sizeof(DoGroupLeadershipAbility_Struct)) {

		Log(Logs::General, Logs::None, "Size mismatch in OP_DoGroupLeadershipAbility expected %i got %i",
			sizeof(DoGroupLeadershipAbility_Struct), app->size);

		DumpPacket(app);

		return;
	}

	DoGroupLeadershipAbility_Struct* dglas = (DoGroupLeadershipAbility_Struct*)app->pBuffer;

	switch (dglas->Ability)
	{
	case GroupLeadershipAbility_MarkNPC:
	{
		if (GetTarget())
		{
			Group* g = GetGroup();
			if (g)
				g->MarkNPC(GetTarget(), dglas->Parameter);
		}
		break;
	}

	case groupAAInspectBuffs:
	{
		Mob *Target = GetTarget();

		if (!Target || !Target->IsClient())
			return;

		if (IsRaidGrouped()) {
			Raid *raid = GetRaid();
			if (!raid)
				return;
			uint32 group_id = raid->GetGroup(this);
			if (group_id > 11 || raid->GroupCount(group_id) < 3)
				return;
			Target->CastToClient()->InspectBuffs(this, raid->GetLeadershipAA(groupAAInspectBuffs, group_id));
			return;
		}

		Group *g = GetGroup();

		if (!g || (g->GroupCount() < 3))
			return;

		Target->CastToClient()->InspectBuffs(this, g->GetLeadershipAA(groupAAInspectBuffs));

		break;
	}

	default:
		Log(Logs::General, Logs::None, "Got unhandled OP_DoGroupLeadershipAbility Ability: %d Parameter: %d",
			dglas->Ability, dglas->Parameter);
		break;
	}
}

void Client::Handle_OP_DuelResponse(const EQApplicationPacket *app)
{
	if (app->size != sizeof(DuelResponse_Struct))
		return;
	DuelResponse_Struct* ds = (DuelResponse_Struct*)app->pBuffer;
	Entity* entity = entity_list.GetID(ds->target_id);
	Entity* initiator = entity_list.GetID(ds->entity_id);
	if (!entity->IsClient() || !initiator->IsClient())
		return;

	entity->CastToClient()->SetDuelTarget(0);
	entity->CastToClient()->SetDueling(false);
	initiator->CastToClient()->SetDuelTarget(0);
	initiator->CastToClient()->SetDueling(false);
	if (GetID() == initiator->GetID())
		entity->CastToClient()->Message_StringID(10, DUEL_DECLINE, initiator->GetName());
	else
		initiator->CastToClient()->Message_StringID(10, DUEL_DECLINE, entity->GetName());
	return;
}

void Client::Handle_OP_DuelResponse2(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Duel_Struct))
		return;

	Duel_Struct* ds = (Duel_Struct*)app->pBuffer;
	Entity* entity = entity_list.GetID(ds->duel_target);
	Entity* initiator = entity_list.GetID(ds->duel_initiator);

	if (entity && initiator && entity == this && initiator->IsClient()) {
		auto outapp = new EQApplicationPacket(OP_RequestDuel, sizeof(Duel_Struct));
		Duel_Struct* ds2 = (Duel_Struct*)outapp->pBuffer;

		ds2->duel_initiator = entity->GetID();
		ds2->duel_target = entity->GetID();
		initiator->CastToClient()->QueuePacket(outapp);

		outapp->SetOpcode(OP_DuelResponse2);
		ds2->duel_initiator = initiator->GetID();

		initiator->CastToClient()->QueuePacket(outapp);

		QueuePacket(outapp);
		SetDueling(true);
		initiator->CastToClient()->SetDueling(true);
		SetDuelTarget(ds->duel_initiator);
		safe_delete(outapp);

		if (IsCasting())
			InterruptSpell();
		if (initiator->CastToClient()->IsCasting())
			initiator->CastToClient()->InterruptSpell();
	}
	return;
}

void Client::Handle_OP_DumpName(const EQApplicationPacket *app)
{
	return;
}

void Client::Handle_OP_Dye(const EQApplicationPacket *app)
{
	if (app->size != sizeof(EQEmu::TintProfile))
		printf("Wrong size of DyeStruct, Got: %i, Expected: %zu\n", app->size, sizeof(EQEmu::TintProfile));
	else {
		EQEmu::TintProfile* dye = (EQEmu::TintProfile*)app->pBuffer;
		DyeArmor(dye);
	}
	return;
}

void Client::Handle_OP_Emote(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Emote_Struct)) {
		Log(Logs::General, Logs::Error, "Received invalid sized "
			"OP_Emote: got %d, expected %d", app->size,
			sizeof(Emote_Struct));
		DumpPacket(app);
		return;
	}

	// Calculate new packet dimensions
	Emote_Struct* in = (Emote_Struct*)app->pBuffer;
	in->message[1023] = '\0';

	const char* name = GetName();
	uint32 len_name = strlen(name);
	uint32 len_msg = strlen(in->message);
	// crash protection -- cheater
	if (len_msg > 512) {
		in->message[512] = '\0';
		len_msg = 512;
	}
	uint32 len_packet = sizeof(in->type) + len_name
		+ len_msg + 1;

	// Construct outgoing packet
	auto outapp = new EQApplicationPacket(OP_Emote, len_packet);
	Emote_Struct* out = (Emote_Struct*)outapp->pBuffer;
	out->type = in->type;
	memcpy(out->message, name, len_name);
	memcpy(&out->message[len_name], in->message, len_msg);

	/*
	if (target && target->IsClient()) {
	entity_list.QueueCloseClients(this, outapp, false, 100, target);

	cptr = outapp->pBuffer + 2;

	// not sure if live does this or not. thought it was a nice feature, but would take a lot to
	// clean up grammatical and other errors. Maybe with a regex parser...
	replacestr((char *)cptr, target->GetName(), "you");
	replacestr((char *)cptr, " he", " you");
	replacestr((char *)cptr, " she", " you");
	replacestr((char *)cptr, " him", " you");
	replacestr((char *)cptr, " her", " you");
	target->CastToClient()->QueuePacket(outapp);

	}
	else
	*/
	entity_list.QueueCloseClients(this, outapp, true, RuleI(Range, Emote), 0, true, FilterSocials);

	safe_delete(outapp);
	return;
}

void Client::Handle_OP_EndLootRequest(const EQApplicationPacket *app)
{
	if (app->size != sizeof(uint32)) {
		std::cout << "Wrong size: OP_EndLootRequest, size=" << app->size << ", expected " << sizeof(uint32) << std::endl;
		return;
	}

	SetLooting(0);

	Entity* entity = entity_list.GetID(*((uint16*)app->pBuffer));
	if (entity == 0) {
		Message(13, "Error: OP_EndLootRequest: Corpse not found (ent = 0)");
		if (ClientVersion() >= EQEmu::versions::ClientVersion::SoD)
			Corpse::SendEndLootErrorPacket(this);
		else
			Corpse::SendLootReqErrorPacket(this);
		return;
	}
	else if (!entity->IsCorpse()) {
		Message(13, "Error: OP_EndLootRequest: Corpse not found (!entity->IsCorpse())");
		Corpse::SendLootReqErrorPacket(this);
		return;
	}
	else {
		entity->CastToCorpse()->EndLoot(this, app);
	}
	return;
}

void Client::Handle_OP_EnvDamage(const EQApplicationPacket *app)
{
	if (!ClientFinishedLoading())
	{
		SetHP(GetHP() - 1);
		return;
	}

	if (app->size != sizeof(EnvDamage2_Struct)) {
		Log(Logs::General, Logs::Error, "Received invalid sized OP_EnvDamage: got %d, expected %d", app->size,
			sizeof(EnvDamage2_Struct));
		DumpPacket(app);
		return;
	}
	EnvDamage2_Struct* ed = (EnvDamage2_Struct*)app->pBuffer;

	int damage = ed->damage;

	if (ed->dmgtype == 252) {

		int mod = spellbonuses.ReduceFallDamage + itembonuses.ReduceFallDamage + aabonuses.ReduceFallDamage;

		damage -= damage * mod / 100;
	}

	if (damage < 0)
		damage = 31337;

	if (admin >= minStatusToAvoidFalling && GetGM()) {
		Message(13, "Your GM status protects you from %i points of type %i environmental damage.", ed->damage, ed->dmgtype);
		SetHP(GetHP() - 1);//needed or else the client wont acknowledge
		return;
	}
	else if (GetInvul()) {
		Message(13, "Your invuln status protects you from %i points of type %i environmental damage.", ed->damage, ed->dmgtype);
		SetHP(GetHP() - 1);//needed or else the client wont acknowledge
		return;
	}
	else if (zone->GetZoneID() == 183 || zone->GetZoneID() == 184) {
		// Hard coded tutorial and load zones for no fall damage
		return;
	}
	else {
		SetHP(GetHP() - (damage * RuleR(Character, EnvironmentDamageMulipliter)));

		/* EVENT_ENVIRONMENTAL_DAMAGE */
		int final_damage = (damage * RuleR(Character, EnvironmentDamageMulipliter));
		char buf[24];
		snprintf(buf, 23, "%u %u %i", ed->damage, ed->dmgtype, final_damage);
		parse->EventPlayer(EVENT_ENVIRONMENTAL_DAMAGE, this, buf, 0);
	}

	if (GetHP() <= 0) {
		mod_client_death_env();
		Death(0, 32000, SPELL_UNKNOWN, EQEmu::skills::SkillHandtoHand);
	}
	SendHPUpdate();
	return;
}

void Client::Handle_OP_FaceChange(const EQApplicationPacket *app)
{
	if (app->size != sizeof(FaceChange_Struct)) {
		Log(Logs::General, Logs::Error, "Invalid size for OP_FaceChange: Expected: %i, Got: %i",
			sizeof(FaceChange_Struct), app->size);
		return;
	}

	// Notify other clients in zone
	entity_list.QueueClients(this, app, false);

	FaceChange_Struct* fc = (FaceChange_Struct*)app->pBuffer;
	m_pp.haircolor = fc->haircolor;
	m_pp.beardcolor = fc->beardcolor;
	m_pp.eyecolor1 = fc->eyecolor1;
	m_pp.eyecolor2 = fc->eyecolor2;
	m_pp.hairstyle = fc->hairstyle;
	m_pp.face = fc->face;
	m_pp.beard = fc->beard;
	m_pp.drakkin_heritage = fc->drakkin_heritage;
	m_pp.drakkin_tattoo = fc->drakkin_tattoo;
	m_pp.drakkin_details = fc->drakkin_details;
	Save();
	Message_StringID(13, FACE_ACCEPTED);
	//Message(13, "Facial features updated.");
	return;
}

void Client::Handle_OP_FeignDeath(const EQApplicationPacket *app)
{
	if (GetClass() != MONK)
		return;
	if (!p_timers.Expired(&database, pTimerFeignDeath, false)) {
		Message(13, "Ability recovery time not yet met.");
		return;
	}

	int reuse = FeignDeathReuseTime;
	reuse -= GetSkillReuseTime(EQEmu::skills::SkillFeignDeath);

	if (reuse < 1)
		reuse = 1;

	p_timers.Start(pTimerFeignDeath, reuse - 1);

	//BreakInvis();

	uint16 primfeign = GetSkill(EQEmu::skills::SkillFeignDeath);
	uint16 secfeign = GetSkill(EQEmu::skills::SkillFeignDeath);
	if (primfeign > 100) {
		primfeign = 100;
		secfeign = secfeign - 100;
		secfeign = secfeign / 2;
	}
	else
		secfeign = 0;

	uint16 totalfeign = primfeign + secfeign;
	if (zone->random.Real(0, 160) > totalfeign) {
		SetFeigned(false);
		entity_list.MessageClose_StringID(this, false, 200, 10, STRING_FEIGNFAILED, GetName());
	}
	else {
		SetFeigned(true);
	}

	CheckIncreaseSkill(EQEmu::skills::SkillFeignDeath, nullptr, 5);
	return;
}

void Client::Handle_OP_FindPersonRequest(const EQApplicationPacket *app)
{
	if (app->size != sizeof(FindPersonRequest_Struct))
		printf("Error in FindPersonRequest_Struct. Expected size of: %zu, but got: %i\n", sizeof(FindPersonRequest_Struct), app->size);
	else {
		FindPersonRequest_Struct* t = (FindPersonRequest_Struct*)app->pBuffer;

		std::vector<FindPerson_Point> points;
		Mob* target = entity_list.GetMob(t->npc_id);

		if (target == nullptr) {
			//empty length packet == not found.
			EQApplicationPacket outapp(OP_FindPersonReply, 0);
			QueuePacket(&outapp);
			return;
		}

		if (!RuleB(Pathing, Find) && RuleB(Bazaar, EnableWarpToTrader) && target->IsClient() && (target->CastToClient()->Trader ||
			target->CastToClient()->Buyer)) {
			Message(15, "Moving you to Trader %s", target->GetName());
			MovePC(zone->GetZoneID(), zone->GetInstanceID(), target->GetX(), target->GetY(), target->GetZ(), 0.0f);
		}

		if (!RuleB(Pathing, Find) || !zone->pathing)
		{
			//fill in the path array...
			//
			points.resize(2);
			points[0].x = GetX();
			points[0].y = GetY();
			points[0].z = GetZ();
			points[1].x = target->GetX();
			points[1].y = target->GetY();
			points[1].z = target->GetZ();
		}
		else
		{
			glm::vec3 Start(GetX(), GetY(), GetZ() + (GetSize() < 6.0 ? 6 : GetSize()) * HEAD_POSITION);
			glm::vec3 End(target->GetX(), target->GetY(), target->GetZ() + (target->GetSize() < 6.0 ? 6 : target->GetSize()) * HEAD_POSITION);

			if (!zone->zonemap->LineIntersectsZone(Start, End, 1.0f, nullptr) && zone->pathing->NoHazards(Start, End))
			{
				points.resize(2);
				points[0].x = Start.x;
				points[0].y = Start.y;
				points[0].z = Start.z;

				points[1].x = End.x;
				points[1].y = End.y;
				points[1].z = End.z;

			}
			else
			{
				std::deque<int> pathlist = zone->pathing->FindRoute(Start, End);

				if (pathlist.empty())
				{
					EQApplicationPacket outapp(OP_FindPersonReply, 0);
					QueuePacket(&outapp);
					return;
				}

				//the client seems to have issues with packets larger than this
				if (pathlist.size() > 36)
				{
					EQApplicationPacket outapp(OP_FindPersonReply, 0);
					QueuePacket(&outapp);
					return;
				}

				// Live appears to send the points in this order:
				// Final destination.
				// Current Position.
				// rest of the points.
				FindPerson_Point p;

				int PointNumber = 0;

				bool LeadsToTeleporter = false;

				glm::vec3 v = zone->pathing->GetPathNodeCoordinates(pathlist.back());

				p.x = v.x;
				p.y = v.y;
				p.z = v.z;
				points.push_back(p);

				p.x = GetX();
				p.y = GetY();
				p.z = GetZ();
				points.push_back(p);

				for (auto Iterator = pathlist.begin(); Iterator != pathlist.end(); ++Iterator)
				{
					if ((*Iterator) == -1) // Teleporter
					{
						LeadsToTeleporter = true;
						break;
					}

					glm::vec3 v = zone->pathing->GetPathNodeCoordinates((*Iterator), false);
					p.x = v.x;
					p.y = v.y;
					p.z = v.z;
					points.push_back(p);
					++PointNumber;
				}

				if (!LeadsToTeleporter)
				{
					p.x = target->GetX();
					p.y = target->GetY();
					p.z = target->GetZ();

					points.push_back(p);
				}

			}
		}

		SendPathPacket(points);
	}
	return;
}

void Client::Handle_OP_Fishing(const EQApplicationPacket *app)
{
	if (!p_timers.Expired(&database, pTimerFishing, false)) {
		Message(13, "Ability recovery time not yet met.");
		return;
	}

	if (CanFish()) {
		parse->EventPlayer(EVENT_FISH_START, this, "", 0);

		//these will trigger GoFish() after a delay if we're able to actually fish, and if not, we won't stop the client from trying again immediately (although we may need to tell it to repop the button)
		p_timers.Start(pTimerFishing, FishingReuseTime - 1);
		fishing_timer.Start();
	}
	return;
	// Changes made based on Bobs work on foraging. Now can set items in the forage database table to
	// forage for.
}

void Client::Handle_OP_Forage(const EQApplicationPacket *app)
{

	if (!p_timers.Expired(&database, pTimerForaging, false)) {
		Message(13, "Ability recovery time not yet met.");
		return;
	}
	p_timers.Start(pTimerForaging, ForagingReuseTime - 1);

	ForageItem();

	return;
}

void Client::Handle_OP_FriendsWho(const EQApplicationPacket *app)
{
	char *FriendsString = (char*)app->pBuffer;
	FriendsWho(FriendsString);
	return;
}

void Client::Handle_OP_GetGuildMOTD(const EQApplicationPacket *app)
{
	Log(Logs::Detail, Logs::Guilds, "Received OP_GetGuildMOTD");

	SendGuildMOTD(true);

	if (IsInAGuild())
	{
		SendGuildURL();
		SendGuildChannel();
	}
}

void Client::Handle_OP_GetGuildsList(const EQApplicationPacket *app)
{
	Log(Logs::Detail, Logs::Guilds, "Received OP_GetGuildsList");

	SendGuildList();
}

void Client::Handle_OP_GMBecomeNPC(const EQApplicationPacket *app)
{
	if (this->Admin() < minStatusToUseGMCommands) {
		Message(13, "Your account has been reported for hacking.");
		database.SetHackerFlag(this->account_name, this->name, "/becomenpc");
		return;
	}
	if (app->size != sizeof(BecomeNPC_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_GMBecomeNPC, size=%i, expected %i", app->size, sizeof(BecomeNPC_Struct));
		return;
	}
	//entity_list.QueueClients(this, app, false);
	BecomeNPC_Struct* bnpc = (BecomeNPC_Struct*)app->pBuffer;

	Mob* cli = (Mob*)entity_list.GetMob(bnpc->id);
	if (cli == 0)
		return;

	if (cli->IsClient())
		cli->CastToClient()->QueuePacket(app);
	cli->SendAppearancePacket(AT_NPCName, 1, true);
	cli->CastToClient()->SetBecomeNPC(true);
	cli->CastToClient()->SetBecomeNPCLevel(bnpc->maxlevel);
	cli->Message_StringID(0, TOGGLE_OFF);
	cli->CastToClient()->tellsoff = true;
	//TODO: Make this toggle a BecomeNPC flag so that it gets updated when people zone in as well; Make combat work with this.
	return;
}

void Client::Handle_OP_GMDelCorpse(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GMDelCorpse_Struct))
		return;
	if (this->Admin() < commandEditPlayerCorpses) {
		Message(13, "Your account has been reported for hacking.");
		database.SetHackerFlag(this->account_name, this->name, "/delcorpse");
		return;
	}
	GMDelCorpse_Struct* dc = (GMDelCorpse_Struct *)app->pBuffer;
	Mob* corpse = entity_list.GetMob(dc->corpsename);
	if (corpse == 0) {
		return;
	}
	if (corpse->IsCorpse() != true) {
		return;
	}
	corpse->CastToCorpse()->Delete();
	std::cout << name << " deleted corpse " << dc->corpsename << std::endl;
	Message(13, "Corpse %s deleted.", dc->corpsename);
	return;
}

void Client::Handle_OP_GMEmoteZone(const EQApplicationPacket *app)
{
	if (this->Admin() < minStatusToUseGMCommands) {
		Message(13, "Your account has been reported for hacking.");
		database.SetHackerFlag(this->account_name, this->name, "/emote");
		return;
	}
	if (app->size != sizeof(GMEmoteZone_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_GMEmoteZone, size=%i, expected %i", app->size, sizeof(GMEmoteZone_Struct));
		return;
	}
	GMEmoteZone_Struct* gmez = (GMEmoteZone_Struct*)app->pBuffer;
	char* newmessage = nullptr;
	if (strstr(gmez->text, "^") == 0)
		entity_list.Message(0, 15, gmez->text);
	else {
		for (newmessage = strtok((char*)gmez->text, "^"); newmessage != nullptr; newmessage = strtok(nullptr, "^"))
			entity_list.Message(0, 15, newmessage);
	}
	return;
}

void Client::Handle_OP_GMEndTraining(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GMTrainEnd_Struct)) {
		Log(Logs::General, Logs::None, "Size mismatch in OP_GMEndTraining expected %i got %i", sizeof(GMTrainEnd_Struct), app->size);
		DumpPacket(app);
		return;
	}
	OPGMEndTraining(app);
	return;
}

void Client::Handle_OP_GMFind(const EQApplicationPacket *app)
{
	if (this->Admin() < minStatusToUseGMCommands) {
		Message(13, "Your account has been reported for hacking.");
		database.SetHackerFlag(this->account_name, this->name, "/find");
		return;
	}
	if (app->size != sizeof(GMSummon_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_GMFind, size=%i, expected %i", app->size, sizeof(GMSummon_Struct));
		return;
	}
	//Break down incoming
	GMSummon_Struct* request = (GMSummon_Struct*)app->pBuffer;
	//Create a new outgoing
	auto outapp = new EQApplicationPacket(OP_GMFind, sizeof(GMSummon_Struct));
	GMSummon_Struct* foundplayer = (GMSummon_Struct*)outapp->pBuffer;
	//Copy the constants
	strcpy(foundplayer->charname, request->charname);
	strcpy(foundplayer->gmname, request->gmname);
	//Check if the NPC exits intrazone...
	Mob* gt = entity_list.GetMob(request->charname);
	if (gt != 0) {
		foundplayer->success = 1;
		foundplayer->x = (int32)gt->GetX();
		foundplayer->y = (int32)gt->GetY();

		foundplayer->z = (int32)gt->GetZ();
		foundplayer->zoneID = zone->GetZoneID();
	}
	//Send the packet...
	FastQueuePacket(&outapp);
	return;
}

void Client::Handle_OP_GMGoto(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GMSummon_Struct)) {
		std::cout << "Wrong size on OP_GMGoto. Got: " << app->size << ", Expected: " << sizeof(GMSummon_Struct) << std::endl;
		return;
	}
	if (this->Admin() < minStatusToUseGMCommands) {
		Message(13, "Your account has been reported for hacking.");
		database.SetHackerFlag(this->account_name, this->name, "/goto");
		return;
	}
	GMSummon_Struct* gmg = (GMSummon_Struct*)app->pBuffer;
	Mob* gt = entity_list.GetMob(gmg->charname);
	if (gt != nullptr) {
		this->MovePC(zone->GetZoneID(), zone->GetInstanceID(), gt->GetX(), gt->GetY(), gt->GetZ(), gt->GetHeading());
	}
	else if (!worldserver.Connected())
		Message(0, "Error: World server disconnected.");
	else {
		auto pack = new ServerPacket(ServerOP_GMGoto, sizeof(ServerGMGoto_Struct));
		memset(pack->pBuffer, 0, pack->size);
		ServerGMGoto_Struct* wsgmg = (ServerGMGoto_Struct*)pack->pBuffer;
		strcpy(wsgmg->myname, this->GetName());
		strcpy(wsgmg->gotoname, gmg->charname);
		wsgmg->admin = admin;
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
	return;
}

void Client::Handle_OP_GMHideMe(const EQApplicationPacket *app)
{
	if (this->Admin() < minStatusToUseGMCommands) {
		Message(13, "Your account has been reported for hacking.");
		database.SetHackerFlag(this->account_name, this->name, "/hideme");
		return;
	}
	if (app->size != sizeof(SpawnAppearance_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_GMHideMe, size=%i, expected %i", app->size, sizeof(SpawnAppearance_Struct));
		return;
	}
	SpawnAppearance_Struct* sa = (SpawnAppearance_Struct*)app->pBuffer;
	Message(13, "#: %i, %i", sa->type, sa->parameter);
	SetHideMe(!sa->parameter);
	return;

}

void Client::Handle_OP_GMKick(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GMKick_Struct))
		return;
	if (this->Admin() < minStatusToKick) {
		Message(13, "Your account has been reported for hacking.");
		database.SetHackerFlag(this->account_name, this->name, "/kick");
		return;
	}
	GMKick_Struct* gmk = (GMKick_Struct *)app->pBuffer;

	Client* client = entity_list.GetClientByName(gmk->name);
	if (client == 0) {
		if (!worldserver.Connected())
			Message(0, "Error: World server disconnected");
		else {
			auto pack = new ServerPacket(ServerOP_KickPlayer, sizeof(ServerKickPlayer_Struct));
			ServerKickPlayer_Struct* skp = (ServerKickPlayer_Struct*)pack->pBuffer;
			strcpy(skp->adminname, gmk->gmname);
			strcpy(skp->name, gmk->name);
			skp->adminrank = this->Admin();
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
	}
	else {
		entity_list.QueueClients(this, app);
		//client->Kick();
	}
	return;
}

void Client::Handle_OP_GMKill(const EQApplicationPacket *app)
{
	if (this->Admin() < minStatusToUseGMCommands) {
		Message(13, "Your account has been reported for hacking.");
		database.SetHackerFlag(this->account_name, this->name, "/kill");
		return;
	}
	if (app->size != sizeof(GMKill_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_GMKill, size=%i, expected %i", app->size, sizeof(GMKill_Struct));
		return;
	}
	GMKill_Struct* gmk = (GMKill_Struct *)app->pBuffer;
	Mob* obj = entity_list.GetMob(gmk->name);
	Client* client = entity_list.GetClientByName(gmk->name);
	if (obj != 0) {
		if (client != 0) {
			entity_list.QueueClients(this, app);
		}
		else {
			obj->Kill();
		}
	}
	else {
		if (!worldserver.Connected())
			Message(0, "Error: World server disconnected");
		else {
			auto pack = new ServerPacket(ServerOP_KillPlayer, sizeof(ServerKillPlayer_Struct));
			ServerKillPlayer_Struct* skp = (ServerKillPlayer_Struct*)pack->pBuffer;
			strcpy(skp->gmname, gmk->gmname);
			strcpy(skp->target, gmk->name);
			skp->admin = this->Admin();
			worldserver.SendPacket(pack);
			safe_delete(pack);
		}
	}
	return;
}

void Client::Handle_OP_GMLastName(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GMLastName_Struct)) {
		std::cout << "Wrong size on OP_GMLastName. Got: " << app->size << ", Expected: " << sizeof(GMLastName_Struct) << std::endl;
		return;
	}
	GMLastName_Struct* gmln = (GMLastName_Struct*)app->pBuffer;
	if (strlen(gmln->lastname) >= 64) {
		Message(13, "/LastName: New last name too long. (max=63)");
	}
	else {
		Client* client = entity_list.GetClientByName(gmln->name);
		if (client == 0) {
			Message(13, "/LastName: %s not found", gmln->name);
		}
		else {
			if (this->Admin() < minStatusToUseGMCommands) {
				Message(13, "Your account has been reported for hacking.");
				database.SetHackerFlag(client->account_name, client->name, "/lastname");
				return;
			}
			else

				client->ChangeLastName(gmln->lastname);
		}
		gmln->unknown[0] = 1;
		gmln->unknown[1] = 1;
		gmln->unknown[2] = 1;
		gmln->unknown[3] = 1;
		entity_list.QueueClients(this, app, false);
	}
	return;
}

void Client::Handle_OP_GMNameChange(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GMName_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_GMNameChange, size=%i, expected %i", app->size, sizeof(GMName_Struct));
		return;
	}
	const GMName_Struct* gmn = (const GMName_Struct *)app->pBuffer;
	if (this->Admin() < minStatusToUseGMCommands) {
		Message(13, "Your account has been reported for hacking.");
		database.SetHackerFlag(this->account_name, this->name, "/name");
		return;
	}
	Client* client = entity_list.GetClientByName(gmn->oldname);
	Log(Logs::General, Logs::Status, "GM(%s) changeing players name. Old:%s New:%s", GetName(), gmn->oldname, gmn->newname);
	bool usedname = database.CheckUsedName((const char*)gmn->newname);
	if (client == 0) {
		Message(13, "%s not found for name change. Operation failed!", gmn->oldname);
		return;
	}
	if ((strlen(gmn->newname) > 63) || (strlen(gmn->newname) == 0)) {
		Message(13, "Invalid number of characters in new name (%s).", gmn->newname);
		return;
	}
	if (!usedname) {
		Message(13, "%s is already in use. Operation failed!", gmn->newname);
		return;

	}
	database.UpdateName(gmn->oldname, gmn->newname);
	strcpy(client->name, gmn->newname);
	client->Save();

	if (gmn->badname == 1) {
		database.AddToNameFilter(gmn->oldname);
	}
	EQApplicationPacket* outapp = app->Copy();
	GMName_Struct* gmn2 = (GMName_Struct*)outapp->pBuffer;
	gmn2->unknown[0] = 1;
	gmn2->unknown[1] = 1;
	gmn2->unknown[2] = 1;
	entity_list.QueueClients(this, outapp, false);
	safe_delete(outapp);
	UpdateWho();
	return;
}

void Client::Handle_OP_GMSearchCorpse(const EQApplicationPacket *app)
{
	// Could make this into a rule, although there is a hard limit since we are using a popup, of 4096 bytes that can
	// be displayed in the window, including all the HTML formatting tags.
	//
	const int maxResults = 10;

	if (app->size < sizeof(GMSearchCorpse_Struct))
	{
		Log(Logs::General, Logs::None, "OP_GMSearchCorpse size lower than expected: got %u expected at least %u",
			app->size, sizeof(GMSearchCorpse_Struct));
		DumpPacket(app);
		return;
	}

	GMSearchCorpse_Struct *gmscs = (GMSearchCorpse_Struct *)app->pBuffer;
	gmscs->Name[63] = '\0';

	auto escSearchString = new char[129];
	database.DoEscapeString(escSearchString, gmscs->Name, strlen(gmscs->Name));

	std::string query = StringFormat("SELECT charname, zone_id, x, y, z, time_of_death, is_rezzed, is_buried "
		"FROM character_corpses WheRE charname LIKE '%%%s%%' ORDER BY charname LIMIT %i",
		escSearchString, maxResults);
	safe_delete_array(escSearchString);
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	if (results.RowCount() == 0)
		return;

	if (results.RowCount() == maxResults)
		Message(clientMessageError, "Your search found too many results; some are not displayed.");
	else
		Message(clientMessageYellow, "There are %i corpse(s) that match the search string '%s'.", results.RowCount(), gmscs->Name);

	char charName[64], time_of_death[20];

	std::string popupText = "<table><tr><td>Name</td><td>Zone</td><td>X</td><td>Y</td><td>Z</td><td>Date</td><td>"
		"Rezzed</td><td>Buried</td></tr><tr><td>&nbsp</td><td></td><td></td><td></td><td></td><td>"
		"</td><td></td><td></td></tr>";

	for (auto row = results.begin(); row != results.end(); ++row) {

		strn0cpy(charName, row[0], sizeof(charName));

		uint32 ZoneID = atoi(row[1]);
		float CorpseX = atof(row[2]);
		float CorpseY = atof(row[3]);
		float CorpseZ = atof(row[4]);

		strn0cpy(time_of_death, row[5], sizeof(time_of_death));

		bool corpseRezzed = atoi(row[6]);
		bool corpseBuried = atoi(row[7]);

		popupText += StringFormat("<tr><td>%s</td><td>%s</td><td>%8.0f</td><td>%8.0f</td><td>%8.0f</td><td>%s</td><td>%s</td><td>%s</td></tr>",
			charName, StaticGetZoneName(ZoneID), CorpseX, CorpseY, CorpseZ, time_of_death,
			corpseRezzed ? "Yes" : "No", corpseBuried ? "Yes" : "No");

		if (popupText.size() > 4000) {
			Message(clientMessageError, "Unable to display all the results.");
			break;
		}

	}

	popupText += "</table>";

	SendPopupToClient("Corpses", popupText.c_str());

}

void Client::Handle_OP_GMServers(const EQApplicationPacket *app)
{
	if (!worldserver.Connected())
		Message(0, "Error: World server disconnected");
	else {
		auto pack = new ServerPacket(ServerOP_ZoneStatus, strlen(this->GetName()) + 2);
		memset(pack->pBuffer, (uint8)admin, 1);
		strcpy((char *)&pack->pBuffer[1], this->GetName());
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
	return;
}

void Client::Handle_OP_GMSummon(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GMSummon_Struct)) {
		std::cout << "Wrong size on OP_GMSummon. Got: " << app->size << ", Expected: " << sizeof(GMSummon_Struct) << std::endl;
		return;
	}
	OPGMSummon(app);
	return;
}

void Client::Handle_OP_GMToggle(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GMToggle_Struct)) {
		std::cout << "Wrong size on OP_GMToggle. Got: " << app->size << ", Expected: " << sizeof(GMToggle_Struct) << std::endl;
		return;
	}
	if (this->Admin() < minStatusToUseGMCommands) {
		Message(13, "Your account has been reported for hacking.");
		database.SetHackerFlag(this->account_name, this->name, "/toggle");
		return;
	}
	GMToggle_Struct *ts = (GMToggle_Struct *)app->pBuffer;
	if (ts->toggle == 0) {
		this->Message_StringID(0, TOGGLE_OFF);
		//Message(0, "Turning tells OFF");
		tellsoff = true;
	}
	else if (ts->toggle == 1) {
		//Message(0, "Turning tells ON");
		this->Message_StringID(0, TOGGLE_ON);
		tellsoff = false;
	}
	else {
		Message(0, "Unkown value in /toggle packet");
	}
	UpdateWho();
	return;
}

void Client::Handle_OP_GMTraining(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GMTrainee_Struct)) {
		Log(Logs::General, Logs::None, "Size mismatch in OP_GMTraining expected %i got %i", sizeof(GMTrainee_Struct), app->size);
		DumpPacket(app);
		return;
	}
	OPGMTraining(app);
	return;
}

void Client::Handle_OP_GMTrainSkill(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GMSkillChange_Struct)) {
		Log(Logs::General, Logs::None, "Size mismatch in OP_GMTrainSkill expected %i got %i", sizeof(GMSkillChange_Struct), app->size);
		DumpPacket(app);
		return;
	}
	OPGMTrainSkill(app);
	return;
}

void Client::Handle_OP_GMZoneRequest(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GMZoneRequest_Struct)) {
		std::cout << "Wrong size on OP_GMZoneRequest. Got: " << app->size << ", Expected: " << sizeof(GMZoneRequest_Struct) << std::endl;
		return;
	}
	if (this->Admin() < minStatusToBeGM) {
		Message(13, "Your account has been reported for hacking.");
		database.SetHackerFlag(this->account_name, this->name, "/zone");
		return;
	}

	GMZoneRequest_Struct* gmzr = (GMZoneRequest_Struct*)app->pBuffer;
	float tarx = -1, tary = -1, tarz = -1;

	int16 minstatus = 0;
	uint8 minlevel = 0;
	char tarzone[32];
	uint16 zid = gmzr->zone_id;
	if (gmzr->zone_id == 0)
		zid = zonesummon_id;
	const char * zname = database.GetZoneName(zid);
	if (zname == nullptr)
		tarzone[0] = 0;
	else
		strcpy(tarzone, zname);

	// this both loads the safe points and does a sanity check on zone name
	if (!database.GetSafePoints(tarzone, 0, &tarx, &tary, &tarz, &minstatus, &minlevel)) {
		tarzone[0] = 0;
	}

	auto outapp = new EQApplicationPacket(OP_GMZoneRequest, sizeof(GMZoneRequest_Struct));
	GMZoneRequest_Struct* gmzr2 = (GMZoneRequest_Struct*)outapp->pBuffer;
	strcpy(gmzr2->charname, this->GetName());
	gmzr2->zone_id = gmzr->zone_id;
	gmzr2->x = tarx;
	gmzr2->y = tary;
	gmzr2->z = tarz;
	// Next line stolen from ZoneChange as well... - This gives us a nicer message than the normal "zone is down" message...
	if (tarzone[0] != 0 && admin >= minstatus && GetLevel() >= minlevel)
		gmzr2->success = 1;
	else {
		std::cout << "GetZoneSafeCoords failed. zoneid = " << gmzr->zone_id << "; czone = " << zone->GetZoneID() << std::endl;
		gmzr2->success = 0;
	}

	QueuePacket(outapp);
	safe_delete(outapp);
	return;
}

void Client::Handle_OP_GMZoneRequest2(const EQApplicationPacket *app)
{
	if (this->Admin() < minStatusToBeGM) {
		Message(13, "Your account has been reported for hacking.");
		database.SetHackerFlag(this->account_name, this->name, "/zone");
		return;
	}
	if (app->size < sizeof(uint32)) {
		Log(Logs::General, Logs::Error, "OP size error: OP_GMZoneRequest2 expected:%i got:%i", sizeof(uint32), app->size);
		return;
	}

	uint32 zonereq = *((uint32 *)app->pBuffer);
	GoToSafeCoords(zonereq, 0);
	return;
}

void Client::Handle_OP_GroupAcknowledge(const EQApplicationPacket *app)
{
	return;
}

void Client::Handle_OP_GroupCancelInvite(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GroupCancel_Struct)) {
		Log(Logs::General, Logs::Error, "Invalid size for OP_GroupCancelInvite: Expected: %i, Got: %i",
			sizeof(GroupCancel_Struct), app->size);
		return;
	}

	GroupCancel_Struct* gf = (GroupCancel_Struct*)app->pBuffer;
	Mob* inviter = entity_list.GetClientByName(gf->name1);

	if (inviter != nullptr)
	{
		if (inviter->IsClient())
			inviter->CastToClient()->QueuePacket(app);
	}
	else
	{
		auto pack = new ServerPacket(ServerOP_GroupCancelInvite, sizeof(GroupCancel_Struct));
		memcpy(pack->pBuffer, gf, sizeof(GroupCancel_Struct));
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}

	if (!GetMerc())
	{
		database.SetGroupID(GetName(), 0, CharacterID(), false);
	}
	return;
}

void Client::Handle_OP_GroupDelete(const EQApplicationPacket *app)
{
	//should check for leader, only they should be able to do this..
	Group* group = GetGroup();
	if (group)
		group->DisbandGroup();

	if (LFP)
		UpdateLFP();

	return;
}

void Client::Handle_OP_GroupDisband(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GroupGeneric_Struct)) {
		Log(Logs::General, Logs::Error, "Invalid size for GroupGeneric_Struct: Expected: %i, Got: %i",
			sizeof(GroupGeneric_Struct), app->size);
		return;
	}

	Log(Logs::General, Logs::None, "Member Disband Request from %s\n", GetName());

	GroupGeneric_Struct* gd = (GroupGeneric_Struct*)app->pBuffer;

	Raid *raid = entity_list.GetRaidByClient(this);
	if (raid)
	{
		Mob* memberToDisband = nullptr;

		if (!raid->IsGroupLeader(GetName()))
			memberToDisband = this;
		else
			memberToDisband = GetTarget();

		if (!memberToDisband)
			memberToDisband = entity_list.GetMob(gd->name2);

		if (!memberToDisband)
			memberToDisband = this;

		if (!memberToDisband->IsClient())
			return;

		//we have a raid.. see if we're in a raid group
		uint32 grp = raid->GetGroup(memberToDisband->GetName());
		bool wasGrpLdr = raid->members[raid->GetPlayerIndex(memberToDisband->GetName())].IsGroupLeader;
		if (grp < 12) {
			if (wasGrpLdr) {
				raid->SetGroupLeader(memberToDisband->GetName(), false);
				for (int x = 0; x < MAX_RAID_MEMBERS; x++)
				{
					if (raid->members[x].GroupNumber == grp)
					{
						if (strlen(raid->members[x].membername) > 0 && strcmp(raid->members[x].membername, memberToDisband->GetName()) != 0)
						{
							raid->SetGroupLeader(raid->members[x].membername);
							break;
						}
					}
				}
			}
			raid->MoveMember(memberToDisband->GetName(), 0xFFFFFFFF);
			raid->GroupUpdate(grp); //break
									//raid->SendRaidGroupRemove(memberToDisband->GetName(), grp);
									//raid->SendGroupUpdate(memberToDisband->CastToClient());
			raid->SendGroupDisband(memberToDisband->CastToClient());
		}
		//we're done
		return;
	}

	Group* group = GetGroup();

	if (!group)
		return;

#ifdef BOTS
	// this block is necessary to allow more control over controlling how bots are zoned or camped.
	if (Bot::GroupHasBot(group)) {
		if (group->IsLeader(this)) {
			if ((GetTarget() == 0 || GetTarget() == this) || (group->GroupCount() < 3)) {
				Bot::ProcessBotGroupDisband(this, std::string());
			}
			else {
				Mob* tempMember = entity_list.GetMob(gd->name1); //Name1 is the target you are disbanding
				if (tempMember && tempMember->IsBot()) {
					tempMember->CastToBot()->RemoveBotFromGroup(tempMember->CastToBot(), group);
					if (LFP)
					{
						// If we are looking for players, update to show we are on our own now.
						UpdateLFP();
					}
					return; //No need to continue from here we were removing a bot from party
				}
			}
		}
	}

	group = GetGroup();
	if (!group) //We must recheck this here.. incase the final bot disbanded the party..otherwise we crash
		return;
#endif
	Mob* memberToDisband = GetTarget();

	if (!memberToDisband)
		memberToDisband = entity_list.GetMob(gd->name2);

	if (memberToDisband) {
		auto group2 = memberToDisband->GetGroup();
		if (group2 != group) // they're not in our group!
			memberToDisband = this;
	}

	if (group->GroupCount() < 3)
	{
		group->DisbandGroup();
		if (GetMerc())
			GetMerc()->Suspend();
	}
	else if (group->IsLeader(this) && GetTarget() == nullptr)
	{
		if (group->GroupCount() > 2 && GetMerc() && !GetMerc()->IsSuspended())
		{
			group->DisbandGroup();
			GetMerc()->MercJoinClientGroup();
		}
		else
		{
			group->DisbandGroup();
			if (GetMerc())
				GetMerc()->Suspend();
		}
	}
	else if (group->IsLeader(this) && (GetTarget() == this || memberToDisband == this))
	{
		LeaveGroup();
		if (GetMerc() && !GetMerc()->IsSuspended())
		{
			GetMerc()->MercJoinClientGroup();
		}
	}
	else
	{
		if (memberToDisband)
		{
			if (group->IsLeader(this))
			{
				// the group leader can kick other members out of the group...
				if (memberToDisband->IsClient())
				{
					group->DelMember(memberToDisband, false);
					Client* memberClient = memberToDisband->CastToClient();
					Merc* memberMerc = memberToDisband->CastToClient()->GetMerc();
					if (memberClient && memberMerc)
					{
						memberMerc->MercJoinClientGroup();
					}
				}
				else if (memberToDisband->IsMerc())
				{
					memberToDisband->CastToMerc()->Suspend();
				}
			}
			else
			{
				// ...but other members can only remove themselves
				group->DelMember(this, false);

				if (GetMerc() && !GetMerc()->IsSuspended())
				{
					GetMerc()->MercJoinClientGroup();
				}
			}
		}
		else
		{
			Log(Logs::General, Logs::Error, "Failed to remove player from group. Unable to find player named %s in player group", gd->name2);
		}
	}
	if (LFP)
	{
		// If we are looking for players, update to show we are on our own now.
		UpdateLFP();
	}

	return;
}

void Client::Handle_OP_GroupFollow(const EQApplicationPacket *app)
{
	Handle_OP_GroupFollow2(app);
}

void Client::Handle_OP_GroupFollow2(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GroupGeneric_Struct)) {
		Log(Logs::General, Logs::Error, "Invalid size for OP_GroupFollow: Expected: %i, Got: %i",
			sizeof(GroupGeneric_Struct), app->size);
		return;
	}

	if (LFP) {
		// If we were looking for players to start our own group, but we accept an invitation to another
		// group, turn LFP off.
		database.SetLFP(CharacterID(), false);
		worldserver.StopLFP(CharacterID());
	}

	GroupGeneric_Struct* gf = (GroupGeneric_Struct*)app->pBuffer;
	Mob* inviter = entity_list.GetClientByName(gf->name1);

	// Inviter and Invitee are in the same zone
	if (inviter != nullptr && inviter->IsClient())
	{
		if (GroupFollow(inviter->CastToClient()))
		{
			strn0cpy(gf->name1, inviter->GetName(), sizeof(gf->name1));
			strn0cpy(gf->name2, GetName(), sizeof(gf->name2));
			inviter->CastToClient()->QueuePacket(app);//notify inviter the client accepted
		}
	}
	else if (inviter == nullptr)
	{
		// Inviter is in another zone - Remove merc from group now if any
		LeaveGroup();

		auto pack = new ServerPacket(ServerOP_GroupFollow, sizeof(ServerGroupFollow_Struct));
		ServerGroupFollow_Struct *sgfs = (ServerGroupFollow_Struct *)pack->pBuffer;
		sgfs->CharacterID = CharacterID();
		strn0cpy(sgfs->gf.name1, gf->name1, sizeof(sgfs->gf.name1));
		strn0cpy(sgfs->gf.name2, gf->name2, sizeof(sgfs->gf.name2));
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void Client::Handle_OP_GroupInvite(const EQApplicationPacket *app)
{
	//this seems to be the initial invite to form a group
	Handle_OP_GroupInvite2(app);
}

void Client::Handle_OP_GroupInvite2(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GroupInvite_Struct)) {
		Log(Logs::General, Logs::Error, "Invalid size for OP_GroupInvite: Expected: %i, Got: %i",
			sizeof(GroupInvite_Struct), app->size);
		return;
	}

	GroupInvite_Struct* gis = (GroupInvite_Struct*)app->pBuffer;

	Mob *Invitee = entity_list.GetMob(gis->invitee_name);

	if (Invitee == this)
	{
		Message_StringID(clientMessageWhite, GROUP_INVITEE_SELF);
		return;
	}

	if (Invitee)
	{
		if (Invitee->IsClient())
		{
			if (Invitee->CastToClient()->MercOnlyOrNoGroup() && !Invitee->IsRaidGrouped())
			{
				if (app->GetOpcode() == OP_GroupInvite2)
				{
					//Make a new packet using all the same information but make sure it's a fixed GroupInvite opcode so we
					//Don't have to deal with GroupFollow2 crap.
					auto outapp =
						new EQApplicationPacket(OP_GroupInvite, sizeof(GroupInvite_Struct));
					memcpy(outapp->pBuffer, app->pBuffer, outapp->size);
					Invitee->CastToClient()->QueuePacket(outapp);
					safe_delete(outapp);
					return;
				}
				else
				{
					//The correct opcode, no reason to bother wasting time reconstructing the packet
					Invitee->CastToClient()->QueuePacket(app);
				}
			}
		}
#ifdef BOTS
		else if (Invitee->IsBot()) {
			Bot::ProcessBotGroupInvite(this, std::string(Invitee->GetName()));
		}
#endif
	}
	else
	{
		auto pack = new ServerPacket(ServerOP_GroupInvite, sizeof(GroupInvite_Struct));
		memcpy(pack->pBuffer, gis, sizeof(GroupInvite_Struct));
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
	return;
}

void Client::Handle_OP_GroupMakeLeader(const EQApplicationPacket *app)
{
	VERIFY_PACKET_LENGTH(OP_GroupMakeLeader, app, GroupMakeLeader_Struct);

	GroupMakeLeader_Struct *gmls = (GroupMakeLeader_Struct *)app->pBuffer;

	Mob* NewLeader = entity_list.GetClientByName(gmls->NewLeader);

	Group* g = GetGroup();

	if (NewLeader && g)
	{
		if (g->IsLeader(this))
			g->ChangeLeader(NewLeader);
		else {
			Log(Logs::General, Logs::None, "Group /makeleader request originated from non-leader member: %s", GetName());
			DumpPacket(app);
		}
	}
}

void Client::Handle_OP_GroupMentor(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GroupMentor_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_GroupMentor, size=%i, expected %i", app->size, sizeof(GroupMentor_Struct));
		DumpPacket(app);
		return;
	}
	GroupMentor_Struct *gms = (GroupMentor_Struct *)app->pBuffer;
	gms->name[63] = '\0';

	if (IsRaidGrouped()) {
		Raid *raid = GetRaid();
		if (!raid)
			return;
		uint32 group_id = raid->GetGroup(this);
		if (group_id > 11)
			return;
		if (strlen(gms->name))
			raid->SetGroupMentor(group_id, gms->percent, gms->name);
		else
			raid->ClearGroupMentor(group_id);
		return;
	}

	Group *group = GetGroup();
	if (!group)
		return;

	if (strlen(gms->name))
		group->SetGroupMentor(gms->percent, gms->name);
	else
		group->ClearGroupMentor();

	return;
}

void Client::Handle_OP_GroupRoles(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GroupRole_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_GroupRoles, size=%i, expected %i", app->size, sizeof(GroupRole_Struct));
		DumpPacket(app);
		return;
	}
	GroupRole_Struct *grs = (GroupRole_Struct*)app->pBuffer;

	Group *g = GetGroup();

	if (!g)
		return;

	switch (grs->RoleNumber)
	{
	case 1: //Main Tank
	{
		if (grs->Toggle)
			g->DelegateMainTank(grs->Name1, grs->Toggle);
		else
			g->UnDelegateMainTank(grs->Name1, grs->Toggle);
		break;
	}
	case 2: //Main Assist
	{
		if (grs->Toggle)
			g->DelegateMainAssist(grs->Name1, grs->Toggle);
		else
			g->UnDelegateMainAssist(grs->Name1, grs->Toggle);
		break;
	}
	case 3: //Puller
	{
		if (grs->Toggle)
			g->DelegatePuller(grs->Name1, grs->Toggle);
		else
			g->UnDelegatePuller(grs->Name1, grs->Toggle);
		break;
	}
	default:
		break;
	}
}

void Client::Handle_OP_GroupUpdate(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GroupUpdate_Struct))
	{
		Log(Logs::General, Logs::None, "Size mismatch on OP_GroupUpdate: got %u expected %u",
			app->size, sizeof(GroupUpdate_Struct));
		DumpPacket(app);
		return;
	}

	GroupUpdate_Struct* gu = (GroupUpdate_Struct*)app->pBuffer;

	switch (gu->action) {
	case groupActMakeLeader:
	{
		Mob* newleader = entity_list.GetClientByName(gu->membername[0]);
		Group* group = this->GetGroup();

		if (newleader && group) {
			// the client only sends this if it's the group leader, but check anyway
			if (group->IsLeader(this))
				group->ChangeLeader(newleader);
			else {
				Log(Logs::General, Logs::None, "Group /makeleader request originated from non-leader member: %s", GetName());
				DumpPacket(app);
			}
		}
		break;
	}

	default:
	{
		Log(Logs::General, Logs::None, "Received unhandled OP_GroupUpdate requesting action %u", gu->action);
		DumpPacket(app);
		return;
	}
	}
}

void Client::Handle_OP_GuildBank(const EQApplicationPacket *app)
{
	if (!GuildBanks)
		return;

	if ((int)zone->GetZoneID() != RuleI(World, GuildBankZoneID))
	{
		Message(13, "The Guild Bank is not available in this zone.");

		return;
	}

	if (app->size < sizeof(uint32)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_GuildBank, size=%i, expected %i", app->size, sizeof(uint32));
		DumpPacket(app);
		return;
	}

	char *Buffer = (char *)app->pBuffer;

	uint32 Action = VARSTRUCT_DECODE_TYPE(uint32, Buffer);
	uint32 sentAction = Action;

	if (!IsInAGuild())
	{
		Message(13, "You must be in a Guild to use the Guild Bank.");

		if (Action == GuildBankDeposit)
			GuildBankDepositAck(true, sentAction);
		else
			GuildBankAck();

		return;
	}

	if (!IsGuildBanker())
	{
		if ((Action != GuildBankDeposit) && (Action != GuildBankViewItem) && (Action != GuildBankWithdraw))
		{
			Log(Logs::General, Logs::Error, "Suspected hacking attempt on guild bank from %s", GetName());

			GuildBankAck();

			return;
		}
	}

	switch (Action)
	{
	case GuildBankPromote:
	{
		if (GuildBanks->IsAreaFull(GuildID(), GuildBankMainArea))
		{
			Message_StringID(13, GUILD_BANK_FULL);

			GuildBankDepositAck(true, sentAction);

			return;
		}

		GuildBankPromote_Struct *gbps = (GuildBankPromote_Struct*)app->pBuffer;

		int Slot = GuildBanks->Promote(GuildID(), gbps->Slot);

		if (Slot >= 0)
		{
			EQEmu::ItemInstance* inst = GuildBanks->GetItem(GuildID(), GuildBankMainArea, Slot, 1);

			if (inst)
			{
				Message_StringID(clientMessageWhite, GUILD_BANK_TRANSFERRED, inst->GetItem()->Name);
				safe_delete(inst);
			}
		}
		else
			Message(13, "Unexpected error while moving item into Guild Bank.");

		GuildBankAck();

		break;
	}

	case GuildBankViewItem:
	{
		GuildBankViewItem_Struct *gbvis = (GuildBankViewItem_Struct*)app->pBuffer;

		EQEmu::ItemInstance* inst = GuildBanks->GetItem(GuildID(), gbvis->Area, gbvis->SlotID, 1);

		if (!inst)
			break;

		SendItemPacket(0, inst, ItemPacketViewLink);

		safe_delete(inst);

		break;
	}

	case GuildBankDeposit:	// Deposit Item
	{
		if (GuildBanks->IsAreaFull(GuildID(), GuildBankDepositArea))
		{
			Message_StringID(13, GUILD_BANK_FULL);

			GuildBankDepositAck(true, sentAction);

			return;
		}

		EQEmu::ItemInstance *CursorItemInst = GetInv().GetItem(EQEmu::inventory::slotCursor);

		bool Allowed = true;

		if (!CursorItemInst)
		{
			Message(13, "No Item on the cursor.");

			GuildBankDepositAck(true, sentAction);

			return;
		}

		const EQEmu::ItemData* CursorItem = CursorItemInst->GetItem();

		if (!CursorItem->NoDrop || CursorItemInst->IsAttuned())
		{
			Allowed = false;
		}
		else if (CursorItemInst->IsNoneEmptyContainer())
		{
			Allowed = false;
		}
		else if (CursorItemInst->IsAugmented())
		{
			Allowed = false;
		}
		else if (CursorItem->NoRent == 0)
		{
			Allowed = false;
		}
		else if (CursorItem->LoreFlag && GuildBanks->HasItem(GuildID(), CursorItem->ID))
		{
			Allowed = false;
		}

		if (!Allowed)
		{
			Message_StringID(13, GUILD_BANK_CANNOT_DEPOSIT);
			GuildBankDepositAck(true, sentAction);

			return;
		}

		if (GuildBanks->AddItem(GuildID(), GuildBankDepositArea, CursorItem->ID, CursorItemInst->GetCharges(), GetName(), GuildBankBankerOnly, ""))
		{
			GuildBankDepositAck(false, sentAction);

			DeleteItemInInventory(EQEmu::inventory::slotCursor, 0, false);
		}

		break;
	}

	case GuildBankPermissions:
	{
		GuildBankPermissions_Struct *gbps = (GuildBankPermissions_Struct*)app->pBuffer;

		if (gbps->Permissions == 1)
			GuildBanks->SetPermissions(GuildID(), gbps->SlotID, gbps->Permissions, gbps->MemberName);
		else
			GuildBanks->SetPermissions(GuildID(), gbps->SlotID, gbps->Permissions, "");

		GuildBankAck();
		break;
	}

	case GuildBankWithdraw:
	{
		if (GetInv()[EQEmu::inventory::slotCursor])
		{
			Message_StringID(13, GUILD_BANK_EMPTY_HANDS);

			GuildBankAck();

			break;
		}

		GuildBankWithdrawItem_Struct *gbwis = (GuildBankWithdrawItem_Struct*)app->pBuffer;

		EQEmu::ItemInstance* inst = GuildBanks->GetItem(GuildID(), gbwis->Area, gbwis->SlotID, gbwis->Quantity);

		if (!inst)
		{
			GuildBankAck();

			break;
		}

		if (!IsGuildBanker() && !GuildBanks->AllowedToWithdraw(GuildID(), gbwis->Area, gbwis->SlotID, GetName()))
		{
			Log(Logs::General, Logs::Error, "Suspected attempted hack on the guild bank from %s", GetName());

			GuildBankAck();

			safe_delete(inst);

			break;
		}

		if (CheckLoreConflict(inst->GetItem()))
		{
			Message_StringID(13, DUP_LORE);

			GuildBankAck();

			safe_delete(inst);

			break;
		}

		if (gbwis->Quantity > 0)
		{
			PushItemOnCursor(*inst);

			SendItemPacket(EQEmu::inventory::slotCursor, inst, ItemPacketLimbo);

			GuildBanks->DeleteItem(GuildID(), gbwis->Area, gbwis->SlotID, gbwis->Quantity);
		}
		else
		{
			Message(0, "Unable to withdraw 0 quantity of %s", inst->GetItem()->Name);
		}

		safe_delete(inst);

		GuildBankAck();

		break;
	}

	case GuildBankSplitStacks:
	{
		if (GuildBanks->IsAreaFull(GuildID(), GuildBankMainArea))
			Message_StringID(13, GUILD_BANK_FULL);
		else
		{
			GuildBankWithdrawItem_Struct *gbwis = (GuildBankWithdrawItem_Struct*)app->pBuffer;

			GuildBanks->SplitStack(GuildID(), gbwis->SlotID, gbwis->Quantity);
		}

		GuildBankAck();

		break;
	}

	case GuildBankMergeStacks:
	{
		GuildBankWithdrawItem_Struct *gbwis = (GuildBankWithdrawItem_Struct*)app->pBuffer;

		GuildBanks->MergeStacks(GuildID(), gbwis->SlotID);

		GuildBankAck();

		break;
	}

	default:
	{
		Message(13, "Unexpected GuildBank action.");

		Log(Logs::General, Logs::Error, "Received unexpected guild bank action code %i from %s", Action, GetName());
	}
	}
}

void Client::Handle_OP_GuildCreate(const EQApplicationPacket *app)
{
	if (IsInAGuild())
	{
		Message(clientMessageError, "You are already in a guild!");
		return;
	}

	if (!RuleB(Guild, PlayerCreationAllowed))
	{
		Message(clientMessageError, "This feature is disabled on this server. Contact a GM or post on your server message boards to create a guild.");
		return;
	}

	if ((Admin() < RuleI(Guild, PlayerCreationRequiredStatus)) ||
		(GetLevel() < RuleI(Guild, PlayerCreationRequiredLevel)) ||
		(database.GetTotalTimeEntitledOnAccount(AccountID()) < (unsigned int)RuleI(Guild, PlayerCreationRequiredTime)))
	{
		Message(clientMessageError, "Your status, level or time playing on this account are insufficient to use this feature.");
		return;
	}

	// The Underfoot client Guild Creation window will only allow a guild name of <= around 30 characters, but the packet is 64 bytes. Sanity check the
	// name anway.
	//

	char *GuildName = (char *)app->pBuffer;
#ifdef DARWIN
#if __DARWIN_C_LEVEL < 200809L
	if (strlen(GuildName) > 60)
#else
	if (strnlen(GuildName, 64) > 60)
#endif // __DARWIN_C_LEVEL
#else
	if (strnlen(GuildName, 64) > 60)
#endif // DARWIN
	{
		Message(clientMessageError, "Guild name too long.");
		return;
	}

	for (unsigned int i = 0; i < strlen(GuildName); ++i)
	{
		if (!isalpha(GuildName[i]) && (GuildName[i] != ' '))
		{
			Message(clientMessageError, "Invalid character in Guild name.");
			return;
		}
	}

	int32 GuildCount = guild_mgr.DoesAccountContainAGuildLeader(AccountID());

	if (GuildCount >= RuleI(Guild, PlayerCreationLimit))
	{
		Message(clientMessageError, "You cannot create this guild because this account may only be leader of %i guilds.", RuleI(Guild, PlayerCreationLimit));
		return;
	}

	if (guild_mgr.GetGuildIDByName(GuildName) != GUILD_NONE)
	{
		Message_StringID(clientMessageError, GUILD_NAME_IN_USE);
		return;
	}

	uint32 NewGuildID = guild_mgr.CreateGuild(GuildName, CharacterID());

	Log(Logs::Detail, Logs::Guilds, "%s: Creating guild %s with leader %d via UF+ GUI. It was given id %lu.", GetName(),
		GuildName, CharacterID(), (unsigned long)NewGuildID);

	if (NewGuildID == GUILD_NONE)
		Message(clientMessageError, "Guild creation failed.");
	else
	{
		if (!guild_mgr.SetGuild(CharacterID(), NewGuildID, GUILD_LEADER))
			Message(clientMessageError, "Unable to set guild leader's guild in the database. Contact a GM.");
		else
		{
			Message(clientMessageYellow, "You are now the leader of %s", GuildName);

			if (zone->GetZoneID() == RuleI(World, GuildBankZoneID) && GuildBanks)
				GuildBanks->SendGuildBank(this);
			SendGuildRanks();
		}
	}
}

void Client::Handle_OP_GuildDelete(const EQApplicationPacket *app)
{
	Log(Logs::Detail, Logs::Guilds, "Received OP_GuildDelete");

	if (!IsInAGuild() || !guild_mgr.IsGuildLeader(GuildID(), CharacterID()))
		Message(0, "You are not a guild leader or not in a guild.");
	else {
		Log(Logs::Detail, Logs::Guilds, "Deleting guild %s (%d)", guild_mgr.GetGuildName(GuildID()), GuildID());
		if (!guild_mgr.DeleteGuild(GuildID()))
			Message(0, "Guild delete failed.");
		else {
			Message(0, "Guild successfully deleted.");
		}
	}
}

void Client::Handle_OP_GuildDemote(const EQApplicationPacket *app)
{
	Log(Logs::Detail, Logs::Guilds, "Received OP_GuildDemote");

	if (app->size != sizeof(GuildDemoteStruct)) {
		Log(Logs::Detail, Logs::Guilds, "Error: app size of %i != size of GuildDemoteStruct of %i\n", app->size, sizeof(GuildDemoteStruct));
		return;
	}

	if (!IsInAGuild())
		Message(0, "Error: You arent in a guild!");
	else if (!guild_mgr.CheckPermission(GuildID(), GuildRank(), GUILD_DEMOTE))
		Message(0, "You dont have permission to invite.");
	else if (!worldserver.Connected())
		Message(0, "Error: World server disconnected");
	else {
		GuildDemoteStruct* demote = (GuildDemoteStruct*)app->pBuffer;

		CharGuildInfo gci;
		if (!guild_mgr.GetCharInfo(demote->target, gci)) {
			Message(0, "Unable to find '%s'", demote->target);
			return;
		}
		if (gci.guild_id != GuildID()) {
			Message(0, "You aren't in the same guild, what do you think you are doing?");
			return;
		}

		if (gci.rank < 1) {
			Message(0, "%s cannot be demoted any further!", demote->target);
			return;
		}
		uint8 rank = gci.rank - 1;


		Log(Logs::Detail, Logs::Guilds, "Demoting %s (%d) from rank %s (%d) to %s (%d) in %s (%d)",
			demote->target, gci.char_id,
			guild_mgr.GetRankName(GuildID(), gci.rank), gci.rank,
			guild_mgr.GetRankName(GuildID(), rank), rank,
			guild_mgr.GetGuildName(GuildID()), GuildID());

		if (!guild_mgr.SetGuildRank(gci.char_id, rank)) {
			Message(13, "Error while setting rank %d on '%s'.", rank, demote->target);
			return;
		}
		Message(0, "Successfully demoted %s to rank %d", demote->target, rank);
	}
	//	SendGuildMembers(GuildID(), true);
	return;
}

void Client::Handle_OP_GuildInvite(const EQApplicationPacket *app)
{
	Log(Logs::Detail, Logs::Guilds, "Received OP_GuildInvite");

	if (app->size != sizeof(GuildCommand_Struct)) {
		std::cout << "Wrong size: OP_GuildInvite, size=" << app->size << ", expected " << sizeof(GuildCommand_Struct) << std::endl;
		return;
	}

	GuildCommand_Struct* gc = (GuildCommand_Struct*)app->pBuffer;

	if (!IsInAGuild())
		Message(0, "Error: You are not in a guild!");
	else if (gc->officer > GUILD_MAX_RANK)
		Message(13, "Invalid rank.");
	else if (!worldserver.Connected())
		Message(0, "Error: World server disconnected");
	else {

		//ok, the invite is also used for changing rank as well.
		Mob* invitee = entity_list.GetMob(gc->othername);

		if (!invitee) {
			Message(13, "Prospective guild member %s must be in zone to preform guild operations on them.", gc->othername);
			return;
		}

		if (invitee->IsClient()) {
			Client* client = invitee->CastToClient();

			//ok, figure out what they are trying to do.
			if (client->GuildID() == GuildID()) {
				//they are already in this guild, must be a promotion or demotion
				if (gc->officer < client->GuildRank()) {
					//demotion
					if (!guild_mgr.CheckPermission(GuildID(), GuildRank(), GUILD_DEMOTE)) {
						Message(13, "You dont have permission to demote.");
						return;
					}

					//we could send this to the member and prompt them to see if they want to
					//be demoted (I guess), but I dont see a point in that.

					Log(Logs::Detail, Logs::Guilds, "%s (%d) is demoting %s (%d) to rank %d in guild %s (%d)",
						GetName(), CharacterID(),
						client->GetName(), client->CharacterID(),
						gc->officer,
						guild_mgr.GetGuildName(GuildID()), GuildID());

					if (!guild_mgr.SetGuildRank(client->CharacterID(), gc->officer)) {
						Message(13, "There was an error during the demotion, DB may now be inconsistent.");
						return;
					}

				}
				else if (gc->officer > client->GuildRank()) {
					//promotion
					if (!guild_mgr.CheckPermission(GuildID(), GuildRank(), GUILD_PROMOTE)) {
						Message(13, "You dont have permission to demote.");
						return;
					}

					Log(Logs::Detail, Logs::Guilds, "%s (%d) is asking to promote %s (%d) to rank %d in guild %s (%d)",
						GetName(), CharacterID(),
						client->GetName(), client->CharacterID(),
						gc->officer,
						guild_mgr.GetGuildName(GuildID()), GuildID());

					//record the promotion with guild manager so we know its valid when we get the reply
					guild_mgr.RecordInvite(client->CharacterID(), GuildID(), gc->officer);

					if (gc->guildeqid == 0)
						gc->guildeqid = GuildID();

					Log(Logs::Detail, Logs::Guilds, "Sending OP_GuildInvite for promotion to %s, length %d", client->GetName(), app->size);
					client->QueuePacket(app);

				}
				else {
					Message(13, "That member is already that rank.");
					return;
				}
			}
			else if (!client->IsInAGuild()) {
				//they are not in this or any other guild, this is an invite
				//
				if (client->GetPendingGuildInvitation())
				{
					Message(13, "That person is already considering a guild invitation.");
					return;
				}

				if (!guild_mgr.CheckPermission(GuildID(), GuildRank(), GUILD_INVITE)) {
					Message(13, "You dont have permission to invite.");
					return;
				}

				Log(Logs::Detail, Logs::Guilds, "Inviting %s (%d) into guild %s (%d)",
					client->GetName(), client->CharacterID(),
					guild_mgr.GetGuildName(GuildID()), GuildID());

				//record the invite with guild manager so we know its valid when we get the reply
				guild_mgr.RecordInvite(client->CharacterID(), GuildID(), gc->officer);

				if (gc->guildeqid == 0)
					gc->guildeqid = GuildID();

				// Convert Membership Level between RoF and previous clients.
				if (client->ClientVersion() < EQEmu::versions::ClientVersion::RoF && ClientVersion() >= EQEmu::versions::ClientVersion::RoF)
				{
					gc->officer = 0;
				}
				if (client->ClientVersion() >= EQEmu::versions::ClientVersion::RoF && ClientVersion() < EQEmu::versions::ClientVersion::RoF)
				{
					gc->officer = 8;
				}

				Log(Logs::Detail, Logs::Guilds, "Sending OP_GuildInvite for invite to %s, length %d", client->GetName(), app->size);
				client->SetPendingGuildInvitation(true);
				client->QueuePacket(app);

			}
			else {
				//they are in some other guild
				Message(13, "Player is in a guild.");
				return;
			}
		}
#ifdef BOTS
		else if (invitee->IsBot()) {
			// The guild system is too tightly coupled with the character_data table so we have to avoid using much of the system
			Bot::ProcessGuildInvite(this, invitee->CastToBot());
			return;
		}
#endif
	}
}

void Client::Handle_OP_GuildInviteAccept(const EQApplicationPacket *app)
{
	Log(Logs::Detail, Logs::Guilds, "Received OP_GuildInviteAccept");

	SetPendingGuildInvitation(false);

	if (app->size != sizeof(GuildInviteAccept_Struct)) {
		std::cout << "Wrong size: OP_GuildInviteAccept, size=" << app->size << ", expected " << sizeof(GuildJoin_Struct) << std::endl;
		return;
	}

	GuildInviteAccept_Struct* gj = (GuildInviteAccept_Struct*)app->pBuffer;

	uint32 guildrank = gj->response;

	if (ClientVersion() >= EQEmu::versions::ClientVersion::RoF)
	{
		if (gj->response > 9)
		{
			//dont care if the check fails (since we dont know the rank), just want to clear the entry.
			guild_mgr.VerifyAndClearInvite(CharacterID(), gj->guildeqid, gj->response);
			worldserver.SendEmoteMessage(gj->inviter, 0, 0, "%s has declined to join the guild.", this->GetName());
			return;
		}
	}
	if (gj->response == 5 || gj->response == 4) {
		//dont care if the check fails (since we dont know the rank), just want to clear the entry.
		guild_mgr.VerifyAndClearInvite(CharacterID(), gj->guildeqid, gj->response);

		worldserver.SendEmoteMessage(gj->inviter, 0, 0, "%s has declined to join the guild.", this->GetName());

		return;
	}

	//uint32 tmpeq = gj->guildeqid;
	if (IsInAGuild() && gj->response == GuildRank())
		Message(0, "Error: You're already in a guild!");
	else if (!worldserver.Connected())
		Message(0, "Error: World server disconnected");
	else {
		Log(Logs::Detail, Logs::Guilds, "Guild Invite Accept: guild %d, response %d, inviter %s, person %s",
			gj->guildeqid, gj->response, gj->inviter, gj->newmember);

		//ok, the invite is also used for changing rank as well.
		Mob* inviter = entity_list.GetMob(gj->inviter);

		if (inviter && inviter->IsClient())
		{
			Client* client = inviter->CastToClient();
			// Convert Membership Level between RoF and previous clients.
			if (client->ClientVersion() < EQEmu::versions::ClientVersion::RoF && ClientVersion() >= EQEmu::versions::ClientVersion::RoF)
			{
				guildrank = 0;
			}
			if (client->ClientVersion() >= EQEmu::versions::ClientVersion::RoF && ClientVersion() < EQEmu::versions::ClientVersion::RoF)
			{
				guildrank = 8;
			}
		}
		//we dont really care a lot about what this packet means, as long as
		//it has been authorized with the guild manager
		if (!guild_mgr.VerifyAndClearInvite(CharacterID(), gj->guildeqid, guildrank)) {
			worldserver.SendEmoteMessage(gj->inviter, 0, 0, "%s has sent an invalid response to your invite!", GetName());
			Message(13, "Invalid invite response packet!");
			return;
		}

		if (gj->guildeqid == GuildID()) {
			//only need to change rank.

			Log(Logs::Detail, Logs::Guilds, "Changing guild rank of %s (%d) to rank %d in guild %s (%d)",
				GetName(), CharacterID(),
				gj->response,
				guild_mgr.GetGuildName(GuildID()), GuildID());

			if (!guild_mgr.SetGuildRank(CharacterID(), gj->response)) {
				Message(13, "There was an error during the rank change, DB may now be inconsistent.");
				return;
			}
		}
		else {

			Log(Logs::Detail, Logs::Guilds, "Adding %s (%d) to guild %s (%d) at rank %d",
				GetName(), CharacterID(),
				guild_mgr.GetGuildName(gj->guildeqid), gj->guildeqid,
				gj->response);

			//change guild and rank

			guildrank = gj->response;

			if (ClientVersion() >= EQEmu::versions::ClientVersion::RoF)
			{
				if (gj->response == 8)
				{
					guildrank = 0;
				}
			}

			if (!guild_mgr.SetGuild(CharacterID(), gj->guildeqid, guildrank)) {
				Message(13, "There was an error during the invite, DB may now be inconsistent.");
				return;
			}
			if (zone->GetZoneID() == RuleI(World, GuildBankZoneID) && GuildBanks)
				GuildBanks->SendGuildBank(this);
		}
	}
}

void Client::Handle_OP_GuildLeader(const EQApplicationPacket *app)
{
	Log(Logs::Detail, Logs::Guilds, "Received OP_GuildLeader");

	if (app->size < 2) {
		Log(Logs::Detail, Logs::Guilds, "Invalid length %d on OP_GuildLeader", app->size);
		return;
	}

	app->pBuffer[app->size - 1] = 0;
	GuildMakeLeader* gml = (GuildMakeLeader*)app->pBuffer;
	if (!IsInAGuild())
		Message(0, "Error: You arent in a guild!");
	else if (GuildRank() != GUILD_LEADER)
		Message(0, "Error: You arent the guild leader!");
	else if (!worldserver.Connected())
		Message(0, "Error: World server disconnected");
	else {

		//NOTE: we could do cross-zone lookups here...

		Client* newleader = entity_list.GetClientByName(gml->target);
		if (newleader) {

			Log(Logs::Detail, Logs::Guilds, "Transfering leadership of %s (%d) to %s (%d)",
				guild_mgr.GetGuildName(GuildID()), GuildID(),
				newleader->GetName(), newleader->CharacterID());

			if (guild_mgr.SetGuildLeader(GuildID(), newleader->CharacterID())) {
				Message(0, "Successfully Transfered Leadership to %s.", gml->target);
				newleader->Message(15, "%s has transfered the guild leadership into your hands.", GetName());
			}
			else
				Message(0, "Could not change leadership at this time.");
		}
		else
			Message(0, "Failed to change leader, could not find target.");
	}
	//	SendGuildMembers(GuildID(), true);
	return;
}

void Client::Handle_OP_GuildManageBanker(const EQApplicationPacket *app)
{

	Log(Logs::Detail, Logs::Guilds, "Got OP_GuildManageBanker of len %d", app->size);
	if (app->size != sizeof(GuildManageBanker_Struct)) {
		Log(Logs::Detail, Logs::Guilds, "Error: app size of %i != size of OP_GuildManageBanker of %i\n", app->size, sizeof(GuildManageBanker_Struct));
		return;
	}
	GuildManageBanker_Struct* gmb = (GuildManageBanker_Struct*)app->pBuffer;

	if (!IsInAGuild()) {
		Message(13, "Your not in a guild!");
		return;
	}

	CharGuildInfo gci;

	if (!guild_mgr.GetCharInfo(gmb->member, gci))
	{
		Message(0, "Unable to find '%s'", gmb->member);
		return;
	}
	bool IsCurrentlyABanker = guild_mgr.GetBankerFlag(gci.char_id);

	bool IsCurrentlyAnAlt = guild_mgr.GetAltFlag(gci.char_id);

	bool NewBankerStatus = gmb->enabled & 0x01;

	bool NewAltStatus = gmb->enabled & 0x02;

	if ((IsCurrentlyABanker != NewBankerStatus) && !guild_mgr.IsGuildLeader(GuildID(), CharacterID()))
	{
		Message(13, "Only the guild leader can assign guild bankers!");
		return;
	}

	if (IsCurrentlyAnAlt != NewAltStatus)
	{
		bool IsAllowed = !strncasecmp(GetName(), gmb->member, strlen(GetName())) || (GuildRank() >= GUILD_OFFICER);

		if (!IsAllowed)
		{
			Message(13, "You are not allowed to change the alt status of %s", gmb->member);
			return;
		}
	}

	if (gci.guild_id != GuildID()) {
		Message(0, "You aren't in the same guild, what do you think you are doing?");
		return;
	}

	if (IsCurrentlyABanker != NewBankerStatus)
	{
		if (!guild_mgr.SetBankerFlag(gci.char_id, NewBankerStatus)) {
			Message(13, "Error setting guild banker flag.");
			return;
		}

		if (NewBankerStatus)
			Message(0, "%s has been made a guild banker.", gmb->member);
		else
			Message(0, "%s is no longer a guild banker.", gmb->member);
	}
	if (IsCurrentlyAnAlt != NewAltStatus)
	{
		if (!guild_mgr.SetAltFlag(gci.char_id, NewAltStatus)) {
			Message(13, "Error setting guild alt flag.");
			return;
		}

		if (NewAltStatus)
			Message(0, "%s has been marked as an alt.", gmb->member);
		else
			Message(0, "%s is no longer marked as an alt.", gmb->member);
	}
}

void Client::Handle_OP_GuildPeace(const EQApplicationPacket *app)
{
	Log(Logs::Detail, Logs::Guilds, "Got OP_GuildPeace of len %d", app->size);
	return;
}

void Client::Handle_OP_GuildPromote(const EQApplicationPacket *app)
{
	Log(Logs::Detail, Logs::Guilds, "Received OP_GuildPromote");

	if (app->size != sizeof(GuildPromoteStruct)) {
		Log(Logs::Detail, Logs::Guilds, "Error: app size of %i != size of GuildDemoteStruct of %i\n", app->size, sizeof(GuildPromoteStruct));
		return;
	}

	if (!IsInAGuild())
		Message(0, "Error: You arent in a guild!");
	else if (!guild_mgr.CheckPermission(GuildID(), GuildRank(), GUILD_PROMOTE))
		Message(0, "You dont have permission to invite.");
	else if (!worldserver.Connected())
		Message(0, "Error: World server disconnected");
	else {
		GuildPromoteStruct* promote = (GuildPromoteStruct*)app->pBuffer;

		CharGuildInfo gci;
		if (!guild_mgr.GetCharInfo(promote->target, gci)) {
			Message(0, "Unable to find '%s'", promote->target);
			return;
		}
		if (gci.guild_id != GuildID()) {
			Message(0, "You aren't in the same guild, what do you think you are doing?");
			return;
		}

		uint8 rank = gci.rank + 1;

		if (rank > GUILD_OFFICER)
		{
			Message(0, "You cannot promote someone to be guild leader. You must use /guildleader.");
			return;
		}


		Log(Logs::Detail, Logs::Guilds, "Promoting %s (%d) from rank %s (%d) to %s (%d) in %s (%d)",
			promote->target, gci.char_id,
			guild_mgr.GetRankName(GuildID(), gci.rank), gci.rank,
			guild_mgr.GetRankName(GuildID(), rank), rank,
			guild_mgr.GetGuildName(GuildID()), GuildID());

		if (!guild_mgr.SetGuildRank(gci.char_id, rank)) {
			Message(13, "Error while setting rank %d on '%s'.", rank, promote->target);
			return;
		}
		Message(0, "Successfully promoted %s to rank %d", promote->target, rank);
	}
	return;
}

void Client::Handle_OP_GuildPublicNote(const EQApplicationPacket *app)
{
	Log(Logs::Detail, Logs::Guilds, "Received OP_GuildPublicNote");

	if (app->size < sizeof(GuildUpdate_PublicNote)) {
		// client calls for a motd on login even if they arent in a guild
		printf("Error: app size of %i < size of OP_GuildPublicNote of %zu\n", app->size, sizeof(GuildUpdate_PublicNote));
		return;
	}
	GuildUpdate_PublicNote* gpn = (GuildUpdate_PublicNote*)app->pBuffer;

	CharGuildInfo gci;
	if (!guild_mgr.GetCharInfo(gpn->target, gci)) {
		Message(0, "Unable to find '%s'", gpn->target);
		return;
	}
	if (gci.guild_id != GuildID()) {
		Message(0, "You aren't in the same guild, what do you think you are doing?");
		return;
	}

	Log(Logs::Detail, Logs::Guilds, "Setting public note on %s (%d) in guild %s (%d) to: %s",
		gpn->target, gci.char_id,
		guild_mgr.GetGuildName(GuildID()), GuildID(),
		gpn->note);

	if (!guild_mgr.SetPublicNote(gci.char_id, gpn->note)) {
		Message(13, "Failed to set public note on %s", gpn->target);
	}
	else {
		Message(0, "Successfully changed public note on %s", gpn->target);
	}
	//	SendGuildMembers(GuildID(), true);
	return;
}

void Client::Handle_OP_GuildRemove(const EQApplicationPacket *app)
{
	Log(Logs::Detail, Logs::Guilds, "Received OP_GuildRemove");

	if (app->size != sizeof(GuildCommand_Struct)) {
		std::cout << "Wrong size: OP_GuildRemove, size=" << app->size << ", expected " << sizeof(GuildCommand_Struct) << std::endl;
		return;
	}
	GuildCommand_Struct* gc = (GuildCommand_Struct*)app->pBuffer;
	if (!IsInAGuild())
		Message(0, "Error: You arent in a guild!");
	// we can always remove ourself, otherwise, our rank needs remove permissions
	else if (strcasecmp(gc->othername, GetName()) != 0 &&
		!guild_mgr.CheckPermission(GuildID(), GuildRank(), GUILD_REMOVE))
		Message(0, "You dont have permission to remove guild members.");
	else if (!worldserver.Connected())
		Message(0, "Error: World server disconnected");
	else {
#ifdef BOTS
		if (Bot::ProcessGuildRemoval(this, gc->othername))
			return;
#endif
		uint32 char_id;
		Client* client = entity_list.GetClientByName(gc->othername);

		if (client) {
			if (!client->IsInGuild(GuildID())) {
				Message(0, "You aren't in the same guild, what do you think you are doing?");
				return;
			}
			char_id = client->CharacterID();

			Log(Logs::Detail, Logs::Guilds, "Removing %s (%d) from guild %s (%d)",
				client->GetName(), client->CharacterID(),
				guild_mgr.GetGuildName(GuildID()), GuildID());
		}
		else {
			CharGuildInfo gci;
			if (!guild_mgr.GetCharInfo(gc->othername, gci)) {
				Message(0, "Unable to find '%s'", gc->othername);
				return;
			}
			if (gci.guild_id != GuildID()) {
				Message(0, "You aren't in the same guild, what do you think you are doing?");
				return;
			}
			char_id = gci.char_id;

			Log(Logs::Detail, Logs::Guilds, "Removing remote/offline %s (%d) into guild %s (%d)",
				gci.char_name.c_str(), gci.char_id,
				guild_mgr.GetGuildName(GuildID()), GuildID());
		}

		if (!guild_mgr.SetGuild(char_id, GUILD_NONE, 0)) {
			auto outapp = new EQApplicationPacket(OP_GuildManageRemove, sizeof(GuildManageRemove_Struct));
			GuildManageRemove_Struct* gm = (GuildManageRemove_Struct*)outapp->pBuffer;
			gm->guildeqid = GuildID();
			strcpy(gm->member, gc->othername);
			Message(0, "%s successfully removed from your guild.", gc->othername);
			entity_list.QueueClientsGuild(this, outapp, false, GuildID());
			safe_delete(outapp);
		}
		else
			Message(0, "Unable to remove %s from your guild.", gc->othername);
	}
	//	SendGuildMembers(GuildID(), true);
	return;
}

void Client::Handle_OP_GuildStatus(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GuildStatus_Struct))
	{
		Log(Logs::General, Logs::None, "Size mismatch in OP_GuildStatus expected %i got %i",
			sizeof(GuildStatus_Struct), app->size);

		DumpPacket(app);

		return;
	}
	GuildStatus_Struct *gss = (GuildStatus_Struct*)app->pBuffer;

	Client *c = entity_list.GetClientByName(gss->Name);

	if (!c)
	{
		Message_StringID(clientMessageWhite, TARGET_PLAYER_FOR_GUILD_STATUS);
		return;
	}

	uint32 TargetGuildID = c->GuildID();

	if (TargetGuildID == GUILD_NONE)
	{
		Message_StringID(clientMessageWhite, NOT_IN_A_GUILD, c->GetName());
		return;
	}

	const char *GuildName = guild_mgr.GetGuildName(TargetGuildID);

	if (!GuildName)
		return;

	bool IsLeader = guild_mgr.CheckPermission(TargetGuildID, c->GuildRank(), GUILD_PROMOTE);
	bool IsOfficer = guild_mgr.CheckPermission(TargetGuildID, c->GuildRank(), GUILD_INVITE);

	if ((TargetGuildID == GuildID()) && (c != this))
	{
		if (IsLeader)
			Message_StringID(clientMessageWhite, LEADER_OF_YOUR_GUILD, c->GetName());
		else if (IsOfficer)
			Message_StringID(clientMessageWhite, OFFICER_OF_YOUR_GUILD, c->GetName());
		else
			Message_StringID(clientMessageWhite, MEMBER_OF_YOUR_GUILD, c->GetName());

		return;
	}

	if (IsLeader)
		Message_StringID(clientMessageWhite, LEADER_OF_X_GUILD, c->GetName(), GuildName);
	else if (IsOfficer)
		Message_StringID(clientMessageWhite, OFFICER_OF_X_GUILD, c->GetName(), GuildName);
	else
		Message_StringID(clientMessageWhite, MEMBER_OF_X_GUILD, c->GetName(), GuildName);
}

void Client::Handle_OP_GuildUpdateURLAndChannel(const EQApplicationPacket *app)
{
	if (app->size != sizeof(GuildUpdateURLAndChannel_Struct))
	{
		Log(Logs::General, Logs::None, "Size mismatch in OP_GuildUpdateURLAndChannel expected %i got %i",
			sizeof(GuildUpdateURLAndChannel_Struct), app->size);

		DumpPacket(app);

		return;
	}

	GuildUpdateURLAndChannel_Struct *guuacs = (GuildUpdateURLAndChannel_Struct*)app->pBuffer;

	if (!IsInAGuild())
		return;

	if (!guild_mgr.IsGuildLeader(GuildID(), CharacterID()))
	{
		Message(13, "Only the guild leader can change the Channel or URL.!");
		return;
	}

	if (guuacs->Action == 0)
		guild_mgr.SetGuildURL(GuildID(), guuacs->Text);
	else
		guild_mgr.SetGuildChannel(GuildID(), guuacs->Text);

}

void Client::Handle_OP_GuildWar(const EQApplicationPacket *app)
{
	Log(Logs::Detail, Logs::Guilds, "Got OP_GuildWar of len %d", app->size);
	return;
}

void Client::Handle_OP_Heartbeat(const EQApplicationPacket *app)
{
	return;
}

void Client::Handle_OP_Hide(const EQApplicationPacket *app)
{
	// newer client respond to OP_CancelSneakHide with OP_Hide with a size of 4 and 0 data
	if (app->size == 4) {
		auto data = app->ReadUInt32(0);
		if (data)
			Log(Logs::Detail, Logs::None, "Got OP_Hide with unexpected data %d", data);
		return;
	}

	if (!HasSkill(EQEmu::skills::SkillHide) && GetSkill(EQEmu::skills::SkillHide) == 0)
	{
		//Can not be able to train hide but still have it from racial though
		return; //You cannot hide if you do not have hide
	}

	if (!p_timers.Expired(&database, pTimerHide, false)) {
		Message(13, "Ability recovery time not yet met.");
		return;
	}
	int reuse = HideReuseTime - GetSkillReuseTime(EQEmu::skills::SkillHide);

	if (reuse < 1)
		reuse = 1;

	p_timers.Start(pTimerHide, reuse - 1);

	float hidechance = ((GetSkill(EQEmu::skills::SkillHide) / 250.0f) + .25) * 100;
	float random = zone->random.Real(0, 100);
	CheckIncreaseSkill(EQEmu::skills::SkillHide, nullptr, 5);
	if (random < hidechance) {
		auto outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
		SpawnAppearance_Struct* sa_out = (SpawnAppearance_Struct*)outapp->pBuffer;
		sa_out->spawn_id = GetID();
		sa_out->type = 0x03;
		sa_out->parameter = 1;
		entity_list.QueueClients(this, outapp, true);
		safe_delete(outapp);
		if (spellbonuses.ShroudofStealth || aabonuses.ShroudofStealth || itembonuses.ShroudofStealth) {
			improved_hidden = true;
			hidden = true;
		}
		else
			hidden = true;
		tmHidden = Timer::GetCurrentTime();
	}
	if (GetClass() == ROGUE) {
		auto outapp = new EQApplicationPacket(OP_SimpleMessage, sizeof(SimpleMessage_Struct));
		SimpleMessage_Struct *msg = (SimpleMessage_Struct *)outapp->pBuffer;
		msg->color = 0x010E;
		Mob *evadetar = GetTarget();
		if (!auto_attack && (evadetar && evadetar->CheckAggro(this)
			&& evadetar->IsNPC())) {
			if (zone->random.Int(0, 260) < (int)GetSkill(EQEmu::skills::SkillHide)) {
				msg->string_id = EVADE_SUCCESS;
				RogueEvade(evadetar);
			}
			else {
				msg->string_id = EVADE_FAIL;
			}
		}
		else {
			if (hidden) {
				msg->string_id = HIDE_SUCCESS;
			}
			else {
				msg->string_id = HIDE_FAIL;
			}
		}
		FastQueuePacket(&outapp);
	}
	return;
}

void Client::Handle_OP_HideCorpse(const EQApplicationPacket *app)
{
	// New OPCode for SOD+ as /hidecorpse is handled serverside now.
	//
	if (app->size != sizeof(HideCorpse_Struct))
	{
		Log(Logs::General, Logs::None, "Size mismatch in OP_HideCorpse expected %i got %i",
			sizeof(HideCorpse_Struct), app->size);

		DumpPacket(app);

		return;
	}

	HideCorpse_Struct *hcs = (HideCorpse_Struct*)app->pBuffer;

	if (hcs->Action == HideCorpseLooted)
		return;

	if ((HideCorpseMode == HideCorpseNone) && (hcs->Action == HideCorpseNone))
		return;

	entity_list.HideCorpses(this, HideCorpseMode, hcs->Action);

	HideCorpseMode = hcs->Action;
}

void Client::Handle_OP_Ignore(const EQApplicationPacket *app)
{
	return;
}

void Client::Handle_OP_Illusion(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Illusion_Struct)) {
		Log(Logs::General, Logs::Error, "Received invalid sized OP_Illusion: got %d, expected %d", app->size,
			sizeof(Illusion_Struct));
		DumpPacket(app);
		return;
	}

	if (!GetGM())
	{
		database.SetMQDetectionFlag(this->AccountName(), this->GetName(), "OP_Illusion sent by non Game Master.", zone->GetShortName());
		return;
	}

	Illusion_Struct* bnpc = (Illusion_Struct*)app->pBuffer;
	//these need to be implemented
	/*
	texture		= bnpc->texture;
	helmtexture	= bnpc->helmtexture;
	luclinface	= bnpc->luclinface;
	*/
	race = bnpc->race;
	size = 0;

	entity_list.QueueClients(this, app);
	return;
}

void Client::Handle_OP_InspectAnswer(const EQApplicationPacket *app)
{

	if (app->size != sizeof(InspectResponse_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_InspectAnswer, size=%i, expected %i", app->size, sizeof(InspectResponse_Struct));
		return;
	}

	//Fills the app sent from client.
	EQApplicationPacket* outapp = app->Copy();
	InspectResponse_Struct* insr = (InspectResponse_Struct*)outapp->pBuffer;
	Mob* tmp = entity_list.GetMob(insr->TargetID);
	const EQEmu::ItemData* item = nullptr;

	int ornamentationAugtype = RuleI(Character, OrnamentationAugmentType);
	for (int16 L = EQEmu::legacy::EQUIPMENT_BEGIN; L <= EQEmu::inventory::slotWaist; L++) {
		const EQEmu::ItemInstance* inst = GetInv().GetItem(L);
		item = inst ? inst->GetItem() : nullptr;

		if (item) {
			strcpy(insr->itemnames[L], item->Name);
			if (inst && inst->GetOrnamentationAug(ornamentationAugtype)) {
				const EQEmu::ItemData *aug_item = inst->GetOrnamentationAug(ornamentationAugtype)->GetItem();
				insr->itemicons[L] = aug_item->Icon;
			}
			else if (inst->GetOrnamentationIcon()) {
				insr->itemicons[L] = inst->GetOrnamentationIcon();
			}
			else {
				insr->itemicons[L] = item->Icon;
			}
		}
		else { insr->itemicons[L] = 0xFFFFFFFF; }
	}

	const EQEmu::ItemInstance* inst = GetInv().GetItem(EQEmu::inventory::slotAmmo);
	item = inst ? inst->GetItem() : nullptr;

	if (item) {
		// another one..I did these, didn't I!!?
		strcpy(insr->itemnames[SoF::invslot::PossessionsAmmo], item->Name);
		insr->itemicons[SoF::invslot::PossessionsAmmo] = item->Icon;
	}
	else { insr->itemicons[SoF::invslot::PossessionsAmmo] = 0xFFFFFFFF; }

	InspectMessage_Struct* newmessage = (InspectMessage_Struct*)insr->text;
	InspectMessage_Struct& playermessage = this->GetInspectMessage();
	memcpy(&playermessage, newmessage, sizeof(InspectMessage_Struct));
	database.SaveCharacterInspectMessage(this->CharacterID(), &playermessage);

	if (tmp != 0 && tmp->IsClient()) { tmp->CastToClient()->QueuePacket(outapp); } // Send answer to requester

	return;
}

void Client::Handle_OP_InspectMessageUpdate(const EQApplicationPacket *app)
{

	if (app->size != sizeof(InspectMessage_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_InspectMessageUpdate, size=%i, expected %i", app->size, sizeof(InspectMessage_Struct));
		return;
	}

	InspectMessage_Struct* newmessage = (InspectMessage_Struct*)app->pBuffer;
	InspectMessage_Struct& playermessage = this->GetInspectMessage();
	memcpy(&playermessage, newmessage, sizeof(InspectMessage_Struct));
	database.SaveCharacterInspectMessage(this->CharacterID(), &playermessage);
}

void Client::Handle_OP_InspectRequest(const EQApplicationPacket *app)
{

	if (app->size != sizeof(Inspect_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_InspectRequest, size=%i, expected %i", app->size, sizeof(Inspect_Struct));
		return;
	}

	Inspect_Struct* ins = (Inspect_Struct*)app->pBuffer;
	Mob* tmp = entity_list.GetMob(ins->TargetID);

	if (tmp != 0 && tmp->IsClient()) {
		if (tmp->CastToClient()->ClientVersion() < EQEmu::versions::ClientVersion::SoF) { tmp->CastToClient()->QueuePacket(app); } // Send request to target
																																   // Inspecting an SoF or later client will make the server handle the request
		else { ProcessInspectRequest(tmp->CastToClient(), this); }
	}

#ifdef BOTS
	if (tmp != 0 && tmp->IsBot()) { Bot::ProcessBotInspectionRequest(tmp->CastToBot(), this); }
#endif

	return;
}

void Client::Handle_OP_InstillDoubt(const EQApplicationPacket *app)
{
	//packet is empty as of 12/14/04

	if (!p_timers.Expired(&database, pTimerInstillDoubt, false)) {
		Message(13, "Ability recovery time not yet met.");
		return;
	}
	p_timers.Start(pTimerInstillDoubt, InstillDoubtReuseTime - 1);

	InstillDoubt(GetTarget());
	return;
}

void Client::Handle_OP_ItemLinkClick(const EQApplicationPacket *app)
{
	if (app->size != sizeof(ItemViewRequest_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size on OP_ItemLinkClick. Got: %i, Expected: %i", app->size,
			sizeof(ItemViewRequest_Struct));
		DumpPacket(app);
		return;
	}

	DumpPacket(app);
	ItemViewRequest_Struct *ivrs = (ItemViewRequest_Struct *)app->pBuffer;

	// todo: verify ivrs->link_hash based on a rule, in case we don't care about people being able to sniff data
	// from the item DB

	const EQEmu::ItemData *item = database.GetItem(ivrs->item_id);
	if (!item) {
		if (ivrs->item_id != SAYLINK_ITEM_ID) {
			Message(13, "Error: The item for the link you have clicked on does not exist!");
			return;
		}
		// This new scheme will shuttle the ID in the first augment for non-silent links
		// and the second augment for silent.
		std::string response = "";
		bool silentsaylink = ivrs->augments[1] > 0 ? true : false;
		int sayid = silentsaylink ? ivrs->augments[1] : ivrs->augments[0];

		if (sayid > 0) {
			std::string query = StringFormat("SELECT `phrase` FROM saylink WHERE `id` = '%i'", sayid);
			auto results = database.QueryDatabase(query);
			if (!results.Success()) {
				Message(13, "Error: The saylink (%s) was not found in the database.", response.c_str());
				return;
			}

			if (results.RowCount() != 1) {
				Message(13, "Error: The saylink (%s) was not found in the database.", response.c_str());
				return;
			}

			auto row = results.begin();
			response = row[0];
		}

		if ((response).size() > 0) {
			if (!mod_saylink(response, silentsaylink)) {
				return;
			}

			if (GetTarget() && GetTarget()->IsNPC()) {
				if (silentsaylink) {
					parse->EventNPC(EVENT_SAY, GetTarget()->CastToNPC(), this, response.c_str(), 0);
					parse->EventPlayer(EVENT_SAY, this, response.c_str(), 0);
				}
				else {
					Message(7, "You say, '%s'", response.c_str());
					ChannelMessageReceived(8, 0, 100, response.c_str());
				}
				return;
			}
			else {
				if (silentsaylink) {
					parse->EventPlayer(EVENT_SAY, this, response.c_str(), 0);
				}
				else {
					Message(7, "You say, '%s'", response.c_str());
					ChannelMessageReceived(8, 0, 100, response.c_str());
				}
				return;
			}
		}
		else {
			Message(13, "Error: Say Link not found or is too long.");
			return;
		}
	}

	EQEmu::ItemInstance *inst =
		database.CreateItem(item, item->MaxCharges, ivrs->augments[0], ivrs->augments[1], ivrs->augments[2],
			ivrs->augments[3], ivrs->augments[4], ivrs->augments[5]);
	if (inst) {
		SendItemPacket(0, inst, ItemPacketViewLink);
		safe_delete(inst);
	}
	return;
}

void Client::Handle_OP_ItemLinkResponse(const EQApplicationPacket *app)
{
	if (app->size != sizeof(LDONItemViewRequest_Struct)) {
		Log(Logs::General, Logs::Error, "OP size error: OP_ItemLinkResponse expected:%i got:%i", sizeof(LDONItemViewRequest_Struct), app->size);
		return;
	}
	LDONItemViewRequest_Struct* item = (LDONItemViewRequest_Struct*)app->pBuffer;
	EQEmu::ItemInstance* inst = database.CreateItem(item->item_id);
	if (inst) {
		SendItemPacket(0, inst, ItemPacketViewLink);
		safe_delete(inst);
	}
	return;
}

void Client::Handle_OP_ItemName(const EQApplicationPacket *app)
{
	if (app->size != sizeof(ItemNamePacket_Struct)) {
		Log(Logs::General, Logs::Error, "Invalid size for ItemNamePacket_Struct: Expected: %i, Got: %i",
			sizeof(ItemNamePacket_Struct), app->size);
		return;
	}
	ItemNamePacket_Struct *p = (ItemNamePacket_Struct*)app->pBuffer;
	const EQEmu::ItemData *item = nullptr;
	if ((item = database.GetItem(p->item_id)) != nullptr) {
		auto outapp = new EQApplicationPacket(OP_ItemName, sizeof(ItemNamePacket_Struct));
		p = (ItemNamePacket_Struct*)outapp->pBuffer;
		memset(p, 0, sizeof(ItemNamePacket_Struct));
		strcpy(p->name, item->Name);
		FastQueuePacket(&outapp);
	}
	return;
}

void Client::Handle_OP_ItemPreview(const EQApplicationPacket *app)
{
	VERIFY_PACKET_LENGTH(OP_ItemPreview, app, ItemPreview_Struct);
	ItemPreview_Struct *ips = (ItemPreview_Struct *)app->pBuffer;

	const EQEmu::ItemData* item = database.GetItem(ips->itemid);

	if (item) {
		auto outapp = new EQApplicationPacket(OP_ItemPreview, strlen(item->Name) + strlen(item->Lore) +
			strlen(item->IDFile) + 898);

		int spacer;
		for (spacer = 0; spacer < 16; spacer++) {
			outapp->WriteUInt8(48);
		}
		outapp->WriteUInt16(256);
		for (spacer = 0; spacer < 7; spacer++) {
			outapp->WriteUInt8(0);
		}
		for (spacer = 0; spacer < 7; spacer++) {
			outapp->WriteUInt8(255);
		}
		outapp->WriteUInt32(0);
		outapp->WriteUInt32(1);
		outapp->WriteUInt32(0);
		outapp->WriteUInt8(237); // Seems to be some kind of counter? increases by 1 for each preview that you do.
		outapp->WriteUInt16(2041); //F907
		for (spacer = 0; spacer < 36; spacer++) {
			outapp->WriteUInt8(0);
		}
		for (spacer = 0; spacer < 4; spacer++) {
			outapp->WriteUInt8(255);
		}
		for (spacer = 0; spacer < 9; spacer++) {
			outapp->WriteUInt8(0);
		}
		for (spacer = 0; spacer < 5; spacer++) {
			outapp->WriteUInt8(255);
		}
		for (spacer = 0; spacer < 5; spacer++) {
			outapp->WriteUInt8(0);
		}
		outapp->WriteString(item->Name);
		outapp->WriteString(item->Lore);
		outapp->WriteUInt8(0);
		outapp->WriteUInt32(ips->itemid);
		outapp->WriteUInt32(item->Weight);
		outapp->WriteUInt8(item->NoRent);
		outapp->WriteUInt8(item->NoDrop);
		outapp->WriteUInt8(item->Attuneable);
		outapp->WriteUInt8(item->Size);
		outapp->WriteUInt32(item->Slots);
		outapp->WriteUInt32(item->Price);
		outapp->WriteUInt32(item->Icon);
		outapp->WriteUInt8(0); //Unknown?
		outapp->WriteUInt8(0); //Placeable flag?
		outapp->WriteUInt32(item->BenefitFlag);
		outapp->WriteUInt8(item->Tradeskills);
		outapp->WriteUInt8(item->CR);
		outapp->WriteUInt8(item->DR);
		outapp->WriteUInt8(item->PR);
		outapp->WriteUInt8(item->MR);
		outapp->WriteUInt8(item->FR);
		outapp->WriteUInt8(item->AStr);
		outapp->WriteUInt8(item->ASta);
		outapp->WriteUInt8(item->AAgi);
		outapp->WriteUInt8(item->ADex);
		outapp->WriteUInt8(item->ACha);
		outapp->WriteUInt8(item->AInt);
		outapp->WriteUInt8(item->AWis);
		outapp->WriteSInt32(item->HP);
		outapp->WriteSInt32(item->Mana);
		outapp->WriteSInt32(item->Endur);
		outapp->WriteSInt32(item->AC);
		outapp->WriteUInt32(item->Regen);
		outapp->WriteUInt32(item->ManaRegen);
		outapp->WriteSInt32(item->EnduranceRegen);
		outapp->WriteUInt32(item->Classes);
		outapp->WriteUInt32(item->Races);
		outapp->WriteUInt32(item->Deity);
		outapp->WriteUInt32(item->SkillModValue);
		outapp->WriteUInt32(0); //SkillModValue
		outapp->WriteUInt32(item->SkillModType);
		outapp->WriteUInt32(0); //SkillModExtra
		outapp->WriteUInt32(item->BaneDmgRace);
		outapp->WriteUInt32(item->BaneDmgBody);
		outapp->WriteUInt32(item->BaneDmgRaceAmt);
		outapp->WriteUInt32(item->BaneDmgAmt);
		outapp->WriteUInt8(item->Magic);
		outapp->WriteUInt32(item->CastTime_);
		outapp->WriteUInt32(item->ReqLevel);
		outapp->WriteUInt32(item->RecLevel);
		outapp->WriteUInt32(item->RecSkill);
		outapp->WriteUInt32(item->BardType);
		outapp->WriteUInt32(item->BardValue);
		outapp->WriteUInt8(item->Light);
		outapp->WriteUInt8(item->Delay);
		outapp->WriteUInt8(item->ElemDmgType);
		outapp->WriteUInt8(item->ElemDmgAmt);
		outapp->WriteUInt8(item->Range);
		outapp->WriteUInt32(item->Damage);
		outapp->WriteUInt32(item->Color);
		outapp->WriteUInt32(0);	// Prestige
		outapp->WriteUInt8(item->ItemType);
		outapp->WriteUInt32(item->Material);
		outapp->WriteUInt32(0); //unknown
		outapp->WriteUInt32(item->EliteMaterial);
		outapp->WriteUInt32(item->HerosForgeModel);
		outapp->WriteUInt32(0);	// unknown
		outapp->WriteUInt32(0); //This is unknown057 from lucy
		for (spacer = 0; spacer < 77; spacer++) { //More Item stats, but some seem to be off based on packet check
			outapp->WriteUInt8(0);
		}
		outapp->WriteUInt32(0xFFFFFFFF); //Unknown but always seen as FF FF FF FF
		outapp->WriteUInt32(0); //Unknown
		for (spacer = 0; spacer < 6; spacer++) { //Augment stuff
			outapp->WriteUInt32(item->AugSlotType[spacer]);
			outapp->WriteUInt8(item->AugSlotVisible[spacer]);
			outapp->WriteUInt8(item->AugSlotUnk2[spacer]);
		}
		outapp->WriteUInt32(0); //New RoF 6th Aug Slot
		outapp->WriteUInt8(1); //^
		outapp->WriteUInt8(0); //^^
		outapp->WriteUInt32(item->LDoNSold);
		outapp->WriteUInt32(item->LDoNTheme);
		outapp->WriteUInt32(item->LDoNPrice);
		outapp->WriteUInt32(item->LDoNSellBackRate);
		for (spacer = 0; spacer < 11; spacer++) { //unknowns
			outapp->WriteUInt8(0);
		}
		outapp->WriteUInt32(0xFFFFFFFF); //Unknown but always seen as FF FF FF FF
		outapp->WriteUInt16(0); //Unknown
		outapp->WriteUInt32(item->Favor); // Tribute
		for (spacer = 0; spacer < 17; spacer++) { //unknowns
			outapp->WriteUInt8(0);
		}
		outapp->WriteUInt32(item->GuildFavor); // Tribute
		outapp->WriteUInt32(0); //Unknown
		outapp->WriteUInt32(0xFFFFFFFF); //Unknown but always seen as FF FF FF FF
		for (spacer = 0; spacer < 11; spacer++) { //unknowns
			outapp->WriteUInt8(0);
		}
		outapp->WriteUInt8(1);
		for (spacer = 0; spacer < 25; spacer++) { //unknowns
			outapp->WriteUInt8(0);
		}
		for (spacer = 0; spacer < 304; spacer++) { //Cast stuff and whole bunch of unknowns
			outapp->WriteUInt8(0);
		}
		outapp->WriteUInt8(142); // Always seen not in the item structure though 8E
		outapp->WriteUInt32(0); //unknown
		outapp->WriteUInt32(1); // Always seen as 1
		outapp->WriteUInt32(0); //unknown
		outapp->WriteUInt32(0xCDCCCC3D); // Unknown
		outapp->WriteUInt32(0);
		outapp->WriteUInt16(8256); //0x4020/8256
		outapp->WriteUInt16(0);
		outapp->WriteUInt32(0xFFFFFFFF); //Unknown but always seen as FF FF FF FF
		outapp->WriteUInt16(0);
		outapp->WriteUInt32(0xFFFFFFFF); //Unknown but always seen as FF FF FF FF
		outapp->WriteUInt32(0); //unknown
		outapp->WriteUInt32(0); //unknown
		outapp->WriteUInt16(0); //unknown
		outapp->WriteUInt32(32831); //0x3F80
		for (spacer = 0; spacer < 24; spacer++) { //whole bunch of unknowns always 0's
			outapp->WriteUInt8(0);
		}
		outapp->WriteUInt8(1);
		for (spacer = 0; spacer < 6; spacer++) { //whole bunch of unknowns always 0's
			outapp->WriteUInt8(0);
		}

		QueuePacket(outapp);
		safe_delete(outapp);
	}
	else
		return;
}

void Client::Handle_OP_ItemVerifyRequest(const EQApplicationPacket *app)
{
	using EQEmu::CastingSlot;
	if (app->size != sizeof(ItemVerifyRequest_Struct))
	{
		Log(Logs::General, Logs::Error, "OP size error: OP_ItemVerifyRequest expected:%i got:%i", sizeof(ItemVerifyRequest_Struct), app->size);
		return;
	}

	ItemVerifyRequest_Struct* request = (ItemVerifyRequest_Struct*)app->pBuffer;
	int32 slot_id;
	int32 target_id;
	int32 spell_id = 0;
	slot_id = request->slot;
	target_id = request->target;


	EQApplicationPacket *outapp = nullptr;
	outapp = new EQApplicationPacket(OP_ItemVerifyReply, sizeof(ItemVerifyReply_Struct));
	ItemVerifyReply_Struct* reply = (ItemVerifyReply_Struct*)outapp->pBuffer;
	reply->slot = slot_id;
	reply->target = target_id;

	QueuePacket(outapp);
	safe_delete(outapp);


	if (IsAIControlled()) {
		this->Message_StringID(13, NOT_IN_CONTROL);
		return;
	}

	if (slot_id < 0) {
		Log(Logs::General, Logs::None, "Unknown slot being used by %s, slot being used is: %i", GetName(), request->slot);
		return;
	}

	const EQEmu::ItemInstance* inst = m_inv[slot_id];
	if (!inst) {
		Message(0, "Error: item not found in inventory slot #%i", slot_id);
		DeleteItemInInventory(slot_id, 0, true);
		return;
	}

	const EQEmu::ItemData* item = inst->GetItem();
	if (!item) {
		Message(0, "Error: item not found in inventory slot #%i", slot_id);
		DeleteItemInInventory(slot_id, 0, true);
		return;
	}

	spell_id = item->Click.Effect;

	if
		(
			spell_id > 0 &&
			(
				!IsValidSpell(spell_id) ||
				casting_spell_id ||
				delaytimer ||
				spellend_timer.Enabled() ||
				IsStunned() ||
				IsFeared() ||
				IsMezzed() ||
				DivineAura() ||
				(spells[spell_id].targettype == ST_Ring) ||
				(IsSilenced() && !IsDiscipline(spell_id)) ||
				(IsAmnesiad() && IsDiscipline(spell_id)) ||
				(IsDetrimentalSpell(spell_id) && !zone->CanDoCombat()) ||
				(inst->IsScaling() && inst->GetExp() <= 0) // charms don't have spells when less than 0
			)
		)
	{
		SendSpellBarEnable(spell_id);
		return;
	}

	// Modern clients don't require pet targeted for item clicks that are ST_Pet
	if (spell_id > 0 && (spells[spell_id].targettype == ST_Pet || spells[spell_id].targettype == ST_SummonedPet))
		target_id = GetPetID();

	Log(Logs::General, Logs::None, "OP ItemVerifyRequest: spell=%i, target=%i, inv=%i", spell_id, target_id, slot_id);

	if (m_inv.SupportsClickCasting(slot_id) || ((item->ItemType == EQEmu::item::ItemTypePotion || item->PotionBelt) && m_inv.SupportsPotionBeltCasting(slot_id))) // sanity check
	{
		EQEmu::ItemInstance* p_inst = (EQEmu::ItemInstance*)inst;

		parse->EventItem(EVENT_ITEM_CLICK, this, p_inst, nullptr, "", slot_id);
		inst = m_inv[slot_id];
		if (!inst)
		{
			return;
		}

		int r;
		bool tryaug = false;
		EQEmu::ItemInstance* clickaug = nullptr;
		EQEmu::ItemData* augitem = nullptr;

		for (r = EQEmu::inventory::socketBegin; r < EQEmu::inventory::SocketCount; r++) {
			const EQEmu::ItemInstance* aug_i = inst->GetAugment(r);
			if (!aug_i)
				continue;
			const EQEmu::ItemData* aug = aug_i->GetItem();
			if (!aug)
				continue;

			if ((aug->Click.Type == EQEmu::item::ItemEffectClick) || (aug->Click.Type == EQEmu::item::ItemEffectExpendable) || (aug->Click.Type == EQEmu::item::ItemEffectEquipClick) || (aug->Click.Type == EQEmu::item::ItemEffectClick2))
			{
				tryaug = true;
				clickaug = (EQEmu::ItemInstance*)aug_i;
				augitem = (EQEmu::ItemData*)aug;
				spell_id = aug->Click.Effect;
				break;
			}
		}

		if ((spell_id <= 0) && (item->ItemType != EQEmu::item::ItemTypeFood && item->ItemType != EQEmu::item::ItemTypeDrink && item->ItemType != EQEmu::item::ItemTypeAlcohol && item->ItemType != EQEmu::item::ItemTypeSpell))
		{
			Log(Logs::General, Logs::None, "Item with no effect right clicked by %s", GetName());
		}
		else if (inst->IsClassCommon())
		{
			if (item->ItemType == EQEmu::item::ItemTypeSpell && (strstr((const char*)item->Name, "Tome of ") || strstr((const char*)item->Name, "Skill: ")))
			{
				DeleteItemInInventory(slot_id, 1, true);
				TrainDiscipline(item->ID);
			}
			else if (item->ItemType == EQEmu::item::ItemTypeSpell)
			{
				return;
			}
			else if ((item->Click.Type == EQEmu::item::ItemEffectClick) || (item->Click.Type == EQEmu::item::ItemEffectExpendable) || (item->Click.Type == EQEmu::item::ItemEffectEquipClick) || (item->Click.Type == EQEmu::item::ItemEffectClick2))
			{
				if (inst->GetCharges() == 0)
				{
					//Message(0, "This item is out of charges.");
					Message_StringID(13, ITEM_OUT_OF_CHARGES);
					return;
				}
				if (GetLevel() >= item->Click.Level2)
				{
					int i = parse->EventItem(EVENT_ITEM_CLICK_CAST, this, p_inst, nullptr, "", slot_id);
					inst = m_inv[slot_id];
					if (!inst)
					{
						return;
					}

					if (i == 0) {
						if (!IsCastWhileInvis(item->Click.Effect))
							CommonBreakInvisible(); // client can't do this for us :(
						CastSpell(item->Click.Effect, target_id, CastingSlot::Item, item->CastTime, 0, 0, slot_id);
					}
				}
				else
				{
					Message_StringID(13, ITEMS_INSUFFICIENT_LEVEL);
					return;
				}
			}
			else if (tryaug)
			{
				if (clickaug->GetCharges() == 0)
				{
					//Message(0, "This item is out of charges.");
					Message_StringID(13, ITEM_OUT_OF_CHARGES);
					return;
				}
				if (GetLevel() >= augitem->Click.Level2)
				{
					int i = parse->EventItem(EVENT_ITEM_CLICK_CAST, this, clickaug, nullptr, "", slot_id);
					inst = m_inv[slot_id];
					if (!inst)
					{
						return;
					}

					if (i == 0) {
						if (!IsCastWhileInvis(augitem->Click.Effect))
							CommonBreakInvisible(); // client can't do this for us :(
						CastSpell(augitem->Click.Effect, target_id, CastingSlot::Item, augitem->CastTime, 0, 0, slot_id);
					}
				}
				else
				{
					Message_StringID(13, ITEMS_INSUFFICIENT_LEVEL);
					return;
				}
			}
			else
			{
				if (ClientVersion() >= EQEmu::versions::ClientVersion::SoD && !inst->IsEquipable(GetBaseRace(), GetClass()))
				{
					if (item->ItemType != EQEmu::item::ItemTypeFood && item->ItemType != EQEmu::item::ItemTypeDrink && item->ItemType != EQEmu::item::ItemTypeAlcohol)
					{
						Log(Logs::General, Logs::None, "Error: unknown item->Click.Type (%i)", item->Click.Type);
					}
					else
					{
						//This is food/drink - consume it
						if (item->ItemType == EQEmu::item::ItemTypeFood && m_pp.hunger_level < 5000)
						{
							Consume(item, item->ItemType, slot_id, false);
						}
						else if (item->ItemType == EQEmu::item::ItemTypeDrink && m_pp.thirst_level < 5000)
						{
							Consume(item, item->ItemType, slot_id, false);
						}
						else if (item->ItemType == EQEmu::item::ItemTypeAlcohol)
						{
#if EQDEBUG >= 1
							Log(Logs::General, Logs::None, "Drinking Alcohol from slot:%i", slot_id);
#endif
							// This Seems to be handled in OP_DeleteItem handling
							//DeleteItemInInventory(slot_id, 1, false);
							//entity_list.MessageClose_StringID(this, true, 50, 0, DRINKING_MESSAGE, GetName(), item->Name);
							//Should add intoxication level to the PP at some point
							//CheckIncreaseSkill(ALCOHOL_TOLERANCE, nullptr, 25);
						}

						if (m_pp.hunger_level > 6000)
							m_pp.hunger_level = 6000;
						if (m_pp.thirst_level > 6000)
							m_pp.thirst_level = 6000;

						EQApplicationPacket *outapp2 = nullptr;
						outapp2 = new EQApplicationPacket(OP_Stamina, sizeof(Stamina_Struct));
						Stamina_Struct* sta = (Stamina_Struct*)outapp2->pBuffer;
						sta->food = m_pp.hunger_level;
						sta->water = m_pp.thirst_level;

						QueuePacket(outapp2);
						safe_delete(outapp2);
					}

				}
				else
				{
					Log(Logs::General, Logs::None, "Error: unknown item->Click.Type (%i)", item->Click.Type);
				}
			}
		}
		else
		{
			Message(0, "Error: item not found in inventory slot #%i", slot_id);
		}
	}
	else
	{
		Message(0, "Error: Invalid inventory slot for using effects (inventory slot #%i)", slot_id);
	}

	return;
}

void Client::Handle_OP_Jump(const EQApplicationPacket *app)
{
	SetEndurance(GetEndurance() - (GetLevel()<20 ? (225 * GetLevel() / 100) : 50));
	return;
}

void Client::Handle_OP_KeyRing(const EQApplicationPacket *app)
{
	KeyRingList();
}

void Client::Handle_OP_LDoNButton(const EQApplicationPacket *app)
{
	if (app->size < sizeof(bool))
	{
		return;
	}

	if (GetPendingAdventureCreate())
	{
		return;
	}

	if (IsOnAdventure())
	{
		return;
	}

	bool* p = (bool*)app->pBuffer;
	if (*p == true)
	{
		auto pack =
			new ServerPacket(ServerOP_AdventureRequestCreate,
				sizeof(ServerAdventureRequestCreate_Struct) + (64 * adv_requested_member_count));
		ServerAdventureRequestCreate_Struct *sac = (ServerAdventureRequestCreate_Struct*)pack->pBuffer;
		strcpy(sac->leader, GetName());
		sac->id = adv_requested_id;
		sac->theme = adv_requested_theme;
		sac->member_count = adv_requested_member_count;
		memcpy((pack->pBuffer + sizeof(ServerAdventureRequestCreate_Struct)), adv_requested_data, (64 * adv_requested_member_count));
		worldserver.SendPacket(pack);
		delete pack;
		PendingAdventureCreate();
		ClearPendingAdventureData();
	}
	else
	{
		ClearPendingAdventureData();
	}
}

void Client::Handle_OP_LDoNDisarmTraps(const EQApplicationPacket *app)
{
	Mob * target = GetTarget();
	if (target->IsNPC())
	{
		if (HasSkill(EQEmu::skills::SkillDisarmTraps))
		{
			if (DistanceSquaredNoZ(m_Position, target->GetPosition()) > RuleI(Adventure, LDoNTrapDistanceUse))
			{
				Message(13, "%s is too far away.", target->GetCleanName());
				return;
			}
			HandleLDoNDisarm(target->CastToNPC(), GetSkill(EQEmu::skills::SkillDisarmTraps), LDoNTypeMechanical);
		}
		else
			Message(13, "You do not have the disarm trap skill.");
	}
}

void Client::Handle_OP_LDoNInspect(const EQApplicationPacket *app)
{
	Mob * target = GetTarget();
	if (target && target->GetClass() == LDON_TREASURE)
		Message(15, "%s", target->GetCleanName());
}

void Client::Handle_OP_LDoNOpen(const EQApplicationPacket *app)
{
	Mob * target = GetTarget();
	if (target && target->IsNPC())
		HandleLDoNOpen(target->CastToNPC());
}

void Client::Handle_OP_LDoNPickLock(const EQApplicationPacket *app)
{
	Mob * target = GetTarget();
	if (target->IsNPC())
	{
		if (HasSkill(EQEmu::skills::SkillPickLock))
		{
			if (DistanceSquaredNoZ(m_Position, target->GetPosition()) > RuleI(Adventure, LDoNTrapDistanceUse))
			{
				Message(13, "%s is too far away.", target->GetCleanName());
				return;
			}
			HandleLDoNPickLock(target->CastToNPC(), GetSkill(EQEmu::skills::SkillPickLock), LDoNTypeMechanical);
		}
		else
			Message(13, "You do not have the pick locks skill.");
	}
}

void Client::Handle_OP_LDoNSenseTraps(const EQApplicationPacket *app)
{
	Mob * target = GetTarget();
	if (target->IsNPC())
	{
		if (HasSkill(EQEmu::skills::SkillSenseTraps))
		{
			if (DistanceSquaredNoZ(m_Position, target->GetPosition()) > RuleI(Adventure, LDoNTrapDistanceUse))
			{
				Message(13, "%s is too far away.", target->GetCleanName());
				return;
			}
			HandleLDoNSenseTraps(target->CastToNPC(), GetSkill(EQEmu::skills::SkillSenseTraps), LDoNTypeMechanical);
		}
		else
			Message(13, "You do not have the sense traps skill.");
	}
}

void Client::Handle_OP_LeadershipExpToggle(const EQApplicationPacket *app)
{
	if (app->size != 1) {
		Log(Logs::General, Logs::None, "Size mismatch in OP_LeadershipExpToggle expected %i got %i", 1, app->size);
		DumpPacket(app);
		return;
	}
	uint8 *mode = (uint8 *)app->pBuffer;
	if (*mode) {
		m_pp.leadAAActive = 1;
		Save();
		Message_StringID(clientMessageYellow, LEADERSHIP_EXP_ON);
	}
	else {
		m_pp.leadAAActive = 0;
		Save();
		Message_StringID(clientMessageYellow, LEADERSHIP_EXP_OFF);
	}
}

void Client::Handle_OP_LeaveAdventure(const EQApplicationPacket *app)
{
	if (!IsOnAdventure())
	{
		return;
	}
	LeaveAdventure();
}

void Client::Handle_OP_LeaveBoat(const EQApplicationPacket *app)
{
	Mob* boat = entity_list.GetMob(this->BoatID);	// find the mob corresponding to the boat id
	if (boat) {
		if ((boat->GetTarget() == this) && boat->GetHateAmount(this) == 0)	// if the client somehow left while still controlling the boat (and the boat isn't attacking them)
			boat->SetTarget(0);			// fix it to stop later problems
	}
	this->BoatID = 0;
	return;
}

void Client::Handle_OP_LFGCommand(const EQApplicationPacket *app)
{
	if (app->size != sizeof(LFG_Struct)) {
		std::cout << "Wrong size on OP_LFGCommand. Got: " << app->size << ", Expected: " << sizeof(LFG_Struct) << std::endl;
		DumpPacket(app);
		return;
	}

	// Process incoming packet
	LFG_Struct* lfg = (LFG_Struct*)app->pBuffer;

	switch (lfg->value & 0xFF) {
	case 0:
		if (LFG) {
			database.SetLFG(CharacterID(), false);
			LFG = false;
			LFGComments[0] = '\0';
		}
		break;
	case 1:
		if (!LFG) {
			LFG = true;
			database.SetLFG(CharacterID(), true);
		}
		LFGFromLevel = lfg->FromLevel;
		LFGToLevel = lfg->ToLevel;
		LFGMatchFilter = lfg->MatchFilter;
		strcpy(LFGComments, lfg->Comments);
		break;
	default:
		Message(0, "Error: unknown LFG value %i", lfg->value);
	}

	UpdateWho();

	// Issue outgoing packet to notify other clients
	auto outapp = new EQApplicationPacket(OP_LFGAppearance, sizeof(LFG_Appearance_Struct));
	LFG_Appearance_Struct* lfga = (LFG_Appearance_Struct*)outapp->pBuffer;
	lfga->spawn_id = this->GetID();
	lfga->lfg = (uint8)LFG;

	entity_list.QueueClients(this, outapp, true);
	safe_delete(outapp);
	return;
}

void Client::Handle_OP_LFGGetMatchesRequest(const EQApplicationPacket *app)
{

	if (app->size != sizeof(LFGGetMatchesRequest_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_LFGGetMatchesRequest, size=%i, expected %i", app->size, sizeof(LFGGetMatchesRequest_Struct));
		DumpPacket(app);
		return;
	}
	LFGGetMatchesRequest_Struct* gmrs = (LFGGetMatchesRequest_Struct*)app->pBuffer;

	if (!worldserver.Connected())
		Message(0, "Error: World server disconnected");
	else {
		auto pack = new ServerPacket(ServerOP_LFGMatches, sizeof(ServerLFGMatchesRequest_Struct));
		ServerLFGMatchesRequest_Struct* smrs = (ServerLFGMatchesRequest_Struct*)pack->pBuffer;
		smrs->FromID = GetID();
		smrs->QuerierLevel = GetLevel();
		strcpy(smrs->FromName, GetName());
		smrs->FromLevel = gmrs->FromLevel;
		smrs->ToLevel = gmrs->ToLevel;
		smrs->Classes = gmrs->Classes;
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void Client::Handle_OP_LFGuild(const EQApplicationPacket *app)
{
	if (app->size < 4)
		return;

	uint32 Command = *((uint32 *)app->pBuffer);

	switch (Command)
	{
	case 0:
	{
		VERIFY_PACKET_LENGTH(OP_LFGuild, app, LFGuild_PlayerToggle_Struct);
		LFGuild_PlayerToggle_Struct *pts = (LFGuild_PlayerToggle_Struct *)app->pBuffer;

#ifdef DARWIN
#if __DARWIN_C_LEVEL < 200809L
		if (strlen(pts->Comment) > 256)
#else
		if (strnlen(pts->Comment, 256) > 256)
#endif // __DARWIN_C_LEVEL
#else
		if (strnlen(pts->Comment, 256) > 256)
#endif // DARWIN
			return;

		auto pack = new ServerPacket(ServerOP_QueryServGeneric, strlen(GetName()) + strlen(pts->Comment) + 38);

		pack->WriteUInt32(zone->GetZoneID());
		pack->WriteUInt32(zone->GetInstanceID());
		pack->WriteString(GetName());
		pack->WriteUInt32(QSG_LFGuild);
		pack->WriteUInt32(QSG_LFGuild_UpdatePlayerInfo);
		pack->WriteUInt32(GetBaseClass());
		pack->WriteUInt32(GetLevel());
		pack->WriteUInt32(GetSpentAA());
		pack->WriteString(pts->Comment);
		pack->WriteUInt32(pts->Toggle);
		pack->WriteUInt32(pts->TimeZone);

		worldserver.SendPacket(pack);
		safe_delete(pack);

		break;
	}
	case 1:
	{
		VERIFY_PACKET_LENGTH(OP_LFGuild, app, LFGuild_GuildToggle_Struct);
		LFGuild_GuildToggle_Struct *gts = (LFGuild_GuildToggle_Struct *)app->pBuffer;

#ifdef DARWIN
#if __DARWIN_C_LEVEL < 200809L
		if (strlen(gts->Comment) > 256)
#else
		if (strnlen(gts->Comment, 256) > 256)
#endif // __DARWIN_C_LEVEL
#else
		if (strnlen(gts->Comment, 256) > 256)
#endif // __DARWIN
			return;

		auto pack =
			new ServerPacket(ServerOP_QueryServGeneric, strlen(GetName()) + strlen(gts->Comment) +
				strlen(guild_mgr.GetGuildName(GuildID())) + 43);

		pack->WriteUInt32(zone->GetZoneID());
		pack->WriteUInt32(zone->GetInstanceID());
		pack->WriteString(GetName());
		pack->WriteUInt32(QSG_LFGuild);
		pack->WriteUInt32(QSG_LFGuild_UpdateGuildInfo);
		pack->WriteString(guild_mgr.GetGuildName(GuildID()));
		pack->WriteString(gts->Comment);
		pack->WriteUInt32(gts->FromLevel);
		pack->WriteUInt32(gts->ToLevel);
		pack->WriteUInt32(gts->Classes);
		pack->WriteUInt32(gts->AACount);
		pack->WriteUInt32(gts->Toggle);
		pack->WriteUInt32(gts->TimeZone);

		worldserver.SendPacket(pack);
		safe_delete(pack);

		break;
	}
	case 3:
	{
		VERIFY_PACKET_LENGTH(OP_LFGuild, app, LFGuild_SearchPlayer_Struct);

		auto pack = new ServerPacket(ServerOP_QueryServGeneric, strlen(GetName()) + 37);

		pack->WriteUInt32(zone->GetZoneID());
		pack->WriteUInt32(zone->GetInstanceID());
		pack->WriteString(GetName());
		pack->WriteUInt32(QSG_LFGuild);
		pack->WriteUInt32(QSG_LFGuild_PlayerMatches);

		LFGuild_SearchPlayer_Struct *sps = (LFGuild_SearchPlayer_Struct *)app->pBuffer;
		pack->WriteUInt32(sps->FromLevel);
		pack->WriteUInt32(sps->ToLevel);
		pack->WriteUInt32(sps->MinAA);
		pack->WriteUInt32(sps->TimeZone);
		pack->WriteUInt32(sps->Classes);

		worldserver.SendPacket(pack);
		safe_delete(pack);

		break;
	}
	case 4:
	{
		VERIFY_PACKET_LENGTH(OP_LFGuild, app, LFGuild_SearchGuild_Struct);

		auto pack = new ServerPacket(ServerOP_QueryServGeneric, strlen(GetName()) + 33);

		pack->WriteUInt32(zone->GetZoneID());
		pack->WriteUInt32(zone->GetInstanceID());
		pack->WriteString(GetName());
		pack->WriteUInt32(QSG_LFGuild);
		pack->WriteUInt32(QSG_LFGuild_GuildMatches);

		LFGuild_SearchGuild_Struct *sgs = (LFGuild_SearchGuild_Struct *)app->pBuffer;

		pack->WriteUInt32(sgs->Level);
		pack->WriteUInt32(sgs->AAPoints);
		pack->WriteUInt32(sgs->TimeZone);
		pack->WriteUInt32(sgs->Class);

		worldserver.SendPacket(pack);
		safe_delete(pack);

		break;
	}
	default:
		break;
	}
}

void Client::Handle_OP_LFPCommand(const EQApplicationPacket *app)
{

	if (app->size != sizeof(LFP_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_LFPCommand, size=%i, expected %i", app->size, sizeof(LFP_Struct));
		DumpPacket(app);
		return;
	}
	LFP_Struct *lfp = (LFP_Struct*)app->pBuffer;

	LFP = lfp->Action != LFPOff;
	database.SetLFP(CharacterID(), LFP);

	if (!LFP) {
		worldserver.StopLFP(CharacterID());
		return;
	}

	GroupLFPMemberEntry LFPMembers[MAX_GROUP_MEMBERS];

	for (unsigned int i = 0; i<MAX_GROUP_MEMBERS; i++) {
		LFPMembers[i].Name[0] = '\0';
		LFPMembers[i].Class = 0;
		LFPMembers[i].Level = 0;
		LFPMembers[i].Zone = 0;
		LFPMembers[i].GuildID = 0xFFFF;
	}

	Group *g = GetGroup();

	// Slot 0 is always for the group leader, or the player if not in a group
	strcpy(LFPMembers[0].Name, GetName());
	LFPMembers[0].Class = GetClass();
	LFPMembers[0].Level = GetLevel();
	LFPMembers[0].Zone = zone->GetZoneID();
	LFPMembers[0].GuildID = GuildID();

	if (g) {
		// This should not happen. The client checks if you are in a group and will not let you put LFP on if
		// you are not the leader.
		if (!g->IsLeader(this)) {
			Log(Logs::General, Logs::Error, "Client sent LFP on for character %s who is grouped but not leader.", GetName());
			return;
		}
		// Fill the LFPMembers array with the rest of the group members, excluding ourself
		// We don't fill in the class, level or zone, because we may not be able to determine
		// them if the other group members are not in this zone. World will fill in this information
		// for us, if it can.
		int NextFreeSlot = 1;
		for (unsigned int i = 0; i < MAX_GROUP_MEMBERS; i++) {
			if (strcasecmp(g->membername[i], LFPMembers[0].Name))
				strcpy(LFPMembers[NextFreeSlot++].Name, g->membername[i]);
		}
	}


	worldserver.UpdateLFP(CharacterID(), lfp->Action, lfp->MatchFilter, lfp->FromLevel, lfp->ToLevel, lfp->Classes,
		lfp->Comments, LFPMembers);


}

void Client::Handle_OP_LFPGetMatchesRequest(const EQApplicationPacket *app)
{

	if (app->size != sizeof(LFPGetMatchesRequest_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_LFPGetMatchesRequest, size=%i, expected %i", app->size, sizeof(LFPGetMatchesRequest_Struct));
		DumpPacket(app);
		return;
	}
	LFPGetMatchesRequest_Struct* gmrs = (LFPGetMatchesRequest_Struct*)app->pBuffer;

	if (!worldserver.Connected())
		Message(0, "Error: World server disconnected");
	else {
		auto pack = new ServerPacket(ServerOP_LFPMatches, sizeof(ServerLFPMatchesRequest_Struct));
		ServerLFPMatchesRequest_Struct* smrs = (ServerLFPMatchesRequest_Struct*)pack->pBuffer;
		smrs->FromID = GetID();
		smrs->FromLevel = gmrs->FromLevel;
		smrs->ToLevel = gmrs->ToLevel;
		smrs->QuerierLevel = GetLevel();
		smrs->QuerierClass = GetClass();
		strcpy(smrs->FromName, GetName());
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}

	return;
}

void Client::Handle_OP_LoadSpellSet(const EQApplicationPacket *app)
{
	if (app->size != sizeof(LoadSpellSet_Struct)) {
		printf("Wrong size of LoadSpellSet_Struct! Expected: %zu, Got: %i\n", sizeof(LoadSpellSet_Struct), app->size);
		return;
	}
	int i;
	LoadSpellSet_Struct* ss = (LoadSpellSet_Struct*)app->pBuffer;
	for (i = 0; i<MAX_PP_MEMSPELL; i++) {
		if (ss->spell[i] != 0xFFFFFFFF)
			UnmemSpell(i, true);
	}
}

void Client::Handle_OP_Logout(const EQApplicationPacket *app)
{
	Log(Logs::Detail, Logs::None, "%s sent a logout packet.", GetName());

	SendLogoutPackets();

	auto outapp = new EQApplicationPacket(OP_LogoutReply);
	FastQueuePacket(&outapp);

	Disconnect();
	return;
}

void Client::Handle_OP_LootItem(const EQApplicationPacket *app)
{
	if (app->size != sizeof(LootingItem_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_LootItem, size=%i, expected %i", app->size, sizeof(LootingItem_Struct));
		return;
	}

	EQApplicationPacket* outapp = nullptr;
	Entity* entity = entity_list.GetID(*((uint16*)app->pBuffer));
	if (entity == 0) {
		Message(13, "Error: OP_LootItem: Corpse not found (ent = 0)");
		outapp = new EQApplicationPacket(OP_LootComplete, 0);
		QueuePacket(outapp);
		safe_delete(outapp);
		return;
	}

	if (entity->IsCorpse()) {
		entity->CastToCorpse()->LootItem(this, app);
		return;
	}
	else {
		Message(13, "Error: Corpse not found! (!ent->IsCorpse())");
		Corpse::SendEndLootErrorPacket(this);
	}

	return;
}

void Client::Handle_OP_LootRequest(const EQApplicationPacket *app)
{
	if (app->size != sizeof(uint32)) {
		std::cout << "Wrong size: OP_LootRequest, size=" << app->size << ", expected " << sizeof(uint32) << std::endl;
		return;
	}

	Entity* ent = entity_list.GetID(*((uint32*)app->pBuffer));
	if (ent == 0) {
		Message(13, "Error: OP_LootRequest: Corpse not found (ent = 0)");
		Corpse::SendLootReqErrorPacket(this);
		return;
	}
	if (ent->IsCorpse())
	{
		SetLooting(ent->GetID()); //store the entity we are looting

		ent->CastToCorpse()->MakeLootRequestPackets(this, app);
		return;
	}
	else {
		std::cout << "npc == 0 LOOTING FOOKED3" << std::endl;
		Message(13, "Error: OP_LootRequest: Corpse not a corpse?");
		Corpse::SendLootReqErrorPacket(this);
	}
	return;
}

void Client::Handle_OP_ManaChange(const EQApplicationPacket *app)
{
	if (app->size == 0) {
		// i think thats the sign to stop the songs
		if (IsBardSong(casting_spell_id) || bardsong != 0)
			InterruptSpell(SONG_ENDS, 0x121);
		else
			InterruptSpell(INTERRUPT_SPELL, 0x121);

		return;
	}
	else	// I don't think the client sends proper manachanges
	{			// with a length, just the 0 len ones for stopping songs
				//ManaChange_Struct* p = (ManaChange_Struct*)app->pBuffer;
		printf("OP_ManaChange from client:\n");
		DumpPacket(app);
	}
	return;
}

/*
#if 0	// I dont think there's an op for this now, and we check this
// when the client is sitting
void Client::Handle_OP_Medding(const EQApplicationPacket *app)
{
if (app->pBuffer[0])
medding = true;
else
medding = false;
return;
}
#endif
*/

void Client::Handle_OP_MemorizeSpell(const EQApplicationPacket *app)
{
	OPMemorizeSpell(app);
	return;
}

void Client::Handle_OP_Mend(const EQApplicationPacket *app)
{
	if (!HasSkill(EQEmu::skills::SkillMend))
		return;

	if (!p_timers.Expired(&database, pTimerMend, false)) {
		Message(13, "Ability recovery time not yet met.");
		return;
	}
	p_timers.Start(pTimerMend, MendReuseTime - 1);

	int mendhp = GetMaxHP() / 4;
	int currenthp = GetHP();
	if (zone->random.Int(0, 199) < (int)GetSkill(EQEmu::skills::SkillMend)) {

		int criticalchance = spellbonuses.CriticalMend + itembonuses.CriticalMend + aabonuses.CriticalMend;

		if (zone->random.Int(0, 99) < criticalchance) {
			mendhp *= 2;
			Message_StringID(4, MEND_CRITICAL);
		}
		SetHP(GetHP() + mendhp);
		SendHPUpdate();
		Message_StringID(4, MEND_SUCCESS);
	}
	else {
		/* the purpose of the following is to make the chance to worsen wounds much less common,
		which is more consistent with the way eq live works.
		according to my math, this should result in the following probability:
		0 skill - 25% chance to worsen
		20 skill - 23% chance to worsen
		50 skill - 16% chance to worsen */
		if ((GetSkill(EQEmu::skills::SkillMend) <= 75) && (zone->random.Int(GetSkill(EQEmu::skills::SkillMend), 100) < 75) && (zone->random.Int(1, 3) == 1))
		{
			SetHP(currenthp > mendhp ? (GetHP() - mendhp) : 1);
			SendHPUpdate();
			Message_StringID(4, MEND_WORSEN);
		}
		else
			Message_StringID(4, MEND_FAIL);
	}

	CheckIncreaseSkill(EQEmu::skills::SkillMend, nullptr, 10);
	return;
}

void Client::Handle_OP_MercenaryCommand(const EQApplicationPacket *app)
{
	if (app->size != sizeof(MercenaryCommand_Struct))
	{
		Message(13, "Size mismatch in OP_MercenaryCommand expected %i got %i", sizeof(MercenaryCommand_Struct), app->size);
		Log(Logs::General, Logs::None, "Size mismatch in OP_MercenaryCommand expected %i got %i", sizeof(MercenaryCommand_Struct), app->size);
		DumpPacket(app);
		return;
	}

	MercenaryCommand_Struct* mc = (MercenaryCommand_Struct*)app->pBuffer;
	uint32 merc_command = mc->MercCommand;	// Seen 0 (zone in with no merc or suspended), 1 (dismiss merc), 5 (normal state), 20 (unknown), 36 (zone in with merc)
	int32 option = mc->Option;	// Seen -1 (zone in with no merc), 0 (setting to passive stance), 1 (normal or setting to balanced stance)

	Log(Logs::General, Logs::Mercenaries, "Command %i, Option %i received from %s.", merc_command, option, GetName());

	if (!RuleB(Mercs, AllowMercs))
		return;

	// Handle the Command here...
	// Will need a list of what every type of command is supposed to do
	// Unsure if there is a server response to this packet
	if (option >= 0)
	{
		Merc* merc = GetMerc();
		GetMercInfo().State = option;

		if (merc)
		{
			uint8 numStances = 0;

			//get number of available stances for the current merc
			std::list<MercStanceInfo> mercStanceList = zone->merc_stance_list[merc->GetMercTemplateID()];
			auto iter = mercStanceList.begin();
			while (iter != mercStanceList.end()) {
				numStances++;
				++iter;
			}

			MercTemplate* mercTemplate = zone->GetMercTemplate(GetMerc()->GetMercTemplateID());
			if (mercTemplate)
			{
				//check to see if selected option is a valid stance slot (option is the slot the stance is in, not the actual stance)
				if (option >= 0 && option < numStances)
				{
					merc->SetStance(mercTemplate->Stances[option]);
					GetMercInfo().Stance = mercTemplate->Stances[option];

					Log(Logs::General, Logs::Mercenaries, "Set Stance: %u for %s (%s)", merc->GetStance(), merc->GetName(), GetName());
				}
			}
		}
	}
}

void Client::Handle_OP_MercenaryDataRequest(const EQApplicationPacket *app)
{
	// The payload is 4 bytes. The EntityID of the Mercenary Liason which are of class 71.
	if (app->size != sizeof(MercenaryMerchantShopRequest_Struct))
	{
		Log(Logs::General, Logs::None, "Size mismatch in OP_MercenaryDataRequest expected 4 got %i", app->size);

		DumpPacket(app);

		return;
	}

	MercenaryMerchantShopRequest_Struct* mmsr = (MercenaryMerchantShopRequest_Struct*)app->pBuffer;
	uint32 merchant_id = mmsr->MercMerchantID;
	uint32 altCurrentType = 19;

	Log(Logs::General, Logs::Mercenaries, "Data Request for Merchant ID (%i) for %s.", merchant_id, GetName());

	//client is requesting data about currently owned mercenary
	if (merchant_id == 0) {

		//send info about your current merc(s)
		if (GetMercInfo().mercid)
		{
			Log(Logs::General, Logs::Mercenaries, "SendMercPersonalInfo Request for %s.", GetName());
			SendMercPersonalInfo();
		}
		else
		{
			Log(Logs::General, Logs::Mercenaries, "SendMercPersonalInfo Not Sent - MercID (%i) for %s.", GetMercInfo().mercid, GetName());
		}
	}

	if (!RuleB(Mercs, AllowMercs)) {
		return;
	}

	NPC* tar = entity_list.GetNPCByID(merchant_id);

	if (tar) {
		int mercTypeCount = 0;
		int mercCount = 0;

		if (DistanceSquared(m_Position, tar->GetPosition()) > USE_NPC_RANGE2)
			return;

		if (tar->GetClass() != MERCERNARY_MASTER) {
			return;
		}

		mercTypeCount = tar->GetNumMercTypes(static_cast<unsigned int>(ClientVersion()));
		mercCount = tar->GetNumMercs(static_cast<unsigned int>(ClientVersion()));

		if (mercCount > MAX_MERC)
			return;

		std::list<MercType> mercTypeList = tar->GetMercTypesList(static_cast<unsigned int>(ClientVersion()));
		std::list<MercData> mercDataList = tar->GetMercsList(static_cast<unsigned int>(ClientVersion()));

		int i = 0;
		int StanceCount = 0;

		for (auto mercListItr = mercDataList.begin(); mercListItr != mercDataList.end(); ++mercListItr) {
			auto siter = zone->merc_stance_list[mercListItr->MercTemplateID].begin();
			for (siter = zone->merc_stance_list[mercListItr->MercTemplateID].begin(); siter != zone->merc_stance_list[mercListItr->MercTemplateID].end(); ++siter)
			{
				StanceCount++;
			}
		}

		auto outapp = new EQApplicationPacket(OP_MercenaryDataResponse, sizeof(MercenaryMerchantList_Struct));
		MercenaryMerchantList_Struct* mml = (MercenaryMerchantList_Struct*)outapp->pBuffer;

		mml->MercTypeCount = mercTypeCount;
		if (mercTypeCount > 0)
		{
			for (auto mercTypeListItr = mercTypeList.begin(); mercTypeListItr != mercTypeList.end();
			++mercTypeListItr) {
				mml->MercGrades[i] = mercTypeListItr->Type;	// DBStringID for Type
				i++;
			}
		}
		mml->MercCount = mercCount;

		if (mercCount > 0)
		{
			i = 0;
			for (auto mercListIter = mercDataList.begin(); mercListIter != mercDataList.end();
			++mercListIter) {
				mml->Mercs[i].MercID = mercListIter->MercTemplateID;
				mml->Mercs[i].MercType = mercListIter->MercType;
				mml->Mercs[i].MercSubType = mercListIter->MercSubType;
				mml->Mercs[i].PurchaseCost = RuleB(Mercs, ChargeMercPurchaseCost) ? Merc::CalcPurchaseCost(mercListIter->MercTemplateID, GetLevel(), 0) : 0;
				mml->Mercs[i].UpkeepCost = RuleB(Mercs, ChargeMercUpkeepCost) ? Merc::CalcUpkeepCost(mercListIter->MercTemplateID, GetLevel(), 0) : 0;
				mml->Mercs[i].Status = 0;
				mml->Mercs[i].AltCurrencyCost = RuleB(Mercs, ChargeMercPurchaseCost) ? Merc::CalcPurchaseCost(mercListIter->MercTemplateID, GetLevel(), altCurrentType) : 0;
				mml->Mercs[i].AltCurrencyUpkeep = RuleB(Mercs, ChargeMercUpkeepCost) ? Merc::CalcUpkeepCost(mercListIter->MercTemplateID, GetLevel(), altCurrentType) : 0;
				mml->Mercs[i].AltCurrencyType = altCurrentType;
				mml->Mercs[i].MercUnk01 = 0;
				mml->Mercs[i].TimeLeft = -1;
				mml->Mercs[i].MerchantSlot = i + 1;
				mml->Mercs[i].MercUnk02 = 1;
				int mercStanceCount = 0;
				auto iter = zone->merc_stance_list[mercListIter->MercTemplateID].begin();
				for (iter = zone->merc_stance_list[mercListIter->MercTemplateID].begin(); iter != zone->merc_stance_list[mercListIter->MercTemplateID].end(); ++iter)
				{
					mercStanceCount++;
				}
				mml->Mercs[i].StanceCount = mercStanceCount;
				mml->Mercs[i].MercUnk03 = 519044964;
				mml->Mercs[i].MercUnk04 = 1;
				//mml->Mercs[i].MercName;
				int stanceindex = 0;
				if (mercStanceCount > 0)
				{
					auto iter2 = zone->merc_stance_list[mercListIter->MercTemplateID].begin();
					while (iter2 != zone->merc_stance_list[mercListIter->MercTemplateID].end())
					{
						mml->Mercs[i].Stances[stanceindex].StanceIndex = stanceindex;
						mml->Mercs[i].Stances[stanceindex].Stance = (iter2->StanceID);
						stanceindex++;
						++iter2;
					}
				}
				i++;
			}
		}
		FastQueuePacket(&outapp);
	}
}

void Client::Handle_OP_MercenaryDataUpdateRequest(const EQApplicationPacket *app)
{
	// The payload is 0 bytes.
	if (app->size != 0)
	{
		Message(13, "Size mismatch in OP_MercenaryDataUpdateRequest expected 0 got %i", app->size);
		Log(Logs::General, Logs::None, "Size mismatch in OP_MercenaryDataUpdateRequest expected 0 got %i", app->size);
		DumpPacket(app);
		return;
	}

	Log(Logs::General, Logs::Mercenaries, "Data Update Request Received for %s.", GetName());

	if (GetMercID())
	{
		SendMercPersonalInfo();
	}
}

void Client::Handle_OP_MercenaryDismiss(const EQApplicationPacket *app)
{
	// The payload is 0 or 1 bytes.
	if (app->size > 1)
	{
		Message(13, "Size mismatch in OP_MercenaryDismiss expected 0 got %i", app->size);
		Log(Logs::General, Logs::None, "Size mismatch in OP_MercenaryDismiss expected 0 got %i", app->size);
		DumpPacket(app);
		return;
	}

	uint8 Command = 0;
	if (app->size > 0)
	{
		char *InBuffer = (char *)app->pBuffer;
		Command = VARSTRUCT_DECODE_TYPE(uint8, InBuffer);
	}

	Log(Logs::General, Logs::Mercenaries, "Dismiss Request ( %i ) Received for %s.", Command, GetName());

	// Handle the dismiss here...
	DismissMerc(GetMercInfo().mercid);

}

void Client::Handle_OP_MercenaryHire(const EQApplicationPacket *app)
{
	// The payload is 16 bytes. First four bytes are the Merc ID (Template ID)
	if (app->size != sizeof(MercenaryMerchantRequest_Struct))
	{
		Log(Logs::General, Logs::None, "Size mismatch in OP_MercenaryHire expected %i got %i", sizeof(MercenaryMerchantRequest_Struct), app->size);

		DumpPacket(app);

		return;
	}

	MercenaryMerchantRequest_Struct* mmrq = (MercenaryMerchantRequest_Struct*)app->pBuffer;
	uint32 merc_template_id = mmrq->MercID;
	uint32 merchant_id = mmrq->MercMerchantID;
	uint32 merc_unk1 = mmrq->MercUnk01;
	uint32 merc_unk2 = mmrq->MercUnk02;

	Log(Logs::General, Logs::Mercenaries, "Template ID (%i), Merchant ID (%i), Unknown1 (%i), Unknown2 (%i), Client: %s", merc_template_id, merchant_id, merc_unk1, merc_unk2, GetName());

	//HirePending = true;
	SetHoTT(0);
	SendTargetCommand(0);

	if (!RuleB(Mercs, AllowMercs))
		return;

	MercTemplate* merc_template = zone->GetMercTemplate(merc_template_id);

	if (merc_template)
	{

		Mob* merchant = entity_list.GetNPCByID(merchant_id);
		if (!CheckCanHireMerc(merchant, merc_template_id))
		{
			return;
		}

		// Set time remaining to max on Hire
		GetMercInfo().MercTimerRemaining = RuleI(Mercs, UpkeepIntervalMS);

		// Get merc, assign it to client & spawn
		Merc* merc = Merc::LoadMerc(this, merc_template, merchant_id, false);

		if (merc)
		{
			SpawnMerc(merc, true);
			merc->Save();

			if (RuleB(Mercs, ChargeMercPurchaseCost))
			{
				uint32 cost = Merc::CalcPurchaseCost(merc_template->MercTemplateID, GetLevel()) * 100; // Cost is in gold
				TakeMoneyFromPP(cost, true);
			}

			// approved hire request
			SendMercMerchantResponsePacket(0);
		}
		else
		{
			//merc failed to spawn
			SendMercMerchantResponsePacket(3);
		}
	}
	else
	{
		//merc doesn't exist in db
		SendMercMerchantResponsePacket(2);
	}
}

void Client::Handle_OP_MercenarySuspendRequest(const EQApplicationPacket *app)
{
	if (app->size != sizeof(SuspendMercenary_Struct))
	{
		Message(13, "Size mismatch in OP_MercenarySuspendRequest expected %i got %i", sizeof(SuspendMercenary_Struct), app->size);
		Log(Logs::General, Logs::None, "Size mismatch in OP_MercenarySuspendRequest expected %i got %i", sizeof(SuspendMercenary_Struct), app->size);
		DumpPacket(app);
		return;
	}

	SuspendMercenary_Struct* sm = (SuspendMercenary_Struct*)app->pBuffer;
	uint32 merc_suspend = sm->SuspendMerc;	// Seen 30 for suspending or unsuspending

	Log(Logs::General, Logs::Mercenaries, "Suspend ( %i ) received for %s.", merc_suspend, GetName());

	if (!RuleB(Mercs, AllowMercs))
		return;

	// Check if the merc is suspended and if so, unsuspend, otherwise suspend it
	SuspendMercCommand();
}

void Client::Handle_OP_MercenaryTimerRequest(const EQApplicationPacket *app)
{
	// The payload is 0 bytes.
	if (app->size > 1)
	{
		Message(13, "Size mismatch in OP_MercenaryTimerRequest expected 0 got %i", app->size);
		Log(Logs::General, Logs::None, "Size mismatch in OP_MercenaryTimerRequest expected 0 got %i", app->size);
		DumpPacket(app);
		return;
	}

	Log(Logs::General, Logs::Mercenaries, "Timer Request received for %s.", GetName());

	if (!RuleB(Mercs, AllowMercs)) {
		return;
	}

	// To Do: Load Mercenary Timer Data to properly populate this reply packet
	// All hard set values for now
	uint32 entityID = 0;
	uint32 mercState = 5;
	uint32 suspendedTime = 0;
	if (GetMercID()) {
		Merc* merc = GetMerc();

		if (merc) {
			entityID = merc->GetID();

			if (GetMercInfo().IsSuspended) {
				mercState = 1;
				suspendedTime = GetMercInfo().SuspendedTime;
			}
		}
	}

	if (entityID > 0) {
		SendMercTimerPacket(entityID, mercState, suspendedTime, GetMercInfo().MercTimerRemaining, RuleI(Mercs, SuspendIntervalMS));
	}
}

void Client::Handle_OP_MoveCoin(const EQApplicationPacket *app)
{
	if (app->size != sizeof(MoveCoin_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size on OP_MoveCoin. Got: %i, Expected: %i", app->size, sizeof(MoveCoin_Struct));
		DumpPacket(app);
		return;
	}
	OPMoveCoin(app);
	return;
}

void Client::Handle_OP_MoveItem(const EQApplicationPacket *app)
{
	if (!CharacterID())
	{
		return;
	}

	if (app->size != sizeof(MoveItem_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_MoveItem, size=%i, expected %i", app->size, sizeof(MoveItem_Struct));
		return;
	}

	MoveItem_Struct* mi = (MoveItem_Struct*)app->pBuffer;
	if (spellend_timer.Enabled() && casting_spell_id && !IsBardSong(casting_spell_id))
	{
		if (mi->from_slot != mi->to_slot && (mi->from_slot <= EQEmu::legacy::GENERAL_END || mi->from_slot > 39) && IsValidSlot(mi->from_slot) && IsValidSlot(mi->to_slot))
		{
			char *detect = nullptr;
			const EQEmu::ItemInstance *itm_from = GetInv().GetItem(mi->from_slot);
			const EQEmu::ItemInstance *itm_to = GetInv().GetItem(mi->to_slot);
			MakeAnyLenString(&detect, "Player issued a move item from %u(item id %u) to %u(item id %u) while casting %u.",
				mi->from_slot,
				itm_from ? itm_from->GetID() : 0,
				mi->to_slot,
				itm_to ? itm_to->GetID() : 0,
				casting_spell_id);
			database.SetMQDetectionFlag(AccountName(), GetName(), detect, zone->GetShortName());
			safe_delete_array(detect);
			Kick(); // Kick client to prevent client and server from getting out-of-sync inventory slots
			return;
		}
	}

	// Illegal bagslot usage checks. Currently, user only receives a message if this check is triggered.
	bool mi_hack = false;

	if (mi->from_slot >= EQEmu::legacy::GENERAL_BAGS_BEGIN && mi->from_slot <= EQEmu::legacy::CURSOR_BAG_END) {
		if (mi->from_slot >= EQEmu::legacy::CURSOR_BAG_BEGIN) { mi_hack = true; }
		else {
			int16 from_parent = m_inv.CalcSlotId(mi->from_slot);
			if (!m_inv[from_parent]) { mi_hack = true; }
			else if (!m_inv[from_parent]->IsClassBag()) { mi_hack = true; }
			else if (m_inv.CalcBagIdx(mi->from_slot) >= m_inv[from_parent]->GetItem()->BagSlots) { mi_hack = true; }
		}
	}

	if (mi->to_slot >= EQEmu::legacy::GENERAL_BAGS_BEGIN && mi->to_slot <= EQEmu::legacy::CURSOR_BAG_END) {
		if (mi->to_slot >= EQEmu::legacy::CURSOR_BAG_BEGIN) { mi_hack = true; }
		else {
			int16 to_parent = m_inv.CalcSlotId(mi->to_slot);
			if (!m_inv[to_parent]) { mi_hack = true; }
			else if (!m_inv[to_parent]->IsClassBag()) { mi_hack = true; }
			else if (m_inv.CalcBagIdx(mi->to_slot) >= m_inv[to_parent]->GetItem()->BagSlots) { mi_hack = true; }
		}
	}

	if (mi_hack) { Message(15, "Caution: Illegal use of inaccessible bag slots!"); }

	if (!SwapItem(mi) && IsValidSlot(mi->from_slot) && IsValidSlot(mi->to_slot)) {
		SwapItemResync(mi);

		bool error = false;
		InterrogateInventory(this, false, true, false, error, false);
		if (error)
			InterrogateInventory(this, true, false, true, error);
	}

	return;
}

void Client::Handle_OP_OpenContainer(const EQApplicationPacket *app)
{
	// Does not exist in Ti client
	// SoF, SoD and UF clients send a 4-byte packet indicating the 'parent' slot
	// SoF, SoD and UF slots are defined by a uint32 value and currently untranslated
	// RoF client sends a 12-byte packet based on the RoF::Structs::ItemSlotStruct

	// RoF structure types are defined as signed uint16 and currently untranslated
	// RoF::struct.SlotType = {0 - Equipment, 1 - Bank, 2 - Shared Bank} // not tested beyond listed types
	// RoF::struct.Unknown2 = 0
	// RoF::struct.MainSlot = { <parent slot range designated by slottype..zero-based> }
	// RoF::struct.SubSlot = -1 (non-child)
	// RoF::struct.AugSlot = -1 (non-child)
	// RoF::struct.Unknown1 = 141 (unsure why, but always appears to be this value..combine containers not tested)

	// SideNote: Watching the slot translations, Unknown1 is showing '141' as well on certain item swaps.
	// Manually looting a corpse results in a from '34' to '68' value for equipment items, '0' to '0' for inventory.
}

void Client::Handle_OP_OpenGuildTributeMaster(const EQApplicationPacket *app)
{
	Log(Logs::Detail, Logs::Tribute, "Received OP_OpenGuildTributeMaster of length %d", app->size);

	if (app->size != sizeof(StartTribute_Struct))
		printf("Error in OP_OpenGuildTributeMaster. Expected size of: %zu, but got: %i\n", sizeof(StartTribute_Struct), app->size);
	else {
		//Opens the guild tribute master window
		StartTribute_Struct* st = (StartTribute_Struct*)app->pBuffer;
		Mob* tribmast = entity_list.GetMob(st->tribute_master_id);
		if (tribmast && tribmast->IsNPC() && tribmast->GetClass() == GUILD_TRIBUTE_MASTER
			&& DistanceSquared(m_Position, tribmast->GetPosition()) <= USE_NPC_RANGE2) {
			st->response = 1;
			QueuePacket(app);
			tribute_master_id = st->tribute_master_id;
			DoTributeUpdate();
		}
		else {
			st->response = 0;
			QueuePacket(app);
		}
	}
	return;
}

void Client::Handle_OP_OpenInventory(const EQApplicationPacket *app)
{
	// Does not exist in Ti, UF or RoF clients
	// SoF and SoD both send a 4-byte packet with a uint32 value of '8'
}

void Client::Handle_OP_OpenTributeMaster(const EQApplicationPacket *app)
{
	Log(Logs::Detail, Logs::Tribute, "Received OP_OpenTributeMaster of length %d", app->size);

	if (app->size != sizeof(StartTribute_Struct))
		printf("Error in OP_OpenTributeMaster. Expected size of: %zu, but got: %i\n", sizeof(StartTribute_Struct), app->size);
	else {
		//Opens the tribute master window
		StartTribute_Struct* st = (StartTribute_Struct*)app->pBuffer;
		Mob* tribmast = entity_list.GetMob(st->tribute_master_id);
		if (tribmast && tribmast->IsNPC() && tribmast->GetClass() == TRIBUTE_MASTER
			&& DistanceSquared(m_Position, tribmast->GetPosition()) <= USE_NPC_RANGE2) {
			st->response = 1;
			QueuePacket(app);
			tribute_master_id = st->tribute_master_id;
			DoTributeUpdate();
		}
		else {
			st->response = 0;
			QueuePacket(app);
		}
	}
	return;
}

void Client::Handle_OP_PDeletePetition(const EQApplicationPacket *app)
{
	if (app->size < 2) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_PDeletePetition, size=%i, expected %i", app->size, 2);
		return;
	}
	if (petition_list.DeletePetitionByCharName((char*)app->pBuffer))
		Message_StringID(0, PETITION_DELETED);
	else
		Message_StringID(0, PETITION_NO_DELETE);
	return;
}

void Client::Handle_OP_PetCommands(const EQApplicationPacket *app)
{
	if (app->size != sizeof(PetCommand_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_PetCommands, size=%i, expected %i", app->size, sizeof(PetCommand_Struct));
		return;
	}
	char val1[20] = { 0 };
	PetCommand_Struct* pet = (PetCommand_Struct*)app->pBuffer;
	Mob* mypet = this->GetPet();
	Mob *target = entity_list.GetMob(pet->target);

	if (!mypet || pet->command == PET_LEADER)
	{
		if (pet->command == PET_LEADER)
		{
			if (mypet && (!GetTarget() || GetTarget() == mypet))
			{
				mypet->Say_StringID(PET_LEADERIS, GetName());
			}
			else if ((mypet = GetTarget()))
			{
				Mob *Owner = mypet->GetOwner();
				if (Owner)
					mypet->Say_StringID(PET_LEADERIS, Owner->GetCleanName());
				else if (mypet->IsNPC())
					mypet->Say_StringID(I_FOLLOW_NOONE);
			}
		}

		return;
	}

	if (mypet->GetPetType() == petTargetLock && (pet->command != PET_HEALTHREPORT && pet->command != PET_GETLOST))
		return;

	// just let the command "/pet get lost" work for familiars
	if (mypet->GetPetType() == petFamiliar && pet->command != PET_GETLOST)
		return;

	uint32 PetCommand = pet->command;

	// Handle Sit/Stand toggle in UF and later.
	/*
	if (GetClientVersion() >= EQClientUnderfoot)
	{
	if (PetCommand == PET_SITDOWN)
	if (mypet->GetPetOrder() == SPO_Sit)
	PetCommand = PET_STANDUP;
	}
	*/

	switch (PetCommand)
	{
	case PET_ATTACK: {
		if (!target)
			break;
		if (target->IsMezzed()) {
			Message_StringID(10, CANNOT_WAKE, mypet->GetCleanName(), target->GetCleanName());
			break;
		}
		if (mypet->IsFeared())
			break; //prevent pet from attacking stuff while feared

		if (!mypet->IsAttackAllowed(target)) {
			mypet->SayTo_StringID(this, NOT_LEGAL_TARGET);
			break;
		}

		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			if (target != this && DistanceSquaredNoZ(mypet->GetPosition(), target->GetPosition()) <= (RuleR(Pets, AttackCommandRange)*RuleR(Pets, AttackCommandRange))) {
				if (mypet->IsPetStop()) {
					mypet->SetPetStop(false);
					SetPetCommandState(PET_BUTTON_STOP, 0);
				}
				if (mypet->IsPetRegroup()) {
					mypet->SetPetRegroup(false);
					SetPetCommandState(PET_BUTTON_REGROUP, 0);
				}
				zone->AddAggroMob();
				// classic acts like qattack
				int hate = 1;
				if (mypet->IsEngaged()) {
					auto top = mypet->GetHateMost();
					if (top && top != target)
						hate += mypet->GetHateAmount(top) - mypet->GetHateAmount(target) + 100; // should be enough to cause target change
				}
				mypet->AddToHateList(target, hate, 0, true, false, false, SPELL_UNKNOWN, true);
				Message_StringID(MT_PetResponse, PET_ATTACKING, mypet->GetCleanName(), target->GetCleanName());
				SetTarget(target);
			}
		}
		break;
	}
	case PET_QATTACK: {
		if (mypet->IsFeared())
			break; //prevent pet from attacking stuff while feared

		if (!GetTarget())
			break;
		if (GetTarget()->IsMezzed()) {
			Message_StringID(10, CANNOT_WAKE, mypet->GetCleanName(), GetTarget()->GetCleanName());
			break;
		}

		if (!mypet->IsAttackAllowed(GetTarget())) {
			mypet->SayTo_StringID(this, NOT_LEGAL_TARGET);
			break;
		}

		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			if (GetTarget() != this && DistanceSquaredNoZ(mypet->GetPosition(), GetTarget()->GetPosition()) <= (RuleR(Pets, AttackCommandRange)*RuleR(Pets, AttackCommandRange))) {
				if (mypet->IsPetStop()) {
					mypet->SetPetStop(false);
					SetPetCommandState(PET_BUTTON_STOP, 0);
				}
				if (mypet->IsPetRegroup()) {
					mypet->SetPetRegroup(false);
					SetPetCommandState(PET_BUTTON_REGROUP, 0);
				}
				zone->AddAggroMob();
				mypet->AddToHateList(GetTarget(), 1, 0, true, false, false, SPELL_UNKNOWN, true);
				Message_StringID(MT_PetResponse, PET_ATTACKING, mypet->GetCleanName(), GetTarget()->GetCleanName());
			}
		}
		break;
	}
	case PET_BACKOFF: {
		if (mypet->IsFeared()) break; //keeps pet running while feared

		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			mypet->SayTo_StringID(this, MT_PetResponse, PET_CALMING);
			mypet->WipeHateList();
			mypet->SetTarget(nullptr);
			if (mypet->IsPetStop()) {
				mypet->SetPetStop(false);
				SetPetCommandState(PET_BUTTON_STOP, 0);
			}
		}
		break;
	}
	case PET_HEALTHREPORT: {
		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			Message_StringID(MT_PetResponse, PET_REPORT_HP, ConvertArrayF(mypet->GetHPRatio(), val1));
			mypet->ShowBuffList(this);
		}
		break;
	}
	case PET_GETLOST: {
		if (mypet->Charmed())
			break;
		if (mypet->GetPetType() == petCharmed || !mypet->IsNPC()) {
			// eqlive ignores this command
			// we could just remove the charm
			// and continue
			mypet->BuffFadeByEffect(SE_Charm);
			break;
		}
		else {
			SetPet(nullptr);
		}

		mypet->SayTo_StringID(this, MT_PetResponse, PET_GETLOST_STRING);
		mypet->CastToNPC()->Depop();

		//Oddly, the client (Titanium) will still allow "/pet get lost" command despite me adding the code below. If someone can figure that out, you can uncomment this code and use it.
		/*
		if((mypet->GetPetType() == petAnimation && GetAA(aaAnimationEmpathy) >= 2) || mypet->GetPetType() != petAnimation) {
		mypet->Say_StringID(PET_GETLOST_STRING);
		mypet->CastToNPC()->Depop();
		}
		*/

		break;
	}
	case PET_GUARDHERE: {
		if (mypet->IsFeared()) break; //could be exploited like PET_BACKOFF

		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			if (mypet->IsNPC()) {
				mypet->SayTo_StringID(this, MT_PetResponse, PET_GUARDINGLIFE);
				mypet->SetPetOrder(SPO_Guard);
				mypet->CastToNPC()->SaveGuardSpot();
				if (!mypet->GetTarget()) // want them to not twitch if they're chasing something down
					mypet->SetCurrentSpeed(0);
				if (mypet->IsPetStop()) {
					mypet->SetPetStop(false);
					SetPetCommandState(PET_BUTTON_STOP, 0);
				}
			}
		}
		break;
	}
	case PET_FOLLOWME: {
		if (mypet->IsFeared()) break; //could be exploited like PET_BACKOFF

		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			mypet->SayTo_StringID(this, MT_PetResponse, PET_FOLLOWING);
			mypet->SetPetOrder(SPO_Follow);
			mypet->SendAppearancePacket(AT_Anim, ANIM_STAND);
			if (mypet->IsPetStop()) {
				mypet->SetPetStop(false);
				SetPetCommandState(PET_BUTTON_STOP, 0);
			}
		}
		break;
	}
	case PET_TAUNT: {
		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			if (mypet->CastToNPC()->IsTaunting())
			{
				Message_StringID(MT_PetResponse, PET_NO_TAUNT);
				mypet->CastToNPC()->SetTaunting(false);
			}
			else
			{
				Message_StringID(MT_PetResponse, PET_DO_TAUNT);
				mypet->CastToNPC()->SetTaunting(true);
			}
		}
		break;
	}
	case PET_TAUNT_ON: {
		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			Message_StringID(MT_PetResponse, PET_DO_TAUNT);
			mypet->CastToNPC()->SetTaunting(true);
		}
		break;
	}
	case PET_TAUNT_OFF: {
		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			Message_StringID(MT_PetResponse, PET_NO_TAUNT);
			mypet->CastToNPC()->SetTaunting(false);
		}
		break;
	}
	case PET_GUARDME: {
		if (mypet->IsFeared()) break; //could be exploited like PET_BACKOFF

		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			mypet->SayTo_StringID(this, MT_PetResponse, PET_GUARDME_STRING);
			mypet->SetPetOrder(SPO_Follow);
			mypet->SendAppearancePacket(AT_Anim, ANIM_STAND);
			if (mypet->IsPetStop()) {
				mypet->SetPetStop(false);
				SetPetCommandState(PET_BUTTON_STOP, 0);
			}
		}
		break;
	}
	case PET_SIT: {
		if (mypet->IsFeared()) break; //could be exploited like PET_BACKOFF

		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			if (mypet->GetPetOrder() == SPO_Sit)
			{
				mypet->SayTo_StringID(this, MT_PetResponse, PET_SIT_STRING);
				mypet->SetPetOrder(SPO_Follow);
				mypet->SendAppearancePacket(AT_Anim, ANIM_STAND);
			}
			else
			{
				mypet->SayTo_StringID(this, MT_PetResponse, PET_SIT_STRING);
				mypet->SetPetOrder(SPO_Sit);
				mypet->SetRunAnimSpeed(0);
				if (!mypet->UseBardSpellLogic())	//maybe we can have a bard pet
					mypet->InterruptSpell(); //No cast 4 u. //i guess the pet should start casting
				mypet->SendAppearancePacket(AT_Anim, ANIM_SIT);
			}
		}
		break;
	}
	case PET_STANDUP: {
		if (mypet->IsFeared()) break; //could be exploited like PET_BACKOFF

		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			mypet->SayTo_StringID(this, MT_PetResponse, PET_SIT_STRING);
			mypet->SetPetOrder(SPO_Follow);
			mypet->SendAppearancePacket(AT_Anim, ANIM_STAND);
		}
		break;
	}
	case PET_SITDOWN: {
		if (mypet->IsFeared()) break; //could be exploited like PET_BACKOFF

		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			mypet->SayTo_StringID(this, MT_PetResponse, PET_SIT_STRING);
			mypet->SetPetOrder(SPO_Sit);
			mypet->SetRunAnimSpeed(0);
			if (!mypet->UseBardSpellLogic())	//maybe we can have a bard pet
				mypet->InterruptSpell(); //No cast 4 u. //i guess the pet should start casting
			mypet->SendAppearancePacket(AT_Anim, ANIM_SIT);
		}
		break;
	}
	case PET_HOLD: {
		if (aabonuses.PetCommands[PetCommand] && mypet->IsNPC()) {
			if (mypet->IsHeld())
			{
				if (m_ClientVersionBit & EQEmu::versions::bit_SoDAndLater)
					Message_StringID(MT_PetResponse, PET_HOLD_SET_OFF);
				mypet->SetHeld(false);
			}
			else
			{
				if (m_ClientVersionBit & EQEmu::versions::bit_SoDAndLater)
					Message_StringID(MT_PetResponse, PET_HOLD_SET_ON);

				if (m_ClientVersionBit & EQEmu::versions::bit_UFAndLater)
					mypet->SayTo_StringID(this, MT_PetResponse, PET_NOW_HOLDING);
				else
					mypet->SayTo_StringID(this, MT_PetResponse, PET_ON_HOLD);

				mypet->SetHeld(true);
			}
			mypet->SetGHeld(false);
			SetPetCommandState(PET_BUTTON_GHOLD, 0);
		}
		break;
	}
	case PET_HOLD_ON: {
		if (aabonuses.PetCommands[PetCommand] && mypet->IsNPC() && !mypet->IsHeld()) {
			if (m_ClientVersionBit & EQEmu::versions::bit_SoDAndLater)
				Message_StringID(MT_PetResponse, PET_HOLD_SET_ON);

			if (m_ClientVersionBit & EQEmu::versions::bit_UFAndLater)
				mypet->SayTo_StringID(this, MT_PetResponse, PET_NOW_HOLDING);
			else
				mypet->SayTo_StringID(this, MT_PetResponse, PET_ON_HOLD);
			mypet->SetHeld(true);
			mypet->SetGHeld(false);
			SetPetCommandState(PET_BUTTON_GHOLD, 0);
		}
		break;
	}
	case PET_HOLD_OFF: {
		if (aabonuses.PetCommands[PetCommand] && mypet->IsNPC() && mypet->IsHeld()) {
			if (m_ClientVersionBit & EQEmu::versions::bit_SoDAndLater)
				Message_StringID(MT_PetResponse, PET_HOLD_SET_OFF);
			mypet->SetHeld(false);
		}
		break;
	}
	case PET_GHOLD: {
		if (aabonuses.PetCommands[PetCommand] && mypet->IsNPC()) {
			if (mypet->IsGHeld())
			{
				if (m_ClientVersionBit & EQEmu::versions::bit_UFAndLater)
					Message_StringID(MT_PetResponse, PET_OFF_GHOLD);
				mypet->SetGHeld(false);
			}
			else
			{
				if (m_ClientVersionBit & EQEmu::versions::bit_UFAndLater) {
					Message_StringID(MT_PetResponse, PET_ON_GHOLD);
					mypet->SayTo_StringID(this, MT_PetResponse, PET_GHOLD_ON_MSG);
				} else {
					mypet->SayTo_StringID(this, MT_PetResponse, PET_ON_HOLD);
				}
				mypet->SetGHeld(true);
			}
			mypet->SetHeld(false);
			SetPetCommandState(PET_BUTTON_HOLD, 0);
		}
		break;
	}
	case PET_GHOLD_ON: {
		if (aabonuses.PetCommands[PetCommand] && mypet->IsNPC()) {
			if (m_ClientVersionBit & EQEmu::versions::bit_UFAndLater) {
				Message_StringID(MT_PetResponse, PET_ON_GHOLD);
				mypet->SayTo_StringID(this, MT_PetResponse, PET_GHOLD_ON_MSG);
			} else {
				mypet->SayTo_StringID(this, MT_PetResponse, PET_ON_HOLD);
			}
			mypet->SetGHeld(true);
			mypet->SetHeld(false);
			SetPetCommandState(PET_BUTTON_HOLD, 0);
		}
		break;
	}
	case PET_GHOLD_OFF: {
		if (aabonuses.PetCommands[PetCommand] && mypet->IsNPC() && mypet->IsGHeld()) {
			if (m_ClientVersionBit & EQEmu::versions::bit_UFAndLater)
				Message_StringID(MT_PetResponse, PET_OFF_GHOLD);
			mypet->SetGHeld(false);
		}
		break;
	}
	case PET_SPELLHOLD: {
		if (aabonuses.PetCommands[PetCommand] && mypet->IsNPC()) {
			if (mypet->IsFeared())
				break;
			if (mypet->IsNoCast()) {
				Message_StringID(MT_PetResponse, PET_CASTING);
				if (m_ClientVersionBit & EQEmu::versions::bit_SoDAndLater)
					Message_StringID(MT_PetResponse, PET_SPELLHOLD_SET_OFF);
				mypet->SetNoCast(false);
			}
			else {
				Message_StringID(MT_PetResponse, PET_NOT_CASTING);
				if (m_ClientVersionBit & EQEmu::versions::bit_SoDAndLater)
					Message_StringID(MT_PetResponse, PET_SPELLHOLD_SET_ON);
				mypet->SetNoCast(true);
			}
		}
		break;
	}
	case PET_SPELLHOLD_ON: {
		if (aabonuses.PetCommands[PetCommand] && mypet->IsNPC()) {
			if (mypet->IsFeared())
				break;
			if (!mypet->IsNoCast()) {
				Message_StringID(MT_PetResponse, PET_NOT_CASTING);
				if (m_ClientVersionBit & EQEmu::versions::bit_SoDAndLater)
					Message_StringID(MT_PetResponse, PET_SPELLHOLD_SET_ON);
				mypet->SetNoCast(true);
			}
		}
		break;
	}
	case PET_SPELLHOLD_OFF: {
		if (aabonuses.PetCommands[PetCommand] && mypet->IsNPC()) {
			if (mypet->IsFeared())
				break;
			if (mypet->IsNoCast()) {
				Message_StringID(MT_PetResponse, PET_CASTING);
				if (m_ClientVersionBit & EQEmu::versions::bit_SoDAndLater)
					Message_StringID(MT_PetResponse, PET_SPELLHOLD_SET_OFF);
				mypet->SetNoCast(false);
			}
		}
		break;
	}
	case PET_FOCUS: {
		if (aabonuses.PetCommands[PetCommand] && mypet->IsNPC()) {
			if (mypet->IsFeared())
				break;
			if (mypet->IsFocused()) {
				Message_StringID(MT_PetResponse, PET_NOT_FOCUSING);
				if (m_ClientVersionBit & EQEmu::versions::bit_SoDAndLater)
					Message_StringID(MT_PetResponse, PET_FOCUS_SET_OFF);
				mypet->SetFocused(false);
			}
			else {
				Message_StringID(MT_PetResponse, PET_NOW_FOCUSING);
				if (m_ClientVersionBit & EQEmu::versions::bit_SoDAndLater)
					Message_StringID(MT_PetResponse, PET_FOCUS_SET_ON);
				mypet->SetFocused(true);
			}
		}
		break;
	}
	case PET_FOCUS_ON: {
		if (aabonuses.PetCommands[PetCommand] && mypet->IsNPC()) {
			if (mypet->IsFeared())
				break;
			if (!mypet->IsFocused()) {
				Message_StringID(MT_PetResponse, PET_NOW_FOCUSING);
				if (m_ClientVersionBit & EQEmu::versions::bit_SoDAndLater)
					Message_StringID(MT_PetResponse, PET_FOCUS_SET_ON);
				mypet->SetFocused(true);
			}
		}
		break;
	}
	case PET_FOCUS_OFF: {
		if (aabonuses.PetCommands[PetCommand] && mypet->IsNPC()) {
			if (mypet->IsFeared())
				break;
			if (mypet->IsFocused()) {
				Message_StringID(MT_PetResponse, PET_NOT_FOCUSING);
				if (m_ClientVersionBit & EQEmu::versions::bit_SoDAndLater)
					Message_StringID(MT_PetResponse, PET_FOCUS_SET_OFF);
				mypet->SetFocused(false);
			}
		}
		break;
	}
	case PET_STOP: {
		if (mypet->IsFeared()) break; //could be exploited like PET_BACKOFF

		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			if (mypet->IsPetStop()) {
				mypet->SetPetStop(false);
			} else {
				mypet->SetPetStop(true);
				mypet->SetCurrentSpeed(0);
				mypet->SetTarget(nullptr);
				if (mypet->IsPetRegroup()) {
					mypet->SetPetRegroup(false);
					SetPetCommandState(PET_BUTTON_REGROUP, 0);
				}
			}
			mypet->SayTo_StringID(this, MT_PetResponse, PET_GETLOST_STRING);
		}
		break;
	}
	case PET_STOP_ON: {
		if (mypet->IsFeared()) break; //could be exploited like PET_BACKOFF

		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			mypet->SetPetStop(true);
			mypet->SetCurrentSpeed(0);
			mypet->SetTarget(nullptr);
			mypet->SayTo_StringID(this, MT_PetResponse, PET_GETLOST_STRING);
			if (mypet->IsPetRegroup()) {
				mypet->SetPetRegroup(false);
				SetPetCommandState(PET_BUTTON_REGROUP, 0);
			}
		}
		break;
	}
	case PET_STOP_OFF: {
		if (mypet->IsFeared()) break; //could be exploited like PET_BACKOFF

		if ((mypet->GetPetType() == petAnimation && aabonuses.PetCommands[PetCommand]) || mypet->GetPetType() != petAnimation) {
			mypet->SetPetStop(false);
			mypet->SayTo_StringID(this, MT_PetResponse, PET_GETLOST_STRING);
		}
		break;
	}
	case PET_REGROUP: {
		if (mypet->IsFeared()) break; //could be exploited like PET_BACKOFF

		if (aabonuses.PetCommands[PetCommand]) {
			if (mypet->IsPetRegroup()) {
				mypet->SetPetRegroup(false);
				mypet->SayTo_StringID(this, MT_PetResponse, PET_OFF_REGROUPING);
			} else {
				mypet->SetPetRegroup(true);
				mypet->SetTarget(nullptr);
				mypet->SayTo_StringID(this, MT_PetResponse, PET_ON_REGROUPING);
				if (mypet->IsPetStop()) {
					mypet->SetPetStop(false);
					SetPetCommandState(PET_BUTTON_STOP, 0);
				}
			}
		}
		break;
	}
	case PET_REGROUP_ON: {
		if (mypet->IsFeared()) break; //could be exploited like PET_BACKOFF

		if (aabonuses.PetCommands[PetCommand]) {
			mypet->SetPetRegroup(true);
			mypet->SetTarget(nullptr);
			mypet->SayTo_StringID(this, MT_PetResponse, PET_ON_REGROUPING);
			if (mypet->IsPetStop()) {
				mypet->SetPetStop(false);
				SetPetCommandState(PET_BUTTON_STOP, 0);
			}
		}
		break;
	}
	case PET_REGROUP_OFF: {
		if (mypet->IsFeared()) break; //could be exploited like PET_BACKOFF

		if (aabonuses.PetCommands[PetCommand]) {
			mypet->SetPetRegroup(false);
			mypet->SayTo_StringID(this, MT_PetResponse, PET_OFF_REGROUPING);
		}
		break;
	}
	default:
		printf("Client attempted to use a unknown pet command:\n");
		break;
	}
}

void Client::Handle_OP_Petition(const EQApplicationPacket *app)
{
	if (app->size <= 1)
		return;
	if (!worldserver.Connected())
		Message(0, "Error: World server disconnected");
	/*else if(petition_list.FindPetitionByAccountName(this->AccountName()))
	{
	Message(0,"You already have a petition in queue, you cannot petition again until this one has been responded to or you have deleted the petition.");
	return;
	}*/
	else
	{
		if (petition_list.FindPetitionByAccountName(AccountName()))
		{
			Message(0, "You already have a petition in the queue, you must wait for it to be answered or use /deletepetition to delete it.");
			return;
		}
		auto pet = new Petition(CharacterID());
		pet->SetAName(this->AccountName());
		pet->SetClass(this->GetClass());
		pet->SetLevel(this->GetLevel());
		pet->SetCName(this->GetName());
		pet->SetRace(this->GetRace());
		pet->SetLastGM("");
		pet->SetCName(this->GetName());
		pet->SetPetitionText((char*)app->pBuffer);
		pet->SetZone(zone->GetZoneID());
		pet->SetUrgency(0);
		petition_list.AddPetition(pet);
		database.InsertPetitionToDB(pet);
		petition_list.UpdateGMQueue();
		petition_list.UpdateZoneListQueue();
		worldserver.SendEmoteMessage(0, 0, 80, 15, "%s has made a petition. #%i", GetName(), pet->GetID());
	}
	return;
}

void Client::Handle_OP_PetitionBug(const EQApplicationPacket *app)
{
	if (app->size != sizeof(PetitionBug_Struct))
		printf("Wrong size of BugStruct! Expected: %zu, Got: %i\n", sizeof(PetitionBug_Struct), app->size);
	else {
		Message(0, "Petition Bugs are not supported, please use /bug.");
	}
	return;
}

void Client::Handle_OP_PetitionCheckIn(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Petition_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_PetitionCheckIn, size=%i, expected %i", app->size, sizeof(Petition_Struct));
		return;
	}
	Petition_Struct* inpet = (Petition_Struct*)app->pBuffer;

	Petition* pet = petition_list.GetPetitionByID(inpet->petnumber);
	//if (inpet->urgency != pet->GetUrgency())
	pet->SetUrgency(inpet->urgency);
	pet->SetLastGM(this->GetName());
	pet->SetGMText(inpet->gmtext);

	pet->SetCheckedOut(false);
	petition_list.UpdatePetition(pet);
	petition_list.UpdateGMQueue();
	petition_list.UpdateZoneListQueue();
	return;
}

void Client::Handle_OP_PetitionCheckout(const EQApplicationPacket *app)
{
	if (app->size != sizeof(uint32)) {
		std::cout << "Wrong size: OP_PetitionCheckout, size=" << app->size << ", expected " << sizeof(uint32) << std::endl;
		return;
	}
	if (!worldserver.Connected())
		Message(0, "Error: World server disconnected");
	else {
		uint32 getpetnum = *((uint32*)app->pBuffer);
		Petition* getpet = petition_list.GetPetitionByID(getpetnum);
		if (getpet != 0) {
			getpet->AddCheckout();
			getpet->SetCheckedOut(true);
			getpet->SendPetitionToPlayer(this->CastToClient());
			petition_list.UpdatePetition(getpet);
			petition_list.UpdateGMQueue();
			petition_list.UpdateZoneListQueue();
		}
	}
	return;
}

void Client::Handle_OP_PetitionDelete(const EQApplicationPacket *app)
{
	if (app->size != sizeof(PetitionUpdate_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_PetitionDelete, size=%i, expected %i", app->size, sizeof(PetitionUpdate_Struct));
		return;
	}
	auto outapp = new EQApplicationPacket(OP_PetitionUpdate, sizeof(PetitionUpdate_Struct));
	PetitionUpdate_Struct* pet = (PetitionUpdate_Struct*)outapp->pBuffer;
	pet->petnumber = *((int*)app->pBuffer);
	pet->color = 0x00;
	pet->status = 0xFFFFFFFF;
	pet->senttime = 0;
	strcpy(pet->accountid, "");
	strcpy(pet->gmsenttoo, "");
	pet->quetotal = petition_list.GetTotalPetitions();
	strcpy(pet->charname, "");
	FastQueuePacket(&outapp);

	if (petition_list.DeletePetition(pet->petnumber) == -1)
		std::cout << "Something is borked with: " << pet->petnumber << std::endl;
	petition_list.ClearPetitions();
	petition_list.UpdateGMQueue();
	petition_list.ReadDatabase();
	petition_list.UpdateZoneListQueue();
	return;
}

void Client::Handle_OP_PetitionQue(const EQApplicationPacket *app)
{
#ifdef _EQDEBUG
	printf("%s looking at petitions..\n", this->GetName());
#endif
	return;
}

void Client::Handle_OP_PetitionRefresh(const EQApplicationPacket *app)
{
	// This is When Client Asks for Petition Again and Again...
	// break is here because it floods the zones and causes lag if it
	// Were to actually do something:P We update on our own schedule now.
	return;
}

void Client::Handle_OP_PetitionResolve(const EQApplicationPacket *app)
{
	Handle_OP_PetitionDelete(app);
}

void Client::Handle_OP_PetitionUnCheckout(const EQApplicationPacket *app)
{
	if (app->size != sizeof(uint32)) {
		std::cout << "Wrong size: OP_PetitionUnCheckout, size=" << app->size << ", expected " << sizeof(uint32) << std::endl;
		return;
	}
	if (!worldserver.Connected())
		Message(0, "Error: World server disconnected");
	else {
		uint32 getpetnum = *((uint32*)app->pBuffer);
		Petition* getpet = petition_list.GetPetitionByID(getpetnum);
		if (getpet != 0) {
			getpet->SetCheckedOut(false);
			petition_list.UpdatePetition(getpet);
			petition_list.UpdateGMQueue();
			petition_list.UpdateZoneListQueue();
		}
	}
	return;
}

void Client::Handle_OP_PlayerStateAdd(const EQApplicationPacket *app)
{
	if (app->size != sizeof(PlayerState_Struct)) {
		std::cout << "Wrong size: OP_PlayerStateAdd, size=" << app->size << ", expected " << sizeof(PlayerState_Struct) << std::endl;
		return;
	}

	PlayerState_Struct *ps = (PlayerState_Struct *)app->pBuffer;
	AddPlayerState(ps->state);

	entity_list.QueueClients(this, app, true);
}

void Client::Handle_OP_PlayerStateRemove(const EQApplicationPacket *app)
{
	if (app->size != sizeof(PlayerState_Struct)) {
		std::cout << "Wrong size: OP_PlayerStateRemove, size=" << app->size << ", expected " << sizeof(PlayerState_Struct) << std::endl;
		return;
	}
	PlayerState_Struct *ps = (PlayerState_Struct *)app->pBuffer;
	RemovePlayerState(ps->state);

	entity_list.QueueClients(this, app, true);
}

void Client::Handle_OP_PickPocket(const EQApplicationPacket *app)
{
	if (app->size != sizeof(PickPocket_Struct))
	{
		Log(Logs::General, Logs::Error, "Size mismatch for Pick Pocket packet");
		DumpPacket(app);
	}

	if (!HasSkill(EQEmu::skills::SkillPickPockets))
	{
		return;
	}

	if (!p_timers.Expired(&database, pTimerBeggingPickPocket, false))
	{
		Message(13, "Ability recovery time not yet met.");
		database.SetMQDetectionFlag(this->AccountName(), this->GetName(), "OP_PickPocket was sent again too quickly.", zone->GetShortName());
		return;
	}
	PickPocket_Struct* pick_in = (PickPocket_Struct*)app->pBuffer;

	Mob* victim = entity_list.GetMob(pick_in->to);
	if (!victim)
		return;

	p_timers.Start(pTimerBeggingPickPocket, 8);
	if (victim == this) {
		Message(0, "You catch yourself red-handed.");
		auto outapp = new EQApplicationPacket(OP_PickPocket, sizeof(sPickPocket_Struct));
		sPickPocket_Struct* pick_out = (sPickPocket_Struct*)outapp->pBuffer;
		pick_out->coin = 0;
		pick_out->from = victim->GetID();
		pick_out->to = GetID();
		pick_out->myskill = GetSkill(EQEmu::skills::SkillPickPockets);
		pick_out->type = 0;
		//if we do not send this packet the client will lock up and require the player to relog.
		QueuePacket(outapp);
		safe_delete(outapp);
	}
	else if (victim->GetOwnerID()) {
		Message(0, "You cannot steal from pets!");
		auto outapp = new EQApplicationPacket(OP_PickPocket, sizeof(sPickPocket_Struct));
		sPickPocket_Struct* pick_out = (sPickPocket_Struct*)outapp->pBuffer;
		pick_out->coin = 0;
		pick_out->from = victim->GetID();
		pick_out->to = GetID();
		pick_out->myskill = GetSkill(EQEmu::skills::SkillPickPockets);
		pick_out->type = 0;
		//if we do not send this packet the client will lock up and require the player to relog.
		QueuePacket(outapp);
		safe_delete(outapp);
	}
	else if (victim->IsNPC()) {
		victim->CastToNPC()->PickPocket(this);
	}
	else {
		Message(0, "Stealing from clients not yet supported.");
		auto outapp = new EQApplicationPacket(OP_PickPocket, sizeof(sPickPocket_Struct));
		sPickPocket_Struct* pick_out = (sPickPocket_Struct*)outapp->pBuffer;
		pick_out->coin = 0;
		pick_out->from = victim->GetID();
		pick_out->to = GetID();
		pick_out->myskill = GetSkill(EQEmu::skills::SkillPickPockets);
		pick_out->type = 0;
		//if we do not send this packet the client will lock up and require the player to relog.
		QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Client::Handle_OP_PopupResponse(const EQApplicationPacket *app)
{

	if (app->size != sizeof(PopupResponse_Struct)) {
		Log(Logs::General, Logs::None, "Size mismatch in OP_PopupResponse expected %i got %i",
			sizeof(PopupResponse_Struct), app->size);
		DumpPacket(app);
		return;
	}
	PopupResponse_Struct *prs = (PopupResponse_Struct*)app->pBuffer;

	// Handle any EQEmu defined popup Ids first
	switch (prs->popupid)
	{
	case POPUPID_UPDATE_SHOWSTATSWINDOW:
		if (GetTarget() && GetTarget()->IsClient())
			GetTarget()->CastToClient()->SendStatsWindow(this, true);
		else
			SendStatsWindow(this, true);
		return;

	default:
		break;
	}

	char buf[16];
	sprintf(buf, "%d\0", prs->popupid);

	parse->EventPlayer(EVENT_POPUP_RESPONSE, this, buf, 0);

	Mob* Target = GetTarget();
	if (Target && Target->IsNPC()) {
		parse->EventNPC(EVENT_POPUP_RESPONSE, Target->CastToNPC(), this, buf, 0);
	}
}

void Client::Handle_OP_PotionBelt(const EQApplicationPacket *app)
{
	if (app->size != sizeof(MovePotionToBelt_Struct)) {
		Log(Logs::General, Logs::None, "Size mismatch in OP_PotionBelt expected %i got %i",
			sizeof(MovePotionToBelt_Struct), app->size);
		DumpPacket(app);
		return;
	}

	MovePotionToBelt_Struct *mptbs = (MovePotionToBelt_Struct*)app->pBuffer;
	if (!EQEmu::ValueWithin(mptbs->SlotNumber, 0U, 3U)) {
		Log(Logs::General, Logs::None, "Client::Handle_OP_PotionBelt mptbs->SlotNumber out of range.");
		return;
	}

	if (mptbs->Action == 0) {
		const EQEmu::ItemData *BaseItem = database.GetItem(mptbs->ItemID);
		if (BaseItem) {
			m_pp.potionbelt.Items[mptbs->SlotNumber].ID = BaseItem->ID;
			m_pp.potionbelt.Items[mptbs->SlotNumber].Icon = BaseItem->Icon;
			strn0cpy(m_pp.potionbelt.Items[mptbs->SlotNumber].Name, BaseItem->Name, sizeof(BaseItem->Name));
			database.SaveCharacterPotionBelt(this->CharacterID(), mptbs->SlotNumber, m_pp.potionbelt.Items[mptbs->SlotNumber].ID, m_pp.potionbelt.Items[mptbs->SlotNumber].Icon);
		}
	}
	else {
		m_pp.potionbelt.Items[mptbs->SlotNumber].ID = 0;
		m_pp.potionbelt.Items[mptbs->SlotNumber].Icon = 0;
		m_pp.potionbelt.Items[mptbs->SlotNumber].Name[0] = '\0';
	}
}

void Client::Handle_OP_PurchaseLeadershipAA(const EQApplicationPacket *app)
{
	if (app->size != sizeof(uint32)) {
		Log(Logs::General, Logs::None, "Size mismatch in OP_LeadershipExpToggle expected %i got %i", 1, app->size);
		DumpPacket(app);
		return;
	}
	uint32 aaid = *((uint32 *)app->pBuffer);

	if (aaid >= _maxLeaderAA)
		return;

	uint32 current_rank = m_pp.leader_abilities.ranks[aaid];
	if (current_rank >= MAX_LEADERSHIP_TIERS) {
		Message(13, "This ability can be trained no further.");
		return;
	}

	uint8 cost = LeadershipAACosts[aaid][current_rank];
	if (cost == 0) {
		Message(13, "This ability can be trained no further.");
		return;
	}

	//TODO: we need to enforce prerequisits

	if (aaid >= raidAAMarkNPC) {
		//it is a raid ability.
		if (cost > m_pp.raid_leadership_points) {
			Message(13, "You do not have enough points to purchase this ability.");
			return;
		}

		//sell them the ability.
		m_pp.raid_leadership_points -= cost;
		m_pp.leader_abilities.ranks[aaid]++;

		database.SaveCharacterLeadershipAA(this->CharacterID(), &m_pp);
	}
	else {
		//it is a group ability.
		if (cost > m_pp.group_leadership_points) {
			Message(13, "You do not have enough points to purchase this ability.");
			return;
		}

		//sell them the ability.
		m_pp.group_leadership_points -= cost;
		m_pp.leader_abilities.ranks[aaid]++;

		database.SaveCharacterLeadershipAA(this->CharacterID(), &m_pp);
	}

	//success, send them an update
	auto outapp = new EQApplicationPacket(OP_UpdateLeadershipAA, sizeof(UpdateLeadershipAA_Struct));
	UpdateLeadershipAA_Struct *u = (UpdateLeadershipAA_Struct *)outapp->pBuffer;
	u->ability_id = aaid;
	u->new_rank = m_pp.leader_abilities.ranks[aaid];
	if (aaid >= raidAAMarkNPC) // raid AA
		u->pointsleft = m_pp.raid_leadership_points;
	else // group AA
		u->pointsleft = m_pp.group_leadership_points;
	FastQueuePacket(&outapp);

	// Update all group members with the new AA the leader has purchased.
	if (IsRaidGrouped()) {
		Raid *r = GetRaid();
		if (!r)
			return;
		if (aaid >= raidAAMarkNPC) {
			r->UpdateRaidAAs();
			r->SendAllRaidLeadershipAA();
		}
		else {
			uint32 gid = r->GetGroup(this);
			r->UpdateGroupAAs(gid);
			r->GroupUpdate(gid, false);
		}
	}
	else if (IsGrouped()) {
		Group *g = GetGroup();
		if (!g)
			return;
		g->UpdateGroupAAs();
		g->SendLeadershipAAUpdate();
	}

}

void Client::Handle_OP_PVPLeaderBoardDetailsRequest(const EQApplicationPacket *app)
{
	// This opcode is sent by the client when the player right clicks a name on the PVP leaderboard and sends
	// further details about the selected player, e.g. Race/Class/AAs/Guild etc.
	//
	if (app->size != sizeof(PVPLeaderBoardDetailsRequest_Struct))
	{
		Log(Logs::General, Logs::None, "Size mismatch in OP_PVPLeaderBoardDetailsRequest expected %i got %i",
			sizeof(PVPLeaderBoardDetailsRequest_Struct), app->size);

		DumpPacket(app);

		return;
	}

	auto outapp = new EQApplicationPacket(OP_PVPLeaderBoardDetailsReply, sizeof(PVPLeaderBoardDetailsReply_Struct));
	PVPLeaderBoardDetailsReply_Struct *pvplbdrs = (PVPLeaderBoardDetailsReply_Struct *)outapp->pBuffer;

	// TODO: Record and send this data.

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::Handle_OP_PVPLeaderBoardRequest(const EQApplicationPacket *app)
{
	// This Opcode is sent by the client when the Leaderboard button on the PVP Stats window is pressed.
	//
	// It has a single uint32 payload which is the sort method:
	//
	// PVPSortByKills = 0, PVPSortByPoints = 1, PVPSortByInfamy = 2
	//
	if (app->size != sizeof(PVPLeaderBoardRequest_Struct))
	{
		Log(Logs::General, Logs::None, "Size mismatch in OP_PVPLeaderBoardRequest expected %i got %i",
			sizeof(PVPLeaderBoardRequest_Struct), app->size);

		DumpPacket(app);

		return;
	}
	/*PVPLeaderBoardRequest_Struct *pvplbrs = (PVPLeaderBoardRequest_Struct *)app->pBuffer;*/	//unused

	auto outapp = new EQApplicationPacket(OP_PVPLeaderBoardReply, sizeof(PVPLeaderBoard_Struct));
	/*PVPLeaderBoard_Struct *pvplb = (PVPLeaderBoard_Struct *)outapp->pBuffer;*/	//unused

																					// TODO: Record and send this data.

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::Handle_OP_RaidCommand(const EQApplicationPacket *app)
{
	if (app->size < sizeof(RaidGeneral_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_RaidCommand, size=%i, expected at least %i", app->size, sizeof(RaidGeneral_Struct));
		DumpPacket(app);
		return;
	}

	RaidGeneral_Struct *ri = (RaidGeneral_Struct*)app->pBuffer;
	switch (ri->action)
	{
	case RaidCommandInviteIntoExisting:
	case RaidCommandInvite: {
		Client *i = entity_list.GetClientByName(ri->player_name);
		if (!i)
			break;
		Group *g = i->GetGroup();
		// These two messages should be generated by the client I think, just do this for now
		if (i->HasRaid()) {
			Message(13, "%s is already in a raid.", i->GetName());
			break;
		}
		if (g && !g->IsLeader(i)) {
			Message(13, "You can only invite an ungrouped player or group leader to join your raid.");
			break;
		}
		//This sends an "invite" to the client in question.
		auto outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(RaidGeneral_Struct));
		RaidGeneral_Struct *rg = (RaidGeneral_Struct*)outapp->pBuffer;
		strn0cpy(rg->leader_name, ri->leader_name, 64);
		strn0cpy(rg->player_name, ri->player_name, 64);

		rg->parameter = 0;
		rg->action = 20;
		i->QueuePacket(outapp);
		safe_delete(outapp);
		break;
	}
	case RaidCommandAcceptInvite: {
		Client *i = entity_list.GetClientByName(ri->player_name);
		if (i) {
			if (IsRaidGrouped()) {
				i->Message_StringID(0, ALREADY_IN_RAID, GetName()); //group failed, must invite members not in raid...
				return;
			}
			Raid *r = entity_list.GetRaidByClient(i);
			if (r) {
				r->VerifyRaid();
				Group *g = GetGroup();
				if (g) {
					if (g->GroupCount() + r->RaidCount() > MAX_RAID_MEMBERS)
					{
						i->Message(13, "Invite failed, group invite would create a raid larger than the maximum number of members allowed.");
						return;
					}
				}
				else {
					if (1 + r->RaidCount() > MAX_RAID_MEMBERS)
					{
						i->Message(13, "Invite failed, member invite would create a raid larger than the maximum number of members allowed.");
						return;
					}
				}
				if (g) {//add us all
					uint32 freeGroup = r->GetFreeGroup();
					Client *addClient = nullptr;
					for (int x = 0; x < 6; x++)
					{
						if (g->members[x]) {
							Client *c = nullptr;
							if (g->members[x]->IsClient())
								c = g->members[x]->CastToClient();
							else
								continue;

							if (!addClient)
							{
								addClient = c;
								r->SetGroupLeader(addClient->GetName());
							}

							r->SendRaidCreate(c);
							r->SendMakeLeaderPacketTo(r->leadername, c);
							if (g->IsLeader(g->members[x]))
								r->AddMember(c, freeGroup, false, true);
							else
								r->AddMember(c, freeGroup);
							r->SendBulkRaid(c);
							if (r->IsLocked()) {
								r->SendRaidLockTo(c);
							}
						}
					}
					g->JoinRaidXTarget(r);
					g->DisbandGroup(true);
					r->GroupUpdate(freeGroup);
				}
				else {
					r->SendRaidCreate(this);
					r->SendMakeLeaderPacketTo(r->leadername, this);
					r->AddMember(this);
					r->SendBulkRaid(this);
					if (r->IsLocked()) {
						r->SendRaidLockTo(this);
					}
				}
			}
			else
			{
				Group *ig = i->GetGroup();
				Group *g = GetGroup();
				if (g) //if our target has a group
				{
					r = new Raid(i);
					entity_list.AddRaid(r);
					r->SetRaidDetails();

					uint32 groupFree = r->GetFreeGroup(); //get a free group
					if (ig) { //if we already have a group then cycle through adding us...
						Client *addClientig = nullptr;
						for (int x = 0; x < 6; x++)
						{
							if (ig->members[x]) {
								if (!addClientig) {
									if (ig->members[x]->IsClient()) {
										addClientig = ig->members[x]->CastToClient();
										r->SetGroupLeader(addClientig->GetName());
									}
								}
								if (ig->IsLeader(ig->members[x])) {
									Client *c = nullptr;
									if (ig->members[x]->IsClient())
										c = ig->members[x]->CastToClient();
									else
										continue;
									r->SendRaidCreate(c);
									r->SendMakeLeaderPacketTo(r->leadername, c);
									r->AddMember(c, groupFree, true, true, true);
									r->SendBulkRaid(c);
									if (r->IsLocked()) {
										r->SendRaidLockTo(c);
									}
								}
								else {
									Client *c = nullptr;
									if (ig->members[x]->IsClient())
										c = ig->members[x]->CastToClient();
									else
										continue;
									r->SendRaidCreate(c);
									r->SendMakeLeaderPacketTo(r->leadername, c);
									r->AddMember(c, groupFree);
									r->SendBulkRaid(c);
									if (r->IsLocked()) {
										r->SendRaidLockTo(c);
									}
								}
							}
						}
						ig->JoinRaidXTarget(r, true);
						ig->DisbandGroup(true);
						r->GroupUpdate(groupFree);
						groupFree = r->GetFreeGroup();
					}
					else { //else just add the inviter
						r->SendRaidCreate(i);
						r->AddMember(i, 0xFFFFFFFF, true, false, true);
					}

					Client *addClient = nullptr;
					//now add the existing group
					for (int x = 0; x < 6; x++)
					{
						if (g->members[x]) {
							if (!addClient)
							{
								if (g->members[x]->IsClient()) {
									addClient = g->members[x]->CastToClient();
									r->SetGroupLeader(addClient->GetName());
								}
							}
							if (g->IsLeader(g->members[x]))
							{
								Client *c = nullptr;
								if (g->members[x]->IsClient())
									c = g->members[x]->CastToClient();
								else
									continue;
								r->SendRaidCreate(c);
								r->SendMakeLeaderPacketTo(r->leadername, c);
								r->AddMember(c, groupFree, false, true);
								r->SendBulkRaid(c);
								if (r->IsLocked()) {
									r->SendRaidLockTo(c);
								}
							}
							else
							{
								Client *c = nullptr;
								if (g->members[x]->IsClient())
									c = g->members[x]->CastToClient();
								else
									continue;
								r->SendRaidCreate(c);
								r->SendMakeLeaderPacketTo(r->leadername, c);
								r->AddMember(c, groupFree);
								r->SendBulkRaid(c);
								if (r->IsLocked()) {
									r->SendRaidLockTo(c);
								}
							}
						}
					}
					g->JoinRaidXTarget(r);
					g->DisbandGroup(true);
					r->GroupUpdate(groupFree);
				}
				else // target does not have a group
				{
					if (ig) {
						r = new Raid(i);
						entity_list.AddRaid(r);
						r->SetRaidDetails();
						Client *addClientig = nullptr;
						for (int x = 0; x < 6; x++)
						{
							if (ig->members[x])
							{
								if (!addClientig) {
									if (ig->members[x]->IsClient()) {
										addClientig = ig->members[x]->CastToClient();
										r->SetGroupLeader(addClientig->GetName());
									}
								}
								if (ig->IsLeader(ig->members[x]))
								{
									Client *c = nullptr;
									if (ig->members[x]->IsClient())
										c = ig->members[x]->CastToClient();
									else
										continue;

									r->SendRaidCreate(c);
									r->SendMakeLeaderPacketTo(r->leadername, c);
									r->AddMember(c, 0, true, true, true);
									r->SendBulkRaid(c);
									if (r->IsLocked()) {
										r->SendRaidLockTo(c);
									}
								}
								else
								{
									Client *c = nullptr;
									if (ig->members[x]->IsClient())
										c = ig->members[x]->CastToClient();
									else
										continue;

									r->SendRaidCreate(c);
									r->SendMakeLeaderPacketTo(r->leadername, c);
									r->AddMember(c, 0);
									r->SendBulkRaid(c);
									if (r->IsLocked()) {
										r->SendRaidLockTo(c);
									}
								}
							}
						}
						r->SendRaidCreate(this);
						r->SendMakeLeaderPacketTo(r->leadername, this);
						r->SendBulkRaid(this);
						ig->JoinRaidXTarget(r, true);
						r->AddMember(this);
						ig->DisbandGroup(true);
						r->GroupUpdate(0);
						if (r->IsLocked()) {
							r->SendRaidLockTo(this);
						}
					}
					else { // neither has a group
						r = new Raid(i);
						entity_list.AddRaid(r);
						r->SetRaidDetails();
						r->SendRaidCreate(i);
						r->SendRaidCreate(this);
						r->SendMakeLeaderPacketTo(r->leadername, this);
						r->AddMember(i, 0xFFFFFFFF, true, false, true);
						r->SendBulkRaid(this);
						r->AddMember(this);
						if (r->IsLocked()) {
							r->SendRaidLockTo(this);
						}
					}
				}
			}
		}
		break;
	}
	case RaidCommandDisband: {
		Raid *r = entity_list.GetRaidByClient(this);
		if (r) {
			//if(this == r->GetLeader()){
			uint32 grp = r->GetGroup(ri->leader_name);

			if (grp < 12) {
				uint32 i = r->GetPlayerIndex(ri->leader_name);
				if (r->members[i].IsGroupLeader) { //assign group leader to someone else
					for (int x = 0; x < MAX_RAID_MEMBERS; x++) {
						if (strlen(r->members[x].membername) > 0 && i != x) {
							if (r->members[x].GroupNumber == grp) {
								r->SetGroupLeader(ri->leader_name, false);
								r->SetGroupLeader(r->members[x].membername);
								r->UpdateGroupAAs(grp);
								break;
							}
						}
					}

				}
				if (r->members[i].IsRaidLeader) {
					for (int x = 0; x < MAX_RAID_MEMBERS; x++) {
						if (strlen(r->members[x].membername) > 0 && strcmp(r->members[x].membername, r->members[i].membername) != 0)
						{
							r->SetRaidLeader(r->members[i].membername, r->members[x].membername);
							r->UpdateRaidAAs();
							r->SendAllRaidLeadershipAA();
							break;
						}
					}
				}
			}

			r->RemoveMember(ri->leader_name);
			Client *c = entity_list.GetClientByName(ri->leader_name);
			if (c)
				r->SendGroupDisband(c);
			else {
				auto pack =
					new ServerPacket(ServerOP_RaidGroupDisband, sizeof(ServerRaidGeneralAction_Struct));
				ServerRaidGeneralAction_Struct* rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
				rga->rid = GetID();
				rga->zoneid = zone->GetZoneID();
				rga->instance_id = zone->GetInstanceID();
				strn0cpy(rga->playername, ri->leader_name, 64);
				worldserver.SendPacket(pack);
				safe_delete(pack);
			}
			//r->SendRaidGroupRemove(ri->leader_name, grp);
			r->GroupUpdate(grp);// break
								//}
		}
		break;
	}
	case RaidCommandMoveGroup:
	{
		Raid *r = entity_list.GetRaidByClient(this);
		if (r)
		{
			if (ri->parameter < 12) //moving to a group
			{
				uint8 grpcount = r->GroupCount(ri->parameter);

				if (grpcount < 6)
				{
					Client *c = entity_list.GetClientByName(ri->leader_name);
					uint32 oldgrp = r->GetGroup(ri->leader_name);
					if (ri->parameter == oldgrp) //don't rejoin grp if we order to join same group.
						break;

					if (r->members[r->GetPlayerIndex(ri->leader_name)].IsGroupLeader)
					{
						r->SetGroupLeader(ri->leader_name, false);
						if (oldgrp < 12) { //we were the leader of our old grp
							for (int x = 0; x < MAX_RAID_MEMBERS; x++) //assign a new grp leader if we can
							{
								if (r->members[x].GroupNumber == oldgrp)
								{
									if (strcmp(ri->leader_name, r->members[x].membername) != 0 && strlen(ri->leader_name) > 0)
									{
										r->SetGroupLeader(r->members[x].membername);
										r->UpdateGroupAAs(oldgrp);
										Client *cgl = entity_list.GetClientByName(r->members[x].membername);
										if (cgl) {
											r->SendRaidRemove(r->members[x].membername, cgl);
											r->SendRaidCreate(cgl);
											r->SendMakeLeaderPacketTo(r->leadername, cgl);
											r->SendRaidAdd(r->members[x].membername, cgl);
											r->SendBulkRaid(cgl);
											if (r->IsLocked()) {
												r->SendRaidLockTo(cgl);
											}
										}
										else {
											auto pack = new ServerPacket(
												ServerOP_RaidChangeGroup,
												sizeof(
													ServerRaidGeneralAction_Struct));
											ServerRaidGeneralAction_Struct *rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
											rga->rid = r->GetID();
											strn0cpy(rga->playername, r->members[x].membername, 64);
											rga->zoneid = zone->GetZoneID();
											rga->instance_id = zone->GetInstanceID();
											worldserver.SendPacket(pack);
											safe_delete(pack);
										}
										break;
									}
								}
							}
						}
					}
					if (grpcount == 0) {
						r->SetGroupLeader(ri->leader_name);
						r->UpdateGroupAAs(ri->parameter);
					}

					r->MoveMember(ri->leader_name, ri->parameter);
					if (c) {
						r->SendGroupDisband(c);
					}
					else {
						auto pack = new ServerPacket(ServerOP_RaidGroupDisband,
							sizeof(ServerRaidGeneralAction_Struct));
						ServerRaidGeneralAction_Struct* rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
						rga->rid = r->GetID();
						rga->zoneid = zone->GetZoneID();
						rga->instance_id = zone->GetInstanceID();
						strn0cpy(rga->playername, ri->leader_name, 64);
						worldserver.SendPacket(pack);
						safe_delete(pack);
					}
					//r->SendRaidGroupAdd(ri->leader_name, ri->parameter);
					//r->SendRaidGroupRemove(ri->leader_name, oldgrp);
					//r->SendGroupUpdate(c);
					//break
					r->GroupUpdate(ri->parameter); //send group update to our new group
					if (oldgrp < 12) //if our old was a group send update there too
						r->GroupUpdate(oldgrp);

					//r->SendMakeGroupLeaderPacketAll();
				}
			}
			else //moving to ungrouped
			{
				Client *c = entity_list.GetClientByName(ri->leader_name);
				uint32 oldgrp = r->GetGroup(ri->leader_name);
				if (r->members[r->GetPlayerIndex(ri->leader_name)].IsGroupLeader) {
					r->SetGroupLeader(ri->leader_name, false);
					for (int x = 0; x < MAX_RAID_MEMBERS; x++)
					{
						if (r->members[x].GroupNumber == oldgrp && strlen(r->members[x].membername) > 0 && strcmp(r->members[x].membername, ri->leader_name) != 0)
						{
							r->SetGroupLeader(r->members[x].membername);
							r->UpdateGroupAAs(oldgrp);
							Client *cgl = entity_list.GetClientByName(r->members[x].membername);
							if (cgl) {
								r->SendRaidRemove(r->members[x].membername, cgl);
								r->SendRaidCreate(cgl);
								r->SendMakeLeaderPacketTo(r->leadername, cgl);
								r->SendRaidAdd(r->members[x].membername, cgl);
								r->SendBulkRaid(cgl);
								if (r->IsLocked()) {
									r->SendRaidLockTo(cgl);
								}
							}
							else {
								auto pack = new ServerPacket(
									ServerOP_RaidChangeGroup,
									sizeof(ServerRaidGeneralAction_Struct));
								ServerRaidGeneralAction_Struct *rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
								rga->rid = r->GetID();
								strn0cpy(rga->playername, r->members[x].membername, 64);
								rga->zoneid = zone->GetZoneID();
								rga->instance_id = zone->GetInstanceID();
								worldserver.SendPacket(pack);
								safe_delete(pack);
							}
							break;
						}
					}
				}
				r->MoveMember(ri->leader_name, 0xFFFFFFFF);
				if (c) {
					r->SendGroupDisband(c);
				}
				else {
					auto pack = new ServerPacket(ServerOP_RaidGroupDisband,
						sizeof(ServerRaidGeneralAction_Struct));
					ServerRaidGeneralAction_Struct* rga = (ServerRaidGeneralAction_Struct*)pack->pBuffer;
					rga->rid = r->GetID();
					rga->zoneid = zone->GetZoneID();
					rga->instance_id = zone->GetInstanceID();
					strn0cpy(rga->playername, ri->leader_name, 64);
					worldserver.SendPacket(pack);
					safe_delete(pack);
				}
				//r->SendRaidGroupRemove(ri->leader_name, oldgrp);
				r->GroupUpdate(oldgrp);
				//r->SendMakeGroupLeaderPacketAll();
			}
		}
		break;
	}
	case RaidCommandRaidLock:
	{
		Raid *r = entity_list.GetRaidByClient(this);
		if (r)
		{
			if (!r->IsLocked())
				r->LockRaid(true);
			else
				r->SendRaidLockTo(this);
		}
		break;
	}
	case RaidCommandRaidUnlock:
	{
		Raid *r = entity_list.GetRaidByClient(this);
		if (r)
		{
			if (r->IsLocked())
				r->LockRaid(false);
			else
				r->SendRaidUnlockTo(this);
		}
		break;
	}
	case RaidCommandLootType2:
	case RaidCommandLootType:
	{
		Raid *r = entity_list.GetRaidByClient(this);
		if (r)
		{
			Message(15, "Loot type changed to: %d.", ri->parameter);
			r->ChangeLootType(ri->parameter);
		}
		break;
	}

	case RaidCommandAddLooter2:
	case RaidCommandAddLooter:
	{
		Raid *r = entity_list.GetRaidByClient(this);
		if (r)
		{
			Message(15, "Adding %s as a raid looter.", ri->leader_name);
			r->AddRaidLooter(ri->leader_name);
		}
		break;
	}

	case RaidCommandRemoveLooter2:
	case RaidCommandRemoveLooter:
	{
		Raid *r = entity_list.GetRaidByClient(this);
		if (r)
		{
			Message(15, "Removing %s as a raid looter.", ri->leader_name);
			r->RemoveRaidLooter(ri->leader_name);
		}
		break;
	}

	case RaidCommandMakeLeader:
	{
		Raid *r = entity_list.GetRaidByClient(this);
		if (r)
		{
			if (strcmp(r->leadername, GetName()) == 0) {
				r->SetRaidLeader(GetName(), ri->leader_name);
				r->UpdateRaidAAs();
				r->SendAllRaidLeadershipAA();
			}
		}
		break;
	}

	case RaidCommandSetMotd:
	{
		Raid *r = entity_list.GetRaidByClient(this);
		if (!r)
			break;
		// we don't use the RaidGeneral here!
		RaidMOTD_Struct *motd = (RaidMOTD_Struct *)app->pBuffer;
		r->SetRaidMOTD(std::string(motd->motd));
		r->SaveRaidMOTD();
		r->SendRaidMOTDToWorld();
		break;
	}

	default: {
		Message(13, "Raid command (%d) NYI", ri->action);
		break;
	}
	}
}

void Client::Handle_OP_RandomReq(const EQApplicationPacket *app)
{
	if (app->size != sizeof(RandomReq_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_RandomReq, size=%i, expected %i", app->size, sizeof(RandomReq_Struct));
		return;
	}
	const RandomReq_Struct* rndq = (const RandomReq_Struct*)app->pBuffer;
	uint32 randLow = rndq->low > rndq->high ? rndq->high : rndq->low;
	uint32 randHigh = rndq->low > rndq->high ? rndq->low : rndq->high;
	uint32 randResult;

	if (randLow == 0 && randHigh == 0)
	{	// defaults
		randLow = 0;
		randHigh = 100;
	}
	randResult = zone->random.Int(randLow, randHigh);

	auto outapp = new EQApplicationPacket(OP_RandomReply, sizeof(RandomReply_Struct));
	RandomReply_Struct* rr = (RandomReply_Struct*)outapp->pBuffer;
	rr->low = randLow;
	rr->high = randHigh;
	rr->result = randResult;
	strcpy(rr->name, GetName());
	entity_list.QueueCloseClients(this, outapp, false, 400);
	safe_delete(outapp);
	return;
}

void Client::Handle_OP_ReadBook(const EQApplicationPacket *app)
{
	if (app->size != sizeof(BookRequest_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_ReadBook, size=%i, expected %i", app->size, sizeof(BookRequest_Struct));
		return;
	}
	BookRequest_Struct* book = (BookRequest_Struct*)app->pBuffer;
	ReadBook(book);
	if (ClientVersion() >= EQEmu::versions::ClientVersion::SoF)
	{
		EQApplicationPacket EndOfBook(OP_FinishWindow, 0);
		QueuePacket(&EndOfBook);
	}
	return;
}

void Client::Handle_OP_RecipeAutoCombine(const EQApplicationPacket *app)
{
	if (app->size != sizeof(RecipeAutoCombine_Struct)) {
		Log(Logs::General, Logs::Error, "Invalid size for RecipeAutoCombine_Struct: Expected: %i, Got: %i",
			sizeof(RecipeAutoCombine_Struct), app->size);
		return;
	}

	RecipeAutoCombine_Struct* rac = (RecipeAutoCombine_Struct*)app->pBuffer;

	Object::HandleAutoCombine(this, rac);
	return;
}

void Client::Handle_OP_RecipeDetails(const EQApplicationPacket *app)
{
	if (app->size < sizeof(uint32)) {
		Log(Logs::General, Logs::Error, "Invalid size for RecipeDetails Request: Expected: %i, Got: %i",
			sizeof(uint32), app->size);
		return;
	}
	uint32 *recipe_id = (uint32*)app->pBuffer;

	SendTradeskillDetails(*recipe_id);

	return;
}

void Client::Handle_OP_RecipesFavorite(const EQApplicationPacket *app)
{
	if (app->size != sizeof(TradeskillFavorites_Struct)) {
		Log(Logs::General, Logs::Error, "Invalid size for TradeskillFavorites_Struct: Expected: %i, Got: %i",
			sizeof(TradeskillFavorites_Struct), app->size);
		return;
	}

	TradeskillFavorites_Struct* tsf = (TradeskillFavorites_Struct*)app->pBuffer;

	Log(Logs::General, Logs::None, "Requested Favorites for: %d - %d\n", tsf->object_type, tsf->some_id);

	// results show that object_type is combiner type
	// some_id = 0 if world combiner, item number otherwise

	// make where clause segment for container(s)
	std::string containers;
	if (tsf->some_id == 0)
		containers += StringFormat(" = %u ", tsf->object_type); // world combiner so no item number
	else
		containers += StringFormat(" in (%u, %u) ", tsf->object_type, tsf->some_id); // container in inventory

	std::string favoriteIDs; //gotta be big enough for 500 IDs
	bool first = true;
	//Assumes item IDs are <10 characters long
	for (uint16 favoriteIndex = 0; favoriteIndex < 500; ++favoriteIndex) {
		if (tsf->favorite_recipes[favoriteIndex] == 0)
			continue;

		if (first) {
			favoriteIDs += StringFormat("%u", tsf->favorite_recipes[favoriteIndex]);
			first = false;
		}
		else
			favoriteIDs += StringFormat(",%u", tsf->favorite_recipes[favoriteIndex]);
	}

	if (first)	//no favorites....
		return;

	const std::string query = StringFormat("SELECT tr.id, tr.name, tr.trivial, "
		"SUM(tre.componentcount), crl.madecount,tr.tradeskill "
		"FROM tradeskill_recipe AS tr "
		"LEFT JOIN tradeskill_recipe_entries AS tre ON tr.id=tre.recipe_id "
		"LEFT JOIN (SELECT recipe_id, madecount "
		"FROM char_recipe_list "
		"WHERE char_id = %u) AS crl ON tr.id=crl.recipe_id "
		"WHERE tr.enabled <> 0 AND tr.id IN (%s) "
		"AND tr.must_learn & 0x20 <> 0x20 AND "
		"((tr.must_learn & 0x3 <> 0 AND crl.madecount IS NOT NULL) "
		"OR (tr.must_learn & 0x3 = 0)) "
		"GROUP BY tr.id "
		"HAVING sum(if(tre.item_id %s AND tre.iscontainer > 0,1,0)) > 0 "
		"LIMIT 100 ", CharacterID(), favoriteIDs.c_str(), containers.c_str());

	TradeskillSearchResults(query, tsf->object_type, tsf->some_id);
	return;
}

void Client::Handle_OP_RecipesSearch(const EQApplicationPacket *app)
{
	if (app->size != sizeof(RecipesSearch_Struct)) {
		Log(Logs::General, Logs::Error, "Invalid size for RecipesSearch_Struct: Expected: %i, Got: %i",
			sizeof(RecipesSearch_Struct), app->size);
		return;
	}

	RecipesSearch_Struct* rss = (RecipesSearch_Struct*)app->pBuffer;
	rss->query[55] = '\0';	//just to be sure.


	Log(Logs::General, Logs::None, "Requested search recipes for: %d - %d\n", rss->object_type, rss->some_id);

	// make where clause segment for container(s)
	char containers[30];
	if (rss->some_id == 0) {
		// world combiner so no item number
		snprintf(containers, 29, "= %u", rss->object_type);
	}
	else {
		// container in inventory
		snprintf(containers, 29, "in (%u,%u)", rss->object_type, rss->some_id);
	}

	std::string searchClause;

	//omit the rlike clause if query is empty
	if (rss->query[0] != 0) {
		char buf[120];	//larger than 2X rss->query
		database.DoEscapeString(buf, rss->query, strlen(rss->query));
		searchClause = StringFormat("name rlike '%s' AND", buf);
	}

	//arbitrary limit of 200 recipes, makes sense to me.
	const std::string query = StringFormat("SELECT tr.id, tr.name, tr.trivial, "
		"SUM(tre.componentcount), crl.madecount,tr.tradeskill "
		"FROM tradeskill_recipe AS tr "
		"LEFT JOIN tradeskill_recipe_entries AS tre ON tr.id = tre.recipe_id "
		"LEFT JOIN (SELECT recipe_id, madecount "
		"FROM char_recipe_list WHERE char_id = %u) AS crl ON tr.id=crl.recipe_id "
		"WHERE %s tr.trivial >= %u AND tr.trivial <= %u AND tr.enabled <> 0 "
		"AND tr.must_learn & 0x20 <> 0x20 "
		"AND ((tr.must_learn & 0x3 <> 0 "
		"AND crl.madecount IS NOT NULL) "
		"OR (tr.must_learn & 0x3 = 0)) "
		"GROUP BY tr.id "
		"HAVING sum(if(tre.item_id %s AND tre.iscontainer > 0,1,0)) > 0 "
		"LIMIT 200 ",
		CharacterID(), searchClause.c_str(),
		rss->mintrivial, rss->maxtrivial, containers);
	TradeskillSearchResults(query, rss->object_type, rss->some_id);
	return;
}

void Client::Handle_OP_ReloadUI(const EQApplicationPacket *app)
{
	if (IsInAGuild())
	{
		SendGuildRanks();
		SendGuildMembers();
	}
	return;
}

void Client::Handle_OP_RemoveBlockedBuffs(const EQApplicationPacket *app)
{
	if (!RuleB(Spells, EnableBlockedBuffs))
		return;

	if (app->size != sizeof(BlockedBuffs_Struct))
	{
		Log(Logs::General, Logs::None, "Size mismatch in OP_RemoveBlockedBuffs expected %i got %i",
			sizeof(BlockedBuffs_Struct), app->size);

		DumpPacket(app);

		return;
	}
	BlockedBuffs_Struct *bbs = (BlockedBuffs_Struct*)app->pBuffer;

	std::set<uint32> *BlockedBuffs = bbs->Pet ? &PetBlockedBuffs : &PlayerBlockedBuffs;

	std::set<uint32> RemovedBuffs;

	if (bbs->Count > 0)
	{
		std::set<uint32>::iterator Iterator;

		auto outapp = new EQApplicationPacket(OP_RemoveBlockedBuffs, sizeof(BlockedBuffs_Struct));

		BlockedBuffs_Struct *obbs = (BlockedBuffs_Struct*)outapp->pBuffer;

		for (unsigned int i = 0; i < BLOCKED_BUFF_COUNT; ++i)
			obbs->SpellID[i] = 0;

		obbs->Pet = bbs->Pet;
		obbs->Initialise = 0;
		obbs->Flags = 0x5a;

		for (unsigned int i = 0; i < bbs->Count; ++i)
		{
			Iterator = BlockedBuffs->find(bbs->SpellID[i]);

			if (Iterator != BlockedBuffs->end())
			{
				RemovedBuffs.insert(bbs->SpellID[i]);

				BlockedBuffs->erase(Iterator);
			}
		}
		obbs->Count = RemovedBuffs.size();

		Iterator = RemovedBuffs.begin();

		unsigned int Element = 0;

		while (Iterator != RemovedBuffs.end())
		{
			obbs->SpellID[Element++] = (*Iterator);
			++Iterator;
		}

		FastQueuePacket(&outapp);
	}
}

void Client::Handle_OP_Report(const EQApplicationPacket *app)
{
	if (!CanUseReport)
	{
		Message_StringID(MT_System, REPORT_ONCE);
		return;
	}

	uint32 size = app->size;
	uint32 current_point = 0;
	std::string reported, reporter;
	std::string current_string;
	int mode = 0;

	while (current_point < size)
	{
		if (mode < 2)
		{
			if (app->pBuffer[current_point] == '|')
			{
				mode++;
			}
			else
			{
				if (mode == 0)
				{
					reported += app->pBuffer[current_point];
				}
				else
				{
					reporter += app->pBuffer[current_point];
				}
			}
			current_point++;
		}
		else
		{
			if (app->pBuffer[current_point] == 0x0a)
			{
				current_string += '\n';
			}
			else if (app->pBuffer[current_point] == 0x00)
			{
				CanUseReport = false;
				database.AddReport(reporter, reported, current_string);
				return;
			}
			else
			{
				current_string += app->pBuffer[current_point];
			}
			current_point++;
		}
	}

	CanUseReport = false;
	database.AddReport(reporter, reported, current_string);
}

void Client::Handle_OP_RequestDuel(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Duel_Struct))
		return;

	EQApplicationPacket* outapp = app->Copy();
	Duel_Struct* ds = (Duel_Struct*)outapp->pBuffer;
	uint32 duel = ds->duel_initiator;
	ds->duel_initiator = ds->duel_target;
	ds->duel_target = duel;
	Entity* entity = entity_list.GetID(ds->duel_target);
	if (GetID() != ds->duel_target && entity->IsClient() && (entity->CastToClient()->IsDueling() && entity->CastToClient()->GetDuelTarget() != 0)) {
		Message_StringID(10, DUEL_CONSIDERING, entity->GetName());
		return;
	}
	if (IsDueling()) {
		Message_StringID(10, DUEL_INPROGRESS);
		return;
	}

	if (GetID() != ds->duel_target && entity->IsClient() && GetDuelTarget() == 0 && !IsDueling() && !entity->CastToClient()->IsDueling() && entity->CastToClient()->GetDuelTarget() == 0) {
		SetDuelTarget(ds->duel_target);
		entity->CastToClient()->SetDuelTarget(GetID());
		ds->duel_target = ds->duel_initiator;
		entity->CastToClient()->FastQueuePacket(&outapp);
		entity->CastToClient()->SetDueling(false);
		SetDueling(false);
	}
	else
		safe_delete(outapp);
	return;
}

void Client::Handle_OP_RequestTitles(const EQApplicationPacket *app)
{

	EQApplicationPacket *outapp = title_manager.MakeTitlesPacket(this);

	if (outapp != nullptr)
		FastQueuePacket(&outapp);
}

void Client::Handle_OP_RespawnWindow(const EQApplicationPacket *app)
{
	// This opcode is sent by the client when the player choses which bind to return to.
	// The client sends just a 4 byte packet with the selection number in it
	//
	if (app->size != 4)
	{
		Log(Logs::General, Logs::None, "Size mismatch in OP_RespawnWindow expected %i got %i",
			4, app->size);
		DumpPacket(app);
		return;
	}
	char *Buffer = (char *)app->pBuffer;

	uint32 Option = VARSTRUCT_DECODE_TYPE(uint32, Buffer);
	HandleRespawnFromHover(Option);
}

void Client::Handle_OP_Rewind(const EQApplicationPacket *app)
{
	if ((rewind_timer.GetRemainingTime() > 1 && rewind_timer.Enabled())) {
		Message_StringID(MT_System, REWIND_WAIT);
	}
	else {
		CastToClient()->MovePC(zone->GetZoneID(), zone->GetInstanceID(), m_RewindLocation.x, m_RewindLocation.y, m_RewindLocation.z, 0, 2, Rewind);
		rewind_timer.Start(30000, true);
	}
}

void Client::Handle_OP_RezzAnswer(const EQApplicationPacket *app)
{
	VERIFY_PACKET_LENGTH(OP_RezzAnswer, app, Resurrect_Struct);

	const Resurrect_Struct* ra = (const Resurrect_Struct*)app->pBuffer;

	Log(Logs::Detail, Logs::Spells, "Received OP_RezzAnswer from client. Pendingrezzexp is %i, action is %s",
		PendingRezzXP, ra->action ? "ACCEPT" : "DECLINE");


	OPRezzAnswer(ra->action, ra->spellid, ra->zone_id, ra->instance_id, ra->x, ra->y, ra->z);

	if (ra->action == 1)
	{
		EQApplicationPacket* outapp = app->Copy();
		// Send the OP_RezzComplete to the world server. This finds it's way to the zone that
		// the rezzed corpse is in to mark the corpse as rezzed.
		outapp->SetOpcode(OP_RezzComplete);
		worldserver.RezzPlayer(outapp, 0, 0, OP_RezzComplete);
		safe_delete(outapp);
	}
	return;
}

void Client::Handle_OP_Sacrifice(const EQApplicationPacket *app)
{

	if (app->size != sizeof(Sacrifice_Struct)) {
		Log(Logs::General, Logs::None, "Size mismatch in OP_Sacrifice expected %i got %i", sizeof(Sacrifice_Struct), app->size);
		DumpPacket(app);
		return;
	}
	Sacrifice_Struct *ss = (Sacrifice_Struct*)app->pBuffer;

	if (!PendingSacrifice) {
		Log(Logs::General, Logs::Error, "Unexpected OP_Sacrifice reply");
		DumpPacket(app);
		return;
	}

	if (ss->Confirm) {
		Client *Caster = entity_list.GetClientByName(SacrificeCaster.c_str());
		if (Caster) Sacrifice(Caster);
	}
	PendingSacrifice = false;
	SacrificeCaster.clear();
}

void Client::Handle_OP_SafeFallSuccess(const EQApplicationPacket *app)	// bit of a misnomer, sent whenever safe fall is used (success of fail)
{
	if (HasSkill(EQEmu::skills::SkillSafeFall)) //this should only get called if the client has safe fall, but just in case...
		CheckIncreaseSkill(EQEmu::skills::SkillSafeFall, nullptr); //check for skill up
}

void Client::Handle_OP_SafePoint(const EQApplicationPacket *app)
{
	return;
}

void Client::Handle_OP_Save(const EQApplicationPacket *app)
{
	// The payload is 192 bytes - Not sure what is contained in payload
	Save();
	return;
}

void Client::Handle_OP_SaveOnZoneReq(const EQApplicationPacket *app)
{
	Handle_OP_Save(app);
}

void Client::Handle_OP_SelectTribute(const EQApplicationPacket *app)
{
	Log(Logs::Detail, Logs::Tribute, "Received OP_SelectTribute of length %d", app->size);

	//we should enforce being near a real tribute master to change this
	//but im not sure how I wanna do that right now.
	if (app->size != sizeof(SelectTributeReq_Struct))
		Log(Logs::General, Logs::Error, "Invalid size on OP_SelectTribute packet");
	else {
		SelectTributeReq_Struct *t = (SelectTributeReq_Struct *)app->pBuffer;
		SendTributeDetails(t->client_id, t->tribute_id);
	}
	return;
}

void Client::Handle_OP_SenseHeading(const EQApplicationPacket *app)
{
	if (!HasSkill(EQEmu::skills::SkillSenseHeading))
		return;

	int chancemod = 0;

	// The client seems to limit sense heading packets based on skill
	// level.  So if we're really low, we don't hit this routine very often.
	// I think it's the GUI deciding when to skill you up.
	// So, I'm adding a mod here which is larger at lower levels so
	// very low levels get a much better chance to skill up when the GUI
	// eventually sends a message.
	if (GetLevel() <= 8)
		chancemod += (9 - level) * 10;

	CheckIncreaseSkill(EQEmu::skills::SkillSenseHeading, nullptr, chancemod);

	return;
}

void Client::Handle_OP_SenseTraps(const EQApplicationPacket *app)
{
	if (!HasSkill(EQEmu::skills::SkillSenseTraps))
		return;

	if (!p_timers.Expired(&database, pTimerSenseTraps, false)) {
		Message(13, "Ability recovery time not yet met.");
		return;
	}

	int reuse = SenseTrapsReuseTime - GetSkillReuseTime(EQEmu::skills::SkillSenseTraps);

	if (reuse < 1)
		reuse = 1;

	p_timers.Start(pTimerSenseTraps, reuse - 1);

	Trap* trap = entity_list.FindNearbyTrap(this, 800);

	CheckIncreaseSkill(EQEmu::skills::SkillSenseTraps, nullptr);

	if (trap && trap->skill > 0) {
		int uskill = GetSkill(EQEmu::skills::SkillSenseTraps);
		if ((zone->random.Int(0, 99) + uskill) >= (zone->random.Int(0, 99) + trap->skill*0.75))
		{
			auto diff = trap->m_Position - glm::vec3(GetPosition());

			if (diff.x == 0 && diff.y == 0)
				Message(MT_Skills, "You sense a trap right under your feet!");
			else if (diff.x > 10 && diff.y > 10)
				Message(MT_Skills, "You sense a trap to the NorthWest.");
			else if (diff.x < -10 && diff.y > 10)
				Message(MT_Skills, "You sense a trap to the NorthEast.");
			else if (diff.y > 10)
				Message(MT_Skills, "You sense a trap to the North.");
			else if (diff.x > 10 && diff.y < -10)
				Message(MT_Skills, "You sense a trap to the SouthWest.");
			else if (diff.x < -10 && diff.y < -10)
				Message(MT_Skills, "You sense a trap to the SouthEast.");
			else if (diff.y < -10)
				Message(MT_Skills, "You sense a trap to the South.");
			else if (diff.x > 10)
				Message(MT_Skills, "You sense a trap to the West.");
			else
				Message(MT_Skills, "You sense a trap to the East.");
			trap->detected = true;

			float angle = CalculateHeadingToTarget(trap->m_Position.x, trap->m_Position.y);

			if (angle < 0)
				angle = (256 + angle);

			angle *= 2;
			MovePC(zone->GetZoneID(), zone->GetInstanceID(), GetX(), GetY(), GetZ(), angle);
			return;
		}
	}
	Message(MT_Skills, "You did not find any traps nearby.");
	return;
}

void Client::Handle_OP_SetGuildMOTD(const EQApplicationPacket *app)
{
	Log(Logs::Detail, Logs::Guilds, "Received OP_SetGuildMOTD");

	if (app->size != sizeof(GuildMOTD_Struct)) {
		// client calls for a motd on login even if they arent in a guild
		printf("Error: app size of %i != size of GuildMOTD_Struct of %zu\n", app->size, sizeof(GuildMOTD_Struct));
		return;
	}
	if (!IsInAGuild()) {
		Message(13, "You are not in a guild!");
		return;
	}
	if (!guild_mgr.CheckPermission(GuildID(), GuildRank(), GUILD_MOTD)) {
		Message(13, "You do not have permissions to edit your guild's MOTD.");
		return;
	}

	GuildMOTD_Struct* gmotd = (GuildMOTD_Struct*)app->pBuffer;

	Log(Logs::Detail, Logs::Guilds, "Setting MOTD for %s (%d) to: %s - %s",
		guild_mgr.GetGuildName(GuildID()), GuildID(), GetName(), gmotd->motd);

	if (!guild_mgr.SetGuildMOTD(GuildID(), gmotd->motd, GetName())) {
		Message(0, "Motd update failed.");
	}

	return;
}

void Client::Handle_OP_SetRunMode(const EQApplicationPacket *app)
{
	if (app->size < sizeof(SetRunMode_Struct)) {
		Log(Logs::General, Logs::Error, "Received invalid sized "
			"OP_SetRunMode: got %d, expected %d", app->size,
			sizeof(SetRunMode_Struct));
		DumpPacket(app);
		return;
	}
	SetRunMode_Struct* rms = (SetRunMode_Struct*)app->pBuffer;
	if (rms->mode)
		runmode = true;
	else
		runmode = false;
	return;
}

void Client::Handle_OP_SetServerFilter(const EQApplicationPacket *app)
{
	if (app->size != sizeof(SetServerFilter_Struct)) {
		Log(Logs::General, Logs::Error, "Received invalid sized "
			"OP_SetServerFilter: got %d, expected %d", app->size,
			sizeof(SetServerFilter_Struct));
		DumpPacket(app);
		return;
	}
	SetServerFilter_Struct* filter = (SetServerFilter_Struct*)app->pBuffer;
	ServerFilter(filter);
	return;
}

void Client::Handle_OP_SetStartCity(const EQApplicationPacket *app)
{
	// if the character has a start city, don't let them use the command
	if (m_pp.binds[4].zoneId != 0 && m_pp.binds[4].zoneId != 189) {
		Message(15, "Your home city has already been set.", m_pp.binds[4].zoneId, database.GetZoneName(m_pp.binds[4].zoneId));
		return;
	}

	if (app->size < 1) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_SetStartCity, size=%i, expected %i", app->size, 1);
		DumpPacket(app);
		return;
	}

	float x(0), y(0), z(0);
	uint32 zoneid = 0;
	uint32 startCity = (uint32)strtol((const char*)app->pBuffer, nullptr, 10);

	std::string query = StringFormat("SELECT zone_id, bind_id, x, y, z FROM start_zones "
		"WHERE player_class=%i AND player_deity=%i AND player_race=%i",
		m_pp.class_, m_pp.deity, m_pp.race);
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		Log(Logs::General, Logs::Error, "No valid start zones found for /setstartcity");
		return;
	}

	bool validCity = false;
	for (auto row = results.begin(); row != results.end(); ++row) {
		if (atoi(row[1]) != 0)
			zoneid = atoi(row[1]);
		else
			zoneid = atoi(row[0]);

		if (zoneid != startCity)
			continue;

		validCity = true;
		x = atof(row[2]);
		y = atof(row[3]);
		z = atof(row[4]);
	}

	if (validCity) {
		Message(15, "Your home city has been set");
		SetStartZone(startCity, x, y, z);
		return;
	}

	query = StringFormat("SELECT zone_id, bind_id FROM start_zones "
		"WHERE player_class=%i AND player_deity=%i AND player_race=%i",
		m_pp.class_, m_pp.deity, m_pp.race);
	results = database.QueryDatabase(query);
	if (!results.Success())
		return;

	Message(15, "Use \"/setstartcity #\" to choose a home city from the following list:");

	for (auto row = results.begin(); row != results.end(); ++row) {
		if (atoi(row[1]) != 0)
			zoneid = atoi(row[1]);
		else
			zoneid = atoi(row[0]);

		char* name = nullptr;
		database.GetZoneLongName(database.GetZoneName(zoneid), &name);
		Message(15, "%d - %s", zoneid, name);
	}

}

void Client::Handle_OP_SetTitle(const EQApplicationPacket *app)
{
	if (app->size != sizeof(SetTitle_Struct)) {
		Log(Logs::General, Logs::None, "Size mismatch in OP_SetTitle expected %i got %i", sizeof(SetTitle_Struct), app->size);
		DumpPacket(app);
		return;
	}

	SetTitle_Struct *sts = (SetTitle_Struct *)app->pBuffer;

	std::string Title;

	if (!sts->is_suffix)
	{
		Title = title_manager.GetPrefix(sts->title_id);
		SetAATitle(Title.c_str());
	}
	else
	{
		Title = title_manager.GetSuffix(sts->title_id);
		SetTitleSuffix(Title.c_str());
	}
}

void Client::Handle_OP_Shielding(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Shielding_Struct)) {
		Log(Logs::General, Logs::Error, "OP size error: OP_Shielding expected:%i got:%i", sizeof(Shielding_Struct), app->size);
		return;
	}
	if (GetClass() != WARRIOR)
	{
		return;
	}

	if (shield_target)
	{
		entity_list.MessageClose_StringID(this, false, 100, 0,
			END_SHIELDING, GetName(), shield_target->GetName());
		for (int y = 0; y < 2; y++)
		{
			if (shield_target->shielder[y].shielder_id == GetID())
			{
				shield_target->shielder[y].shielder_id = 0;
				shield_target->shielder[y].shielder_bonus = 0;
			}
		}
	}
	Shielding_Struct* shield = (Shielding_Struct*)app->pBuffer;
	shield_target = entity_list.GetMob(shield->target_id);
	bool ack = false;
	EQEmu::ItemInstance* inst = GetInv().GetItem(EQEmu::inventory::slotSecondary);
	if (!shield_target)
		return;
	if (inst)
	{
		const EQEmu::ItemData* shield = inst->GetItem();
		if (shield && shield->ItemType == EQEmu::item::ItemTypeShield)
		{
			for (int x = 0; x < 2; x++)
			{
				if (shield_target->shielder[x].shielder_id == 0)
				{
					entity_list.MessageClose_StringID(this, false, 100, 0,
						START_SHIELDING, GetName(), shield_target->GetName());
					shield_target->shielder[x].shielder_id = GetID();
					int shieldbonus = shield->AC * 2;
					switch (GetAA(197))
					{
					case 1:
						shieldbonus = shieldbonus * 115 / 100;
						break;
					case 2:
						shieldbonus = shieldbonus * 125 / 100;
						break;
					case 3:
						shieldbonus = shieldbonus * 150 / 100;
						break;
					}
					shield_target->shielder[x].shielder_bonus = shieldbonus;
					shield_timer.Start();
					ack = true;
					break;
				}
			}
		}
		else
		{
			Message(0, "You must have a shield equipped to shield a target!");
			shield_target = 0;
			return;
		}
	}
	else
	{
		Message(0, "You must have a shield equipped to shield a target!");
		shield_target = 0;
		return;
	}
	if (!ack)
	{
		Message_StringID(0, ALREADY_SHIELDED);
		shield_target = 0;
		return;
	}
	return;
}

void Client::Handle_OP_ShopEnd(const EQApplicationPacket *app)
{
	EQApplicationPacket empty(OP_ShopEndConfirm);
	QueuePacket(&empty);
	return;
}

void Client::Handle_OP_ShopPlayerBuy(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Merchant_Sell_Struct)) {
		Log(Logs::General, Logs::Error, "Invalid size on OP_ShopPlayerBuy: Expected %i, Got %i",
			sizeof(Merchant_Sell_Struct), app->size);
		return;
	}
	RDTSC_Timer t1;
	t1.start();
	Merchant_Sell_Struct* mp = (Merchant_Sell_Struct*)app->pBuffer;
#if EQDEBUG >= 5
	Log(Logs::General, Logs::None, "%s, purchase item..", GetName());
	DumpPacket(app);
#endif

	int merchantid;
	bool tmpmer_used = false;
	Mob* tmp = entity_list.GetMob(mp->npcid);

	if (tmp == 0 || !tmp->IsNPC() || tmp->GetClass() != MERCHANT)
		return;

	if (mp->quantity < 1) return;

	//you have to be somewhat close to them to be properly using them
	if (DistanceSquared(m_Position, tmp->GetPosition()) > USE_NPC_RANGE2)
		return;

	merchantid = tmp->CastToNPC()->MerchantType;

	uint32 item_id = 0;
	std::list<MerchantList> merlist = zone->merchanttable[merchantid];
	std::list<MerchantList>::const_iterator itr;
	for (itr = merlist.begin(); itr != merlist.end(); ++itr) {
		MerchantList ml = *itr;
		if (GetLevel() < ml.level_required) {
			continue;
		}

		if (mp->itemslot == ml.slot) {
			item_id = ml.item;
			break;
		}
	}
	const EQEmu::ItemData* item = nullptr;
	uint32 prevcharges = 0;
	if (item_id == 0) { //check to see if its on the temporary table
		std::list<TempMerchantList> tmp_merlist = zone->tmpmerchanttable[tmp->GetNPCTypeID()];
		std::list<TempMerchantList>::const_iterator tmp_itr;
		TempMerchantList ml;
		for (tmp_itr = tmp_merlist.begin(); tmp_itr != tmp_merlist.end(); ++tmp_itr) {
			ml = *tmp_itr;
			if (mp->itemslot == ml.slot) {
				item_id = ml.item;
				tmpmer_used = true;
				prevcharges = ml.charges;
				break;
			}
		}
	}
	item = database.GetItem(item_id);
	if (!item) {
		//error finding item, client didnt get the update packet for whatever reason, roleplay a tad
		Message(15, "%s tells you 'Sorry, that item is for display purposes only.' as they take the item off the shelf.", tmp->GetCleanName());
		auto delitempacket = new EQApplicationPacket(OP_ShopDelItem, sizeof(Merchant_DelItem_Struct));
		Merchant_DelItem_Struct* delitem = (Merchant_DelItem_Struct*)delitempacket->pBuffer;
		delitem->itemslot = mp->itemslot;
		delitem->npcid = mp->npcid;
		delitem->playerid = mp->playerid;
		delitempacket->priority = 6;
		entity_list.QueueCloseClients(tmp, delitempacket); //que for anyone that could be using the merchant so they see the update
		safe_delete(delitempacket);
		return;
	}
	if (CheckLoreConflict(item))
	{
		Message(15, "You can only have one of a lore item.");
		return;
	}
	if (tmpmer_used && (mp->quantity > prevcharges || item->MaxCharges > 1))
	{
		if (prevcharges > item->MaxCharges && item->MaxCharges > 1)
			mp->quantity = item->MaxCharges;
		else
			mp->quantity = prevcharges;
	}

	// Item's stackable, but the quantity they want to buy exceeds the max stackable quantity.
	if (item->Stackable && mp->quantity > item->StackSize)
		mp->quantity = item->StackSize;

	auto outapp = new EQApplicationPacket(OP_ShopPlayerBuy, sizeof(Merchant_Sell_Struct));
	Merchant_Sell_Struct* mpo = (Merchant_Sell_Struct*)outapp->pBuffer;
	mpo->quantity = mp->quantity;
	mpo->playerid = mp->playerid;
	mpo->npcid = mp->npcid;
	mpo->itemslot = mp->itemslot;

	int16 freeslotid = INVALID_INDEX;
	int16 charges = 0;
	if (item->Stackable || item->MaxCharges > 1)
		charges = mp->quantity;
	else
		charges = item->MaxCharges;

	EQEmu::ItemInstance* inst = database.CreateItem(item, charges);

	int SinglePrice = 0;
	if (RuleB(Merchant, UsePriceMod))
		SinglePrice = (item->Price * (RuleR(Merchant, SellCostMod)) * item->SellRate * Client::CalcPriceMod(tmp, false));
	else
		SinglePrice = (item->Price * (RuleR(Merchant, SellCostMod)) * item->SellRate);

	if (item->MaxCharges > 1)
		mpo->price = SinglePrice;
	else
		mpo->price = SinglePrice * mp->quantity;

	if (mpo->price < 0)
	{
		safe_delete(outapp);
		safe_delete(inst);
		return;
	}

	// this area needs some work..two inventory insertion check failure points
	// below do not return player's money..is this the intended behavior?

	if (!TakeMoneyFromPP(mpo->price))
	{
		char *hacker_str = nullptr;
		MakeAnyLenString(&hacker_str, "Vendor Cheat: attempted to buy %i of %i: %s that cost %d cp but only has %d pp %d gp %d sp %d cp\n",
			mpo->quantity, item->ID, item->Name,
			mpo->price, m_pp.platinum, m_pp.gold, m_pp.silver, m_pp.copper);
		database.SetMQDetectionFlag(AccountName(), GetName(), hacker_str, zone->GetShortName());
		safe_delete_array(hacker_str);
		safe_delete(outapp);
		safe_delete(inst);
		return;
	}

	bool stacked = TryStacking(inst);
	if (!stacked)
		freeslotid = m_inv.FindFreeSlot(false, true, item->Size);

	// shouldn't we be reimbursing if these two fail?

	//make sure we are not completely full...
	if (freeslotid == EQEmu::inventory::slotCursor) {
		if (m_inv.GetItem(EQEmu::inventory::slotCursor) != nullptr) {
			Message(13, "You do not have room for any more items.");
			safe_delete(outapp);
			safe_delete(inst);
			return;
		}
	}

	if (!stacked && freeslotid == INVALID_INDEX)
	{
		Message(13, "You do not have room for any more items.");
		safe_delete(outapp);
		safe_delete(inst);
		return;
	}

	std::string packet;
	if (!stacked && inst) {
		PutItemInInventory(freeslotid, *inst);
		SendItemPacket(freeslotid, inst, ItemPacketTrade);
	}
	else if (!stacked) {
		Log(Logs::General, Logs::Error, "OP_ShopPlayerBuy: item->ItemClass Unknown! Type: %i", item->ItemClass);
	}
	QueuePacket(outapp);
	if (inst && tmpmer_used) {
		int32 new_charges = prevcharges - mp->quantity;
		zone->SaveTempItem(merchantid, tmp->GetNPCTypeID(), item_id, new_charges);
		if (new_charges <= 0) {
			auto delitempacket = new EQApplicationPacket(OP_ShopDelItem, sizeof(Merchant_DelItem_Struct));
			Merchant_DelItem_Struct* delitem = (Merchant_DelItem_Struct*)delitempacket->pBuffer;
			delitem->itemslot = mp->itemslot;
			delitem->npcid = mp->npcid;
			delitem->playerid = mp->playerid;
			delitempacket->priority = 6;
			entity_list.QueueClients(tmp, delitempacket); //que for anyone that could be using the merchant so they see the update
			safe_delete(delitempacket);
		}
		else {
			// Update the charges/quantity in the merchant window
			inst->SetCharges(new_charges);
			inst->SetPrice(SinglePrice);
			inst->SetMerchantSlot(mp->itemslot);
			inst->SetMerchantCount(new_charges);

			SendItemPacket(mp->itemslot, inst, ItemPacketMerchant);
		}
	}
	safe_delete(inst);
	safe_delete(outapp);

	// start QS code
	// stacking purchases not supported at this time - entire process will need some work to catch them properly
	if (RuleB(QueryServ, PlayerLogMerchantTransactions)) {
		auto qspack =
			new ServerPacket(ServerOP_QSPlayerLogMerchantTransactions,
				sizeof(QSMerchantLogTransaction_Struct) + sizeof(QSTransactionItems_Struct));
		QSMerchantLogTransaction_Struct* qsaudit = (QSMerchantLogTransaction_Struct*)qspack->pBuffer;

		qsaudit->zone_id = zone->GetZoneID();
		qsaudit->merchant_id = tmp->CastToNPC()->MerchantType;
		qsaudit->merchant_money.platinum = 0;
		qsaudit->merchant_money.gold = 0;
		qsaudit->merchant_money.silver = 0;
		qsaudit->merchant_money.copper = 0;
		qsaudit->merchant_count = 1;
		qsaudit->char_id = character_id;
		qsaudit->char_money.platinum = (mpo->price / 1000);
		qsaudit->char_money.gold = (mpo->price / 100) % 10;
		qsaudit->char_money.silver = (mpo->price / 10) % 10;
		qsaudit->char_money.copper = mpo->price % 10;
		qsaudit->char_count = 0;

		qsaudit->items[0].char_slot = freeslotid == INVALID_INDEX ? 0 : freeslotid;
		qsaudit->items[0].item_id = item->ID;
		qsaudit->items[0].charges = mpo->quantity;

		const EQEmu::ItemInstance* audit_inst = m_inv[freeslotid];

		if (audit_inst) {
			qsaudit->items[0].aug_1 = audit_inst->GetAugmentItemID(0);
			qsaudit->items[0].aug_2 = audit_inst->GetAugmentItemID(1);
			qsaudit->items[0].aug_3 = audit_inst->GetAugmentItemID(2);
			qsaudit->items[0].aug_4 = audit_inst->GetAugmentItemID(3);
			qsaudit->items[0].aug_5 = audit_inst->GetAugmentItemID(4);
		}
		else {
			qsaudit->items[0].aug_1 = 0;
			qsaudit->items[0].aug_2 = 0;
			qsaudit->items[0].aug_3 = 0;
			qsaudit->items[0].aug_4 = 0;
			qsaudit->items[0].aug_5 = 0;

			if (freeslotid != INVALID_INDEX) {
				Log(Logs::General, Logs::Error, "Handle_OP_ShopPlayerBuy: QS Audit could not locate merchant (%u) purchased item in player (%u) inventory slot (%i)",
					qsaudit->merchant_id, qsaudit->char_id, freeslotid);
			}
		}

		audit_inst = nullptr;

		if (worldserver.Connected()) { worldserver.SendPacket(qspack); }
		safe_delete(qspack);
	}
	// end QS code

	if (RuleB(EventLog, RecordBuyFromMerchant))
		LogMerchant(this, tmp, mpo->quantity, mpo->price, item, true);

	if ((RuleB(Character, EnableDiscoveredItems)))
	{
		if (!GetGM() && !IsDiscovered(item_id))
			DiscoverItem(item_id);
	}

	t1.stop();
	std::cout << "At 1: " << t1.getDuration() << std::endl;
	return;
}
void Client::Handle_OP_ShopPlayerSell(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Merchant_Purchase_Struct)) {
		Log(Logs::General, Logs::Error, "Invalid size on OP_ShopPlayerSell: Expected %i, Got %i",
			sizeof(Merchant_Purchase_Struct), app->size);
		return;
	}
	RDTSC_Timer t1(true);
	Merchant_Purchase_Struct* mp = (Merchant_Purchase_Struct*)app->pBuffer;

	Mob* vendor = entity_list.GetMob(mp->npcid);

	if (vendor == 0 || !vendor->IsNPC() || vendor->GetClass() != MERCHANT)
		return;

	//you have to be somewhat close to them to be properly using them
	if (DistanceSquared(m_Position, vendor->GetPosition()) > USE_NPC_RANGE2)
		return;

	uint32 price = 0;
	uint32 itemid = GetItemIDAt(mp->itemslot);
	if (itemid == 0)
		return;
	const EQEmu::ItemData* item = database.GetItem(itemid);
	EQEmu::ItemInstance* inst = GetInv().GetItem(mp->itemslot);
	if (!item || !inst) {
		Message(13, "You seemed to have misplaced that item..");
		return;
	}
	if (mp->quantity > 1)
	{
		if ((inst->GetCharges() < 0) || (mp->quantity > (uint32)inst->GetCharges()))
			return;
	}

	if (!item->NoDrop) {
		//Message(13,"%s tells you, 'LOL NOPE'", vendor->GetName());
		return;
	}

	uint32 cost_quantity = mp->quantity;
	if (inst->IsCharged())
		uint32 cost_quantity = 1;

	uint32 i;

	if (RuleB(Merchant, UsePriceMod)) {
		for (i = 1; i <= cost_quantity; i++) {
			price = (uint32)((item->Price * i)*(RuleR(Merchant, BuyCostMod))*Client::CalcPriceMod(vendor, true) + 0.5); // need to round up, because client does it automatically when displaying price
			if (price > 4000000000) {
				cost_quantity = i;
				mp->quantity = i;
				break;
			}
		}
	}
	else {
		for (i = 1; i <= cost_quantity; i++) {
			price = (uint32)((item->Price * i)*(RuleR(Merchant, BuyCostMod)) + 0.5); // need to round up, because client does it automatically when displaying price
			if (price > 4000000000) {
				cost_quantity = i;
				mp->quantity = i;
				break;
			}
		}
	}

	AddMoneyToPP(price, false);

	if (inst->IsStackable() || inst->IsCharged())
	{
		unsigned int i_quan = inst->GetCharges();
		if (mp->quantity > i_quan || inst->IsCharged())
			mp->quantity = i_quan;
	}
	else
		mp->quantity = 1;

	if (RuleB(EventLog, RecordSellToMerchant))
		LogMerchant(this, vendor, mp->quantity, price, item, false);

	int charges = mp->quantity;
	//Hack workaround so usable items with 0 charges aren't simply deleted
	if (charges == 0 && item->ItemType != 11 && item->ItemType != 17 && item->ItemType != 19 && item->ItemType != 21)
		charges = 1;

	int freeslot = 0;
	if (charges > 0 && (freeslot = zone->SaveTempItem(vendor->CastToNPC()->MerchantType, vendor->GetNPCTypeID(), itemid, charges, true)) > 0) {
		EQEmu::ItemInstance* inst2 = inst->Clone();

		while (true) {
			if (inst2 == nullptr)
				break;

			if (RuleB(Merchant, UsePriceMod)) {
				inst2->SetPrice(item->Price*(RuleR(Merchant, SellCostMod))*item->SellRate*Client::CalcPriceMod(vendor, false));
			}
			else
				inst2->SetPrice(item->Price*(RuleR(Merchant, SellCostMod))*item->SellRate);
			inst2->SetMerchantSlot(freeslot);

			uint32 MerchantQuantity = zone->GetTempMerchantQuantity(vendor->GetNPCTypeID(), freeslot);

			if (inst2->IsStackable()) {
				inst2->SetCharges(MerchantQuantity);
			}
			inst2->SetMerchantCount(MerchantQuantity);

			SendItemPacket(freeslot - 1, inst2, ItemPacketMerchant);
			safe_delete(inst2);

			break;
		}
	}

	// start QS code
	if (RuleB(QueryServ, PlayerLogMerchantTransactions)) {
		auto qspack =
			new ServerPacket(ServerOP_QSPlayerLogMerchantTransactions,
				sizeof(QSMerchantLogTransaction_Struct) + sizeof(QSTransactionItems_Struct));
		QSMerchantLogTransaction_Struct* qsaudit = (QSMerchantLogTransaction_Struct*)qspack->pBuffer;

		qsaudit->zone_id = zone->GetZoneID();
		qsaudit->merchant_id = vendor->CastToNPC()->MerchantType;
		qsaudit->merchant_money.platinum = (price / 1000);
		qsaudit->merchant_money.gold = (price / 100) % 10;
		qsaudit->merchant_money.silver = (price / 10) % 10;
		qsaudit->merchant_money.copper = price % 10;
		qsaudit->merchant_count = 0;
		qsaudit->char_id = character_id;
		qsaudit->char_money.platinum = 0;
		qsaudit->char_money.gold = 0;
		qsaudit->char_money.silver = 0;
		qsaudit->char_money.copper = 0;
		qsaudit->char_count = 1;

		qsaudit->items[0].char_slot = mp->itemslot;
		qsaudit->items[0].item_id = itemid;
		qsaudit->items[0].charges = charges;
		qsaudit->items[0].aug_1 = m_inv[mp->itemslot]->GetAugmentItemID(1);
		qsaudit->items[0].aug_2 = m_inv[mp->itemslot]->GetAugmentItemID(2);
		qsaudit->items[0].aug_3 = m_inv[mp->itemslot]->GetAugmentItemID(3);
		qsaudit->items[0].aug_4 = m_inv[mp->itemslot]->GetAugmentItemID(4);
		qsaudit->items[0].aug_5 = m_inv[mp->itemslot]->GetAugmentItemID(5);

		if (worldserver.Connected()) { worldserver.SendPacket(qspack); }
		safe_delete(qspack);
	}
	// end QS code

	// Now remove the item from the player, this happens regardless of outcome
	if (!inst->IsStackable())
		this->DeleteItemInInventory(mp->itemslot, 0, false);
	else {
		// HACK: DeleteItemInInventory uses int8 for quantity type. There is no consistent use of types in code in this path so for now iteratively delete from inventory.
		if (mp->quantity > 255) {
			uint32 temp = mp->quantity;
			while (temp > 255 && temp != 0) {
				// Delete chunks of 255
				this->DeleteItemInInventory(mp->itemslot, 255, false);
				temp -= 255;
			}
			if (temp != 0) {
				// Delete remaining
				this->DeleteItemInInventory(mp->itemslot, temp, false);
			}
		}
		else {
			this->DeleteItemInInventory(mp->itemslot, mp->quantity, false);
		}
	}


	//This forces the price to show up correctly for charged items.
	if (inst->IsCharged())
		mp->quantity = 1;

	auto outapp = new EQApplicationPacket(OP_ShopPlayerSell, sizeof(Merchant_Purchase_Struct));
	Merchant_Purchase_Struct* mco = (Merchant_Purchase_Struct*)outapp->pBuffer;
	mco->npcid = vendor->GetID();
	mco->itemslot = mp->itemslot;
	mco->quantity = mp->quantity;
	mco->price = price;
	QueuePacket(outapp);
	safe_delete(outapp);
	SendMoneyUpdate();
	t1.start();
	Save(1);
	t1.stop();
	std::cout << "Save took: " << t1.getDuration() << std::endl;
	return;
}

void Client::Handle_OP_ShopRequest(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Merchant_Click_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_ShopRequest, size=%i, expected %i", app->size, sizeof(Merchant_Click_Struct));
		return;
	}

	Merchant_Click_Struct* mc = (Merchant_Click_Struct*)app->pBuffer;

	// Send back opcode OP_ShopRequest - tells client to open merchant window.
	//EQApplicationPacket* outapp = new EQApplicationPacket(OP_ShopRequest, sizeof(Merchant_Click_Struct));
	//Merchant_Click_Struct* mco=(Merchant_Click_Struct*)outapp->pBuffer;
	int merchantid = 0;
	Mob* tmp = entity_list.GetMob(mc->npcid);

	if (tmp == 0 || !tmp->IsNPC() || tmp->GetClass() != MERCHANT)
		return;

	//you have to be somewhat close to them to be properly using them
	if (DistanceSquared(m_Position, tmp->GetPosition()) > USE_NPC_RANGE2)
		return;

	merchantid = tmp->CastToNPC()->MerchantType;

	int action = 1;
	if (merchantid == 0) {
		auto outapp = new EQApplicationPacket(OP_ShopRequest, sizeof(Merchant_Click_Struct));
		Merchant_Click_Struct* mco = (Merchant_Click_Struct*)outapp->pBuffer;
		mco->npcid = mc->npcid;
		mco->playerid = 0;
		mco->command = 1;		//open...
		mco->rate = 1.0;
		QueuePacket(outapp);
		safe_delete(outapp);
		return;
	}
	if (tmp->IsEngaged()) {
		this->Message_StringID(0, MERCHANT_BUSY);
		action = 0;
	}
	if (GetFeigned() || IsInvisible())
	{
		Message(0, "You cannot use a merchant right now.");
		action = 0;
	}
	int primaryfaction = tmp->CastToNPC()->GetPrimaryFaction();
	int factionlvl = GetFactionLevel(CharacterID(), tmp->CastToNPC()->GetNPCTypeID(), GetRace(), GetClass(), GetDeity(), primaryfaction, tmp);
	if (factionlvl >= 7) {
		MerchantRejectMessage(tmp, primaryfaction);
		action = 0;
	}

	if (tmp->Charmed())
		action = 0;

	// 1199 I don't have time for that now. etc
	if (!tmp->CastToNPC()->IsMerchantOpen()) {
		tmp->Say_StringID(zone->random.Int(1199, 1202));
		action = 0;
	}

	auto outapp = new EQApplicationPacket(OP_ShopRequest, sizeof(Merchant_Click_Struct));
	Merchant_Click_Struct* mco = (Merchant_Click_Struct*)outapp->pBuffer;

	mco->npcid = mc->npcid;
	mco->playerid = 0;
	mco->command = action; // Merchant command 0x01 = open
	if (RuleB(Merchant, UsePriceMod)) {
		mco->rate = 1 / ((RuleR(Merchant, BuyCostMod))*Client::CalcPriceMod(tmp, true)); // works
	}
	else
		mco->rate = 1 / (RuleR(Merchant, BuyCostMod));

	outapp->priority = 6;
	QueuePacket(outapp);
	safe_delete(outapp);

	if (action == 1)
		BulkSendMerchantInventory(merchantid, tmp->GetNPCTypeID());

	return;
}

void Client::Handle_OP_Sneak(const EQApplicationPacket *app)
{
	if (!HasSkill(EQEmu::skills::SkillSneak) && GetSkill(EQEmu::skills::SkillSneak) == 0) {
		return; //You cannot sneak if you do not have sneak
	}

	if (!p_timers.Expired(&database, pTimerSneak, false)) {
		Message(13, "Ability recovery time not yet met.");
		return;
	}
	p_timers.Start(pTimerSneak, SneakReuseTime - 1);

	bool was = sneaking;
	if (sneaking) {
		sneaking = false;
		hidden = false;
		improved_hidden = false;
		auto outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
		SpawnAppearance_Struct* sa_out = (SpawnAppearance_Struct*)outapp->pBuffer;
		sa_out->spawn_id = GetID();
		sa_out->type = 0x03;
		sa_out->parameter = 0;
		entity_list.QueueClients(this, outapp, true);
		safe_delete(outapp);
	}
	else {
		CheckIncreaseSkill(EQEmu::skills::SkillSneak, nullptr, 5);
	}
	float hidechance = ((GetSkill(EQEmu::skills::SkillSneak) / 300.0f) + .25) * 100;
	float random = zone->random.Real(0, 99);
	if (!was && random < hidechance) {
		sneaking = true;
	}
	auto outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
	SpawnAppearance_Struct* sa_out = (SpawnAppearance_Struct*)outapp->pBuffer;
	sa_out->spawn_id = GetID();
	sa_out->type = 0x0F;
	sa_out->parameter = sneaking;
	QueuePacket(outapp);
	safe_delete(outapp);
	if (GetClass() == ROGUE) {
		outapp = new EQApplicationPacket(OP_SimpleMessage, 12);
		SimpleMessage_Struct *msg = (SimpleMessage_Struct *)outapp->pBuffer;
		msg->color = 0x010E;
		if (sneaking) {
			msg->string_id = 347;
		}
		else {
			msg->string_id = 348;
		}
		FastQueuePacket(&outapp);
	}
	return;
}

void Client::Handle_OP_SpawnAppearance(const EQApplicationPacket *app)
{
	if (app->size != sizeof(SpawnAppearance_Struct)) {
		std::cout << "Wrong size on OP_SpawnAppearance. Got: " << app->size << ", Expected: " << sizeof(SpawnAppearance_Struct) << std::endl;
		return;
	}
	SpawnAppearance_Struct* sa = (SpawnAppearance_Struct*)app->pBuffer;

	if (sa->spawn_id != GetID())
		return;

	if (sa->type == AT_Invis) {
		if (sa->parameter != 0)
		{
			if (!HasSkill(EQEmu::skills::SkillHide) && GetSkill(EQEmu::skills::SkillHide) == 0)
			{
				if (ClientVersion() < EQEmu::versions::ClientVersion::SoF)
				{
					char *hack_str = nullptr;
					MakeAnyLenString(&hack_str, "Player sent OP_SpawnAppearance with AT_Invis: %i", sa->parameter);
					database.SetMQDetectionFlag(this->account_name, this->name, hack_str, zone->GetShortName());
					safe_delete_array(hack_str);
				}
			}
			return;
		}
		invisible = false;
		hidden = false;
		improved_hidden = false;
		entity_list.QueueClients(this, app, true);
		return;
	}
	else if (sa->type == AT_Anim) {
		if (IsAIControlled())
			return;

		if (sa->parameter == ANIM_STAND) {
			SetAppearance(eaStanding);
			playeraction = 0;
			SetFeigned(false);
			BindWound(this, false, true);
			camp_timer.Disable();
		}
		else if (sa->parameter == ANIM_SIT) {
			SetAppearance(eaSitting);
			playeraction = 1;
			if (!UseBardSpellLogic())
				InterruptSpell();
			SetFeigned(false);
			BindWound(this, false, true);
		}
		else if (sa->parameter == ANIM_CROUCH) {
			if (!UseBardSpellLogic())
				InterruptSpell();
			SetAppearance(eaCrouching);
			playeraction = 2;
			SetFeigned(false);
		}
		else if (sa->parameter == ANIM_DEATH) { // feign death too
			SetAppearance(eaDead);
			playeraction = 3;
			InterruptSpell();
		}
		else if (sa->parameter == ANIM_LOOT) {
			SetAppearance(eaLooting);
			playeraction = 4;
			SetFeigned(false);
		}

		else {
			Log(Logs::Detail, Logs::Error, "Client %s :: unknown appearance %i", name, (int)sa->parameter);
			return;
		}

		entity_list.QueueClients(this, app, true);
	}
	else if (sa->type == AT_Anon) {
		if (!anon_toggle_timer.Check()) {
			return;
		}

		// For Anon/Roleplay
		if (sa->parameter == 1) { // Anon
			m_pp.anon = 1;
		}
		else if ((sa->parameter == 2) || (sa->parameter == 3)) { // This is Roleplay, or anon+rp
			m_pp.anon = 2;
		}
		else if (sa->parameter == 0) { // This is Non-Anon
			m_pp.anon = 0;
		}
		else {
			Log(Logs::Detail, Logs::Error, "Client %s :: unknown Anon/Roleplay Switch %i", name, (int)sa->parameter);
			return;
		}
		entity_list.QueueClients(this, app, true);
		UpdateWho();
	}
	else if ((sa->type == AT_HP) && (dead == 0)) {
		return;
	}
	else if (sa->type == AT_AFK) {
		if (afk_toggle_timer.Check()) {
			AFK = (sa->parameter == 1);
			entity_list.QueueClients(this, app, true);
		}
	}
	else if (sa->type == AT_Split) {
		m_pp.autosplit = (sa->parameter == 1);
	}
	else if (sa->type == AT_Sneak) {
		if (sneaking == 0)
			return;

		if (sa->parameter != 0)
		{
			if (!HasSkill(EQEmu::skills::SkillSneak))
			{
				char *hack_str = nullptr;
				MakeAnyLenString(&hack_str, "Player sent OP_SpawnAppearance with AT_Sneak: %i", sa->parameter);
				database.SetMQDetectionFlag(this->account_name, this->name, hack_str, zone->GetShortName());
				safe_delete_array(hack_str);
			}
			return;
		}
		sneaking = 0;
		entity_list.QueueClients(this, app, true);
	}
	else if (sa->type == AT_Size)
	{
		char *hack_str = nullptr;
		MakeAnyLenString(&hack_str, "Player sent OP_SpawnAppearance with AT_Size: %i", sa->parameter);
		database.SetMQDetectionFlag(this->account_name, this->name, hack_str, zone->GetShortName());
		safe_delete_array(hack_str);
	}
	else if (sa->type == AT_Light)	// client emitting light (lightstone, shiny shield)
	{
		//don't do anything with this
	}
	else if (sa->type == AT_Levitate)
	{
		// don't do anything with this, we tell the client when it's
		// levitating, not the other way around
	}
	else if (sa->type == AT_ShowHelm)
	{
		if (helm_toggle_timer.Check()) {
			m_pp.showhelm = (sa->parameter == 1);
			entity_list.QueueClients(this, app, true);
		}
	}
	else {
		std::cout << "Unknown SpawnAppearance type: 0x" << std::hex << std::setw(4) << std::setfill('0') << sa->type << std::dec
			<< " value: 0x" << std::hex << std::setw(8) << std::setfill('0') << sa->parameter << std::dec << std::endl;
	}
	return;
}

void Client::Handle_OP_Split(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Split_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_Split, size=%i, expected %i", app->size, sizeof(Split_Struct));
		return;
	}
	// The client removes the money on its own, but we have to
	// update our state anyway, and make sure they had enough to begin
	// with.
	Split_Struct *split = (Split_Struct *)app->pBuffer;
	//Per the note above, Im not exactly sure what to do on error
	//to notify the client of the error...
	if (!isgrouped) {
		Message(13, "You can not split money if your not in a group.");
		return;
	}
	Group *cgroup = GetGroup();
	if (cgroup == nullptr) {
		//invalid group, not sure if we should say more...
		Message(13, "You can not split money if your not in a group.");
		return;
	}

	if (!TakeMoneyFromPP(static_cast<uint64>(split->copper) +
		10 * static_cast<uint64>(split->silver) +
		100 * static_cast<uint64>(split->gold) +
		1000 * static_cast<uint64>(split->platinum))) {
		Message(13, "You do not have enough money to do that split.");
		return;
	}
	cgroup->SplitMoney(split->copper, split->silver, split->gold, split->platinum);

	return;

}

void Client::Handle_OP_Surname(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Surname_Struct))
	{
		Log(Logs::General, Logs::None, "Size mismatch in Surname expected %i got %i", sizeof(Surname_Struct), app->size);
		return;
	}

	if (!p_timers.Expired(&database, pTimerSurnameChange, false) && !GetGM())
	{
		Message(15, "You may only change surnames once every 7 days, your /surname is currently on cooldown.");
		return;
	}

	if (GetLevel() < 20)
	{
		Message_StringID(15, SURNAME_LEVEL);
		return;
	}

	Surname_Struct* surname = (Surname_Struct*)app->pBuffer;

	char *c = nullptr;
	bool first = true;
	for (c = surname->lastname; *c; c++)
	{
		if (first)
		{
			*c = toupper(*c);
			first = false;
		}
		else
		{
			*c = tolower(*c);
		}
	}

	if (strlen(surname->lastname) >= 20) {
		Message_StringID(15, SURNAME_TOO_LONG);
		return;
	}

	if (!database.CheckNameFilter(surname->lastname, true))
	{
		Message_StringID(15, SURNAME_REJECTED);
		return;
	}

	ChangeLastName(surname->lastname);
	p_timers.Start(pTimerSurnameChange, 604800);

	EQApplicationPacket* outapp = app->Copy();
	outapp = app->Copy();
	surname = (Surname_Struct*)outapp->pBuffer;
	surname->unknown0064 = 1;
	FastQueuePacket(&outapp);
	return;
}

void Client::Handle_OP_SwapSpell(const EQApplicationPacket *app)
{

	if (app->size != sizeof(SwapSpell_Struct)) {
		std::cout << "Wrong size on OP_SwapSpell. Got: " << app->size << ", Expected: " << sizeof(SwapSpell_Struct) << std::endl;
		return;
	}
	const SwapSpell_Struct* swapspell = (const SwapSpell_Struct*)app->pBuffer;
	int swapspelltemp;

	if (swapspell->from_slot < 0 || swapspell->from_slot > MAX_PP_SPELLBOOK || swapspell->to_slot < 0 || swapspell->to_slot > MAX_PP_SPELLBOOK)
		return;

	swapspelltemp = m_pp.spell_book[swapspell->from_slot];
	if (swapspelltemp < 0) {
		return;
	}
	m_pp.spell_book[swapspell->from_slot] = m_pp.spell_book[swapspell->to_slot];
	m_pp.spell_book[swapspell->to_slot] = swapspelltemp;

	/* Save Spell Swaps */
	if (!database.SaveCharacterSpell(this->CharacterID(), m_pp.spell_book[swapspell->from_slot], swapspell->from_slot)) {
		database.DeleteCharacterSpell(this->CharacterID(), m_pp.spell_book[swapspell->from_slot], swapspell->from_slot);
	}
	if (!database.SaveCharacterSpell(this->CharacterID(), swapspelltemp, swapspell->to_slot)) {
		database.DeleteCharacterSpell(this->CharacterID(), swapspelltemp, swapspell->to_slot);
	}

	QueuePacket(app);
	return;
}

void Client::Handle_OP_TargetCommand(const EQApplicationPacket *app)
{
	if (app->size != sizeof(ClientTarget_Struct)) {
		Log(Logs::General, Logs::Error, "OP size error: OP_TargetMouse expected:%i got:%i", sizeof(ClientTarget_Struct), app->size);
		return;
	}

	if (GetTarget())
	{
		GetTarget()->IsTargeted(-1);
	}

	// Locate and cache new target
	ClientTarget_Struct* ct = (ClientTarget_Struct*)app->pBuffer;
	pClientSideTarget = ct->new_target;
	if (!IsAIControlled())
	{
		Mob *nt = entity_list.GetMob(ct->new_target);
		if (nt)
		{
			SetTarget(nt);
			bool inspect_buffs = false;
			// rank 1 gives you ability to see NPC buffs in target window (SoD+)
			if (nt->IsNPC()) {
				if (IsRaidGrouped()) {
					Raid *raid = GetRaid();
					if (raid) {
						uint32 gid = raid->GetGroup(this);
						if (gid < 12 && raid->GroupCount(gid) > 2)
							inspect_buffs = raid->GetLeadershipAA(groupAAInspectBuffs, gid);
					}
				}
				else {
					Group *group = GetGroup();
					if (group && group->GroupCount() > 2)
						inspect_buffs = group->GetLeadershipAA(groupAAInspectBuffs);
				}
			}
			if (GetGM() || RuleB(Spells, AlwaysSendTargetsBuffs) || nt == this || inspect_buffs || (nt->IsClient() && !nt->CastToClient()->GetPVP()) ||
				(nt->IsPet() && nt->GetOwner() && nt->GetOwner()->IsClient() && !nt->GetOwner()->CastToClient()->GetPVP()) ||
#ifdef BOTS
				(nt->IsBot() && nt->GetOwner() && nt->GetOwner()->IsClient() && !nt->GetOwner()->CastToClient()->GetPVP()) || // TODO: bot pets
#endif
				(nt->IsMerc() && nt->GetOwner() && nt->GetOwner()->IsClient() && !nt->GetOwner()->CastToClient()->GetPVP()))
			{
				nt->SendBuffsToClient(this);
			}
		}
		else
		{
			SetTarget(nullptr);
			SetHoTT(0);
			UpdateXTargetType(TargetsTarget, nullptr);

			Group *g = GetGroup();

			if (g && g->HasRole(this, RoleAssist))
				g->SetGroupAssistTarget(0);

			if (g && g->HasRole(this, RoleTank))
				g->SetGroupTankTarget(0);

			if (g && g->HasRole(this, RolePuller))
				g->SetGroupPullerTarget(0);

			return;
		}
	}
	else
	{
		SetTarget(nullptr);
		SetHoTT(0);
		UpdateXTargetType(TargetsTarget, nullptr);
		return;
	}

	// HoTT
	if (GetTarget() && GetTarget()->GetTarget())
	{
		SetHoTT(GetTarget()->GetTarget()->GetID());
		UpdateXTargetType(TargetsTarget, GetTarget()->GetTarget());
	}
	else
	{
		SetHoTT(0);
		UpdateXTargetType(TargetsTarget, nullptr);
	}

	Group *g = GetGroup();

	if (g && g->HasRole(this, RoleAssist))
		g->SetGroupAssistTarget(GetTarget());

	if (g && g->HasRole(this, RoleTank))
		g->SetGroupTankTarget(GetTarget());

	if (g && g->HasRole(this, RolePuller))
		g->SetGroupPullerTarget(GetTarget());

	// For /target, send reject or success packet
	if (app->GetOpcode() == OP_TargetCommand) {
		if (GetTarget() && !GetTarget()->CastToMob()->IsInvisible(this) && (DistanceSquared(m_Position, GetTarget()->GetPosition()) <= TARGETING_RANGE*TARGETING_RANGE || GetGM())) {
			if (GetTarget()->GetBodyType() == BT_NoTarget2 || GetTarget()->GetBodyType() == BT_Special
				|| GetTarget()->GetBodyType() == BT_NoTarget)
			{
				//Targeting something we shouldn't with /target
				//but the client allows this without MQ so you don't flag it
				auto outapp = new EQApplicationPacket(OP_TargetReject, sizeof(TargetReject_Struct));
				outapp->pBuffer[0] = 0x2f;
				outapp->pBuffer[1] = 0x01;
				outapp->pBuffer[4] = 0x0d;
				if (GetTarget())
				{
					SetTarget(nullptr);
				}
				QueuePacket(outapp);
				safe_delete(outapp);
				return;
			}

			QueuePacket(app);
			EQApplicationPacket hp_app;
			GetTarget()->IsTargeted(1);
			GetTarget()->CreateHPPacket(&hp_app);
			QueuePacket(&hp_app, false);
		}
		else
		{
			auto outapp = new EQApplicationPacket(OP_TargetReject, sizeof(TargetReject_Struct));
			outapp->pBuffer[0] = 0x2f;
			outapp->pBuffer[1] = 0x01;
			outapp->pBuffer[4] = 0x0d;
			if (GetTarget())
			{
				SetTarget(nullptr);
			}
			QueuePacket(outapp);
			safe_delete(outapp);
		}
	}
	else
	{
		if (GetTarget())
		{
			if (GetGM())
			{
				GetTarget()->IsTargeted(1);
				return;
			}
			else if (RuleB(Character, AllowMQTarget))
			{
				GetTarget()->IsTargeted(1);
				return;
			}
			else if (IsAssistExempted())
			{
				GetTarget()->IsTargeted(1);
				SetAssistExemption(false);
				return;
			}
			else if (GetTarget()->IsClient())
			{
				//make sure this client is in our raid/group
				GetTarget()->IsTargeted(1);
				return;
			}
			else if (GetTarget()->GetBodyType() == BT_NoTarget2 || GetTarget()->GetBodyType() == BT_Special
				|| GetTarget()->GetBodyType() == BT_NoTarget)
			{
				char *hacker_str = nullptr;
				MakeAnyLenString(&hacker_str, "%s attempting to target something untargetable, %s bodytype: %i\n",
					GetName(), GetTarget()->GetName(), (int)GetTarget()->GetBodyType());
				database.SetMQDetectionFlag(AccountName(), GetName(), hacker_str, zone->GetShortName());
				safe_delete_array(hacker_str);
				SetTarget((Mob*)nullptr);
				return;
			}
			else if (IsPortExempted())
			{
				GetTarget()->IsTargeted(1);
				return;
			}
			else if (IsSenseExempted())
			{
				GetTarget()->IsTargeted(1);
				SetSenseExemption(false);
				return;
			}
			else if (IsXTarget(GetTarget()))
			{
				GetTarget()->IsTargeted(1);
				return;
			}
			else if (GetTarget()->IsPetOwnerClient())
			{
				GetTarget()->IsTargeted(1);
				return;
			}
			else if (GetBindSightTarget())
			{
				if (DistanceSquared(GetBindSightTarget()->GetPosition(), GetTarget()->GetPosition()) > (zone->newzone_data.maxclip*zone->newzone_data.maxclip))
				{
					if (DistanceSquared(m_Position, GetTarget()->GetPosition()) > (zone->newzone_data.maxclip*zone->newzone_data.maxclip))
					{
						char *hacker_str = nullptr;
						MakeAnyLenString(&hacker_str, "%s attempting to target something beyond the clip plane of %.2f units,"
							" from (%.2f, %.2f, %.2f) to %s (%.2f, %.2f, %.2f)", GetName(),
							(zone->newzone_data.maxclip*zone->newzone_data.maxclip),
							GetX(), GetY(), GetZ(), GetTarget()->GetName(), GetTarget()->GetX(), GetTarget()->GetY(), GetTarget()->GetZ());
						database.SetMQDetectionFlag(AccountName(), GetName(), hacker_str, zone->GetShortName());
						safe_delete_array(hacker_str);
						SetTarget(nullptr);
						return;
					}
				}
			}
			else if (DistanceSquared(m_Position, GetTarget()->GetPosition()) > (zone->newzone_data.maxclip*zone->newzone_data.maxclip))
			{
				char *hacker_str = nullptr;
				MakeAnyLenString(&hacker_str, "%s attempting to target something beyond the clip plane of %.2f units,"
					" from (%.2f, %.2f, %.2f) to %s (%.2f, %.2f, %.2f)", GetName(),
					(zone->newzone_data.maxclip*zone->newzone_data.maxclip),
					GetX(), GetY(), GetZ(), GetTarget()->GetName(), GetTarget()->GetX(), GetTarget()->GetY(), GetTarget()->GetZ());
				database.SetMQDetectionFlag(AccountName(), GetName(), hacker_str, zone->GetShortName());
				safe_delete_array(hacker_str);
				SetTarget(nullptr);
				return;
			}

			GetTarget()->IsTargeted(1);
		}
	}
	return;
}

void Client::Handle_OP_TargetMouse(const EQApplicationPacket *app)
{
	Handle_OP_TargetCommand(app);
}

void Client::Handle_OP_TaskHistoryRequest(const EQApplicationPacket *app)
{

	if (app->size != sizeof(TaskHistoryRequest_Struct)) {
		Log(Logs::General, Logs::None, "Size mismatch in OP_TaskHistoryRequest expected %i got %i",
			sizeof(TaskHistoryRequest_Struct), app->size);
		DumpPacket(app);
		return;
	}
	TaskHistoryRequest_Struct *ths = (TaskHistoryRequest_Struct*)app->pBuffer;

	if (RuleB(TaskSystem, EnableTaskSystem) && taskstate)
		taskstate->SendTaskHistory(this, ths->TaskIndex);
}

void Client::Handle_OP_Taunt(const EQApplicationPacket *app)
{
	if (app->size != sizeof(ClientTarget_Struct)) {
		std::cout << "Wrong size on OP_Taunt. Got: " << app->size << ", Expected: " << sizeof(ClientTarget_Struct) << std::endl;
		return;
	}

	if (!p_timers.Expired(&database, pTimerTaunt, false)) {
		Message(13, "Ability recovery time not yet met.");
		return;
	}
	p_timers.Start(pTimerTaunt, TauntReuseTime - 1);

	if (GetTarget() == nullptr || !GetTarget()->IsNPC())
		return;

	Taunt(GetTarget()->CastToNPC(), false);
	return;
}

void Client::Handle_OP_TestBuff(const EQApplicationPacket *app)
{
	return;
}

void Client::Handle_OP_TGB(const EQApplicationPacket *app)
{
	OPTGB(app);
	return;
}

void Client::Handle_OP_Track(const EQApplicationPacket *app)
{
	if (GetClass() != RANGER && GetClass() != DRUID && GetClass() != BARD)
		return;

	if (GetSkill(EQEmu::skills::SkillTracking) == 0)
		SetSkill(EQEmu::skills::SkillTracking, 1);
	else
		CheckIncreaseSkill(EQEmu::skills::SkillTracking, nullptr, 15);

	if (!entity_list.MakeTrackPacket(this))
		Log(Logs::General, Logs::Error, "Unable to generate OP_Track packet requested by client.");

	return;
}

void Client::Handle_OP_TrackTarget(const EQApplicationPacket *app)
{
	int PlayerClass = GetClass();

	if ((PlayerClass != RANGER) && (PlayerClass != DRUID) && (PlayerClass != BARD))
		return;

	if (app->size != sizeof(TrackTarget_Struct))
	{
		Log(Logs::General, Logs::Error, "Invalid size for OP_TrackTarget: Expected: %i, Got: %i",
			sizeof(TrackTarget_Struct), app->size);
		return;
	}

	TrackTarget_Struct *tts = (TrackTarget_Struct*)app->pBuffer;

	TrackingID = tts->EntityID;
}

void Client::Handle_OP_TrackUnknown(const EQApplicationPacket *app)
{
	// size 0 send right after OP_Track
	return;
}

void Client::Handle_OP_TradeAcceptClick(const EQApplicationPacket *app)
{
	Mob* with = trade->With();
	trade->state = TradeAccepted;

	if (with && with->IsClient()) {
		//finish trade...
		// Have both accepted?
		Client* other = with->CastToClient();
		other->QueuePacket(app);

		if (other->trade->state == trade->state) {
			other->trade->state = TradeCompleting;
			trade->state = TradeCompleting;

			if (CheckTradeLoreConflict(other) || other->CheckTradeLoreConflict(this)) {
				Message_StringID(13, TRADE_CANCEL_LORE);
				other->Message_StringID(13, TRADE_CANCEL_LORE);
				this->FinishTrade(this);
				other->FinishTrade(other);
				other->trade->Reset();
				trade->Reset();
			}
			else if (CheckTradeNonDroppable()) {
				Message_StringID(13, TRADE_HAS_BEEN_CANCELLED);
				other->Message_StringID(13, TRADE_HAS_BEEN_CANCELLED);
				this->FinishTrade(this);
				other->FinishTrade(other);
				other->trade->Reset();
				trade->Reset();
				Message(15, "Hacking activity detected in trade transaction.");
				// TODO: query (this) as a hacker
			}
			else if (other->CheckTradeNonDroppable()) {
				Message_StringID(13, TRADE_HAS_BEEN_CANCELLED);
				other->Message_StringID(13, TRADE_HAS_BEEN_CANCELLED);
				this->FinishTrade(this);
				other->FinishTrade(other);
				other->trade->Reset();
				trade->Reset();
				other->Message(15, "Hacking activity detected in trade transaction.");
				// TODO: query (other) as a hacker
			}
			else {
				// Audit trade to database for both trade streams
				other->trade->LogTrade();
				trade->LogTrade();

				// start QS code
				if (RuleB(QueryServ, PlayerLogTrades)) {
					QSPlayerLogTrade_Struct event_entry;
					std::list<void*> event_details;

					memset(&event_entry, 0, sizeof(QSPlayerLogTrade_Struct));

					// Perform actual trade
					this->FinishTrade(other, true, &event_entry, &event_details);
					other->FinishTrade(this, false, &event_entry, &event_details);

					event_entry._detail_count = event_details.size();

					auto qs_pack = new ServerPacket(
						ServerOP_QSPlayerLogTrades,
						sizeof(QSPlayerLogTrade_Struct) +
						(sizeof(QSTradeItems_Struct) * event_entry._detail_count));
					QSPlayerLogTrade_Struct* qs_buf = (QSPlayerLogTrade_Struct*)qs_pack->pBuffer;

					memcpy(qs_buf, &event_entry, sizeof(QSPlayerLogTrade_Struct));

					int offset = 0;

					for (auto iter = event_details.begin(); iter != event_details.end();
					++iter, ++offset) {
						QSTradeItems_Struct* detail = reinterpret_cast<QSTradeItems_Struct*>(*iter);
						qs_buf->items[offset] = *detail;
						safe_delete(detail);
					}

					event_details.clear();

					if (worldserver.Connected())
						worldserver.SendPacket(qs_pack);

					safe_delete(qs_pack);
					// end QS code
				}
				else {
					this->FinishTrade(other);
					other->FinishTrade(this);
				}

				other->trade->Reset();
				trade->Reset();
			}
			// All done
			auto outapp = new EQApplicationPacket(OP_FinishTrade, 0);
			other->QueuePacket(outapp);
			this->FastQueuePacket(&outapp);
		}
	}
	// Trading with a Mob object that is not a Client.
	else if (with) {
		auto outapp = new EQApplicationPacket(OP_FinishTrade, 0);
		QueuePacket(outapp);
		safe_delete(outapp);
		if (with->IsNPC()) {
			// Audit trade to database for player trade stream
			if (RuleB(QueryServ, PlayerLogHandins)) {
				QSPlayerLogHandin_Struct event_entry;
				std::list<void*> event_details;

				memset(&event_entry, 0, sizeof(QSPlayerLogHandin_Struct));

				FinishTrade(with->CastToNPC(), false, &event_entry, &event_details);

				event_entry._detail_count = event_details.size();

				auto qs_pack =
					new ServerPacket(ServerOP_QSPlayerLogHandins,
						sizeof(QSPlayerLogHandin_Struct) +
						(sizeof(QSHandinItems_Struct) * event_entry._detail_count));
				QSPlayerLogHandin_Struct* qs_buf = (QSPlayerLogHandin_Struct*)qs_pack->pBuffer;

				memcpy(qs_buf, &event_entry, sizeof(QSPlayerLogHandin_Struct));

				int offset = 0;

				for (auto iter = event_details.begin(); iter != event_details.end(); ++iter, ++offset) {
					QSHandinItems_Struct* detail = reinterpret_cast<QSHandinItems_Struct*>(*iter);
					qs_buf->items[offset] = *detail;
					safe_delete(detail);
				}

				event_details.clear();

				if (worldserver.Connected())
					worldserver.SendPacket(qs_pack);

				safe_delete(qs_pack);
			}
			else {
				FinishTrade(with->CastToNPC());
			}
		}
#ifdef BOTS
		// TODO: Log Bot trades
		else if (with->IsBot())
			with->CastToBot()->FinishTrade(this, Bot::BotTradeClientNormal);
#endif
		trade->Reset();
	}


	return;
}

void Client::Handle_OP_TradeBusy(const EQApplicationPacket *app)
{
	if (app->size != sizeof(TradeBusy_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_TradeBusy, size=%i, expected %i", app->size, sizeof(TradeBusy_Struct));
		return;
	}
	// Trade request recipient is cancelling the trade due to being busy
	// Trade requester gets message "I'm busy right now"
	// Send busy message on to trade initiator if client
	TradeBusy_Struct* msg = (TradeBusy_Struct*)app->pBuffer;
	Mob* tradee = entity_list.GetMob(msg->to_mob_id);

	if (tradee && tradee->IsClient()) {
		tradee->CastToClient()->QueuePacket(app);
	}
	return;
}

void Client::Handle_OP_Trader(const EQApplicationPacket *app)
{
	// Bazaar Trader:
	//
	// SoF sends 1 or more unhandled OP_Trader packets of size 96 when a trade has completed.
	// I don't know what they are for (yet), but it doesn't seem to matter that we ignore them.


	uint32 max_items = 80;

	/*
	if (GetClientVersion() >= EQClientRoF)
	max_items = 200;
	*/

	//Show Items
	if (app->size == sizeof(Trader_ShowItems_Struct))
	{
		Trader_ShowItems_Struct* sis = (Trader_ShowItems_Struct*)app->pBuffer;

		switch (sis->Code)
		{
		case BazaarTrader_EndTraderMode: {
			Trader_EndTrader();
			Log(Logs::Detail, Logs::Trading, "Client::Handle_OP_Trader: End Trader Session");
			break;
		}
		case BazaarTrader_EndTransaction: {

			Client* c = entity_list.GetClientByID(sis->TraderID);
			if (c)
			{
				c->WithCustomer(0);
				Log(Logs::Detail, Logs::Trading, "Client::Handle_OP_Trader: End Transaction");
			}
			else
				Log(Logs::Detail, Logs::Trading, "Client::Handle_OP_Trader: Null Client Pointer");

			break;
		}
		case BazaarTrader_ShowItems: {
			Trader_ShowItems();
			Log(Logs::Detail, Logs::Trading, "Client::Handle_OP_Trader: Show Trader Items");
			break;
		}
		default: {
			Log(Logs::Detail, Logs::Trading, "Unhandled action code in OP_Trader ShowItems_Struct");
			break;
		}
		}
	}
	else if (app->size == sizeof(ClickTrader_Struct))
	{
		if (Buyer) {
			Trader_EndTrader();
			Message(13, "You cannot be a Trader and Buyer at the same time.");
			return;
		}

		ClickTrader_Struct* ints = (ClickTrader_Struct*)app->pBuffer;

		if (ints->Code == BazaarTrader_StartTraderMode)
		{
			GetItems_Struct* gis = GetTraderItems();

			Log(Logs::Detail, Logs::Trading, "Client::Handle_OP_Trader: Start Trader Mode");
			// Verify there are no NODROP or items with a zero price
			bool TradeItemsValid = true;

			for (uint32 i = 0; i < max_items; i++) {

				if (gis->Items[i] == 0) break;

				if (ints->ItemCost[i] == 0) {
					Message(13, "Item in Trader Satchel with no price. Unable to start trader mode");
					TradeItemsValid = false;
					break;
				}
				const EQEmu::ItemData *Item = database.GetItem(gis->Items[i]);

				if (!Item) {
					Message(13, "Unexpected error. Unable to start trader mode");
					TradeItemsValid = false;
					break;
				}

				if (Item->NoDrop == 0) {
					Message(13, "NODROP Item in Trader Satchel. Unable to start trader mode");
					TradeItemsValid = false;
					break;
				}
			}

			if (!TradeItemsValid) {
				Trader_EndTrader();
				return;
			}

			for (uint32 i = 0; i < max_items; i++) {
				if (database.GetItem(gis->Items[i])) {
					database.SaveTraderItem(this->CharacterID(), gis->Items[i], gis->SerialNumber[i],
						gis->Charges[i], ints->ItemCost[i], i);

					auto inst = FindTraderItemBySerialNumber(gis->SerialNumber[i]);
					if (inst)
						inst->SetPrice(ints->ItemCost[i]);
				}
				else {
					//return; //sony doesnt memset so assume done on first bad item
					break;
				}

			}
			safe_delete(gis);

			this->Trader_StartTrader();

			// This refreshes the Trader window to display the End Trader button
			if (ClientVersion() >= EQEmu::versions::ClientVersion::RoF)
			{
				auto outapp = new EQApplicationPacket(OP_Trader, sizeof(TraderStatus_Struct));
				TraderStatus_Struct* tss = (TraderStatus_Struct*)outapp->pBuffer;
				tss->Code = BazaarTrader_StartTraderMode2;
				QueuePacket(outapp);
				safe_delete(outapp);
			}
		}
		else {
			Log(Logs::Detail, Logs::Trading, "Client::Handle_OP_Trader: Unknown TraderStruct code of: %i\n",
				ints->Code);

			Log(Logs::General, Logs::Error, "Unknown TraderStruct code of: %i\n", ints->Code);
		}
	}
	else if (app->size == sizeof(TraderStatus_Struct))
	{
		TraderStatus_Struct* tss = (TraderStatus_Struct*)app->pBuffer;

		Log(Logs::Detail, Logs::Trading, "Client::Handle_OP_Trader: Trader Status Code: %d", tss->Code);

		switch (tss->Code)
		{
		case BazaarTrader_EndTraderMode: {
			Trader_EndTrader();
			Log(Logs::Detail, Logs::Trading, "Client::Handle_OP_Trader: End Trader Session");
			break;
		}
		case BazaarTrader_ShowItems: {
			Trader_ShowItems();
			Log(Logs::Detail, Logs::Trading, "Client::Handle_OP_Trader: Show Trader Items");
			break;
		}
		default: {
			Log(Logs::Detail, Logs::Trading, "Unhandled action code in OP_Trader ShowItems_Struct");
			break;
		}
		}


	}
	else if (app->size == sizeof(TraderPriceUpdate_Struct))
	{
		Log(Logs::Detail, Logs::Trading, "Client::Handle_OP_Trader: Trader Price Update");
		HandleTraderPriceUpdate(app);
	}
	else {
		Log(Logs::Detail, Logs::Trading, "Unknown size for OP_Trader: %i\n", app->size);
		Log(Logs::General, Logs::Error, "Unknown size for OP_Trader: %i\n", app->size);
		DumpPacket(app);
		return;
	}

	return;
}

void Client::Handle_OP_TraderBuy(const EQApplicationPacket *app)
{
	// Bazaar Trader:
	//
	// Client has elected to buy an item from a Trader
	//
	if (app->size != sizeof(TraderBuy_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_TraderBuy, size=%i, expected %i", app->size, sizeof(TraderBuy_Struct));
		return;
	}

	TraderBuy_Struct* tbs = (TraderBuy_Struct*)app->pBuffer;

	if (Client* Trader = entity_list.GetClientByID(tbs->TraderID)) {
		BuyTraderItem(tbs, Trader, app);
		Log(Logs::Detail, Logs::Trading, "Client::Handle_OP_TraderBuy: Buy Trader Item ");
	}
	else {
		Log(Logs::Detail, Logs::Trading, "Client::Handle_OP_TraderBuy: Null Client Pointer");
	}


	return;
}

void Client::Handle_OP_TradeRequest(const EQApplicationPacket *app)
{
	if (app->size != sizeof(TradeRequest_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_TradeRequest, size=%i, expected %i", app->size, sizeof(TradeRequest_Struct));
		return;
	}
	// Client requesting a trade session from an npc/client
	// Trade session not started until OP_TradeRequestAck is sent

	CommonBreakInvisible();

	// Pass trade request on to recipient
	TradeRequest_Struct* msg = (TradeRequest_Struct*)app->pBuffer;
	Mob* tradee = entity_list.GetMob(msg->to_mob_id);

	if (tradee && tradee->IsClient()) {
		tradee->CastToClient()->QueuePacket(app);
	}
#ifndef BOTS
	else if (tradee && tradee->IsNPC()) {
#else
	else if (tradee && (tradee->IsNPC() || tradee->IsBot())) {
#endif
		//npcs always accept
		trade->Start(msg->to_mob_id);

		auto outapp = new EQApplicationPacket(OP_TradeRequestAck, sizeof(TradeRequest_Struct));
		TradeRequest_Struct* acc = (TradeRequest_Struct*)outapp->pBuffer;
		acc->from_mob_id = msg->to_mob_id;
		acc->to_mob_id = msg->from_mob_id;
		FastQueuePacket(&outapp);
		safe_delete(outapp);
	}
	return;
	}

void Client::Handle_OP_TradeRequestAck(const EQApplicationPacket *app)
{
	if (app->size != sizeof(TradeRequest_Struct)) {
		Log(Logs::General, Logs::Error, "Wrong size: OP_TradeRequestAck, size=%i, expected %i", app->size, sizeof(TradeRequest_Struct));
		return;
	}
	// Trade request recipient is acknowledging they are able to trade
	// After this, the trade session has officially started
	// Send ack on to trade initiator if client
	TradeRequest_Struct* msg = (TradeRequest_Struct*)app->pBuffer;
	Mob* tradee = entity_list.GetMob(msg->to_mob_id);

	if (tradee && tradee->IsClient()) {
		trade->Start(msg->to_mob_id);
		tradee->CastToClient()->QueuePacket(app);
	}
	return;
}

void Client::Handle_OP_TraderShop(const EQApplicationPacket *app)
{
	// Bazaar Trader:

	if (app->size == sizeof(TraderClick_Struct))
	{

		TraderClick_Struct* tcs = (TraderClick_Struct*)app->pBuffer;

		Log(Logs::Detail, Logs::Trading, "Handle_OP_TraderShop: TraderClick_Struct TraderID %d, Code %d, Unknown008 %d, Approval %d",
			tcs->TraderID, tcs->Code, tcs->Unknown008, tcs->Approval);

		if (tcs->Code == BazaarWelcome)
		{
			Log(Logs::Detail, Logs::Trading, "Client::Handle_OP_TraderShop: Sent Bazaar Welcome Info");
			SendBazaarWelcome();
		}
		else
		{
			// This is when a potential purchaser right clicks on this client who is in Trader mode to
			// browse their goods.
			auto outapp = new EQApplicationPacket(OP_TraderShop, sizeof(TraderClick_Struct));

			TraderClick_Struct* outtcs = (TraderClick_Struct*)outapp->pBuffer;

			Client* Trader = entity_list.GetClientByID(tcs->TraderID);

			if (Trader)
			{
				outtcs->Approval = Trader->WithCustomer(GetID());
				Log(Logs::Detail, Logs::Trading, "Client::Handle_OP_TraderShop: Shop Request (%s) to (%s) with Approval: %d", GetCleanName(), Trader->GetCleanName(), outtcs->Approval);
			}
			else {
				Log(Logs::Detail, Logs::Trading, "Client::Handle_OP_TraderShop: entity_list.GetClientByID(tcs->traderid)"
					" returned a nullptr pointer");
				safe_delete(outapp);
				return;
			}

			outtcs->TraderID = tcs->TraderID;

			outtcs->Unknown008 = 0x3f800000;

			QueuePacket(outapp);


			if (outtcs->Approval) {
				this->BulkSendTraderInventory(Trader->CharacterID());
				Trader->Trader_CustomerBrowsing(this);
				TraderID = tcs->TraderID;
				Log(Logs::Detail, Logs::Trading, "Client::Handle_OP_TraderShop: Trader Inventory Sent");
			}
			else
			{
				Message_StringID(clientMessageYellow, TRADER_BUSY);
				Log(Logs::Detail, Logs::Trading, "Client::Handle_OP_TraderShop: Trader Busy");
			}

			safe_delete(outapp);
			return;
		}

	}
	else if (app->size == sizeof(BazaarWelcome_Struct))
	{
		// RoF+
		// Client requested Bazaar Welcome Info (Trader and Item Total Counts)
		SendBazaarWelcome();
		Log(Logs::Detail, Logs::Trading, "Client::Handle_OP_TraderShop: Sent Bazaar Welcome Info");
	}
	else if (app->size == sizeof(TraderBuy_Struct))
	{
		// RoF+
		// Customer has purchased an item from the Trader

		TraderBuy_Struct* tbs = (TraderBuy_Struct*)app->pBuffer;

		if (Client* Trader = entity_list.GetClientByID(tbs->TraderID))
		{
			BuyTraderItem(tbs, Trader, app);
			Log(Logs::Detail, Logs::Trading, "Handle_OP_TraderShop: Buy Action %d, Price %d, Trader %d, ItemID %d, Quantity %d, ItemName, %s",
				tbs->Action, tbs->Price, tbs->TraderID, tbs->ItemID, tbs->Quantity, tbs->ItemName);
		}
		else
		{
			Log(Logs::Detail, Logs::Trading, "OP_TraderShop: Null Client Pointer");
		}
	}
	else if (app->size == 4)
	{
		// RoF+
		// Customer has closed the trade window
		uint32 Command = *((uint32 *)app->pBuffer);

		if (Command == 4)
		{
			Client* c = entity_list.GetClientByID(TraderID);
			TraderID = 0;
			if (c)
			{
				c->WithCustomer(0);
				Log(Logs::Detail, Logs::Trading, "Client::Handle_OP_Trader: End Transaction - Code %d", Command);
			}
			else
			{
				Log(Logs::Detail, Logs::Trading, "Client::Handle_OP_Trader: Null Client Pointer for Trader - Code %d", Command);
			}
			EQApplicationPacket empty(OP_ShopEndConfirm);
			QueuePacket(&empty);
		}
		else
		{
			Log(Logs::Detail, Logs::Trading, "Client::Handle_OP_Trader: Unhandled Code %d", Command);
		}
	}
	else
	{
		Log(Logs::Detail, Logs::Trading, "Unknown size for OP_TraderShop: %i\n", app->size);
		Log(Logs::General, Logs::Error, "Unknown size for OP_TraderShop: %i\n", app->size);
		DumpPacket(app);
		return;
	}
}

void Client::Handle_OP_TradeSkillCombine(const EQApplicationPacket *app)
{
	if (app->size != sizeof(NewCombine_Struct)) {
		Log(Logs::General, Logs::Error, "Invalid size for NewCombine_Struct: Expected: %i, Got: %i",
			sizeof(NewCombine_Struct), app->size);
		return;
	}
	/*if (m_tradeskill_object == nullptr) {
	Message(13, "Error: Server is not aware of the tradeskill container you are attempting to use");
	return;
	}*/

	//fixed this to work for non-world objects

	// Delegate to tradeskill object to perform combine
	NewCombine_Struct* in_combine = (NewCombine_Struct*)app->pBuffer;
	Object::HandleCombine(this, in_combine, m_tradeskill_object);
	return;
}

void Client::Handle_OP_Translocate(const EQApplicationPacket *app)
{

	if (app->size != sizeof(Translocate_Struct)) {
		Log(Logs::General, Logs::None, "Size mismatch in OP_Translocate expected %i got %i", sizeof(Translocate_Struct), app->size);
		DumpPacket(app);
		return;
	}
	Translocate_Struct *its = (Translocate_Struct*)app->pBuffer;

	if (!PendingTranslocate)
		return;

	if ((RuleI(Spells, TranslocateTimeLimit) > 0) && (time(nullptr) > (TranslocateTime + RuleI(Spells, TranslocateTimeLimit)))) {
		Message(13, "You did not accept the Translocate within the required time limit.");
		PendingTranslocate = false;
		return;
	}

	if (its->Complete == 1) {

		int SpellID = PendingTranslocateData.spell_id;
		int i = parse->EventSpell(EVENT_SPELL_EFFECT_TRANSLOCATE_COMPLETE, nullptr, this, SpellID, 0);

		if (i == 0)
		{
			// If the spell has a translocate to bind effect, AND we are already in the zone the client
			// is bound in, use the GoToBind method. If we send OP_Translocate in this case, the client moves itself
			// to the bind coords it has from the PlayerProfile, but with the X and Y reversed. I suspect they are
			// reversed in the pp, and since spells like Gate are handled serverside, this has not mattered before.
			if (((SpellID == 1422) || (SpellID == 1334) || (SpellID == 3243)) &&
				(zone->GetZoneID() == PendingTranslocateData.zone_id &&
					zone->GetInstanceID() == PendingTranslocateData.instance_id))
			{
				PendingTranslocate = false;
				GoToBind();
				return;
			}

			////Was sending the packet back to initiate client zone...
			////but that could be abusable, so lets go through proper channels
			MovePC(PendingTranslocateData.zone_id, PendingTranslocateData.instance_id,
				PendingTranslocateData.x, PendingTranslocateData.y,
				PendingTranslocateData.z, PendingTranslocateData.heading, 0, ZoneSolicited);
		}
	}

	PendingTranslocate = false;
}

void Client::Handle_OP_TributeItem(const EQApplicationPacket *app)
{
	Log(Logs::Detail, Logs::Tribute, "Received OP_TributeItem of length %d", app->size);

	//player donates an item...
	if (app->size != sizeof(TributeItem_Struct))
		printf("Error in OP_TributeItem. Expected size of: %zu, but got: %i\n", sizeof(StartTribute_Struct), app->size);
	else {
		TributeItem_Struct* t = (TributeItem_Struct*)app->pBuffer;

		tribute_master_id = t->tribute_master_id;
		//make sure they are dealing with a valid tribute master
		Mob* tribmast = entity_list.GetMob(t->tribute_master_id);
		if (!tribmast || !tribmast->IsNPC() || tribmast->GetClass() != TRIBUTE_MASTER)
			return;
		if (DistanceSquared(m_Position, tribmast->GetPosition()) > USE_NPC_RANGE2)
			return;

		t->tribute_points = TributeItem(t->slot, t->quantity);

		Log(Logs::Detail, Logs::Tribute, "Sending tribute item reply with %d points", t->tribute_points);

		QueuePacket(app);
	}
	return;
}

void Client::Handle_OP_TributeMoney(const EQApplicationPacket *app)
{
	Log(Logs::Detail, Logs::Tribute, "Received OP_TributeMoney of length %d", app->size);

	//player donates money
	if (app->size != sizeof(TributeMoney_Struct))
		printf("Error in OP_TributeMoney. Expected size of: %zu, but got: %i\n", sizeof(StartTribute_Struct), app->size);
	else {
		TributeMoney_Struct* t = (TributeMoney_Struct*)app->pBuffer;

		tribute_master_id = t->tribute_master_id;
		//make sure they are dealing with a valid tribute master
		Mob* tribmast = entity_list.GetMob(t->tribute_master_id);
		if (!tribmast || !tribmast->IsNPC() || tribmast->GetClass() != TRIBUTE_MASTER)
			return;
		if (DistanceSquared(m_Position, tribmast->GetPosition()) > USE_NPC_RANGE2)
			return;

		t->tribute_points = TributeMoney(t->platinum);

		Log(Logs::Detail, Logs::Tribute, "Sending tribute money reply with %d points", t->tribute_points);

		QueuePacket(app);
	}
	return;
}

void Client::Handle_OP_TributeNPC(const EQApplicationPacket *app)
{
	Log(Logs::Detail, Logs::Tribute, "Received OP_TributeNPC of length %d", app->size);

	return;
}

void Client::Handle_OP_TributeToggle(const EQApplicationPacket *app)
{
	Log(Logs::Detail, Logs::Tribute, "Received OP_TributeToggle of length %d", app->size);

	if (app->size != sizeof(uint32))
		Log(Logs::General, Logs::Error, "Invalid size on OP_TributeToggle packet");
	else {
		uint32 *val = (uint32 *)app->pBuffer;
		ToggleTribute(*val ? true : false);
	}
	return;
}

void Client::Handle_OP_TributeUpdate(const EQApplicationPacket *app)
{
	Log(Logs::Detail, Logs::Tribute, "Received OP_TributeUpdate of length %d", app->size);

	//sent when the client changes their tribute settings...
	if (app->size != sizeof(TributeInfo_Struct))
		Log(Logs::General, Logs::Error, "Invalid size on OP_TributeUpdate packet");
	else {
		TributeInfo_Struct *t = (TributeInfo_Struct *)app->pBuffer;
		ChangeTributeSettings(t);
	}
	return;
}

void Client::Handle_OP_VetClaimRequest(const EQApplicationPacket *app)
{
	if (app->size < sizeof(VeteranClaim)) {
		Log(Logs::General, Logs::None,
			"OP_VetClaimRequest size lower than expected: got %u expected at least %u", app->size,
			sizeof(VeteranClaim));
		DumpPacket(app);
		return;
	}

	VeteranClaim *vcr = (VeteranClaim *)app->pBuffer;

	if (vcr->claim_id == 0xFFFFFFFF) { // request update packet
		SendRewards();
		return;
	}
	// try to claim something!
	auto vetapp = new EQApplicationPacket(OP_VetClaimReply, sizeof(VeteranClaim));
	VeteranClaim *cr = (VeteranClaim *)vetapp->pBuffer;
	strcpy(cr->name, GetName());
	cr->claim_id = vcr->claim_id;

	if (!TryReward(vcr->claim_id))
		cr->action = 1;
	else
		cr->action = 0;

	FastQueuePacket(&vetapp);
}

void Client::Handle_OP_VoiceMacroIn(const EQApplicationPacket *app)
{

	if (app->size != sizeof(VoiceMacroIn_Struct)) {

		Log(Logs::General, Logs::None, "Size mismatch in OP_VoiceMacroIn expected %i got %i",
			sizeof(VoiceMacroIn_Struct), app->size);

		DumpPacket(app);

		return;
	}

	if (!RuleB(Chat, EnableVoiceMacros)) return;

	VoiceMacroIn_Struct* vmi = (VoiceMacroIn_Struct*)app->pBuffer;

	VoiceMacroReceived(vmi->Type, vmi->Target, vmi->MacroNumber);

}

void Client::Handle_OP_WearChange(const EQApplicationPacket *app)
{
	if (app->size != sizeof(WearChange_Struct)) {
		std::cout << "Wrong size: OP_WearChange, size=" << app->size << ", expected " << sizeof(WearChange_Struct) << std::endl;
		return;
	}

	WearChange_Struct* wc = (WearChange_Struct*)app->pBuffer;
	if (wc->spawn_id != GetID())
		return;

	// Hero Forge ID needs to be fixed here as RoF2 appears to send an incorrect value.
	if (wc->hero_forge_model != 0 && wc->wear_slot_id >= 0 && wc->wear_slot_id < EQEmu::textures::weaponPrimary)
		wc->hero_forge_model = GetHerosForgeModel(wc->wear_slot_id);

	// we could maybe ignore this and just send our own from moveitem
	entity_list.QueueClients(this, app, true);
	return;
}

void Client::Handle_OP_WhoAllRequest(const EQApplicationPacket *app)
{
	if (app->size != sizeof(Who_All_Struct)) {
		std::cout << "Wrong size on OP_WhoAll. Got: " << app->size << ", Expected: " << sizeof(Who_All_Struct) << std::endl;
		return;
	}
	Who_All_Struct* whoall = (Who_All_Struct*)app->pBuffer;

	if (whoall->type == 0) // SoF only, for regular /who
		entity_list.ZoneWho(this, whoall);
	else
		WhoAll(whoall);
	return;
}

void Client::Handle_OP_XTargetAutoAddHaters(const EQApplicationPacket *app)
{
	if (app->size != 1)
	{
		Log(Logs::General, Logs::None, "Size mismatch in OP_XTargetAutoAddHaters, expected 1, got %i", app->size);
		DumpPacket(app);
		return;
	}

	XTargetAutoAddHaters = app->ReadUInt8(0);
	SetDirtyAutoHaters();
}

void Client::Handle_OP_XTargetOpen(const EQApplicationPacket *app)
{
	if (app->size != 4) {
		Log(Logs::General, Logs::None, "Size mismatch in OP_XTargetOpen, expected 1, got %i", app->size);
		DumpPacket(app);
		return;
	}

	auto outapp = new EQApplicationPacket(OP_XTargetOpenResponse, 0);
	FastQueuePacket(&outapp);
}

void Client::Handle_OP_XTargetRequest(const EQApplicationPacket *app)
{
	if (app->size < 12)
	{
		Log(Logs::General, Logs::None, "Size mismatch in OP_XTargetRequest, expected at least 12, got %i", app->size);
		DumpPacket(app);
		return;
	}

	uint32 Unknown000 = app->ReadUInt32(0);

	if (Unknown000 != 1)
		return;

	uint32 Slot = app->ReadUInt32(4);

	if (Slot >= XTARGET_HARDCAP)
		return;

	XTargetType Type = (XTargetType)app->ReadUInt32(8);

	XTargets[Slot].Type = Type;
	XTargets[Slot].ID = 0;
	XTargets[Slot].Name[0] = 0;

	switch (Type)
	{
	case Empty:
	case Auto:
	{
		break;
	}

	case CurrentTargetPC:
	{
		char Name[65];

		app->ReadString(Name, 12, 64);
		Client *c = entity_list.GetClientByName(Name);
		if (c)
		{
			XTargets[Slot].ID = c->GetID();
			strncpy(XTargets[Slot].Name, c->GetName(), 64);
		}
		else
		{
			strncpy(XTargets[Slot].Name, Name, 64);
		}
		SendXTargetPacket(Slot, c);

		break;
	}

	case CurrentTargetNPC:
	{
		char Name[65];
		app->ReadString(Name, 12, 64);
		Mob *m = entity_list.GetMob(Name);
		if (m)
		{
			XTargets[Slot].ID = m->GetID();
			SendXTargetPacket(Slot, m);
			break;
		}
	}

	case TargetsTarget:
	{
		if (GetTarget())
			UpdateXTargetType(TargetsTarget, GetTarget()->GetTarget());
		else
			UpdateXTargetType(TargetsTarget, nullptr);

		break;
	}

	case GroupTank:
	{
		Group *g = GetGroup();

		if (g)
		{
			Client *c = entity_list.GetClientByName(g->GetMainTankName());

			if (c)
			{
				XTargets[Slot].ID = c->GetID();
				strncpy(XTargets[Slot].Name, c->GetName(), 64);
			}
			else
			{
				strncpy(XTargets[Slot].Name, g->GetMainTankName(), 64);
			}
			SendXTargetPacket(Slot, c);
		}
		break;
	}
	case GroupTankTarget:
	{
		Group *g = GetGroup();

		if (g)
			g->NotifyTankTarget(this);

		break;
	}

	case GroupAssist:
	{
		Group *g = GetGroup();

		if (g)
		{
			Client *c = entity_list.GetClientByName(g->GetMainAssistName());

			if (c)
			{
				XTargets[Slot].ID = c->GetID();
				strncpy(XTargets[Slot].Name, c->GetName(), 64);
			}
			else
			{
				strncpy(XTargets[Slot].Name, g->GetMainAssistName(), 64);
			}
			SendXTargetPacket(Slot, c);
		}
		break;
	}

	case GroupAssistTarget:
	{

		Group *g = GetGroup();

		if (g)
			g->NotifyAssistTarget(this);

		break;
	}

	case Puller:
	{
		Group *g = GetGroup();

		if (g)
		{
			Client *c = entity_list.GetClientByName(g->GetPullerName());

			if (c)
			{
				XTargets[Slot].ID = c->GetID();
				strncpy(XTargets[Slot].Name, c->GetName(), 64);
			}
			else
			{
				strncpy(XTargets[Slot].Name, g->GetPullerName(), 64);
			}
			SendXTargetPacket(Slot, c);
		}
		break;
	}

	case PullerTarget:
	{

		Group *g = GetGroup();

		if (g)
			g->NotifyPullerTarget(this);

		break;
	}

	case GroupMarkTarget1:
	case GroupMarkTarget2:
	case GroupMarkTarget3:
	{
		Group *g = GetGroup();

		if (g)
			g->SendMarkedNPCsToMember(this);

		break;
	}

	case RaidAssist1:
	case RaidAssist2:
	case RaidAssist3:
	case RaidAssist1Target:
	case RaidAssist2Target:
	case RaidAssist3Target:
	case RaidMarkTarget1:
	case RaidMarkTarget2:
	case RaidMarkTarget3:
	{
		// Not implemented yet.
		break;
	}

	case MyPet:
	{
		Mob *m = GetPet();
		if (m)
		{
			XTargets[Slot].ID = m->GetID();
			SendXTargetPacket(Slot, m);

		}
		break;
	}
	case MyPetTarget:
	{
		Mob *m = GetPet();

		if (m)
			m = m->GetTarget();

		if (m)
		{
			XTargets[Slot].ID = m->GetID();
			SendXTargetPacket(Slot, m);

		}
		break;
	}

	default:
		Log(Logs::General, Logs::None, "Unhandled XTarget Type %i", Type);
		break;
	}

}

void Client::Handle_OP_YellForHelp(const EQApplicationPacket *app)
{
	auto outapp = new EQApplicationPacket(OP_YellForHelp, 4);
	*(uint32 *)outapp->pBuffer = GetID();
	entity_list.QueueCloseClients(this, outapp, true, 100.0);
	safe_delete(outapp);
	return;
}

void Client::Handle_OP_ResetAA(const EQApplicationPacket *app)
{
	if (Admin() >= 50) {
		Message(0, "Resetting AA points.");
		ResetAA();
	}
	return;
}
