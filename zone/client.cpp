/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.org)

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
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// for windows compile
#ifndef _WINDOWS
	#include <stdarg.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include "../common/unix.h"
#endif

extern volatile bool RunLoops;

#include "../common/eqemu_logsys.h"
#include "../common/features.h"
#include "../common/spdat.h"
#include "../common/guilds.h"
#include "../common/rulesys.h"
#include "../common/string_util.h"
#include "../common/data_verification.h"
#include "../common/profanity_manager.h"
#include "data_bucket.h"
#include "expedition.h"
#include "expedition_database.h"
#include "expedition_request.h"
#include "position.h"
#include "worldserver.h"
#include "zonedb.h"
#include "zone_store.h"
#include "petitions.h"
#include "command.h"
#include "water_map.h"
#ifdef BOTS
#include "bot_command.h"
#endif
#include "string_ids.h"

#include "guild_mgr.h"
#include "quest_parser_collection.h"
#include "queryserv.h"
#include "mob_movement_manager.h"
#include "../common/content/world_content_service.h"
#include "../common/expedition_lockout_timer.h"

extern QueryServ* QServ;
extern EntityList entity_list;
extern Zone* zone;
extern volatile bool is_zone_loaded;
extern WorldServer worldserver;
extern uint32 numclients;
extern PetitionList petition_list;
bool commandlogged;
char entirecommand[255];

void UpdateWindowTitle(char* iNewTitle);

Client::Client(EQStreamInterface* ieqs)
: Mob("No name",	// name
	"",	// lastname
	0,	// cur_hp
	0,	// max_hp
	0,	// gender
	0,	// race
	0,	// class
	BT_Humanoid,	// bodytype
	0,	// deity
	0,	// level
	0,	// npctypeid
	0,	// size
	0.7,	// runspeed
	glm::vec4(),
	0,	// light - verified for client innate_light value
	0xFF,	// texture
	0xFF,	// helmtexture
	0,	// ac
	0,	// atk
	0,	// str
	0,	// sta
	0,	// dex
	0,	// agi
	0,	// int
	0,	// wis
	0,	// cha
	0,	// Luclin Hair Colour
	0,	// Luclin Beard Color
	0,	// Luclin Eye1
	0,	// Luclin Eye2
	0,	// Luclin Hair Style
	0,	// Luclin Face
	0,	// Luclin Beard
	0,	// Drakkin Heritage
	0,	// Drakkin Tattoo
	0,	// Drakkin Details
	EQ::TintProfile(),	// Armor Tint
	0xff,	// AA Title
	0,	// see_invis
	0,	// see_invis_undead
	0,
	0,
	0,
	0,
	0,	// qglobal
	0,	// maxlevel
	0,	// scalerate
	0,
	0,
	0,
	0,
	0,
	0,
	false
	),
  hpupdate_timer(2000),
  camp_timer(29000),
  process_timer(100),
  consume_food_timer(CONSUMPTION_TIMER),
  zoneinpacket_timer(1000),
  linkdead_timer(RuleI(Zone,ClientLinkdeadMS)),
  dead_timer(2000),
  global_channel_timer(1000),
  shield_timer(500),
  fishing_timer(8000),
  endupkeep_timer(1000),
  forget_timer(0),
  autosave_timer(RuleI(Character, AutosaveIntervalS) * 1000),
  client_scan_npc_aggro_timer(RuleI(Aggro, ClientAggroCheckIdleInterval)),
  client_zone_wide_full_position_update_timer(5 * 60 * 1000),
  tribute_timer(Tribute_duration),
  proximity_timer(ClientProximity_interval),
  TaskPeriodic_Timer(RuleI(TaskSystem, PeriodicCheckTimer) * 1000),
  charm_update_timer(6000),
  rest_timer(1),
  charm_class_attacks_timer(3000),
  charm_cast_timer(3500),
  qglobal_purge_timer(30000),
  TrackingTimer(2000),
  RespawnFromHoverTimer(0),
  merc_timer(RuleI(Mercs, UpkeepIntervalMS)),
  ItemTickTimer(10000),
  ItemQuestTimer(500),
  anon_toggle_timer(250),
  afk_toggle_timer(250),
  helm_toggle_timer(250),
  aggro_meter_timer(AGGRO_METER_UPDATE_MS),
  m_Proximity(FLT_MAX, FLT_MAX, FLT_MAX), //arbitrary large number
	m_ZoneSummonLocation(-2.0f,-2.0f,-2.0f,-2.0f),
  m_AutoAttackPosition(0.0f, 0.0f, 0.0f, 0.0f),
  m_AutoAttackTargetLocation(0.0f, 0.0f, 0.0f),
  last_region_type(RegionTypeUnsupported),
  m_dirtyautohaters(false),
  mob_close_scan_timer(6000),
  hp_self_update_throttle_timer(300),
  hp_other_update_throttle_timer(500),
  position_update_timer(10000),
  consent_throttle_timer(2000),
  tmSitting(0)
{

	for (int client_filter = 0; client_filter < _FilterCount; client_filter++)
		ClientFilters[client_filter] = FilterShow;

	mMovementManager->AddClient(this);
	character_id = 0;
	conn_state = NoPacketsReceived;
	client_data_loaded = false;
	feigned = false;
	berserk = false;
	dead = false;
	eqs = ieqs;
	ip = eqs->GetRemoteIP();
	port = ntohs(eqs->GetRemotePort());
	client_state = CLIENT_CONNECTING;
	Trader=false;
	Buyer = false;
	Haste = 0;
	CustomerID = 0;
	TraderID = 0;
	TrackingID = 0;
	WID = 0;
	account_id = 0;
	admin = 0;
	lsaccountid = 0;
	shield_target = nullptr;
	guild_id = GUILD_NONE;
	guildrank = 0;
	GuildBanker = false;
	memset(lskey, 0, sizeof(lskey));
	strcpy(account_name, "");
	tellsoff = false;
	last_reported_mana = 0;
	last_reported_endurance = 0;
	last_reported_endurance_percent = 0;
	last_reported_mana_percent = 0;
	gm_hide_me = false;
	AFK = false;
	LFG = false;
	LFGFromLevel = 0;
	LFGToLevel = 0;
	LFGMatchFilter = false;
	LFGComments[0] = '\0';
	LFP = false;
	gmspeed = 0;
	playeraction = 0;
	SetTarget(0);
	auto_attack = false;
	auto_fire = false;
	runmode = false;
	linkdead_timer.Disable();
	zonesummon_id = 0;
	zonesummon_ignorerestrictions = 0;
	bZoning = false;
	zone_mode = ZoneUnsolicited;
	casting_spell_id = 0;
	npcflag = false;
	npclevel = 0;
	pQueuedSaveWorkID = 0;
	position_update_same_count = 0;
	fishing_timer.Disable();
	shield_timer.Disable();
	dead_timer.Disable();
	camp_timer.Disable();
	autosave_timer.Disable();
	GetMercTimer()->Disable();
	instalog = false;
	pLastUpdate = 0;
	pLastUpdateWZ = 0;
	m_pp.autosplit = false;
	// initialise haste variable
	m_tradeskill_object = nullptr;
	delaytimer = false;
	PendingRezzXP = -1;
	PendingRezzDBID = 0;
	PendingRezzSpellID = 0;
	numclients++;
	// emuerror;
	UpdateWindowTitle(nullptr);
	horseId = 0;
	tgb = false;
	tribute_master_id = 0xFFFFFFFF;
	tribute_timer.Disable();
	task_state         = nullptr;
	TotalSecondsPlayed = 0;
	keyring.clear();
	bind_sight_target = nullptr;
	p_raid_instance = nullptr;
	mercid = 0;
	mercSlot = 0;
	InitializeMercInfo();
	SetMerc(0);
	if (RuleI(World, PVPMinLevel) > 0 && level >= RuleI(World, PVPMinLevel) && m_pp.pvp == 0) SetPVP(true, false);
	dynamiczone_removal_timer.Disable();

	//for good measure:
	memset(&m_pp, 0, sizeof(m_pp));
	memset(&m_epp, 0, sizeof(m_epp));
	PendingTranslocate = false;
	PendingSacrifice = false;
	controlling_boat_id = 0;
	controlled_mob_id = 0;

	if (!RuleB(Character, PerCharacterQglobalMaxLevel) && !RuleB(Character, PerCharacterBucketMaxLevel)) {
		SetClientMaxLevel(0);
	} else if (RuleB(Character, PerCharacterQglobalMaxLevel)) {
		int client_max_level = GetCharMaxLevelFromQGlobal();
		SetClientMaxLevel(client_max_level);
	} else if (RuleB(Character, PerCharacterBucketMaxLevel)) {
		int client_max_level = GetCharMaxLevelFromBucket();
		SetClientMaxLevel(client_max_level);
	}

	KarmaUpdateTimer = new Timer(RuleI(Chat, KarmaUpdateIntervalMS));
	GlobalChatLimiterTimer = new Timer(RuleI(Chat, IntervalDurationMS));
	AttemptedMessages = 0;
	TotalKarma = 0;
	m_ClientVersion = EQ::versions::ClientVersion::Unknown;
	m_ClientVersionBit = 0;
	AggroCount = 0;
	ooc_regen = false;
	AreaHPRegen = 1.0f;
	AreaManaRegen = 1.0f;
	AreaEndRegen = 1.0f;
	XPRate = 100;
	current_endurance = 0;

	CanUseReport = true;
	aa_los_them_mob = nullptr;
	los_status = false;
	los_status_facing = false;
	qGlobals = nullptr;
	HideCorpseMode = HideCorpseNone;
	PendingGuildInvitation = false;

	current_endurance = 0;

	InitializeBuffSlots();

	adventure_request_timer = nullptr;
	adventure_create_timer = nullptr;
	adventure_leave_timer = nullptr;
	adventure_door_timer = nullptr;
	adv_requested_data = nullptr;
	adventure_stats_timer = nullptr;
	adventure_leaderboard_timer = nullptr;
	adv_data = nullptr;
	adv_requested_theme = 0;
	adv_requested_id = 0;
	adv_requested_member_count = 0;

	for(int i = 0; i < XTARGET_HARDCAP; ++i)
	{
		XTargets[i].Type = Auto;
		XTargets[i].ID = 0;
		XTargets[i].Name[0] = 0;
		XTargets[i].dirty = false;
	}
	MaxXTargets = 5;
	XTargetAutoAddHaters = true;
	m_autohatermgr.SetOwner(this, nullptr, nullptr);
	m_activeautohatermgr = &m_autohatermgr;
	LoadAccountFlags();

	initial_respawn_selection = 0;
	alternate_currency_loaded = false;

	interrogateinv_flag = false;

	trapid = 0;

	for (int i = 0; i < InnateSkillMax; ++i)
		m_pp.InnateSkills[i] = InnateDisabled;

	temp_pvp = false;
	is_client_moving = false;

	/**
	 * GM
	 */
	SetDisplayMobInfoWindow(true);
	SetDevToolsEnabled(true);

#ifdef BOTS
	bot_owner_options[booDeathMarquee] = false;
	bot_owner_options[booStatsUpdate] = false;
	bot_owner_options[booSpawnMessageSay] = false;
	bot_owner_options[booSpawnMessageTell] = true;
	bot_owner_options[booSpawnMessageClassSpecific] = true;
	bot_owner_options[booAltCombat] = RuleB(Bots, AllowOwnerOptionAltCombat);
	bot_owner_options[booAutoDefend] = RuleB(Bots, AllowOwnerOptionAutoDefend);
	bot_owner_options[booBuffCounter] = false;
	bot_owner_options[booMonkWuMessage] = false;

	SetBotPulling(false);
	SetBotPrecombat(false);
#endif

	AI_Init();
}

Client::~Client() {
	mMovementManager->RemoveClient(this);

#ifdef BOTS
	Bot::ProcessBotOwnerRefDelete(this);
#endif
	if(IsInAGuild())
		guild_mgr.SendGuildMemberUpdateToWorld(GetName(), GuildID(), 0, time(nullptr));

	Mob* horse = entity_list.GetMob(this->CastToClient()->GetHorseId());
	if (horse)
		horse->Depop();

	Mob* merc = entity_list.GetMob(this->GetMercID());
	if (merc)
		merc->Depop();

	if(Trader)
		database.DeleteTraderItem(this->CharacterID());

	if(Buyer)
		ToggleBuyerMode(false);

	if(conn_state != ClientConnectFinished) {
		LogDebug("Client [{}] was destroyed before reaching the connected state:", GetName());
		ReportConnectingState();
	}

	if(m_tradeskill_object != nullptr) {
		m_tradeskill_object->Close();
		m_tradeskill_object = nullptr;
	}

	close_mobs.clear();

	if(IsDueling() && GetDuelTarget() != 0) {
		Entity* entity = entity_list.GetID(GetDuelTarget());
		if(entity != nullptr && entity->IsClient()) {
			entity->CastToClient()->SetDueling(false);
			entity->CastToClient()->SetDuelTarget(0);
			entity_list.DuelMessage(entity->CastToClient(),this,true);
		}
	}

	if (shield_target) {
		for (int y = 0; y < 2; y++) {
			if (shield_target->shielder[y].shielder_id == GetID()) {
				shield_target->shielder[y].shielder_id = 0;
				shield_target->shielder[y].shielder_bonus = 0;
			}
		}
		shield_target = nullptr;
	}

	if(GetTarget())
		GetTarget()->IsTargeted(-1);

	//if we are in a group and we are not zoning, force leave the group
	if(isgrouped && !bZoning && is_zone_loaded)
		LeaveGroup();

	UpdateWho(2);

	if(IsHoveringForRespawn())
	{
		m_pp.zone_id = m_pp.binds[0].zone_id;
		m_pp.zoneInstance = m_pp.binds[0].instance_id;
		m_Position.x = m_pp.binds[0].x;
		m_Position.y = m_pp.binds[0].y;
		m_Position.z = m_pp.binds[0].z;
	}

	// we save right now, because the client might be zoning and the world
	// will need this data right away
	Save(2); // This fails when database destructor is called first on shutdown

	safe_delete(task_state);
	safe_delete(KarmaUpdateTimer);
	safe_delete(GlobalChatLimiterTimer);
	safe_delete(qGlobals);
	safe_delete(adventure_request_timer);
	safe_delete(adventure_create_timer);
	safe_delete(adventure_leave_timer);
	safe_delete(adventure_door_timer);
	safe_delete(adventure_stats_timer);
	safe_delete(adventure_leaderboard_timer);
	safe_delete_array(adv_requested_data);
	safe_delete_array(adv_data);

	ClearRespawnOptions();

	numclients--;
	UpdateWindowTitle(nullptr);
	if(zone)
		zone->RemoveAuth(GetName(), lskey);

	//let the stream factory know were done with this stream
	eqs->Close();
	eqs->ReleaseFromUse();
	safe_delete(eqs);

	UninitializeBuffSlots();
}

void Client::SendZoneInPackets()
{

	//////////////////////////////////////////////////////
	// Spawn Appearance Packet
	auto outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
	SpawnAppearance_Struct* sa = (SpawnAppearance_Struct*)outapp->pBuffer;
	sa->type = AT_SpawnID;			// Is 0x10 used to set the player id?
	sa->parameter = GetID();	// Four bytes for this parameter...
	outapp->priority = 6;
	QueuePacket(outapp);
	safe_delete(outapp);

	// Inform the world about the client
	outapp = new EQApplicationPacket();

	CreateSpawnPacket(outapp);
	outapp->priority = 6;
	if (!GetHideMe()) entity_list.QueueClients(this, outapp, true);
	safe_delete(outapp);
	SetSpawned();
	if (GetPVP(false))	//force a PVP update until we fix the spawn struct
		SendAppearancePacket(AT_PVP, GetPVP(false), true, false);

	//Send AA Exp packet:
	if (GetLevel() >= 51)
		SendAlternateAdvancementStats();

	// Send exp packets
	outapp = new EQApplicationPacket(OP_ExpUpdate, sizeof(ExpUpdate_Struct));
	ExpUpdate_Struct* eu = (ExpUpdate_Struct*)outapp->pBuffer;
	uint32 tmpxp1 = GetEXPForLevel(GetLevel() + 1);
	uint32 tmpxp2 = GetEXPForLevel(GetLevel());

	// Crash bug fix... Divide by zero when tmpxp1 and 2 equalled each other, most likely the error case from GetEXPForLevel() (invalid class, etc)
	if (tmpxp1 != tmpxp2 && tmpxp1 != 0xFFFFFFFF && tmpxp2 != 0xFFFFFFFF) {
		float tmpxp = (float)((float)m_pp.exp - tmpxp2) / ((float)tmpxp1 - tmpxp2);
		eu->exp = (uint32)(330.0f * tmpxp);
		outapp->priority = 6;
		QueuePacket(outapp);
	}
	safe_delete(outapp);

	SendAlternateAdvancementTimers();

	outapp = new EQApplicationPacket(OP_RaidUpdate, sizeof(ZoneInSendName_Struct));
	ZoneInSendName_Struct* zonesendname = (ZoneInSendName_Struct*)outapp->pBuffer;
	strcpy(zonesendname->name, m_pp.name);
	strcpy(zonesendname->name2, m_pp.name);
	zonesendname->unknown0 = 0x0A;
	QueuePacket(outapp);
	safe_delete(outapp);

	if (IsInAGuild()) {
		SendGuildMembers();
		SendGuildURL();
		SendGuildChannel();
		SendGuildLFGuildStatus();
	}
	SendLFGuildStatus();

	//No idea why live sends this if even were not in a guild
	SendGuildMOTD();
}

void Client::SendLogoutPackets() {

	auto outapp = new EQApplicationPacket(OP_CancelTrade, sizeof(CancelTrade_Struct));
	CancelTrade_Struct* ct = (CancelTrade_Struct*) outapp->pBuffer;
	ct->fromid = GetID();
	ct->action = groupActUpdate;
	FastQueuePacket(&outapp);

	outapp = new EQApplicationPacket(OP_PreLogoutReply);
	FastQueuePacket(&outapp);

}

void Client::ReportConnectingState() {
	switch(conn_state) {
	case NoPacketsReceived:		//havent gotten anything
		LogDebug("Client has not sent us an initial zone entry packet");
		break;
	case ReceivedZoneEntry:		//got the first packet, loading up PP
		LogDebug("Client sent initial zone packet, but we never got their player info from the database");
		break;
	case PlayerProfileLoaded:	//our DB work is done, sending it
		LogDebug("We were sending the player profile, tributes, tasks, spawns, time and weather, but never finished");
		break;
	case ZoneInfoSent:		//includes PP, tributes, tasks, spawns, time and weather
		LogDebug("We successfully sent player info and spawns, waiting for client to request new zone");
		break;
	case NewZoneRequested:	//received and sent new zone request
		LogDebug("We received client's new zone request, waiting for client spawn request");
		break;
	case ClientSpawnRequested:	//client sent ReqClientSpawn
		LogDebug("We received the client spawn request, and were sending objects, doors, zone points and some other stuff, but never finished");
		break;
	case ZoneContentsSent:		//objects, doors, zone points
		LogDebug("The rest of the zone contents were successfully sent, waiting for client ready notification");
		break;
	case ClientReadyReceived:	//client told us its ready, send them a bunch of crap like guild MOTD, etc
		LogDebug("We received client ready notification, but never finished Client::CompleteConnect");
		break;
	case ClientConnectFinished:	//client finally moved to finished state, were done here
		LogDebug("Client is successfully connected");
		break;
	};
}

bool Client::SaveAA() {
	std::string iquery;
	int spentpoints = 0;
	int i = 0;
	for(auto &rank : aa_ranks) {
		AA::Ability *ability = zone->GetAlternateAdvancementAbility(rank.first);
		if(!ability)
			continue;

		if(rank.second.first > 0) {
			AA::Rank *r = ability->GetRankByPointsSpent(rank.second.first);

			if(!r)
				continue;

			spentpoints += r->total_cost;

			if(i == 0) {
				iquery = StringFormat("REPLACE INTO `character_alternate_abilities` (id, aa_id, aa_value, charges)"
									  " VALUES (%u, %u, %u, %u)", character_id, ability->first_rank_id, rank.second.first, rank.second.second);
			} else {
				iquery += StringFormat(", (%u, %u, %u, %u)", character_id, ability->first_rank_id, rank.second.first, rank.second.second);
			}
			i++;
		}
	}

	m_pp.aapoints_spent = spentpoints + m_epp.expended_aa;

	if(iquery.length() > 0) {
		database.QueryDatabase(iquery);
	}

	return true;
}

void Client::RemoveExpendedAA(int aa_id)
{
	database.QueryDatabase(StringFormat("DELETE from `character_alternate_abilities` WHERE `id` = %d and `aa_id` = %d", character_id, aa_id));
}

bool Client::Save(uint8 iCommitNow) {
	if(!ClientDataLoaded())
		return false;

	/* Wrote current basics to PP for saves */
	m_pp.x = m_Position.x;
	m_pp.y = m_Position.y;
	m_pp.z = m_Position.z;
	m_pp.guildrank = guildrank;
	m_pp.heading = m_Position.w;

	/* Mana and HP */
	if (GetHP() <= 0) {
		m_pp.cur_hp = GetMaxHP();
	}
	else {
		m_pp.cur_hp = GetHP();
	}

	m_pp.mana = current_mana;
	m_pp.endurance = current_endurance;

	/* Save Character Currency */
	database.SaveCharacterCurrency(CharacterID(), &m_pp);

	/* Save Current Bind Points */
	for (int i = 0; i < 5; i++)
		if (m_pp.binds[i].zone_id)
			database.SaveCharacterBindPoint(CharacterID(), m_pp.binds[i], i);

	/* Save Character Buffs */
	database.SaveBuffs(this);

	/* Total Time Played */
	TotalSecondsPlayed += (time(nullptr) - m_pp.lastlogin);
	m_pp.timePlayedMin = (TotalSecondsPlayed / 60);
	m_pp.RestTimer = GetRestTimer();

	/* Save Mercs */
	if (GetMercInfo().MercTimerRemaining > RuleI(Mercs, UpkeepIntervalMS)) {
		GetMercInfo().MercTimerRemaining = RuleI(Mercs, UpkeepIntervalMS);
	}

	if (GetMercTimer()->Enabled()) {
		GetMercInfo().MercTimerRemaining = GetMercTimer()->GetRemainingTime();
	}

	if (dead || (!GetMerc() && !GetMercInfo().IsSuspended)) {
		memset(&m_mercinfo, 0, sizeof(struct MercInfo));
	}

	m_pp.lastlogin = time(nullptr);

	if (GetPet() && GetPet()->CastToNPC()->GetPetSpellID() && !dead) {
		NPC *pet = GetPet()->CastToNPC();
		m_petinfo.SpellID = pet->CastToNPC()->GetPetSpellID();
		m_petinfo.HP = pet->GetHP();
		m_petinfo.Mana = pet->GetMana();
		pet->GetPetState(m_petinfo.Buffs, m_petinfo.Items, m_petinfo.Name);
		m_petinfo.petpower = pet->GetPetPower();
		m_petinfo.size = pet->GetSize();
		m_petinfo.taunting = pet->CastToNPC()->IsTaunting();
	} else {
		memset(&m_petinfo, 0, sizeof(struct PetInfo));
	}
	database.SavePetInfo(this);

	if(tribute_timer.Enabled()) {
		m_pp.tribute_time_remaining = tribute_timer.GetRemainingTime();
	}
	else {
		m_pp.tribute_time_remaining = 0xFFFFFFFF; m_pp.tribute_active = 0;
	}

	if (m_pp.hunger_level < 0)
		m_pp.hunger_level = 0;

	if (m_pp.thirst_level < 0)
		m_pp.thirst_level = 0;

	p_timers.Store(&database);

	database.SaveCharacterTribute(this->CharacterID(), &m_pp);
	SaveTaskState(); /* Save Character Task */

	LogFood("Client::Save - hunger_level: [{}] thirst_level: [{}]", m_pp.hunger_level, m_pp.thirst_level);

	// perform snapshot before SaveCharacterData() so that m_epp will contain the updated time
	if (RuleB(Character, ActiveInvSnapshots) && time(nullptr) >= GetNextInvSnapshotTime()) {
		if (database.SaveCharacterInvSnapshot(CharacterID())) {
			SetNextInvSnapshot(RuleI(Character, InvSnapshotMinIntervalM));
		}
		else {
			SetNextInvSnapshot(RuleI(Character, InvSnapshotMinRetryM));
		}
	}

	database.SaveCharacterData(this->CharacterID(), this->AccountID(), &m_pp, &m_epp); /* Save Character Data */

	return true;
}

void Client::SaveBackup() {
}

CLIENTPACKET::CLIENTPACKET()
{
	app = nullptr;
	ack_req = false;
}

CLIENTPACKET::~CLIENTPACKET()
{
	safe_delete(app);
}

//this assumes we do not own pApp, and clones it.
bool Client::AddPacket(const EQApplicationPacket *pApp, bool bAckreq) {
	if (!pApp)
		return false;
	if(!zoneinpacket_timer.Enabled()) {
		//drop the packet because it will never get sent.
		return(false);
	}

	auto c = std::make_unique<CLIENTPACKET>();

	c->ack_req = bAckreq;
	c->app = pApp->Copy();

	clientpackets.push_back(std::move(c));
	return true;
}

//this assumes that it owns the object pointed to by *pApp
bool Client::AddPacket(EQApplicationPacket** pApp, bool bAckreq) {
	if (!pApp || !(*pApp))
		return false;
	if(!zoneinpacket_timer.Enabled()) {
		//drop the packet because it will never get sent.
		return(false);
	}
	auto c = std::make_unique<CLIENTPACKET>();

	c->ack_req = bAckreq;
	c->app = *pApp;
	*pApp = nullptr;

	clientpackets.push_back(std::move(c));
	return true;
}

bool Client::SendAllPackets() {
	CLIENTPACKET* cp = nullptr;
	while (!clientpackets.empty()) {
		cp = clientpackets.front().get();
		if(eqs)
			eqs->FastQueuePacket((EQApplicationPacket **)&cp->app, cp->ack_req);
		clientpackets.pop_front();
		Log(Logs::Moderate, Logs::PacketClientServer, "Transmitting a packet");
	}
	return true;
}

void Client::QueuePacket(const EQApplicationPacket* app, bool ack_req, CLIENT_CONN_STATUS required_state, eqFilterType filter) {
	if(filter!=FilterNone){
		//this is incomplete... no support for FilterShowGroupOnly or FilterShowSelfOnly
		if(GetFilter(filter) == FilterHide)
			return; //Client has this filter on, no need to send packet
	}
	if(client_state != CLIENT_CONNECTED && required_state == CLIENT_CONNECTED){
		AddPacket(app, ack_req);
		return;
	}

	// if the program doesnt care about the status or if the status isnt what we requested
	if (required_state != CLIENT_CONNECTINGALL && client_state != required_state)
	{
		// todo: save packets for later use
		AddPacket(app, ack_req);
	}
	else
		if(eqs)
			eqs->QueuePacket(app, ack_req);
}

void Client::FastQueuePacket(EQApplicationPacket** app, bool ack_req, CLIENT_CONN_STATUS required_state) {
	// if the program doesnt care about the status or if the status isnt what we requested
	if (required_state != CLIENT_CONNECTINGALL && client_state != required_state) {
		// todo: save packets for later use
		AddPacket(app, ack_req);
		return;
	}
	else {
		if(eqs)
			eqs->FastQueuePacket((EQApplicationPacket **)app, ack_req);
		else if (app && (*app))
			delete *app;
		*app = nullptr;
	}
	return;
}

void Client::ChannelMessageReceived(uint8 chan_num, uint8 language, uint8 lang_skill, const char* orig_message, const char* targetname) {
	char message[4096];
	strn0cpy(message, orig_message, sizeof(message));

	LogDebug("Client::ChannelMessageReceived() Channel:[{}] message:[{}]", chan_num, message);

	if (targetname == nullptr) {
		targetname = (!GetTarget()) ? "" : GetTarget()->GetName();
	}

	if(RuleB(Chat, EnableAntiSpam))
	{
		if(strcmp(targetname, "discard") != 0)
		{
			if(chan_num == ChatChannel_Shout || chan_num == ChatChannel_Auction || chan_num == ChatChannel_OOC || chan_num == ChatChannel_Tell)
			{
				if(GlobalChatLimiterTimer)
				{
					if(GlobalChatLimiterTimer->Check(false))
					{
						GlobalChatLimiterTimer->Start(RuleI(Chat, IntervalDurationMS));
						AttemptedMessages = 0;
					}
				}

				uint32 AllowedMessages = RuleI(Chat, MinimumMessagesPerInterval) + TotalKarma;
				AllowedMessages = AllowedMessages > RuleI(Chat, MaximumMessagesPerInterval) ? RuleI(Chat, MaximumMessagesPerInterval) : AllowedMessages;

				if(RuleI(Chat, MinStatusToBypassAntiSpam) <= Admin())
					AllowedMessages = 10000;

				AttemptedMessages++;
				if(AttemptedMessages > AllowedMessages)
				{
					if(AttemptedMessages > RuleI(Chat, MaxMessagesBeforeKick))
					{
						Kick("Sent too many chat messages at once.");
						return;
					}
					if(GlobalChatLimiterTimer)
					{
						Message(0, "You have been rate limited, you can send more messages in %i seconds.",
							GlobalChatLimiterTimer->GetRemainingTime() / 1000);
						return;
					}
					else
					{
						Message(0, "You have been rate limited, you can send more messages in 60 seconds.");
						return;
					}
				}
			}
		}
	}

	/* Logs Player Chat */
	if (RuleB(QueryServ, PlayerLogChat)) {
		auto pack = new ServerPacket(ServerOP_Speech, sizeof(Server_Speech_Struct) + strlen(message) + 1);
		Server_Speech_Struct* sem = (Server_Speech_Struct*) pack->pBuffer;

		if(chan_num == ChatChannel_Guild)
			sem->guilddbid = GuildID();
		else
			sem->guilddbid = 0;

		strcpy(sem->message, message);
		sem->minstatus = this->Admin();
		sem->type = chan_num;
		if(targetname != 0)
			strcpy(sem->to, targetname);

		if(GetName() != 0)
			strcpy(sem->from, GetName());

		if(worldserver.Connected())
			worldserver.SendPacket(pack);
		safe_delete(pack);
	}

	//Return true to proceed, false to return
	if(!mod_client_message(message, chan_num)) { return; }

	// Garble the message based on drunkness
	if (m_pp.intoxication > 0 && !(RuleB(Chat, ServerWideOOC) && chan_num == ChatChannel_OOC) && !GetGM()) {
		GarbleMessage(message, (int)(m_pp.intoxication / 3));
		language = 0; // No need for language when drunk
		lang_skill = 100;
	}

	// some channels don't use languages
	if (chan_num == ChatChannel_OOC || chan_num == ChatChannel_GMSAY || chan_num == ChatChannel_Broadcast || chan_num == ChatChannel_Petition)
	{
		language = 0;
		lang_skill = 100;
	}

	// Censor the message
	if (EQ::ProfanityManager::IsCensorshipActive() && (chan_num != ChatChannel_Say))
		EQ::ProfanityManager::RedactMessage(message);

	switch(chan_num)
	{
	case ChatChannel_Guild: { /* Guild Chat */
		if (!IsInAGuild())
			MessageString(Chat::DefaultText, GUILD_NOT_MEMBER2);	//You are not a member of any guild.
		else if (!guild_mgr.CheckPermission(GuildID(), GuildRank(), GUILD_SPEAK))
			Message(0, "Error: You dont have permission to speak to the guild.");
		else if (!worldserver.SendChannelMessage(this, targetname, chan_num, GuildID(), language, lang_skill, message))
			Message(0, "Error: World server disconnected");
		break;
	}
	case ChatChannel_Group: { /* Group Chat */
		Raid* raid = entity_list.GetRaidByClient(this);
		if(raid) {
			raid->RaidGroupSay((const char*) message, this, language, lang_skill);
			break;
		}

		Group* group = GetGroup();
		if(group != nullptr) {
			group->GroupMessage(this,language,lang_skill,(const char*) message);
		}
		break;
	}
	case ChatChannel_Raid: { /* Raid Say */
		Raid* raid = entity_list.GetRaidByClient(this);
		if(raid){
			raid->RaidSay((const char*) message, this, language, lang_skill);
		}
		break;
	}
	case ChatChannel_Shout: { /* Shout */
		Mob *sender = this;
		if (GetPet() && GetTarget() == GetPet() && GetPet()->FindType(SE_VoiceGraft))
			sender = GetPet();

		entity_list.ChannelMessage(sender, chan_num, language, lang_skill, message);
		break;
	}
	case ChatChannel_Auction: { /* Auction */
		if(RuleB(Chat, ServerWideAuction))
		{
			if(!global_channel_timer.Check())
			{
				if(strlen(targetname) == 0)
					ChannelMessageReceived(chan_num, language, lang_skill, message, "discard"); //Fast typer or spammer??
				else
					return;
			}

			if(GetRevoked())
			{
				Message(0, "You have been revoked. You may not talk on Auction.");
				return;
			}

			if(TotalKarma < RuleI(Chat, KarmaGlobalChatLimit))
			{
				if(GetLevel() < RuleI(Chat, GlobalChatLevelLimit))
				{
					Message(0, "You do not have permission to talk in Auction at this time.");
					return;
				}
			}

			if (!worldserver.SendChannelMessage(this, 0, chan_num, 0, language, lang_skill, message))
			Message(0, "Error: World server disconnected");
		}
		else if(!RuleB(Chat, ServerWideAuction)) {
			Mob *sender = this;

			if (GetPet() && GetTarget() == GetPet() && GetPet()->FindType(SE_VoiceGraft))
			sender = GetPet();

			entity_list.ChannelMessage(sender, chan_num, language, lang_skill, message);
		}
		break;
	}
	case ChatChannel_OOC: { /* OOC */
		if(RuleB(Chat, ServerWideOOC))
		{
			if(!global_channel_timer.Check())
			{
				if(strlen(targetname) == 0)
					ChannelMessageReceived(chan_num, language, lang_skill, message, "discard"); //Fast typer or spammer??
				else
					return;
			}
			if(worldserver.IsOOCMuted() && admin < 100)
			{
				Message(0,"OOC has been muted. Try again later.");
				return;
			}

			if(GetRevoked())
			{
				Message(0, "You have been revoked. You may not talk on OOC.");
				return;
			}

			if(TotalKarma < RuleI(Chat, KarmaGlobalChatLimit))
			{
				if(GetLevel() < RuleI(Chat, GlobalChatLevelLimit))
				{
					Message(0, "You do not have permission to talk in OOC at this time.");
					return;
				}
			}

			if (!worldserver.SendChannelMessage(this, 0, chan_num, 0, language, lang_skill, message))
			{
				Message(0, "Error: World server disconnected");
			}
		}
		else if(!RuleB(Chat, ServerWideOOC))
		{
			Mob *sender = this;

			if (GetPet() && GetTarget() == GetPet() && GetPet()->FindType(SE_VoiceGraft))
				sender = GetPet();

			entity_list.ChannelMessage(sender, chan_num, language, lang_skill, message);
		}
		break;
	}
	case ChatChannel_Broadcast: /* Broadcast */
	case ChatChannel_GMSAY: { /* GM Say */
		if (!(admin >= 80))
			Message(0, "Error: Only GMs can use this channel");
		else if (!worldserver.SendChannelMessage(this, targetname, chan_num, 0, language, lang_skill, message))
			Message(0, "Error: World server disconnected");
		break;
	}
	case ChatChannel_Tell: { /* Tell */
			if(!global_channel_timer.Check())
			{
				if(strlen(targetname) == 0)
					ChannelMessageReceived(7, language, lang_skill, message, "discard"); //Fast typer or spammer??
				else
					return;
			}

			if(GetRevoked())
			{
				Message(0, "You have been revoked. You may not send tells.");
				return;
			}

			if(TotalKarma < RuleI(Chat, KarmaGlobalChatLimit))
			{
				if(GetLevel() < RuleI(Chat, GlobalChatLevelLimit))
				{
					Message(0, "You do not have permission to send tells at this time.");
					return;
				}
			}

			char target_name[64];

			if(targetname)
			{
				size_t i = strlen(targetname);
				int x;
				for(x = 0; x < i; ++x)
				{
					if(targetname[x] == '%')
					{
						target_name[x] = '/';
					}
					else
					{
						target_name[x] = targetname[x];
					}
				}
				target_name[x] = '\0';
			}

			if(!worldserver.SendChannelMessage(this, target_name, chan_num, 0, language, lang_skill, message))
				Message(0, "Error: World server disconnected");
		break;
	}
	case ChatChannel_Say: { /* Say */
		if(message[0] == COMMAND_CHAR) {
			if(command_dispatch(this, message) == -2) {
				if (parse->PlayerHasQuestSub(EVENT_COMMAND)) {
					int i = parse->EventPlayer(EVENT_COMMAND, this, message, 0);
					if (i == 0 && !RuleB(Chat, SuppressCommandErrors)) {
						Message(Chat::Red, "Command '%s' not recognized.", message);
					}
				}
				else if (parse->PlayerHasQuestSub(EVENT_SAY)) {
					int i = parse->EventPlayer(EVENT_SAY, this, message, 0);
					if (i == 0 && !RuleB(Chat, SuppressCommandErrors)) {
						Message(Chat::Red, "Command '%s' not recognized.", message);
					}
				}
				else {
					if (!RuleB(Chat, SuppressCommandErrors)) {
						Message(Chat::Red, "Command '%s' not recognized.", message);
					}
				}
			}
			break;
		}

#ifdef BOTS
		if (message[0] == BOT_COMMAND_CHAR) {
			if (bot_command_dispatch(this, message) == -2) {
				if (parse->PlayerHasQuestSub(EVENT_BOT_COMMAND)) {
					int i = parse->EventPlayer(EVENT_BOT_COMMAND, this, message, 0);
					if (i == 0 && !RuleB(Chat, SuppressCommandErrors)) {
						Message(Chat::Red, "Bot command '%s' not recognized.", message);
					}
				}
				else if (parse->PlayerHasQuestSub(EVENT_SAY)) {
					int i = parse->EventPlayer(EVENT_SAY, this, message, 0);
					if (i == 0 && !RuleB(Chat, SuppressCommandErrors)) {
						Message(Chat::Red, "Bot command '%s' not recognized.", message);
					}
				}
				else {
					if (!RuleB(Chat, SuppressCommandErrors)) {
						Message(Chat::Red, "Bot command '%s' not recognized.", message);
					}
				}
			}
			break;
		}
#endif

		if (EQ::ProfanityManager::IsCensorshipActive()) {
			EQ::ProfanityManager::RedactMessage(message);
		}

		Mob* sender = this;
		if (GetPet() && GetTarget() == GetPet() && GetPet()->FindType(SE_VoiceGraft))
			sender = GetPet();

		entity_list.ChannelMessage(sender, chan_num, language, lang_skill, message);
		parse->EventPlayer(EVENT_SAY, this, message, language);

		if (sender != this)
			break;

		if(quest_manager.ProximitySayInUse())
			entity_list.ProcessProximitySay(message, this, language);

		if (GetTarget() != 0 && GetTarget()->IsNPC() &&
			!IsInvisible(GetTarget())) {
			if(!GetTarget()->CastToNPC()->IsEngaged()) {
				CheckLDoNHail(GetTarget());
				CheckEmoteHail(GetTarget(), message);

				if(DistanceNoZ(m_Position, GetTarget()->GetPosition()) <= RuleI(Range, Say)) {
					NPC *tar = GetTarget()->CastToNPC();
					parse->EventNPC(EVENT_SAY, tar->CastToNPC(), this, message, language);

					if(RuleB(TaskSystem, EnableTaskSystem)) {
						if(UpdateTasksOnSpeakWith(tar->GetNPCTypeID())) {
							tar->DoQuestPause(this);
						}
					}
				}
			}
			else {
				if (DistanceSquaredNoZ(m_Position, GetTarget()->GetPosition()) <= RuleI(Range, Say)) {
					parse->EventNPC(EVENT_AGGRO_SAY, GetTarget()->CastToNPC(), this, message, language);
				}
			}

		}
		break;
	}
	case ChatChannel_UCSRelay:
	{
		// UCS Relay for Underfoot and later.
		if(!worldserver.SendChannelMessage(this, 0, chan_num, 0, language, lang_skill, message))
			Message(0, "Error: World server disconnected");
		break;
	}
	case ChatChannel_Emotes:
	{
		// Emotes for Underfoot and later.
		// crash protection -- cheater
		message[1023] = '\0';
		size_t msg_len = strlen(message);
		if (msg_len > 512)
			message[512] = '\0';

		auto outapp = new EQApplicationPacket(OP_Emote, 4 + msg_len + strlen(GetName()) + 2);
		Emote_Struct* es = (Emote_Struct*)outapp->pBuffer;
		char *Buffer = (char *)es;
		Buffer += 4;
		snprintf(Buffer, sizeof(Emote_Struct) - 4, "%s %s", GetName(), message);
		entity_list.QueueCloseClients(this, outapp, true, RuleI(Range, Emote), 0, true, FilterSocials);
		safe_delete(outapp);
		break;
	}
	default: {
		Message(0, "Channel (%i) not implemented", (uint16)chan_num);
	}
	}
}

void Client::ChannelMessageSend(const char* from, const char* to, uint8 chan_num, uint8 language, uint8 lang_skill, const char* message, ...) {
	if ((chan_num==11 && !(this->GetGM())) || (chan_num==10 && this->Admin()<80)) // dont need to send /pr & /petition to everybody
		return;
	va_list argptr;
	char buffer[4096];
	char message_sender[64];

	va_start(argptr, message);
	vsnprintf(buffer, 4096, message, argptr);
	va_end(argptr);

	EQApplicationPacket app(OP_ChannelMessage, sizeof(ChannelMessage_Struct)+strlen(buffer)+1);
	ChannelMessage_Struct* cm = (ChannelMessage_Struct*)app.pBuffer;

	if (from == 0)
		strcpy(cm->sender, "ZServer");
	else if (from[0] == 0)
		strcpy(cm->sender, "ZServer");
	else {
		CleanMobName(from, message_sender);
		strcpy(cm->sender, message_sender);
	}
	if (to != 0)
		strcpy((char *) cm->targetname, to);
	else if (chan_num == ChatChannel_Tell)
		strcpy(cm->targetname, m_pp.name);
	else
		cm->targetname[0] = 0;

	uint8 ListenerSkill;

	if (language < MAX_PP_LANGUAGE) {
		ListenerSkill = m_pp.languages[language];
		if (ListenerSkill < 24) {
			cm->language = (MAX_PP_LANGUAGE - 1); // in an unknown tongue
		}
		else {
			cm->language = language;
		}
	}
	else {
		ListenerSkill = m_pp.languages[0];
		cm->language = 0;
	}

	// set effective language skill = lower of sender and receiver skills
	int32 EffSkill = (lang_skill < ListenerSkill ? lang_skill : ListenerSkill);
	if (EffSkill > 100)	// maximum language skill is 100
		EffSkill = 100;
	cm->skill_in_language = EffSkill;

	cm->chan_num = chan_num;
	strcpy(&cm->message[0], buffer);
	QueuePacket(&app);

	bool senderCanTrainSelf = RuleB(Client, SelfLanguageLearning);
	bool weAreNotSender = strcmp(this->GetCleanName(), cm->sender);

	if (senderCanTrainSelf || weAreNotSender) {
		if ((chan_num == ChatChannel_Group) && (ListenerSkill < 100)) {	// group message in unmastered language, check for skill up
			if (m_pp.languages[language] <= lang_skill)
				CheckLanguageSkillIncrease(language, lang_skill);
		}
	}
}

void Client::Message(uint32 type, const char* message, ...) {
	if (GetFilter(FilterSpellDamage) == FilterHide && type == Chat::NonMelee)
		return;
	if (GetFilter(FilterMeleeCrits) == FilterHide && type == Chat::MeleeCrit) //98 is self...
		return;
	if (GetFilter(FilterSpellCrits) == FilterHide && type == Chat::SpellCrit)
		return;

	va_list argptr;
	auto buffer = new char[4096];
	va_start(argptr, message);
	vsnprintf(buffer, 4096, message, argptr);
	va_end(argptr);

	SerializeBuffer buf(sizeof(SpecialMesgHeader_Struct) + 12 + 64 + 64);
	buf.WriteInt8(static_cast<int8>(Journal::SpeakMode::Raw));
	buf.WriteInt8(static_cast<int8>(Journal::Mode::None));
	buf.WriteInt8(0); // language
	buf.WriteUInt32(type);
	buf.WriteUInt32(0); // target spawn ID used for journal filtering, ignored here
	buf.WriteString(""); // send name, not applicable here
	buf.WriteInt32(0); // location, client seems to ignore
	buf.WriteInt32(0);
	buf.WriteInt32(0);
	buf.WriteString(buffer);

	auto app = new EQApplicationPacket(OP_SpecialMesg, buf);

	FastQueuePacket(&app);

	safe_delete_array(buffer);
}

void Client::FilteredMessage(Mob *sender, uint32 type, eqFilterType filter, const char* message, ...) {
	if (!FilteredMessageCheck(sender, filter))
		return;

	va_list argptr;
	auto buffer = new char[4096];
	va_start(argptr, message);
	vsnprintf(buffer, 4096, message, argptr);
	va_end(argptr);

	SerializeBuffer buf(sizeof(SpecialMesgHeader_Struct) + 12 + 64 + 64);
	buf.WriteInt8(static_cast<int8>(Journal::SpeakMode::Raw));
	buf.WriteInt8(static_cast<int8>(Journal::Mode::None));
	buf.WriteInt8(0); // language
	buf.WriteUInt32(type);
	buf.WriteUInt32(0); // target spawn ID used for journal filtering, ignored here
	buf.WriteString(""); // send name, not applicable here
	buf.WriteInt32(0); // location, client seems to ignore
	buf.WriteInt32(0);
	buf.WriteInt32(0);
	buf.WriteString(buffer);

	auto app = new EQApplicationPacket(OP_SpecialMesg, buf);

	FastQueuePacket(&app);

	safe_delete_array(buffer);
}

void Client::SetMaxHP() {
	if(dead)
		return;
	SetHP(CalcMaxHP());
	SendHPUpdate();
	Save();
}

bool Client::UpdateLDoNPoints(uint32 theme_id, int points) {

/* make sure total stays in sync with individual buckets
	m_pp.ldon_points_available = m_pp.ldon_points_guk
		+m_pp.ldon_points_mir
		+m_pp.ldon_points_mmc
		+m_pp.ldon_points_ruj
		+m_pp.ldon_points_tak; */

	if(points < 0) {
		if(m_pp.ldon_points_available < (0 - points))
			return false;
	}

	switch (theme_id)
	{
		case 0: { // No theme, so distribute evenly across all
			int split_points = (points / 5);
			int guk_points = (split_points + (points % 5));
			int mir_points = split_points;
			int mmc_points = split_points;
			int ruj_points = split_points;
			int tak_points = split_points;
			split_points = 0;
			if(points < 0) {
				if(m_pp.ldon_points_available < (0 - points)) {
					return false;
				}

				if(m_pp.ldon_points_guk < (0 - guk_points)) {
					mir_points += (guk_points + m_pp.ldon_points_guk);
					guk_points = (0 - m_pp.ldon_points_guk);
				}

				if(m_pp.ldon_points_mir < (0 - mir_points)) {
					mmc_points += (mir_points + m_pp.ldon_points_mir);
					mir_points = (0 - m_pp.ldon_points_mir);
				}
				
				if(m_pp.ldon_points_mmc < (0 - mmc_points)) {
					ruj_points += (mmc_points + m_pp.ldon_points_mmc);
					mmc_points = (0 - m_pp.ldon_points_mmc);
				}

				if(m_pp.ldon_points_ruj < (0 - ruj_points)) {
					tak_points += (ruj_points + m_pp.ldon_points_ruj);
					ruj_points = (0 - m_pp.ldon_points_ruj);
				}

				if(m_pp.ldon_points_tak < (0 - tak_points)) {
					split_points = (tak_points + m_pp.ldon_points_tak);
					tak_points = (0 - m_pp.ldon_points_tak);
				}
			}
			m_pp.ldon_points_guk += guk_points;
			m_pp.ldon_points_mir += mir_points;
			m_pp.ldon_points_mmc += mmc_points;
			m_pp.ldon_points_ruj += ruj_points;
			m_pp.ldon_points_tak += tak_points;
			points -= split_points;
			if (split_points != 0) // if anything left, recursively loop thru again
				UpdateLDoNPoints(0, split_points);

			break;
		}
		case 1:	{
			if(points < 0) {
				if(m_pp.ldon_points_guk < (0 - points))
					return false;
			}
			m_pp.ldon_points_guk += points;
			break;
		}
		case 2: {
			if(points < 0) {
				if(m_pp.ldon_points_mir < (0 - points))
					return false;
			}
			m_pp.ldon_points_mir += points;
			break;
		}
		case 3: {
			if(points < 0) {
				if(m_pp.ldon_points_mmc < (0 - points))
					return false;
			}
			m_pp.ldon_points_mmc += points;
			break;
		}
		case 4: {
			if(points < 0) {
				if(m_pp.ldon_points_ruj < (0 - points))
					return false;
			}
			m_pp.ldon_points_ruj += points;
			break;
		}
		case 5: {
			if(points < 0) {
				if(m_pp.ldon_points_tak < (0 - points))
					return false;
			}
			m_pp.ldon_points_tak += points;
			break;
		}
	}
	m_pp.ldon_points_available += points;
	auto outapp = new EQApplicationPacket(OP_AdventurePointsUpdate, sizeof(AdventurePoints_Update_Struct));
	AdventurePoints_Update_Struct* apus = (AdventurePoints_Update_Struct*)outapp->pBuffer;
	apus->ldon_available_points = m_pp.ldon_points_available;
	apus->ldon_guk_points = m_pp.ldon_points_guk;
	apus->ldon_mirugal_points = m_pp.ldon_points_mir;
	apus->ldon_mistmoore_points = m_pp.ldon_points_mmc;
	apus->ldon_rujarkian_points = m_pp.ldon_points_ruj;
	apus->ldon_takish_points = m_pp.ldon_points_tak;
	outapp->priority = 6;
	QueuePacket(outapp);
	safe_delete(outapp);
	return true;
}

void Client::SetSkill(EQ::skills::SkillType skillid, uint16 value) {
	if (skillid > EQ::skills::HIGHEST_SKILL)
		return;
	m_pp.skills[skillid] = value; // We need to be able to #setskill 254 and 255 to reset skills

	database.SaveCharacterSkill(this->CharacterID(), skillid, value);

	auto outapp = new EQApplicationPacket(OP_SkillUpdate, sizeof(SkillUpdate_Struct));
	SkillUpdate_Struct* skill = (SkillUpdate_Struct*)outapp->pBuffer;
	skill->skillId=skillid;
	skill->value=value;
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::IncreaseLanguageSkill(int skill_id, int value) {

	if (skill_id >= MAX_PP_LANGUAGE)
		return; //Invalid lang id

	m_pp.languages[skill_id] += value;

	if (m_pp.languages[skill_id] > 100) //Lang skill above max
		m_pp.languages[skill_id] = 100;

	database.SaveCharacterLanguage(this->CharacterID(), skill_id, m_pp.languages[skill_id]);

	auto outapp = new EQApplicationPacket(OP_SkillUpdate, sizeof(SkillUpdate_Struct));
	SkillUpdate_Struct* skill = (SkillUpdate_Struct*)outapp->pBuffer;
	skill->skillId = 100 + skill_id;
	skill->value = m_pp.languages[skill_id];
	QueuePacket(outapp);
	safe_delete(outapp);

	MessageString( Chat::Skills, LANG_SKILL_IMPROVED ); //Notify client
}

void Client::AddSkill(EQ::skills::SkillType skillid, uint16 value) {
	if (skillid > EQ::skills::HIGHEST_SKILL)
		return;
	value = GetRawSkill(skillid) + value;
	uint16 max = GetMaxSkillAfterSpecializationRules(skillid, MaxSkill(skillid));
	if (value > max)
		value = max;
	SetSkill(skillid, value);
}

void Client::SendSound(){//Makes a sound.
	auto outapp = new EQApplicationPacket(OP_Sound, 68);
	unsigned char x[68];
	memset(x, 0, 68);
	x[0]=0x22;
	memset(&x[4],0x8002,sizeof(uint16));
	memset(&x[8],0x8624,sizeof(uint16));
	memset(&x[12],0x4A01,sizeof(uint16));
	x[16]=0x05;
	x[28]=0x00;//change this value to give gold to the client
	memset(&x[40],0xFFFFFFFF,sizeof(uint32));
	memset(&x[44],0xFFFFFFFF,sizeof(uint32));
	memset(&x[48],0xFFFFFFFF,sizeof(uint32));
	memset(&x[52],0xFFFFFFFF,sizeof(uint32));
	memset(&x[56],0xFFFFFFFF,sizeof(uint32));
	memset(&x[60],0xFFFFFFFF,sizeof(uint32));
	memset(&x[64],0xffffffff,sizeof(uint32));
	memcpy(outapp->pBuffer,x,outapp->size);
	QueuePacket(outapp);
	safe_delete(outapp);

}
void Client::UpdateWho(uint8 remove) {
	if (account_id == 0)
		return;
	if (!worldserver.Connected())
		return;
	auto pack = new ServerPacket(ServerOP_ClientList, sizeof(ServerClientList_Struct));
	ServerClientList_Struct* scl = (ServerClientList_Struct*) pack->pBuffer;
	scl->remove = remove;
	scl->wid = this->GetWID();
	scl->IP = this->GetIP();
	scl->charid = this->CharacterID();
	strcpy(scl->name, this->GetName());

	scl->gm = GetGM();
	scl->Admin = this->Admin();
	scl->AccountID = this->AccountID();
	strcpy(scl->AccountName, this->AccountName());
	scl->LSAccountID = this->LSAccountID();
	strn0cpy(scl->lskey, lskey, sizeof(scl->lskey));
	scl->zone = zone->GetZoneID();
	scl->instance_id = zone->GetInstanceID();
	scl->race = this->GetRace();
	scl->class_ = GetClass();
	scl->level = GetLevel();
	if (m_pp.anon == 0)
		scl->anon = 0;
	else if (m_pp.anon == 1)
		scl->anon = 1;
	else if (m_pp.anon >= 2)
		scl->anon = 2;

	scl->ClientVersion = static_cast<unsigned int>(ClientVersion());
	scl->tellsoff = tellsoff;
	scl->guild_id = guild_id;
	scl->LFG = LFG;
	if(LFG) {
		scl->LFGFromLevel = LFGFromLevel;
		scl->LFGToLevel = LFGToLevel;
		scl->LFGMatchFilter = LFGMatchFilter;
		memcpy(scl->LFGComments, LFGComments, sizeof(scl->LFGComments));
	}

	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void Client::WhoAll(Who_All_Struct* whom) {

	if (!worldserver.Connected())
		Message(0, "Error: World server disconnected");
	else {
		auto pack = new ServerPacket(ServerOP_Who, sizeof(ServerWhoAll_Struct));
		ServerWhoAll_Struct* whoall = (ServerWhoAll_Struct*) pack->pBuffer;
		whoall->admin = this->Admin();
		whoall->fromid=this->GetID();
		strcpy(whoall->from, this->GetName());
		strn0cpy(whoall->whom, whom->whom, 64);
		whoall->lvllow = whom->lvllow;
		whoall->lvlhigh = whom->lvlhigh;
		whoall->gmlookup = whom->gmlookup;
		whoall->wclass = whom->wclass;
		whoall->wrace = whom->wrace;
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void Client::FriendsWho(char *FriendsString) {

	if (!worldserver.Connected())
		Message(0, "Error: World server disconnected");
	else {
		auto pack =
		    new ServerPacket(ServerOP_FriendsWho, sizeof(ServerFriendsWho_Struct) + strlen(FriendsString));
		ServerFriendsWho_Struct* FriendsWho = (ServerFriendsWho_Struct*) pack->pBuffer;
		FriendsWho->FromID = this->GetID();
		strcpy(FriendsWho->FromName, GetName());
		strcpy(FriendsWho->FriendsString, FriendsString);
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void Client::UpdateAdmin(bool iFromDB) {
	int16 tmp = admin;
	if (iFromDB)
		admin = database.CheckStatus(account_id);
	if (tmp == admin && iFromDB)
		return;

	if(m_pp.gm)
	{
		LogInfo("[{}] - [{}] is a GM", __FUNCTION__ , GetName());
// no need for this, having it set in pp you already start as gm
// and it's also set in your spawn packet so other people see it too
//		SendAppearancePacket(AT_GM, 1, false);
		petition_list.UpdateGMQueue();
	}

	UpdateWho();
}

void Client::SetStats(uint8 type,int16 set_val){
	if(type>STAT_DISEASE){
		printf("Error in Client::IncStats, received invalid type of: %i\n",type);
		return;
	}
	auto outapp = new EQApplicationPacket(OP_IncreaseStats, sizeof(IncreaseStat_Struct));
	IncreaseStat_Struct* iss=(IncreaseStat_Struct*)outapp->pBuffer;
	switch(type){
		case STAT_STR:
			if(set_val>0)
				iss->str=set_val;
			if(set_val<0)
				m_pp.STR=0;
			else if(set_val>255)
				m_pp.STR=255;
			else
				m_pp.STR=set_val;
			break;
		case STAT_STA:
			if(set_val>0)
				iss->sta=set_val;
			if(set_val<0)
				m_pp.STA=0;
			else if(set_val>255)
				m_pp.STA=255;
			else
				m_pp.STA=set_val;
			break;
		case STAT_AGI:
			if(set_val>0)
				iss->agi=set_val;
			if(set_val<0)
				m_pp.AGI=0;
			else if(set_val>255)
				m_pp.AGI=255;
			else
				m_pp.AGI=set_val;
			break;
		case STAT_DEX:
			if(set_val>0)
				iss->dex=set_val;
			if(set_val<0)
				m_pp.DEX=0;
			else if(set_val>255)
				m_pp.DEX=255;
			else
				m_pp.DEX=set_val;
			break;
		case STAT_INT:
			if(set_val>0)
				iss->int_=set_val;
			if(set_val<0)
				m_pp.INT=0;
			else if(set_val>255)
				m_pp.INT=255;
			else
				m_pp.INT=set_val;
			break;
		case STAT_WIS:
			if(set_val>0)
				iss->wis=set_val;
			if(set_val<0)
				m_pp.WIS=0;
			else if(set_val>255)
				m_pp.WIS=255;
			else
				m_pp.WIS=set_val;
			break;
		case STAT_CHA:
			if(set_val>0)
				iss->cha=set_val;
			if(set_val<0)
				m_pp.CHA=0;
			else if(set_val>255)
				m_pp.CHA=255;
			else
				m_pp.CHA=set_val;
		break;
	}
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::IncStats(uint8 type,int16 increase_val){
	if(type>STAT_DISEASE){
		printf("Error in Client::IncStats, received invalid type of: %i\n",type);
		return;
	}
	auto outapp = new EQApplicationPacket(OP_IncreaseStats, sizeof(IncreaseStat_Struct));
	IncreaseStat_Struct* iss=(IncreaseStat_Struct*)outapp->pBuffer;
	switch(type){
		case STAT_STR:
			if(increase_val>0)
				iss->str=increase_val;
			if((m_pp.STR+increase_val*2)<0)
				m_pp.STR=0;
			else if((m_pp.STR+increase_val*2)>255)
				m_pp.STR=255;
			else
				m_pp.STR+=increase_val*2;
			break;
		case STAT_STA:
			if(increase_val>0)
				iss->sta=increase_val;
			if((m_pp.STA+increase_val*2)<0)
				m_pp.STA=0;
			else if((m_pp.STA+increase_val*2)>255)
				m_pp.STA=255;
			else
				m_pp.STA+=increase_val*2;
			break;
		case STAT_AGI:
			if(increase_val>0)
				iss->agi=increase_val;
			if((m_pp.AGI+increase_val*2)<0)
				m_pp.AGI=0;
			else if((m_pp.AGI+increase_val*2)>255)
				m_pp.AGI=255;
			else
				m_pp.AGI+=increase_val*2;
			break;
		case STAT_DEX:
			if(increase_val>0)
				iss->dex=increase_val;
			if((m_pp.DEX+increase_val*2)<0)
				m_pp.DEX=0;
			else if((m_pp.DEX+increase_val*2)>255)
				m_pp.DEX=255;
			else
				m_pp.DEX+=increase_val*2;
			break;
		case STAT_INT:
			if(increase_val>0)
				iss->int_=increase_val;
			if((m_pp.INT+increase_val*2)<0)
				m_pp.INT=0;
			else if((m_pp.INT+increase_val*2)>255)
				m_pp.INT=255;
			else
				m_pp.INT+=increase_val*2;
			break;
		case STAT_WIS:
			if(increase_val>0)
				iss->wis=increase_val;
			if((m_pp.WIS+increase_val*2)<0)
				m_pp.WIS=0;
			else if((m_pp.WIS+increase_val*2)>255)
				m_pp.WIS=255;
			else
				m_pp.WIS+=increase_val*2;
			break;
		case STAT_CHA:
			if(increase_val>0)
				iss->cha=increase_val;
			if((m_pp.CHA+increase_val*2)<0)
				m_pp.CHA=0;
			else if((m_pp.CHA+increase_val*2)>255)
				m_pp.CHA=255;
			else
				m_pp.CHA+=increase_val*2;
			break;
	}
	QueuePacket(outapp);
	safe_delete(outapp);
}

const int32& Client::SetMana(int32 amount) {
	bool update = false;
	if (amount < 0)
		amount = 0;
	if (amount > GetMaxMana())
		amount = GetMaxMana();
	if (amount != current_mana)
		update = true;
	current_mana = amount;
	if (update)
		Mob::SetMana(amount);
	CheckManaEndUpdate();
	return current_mana;
}

void Client::CheckManaEndUpdate() {
	if (!Connected())
		return;

	if (last_reported_mana != current_mana || last_reported_endurance != current_endurance) {

		if (ClientVersion() >= EQ::versions::ClientVersion::SoD) {
			SendManaUpdate();
			SendEnduranceUpdate();
		}

		auto outapp = new EQApplicationPacket(OP_ManaChange, sizeof(ManaChange_Struct));
		ManaChange_Struct* mana_change = (ManaChange_Struct*)outapp->pBuffer;
		mana_change->new_mana = current_mana;
		mana_change->stamina = current_endurance;
		mana_change->spell_id = casting_spell_id;
		mana_change->keepcasting = 1;
		mana_change->padding[0] = 0;
		mana_change->padding[1] = 0;
		mana_change->padding[2] = 0;
		outapp->priority = 6;
		QueuePacket(outapp);
		safe_delete(outapp);

		/* Let others know when our mana percent has changed */
		if (this->GetManaPercent() != last_reported_mana_percent) {
			Group *group = this->GetGroup();
			Raid *raid = this->GetRaid();

			if (raid) {
				raid->SendManaPacketFrom(this);
			}
			else if (group) {
				group->SendManaPacketFrom(this);
			}

			auto mana_packet = new EQApplicationPacket(OP_ManaUpdate, sizeof(ManaUpdate_Struct));
			ManaUpdate_Struct* mana_update = (ManaUpdate_Struct*)mana_packet->pBuffer;
			mana_update->cur_mana = GetMana();
			mana_update->max_mana = GetMaxMana();
			mana_update->spawn_id = GetID();
			if ((ClientVersionBit() & EQ::versions::ClientVersionBitmask::maskSoDAndLater) != 0)
				QueuePacket(mana_packet); // do we need this with the OP_ManaChange packet above?
			entity_list.QueueClientsByXTarget(this, mana_packet, false, EQ::versions::ClientVersionBitmask::maskSoDAndLater);
			safe_delete(mana_packet);

			last_reported_mana_percent = this->GetManaPercent();
		}

		/* Let others know when our endurance percent has changed */
		if (this->GetEndurancePercent() != last_reported_endurance_percent) {
			Group *group = this->GetGroup();
			Raid *raid = this->GetRaid();

			if (raid) {
				raid->SendEndurancePacketFrom(this);
			}
			else if (group) {
				group->SendEndurancePacketFrom(this);
			}

			auto endurance_packet = new EQApplicationPacket(OP_EnduranceUpdate, sizeof(EnduranceUpdate_Struct));
			EnduranceUpdate_Struct* endurance_update = (EnduranceUpdate_Struct*)endurance_packet->pBuffer;
			endurance_update->cur_end = GetEndurance();
			endurance_update->max_end = GetMaxEndurance();
			endurance_update->spawn_id = GetID();
			if ((ClientVersionBit() & EQ::versions::ClientVersionBitmask::maskSoDAndLater) != 0)
				QueuePacket(endurance_packet); // do we need this with the OP_ManaChange packet above?
			entity_list.QueueClientsByXTarget(this, endurance_packet, false, EQ::versions::ClientVersionBitmask::maskSoDAndLater);
			safe_delete(endurance_packet);

			last_reported_endurance_percent = this->GetEndurancePercent();
		}

		last_reported_mana = current_mana;
		last_reported_endurance = current_endurance;
	}
}

// sends mana update to self
void Client::SendManaUpdate()
{
	auto mana_app = new EQApplicationPacket(OP_ManaUpdate, sizeof(ManaUpdate_Struct));
	ManaUpdate_Struct* mana_update = (ManaUpdate_Struct*)mana_app->pBuffer;
	mana_update->cur_mana = GetMana();
	mana_update->max_mana = GetMaxMana();
	mana_update->spawn_id = GetID();
	QueuePacket(mana_app);
	safe_delete(mana_app);
}

// sends endurance update to self
void Client::SendEnduranceUpdate()
{
	auto end_app = new EQApplicationPacket(OP_EnduranceUpdate, sizeof(EnduranceUpdate_Struct));
	EnduranceUpdate_Struct* endurance_update = (EnduranceUpdate_Struct*)end_app->pBuffer;
	endurance_update->cur_end = GetEndurance();
	endurance_update->max_end = GetMaxEndurance();
	endurance_update->spawn_id = GetID();
	QueuePacket(end_app);
	safe_delete(end_app);
}

void Client::FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho)
{
	Mob::FillSpawnStruct(ns, ForWho);

	// Populate client-specific spawn information
	ns->spawn.afk		= AFK;
	ns->spawn.lfg		= LFG; // afk and lfg are cleared on zoning on live
	ns->spawn.anon		= m_pp.anon;
	ns->spawn.gm		= GetGM() ? 1 : 0;
	ns->spawn.guildID	= GuildID();
//	ns->spawn.linkdead	= IsLD() ? 1 : 0;
//	ns->spawn.pvp		= GetPVP(false) ? 1 : 0;
	ns->spawn.show_name = true;

	strcpy(ns->spawn.title, m_pp.title);
	strcpy(ns->spawn.suffix, m_pp.suffix);

	if (IsBecomeNPC() == true)
		ns->spawn.NPC = 1;
	else if (ForWho == this)
		ns->spawn.NPC = 10;
	else
		ns->spawn.NPC = 0;
	ns->spawn.is_pet = 0;

	if (!IsInAGuild()) {
		ns->spawn.guildrank = 0xFF;
	} else {
		ns->spawn.guildrank = guild_mgr.GetDisplayedRank(GuildID(), GuildRank(), AccountID());
	}
	ns->spawn.size			= 0; // Changing size works, but then movement stops! (wth?)
	ns->spawn.runspeed		= (gmspeed == 0) ? runspeed : 3.125f;
	ns->spawn.showhelm = m_pp.showhelm ? 1 : 0;

	UpdateEquipmentLight();
	UpdateActiveLight();
	ns->spawn.light = m_Light.Type[EQ::lightsource::LightActive];
}

bool Client::GMHideMe(Client* client) {
	if (gm_hide_me) {
		if (client == 0)
			return true;
		else if (admin > client->Admin())
			return true;
		else
			return false;
	}
	else
		return false;
}

void Client::Duck() {
	SetAppearance(eaCrouching, false);
}

void Client::Stand() {
	SetAppearance(eaStanding, false);
}

void Client::ChangeLastName(const char* in_lastname) {
	memset(m_pp.last_name, 0, sizeof(m_pp.last_name));
	strn0cpy(m_pp.last_name, in_lastname, sizeof(m_pp.last_name));
	auto outapp = new EQApplicationPacket(OP_GMLastName, sizeof(GMLastName_Struct));
	GMLastName_Struct* gmn = (GMLastName_Struct*)outapp->pBuffer;
	strcpy(gmn->name, name);
	strcpy(gmn->gmname, name);
	strcpy(gmn->lastname, in_lastname);
	gmn->unknown[0]=1;
	gmn->unknown[1]=1;
	gmn->unknown[2]=1;
	gmn->unknown[3]=1;
	entity_list.QueueClients(this, outapp, false);
	// Send name update packet here... once know what it is
	safe_delete(outapp);
}

bool Client::ChangeFirstName(const char* in_firstname, const char* gmname)
{
	// check duplicate name
	bool usedname = database.CheckUsedName((const char*) in_firstname);
	if (!usedname) {
		return false;
	}

	// update character_
	if(!database.UpdateName(GetName(), in_firstname))
		return false;

	// update pp
	memset(m_pp.name, 0, sizeof(m_pp.name));
	snprintf(m_pp.name, sizeof(m_pp.name), "%s", in_firstname);
	strcpy(name, m_pp.name);
	Save();

	// send name update packet
	auto outapp = new EQApplicationPacket(OP_GMNameChange, sizeof(GMName_Struct));
	GMName_Struct* gmn=(GMName_Struct*)outapp->pBuffer;
	strn0cpy(gmn->gmname,gmname,64);
	strn0cpy(gmn->oldname,GetName(),64);
	strn0cpy(gmn->newname,in_firstname,64);
	gmn->unknown[0] = 1;
	gmn->unknown[1] = 1;
	gmn->unknown[2] = 1;
	entity_list.QueueClients(this, outapp, false);
	safe_delete(outapp);

	// finally, update the /who list
	UpdateWho();

	// success
	return true;
}

void Client::SetGM(bool toggle) {
	m_pp.gm = toggle ? 1 : 0;
	m_inv.SetGMInventory((bool)m_pp.gm);
	Message(Chat::Red, "You are %s a GM.", m_pp.gm ? "now" : "no longer");
	SendAppearancePacket(AT_GM, m_pp.gm);
	Save();
	UpdateWho();
}

void Client::ReadBook(BookRequest_Struct *book) {
	int16 book_language=0;
	char *txtfile = book->txtfile;

	if(txtfile[0] == '0' && txtfile[1] == '\0') {
		//invalid book... coming up on non-book items.
		return;
	}

	std::string booktxt2 = content_db.GetBook(txtfile, &book_language);
	int length = booktxt2.length();

	if (booktxt2[0] != '\0') {
#if EQDEBUG >= 6
		LogInfo("Client::ReadBook() textfile:[{}] Text:[{}]", txtfile, booktxt2.c_str());
#endif
		auto outapp = new EQApplicationPacket(OP_ReadBook, length + sizeof(BookText_Struct));

		BookText_Struct *out = (BookText_Struct *) outapp->pBuffer;
		out->window = book->window;


		if (ClientVersion() >= EQ::versions::ClientVersion::SoF) {
			// Find out what slot the book was read from.
			// SoF+ need to look up book type for the output message.
			int16	read_from_slot;

			if (book->subslot >= 0) {
				uint16 offset;
				offset = (book->invslot-23) * 10;	// How many packs to skip.
				read_from_slot = 251 + offset + book->subslot;
			}
			else {
				read_from_slot = book->invslot -1;
			}

			const EQ::ItemInstance *inst = nullptr;

			if (read_from_slot <= EQ::invbag::GENERAL_BAGS_END)
				{
				inst = m_inv[read_from_slot];
				}

			if(inst)
				out->type = inst->GetItem()->Book;
			else
				out->type = book->type;
		}
		else {
			out->type = book->type;
		}
		out->invslot = book->invslot;

		memcpy(out->booktext, booktxt2.c_str(), length);

		if (book_language > 0 && book_language < MAX_PP_LANGUAGE) {
			if (m_pp.languages[book_language] < 100) {
				GarbleMessage(out->booktext, (100 - m_pp.languages[book_language]));
			}
		}

		QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Client::QuestReadBook(const char* text, uint8 type) {
	std::string booktxt2 = text;
	int length = booktxt2.length();
	if (booktxt2[0] != '\0') {
		auto outapp = new EQApplicationPacket(OP_ReadBook, length + sizeof(BookText_Struct));
		BookText_Struct *out = (BookText_Struct *) outapp->pBuffer;
		out->window = 0xFF;
		out->type = type;
		out->invslot = 0;
		memcpy(out->booktext, booktxt2.c_str(), length);
		QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Client::SendClientMoneyUpdate(uint8 type,uint32 amount){
	auto outapp = new EQApplicationPacket(OP_TradeMoneyUpdate, sizeof(TradeMoneyUpdate_Struct));
	TradeMoneyUpdate_Struct* mus= (TradeMoneyUpdate_Struct*)outapp->pBuffer;
	mus->amount=amount;
	mus->trader=0;
	mus->type=type;
	QueuePacket(outapp);
	safe_delete(outapp);
}

bool Client::TakeMoneyFromPP(uint64 copper, bool updateclient) {
	int64 copperpp,silver,gold,platinum;
	copperpp = m_pp.copper;
	silver = static_cast<int64>(m_pp.silver) * 10;
	gold = static_cast<int64>(m_pp.gold) * 100;
	platinum = static_cast<int64>(m_pp.platinum) * 1000;

	int64 clienttotal = copperpp + silver + gold + platinum;

	clienttotal -= copper;
	if(clienttotal < 0)
	{
		return false; // Not enough money!
	}
	else
	{
		copperpp -= copper;
		if(copperpp <= 0)
		{
			copper = std::abs(copperpp);
			m_pp.copper = 0;
		}
		else
		{
			m_pp.copper = copperpp;
			if(updateclient)
				SendMoneyUpdate();
			SaveCurrency();
			return true;
		}
		silver -= copper;
		if(silver <= 0)
		{
			copper = std::abs(silver);
			m_pp.silver = 0;
		}
		else
		{
			m_pp.silver = silver/10;
			m_pp.copper += (silver-(m_pp.silver*10));
			if(updateclient)
				SendMoneyUpdate();
			SaveCurrency();
			return true;
		}

		gold -=copper;

		if(gold <= 0)
		{
			copper = std::abs(gold);
			m_pp.gold = 0;
		}
		else
		{
			m_pp.gold = gold/100;
			uint64 silvertest = (gold-(static_cast<uint64>(m_pp.gold)*100))/10;
			m_pp.silver += silvertest;
			uint64 coppertest = (gold-(static_cast<uint64>(m_pp.gold)*100+silvertest*10));
			m_pp.copper += coppertest;
			if(updateclient)
				SendMoneyUpdate();
			SaveCurrency();
			return true;
		}

		platinum -= copper;

		//Impossible for plat to be negative, already checked above

		m_pp.platinum = platinum/1000;
		uint64 goldtest = (platinum-(static_cast<uint64>(m_pp.platinum)*1000))/100;
		m_pp.gold += goldtest;
		uint64 silvertest = (platinum-(static_cast<uint64>(m_pp.platinum)*1000+goldtest*100))/10;
		m_pp.silver += silvertest;
		uint64 coppertest = (platinum-(static_cast<uint64>(m_pp.platinum)*1000+goldtest*100+silvertest*10));
		m_pp.copper = coppertest;
		if(updateclient)
			SendMoneyUpdate();
		RecalcWeight();
		SaveCurrency();
		return true;
	}
}

void Client::AddMoneyToPP(uint64 copper, bool updateclient){
	uint64 tmp;
	uint64 tmp2;
	tmp = copper;

	/* Add Amount of Platinum */
	tmp2 = tmp/1000;
	int32 new_val = m_pp.platinum + tmp2;
	if(new_val < 0) { m_pp.platinum = 0; }
	else { m_pp.platinum = m_pp.platinum + tmp2; }
	tmp-=tmp2*1000;

	//if (updateclient)
	//	SendClientMoneyUpdate(3,tmp2);

	/* Add Amount of Gold */
	tmp2 = tmp/100;
	new_val = m_pp.gold + tmp2;
	if(new_val < 0) { m_pp.gold = 0; }
	else { m_pp.gold = m_pp.gold + tmp2; }

	tmp-=tmp2*100;
	//if (updateclient)
	//	SendClientMoneyUpdate(2,tmp2);

	/* Add Amount of Silver */
	tmp2 = tmp/10;
	new_val = m_pp.silver + tmp2;
	if(new_val < 0) {
		m_pp.silver = 0;
	} else {
		m_pp.silver = m_pp.silver + tmp2;
	}
	tmp-=tmp2*10;
	//if (updateclient)
	//	SendClientMoneyUpdate(1,tmp2);

	// Add Copper
	//tmp	= tmp - (tmp2* 10);
	//if (updateclient)
	//	SendClientMoneyUpdate(0,tmp);
	tmp2 = tmp;
	new_val = m_pp.copper + tmp2;
	if(new_val < 0) {
		m_pp.copper = 0;
	} else {
		m_pp.copper = m_pp.copper + tmp2;
	}


	//send them all at once, since the above code stopped working.
	if(updateclient)
		SendMoneyUpdate();

	RecalcWeight();

	SaveCurrency();

	LogDebug("Client::AddMoneyToPP() [{}] should have: plat:[{}] gold:[{}] silver:[{}] copper:[{}]", GetName(), m_pp.platinum, m_pp.gold, m_pp.silver, m_pp.copper);
}

void Client::EVENT_ITEM_ScriptStopReturn(){
	/* Set a timestamp in an entity variable for plugin check_handin.pl in return_items
		This will stopgap players from items being returned if global_npc.pl has a catch all return_items
	*/
	struct timeval read_time;
	char buffer[50];
	gettimeofday(&read_time, 0);
	sprintf(buffer, "%li.%li \n", read_time.tv_sec, read_time.tv_usec);
	this->SetEntityVariable("Stop_Return", buffer);
}

void Client::AddMoneyToPP(uint32 copper, uint32 silver, uint32 gold, uint32 platinum, bool updateclient){
	this->EVENT_ITEM_ScriptStopReturn();

	int32 new_value = m_pp.platinum + platinum;
	if(new_value >= 0 && new_value > m_pp.platinum)
		m_pp.platinum += platinum;

	new_value = m_pp.gold + gold;
	if(new_value >= 0 && new_value > m_pp.gold)
		m_pp.gold += gold;

	new_value = m_pp.silver + silver;
	if(new_value >= 0 && new_value > m_pp.silver)
		m_pp.silver += silver;

	new_value = m_pp.copper + copper;
	if(new_value >= 0 && new_value > m_pp.copper)
		m_pp.copper += copper;

	if(updateclient)
		SendMoneyUpdate();

	RecalcWeight();
	SaveCurrency();

#if (EQDEBUG>=5)
		LogDebug("Client::AddMoneyToPP() [{}] should have: plat:[{}] gold:[{}] silver:[{}] copper:[{}]",
			GetName(), m_pp.platinum, m_pp.gold, m_pp.silver, m_pp.copper);
#endif
}

void Client::SendMoneyUpdate() {
	auto outapp = new EQApplicationPacket(OP_MoneyUpdate, sizeof(MoneyUpdate_Struct));
	MoneyUpdate_Struct* mus= (MoneyUpdate_Struct*)outapp->pBuffer;

	mus->platinum = m_pp.platinum;
	mus->gold = m_pp.gold;
	mus->silver = m_pp.silver;
	mus->copper = m_pp.copper;

	FastQueuePacket(&outapp);
}

bool Client::HasMoney(uint64 Copper) {

	if ((static_cast<uint64>(m_pp.copper) +
		(static_cast<uint64>(m_pp.silver) * 10) +
		(static_cast<uint64>(m_pp.gold) * 100) +
		(static_cast<uint64>(m_pp.platinum) * 1000)) >= Copper)
		return true;

	return false;
}

uint64 Client::GetCarriedMoney() {

	return ((static_cast<uint64>(m_pp.copper) +
		(static_cast<uint64>(m_pp.silver) * 10) +
		(static_cast<uint64>(m_pp.gold) * 100) +
		(static_cast<uint64>(m_pp.platinum) * 1000)));
}

uint64 Client::GetAllMoney() {

	return (
		(static_cast<uint64>(m_pp.copper) +
		(static_cast<uint64>(m_pp.silver) * 10) +
		(static_cast<uint64>(m_pp.gold) * 100) +
		(static_cast<uint64>(m_pp.platinum) * 1000) +
		(static_cast<uint64>(m_pp.copper_bank) +
		(static_cast<uint64>(m_pp.silver_bank) * 10) +
		(static_cast<uint64>(m_pp.gold_bank) * 100) +
		(static_cast<uint64>(m_pp.platinum_bank) * 1000) +
		(static_cast<uint64>(m_pp.copper_cursor) +
		(static_cast<uint64>(m_pp.silver_cursor) * 10) +
		(static_cast<uint64>(m_pp.gold_cursor) * 100) +
		(static_cast<uint64>(m_pp.platinum_cursor) * 1000) +
		(static_cast<uint64>(m_pp.platinum_shared) * 1000)))));
}

bool Client::CheckIncreaseSkill(EQ::skills::SkillType skillid, Mob *against_who, int chancemodi) {
	if (IsDead() || IsUnconscious())
		return false;
	if (IsAIControlled()) // no skillups while chamred =p
		return false;
	if (skillid > EQ::skills::HIGHEST_SKILL)
		return false;
	int skillval = GetRawSkill(skillid);
	int maxskill = GetMaxSkillAfterSpecializationRules(skillid, MaxSkill(skillid));
	char buffer[24] = { 0 };
	snprintf(buffer, 23, "%d %d", skillid, skillval);
	parse->EventPlayer(EVENT_USE_SKILL, this, buffer, 0);
	if (against_who) {
		if (
			against_who->GetSpecialAbility(IMMUNE_AGGRO) || 
			against_who->GetSpecialAbility(IMMUNE_AGGRO_CLIENT) || 
			against_who->IsClient() || 
			GetLevelCon(against_who->GetLevel()) == CON_GRAY
		) {
			//false by default
			if (!mod_can_increase_skill(skillid, against_who)) {
				return false;
			}
		}
	}

	// Make sure we're not already at skill cap
	if (skillval < maxskill)
	{
		// the higher your current skill level, the harder it is
		int32 Chance = 10 + chancemodi + ((252 - skillval) / 20);

		Chance = (Chance * RuleI(Character, SkillUpModifier) / 100);

		Chance = mod_increase_skill_chance(Chance, against_who);

		if(Chance < 1)
			Chance = 1; // Make it always possible

		if(zone->random.Real(0, 99) < Chance)
		{
			SetSkill(skillid, GetRawSkill(skillid) + 1);
			LogSkills("Skill [{}] at value [{}] successfully gain with [{}]% chance (mod [{}])", skillid, skillval, Chance, chancemodi);
			return true;
		} else {
			LogSkills("Skill [{}] at value [{}] failed to gain with [{}]% chance (mod [{}])", skillid, skillval, Chance, chancemodi);
		}
	} else {
		LogSkills("Skill [{}] at value [{}] cannot increase due to maxmum [{}]", skillid, skillval, maxskill);
	}
	return false;
}

void Client::CheckLanguageSkillIncrease(uint8 langid, uint8 TeacherSkill) {
	if (IsDead() || IsUnconscious())
		return;
	if (IsAIControlled())
		return;
	if (langid >= MAX_PP_LANGUAGE)
		return;		// do nothing if langid is an invalid language

	int LangSkill = m_pp.languages[langid];		// get current language skill

	if (LangSkill < 100) {	// if the language isn't already maxed
		int32 Chance = 5 + ((TeacherSkill - LangSkill)/10);	// greater chance to learn if teacher's skill is much higher than yours
		Chance = (Chance * RuleI(Character, SkillUpModifier)/100);

		if(zone->random.Real(0,100) < Chance) {	// if they make the roll
			IncreaseLanguageSkill(langid);	// increase the language skill by 1
			LogSkills("Language [{}] at value [{}] successfully gain with [{}] % chance", langid, LangSkill, Chance);
		}
		else
			LogSkills("Language [{}] at value [{}] failed to gain with [{}] % chance", langid, LangSkill, Chance);
	}
}

bool Client::HasSkill(EQ::skills::SkillType skill_id) const {
	return((GetSkill(skill_id) > 0) && CanHaveSkill(skill_id));
}

bool Client::CanHaveSkill(EQ::skills::SkillType skill_id) const {
	if (ClientVersion() < EQ::versions::ClientVersion::RoF2 && class_ == BERSERKER && skill_id == EQ::skills::Skill1HPiercing)
		skill_id = EQ::skills::Skill2HPiercing;

	return(content_db.GetSkillCap(GetClass(), skill_id, RuleI(Character, MaxLevel)) > 0);
	//if you don't have it by max level, then odds are you never will?
}

uint16 Client::MaxSkill(EQ::skills::SkillType skillid, uint16 class_, uint16 level) const {
	if (ClientVersion() < EQ::versions::ClientVersion::RoF2 && class_ == BERSERKER && skillid == EQ::skills::Skill1HPiercing)
		skillid = EQ::skills::Skill2HPiercing;

	return(content_db.GetSkillCap(class_, skillid, level));
}

uint8 Client::SkillTrainLevel(EQ::skills::SkillType skillid, uint16 class_)
{
	if (ClientVersion() < EQ::versions::ClientVersion::RoF2 && class_ == BERSERKER && skillid == EQ::skills::Skill1HPiercing)
		skillid = EQ::skills::Skill2HPiercing;

	return(content_db.GetTrainLevel(class_, skillid, RuleI(Character, MaxLevel)));
}

uint16 Client::GetMaxSkillAfterSpecializationRules(EQ::skills::SkillType skillid, uint16 maxSkill)
{
	uint16 Result = maxSkill;

	uint16 PrimarySpecialization = 0, SecondaryForte = 0;

	uint16 PrimarySkillValue = 0, SecondarySkillValue = 0;

	uint16 MaxSpecializations = GetAA(aaSecondaryForte) ? 2 : 1;

	if (skillid >= EQ::skills::SkillSpecializeAbjure && skillid <= EQ::skills::SkillSpecializeEvocation)
	{
		bool HasPrimarySpecSkill = false;

		int NumberOfPrimarySpecSkills = 0;

		for (int i = EQ::skills::SkillSpecializeAbjure; i <= EQ::skills::SkillSpecializeEvocation; ++i)
		{
			if(m_pp.skills[i] > 50)
			{
				HasPrimarySpecSkill = true;
				NumberOfPrimarySpecSkills++;
			}
			if(m_pp.skills[i] > PrimarySkillValue)
			{
				if(PrimarySkillValue > SecondarySkillValue)
				{
					SecondarySkillValue = PrimarySkillValue;
					SecondaryForte = PrimarySpecialization;
				}

				PrimarySpecialization = i;
				PrimarySkillValue = m_pp.skills[i];
			}
			else if(m_pp.skills[i] > SecondarySkillValue)
			{
				SecondaryForte = i;
				SecondarySkillValue = m_pp.skills[i];
			}
		}

		if(SecondarySkillValue <=50)
			SecondaryForte = 0;

		if(HasPrimarySpecSkill)
		{
			if(NumberOfPrimarySpecSkills <= MaxSpecializations)
			{
				if(MaxSpecializations == 1)
				{
					if(skillid != PrimarySpecialization)
					{
						Result = 50;
					}
				}
				else
				{
					if((skillid != PrimarySpecialization) && ((skillid == SecondaryForte) || (SecondaryForte == 0)))
					{
						if((PrimarySkillValue > 100) || (!SecondaryForte))
							Result = 100;
					}
					else if(skillid != PrimarySpecialization)
					{
						Result = 50;
					}
				}
			}
			else
			{
				Message(Chat::Red, "Your spell casting specializations skills have been reset. "
						"Only %i primary specialization skill is allowed.", MaxSpecializations);

				for (int i = EQ::skills::SkillSpecializeAbjure; i <= EQ::skills::SkillSpecializeEvocation; ++i)
					SetSkill((EQ::skills::SkillType)i, 1);

				Save();

				LogInfo("Reset [{}]'s caster specialization skills to 1"
								"Too many specializations skills were above 50.", GetCleanName());
			}

		}
	}

	Result += spellbonuses.RaiseSkillCap[skillid] + itembonuses.RaiseSkillCap[skillid] + aabonuses.RaiseSkillCap[skillid];

	if (skillid == EQ::skills::SkillType::SkillForage)
		Result += aabonuses.GrantForage;

	return Result;
}

void Client::SetPVP(bool toggle, bool message) {
	m_pp.pvp = toggle ? 1 : 0;

	if (message) {
		if(GetPVP())
			this->MessageString(Chat::Shout,PVP_ON);
		else
			Message(Chat::Red, "You no longer follow the ways of discord.");
	}

	SendAppearancePacket(AT_PVP, GetPVP());
	Save();
}

void Client::Kick(const std::string &reason) {
	client_state = CLIENT_KICKED;

	LogClientLogin("Client [{}] kicked, reason [{}]", GetCleanName(), reason.c_str());
}

void Client::WorldKick() {
	auto outapp = new EQApplicationPacket(OP_GMKick, sizeof(GMKick_Struct));
	GMKick_Struct* gmk = (GMKick_Struct *)outapp->pBuffer;
	strcpy(gmk->name,GetName());
	QueuePacket(outapp);
	safe_delete(outapp);
	Kick("World kick issued");
}

void Client::GMKill() {
	auto outapp = new EQApplicationPacket(OP_GMKill, sizeof(GMKill_Struct));
	GMKill_Struct* gmk = (GMKill_Struct *)outapp->pBuffer;
	strcpy(gmk->name,GetName());
	QueuePacket(outapp);
	safe_delete(outapp);
}

bool Client::CheckAccess(int16 iDBLevel, int16 iDefaultLevel) {
	if ((admin >= iDBLevel) || (iDBLevel == 255 && admin >= iDefaultLevel))
		return true;
	else
		return false;
}

void Client::MemorizeSpell(uint32 slot,uint32 spellid,uint32 scribing){
	if (slot < 0 || slot >= EQ::spells::DynamicLookup(ClientVersion(), GetGM())->SpellbookSize)
		return;
	if ((spellid < 3 || spellid > EQ::spells::DynamicLookup(ClientVersion(), GetGM())->SpellIdMax) && spellid != 0xFFFFFFFF)
		return;

	auto outapp = new EQApplicationPacket(OP_MemorizeSpell, sizeof(MemorizeSpell_Struct));
	MemorizeSpell_Struct* mss=(MemorizeSpell_Struct*)outapp->pBuffer;
	mss->scribing=scribing;
	mss->slot=slot;
	mss->spell_id=spellid;
	outapp->priority = 5;
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SetFeigned(bool in_feigned) {
	if (in_feigned)
	{
		if(RuleB(Character, FeignKillsPet))
		{
			SetPet(0);
		}
		SetHorseId(0);
		entity_list.ClearFeignAggro(this);
		forget_timer.Start(FeignMemoryDuration);
	} else {
		forget_timer.Disable();
	}
	feigned=in_feigned;
 }

void Client::LogMerchant(Client* player, Mob* merchant, uint32 quantity, uint32 price, const EQ::ItemData* item, bool buying)
{
	if(!player || !merchant || !item)
		return;

	std::string LogText = "Qty: ";

	char Buffer[255];
	memset(Buffer, 0, sizeof(Buffer));

	snprintf(Buffer, sizeof(Buffer)-1, "%3i", quantity);
	LogText += Buffer;
	snprintf(Buffer, sizeof(Buffer)-1, "%10i", price);
	LogText += " TotalValue: ";
	LogText += Buffer;
	snprintf(Buffer, sizeof(Buffer)-1, " ItemID: %7i", item->ID);
	LogText += Buffer;
	LogText += " ";
	snprintf(Buffer, sizeof(Buffer)-1, " %s", item->Name);
	LogText += Buffer;

	if (buying==true) {
		database.logevents(player->AccountName(),player->AccountID(),player->admin,player->GetName(),merchant->GetName(),"Buying from Merchant",LogText.c_str(),2);
	}
	else {
		database.logevents(player->AccountName(),player->AccountID(),player->admin,player->GetName(),merchant->GetName(),"Selling to Merchant",LogText.c_str(),3);
	}
}

void Client::Disarm(Client* disarmer, int chance) {
	int16 slot = EQ::invslot::SLOT_INVALID;
	const EQ::ItemInstance *inst = this->GetInv().GetItem(EQ::invslot::slotPrimary);
	if (inst && inst->IsWeapon()) {
		slot = EQ::invslot::slotPrimary;
	}
	else {
		inst = this->GetInv().GetItem(EQ::invslot::slotSecondary);
		if (inst && inst->IsWeapon())
			slot = EQ::invslot::slotSecondary;
	}
	if (slot != EQ::invslot::SLOT_INVALID && inst->IsClassCommon()) {
		// We have an item that can be disarmed.
		if (zone->random.Int(0, 1000) <= chance) {
			// Find a free inventory slot
			int16 slot_id = EQ::invslot::SLOT_INVALID;
			slot_id = m_inv.FindFreeSlot(false, true, inst->GetItem()->Size, (inst->GetItem()->ItemType == EQ::item::ItemTypeArrow));
			if (slot_id != EQ::invslot::SLOT_INVALID)
			{
				EQ::ItemInstance *InvItem = m_inv.PopItem(slot);
				if (InvItem) { // there should be no way it is not there, but check anyway
					EQApplicationPacket* outapp = new EQApplicationPacket(OP_MoveItem, sizeof(MoveItem_Struct));
					MoveItem_Struct* mi = (MoveItem_Struct*)outapp->pBuffer;
					mi->from_slot = slot;
					mi->to_slot = 0xFFFFFFFF;
					if (inst->IsStackable()) // it should not be stackable
						mi->number_in_stack = inst->GetCharges();
					else
						mi->number_in_stack = 0;
					FastQueuePacket(&outapp); // this deletes item from the weapon slot on the client
					if (PutItemInInventory(slot_id, *InvItem, true))
						database.SaveInventory(this->CharacterID(), NULL, slot);
					auto matslot = (slot == EQ::invslot::slotPrimary ? EQ::textures::weaponPrimary : EQ::textures::weaponSecondary);
					if (matslot != EQ::textures::materialInvalid)
						SendWearChange(matslot);
				}
				MessageString(Chat::Skills, DISARMED);
				if (disarmer != this)
					disarmer->MessageString(Chat::Skills, DISARM_SUCCESS, this->GetCleanName());
				if (chance != 1000)
					disarmer->CheckIncreaseSkill(EQ::skills::SkillDisarm, nullptr, 4);
				CalcBonuses();
				// CalcEnduranceWeightFactor();
				return;
			}
			disarmer->MessageString(Chat::Skills, DISARM_FAILED);
			if (chance != 1000)
				disarmer->CheckIncreaseSkill(EQ::skills::SkillDisarm, nullptr, 2);
			return;
		}
	}
	disarmer->MessageString(Chat::Skills, DISARM_FAILED);
}

bool Client::BindWound(Mob *bindmob, bool start, bool fail)
{
	EQApplicationPacket *outapp = nullptr;
	if (!fail) {
		outapp = new EQApplicationPacket(OP_Bind_Wound, sizeof(BindWound_Struct));
		BindWound_Struct *bind_out = (BindWound_Struct *)outapp->pBuffer;
		// Start bind
		if (!bindwound_timer.Enabled()) {
			// make sure we actually have a bandage... and consume it.
			int16 bslot = m_inv.HasItemByUse(EQ::item::ItemTypeBandage, 1, invWhereWorn | invWherePersonal);
			if (bslot == INVALID_INDEX) {
				bind_out->type = 3;
				QueuePacket(outapp);
				bind_out->type = 7; // this is the wrong message, dont know the right one.
				QueuePacket(outapp);
				safe_delete(outapp);
				return (true);
			}
			DeleteItemInInventory(bslot, 1, true); // do we need client update?

			// start complete timer
			bindwound_timer.Start(10000);
			bindwound_target = bindmob;

			// Send client unlock
			bind_out->type = 3;
			QueuePacket(outapp);
			bind_out->type = 0;
			// Client Unlocked
			if (!bindmob) {
				// send "bindmob dead" to client
				bind_out->type = 4;
				QueuePacket(outapp);
				bind_out->type = 0;
				bindwound_timer.Disable();
				bindwound_target = 0;
			} else {
				// send bindmob "stand still"
				if (!bindmob->IsAIControlled() && bindmob != this) {
					bindmob->CastToClient()->MessageString(Chat::Yellow,
										  YOU_ARE_BEING_BANDAGED);
				} else if (bindmob->IsAIControlled() && bindmob != this) {
					; // Tell IPC to stand still?
				} else {
					; // Binding self
				}
			}
		} else if (bindwound_timer.Check()) // Did the timer finish?
		{
			// finish bind
			// disable complete timer
			bindwound_timer.Disable();
			bindwound_target = 0;
			if (!bindmob) {
				// send "bindmob gone" to client
				bind_out->type = 5; // not in zone
				QueuePacket(outapp);
				bind_out->type = 0;
			}

			else {
				if (!GetFeigned() && (DistanceSquared(bindmob->GetPosition(), m_Position) <= 400)) {
					// send bindmob bind done
					if (!bindmob->IsAIControlled() && bindmob != this) {

					} else if (bindmob->IsAIControlled() && bindmob != this) {
						// Tell IPC to resume??
					} else {
						// Binding self
					}
					// Send client bind done

					bind_out->type = 1; // Done
					QueuePacket(outapp);
					bind_out->type = 0;
					CheckIncreaseSkill(EQ::skills::SkillBindWound, nullptr, 5);

					if (RuleB(Character, UseOldBindWound)) {
						int maxHPBonus = spellbonuses.MaxBindWound + itembonuses.MaxBindWound +
							aabonuses.MaxBindWound;

						int max_percent = 50 + maxHPBonus;

						if (GetClass() == MONK && GetSkill(EQ::skills::SkillBindWound) > 200) {
							max_percent = 70 + maxHPBonus;
						}

						max_percent = mod_bindwound_percent(max_percent, bindmob);

						int max_hp = bindmob->GetMaxHP() * max_percent / 100;

						// send bindmob new hp's
						if (bindmob->GetHP() < bindmob->GetMaxHP() && bindmob->GetHP() <= (max_hp)-1) {
							// 0.120 per skill point, 0.60 per skill level, minimum 3 max 30
							int bindhps = 3;

							if (GetSkill(EQ::skills::SkillBindWound) > 200) {
								bindhps += GetSkill(EQ::skills::SkillBindWound) * 4 / 10;
							}
							else if (GetSkill(EQ::skills::SkillBindWound) >= 10) {
								bindhps += GetSkill(EQ::skills::SkillBindWound) / 4;
							}

							// Implementation of aaMithanielsBinding is a guess (the multiplier)
							int bindBonus = spellbonuses.BindWound + itembonuses.BindWound +
								aabonuses.BindWound;

							bindhps += bindhps * bindBonus / 100;

							bindhps = mod_bindwound_hp(bindhps, bindmob);

							// if the bind takes them above the max bindable
							// cap it at that value. Dont know if live does it this way
							// but it makes sense to me.
							int chp = bindmob->GetHP() + bindhps;
							if (chp > max_hp)
								chp = max_hp;

							bindmob->SetHP(chp);
							bindmob->SendHPUpdate();
						}
						else {
							// I dont have the real, live
							Message(Chat::Yellow, "You cannot bind wounds above %d%% hitpoints.",
								max_percent);
							if (bindmob != this && bindmob->IsClient())
								bindmob->CastToClient()->Message(
								15,
								"You cannot have your wounds bound above %d%% hitpoints.",
								max_percent);
							// Too many hp message goes here.
						}
					}
					else {
						int percent_base = 50;
						if (GetRawSkill(EQ::skills::SkillBindWound) > 200) {
							if ((GetClass() == MONK) || (GetClass() == BEASTLORD))
								percent_base = 70;
							else if ((GetLevel() > 50) && ((GetClass() == WARRIOR) || (GetClass() == ROGUE) || (GetClass() == CLERIC)))
								percent_base = 70;
						}

						int percent_bonus = 0;
						if (percent_base >= 70)
							percent_bonus = spellbonuses.MaxBindWound + itembonuses.MaxBindWound + aabonuses.MaxBindWound;

						int max_percent = percent_base + percent_bonus;
						if (max_percent < 0)
							max_percent = 0;
						if (max_percent > 100)
							max_percent = 100;

						max_percent = mod_bindwound_percent(max_percent, bindmob);

						int max_hp = (bindmob->GetMaxHP() * max_percent) / 100;
						if (max_hp > bindmob->GetMaxHP())
							max_hp = bindmob->GetMaxHP();

						if (bindmob->GetHP() < bindmob->GetMaxHP() && bindmob->GetHP() < max_hp) {
							int bindhps = 3; // base bind hp
							if (percent_base >= 70)
								bindhps = (GetSkill(EQ::skills::SkillBindWound) * 4) / 10; // 8:5 skill-to-hp ratio
							else if (GetSkill(EQ::skills::SkillBindWound) >= 12)
								bindhps = GetSkill(EQ::skills::SkillBindWound) / 4; // 4:1 skill-to-hp ratio

							int bonus_hp_percent = 0;
							if (percent_base >= 70)
								bonus_hp_percent = spellbonuses.BindWound + itembonuses.BindWound + aabonuses.BindWound;

							bindhps += (bindhps * bonus_hp_percent) / 100;

							if (bindhps < 3)
								bindhps = 3;

							bindhps = mod_bindwound_hp(bindhps, bindmob);

							bindhps += bindmob->GetHP();
							if (bindhps > max_hp)
								bindhps = max_hp;

							bindmob->SetHP(bindhps);
							bindmob->SendHPUpdate();
						}
						else {
							Message(Chat::Yellow, "You cannot bind wounds above %d%% hitpoints", max_percent);
							if (bindmob != this && bindmob->IsClient())
								bindmob->CastToClient()->Message(Chat::Yellow, "You cannot have your wounds bound above %d%% hitpoints", max_percent);
						}
					}
				}
				else {
					// Send client bind failed
					if (bindmob != this)
						bind_out->type = 6; // They moved
					else
						bind_out->type = 7; // Bandager moved

					QueuePacket(outapp);
					bind_out->type = 0;
				}
			}
		}
	} else if (bindwound_timer.Enabled()) {
		// You moved
		outapp = new EQApplicationPacket(OP_Bind_Wound, sizeof(BindWound_Struct));
		BindWound_Struct *bind_out = (BindWound_Struct *)outapp->pBuffer;
		bindwound_timer.Disable();
		bindwound_target = 0;
		bind_out->type = 7;
		QueuePacket(outapp);
		bind_out->type = 3;
		QueuePacket(outapp);
	}
	safe_delete(outapp);
	return true;
}

void Client::SetMaterial(int16 in_slot, uint32 item_id)
{
	const EQ::ItemData *item = database.GetItem(item_id);
	if (item && item->IsClassCommon()) {
		uint8 matslot = EQ::InventoryProfile::CalcMaterialFromSlot(in_slot);
		if (matslot != EQ::textures::materialInvalid) {
			m_pp.item_material.Slot[matslot].Material = GetEquipmentMaterial(matslot);
		}
	}
}

void Client::ServerFilter(SetServerFilter_Struct* filter){

/*	this code helps figure out the filter IDs in the packet if needed
	static SetServerFilter_Struct ssss;
	int r;
	uint32 *o = (uint32 *) &ssss;
	uint32 *n = (uint32 *) filter;
	for(r = 0; r < (sizeof(SetServerFilter_Struct)/4); r++) {
		if(*o != *n)
			LogFile->write(EQEMuLog::Debug, "Filter %d changed from %d to %d", r, *o, *n);
		o++; n++;
	}
	memcpy(&ssss, filter, sizeof(SetServerFilter_Struct));
*/
#define Filter0(type) \
	if(filter->filters[type] == 1) \
		ClientFilters[type] = FilterShow; \
	else \
		ClientFilters[type] = FilterHide;
#define Filter1(type) \
	if(filter->filters[type] == 0) \
		ClientFilters[type] = FilterShow; \
	else \
		ClientFilters[type] = FilterHide;

	Filter0(FilterGuildChat);
	Filter0(FilterSocials);
	Filter0(FilterGroupChat);
	Filter0(FilterShouts);
	Filter0(FilterAuctions);
	Filter0(FilterOOC);
	Filter0(FilterBadWords);

	if(filter->filters[FilterPCSpells] == 0)
		ClientFilters[FilterPCSpells] = FilterShow;
	else if(filter->filters[FilterPCSpells] == 1)
		ClientFilters[FilterPCSpells] = FilterHide;
	else
		ClientFilters[FilterPCSpells] = FilterShowGroupOnly;

	Filter1(FilterNPCSpells);

	if(filter->filters[FilterBardSongs] == 0)
		ClientFilters[FilterBardSongs] = FilterShow;
	else if(filter->filters[FilterBardSongs] == 1)
		ClientFilters[FilterBardSongs] = FilterShowSelfOnly;
	else if(filter->filters[FilterBardSongs] == 2)
		ClientFilters[FilterBardSongs] = FilterShowGroupOnly;
	else
		ClientFilters[FilterBardSongs] = FilterHide;

	if(filter->filters[FilterSpellCrits] == 0)
		ClientFilters[FilterSpellCrits] = FilterShow;
	else if(filter->filters[FilterSpellCrits] == 1)
		ClientFilters[FilterSpellCrits] = FilterShowSelfOnly;
	else
		ClientFilters[FilterSpellCrits] = FilterHide;

	if (filter->filters[FilterMeleeCrits] == 0)
		ClientFilters[FilterMeleeCrits] = FilterShow;
	else if (filter->filters[FilterMeleeCrits] == 1)
		ClientFilters[FilterMeleeCrits] = FilterShowSelfOnly;
	else
		ClientFilters[FilterMeleeCrits] = FilterHide;

	if(filter->filters[FilterSpellDamage] == 0)
		ClientFilters[FilterSpellDamage] = FilterShow;
	else if(filter->filters[FilterSpellDamage] == 1)
		ClientFilters[FilterSpellDamage] = FilterShowSelfOnly;
	else
		ClientFilters[FilterSpellDamage] = FilterHide;

	Filter0(FilterMyMisses);
	Filter0(FilterOthersMiss);
	Filter0(FilterOthersHit);
	Filter0(FilterMissedMe);
	Filter1(FilterDamageShields);

	if (ClientVersionBit() & EQ::versions::maskSoDAndLater) {
		if (filter->filters[FilterDOT] == 0)
			ClientFilters[FilterDOT] = FilterShow;
		else if (filter->filters[FilterDOT] == 1)
			ClientFilters[FilterDOT] = FilterShowSelfOnly;
		else if (filter->filters[FilterDOT] == 2)
			ClientFilters[FilterDOT] = FilterShowGroupOnly;
		else
			ClientFilters[FilterDOT] = FilterHide;
	} else {
		if (filter->filters[FilterDOT] == 0) // show functions as self only
			ClientFilters[FilterDOT] = FilterShowSelfOnly;
		else
			ClientFilters[FilterDOT] = FilterHide;
	}

	Filter1(FilterPetHits);
	Filter1(FilterPetMisses);
	Filter1(FilterFocusEffects);
	Filter1(FilterPetSpells);

	if (ClientVersionBit() & EQ::versions::maskSoDAndLater) {
		if (filter->filters[FilterHealOverTime] == 0)
			ClientFilters[FilterHealOverTime] = FilterShow;
		// This is called 'Show Mine Only' in the clients, but functions the same as show
		// so instead of apply special logic, just set to show
		else if (filter->filters[FilterHealOverTime] == 1)
			ClientFilters[FilterHealOverTime] = FilterShow;
		else
			ClientFilters[FilterHealOverTime] = FilterHide;
	} else {
		// these clients don't have a 'self only' filter
		Filter1(FilterHealOverTime);
	}
}

// this version is for messages with no parameters
void Client::MessageString(uint32 type, uint32 string_id, uint32 distance)
{
	if (GetFilter(FilterSpellDamage) == FilterHide && type == Chat::NonMelee)
		return;
	if (GetFilter(FilterMeleeCrits) == FilterHide && type == Chat::MeleeCrit) //98 is self...
		return;
	if (GetFilter(FilterSpellCrits) == FilterHide && type == Chat::SpellCrit)
		return;
	auto outapp = new EQApplicationPacket(OP_SimpleMessage, 12);
	SimpleMessage_Struct* sms = (SimpleMessage_Struct*)outapp->pBuffer;
	sms->color=type;
	sms->string_id=string_id;

	sms->unknown8=0;

	if(distance>0)
		entity_list.QueueCloseClients(this,outapp,false,distance);
	else
		QueuePacket(outapp);
	safe_delete(outapp);
}

//
// this list of 9 args isn't how I want to do it, but to use va_arg
// you have to know how many args you're expecting, and to do that we have
// to load the eqstr file and count them in the string.
// This hack sucks but it's gonna work for now.
//
void Client::MessageString(uint32 type, uint32 string_id, const char* message1,
	const char* message2,const char* message3,const char* message4,
	const char* message5,const char* message6,const char* message7,
	const char* message8,const char* message9, uint32 distance)
{
	if (GetFilter(FilterSpellDamage) == FilterHide && type == Chat::NonMelee)
		return;
	if (GetFilter(FilterMeleeCrits) == FilterHide && type == Chat::MeleeCrit) //98 is self...
		return;
	if (GetFilter(FilterSpellCrits) == FilterHide && type == Chat::SpellCrit)
		return;
	if (GetFilter(FilterDamageShields) == FilterHide && type == Chat::DamageShield)
		return;

	if (type == Chat::Emote)
		type = 4;

	if (!message1) {
		MessageString(type, string_id);	// use the simple message instead
		return;
	}

	const char *message_arg[] = {
		message1, message2, message3, message4, message5,
		message6, message7, message8, message9
	};

	SerializeBuffer buf(20);
	buf.WriteInt32(0); // unknown
	buf.WriteInt32(string_id);
	buf.WriteInt32(type);
	for (auto &m : message_arg) {
		if (m == nullptr)
			break;
		buf.WriteString(m);
	}

	buf.WriteInt8(0); // prevent oob in packet translation, maybe clean that up sometime

	auto outapp = std::make_unique<EQApplicationPacket>(OP_FormattedMessage, buf);

	if (distance > 0)
		entity_list.QueueCloseClients(this, outapp.get(), false, distance);
	else
		QueuePacket(outapp.get());
}

void Client::MessageString(const CZClientMessageString_Struct* msg)
{
	if (msg)
	{
		if (msg->args_size == 0)
		{
			MessageString(msg->chat_type, msg->string_id);
		}
		else
		{
			uint32_t outsize = sizeof(FormattedMessage_Struct) + msg->args_size;
			auto outapp = std::make_unique<EQApplicationPacket>(OP_FormattedMessage, outsize);
			auto outbuf = reinterpret_cast<FormattedMessage_Struct*>(outapp->pBuffer);
			outbuf->string_id = msg->string_id;
			outbuf->type = msg->chat_type;
			memcpy(outbuf->message, msg->args, msg->args_size);
			QueuePacket(outapp.get());
		}
	}
}

// helper function, returns true if we should see the message
bool Client::FilteredMessageCheck(Mob *sender, eqFilterType filter)
{
	eqFilterMode mode = GetFilter(filter);
	// easy ones first
	if (mode == FilterShow)
		return true;
	else if (mode == FilterHide)
		return false;

	if (!sender && mode == FilterHide) {
		return false;
	} else if (sender) {
		if (this == sender) {
			if (mode == FilterHide) // don't need to check others
				return false;
		} else if (mode == FilterShowSelfOnly) { // we know sender isn't us
			return false;
		} else if (mode == FilterShowGroupOnly) {
			Group *g = GetGroup();
			Raid *r = GetRaid();
			if (g) {
				if (g->IsGroupMember(sender))
					return true;
			} else if (r && sender->IsClient()) {
				uint32 rgid1 = r->GetGroup(this);
				uint32 rgid2 = r->GetGroup(sender->CastToClient());
				if (rgid1 != 0xFFFFFFFF && rgid1 == rgid2)
					return true;
			} else {
				return false;
			}
		}
	}

	// we passed our checks
	return true;
}

void Client::FilteredMessageString(Mob *sender, uint32 type,
		eqFilterType filter, uint32 string_id)
{
	if (!FilteredMessageCheck(sender, filter))
		return;

	auto outapp = new EQApplicationPacket(OP_SimpleMessage, 12);
	SimpleMessage_Struct *sms = (SimpleMessage_Struct *)outapp->pBuffer;
	sms->color = type;
	sms->string_id = string_id;

	sms->unknown8 = 0;

	QueuePacket(outapp);
	safe_delete(outapp);

	return;
}

void Client::FilteredMessageString(Mob *sender, uint32 type, eqFilterType filter, uint32 string_id,
		const char *message1, const char *message2, const char *message3,
		const char *message4, const char *message5, const char *message6,
		const char *message7, const char *message8, const char *message9)
{
	if (!FilteredMessageCheck(sender, filter))
		return;

	if (type == Chat::Emote)
		type = 4;

	if (!message1) {
		FilteredMessageString(sender, type, filter, string_id);	// use the simple message instead
		return;
	}

	const char *message_arg[] = {
		message1, message2, message3, message4, message5,
		message6, message7, message8, message9
	};

	SerializeBuffer buf(20);
	buf.WriteInt32(0); // unknown
	buf.WriteInt32(string_id);
	buf.WriteInt32(type);
	for (auto &m : message_arg) {
		if (m == nullptr)
			break;
		buf.WriteString(m);
	}

	buf.WriteInt8(0); // prevent oob in packet translation, maybe clean that up sometime

	auto outapp = std::make_unique<EQApplicationPacket>(OP_FormattedMessage, buf);

	QueuePacket(outapp.get());
}

void Client::Tell_StringID(uint32 string_id, const char *who, const char *message)
{
	char string_id_str[10];
	snprintf(string_id_str, 10, "%d", string_id);

	MessageString(Chat::EchoTell, TELL_QUEUED_MESSAGE, who, string_id_str, message);
}

void Client::SetTint(int16 in_slot, uint32 color) {
	EQ::textures::Tint_Struct new_color;
	new_color.Color = color;
	SetTint(in_slot, new_color);
	database.SaveCharacterMaterialColor(this->CharacterID(), in_slot, color);
}

// Still need to reconcile bracer01 versus bracer02
void Client::SetTint(int16 in_slot, EQ::textures::Tint_Struct& color) {

	uint8 matslot = EQ::InventoryProfile::CalcMaterialFromSlot(in_slot);
	if (matslot != EQ::textures::materialInvalid)
	{
		m_pp.item_tint.Slot[matslot].Color = color.Color;
		database.SaveCharacterMaterialColor(this->CharacterID(), in_slot, color.Color);
	}

}

void Client::SetHideMe(bool flag)
{
	EQApplicationPacket app;

	gm_hide_me = flag;

	if(gm_hide_me)
	{
		database.SetHideMe(AccountID(),true);
		CreateDespawnPacket(&app, false);
		entity_list.RemoveFromTargets(this);
		trackable = false;
	}
	else
	{
		database.SetHideMe(AccountID(),false);
		CreateSpawnPacket(&app);
		trackable = true;
	}

	entity_list.QueueClientsStatus(this, &app, true, 0, Admin()-1);
}

void Client::SetLanguageSkill(int langid, int value)
{
	if (langid >= MAX_PP_LANGUAGE)
		return; //Invalid Language

	if (value > 100)
		value = 100; //Max lang value

	m_pp.languages[langid] = value;
	database.SaveCharacterLanguage(this->CharacterID(), langid, value);

	auto outapp = new EQApplicationPacket(OP_SkillUpdate, sizeof(SkillUpdate_Struct));
	SkillUpdate_Struct* skill = (SkillUpdate_Struct*)outapp->pBuffer;
	skill->skillId = 100 + langid;
	skill->value = m_pp.languages[langid];
	QueuePacket(outapp);
	safe_delete(outapp);

	MessageString( Chat::Skills, LANG_SKILL_IMPROVED ); //Notify the client
}

void Client::LinkDead()
{
	if (GetGroup())
	{
		entity_list.MessageGroup(this,true,15,"%s has gone linkdead.",GetName());
		GetGroup()->DelMember(this);
		if (GetMerc())
		{
			GetMerc()->RemoveMercFromGroup(GetMerc(), GetMerc()->GetGroup());
		}
	}
	Raid *raid = entity_list.GetRaidByClient(this);
	if(raid){
		raid->MemberZoned(this);
	}

	Expedition* expedition = GetExpedition();
	if (expedition)
	{
		expedition->SetMemberStatus(this, DynamicZoneMemberStatus::LinkDead);
	}

//	save_timer.Start(2500);
	linkdead_timer.Start(RuleI(Zone,ClientLinkdeadMS));
	SendAppearancePacket(AT_Linkdead, 1);
	client_state = CLIENT_LINKDEAD;
	AI_Start(CLIENT_LD_TIMEOUT);
}

uint8 Client::SlotConvert(uint8 slot,bool bracer){
	uint8 slot2 = 0; // why are we returning MainCharm instead of INVALID_INDEX? (must be a pre-charm segment...)
	if(bracer)
		return EQ::invslot::slotWrist2;
	switch(slot) {
	case EQ::textures::armorHead:
		slot2 = EQ::invslot::slotHead;
		break;
	case EQ::textures::armorChest:
		slot2 = EQ::invslot::slotChest;
		break;
	case EQ::textures::armorArms:
		slot2 = EQ::invslot::slotArms;
		break;
	case EQ::textures::armorWrist:
		slot2 = EQ::invslot::slotWrist1;
		break;
	case EQ::textures::armorHands:
		slot2 = EQ::invslot::slotHands;
		break;
	case EQ::textures::armorLegs:
		slot2 = EQ::invslot::slotLegs;
		break;
	case EQ::textures::armorFeet:
		slot2 = EQ::invslot::slotFeet;
		break;
	}
	return slot2;
}

uint8 Client::SlotConvert2(uint8 slot){
	uint8 slot2 = 0; // same as above...
	switch(slot){
	case EQ::invslot::slotHead:
		slot2 = EQ::textures::armorHead;
		break;
	case EQ::invslot::slotChest:
		slot2 = EQ::textures::armorChest;
		break;
	case EQ::invslot::slotArms:
		slot2 = EQ::textures::armorArms;
		break;
	case EQ::invslot::slotWrist1:
		slot2 = EQ::textures::armorWrist;
		break;
	case EQ::invslot::slotHands:
		slot2 = EQ::textures::armorHands;
		break;
	case EQ::invslot::slotLegs:
		slot2 = EQ::textures::armorLegs;
		break;
	case EQ::invslot::slotFeet:
		slot2 = EQ::textures::armorFeet;
		break;
	}
	return slot2;
}

void Client::Escape()
{
	entity_list.RemoveFromTargets(this, true);
	SetInvisible(1);

	MessageString(Chat::Skills, ESCAPE);
}

float Client::CalcPriceMod(Mob* other, bool reverse)
{
	float chaformula = 0;
	if (other)
	{
		int factionlvl = GetFactionLevel(CharacterID(), other->CastToNPC()->GetNPCTypeID(), GetFactionRace(), GetClass(), GetDeity(), other->CastToNPC()->GetPrimaryFaction(), other);
		if (factionlvl >= FACTION_APPREHENSIVE) // Apprehensive or worse.
		{
			if (GetCHA() > 103)
			{
				chaformula = (GetCHA() - 103)*((-(RuleR(Merchant, ChaBonusMod))/100)*(RuleI(Merchant, PriceBonusPct))); // This will max out price bonus.
				if (chaformula < -1*(RuleI(Merchant, PriceBonusPct)))
					chaformula = -1*(RuleI(Merchant, PriceBonusPct));
			}
			else if (GetCHA() < 103)
			{
				chaformula = (103 - GetCHA())*(((RuleR(Merchant, ChaPenaltyMod))/100)*(RuleI(Merchant, PricePenaltyPct))); // This will bottom out price penalty.
				if (chaformula > 1*(RuleI(Merchant, PricePenaltyPct)))
					chaformula = 1*(RuleI(Merchant, PricePenaltyPct));
			}
		}
		if (factionlvl <= FACTION_INDIFFERENT) // Indifferent or better.
		{
			if (GetCHA() > 75)
			{
				chaformula = (GetCHA() - 75)*((-(RuleR(Merchant, ChaBonusMod))/100)*(RuleI(Merchant, PriceBonusPct))); // This will max out price bonus.
				if (chaformula < -1*(RuleI(Merchant, PriceBonusPct)))
					chaformula = -1*(RuleI(Merchant, PriceBonusPct));
			}
			else if (GetCHA() < 75)
			{
				chaformula = (75 - GetCHA())*(((RuleR(Merchant, ChaPenaltyMod))/100)*(RuleI(Merchant, PricePenaltyPct))); // Faction modifier keeps up from reaching bottom price penalty.
				if (chaformula > 1*(RuleI(Merchant, PricePenaltyPct)))
					chaformula = 1*(RuleI(Merchant, PricePenaltyPct));
			}
		}
	}

	if (reverse)
		chaformula *= -1; //For selling
	//Now we have, for example, 10
	chaformula /= 100; //Convert to 0.10
	chaformula += 1; //Convert to 1.10;
	return chaformula; //Returns 1.10, expensive stuff!
}

//neat idea from winter's roar, not implemented
void Client::Insight(uint32 t_id)
{
	Mob* who = entity_list.GetMob(t_id);
	if (!who)
		return;
	if (!who->IsNPC())
	{
		Message(0,"This ability can only be used on NPCs.");
		return;
	}
	if (Distance(static_cast<glm::vec3>(m_Position), static_cast<glm::vec3>(who->GetPosition())) > 200)
	{
		Message(0,"You must get closer to your target!");
		return;
	}
	if (!CheckLosFN(who))
	{
		Message(0,"You must be able to see your target!");
		return;
	}
	char hitpoints[64];
	char resists[320];
	char dmg[64];
	memset(hitpoints,0,sizeof(hitpoints));
	memset(resists,0,sizeof(resists));
	memset(dmg,0,sizeof(dmg));
	//Start with HP blah
	int avg_hp = GetLevelHP(who->GetLevel());
	int cur_hp = who->GetHP();
	if (cur_hp == avg_hp)
	{
		strn0cpy(hitpoints,"averagely tough",32);
	}
	else if (cur_hp >= avg_hp*5)
	{
		strn0cpy(hitpoints,"extremely tough",32);
	}
	else if (cur_hp >= avg_hp*4)
	{
		strn0cpy(hitpoints,"exceptionally tough",32);
	}
	else if (cur_hp >= avg_hp*3)
	{
		strn0cpy(hitpoints,"very tough",32);
	}
	else if (cur_hp >= avg_hp*2)
	{
		strn0cpy(hitpoints,"quite tough",32);
	}
	else if (cur_hp >= avg_hp*1.25)
	{
		strn0cpy(hitpoints,"rather tough",32);
	}
	else if (cur_hp > avg_hp)
	{
		strn0cpy(hitpoints,"slightly tough",32);
	}
	else if (cur_hp <= avg_hp*0.20)
	{
		strn0cpy(hitpoints,"extremely frail",32);
	}
	else if (cur_hp <= avg_hp*0.25)
	{
		strn0cpy(hitpoints,"exceptionally frail",32);
	}
	else if (cur_hp <= avg_hp*0.33)
	{
		strn0cpy(hitpoints,"very frail",32);
	}
	else if (cur_hp <= avg_hp*0.50)
	{
		strn0cpy(hitpoints,"quite frail",32);
	}
	else if (cur_hp <= avg_hp*0.75)
	{
		strn0cpy(hitpoints,"rather frail",32);
	}
	else if (cur_hp < avg_hp)
	{
		strn0cpy(hitpoints,"slightly frail",32);
	}

	int avg_dmg = who->CastToNPC()->GetMaxDamage(who->GetLevel());
	int cur_dmg = who->CastToNPC()->GetMaxDMG();
	if (cur_dmg == avg_dmg)
	{
		strn0cpy(dmg,"averagely strong",32);
	}
	else if (cur_dmg >= avg_dmg*4)
	{
		strn0cpy(dmg,"extremely strong",32);
	}
	else if (cur_dmg >= avg_dmg*3)
	{
		strn0cpy(dmg,"exceptionally strong",32);
	}
	else if (cur_dmg >= avg_dmg*2)
	{
		strn0cpy(dmg,"very strong",32);
	}
	else if (cur_dmg >= avg_dmg*1.25)
	{
		strn0cpy(dmg,"quite strong",32);
	}
	else if (cur_dmg >= avg_dmg*1.10)
	{
		strn0cpy(dmg,"rather strong",32);
	}
	else if (cur_dmg > avg_dmg)
	{
		strn0cpy(dmg,"slightly strong",32);
	}
	else if (cur_dmg <= avg_dmg*0.20)
	{
		strn0cpy(dmg,"extremely weak",32);
	}
	else if (cur_dmg <= avg_dmg*0.25)
	{
		strn0cpy(dmg,"exceptionally weak",32);
	}
	else if (cur_dmg <= avg_dmg*0.33)
	{
		strn0cpy(dmg,"very weak",32);
	}
	else if (cur_dmg <= avg_dmg*0.50)
	{
		strn0cpy(dmg,"quite weak",32);
	}
	else if (cur_dmg <= avg_dmg*0.75)
	{
		strn0cpy(dmg,"rather weak",32);
	}
	else if (cur_dmg < avg_dmg)
	{
		strn0cpy(dmg,"slightly weak",32);
	}

	//Resists
	int res;
	int i = 1;

	//MR
	res = who->GetResist(i);
	i++;
	if (res >= 1000)
	{
		strcat(resists,"immune");
	}
	else if (res >= 500)
	{
		strcat(resists,"practically immune");
	}
	else if (res >= 250)
	{
		strcat(resists,"exceptionally resistant");
	}
	else if (res >= 150)
	{
		strcat(resists,"very resistant");
	}
	else if (res >= 100)
	{
		strcat(resists,"fairly resistant");
	}
	else if (res >= 50)
	{
		strcat(resists,"averagely resistant");
	}
	else if (res >= 25)
	{
		strcat(resists,"weakly resistant");
	}
	else
	{
		strcat(resists,"barely resistant");
	}
	strcat(resists," to magic, ");

	//FR
	res = who->GetResist(i);
	i++;
	if (res >= 1000)
	{
		strcat(resists,"immune");
	}
	else if (res >= 500)
	{
		strcat(resists,"practically immune");
	}
	else if (res >= 250)
	{
		strcat(resists,"exceptionally resistant");
	}
	else if (res >= 150)
	{
		strcat(resists,"very resistant");
	}
	else if (res >= 100)
	{
		strcat(resists,"fairly resistant");
	}
	else if (res >= 50)
	{
		strcat(resists,"averagely resistant");
	}
	else if (res >= 25)
	{
		strcat(resists,"weakly resistant");
	}
	else
	{
		strcat(resists,"barely resistant");
	}
	strcat(resists," to fire, ");

	//CR
	res = who->GetResist(i);
	i++;
	if (res >= 1000)
	{
		strcat(resists,"immune");
	}
	else if (res >= 500)
	{
		strcat(resists,"practically immune");
	}
	else if (res >= 250)
	{
		strcat(resists,"exceptionally resistant");
	}
	else if (res >= 150)
	{
		strcat(resists,"very resistant");
	}
	else if (res >= 100)
	{
		strcat(resists,"fairly resistant");
	}
	else if (res >= 50)
	{
		strcat(resists,"averagely resistant");
	}
	else if (res >= 25)
	{
		strcat(resists,"weakly resistant");
	}
	else
	{
		strcat(resists,"barely resistant");
	}
	strcat(resists," to cold, ");

	//PR
	res = who->GetResist(i);
	i++;
	if (res >= 1000)
	{
		strcat(resists,"immune");
	}
	else if (res >= 500)
	{
		strcat(resists,"practically immune");
	}
	else if (res >= 250)
	{
		strcat(resists,"exceptionally resistant");
	}
	else if (res >= 150)
	{
		strcat(resists,"very resistant");
	}
	else if (res >= 100)
	{
		strcat(resists,"fairly resistant");
	}
	else if (res >= 50)
	{
		strcat(resists,"averagely resistant");
	}
	else if (res >= 25)
	{
		strcat(resists,"weakly resistant");
	}
	else
	{
		strcat(resists,"barely resistant");
	}
	strcat(resists," to poison, and ");

	//MR
	res = who->GetResist(i);
	i++;
	if (res >= 1000)
	{
		strcat(resists,"immune");
	}
	else if (res >= 500)
	{
		strcat(resists,"practically immune");
	}
	else if (res >= 250)
	{
		strcat(resists,"exceptionally resistant");
	}
	else if (res >= 150)
	{
		strcat(resists,"very resistant");
	}
	else if (res >= 100)
	{
		strcat(resists,"fairly resistant");
	}
	else if (res >= 50)
	{
		strcat(resists,"averagely resistant");
	}
	else if (res >= 25)
	{
		strcat(resists,"weakly resistant");
	}
	else
	{
		strcat(resists,"barely resistant");
	}
	strcat(resists," to disease.");

	Message(0,"Your target is a level %i %s. It appears %s and %s for its level. It seems %s",who->GetLevel(),GetClassIDName(who->GetClass(),1),dmg,hitpoints,resists);
}

void Client::GetGroupAAs(GroupLeadershipAA_Struct *into) const {
	memcpy(into, &m_pp.leader_abilities.group, sizeof(GroupLeadershipAA_Struct));
}

void Client::GetRaidAAs(RaidLeadershipAA_Struct *into) const {
	memcpy(into, &m_pp.leader_abilities.raid, sizeof(RaidLeadershipAA_Struct));
}

void Client::EnteringMessages(Client* client)
{
	//server rules
	std::string rules;
	if(database.GetVariable("Rules", rules))
	{
		uint8 flag = database.GetAgreementFlag(client->AccountID());
		if(!flag)
		{
			client->Message(Chat::Red,"You must agree to the Rules, before you can move. (type #serverrules to view the rules)");
			client->Message(Chat::Red,"You must agree to the Rules, before you can move. (type #serverrules to view the rules)");
			client->Message(Chat::Red,"You must agree to the Rules, before you can move. (type #serverrules to view the rules)");
			client->SendAppearancePacket(AT_Anim, ANIM_FREEZE);
		}
	}
}

void Client::SendRules(Client* client)
{
	std::string rules;

	if (!database.GetVariable("Rules", rules))
		return;

	auto lines = SplitString(rules, '\n');
	for (auto&& e : lines)
		client->Message(Chat::White, "%s", e.c_str());
}

void Client::SetEndurance(int32 newEnd)
{
	/*Endurance can't be less than 0 or greater than max*/
	if(newEnd < 0)
		newEnd = 0;
	else if(newEnd > GetMaxEndurance()){
		newEnd = GetMaxEndurance();
	}

	current_endurance = newEnd;
	CheckManaEndUpdate();
}

void Client::SacrificeConfirm(Client *caster)
{
	auto outapp = new EQApplicationPacket(OP_Sacrifice, sizeof(Sacrifice_Struct));
	Sacrifice_Struct *ss = (Sacrifice_Struct *)outapp->pBuffer;

	if (!caster || PendingSacrifice) {
		safe_delete(outapp);
		return;
	}

	if (GetLevel() < RuleI(Spells, SacrificeMinLevel)) {
		caster->MessageString(Chat::Red, SAC_TOO_LOW); // This being is not a worthy sacrifice.
		safe_delete(outapp);
		return;
	}

	if (GetLevel() > RuleI(Spells, SacrificeMaxLevel)) {
		caster->MessageString(Chat::Red, SAC_TOO_HIGH);
		safe_delete(outapp);
		return;
	}

	ss->CasterID = caster->GetID();
	ss->TargetID = GetID();
	ss->Confirm = 0;
	QueuePacket(outapp);
	safe_delete(outapp);
	// We store the Caster's name, because when the packet comes back, it only has the victim's entityID in it,
	// not the caster.
	SacrificeCaster += caster->GetName();
	PendingSacrifice = true;
}

//Essentially a special case death function
void Client::Sacrifice(Client *caster)
{
	if (GetLevel() >= RuleI(Spells, SacrificeMinLevel) && GetLevel() <= RuleI(Spells, SacrificeMaxLevel)) {
		int exploss = (int)(GetLevel() * (GetLevel() / 18.0) * 12000);
		if (exploss < GetEXP()) {
			SetEXP(GetEXP() - exploss, GetAAXP());
			SendLogoutPackets();

			// make our become corpse packet, and queue to ourself before OP_Death.
			EQApplicationPacket app2(OP_BecomeCorpse, sizeof(BecomeCorpse_Struct));
			BecomeCorpse_Struct *bc = (BecomeCorpse_Struct *)app2.pBuffer;
			bc->spawn_id = GetID();
			bc->x = GetX();
			bc->y = GetY();
			bc->z = GetZ();
			QueuePacket(&app2);

			// make death packet
			EQApplicationPacket app(OP_Death, sizeof(Death_Struct));
			Death_Struct *d = (Death_Struct *)app.pBuffer;
			d->spawn_id = GetID();
			d->killer_id = caster ? caster->GetID() : 0;
			d->bindzoneid = GetPP().binds[0].zone_id;
			d->spell_id = SPELL_UNKNOWN;
			d->attack_skill = 0xe7;
			d->damage = 0;
			app.priority = 6;
			entity_list.QueueClients(this, &app);

			BuffFadeAll();
			UnmemSpellAll();
			Group *g = GetGroup();
			if (g) {
				g->MemberZoned(this);
			}
			Raid *r = entity_list.GetRaidByClient(this);
			if (r) {
				r->MemberZoned(this);
			}
			ClearAllProximities();
			if (RuleB(Character, LeaveCorpses)) {
				auto new_corpse = new Corpse(this, 0);
				entity_list.AddCorpse(new_corpse, GetID());
				SetID(0);
				entity_list.QueueClients(this, &app2, true);
			}
			Save();
			GoToDeath();
			if (caster) // I guess it's possible?
				caster->SummonItem(RuleI(Spells, SacrificeItemID));
		}
	} else {
		caster->MessageString(Chat::Red, SAC_TOO_LOW); // This being is not a worthy sacrifice.
	}
}

void Client::SendOPTranslocateConfirm(Mob *Caster, uint16 SpellID) {

	if(!Caster || PendingTranslocate)
		return;

	const SPDat_Spell_Struct &Spell = spells[SpellID];

	auto outapp = new EQApplicationPacket(OP_Translocate, sizeof(Translocate_Struct));
	Translocate_Struct *ts = (Translocate_Struct*)outapp->pBuffer;

	strcpy(ts->Caster, Caster->GetName());
	PendingTranslocateData.spell_id = ts->SpellID = SpellID;

	if((SpellID == 1422) || (SpellID == 1334) || (SpellID == 3243)) {
		PendingTranslocateData.zone_id = ts->ZoneID = m_pp.binds[0].zone_id;
		PendingTranslocateData.instance_id = m_pp.binds[0].instance_id;
		PendingTranslocateData.x = ts->x = m_pp.binds[0].x;
		PendingTranslocateData.y = ts->y = m_pp.binds[0].y;
		PendingTranslocateData.z = ts->z = m_pp.binds[0].z;
		PendingTranslocateData.heading = m_pp.binds[0].heading;
	}
	else {
		PendingTranslocateData.zone_id = ts->ZoneID = ZoneID(Spell.teleport_zone);
		PendingTranslocateData.instance_id = 0;
		PendingTranslocateData.y = ts->y = Spell.base[0];
		PendingTranslocateData.x = ts->x = Spell.base[1];
		PendingTranslocateData.z = ts->z = Spell.base[2];
		PendingTranslocateData.heading = 0.0;
	}

	ts->unknown008 = 0;
	ts->Complete = 0;

	PendingTranslocate = true;
	TranslocateTime = time(nullptr);

	QueuePacket(outapp);
	safe_delete(outapp);

	return;
}
void Client::SendPickPocketResponse(Mob *from, uint32 amt, int type, const EQ::ItemData* item){
	auto outapp = new EQApplicationPacket(OP_PickPocket, sizeof(sPickPocket_Struct));
	sPickPocket_Struct *pick_out = (sPickPocket_Struct *)outapp->pBuffer;
	pick_out->coin = amt;
	pick_out->from = GetID();
	pick_out->to = from->GetID();
		pick_out->myskill = GetSkill(EQ::skills::SkillPickPockets);

	if ((type >= PickPocketPlatinum) && (type <= PickPocketCopper) && (amt == 0))
		type = PickPocketFailed;

	pick_out->type = type;
	if (item)
		strcpy(pick_out->itemname, item->Name);
	else
		pick_out->itemname[0] = '\0';
	// if we do not send this packet the client will lock up and require the player to relog.
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SetHoTT(uint32 mobid) {
	auto outapp = new EQApplicationPacket(OP_TargetHoTT, sizeof(ClientTarget_Struct));
	ClientTarget_Struct *ct = (ClientTarget_Struct *) outapp->pBuffer;
	ct->new_target = mobid;
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendPopupToClient(const char *Title, const char *Text, uint32 PopupID, uint32 Buttons, uint32 Duration)
{
	auto outapp = new EQApplicationPacket(OP_OnLevelMessage, sizeof(OnLevelMessage_Struct));

	OnLevelMessage_Struct *olms = (OnLevelMessage_Struct *) outapp->pBuffer;

	if ((strlen(Title) > (sizeof(olms->Title) - 1)) || (strlen(Text) > (sizeof(olms->Text) - 1))) {
		safe_delete(outapp);
		return;
	}

	strcpy(olms->Title, Title);
	strcpy(olms->Text, Text);

	olms->Buttons = Buttons;

	if (Duration > 0) {
		olms->Duration = Duration * 1000;
	}
	else {
		olms->Duration = 0xffffffff;
	}

	olms->PopupID    = PopupID;
	olms->NegativeID = 0;

	sprintf(olms->ButtonName0, "%s", "Yes");
	sprintf(olms->ButtonName1, "%s", "No");
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendFullPopup(
	const char *Title,
	const char *Text,
	uint32 PopupID,
	uint32 NegativeID,
	uint32 Buttons,
	uint32 Duration,
	const char *ButtonName0,
	const char *ButtonName1,
	uint32 SoundControls
)
{
	auto outapp = new EQApplicationPacket(OP_OnLevelMessage, sizeof(OnLevelMessage_Struct));

	OnLevelMessage_Struct *olms = (OnLevelMessage_Struct *) outapp->pBuffer;

	if ((strlen(Text) > (sizeof(olms->Text) - 1)) || (strlen(Title) > (sizeof(olms->Title) - 1))) {
		safe_delete(outapp);
		return;
	}

	if (ButtonName0 && ButtonName1 && ((strlen(ButtonName0) > (sizeof(olms->ButtonName0) - 1)) ||
									   (strlen(ButtonName1) > (sizeof(olms->ButtonName1) - 1)))) {
		safe_delete(outapp);
		return;
	}

	strcpy(olms->Title, Title);
	strcpy(olms->Text, Text);

	olms->Buttons = Buttons;

	if (ButtonName0 == nullptr || ButtonName1 == nullptr) {
		sprintf(olms->ButtonName0, "%s", "Yes");
		sprintf(olms->ButtonName1, "%s", "No");
	}
	else {
		strcpy(olms->ButtonName0, ButtonName0);
		strcpy(olms->ButtonName1, ButtonName1);
	}

	if (Duration > 0) {
		olms->Duration = Duration * 1000;
	}
	else {
		olms->Duration = 0xffffffff;
	}

	olms->PopupID       = PopupID;
	olms->NegativeID    = NegativeID;
	olms->SoundControls = SoundControls;

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendWindow(
	uint32 PopupID,
	uint32 NegativeID,
	uint32 Buttons,
	const char *ButtonName0,
	const char *ButtonName1,
	uint32 Duration,
	int title_type,
	Client *target,
	const char *Title,
	const char *Text,
	...
)
{
	va_list argptr;
	char    buffer[4096];

	va_start(argptr, Text);
	vsnprintf(buffer, sizeof(buffer), Text, argptr);
	va_end(argptr);

	size_t len = strlen(buffer);

	auto                  app   = new EQApplicationPacket(OP_OnLevelMessage, sizeof(OnLevelMessage_Struct));
	OnLevelMessage_Struct *olms = (OnLevelMessage_Struct *) app->pBuffer;

	if (strlen(Text) > (sizeof(olms->Text) - 1)) {
		safe_delete(app);
		return;
	}

	if (!target) {
		title_type = 0;
	}

	switch (title_type) {
		case 1: {
			char name[64] = "";
			strcpy(name, target->GetName());
			if (target->GetLastName()) {
				char last_name[64] = "";
				strcpy(last_name, target->GetLastName());
				strcat(name, " ");
				strcat(name, last_name);
			}
			strcpy(olms->Title, name);
			break;
		}
		case 2: {
			if (target->GuildID()) {
				char *guild_name = (char *) guild_mgr.GetGuildName(target->GuildID());
				strcpy(olms->Title, guild_name);
			}
			else {
				strcpy(olms->Title, "No Guild");
			}
			break;
		}
		default: {
			strcpy(olms->Title, Title);
			break;
		}
	}

	memcpy(olms->Text, buffer, len + 1);

	olms->Buttons = Buttons;

	sprintf(olms->ButtonName0, "%s", ButtonName0);
	sprintf(olms->ButtonName1, "%s", ButtonName1);

	if (Duration > 0) {
		olms->Duration = Duration * 1000;
	}
	else {
		olms->Duration = 0xffffffff;
	}

	olms->PopupID    = PopupID;
	olms->NegativeID = NegativeID;

	FastQueuePacket(&app);
}

void Client::KeyRingLoad()
{
	std::string query = StringFormat("SELECT item_id FROM keyring "
									"WHERE char_id = '%i' ORDER BY item_id", character_id);
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	for (auto row = results.begin(); row != results.end(); ++row)
		keyring.push_back(atoi(row[0]));

}

void Client::KeyRingAdd(uint32 item_id)
{
	if(0==item_id)
		return;

	bool found = KeyRingCheck(item_id);
	if (found)
		return;

	std::string query = StringFormat("INSERT INTO keyring(char_id, item_id) VALUES(%i, %i)", character_id, item_id);
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	Message(Chat::LightBlue,"Added to keyring.");

	keyring.push_back(item_id);
}

bool Client::KeyRingCheck(uint32 item_id)
{
	for (auto iter = keyring.begin(); iter != keyring.end(); ++iter) {
		if(*iter == item_id)
			return true;
	}
	return false;
}

void Client::KeyRingList()
{
	Message(Chat::LightBlue,"Keys on Keyring:");
	const EQ::ItemData *item = nullptr;
	for (auto iter = keyring.begin(); iter != keyring.end(); ++iter) {
		if ((item = database.GetItem(*iter))!=nullptr) {
			Message(Chat::LightBlue,item->Name);
		}
	}
}

bool Client::IsDiscovered(uint32 itemid) {

	std::string query = StringFormat("SELECT count(*) FROM discovered_items WHERE item_id = '%lu'", itemid);
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	auto row = results.begin();
	if (!atoi(row[0]))
		return false;

	return true;
}

void Client::DiscoverItem(uint32 itemid) {

	std::string query = StringFormat("INSERT INTO discovered_items "
									"SET item_id = %lu, char_name = '%s', "
									"discovered_date = UNIX_TIMESTAMP(), account_status = %i",
									itemid, GetName(), Admin());
	auto results = database.QueryDatabase(query);

	parse->EventPlayer(EVENT_DISCOVER_ITEM, this, "", itemid);
}

void Client::UpdateLFP() {

	Group *g = GetGroup();

	if(g && !g->IsLeader(this)) {
		database.SetLFP(CharacterID(), false);
		worldserver.StopLFP(CharacterID());
		LFP = false;
		return;
	}

	GroupLFPMemberEntry LFPMembers[MAX_GROUP_MEMBERS];

	for(unsigned int i=0; i<MAX_GROUP_MEMBERS; i++) {
		LFPMembers[i].Name[0] = '\0';
		LFPMembers[i].Class = 0;
		LFPMembers[i].Level = 0;
		LFPMembers[i].Zone = 0;
	}

	// Slot 0 is always for the group leader, or the player if not in a group
	strcpy(LFPMembers[0].Name, GetName());
	LFPMembers[0].Class = GetClass();
	LFPMembers[0].Level = GetLevel();
	LFPMembers[0].Zone = zone->GetZoneID();

	if(g) {
		// Fill the LFPMembers array with the rest of the group members, excluding ourself
		// We don't fill in the class, level or zone, because we may not be able to determine
		// them if the other group members are not in this zone. World will fill in this information
		// for us, if it can.
		int NextFreeSlot = 1;
		for(unsigned int i = 0; i < MAX_GROUP_MEMBERS; i++) {
			if((g->membername[i][0] != '\0') && strcasecmp(g->membername[i], LFPMembers[0].Name))
				strcpy(LFPMembers[NextFreeSlot++].Name, g->membername[i]);
		}
	}
	worldserver.UpdateLFP(CharacterID(), LFPMembers);
}

bool Client::GroupFollow(Client* inviter) {

	if (inviter)
	{
		isgrouped = true;
		Raid* raid = entity_list.GetRaidByClient(inviter);
		Raid* iraid = entity_list.GetRaidByClient(this);

		//inviter has a raid don't do group stuff instead do raid stuff!
		if (raid)
		{
			// Suspend the merc while in a raid (maybe a rule could be added for this)
			if (GetMerc())
				GetMerc()->Suspend();

			uint32 groupToUse = 0xFFFFFFFF;
			for (int x = 0; x < MAX_RAID_MEMBERS; x++)
			{
				if (raid->members[x].member)
				{
					//this assumes the inviter is in the zone
					if (raid->members[x].member == inviter){
						groupToUse = raid->members[x].GroupNumber;
						break;
					}
				}
			}
			if (iraid == raid)
			{
				//both in same raid
				uint32 ngid = raid->GetGroup(inviter->GetName());
				if (raid->GroupCount(ngid) < 6)
				{
					raid->MoveMember(GetName(), ngid);
					raid->SendGroupDisband(this);
					//raid->SendRaidGroupAdd(GetName(), ngid);
					//raid->SendGroupUpdate(this);
					raid->GroupUpdate(ngid); //break
				}
				return false;
			}
			if (raid->RaidCount() < MAX_RAID_MEMBERS)
			{
				// okay, so we now have a single client (this) joining a group in a raid
				// And they're not already in the raid (which is above and doesn't need xtarget shit)
				if (!GetXTargetAutoMgr()->empty()) {
					raid->GetXTargetAutoMgr()->merge(*GetXTargetAutoMgr());
					GetXTargetAutoMgr()->clear();
					RemoveAutoXTargets();
				}

				SetXTargetAutoMgr(raid->GetXTargetAutoMgr());
				if (!GetXTargetAutoMgr()->empty())
					SetDirtyAutoHaters();

				if (raid->GroupCount(groupToUse) < 6)
				{
					raid->SendRaidCreate(this);
					raid->SendMakeLeaderPacketTo(raid->leadername, this);
					raid->AddMember(this, groupToUse);
					raid->SendBulkRaid(this);
					//raid->SendRaidGroupAdd(GetName(), groupToUse);
					//raid->SendGroupUpdate(this);
					raid->GroupUpdate(groupToUse); //break
					if (raid->IsLocked())
					{
						raid->SendRaidLockTo(this);
					}
					return false;
				}
				else
				{
					raid->SendRaidCreate(this);
					raid->SendMakeLeaderPacketTo(raid->leadername, this);
					raid->AddMember(this);
					raid->SendBulkRaid(this);
					if (raid->IsLocked())
					{
						raid->SendRaidLockTo(this);
					}
					return false;
				}
			}
		}

		Group* group = entity_list.GetGroupByClient(inviter);

		if (!group)
		{
			//Make new group
			group = new Group(inviter);

			if (!group)
			{
				return false;
			}

			entity_list.AddGroup(group);

			if (group->GetID() == 0)
			{
				Message(Chat::Red, "Unable to get new group id. Cannot create group.");
				inviter->Message(Chat::Red, "Unable to get new group id. Cannot create group.");
				return false;
			}

			//now we have a group id, can set inviter's id
			database.SetGroupID(inviter->GetName(), group->GetID(), inviter->CharacterID(), false);
			database.SetGroupLeaderName(group->GetID(), inviter->GetName());
			group->UpdateGroupAAs();

			//Invite the inviter into the group first.....dont ask
			if (inviter->ClientVersion() < EQ::versions::ClientVersion::SoD)
			{
				auto outapp = new EQApplicationPacket(OP_GroupUpdate, sizeof(GroupJoin_Struct));
				GroupJoin_Struct* outgj = (GroupJoin_Struct*)outapp->pBuffer;
				strcpy(outgj->membername, inviter->GetName());
				strcpy(outgj->yourname, inviter->GetName());
				outgj->action = groupActInviteInitial; // 'You have formed the group'.
				group->GetGroupAAs(&outgj->leader_aas);
				inviter->QueuePacket(outapp);
				safe_delete(outapp);
			}
			else
			{
				// SoD and later
				inviter->SendGroupCreatePacket();
				inviter->SendGroupLeaderChangePacket(inviter->GetName());
				inviter->SendGroupJoinAcknowledge();
			}
			group->GetXTargetAutoMgr()->merge(*inviter->GetXTargetAutoMgr());
			inviter->GetXTargetAutoMgr()->clear();
			inviter->SetXTargetAutoMgr(group->GetXTargetAutoMgr());
		}

		if (!group)
		{
			return false;
		}

		// Remove merc from old group before adding client to the new one
		if (GetMerc() && GetMerc()->HasGroup())
		{
			GetMerc()->RemoveMercFromGroup(GetMerc(), GetMerc()->GetGroup());
		}

		if (!group->AddMember(this))
		{
			// If failed to add client to new group, regroup with merc
			if (GetMerc())
			{
				GetMerc()->MercJoinClientGroup();
			}
			else
			{
				isgrouped = false;
			}
			return false;
		}

		if (ClientVersion() >= EQ::versions::ClientVersion::SoD)
		{
			SendGroupJoinAcknowledge();
		}

		// Temporary hack for SoD, as things seem to work quite differently
		if (inviter->IsClient() && inviter->ClientVersion() >= EQ::versions::ClientVersion::SoD)
		{
			database.RefreshGroupFromDB(inviter);
		}

		// Add the merc back into the new group if possible
		if (GetMerc())
		{
			GetMerc()->MercJoinClientGroup();
		}

		if (inviter->IsLFP())
		{
			// If the player who invited us to a group is LFP, have them update world now that we have joined their group.
			inviter->UpdateLFP();
		}

		database.RefreshGroupFromDB(this);
		group->SendHPManaEndPacketsTo(this);
		//send updates to clients out of zone...
		group->SendGroupJoinOOZ(this);
		return true;
	}
	return false;
}

uint16 Client::GetPrimarySkillValue()
{
	EQ::skills::SkillType skill = EQ::skills::HIGHEST_SKILL; //because nullptr == 0, which is 1H Slashing, & we want it to return 0 from GetSkill
	bool equiped = m_inv.GetItem(EQ::invslot::slotPrimary);

	if (!equiped)
		skill = EQ::skills::SkillHandtoHand;

	else {

		uint8 type = m_inv.GetItem(EQ::invslot::slotPrimary)->GetItem()->ItemType; //is this the best way to do this?

		switch (type) {
		case EQ::item::ItemType1HSlash: // 1H Slashing
			skill = EQ::skills::Skill1HSlashing;
			break;
		case EQ::item::ItemType2HSlash: // 2H Slashing
			skill = EQ::skills::Skill2HSlashing;
			break;
		case EQ::item::ItemType1HPiercing: // Piercing
			skill = EQ::skills::Skill1HPiercing;
			break;
		case EQ::item::ItemType1HBlunt: // 1H Blunt
			skill = EQ::skills::Skill1HBlunt;
			break;
		case EQ::item::ItemType2HBlunt: // 2H Blunt
			skill = EQ::skills::Skill2HBlunt;
			break;
		case EQ::item::ItemType2HPiercing: // 2H Piercing
			if (IsClient() && CastToClient()->ClientVersion() < EQ::versions::ClientVersion::RoF2)
				skill = EQ::skills::Skill1HPiercing;
			else
				skill = EQ::skills::Skill2HPiercing;
			break;
		case EQ::item::ItemTypeMartial: // Hand to Hand
			skill = EQ::skills::SkillHandtoHand;
			break;
		default: // All other types default to Hand to Hand
			skill = EQ::skills::SkillHandtoHand;
			break;
		}
	}

	return GetSkill(skill);
}

uint32 Client::GetTotalATK()
{
	uint32 AttackRating = 0;
	uint32 WornCap = itembonuses.ATK;

	if(IsClient()) {
		AttackRating = ((WornCap * 1.342) + (GetSkill(EQ::skills::SkillOffense) * 1.345) + ((GetSTR() - 66) * 0.9) + (GetPrimarySkillValue() * 2.69));
		AttackRating += aabonuses.ATK + GroupLeadershipAAOffenseEnhancement();

		if (AttackRating < 10)
			AttackRating = 10;
	}
	else
		AttackRating = GetATK();

	AttackRating += spellbonuses.ATK;

	return AttackRating;
}

uint32 Client::GetATKRating()
{
	uint32 AttackRating = 0;
	if(IsClient()) {
		AttackRating = (GetSkill(EQ::skills::SkillOffense) * 1.345) + ((GetSTR() - 66) * 0.9) + (GetPrimarySkillValue() * 2.69);

		if (AttackRating < 10)
			AttackRating = 10;
	}
	return AttackRating;
}

void Client::VoiceMacroReceived(uint32 Type, char *Target, uint32 MacroNumber) {

	uint32 GroupOrRaidID = 0;

	switch(Type) {

		case VoiceMacroGroup: {

			Group* g = GetGroup();

			if(g)
				GroupOrRaidID = g->GetID();
			else
				return;

			break;
		}

		case VoiceMacroRaid: {

			Raid* r = GetRaid();

			if(r)
				GroupOrRaidID = r->GetID();
			else
				return;

			break;
		}
	}

	if(!worldserver.SendVoiceMacro(this, Type, Target, MacroNumber, GroupOrRaidID))
		Message(0, "Error: World server disconnected");
}

void Client::ClearGroupAAs() {
	for(unsigned int i = 0; i < MAX_GROUP_LEADERSHIP_AA_ARRAY; i++)
		m_pp.leader_abilities.ranks[i] = 0;

	m_pp.group_leadership_points = 0;
	m_pp.raid_leadership_points = 0;
	m_pp.group_leadership_exp = 0;
	m_pp.raid_leadership_exp = 0;

	Save();
	database.SaveCharacterLeadershipAA(this->CharacterID(), &m_pp);
}

void Client::UpdateGroupAAs(int32 points, uint32 type) {
	switch(type) {
		case 0: { m_pp.group_leadership_points += points; break; }
		case 1: { m_pp.raid_leadership_points += points; break; }
	}
	SendLeadershipEXPUpdate();
}

bool Client::IsLeadershipEXPOn() {

	if(!m_pp.leadAAActive)
		return false;

	Group *g = GetGroup();

	if (g && g->IsLeader(this) && g->GroupCount() > 2)
		return true;

	Raid *r = GetRaid();

	if (!r)
		return false;

	// raid leaders can only gain raid AA XP
	if (r->IsLeader(this)) {
		if (r->RaidCount() > 17)
			return true;
		else
			return false;
	}

	uint32 gid = r->GetGroup(this);

	if (gid > 11) // not in a group
		return false;

	if (r->IsGroupLeader(GetName()) && r->GroupCount(gid) > 2)
		return true;

	return false;

}

int Client::GetAggroCount() {
	return AggroCount;
}

// we pass in for book keeping if RestRegen is enabled
void Client::IncrementAggroCount(bool raid_target)
{
	// This method is called when a client is added to a mob's hate list. It turns the clients aggro flag on so
	// rest state regen is stopped, and for SoF, it sends the opcode to show the crossed swords in-combat indicator.
	AggroCount++;

	if(!RuleB(Character, RestRegenEnabled))
		return;

	uint32 newtimer = raid_target ? RuleI(Character, RestRegenRaidTimeToActivate) : RuleI(Character, RestRegenTimeToActivate);

	// When our aggro count is 1 here, we are exiting rest state. We need to pause our current timer, if we have time remaining
	// We should not actually have to do anything to the Timer object since the AggroCount counter blocks it from being checked
	// and will have it's timer changed when we exit combat so let's not do any extra work
	if (AggroCount == 1 && rest_timer.GetRemainingTime()) // the Client::rest_timer is never disabled, so don't need to check
		m_pp.RestTimer = std::max(1u, rest_timer.GetRemainingTime() / 1000); // I guess round up?

	// save the new timer if it's higher
	m_pp.RestTimer = std::max(m_pp.RestTimer, newtimer);

	// If we already had aggro before this method was called, the combat indicator should already be up for SoF clients,
	// so we don't need to send it again.
	//
	if(AggroCount > 1)
		return;

	if (ClientVersion() >= EQ::versions::ClientVersion::SoF) {
		auto outapp = new EQApplicationPacket(OP_RestState, 1);
		char *Buffer = (char *)outapp->pBuffer;
		VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0x01);
		QueuePacket(outapp);
		safe_delete(outapp);
	}

}

void Client::DecrementAggroCount()
{
	// This should be called when a client is removed from a mob's hate list (it dies or is memblurred).
	// It checks whether any other mob is aggro on the player, and if not, starts the rest timer.
	// For SoF, the opcode to start the rest state countdown timer in the UI is sent.

	// If we didn't have aggro before, this method should not have been called.
	if(!AggroCount)
		return;

	AggroCount--;

	if(!RuleB(Character, RestRegenEnabled))
		return;

	// Something else is still aggro on us, can't rest yet.
	if (AggroCount)
		return;

	rest_timer.Start(m_pp.RestTimer * 1000);

	if (ClientVersion() >= EQ::versions::ClientVersion::SoF) {
		auto outapp = new EQApplicationPacket(OP_RestState, 5);
		char *Buffer = (char *)outapp->pBuffer;
		VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0x00);
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, m_pp.RestTimer);
		QueuePacket(outapp);
		safe_delete(outapp);
	}
}

// when we cast a beneficial spell we need to steal our targets current timer
// That's what we use this for
void Client::UpdateRestTimer(uint32 new_timer)
{
	// their timer was 0, so we don't do anything
	if (new_timer == 0)
		return;

	if (!RuleB(Character, RestRegenEnabled))
		return;

	// so if we're currently on aggro, we check our saved timer
	if (AggroCount) {
		if (m_pp.RestTimer < new_timer) // our timer needs to be updated, don't need to update client here
			m_pp.RestTimer = new_timer;
	} else { // if we're not aggro, we need to check if current timer needs updating
		if (rest_timer.GetRemainingTime() / 1000 < new_timer) {
			rest_timer.Start(new_timer * 1000);
			if (ClientVersion() >= EQ::versions::ClientVersion::SoF) {
				auto outapp = new EQApplicationPacket(OP_RestState, 5);
				char *Buffer = (char *)outapp->pBuffer;
				VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0x00);
				VARSTRUCT_ENCODE_TYPE(uint32, Buffer, new_timer);
				QueuePacket(outapp);
				safe_delete(outapp);
			}
		}
	}
}

void Client::SendPVPStats()
{
	// This sends the data to the client to populate the PVP Stats Window.
	//
	// When the PVP Stats window is opened, no opcode is sent. Therefore this method should be called
	// from Client::CompleteConnect, and also when the player makes a PVP kill.
	//
	auto outapp = new EQApplicationPacket(OP_PVPStats, sizeof(PVPStats_Struct));
	PVPStats_Struct *pvps = (PVPStats_Struct *)outapp->pBuffer;

	pvps->Kills = m_pp.PVPKills;
	pvps->Deaths = m_pp.PVPDeaths;
	pvps->PVPPointsAvailable = m_pp.PVPCurrentPoints;
	pvps->TotalPVPPoints = m_pp.PVPCareerPoints;
	pvps->BestKillStreak = m_pp.PVPBestKillStreak;
	pvps->WorstDeathStreak = m_pp.PVPWorstDeathStreak;
	pvps->CurrentKillStreak = m_pp.PVPCurrentKillStreak;

	// TODO: Record and send other PVP Stats

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendCrystalCounts()
{
	auto outapp = new EQApplicationPacket(OP_CrystalCountUpdate, sizeof(CrystalCountUpdate_Struct));
	CrystalCountUpdate_Struct *ccus = (CrystalCountUpdate_Struct *)outapp->pBuffer;

	ccus->CurrentRadiantCrystals = GetRadiantCrystals();
	ccus->CurrentEbonCrystals = GetEbonCrystals();
	ccus->CareerRadiantCrystals = m_pp.careerRadCrystals;
	ccus->CareerEbonCrystals = m_pp.careerEbonCrystals;


	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendDisciplineTimers()
{

	auto outapp = new EQApplicationPacket(OP_DisciplineTimer, sizeof(DisciplineTimer_Struct));
	DisciplineTimer_Struct *dts = (DisciplineTimer_Struct *)outapp->pBuffer;

	for(unsigned int i = 0; i < MAX_DISCIPLINE_TIMERS; ++i)
	{
		uint32 RemainingTime = p_timers.GetRemainingTime(pTimerDisciplineReuseStart + i);

		if(RemainingTime > 0)
		{
			dts->TimerID = i;
			dts->Duration = RemainingTime;
			QueuePacket(outapp);
		}
	}

	safe_delete(outapp);
}

void Client::SendRespawnBinds()
{
	// This sends the data to the client to populate the Respawn from Death Window.
	//
	// This should be sent after OP_Death for SoF clients
	// Client will respond with a 4 byte packet that includes the number of the selection made
	//

		//If no options have been given, default to Bind + Rez
	if (respawn_options.empty())
	{
		BindStruct* b = &m_pp.binds[0];
		RespawnOption opt;
		opt.name = "Bind Location";
		opt.zone_id = b->zone_id;
		opt.instance_id = b->instance_id;
		opt.x = b->x;
		opt.y = b->y;
		opt.z = b->z;
		opt.heading = b->heading;
		respawn_options.push_front(opt);
	}
	//Rez is always added at the end
	RespawnOption rez;
	rez.name = "Resurrect";
	rez.zone_id = zone->GetZoneID();
	rez.instance_id = zone->GetInstanceID();
	rez.x = GetX();
	rez.y = GetY();
	rez.z = GetZ();
	rez.heading = GetHeading();
	respawn_options.push_back(rez);

	int num_options = respawn_options.size();
	uint32 PacketLength = 17 + (26 * num_options); //Header size + per-option invariant size

	std::list<RespawnOption>::iterator itr;
	RespawnOption* opt = nullptr;

	//Find string size for each option
	for (itr = respawn_options.begin(); itr != respawn_options.end(); ++itr)
	{
		opt = &(*itr);
		PacketLength += opt->name.size() + 1; //+1 for cstring
	}

	auto outapp = new EQApplicationPacket(OP_RespawnWindow, PacketLength);
	char* buffer = (char*)outapp->pBuffer;

	//Packet header
	VARSTRUCT_ENCODE_TYPE(uint32, buffer, initial_respawn_selection); //initial selection (from 0)
	VARSTRUCT_ENCODE_TYPE(uint32, buffer, RuleI(Character, RespawnFromHoverTimer) * 1000);
	VARSTRUCT_ENCODE_TYPE(uint32, buffer, 0); //unknown
	VARSTRUCT_ENCODE_TYPE(uint32, buffer, num_options); //number of options to display

	//Individual options
	int count = 0;
	for (itr = respawn_options.begin(); itr != respawn_options.end(); ++itr)
	{
		opt = &(*itr);
		VARSTRUCT_ENCODE_TYPE(uint32, buffer, count++); //option num (from 0)
		VARSTRUCT_ENCODE_TYPE(uint32, buffer, opt->zone_id);
		VARSTRUCT_ENCODE_TYPE(float, buffer, opt->x);
		VARSTRUCT_ENCODE_TYPE(float, buffer, opt->y);
		VARSTRUCT_ENCODE_TYPE(float, buffer, opt->z);
		VARSTRUCT_ENCODE_TYPE(float, buffer, opt->heading);
		VARSTRUCT_ENCODE_STRING(buffer, opt->name.c_str());
		VARSTRUCT_ENCODE_TYPE(uint8, buffer, (count == num_options)); //is this one Rez (the last option)?
	}

	QueuePacket(outapp);
	safe_delete(outapp);
	return;
}

void Client::HandleLDoNOpen(NPC *target)
{
	if(target)
	{
		if(target->GetClass() != LDON_TREASURE)
		{
			LogDebug("[{}] tried to open [{}] but [{}] was not a treasure chest",
				GetName(), target->GetName(), target->GetName());
			return;
		}

		if(DistanceSquaredNoZ(m_Position, target->GetPosition()) > RuleI(Adventure, LDoNTrapDistanceUse))
		{
			LogDebug("[{}] tried to open [{}] but [{}] was out of range",
				GetName(), target->GetName(), target->GetName());
			Message(Chat::Red, "Treasure chest out of range.");
			return;
		}

		if(target->IsLDoNTrapped())
		{
			if(target->GetLDoNTrapSpellID() != 0)
			{
				MessageString(Chat::Red, LDON_ACCIDENT_SETOFF2);
				target->SpellFinished(target->GetLDoNTrapSpellID(), this, EQ::spells::CastingSlot::Item, 0, -1, spells[target->GetLDoNTrapSpellID()].ResistDiff);
				target->SetLDoNTrapSpellID(0);
				target->SetLDoNTrapped(false);
				target->SetLDoNTrapDetected(false);
			}
			else
			{
				target->SetLDoNTrapSpellID(0);
				target->SetLDoNTrapped(false);
				target->SetLDoNTrapDetected(false);
			}
		}

		if(target->IsLDoNLocked())
		{
			MessageString(Chat::Skills, LDON_STILL_LOCKED, target->GetCleanName());
			return;
		}
		else
		{
			target->AddToHateList(this, 0, 500000, false, false, false);
			if(target->GetLDoNTrapType() != 0)
			{
				if(GetRaid())
				{
					GetRaid()->SplitExp(target->GetLevel()*target->GetLevel()*2625/10, target);
				}
				else if(GetGroup())
				{
					GetGroup()->SplitExp(target->GetLevel()*target->GetLevel()*2625/10, target);
				}
				else
				{
					AddEXP(target->GetLevel()*target->GetLevel()*2625/10, GetLevelCon(target->GetLevel()));
				}
			}
			target->Death(this, 0, SPELL_UNKNOWN, EQ::skills::SkillHandtoHand);
		}
	}
}

void Client::HandleLDoNSenseTraps(NPC *target, uint16 skill, uint8 type)
{
	if(target && target->GetClass() == LDON_TREASURE)
	{
		if(target->IsLDoNTrapped())
		{
			if((target->GetLDoNTrapType() == LDoNTypeCursed || target->GetLDoNTrapType() == LDoNTypeMagical) && type != target->GetLDoNTrapType())
			{
				MessageString(Chat::Skills, LDON_CANT_DETERMINE_TRAP, target->GetCleanName());
				return;
			}

			if(target->IsLDoNTrapDetected())
			{
				MessageString(Chat::Skills, LDON_CERTAIN_TRAP, target->GetCleanName());
			}
			else
			{
				int check = LDoNChest_SkillCheck(target, skill);
				switch(check)
				{
				case -1:
				case 0:
					MessageString(Chat::Skills, LDON_DONT_KNOW_TRAPPED, target->GetCleanName());
					break;
				case 1:
					MessageString(Chat::Skills, LDON_CERTAIN_TRAP, target->GetCleanName());
					target->SetLDoNTrapDetected(true);
					break;
				default:
					break;
				}
			}
		}
		else
		{
			MessageString(Chat::Skills, LDON_CERTAIN_NOT_TRAP, target->GetCleanName());
		}
	}
}

void Client::HandleLDoNDisarm(NPC *target, uint16 skill, uint8 type)
{
	if(target)
	{
		if(target->GetClass() == LDON_TREASURE)
		{
			if(!target->IsLDoNTrapped())
			{
				MessageString(Chat::Skills, LDON_WAS_NOT_TRAPPED, target->GetCleanName());
				return;
			}

			if((target->GetLDoNTrapType() == LDoNTypeCursed || target->GetLDoNTrapType() == LDoNTypeMagical) && type != target->GetLDoNTrapType())
			{
				MessageString(Chat::Skills, LDON_HAVE_NOT_DISARMED, target->GetCleanName());
				return;
			}

			int check = 0;
			if(target->IsLDoNTrapDetected())
			{
				check = LDoNChest_SkillCheck(target, skill);
			}
			else
			{
				check = LDoNChest_SkillCheck(target, skill*33/100);
			}
			switch(check)
			{
			case 1:
				target->SetLDoNTrapDetected(false);
				target->SetLDoNTrapped(false);
				target->SetLDoNTrapSpellID(0);
				MessageString(Chat::Skills, LDON_HAVE_DISARMED, target->GetCleanName());
				break;
			case 0:
				MessageString(Chat::Skills, LDON_HAVE_NOT_DISARMED, target->GetCleanName());
				break;
			case -1:
				MessageString(Chat::Red, LDON_ACCIDENT_SETOFF2);
				target->SpellFinished(target->GetLDoNTrapSpellID(), this, EQ::spells::CastingSlot::Item, 0, -1, spells[target->GetLDoNTrapSpellID()].ResistDiff);
				target->SetLDoNTrapSpellID(0);
				target->SetLDoNTrapped(false);
				target->SetLDoNTrapDetected(false);
				break;
			}
		}
	}
}

void Client::HandleLDoNPickLock(NPC *target, uint16 skill, uint8 type)
{
	if(target)
	{
		if(target->GetClass() == LDON_TREASURE)
		{
			if(target->IsLDoNTrapped())
			{
				MessageString(Chat::Red, LDON_ACCIDENT_SETOFF2);
				target->SpellFinished(target->GetLDoNTrapSpellID(), this, EQ::spells::CastingSlot::Item, 0, -1, spells[target->GetLDoNTrapSpellID()].ResistDiff);
				target->SetLDoNTrapSpellID(0);
				target->SetLDoNTrapped(false);
				target->SetLDoNTrapDetected(false);
			}

			if(!target->IsLDoNLocked())
			{
				MessageString(Chat::Skills, LDON_WAS_NOT_LOCKED, target->GetCleanName());
				return;
			}

			if((target->GetLDoNTrapType() == LDoNTypeCursed || target->GetLDoNTrapType() == LDoNTypeMagical) && type != target->GetLDoNTrapType())
			{
				Message(Chat::Skills, "You cannot unlock %s with this skill.", target->GetCleanName());
				return;
			}

			int check = LDoNChest_SkillCheck(target, skill);

			switch(check)
			{
			case 0:
			case -1:
				MessageString(Chat::Skills, LDON_PICKLOCK_FAILURE, target->GetCleanName());
				break;
			case 1:
				target->SetLDoNLocked(false);
				MessageString(Chat::Skills, LDON_PICKLOCK_SUCCESS, target->GetCleanName());
				break;
			}
		}
	}
}

int	Client::LDoNChest_SkillCheck(NPC *target, int skill)
{
	if(!target)
		return -1;

	int	chest_difficulty = target->GetLDoNLockedSkill() == 0 ? (target->GetLevel() * 5) : target->GetLDoNLockedSkill();
	float base_difficulty = RuleR(Adventure, LDoNBaseTrapDifficulty);

	if(chest_difficulty == 0)
		chest_difficulty = 5;

	float chance = ((100.0f - base_difficulty) * ((float)skill / (float)chest_difficulty));

	if(chance > (100.0f - base_difficulty))
	{
		chance = 100.0f - base_difficulty;
	}

	float d100 = (float)zone->random.Real(0, 100);

	if(d100 <= chance)
		return 1;
	else
	{
		if(d100 > (chance + RuleR(Adventure, LDoNCriticalFailTrapThreshold)))
			return -1;
	}

	return 0;
}

void Client::SummonAndRezzAllCorpses()
{
	PendingRezzXP = -1;

	auto Pack = new ServerPacket(ServerOP_DepopAllPlayersCorpses, sizeof(ServerDepopAllPlayersCorpses_Struct));

	ServerDepopAllPlayersCorpses_Struct *sdapcs = (ServerDepopAllPlayersCorpses_Struct*)Pack->pBuffer;

	sdapcs->CharacterID = CharacterID();
	sdapcs->ZoneID = zone->GetZoneID();
	sdapcs->InstanceID = zone->GetInstanceID();

	worldserver.SendPacket(Pack);

	safe_delete(Pack);

	entity_list.RemoveAllCorpsesByCharID(CharacterID());

	int CorpseCount = database.SummonAllCharacterCorpses(CharacterID(), zone->GetZoneID(), zone->GetInstanceID(), GetPosition());
	if(CorpseCount <= 0)
	{
		Message(Chat::Yellow, "You have no corpses to summnon.");
		return;
	}

	int RezzExp = entity_list.RezzAllCorpsesByCharID(CharacterID());

	if(RezzExp > 0)
		SetEXP(GetEXP() + RezzExp, GetAAXP(), true);

	Message(Chat::Yellow, "All your corpses have been summoned to your feet and have received a 100% resurrection.");
}

void Client::SummonAllCorpses(const glm::vec4& position)
{
	auto summonLocation = position;
	if(IsOrigin(position) && position.w == 0.0f)
		summonLocation = GetPosition();

	auto Pack = new ServerPacket(ServerOP_DepopAllPlayersCorpses, sizeof(ServerDepopAllPlayersCorpses_Struct));

	ServerDepopAllPlayersCorpses_Struct *sdapcs = (ServerDepopAllPlayersCorpses_Struct*)Pack->pBuffer;

	sdapcs->CharacterID = CharacterID();
	sdapcs->ZoneID = zone->GetZoneID();
	sdapcs->InstanceID = zone->GetInstanceID();

	worldserver.SendPacket(Pack);

	safe_delete(Pack);

	entity_list.RemoveAllCorpsesByCharID(CharacterID());

	database.SummonAllCharacterCorpses(CharacterID(), zone->GetZoneID(), zone->GetInstanceID(), summonLocation);
}

void Client::DepopAllCorpses()
{
	auto Pack = new ServerPacket(ServerOP_DepopAllPlayersCorpses, sizeof(ServerDepopAllPlayersCorpses_Struct));

	ServerDepopAllPlayersCorpses_Struct *sdapcs = (ServerDepopAllPlayersCorpses_Struct*)Pack->pBuffer;

	sdapcs->CharacterID = CharacterID();
	sdapcs->ZoneID = zone->GetZoneID();
	sdapcs->InstanceID = zone->GetInstanceID();

	worldserver.SendPacket(Pack);

	safe_delete(Pack);

	entity_list.RemoveAllCorpsesByCharID(CharacterID());
}

void Client::DepopPlayerCorpse(uint32 dbid)
{
	auto Pack = new ServerPacket(ServerOP_DepopPlayerCorpse, sizeof(ServerDepopPlayerCorpse_Struct));

	ServerDepopPlayerCorpse_Struct *sdpcs = (ServerDepopPlayerCorpse_Struct*)Pack->pBuffer;

	sdpcs->DBID = dbid;
	sdpcs->ZoneID = zone->GetZoneID();
	sdpcs->InstanceID = zone->GetInstanceID();

	worldserver.SendPacket(Pack);

	safe_delete(Pack);

	entity_list.RemoveCorpseByDBID(dbid);
}

void Client::BuryPlayerCorpses()
{
	database.BuryAllCharacterCorpses(CharacterID());
}

void Client::NotifyNewTitlesAvailable()
{
	auto outapp = new EQApplicationPacket(OP_NewTitlesAvailable, 0);

	QueuePacket(outapp);

	safe_delete(outapp);

}

void Client::SetStartZone(uint32 zoneid, float x, float y, float z, float heading)
{
	// setting city to zero allows the player to use /setstartcity to set the city themselves
	if(zoneid == 0) {
		m_pp.binds[4].zone_id = 0;
		this->Message(Chat::Yellow,"Your starting city has been reset. Use /setstartcity to choose a new one");
		return;
	}

	// check to make sure the zone is valid
	const char *target_zone_name = ZoneName(zoneid);
	if(target_zone_name == nullptr)
		return;

	m_pp.binds[4].zone_id = zoneid;
	if(zone->GetInstanceID() != 0 && zone->IsInstancePersistent()) {
		m_pp.binds[4].instance_id = zone->GetInstanceID();
	}

	if (x == 0 && y == 0 && z == 0) {
		content_db.GetSafePoints(
			ZoneName(m_pp.binds[4].zone_id),
			0,
			&m_pp.binds[4].x,
			&m_pp.binds[4].y,
			&m_pp.binds[4].z,
			&m_pp.binds[4].heading
		);
	}
	else {
		m_pp.binds[4].x = x;
		m_pp.binds[4].y = y;
		m_pp.binds[4].z = z;
		m_pp.binds[4].heading = heading;
	}
}

uint32 Client::GetStartZone()
{
	return m_pp.binds[4].zone_id;
}

void Client::ShowSkillsWindow()
{
	const char *WindowTitle = "Skills";
	std::string WindowText;
	std::map<EQ::skills::SkillType, std::string> Skills = EQ::skills::GetSkillTypeMap();

	if (ClientVersion() < EQ::versions::ClientVersion::RoF2)
		Skills[EQ::skills::Skill1HPiercing] = "Piercing";

	// print out all available skills
	for (auto skills_iter : Skills) {
		if (skills_iter.first == EQ::skills::Skill2HPiercing && ClientVersion() < EQ::versions::ClientVersion::RoF2)
			continue;
		if (!GetSkill(skills_iter.first) && !MaxSkill(skills_iter.first))
			continue;

		WindowText += skills_iter.second;
		// line up the values
		WindowText += "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
		WindowText += itoa(this->GetSkill(skills_iter.first));
		if (MaxSkill(skills_iter.first) > 0) {
			WindowText += "/";
			WindowText += itoa(this->GetMaxSkillAfterSpecializationRules(skills_iter.first, this->MaxSkill(skills_iter.first)));
		}
		WindowText += "<br>";
	}
	this->SendPopupToClient(WindowTitle, WindowText.c_str());
}

void Client::Signal(uint32 data)
{
	char buf[32];
	snprintf(buf, 31, "%d", data);
	buf[31] = '\0';
	parse->EventPlayer(EVENT_SIGNAL, this, buf, 0);
}

void Client::SendRewards()
{
	std::vector<ClientReward> rewards;
	std::string query = StringFormat("SELECT reward_id, amount "
									"FROM account_rewards "
									"WHERE account_id = %i "
									"ORDER BY reward_id", AccountID());
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		ClientReward cr;
		cr.id = atoi(row[0]);
		cr.amount = atoi(row[1]);
		rewards.push_back(cr);
	}

	if(rewards.empty())
		return;

	auto vetapp = new EQApplicationPacket(OP_VetRewardsAvaliable, (sizeof(InternalVeteranReward) * rewards.size()));
	uchar *data = vetapp->pBuffer;
	for(int i = 0; i < rewards.size(); ++i) {
		InternalVeteranReward *ivr = (InternalVeteranReward*)data;
		ivr->claim_id = rewards[i].id;
		ivr->number_available = rewards[i].amount;
		auto iter = zone->VeteranRewards.begin();
		for (;iter != zone->VeteranRewards.end(); ++iter)
			if((*iter).claim_id == rewards[i].id)
				break;

		if(iter != zone->VeteranRewards.end()) {
			InternalVeteranReward ivro = (*iter);
			ivr->claim_count = ivro.claim_count;
			for(int x = 0; x < ivro.claim_count; ++x) {
				ivr->items[x].item_id = ivro.items[x].item_id;
				ivr->items[x].charges = ivro.items[x].charges;
				strcpy(ivr->items[x].item_name, ivro.items[x].item_name);
			}
		}

		data += sizeof(InternalVeteranReward);
	}

	FastQueuePacket(&vetapp);
}

bool Client::TryReward(uint32 claim_id)
{
	// Make sure we have an open spot
	// Make sure we have it in our acct and count > 0
	// Make sure the entry was found
	// If we meet all the criteria:
	// Decrement our count by 1 if it > 1 delete if it == 1
	// Create our item in bag if necessary at the free inv slot
	// save
	uint32 free_slot = 0xFFFFFFFF;

	for (int i = EQ::invslot::GENERAL_BEGIN; i <= EQ::invslot::GENERAL_END; ++i) {
		EQ::ItemInstance *item = GetInv().GetItem(i);
		if (!item) {
			free_slot = i;
			break;
		}
	}

	if (free_slot == 0xFFFFFFFF)
		return false;

	char errbuf[MYSQL_ERRMSG_SIZE];
	std::string query = StringFormat("SELECT amount FROM account_rewards "
					 "WHERE account_id = %i AND reward_id = %i",
					 AccountID(), claim_id);
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return false;

	if (results.RowCount() == 0)
		return false;

	auto row = results.begin();

	uint32 amt = atoi(row[0]);
	if (amt == 0)
		return false;

	auto iter = std::find_if(zone->VeteranRewards.begin(), zone->VeteranRewards.end(),
			[claim_id](const InternalVeteranReward &a) { return a.claim_id == claim_id; });

	if (iter == zone->VeteranRewards.end())
		return false;

	if (amt == 1) {
		query = StringFormat("DELETE FROM account_rewards "
				     "WHERE account_id = %i AND reward_id = %i",
				     AccountID(), claim_id);
		auto results = database.QueryDatabase(query);
	} else {
		query = StringFormat("UPDATE account_rewards SET amount = (amount-1) "
				     "WHERE account_id = %i AND reward_id = %i",
				     AccountID(), claim_id);
		auto results = database.QueryDatabase(query);
	}

	auto &ivr = (*iter);
	EQ::ItemInstance *claim = database.CreateItem(ivr.items[0].item_id, ivr.items[0].charges);
	if (!claim) {
		Save();
		return true;
	}

	bool lore_conflict = CheckLoreConflict(claim->GetItem());

	for (int y = 1; y < 8; y++)
		if (ivr.items[y].item_id && claim->GetItem()->ItemClass == 1) {
			EQ::ItemInstance *item_temp = database.CreateItem(ivr.items[y].item_id, ivr.items[y].charges);
			if (item_temp) {
				if (CheckLoreConflict(item_temp->GetItem())) {
					lore_conflict = true;
					DuplicateLoreMessage(ivr.items[y].item_id);
				}
				claim->PutItem(y - 1, *item_temp);
				safe_delete(item_temp);
			}
		}

	if (lore_conflict) {
		safe_delete(claim);
		return true;
	}

	PutItemInInventory(free_slot, *claim);
	SendItemPacket(free_slot, claim, ItemPacketTrade);
	safe_delete(claim);

	Save();
	return true;
}

uint32 Client::GetLDoNPointsTheme(uint32 t)
{
	switch(t)
	{
	case 1:
		return m_pp.ldon_points_guk;
	case 2:
		return m_pp.ldon_points_mir;
	case 3:
		return m_pp.ldon_points_mmc;
	case 4:
		return m_pp.ldon_points_ruj;
	case 5:
		return m_pp.ldon_points_tak;
	default:
		return 0;
	}
}

uint32 Client::GetLDoNWinsTheme(uint32 t)
{
	switch(t)
	{
	case 1:
		return m_pp.ldon_wins_guk;
	case 2:
		return m_pp.ldon_wins_mir;
	case 3:
		return m_pp.ldon_wins_mmc;
	case 4:
		return m_pp.ldon_wins_ruj;
	case 5:
		return m_pp.ldon_wins_tak;
	default:
		return 0;
	}
}

uint32 Client::GetLDoNLossesTheme(uint32 t)
{
	switch(t)
	{
	case 1:
		return m_pp.ldon_losses_guk;
	case 2:
		return m_pp.ldon_losses_mir;
	case 3:
		return m_pp.ldon_losses_mmc;
	case 4:
		return m_pp.ldon_losses_ruj;
	case 5:
		return m_pp.ldon_losses_tak;
	default:
		return 0;
	}
}

void Client::AddLDoNLoss(uint32 theme_id)
{
	switch (theme_id)
	{
		case 1:
			m_pp.ldon_losses_guk += 1;
			database.UpdateAdventureStatsEntry(CharacterID(), theme_id, false);
			break;
		case 2:
			m_pp.ldon_losses_mir += 1;
			database.UpdateAdventureStatsEntry(CharacterID(), theme_id, false);
			break;
		case 3:
			m_pp.ldon_losses_mmc += 1;
			database.UpdateAdventureStatsEntry(CharacterID(), theme_id, false);
			break;
		case 4:
			m_pp.ldon_losses_ruj += 1;
			database.UpdateAdventureStatsEntry(CharacterID(), theme_id, false);
			break;
		case 5:
			m_pp.ldon_losses_tak += 1;
			database.UpdateAdventureStatsEntry(CharacterID(), theme_id, false);
			break;
		default:
			return;
	}
}

void Client::AddLDoNWin(uint32 theme_id)
{
	switch (theme_id)
	{
		case 1:
			m_pp.ldon_wins_guk += 1;
			database.UpdateAdventureStatsEntry(CharacterID(), theme_id, true);
			break;
		case 2:
			m_pp.ldon_wins_mir += 1;
			database.UpdateAdventureStatsEntry(CharacterID(), theme_id, true);
			break;
		case 3:
			m_pp.ldon_wins_mmc += 1;
			database.UpdateAdventureStatsEntry(CharacterID(), theme_id, true);
			break;
		case 4:
			m_pp.ldon_wins_ruj += 1;
			database.UpdateAdventureStatsEntry(CharacterID(), theme_id, true);
			break;
		case 5:
			m_pp.ldon_wins_tak += 1;
			database.UpdateAdventureStatsEntry(CharacterID(), theme_id, true);
			break;
		default:
			return;
	}
}


void Client::SuspendMinion()
{
	NPC *CurrentPet = GetPet()->CastToNPC();

	int AALevel = GetAA(aaSuspendedMinion);

	if(AALevel == 0)
		return;

	if(GetLevel() < 62)
		return;

	if(!CurrentPet)
	{
		if(m_suspendedminion.SpellID > 0)
		{
			if (m_suspendedminion.SpellID >= SPDAT_RECORDS) {
				Message(Chat::Red, "Invalid suspended minion spell id (%u).", m_suspendedminion.SpellID);
				memset(&m_suspendedminion, 0, sizeof(PetInfo));
				return;
			}

			MakePoweredPet(m_suspendedminion.SpellID, spells[m_suspendedminion.SpellID].teleport_zone,
				m_suspendedminion.petpower, m_suspendedminion.Name, m_suspendedminion.size);

			CurrentPet = GetPet()->CastToNPC();

			if(!CurrentPet)
			{
				Message(Chat::Red, "Failed to recall suspended minion.");
				return;
			}

			if(AALevel >= 2)
			{
				CurrentPet->SetPetState(m_suspendedminion.Buffs, m_suspendedminion.Items);

				CurrentPet->SendPetBuffsToClient();
			}
			CurrentPet->CalcBonuses();

			CurrentPet->SetHP(m_suspendedminion.HP);

			CurrentPet->SetMana(m_suspendedminion.Mana);

			CurrentPet->SetTaunting(m_suspendedminion.taunting);

			MessageString(Chat::Magenta, SUSPEND_MINION_UNSUSPEND, CurrentPet->GetCleanName());

			memset(&m_suspendedminion, 0, sizeof(struct PetInfo));
			// TODO: These pet command states need to be synced ...
			// Will just fix them for now
			if (m_ClientVersionBit & EQ::versions::maskUFAndLater) {
				SetPetCommandState(PET_BUTTON_SIT, 0);
				SetPetCommandState(PET_BUTTON_STOP, 0);
				SetPetCommandState(PET_BUTTON_REGROUP, 0);
				SetPetCommandState(PET_BUTTON_FOLLOW, 1);
				SetPetCommandState(PET_BUTTON_GUARD, 0);
				// Taunt saved on client side for logging on with pet
				// In our db for when we zone.
				SetPetCommandState(PET_BUTTON_HOLD, 0);
				SetPetCommandState(PET_BUTTON_GHOLD, 0);
				SetPetCommandState(PET_BUTTON_FOCUS, 0);
				SetPetCommandState(PET_BUTTON_SPELLHOLD, 0);
			}
		}
		else
			return;

	}
	else
	{
		uint16 SpellID = CurrentPet->GetPetSpellID();

		if(SpellID)
		{
			if(m_suspendedminion.SpellID > 0)
			{
				MessageString(Chat::Red,ONLY_ONE_PET);

				return;
			}
			else if(CurrentPet->IsEngaged())
			{
				MessageString(Chat::Red,SUSPEND_MINION_FIGHTING);

				return;
			}
			else if(entity_list.Fighting(CurrentPet))
			{
				MessageString(Chat::Blue,SUSPEND_MINION_HAS_AGGRO);
			}
			else
			{
				m_suspendedminion.SpellID = SpellID;

				m_suspendedminion.HP = CurrentPet->GetHP();;

				m_suspendedminion.Mana = CurrentPet->GetMana();
				m_suspendedminion.petpower = CurrentPet->GetPetPower();
				m_suspendedminion.size = CurrentPet->GetSize();

				if(AALevel >= 2)
					CurrentPet->GetPetState(m_suspendedminion.Buffs, m_suspendedminion.Items, m_suspendedminion.Name);
				else
					strn0cpy(m_suspendedminion.Name, CurrentPet->GetName(), 64); // Name stays even at rank 1

				MessageString(Chat::Magenta, SUSPEND_MINION_SUSPEND, CurrentPet->GetCleanName());

				CurrentPet->Depop(false);

				SetPetID(0);
			}
		}
		else
		{
			MessageString(Chat::Red, ONLY_SUMMONED_PETS);

			return;
		}
	}
}

void Client::AddPVPPoints(uint32 Points)
{
	m_pp.PVPCurrentPoints += Points;
	m_pp.PVPCareerPoints += Points;

	Save();

	SendPVPStats();
}

void Client::AddCrystals(uint32 Radiant, uint32 Ebon)
{
	m_pp.currentRadCrystals += Radiant;
	m_pp.careerRadCrystals += Radiant;
	m_pp.currentEbonCrystals += Ebon;
	m_pp.careerEbonCrystals += Ebon;

	SaveCurrency();

	SendCrystalCounts();
}

void Client::SetEbonCrystals(uint32 value) {
	m_pp.currentEbonCrystals = value;
	SaveCurrency();
	SendCrystalCounts();
}

void Client::SetRadiantCrystals(uint32 value) {
	m_pp.currentRadCrystals = value;
	SaveCurrency();
	SendCrystalCounts();
}

// Processes a client request to inspect a SoF+ client's equipment.
void Client::ProcessInspectRequest(Client* requestee, Client* requester) {
	if(requestee && requester) {
		auto outapp = new EQApplicationPacket(OP_InspectAnswer, sizeof(InspectResponse_Struct));
		InspectResponse_Struct* insr = (InspectResponse_Struct*) outapp->pBuffer;
		insr->TargetID = requester->GetID();
		insr->playerid = requestee->GetID();

		const EQ::ItemData* item = nullptr;
		const EQ::ItemInstance* inst = nullptr;
		int ornamentationAugtype = RuleI(Character, OrnamentationAugmentType);
		for(int16 L = EQ::invslot::EQUIPMENT_BEGIN; L <= EQ::invslot::EQUIPMENT_END; L++) {
			inst = requestee->GetInv().GetItem(L);

			if(inst) {
				item = inst->GetItem();
				if(item) {
					strcpy(insr->itemnames[L], item->Name);

					const EQ::ItemData *aug_item = nullptr;
					if (inst->GetOrnamentationAug(ornamentationAugtype))
						inst->GetOrnamentationAug(ornamentationAugtype)->GetItem();

					if (aug_item)
						insr->itemicons[L] = aug_item->Icon;
					else if (inst->GetOrnamentationIcon())
						insr->itemicons[L] = inst->GetOrnamentationIcon();
					else
						insr->itemicons[L] = item->Icon;
				}
				else {
					insr->itemnames[L][0] = '\0';
					insr->itemicons[L] = 0xFFFFFFFF;
				}
			}
			else {
				insr->itemnames[L][0] = '\0';
				insr->itemicons[L] = 0xFFFFFFFF;
			}
		}

		strcpy(insr->text, requestee->GetInspectMessage().text);

		// There could be an OP for this..or not... (Ti clients are not processed here..this message is generated client-side)
		if(requestee->IsClient() && (requestee != requester)) { requestee->Message(Chat::White, "%s is looking at your equipment...", requester->GetName()); }

		requester->QueuePacket(outapp); // Send answer to requester
		safe_delete(outapp);
	}
}

void Client::GuildBankAck()
{
	auto outapp = new EQApplicationPacket(OP_GuildBank, sizeof(GuildBankAck_Struct));

	GuildBankAck_Struct *gbas = (GuildBankAck_Struct*) outapp->pBuffer;

	gbas->Action = GuildBankAcknowledge;

	FastQueuePacket(&outapp);
}

void Client::GuildBankDepositAck(bool Fail, int8 action)
{

	auto outapp = new EQApplicationPacket(OP_GuildBank, sizeof(GuildBankDepositAck_Struct));

	GuildBankDepositAck_Struct *gbdas = (GuildBankDepositAck_Struct*) outapp->pBuffer;

	gbdas->Action = action;

	gbdas->Fail = Fail ? 1 : 0;

	FastQueuePacket(&outapp);
}

void Client::ClearGuildBank()
{
	auto outapp = new EQApplicationPacket(OP_GuildBank, sizeof(GuildBankClear_Struct));

	GuildBankClear_Struct *gbcs = (GuildBankClear_Struct*) outapp->pBuffer;

	gbcs->Action = GuildBankBulkItems;
	gbcs->DepositAreaCount = 0;
	gbcs->MainAreaCount = 0;

	FastQueuePacket(&outapp);
}

void Client::SendGroupCreatePacket()
{
	// For SoD and later clients, this is sent the Group Leader upon initial creation of the group
	//
	auto outapp = new EQApplicationPacket(OP_GroupUpdateB, 32 + strlen(GetName()));

	char *Buffer = (char *)outapp->pBuffer;
	// Header
	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);	// group ID probably
	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 1);	// count of members in packet
	VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// Null Leader name, shouldn't be null besides this case

	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);	// Member 0, index
	VARSTRUCT_ENCODE_STRING(Buffer, GetName());	// group member name
	VARSTRUCT_ENCODE_TYPE(uint16, Buffer, 0);	// merc flag
	VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// owner name (if merc)
	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, GetLevel());	// level
	VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// group tank flag
	VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// group assist flag
	VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// group puller flag
	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);	// offline flag
	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);	// timestamp

	FastQueuePacket(&outapp);
}

void Client::SendGroupLeaderChangePacket(const char *LeaderName)
{
	// For SoD and later, send name of Group Leader to this client

	auto outapp = new EQApplicationPacket(OP_GroupLeaderChange, sizeof(GroupLeaderChange_Struct));

	GroupLeaderChange_Struct *glcs = (GroupLeaderChange_Struct*)outapp->pBuffer;

	strn0cpy(glcs->LeaderName, LeaderName, sizeof(glcs->LeaderName));

	FastQueuePacket(&outapp);
}

void Client::SendGroupJoinAcknowledge()
{
	// For SoD and later, This produces the 'You have joined the group' message.
	auto outapp = new EQApplicationPacket(OP_GroupAcknowledge, 4);
	FastQueuePacket(&outapp);
}

void Client::SendAdventureError(const char *error)
{
	size_t error_size = strlen(error);
	auto outapp = new EQApplicationPacket(OP_AdventureInfo, (error_size + 2));
	strn0cpy((char*)outapp->pBuffer, error, error_size);
	FastQueuePacket(&outapp);
}

void Client::SendAdventureDetails()
{
	if(adv_data)
	{
		ServerSendAdventureData_Struct *ad = (ServerSendAdventureData_Struct*)adv_data;
		auto outapp = new EQApplicationPacket(OP_AdventureData, sizeof(AdventureRequestResponse_Struct));
		AdventureRequestResponse_Struct *arr = (AdventureRequestResponse_Struct*)outapp->pBuffer;
		arr->unknown000 = 0xBFC40100;
		arr->unknown2080 = 0x0A;
		arr->risk = ad->risk;
		strcpy(arr->text, ad->text);

		if(ad->time_to_enter != 0)
		{
			arr->timetoenter = ad->time_to_enter;
		}
		else
		{
			arr->timeleft = ad->time_left;
		}

		if(ad->zone_in_id == zone->GetZoneID())
		{
			arr->y = ad->x;
			arr->x = ad->y;
			arr->showcompass = 1;
		}
		FastQueuePacket(&outapp);

		SendAdventureCount(ad->count, ad->total);
	}
	else
	{
		ServerSendAdventureData_Struct *ad = (ServerSendAdventureData_Struct*)adv_data;
		auto outapp = new EQApplicationPacket(OP_AdventureData, sizeof(AdventureRequestResponse_Struct));
		FastQueuePacket(&outapp);
	}
}

void Client::SendAdventureCount(uint32 count, uint32 total)
{
	auto outapp = new EQApplicationPacket(OP_AdventureUpdate, sizeof(AdventureCountUpdate_Struct));
	AdventureCountUpdate_Struct *acu = (AdventureCountUpdate_Struct*)outapp->pBuffer;
	acu->current = count;
	acu->total = total;
	FastQueuePacket(&outapp);
}

void Client::NewAdventure(int id, int theme, const char *text, int member_count, const char *members)
{
	size_t text_size = strlen(text);
	auto outapp = new EQApplicationPacket(OP_AdventureDetails, text_size + 2);
	strn0cpy((char*)outapp->pBuffer, text, text_size);
	FastQueuePacket(&outapp);

	adv_requested_id = id;
	adv_requested_theme = theme;
	safe_delete_array(adv_requested_data);
	adv_requested_member_count = member_count;
	adv_requested_data = new char[64 * member_count];
	memcpy(adv_requested_data, members, (64 * member_count));
}

void Client::ClearPendingAdventureData()
{
	adv_requested_id = 0;
	adv_requested_theme = 0;
	safe_delete_array(adv_requested_data);
	adv_requested_member_count = 0;
}

bool Client::IsOnAdventure()
{
	if(adv_data)
	{
		ServerSendAdventureData_Struct *ad = (ServerSendAdventureData_Struct*)adv_data;
		if(ad->zone_in_id == 0)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	return false;
}

void Client::LeaveAdventure()
{
	if(!GetPendingAdventureLeave())
	{
		PendingAdventureLeave();
		auto pack = new ServerPacket(ServerOP_AdventureLeave, 64);
		strcpy((char*)pack->pBuffer, GetName());
		worldserver.SendPacket(pack);
		delete pack;
	}
}

void Client::ClearCurrentAdventure()
{
	if(adv_data)
	{
		ServerSendAdventureData_Struct* ds = (ServerSendAdventureData_Struct*)adv_data;
		if(ds->finished_adventures > 0)
		{
			ds->instance_id = 0;
			ds->risk = 0;
			memset(ds->text, 0, 512);
			ds->time_left = 0;
			ds->time_to_enter = 0;
			ds->x = 0;
			ds->y = 0;
			ds->zone_in_id = 0;
			ds->zone_in_object = 0;
		}
		else
		{
			safe_delete(adv_data);
		}

		SendAdventureError("You are not currently assigned to an adventure.");
	}
}

void Client::AdventureFinish(bool win, int theme, int points)
{
	UpdateLDoNPoints(theme, points);
	auto outapp = new EQApplicationPacket(OP_AdventureFinish, sizeof(AdventureFinish_Struct));
	AdventureFinish_Struct *af = (AdventureFinish_Struct*)outapp->pBuffer;
	af->win_lose = win ? 1 : 0;
	af->points = points;
	FastQueuePacket(&outapp);
}

void Client::CheckLDoNHail(Mob *target)
{
	if(!zone->adv_data)
	{
		return;
	}

	if(!target || !target->IsNPC())
	{
		return;
	}

	if(target->GetOwnerID() != 0)
	{
		return;
	}

	ServerZoneAdventureDataReply_Struct* ds = (ServerZoneAdventureDataReply_Struct*)zone->adv_data;
	if(ds->type != Adventure_Rescue)
	{
		return;
	}

	if(ds->data_id != target->GetNPCTypeID())
	{
		return;
	}

	if(entity_list.CheckNPCsClose(target) != 0)
	{
		target->Say("You're here to save me? I couldn't possibly risk leaving yet. There are "
			"far too many of those horrid things out there waiting to recapture me! Please get"
			" rid of some more of those vermin and then we can try to leave.");
		return;
	}

	Mob *pet = GetPet();
	if(pet)
	{
		if(pet->GetPetType() == petCharmed)
		{
			pet->BuffFadeByEffect(SE_Charm);
		}
		else if(pet->GetPetType() == petNPCFollow)
		{
			pet->SetOwnerID(0);
		}
		else
		{
			pet->Depop();
		}
	}

	SetPet(target);
	target->SetOwnerID(GetID());
	target->Say("Wonderful! Someone to set me free! I feared for my life for so long,"
		" never knowing when they might choose to end my life. Now that you're here though"
		" I can rest easy. Please help me find my way out of here as soon as you can"
		" I'll stay close behind you!");
}

void Client::CheckEmoteHail(Mob *target, const char* message)
{
	if(
		(message[0] != 'H'	&&
		message[0] != 'h')	||
		message[1] != 'a'	||
		message[2] != 'i'	||
		message[3] != 'l'){
		return;
	}

	if(!target || !target->IsNPC())
	{
		return;
	}

	if(target->GetOwnerID() != 0)
	{
		return;
	}
	uint16 emoteid = target->GetEmoteID();
	if(emoteid != 0)
		target->CastToNPC()->DoNPCEmote(HAILED,emoteid);
}

void Client::MarkSingleCompassLoc(float in_x, float in_y, float in_z, uint8 count)
{
	m_has_quest_compass = (count != 0);
	m_quest_compass.x = in_x;
	m_quest_compass.y = in_y;
	m_quest_compass.z = in_z;

	SendDzCompassUpdate();
}

void Client::SendZonePoints()
{
	int                             count = 0;
	LinkedListIterator<ZonePoint *> iterator(zone->zone_point_list);
	iterator.Reset();
	while (iterator.MoreElements()) {
		ZonePoint *data = iterator.GetData();

		if (ClientVersionBit() & data->client_version_mask) {
			count++;
		}

		iterator.Advance();
	}

	uint32 zpsize = sizeof(ZonePoints) + ((count + 1) * sizeof(ZonePoint_Entry));
	auto outapp = new EQApplicationPacket(OP_SendZonepoints, zpsize);
	ZonePoints* zp = (ZonePoints*)outapp->pBuffer;
	zp->count = count;

	int i = 0;
	iterator.Reset();
	while(iterator.MoreElements())
	{
		ZonePoint* data = iterator.GetData();

		LogZonePoints(
			"Sending zone point to client [{}] mask [{}] x [{}] y [{}] z [{}] number [{}]",
			GetCleanName(),
			ClientVersionBit() & data->client_version_mask ? "true" : "false",
			data->x,
			data->y,
			data->z,
			data->number
		);

		if(ClientVersionBit() & data->client_version_mask)
		{
			zp->zpe[i].iterator = data->number;
			zp->zpe[i].x = data->target_x;
			zp->zpe[i].y = data->target_y;
			zp->zpe[i].z = data->target_z;
			zp->zpe[i].heading = data->target_heading;
			zp->zpe[i].zoneid = data->target_zone_id;
			zp->zpe[i].zoneinstance = data->target_zone_instance;
			i++;
		}
		iterator.Advance();
	}

	FastQueuePacket(&outapp);
}

void Client::SendTargetCommand(uint32 EntityID)
{
	auto outapp = new EQApplicationPacket(OP_TargetCommand, sizeof(ClientTarget_Struct));
	ClientTarget_Struct *cts = (ClientTarget_Struct*)outapp->pBuffer;
	cts->new_target = EntityID;
	FastQueuePacket(&outapp);
}

void Client::LocateCorpse()
{
	Corpse *ClosestCorpse = nullptr;
	if(!GetTarget())
		ClosestCorpse = entity_list.GetClosestCorpse(this, nullptr);
	else if(GetTarget()->IsCorpse())
		ClosestCorpse = entity_list.GetClosestCorpse(this, GetTarget()->CastToCorpse()->GetOwnerName());
	else
		ClosestCorpse = entity_list.GetClosestCorpse(this, GetTarget()->GetCleanName());

	if(ClosestCorpse)
	{
		MessageString(Chat::Spells, SENSE_CORPSE_DIRECTION);
		SetHeading(CalculateHeadingToTarget(ClosestCorpse->GetX(), ClosestCorpse->GetY()));
		SetTarget(ClosestCorpse);
		SendTargetCommand(ClosestCorpse->GetID());
		SentPositionPacket(0.0f, 0.0f, 0.0f, 0.0f, 0, true);
	}
	else if(!GetTarget())
		MessageString(Chat::Red, SENSE_CORPSE_NONE);
	else
		MessageString(Chat::Red, SENSE_CORPSE_NOT_NAME);
}

void Client::NPCSpawn(NPC *target_npc, const char *identifier, uint32 extra)
{
	if (!target_npc || !identifier)
		return;

	std::string id = identifier;
	for(int i = 0; i < id.length(); ++i)
	{
		id[i] = tolower(id[i]);
	}

	if (id == "create") {
		// extra tries to create the npc_type ID within the range for the current zone (zone_id * 1000)
		content_db.NPCSpawnDB(0, zone->GetShortName(), zone->GetInstanceVersion(), this, target_npc->CastToNPC(), extra);
	}
	else if (id == "add") {
		// extra sets the respawn timer for add
		content_db.NPCSpawnDB(1, zone->GetShortName(), zone->GetInstanceVersion(), this, target_npc->CastToNPC(), extra);
	}
	else if (id == "update") {
		content_db.NPCSpawnDB(2, zone->GetShortName(), zone->GetInstanceVersion(), this, target_npc->CastToNPC());
	}
	else if (id == "remove") {
		content_db.NPCSpawnDB(3, zone->GetShortName(), zone->GetInstanceVersion(), this, target_npc->CastToNPC());
		target_npc->Depop(false);
	}
	else if (id == "delete") {
		content_db.NPCSpawnDB(4, zone->GetShortName(), zone->GetInstanceVersion(), this, target_npc->CastToNPC());
		target_npc->Depop(false);
	}
	else {
		return;
	}
}

bool Client::IsDraggingCorpse(uint16 CorpseID)
{
	for (auto It = DraggedCorpses.begin(); It != DraggedCorpses.end(); ++It) {
		if (It->second == CorpseID)
			return true;
	}

	return false;
}

void Client::DragCorpses()
{
	for (auto It = DraggedCorpses.begin(); It != DraggedCorpses.end(); ++It) {
		Mob *corpse = entity_list.GetMob(It->second);

		if (corpse && corpse->IsPlayerCorpse() &&
				(DistanceSquaredNoZ(m_Position, corpse->GetPosition()) <= RuleR(Character, DragCorpseDistance)))
			continue;

		if (!corpse || !corpse->IsPlayerCorpse() ||
				corpse->CastToCorpse()->IsBeingLooted() ||
				!corpse->CastToCorpse()->Summon(this, false, false)) {
			MessageString(Chat::DefaultText, CORPSEDRAG_STOP);
			It = DraggedCorpses.erase(It);
			if (It == DraggedCorpses.end())
				break;
		}
	}
}

void Client::ConsentCorpses(std::string consent_name, bool deny)
{
	if (strcasecmp(consent_name.c_str(), GetName()) == 0) {
		MessageString(Chat::Red, CONSENT_YOURSELF);
	}
	else if (!consent_throttle_timer.Check()) {
		MessageString(Chat::Red, CONSENT_WAIT);
	}
	else {
		auto pack = new ServerPacket(ServerOP_Consent, sizeof(ServerOP_Consent_Struct));
		ServerOP_Consent_Struct* scs = (ServerOP_Consent_Struct*)pack->pBuffer;
		strn0cpy(scs->grantname, consent_name.c_str(), sizeof(scs->grantname));
		strn0cpy(scs->ownername, GetName(), sizeof(scs->ownername));
		strn0cpy(scs->zonename, "Unknown", sizeof(scs->zonename));
		scs->permission = deny ? 0 : 1;
		scs->zone_id = zone->GetZoneID();
		scs->instance_id = zone->GetInstanceID();
		scs->consent_type = EQ::consent::Normal;
		scs->consent_id = 0;
		if (strcasecmp(scs->grantname, "group") == 0) {
			if (!deny) {
				Group* grp = GetGroup();
				scs->consent_id = grp ? grp->GetID() : 0;
			}
			scs->consent_type = EQ::consent::Group;
		}
		else if (strcasecmp(scs->grantname, "raid") == 0) {
			if (!deny) {
				Raid* raid = GetRaid();
				scs->consent_id = raid ? raid->GetID() : 0;
			}
			scs->consent_type = EQ::consent::Raid;
		}
		else if (strcasecmp(scs->grantname, "guild") == 0) {
			if (!deny) {
				scs->consent_id = GuildID();
			}
			scs->consent_type = EQ::consent::Guild;
			// update all corpses in db so buried/unloaded corpses see new consent id
			database.UpdateCharacterCorpseConsent(CharacterID(), scs->consent_id);
		}
		worldserver.SendPacket(pack);
		safe_delete(pack);
	}
}

void Client::Doppelganger(uint16 spell_id, Mob *target, const char *name_override, int pet_count, int pet_duration)
{
	if(!target || !IsValidSpell(spell_id) || this->GetID() == target->GetID())
		return;

	PetRecord record;
	if(!database.GetPetEntry(spells[spell_id].teleport_zone, &record))
	{
		LogError("Unknown doppelganger spell id: [{}], check pets table", spell_id);
		Message(Chat::Red, "Unable to find data for pet %s", spells[spell_id].teleport_zone);
		return;
	}

	SwarmPet_Struct pet;
	pet.count = pet_count;
	pet.duration = pet_duration;
	pet.npc_id = record.npc_type;

	NPCType *made_npc = nullptr;

	const NPCType *npc_type = content_db.LoadNPCTypesData(pet.npc_id);
	if(npc_type == nullptr) {
		LogError("Unknown npc type for doppelganger spell id: [{}]", spell_id);
		Message(0,"Unable to find pet!");
		return;
	}
	// make a custom NPC type for this
	made_npc = new NPCType;
	memcpy(made_npc, npc_type, sizeof(NPCType));

	strcpy(made_npc->name, name_override);
	made_npc->level = GetLevel();
	made_npc->race = GetRace();
	made_npc->gender = GetGender();
	made_npc->size = GetSize();
	made_npc->AC = GetAC();
	made_npc->STR = GetSTR();
	made_npc->STA = GetSTA();
	made_npc->DEX = GetDEX();
	made_npc->AGI = GetAGI();
	made_npc->MR = GetMR();
	made_npc->FR = GetFR();
	made_npc->CR = GetCR();
	made_npc->DR = GetDR();
	made_npc->PR = GetPR();
	made_npc->Corrup = GetCorrup();
	made_npc->PhR = GetPhR();
	// looks
	made_npc->texture = GetEquipmentMaterial(EQ::textures::armorChest);
	made_npc->helmtexture = GetEquipmentMaterial(EQ::textures::armorHead);
	made_npc->haircolor = GetHairColor();
	made_npc->beardcolor = GetBeardColor();
	made_npc->eyecolor1 = GetEyeColor1();
	made_npc->eyecolor2 = GetEyeColor2();
	made_npc->hairstyle = GetHairStyle();
	made_npc->luclinface = GetLuclinFace();
	made_npc->beard = GetBeard();
	made_npc->drakkin_heritage = GetDrakkinHeritage();
	made_npc->drakkin_tattoo = GetDrakkinTattoo();
	made_npc->drakkin_details = GetDrakkinDetails();
	made_npc->d_melee_texture1 = GetEquipmentMaterial(EQ::textures::weaponPrimary);
	made_npc->d_melee_texture2 = GetEquipmentMaterial(EQ::textures::weaponSecondary);
	for (int i = EQ::textures::textureBegin; i <= EQ::textures::LastTexture; i++)	{
		made_npc->armor_tint.Slot[i].Color = GetEquipmentColor(i);
	}
	made_npc->loottable_id = 0;

	npc_type = made_npc;

	int summon_count = 0;
	summon_count = pet.count;

	if(summon_count > MAX_SWARM_PETS)
		summon_count = MAX_SWARM_PETS;

	static const glm::vec2 swarmPetLocations[MAX_SWARM_PETS] = {
		glm::vec2(5, 5), glm::vec2(-5, 5), glm::vec2(5, -5), glm::vec2(-5, -5),
		glm::vec2(10, 10), glm::vec2(-10, 10), glm::vec2(10, -10), glm::vec2(-10, -10),
		glm::vec2(8, 8), glm::vec2(-8, 8), glm::vec2(8, -8), glm::vec2(-8, -8)
	};

	while(summon_count > 0) {
		NPCType *npc_dup = nullptr;
		if(made_npc != nullptr) {
			npc_dup = new NPCType;
			memcpy(npc_dup, made_npc, sizeof(NPCType));
		}

		NPC* swarm_pet_npc = new NPC(
				(npc_dup!=nullptr)?npc_dup:npc_type,	//make sure we give the NPC the correct data pointer
				0,
				GetPosition() + glm::vec4(swarmPetLocations[summon_count], 0.0f, 0.0f),
				GravityBehavior::Water);

		if(!swarm_pet_npc->GetSwarmInfo()){
			auto nSI = new SwarmPet;
			swarm_pet_npc->SetSwarmInfo(nSI);
			swarm_pet_npc->GetSwarmInfo()->duration = new Timer(pet_duration*1000);
		}
		else{
			swarm_pet_npc->GetSwarmInfo()->duration->Start(pet_duration*1000);
		}

		swarm_pet_npc->StartSwarmTimer(pet_duration * 1000);

		swarm_pet_npc->GetSwarmInfo()->owner_id = GetID();

		// Give the pets alittle more agro than the caster and then agro them on the target
		target->AddToHateList(swarm_pet_npc, (target->GetHateAmount(this) + 100), (target->GetDamageAmount(this) + 100));
		swarm_pet_npc->AddToHateList(target, 1000, 1000);
		swarm_pet_npc->GetSwarmInfo()->target = target->GetID();

		//we allocated a new NPC type object, give the NPC ownership of that memory
		if(npc_dup != nullptr)
			swarm_pet_npc->GiveNPCTypeData(npc_dup);

		entity_list.AddNPC(swarm_pet_npc);
		summon_count--;
	}
}

void Client::AssignToInstance(uint16 instance_id)
{
	database.AddClientToInstance(instance_id, CharacterID());
}

void Client::RemoveFromInstance(uint16 instance_id)
{
	database.RemoveClientFromInstance(instance_id, CharacterID());
}

void Client::SendStatsWindow(Client* client, bool use_window)
{
	// Define the types of page breaks we need
	std::string indP = "&nbsp;";
	std::string indS = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
	std::string indM = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
	std::string indL = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
	std::string div = " | ";

	std::string color_red = "<c \"#993333\">";
	std::string color_blue = "<c \"#9999FF\">";
	std::string color_green = "<c \"#33FF99\">";
	std::string bright_green = "<c \"#7CFC00\">";
	std::string bright_red = "<c \"#FF0000\">";
	std::string heroic_color = "<c \"#d6b228\"> +";

	// Set Class
	std::string class_Name = itoa(GetClass());
	std::string class_List[] = { "WAR", "CLR", "PAL", "RNG", "SHD", "DRU", "MNK", "BRD", "ROG", "SHM", "NEC", "WIZ", "MAG", "ENC", "BST", "BER" };

	if (GetClass() < 17 && GetClass() > 0) {
		class_Name = class_List[GetClass() - 1];
	}

	// Race
	std::string race_name = GetRaceIDName(GetRace());

	/*##########################################################
	^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		H/M/E String
	^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	##########################################################*/
	std::string HME_row = "";
	//Loop Variables
	/*===========================*/
	std::string		cur_field = "";
	std::string		total_field = "";
	std::string		cur_name = "";
	std::string		cur_spacing = "";
	std::string		cur_color = "";

	int				hme_rows = 3; // Rows in display
	int				max_HME_value_len = 9; // 9 digits in the displayed value

	for(int hme_row_counter = 0; hme_row_counter < hme_rows; hme_row_counter++)
	{
		switch(hme_row_counter) {
			case 0: {
				cur_name = " H: ";
				cur_field = itoa(GetHP());
				total_field = itoa(GetMaxHP());
				break;
			}
			case 1: {
				if(CalcMaxMana() > 0) {
					cur_name = " M: ";
					cur_field = itoa(GetMana());
					total_field = itoa(CalcMaxMana());
				}
				else { continue; }

				break;
			}
			case 2: {
				cur_name = " E: ";
				cur_field = itoa(GetEndurance());
				total_field = itoa(GetMaxEndurance());
				break;
			}
			default: { break; }
		}
		if(cur_field.compare(total_field) == 0) { cur_color = bright_green; }
		else { cur_color = bright_red; }

		cur_spacing.clear();
		for(int a = cur_field.size(); a < max_HME_value_len; a++) { cur_spacing += " ."; }

		HME_row += indM + cur_name + cur_spacing + cur_color + cur_field + "</c> / " + total_field + "<br>";
	}
	/*##########################################################
	^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		Regen String
	^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	##########################################################*/
	std::string regen_string;
	//Loop Variables
	/*===========================*/
	std::string		regen_row_header = "";
	std::string		regen_row_color = "";
	std::string		base_regen_field = "";
	std::string		base_regen_spacing = "";
	std::string		item_regen_field = "";
	std::string		item_regen_spacing = "";
	std::string		cap_regen_field = "";
	std::string		cap_regen_spacing = "";
	std::string		spell_regen_field = "";
	std::string		spell_regen_spacing = "";
	std::string		aa_regen_field = "";
	std::string		aa_regen_spacing = "";
	std::string		total_regen_field = "";
	int		regen_rows = 3; // Number of rows
	int		max_regen_value_len = 5; // 5 digits in the displayed value(larger values will not get cut off, this is just a baseline)

	for(int regen_row_counter = 0; regen_row_counter < regen_rows; regen_row_counter++)
	{
		switch(regen_row_counter)
		{
			case 0: {
				regen_row_header = "H: ";
				regen_row_color = color_red;

				base_regen_field = itoa(LevelRegen());
				item_regen_field = itoa(itembonuses.HPRegen);
				cap_regen_field = itoa(CalcHPRegenCap());
				spell_regen_field = itoa(spellbonuses.HPRegen);
				aa_regen_field = itoa(aabonuses.HPRegen);
				total_regen_field = itoa(CalcHPRegen(true));
				break;
			}
			case 1: {
				if(CalcMaxMana() > 0) {
					regen_row_header = "M: ";
					regen_row_color = color_blue;

					base_regen_field = itoa(CalcBaseManaRegen());
					item_regen_field = itoa(itembonuses.ManaRegen);
					cap_regen_field = itoa(CalcManaRegenCap());
					spell_regen_field = itoa(spellbonuses.ManaRegen);
					aa_regen_field = itoa(aabonuses.ManaRegen);
					total_regen_field = itoa(CalcManaRegen(true));
				}
				else { continue; }
				break;
			}
			case 2: {
				regen_row_header = "E: ";
				regen_row_color = color_green;

				base_regen_field = itoa(((GetLevel() * 4 / 10) + 2));
				item_regen_field = itoa(itembonuses.EnduranceRegen);
				cap_regen_field = itoa(CalcEnduranceRegenCap());
				spell_regen_field = itoa(spellbonuses.EnduranceRegen);
				aa_regen_field = itoa(aabonuses.EnduranceRegen);
				total_regen_field = itoa(CalcEnduranceRegen(true));
				break;
			}
			default: { break; }
		}

		base_regen_spacing.clear();
		item_regen_spacing.clear();
		cap_regen_spacing.clear();
		spell_regen_spacing.clear();
		aa_regen_spacing.clear();

		for(int b = base_regen_field.size(); b < max_regen_value_len; b++) { base_regen_spacing += " ."; }
		for(int b = item_regen_field.size(); b < max_regen_value_len; b++) { item_regen_spacing += " ."; }
		for(int b = cap_regen_field.size(); b < max_regen_value_len; b++) { cap_regen_spacing += " ."; }
		for(int b = spell_regen_field.size(); b < max_regen_value_len; b++) { spell_regen_spacing += " ."; }
		for(int b = aa_regen_field.size(); b < max_regen_value_len; b++) { aa_regen_spacing += " ."; }

		regen_string += indS + regen_row_color + regen_row_header + base_regen_spacing + base_regen_field;
		regen_string += div + item_regen_spacing + item_regen_field + " (" + cap_regen_field;
		regen_string += ") " + cap_regen_spacing + div + spell_regen_spacing + spell_regen_field;
		regen_string += div + aa_regen_spacing + aa_regen_field + div + total_regen_field + "</c><br>";
	}
	/*##########################################################
	^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		Stat String
	^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	##########################################################*/
	std::string stat_field = "";
	//Loop Variables
	/*===========================*/
	//first field(stat)
	std::string		a_stat = "";;
	std::string		a_stat_name = "";
	std::string		a_stat_spacing = "";
	//second field(heroic stat)
	std::string		h_stat = "";
	std::string		h_stat_spacing = "";
	//third field(resist)
	std::string		a_resist = "";
	std::string		a_resist_name = "";
	std::string		a_resist_spacing = "";
	//fourth field(heroic resist)
	std::string		h_resist_field = "";

	int				stat_rows = 7; // Number of rows
	int				max_stat_value_len = 3; // 3 digits in the displayed value

	for(int stat_row_counter = 0; stat_row_counter < stat_rows; stat_row_counter++)
	{
		switch(stat_row_counter) {
			case 0: {
				a_stat_name = " STR: ";
				a_resist_name = "MR: ";
				a_stat = itoa(GetSTR());
				h_stat = itoa(GetHeroicSTR());
				a_resist = itoa(GetMR());
				h_resist_field = itoa(GetHeroicMR());
				break;
			}
			case 1: {
				a_stat_name = " STA: ";
				a_resist_name = "CR: ";
				a_stat = itoa(GetSTA());
				h_stat = itoa(GetHeroicSTA());
				a_resist = itoa(GetCR());
				h_resist_field = itoa(GetHeroicCR());
				break;
			}
			case 2: {
				a_stat_name = " AGI : ";
				a_resist_name = "FR: ";
				a_stat = itoa(GetAGI());
				h_stat = itoa(GetHeroicAGI());
				a_resist = itoa(GetFR());
				h_resist_field = itoa(GetHeroicFR());
				break;
			}
			case 3: {
				a_stat_name = " DEX: ";
				a_resist_name = "PR: ";
				a_stat = itoa(GetDEX());
				h_stat = itoa(GetHeroicDEX());
				a_resist = itoa(GetPR());
				h_resist_field = itoa(GetHeroicPR());
				break;
			}
			case 4: {
				a_stat_name = " INT : ";
				a_resist_name = "DR: ";
				a_stat = itoa(GetINT());
				h_stat = itoa(GetHeroicINT());
				a_resist = itoa(GetDR());
				h_resist_field = itoa(GetHeroicDR());
				break;
			}
			case 5: {
				a_stat_name = " WIS: ";
				a_resist_name = "Cp: ";
				a_stat = itoa(GetWIS());
				h_stat = itoa(GetHeroicWIS());
				a_resist = itoa(GetCorrup());
				h_resist_field = itoa(GetHeroicCorrup());
				break;
			}
			case 6: {
				a_stat_name = " CHA: ";
				a_resist_name = "PhR: "; // Not implemented for clients yet
				a_stat = itoa(GetCHA());
				h_stat = itoa(GetHeroicCHA());
				a_resist = itoa(GetPhR());
				h_resist_field = itoa(GetHeroicPhR());
				break;
			}
			default: { break; }
		}

		a_stat_spacing.clear();
		h_stat_spacing.clear();
		a_resist_spacing.clear();

		for(int a = a_stat.size(); a < max_stat_value_len; a++) { a_stat_spacing += " . "; }
		for(int h = h_stat.size(); h < 20; h++) { h_stat_spacing += " . "; }
		for(int h = a_resist.size(); h < max_stat_value_len; h++) { a_resist_spacing += " . "; }

		stat_field += indP + a_stat_name + a_stat_spacing + a_stat + heroic_color + h_stat + "</c>";
		stat_field += h_stat_spacing + a_resist_name + a_resist_spacing + a_resist + heroic_color + h_resist_field + "</c>";
		if(stat_row_counter < 6) {
			stat_field += "<br>";
		}
	}
	/*##########################################################
	^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		Mod2 String
	^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	##########################################################*/
	std::string mod2_field = "";
	//Loop Variables
	/*===========================*/
	std::string		mod2a = "";
	std::string		mod2a_name = "";
	std::string		mod2a_spacing = "";
	std::string		mod2a_cap = "";
	std::string		mod_row_spacing = "";
	std::string		mod2b = "";
	std::string		mod2b_name = "";
	std::string		mod2b_spacing = "";
	std::string		mod2b_cap = "";
	int				mod2a_space_count;
	int				mod2b_space_count;

	int		mod2_rows = 4;
	int		max_mod2_value_len = 3; // 3 digits in the displayed value

	for(int mod2_row_counter = 0; mod2_row_counter < mod2_rows; mod2_row_counter++)
	{
		switch (mod2_row_counter)
		{
			case 0: {
				mod2a_name = "Avoidance: ";
				mod2b_name = "Combat Effects: ";
				mod2a = itoa(GetAvoidance());
				mod2a_cap = itoa(RuleI(Character, ItemAvoidanceCap));
				mod2b = itoa(GetCombatEffects());
				mod2b_cap = itoa(RuleI(Character, ItemCombatEffectsCap));
				mod2a_space_count = 2;
				mod2b_space_count = 0;
				break;
			}
			case 1: {
				mod2a_name = "Accuracy: ";
				mod2b_name = "Strike Through: ";
				mod2a = itoa(GetAccuracy());
				mod2a_cap = itoa(RuleI(Character, ItemAccuracyCap));
				mod2b = itoa(GetStrikeThrough());
				mod2b_cap = itoa(RuleI(Character, ItemStrikethroughCap));
				mod2a_space_count = 3;
				mod2b_space_count = 1;
				break;
			}
			case 2: {
				mod2a_name = "Shielding: ";
				mod2b_name = "Spell Shielding: ";
				mod2a = itoa(GetShielding());
				mod2a_cap = itoa(RuleI(Character, ItemShieldingCap));
				mod2b = itoa(GetSpellShield());
				mod2b_cap = itoa(RuleI(Character, ItemSpellShieldingCap));
				mod2a_space_count = 2;
				mod2b_space_count = 1;
				break;
			}
			case 3: {
				mod2a_name = "Stun Resist: ";
				mod2b_name = "DoT Shielding: ";
				mod2a = itoa(GetStunResist());
				mod2a_cap = itoa(RuleI(Character, ItemStunResistCap));
				mod2b = itoa(GetDoTShield());
				mod2b_cap = itoa(RuleI(Character, ItemDoTShieldingCap));
				mod2a_space_count = 0;
				mod2b_space_count = 2;
				break;
			}
		}

		mod2a_spacing.clear();
		mod_row_spacing.clear();
		mod2b_spacing.clear();

		for(int a = mod2a.size(); a < (max_mod2_value_len + mod2a_space_count); a++) { mod2a_spacing += " . "; }
		for(int a = mod2a_cap.size(); a < 6 ; a++) { mod_row_spacing += " . "; }
		for(int a = mod2b.size(); a < (max_mod2_value_len + mod2b_space_count); a++) { mod2b_spacing += " . "; }

		mod2_field += indP + mod2a_name + mod2a_spacing + mod2a + " / " + mod2a_cap + mod_row_spacing;
		mod2_field += mod2b_name + mod2b_spacing + mod2b + " / " + mod2b_cap + "<br>";
	}

	uint32 rune_number = 0;
	uint32 magic_rune_number = 0;
	uint32 buff_count = GetMaxTotalSlots();
	for (int i=0; i < buff_count; i++) {
		if (buffs[i].spellid != SPELL_UNKNOWN) {
			if (buffs[i].melee_rune > 0) { rune_number += buffs[i].melee_rune; }

			if (buffs[i].magic_rune > 0) { magic_rune_number += buffs[i].magic_rune; }
		}
	}

	int shield_ac = 0;
	GetRawACNoShield(shield_ac);

	std::string skill_list[] = {
		"1H Blunt","1H Slashing","2H Blunt","2H Slashing","Abjuration",
		"Alteration","Apply Poison","Archery","Backstab","Bind Wound",
		"Bash","Block","Brass Instruments","Channeling","Conjuration",
		"Defense","Disarm","Disarm Traps","Divination","Dodge",
		"Double Attack","Dragon Punch","Dual Wield","Eagle Strike","Evocation",
		"Feign Death","Flying Kick","Forage","Hand To Hand","Hide",
		"Kick","Meditate","Mend","Offense","Parry",
		"Pick Lock","1H Piercing","Riposte","Round Kick","Safe Fall",
		"Sense Heading","Singing","Sneak","Specialize Abjuration","Specialize Alteration",
		"Specialize Conjuration","Specialize Divination","Specialize Evocation","Pick Pockets","Stringed Instruments",
		"Swimming","Throwing","Tiger Claw","Tracking","Wind Instruments",
		"Fishing","Make Poison","Tinkering","Research","Alchemy",
		"Baking","Tailoring","Sense Traps","Blacksmithing","Fletching",
		"Brewing","Alcohol_Tolerance","Begging","Jewelry Making","Pottery",
		"Percussion Instruments","Intimidation","Berserking","Taunt","Frenzy",
		"Remove Traps","Triple Attack","2H Piercing"
	};

	std::string skill_mods = "";
	for (int j = 0; j <= EQ::skills::HIGHEST_SKILL; j++) {
		if(itembonuses.skillmod[j] > 0)
			skill_mods += indP + skill_list[j] + " : +" + itoa(itembonuses.skillmod[j]) + "%<br>";
		else if(itembonuses.skillmod[j] < 0)
			skill_mods += indP + skill_list[j] + " : -" + itoa(itembonuses.skillmod[j]) + "%<br>";
	}

	std::string skill_dmgs = "";
	for (int j = 0; j <= EQ::skills::HIGHEST_SKILL; j++) {
		if((itembonuses.SkillDamageAmount[j] + spellbonuses.SkillDamageAmount[j]) > 0)
			skill_dmgs += indP + skill_list[j] + " : +" + itoa(itembonuses.SkillDamageAmount[j] + spellbonuses.SkillDamageAmount[j]) + "<br>";
		else if((itembonuses.SkillDamageAmount[j] + spellbonuses.SkillDamageAmount[j]) < 0)
			skill_dmgs += indP + skill_list[j] + " : -" + itoa(itembonuses.SkillDamageAmount[j] + spellbonuses.SkillDamageAmount[j]) + "<br>";
	}

	std::string faction_item_string = "";
	char faction_buf[256];

	for (auto iter = item_faction_bonuses.begin(); iter != item_faction_bonuses.end(); ++iter) {
		memset(&faction_buf, 0, sizeof(faction_buf));

		if(!content_db.GetFactionName((int32)((*iter).first), faction_buf, sizeof(faction_buf)))
			strcpy(faction_buf, "Not in DB");

		if((*iter).second > 0) {
			faction_item_string += indP + faction_buf + " : +" + itoa((*iter).second) + "<br>";
		}
		else if((*iter).second < 0) {
			faction_item_string += indP + faction_buf + " : -" + itoa((*iter).second) + "<br>";
		}
	}

	std::string bard_info = "";
	if(GetClass() == BARD) {
		bard_info = indP + "Singing: " + itoa(GetSingMod()) + "<br>" +
					indP + "Brass: " + itoa(GetBrassMod()) + "<br>" +
					indP + "String: " + itoa(GetStringMod()) + "<br>" +
					indP + "Percussion: " + itoa(GetPercMod()) + "<br>" +
					indP + "Wind: " + itoa(GetWindMod()) + "<br>";
	}

	EQ::skills::SkillType skill = EQ::skills::SkillHandtoHand;
	auto *inst = GetInv().GetItem(EQ::invslot::slotPrimary);
	if (inst && inst->IsClassCommon()) {
		switch (inst->GetItem()->ItemType) {
		case EQ::item::ItemType1HSlash:
			skill = EQ::skills::Skill1HSlashing;
			break;
		case EQ::item::ItemType2HSlash:
			skill = EQ::skills::Skill2HSlashing;
			break;
		case EQ::item::ItemType1HPiercing:
			skill = EQ::skills::Skill1HPiercing;
			break;
		case EQ::item::ItemType1HBlunt:
			skill = EQ::skills::Skill1HBlunt;
			break;
		case EQ::item::ItemType2HBlunt:
			skill = EQ::skills::Skill2HBlunt;
			break;
		case EQ::item::ItemType2HPiercing:
			if (ClientVersion() < EQ::versions::ClientVersion::RoF2)
				skill = EQ::skills::Skill1HPiercing;
			else
				skill = EQ::skills::Skill2HPiercing;
			break;
		default:
			break;
		}
	}

	std::ostringstream final_string;
					final_string <<
	/*	C/L/R	*/	indP << "Class: " << class_Name << indS << "Level: " << static_cast<int>(GetLevel()) << indS << "Race: " << race_name << "<br>" <<
	/*	Runes	*/	indP << "Rune: " << rune_number << indL << indS << "Spell Rune: " << magic_rune_number << "<br>" <<
	/*	HP/M/E	*/	HME_row <<
	/*	DS		*/	indP << "DS: " << (itembonuses.DamageShield + spellbonuses.DamageShield*-1) << " (Spell: " << (spellbonuses.DamageShield*-1) << " + Item: " << itembonuses.DamageShield << " / " << RuleI(Character, ItemDamageShieldCap) << ")<br>" <<
	/*	Atk		*/	indP << "<c \"#CCFF00\">tohit: " << compute_tohit(skill) << " / " << GetTotalToHit(skill, 0) << "</c><br>" <<
	/*	Atk2	*/	indP << "- Offense: " << offense(skill) << " | Item: " << itembonuses.ATK << " (" << RuleI(Character, ItemATKCap) << ")~Used: " << (itembonuses.ATK * 1.342) << " | Spell: " << spellbonuses.ATK << "<br>" <<
	/*	AC		*/	indP << "<c \"#CCFF00\">mitigation AC: " << GetMitigationAC() << "</c><br>" <<
	/*	AC2		*/	indP << "- defense: " << compute_defense() << " / " << GetTotalDefense() << " | Spell: " << spellbonuses.AC << " | Shield: " << shield_ac << "<br>" <<
	/*	Haste	*/	indP << "<c \"#CCFF00\">Haste: " << GetHaste() << "</c><br>" <<
	/*	Haste2	*/	indP << " - Item: " << itembonuses.haste << " + Spell: " << (spellbonuses.haste + spellbonuses.hastetype2) << " (Cap: " << RuleI(Character, HasteCap) << ") | Over: " << (spellbonuses.hastetype3 + ExtraHaste) << "<br>" <<
	/*	RunSpeed*/	indP << "<c \"#CCFF00\">Runspeed: " << GetRunspeed() << "</c><br>" <<
	/* RegenLbl	*/	indL << indS << "Regen<br>" << indS << indP << indP << " Base | Items (Cap) " << indP << " | Spell | A.A.s | Total<br>" <<
	/*	Regen	*/	regen_string << "<br>" <<
	/*	Stats	*/	stat_field << "<br><br>" <<
	/*	Mod2s	*/	mod2_field << "<br>" <<
	/*	HealAmt	*/	indP << "Heal Amount: " << GetHealAmt() << " / " << RuleI(Character, ItemHealAmtCap) << "<br>" <<
	/*	SpellDmg*/	indP << "Spell Dmg: " << GetSpellDmg() << " / " << RuleI(Character, ItemSpellDmgCap) << "<br>" <<
	/*	Clair	*/	indP << "Clairvoyance: " << GetClair() << " / " << RuleI(Character, ItemClairvoyanceCap) << "<br>" <<
	/*	DSMit	*/	indP << "Dmg Shld Mit: " << GetDSMit() << " / " << RuleI(Character, ItemDSMitigationCap) << "<br><br>";
	if(GetClass() == BARD)
		final_string << bard_info << "<br>";
	if(skill_mods.size() > 0)
		final_string << skill_mods << "<br>";
	if(skill_dmgs.size() > 0)
		final_string << skill_dmgs << "<br>";
	if(faction_item_string.size() > 0)
		final_string << faction_item_string;

	std::string final_stats = final_string.str();

	if(use_window) {
		if(final_stats.size() < 4096)
		{
			uint32 Buttons = (client->ClientVersion() < EQ::versions::ClientVersion::SoD) ? 0 : 1;
			client->SendWindow(0, POPUPID_UPDATE_SHOWSTATSWINDOW, Buttons, "Cancel", "Update", 0, 1, this, "", "%s", final_stats.c_str());
			goto Extra_Info;
		}
		else {
			client->Message(Chat::Yellow, "The window has exceeded its character limit, displaying stats to chat window:");
		}
	}

	client->Message(Chat::Yellow, "~~~~~ %s %s ~~~~~", GetCleanName(), GetLastName());
	client->Message(Chat::White, " Level: %i Class: %i Race: %i DS: %i/%i Size: %1.1f  Weight: %.1f/%d  ", GetLevel(), GetClass(), GetRace(), GetDS(), RuleI(Character, ItemDamageShieldCap), GetSize(), (float)CalcCurrentWeight() / 10.0f, GetSTR());
	client->Message(Chat::White, " HP: %i/%i  HP Regen: %i/%i",GetHP(), GetMaxHP(), CalcHPRegen(), CalcHPRegenCap());
	client->Message(Chat::White, " compute_tohit: %i TotalToHit: %i", compute_tohit(skill), GetTotalToHit(skill, 0));
	client->Message(Chat::White, " compute_defense: %i TotalDefense: %i", compute_defense(), GetTotalDefense());
	client->Message(Chat::White, " offense: %i mitigation ac: %i", offense(skill), GetMitigationAC());
	if(CalcMaxMana() > 0)
		client->Message(Chat::White, " Mana: %i/%i  Mana Regen: %i/%i", GetMana(), GetMaxMana(), CalcManaRegen(), CalcManaRegenCap());
	client->Message(Chat::White, " End.: %i/%i  End. Regen: %i/%i",GetEndurance(), GetMaxEndurance(), CalcEnduranceRegen(), CalcEnduranceRegenCap());
	client->Message(Chat::White, " ATK: %i  Worn/Spell ATK %i/%i  Server Side ATK: %i", GetTotalATK(), RuleI(Character, ItemATKCap), GetATKBonus(), GetATK());
	client->Message(Chat::White, " Haste: %i / %i (Item: %i + Spell: %i + Over: %i)", GetHaste(), RuleI(Character, HasteCap), itembonuses.haste, spellbonuses.haste + spellbonuses.hastetype2, spellbonuses.hastetype3 + ExtraHaste);
	client->Message(Chat::White, " STR: %i  STA: %i  DEX: %i  AGI: %i  INT: %i  WIS: %i  CHA: %i", GetSTR(), GetSTA(), GetDEX(), GetAGI(), GetINT(), GetWIS(), GetCHA());
	client->Message(Chat::White, " hSTR: %i  hSTA: %i  hDEX: %i  hAGI: %i  hINT: %i  hWIS: %i  hCHA: %i", GetHeroicSTR(), GetHeroicSTA(), GetHeroicDEX(), GetHeroicAGI(), GetHeroicINT(), GetHeroicWIS(), GetHeroicCHA());
	client->Message(Chat::White, " MR: %i  PR: %i  FR: %i  CR: %i  DR: %i Corruption: %i PhR: %i", GetMR(), GetPR(), GetFR(), GetCR(), GetDR(), GetCorrup(), GetPhR());
	client->Message(Chat::White, " hMR: %i  hPR: %i  hFR: %i  hCR: %i  hDR: %i hCorruption: %i", GetHeroicMR(), GetHeroicPR(), GetHeroicFR(), GetHeroicCR(), GetHeroicDR(), GetHeroicCorrup());
	client->Message(Chat::White, " Shielding: %i  Spell Shield: %i  DoT Shielding: %i Stun Resist: %i  Strikethrough: %i  Avoidance: %i  Accuracy: %i  Combat Effects: %i", GetShielding(), GetSpellShield(), GetDoTShield(), GetStunResist(), GetStrikeThrough(), GetAvoidance(), GetAccuracy(), GetCombatEffects());
	client->Message(Chat::White, " Heal Amt.: %i  Spell Dmg.: %i  Clairvoyance: %i DS Mitigation: %i", GetHealAmt(), GetSpellDmg(), GetClair(), GetDSMit());
	if(GetClass() == BARD)
		client->Message(Chat::White, " Singing: %i  Brass: %i  String: %i Percussion: %i Wind: %i", GetSingMod(), GetBrassMod(), GetStringMod(), GetPercMod(), GetWindMod());

	Extra_Info:

	client->Message(Chat::White, " BaseRace: %i  Gender: %i  BaseGender: %i Texture: %i  HelmTexture: %i", GetBaseRace(), GetGender(), GetBaseGender(), GetTexture(), GetHelmTexture());
	if (client->Admin() >= 100) {
		client->Message(Chat::White, "  CharID: %i  EntityID: %i  PetID: %i  OwnerID: %i  AIControlled: %i  Targetted: %i", CharacterID(), GetID(), GetPetID(), GetOwnerID(), IsAIControlled(), targeted);
	}
}

void Client::SendAltCurrencies() {
	if (ClientVersion() >= EQ::versions::ClientVersion::SoF) {
		uint32 count = zone->AlternateCurrencies.size();
		if(count == 0) {
			return;
		}

		auto outapp =
		    new EQApplicationPacket(OP_AltCurrency, sizeof(AltCurrencyPopulate_Struct) +
								sizeof(AltCurrencyPopulateEntry_Struct) * count);
		AltCurrencyPopulate_Struct *altc = (AltCurrencyPopulate_Struct*)outapp->pBuffer;
		altc->opcode = ALT_CURRENCY_OP_POPULATE;
		altc->count = count;

		uint32 i = 0;
		auto iter = zone->AlternateCurrencies.begin();
		while(iter != zone->AlternateCurrencies.end()) {
			const EQ::ItemData* item = database.GetItem((*iter).item_id);
			altc->entries[i].currency_number = (*iter).id;
			altc->entries[i].unknown00 = 1;
			altc->entries[i].currency_number2 = (*iter).id;
			altc->entries[i].item_id = (*iter).item_id;
			if(item) {
				altc->entries[i].item_icon = item->Icon;
				altc->entries[i].stack_size = item->StackSize;
			} else {
				altc->entries[i].item_icon = 1000;
				altc->entries[i].stack_size = 1000;
			}
			i++;
			++iter;
		}

		FastQueuePacket(&outapp);
	}
}

void Client::SetAlternateCurrencyValue(uint32 currency_id, uint32 new_amount)
{
	alternate_currency[currency_id] = new_amount;
	database.UpdateAltCurrencyValue(CharacterID(), currency_id, new_amount);
	SendAlternateCurrencyValue(currency_id);
}

void Client::AddAlternateCurrencyValue(uint32 currency_id, int32 amount, int8 method)
{

	/* Added via Quest, rest of the logging methods may be done inline due to information available in that area of the code */
	if (method == 1){
		/* QS: PlayerLogAlternateCurrencyTransactions :: Cursor to Item Storage */
		if (RuleB(QueryServ, PlayerLogAlternateCurrencyTransactions)){
			std::string event_desc = StringFormat("Added via Quest :: Cursor to Item :: alt_currency_id:%i amount:%i in zoneid:%i instid:%i", currency_id, this->GetZoneID(), this->GetInstanceID());
			QServ->PlayerLogEvent(Player_Log_Alternate_Currency_Transactions, this->CharacterID(), event_desc);
		}
	}

	if(amount == 0) {
		return;
	}

	if(!alternate_currency_loaded) {
		alternate_currency_queued_operations.push(std::make_pair(currency_id, amount));
		return;
	}

	int new_value = 0;
	auto iter = alternate_currency.find(currency_id);
	if(iter == alternate_currency.end()) {
		new_value = amount;
	} else {
		new_value = (*iter).second + amount;
	}

	if(new_value < 0) {
		alternate_currency[currency_id] = 0;
		database.UpdateAltCurrencyValue(CharacterID(), currency_id, 0);
	} else {
		alternate_currency[currency_id] = new_value;
		database.UpdateAltCurrencyValue(CharacterID(), currency_id, new_value);
	}
	SendAlternateCurrencyValue(currency_id);
}

void Client::SendAlternateCurrencyValues()
{
	auto iter = zone->AlternateCurrencies.begin();
	while(iter != zone->AlternateCurrencies.end()) {
		SendAlternateCurrencyValue((*iter).id, false);
		++iter;
	}
}

void Client::SendAlternateCurrencyValue(uint32 currency_id, bool send_if_null)
{
	uint32 value = GetAlternateCurrencyValue(currency_id);
	if(value > 0 || (value == 0 && send_if_null)) {
		auto outapp = new EQApplicationPacket(OP_AltCurrency, sizeof(AltCurrencyUpdate_Struct));
		AltCurrencyUpdate_Struct *update = (AltCurrencyUpdate_Struct*)outapp->pBuffer;
		update->opcode = 7;
		strcpy(update->name, GetName());
		update->currency_number = currency_id;
		update->amount = value;
		update->unknown072 = 1;
		FastQueuePacket(&outapp);
	}
}

uint32 Client::GetAlternateCurrencyValue(uint32 currency_id) const
{
	auto iter = alternate_currency.find(currency_id);
	if(iter == alternate_currency.end()) {
		return 0;
	} else {
		return (*iter).second;
	}
}

void Client::ProcessAlternateCurrencyQueue() {
	while(!alternate_currency_queued_operations.empty()) {
		std::pair<uint32, int32> op = alternate_currency_queued_operations.front();

		AddAlternateCurrencyValue(op.first, op.second);

		alternate_currency_queued_operations.pop();
	}
}

void Client::OpenLFGuildWindow()
{
	auto outapp = new EQApplicationPacket(OP_LFGuild, 8);

	outapp->WriteUInt32(6);

	FastQueuePacket(&outapp);
}

bool Client::IsXTarget(const Mob *m) const
{
	if(!XTargettingAvailable() || !m || (m->GetID() == 0))
		return false;

	for(int i = 0; i < GetMaxXTargets(); ++i)
	{
		if(XTargets[i].ID == m->GetID())
			return true;
	}
	return false;
}

bool Client::IsClientXTarget(const Client *c) const
{
	if(!XTargettingAvailable() || !c)
		return false;

	for(int i = 0; i < GetMaxXTargets(); ++i)
	{
		if(!strcasecmp(XTargets[i].Name, c->GetName()))
			return true;
	}
	return false;
}


void Client::UpdateClientXTarget(Client *c)
{
	if(!XTargettingAvailable() || !c)
		return;

	for(int i = 0; i < GetMaxXTargets(); ++i)
	{
		if(!strcasecmp(XTargets[i].Name, c->GetName()))
		{
			XTargets[i].ID = c->GetID();
			SendXTargetPacket(i, c);
		}
	}
}

// IT IS NOT SAFE TO CALL THIS IF IT'S NOT INITIAL AGGRO
void Client::AddAutoXTarget(Mob *m, bool send)
{
	m_activeautohatermgr->increment_count(m);

	if (!XTargettingAvailable() || !XTargetAutoAddHaters || IsXTarget(m))
		return;

	for(int i = 0; i < GetMaxXTargets(); ++i)
	{
		if((XTargets[i].Type == Auto) && (XTargets[i].ID == 0))
		{
			XTargets[i].ID = m->GetID();
			if (send) // if we don't send we're bulk sending updates later on
				SendXTargetPacket(i, m);
			else
				XTargets[i].dirty = true;
			break;
		}
	}
}

void Client::RemoveXTarget(Mob *m, bool OnlyAutoSlots)
{
	m_activeautohatermgr->decrement_count(m);
	// now we may need to clean up our CurrentTargetNPC entries
	for (int i = 0; i < GetMaxXTargets(); ++i) {
		if (XTargets[i].Type == CurrentTargetNPC && XTargets[i].ID == m->GetID()) {
			XTargets[i].Type = Auto;
			XTargets[i].ID = 0;
			XTargets[i].dirty = true;
		}
	}
}

void Client::UpdateXTargetType(XTargetType Type, Mob *m, const char *Name)
{
	if(!XTargettingAvailable())
		return;

	for(int i = 0; i < GetMaxXTargets(); ++i)
	{
		if(XTargets[i].Type == Type)
		{
			if(m)
				XTargets[i].ID = m->GetID();
			else
				XTargets[i].ID = 0;

			if(Name)
				strncpy(XTargets[i].Name, Name, 64);

			SendXTargetPacket(i, m);
		}
	}
}

void Client::SendXTargetPacket(uint32 Slot, Mob *m)
{
	if(!XTargettingAvailable())
		return;

	uint32 PacketSize = 18;

	if(m)
		PacketSize += strlen(m->GetCleanName());
	else
	{
		PacketSize += strlen(XTargets[Slot].Name);
	}

	auto outapp = new EQApplicationPacket(OP_XTargetResponse, PacketSize);
	outapp->WriteUInt32(GetMaxXTargets());
	outapp->WriteUInt32(1);
	outapp->WriteUInt32(Slot);
	if(m)
	{
		outapp->WriteUInt8(1);
	}
	else
	{
		if (strlen(XTargets[Slot].Name) && ((XTargets[Slot].Type == CurrentTargetPC) ||
			(XTargets[Slot].Type == GroupTank) ||
			(XTargets[Slot].Type == GroupAssist) ||
			(XTargets[Slot].Type == Puller) ||
			(XTargets[Slot].Type == RaidAssist1) ||
			(XTargets[Slot].Type == RaidAssist2) ||
			(XTargets[Slot].Type == RaidAssist3)))
		{
			outapp->WriteUInt8(2);
		}
		else
		{
			outapp->WriteUInt8(0);
		}
	}
	outapp->WriteUInt32(XTargets[Slot].ID);
	outapp->WriteString(m ? m->GetCleanName() : XTargets[Slot].Name);
	FastQueuePacket(&outapp);
}

// This is a bulk packet, we use it when we remove something since we need to reorder the xtargets and maybe
// add new mobs! Currently doesn't check if there is a dirty flag set, so it should only be called when there is
void Client::SendXTargetUpdates()
{
	if (!XTargettingAvailable())
		return;

	int count = 0;
	// header is 4 bytes max xtargets, 4 bytes count
	// entry is 4 bytes slot, 1 byte unknown, 4 bytes ID, 65 char name
	auto outapp = new EQApplicationPacket(OP_XTargetResponse, 8 + 74 * GetMaxXTargets()); // fuck it max size
	outapp->WriteUInt32(GetMaxXTargets());
	outapp->WriteUInt32(1); // we will correct this later
	for (int i = 0; i < GetMaxXTargets(); ++i) {
		if (XTargets[i].dirty) {
			outapp->WriteUInt32(i);
			// MQ2 checks this for valid mobs, so 0 is bad here at least ...
			outapp->WriteUInt8(XTargets[i].ID ? 1 : 0);
			outapp->WriteUInt32(XTargets[i].ID);
			outapp->WriteString(XTargets[i].Name);
			count++;
			XTargets[i].dirty = false;
		}
	}

	// RemoveXTarget probably got called with a mob not on our xtargets
	if (count == 0) {
		safe_delete(outapp);
		return;
	}

	auto newbuff = new uchar[outapp->GetWritePosition()];
	memcpy(newbuff, outapp->pBuffer, outapp->GetWritePosition());
	safe_delete_array(outapp->pBuffer);
	outapp->pBuffer = newbuff;
	outapp->size = outapp->GetWritePosition();
	outapp->SetWritePosition(4);
	outapp->WriteUInt32(count);
	FastQueuePacket(&outapp);
}

void Client::RemoveGroupXTargets()
{
	if(!XTargettingAvailable())
		return;

	for(int i = 0; i < GetMaxXTargets(); ++i)
	{
		if ((XTargets[i].Type == GroupTank) ||
			(XTargets[i].Type == GroupAssist) ||
			(XTargets[i].Type == Puller) ||
			(XTargets[i].Type == RaidAssist1) ||
			(XTargets[i].Type == RaidAssist2) ||
			(XTargets[i].Type == RaidAssist3) ||
			(XTargets[i].Type == GroupMarkTarget1) ||
			(XTargets[i].Type == GroupMarkTarget2) ||
			(XTargets[i].Type == GroupMarkTarget3))
		{
			XTargets[i].ID = 0;
			XTargets[i].Name[0] = 0;
			SendXTargetPacket(i, nullptr);
		}
	}
}

void Client::RemoveAutoXTargets()
{
	if(!XTargettingAvailable())
		return;

	for(int i = 0; i < GetMaxXTargets(); ++i)
	{
		if(XTargets[i].Type == Auto)
		{
			XTargets[i].ID = 0;
			XTargets[i].Name[0] = 0;
			SendXTargetPacket(i, nullptr);
		}
	}
}

void Client::ShowXTargets(Client *c)
{
	if(!c)
		return;

	for(int i = 0; i < GetMaxXTargets(); ++i)
		c->Message(Chat::White, "Xtarget Slot: %i, Type: %2i, ID: %4i, Name: %s", i, XTargets[i].Type, XTargets[i].ID, XTargets[i].Name);
	auto &list = GetXTargetAutoMgr()->get_list();
	 // yeah, I kept having to do something for debugging to tell if managers were the same object or not :P
	 // so lets use the address as an "ID"
	c->Message(Chat::White, "XTargetAutoMgr ID %p size %d", GetXTargetAutoMgr(), list.size());
	int count = 0;
	for (auto &e : list) {
		c->Message(Chat::White, "spawn id %d count %d", e.spawn_id, e.count);
		count++;
		if (count == 20) { // lets not spam too many ...
			c->Message(Chat::White, " ... ");
			break;
		}
	}
}

void Client::ProcessXTargetAutoHaters()
{
	if (!XTargettingAvailable())
		return;

	// move shit up! If the removed NPC was in a CurrentTargetNPC slot it becomes Auto
	// and we need to potentially fill it
	std::queue<int> empty_slots;
	for (int i = 0; i < GetMaxXTargets(); ++i) {
		if (XTargets[i].Type != Auto)
			continue;

		if (XTargets[i].ID != 0 && !GetXTargetAutoMgr()->contains_mob(XTargets[i].ID)) {
			XTargets[i].ID = 0;
			XTargets[i].dirty = true;
		}

		if (XTargets[i].ID == 0) {
			empty_slots.push(i);
			continue;
		}

		if (XTargets[i].ID != 0 && !empty_slots.empty()) {
			int temp = empty_slots.front();
			std::swap(XTargets[i], XTargets[temp]);
			XTargets[i].dirty = XTargets[temp].dirty = true;
			empty_slots.pop();
			empty_slots.push(i);
		}
	}
	// okay, now we need to check if we have any empty slots and if we have aggro
	// We make the assumption that if we shuffled the NPCs up that they're still on the aggro
	// list in the same order. We could probably do this better and try to calc if
	// there are new NPCs for our empty slots on the manager, but ahhh fuck it.
	if (!empty_slots.empty() && !GetXTargetAutoMgr()->empty() && XTargetAutoAddHaters) {
		auto &haters = GetXTargetAutoMgr()->get_list();
		for (auto &e : haters) {
			auto *mob = entity_list.GetMob(e.spawn_id);
			if (mob && !IsXTarget(mob)) {
				auto slot = empty_slots.front();
				empty_slots.pop();
				XTargets[slot].dirty = true;
				XTargets[slot].ID = mob->GetID();
				strn0cpy(XTargets[slot].Name, mob->GetCleanName(), 64);
			}
			if (empty_slots.empty())
				break;
		}
	}
	m_dirtyautohaters = false;
	SendXTargetUpdates();
}

// This function is called when a client is added to a group
// Group leader joining isn't handled by this function
void Client::JoinGroupXTargets(Group *g)
{
	if (!g)
		return;

	if (!GetXTargetAutoMgr()->empty()) {
		g->GetXTargetAutoMgr()->merge(*GetXTargetAutoMgr());
		GetXTargetAutoMgr()->clear();
		RemoveAutoXTargets();
	}

	SetXTargetAutoMgr(g->GetXTargetAutoMgr());

	if (!GetXTargetAutoMgr()->empty())
		SetDirtyAutoHaters();
}

// This function is called when a client leaves a group
void Client::LeaveGroupXTargets(Group *g)
{
	if (!g)
		return;

	SetXTargetAutoMgr(nullptr); // this will set it back to our manager
	RemoveAutoXTargets();
	entity_list.RefreshAutoXTargets(this); // this will probably break the temporal ordering, but whatever
	// We now have a rebuilt, valid auto hater manager, so we need to demerge from the groups
	if (!GetXTargetAutoMgr()->empty()) {
		GetXTargetAutoMgr()->demerge(*g->GetXTargetAutoMgr()); // this will remove entries where we only had aggro
		SetDirtyAutoHaters();
	}
}

// This function is called when a client leaves a group
void Client::LeaveRaidXTargets(Raid *r)
{
	if (!r)
		return;

	SetXTargetAutoMgr(nullptr); // this will set it back to our manager
	RemoveAutoXTargets();
	entity_list.RefreshAutoXTargets(this); // this will probably break the temporal ordering, but whatever
	// We now have a rebuilt, valid auto hater manager, so we need to demerge from the groups
	if (!GetXTargetAutoMgr()->empty()) {
		GetXTargetAutoMgr()->demerge(*r->GetXTargetAutoMgr()); // this will remove entries where we only had aggro
		SetDirtyAutoHaters();
	}
}

void Client::SetMaxXTargets(uint8 NewMax)
{
	if(!XTargettingAvailable())
		return;

	if(NewMax > XTARGET_HARDCAP)
		return;

	MaxXTargets = NewMax;

	Save(0);

	for(int i = MaxXTargets; i < XTARGET_HARDCAP; ++i)
	{
		XTargets[i].Type = Auto;
		XTargets[i].ID = 0;
		XTargets[i].Name[0] = 0;
	}

	auto outapp = new EQApplicationPacket(OP_XTargetResponse, 8);
	outapp->WriteUInt32(GetMaxXTargets());
	outapp->WriteUInt32(0);
	FastQueuePacket(&outapp);
}

const char* Client::GetRacePlural(Client* client) {

	switch (client->CastToMob()->GetRace()) {
		case HUMAN:
			return "Humans"; break;
		case BARBARIAN:
			return "Barbarians"; break;
		case ERUDITE:
			return "Erudites"; break;
		case WOOD_ELF:
			return "Wood Elves"; break;
		case HIGH_ELF:
			return "High Elves"; break;
		case DARK_ELF:
			return "Dark Elves"; break;
		case HALF_ELF:
			return "Half Elves"; break;
		case DWARF:
			return "Dwarves"; break;
		case TROLL:
			return "Trolls"; break;
		case OGRE:
			return "Ogres"; break;
		case HALFLING:
			return "Halflings"; break;
		case GNOME:
			return "Gnomes"; break;
		case IKSAR:
			return "Iksar"; break;
		case VAHSHIR:
			return "Vah Shir"; break;
		case FROGLOK:
			return "Frogloks"; break;
		case DRAKKIN:
			return "Drakkin"; break;
		default:
			return "Races"; break;
	}
}

const char* Client::GetClassPlural(Client* client) {

	switch (client->CastToMob()->GetClass()) {
		case WARRIOR:
			return "Warriors"; break;
		case CLERIC:
			return "Clerics"; break;
		case PALADIN:
			return "Paladins"; break;
		case RANGER:
			return "Rangers"; break;
		case SHADOWKNIGHT:
			return "Shadowknights"; break;
		case DRUID:
			return "Druids"; break;
		case MONK:
			return "Monks"; break;
		case BARD:
			return "Bards"; break;
		case ROGUE:
			return "Rogues"; break;
		case SHAMAN:
			return "Shamen"; break;
		case NECROMANCER:
			return "Necromancers"; break;
		case WIZARD:
			return "Wizards"; break;
		case MAGICIAN:
			return "Magicians"; break;
		case ENCHANTER:
			return "Enchanters"; break;
		case BEASTLORD:
			return "Beastlords"; break;
		case BERSERKER:
			return "Berserkers"; break;
		default:
			return "Classes"; break;
	}
}


void Client::SendWebLink(const char *website)
{
	size_t len = strlen(website) + 1;
	if(website != 0 && len > 1)
	{
		auto outapp = new EQApplicationPacket(OP_Weblink, sizeof(Weblink_Struct) + len);
		Weblink_Struct *wl = (Weblink_Struct*)outapp->pBuffer;
		memcpy(wl->weblink, website, len);
		wl->weblink[len] = '\0';

		FastQueuePacket(&outapp);
	}
}

void Client::SendMercPersonalInfo()
{
	uint32 mercTypeCount = 1;
	uint32 mercCount = 1; //TODO: Un-hardcode this and support multiple mercs like in later clients than SoD.
	uint32 i = 0;
	uint32 altCurrentType = 19; //TODO: Implement alternate currency purchases involving mercs!

	MercTemplate *mercData = &zone->merc_templates[GetMercInfo().MercTemplateID];

	int stancecount = 0;
	stancecount += zone->merc_stance_list[GetMercInfo().MercTemplateID].size();
	if(stancecount > MAX_MERC_STANCES || mercCount > MAX_MERC || mercTypeCount > MAX_MERC_GRADES)
	{
		Log(Logs::General, Logs::Mercenaries, "SendMercPersonalInfo canceled: (%i) (%i) (%i) for %s", stancecount, mercCount, mercTypeCount, GetName());
		SendMercMerchantResponsePacket(0);
		return;
	}

	if(mercData)
	{
		if (ClientVersion() >= EQ::versions::ClientVersion::RoF)
		{
			if (mercCount > 0)
			{
				auto outapp =
				    new EQApplicationPacket(OP_MercenaryDataUpdate, sizeof(MercenaryDataUpdate_Struct));
				MercenaryDataUpdate_Struct* mdus = (MercenaryDataUpdate_Struct*)outapp->pBuffer;
				mdus->MercStatus = 0;
				mdus->MercCount = mercCount;
				mdus->MercData[i].MercID = mercData->MercTemplateID;
				mdus->MercData[i].MercType = mercData->MercType;
				mdus->MercData[i].MercSubType = mercData->MercSubType;
				mdus->MercData[i].PurchaseCost = Merc::CalcPurchaseCost(mercData->MercTemplateID, GetLevel(), 0);
				mdus->MercData[i].UpkeepCost = Merc::CalcUpkeepCost(mercData->MercTemplateID, GetLevel(), 0);
				mdus->MercData[i].Status = 0;
				mdus->MercData[i].AltCurrencyCost = Merc::CalcPurchaseCost(mercData->MercTemplateID, GetLevel(), altCurrentType);
				mdus->MercData[i].AltCurrencyUpkeep = Merc::CalcPurchaseCost(mercData->MercTemplateID, GetLevel(), altCurrentType);
				mdus->MercData[i].AltCurrencyType = altCurrentType;
				mdus->MercData[i].MercUnk01 = 0;
				mdus->MercData[i].TimeLeft = GetMercInfo().MercTimerRemaining;	//GetMercTimer().GetRemainingTime();
				mdus->MercData[i].MerchantSlot = i + 1;
				mdus->MercData[i].MercUnk02 = 1;
				mdus->MercData[i].StanceCount = zone->merc_stance_list[mercData->MercTemplateID].size();
				mdus->MercData[i].MercUnk03 = 0;
				mdus->MercData[i].MercUnk04 = 1;
				strn0cpy(mdus->MercData[i].MercName, GetMercInfo().merc_name , sizeof(mdus->MercData[i].MercName));
				uint32 stanceindex = 0;
				if (mdus->MercData[i].StanceCount != 0)
				{
					auto iter = zone->merc_stance_list[mercData->MercTemplateID].begin();
					while(iter != zone->merc_stance_list[mercData->MercTemplateID].end())
					{
						mdus->MercData[i].Stances[stanceindex].StanceIndex = stanceindex;
						mdus->MercData[i].Stances[stanceindex].Stance = (iter->StanceID);
						stanceindex++;
						++iter;
					}
				}

				mdus->MercData[i].MercUnk05 = 1;
				FastQueuePacket(&outapp);
				safe_delete(outapp);
				return;
			}
		}
		else
		{
			if(mercTypeCount > 0 && mercCount > 0)
			{
				auto outapp = new EQApplicationPacket(OP_MercenaryDataResponse,
								      sizeof(MercenaryMerchantList_Struct));
				MercenaryMerchantList_Struct* mml = (MercenaryMerchantList_Struct*)outapp->pBuffer;
				mml->MercTypeCount = mercTypeCount; //We should only have one merc entry.
				mml->MercGrades[i] = 1;
				mml->MercCount = mercCount;
				mml->Mercs[i].MercID = mercData->MercTemplateID;
				mml->Mercs[i].MercType = mercData->MercType;
				mml->Mercs[i].MercSubType = mercData->MercSubType;
				mml->Mercs[i].PurchaseCost = RuleB(Mercs, ChargeMercPurchaseCost) ? Merc::CalcPurchaseCost(mercData->MercTemplateID, GetLevel(), 0): 0;
				mml->Mercs[i].UpkeepCost = RuleB(Mercs, ChargeMercUpkeepCost) ? Merc::CalcUpkeepCost(mercData->MercTemplateID, GetLevel(), 0): 0;
				mml->Mercs[i].Status = 0;
				mml->Mercs[i].AltCurrencyCost = RuleB(Mercs, ChargeMercPurchaseCost) ? Merc::CalcPurchaseCost(mercData->MercTemplateID, GetLevel(), altCurrentType): 0;
				mml->Mercs[i].AltCurrencyUpkeep = RuleB(Mercs, ChargeMercUpkeepCost) ? Merc::CalcUpkeepCost(mercData->MercTemplateID, GetLevel(), altCurrentType): 0;
				mml->Mercs[i].AltCurrencyType = altCurrentType;
				mml->Mercs[i].MercUnk01 = 0;
				mml->Mercs[i].TimeLeft = GetMercInfo().MercTimerRemaining;
				mml->Mercs[i].MerchantSlot = i + 1;
				mml->Mercs[i].MercUnk02 = 1;
				mml->Mercs[i].StanceCount = zone->merc_stance_list[mercData->MercTemplateID].size();
				mml->Mercs[i].MercUnk03 = 0;
				mml->Mercs[i].MercUnk04 = 1;
				strn0cpy(mml->Mercs[i].MercName, GetMercInfo().merc_name , sizeof(mml->Mercs[i].MercName));
				int stanceindex = 0;
				if(mml->Mercs[i].StanceCount != 0)
				{
					auto iter = zone->merc_stance_list[mercData->MercTemplateID].begin();
					while(iter != zone->merc_stance_list[mercData->MercTemplateID].end())
					{
						mml->Mercs[i].Stances[stanceindex].StanceIndex = stanceindex;
						mml->Mercs[i].Stances[stanceindex].Stance = (iter->StanceID);
						stanceindex++;
						++iter;
					}
				}
				FastQueuePacket(&outapp);
				safe_delete(outapp);
				return;
			}
		}
		Log(Logs::General, Logs::Mercenaries, "SendMercPersonalInfo Send Successful for %s.", GetName());

		SendMercMerchantResponsePacket(0);
	}
	else
	{
		Log(Logs::General, Logs::Mercenaries, "SendMercPersonalInfo Send Failed Due to no MercData (%i) for %s", GetMercInfo().MercTemplateID, GetName());
	}

}

void Client::SendClearMercInfo()
{
	auto outapp = new EQApplicationPacket(OP_MercenaryDataUpdate, sizeof(NoMercenaryHired_Struct));
	NoMercenaryHired_Struct *nmhs = (NoMercenaryHired_Struct*)outapp->pBuffer;
	nmhs->MercStatus = -1;
	nmhs->MercCount = 0;
	nmhs->MercID = 1;
	FastQueuePacket(&outapp);
}


void Client::DuplicateLoreMessage(uint32 ItemID)
{
	if (!(m_ClientVersionBit & EQ::versions::maskRoFAndLater))
	{
		MessageString(Chat::White, PICK_LORE);
		return;
	}

	const EQ::ItemData *item = database.GetItem(ItemID);

	if(!item)
		return;

	MessageString(Chat::White, PICK_LORE, item->Name);
}

void Client::GarbleMessage(char *message, uint8 variance)
{
	// Garble message by variance%
	const char alpha_list[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"; // only change alpha characters for now
	const char delimiter = 0x12;
	int delimiter_count = 0;

	// Don't garble # commands
	if (message[0] == COMMAND_CHAR)
		return;

#ifdef BOTS
	if (message[0] == BOT_COMMAND_CHAR)
		return;
#endif

	for (size_t i = 0; i < strlen(message); i++) {
		// Client expects hex values inside of a text link body
		if (message[i] == delimiter) {
			if (!(delimiter_count & 1)) { i += EQ::constants::SAY_LINK_BODY_SIZE; }
			++delimiter_count;
			continue;
		}

		uint8 chance = (uint8)zone->random.Int(0, 115); // variation just over worst possible scrambling
		if (isalpha((unsigned char)message[i]) && (chance <= variance)) {
			uint8 rand_char = (uint8)zone->random.Int(0,51); // choose a random character from the alpha list
			message[i] = alpha_list[rand_char];
		}
	}
}

// returns what Other thinks of this
FACTION_VALUE Client::GetReverseFactionCon(Mob* iOther) {
	if (GetOwnerID()) {
		return GetOwnerOrSelf()->GetReverseFactionCon(iOther);
	}

	iOther = iOther->GetOwnerOrSelf();

	if (iOther->GetPrimaryFaction() < 0)
		return GetSpecialFactionCon(iOther);

	if (iOther->GetPrimaryFaction() == 0)
		return FACTION_INDIFFERENT;

	return GetFactionLevel(CharacterID(), 0, GetFactionRace(), GetClass(), GetDeity(), iOther->GetPrimaryFaction(), iOther);
}

bool Client::ReloadCharacterFaction(Client *c, uint32 facid, uint32 charid)
{
	if (database.SetCharacterFactionLevel(charid, facid, 0, 0, factionvalues))
		return true;
	else
		return false;
}

//o--------------------------------------------------------------
//| Name: GetFactionLevel; Dec. 16, 2001
//o--------------------------------------------------------------
//| Notes: Gets the characters faction standing with the specified NPC.
//|			Will return Indifferent on failure.
//o--------------------------------------------------------------
FACTION_VALUE Client::GetFactionLevel(uint32 char_id, uint32 npc_id, uint32 p_race, uint32 p_class, uint32 p_deity, int32 pFaction, Mob* tnpc)
{
	if (pFaction < 0)
		return GetSpecialFactionCon(tnpc);
	FACTION_VALUE fac = FACTION_INDIFFERENT;
	int32 tmpFactionValue;
	FactionMods fmods;

	// few optimizations
	if (GetFeigned())
		return FACTION_INDIFFERENT;
	if(!zone->CanDoCombat())
		return FACTION_INDIFFERENT;
	if (invisible_undead && tnpc && !tnpc->SeeInvisibleUndead())
		return FACTION_INDIFFERENT;
	if (IsInvisible(tnpc))
		return FACTION_INDIFFERENT;
	if (tnpc && tnpc->GetOwnerID() != 0) // pets con amiably to owner and indiff to rest
	{
		if (char_id == tnpc->GetOwner()->CastToClient()->CharacterID())
			return FACTION_AMIABLE;
		else
			return FACTION_INDIFFERENT;
	}

	//First get the NPC's Primary faction
	if(pFaction > 0)
	{
		//Get the faction data from the database
		if(content_db.GetFactionData(&fmods, p_class, p_race, p_deity, pFaction))
		{
			//Get the players current faction with pFaction
			tmpFactionValue = GetCharacterFactionLevel(pFaction);
			//Tack on any bonuses from Alliance type spell effects
			tmpFactionValue += GetFactionBonus(pFaction);
			tmpFactionValue += GetItemFactionBonus(pFaction);
			//Return the faction to the client
			fac = CalculateFaction(&fmods, tmpFactionValue);
		}
	}
	else
	{
		return(FACTION_INDIFFERENT);
	}

	// merchant fix
	if (tnpc && tnpc->IsNPC() && tnpc->CastToNPC()->MerchantType && (fac == FACTION_THREATENLY || fac == FACTION_SCOWLS))
		fac = FACTION_DUBIOUS;

	if (tnpc != 0 && fac != FACTION_SCOWLS && tnpc->CastToNPC()->CheckAggro(this))
		fac = FACTION_THREATENLY;

	return fac;
}

//Sets the characters faction standing with the specified NPC.
void Client::SetFactionLevel(uint32 char_id, uint32 npc_id, uint8 char_class, uint8 char_race, uint8 char_deity, bool quest)
{
	int32 faction_id[MAX_NPC_FACTIONS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int32 npc_value[MAX_NPC_FACTIONS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	uint8 temp[MAX_NPC_FACTIONS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int32 current_value;

	// Get the npc faction list
	if (!content_db.GetNPCFactionList(npc_id, faction_id, npc_value, temp)) {
		return;
	}

	for (int i = 0; i < MAX_NPC_FACTIONS; i++) {
		int32 faction_before_hit;
		int32 faction_to_use_for_messaging;
		FactionMods fm;
		int32 this_faction_max;
		int32 this_faction_min;

		if (faction_id[i] <= 0)
			continue;

		// Find out starting faction for this faction
		// It needs to be used to adj max and min personal
		// The range is still the same, 1200-3000(4200), but adjusted for base
		content_db.GetFactionData(&fm, GetClass(), GetFactionRace(), GetDeity(), faction_id[i]);

		if (quest)
		{
			//The ole switcheroo
			if (npc_value[i] > 0)
				npc_value[i] = -std::abs(npc_value[i]);
			else if (npc_value[i] < 0)
				npc_value[i] = std::abs(npc_value[i]);
		}

		// Adjust the amount you can go up or down so the resulting range
		// is PERSONAL_MAX - PERSONAL_MIN
		//
		// Adjust these values for cases where starting faction is below
		// min or above max by not allowing any earn in those directions.
		this_faction_min = fm.min - fm.base;
		this_faction_min = std::min(0, this_faction_min);
		this_faction_max = fm.max - fm.base;
		this_faction_max = std::max(0, this_faction_max);

		// Get the characters current value with that faction
		current_value = GetCharacterFactionLevel(faction_id[i]);
		faction_before_hit = current_value;

		UpdatePersonalFaction(char_id, npc_value[i], faction_id[i], &current_value, temp[i], this_faction_min, this_faction_max);

		//Message(Chat::Lime, "Min(%d) Max(%d) Before(%d), After(%d)\n", this_faction_min, this_faction_max, faction_before_hit, current_value);

		SendFactionMessage(npc_value[i], faction_id[i], faction_before_hit, current_value, temp[i], this_faction_min, this_faction_max);
	}

	return;
}

void Client::SetFactionLevel2(uint32 char_id, int32 faction_id, uint8 char_class, uint8 char_race, uint8 char_deity, int32 value, uint8 temp)
{
	int32 current_value;

	//Get the npc faction list
	if(faction_id > 0 && value != 0) {
		int32 faction_before_hit;
		FactionMods fm;
		int32 this_faction_max;
		int32 this_faction_min;

		// Find out starting faction for this faction
		// It needs to be used to adj max and min personal
		// The range is still the same, 1200-3000(4200), but adjusted for base
		content_db.GetFactionData(&fm, GetClass(), GetFactionRace(), GetDeity(),
			faction_id);

		// Adjust the amount you can go up or down so the resulting range
		// is PERSONAL_MAX - PERSONAL_MIN
		//
		// Adjust these values for cases where starting faction is below
		// min or above max by not allowing any earn/loss in those directions.
		// At least one faction starts out way below min, so we don't want
		// to allow loses in those cases, just massive gains.
		this_faction_min = fm.min - fm.base;
		this_faction_min = std::min(0, this_faction_min);
		this_faction_max = fm.max - fm.base;
		this_faction_max = std::max(0, this_faction_max);

		//Get the faction modifiers
		current_value = GetCharacterFactionLevel(faction_id);
		faction_before_hit = current_value;

		UpdatePersonalFaction(char_id, value, faction_id, &current_value, temp, this_faction_min, this_faction_max);

		//Message(Chat::Lime, "Min(%d) Max(%d) Before(%d), After(%d)\n", this_faction_min, this_faction_max, faction_before_hit, current_value);

		SendFactionMessage(value, faction_id, faction_before_hit, current_value, temp, this_faction_min, this_faction_max);
	}

	return;
}

int32 Client::GetCharacterFactionLevel(int32 faction_id)
{
	if (faction_id <= 0)
		return 0;
	faction_map::iterator res;
	res = factionvalues.find(faction_id);
	if (res == factionvalues.end())
		return 0;
	return res->second;
}

// Common code to set faction level.
// Applies HeroicCHA is it applies
// Checks for bottom out and max faction and old faction db entries
// Updates the faction if we are not minned, maxed or we need to repair

void Client::UpdatePersonalFaction(int32 char_id, int32 npc_value, int32 faction_id, int32 *current_value, int32 temp, int32 this_faction_min, int32 this_faction_max)
{
	bool repair = false;
	bool change = false;

	if (this->itembonuses.HeroicCHA)
	{
		int faction_mod = itembonuses.HeroicCHA / 5;
		// If our result isn't truncated, then just do that
		if (npc_value * faction_mod / 100 != 0)
			npc_value += npc_value * faction_mod / 100;
		// If our result is truncated, then double a mob's value every once and a while to equal what they would have got
		else
		{
			if (zone->random.Int(0, 100) < faction_mod)
				npc_value *= 2;
		}
	}

	// Set flag when to update db
	// Repair needed, as db changes could modify a base value for a faction
	// and we need to auto correct when that happens.
	if (*current_value > this_faction_max)
	{
		*current_value = this_faction_max;
		repair = true;
	}
	else if (*current_value < this_faction_min)
	{
		*current_value = this_faction_min;
		repair = true;
	}
	else if ((m_pp.gm != 1) && (npc_value != 0) &&
		((npc_value > 0 && *current_value != this_faction_max) ||
		((npc_value < 0 && *current_value != this_faction_min))))
		change = true;

	if (change || repair)
	{
		*current_value += npc_value;

		if (*current_value > this_faction_max)
			*current_value = this_faction_max;
		else if (*current_value < this_faction_min)
			*current_value = this_faction_min;

		database.SetCharacterFactionLevel(char_id, faction_id, *current_value, temp, factionvalues);
	}

return;
}

// returns the character's faction level, adjusted for racial, class, and deity modifiers
int32 Client::GetModCharacterFactionLevel(int32 faction_id) {
	int32 Modded = GetCharacterFactionLevel(faction_id);
	FactionMods fm;
	if (content_db.GetFactionData(&fm, GetClass(), GetFactionRace(), GetDeity(), faction_id))
	{
		Modded += fm.base + fm.class_mod + fm.race_mod + fm.deity_mod;

		//Tack on any bonuses from Alliance type spell effects
		Modded += GetFactionBonus(faction_id);
		Modded += GetItemFactionBonus(faction_id);
	}

	return Modded;
}

void Client::MerchantRejectMessage(Mob *merchant, int primaryfaction)
{
	int messageid = 0;
	int32 tmpFactionValue = 0;
	int32 lowestvalue = 0;
	FactionMods fmod;

	// If a faction is involved, get the data.
	if (primaryfaction > 0) {
		if (content_db.GetFactionData(&fmod, GetClass(), GetFactionRace(), GetDeity(), primaryfaction)) {
			tmpFactionValue = GetCharacterFactionLevel(primaryfaction);
			lowestvalue = std::min(std::min(tmpFactionValue, fmod.deity_mod),
						  std::min(fmod.class_mod, fmod.race_mod));
		}
	}
	// If no primary faction or biggest influence is your faction hit
	if (primaryfaction <= 0 || lowestvalue == tmpFactionValue) {
		merchant->SayString(zone->random.Int(WONT_SELL_DEEDS1, WONT_SELL_DEEDS6));
	} else if (lowestvalue == fmod.race_mod) { // race biggest
		// Non-standard race (ex. illusioned to wolf)
		if (GetRace() > PLAYER_RACE_COUNT) {
			messageid = zone->random.Int(1, 3); // these aren't sequential StringIDs :(
			switch (messageid) {
			case 1:
				messageid = WONT_SELL_NONSTDRACE1;
				break;
			case 2:
				messageid = WONT_SELL_NONSTDRACE2;
				break;
			case 3:
				messageid = WONT_SELL_NONSTDRACE3;
				break;
			default: // w/e should never happen
				messageid = WONT_SELL_NONSTDRACE1;
				break;
			}
			merchant->SayString(messageid);
		} else { // normal player races
			messageid = zone->random.Int(1, 4);
			switch (messageid) {
			case 1:
				messageid = WONT_SELL_RACE1;
				break;
			case 2:
				messageid = WONT_SELL_RACE2;
				break;
			case 3:
				messageid = WONT_SELL_RACE3;
				break;
			case 4:
				messageid = WONT_SELL_RACE4;
				break;
			default: // w/e should never happen
				messageid = WONT_SELL_RACE1;
				break;
			}
			merchant->SayString(messageid, itoa(GetRace()));
		}
	} else if (lowestvalue == fmod.class_mod) {
		merchant->SayString(zone->random.Int(WONT_SELL_CLASS1, WONT_SELL_CLASS5), itoa(GetClass()));
	} else {
		// Must be deity - these two sound the best for that.
		// Can't use a message with a field, GUI wants class/race names.
		// for those message IDs.  These are straight text.
		merchant->SayString(zone->random.Int(WONT_SELL_DEEDS1, WONT_SELL_DEEDS2));
	}
	return;
}

//o--------------------------------------------------------------
//| Name: SendFactionMessage
//o--------------------------------------------------------------
//| Purpose: Send faction change message to client
//o--------------------------------------------------------------
void Client::SendFactionMessage(int32 tmpvalue, int32 faction_id, int32 faction_before_hit, int32 totalvalue, uint8 temp, int32 this_faction_min, int32 this_faction_max)
{
	char name[50];
	int32 faction_value;

	// If we're dropping from MAX or raising from MIN or repairing,
	// we should base the message on the new updated value so we don't show
	// a min MAX message
	//
	// If we're changing any other place, we use the value before the
	// hit.  For example, if we go from 1199 to 1200 which is the MAX
	// we still want to say faction got better this time around.

	if ( (faction_before_hit >= this_faction_max) ||
	     (faction_before_hit <= this_faction_min))
		faction_value = totalvalue;
	else
		faction_value = faction_before_hit;

	// default to Faction# if we couldn't get the name from the ID
	if (content_db.GetFactionName(faction_id, name, sizeof(name)) == false)
		snprintf(name, sizeof(name), "Faction%i", faction_id);

	if (tmpvalue == 0 || temp == 1 || temp == 2)
		return;
	else if (faction_value >= this_faction_max)
		MessageString(Chat::Yellow, FACTION_BEST, name);
	else if (faction_value <= this_faction_min)
		MessageString(Chat::Yellow, FACTION_WORST, name);
	else if (tmpvalue > 0 && faction_value < this_faction_max && !RuleB(Client, UseLiveFactionMessage))
		MessageString(Chat::Yellow, FACTION_BETTER, name);
	else if (tmpvalue < 0 && faction_value > this_faction_min && !RuleB(Client, UseLiveFactionMessage))
		MessageString(Chat::Yellow, FACTION_WORSE, name);
	else if (RuleB(Client, UseLiveFactionMessage))
		Message(Chat::Yellow, "Your faction standing with %s has been adjusted by %i.", name, tmpvalue); //New Live faction message (14261)

	return;
}

void Client::LoadAccountFlags()
{

	accountflags.clear();
	std::string query = StringFormat("SELECT p_flag, p_value "
									"FROM account_flags WHERE p_accid = '%d'",
									account_id);
	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return;
	}

	for (auto row = results.begin(); row != results.end(); ++row)
		accountflags[row[0]] = row[1];
}

void Client::SetAccountFlag(std::string flag, std::string val) {

	std::string query = StringFormat("REPLACE INTO account_flags (p_accid, p_flag, p_value) "
									"VALUES( '%d', '%s', '%s')",
									account_id, flag.c_str(), val.c_str());
	auto results = database.QueryDatabase(query);
	if(!results.Success()) {
		return;
	}

	accountflags[flag] = val;
}

std::string Client::GetAccountFlag(std::string flag)
{
	return(accountflags[flag]);
}

void Client::TickItemCheck()
{
	int i;

	if(zone->tick_items.empty()) { return; }

	//Scan equip, general, cursor slots for items
	for (i = EQ::invslot::POSSESSIONS_BEGIN; i <= EQ::invslot::POSSESSIONS_END; i++)
	{
		TryItemTick(i);
	}
	//Scan bags
	for (i = EQ::invbag::GENERAL_BAGS_BEGIN; i <= EQ::invbag::CURSOR_BAG_END; i++)
	{
		TryItemTick(i);
	}
}

void Client::TryItemTick(int slot)
{
	int iid = 0;
	const EQ::ItemInstance* inst = m_inv[slot];
	if(inst == 0) { return; }

	iid = inst->GetID();

	if(zone->tick_items.count(iid) > 0)
	{
		if (GetLevel() >= zone->tick_items[iid].level && zone->random.Int(0, 100) >= (100 - zone->tick_items[iid].chance) && (zone->tick_items[iid].bagslot || slot <= EQ::invslot::EQUIPMENT_END))
		{
			EQ::ItemInstance* e_inst = (EQ::ItemInstance*)inst;
			parse->EventItem(EVENT_ITEM_TICK, this, e_inst, nullptr, "", slot);
		}
	}

	//Only look at augs in main inventory
	if (slot > EQ::invslot::EQUIPMENT_END) { return; }

	for (int x = EQ::invaug::SOCKET_BEGIN; x <= EQ::invaug::SOCKET_END; ++x)
	{
		EQ::ItemInstance * a_inst = inst->GetAugment(x);
		if(!a_inst) { continue; }

		iid = a_inst->GetID();

		if(zone->tick_items.count(iid) > 0)
		{
			if( GetLevel() >= zone->tick_items[iid].level && zone->random.Int(0, 100) >= (100 - zone->tick_items[iid].chance) )
			{
				EQ::ItemInstance* e_inst = (EQ::ItemInstance*)a_inst;
				parse->EventItem(EVENT_ITEM_TICK, this, e_inst, nullptr, "", slot);
			}
		}
	}
}

void Client::ItemTimerCheck()
{
	int i;
	for (i = EQ::invslot::POSSESSIONS_BEGIN; i <= EQ::invslot::POSSESSIONS_END; i++)
	{
		TryItemTimer(i);
	}
	for (i = EQ::invbag::GENERAL_BAGS_BEGIN; i <= EQ::invbag::CURSOR_BAG_END; i++)
	{
		TryItemTimer(i);
	}
}

void Client::TryItemTimer(int slot)
{
	EQ::ItemInstance* inst = m_inv.GetItem(slot);
	if(!inst) {
		return;
	}

	auto item_timers = inst->GetTimers();
	auto it_iter = item_timers.begin();
	while(it_iter != item_timers.end()) {
		if(it_iter->second.Check()) {
			parse->EventItem(EVENT_TIMER, this, inst, nullptr, it_iter->first, 0);
		}
		++it_iter;
	}

	if (slot > EQ::invslot::EQUIPMENT_END) {
		return;
	}

	for (int x = EQ::invaug::SOCKET_BEGIN; x <= EQ::invaug::SOCKET_END; ++x)
	{
		EQ::ItemInstance * a_inst = inst->GetAugment(x);
		if(!a_inst) {
			continue;
		}

		auto item_timers = a_inst->GetTimers();
		auto it_iter = item_timers.begin();
		while(it_iter != item_timers.end()) {
			if(it_iter->second.Check()) {
				parse->EventItem(EVENT_TIMER, this, a_inst, nullptr, it_iter->first, 0);
			}
			++it_iter;
		}
	}
}

void Client::SendItemScale(EQ::ItemInstance *inst) {
	int slot = m_inv.GetSlotByItemInst(inst);
	if(slot != -1) {
		inst->ScaleItem();
		SendItemPacket(slot, inst, ItemPacketCharmUpdate);
		CalcBonuses();
	}
}

void Client::AddRespawnOption(std::string option_name, uint32 zoneid, uint16 instance_id, float x, float y, float z, float heading, bool initial_selection, int8 position)
{
	//If respawn window is already open, any changes would create an inconsistency with the client
	if (IsHoveringForRespawn()) { return; }

	if (zoneid == 0)
		zoneid = zone->GetZoneID();

	//Create respawn option
	RespawnOption res_opt;
	res_opt.name = option_name;
	res_opt.zone_id = zoneid;
	res_opt.instance_id = instance_id;
	res_opt.x = x;
	res_opt.y = y;
	res_opt.z = z;
	res_opt.heading = heading;

	if (position == -1 || position >= respawn_options.size())
	{
		//No position specified, or specified beyond the end, simply append
		respawn_options.push_back(res_opt);
		//Make this option the initial selection for the window if desired
		if (initial_selection)
			initial_respawn_selection = static_cast<uint8>(respawn_options.size()) - 1;
	}
	else if (position == 0)
	{
		respawn_options.push_front(res_opt);
		if (initial_selection)
			initial_respawn_selection = 0;
	}
	else
	{
		//Insert new option between existing options
		std::list<RespawnOption>::iterator itr;
		uint8 pos = 0;
		for (itr = respawn_options.begin(); itr != respawn_options.end(); ++itr)
		{
			if (pos++ == position)
			{
				respawn_options.insert(itr,res_opt);
				//Make this option the initial selection for the window if desired
				if (initial_selection)
					initial_respawn_selection = pos;
				return;
			}
		}
	}
}

bool Client::RemoveRespawnOption(std::string option_name)
{
	//If respawn window is already open, any changes would create an inconsistency with the client
	if (IsHoveringForRespawn() || respawn_options.empty()) { return false; }

	bool had = false;
	RespawnOption* opt = nullptr;
	std::list<RespawnOption>::iterator itr;
	for (itr = respawn_options.begin(); itr != respawn_options.end(); ++itr)
	{
		opt = &(*itr);
		if (opt->name.compare(option_name) == 0)
		{
			itr = respawn_options.erase(itr);
			had = true;
			//could be more with the same name, so keep going...
		}
	}
	return had;
}

bool Client::RemoveRespawnOption(uint8 position)
{
	//If respawn window is already open, any changes would create an inconsistency with the client
	if (IsHoveringForRespawn() || respawn_options.empty()) { return false; }

	//Easy cases first...
	if (position == 0)
	{
		respawn_options.pop_front();
		return true;
	}
	else if (position == (respawn_options.size() - 1))
	{
		respawn_options.pop_back();
		return true;
	}

	std::list<RespawnOption>::iterator itr;
	uint8 pos = 0;
	for (itr = respawn_options.begin(); itr != respawn_options.end(); ++itr)
	{
		if (pos++ == position)
		{
			respawn_options.erase(itr);
			return true;
		}
	}
	return false;
}

void Client::SetHunger(int32 in_hunger)
{
	EQApplicationPacket *outapp = nullptr;
	outapp = new EQApplicationPacket(OP_Stamina, sizeof(Stamina_Struct));
	Stamina_Struct* sta = (Stamina_Struct*)outapp->pBuffer;
	sta->food = in_hunger;
	sta->water = m_pp.thirst_level > 6000 ? 6000 : m_pp.thirst_level;

	m_pp.hunger_level = in_hunger;

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SetThirst(int32 in_thirst)
{
	EQApplicationPacket *outapp = nullptr;
	outapp = new EQApplicationPacket(OP_Stamina, sizeof(Stamina_Struct));
	Stamina_Struct* sta = (Stamina_Struct*)outapp->pBuffer;
	sta->food = m_pp.hunger_level > 6000 ? 6000 : m_pp.hunger_level;
	sta->water = in_thirst;

	m_pp.thirst_level = in_thirst;

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SetConsumption(int32 in_hunger, int32 in_thirst)
{
	EQApplicationPacket *outapp = nullptr;
	outapp = new EQApplicationPacket(OP_Stamina, sizeof(Stamina_Struct));
	Stamina_Struct* sta = (Stamina_Struct*)outapp->pBuffer;
	sta->food = in_hunger;
	sta->water = in_thirst;

	m_pp.hunger_level = in_hunger;
	m_pp.thirst_level = in_thirst;

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::Consume(const EQ::ItemData *item, uint8 type, int16 slot, bool auto_consume)
{
	if (!item)
		return;

	int increase = item->CastTime_ * 100;
	if (!auto_consume) // force feeding is half as effective
		increase /= 2;

	if (increase < 0) // wasn't food? oh well
		return;

	if (type == EQ::item::ItemTypeFood) {
		increase = mod_food_value(item, increase);

		if (increase < 0)
			return;

		m_pp.hunger_level += increase;

		LogFood("Consuming food, points added to hunger_level: [{}] - current_hunger: [{}]", increase, m_pp.hunger_level);

		DeleteItemInInventory(slot, 1, false);

		if (!auto_consume) // no message if the client consumed for us
			entity_list.MessageCloseString(this, true, 50, 0, EATING_MESSAGE, GetName(), item->Name);

		LogFood("Eating from slot: [{}]", (int)slot);

	} else {
		increase = mod_drink_value(item, increase);

		if (increase < 0)
			return;

		m_pp.thirst_level += increase;

		DeleteItemInInventory(slot, 1, false);

		LogFood("Consuming drink, points added to thirst_level: [{}] current_thirst: [{}]", increase, m_pp.thirst_level);

		if (!auto_consume) // no message if the client consumed for us
			entity_list.MessageCloseString(this, true, 50, 0, DRINKING_MESSAGE, GetName(), item->Name);

		LogFood("Drinking from slot: [{}]", (int)slot);
	}
}

void Client::SendMarqueeMessage(uint32 type, uint32 priority, uint32 fade_in, uint32 fade_out, uint32 duration, std::string msg)
{
	if(duration == 0 || msg.length() == 0) {
		return;
	}

	EQApplicationPacket outapp(OP_Marquee, sizeof(ClientMarqueeMessage_Struct) + msg.length());
	ClientMarqueeMessage_Struct *cms = (ClientMarqueeMessage_Struct*)outapp.pBuffer;

	cms->type = type;
	cms->unk04 = 10;
	cms->priority = priority;
	cms->fade_in_time = fade_in;
	cms->fade_out_time = fade_out;
	cms->duration = duration;
	strcpy(cms->msg, msg.c_str());

	QueuePacket(&outapp);
}

void Client::PlayMP3(const char* fname)
{
	std::string filename = fname;
	auto outapp = new EQApplicationPacket(OP_PlayMP3, filename.length() + 1);
	PlayMP3_Struct* buf = (PlayMP3_Struct*)outapp->pBuffer;
	strncpy(buf->filename, fname, filename.length());
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::ExpeditionSay(const char *str, int ExpID) {

	std::string query = StringFormat("SELECT `player_name` FROM `cust_inst_players` "
									"WHERE `inst_id` = %i", ExpID);
	auto results = database.QueryDatabase(query);
	if (!results.Success())
		return;

	if(results.RowCount() == 0) {
		this->Message(Chat::Lime, "You say to the expedition, '%s'", str);
		return;
	}

	for(auto row = results.begin(); row != results.end(); ++row) {
		const char* charName = row[0];
		if(strcmp(charName, this->GetCleanName()) != 0)
			worldserver.SendEmoteMessage(charName, 0, 0, 14, "%s says to the expedition, '%s'", this->GetCleanName(), str);
		// ChannelList->CreateChannel(ChannelName, ChannelOwner, ChannelPassword, true, atoi(row[3]));
	}


}

void Client::ShowNumHits()
{
	uint32 buffcount = GetMaxTotalSlots();
	for (uint32 buffslot = 0; buffslot < buffcount; buffslot++) {
		const Buffs_Struct &curbuff = buffs[buffslot];
		if (curbuff.spellid != SPELL_UNKNOWN && curbuff.numhits)
			Message(0, "You have %d hits left on %s", curbuff.numhits, GetSpellName(curbuff.spellid));
	}
	return;
}

int Client::GetQuiverHaste(int delay)
{
	const EQ::ItemInstance *pi = nullptr;
	for (int r = EQ::invslot::GENERAL_BEGIN; r <= EQ::invslot::GENERAL_END; r++) {
		pi = GetInv().GetItem(r);
		if (pi && pi->IsClassBag() && pi->GetItem()->BagType == EQ::item::BagTypeQuiver &&
		    pi->GetItem()->BagWR > 0)
			break;
		if (r == EQ::invslot::GENERAL_END)
			// we will get here if we don't find a valid quiver
			return 0;
	}
	return (pi->GetItem()->BagWR * 0.0025f * delay) + 1;
}

void Client::SendColoredText(uint32 color, std::string message)
{
	// arbitrary size limit
	if (message.size() > 512) // live does send this with empty strings sometimes ...
		return;
	auto outapp = new EQApplicationPacket(OP_ColoredText, sizeof(ColoredText_Struct) + message.size());
	ColoredText_Struct *cts = (ColoredText_Struct *)outapp->pBuffer;
	cts->color = color;
	strcpy(cts->msg, message.c_str());
	QueuePacket(outapp);
	safe_delete(outapp);
}


void Client::QuestReward(Mob* target, uint32 copper, uint32 silver, uint32 gold, uint32 platinum, uint32 itemid, uint32 exp, bool faction) {

	auto outapp = new EQApplicationPacket(OP_Sound, sizeof(QuestReward_Struct));
	memset(outapp->pBuffer, 0, sizeof(QuestReward_Struct));
	QuestReward_Struct* qr = (QuestReward_Struct*)outapp->pBuffer;

	qr->mob_id = target ? target->GetID() : 0;		// Entity ID for the from mob name
	qr->target_id = GetID();			// The Client ID (this)
	qr->copper = copper;
	qr->silver = silver;
	qr->gold = gold;
	qr->platinum = platinum;
	qr->item_id[0] = itemid;
	qr->exp_reward = exp;

	if (copper > 0 || silver > 0 || gold > 0 || platinum > 0)
		AddMoneyToPP(copper, silver, gold, platinum, false);

	if (itemid > 0)
		SummonItem(itemid, -1, 0, 0, 0, 0, 0, false, EQ::invslot::slotCursor);

	if (faction)
	{
		if (target && target->IsNPC())
		{
			int32 nfl_id = target->CastToNPC()->GetNPCFactionID();
			SetFactionLevel(CharacterID(), nfl_id, GetBaseClass(), GetBaseRace(), GetDeity(), true);
			qr->faction = target->CastToNPC()->GetPrimaryFaction();
			qr->faction_mod = 1; // Too lazy to get real value, not sure if this is even used by client anyhow.
		}
	}

	if (exp > 0)
		AddEXP(exp);

	QueuePacket(outapp, true, Client::CLIENT_CONNECTED);
	safe_delete(outapp);
}

void Client::QuestReward(Mob* target, const QuestReward_Struct &reward, bool faction)
{
	auto outapp = new EQApplicationPacket(OP_Sound, sizeof(QuestReward_Struct));
	memset(outapp->pBuffer, 0, sizeof(QuestReward_Struct));
	QuestReward_Struct* qr = (QuestReward_Struct*)outapp->pBuffer;

	memcpy(qr, &reward, sizeof(QuestReward_Struct));

	// not set in caller because reasons
	qr->mob_id = target ? target->GetID() : 0;		// Entity ID for the from mob name

	if (reward.copper > 0 || reward.silver > 0 || reward.gold > 0 || reward.platinum > 0)
		AddMoneyToPP(reward.copper, reward.silver, reward.gold, reward.platinum, false);

	for (int i = 0; i < QUESTREWARD_COUNT; ++i)
		if (reward.item_id[i] > 0)
			SummonItem(reward.item_id[i], 0, 0, 0, 0, 0, 0, false, EQ::invslot::slotCursor);

	if (faction)
	{
		if (target && target->IsNPC())
		{
			int32 nfl_id = target->CastToNPC()->GetNPCFactionID();
			SetFactionLevel(CharacterID(), nfl_id, GetBaseClass(), GetBaseRace(), GetDeity(), true);
			qr->faction = target->CastToNPC()->GetPrimaryFaction();
			qr->faction_mod = 1; // Too lazy to get real value, not sure if this is even used by client anyhow.
		}
	}

	if (reward.exp_reward> 0)
		AddEXP(reward.exp_reward);

	QueuePacket(outapp, true, Client::CLIENT_CONNECTED);
	safe_delete(outapp);
}

void Client::SendHPUpdateMarquee(){
	if (!this || !this->IsClient() || !this->current_hp || !this->max_hp)
		return;

	/* Health Update Marquee Display: Custom*/
	uint8 health_percentage = (uint8)(this->current_hp * 100 / this->max_hp);
	if (health_percentage >= 100)
		return;

	std::string health_update_notification = StringFormat("Health: %u%%", health_percentage);
	this->SendMarqueeMessage(Chat::Yellow, 510, 0, 3000, 3000, health_update_notification);
}

uint32 Client::GetMoney(uint8 type, uint8 subtype) {
	uint32 value = 0;
	switch (type) {
		case 0: {
			switch (subtype) {
				case 0:
					value = static_cast<uint32>(m_pp.copper);
					break;
				case 1:
					value = static_cast<uint32>(m_pp.copper_bank);
					break;
				case 2:
					value = static_cast<uint32>(m_pp.copper_cursor);
					break;
				default:
					break;
			}
			break;
		}
		case 1: {
			switch (subtype) {
				case 0:
					value = static_cast<uint32>(m_pp.silver);
					break;
				case 1:
					value = static_cast<uint32>(m_pp.silver_bank);
					break;
				case 2:
					value = static_cast<uint32>(m_pp.silver_cursor);
					break;
				default:
					break;
			}
			break;
		}
		case 2: {
			switch (subtype) {
				case 0:
					value = static_cast<uint32>(m_pp.gold);
					break;
				case 1:
					value = static_cast<uint32>(m_pp.gold_bank);
					break;
				case 2:
					value = static_cast<uint32>(m_pp.gold_cursor);
					break;
				default:
					break;
			}
			break;
		}
		case 3: {
			switch (subtype) {
				case 0:
					value = static_cast<uint32>(m_pp.platinum);
					break;
				case 1:
					value = static_cast<uint32>(m_pp.platinum_bank);
					break;
				case 2:
					value = static_cast<uint32>(m_pp.platinum_cursor);
					break;
				case 3:
					value = static_cast<uint32>(m_pp.platinum_shared);
					break;
				default:
					break;
			}
			break;
		}
		default:
			break;
	}
	return value;
}

int Client::GetAccountAge() {
	return (time(nullptr) - GetAccountCreation());
}

void Client::CheckRegionTypeChanges()
{
	if (!zone->HasWaterMap())
		return;

	auto new_region = zone->watermap->ReturnRegionType(glm::vec3(m_Position));

	// still same region, do nothing
	if (last_region_type == new_region)
		return;

	// If we got out of water clear any water aggro for water only npcs
	if (last_region_type == RegionTypeWater) {
		entity_list.ClearWaterAggro(this);
	}

	// region type changed
	last_region_type = new_region;

	// PVP is the only state we need to keep track of, so we can just return now for PVP servers
	if (RuleI(World, PVPSettings) > 0)
		return;

	if (last_region_type == RegionTypePVP)
		temp_pvp = true;
	else if (temp_pvp)
		temp_pvp = false;
}

void Client::ProcessAggroMeter()
{
	if (!AggroMeterAvailable()) {
		aggro_meter_timer.Disable();
		return;
	}

	// we need to decide if we need to send OP_AggroMeterTargetInfo now
	// This packet sends the current lock target ID and the current target ID
	// target ID will be either our target or our target of target when we're targeting a PC
	bool send_targetinfo = false;
	auto cur_tar = GetTarget();

	// probably should have PVP rules ...
	if (cur_tar && cur_tar != this) {
		if (cur_tar->IsNPC() && !cur_tar->IsPetOwnerClient() && cur_tar->GetID() != m_aggrometer.get_target_id()) {
			m_aggrometer.set_target_id(cur_tar->GetID());
			send_targetinfo = true;
		} else if ((cur_tar->IsPetOwnerClient() || cur_tar->IsClient()) && cur_tar->GetTarget() && cur_tar->GetTarget()->GetID() != m_aggrometer.get_target_id()) {
			m_aggrometer.set_target_id(cur_tar->GetTarget()->GetID());
			send_targetinfo = true;
		}
	} else if (m_aggrometer.get_target_id()) {
		m_aggrometer.set_target_id(0);
		send_targetinfo = true;
	}

	if (m_aggrometer.update_lock())
		send_targetinfo = true;

	if (send_targetinfo) {
		auto app = new EQApplicationPacket(OP_AggroMeterTargetInfo, sizeof(uint32) * 2);
		app->WriteUInt32(m_aggrometer.get_lock_id());
		app->WriteUInt32(m_aggrometer.get_target_id());
		FastQueuePacket(&app);
	}

	// we could just calculate how big the packet would need to be ... but it's easier this way :P should be 87 bytes
	auto app = new EQApplicationPacket(OP_AggroMeterUpdate, m_aggrometer.max_packet_size());

	cur_tar = entity_list.GetMob(m_aggrometer.get_target_id());

	// first we must check the secondary
	// TODO: lock target should affect secondary as well
	bool send = false;
	Mob *secondary = nullptr;
	bool has_aggro = false;
	if (cur_tar) {
		if (cur_tar->GetTarget() == this) {// we got aggro
			secondary = cur_tar->GetSecondaryHate(this);
			has_aggro = true;
		} else {
			secondary = cur_tar->CheckAggro(cur_tar->GetTarget()) ? cur_tar->GetTarget() : nullptr; // make sure they are targeting for aggro reasons
		}
	}

	if (secondary && secondary->GetID() != m_aggrometer.get_secondary_id()) {
		m_aggrometer.set_secondary_id(secondary->GetID());
		app->WriteUInt8(1);
		app->WriteUInt32(m_aggrometer.get_secondary_id());
		send = true;
	} else if (!secondary && m_aggrometer.get_secondary_id()) {
		m_aggrometer.set_secondary_id(0);
		app->WriteUInt8(1);
		app->WriteUInt32(0);
		send = true;
	} else { // might not need to send in this case
		app->WriteUInt8(0);
	}

	auto count_offset = app->GetWritePosition();
	app->WriteUInt8(0);

	int count = 0;
	auto add_entry = [&app, &count, this](AggroMeter::AggroTypes i) {
		count++;
		app->WriteUInt8(i);
		app->WriteUInt16(m_aggrometer.get_pct(i));
	};
	// TODO: Player entry should either be lock or yourself, ignoring lock for now
	// player, secondary, and group depend on your target/lock
	if (cur_tar) {
		if (m_aggrometer.set_pct(AggroMeter::AT_Player, cur_tar->GetHateRatio(cur_tar->GetTarget(), this)))
			add_entry(AggroMeter::AT_Player);

		if (m_aggrometer.set_pct(AggroMeter::AT_Secondary, has_aggro ? cur_tar->GetHateRatio(this, secondary) : secondary ? 100 : 0))
			add_entry(AggroMeter::AT_Secondary);

		// fuuuuuuuuuuuuuuuuuuuuuuuucckkkkkkkkkkkkkkk raids
		if (IsRaidGrouped()) {
			auto raid = GetRaid();
			if (raid) {
				auto gid = raid->GetGroup(this);
				if (gid < 12) {
					int at_id = AggroMeter::AT_Group1;
					for (int i = 0; i < MAX_RAID_MEMBERS; ++i) {
						if (raid->members[i].member && raid->members[i].member != this && raid->members[i].GroupNumber == gid) {
							if (m_aggrometer.set_pct(static_cast<AggroMeter::AggroTypes>(at_id), cur_tar->GetHateRatio(cur_tar->GetTarget(), raid->members[i].member)))
								add_entry(static_cast<AggroMeter::AggroTypes>(at_id));
							at_id++;
							if (at_id > AggroMeter::AT_Group5)
								break;
						}
					}
				}
			}
		} else if (IsGrouped()) {
			auto group = GetGroup();
			if (group) {
				int at_id = AggroMeter::AT_Group1;
				for (int i = 0; i < MAX_GROUP_MEMBERS; ++i) {
					if (group->members[i] && group->members[i] != this) {
						if (m_aggrometer.set_pct(static_cast<AggroMeter::AggroTypes>(at_id), cur_tar->GetHateRatio(cur_tar->GetTarget(), group->members[i])))
							add_entry(static_cast<AggroMeter::AggroTypes>(at_id));
						at_id++;
					}
				}
			}
		}
	} else { // we might need to clear out some data now
		if (m_aggrometer.set_pct(AggroMeter::AT_Player, 0))
			add_entry(AggroMeter::AT_Player);
		if (m_aggrometer.set_pct(AggroMeter::AT_Secondary, 0))
			add_entry(AggroMeter::AT_Secondary);
		if (m_aggrometer.set_pct(AggroMeter::AT_Group1, 0))
			add_entry(AggroMeter::AT_Group1);
		if (m_aggrometer.set_pct(AggroMeter::AT_Group2, 0))
			add_entry(AggroMeter::AT_Group2);
		if (m_aggrometer.set_pct(AggroMeter::AT_Group3, 0))
			add_entry(AggroMeter::AT_Group3);
		if (m_aggrometer.set_pct(AggroMeter::AT_Group4, 0))
			add_entry(AggroMeter::AT_Group4);
		if (m_aggrometer.set_pct(AggroMeter::AT_Group5, 0))
			add_entry(AggroMeter::AT_Group5);
	}

	// now to go over our xtargets
	// if the entry is an NPC it's our hate relative to the NPCs current tank
	// if it's a PC, it's their hate relative to our current target
	for (int i = 0; i < GetMaxXTargets(); ++i) {
		if (XTargets[i].ID) {
			auto mob = entity_list.GetMob(XTargets[i].ID);
			if (mob) {
				int ratio = 0;
				if (mob->IsNPC())
					ratio = mob->GetHateRatio(mob->GetTarget(), this);
				else if (cur_tar)
					ratio = cur_tar->GetHateRatio(cur_tar->GetTarget(), mob);
				if (m_aggrometer.set_pct(static_cast<AggroMeter::AggroTypes>(AggroMeter::AT_XTarget1 + i), ratio))
					add_entry(static_cast<AggroMeter::AggroTypes>(AggroMeter::AT_XTarget1 + i));
			}
		}
	}

	if (send || count) {
		app->size = app->GetWritePosition(); // this should be safe, although not recommended
		// but this way we can have a smaller buffer created for the packet dispatched to the client w/o resizing this one
		app->SetWritePosition(count_offset);
		app->WriteUInt8(count);
		FastQueuePacket(&app);
	} else {
		safe_delete(app);
	}
}

void Client::SetPetCommandState(int button, int state)
{
	auto app = new EQApplicationPacket(OP_PetCommandState, sizeof(PetCommandState_Struct));
	auto pcs = (PetCommandState_Struct *)app->pBuffer;
	pcs->button_id = button;
	pcs->state = state;
	FastQueuePacket(&app);
}

bool Client::CanMedOnHorse()
{
	// no horse is false
	if (GetHorseId() == 0)
		return false;

	// can't med while attacking
	if (auto_attack)
		return false;

	return animation == 0 && m_Delta.x == 0.0f && m_Delta.y == 0.0f; // TODO: animation is SpeedRun
}

void Client::EnableAreaHPRegen(int value)
{
	AreaHPRegen = value * 0.001f;
	SendAppearancePacket(AT_AreaHPRegen, value, false);
}

void Client::DisableAreaHPRegen()
{
	AreaHPRegen = 1.0f;
	SendAppearancePacket(AT_AreaHPRegen, 1000, false);
}

void Client::EnableAreaManaRegen(int value)
{
	AreaManaRegen = value * 0.001f;
	SendAppearancePacket(AT_AreaManaRegen, value, false);
}

void Client::DisableAreaManaRegen()
{
	AreaManaRegen = 1.0f;
	SendAppearancePacket(AT_AreaManaRegen, 1000, false);
}

void Client::EnableAreaEndRegen(int value)
{
	AreaEndRegen = value * 0.001f;
	SendAppearancePacket(AT_AreaEndRegen, value, false);
}

void Client::DisableAreaEndRegen()
{
	AreaEndRegen = 1.0f;
	SendAppearancePacket(AT_AreaEndRegen, 1000, false);
}

void Client::EnableAreaRegens(int value)
{
	EnableAreaHPRegen(value);
	EnableAreaManaRegen(value);
	EnableAreaEndRegen(value);
}

void Client::DisableAreaRegens()
{
	DisableAreaHPRegen();
	DisableAreaManaRegen();
	DisableAreaEndRegen();
}

void Client::InitInnates()
{
	// this function on the client also inits the level one innate skills (like swimming, hide, etc)
	// we won't do that here, lets just do the InnateSkills for now. Basically translation of what the client is doing
	// A lot of these we could probably have ignored because they have no known use or are 100% client side
	// but I figured just in case we'll do them all out
	//
	// The client calls this in a few places. When you remove a vision buff and in SetHeights, which is called in
	// illusions, mounts, and a bunch of other cases. All of the calls to InitInnates are wrapped in restoring regen
	// besides the call initializing the first time
	auto race = GetRace();
	auto class_ = GetClass();

	for (int i = 0; i < InnateSkillMax; ++i)
		m_pp.InnateSkills[i] = InnateDisabled;

	m_pp.InnateSkills[InnateInspect] = InnateEnabled;
	m_pp.InnateSkills[InnateOpen] = InnateEnabled;
	if (race >= RT_FROGLOK_3) {
		if (race == RT_SKELETON_2 || race == RT_FROGLOK_3)
			m_pp.InnateSkills[InnateUltraVision] = InnateEnabled;
		else
			m_pp.InnateSkills[InnateInfravision] = InnateEnabled;
	}
	switch (race) {
	case RT_BARBARIAN:
	case RT_BARBARIAN_2:
		m_pp.InnateSkills[InnateSlam] = InnateEnabled;
		break;
	case RT_ERUDITE:
	case RT_ERUDITE_2:
		m_pp.InnateSkills[InnateLore] = InnateEnabled;
		break;
	case RT_WOOD_ELF:
	case RT_GUARD_3:
		m_pp.InnateSkills[InnateInfravision] = InnateEnabled;
		break;
	case RT_HIGH_ELF:
	case RT_GUARD_2:
		m_pp.InnateSkills[InnateInfravision] = InnateEnabled;
		m_pp.InnateSkills[InnateLore] = InnateEnabled;
		break;
	case RT_DARK_ELF:
	case RT_DARK_ELF_2:
	case RT_VAMPIRE_2:
		m_pp.InnateSkills[InnateUltraVision] = InnateEnabled;
		break;
	case RT_TROLL:
	case RT_TROLL_2:
		m_pp.InnateSkills[InnateRegen] = InnateEnabled;
		m_pp.InnateSkills[InnateSlam] = InnateEnabled;
		m_pp.InnateSkills[InnateInfravision] = InnateEnabled;
		break;
	case RT_DWARF:
	case RT_DWARF_2:
		m_pp.InnateSkills[InnateInfravision] = InnateEnabled;
		break;
	case RT_OGRE:
	case RT_OGRE_2:
		m_pp.InnateSkills[InnateInfravision] = InnateEnabled;
		m_pp.InnateSkills[InnateSlam] = InnateEnabled;
		m_pp.InnateSkills[InnateNoBash] = InnateEnabled;
		m_pp.InnateSkills[InnateBashDoor] = InnateEnabled;
		break;
	case RT_HALFLING:
	case RT_HALFLING_2:
		m_pp.InnateSkills[InnateInfravision] = InnateEnabled;
		break;
	case RT_GNOME:
		m_pp.InnateSkills[InnateInfravision] = InnateEnabled;
		m_pp.InnateSkills[InnateLore] = InnateEnabled;
		break;
	case RT_IKSAR:
		m_pp.InnateSkills[InnateRegen] = InnateEnabled;
		m_pp.InnateSkills[InnateInfravision] = InnateEnabled;
		break;
	case RT_VAH_SHIR:
		m_pp.InnateSkills[InnateInfravision] = InnateEnabled;
		break;
	case RT_FROGLOK_2:
	case RT_GHOST:
	case RT_GHOUL:
	case RT_SKELETON:
	case RT_VAMPIRE:
	case RT_WILL_O_WISP:
	case RT_ZOMBIE:
	case RT_SPECTRE:
	case RT_GHOST_2:
	case RT_GHOST_3:
	case RT_DRAGON_2:
	case RT_INNORUUK:
		m_pp.InnateSkills[InnateUltraVision] = InnateEnabled;
		break;
	case RT_HUMAN:
	case RT_GUARD:
	case RT_BEGGAR:
	case RT_HUMAN_2:
	case RT_HUMAN_3:
	case RT_FROGLOK_3: // client does froglok weird, but this should work out fine
		break;
	default:
		m_pp.InnateSkills[InnateInfravision] = InnateEnabled;
		break;
	}

	switch (class_) {
	case DRUID:
		m_pp.InnateSkills[InnateHarmony] = InnateEnabled;
		break;
	case BARD:
		m_pp.InnateSkills[InnateReveal] = InnateEnabled;
		break;
	case ROGUE:
		m_pp.InnateSkills[InnateSurprise] = InnateEnabled;
		m_pp.InnateSkills[InnateReveal] = InnateEnabled;
		break;
	case RANGER:
		m_pp.InnateSkills[InnateAwareness] = InnateEnabled;
		break;
	case MONK:
		m_pp.InnateSkills[InnateSurprise] = InnateEnabled;
		m_pp.InnateSkills[InnateAwareness] = InnateEnabled;
	default:
		break;
	}
}

bool Client::GetDisplayMobInfoWindow() const
{
	return display_mob_info_window;
}

void Client::SetDisplayMobInfoWindow(bool display_mob_info_window)
{
	Client::display_mob_info_window = display_mob_info_window;
}

bool Client::IsDevToolsEnabled() const
{
	return dev_tools_enabled && RuleB(World, EnableDevTools);
}

void Client::SetDevToolsEnabled(bool in_dev_tools_enabled)
{
	Client::dev_tools_enabled = in_dev_tools_enabled;
}

/**
 * @param model_id
 */
void Client::SetPrimaryWeaponOrnamentation(uint32 model_id)
{
	auto primary_item = m_inv.GetItem(EQ::invslot::slotPrimary);
	if (primary_item) {
		database.QueryDatabase(
			StringFormat(
				"UPDATE `inventory` SET `ornamentidfile` = %i WHERE `charid` = %i AND `slotid` = %i",
				model_id,
				character_id,
				EQ::invslot::slotPrimary
			));

		primary_item->SetOrnamentationIDFile(model_id);
		SendItemPacket(EQ::invslot::slotPrimary, primary_item, ItemPacketTrade);
		WearChange(EQ::textures::weaponPrimary, static_cast<uint16>(model_id), 0);

		Message(Chat::Yellow, "Your primary weapon appearance has been modified");
	}
}

/**
 * @param model_id
 */
void Client::SetSecondaryWeaponOrnamentation(uint32 model_id)
{
	auto secondary_item = m_inv.GetItem(EQ::invslot::slotSecondary);
	if (secondary_item) {
		database.QueryDatabase(
			StringFormat(
				"UPDATE `inventory` SET `ornamentidfile` = %i WHERE `charid` = %i AND `slotid` = %i",
				model_id,
				character_id,
				EQ::invslot::slotSecondary
			));

		secondary_item->SetOrnamentationIDFile(model_id);
		SendItemPacket(EQ::invslot::slotSecondary, secondary_item, ItemPacketTrade);
		WearChange(EQ::textures::weaponSecondary, static_cast<uint16>(model_id), 0);

		Message(Chat::Yellow, "Your secondary weapon appearance has been modified");
	}
}

/**
 * Used in #goto <player_name>
 *
 * @param player_name
 */
bool Client::GotoPlayer(std::string player_name)
{
	std::string query = StringFormat(
		"SELECT"
		"    character_data.zone_id,"
		"    character_data.zone_instance,"
		"    character_data.x,"
		"    character_data.y,"
		"    character_data.z,"
		"    character_data.heading "
		"FROM"
		"    character_data "
		"WHERE"
		"    TRUE"
		"    AND character_data.name = '%s'"
		"    AND character_data.last_login > (UNIX_TIMESTAMP() - 600) LIMIT 1", player_name.c_str());

	auto results = database.QueryDatabase(query);
	if (!results.Success()) {
		return false;
	}

	for (auto row = results.begin(); row != results.end(); ++row) {
		auto zone_id     = static_cast<uint32>(atoi(row[0]));
		auto instance_id = static_cast<uint16>(atoi(row[1]));
		auto x           = static_cast<float>(atof(row[2]));
		auto y           = static_cast<float>(atof(row[3]));
		auto z           = static_cast<float>(atof(row[4]));
		auto heading     = static_cast<float>(atof(row[5]));

		if (instance_id > 0 && !database.CheckInstanceExists(instance_id)) {
			this->Message(Chat::Yellow, "Instance no longer exists...");
			return false;
		}

		if (instance_id > 0) {
			database.AddClientToInstance(instance_id, this->CharacterID());
		}

		this->MovePC(zone_id, instance_id, x, y, z, heading);

		return true;
	}

	return false;
}

glm::vec4 &Client::GetLastPositionBeforeBulkUpdate()
{
	return last_position_before_bulk_update;
}

/**
 * @param in_last_position_before_bulk_update
 */
void Client::SetLastPositionBeforeBulkUpdate(glm::vec4 in_last_position_before_bulk_update)
{
	Client::last_position_before_bulk_update = in_last_position_before_bulk_update;
}

#ifdef BOTS

bool Client::GetBotOption(BotOwnerOption boo) const {

	if (boo < _booCount) {
		return bot_owner_options[boo];
	}

	return false;
}

void Client::SetBotOption(BotOwnerOption boo, bool flag) {

	if (boo < _booCount) {
		bot_owner_options[boo] = flag;
	}
}

#endif

void Client::SendToGuildHall()
{
	std::string zone_short_name = "guildhall";
	uint32      zone_id         = ZoneID(zone_short_name.c_str());
	if (zone_id == 0) {
		return;
	}

	uint32      expiration_time         = (RuleI(Instances, GuildHallExpirationDays) * 86400);
	uint16      instance_id             = 0;
	std::string guild_hall_instance_key = fmt::format("guild-hall-instance-{}", GuildID());
	std::string instance_data           = DataBucket::GetData(guild_hall_instance_key);
	if (!instance_data.empty() && std::stoi(instance_data) > 0) {
		instance_id = std::stoi(instance_data);
	}

	if (instance_id <= 0) {
		if (!database.GetUnusedInstanceID(instance_id)) {
			Message(Chat::Red, "Server was unable to find a free instance id.");
			return;
		}

		if (!database.CreateInstance(instance_id, zone_id, 1, expiration_time)) {
			Message(Chat::Red, "Server was unable to create a new instance.");
			return;
		}

		DataBucket::SetData(
			guild_hall_instance_key,
			std::to_string(instance_id),
			std::to_string(expiration_time)
		);
	}

	AssignToInstance(instance_id);
	MovePC(345, instance_id, -1.00, -1.00, 3.34, 0, 1);
}

void Client::CheckVirtualZoneLines()
{
	for (auto &virtual_zone_point : zone->virtual_zone_point_list) {
		float half_width = ((float) virtual_zone_point.width / 2);

		if (
			GetX() > (virtual_zone_point.x - half_width) &&
			GetX() < (virtual_zone_point.x + half_width) &&
			GetY() > (virtual_zone_point.y - half_width) &&
			GetY() < (virtual_zone_point.y + half_width) &&
			GetZ() >= (virtual_zone_point.z - 10) &&
			GetZ() < (virtual_zone_point.z + (float) virtual_zone_point.height)
			) {

			MovePC(
				virtual_zone_point.target_zone_id,
				virtual_zone_point.target_instance,
				virtual_zone_point.target_x,
				virtual_zone_point.target_y,
				virtual_zone_point.target_z,
				virtual_zone_point.target_heading
			);

			LogZonePoints(
				"Virtual Zone Box Sending player [{}] to [{}]",
				GetCleanName(),
				zone_store.GetZoneLongName(virtual_zone_point.target_zone_id)
			);
		}
	}
}

void Client::ShowDevToolsMenu()
{
	std::string menu_commands_search;
	std::string menu_commands_show;
	std::string reload_commands_show;
	std::string devtools_toggle;

	/**
	 * Search entity commands
	 */
	menu_commands_search += "[" + EQ::SayLinkEngine::GenerateQuestSaylink("#list npcs", false, "NPC") + "] ";
	menu_commands_search += "[" + EQ::SayLinkEngine::GenerateQuestSaylink("#list players", false, "Players") + "] ";
	menu_commands_search += "[" + EQ::SayLinkEngine::GenerateQuestSaylink("#list corpses", false, "Corpses") + "] ";
	menu_commands_search += "[" + EQ::SayLinkEngine::GenerateQuestSaylink("#list doors", false, "Doors") + "] ";
	menu_commands_search += "[" + EQ::SayLinkEngine::GenerateQuestSaylink("#list objects", false, "Objects") + "] ";
	menu_commands_search += "[" + EQ::SayLinkEngine::GenerateQuestSaylink("#fz", false, "Zones") + "] ";
	menu_commands_search += "[" + EQ::SayLinkEngine::GenerateQuestSaylink("#fi", false, "Items") + "] ";

	/**
	 * Show
	 */
	menu_commands_show += "[" + EQ::SayLinkEngine::GenerateQuestSaylink("#showzonepoints", false, "Zone Points") + "] ";
	menu_commands_show += "[" + EQ::SayLinkEngine::GenerateQuestSaylink("#showzonegloballoot", false, "Zone Global Loot") + "] ";

	/**
	 * Reload
	 */
	reload_commands_show += "[" + EQ::SayLinkEngine::GenerateQuestSaylink("#rq", false, "Quests") + "] ";
	reload_commands_show += "[" + EQ::SayLinkEngine::GenerateQuestSaylink("#reloadmerchants", false, "Merchants") + "] ";
	reload_commands_show += "[" + EQ::SayLinkEngine::GenerateQuestSaylink("#reloadallrules", false, "Rules Globally") + "] ";
	reload_commands_show += "[" + EQ::SayLinkEngine::GenerateQuestSaylink("#reloadstatic", false, "Ground Spawns") + "] ";
	reload_commands_show += "[" + EQ::SayLinkEngine::GenerateQuestSaylink("#reloadstatic", false, "Alternate Currencies") + "] ";
	reload_commands_show += "[" + EQ::SayLinkEngine::GenerateQuestSaylink("#reloadstatic", false, "DB Emotes") + "] ";
	reload_commands_show += "[" + EQ::SayLinkEngine::GenerateQuestSaylink("#reloadstatic", false, "Doors") + "] ";
	reload_commands_show += "[" + EQ::SayLinkEngine::GenerateQuestSaylink("#reloadtraps", false, "Traps") + "] ";
	reload_commands_show += "[" + EQ::SayLinkEngine::GenerateQuestSaylink("#reloadzps", false, "Zone Points") + "] ";

	/**
	 * Show window status
	 */
	devtools_toggle = "Disabled [" + EQ::SayLinkEngine::GenerateQuestSaylink("#devtools enable", false, "Enable") + "] ";
	if (IsDevToolsEnabled()) {
		devtools_toggle = "Enabled [" + EQ::SayLinkEngine::GenerateQuestSaylink("#devtools disable", false, "Disable") + "] ";
	}

	/**
	 * Print menu
	 */
	SendChatLineBreak();
	Message(
		Chat::White, "| [Devtools] %s Show this menu with %s | Current expansion [%s]",
		devtools_toggle.c_str(),
		EQ::SayLinkEngine::GenerateQuestSaylink("#dev", false, "#dev").c_str(),
		content_service.GetCurrentExpansionName().c_str()
	);
	Message(Chat::White, "| [Devtools] Search %s", menu_commands_search.c_str());
	Message(Chat::White, "| [Devtools] Show %s", menu_commands_show.c_str());
	Message(Chat::White, "| [Devtools] Reload %s", reload_commands_show.c_str());
	Message(Chat::White, "| [Devtools] Search commands with #help <search>");
	SendChatLineBreak();
}

void Client::SendChatLineBreak(uint16 color) {
	Message(color, "------------------------------------------------");
}

void Client::SendCrossZoneMessage(
	Client* client, const std::string& character_name, uint16_t chat_type, const std::string& message)
{
	// if client is null, falls back to sending a cross zone message by name
	if (!client && !character_name.empty())
	{
		client = entity_list.GetClientByName(character_name.c_str());
	}

	if (client)
	{
		client->Message(chat_type, message.c_str());
	}
	else if (!character_name.empty() && !message.empty())
	{
		uint32_t pack_size = sizeof(CZMessagePlayer_Struct);
		auto pack = std::make_unique<ServerPacket>(ServerOP_CZMessagePlayer, pack_size);
		auto buf = reinterpret_cast<CZMessagePlayer_Struct*>(pack->pBuffer);
		buf->type = chat_type;
		strn0cpy(buf->character_name, character_name.c_str(), sizeof(buf->character_name));
		strn0cpy(buf->message, message.c_str(), sizeof(buf->message));

		worldserver.SendPacket(pack.get());
	}
}

void Client::SendCrossZoneMessageString(
	Client* client, const std::string& character_name, uint16_t chat_type,
	uint32_t string_id, const std::initializer_list<std::string>& arguments)
{
	// if client is null, falls back to sending a cross zone message by name
	if (!client && !character_name.empty()) // double check client isn't in this zone
	{
		client = entity_list.GetClientByName(character_name.c_str());
	}

	if (!client && character_name.empty())
	{
		return;
	}

	SerializeBuffer argument_buffer;
	for (const auto& argument : arguments)
	{
		argument_buffer.WriteString(argument);
	}

	uint32_t args_size = static_cast<uint32_t>(argument_buffer.size());
	uint32_t pack_size = sizeof(CZClientMessageString_Struct) + args_size;
	auto pack = std::make_unique<ServerPacket>(ServerOP_CZClientMessageString, pack_size);
	auto buf = reinterpret_cast<CZClientMessageString_Struct*>(pack->pBuffer);
	buf->string_id = string_id;
	buf->chat_type = chat_type;
	strn0cpy(buf->character_name, character_name.c_str(), sizeof(buf->character_name));
	buf->args_size = args_size;
	memcpy(buf->args, argument_buffer.buffer(), argument_buffer.size());

	if (client)
	{
		client->MessageString(buf);
	}
	else
	{
		worldserver.SendPacket(pack.get());
	}
}

void Client::UpdateExpeditionInfoAndLockouts()
{
	// this is processed by client after entering a zone
	SendDzCompassUpdate();

	m_expedition_lockouts = ExpeditionDatabase::LoadCharacterLockouts(CharacterID());

	auto expedition = GetExpedition();
	if (expedition)
	{
		expedition->SendClientExpeditionInfo(this);

		// live synchronizes lockouts obtained during the active expedition to
		// members once they zone into the expedition's dynamic zone instance
		if (expedition->GetDynamicZone().IsCurrentZoneDzInstance())
		{
			expedition->SyncCharacterLockouts(CharacterID(), m_expedition_lockouts);
			expedition->SetMemberStatus(this, DynamicZoneMemberStatus::InDynamicZone);
		}
		else
		{
			expedition->SetMemberStatus(this, DynamicZoneMemberStatus::Online);
		}
	}

	SendExpeditionLockoutTimers();

	// ask world for any pending invite we saved from a previous zone
	RequestPendingExpeditionInvite();
}

Expedition* Client::CreateExpedition(DynamicZone& dz_instance, ExpeditionRequest& request)
{
	return Expedition::TryCreate(this, dz_instance, request);
}

Expedition* Client::CreateExpedition(
	const std::string& zone_name, uint32 version, uint32 duration, const std::string& expedition_name,
	uint32 min_players, uint32 max_players, bool disable_messages)
{
	DynamicZone dz_instance{ ZoneID(zone_name), version, duration, DynamicZoneType::Expedition };
	ExpeditionRequest request{ expedition_name, min_players, max_players, disable_messages };
	return Expedition::TryCreate(this, dz_instance, request);
}

Expedition* Client::GetExpedition() const
{
	if (zone && m_expedition_id)
	{
		auto expedition_cache_iter = zone->expedition_cache.find(m_expedition_id);
		if (expedition_cache_iter != zone->expedition_cache.end())
		{
			return expedition_cache_iter->second.get();
		}
	}
	return nullptr;
}

void Client::AddExpeditionLockout(const ExpeditionLockoutTimer& lockout, bool update_db)
{
	// todo: support for account based lockouts like live AoC expeditions

	// if client already has this lockout, we're replacing it with the new one
	m_expedition_lockouts.erase(std::remove_if(m_expedition_lockouts.begin(), m_expedition_lockouts.end(),
		[&](const ExpeditionLockoutTimer& existing_lockout) {
			return existing_lockout.IsSameLockout(lockout);
		}
	), m_expedition_lockouts.end());

	m_expedition_lockouts.emplace_back(lockout);

	if (update_db) // for quest api
	{
		ExpeditionDatabase::InsertCharacterLockouts(CharacterID(), { lockout });
	}

	SendExpeditionLockoutTimers();
}

void Client::AddNewExpeditionLockout(
	const std::string& expedition_name, const std::string& event_name, uint32_t seconds, std::string uuid)
{
	auto lockout = ExpeditionLockoutTimer::CreateLockout(expedition_name, event_name, seconds, uuid);
	AddExpeditionLockout(lockout, true);
}

void Client::AddExpeditionLockoutDuration(
	const std::string& expedition_name, const std::string& event_name, int seconds,
	const std::string& uuid, bool update_db)
{
	auto it = std::find_if(m_expedition_lockouts.begin(), m_expedition_lockouts.end(),
		[&](const ExpeditionLockoutTimer& lockout) {
			return lockout.IsSameLockout(expedition_name, event_name);
		});

	if (it != m_expedition_lockouts.end())
	{
		it->AddLockoutTime(seconds);

		if (!uuid.empty())
		{
			it->SetUUID(uuid);
		}

		if (update_db)
		{
			ExpeditionDatabase::InsertCharacterLockouts(CharacterID(), { *it });
		}

		SendExpeditionLockoutTimers();
	}
	else if (seconds > 0) // missing lockouts inserted for reductions would be instantly expired
	{
		auto lockout = ExpeditionLockoutTimer::CreateLockout(expedition_name, event_name, seconds, uuid);
		AddExpeditionLockout(lockout, update_db);
	}
}

void Client::RemoveExpeditionLockout(
	const std::string& expedition_name, const std::string& event_name, bool update_db)
{
	m_expedition_lockouts.erase(std::remove_if(m_expedition_lockouts.begin(), m_expedition_lockouts.end(),
		[&](const ExpeditionLockoutTimer& lockout) {
			return lockout.IsSameLockout(expedition_name, event_name);
		}
	), m_expedition_lockouts.end());

	if (update_db) // for quest api
	{
		ExpeditionDatabase::DeleteCharacterLockout(CharacterID(), expedition_name, event_name);
	}

	SendExpeditionLockoutTimers();
}

void Client::RemoveAllExpeditionLockouts(const std::string& expedition_name, bool update_db)
{
	if (expedition_name.empty())
	{
		if (update_db)
		{
			ExpeditionDatabase::DeleteAllCharacterLockouts(CharacterID());
		}
		m_expedition_lockouts.clear();
	}
	else
	{
		if (update_db)
		{
			ExpeditionDatabase::DeleteAllCharacterLockouts(CharacterID(), expedition_name);
		}

		m_expedition_lockouts.erase(std::remove_if(m_expedition_lockouts.begin(), m_expedition_lockouts.end(),
			[&](const ExpeditionLockoutTimer& lockout) {
				return lockout.GetExpeditionName() == expedition_name;
			}
		), m_expedition_lockouts.end());
	}

	SendExpeditionLockoutTimers();
}

const ExpeditionLockoutTimer* Client::GetExpeditionLockout(
	const std::string& expedition_name, const std::string& event_name, bool include_expired) const
{
	for (const auto& expedition_lockout : m_expedition_lockouts)
	{
		if ((include_expired || !expedition_lockout.IsExpired()) &&
		    expedition_lockout.IsSameLockout(expedition_name, event_name))
		{
			return &expedition_lockout;
		}
	}
	return nullptr;
}

std::vector<ExpeditionLockoutTimer> Client::GetExpeditionLockouts(
	const std::string& expedition_name, bool include_expired)
{
	std::vector<ExpeditionLockoutTimer> lockouts;
	for (const auto& lockout : m_expedition_lockouts)
	{
		if ((include_expired || !lockout.IsExpired()) &&
		    lockout.GetExpeditionName() == expedition_name)
		{
			lockouts.emplace_back(lockout);
		}
	}
	return lockouts;
}

bool Client::HasExpeditionLockout(
	const std::string& expedition_name, const std::string& event_name, bool include_expired)
{
	return (GetExpeditionLockout(expedition_name, event_name, include_expired) != nullptr);
}

void Client::SendExpeditionLockoutTimers()
{
	std::vector<ExpeditionLockoutTimerEntry_Struct> lockout_entries;

	// client displays lockouts rounded down to nearest minute, send lockouts
	// with 60s offset added to compensate (live does this too)
	constexpr uint32_t rounding_seconds = 60;

	// erases expired lockouts while building lockout timer list
	for (auto it = m_expedition_lockouts.begin(); it != m_expedition_lockouts.end();)
	{
		uint32_t seconds_remaining = it->GetSecondsRemaining();
		if (seconds_remaining == 0)
		{
			it = m_expedition_lockouts.erase(it);
		}
		else
		{
			ExpeditionLockoutTimerEntry_Struct lockout;
			strn0cpy(lockout.expedition_name, it->GetExpeditionName().c_str(), sizeof(lockout.expedition_name));
			lockout.seconds_remaining = seconds_remaining + rounding_seconds;
			lockout.event_type = it->IsReplayTimer() ? Expedition::REPLAY_TIMER_ID : Expedition::EVENT_TIMER_ID;
			strn0cpy(lockout.event_name, it->GetEventName().c_str(), sizeof(lockout.event_name));

			lockout_entries.emplace_back(lockout);
			++it;
		}
	}

	uint32_t lockout_count = static_cast<uint32_t>(lockout_entries.size());
	uint32_t lockout_entries_size = sizeof(ExpeditionLockoutTimerEntry_Struct) * lockout_count;
	uint32_t outsize = sizeof(ExpeditionLockoutTimers_Struct) + lockout_entries_size;
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzExpeditionLockoutTimers, outsize);
	auto outbuf = reinterpret_cast<ExpeditionLockoutTimers_Struct*>(outapp->pBuffer);
	outbuf->count = lockout_count;
	if (!lockout_entries.empty())
	{
		memcpy(outbuf->timers, lockout_entries.data(), lockout_entries_size);
	}
	QueuePacket(outapp.get());
}

void Client::RequestPendingExpeditionInvite()
{
	uint32_t packsize = sizeof(ServerExpeditionCharacterID_Struct);
	auto pack = std::make_unique<ServerPacket>(ServerOP_ExpeditionRequestInvite, packsize);
	auto packbuf = reinterpret_cast<ServerExpeditionCharacterID_Struct*>(pack->pBuffer);
	packbuf->character_id = CharacterID();
	worldserver.SendPacket(pack.get());
}

void Client::DzListTimers()
{
	// only lists player's current replay timer lockouts, not all event lockouts
	bool found = false;
	for (const auto& lockout : m_expedition_lockouts)
	{
		if (lockout.IsReplayTimer())
		{
			found = true;
			auto time_remaining = lockout.GetDaysHoursMinutesRemaining();
			MessageString(
				Chat::Yellow, DZLIST_REPLAY_TIMER,
				time_remaining.days.c_str(),
				time_remaining.hours.c_str(),
				time_remaining.mins.c_str(),
				lockout.GetExpeditionName().c_str()
			);
		}
	}

	if (!found)
	{
		MessageString(Chat::Yellow, EXPEDITION_NO_TIMERS);
	}
}

void Client::SetDzRemovalTimer(bool enable_timer)
{
	uint32_t timer_ms = RuleI(DynamicZone, ClientRemovalDelayMS);

	LogDynamicZones(
		"Character [{}] instance [{}] removal timer enabled: [{}] delay (ms): [{}]",
		CharacterID(), zone ? zone->GetInstanceID() : 0, enable_timer, timer_ms
	);

	if (enable_timer)
	{
		dynamiczone_removal_timer.Start(timer_ms);
	}
	else
	{
		dynamiczone_removal_timer.Disable();
	}
}

void Client::SendDzCompassUpdate()
{
	// client may be associated with multiple dynamic zone compasses in this zone
	std::vector<DynamicZoneCompassEntry_Struct> compass_entries;

	for (const auto& client_dz : GetDynamicZones())
	{
		auto compass = client_dz->GetCompassLocation();
		if (zone && zone->IsZone(compass.zone_id, 0))
		{
			DynamicZoneCompassEntry_Struct entry;
			entry.dz_zone_id = client_dz->GetZoneID();
			entry.dz_instance_id = client_dz->GetInstanceID();
			entry.dz_type = static_cast<uint32_t>(client_dz->GetType());
			entry.x = compass.x;
			entry.y = compass.y;
			entry.z = compass.z;

			compass_entries.emplace_back(entry);
		}
	}

	// compass set via MarkSingleCompassLocation()
	if (m_has_quest_compass)
	{
		DynamicZoneCompassEntry_Struct entry;
		entry.dz_zone_id = 0;
		entry.dz_instance_id = 0;
		entry.dz_type = 0;
		entry.x = m_quest_compass.x;
		entry.y = m_quest_compass.y;
		entry.z = m_quest_compass.z;

		compass_entries.emplace_back(entry);
	}

	QueuePacket(CreateCompassPacket(compass_entries).get());
}

std::unique_ptr<EQApplicationPacket> Client::CreateCompassPacket(
	const std::vector<DynamicZoneCompassEntry_Struct>& compass_entries)
{
	uint32 count = static_cast<uint32_t>(compass_entries.size());
	uint32 entries_size = sizeof(DynamicZoneCompassEntry_Struct) * count;
	uint32 outsize = sizeof(DynamicZoneCompass_Struct) + entries_size;
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzCompass, outsize);
	auto outbuf = reinterpret_cast<DynamicZoneCompass_Struct*>(outapp->pBuffer);
	outbuf->count = count;
	memcpy(outbuf->entries, compass_entries.data(), entries_size);

	return outapp;
}

void Client::GoToDzSafeReturnOrBind(const DynamicZone* dynamic_zone)
{
	if (dynamic_zone)
	{
		auto safereturn = dynamic_zone->GetSafeReturnLocation();
		if (safereturn.zone_id != 0)
		{
			LogDynamicZonesDetail("Sending [{}] to safereturn zone [{}]", CharacterID(), safereturn.zone_id);
			MovePC(safereturn.zone_id, 0, safereturn.x, safereturn.y, safereturn.z, safereturn.heading);
			return;
		}
	}

	GoToBind();
}

std::vector<DynamicZone*> Client::GetDynamicZones(uint32_t zone_id, int zone_version)
{
	std::vector<DynamicZone*> client_dzs;

	// check client systems for any associated dynamic zones optionally filtered by zone
	Expedition* expedition = GetExpedition();
	if (expedition &&
	   (zone_id == 0 || expedition->GetDynamicZone().GetZoneID() == zone_id) &&
	   (zone_version < 0 || expedition->GetDynamicZone().GetZoneVersion() == zone_version))
	{
		client_dzs.emplace_back(&expedition->GetDynamicZone());
	}

	// todo: tasks, missions (shared tasks), and quests with an associated dz to zone_id

	return client_dzs;
}

void Client::MovePCDynamicZone(uint32 zone_id, int zone_version, bool msg_if_invalid)
{
	if (zone_id == 0)
	{
		return;
	}

	auto client_dzs = GetDynamicZones(zone_id, zone_version);

	if (client_dzs.empty())
	{
		if (msg_if_invalid)
		{
			MessageString(Chat::Red, DYNAMICZONE_WAY_IS_BLOCKED); // unconfirmed message
		}
	}
	else if (client_dzs.size() == 1)
	{
		auto dz = client_dzs.front();
		DynamicZoneLocation zonein = dz->GetZoneInLocation();
		ZoneMode zone_mode = dz->HasZoneInLocation() ? ZoneMode::ZoneSolicited : ZoneMode::ZoneToSafeCoords;
		MovePC(zone_id, dz->GetInstanceID(), zonein.x, zonein.y, zonein.z, zonein.heading, 0, zone_mode);
	}
	else
	{
		LogDynamicZonesDetail("Sending DzSwitchListWnd to [{}] for zone [{}] with [{}] dynamic zone(s)",
			CharacterID(), zone_id, client_dzs.size());

		// client has more than one dz for this zone, send out the switchlist window
		QueuePacket(CreateDzSwitchListPacket(client_dzs).get());
	}
}

std::unique_ptr<EQApplicationPacket> Client::CreateDzSwitchListPacket(
	const std::vector<DynamicZone*>& client_dzs)
{
	uint32 count = static_cast<uint32_t>(client_dzs.size());
	uint32 entries_size = sizeof(DynamicZoneChooseZoneEntry_Struct) * count;
	uint32 outsize = sizeof(DynamicZoneChooseZone_Struct) + entries_size;
	auto outapp = std::make_unique<EQApplicationPacket>(OP_DzChooseZone, outsize);
	auto outbuf = reinterpret_cast<DynamicZoneChooseZone_Struct*>(outapp->pBuffer);
	outbuf->count = count;
	for (int i = 0; i < client_dzs.size(); ++i)
	{
		outbuf->choices[i].dz_zone_id = client_dzs[i]->GetZoneID();
		outbuf->choices[i].dz_instance_id = client_dzs[i]->GetInstanceID();
		outbuf->choices[i].dz_type = static_cast<uint32_t>(client_dzs[i]->GetType());
		strn0cpy(outbuf->choices[i].description, client_dzs[i]->GetName().c_str(), sizeof(outbuf->choices[i].description));
		strn0cpy(outbuf->choices[i].leader_name, client_dzs[i]->GetLeaderName().c_str(), sizeof(outbuf->choices[i].leader_name));
	}
	return outapp;
}

void Client::MovePCDynamicZone(const std::string& zone_name, int zone_version, bool msg_if_invalid)
{
	auto zone_id = ZoneID(zone_name.c_str());
	MovePCDynamicZone(zone_id, zone_version, msg_if_invalid);
}

void Client::Fling(float value, float target_x, float target_y, float target_z, bool ignore_los, bool clipping) {	
	BuffFadeByEffect(SE_Levitate);
	if (CheckLosFN(target_x, target_y, target_z, 6.0f) || ignore_los) {
		auto outapp_fling = new EQApplicationPacket(OP_Fling, sizeof(fling_struct));
		fling_struct* flingTo = (fling_struct*)outapp_fling->pBuffer;
		if(clipping)
			flingTo->collision = 0;
		else
			flingTo->collision = -1;
		
		flingTo->travel_time = -1;
		flingTo->unk3 = 1;
		flingTo->disable_fall_damage = 1;
		flingTo->speed_z = value;
		flingTo->new_y = target_y;
		flingTo->new_x = target_x;
		flingTo->new_z = target_z;
		outapp_fling->priority = 6;
		FastQueuePacket(&outapp_fling);
	}
}

std::vector<int> Client::GetLearnableDisciplines(uint8 min_level, uint8 max_level) {
	bool SpellGlobalRule = RuleB(Spells, EnableSpellGlobals);
	bool SpellBucketRule = RuleB(Spells, EnableSpellBuckets);
	bool SpellGlobalCheckResult = false;
	bool SpellBucketCheckResult = false;
	std::vector<int> learnable_disciplines;
	for (int spell_id = 0; spell_id < SPDAT_RECORDS; ++spell_id) {
		bool learnable = false;
		if (!IsValidSpell(spell_id))
			continue;
		if (!IsDiscipline(spell_id))
			continue;
		if (spells[spell_id].classes[WARRIOR] == 0)
			continue;
		if (max_level > 0 && spells[spell_id].classes[m_pp.class_ - 1] > max_level)
			continue;
		if (min_level > 1 && spells[spell_id].classes[m_pp.class_ - 1] < min_level)
			continue;
		if (spells[spell_id].skill == 52)
			continue;
		if (RuleB(Spells, UseCHAScribeHack) && spells[spell_id].effectid[EFFECT_COUNT - 1] == 10)
			continue;
		if (HasDisciplineLearned(spell_id))
			continue;

		if (SpellGlobalRule) {
			SpellGlobalCheckResult = SpellGlobalCheck(spell_id, CharacterID());
			if (SpellGlobalCheckResult) {
				learnable = true;
			}
		} else if (SpellBucketRule) {
			SpellBucketCheckResult = SpellBucketCheck(spell_id, CharacterID());
			if (SpellBucketCheckResult) {
				learnable = true;
			}
		} else {
			learnable = true;
		}

		if (learnable) {
			learnable_disciplines.push_back(spell_id);
		}
	}		
	return learnable_disciplines;
}

std::vector<int> Client::GetLearnedDisciplines() {
	std::vector<int> learned_disciplines;
	for (int index = 0; index < MAX_PP_DISCIPLINES; index++) {
		if (IsValidSpell(m_pp.disciplines.values[index])) {
			learned_disciplines.push_back(m_pp.disciplines.values[index]);
		}
	}		
	return learned_disciplines;
}

std::vector<int> Client::GetMemmedSpells() {
	std::vector<int> memmed_spells;
	for (int index = 0; index < EQ::spells::SPELL_GEM_COUNT; index++) {
		if (IsValidSpell(m_pp.mem_spells[index])) {
			memmed_spells.push_back(m_pp.mem_spells[index]);
		}
	}		
	return memmed_spells;
}

std::vector<int> Client::GetScribeableSpells(uint8 min_level, uint8 max_level) {
	bool SpellGlobalRule = RuleB(Spells, EnableSpellGlobals);
	bool SpellBucketRule = RuleB(Spells, EnableSpellBuckets);
	bool SpellGlobalCheckResult = false;
	bool SpellBucketCheckResult = false;
	std::vector<int> scribeable_spells;
	for (int spell_id = 0; spell_id < SPDAT_RECORDS; ++spell_id) {
		bool scribeable = false;
		if (!IsValidSpell(spell_id))
			continue;
		if (IsDiscipline(spell_id))
			continue;
		if (spells[spell_id].classes[WARRIOR] == 0)
			continue;
		if (max_level > 0 && spells[spell_id].classes[m_pp.class_ - 1] > max_level)
			continue;
		if (min_level > 1 && spells[spell_id].classes[m_pp.class_ - 1] < min_level)
			continue;
		if (spells[spell_id].skill == 52)
			continue;
		if (RuleB(Spells, UseCHAScribeHack) && spells[spell_id].effectid[EFFECT_COUNT - 1] == 10)
			continue;
		if (HasSpellScribed(spell_id))
			continue;

		if (SpellGlobalRule) {
			SpellGlobalCheckResult = SpellGlobalCheck(spell_id, CharacterID());
			if (SpellGlobalCheckResult) {
				scribeable = true;
			}
		} else if (SpellBucketRule) {
			SpellBucketCheckResult = SpellBucketCheck(spell_id, CharacterID());
			if (SpellBucketCheckResult) {
				scribeable = true;
			}
		} else {
			scribeable = true;
		}

		if (scribeable) {
			scribeable_spells.push_back(spell_id);
		}
	}		
	return scribeable_spells;
}

std::vector<int> Client::GetScribedSpells() {
	std::vector<int> scribed_spells;
	for(int index = 0; index < EQ::spells::SPELLBOOK_SIZE; index++) {
		if (IsValidSpell(m_pp.spell_book[index])) {
			scribed_spells.push_back(m_pp.spell_book[index]);
		}
	}
	return scribed_spells;
}

void Client::SetAnon(uint8 anon_flag) {
	m_pp.anon = anon_flag;
	auto outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
	SpawnAppearance_Struct* spawn_appearance = (SpawnAppearance_Struct*)outapp->pBuffer;
	spawn_appearance->spawn_id = this->GetID();
	spawn_appearance->type = AT_Anon;
	spawn_appearance->parameter = anon_flag;
	entity_list.QueueClients(this, outapp);
	Save();
	UpdateWho();
	safe_delete(outapp);
}

void Client::SetAFK(uint8 afk_flag) {
	AFK = afk_flag;
	auto outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
	SpawnAppearance_Struct* spawn_appearance = (SpawnAppearance_Struct*)outapp->pBuffer;
	spawn_appearance->spawn_id = this->GetID();
	spawn_appearance->type = AT_AFK;
	spawn_appearance->parameter = afk_flag;
	entity_list.QueueClients(this, outapp);
	safe_delete(outapp);
}


void Client::SendToInstance(std::string instance_type, std::string zone_short_name, uint32 instance_version, float x, float y, float z, float heading, std::string instance_identifier, uint32 duration) {
	uint32 zone_id = ZoneID(zone_short_name);
	std::string current_instance_type = str_tolower(instance_type);
	std::string instance_type_name = "public";
	if (current_instance_type.find("solo") != std::string::npos) {
		instance_type_name = GetCleanName();
	} else if (current_instance_type.find("group") != std::string::npos) {
		uint32 group_id = (GetGroup() ? GetGroup()->GetID() : 0);
		instance_type_name = itoa(group_id);
	} else if (current_instance_type.find("raid") != std::string::npos) {
		uint32 raid_id = (GetRaid() ? GetRaid()->GetID() : 0);
		instance_type_name = itoa(raid_id);
	} else if (current_instance_type.find("guild") != std::string::npos) {
		uint32 guild_id = (GuildID() > 0 ? GuildID() : 0);
		instance_type_name = itoa(guild_id);
	}

	std::string full_bucket_name = fmt::format(
		"{}_{}_{}_{}",
		current_instance_type,
		instance_type_name,
		instance_identifier,
		zone_short_name
	);
	std::string current_bucket_value = DataBucket::GetData(full_bucket_name);
	uint16 instance_id = 0;

	if (current_bucket_value.length() > 0) {
		instance_id = atoi(current_bucket_value.c_str());
	} else {
		if(!database.GetUnusedInstanceID(instance_id)) {
			Message(Chat::White, "Server was unable to find a free instance id.");
			return;
		}

		if(!database.CreateInstance(instance_id, zone_id, instance_version, duration)) {
			Message(Chat::White, "Server was unable to create a new instance.");
			return;
		}

		DataBucket::SetData(full_bucket_name, itoa(instance_id), itoa(duration));		
	}

	AssignToInstance(instance_id);
	MovePC(zone_id, instance_id, x, y, z, heading);
}

int Client::CountItem(uint32 item_id)
{
	int quantity = 0;
	EQ::ItemInstance *item = nullptr;
	static const int16 slots[][2] = {
		{ EQ::invslot::POSSESSIONS_BEGIN, EQ::invslot::POSSESSIONS_END },
		{ EQ::invbag::GENERAL_BAGS_BEGIN, EQ::invbag::GENERAL_BAGS_END },
		{ EQ::invbag::CURSOR_BAG_BEGIN, EQ::invbag::CURSOR_BAG_END},
		{ EQ::invslot::BANK_BEGIN, EQ::invslot::BANK_END },
		{ EQ::invbag::BANK_BAGS_BEGIN, EQ::invbag::BANK_BAGS_END },
		{ EQ::invslot::SHARED_BANK_BEGIN, EQ::invslot::SHARED_BANK_END },
		{ EQ::invbag::SHARED_BANK_BAGS_BEGIN, EQ::invbag::SHARED_BANK_BAGS_END },
	};
	const size_t size = sizeof(slots) / sizeof(slots[0]);
	for (int slot_index = 0; slot_index < size; ++slot_index) {
		for (int slot_id = slots[slot_index][0]; slot_id <= slots[slot_index][1]; ++slot_id) {
			item = GetInv().GetItem(slot_id);
			if (item && item->GetID() == item_id) {
				quantity += (item->IsStackable() ? item->GetCharges() : 1);
			}
		}
	}

	return quantity;
}

void Client::RemoveItem(uint32 item_id, uint32 quantity)
{
	EQ::ItemInstance *item = nullptr;
	static const int16 slots[][2] = {
		{ EQ::invslot::POSSESSIONS_BEGIN, EQ::invslot::POSSESSIONS_END },
		{ EQ::invbag::GENERAL_BAGS_BEGIN, EQ::invbag::GENERAL_BAGS_END },
		{ EQ::invbag::CURSOR_BAG_BEGIN, EQ::invbag::CURSOR_BAG_END},
		{ EQ::invslot::BANK_BEGIN, EQ::invslot::BANK_END },
		{ EQ::invbag::BANK_BAGS_BEGIN, EQ::invbag::BANK_BAGS_END },
		{ EQ::invslot::SHARED_BANK_BEGIN, EQ::invslot::SHARED_BANK_END },
		{ EQ::invbag::SHARED_BANK_BAGS_BEGIN, EQ::invbag::SHARED_BANK_BAGS_END },
	};
	int removed_count = 0;
	const size_t size = sizeof(slots) / sizeof(slots[0]);
	for (int slot_index = 0; slot_index < size; ++slot_index) {		
		for (int slot_id = slots[slot_index][0]; slot_id <= slots[slot_index][1]; ++slot_id) {
			if (removed_count == quantity) {
				break;
			}

			item = GetInv().GetItem(slot_id);
			if (item && item->GetID() == item_id) {
				int stack_size = item->IsStackable() ? item->GetCharges() : 1;
				if ((removed_count + stack_size) <= quantity) {
					removed_count += stack_size;
					DeleteItemInInventory(slot_id, stack_size, true);
				} else {
					int amount_left = (quantity - removed_count);
					if (amount_left > 0 && stack_size >= amount_left) {
						removed_count += amount_left;
						DeleteItemInInventory(slot_id, amount_left, true);
					}
				}
			}
		}
	}
}
