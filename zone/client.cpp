/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2003 EQEMu Development Team (http://eqemulator.org)

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
#include "../common/debug.h"
#include <iostream>
#include <iomanip>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <math.h>

// for windows compile
#ifdef _WINDOWS
#define abs64 _abs64
#define snprintf	_snprintf
#if (_MSC_VER < 1500)
	#define vsnprintf	_vsnprintf
#endif
#define strncasecmp	_strnicmp
#define strcasecmp  _stricmp
#else
#include <stdarg.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../common/unix.h"
#define abs64 abs
#endif

extern volatile bool RunLoops;

#include "../common/features.h"
#include "masterentity.h"
#include "worldserver.h"
#include "../common/misc.h"
#include "zonedb.h"
#include "../common/spdat.h"
#include "net.h"
#include "../common/packet_dump.h"
#include "../common/packet_functions.h"
#include "petitions.h"
#include "../common/serverinfo.h"
#include "../common/ZoneNumbers.h"
#include "../common/moremath.h"
#include "../common/guilds.h"
#include "../common/breakdowns.h"
#include "../common/rulesys.h"
#include "../common/StringUtil.h"
#include "forage.h"
#include "command.h"
#include "StringIDs.h"
#include "NpcAI.h"
#include "client_logs.h"
#include "guild_mgr.h"
#include "QuestParserCollection.h"


extern EntityList entity_list;
extern Zone* zone;
extern volatile bool ZoneLoaded;
extern WorldServer worldserver;
extern uint32 numclients;
extern PetitionList petition_list;
bool commandlogged;
char entirecommand[255];
extern DBAsyncFinishedQueue MTdbafq;
extern DBAsync *dbasync;

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
	0,	// heading
	0,	// x
	0,	// y
	0,	// z
	0,	// light
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
	0,	// Armor Tint
	0xff,	// AA Title
	0,	// see_invis
	0,	// see_invis_undead
	0,
	0,
	0,
	0,
	0,	// qglobal
	0,	// maxlevel
	0	// scalerate

	),
	//these must be listed in the order they appear in client.h
	position_timer(250),
	hpupdate_timer(1800),
	camp_timer(29000),
	process_timer(100),
	stamina_timer(40000),
	zoneinpacket_timer(3000),
	linkdead_timer(RuleI(Zone,ClientLinkdeadMS)),
	dead_timer(2000),
	global_channel_timer(1000),
	shield_timer(500),
	fishing_timer(8000),
	endupkeep_timer(1000),
	forget_timer(0),
	autosave_timer(RuleI(Character, AutosaveIntervalS)*1000),
#ifdef REVERSE_AGGRO
	scanarea_timer(AIClientScanarea_delay),
#endif
	tribute_timer(Tribute_duration),
#ifdef PACKET_UPDATE_MANAGER
	update_manager(ieqs),
#endif
	proximity_timer(ClientProximity_interval),
	TaskPeriodic_Timer(RuleI(TaskSystem, PeriodicCheckTimer) * 1000),
	charm_update_timer(60000),
	rest_timer(1),
	charm_class_attacks_timer(3000),
	charm_cast_timer(3500),
	qglobal_purge_timer(30000),
	TrackingTimer(2000),
	RespawnFromHoverTimer(0),
	merc_timer(RuleI(Mercs, UpkeepIntervalMS)),
	ItemTickTimer(10000)
{
	for(int cf=0; cf < _FilterCount; cf++)
		ClientFilters[cf] = FilterShow;
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
	CustomerID = 0;
	TrackingID = 0;
	WID = 0;
	account_id = 0;
	admin = 0;
	lsaccountid = 0;
	shield_target = nullptr;
	SQL_log = nullptr;
	guild_id = GUILD_NONE;
	guildrank = 0;
	GuildBanker = false;
	memset(lskey, 0, sizeof(lskey));
	strcpy(account_name, "");
	tellsoff = false;
	last_reported_mana = 0;
	last_reported_endur = 0;
	gmhideme = false;
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
	linkdead_timer.Disable();
	zonesummon_x = -2;
	zonesummon_y = -2;
	zonesummon_z = -2;
	zonesummon_id = 0;
	zonesummon_ignorerestrictions = 0;
	zoning = false;
	zone_mode = ZoneUnsolicited;
	proximity_x = FLT_MAX;	//arbitrary large number
	proximity_y = FLT_MAX;
	proximity_z = FLT_MAX;
	casting_spell_id = 0;
	npcflag = false;
	npclevel = 0;
	pQueuedSaveWorkID = 0;
	position_timer_counter = 0;
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
	UpdateWindowTitle();
	horseId = 0;
	tgb = false;
	tribute_master_id = 0xFFFFFFFF;
	tribute_timer.Disable();
	taskstate = nullptr;
	TotalSecondsPlayed = 0;
	keyring.clear();
	bind_sight_target = nullptr;
	mercid = 0;
	mercSlot = 0;
	InitializeMercInfo();
	SetMerc(0);

	logging_enabled = CLIENT_DEFAULT_LOGGING_ENABLED;

	//for good measure:
	memset(&m_pp, 0, sizeof(m_pp));
	memset(&m_epp, 0, sizeof(m_epp));
	PendingTranslocate = false;
	PendingSacrifice = false;
	BoatID = 0;

	KarmaUpdateTimer = new Timer(RuleI(Chat, KarmaUpdateIntervalMS));
	GlobalChatLimiterTimer = new Timer(RuleI(Chat, IntervalDurationMS));
	AttemptedMessages = 0;
	TotalKarma = 0;
	ClientVersion = EQClientUnknown;
	ClientVersionBit = 0;
	AggroCount = 0;
	RestRegenHP = 0;
	RestRegenMana = 0;
	RestRegenEndurance = 0;
	XPRate = 100;
	cur_end = 0;

	m_TimeSinceLastPositionCheck = 0;
	m_DistanceSinceLastPositionCheck = 0.0f;
	m_ShadowStepExemption = 0;
	m_KnockBackExemption = 0;
	m_PortExemption = 0;
	m_SenseExemption = 0;
	m_CheatDetectMoved = false;
	CanUseReport = true;
	aa_los_me.x = 0;
	aa_los_me.y = 0;
	aa_los_me.z = 0;
	aa_los_them.x = 0;
	aa_los_them.y = 0;
	aa_los_them.z = 0;
	aa_los_them_mob = nullptr;
	los_status = false;
	qGlobals = nullptr;
	HideCorpseMode = HideCorpseNone;
	PendingGuildInvitation = false;

	cur_end = 0;

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
	}
	MaxXTargets = 5;
	XTargetAutoAddHaters = true;
	LoadAccountFlags();
}

Client::~Client() {
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
		LogFile->write(EQEMuLog::Debug, "Client '%s' was destroyed before reaching the connected state:", GetName());
		ReportConnectingState();
	}

	if(m_tradeskill_object != nullptr) {
		m_tradeskill_object->Close();
		m_tradeskill_object = nullptr;
	}

#ifdef CLIENT_LOGS
	client_logs.unsubscribeAll(this);
#endif


//	if(AbilityTimer || GetLevel()>=51)
//		database.UpdateAndDeleteAATimers(CharacterID());

	ChangeSQLLog(nullptr);
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
	if(isgrouped && !zoning && ZoneLoaded)
		LeaveGroup();

	UpdateWho(2);

	if(IsHoveringForRespawn())
	{
		m_pp.zone_id = m_pp.binds[0].zoneId;
		m_pp.zoneInstance = 0;
		x_pos = m_pp.binds[0].x;
		y_pos = m_pp.binds[0].y;
		z_pos = m_pp.binds[0].z;
	}

	// we save right now, because the client might be zoning and the world
	// will need this data right away
	Save(2); // This fails when database destructor is called first on shutdown

	safe_delete(taskstate);
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

	numclients--;
	UpdateWindowTitle();
	if(zone)
	zone->RemoveAuth(GetName());

	//let the stream factory know were done with this stream
	eqs->Close();
	eqs->ReleaseFromUse();

	entity_list.RemoveClient(this);
	UninitializeBuffSlots();
}

void Client::SendLogoutPackets() {

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_CancelTrade, sizeof(CancelTrade_Struct));
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
		LogFile->write(EQEMuLog::Debug, "Client has not sent us an initial zone entry packet.");
		break;
	case ReceivedZoneEntry:		//got the first packet, loading up PP
		LogFile->write(EQEMuLog::Debug, "Client sent initial zone packet, but we never got their player info from the database.");
		break;
	case PlayerProfileLoaded:	//our DB work is done, sending it
		LogFile->write(EQEMuLog::Debug, "We were sending the player profile, tributes, tasks, spawns, time and weather, but never finished.");
		break;
	case ZoneInfoSent:		//includes PP, tributes, tasks, spawns, time and weather
		LogFile->write(EQEMuLog::Debug, "We successfully sent player info and spawns, waiting for client to request new zone.");
		break;
	case NewZoneRequested:	//received and sent new zone request
		LogFile->write(EQEMuLog::Debug, "We received client's new zone request, waiting for client spawn request.");
		break;
	case ClientSpawnRequested:	//client sent ReqClientSpawn
		LogFile->write(EQEMuLog::Debug, "We received the client spawn request, and were sending objects, doors, zone points and some other stuff, but never finished.");
		break;
	case ZoneContentsSent:		//objects, doors, zone points
		LogFile->write(EQEMuLog::Debug, "The rest of the zone contents were successfully sent, waiting for client ready notification.");
		break;
	case ClientReadyReceived:	//client told us its ready, send them a bunch of crap like guild MOTD, etc
		LogFile->write(EQEMuLog::Debug, "We received client ready notification, but never finished Client::CompleteConnect");
		break;
	case ClientConnectFinished:	//client finally moved to finished state, were done here
		LogFile->write(EQEMuLog::Debug, "Client is successfully connected.");
		break;
	};
}

bool Client::Save(uint8 iCommitNow) {
#if 0
// Orig. Offset: 344 / 0x00000000
//		Length: 36 / 0x00000024
	unsigned char rawData[36] =
{
	0x0D, 0x30, 0xE1, 0x30, 0x1E, 0x10, 0x22, 0x10, 0x20, 0x10, 0x21, 0x10, 0x1C, 0x20, 0x1F, 0x10,
	0x7C, 0x10, 0x68, 0x10, 0x51, 0x10, 0x78, 0x10, 0xBD, 0x10, 0xD2, 0x10, 0xCD, 0x10, 0xD1, 0x10,
	0x01, 0x10, 0x6D, 0x10
} ;
	for (int tmp = 0;tmp <=35;tmp++){
		m_pp.unknown0256[89+tmp] = rawData[tmp];
	}
#endif

	if(!ClientDataLoaded())
		return false;
	_ZP(Client_Save);

	m_pp.x = x_pos;
	m_pp.y = y_pos;
	m_pp.z = z_pos;
	m_pp.guildrank=guildrank;
	m_pp.heading = heading;

	// Temp Hack for signed values until we get the root of the problem changed over to signed...
	if (m_pp.copper < 0) { m_pp.copper = 0; }
	if (m_pp.silver < 0) { m_pp.silver = 0; }
	if (m_pp.gold < 0) { m_pp.gold = 0; }
	if (m_pp.platinum < 0) { m_pp.platinum = 0; }
	if (m_pp.copper_bank < 0) { m_pp.copper_bank = 0; }
	if (m_pp.silver_bank < 0) { m_pp.silver_bank = 0; }
	if (m_pp.gold_bank < 0) { m_pp.gold_bank = 0; }
	if (m_pp.platinum_bank < 0) { m_pp.platinum_bank = 0; }


	int spentpoints=0;
	for(int a=0;a < MAX_PP_AA_ARRAY;a++) {
		uint32 points = aa[a]->value;
		if(points > HIGHEST_AA_VALUE) // Unifying this
		{
			aa[a]->value = HIGHEST_AA_VALUE;
			points = HIGHEST_AA_VALUE;
		}
		if (points > 0)
		{
			SendAA_Struct* curAA = zone->FindAA(aa[a]->AA-aa[a]->value+1);
			if(curAA)
			{
				for (int rank=0; rank<points; rank++)
				{
					std::map<uint32, AALevelCost_Struct>::iterator RequiredLevel = AARequiredLevelAndCost.find(aa[a]->AA-aa[a]->value + 1 + rank);

					if(RequiredLevel != AARequiredLevelAndCost.end())
					{
						spentpoints += RequiredLevel->second.Cost;
					}
					else
						spentpoints += (curAA->cost + (curAA->cost_inc * rank));
				}
			}
		}
	}

	m_pp.aapoints_spent = spentpoints + m_epp.expended_aa;

	if (GetHP() <= 0) {
		m_pp.cur_hp = GetMaxHP();
	}
	else
		m_pp.cur_hp = GetHP();

	m_pp.mana = cur_mana;
	m_pp.endurance = cur_end;

	database.SaveBuffs(this);

	TotalSecondsPlayed += (time(nullptr) - m_pp.lastlogin);
	m_pp.timePlayedMin = (TotalSecondsPlayed / 60);
	m_pp.RestTimer = rest_timer.GetRemainingTime() / 1000;

	if(GetMercInfo().MercTimerRemaining > RuleI(Mercs, UpkeepIntervalMS))
		GetMercInfo().MercTimerRemaining = RuleI(Mercs, UpkeepIntervalMS);

	if(GetMercTimer()->Enabled()) {
		GetMercInfo().MercTimerRemaining = GetMercTimer()->GetRemainingTime();
	}

	if (GetMerc() && !dead) {

	} else {
		memset(&m_mercinfo, 0, sizeof(struct MercInfo));
	}

	m_pp.lastlogin = time(nullptr);
	if (pQueuedSaveWorkID) {
		dbasync->CancelWork(pQueuedSaveWorkID);
		pQueuedSaveWorkID = 0;
	}

	if (GetPet() && !GetPet()->IsFamiliar() && GetPet()->CastToNPC()->GetPetSpellID() && !dead) {
		NPC *pet = GetPet()->CastToNPC();
		m_petinfo.SpellID = pet->CastToNPC()->GetPetSpellID();
		m_petinfo.HP = pet->GetHP();
		m_petinfo.Mana = pet->GetMana();
		pet->GetPetState(m_petinfo.Buffs, m_petinfo.Items, m_petinfo.Name);
		m_petinfo.petpower = pet->GetPetPower();
	} else {
		memset(&m_petinfo, 0, sizeof(struct PetInfo));
	}
	database.SavePetInfo(this);

	if(tribute_timer.Enabled()) {
		m_pp.tribute_time_remaining = tribute_timer.GetRemainingTime();
	} else {
		m_pp.tribute_time_remaining = 0xFFFFFFFF;
		m_pp.tribute_active = 0;
	}

	p_timers.Store(&database);

//	printf("Dumping inventory on save:\n");
//	m_inv.dumpInventory();

	SaveTaskState();
	if (iCommitNow <= 1) {
		char* query = 0;
		uint32_breakdown workpt;
		workpt.b4() = DBA_b4_Entity;
		workpt.w2_3() = GetID();
		workpt.b1() = DBA_b1_Entity_Client_Save;
		DBAsyncWork* dbaw = new DBAsyncWork(&database, &MTdbafq, workpt, DBAsync::Write, 0xFFFFFFFF);
		dbaw->AddQuery(iCommitNow == 0 ? true : false, &query, database.SetPlayerProfile_MQ(&query, account_id, character_id, &m_pp, &m_inv, &m_epp, 0, 0, MaxXTargets), false);
		if (iCommitNow == 0){
			pQueuedSaveWorkID = dbasync->AddWork(&dbaw, 2500);
		}
		else {
			dbasync->AddWork(&dbaw, 0);
			SaveBackup();
		}
		safe_delete_array(query);
		return true;
	}
	else if (database.SetPlayerProfile(account_id, character_id, &m_pp, &m_inv, &m_epp, 0, 0, MaxXTargets)) {
		SaveBackup();
	}
	else {
		std::cerr << "Failed to update player profile" << std::endl;
		return false;
	}

	return true;
}

void Client::SaveBackup() {
	if (!RunLoops)
		return;
	char* query = 0;
	DBAsyncWork* dbaw = new DBAsyncWork(&database, &DBAsyncCB_CharacterBackup, this->CharacterID(), DBAsync::Read);
	dbaw->AddQuery(0, &query, MakeAnyLenString(&query, "Select id, UNIX_TIMESTAMP()-UNIX_TIMESTAMP(ts) as age from character_backup where charid=%u and backupreason=0 order by ts asc", this->CharacterID()), true);
	dbasync->AddWork(&dbaw, 0);
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
	CLIENTPACKET *c = new CLIENTPACKET;

	c->ack_req = bAckreq;
	c->app = pApp->Copy();

	clientpackets.Append(c);
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
	CLIENTPACKET *c = new CLIENTPACKET;

	c->ack_req = bAckreq;
	c->app = *pApp;
	*pApp = 0;

	clientpackets.Append(c);
	return true;
}

bool Client::SendAllPackets() {
	LinkedListIterator<CLIENTPACKET*> iterator(clientpackets);

	CLIENTPACKET* cp = 0;
	iterator.Reset();
	while(iterator.MoreElements()) {
		cp = iterator.GetData();
		if(eqs)
			eqs->FastQueuePacket((EQApplicationPacket **)&cp->app, cp->ack_req);
		iterator.RemoveCurrent();
#if EQDEBUG >= 6
		LogFile->write(EQEMuLog::Normal, "Transmitting a packet");
#endif
	}
	return true;
}

void Client::QueuePacket(const EQApplicationPacket* app, bool ack_req, CLIENT_CONN_STATUS required_state, eqFilterType filter) {
/*	if (app->opcode==0x9999) {
		cout << "Sending an unknown opcode from: " << endl;
		print_stacktrace();
	}
	if (app->opcode==OP_SkillUpdate) {
		cout << "Sending OP_SkillUpdate from: " << endl;
		print_stacktrace();
	}
*/
	_ZP(Client_QueuePacket);
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
//		LogFile->write(EQEMuLog::Normal, "Adding Packet to list (%d) (%d)", app->GetOpcode(), (int)required_state);
	}
	else
		if(eqs)
			eqs->QueuePacket(app, ack_req);
}

void Client::FastQueuePacket(EQApplicationPacket** app, bool ack_req, CLIENT_CONN_STATUS required_state) {

	//cout << "Sending: 0x" << hex << setw(4) << setfill('0') << (*app)->GetOpcode() << dec << ", size=" << (*app)->size << endl;

	// if the program doesnt care about the status or if the status isnt what we requested
	if (required_state != CLIENT_CONNECTINGALL && client_state != required_state) {
		// todo: save packets for later use
		AddPacket(app, ack_req);
//		LogFile->write(EQEMuLog::Normal, "Adding Packet to list (%d) (%d)", (*app)->GetOpcode(), (int)required_state);
		return;
	}
	else {
		if(eqs)
			eqs->FastQueuePacket((EQApplicationPacket **)app, ack_req);
		else if (app && (*app))
			delete *app;
		*app = 0;
	}
	return;
}

void Client::ChannelMessageReceived(uint8 chan_num, uint8 language, uint8 lang_skill, const char* orig_message, const char* targetname) {
	char message[4096];
	strn0cpy(message, orig_message, sizeof(message));


	#if EQDEBUG >= 11
		LogFile->write(EQEMuLog::Debug,"Client::ChannelMessageReceived() Channel:%i message:'%s'", chan_num, message);
	#endif

	if (targetname == nullptr) {
		targetname = (!GetTarget()) ? "" : GetTarget()->GetName();
	}

	if(RuleB(Chat, EnableAntiSpam))
	{
		if(strcmp(targetname, "discard") != 0)
		{
			if(chan_num == 3 || chan_num == 4 || chan_num == 5 || chan_num == 7)
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
						Kick();
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


	if(RuleB(QueryServ, PlayerChatLogging)) {
		ServerPacket* pack = new ServerPacket(ServerOP_Speech, sizeof(Server_Speech_Struct) + strlen(message) + 1);
		Server_Speech_Struct* sem = (Server_Speech_Struct*) pack->pBuffer;

		if(chan_num == 0)
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

		pack->Deflate();
		if(worldserver.Connected())
			worldserver.SendPacket(pack);
		safe_delete(pack);
	}

	//Return true to proceed, false to return
	if(!mod_client_message(message, chan_num)) { return; }

	// Garble the message based on drunkness
	if (m_pp.intoxication > 0) {
		GarbleMessage(message, (int)(m_pp.intoxication / 3));
		language = 0; // No need for language when drunk
	}

	switch(chan_num)
	{
	case 0: { // GuildChat
		if (!IsInAGuild())
			Message_StringID(MT_DefaultText, GUILD_NOT_MEMBER2);	//You are not a member of any guild.
		else if (!guild_mgr.CheckPermission(GuildID(), GuildRank(), GUILD_SPEAK))
			Message(0, "Error: You dont have permission to speak to the guild.");
		else if (!worldserver.SendChannelMessage(this, targetname, chan_num, GuildID(), language, message))
			Message(0, "Error: World server disconnected");
		break;
	}
	case 2: { // GroupChat
		Raid* raid = entity_list.GetRaidByClient(this);
		if(raid) {
			raid->RaidGroupSay((const char*) message, this);
			break;
		}

		Group* group = GetGroup();
		if(group != nullptr) {
			group->GroupMessage(this,language,lang_skill,(const char*) message);
		}
		break;
	}
	case 15: { //raid say
		Raid* raid = entity_list.GetRaidByClient(this);
		if(raid){
			raid->RaidSay((const char*) message, this);
		}
		break;
	}
	case 3: { // Shout
		Mob *sender = this;
		if (GetPet() && GetPet()->FindType(SE_VoiceGraft))
			sender = GetPet();

		entity_list.ChannelMessage(sender, chan_num, language, lang_skill, message);
		break;
	}
	case 4: { // Auction
		if(RuleB(Chat, ServerWideAuction))
		{
			if(!global_channel_timer.Check())
			{
				if(strlen(targetname) == 0)
					ChannelMessageReceived(5, language, lang_skill, message, "discard"); //Fast typer or spammer??
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

			if (!worldserver.SendChannelMessage(this, 0, 4, 0, language, message))
			Message(0, "Error: World server disconnected");
		}
		else if(!RuleB(Chat, ServerWideAuction)) {
			Mob *sender = this;

			if (GetPet() && GetPet()->FindType(SE_VoiceGraft))
			sender = GetPet();

		entity_list.ChannelMessage(sender, chan_num, language, message);
		}
		break;
	}
	case 5: { // OOC
		if(RuleB(Chat, ServerWideOOC))
		{
			if(!global_channel_timer.Check())
			{
				if(strlen(targetname) == 0)
					ChannelMessageReceived(5, language, lang_skill, message, "discard"); //Fast typer or spammer??
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

			if (!worldserver.SendChannelMessage(this, 0, 5, 0, language, message))
			{
				Message(0, "Error: World server disconnected");
			}
		}
		else if(!RuleB(Chat, ServerWideOOC))
		{
			Mob *sender = this;

			if (GetPet() && GetPet()->FindType(SE_VoiceGraft))
				sender = GetPet();

			entity_list.ChannelMessage(sender, chan_num, language, message);
		}
		break;
	}
	case 6: // Broadcast
	case 11: { // GMSay
		if (!(admin >= 80))
			Message(0, "Error: Only GMs can use this channel");
		else if (!worldserver.SendChannelMessage(this, targetname, chan_num, 0, language, message))
			Message(0, "Error: World server disconnected");
		break;
	}
	case 7: { // Tell
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

			if(!worldserver.SendChannelMessage(this, target_name, chan_num, 0, language, message))
				Message(0, "Error: World server disconnected");
		break;
	}
	case 8: { // /say
		if(message[0] == COMMAND_CHAR) {
			if(command_dispatch(this, message) == -2) {
				if(RuleB(Chat, FlowCommandstoPerl_EVENT_SAY)) {
					if(parse->PlayerHasQuestSub("EVENT_SAY")) {
						parse->EventPlayer(EVENT_SAY, this, message, language);
					}
				} else {
					this->Message(13, "Command '%s' not recognized.", message);
				}
			}
			break;
		}
		Mob* sender = this;
		if (GetPet() && GetPet()->FindType(SE_VoiceGraft))
			sender = GetPet();

		printf("Message: %s\n",message);
		entity_list.ChannelMessage(sender, chan_num, language, lang_skill, message);
		if(parse->PlayerHasQuestSub("EVENT_SAY"))
		{
			parse->EventPlayer(EVENT_SAY, this, message, language);
		}

		if (sender != this)
			break;

		if(quest_manager.ProximitySayInUse())
			entity_list.ProcessProximitySay(message, this, language);

		if (GetTarget() != 0 && GetTarget()->IsNPC()) {
			if(!GetTarget()->CastToNPC()->IsEngaged()) {
				CheckLDoNHail(GetTarget());
				CheckEmoteHail(GetTarget(), message);

				if(parse->HasQuestSub(GetTarget()->GetNPCTypeID(), "EVENT_SAY")){
					if (DistNoRootNoZ(*GetTarget()) <= 200) {
						if(GetTarget()->CastToNPC()->IsMoving() && !GetTarget()->CastToNPC()->IsOnHatelist(GetTarget()))
							GetTarget()->CastToNPC()->PauseWandering(RuleI(NPC, SayPauseTimeInSec));
						parse->EventNPC(EVENT_SAY, GetTarget()->CastToNPC(), this, message, language);
					}
				}

				if (RuleB(TaskSystem, EnableTaskSystem) && DistNoRootNoZ(*GetTarget()) <= 200) {

					if(GetTarget()->CastToNPC()->IsMoving() && !GetTarget()->CastToNPC()->IsOnHatelist(GetTarget()))
						GetTarget()->CastToNPC()->PauseWandering(RuleI(NPC, SayPauseTimeInSec));

					if(UpdateTasksOnSpeakWith(GetTarget()->GetNPCTypeID())) {
						// If the client had an activity to talk to this NPC, make the NPC turn to face him if
						// he isn't moving. Makes things look better.
						if(!GetTarget()->CastToNPC()->IsMoving())
							GetTarget()->FaceTarget(this);
					}
				}
			}
			else {
				if(parse->HasQuestSub(GetTarget()->GetNPCTypeID(), "EVENT_AGGRO_SAY")) {
					if (DistNoRootNoZ(*GetTarget()) <= 200) {
						parse->EventNPC(EVENT_AGGRO_SAY, GetTarget()->CastToNPC(), this, message, language);
					}
				}
			}

		}
		break;
	}
	case 20:
	{
		// UCS Relay for Underfoot and later.
		if(!worldserver.SendChannelMessage(this, 0, chan_num, 0, language, message))
			Message(0, "Error: World server disconnected");
		break;
	}
	case 22:
	{
		// Emotes for Underfoot and later.
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_Emote, 4 + strlen(message) + strlen(GetName()) + 2);
		Emote_Struct* es = (Emote_Struct*)outapp->pBuffer;
		char *Buffer = (char *)es;
		Buffer += 4;
		snprintf(Buffer, sizeof(Emote_Struct) - 4, "%s %s", GetName(), message);
		entity_list.QueueCloseClients(this, outapp, true, 100, 0, true, FilterSocials);
		safe_delete(outapp);
		break;
	}
	default: {
		Message(0, "Channel (%i) not implemented", (uint16)chan_num);
	}
	}
}

// if no language skill is specified, call the function with a skill of 100.
void Client::ChannelMessageSend(const char* from, const char* to, uint8 chan_num, uint8 language, const char* message, ...) {
	ChannelMessageSend(from, to, chan_num, language, 100, message);
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
	else if (chan_num == 7)
		strcpy(cm->targetname, m_pp.name);
	else
		cm->targetname[0] = 0;

	uint8 ListenerSkill;

	if (language < MAX_PP_LANGUAGE) {
		ListenerSkill = m_pp.languages[language];
		if (ListenerSkill == 0) {
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

	// Garble the message based on listener skill
	if (ListenerSkill < 100) {
		GarbleMessage(buffer, (100 - ListenerSkill));
	}

	cm->chan_num = chan_num;
	strcpy(&cm->message[0], buffer);
	QueuePacket(&app);

	if ((chan_num == 2) && (ListenerSkill < 100)) {	// group message in unmastered language, check for skill up
		if ((m_pp.languages[language] <= lang_skill) && (from != this->GetName()))
			CheckLanguageSkillIncrease(language, lang_skill);
	}
}

void Client::Message(uint32 type, const char* message, ...) {
	if (GetFilter(FilterSpellDamage) == FilterHide && type == MT_NonMelee)
		return;
	if (GetFilter(FilterMeleeCrits) == FilterHide && type == MT_CritMelee) //98 is self...
		return;
	if (GetFilter(FilterSpellCrits) == FilterHide && type == MT_SpellCrits)
		return;

	va_list argptr;
	char *buffer = new char[4096];
	va_start(argptr, message);
	vsnprintf(buffer, 4096, message, argptr);
	va_end(argptr);

	size_t len = strlen(buffer);

	//client dosent like our packet all the time unless
	//we make it really big, then it seems to not care that
	//our header is malformed.
	//len = 4096 - sizeof(SpecialMesg_Struct);

	uint32 len_packet = sizeof(SpecialMesg_Struct)+len;
	EQApplicationPacket* app = new EQApplicationPacket(OP_SpecialMesg, len_packet);
	SpecialMesg_Struct* sm=(SpecialMesg_Struct*)app->pBuffer;
	sm->header[0] = 0x00; // Header used for #emote style messages..
	sm->header[1] = 0x00; // Play around with these to see other types
	sm->header[2] = 0x00;
	sm->msg_type = type;
	memcpy(sm->message, buffer, len+1);

	FastQueuePacket(&app);

	safe_delete_array(buffer);
}

void Client::QuestJournalledMessage(const char *npcname, const char* message) {

	// npcnames longer than 60 characters crash the client when they log back in
	const int MaxNPCNameLength = 60;
	// I assume there is an upper safe limit on the message length. Don't know what it is, but 4000 doesn't crash
	// the client.
	const int MaxMessageLength = 4000;

	char OutNPCName[MaxNPCNameLength+1];
	char OutMessage[MaxMessageLength+1];

	// Apparently Visual C++ snprintf is not C99 compliant and doesn't put the null terminator
	// in if the formatted string >= the maximum length, so we put it in.
	//
	snprintf(OutNPCName, MaxNPCNameLength, "%s", npcname); OutNPCName[MaxNPCNameLength]='\0';
	snprintf(OutMessage, MaxMessageLength, "%s", message); OutMessage[MaxMessageLength]='\0';

	uint32 len_packet = sizeof(SpecialMesg_Struct) + strlen(OutNPCName) + strlen(OutMessage);
	EQApplicationPacket* app = new EQApplicationPacket(OP_SpecialMesg, len_packet);
	SpecialMesg_Struct* sm=(SpecialMesg_Struct*)app->pBuffer;

	sm->header[0] = 0;
	sm->header[1] = 2;
	sm->header[2] = 0;
	sm->msg_type = 0x0a;
	sm->target_spawn_id = GetID();

	char *dest = &sm->sayer[0];

	memcpy(dest, OutNPCName, strlen(OutNPCName) + 1);

	dest = dest + strlen(OutNPCName) + 13;

	memcpy(dest, OutMessage, strlen(OutMessage) + 1);

	QueuePacket(app);

	safe_delete(app);
}

void Client::SetMaxHP() {
	if(dead)
		return;
	SetHP(CalcMaxHP());
	SendHPUpdate();
	Save();
}

bool Client::UpdateLDoNPoints(int32 points, uint32 theme)
{

/* make sure total stays in sync with individual buckets
	m_pp.ldon_points_available = m_pp.ldon_points_guk
		+m_pp.ldon_points_mir
		+m_pp.ldon_points_mmc
		+m_pp.ldon_points_ruj
		+m_pp.ldon_points_tak; */

	if(points < 0)
	{
		if(m_pp.ldon_points_available < (0-points))
			return false;
	}
	switch(theme)
	{
	// handle generic points (theme=0)
	case 0:
		{	// no theme, so distribute evenly across all
			int splitpts=points/5;
			int gukpts=splitpts+(points%5);
			int mirpts=splitpts;
			int mmcpts=splitpts;
			int rujpts=splitpts;
			int takpts=splitpts;

			splitpts=0;

			if(points < 0)
			{
				if(m_pp.ldon_points_available < (0-points))
				{
					return false;
				}
				if(m_pp.ldon_points_guk < (0-gukpts))
				{
					mirpts+=gukpts+m_pp.ldon_points_guk;
					gukpts=0-m_pp.ldon_points_guk;
				}
				if(m_pp.ldon_points_mir < (0-mirpts))
				{
					mmcpts+=mirpts+m_pp.ldon_points_mir;
					mirpts=0-m_pp.ldon_points_mir;
				}
				if(m_pp.ldon_points_mmc < (0-mmcpts))
				{
					rujpts+=mmcpts+m_pp.ldon_points_mmc;
					mmcpts=0-m_pp.ldon_points_mmc;
				}
				if(m_pp.ldon_points_ruj < (0-rujpts))
				{
					takpts+=rujpts+m_pp.ldon_points_ruj;
					rujpts=0-m_pp.ldon_points_ruj;
				}
				if(m_pp.ldon_points_tak < (0-takpts))
				{
					splitpts=takpts+m_pp.ldon_points_tak;
					takpts=0-m_pp.ldon_points_tak;
				}
			}
			m_pp.ldon_points_guk += gukpts;
			m_pp.ldon_points_mir+=mirpts;
			m_pp.ldon_points_mmc += mmcpts;
			m_pp.ldon_points_ruj += rujpts;
			m_pp.ldon_points_tak += takpts;
			points-=splitpts;
		// if anything left, recursively loop thru again
			if (splitpts !=0)
				UpdateLDoNPoints(splitpts,0);
			break;
		}
	case 1:
		{
			if(points < 0)
			{
				if(m_pp.ldon_points_guk < (0-points))
					return false;
			}
			m_pp.ldon_points_guk += points;
			break;
		}
	case 2:
		{
			if(points < 0)
			{
				if(m_pp.ldon_points_mir < (0-points))
					return false;
			}
			m_pp.ldon_points_mir += points;
			break;
		}
	case 3:
		{
			if(points < 0)
			{
				if(m_pp.ldon_points_mmc < (0-points))
					return false;
			}
			m_pp.ldon_points_mmc += points;
			break;
		}
	case 4:
		{
			if(points < 0)
			{
				if(m_pp.ldon_points_ruj < (0-points))
					return false;
			}
			m_pp.ldon_points_ruj += points;
			break;
		}
	case 5:
		{
			if(points < 0)
			{
				if(m_pp.ldon_points_tak < (0-points))
					return false;
			}
			m_pp.ldon_points_tak += points;
			break;
		}
	}
	m_pp.ldon_points_available += points;
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_AdventurePointsUpdate, sizeof(AdventurePoints_Update_Struct));
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

	return(false);
}

void Client::SetSkill(SkillType skillid, uint16 value) {
	if (skillid > HIGHEST_SKILL)
		return;
	m_pp.skills[skillid] = value; // We need to be able to #setskill 254 and 255 to reset skills

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_SkillUpdate, sizeof(SkillUpdate_Struct));
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

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_SkillUpdate, sizeof(SkillUpdate_Struct));
	SkillUpdate_Struct* skill = (SkillUpdate_Struct*)outapp->pBuffer;
	skill->skillId = 100 + skill_id;
	skill->value = m_pp.languages[skill_id];
	QueuePacket(outapp);
	safe_delete(outapp);

	Message_StringID( MT_Skills, LANG_SKILL_IMPROVED ); //Notify client
}

void Client::AddSkill(SkillType skillid, uint16 value) {
	if (skillid > HIGHEST_SKILL)
		return;
	value = GetRawSkill(skillid) + value;
	uint16 max = GetMaxSkillAfterSpecializationRules(skillid, MaxSkill(skillid));
	if (value > max)
		value = max;
	SetSkill(skillid, value);
}

void Client::SendSound(){//Makes a sound.
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_Sound, 68);
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
	ServerPacket* pack = new ServerPacket(ServerOP_ClientList, sizeof(ServerClientList_Struct));
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

	scl->ClientVersion = GetClientVersion();
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
		ServerPacket* pack = new ServerPacket(ServerOP_Who, sizeof(ServerWhoAll_Struct));
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
		ServerPacket* pack = new ServerPacket(ServerOP_FriendsWho, sizeof(ServerFriendsWho_Struct) + strlen(FriendsString));
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
#if EQDEBUG >= 5
		printf("%s is a GM\n", GetName());
#endif
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
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_IncreaseStats,sizeof(IncreaseStat_Struct));
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
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_IncreaseStats,sizeof(IncreaseStat_Struct));
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
	if (amount != cur_mana)
		update = true;
	cur_mana = amount;
	if (update)
		Mob::SetMana(amount);
	SendManaUpdatePacket();
	return cur_mana;
}

void Client::SendManaUpdatePacket() {
	if (!Connected() || IsCasting())
		return;

	if (GetClientVersion() >= EQClientSoD) {
		SendManaUpdate();
		SendEnduranceUpdate();
	}

	//cout << "Sending mana update: " << (cur_mana - last_reported_mana) << endl;
	if (last_reported_mana != cur_mana || last_reported_endur != cur_end) {



		EQApplicationPacket* outapp = new EQApplicationPacket(OP_ManaChange, sizeof(ManaChange_Struct));
		ManaChange_Struct* manachange = (ManaChange_Struct*)outapp->pBuffer;
		manachange->new_mana = cur_mana;
		manachange->stamina = cur_end;
		manachange->spell_id = casting_spell_id;	//always going to be 0... since we check IsCasting()
		outapp->priority = 6;
		QueuePacket(outapp);
		safe_delete(outapp);

		Group *g = GetGroup();

		if(g)
		{
			outapp = new EQApplicationPacket(OP_MobManaUpdate, sizeof(MobManaUpdate_Struct));
			EQApplicationPacket *outapp2 = new EQApplicationPacket(OP_MobEnduranceUpdate, sizeof(MobEnduranceUpdate_Struct));

			MobManaUpdate_Struct *mmus = (MobManaUpdate_Struct *)outapp->pBuffer;
			MobEnduranceUpdate_Struct *meus = (MobEnduranceUpdate_Struct *)outapp2->pBuffer;

			mmus->spawn_id = meus->spawn_id = GetID();

			mmus->mana = GetManaPercent();
			meus->endurance = GetEndurancePercent();


			for(int i = 0; i < MAX_GROUP_MEMBERS; ++i)
				if(g->members[i] && g->members[i]->IsClient() && (g->members[i] != this) && (g->members[i]->CastToClient()->GetClientVersion() >= EQClientSoD))
				{
					g->members[i]->CastToClient()->QueuePacket(outapp);
					g->members[i]->CastToClient()->QueuePacket(outapp2);
				}

			safe_delete(outapp);
			safe_delete(outapp2);
		}


		last_reported_mana = cur_mana;
		last_reported_endur = cur_end;
	}
}

// sends mana update to self
void Client::SendManaUpdate()
{
	EQApplicationPacket* mana_app = new EQApplicationPacket(OP_ManaUpdate,sizeof(ManaUpdate_Struct));
	ManaUpdate_Struct* mus = (ManaUpdate_Struct*)mana_app->pBuffer;
	mus->cur_mana = GetMana();
	mus->max_mana = GetMaxMana();
	mus->spawn_id = GetID();
	QueuePacket(mana_app);
	entity_list.QueueClientsByXTarget(this, mana_app, false);
	safe_delete(mana_app);
}

// sends endurance update to self
void Client::SendEnduranceUpdate()
{
	EQApplicationPacket* end_app = new EQApplicationPacket(OP_EnduranceUpdate,sizeof(EnduranceUpdate_Struct));
	EnduranceUpdate_Struct* eus = (EnduranceUpdate_Struct*)end_app->pBuffer;
	eus->cur_end = GetEndurance();
	eus->max_end = GetMaxEndurance();
	eus->spawn_id = GetID();
	QueuePacket(end_app);
	entity_list.QueueClientsByXTarget(this, end_app, false);
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
//	ns->spawn.pvp		= GetPVP() ? 1 : 0;


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
	if (!m_pp.showhelm) ns->spawn.showhelm = 0;

	// pp also hold this info; should we pull from there or inventory?
	// (update: i think pp should do it, as this holds LoY dye - plus, this is ugly code with Inventory!)
	const Item_Struct* item = nullptr;
	const ItemInst* inst = nullptr;
	if ((inst = m_inv[SLOT_HANDS]) && inst->IsType(ItemClassCommon)) {
		item = inst->GetItem();
		ns->spawn.equipment[MATERIAL_HANDS]	= item->Material;
		ns->spawn.colors[MATERIAL_HANDS].color	= GetEquipmentColor(MATERIAL_HANDS);
	}
	if ((inst = m_inv[SLOT_HEAD]) && inst->IsType(ItemClassCommon)) {
		item = inst->GetItem();
		ns->spawn.equipment[MATERIAL_HEAD]	= item->Material;
		ns->spawn.colors[MATERIAL_HEAD].color	= GetEquipmentColor(MATERIAL_HEAD);
	}
	if ((inst = m_inv[SLOT_ARMS]) && inst->IsType(ItemClassCommon)) {
		item = inst->GetItem();
		ns->spawn.equipment[MATERIAL_ARMS]	= item->Material;
		ns->spawn.colors[MATERIAL_ARMS].color	= GetEquipmentColor(MATERIAL_ARMS);
	}
	if ((inst = m_inv[SLOT_BRACER01]) && inst->IsType(ItemClassCommon)) {
		item = inst->GetItem();
		ns->spawn.equipment[MATERIAL_BRACER]= item->Material;
		ns->spawn.colors[MATERIAL_BRACER].color	= GetEquipmentColor(MATERIAL_BRACER);
	}
	if ((inst = m_inv[SLOT_BRACER02]) && inst->IsType(ItemClassCommon)) {
		item = inst->GetItem();
		ns->spawn.equipment[MATERIAL_BRACER]= item->Material;
		ns->spawn.colors[MATERIAL_BRACER].color	= GetEquipmentColor(MATERIAL_BRACER);
	}
	if ((inst = m_inv[SLOT_CHEST]) && inst->IsType(ItemClassCommon)) {
		item = inst->GetItem();
		ns->spawn.equipment[MATERIAL_CHEST]	= item->Material;
		ns->spawn.colors[MATERIAL_CHEST].color	= GetEquipmentColor(MATERIAL_CHEST);
	}
	if ((inst = m_inv[SLOT_LEGS]) && inst->IsType(ItemClassCommon)) {
		item = inst->GetItem();
		ns->spawn.equipment[MATERIAL_LEGS]	= item->Material;
		ns->spawn.colors[MATERIAL_LEGS].color	= GetEquipmentColor(MATERIAL_LEGS);
	}
	if ((inst = m_inv[SLOT_FEET]) && inst->IsType(ItemClassCommon)) {
		item = inst->GetItem();
		ns->spawn.equipment[MATERIAL_FEET]	= item->Material;
		ns->spawn.colors[MATERIAL_FEET].color	= GetEquipmentColor(MATERIAL_FEET);
	}
	if ((inst = m_inv[SLOT_PRIMARY]) && inst->IsType(ItemClassCommon)) {
		item = inst->GetItem();
		if (strlen(item->IDFile) > 2)
			ns->spawn.equipment[MATERIAL_PRIMARY] = atoi(&item->IDFile[2]);
	}
	if ((inst = m_inv[SLOT_SECONDARY]) && inst->IsType(ItemClassCommon)) {
		item = inst->GetItem();
		if (strlen(item->IDFile) > 2)
			ns->spawn.equipment[MATERIAL_SECONDARY] = atoi(&item->IDFile[2]);
	}

	//these two may be related to ns->spawn.texture
	/*
	ns->spawn.npc_armor_graphic = texture;
	ns->spawn.npc_helm_graphic = helmtexture;
	*/

	//filling in some unknowns to make the client happy
//	ns->spawn.unknown0002[2] = 3;

}

bool Client::GMHideMe(Client* client) {
	if (gmhideme) {
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
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GMLastName, sizeof(GMLastName_Struct));
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
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GMNameChange, sizeof(GMName_Struct));
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
	Message(13, "You are %s a GM.", m_pp.gm ? "now" : "no longer");
	SendAppearancePacket(AT_GM, m_pp.gm);
	Save();
	UpdateWho();
}

void Client::ReadBook(BookRequest_Struct *book) {
	char *txtfile = book->txtfile;

	if(txtfile[0] == '0' && txtfile[1] == '\0') {
		//invalid book... coming up on non-book items.
		return;
	}

	std::string booktxt2 = database.GetBook(txtfile);
	int length = booktxt2.length();

	if (booktxt2[0] != '\0') {
#if EQDEBUG >= 6
		LogFile->write(EQEMuLog::Normal,"Client::ReadBook() textfile:%s Text:%s", txtfile, booktxt2.c_str());
#endif
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_ReadBook, length + sizeof(BookText_Struct));

		BookText_Struct *out = (BookText_Struct *) outapp->pBuffer;
		out->window = book->window;
		if(GetClientVersion() >= EQClientSoF)
		{
			const ItemInst *inst = m_inv[book->invslot];
			if(inst)
				out->type = inst->GetItem()->Book;
			else
				out->type = book->type;
		}
		else
		{
			out->type = book->type;
		}
		out->invslot = book->invslot;
		memcpy(out->booktext, booktxt2.c_str(), length);

		QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Client::QuestReadBook(const char* text, uint8 type) {
	std::string booktxt2 = text;
	int length = booktxt2.length();
	if (booktxt2[0] != '\0') {
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_ReadBook, length + sizeof(BookText_Struct));
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
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_TradeMoneyUpdate,sizeof(TradeMoneyUpdate_Struct));
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
			copper = abs64(copperpp);
			m_pp.copper = 0;
		}
		else
		{
			m_pp.copper = copperpp;
			if(updateclient)
				SendMoneyUpdate();
			Save();
			return true;
		}
		silver -= copper;
		if(silver <= 0)
		{
			copper = abs64(silver);
			m_pp.silver = 0;
		}
		else
		{
			m_pp.silver = silver/10;
			m_pp.copper += (silver-(m_pp.silver*10));
			if(updateclient)
				SendMoneyUpdate();
			Save();
			return true;
		}

		gold -=copper;

		if(gold <= 0)
		{
			copper = abs64(gold);
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
			Save();
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
		Save();
		return true;
	}
}

void Client::AddMoneyToPP(uint64 copper, bool updateclient){
	uint64 tmp;
	uint64 tmp2;
	tmp = copper;

	// Add Amount of Platinum
	tmp2 = tmp/1000;
	int32 new_val = m_pp.platinum + tmp2;
	if(new_val < 0) {
		m_pp.platinum = 0;
	} else {
		m_pp.platinum = m_pp.platinum + tmp2;
	}
	tmp-=tmp2*1000;

	//if (updateclient)
	//	SendClientMoneyUpdate(3,tmp2);

	// Add Amount of Gold
	tmp2 = tmp/100;
	new_val = m_pp.gold + tmp2;
	if(new_val < 0) {
		m_pp.gold = 0;
	} else {
		m_pp.gold = m_pp.gold + tmp2;
	}
	tmp-=tmp2*100;
	//if (updateclient)
	//	SendClientMoneyUpdate(2,tmp2);

	// Add Amount of Silver
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

	Save();

	LogFile->write(EQEMuLog::Debug, "Client::AddMoneyToPP() %s should have: plat:%i gold:%i silver:%i copper:%i", GetName(), m_pp.platinum, m_pp.gold, m_pp.silver, m_pp.copper);
}

void Client::AddMoneyToPP(uint32 copper, uint32 silver, uint32 gold, uint32 platinum, bool updateclient){

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
	Save();

#if (EQDEBUG>=5)
		LogFile->write(EQEMuLog::Debug, "Client::AddMoneyToPP() %s should have: plat:%i gold:%i silver:%i copper:%i",
			GetName(), m_pp.platinum, m_pp.gold, m_pp.silver, m_pp.copper);
#endif
}

void Client::SendMoneyUpdate() {
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_MoneyUpdate,sizeof(MoneyUpdate_Struct));
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

bool Client::CheckIncreaseSkill(SkillType skillid, Mob *against_who, int chancemodi) {
	if (IsAIControlled()) // no skillups while chamred =p
		return false;
	if (skillid > HIGHEST_SKILL)
		return false;
	int skillval = GetRawSkill(skillid);
	int maxskill = GetMaxSkillAfterSpecializationRules(skillid, MaxSkill(skillid));

	if(against_who)
	{
		if(against_who->SpecAttacks[IMMUNE_AGGRO] || against_who->IsClient() ||
			GetLevelCon(against_who->GetLevel()) == CON_GREEN)
		{
			//false by default
			return mod_can_increase_skill(skillid, against_who);
		}
	}

	// Make sure we're not already at skill cap
	if (skillval < maxskill)
	{
		// the higher your current skill level, the harder it is
		int16 Chance = 10 + chancemodi + ((252 - skillval) / 20);
		if (Chance < 1)
			Chance = 1; // Make it always possible
		Chance = (Chance * RuleI(Character, SkillUpModifier) / 100);

		Chance = mod_increase_skill_chance(Chance, against_who);

		if(MakeRandomFloat(0, 99) < Chance)
		{
			SetSkill(skillid, GetRawSkill(skillid) + 1);
			_log(SKILLS__GAIN, "Skill %d at value %d successfully gain with %.4f%%chance (mod %d)", skillid, skillval, Chance, chancemodi);
			return true;
		} else {
			_log(SKILLS__GAIN, "Skill %d at value %d failed to gain with %.4f%%chance (mod %d)", skillid, skillval, Chance, chancemodi);
		}
	} else {
		_log(SKILLS__GAIN, "Skill %d at value %d cannot increase due to maxmum %d", skillid, skillval, maxskill);
	}
	return false;
}

void Client::CheckLanguageSkillIncrease(uint8 langid, uint8 TeacherSkill) {
	if (langid >= MAX_PP_LANGUAGE)
		return;		// do nothing if langid is an invalid language

	int LangSkill = m_pp.languages[langid];		// get current language skill

	if (LangSkill < 100) {	// if the language isn't already maxed
		int16 Chance = 5 + ((TeacherSkill - LangSkill)/10);	// greater chance to learn if teacher's skill is much higher than yours
		Chance = (Chance * RuleI(Character, SkillUpModifier)/100);

		if(MakeRandomFloat(0,100) < Chance) {	// if they make the roll
			IncreaseLanguageSkill(langid);	// increase the language skill by 1
			_log(SKILLS__GAIN, "Language %d at value %d successfully gain with %.4f%%chance", langid, LangSkill, Chance);
		}
		else
			_log(SKILLS__GAIN, "Language %d at value %d failed to gain with %.4f%%chance", langid, LangSkill, Chance);
	}
}

bool Client::HasSkill(SkillType skill_id) const {
	return((GetSkill(skill_id) > 0) && CanHaveSkill(skill_id));
}

bool Client::CanHaveSkill(SkillType skill_id) const {
	return(database.GetSkillCap(GetClass(), skill_id, RuleI(Character, MaxLevel)) > 0);
	//if you don't have it by max level, then odds are you never will?
}

uint16 Client::MaxSkill(SkillType skillid, uint16 class_, uint16 level) const {
	return(database.GetSkillCap(class_, skillid, level));
}

uint8 Client::SkillTrainLevel(SkillType skillid, uint16 class_){
	return(database.GetTrainLevel(class_, skillid, RuleI(Character, MaxLevel)));
}

uint16 Client::GetMaxSkillAfterSpecializationRules(SkillType skillid, uint16 maxSkill)
{
	uint16 Result = maxSkill;

	uint16 PrimarySpecialization = 0, SecondaryForte = 0;

	uint16 PrimarySkillValue = 0, SecondarySkillValue = 0;

	uint16 MaxSpecializations = GetAA(aaSecondaryForte) ? 2 : 1;

	if(skillid >= SPECIALIZE_ABJURE && skillid <= SPECIALIZE_EVOCATION)
	{
		bool HasPrimarySpecSkill = false;

		int NumberOfPrimarySpecSkills = 0;

		for(int i = SPECIALIZE_ABJURE; i <= SPECIALIZE_EVOCATION; ++i)
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
				Message(13, "Your spell casting specializations skills have been reset. "
						"Only %i primary specialization skill is allowed.", MaxSpecializations);

				for(int i = SPECIALIZE_ABJURE; i <= SPECIALIZE_EVOCATION; ++i)
					SetSkill((SkillType)i, 1);

				Save();

				LogFile->write(EQEMuLog::Normal, "Reset %s's caster specialization skills to 1. "
								"Too many specializations skills were above 50.", GetCleanName());
			}

		}
	}
	// This should possibly be handled by bonuses rather than here.
	switch(skillid)
	{
		case TRACKING:
		{
			Result += ((GetAA(aaAdvancedTracking) * 10) + (GetAA(aaTuneofPursuance) * 10));
			break;
		}

		default:
			break;
	}

	return Result;
}

void Client::SetPVP(bool toggle) {
	m_pp.pvp = toggle ? 1 : 0;

	if(GetPVP())
		this->Message_StringID(MT_Shout,PVP_ON);
	else
		Message(13, "You no longer follow the ways of discord.");

	SendAppearancePacket(AT_PVP, GetPVP());
	Save();
}

void Client::WorldKick() {
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GMKick, sizeof(GMKick_Struct));
	GMKick_Struct* gmk = (GMKick_Struct *)outapp->pBuffer;
	strcpy(gmk->name,GetName());
	QueuePacket(outapp);
	safe_delete(outapp);
	Kick();
}

void Client::GMKill() {
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_GMKill, sizeof(GMKill_Struct));
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
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_MemorizeSpell,sizeof(MemorizeSpell_Struct));
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

void Client::LogMerchant(Client* player, Mob* merchant, uint32 quantity, uint32 price, const Item_Struct* item, bool buying)
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

void Client::LogLoot(Client* player, Corpse* corpse, const Item_Struct* item){
	char* logtext;
	char itemid[100];
	char itemname[100];
	char coinloot[100];
	if (item!=0){
		memset(itemid,0,sizeof(itemid));
		memset(itemname,0,sizeof(itemid));
		itoa(item->ID,itemid,10);
		sprintf(itemname,"%s",item->Name);
		logtext=itemname;

		strcat(logtext,"(");
		strcat(logtext,itemid);
		strcat(logtext,") Looted");
		database.logevents(player->AccountName(),player->AccountID(),player->admin,player->GetName(),corpse->orgname,"Looting Item",logtext,4);
	}
	else{
		if ((corpse->GetPlatinum() + corpse->GetGold() + corpse->GetSilver() + corpse->GetCopper())>0) {
			memset(coinloot,0,sizeof(coinloot));
			sprintf(coinloot,"%i PP %i GP %i SP %i CP",corpse->GetPlatinum(),corpse->GetGold(),corpse->GetSilver(),corpse->GetCopper());
			logtext=coinloot;
			strcat(logtext," Looted");
			if (corpse->GetPlatinum()>10000)
				database.logevents(player->AccountName(),player->AccountID(),player->admin,player->GetName(),corpse->orgname,"Excessive Loot!",logtext,9);
			else
				database.logevents(player->AccountName(),player->AccountID(),player->admin,player->GetName(),corpse->orgname,"Looting Money",logtext,5);
		}
	}
}


bool Client::BindWound(Mob* bindmob, bool start, bool fail){
	EQApplicationPacket* outapp = 0;
	if(!fail) {
		outapp = new EQApplicationPacket(OP_Bind_Wound, sizeof(BindWound_Struct));
		BindWound_Struct* bind_out = (BindWound_Struct*) outapp->pBuffer;
		// Start bind
		if(!bindwound_timer.Enabled()) {
			//make sure we actually have a bandage... and consume it.
			int16 bslot = m_inv.HasItemByUse(ItemTypeBandage, 1, invWhereWorn|invWherePersonal);
			if(bslot == SLOT_INVALID) {
				bind_out->type = 3;
				QueuePacket(outapp);
				bind_out->type = 7;	//this is the wrong message, dont know the right one.
				QueuePacket(outapp);
				return(true);
			}
			DeleteItemInInventory(bslot, 1, true);	//do we need client update?

			// start complete timer
			bindwound_timer.Start(10000);
			bindwound_target = bindmob;

			// Send client unlock
			bind_out->type = 3;
			QueuePacket(outapp);
			bind_out->type = 0;
			// Client Unlocked
			if(!bindmob) {
				// send "bindmob dead" to client
				bind_out->type = 4;
				QueuePacket(outapp);
				bind_out->type = 0;
				bindwound_timer.Disable();
				bindwound_target = 0;
			}
			else {
				// send bindmob "stand still"
				if(!bindmob->IsAIControlled() && bindmob != this ) {
					bind_out->type = 2; // ?
					//bind_out->type = 3; // ?
					bind_out->to = GetID(); // ?
					bindmob->CastToClient()->QueuePacket(outapp);
					bind_out->type = 0;
					bind_out->to = 0;
				}
				else if (bindmob->IsAIControlled() && bindmob != this ){
					; // Tell IPC to stand still?
				}
				else {
					; // Binding self
				}
			}
		} else {
		// finish bind
			// disable complete timer
			bindwound_timer.Disable();
			bindwound_target = 0;
			if(!bindmob){
					// send "bindmob gone" to client
					bind_out->type = 5; // not in zone
					QueuePacket(outapp);
					bind_out->type = 0;
			}

			else {
				if (!GetFeigned() && (bindmob->DistNoRoot(*this) <= 400)) {
					// send bindmob bind done
					if(!bindmob->IsAIControlled() && bindmob != this ) {

					}
					else if(bindmob->IsAIControlled() && bindmob != this ) {
					// Tell IPC to resume??
					}
					else {
					// Binding self
					}
					// Send client bind done

					//this is taken care of on start of bind, not finish now, and is improved
					//DeleteItemInInventory(m_inv.HasItem(13009, 1), 1, true);

					bind_out->type = 1; // Done
					QueuePacket(outapp);
					bind_out->type = 0;
					CheckIncreaseSkill(BIND_WOUND, nullptr, 5);

					int maxHPBonus = spellbonuses.MaxBindWound + itembonuses.MaxBindWound + aabonuses.MaxBindWound;

					int max_percent = 50 + 10 * maxHPBonus;

					if(GetClass() == MONK && GetSkill(BIND_WOUND) > 200) {
						max_percent = 70 + 10 * maxHPBonus;
					}

					max_percent = mod_bindwound_percent(max_percent, bindmob);

					int max_hp = bindmob->GetMaxHP()*max_percent/100;

					// send bindmob new hp's
					if (bindmob->GetHP() < bindmob->GetMaxHP() && bindmob->GetHP() <= (max_hp)-1){
						// 0.120 per skill point, 0.60 per skill level, minimum 3 max 30
						int bindhps = 3;


						if (GetSkill(BIND_WOUND) > 200) {
							bindhps += GetSkill(BIND_WOUND)*4/10;
						} else if (GetSkill(BIND_WOUND) >= 10) {
							bindhps += GetSkill(BIND_WOUND)/4;
						}

						//Implementation of aaMithanielsBinding is a guess (the multiplier)
						int bindBonus = spellbonuses.BindWound + itembonuses.BindWound + aabonuses.BindWound;

						bindhps += bindhps*bindBonus / 100;

						bindhps = mod_bindwound_hp(bindhps, bindmob);

						//if the bind takes them above the max bindable
						//cap it at that value. Dont know if live does it this way
						//but it makes sense to me.
						int chp = bindmob->GetHP() + bindhps;
						if(chp > max_hp)
							chp = max_hp;

						bindmob->SetHP(chp);
						bindmob->SendHPUpdate();
					}
					else {
						//I dont have the real, live
						Message(15, "You cannot bind wounds above %d%% hitpoints.", max_percent);
						if(bindmob->IsClient())
							bindmob->CastToClient()->Message(15, "You cannot have your wounds bound above %d%% hitpoints.", max_percent);
						// Too many hp message goes here.
					}
				}
				else {
					// Send client bind failed
					if(bindmob != this)
						bind_out->type = 6; // They moved
					else
						bind_out->type = 7; // Bandager moved

					QueuePacket(outapp);
					bind_out->type = 0;
				}
			}
		}
	}
	else if (bindwound_timer.Enabled()) {
		// You moved
		outapp = new EQApplicationPacket(OP_Bind_Wound, sizeof(BindWound_Struct));
		BindWound_Struct* bind_out = (BindWound_Struct*) outapp->pBuffer;
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

void Client::SetMaterial(int16 in_slot, uint32 item_id){
	const Item_Struct* item = database.GetItem(item_id);
	if (item && (item->ItemClass==ItemClassCommon)) {
		if (in_slot==SLOT_HEAD)
			m_pp.item_material[MATERIAL_HEAD]		= item->Material;
		else if (in_slot==SLOT_CHEST)
			m_pp.item_material[MATERIAL_CHEST]		= item->Material;
		else if (in_slot==SLOT_ARMS)
			m_pp.item_material[MATERIAL_ARMS]		= item->Material;
		else if (in_slot==SLOT_BRACER01)
			m_pp.item_material[MATERIAL_BRACER]		= item->Material;
		else if (in_slot==SLOT_BRACER02)
			m_pp.item_material[MATERIAL_BRACER]		= item->Material;
		else if (in_slot==SLOT_HANDS)
			m_pp.item_material[MATERIAL_HANDS]		= item->Material;
		else if (in_slot==SLOT_LEGS)
			m_pp.item_material[MATERIAL_LEGS]		= item->Material;
		else if (in_slot==SLOT_FEET)
			m_pp.item_material[MATERIAL_FEET]		= item->Material;
		else if (in_slot==SLOT_PRIMARY)
			m_pp.item_material[MATERIAL_PRIMARY]	= atoi(item->IDFile+2);
		else if (in_slot==SLOT_SECONDARY)
			m_pp.item_material[MATERIAL_SECONDARY]	= atoi(item->IDFile+2);
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

	Filter1(FilterMeleeCrits);

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
	Filter1(FilterDOT);
	Filter1(FilterPetHits);
	Filter1(FilterPetMisses);
	Filter1(FilterFocusEffects);
	Filter1(FilterPetSpells);
	Filter1(FilterHealOverTime);
}

// this version is for messages with no parameters
void Client::Message_StringID(uint32 type, uint32 string_id, uint32 distance)
{
	if (GetFilter(FilterSpellDamage) == FilterHide && type == MT_NonMelee)
		return;
	if (GetFilter(FilterMeleeCrits) == FilterHide && type == MT_CritMelee) //98 is self...
		return;
	if (GetFilter(FilterSpellCrits) == FilterHide && type == MT_SpellCrits)
		return;
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_SimpleMessage,12);
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
void Client::Message_StringID(uint32 type, uint32 string_id, const char* message1,
	const char* message2,const char* message3,const char* message4,
	const char* message5,const char* message6,const char* message7,
	const char* message8,const char* message9, uint32 distance)
{
	if (GetFilter(FilterSpellDamage) == FilterHide && type == MT_NonMelee)
		return;
	if (GetFilter(FilterMeleeCrits) == FilterHide && type == MT_CritMelee) //98 is self...
		return;
	if (GetFilter(FilterSpellCrits) == FilterHide && type == MT_SpellCrits)
		return;
	if (GetFilter(FilterDamageShields) == FilterHide && type == MT_DS)
		return;

	int i, argcount, length;
	char *bufptr;
	const char *message_arg[9] = {0};

	if(type==MT_Emote)
		type=4;

	if(!message1)
	{
		Message_StringID(type, string_id);	// use the simple message instead
		return;
	}

	i = 0;
	message_arg[i++] = message1;
	message_arg[i++] = message2;
	message_arg[i++] = message3;
	message_arg[i++] = message4;
	message_arg[i++] = message5;
	message_arg[i++] = message6;
	message_arg[i++] = message7;
	message_arg[i++] = message8;
	message_arg[i++] = message9;

	for(argcount = length = 0; message_arg[argcount]; argcount++)
		length += strlen(message_arg[argcount]) + 1;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_FormattedMessage, length+13);
	FormattedMessage_Struct *fm = (FormattedMessage_Struct *)outapp->pBuffer;
	fm->string_id = string_id;
	fm->type = type;
	bufptr = fm->message;
	for(i = 0; i < argcount; i++)
	{
		strcpy(bufptr, message_arg[i]);
		bufptr += strlen(message_arg[i]) + 1;
	}


	if(distance>0)
		entity_list.QueueCloseClients(this,outapp,false,distance);
	else
		QueuePacket(outapp);
	safe_delete(outapp);
}


void Client::SetTint(int16 in_slot, uint32 color) {
	Color_Struct new_color;
	new_color.color = color;
	SetTint(in_slot, new_color);
}

// Still need to reconcile bracer01 versus bracer02
void Client::SetTint(int16 in_slot, Color_Struct& color) {
	if (in_slot==SLOT_HEAD)
		m_pp.item_tint[MATERIAL_HEAD].color=color.color;
	else if (in_slot==SLOT_ARMS)
		m_pp.item_tint[MATERIAL_ARMS].color=color.color;
	else if (in_slot==SLOT_BRACER01)
		m_pp.item_tint[MATERIAL_BRACER].color=color.color;
	else if (in_slot==SLOT_BRACER02)
		m_pp.item_tint[MATERIAL_BRACER].color=color.color;
	else if (in_slot==SLOT_HANDS)
		m_pp.item_tint[MATERIAL_HANDS].color=color.color;
	else if (in_slot==SLOT_PRIMARY)
		m_pp.item_tint[MATERIAL_PRIMARY].color=color.color;
	else if (in_slot==SLOT_SECONDARY)
		m_pp.item_tint[MATERIAL_SECONDARY].color=color.color;
	else if (in_slot==SLOT_CHEST)
		m_pp.item_tint[MATERIAL_CHEST].color=color.color;
	else if (in_slot==SLOT_LEGS)
		m_pp.item_tint[MATERIAL_LEGS].color=color.color;
	else if (in_slot==SLOT_FEET)
		m_pp.item_tint[MATERIAL_FEET].color=color.color;
}

void Client::SetHideMe(bool flag)
{
	EQApplicationPacket app;

	gmhideme = flag;

	if(gmhideme)
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

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_SkillUpdate, sizeof(SkillUpdate_Struct));
	SkillUpdate_Struct* skill = (SkillUpdate_Struct*)outapp->pBuffer;
	skill->skillId = 100 + langid;
	skill->value = m_pp.languages[langid];
	QueuePacket(outapp);
	safe_delete(outapp);

	Message_StringID( MT_Skills, LANG_SKILL_IMPROVED ); //Notify the client
}

void Client::LinkDead()
{
	if (GetGroup())
	{
		entity_list.MessageGroup(this,true,15,"%s has gone linkdead.",GetName());
		GetGroup()->DelMember(this);
	}
	Raid *raid = entity_list.GetRaidByClient(this);
	if(raid){
		raid->MemberZoned(this);
	}
//	save_timer.Start(2500);
	linkdead_timer.Start(RuleI(Zone,ClientLinkdeadMS));
	SendAppearancePacket(AT_Linkdead, 1);
	client_state = CLIENT_LINKDEAD;
	AI_Start(CLIENT_LD_TIMEOUT);
}

uint8 Client::SlotConvert(uint8 slot,bool bracer){
	uint8 slot2=0;
	if(bracer)
		return SLOT_BRACER02;
	switch(slot){
		case MATERIAL_HEAD:
			slot2=SLOT_HEAD;
			break;
		case MATERIAL_CHEST:
			slot2=SLOT_CHEST;
			break;
		case MATERIAL_ARMS:
			slot2=SLOT_ARMS;
			break;
		case MATERIAL_BRACER:
			slot2=SLOT_BRACER01;
			break;
		case MATERIAL_HANDS:
			slot2=SLOT_HANDS;
			break;
		case MATERIAL_LEGS:
			slot2=SLOT_LEGS;
			break;
		case MATERIAL_FEET:
			slot2=SLOT_FEET;
			break;
		}
	return slot2;
}

uint8 Client::SlotConvert2(uint8 slot){
	uint8 slot2=0;
	switch(slot){
		case SLOT_HEAD:
			slot2=MATERIAL_HEAD;
			break;
		case SLOT_CHEST:
			slot2=MATERIAL_CHEST;
			break;
		case SLOT_ARMS:
			slot2=MATERIAL_ARMS;
			break;
		case SLOT_BRACER01:
			slot2=MATERIAL_BRACER;
			break;
		case SLOT_HANDS:
			slot2=MATERIAL_HANDS;
			break;
		case SLOT_LEGS:
			slot2=MATERIAL_LEGS;
			break;
		case SLOT_FEET:
			slot2=MATERIAL_FEET;
			break;
		}
	return slot2;
}

void Client::Escape()
{
	entity_list.RemoveFromTargets(this, true);
	SetInvisible(1);

	Message_StringID(MT_Skills, ESCAPE);
}

float Client::CalcPriceMod(Mob* other, bool reverse)
{
	float chaformula = 0;
	if (other)
	{
		int factionlvl = GetFactionLevel(CharacterID(), other->CastToNPC()->GetNPCTypeID(), GetRace(), GetClass(), GetDeity(), other->CastToNPC()->GetPrimaryFaction(), other);
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
	if (Dist(*who) > 200)
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

	Message(0,"Your target is a level %i %s. It appears %s and %s for its level. It seems %s",who->GetLevel(),GetEQClassName(who->GetClass(),1),dmg,hitpoints,resists);
}

void Client::ChangeSQLLog(const char *file) {
	if(SQL_log != nullptr) {
		fclose(SQL_log);
		SQL_log = nullptr;
	}
	if(file != nullptr) {
		if(strstr(file, "..") != nullptr) {
			Message(13, ".. is forbibben in SQL log file names.");
			return;
		}
		char buf[512];
		snprintf(buf, 511, "%s%s", SQL_LOG_PATH, file);
		buf[511] = '\0';
		SQL_log = fopen(buf, "a");
		if(SQL_log == nullptr) {
			Message(13, "Unable to open SQL log file: %s\n", strerror(errno));
		}
	}
}

void Client::LogSQL(const char *fmt, ...) {
	if(SQL_log == nullptr)
		return;

	va_list argptr;
	va_start(argptr, fmt);
	vfprintf(SQL_log, fmt, argptr );
	fputc('\n', SQL_log);
	va_end(argptr);
}

void Client::GetGroupAAs(GroupLeadershipAA_Struct *into) const {
	memcpy(into, &m_pp.leader_abilities, sizeof(GroupLeadershipAA_Struct));
}

void Client::EnteringMessages(Client* client)
{
	//server rules
	char *rules;
	rules = new char [4096];

	if(database.GetVariable("Rules", rules, 4096))
	{
		uint8 flag = database.GetAgreementFlag(client->AccountID());
		if(!flag)
		{
			client->Message(13,"You must agree to the Rules, before you can move. (type #serverrules to view the rules)");
			client->Message(13,"You must agree to the Rules, before you can move. (type #serverrules to view the rules)");
			client->Message(13,"You must agree to the Rules, before you can move. (type #serverrules to view the rules)");
			client->SendAppearancePacket(AT_Anim, ANIM_FREEZE);
		}
	}
	safe_delete_array(rules);
}

void Client::SendRules(Client* client)
{
	char *rules;
	rules = new char [4096];
	char *ptr;

	database.GetVariable("Rules", rules, 4096);

	ptr = strtok(rules, "\n");
	while(ptr != nullptr)
	{

		client->Message(0,"%s",ptr);
		ptr = strtok(nullptr, "\n");
	}
	safe_delete_array(rules);
}

void Client::SetEndurance(int32 newEnd)
{
	/*Endurance can't be less than 0 or greater than max*/
	if(newEnd < 0)
		newEnd = 0;
	else if(newEnd > GetMaxEndurance()){
		newEnd = GetMaxEndurance();
	}

	cur_end = newEnd;
	SendManaUpdatePacket();
}

void Client::SacrificeConfirm(Client *caster) {

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_Sacrifice, sizeof(Sacrifice_Struct));
	Sacrifice_Struct *ss = (Sacrifice_Struct*)outapp->pBuffer;

	if(!caster || PendingSacrifice) return;

	if(GetLevel() < RuleI(Spells, SacrificeMinLevel)){
		caster->Message_StringID(13, SAC_TOO_LOW);	//This being is not a worthy sacrifice.
		return;
	}
	if (GetLevel() > RuleI(Spells, SacrificeMaxLevel)) {
		caster->Message_StringID(13, SAC_TOO_HIGH);
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
				if(GetLevel() >= RuleI(Spells, SacrificeMinLevel) && GetLevel() <= RuleI(Spells, SacrificeMaxLevel)){
					int exploss = (int)(GetLevel() * (GetLevel() / 18.0) * 12000);
					if(exploss < GetEXP()){
						SetEXP(GetEXP()-exploss, GetAAXP());
						SendLogoutPackets();

						//make our become corpse packet, and queue to ourself before OP_Death.
						EQApplicationPacket app2(OP_BecomeCorpse, sizeof(BecomeCorpse_Struct));
						BecomeCorpse_Struct* bc = (BecomeCorpse_Struct*)app2.pBuffer;
						bc->spawn_id = GetID();
						bc->x = GetX();
						bc->y = GetY();
						bc->z = GetZ();
						QueuePacket(&app2);

						// make death packet
						EQApplicationPacket app(OP_Death, sizeof(Death_Struct));
						Death_Struct* d = (Death_Struct*)app.pBuffer;
						d->spawn_id = GetID();
						d->killer_id = caster ? caster->GetID() : 0;
						d->bindzoneid = GetPP().binds[0].zoneId;
						d->spell_id = SPELL_UNKNOWN;
						d->attack_skill = 0xe7;
						d->damage = 0;
						app.priority = 6;
						entity_list.QueueClients(this, &app);

						BuffFadeAll();
						UnmemSpellAll();
						Group *g = GetGroup();
						if(g){
							g->MemberZoned(this);
						}
						Raid *r = entity_list.GetRaidByClient(this);
						if(r){
							r->MemberZoned(this);
						}
						ClearAllProximities();
						if(RuleB(Character, LeaveCorpses)){
							Corpse *new_corpse = new Corpse(this, 0);
							entity_list.AddCorpse(new_corpse, GetID());
							SetID(0);
							entity_list.QueueClients(this, &app2, true);
						}
						Save();
						GoToDeath();
						caster->SummonItem(RuleI(Spells, SacrificeItemID));
					}
				}
				else{
					caster->Message_StringID(13, SAC_TOO_LOW);	//This being is not a worthy sacrifice.
				}
}

void Client::SendOPTranslocateConfirm(Mob *Caster, uint16 SpellID) {

	if(!Caster || PendingTranslocate) return;

	const SPDat_Spell_Struct &Spell = spells[SpellID];

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_Translocate, sizeof(Translocate_Struct));
	Translocate_Struct *ts = (Translocate_Struct*)outapp->pBuffer;

	strcpy(ts->Caster, Caster->GetName());
	ts->SpellID = SpellID;

	if((SpellID == 1422) || (SpellID == 1334) || (SpellID == 3243)) {
		ts->ZoneID = m_pp.binds[0].zoneId;
		ts->x = m_pp.binds[0].x;
		ts->y = m_pp.binds[0].y;
		ts->z = m_pp.binds[0].z;
	}
	else {
		ts->ZoneID = database.GetZoneID(Spell.teleport_zone);
		ts->y = Spell.base[0];
		ts->x = Spell.base[1];
		ts->z = Spell.base[2];
	}

	ts->unknown008 = 0;
	ts->Complete = 0;

	PendingTranslocateData = *ts;
	PendingTranslocate=true;
	TranslocateTime = time(nullptr);

	QueuePacket(outapp);
	safe_delete(outapp);

	return;
}
void Client::SendPickPocketResponse(Mob *from, uint32 amt, int type, const Item_Struct* item){
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_PickPocket, sizeof(sPickPocket_Struct));
		sPickPocket_Struct* pick_out = (sPickPocket_Struct*) outapp->pBuffer;
		pick_out->coin = amt;
		pick_out->from = GetID();
		pick_out->to = from->GetID();
		pick_out->myskill = GetSkill(PICK_POCKETS);

		if((type >= PickPocketPlatinum) && (type <= PickPocketCopper) && (amt == 0))
			type = PickPocketFailed;

		pick_out->type = type;
		if(item)
			strcpy(pick_out->itemname, item->Name);
		else
			pick_out->itemname[0] = '\0';
		//if we do not send this packet the client will lock up and require the player to relog.
		QueuePacket(outapp);
		safe_delete(outapp);
}

void Client::SetHoTT(uint32 mobid) {
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_TargetHoTT, sizeof(ClientTarget_Struct));
	ClientTarget_Struct *ct = (ClientTarget_Struct *) outapp->pBuffer;
	ct->new_target = mobid;
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendPopupToClient(const char *Title, const char *Text, uint32 PopupID, uint32 Buttons, uint32 Duration) {

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_OnLevelMessage, sizeof(OnLevelMessage_Struct));
	OnLevelMessage_Struct *olms = (OnLevelMessage_Struct *) outapp->pBuffer;

	if((strlen(Title) > (sizeof(olms->Title)-1)) ||
		(strlen(Text) > (sizeof(olms->Text)-1))) return;

	strcpy(olms->Title, Title);
	strcpy(olms->Text, Text);

	olms->Buttons = Buttons;

	if(Duration > 0)
		olms->Duration = Duration * 1000;
	else
		olms->Duration = 0xffffffff;

	olms->PopupID = PopupID;
	olms->NegativeID = 0;

	sprintf(olms->ButtonName0, "%s", "Yes");
	sprintf(olms->ButtonName1, "%s", "No");
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendWindow(uint32 PopupID, uint32 NegativeID, uint32 Buttons, const char *ButtonName0, const char *ButtonName1, uint32 Duration, int title_type, Client* target, const char *Title, const char *Text, ...) {
	va_list argptr;
	char buffer[4096];

	va_start(argptr, Text);
	vsnprintf(buffer, sizeof(buffer), Text, argptr);
	va_end(argptr);

	size_t len = strlen(buffer);

	EQApplicationPacket* app = new EQApplicationPacket(OP_OnLevelMessage, sizeof(OnLevelMessage_Struct));
	OnLevelMessage_Struct* olms=(OnLevelMessage_Struct*)app->pBuffer;

	if(strlen(Text) > (sizeof(olms->Text)-1))
		return;

	if(!target)
		title_type = 0;

	switch (title_type)
	{
		case 1: {
			char name[64] = "";
			strcpy(name, target->GetName());
			if(target->GetLastName()) {
				char last_name[64] = "";
				strcpy(last_name, target->GetLastName());
				strcat(name, " ");
				strcat(name, last_name);
			}
			strcpy(olms->Title, name);
			break;
		}
		case 2: {
			if(target->GuildID()) {
				char *guild_name = (char*)guild_mgr.GetGuildName(target->GuildID());
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

	memcpy(olms->Text, buffer, len+1);

	olms->Buttons = Buttons;

	sprintf(olms->ButtonName0, "%s", ButtonName0);
	sprintf(olms->ButtonName1, "%s", ButtonName1);

	if(Duration > 0)
		olms->Duration = Duration * 1000;
	else
		olms->Duration = 0xffffffff;

	olms->PopupID = PopupID;
	olms->NegativeID = NegativeID;

	FastQueuePacket(&app);
}

void Client::KeyRingLoad()
{
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	query = new char[256];

	sprintf(query, "SELECT item_id FROM keyring WHERE char_id='%i' ORDER BY item_id",character_id);
	if (database.RunQuery(query, strlen(query), errbuf, &result))
	{
		safe_delete_array(query);
		while(0 != (row = mysql_fetch_row(result))){
			keyring.push_back(atoi(row[0]));
		}
		mysql_free_result(result);
	}else {
		std::cerr << "Error in Client::KeyRingLoad query '" << query << "' " << errbuf << std::endl;
		safe_delete_array(query);
		return;
	}
}

void Client::KeyRingAdd(uint32 item_id)
{
	if(0==item_id)return;
	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	uint32 affected_rows = 0;
	query = new char[256];
	bool bFound = KeyRingCheck(item_id);
	if(!bFound){
		sprintf(query, "INSERT INTO keyring(char_id,item_id) VALUES(%i,%i)",character_id,item_id);
		if(database.RunQuery(query, strlen(query), errbuf, 0, &affected_rows))
		{
			Message(4,"Added to keyring.");
			safe_delete_array(query);
		}
		else
		{
			std::cerr << "Error in Doors::HandleClick query '" << query << "' " << errbuf << std::endl;
			safe_delete_array(query);
			return;
		}
		keyring.push_back(item_id);
	}
}

bool Client::KeyRingCheck(uint32 item_id)
{
	for(std::list<uint32>::iterator iter = keyring.begin();
		iter != keyring.end();
		++iter)
	{
		if(*iter == item_id)
			return true;
	}
	return false;
}

void Client::KeyRingList()
{
	Message(4,"Keys on Keyring:");
	const Item_Struct *item = 0;
	for(std::list<uint32>::iterator iter = keyring.begin();
		iter != keyring.end();
		++iter)
	{
		if ((item = database.GetItem(*iter))!=nullptr) {
			Message(4,item->Name);
		}
	}
}

bool Client::IsDiscovered(uint32 itemid) {

	char errbuf[MYSQL_ERRMSG_SIZE];
	char *query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if (database.RunQuery(query, MakeAnyLenString(&query, "SELECT count(*) FROM discovered_items WHERE item_id = '%lu'", itemid), errbuf, &result))
	{
		row = mysql_fetch_row(result);
		if (atoi(row[0]))
		{
			mysql_free_result(result);
			safe_delete_array(query);
			return true;
		}
	}
	else
	{
		std::cerr << "Error in IsDiscovered query '" << query << "' " << errbuf << std::endl;
	}
	mysql_free_result(result);
	safe_delete_array(query);
	return false;
}

void Client::DiscoverItem(uint32 itemid) {

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	if (database.RunQuery(query,MakeAnyLenString(&query, "INSERT INTO discovered_items SET item_id=%lu, char_name='%s', discovered_date=UNIX_TIMESTAMP(), account_status=%i", itemid, GetName(), Admin()), errbuf, &result))
	{
		mysql_free_result(result);
	}
	safe_delete_array(query);

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

uint16 Client::GetPrimarySkillValue()
{
	SkillType skill = HIGHEST_SKILL; //because nullptr == 0, which is 1H Slashing, & we want it to return 0 from GetSkill
	bool equiped = m_inv.GetItem(13);

	if (!equiped)
		skill = HAND_TO_HAND;

	else {

		uint8 type = m_inv.GetItem(13)->GetItem()->ItemType; //is this the best way to do this?

		switch (type)
		{
			case ItemType1HS: // 1H Slashing
			{
				skill = _1H_SLASHING;
				break;
			}
			case ItemType2HS: // 2H Slashing
			{
				skill = _2H_SLASHING;
				break;
			}
			case ItemTypePierce: // Piercing
			{
				skill = PIERCING;
				break;
			}
			case ItemType1HB: // 1H Blunt
			{
				skill = _1H_BLUNT;
				break;
			}
			case ItemType2HB: // 2H Blunt
			{
				skill = _2H_BLUNT;
				break;
			}
			case ItemType2HPierce: // 2H Piercing
			{
				skill = PIERCING;
				break;
			}
			case ItemTypeHand2Hand: // Hand to Hand
			{
				skill = HAND_TO_HAND;
				break;
			}
			default: // All other types default to Hand to Hand
			{
				skill = HAND_TO_HAND;
				break;
			}
		}
	}

	return GetSkill(skill);
}

uint16 Client::GetTotalATK()
{
	uint16 AttackRating = 0;
	uint16 WornCap = itembonuses.ATK;

	if(IsClient()) {
		AttackRating = ((WornCap * 1.342) + (GetSkill(OFFENSE) * 1.345) + ((GetSTR() - 66) * 0.9) + (GetPrimarySkillValue() * 2.69));
		AttackRating += aabonuses.ATK + GroupLeadershipAAOffenseEnhancement();

		if (AttackRating < 10)
			AttackRating = 10;
	}
	else
		AttackRating = GetATK();

	AttackRating += spellbonuses.ATK;

	return AttackRating;
}

uint16 Client::GetATKRating()
{
	uint16 AttackRating = 0;
	if(IsClient()) {
		AttackRating = (GetSkill(OFFENSE) * 1.345) + ((GetSTR() - 66) * 0.9) + (GetPrimarySkillValue() * 2.69);

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
}

void Client::UpdateGroupAAs(int32 points, uint32 type) {

	switch(type)
	{
	case 0:
		{
		m_pp.group_leadership_points += points;
		break;
		}
	case 1:
		{
		m_pp.raid_leadership_points += points;
		break;
		}
	}
	SendLeadershipEXPUpdate();
}

bool Client::IsLeadershipEXPOn()
{

	if(!m_pp.leadAAActive)
		return false;

	Group *g = GetGroup();

	if(g && g->IsLeader(this) && (g->GroupCount() > 2))
		return true;

	Raid *r = GetRaid();

	if(r && r->IsLeader(this) && (r->RaidCount() > 17))
		return true;

	return false;

}

int Client::GetAggroCount() {
	return AggroCount;
}

void Client::IncrementAggroCount() {

	// This method is called when a client is added to a mob's hate list. It turns the clients aggro flag on so
	// rest state regen is stopped, and for SoF, it sends the opcode to show the crossed swords in-combat indicator.
	//
	//
	AggroCount++;

	if(!RuleI(Character, RestRegenPercent))
		return;

	// If we already had aggro before this method was called, the combat indicator should already be up for SoF clients,
	// so we don't need to send it again.
	//
	if(AggroCount > 1)
		return;

	if(GetClientVersion() >= EQClientSoF) {

		EQApplicationPacket *outapp = new EQApplicationPacket(OP_RestState, 1);
		char *Buffer = (char *)outapp->pBuffer;
		VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0x01);
		QueuePacket(outapp);
		safe_delete(outapp);
	}

}

void Client::DecrementAggroCount() {

	// This should be called when a client is removed from a mob's hate list (it dies or is memblurred).
	// It checks whether any other mob is aggro on the player, and if not, starts the rest timer.
	// For SoF, the opcode to start the rest state countdown timer in the UI is sent.
	//

	// If we didn't have aggro before, this method should not have been called.
	if(!AggroCount)
		return;

	AggroCount--;

	if(!RuleI(Character, RestRegenPercent))
		return;

	// Something else is still aggro on us, can't rest yet.
	if(AggroCount) return;

	rest_timer.Start(RuleI(Character, RestRegenTimeToActivate) * 1000);

	if(GetClientVersion() >= EQClientSoF) {

		EQApplicationPacket *outapp = new EQApplicationPacket(OP_RestState, 5);
		char *Buffer = (char *)outapp->pBuffer;
		VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0x00);
		VARSTRUCT_ENCODE_TYPE(uint32, Buffer, RuleI(Character, RestRegenTimeToActivate));
		QueuePacket(outapp);
		safe_delete(outapp);
	}
}

void Client::SendPVPStats()
{
	// This sends the data to the client to populate the PVP Stats Window.
	//
	// When the PVP Stats window is opened, no opcode is sent. Therefore this method should be called
	// from Client::CompleteConnect, and also when the player makes a PVP kill.
	//
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_PVPStats, sizeof(PVPStats_Struct));
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
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_CrystalCountUpdate, sizeof(CrystalCountUpdate_Struct));
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

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_DisciplineTimer, sizeof(DisciplineTimer_Struct));
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


	const char* BindName = "Bind Location";
	const char* Resurrect = "Resurrect";

	int PacketLength;

	PacketLength = 17 + (26 * 2) + strlen(BindName) + strlen(Resurrect);	// SoF

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_RespawnWindow, PacketLength);

	char *Buffer = (char *)outapp->pBuffer;

	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);	// Unknown
	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, RuleI(Character, RespawnFromHoverTimer) * 1000);
	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);	// Unknown
	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 2);	// Two options, Bind or Rez


	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);	// Entry 0
	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, m_pp.binds[0].zoneId);
	VARSTRUCT_ENCODE_TYPE(float, Buffer, m_pp.binds[0].x);
	VARSTRUCT_ENCODE_TYPE(float, Buffer, m_pp.binds[0].y);
	VARSTRUCT_ENCODE_TYPE(float, Buffer, m_pp.binds[0].z);
	VARSTRUCT_ENCODE_TYPE(float, Buffer, m_pp.binds[0].heading);
	VARSTRUCT_ENCODE_STRING(Buffer, BindName);
	VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);

	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 1);	// Entry 1
	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, zone->GetZoneID());
	VARSTRUCT_ENCODE_TYPE(float, Buffer, GetX());
	VARSTRUCT_ENCODE_TYPE(float, Buffer, GetY());
	VARSTRUCT_ENCODE_TYPE(float, Buffer, GetZ());
	VARSTRUCT_ENCODE_TYPE(float, Buffer, GetHeading());
	VARSTRUCT_ENCODE_STRING(Buffer, Resurrect);
	VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 1);

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
			LogFile->write(EQEMuLog::Debug, "%s tried to open %s but %s was not a treasure chest.",
				GetName(), target->GetName(), target->GetName());
			return;
		}

		if(DistNoRootNoZ(*target) > RuleI(Adventure, LDoNTrapDistanceUse))
		{
			LogFile->write(EQEMuLog::Debug, "%s tried to open %s but %s was out of range",
				GetName(), target->GetName(), target->GetName());
			Message(13, "Treasure chest out of range.");
			return;
		}

		if(target->IsLDoNTrapped())
		{
			if(target->GetLDoNTrapSpellID() != 0)
			{
				Message_StringID(13, LDON_ACCIDENT_SETOFF2);
				target->SpellFinished(target->GetLDoNTrapSpellID(), this, 10, 0, -1, spells[target->GetLDoNTrapSpellID()].ResistDiff);
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
			Message_StringID(MT_Skills, LDON_STILL_LOCKED, target->GetCleanName());
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
			target->Death(this, 1, SPELL_UNKNOWN, HAND_TO_HAND);
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
				Message_StringID(MT_Skills, LDON_CANT_DETERMINE_TRAP, target->GetCleanName());
				return;
			}

			if(target->IsLDoNTrapDetected())
			{
				Message_StringID(MT_Skills, LDON_CERTAIN_TRAP, target->GetCleanName());
			}
			else
			{
				int check = LDoNChest_SkillCheck(target, skill);
				switch(check)
				{
				case -1:
				case 0:
					Message_StringID(MT_Skills, LDON_DONT_KNOW_TRAPPED, target->GetCleanName());
					break;
				case 1:
					Message_StringID(MT_Skills, LDON_CERTAIN_TRAP, target->GetCleanName());
					target->SetLDoNTrapDetected(true);
					break;
				default:
					break;
				}
			}
		}
		else
		{
			Message_StringID(MT_Skills, LDON_CERTAIN_NOT_TRAP, target->GetCleanName());
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
				Message_StringID(MT_Skills, LDON_WAS_NOT_TRAPPED, target->GetCleanName());
				return;
			}

			if((target->GetLDoNTrapType() == LDoNTypeCursed || target->GetLDoNTrapType() == LDoNTypeMagical) && type != target->GetLDoNTrapType())
			{
				Message_StringID(MT_Skills, LDON_HAVE_NOT_DISARMED, target->GetCleanName());
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
				Message_StringID(MT_Skills, LDON_HAVE_DISARMED, target->GetCleanName());
				break;
			case 0:
				Message_StringID(MT_Skills, LDON_HAVE_NOT_DISARMED, target->GetCleanName());
				break;
			case -1:
				Message_StringID(13, LDON_ACCIDENT_SETOFF2);
				target->SpellFinished(target->GetLDoNTrapSpellID(), this, 10, 0, -1, spells[target->GetLDoNTrapSpellID()].ResistDiff);
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
				Message_StringID(13, LDON_ACCIDENT_SETOFF2);
				target->SpellFinished(target->GetLDoNTrapSpellID(), this, 10, 0, -1, spells[target->GetLDoNTrapSpellID()].ResistDiff);
				target->SetLDoNTrapSpellID(0);
				target->SetLDoNTrapped(false);
				target->SetLDoNTrapDetected(false);
			}

			if(!target->IsLDoNLocked())
			{
				Message_StringID(MT_Skills, LDON_WAS_NOT_LOCKED, target->GetCleanName());
				return;
			}

			if((target->GetLDoNTrapType() == LDoNTypeCursed || target->GetLDoNTrapType() == LDoNTypeMagical) && type != target->GetLDoNTrapType())
			{
				Message(MT_Skills, "You cannot unlock %s with this skill.", target->GetCleanName());
				return;
			}

			int check = LDoNChest_SkillCheck(target, skill);

			switch(check)
			{
			case 0:
			case -1:
				Message_StringID(MT_Skills, LDON_PICKLOCK_FAILURE, target->GetCleanName());
				break;
			case 1:
				target->SetLDoNLocked(false);
				Message_StringID(MT_Skills, LDON_PICKLOCK_SUCCESS, target->GetCleanName());
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

	float d100 = (float)MakeRandomFloat(0, 100);

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

	ServerPacket *Pack = new ServerPacket(ServerOP_DepopAllPlayersCorpses, sizeof(ServerDepopAllPlayersCorpses_Struct));

	ServerDepopAllPlayersCorpses_Struct *sdapcs = (ServerDepopAllPlayersCorpses_Struct*)Pack->pBuffer;

	sdapcs->CharacterID = CharacterID();
	sdapcs->ZoneID = zone->GetZoneID();
	sdapcs->InstanceID = zone->GetInstanceID();

	worldserver.SendPacket(Pack);

	safe_delete(Pack);

	entity_list.RemoveAllCorpsesByCharID(CharacterID());

	int CorpseCount = database.SummonAllPlayerCorpses(CharacterID(), zone->GetZoneID(), zone->GetInstanceID(),
								GetX(), GetY(), GetZ(), GetHeading());
	if(CorpseCount <= 0)
	{
		Message(clientMessageYellow, "You have no corpses to summnon.");
		return;
	}

	int RezzExp = entity_list.RezzAllCorpsesByCharID(CharacterID());

	if(RezzExp > 0)
		SetEXP(GetEXP() + RezzExp, GetAAXP(), true);

	Message(clientMessageYellow, "All your corpses have been summoned to your feet and have received a 100% resurrection.");
}

void Client::SummonAllCorpses(float dest_x, float dest_y, float dest_z, float dest_heading)
{

	if(dest_x == 0 && dest_y == 0 && dest_z == 0 && dest_heading == 0)
	{
		dest_x = GetX(); dest_y = GetY(); dest_z = GetZ(); dest_heading = GetHeading();
	}

	ServerPacket *Pack = new ServerPacket(ServerOP_DepopAllPlayersCorpses, sizeof(ServerDepopAllPlayersCorpses_Struct));

	ServerDepopAllPlayersCorpses_Struct *sdapcs = (ServerDepopAllPlayersCorpses_Struct*)Pack->pBuffer;

	sdapcs->CharacterID = CharacterID();
	sdapcs->ZoneID = zone->GetZoneID();
	sdapcs->InstanceID = zone->GetInstanceID();

	worldserver.SendPacket(Pack);

	safe_delete(Pack);

	entity_list.RemoveAllCorpsesByCharID(CharacterID());

	int CorpseCount = database.SummonAllPlayerCorpses(CharacterID(), zone->GetZoneID(), zone->GetInstanceID(),
								dest_x, dest_y, dest_z, dest_heading);
	if(CorpseCount <= 0)
	{
		return;
	}
}

void Client::DepopAllCorpses()
{
	ServerPacket *Pack = new ServerPacket(ServerOP_DepopAllPlayersCorpses, sizeof(ServerDepopAllPlayersCorpses_Struct));

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
	ServerPacket *Pack = new ServerPacket(ServerOP_DepopPlayerCorpse, sizeof(ServerDepopPlayerCorpse_Struct));

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
	database.BuryAllPlayerCorpses(CharacterID());
}

void Client::NotifyNewTitlesAvailable()
{
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_NewTitlesAvailable, 0);

	QueuePacket(outapp);

	safe_delete(outapp);

}

void Client::SetStartZone(uint32 zoneid, float x, float y, float z)
{
	// setting city to zero allows the player to use /setstartcity to set the city themselves
	if(zoneid == 0) {
		m_pp.binds[4].zoneId = 0;
		this->Message(15,"Your starting city has been reset. Use /setstartcity to choose a new one");
		return;
	}

	// check to make sure the zone is valid
	const char *target_zone_name = database.GetZoneName(zoneid);
	if(target_zone_name == nullptr)
		return;

	m_pp.binds[4].zoneId = zoneid;
	if (x == 0 && y == 0 && z ==0)
		database.GetSafePoints(m_pp.binds[4].zoneId, 0, &m_pp.binds[4].x, &m_pp.binds[4].y, &m_pp.binds[4].z);
	else {
		m_pp.binds[4].x = x;
		m_pp.binds[4].y = y;
		m_pp.binds[4].z = z;
	}
}

uint32 Client::GetStartZone()
{
	return m_pp.binds[4].zoneId;
}

void Client::ShowSkillsWindow()
{
	const char *WindowTitle = "Skills";
	std::string WindowText;
	// using a map for easy alphabetizing of the skills list
	std::map<std::string, SkillType> Skills;
	std::map<std::string, SkillType>::iterator it;

	// this list of names must keep the same order as that in common/skills.h
	const char* SkillName[] = {"1H Blunt","1H Slashing","2H Blunt","2H Slashing","Abjuration","Alteration","Apply Poison","Archery",
		"Backstab","Bind Wound","Bash","Block","Brass Instruments","Channeling","Conjuration","Defense","Disarm","Disarm Traps","Divination",
		"Dodge","Double Attack","Dragon Punch","Dual Wield","Eagle Strike","Evocation","Feign Death","Flying Kick","Forage","Hand to Hand",
		"Hide","Kick","Meditate","Mend","Offense","Parry","Pick Lock","Piercing","Ripost","Round Kick","Safe Fall","Sense Heading",
		"Singing","Sneak","Specialize Abjuration","Specialize Alteration","Specialize Conjuration","Specialize Divination","Specialize Evocation","Pick Pockets",
		"Stringed Instruments","Swimming","Throwing","Tiger Claw","Tracking","Wind Instruments","Fishing","Make Poison","Tinkering","Research",
		"Alchemy","Baking","Tailoring","Sense Traps","Blacksmithing","Fletching","Brewing","Alcohol Tolerance","Begging","Jewelry Making",
		"Pottery","Percussion Instruments","Intimidation","Berserking","Taunt","Frenzy"};
	for(int i = 0; i <= (int)HIGHEST_SKILL; i++)
		Skills[SkillName[i]] = (SkillType)i;

	// print out all available skills
	for(it = Skills.begin(); it != Skills.end(); it++) {
		if(GetSkill(it->second) > 0 || MaxSkill(it->second) > 0) {
			WindowText += it->first;
			// line up the values
			for (int j = 0; j < 5; j++)
				WindowText += "&nbsp;";
			WindowText += itoa(this->GetSkill(it->second));
			if (MaxSkill(it->second) > 0) {
				WindowText += "/";
				WindowText += itoa(this->GetMaxSkillAfterSpecializationRules(it->second,this->MaxSkill(it->second)));
			}
			WindowText += "<br>";
		}
	}
	this->SendPopupToClient(WindowTitle, WindowText.c_str());
}


void Client::SetShadowStepExemption(bool v)
{
	if(v == true)
	{
		uint32 cur_time = Timer::GetCurrentTime();
		if((cur_time - m_TimeSinceLastPositionCheck) > 1000)
		{
			float speed = (m_DistanceSinceLastPositionCheck * 100) / (float)(cur_time - m_TimeSinceLastPositionCheck);
			float runs = GetRunspeed();
			if(speed > (runs * RuleR(Zone, MQWarpDetectionDistanceFactor)))
			{
				printf("%s %i moving too fast! moved: %.2f in %ims, speed %.2f\n", __FILE__, __LINE__,
					m_DistanceSinceLastPositionCheck, (cur_time - m_TimeSinceLastPositionCheck), speed);
				if(!GetGMSpeed() && (runs >= GetBaseRunspeed() || (speed > (GetBaseRunspeed() * RuleR(Zone, MQWarpDetectionDistanceFactor)))))
				{
					if(IsShadowStepExempted())
					{
						if(m_DistanceSinceLastPositionCheck > 800)
						{
							CheatDetected(MQWarpShadowStep, GetX(), GetY(), GetZ());
						}
					}
					else if(IsKnockBackExempted())
					{
						//still potential to trigger this if you're knocked back off a
						//HUGE fall that takes > 2.5 seconds
						if(speed > 30.0f)
						{
							CheatDetected(MQWarpKnockBack, GetX(), GetY(), GetZ());
						}
					}
					else if(!IsPortExempted())
					{
						if(!IsMQExemptedArea(zone->GetZoneID(), GetX(), GetY(), GetZ()))
						{
							if(speed > (runs * 2 * RuleR(Zone, MQWarpDetectionDistanceFactor)))
							{
								CheatDetected(MQWarp, GetX(), GetY(), GetZ());
								m_TimeSinceLastPositionCheck = cur_time;
								m_DistanceSinceLastPositionCheck = 0.0f;
								//Death(this, 10000000, SPELL_UNKNOWN, _1H_BLUNT);
							}
							else
							{
								CheatDetected(MQWarpLight, GetX(), GetY(), GetZ());
							}
						}
					}
				}
			}
		}
		m_TimeSinceLastPositionCheck = cur_time;
		m_DistanceSinceLastPositionCheck = 0.0f;
	}
	m_ShadowStepExemption = v;
}

void Client::SetKnockBackExemption(bool v)
{
	if(v == true)
	{
		uint32 cur_time = Timer::GetCurrentTime();
		if((cur_time - m_TimeSinceLastPositionCheck) > 1000)
		{
			float speed = (m_DistanceSinceLastPositionCheck * 100) / (float)(cur_time - m_TimeSinceLastPositionCheck);
			float runs = GetRunspeed();
			if(speed > (runs * RuleR(Zone, MQWarpDetectionDistanceFactor)))
			{
				if(!GetGMSpeed() && (runs >= GetBaseRunspeed() || (speed > (GetBaseRunspeed() * RuleR(Zone, MQWarpDetectionDistanceFactor)))))
				{
					printf("%s %i moving too fast! moved: %.2f in %ims, speed %.2f\n", __FILE__, __LINE__,
					m_DistanceSinceLastPositionCheck, (cur_time - m_TimeSinceLastPositionCheck), speed);
					if(IsShadowStepExempted())
					{
						if(m_DistanceSinceLastPositionCheck > 800)
						{
							CheatDetected(MQWarpShadowStep, GetX(), GetY(), GetZ());
						}
					}
					else if(IsKnockBackExempted())
					{
						//still potential to trigger this if you're knocked back off a
						//HUGE fall that takes > 2.5 seconds
						if(speed > 30.0f)
						{
							CheatDetected(MQWarpKnockBack, GetX(), GetY(), GetZ());
						}
					}
					else if(!IsPortExempted())
					{
						if(!IsMQExemptedArea(zone->GetZoneID(), GetX(), GetY(), GetZ()))
						{
							if(speed > (runs * 2 * RuleR(Zone, MQWarpDetectionDistanceFactor)))
							{
								m_TimeSinceLastPositionCheck = cur_time;
								m_DistanceSinceLastPositionCheck = 0.0f;
								CheatDetected(MQWarp, GetX(), GetY(), GetZ());
								//Death(this, 10000000, SPELL_UNKNOWN, _1H_BLUNT);
							}
							else
							{
								CheatDetected(MQWarpLight, GetX(), GetY(), GetZ());
							}
						}
					}
				}
			}
		}
		m_TimeSinceLastPositionCheck = cur_time;
		m_DistanceSinceLastPositionCheck = 0.0f;
	}
	m_KnockBackExemption = v;
}

void Client::SetPortExemption(bool v)
{
	if(v == true)
	{
		uint32 cur_time = Timer::GetCurrentTime();
		if((cur_time - m_TimeSinceLastPositionCheck) > 1000)
		{
			float speed = (m_DistanceSinceLastPositionCheck * 100) / (float)(cur_time - m_TimeSinceLastPositionCheck);
			float runs = GetRunspeed();
			if(speed > (runs * RuleR(Zone, MQWarpDetectionDistanceFactor)))
			{
				if(!GetGMSpeed() && (runs >= GetBaseRunspeed() || (speed > (GetBaseRunspeed() * RuleR(Zone, MQWarpDetectionDistanceFactor)))))
				{
					printf("%s %i moving too fast! moved: %.2f in %ims, speed %.2f\n", __FILE__, __LINE__,
					m_DistanceSinceLastPositionCheck, (cur_time - m_TimeSinceLastPositionCheck), speed);
					if(IsShadowStepExempted())
					{
						if(m_DistanceSinceLastPositionCheck > 800)
						{
								CheatDetected(MQWarpShadowStep, GetX(), GetY(), GetZ());
						}
					}
					else if(IsKnockBackExempted())
					{
						//still potential to trigger this if you're knocked back off a
						//HUGE fall that takes > 2.5 seconds
						if(speed > 30.0f)
						{
							CheatDetected(MQWarpKnockBack, GetX(), GetY(), GetZ());
						}
					}
					else if(!IsPortExempted())
					{
						if(!IsMQExemptedArea(zone->GetZoneID(), GetX(), GetY(), GetZ()))
						{
							if(speed > (runs * 2 * RuleR(Zone, MQWarpDetectionDistanceFactor)))
							{
								m_TimeSinceLastPositionCheck = cur_time;
								m_DistanceSinceLastPositionCheck = 0.0f;
								CheatDetected(MQWarp, GetX(), GetY(), GetZ());
								//Death(this, 10000000, SPELL_UNKNOWN, _1H_BLUNT);
							}
							else
							{
								CheatDetected(MQWarpLight, GetX(), GetY(), GetZ());
							}
						}
					}
				}
			}
		}
		m_TimeSinceLastPositionCheck = cur_time;
		m_DistanceSinceLastPositionCheck = 0.0f;
	}
	m_PortExemption = v;
}

void Client::Signal(uint32 data)
{
	char buf[32];
	snprintf(buf, 31, "%d", data);
	buf[31] = '\0';
	parse->EventPlayer(EVENT_SIGNAL, this, buf, 0);
}

const bool Client::IsMQExemptedArea(uint32 zoneID, float x, float y, float z) const
{
	float max_dist = 90000;
	switch(zoneID)
	{
	case 2:
		{
			float delta = (x-(-713.6));
			delta *= delta;
			float distance = delta;
			delta = (y-(-160.2));
			delta *= delta;
			distance += delta;
			delta = (z-(-12.8));
			delta *= delta;
			distance += delta;

			if(distance < max_dist)
				return true;

			delta = (x-(-153.8));
			delta *= delta;
			distance = delta;
			delta = (y-(-30.3));
			delta *= delta;
			distance += delta;
			delta = (z-(8.2));
			delta *= delta;
			distance += delta;

			if(distance < max_dist)
				return true;

			break;
		}
	case 9:
	{
		float delta = (x-(-682.5));
		delta *= delta;
		float distance = delta;
		delta = (y-(147.0));
		delta *= delta;
		distance += delta;
		delta = (z-(-9.9));
		delta *= delta;
		distance += delta;

		if(distance < max_dist)
			return true;

		delta = (x-(-655.4));
		delta *= delta;
		distance = delta;
		delta = (y-(10.5));
		delta *= delta;
		distance += delta;
		delta = (z-(-51.8));
		delta *= delta;
		distance += delta;

		if(distance < max_dist)
			return true;

		break;
	}
	case 62:
	case 75:
	case 114:
	case 209:
	{
		//The portals are so common in paineel/felwitheb that checking
		//distances wouldn't be worth it cause unless you're porting to the
		//start field you're going to be triggering this and that's a level of
		//accuracy I'm willing to sacrifice
		return true;
		break;
	}

	case 24:
	{
		float delta = (x-(-183.0));
		delta *= delta;
		float distance = delta;
		delta = (y-(-773.3));
		delta *= delta;
		distance += delta;
		delta = (z-(54.1));
		delta *= delta;
		distance += delta;

		if(distance < max_dist)
			return true;

		delta = (x-(-8.8));
		delta *= delta;
		distance = delta;
		delta = (y-(-394.1));
		delta *= delta;
		distance += delta;
		delta = (z-(41.1));
		delta *= delta;
		distance += delta;

		if(distance < max_dist)
			return true;

		delta = (x-(-310.3));
		delta *= delta;
		distance = delta;
		delta = (y-(-1411.6));
		delta *= delta;
		distance += delta;
		delta = (z-(-42.8));
		delta *= delta;
		distance += delta;

		if(distance < max_dist)
			return true;

		delta = (x-(-183.1));
		delta *= delta;
		distance = delta;
		delta = (y-(-1409.8));
		delta *= delta;
		distance += delta;
		delta = (z-(37.1));
		delta *= delta;
		distance += delta;

		if(distance < max_dist)
			return true;

		break;
	}

	case 110:
	case 34:
	case 96:
	case 93:
	case 68:
	case 84:
		{
			if(GetBoatID() != 0)
				return true;
			break;
		}
	default:
		break;
	}
	return false;
}

void Client::SendRewards()
{
	std::vector<ClientReward> rewards;
	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;

	if(database.RunQuery(query,MakeAnyLenString(&query,"SELECT reward_id, amount FROM"
		" account_rewards WHERE account_id=%i ORDER by reward_id", AccountID()),
		errbuf,&result))
	{
		while((row = mysql_fetch_row(result)))
		{
			ClientReward cr;
			cr.id = atoi(row[0]);
			cr.amount = atoi(row[1]);
			rewards.push_back(cr);
		}
		mysql_free_result(result);
		safe_delete_array(query);
	}
	else
	{
		LogFile->write(EQEMuLog::Error, "Error in Client::SendRewards(): %s (%s)", query, errbuf);
		safe_delete_array(query);
		return;
	}

	if(rewards.size() > 0)
	{
		EQApplicationPacket *vetapp = new EQApplicationPacket(OP_VetRewardsAvaliable, (sizeof(InternalVeteranReward) * rewards.size()));
		uchar *data = vetapp->pBuffer;
		for(int i = 0; i < rewards.size(); ++i)
		{
			InternalVeteranReward *ivr = (InternalVeteranReward*)data;
			ivr->claim_id = rewards[i].id;
			ivr->number_available = rewards[i].amount;
			std::list<InternalVeteranReward>::iterator iter = zone->VeteranRewards.begin();
			while(iter != zone->VeteranRewards.end())
			{
				if((*iter).claim_id == rewards[i].id)
				{
					break;
				}
				iter++;
			}

			if(iter != zone->VeteranRewards.end())
			{
				InternalVeteranReward ivro = (*iter);
				ivr->claim_count = ivro.claim_count;
				for(int x = 0; x < ivro.claim_count; ++x)
				{
					ivr->items[x].item_id = ivro.items[x].item_id;
					ivr->items[x].charges = ivro.items[x].charges;
					strcpy(ivr->items[x].item_name, ivro.items[x].item_name);
				}
			}

			data += sizeof(InternalVeteranReward);
		}
		FastQueuePacket(&vetapp);
	}
}

bool Client::TryReward(uint32 claim_id)
{
	//Make sure we have an open spot
	//Make sure we have it in our acct and count > 0
	//Make sure the entry was found
	//If we meet all the criteria:
	//Decrement our count by 1 if it > 1 delete if it == 1
	//Create our item in bag if necessary at the free inv slot
	//save
	uint32 free_slot = 0xFFFFFFFF;

	for(int i = 22; i < 30; ++i)
	{
		ItemInst *item = GetInv().GetItem(i);
		if(!item)
		{
			free_slot = i;
			break;
		}
	}

	if(free_slot == 0xFFFFFFFF)
	{
		return false;
	}

	char errbuf[MYSQL_ERRMSG_SIZE];
	char* query = 0;
	MYSQL_RES *result;
	MYSQL_ROW row;
	uint32 amt = 0;

	if(database.RunQuery(query,MakeAnyLenString(&query,"SELECT amount FROM"
		" account_rewards WHERE account_id=%i AND reward_id=%i", AccountID(), claim_id),
		errbuf,&result))
	{
		row = mysql_fetch_row(result);
		if(row)
		{
			amt = atoi(row[0]);
		}
		else
		{
			mysql_free_result(result);
			safe_delete_array(query);
			return false;
		}
		mysql_free_result(result);
		safe_delete_array(query);
	}
	else
	{
		LogFile->write(EQEMuLog::Error, "Error in Client::TryReward(): %s (%s)", query, errbuf);
		safe_delete_array(query);
		return false;
	}

	if(amt == 0)
	{
		return false;
	}

	std::list<InternalVeteranReward>::iterator iter = zone->VeteranRewards.begin();
	while(iter != zone->VeteranRewards.end())
	{
		if((*iter).claim_id == claim_id)
		{
			break;
		}
		iter++;
	}

	if(iter == zone->VeteranRewards.end())
	{
		return false;
	}

	if(amt == 1)
	{
		if(!database.RunQuery(query,MakeAnyLenString(&query,"DELETE FROM"
			" account_rewards WHERE account_id=%i AND reward_id=%i", AccountID(), claim_id),
			errbuf))
		{
			LogFile->write(EQEMuLog::Error, "Error in Client::TryReward(): %s (%s)", query, errbuf);
			safe_delete_array(query);
		}
		else
		{
			safe_delete_array(query);
		}
	}
	else
	{
		if(!database.RunQuery(query,MakeAnyLenString(&query,"UPDATE account_rewards SET amount=(amount-1)"
			" WHERE account_id=%i AND reward_id=%i", AccountID(), claim_id),
			errbuf))
		{
			LogFile->write(EQEMuLog::Error, "Error in Client::TryReward(): %s (%s)", query, errbuf);
			safe_delete_array(query);
		}
		else
		{
			safe_delete_array(query);
		}
	}

	InternalVeteranReward ivr = (*iter);
	ItemInst *claim = database.CreateItem(ivr.items[0].item_id, ivr.items[0].charges);
	if(claim)
	{
		bool lore_conflict = false;
		if(CheckLoreConflict(claim->GetItem()))
		{
			lore_conflict = true;
		}

		for(int y = 1; y < 8; y++)
		{
			if(ivr.items[y].item_id)
			{
				if(claim->GetItem()->ItemClass == 1)
				{
					ItemInst *item_temp = database.CreateItem(ivr.items[y].item_id, ivr.items[y].charges);
					if(item_temp)
					{
						if(CheckLoreConflict(item_temp->GetItem()))
						{
							lore_conflict = true;
							DuplicateLoreMessage(ivr.items[y].item_id);
						}
						claim->PutItem(y-1, *item_temp);
					}
				}
			}
		}

		if(lore_conflict)
		{
			safe_delete(claim);
			return true;
		}
		else
		{
			PutItemInInventory(free_slot, *claim);
			SendItemPacket(free_slot, claim, ItemPacketTrade);
		}
	}

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

void Client::UpdateLDoNWins(uint32 t, int32 n)
{
	switch(t)
	{
	case 1:
		m_pp.ldon_wins_guk = n;
		break;
	case 2:
		m_pp.ldon_wins_mir = n;
		break;
	case 3:
		m_pp.ldon_wins_mmc = n;
		break;
	case 4:
		m_pp.ldon_wins_ruj = n;
		break;
	case 5:
		m_pp.ldon_wins_tak = n;
		break;
	default:
		return;
	}
}

void Client::UpdateLDoNLosses(uint32 t, int32 n)
{
	switch(t)
	{
	case 1:
		m_pp.ldon_losses_guk = n;
		break;
	case 2:
		m_pp.ldon_losses_mir = n;
		break;
	case 3:
		m_pp.ldon_losses_mmc = n;
		break;
	case 4:
		m_pp.ldon_losses_ruj = n;
		break;
	case 5:
		m_pp.ldon_losses_tak = n;
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
			MakePoweredPet(m_suspendedminion.SpellID, spells[m_suspendedminion.SpellID].teleport_zone,
				m_suspendedminion.petpower, m_suspendedminion.Name);

			CurrentPet = GetPet()->CastToNPC();

			if(!CurrentPet)
			{
				Message(13, "Failed to recall suspended minion.");
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

			Message_StringID(clientMessageTell, SUSPEND_MINION_UNSUSPEND, CurrentPet->GetCleanName());

			memset(&m_suspendedminion, 0, sizeof(struct PetInfo));
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
				Message_StringID(clientMessageError,ONLY_ONE_PET);

				return;
			}
			else if(CurrentPet->IsEngaged())
			{
				Message_StringID(clientMessageError,SUSPEND_MINION_FIGHTING);

				return;
			}
			else if(entity_list.Fighting(CurrentPet))
			{
				Message_StringID(clientMessageBlue,SUSPEND_MINION_HAS_AGGRO);
			}
			else
			{
				m_suspendedminion.SpellID = SpellID;

				m_suspendedminion.HP = CurrentPet->GetHP();;

				m_suspendedminion.Mana = CurrentPet->GetMana();
				m_suspendedminion.petpower = CurrentPet->GetPetPower();

				if(AALevel >= 2)
					CurrentPet->GetPetState(m_suspendedminion.Buffs, m_suspendedminion.Items, m_suspendedminion.Name);
				else
					strn0cpy(m_suspendedminion.Name, CurrentPet->GetName(), 64); // Name stays even at rank 1

				Message_StringID(clientMessageTell, SUSPEND_MINION_SUSPEND, CurrentPet->GetCleanName());

				CurrentPet->Depop(false);

				SetPetID(0);
			}
		}
		else
		{
			Message_StringID(clientMessageError, ONLY_SUMMONED_PETS);

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

	Save();

	SendCrystalCounts();
}

// Processes a client request to inspect a SoF client's equipment.
void Client::ProcessInspectRequest(Client* requestee, Client* requester) {
	if(requestee && requester) {
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_InspectAnswer, sizeof(InspectResponse_Struct));
		InspectResponse_Struct* insr = (InspectResponse_Struct*) outapp->pBuffer;
		insr->TargetID = requester->GetID();
		insr->playerid = requestee->GetID();

		const Item_Struct* item = nullptr;
		const ItemInst* inst = nullptr;

		for(int16 L = 0; L <= 20; L++) {
			inst = requestee->GetInv().GetItem(L);

			if(inst) {
				item = inst->GetItem();
				if(item) {
					strcpy(insr->itemnames[L], item->Name);
					insr->itemicons[L] = item->Icon;
				}
				else
					insr->itemicons[L] = 0xFFFFFFFF;
			}
		}

		inst = requestee->GetInv().GetItem(9999);

		if(inst) {
			item = inst->GetItem();
			if(item) {
				strcpy(insr->itemnames[21], item->Name);
				insr->itemicons[21] = item->Icon;
			}
			else
				insr->itemicons[21] = 0xFFFFFFFF;
		}

		inst = requestee->GetInv().GetItem(21);

		if(inst) {
			item = inst->GetItem();
			if(item) {
				strcpy(insr->itemnames[22], item->Name);
				insr->itemicons[22] = item->Icon;
			}
			else
				insr->itemicons[22] = 0xFFFFFFFF;
		}

		strcpy(insr->text, requestee->GetInspectMessage().text);

		// There could be an OP for this..or not... (Ti clients are not processed here..this message is generated client-side)
		if(requestee->IsClient() && (requestee != requester)) { requestee->Message(0, "%s is looking at your equipment...", requester->GetName()); }

		requester->QueuePacket(outapp); // Send answer to requester
		safe_delete(outapp);
	}
}

void Client::GuildBankAck()
{
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_GuildBank, sizeof(GuildBankAck_Struct));

	GuildBankAck_Struct *gbas = (GuildBankAck_Struct*) outapp->pBuffer;

	gbas->Action = GuildBankAcknowledge;

	FastQueuePacket(&outapp);
}

void Client::GuildBankDepositAck(bool Fail)
{

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_GuildBank, sizeof(GuildBankDepositAck_Struct));

	GuildBankDepositAck_Struct *gbdas = (GuildBankDepositAck_Struct*) outapp->pBuffer;

	gbdas->Action = GuildBankDeposit;

	gbdas->Fail = Fail ? 1 : 0;

	FastQueuePacket(&outapp);
}

void Client::ClearGuildBank()
{
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_GuildBank, sizeof(GuildBankClear_Struct));

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
	EQApplicationPacket *outapp=new EQApplicationPacket(OP_GroupUpdateB, 32 + strlen(GetName()));

	char *Buffer = (char *)outapp->pBuffer;
	// Header
	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 1);
	VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// Null Leader name

	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);	// Member 0
	VARSTRUCT_ENCODE_STRING(Buffer, GetName());
	VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);
	VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);
	VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);	// This is a string
	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, GetLevel());
	VARSTRUCT_ENCODE_TYPE(uint8, Buffer, 0);
	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
	VARSTRUCT_ENCODE_TYPE(uint32, Buffer, 0);
	VARSTRUCT_ENCODE_TYPE(uint16, Buffer, 0);

	FastQueuePacket(&outapp);
}

void Client::SendGroupLeaderChangePacket(const char *LeaderName)
{
	// For SoD and later, send name of Group Leader to this client

	EQApplicationPacket *outapp=new EQApplicationPacket(OP_GroupLeaderChange, sizeof(GroupLeaderChange_Struct));

	GroupLeaderChange_Struct *glcs = (GroupLeaderChange_Struct*)outapp->pBuffer;

	strn0cpy(glcs->LeaderName, LeaderName, sizeof(glcs->LeaderName));

	FastQueuePacket(&outapp);
}

void Client::SendGroupJoinAcknowledge()
{
	// For SoD and later, This produces the 'You have joined the group' message.
	EQApplicationPacket* outapp=new EQApplicationPacket(OP_GroupAcknowledge, 4);
	FastQueuePacket(&outapp);
}

void Client::SendAdventureError(const char *error)
{
	size_t error_size = strlen(error);
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_AdventureInfo, (error_size + 2));
	strn0cpy((char*)outapp->pBuffer, error, error_size);
	FastQueuePacket(&outapp);
}

void Client::SendAdventureDetails()
{
	if(adv_data)
	{
		ServerSendAdventureData_Struct *ad = (ServerSendAdventureData_Struct*)adv_data;
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_AdventureData, sizeof(AdventureRequestResponse_Struct));
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
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_AdventureData, sizeof(AdventureRequestResponse_Struct));
		FastQueuePacket(&outapp);
	}
}

void Client::SendAdventureCount(uint32 count, uint32 total)
{
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_AdventureUpdate, sizeof(AdventureCountUpdate_Struct));
	AdventureCountUpdate_Struct *acu = (AdventureCountUpdate_Struct*)outapp->pBuffer;
	acu->current = count;
	acu->total = total;
	FastQueuePacket(&outapp);
}

void Client::NewAdventure(int id, int theme, const char *text, int member_count, const char *members)
{
	size_t text_size = strlen(text);
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_AdventureDetails, text_size + 2);
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
		ServerPacket *pack = new ServerPacket(ServerOP_AdventureLeave, 64);
		strcpy((char*)pack->pBuffer, GetName());
		pack->Deflate();
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
	UpdateLDoNPoints(points, theme);
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_AdventureFinish, sizeof(AdventureFinish_Struct));
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

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_DzCompass, sizeof(ExpeditionInfo_Struct) + sizeof(ExpeditionCompassEntry_Struct) * count);
	ExpeditionCompass_Struct *ecs = (ExpeditionCompass_Struct*)outapp->pBuffer;
	//ecs->clientid = GetID();
	ecs->count = count;

	if (count) {
		ecs->entries[0].x = in_x;
		ecs->entries[0].y = in_y;
		ecs->entries[0].z = in_z;
	}

	FastQueuePacket(&outapp);
	safe_delete(outapp);
}

void Client::SendZonePoints()
{
	int count = 0;
	LinkedListIterator<ZonePoint*> iterator(zone->zone_point_list);
	iterator.Reset();
	while(iterator.MoreElements())
	{
		ZonePoint* data = iterator.GetData();
		if(GetClientVersionBit() & data->client_version_mask)
		{
			count++;
		}
		iterator.Advance();
	}

	uint32 zpsize = sizeof(ZonePoints) + ((count + 1) * sizeof(ZonePoint_Entry));
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_SendZonepoints, zpsize);
	ZonePoints* zp = (ZonePoints*)outapp->pBuffer;
	zp->count = count;

	int i = 0;
	iterator.Reset();
	while(iterator.MoreElements())
	{
		ZonePoint* data = iterator.GetData();
		if(GetClientVersionBit() & data->client_version_mask)
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
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_TargetCommand, sizeof(ClientTarget_Struct));
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
		Message_StringID(MT_Spells, SENSE_CORPSE_DIRECTION);
		SetHeading(CalculateHeadingToTarget(ClosestCorpse->GetX(), ClosestCorpse->GetY()));
		SetTarget(ClosestCorpse);
		SendTargetCommand(ClosestCorpse->GetID());
		SendPosUpdate(2);
	}
	else if(!GetTarget())
		Message_StringID(clientMessageError, SENSE_CORPSE_NONE);
	else
		Message_StringID(clientMessageError, SENSE_CORPSE_NOT_NAME);
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
		database.NPCSpawnDB(0, zone->GetShortName(), zone->GetInstanceVersion(), this, target_npc->CastToNPC(), extra);
	}
	else if (id == "add") {
		// extra sets the respawn timer for add
		database.NPCSpawnDB(1, zone->GetShortName(), zone->GetInstanceVersion(), this, target_npc->CastToNPC(), extra);
	}
	else if (id == "update") {
		database.NPCSpawnDB(2, zone->GetShortName(), zone->GetInstanceVersion(), this, target_npc->CastToNPC());
	}
	else if (id == "remove") {
		database.NPCSpawnDB(3, zone->GetShortName(), zone->GetInstanceVersion(), this, target_npc->CastToNPC());
		target_npc->Depop(false);
	}
	else if (id == "delete") {
		database.NPCSpawnDB(4, zone->GetShortName(), zone->GetInstanceVersion(), this, target_npc->CastToNPC());
		target_npc->Depop(false);
	}
	else {
		return;
	}
}

bool Client::IsDraggingCorpse(const char *CorpseName)
{
	for(std::list<std::string>::iterator Iterator = DraggedCorpses.begin(); Iterator != DraggedCorpses.end(); ++Iterator)
	{
		if(!strcasecmp((*Iterator).c_str(), CorpseName))
			return true;
	}

	return false;
}

void Client::DragCorpses()
{
	for(std::list<std::string>::iterator Iterator = DraggedCorpses.begin(); Iterator != DraggedCorpses.end(); ++Iterator)
	{
		Mob* corpse = entity_list.GetMob((*Iterator).c_str());

		if(corpse && corpse->IsPlayerCorpse() && (DistNoRootNoZ(*corpse) <= RuleR(Character, DragCorpseDistance)))
			continue;

		if(!corpse || !corpse->IsPlayerCorpse() || corpse->CastToCorpse()->IsBeingLooted() || !corpse->CastToCorpse()->Summon(this, false, false))
		{
			Message_StringID(MT_DefaultText, CORPSEDRAG_STOP);
			Iterator = DraggedCorpses.erase(Iterator);
		}
	}
}
void Client::Doppelganger(uint16 spell_id, Mob *target, const char *name_override, int pet_count, int pet_duration)
{
	if(!target || !IsValidSpell(spell_id) || this->GetID() == target->GetID())
		return;

	PetRecord record;
	if(!database.GetPetEntry(spells[spell_id].teleport_zone, &record))
	{
		LogFile->write(EQEMuLog::Error, "Unknown doppelganger spell id: %d, check pets table", spell_id);
		Message(13, "Unable to find data for pet %s", spells[spell_id].teleport_zone);
		return;
	}

	AA_SwarmPet pet;
	pet.count = pet_count;
	pet.duration = pet_duration;
	pet.npc_id = record.npc_type;

	NPCType *made_npc = nullptr;

	const NPCType *npc_type = database.GetNPCType(pet.npc_id);
	if(npc_type == nullptr) {
		LogFile->write(EQEMuLog::Error, "Unknown npc type for doppelganger spell id: %d", spell_id);
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
	// looks
	made_npc->texture = GetEquipmentMaterial(1);
	made_npc->helmtexture = GetEquipmentMaterial(0);
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
	made_npc->d_meele_texture1 = GetEquipmentMaterial(7);
	made_npc->d_meele_texture2 = GetEquipmentMaterial(8);
	for (int i = 0; i < MAX_MATERIALS; i++)	{
		made_npc->armor_tint[i] = GetEquipmentColor(i);
	}
	made_npc->loottable_id = 0;

	npc_type = made_npc;

	int summon_count = 0;
	summon_count = pet.count;

	if(summon_count > MAX_SWARM_PETS)
		summon_count = MAX_SWARM_PETS;

	static const float swarm_pet_x[MAX_SWARM_PETS] = { 5, -5, 5, -5, 10, -10, 10, -10, 8, -8, 8, -8 };
	static const float swarm_pet_y[MAX_SWARM_PETS] = { 5, 5, -5, -5, 10, 10, -10, -10, 8, 8, -8, -8 };
	TempPets(true);

	while(summon_count > 0) {
		NPCType *npc_dup = nullptr;
		if(made_npc != nullptr) {
			npc_dup = new NPCType;
			memcpy(npc_dup, made_npc, sizeof(NPCType));
		}

		NPC* npca = new NPC(
				(npc_dup!=nullptr)?npc_dup:npc_type,	//make sure we give the NPC the correct data pointer
				0,
				GetX()+swarm_pet_x[summon_count], GetY()+swarm_pet_y[summon_count],
				GetZ(), GetHeading(), FlyMode3);

		if(!npca->GetSwarmInfo()){
			AA_SwarmPetInfo* nSI = new AA_SwarmPetInfo;
			npca->SetSwarmInfo(nSI);
			npca->GetSwarmInfo()->duration = new Timer(pet_duration*1000);
		}
		else{
			npca->GetSwarmInfo()->duration->Start(pet_duration*1000);
		}

		npca->GetSwarmInfo()->owner_id = GetID();

		// Give the pets alittle more agro than the caster and then agro them on the target
		target->AddToHateList(npca, (target->GetHateAmount(this) + 100), (target->GetDamageAmount(this) + 100));
		npca->AddToHateList(target, 1000, 1000);
		npca->GetSwarmInfo()->target = target->GetID();

		//we allocated a new NPC type object, give the NPC ownership of that memory
		if(npc_dup != nullptr)
			npca->GiveNPCTypeData(npc_dup);

		entity_list.AddNPC(npca);
		summon_count--;
	}
}

void Client::AssignToInstance(uint16 instance_id)
{
	database.AddClientToInstance(instance_id, CharacterID());
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
	std::string class_List[] = { "WAR", "CLR", "PAL", "RNG", "SK", "DRU", "MNK", "BRD", "ROG", "SHM", "NEC", "WIZ", "MAG", "ENC", "BST", "BER" };

	if(GetClass() < 17 && GetClass() > 0) { class_Name = class_List[GetClass()-1]; }

	// Race
	std::string race_Name = itoa(GetRace());
	switch(GetRace())
	{
		case 1: race_Name = "Human";		break;
		case 2:	race_Name = "Barbarian";	break;
		case 3:	race_Name = "Erudite";		break;
		case 4:	race_Name = "Wood Elf";		break;
		case 5:	race_Name = "High Elf";		break;
		case 6:	race_Name = "Dark Elf";		break;
		case 7:	race_Name = "Half Elf";		break;
		case 8:	race_Name = "Dwarf";		break;
		case 9:	race_Name = "Troll";		break;
		case 10: race_Name = "Ogre";		break;
		case 11: race_Name = "Halfing";		break;
		case 12: race_Name = "Gnome";		break;
		case 128: race_Name = "Iksar";		break;
		case 130: race_Name = "Vah Shir";	break;
		case 330: race_Name = "Froglok";	break;
		case 522: race_Name = "Drakkin";	break;
		default: break;
	}
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
				total_regen_field = itoa(CalcHPRegen());
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
					total_regen_field = itoa(CalcManaRegen());
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
				total_regen_field = itoa(CalcEnduranceRegen());
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
				a_stat = itoa(GetCHA());
				h_stat = itoa(GetHeroicCHA());
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
		if(stat_row_counter < 6) {
			stat_field += h_stat_spacing + a_resist_name + a_resist_spacing + a_resist + heroic_color + h_resist_field + "</c><br>";
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
			if ((HasRune() || HasPartialMeleeRune()) && buffs[i].melee_rune > 0) { rune_number += buffs[i].melee_rune; }

			if ((HasSpellRune() || HasPartialSpellRune()) && buffs[i].magic_rune > 0) { magic_rune_number += buffs[i].magic_rune; }
		}
	}

	int shield_ac = 0;
	GetRawACNoShield(shield_ac);

	std::string skill_list[] = {
		"1H Blunt","1H Slashing","2H Blunt","2H Slashing","Abjuration","Alteration","Apply Poison","Archery","Backstab","Bind Wound","Bash","Block","Brass Instruments","Channeling","Conjuration",
		"Defense","Disarm","Disarm Traps","Divination","Dodge","Double Attack","Dragon Punch","Dual Wield","Eagle Strike","Evocation","Feign Death","Flying Kick","Forage","Hand To Hand","Hide","Kick",
		"Meditate","Mend","Offense","Parry","Pick Lock","Piercing","Riposte","Round Kick","Safe Fall","Sense Heading","Singing","Sneak","Specialize Abjuration","Specialize Alteration","Specialize Conjuration",
		"Specialize Divination","Specialize Evocation","Pick Pockets","Stringed_Instruments","Swimming","Throwing","Tiger Claw","Tracking","Wind Instruments","Fishing","Make Poison","Tinkering","Research","Alchemy",
		"Baking","Tailoring","Sense Traps","Blacksmithing","Fletching","Brewing","Alcohol_Tolerance","Begging","Jewelry Making","Pottery","Percussion Instruments","Intimidation","Berserking","Taunt","Frenzy"
	};

	std::string skill_mods = "";
	for(int j = 0; j <= HIGHEST_SKILL; j++) {
		if(itembonuses.skillmod[j] > 0)
			skill_mods += indP + skill_list[j] + " : +" + itoa(itembonuses.skillmod[j]) + "%<br>";
		else if(itembonuses.skillmod[j] < 0)
			skill_mods += indP + skill_list[j] + " : -" + itoa(itembonuses.skillmod[j]) + "%<br>";
	}

	std::string skill_dmgs = "";
	for(int j = 0; j <= HIGHEST_SKILL; j++) {
		if((itembonuses.SkillDamageAmount[j] + spellbonuses.SkillDamageAmount[j]) > 0)
			skill_dmgs += indP + skill_list[j] + " : +" + itoa(itembonuses.SkillDamageAmount[j] + spellbonuses.SkillDamageAmount[j]) + "<br>";
		else if((itembonuses.SkillDamageAmount[j] + spellbonuses.SkillDamageAmount[j]) < 0)
			skill_dmgs += indP + skill_list[j] + " : -" + itoa(itembonuses.SkillDamageAmount[j] + spellbonuses.SkillDamageAmount[j]) + "<br>";
	}

	std::string faction_item_string = "";
	char faction_buf[256];

	for(std::map <uint32, int32>::iterator iter = item_faction_bonuses.begin();
		iter != item_faction_bonuses.end();
		iter++)
	{
		memset(&faction_buf, 0, sizeof(faction_buf));

		if(!database.GetFactionName((int32)((*iter).first), faction_buf, sizeof(faction_buf)))
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

	std::string final_stats = "" +
	/*	C/L/R	*/	indP + "Class: " + class_Name + indS + "Level: " + itoa(GetLevel()) + indS + "Race: " + race_Name + "<br>" +
	/*	Runes	*/	indP + "Rune: " + itoa(rune_number) + indL + indS + "Spell Rune: " + itoa(magic_rune_number) + "<br>" +
	/*	HP/M/E	*/	HME_row +
	/*	DS		*/	indP + "DS: " + itoa(itembonuses.DamageShield + spellbonuses.DamageShield*-1) + " (Spell: " + itoa(spellbonuses.DamageShield*-1) + " + Item: " + itoa(itembonuses.DamageShield) + " / " + itoa(RuleI(Character, ItemDamageShieldCap)) + ")<br>" +
	/*	Atk		*/	indP + "<c \"#CCFF00\">ATK: " + itoa(GetTotalATK()) + "</c><br>" +
	/*	Atk2	*/	indP + "- Base: " + itoa(GetATKRating()) + " | Item: " + itoa(itembonuses.ATK) + " (" + itoa(RuleI(Character, ItemATKCap)) + ")~Used: " + itoa((itembonuses.ATK * 1.342)) + " | Spell: " + itoa(spellbonuses.ATK) + "<br>" +
	/*	AC		*/	indP + "<c \"#CCFF00\">AC: " + itoa(CalcAC()) + "</c><br>" +
	/*	AC2		*/	indP + "- Mit: " + itoa(GetACMit()) + " | Avoid: " + itoa(GetACAvoid()) + " | Spell: " + itoa(spellbonuses.AC) + " | Shield: " + itoa(shield_ac) + "<br>" +
	/*	Haste	*/	indP + "<c \"#CCFF00\">Haste: " + itoa(GetHaste()) + "</c><br>" +
	/*	Haste2	*/	indP + " - Item: " + itoa(itembonuses.haste) + " + Spell: " + itoa(spellbonuses.haste + spellbonuses.hastetype2) + " (Cap: " + itoa(RuleI(Character, HasteCap)) + ") | Over: " + itoa(spellbonuses.hastetype3 + ExtraHaste) + "<br><br>" +
	/* RegenLbl	*/	indL + indS + "Regen<br>" + indS + indP + indP + " Base | Items (Cap) " + indP + " | Spell | A.A.s | Total<br>" +
	/*	Regen	*/	regen_string + "<br>" +
	/*	Stats	*/	stat_field + "<br><br>" +
	/*	Mod2s	*/	mod2_field + "<br>" +
	/*	HealAmt	*/	indP + "Heal Amount: " + itoa(GetHealAmt()) + " / " + itoa(RuleI(Character, ItemHealAmtCap)) + "<br>" +
	/*	SpellDmg*/	indP + "Spell Dmg: " + itoa(GetSpellDmg()) + " / " + itoa(RuleI(Character, ItemSpellDmgCap)) + "<br>" +
	/*	Clair	*/	indP + "Clairvoyance: " + itoa(GetClair()) + " / " + itoa(RuleI(Character, ItemClairvoyanceCap)) + "<br>" +
	/*	DSMit	*/	indP + "Dmg Shld Mit: " + itoa(GetDSMit()) + " / " + itoa(RuleI(Character, ItemDSMitigationCap)) + "<br><br>";
	if(GetClass() == BARD)
		final_stats += bard_info + "<br>";
	if(skill_mods.size() > 0)
		final_stats += skill_mods + "<br>";
	if(skill_dmgs.size() > 0)
		final_stats += skill_dmgs + "<br>";
	if(faction_item_string.size() > 0)
		final_stats += faction_item_string;


	if(use_window) {
		if(final_stats.size() < 4096)
		{
			uint32 Buttons = (client->GetClientVersion() < EQClientSoD) ? 0 : 1;
			client->SendWindow(0, POPUPID_UPDATE_SHOWSTATSWINDOW, Buttons, "Cancel", "Update", 0, 1, this, "", "%s", final_stats.c_str());
			goto Extra_Info;
		}
		else {
			client->Message(15, "The window has exceeded its character limit, displaying stats to chat window:");
		}
	}

	client->Message(15, "~~~~~ %s %s ~~~~~", GetCleanName(), GetLastName());
	client->Message(0, " Level: %i Class: %i Race: %i DS: %i/%i Size: %1.1f  Weight: %.1f/%d  ", GetLevel(), GetClass(), GetRace(), GetDS(), RuleI(Character, ItemDamageShieldCap), GetSize(), (float)CalcCurrentWeight() / 10.0f, GetSTR());
	client->Message(0, " HP: %i/%i  HP Regen: %i/%i",GetHP(), GetMaxHP(), CalcHPRegen(), CalcHPRegenCap());
	client->Message(0, " AC: %i ( Mit.: %i + Avoid.: %i + Spell: %i ) | Shield AC: %i", CalcAC(), GetACMit(), GetACAvoid(), spellbonuses.AC, shield_ac);
	if(CalcMaxMana() > 0)
		client->Message(0, " Mana: %i/%i  Mana Regen: %i/%i", GetMana(), GetMaxMana(), CalcManaRegen(), CalcManaRegenCap());
	client->Message(0, " End.: %i/%i  End. Regen: %i/%i",GetEndurance(), GetMaxEndurance(), CalcEnduranceRegen(), CalcEnduranceRegenCap());
	client->Message(0, " ATK: %i  Worn/Spell ATK %i/%i  Server Side ATK: %i", GetTotalATK(), RuleI(Character, ItemATKCap), GetATKBonus(), GetATK());
	client->Message(0, " Haste: %i / %i (Item: %i + Spell: %i + Over: %i)", GetHaste(), RuleI(Character, HasteCap), itembonuses.haste, spellbonuses.haste + spellbonuses.hastetype2, spellbonuses.hastetype3 + ExtraHaste);
	client->Message(0, " STR: %i  STA: %i  DEX: %i  AGI: %i  INT: %i  WIS: %i  CHA: %i", GetSTR(), GetSTA(), GetDEX(), GetAGI(), GetINT(), GetWIS(), GetCHA());
	client->Message(0, " hSTR: %i  hSTA: %i  hDEX: %i  hAGI: %i  hINT: %i  hWIS: %i  hCHA: %i", GetHeroicSTR(), GetHeroicSTA(), GetHeroicDEX(), GetHeroicAGI(), GetHeroicINT(), GetHeroicWIS(), GetHeroicCHA());
	client->Message(0, " MR: %i  PR: %i  FR: %i  CR: %i  DR: %i Corruption: %i", GetMR(), GetPR(), GetFR(), GetCR(), GetDR(), GetCorrup());
	client->Message(0, " hMR: %i  hPR: %i  hFR: %i  hCR: %i  hDR: %i hCorruption: %i", GetHeroicMR(), GetHeroicPR(), GetHeroicFR(), GetHeroicCR(), GetHeroicDR(), GetHeroicCorrup());
	client->Message(0, " Shielding: %i  Spell Shield: %i  DoT Shielding: %i Stun Resist: %i  Strikethrough: %i  Avoidance: %i  Accuracy: %i  Combat Effects: %i", GetShielding(), GetSpellShield(), GetDoTShield(), GetStunResist(), GetStrikeThrough(), GetAvoidance(), GetAccuracy(), GetCombatEffects());
	client->Message(0, " Heal Amt.: %i  Spell Dmg.: %i  Clairvoyance: %i DS Mitigation: %i", GetHealAmt(), GetSpellDmg(), GetClair(), GetDSMit());
	if(GetClass() == BARD)
		client->Message(0, " Singing: %i  Brass: %i  String: %i Percussion: %i Wind: %i", GetSingMod(), GetBrassMod(), GetStringMod(), GetPercMod(), GetWindMod());

	Extra_Info:

	client->Message(0, " BaseRace: %i  Gender: %i  BaseGender: %i Texture: %i  HelmTexture: %i", GetBaseRace(), GetGender(), GetBaseGender(), GetTexture(), GetHelmTexture());
	if (client->Admin() >= 100) {
		client->Message(0, "  CharID: %i  EntityID: %i  PetID: %i  OwnerID: %i  AIControlled: %i  Targetted: %i", CharacterID(), GetID(), GetPetID(), GetOwnerID(), IsAIControlled(), targeted);
	}
}

void Client::SendAltCurrencies() {
	if(GetClientVersion() >= EQClientSoF) {
		uint32 count = zone->AlternateCurrencies.size();
		if(count == 0) {
			return;
		}

		EQApplicationPacket *outapp = new EQApplicationPacket(OP_AltCurrency,
			sizeof(AltCurrencyPopulate_Struct) + sizeof(AltCurrencyPopulateEntry_Struct) * count);
		AltCurrencyPopulate_Struct *altc = (AltCurrencyPopulate_Struct*)outapp->pBuffer;
		altc->opcode = ALT_CURRENCY_OP_POPULATE;
		altc->count = count;

		uint32 i = 0;
		std::list<AltCurrencyDefinition_Struct>::iterator iter = zone->AlternateCurrencies.begin();
		while(iter != zone->AlternateCurrencies.end()) {
			const Item_Struct* item = database.GetItem((*iter).item_id);
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
			iter++;
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

void Client::AddAlternateCurrencyValue(uint32 currency_id, int32 amount)
{
	if(amount == 0) {
		return;
	}

	int new_value = 0;
	std::map<uint32, uint32>::iterator iter = alternate_currency.find(currency_id);
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
	std::list<AltCurrencyDefinition_Struct>::iterator iter = zone->AlternateCurrencies.begin();
	while(iter != zone->AlternateCurrencies.end()) {
		SendAlternateCurrencyValue((*iter).id, false);
		iter++;
	}
}

void Client::SendAlternateCurrencyValue(uint32 currency_id, bool send_if_null)
{
	uint32 value = GetAlternateCurrencyValue(currency_id);
	if(value > 0 || (value == 0 && send_if_null)) {
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_AltCurrency, sizeof(AltCurrencyUpdate_Struct));
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
	std::map<uint32, uint32>::const_iterator iter = alternate_currency.find(currency_id);
	if(iter == alternate_currency.end()) {
		return 0;
	} else {
		return (*iter).second;
	}
}

void Client::OpenLFGuildWindow()
{
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_LFGuild, 8);

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

void Client::AddAutoXTarget(Mob *m)
{
	if(!XTargettingAvailable() || !XTargetAutoAddHaters)
		return;

	if(IsXTarget(m))
		return;

	for(int i = 0; i < GetMaxXTargets(); ++i)
	{
		if((XTargets[i].Type == Auto) && (XTargets[i].ID == 0))
		{
			XTargets[i].ID = m->GetID();
			SendXTargetPacket(i, m);
			break;
		}
	}
}

void Client::RemoveXTarget(Mob *m, bool OnlyAutoSlots)
{
	if(!XTargettingAvailable())
		return;

	bool HadFreeAutoSlotsBefore = false;

	int FreedAutoSlots = 0;

	if(m->GetID() == 0)
		return;

	for(int i = 0; i < GetMaxXTargets(); ++i)
	{
		if(OnlyAutoSlots && (XTargets[i].Type !=Auto))
			continue;

		if(XTargets[i].ID == m->GetID())
		{
			if(XTargets[i].Type == CurrentTargetNPC)
				XTargets[i].Type = Auto;

			if(XTargets[i].Type == Auto)
				++FreedAutoSlots;

			XTargets[i].ID = 0;

			SendXTargetPacket(i, nullptr);
		}
		else
		{
			if((XTargets[i].Type == Auto) && (XTargets[i].ID == 0))
				HadFreeAutoSlotsBefore = true;
		}
	}
	// If there are more mobs aggro on us than we had auto-hate slots, add one of those haters into the slot(s) we just freed up.
	if(!HadFreeAutoSlotsBefore && FreedAutoSlots)
		entity_list.RefreshAutoXTargets(this);
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

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_XTargetResponse, PacketSize);
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
		c->Message(0, "Xtarget Slot: %i, Type: %2i, ID: %4i, Name: %s", i, XTargets[i].Type, XTargets[i].ID, XTargets[i].Name);
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

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_XTargetResponse, 8);
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
	if(website != 0)
	{
		std::string str = website;
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_Weblink, sizeof(Weblink_Struct) + str.length() + 1);
		Weblink_Struct *wl = (Weblink_Struct*)outapp->pBuffer;
		memcpy(wl->weblink, str.c_str(), str.length() + 1);
		wl->weblink[str.length() + 1] = '\0';

		FastQueuePacket(&outapp);
	}
}

void Client::SendMercPersonalInfo()
{
uint32 mercTypeCount = 1;
uint32 mercCount = 1; //TODO: Un-hardcode this and support multiple mercs like in later clients than SoD.
//uint32 packetSize = 0;
uint32 i=0;
uint32 altCurrentType = 19; //TODO: Implement alternate currency purchases involving mercs!

	if (GetClientVersion() >= EQClientRoF)
	{
		MercTemplate *mercData = &zone->merc_templates[GetMercInfo().MercTemplateID];

		if (mercData)
		{
			int i = 0;
			int stancecount = 0;
			stancecount += zone->merc_stance_list[GetMercInfo().MercTemplateID].size();

			if(stancecount > MAX_MERC_STANCES || mercCount > MAX_MERC || mercTypeCount > MAX_MERC_GRADES)
			{
				SendMercMerchantResponsePacket(0);
				return;
			}
			if (mercCount > 0 && mercCount)
			{
				EQApplicationPacket *outapp = new EQApplicationPacket(OP_MercenaryDataUpdate, sizeof(MercenaryDataUpdate_Struct));
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
					std::list<MercStanceInfo>::iterator iter = zone->merc_stance_list[mercData->MercTemplateID].begin();
					while(iter != zone->merc_stance_list[mercData->MercTemplateID].end())
					{
						mdus->MercData[i].Stances[stanceindex].StanceIndex = stanceindex;
						mdus->MercData[i].Stances[stanceindex].Stance = (iter->StanceID);
						stanceindex++;
						iter++;
					}
				}

				mdus->MercData[i].MercUnk05 = 1;
				FastQueuePacket(&outapp);
				return;
			}
		}
	}
	else
	{
		int stancecount = 0;
		stancecount += zone->merc_stance_list[GetMercInfo().MercTemplateID].size();

		if(mercCount > MAX_MERC || mercTypeCount > MAX_MERC_GRADES)
		{
			if (GetClientVersion() == EQClientSoD)
			{
				SendMercMerchantResponsePacket(0);
			}
			return;
		}

		EQApplicationPacket *outapp = new EQApplicationPacket(OP_MercenaryDataResponse, sizeof(MercenaryMerchantList_Struct));
		MercenaryMerchantList_Struct* mml = (MercenaryMerchantList_Struct*)outapp->pBuffer;
		MercTemplate *mercData = &zone->merc_templates[GetMercInfo().MercTemplateID];


		if(mercData)
		{
			if(mercTypeCount > 0)
			{
				mml->MercTypeCount = mercTypeCount; //We only should have one merc entry.
				mml->MercGrades[i] = 1;
			}
			mml->MercCount = mercCount;
			if(mercCount > 0)
			{

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
				//mml->Mercs[i].MercName;
				int stanceindex = 0;
				if(mml->Mercs[i].StanceCount != 0)
				{
					std::list<MercStanceInfo>::iterator iter = zone->merc_stance_list[mercData->MercTemplateID].begin();
					while(iter != zone->merc_stance_list[mercData->MercTemplateID].end())
					{
						mml->Mercs[i].Stances[stanceindex].StanceIndex = stanceindex;
						mml->Mercs[i].Stances[stanceindex].Stance = (iter->StanceID);
						stanceindex++;
						iter++;
					}
				}
				FastQueuePacket(&outapp);
			}
			else
			{
				safe_delete(outapp);
				if (GetClientVersion() == EQClientSoD)
				{
					SendMercMerchantResponsePacket(0);
				}
				return;
			}
			if (GetClientVersion() == EQClientSoD)
			{
				SendMercMerchantResponsePacket(0);
			}
		}
		else
		{
			safe_delete(outapp);
			if (GetClientVersion() == EQClientSoD)
			{
			SendMercMerchantResponsePacket(0);
			}
			return;
		}
	}
}

void Client::SendClearMercInfo()
{
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_MercenaryDataUpdate, sizeof(NoMercenaryHired_Struct));
	NoMercenaryHired_Struct *nmhs = (NoMercenaryHired_Struct*)outapp->pBuffer;
	nmhs->MercStatus = -1;
	nmhs->MercCount = 0;
	nmhs->MercID = 1;
	FastQueuePacket(&outapp);
}


void Client::DuplicateLoreMessage(uint32 ItemID)
{
	if(!(ClientVersionBit & BIT_RoFAndLater))
	{
		Message_StringID(0, PICK_LORE);
		return;
	}

	const Item_Struct *item = database.GetItem(ItemID);

	if(!item)
		return;

	Message_StringID(0, PICK_LORE, item->Name);
}

void Client::GarbleMessage(char *message, uint8 variance)
{
	// Garble message by variance%
	const char alpha_list[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"; // only change alpha characters for now

	for (size_t i = 0; i < strlen(message); i++) {
		uint8 chance = (uint8)MakeRandomInt(0, 115); // variation just over worst possible scrambling
		if (isalpha(message[i]) && (chance <= variance)) {
			uint8 rand_char = (uint8)MakeRandomInt(0,51); // choose a random character from the alpha list
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

	return GetFactionLevel(CharacterID(), 0, GetRace(), GetClass(), GetDeity(), iOther->GetPrimaryFaction(), iOther);
}

//o--------------------------------------------------------------
//| Name: GetFactionLevel; rembrant, Dec. 16, 2001
//o--------------------------------------------------------------
//| Notes: Gets the characters faction standing with the specified NPC.
//|			Will return Indifferent on failure.
//o--------------------------------------------------------------
FACTION_VALUE Client::GetFactionLevel(uint32 char_id, uint32 npc_id, uint32 p_race, uint32 p_class, uint32 p_deity, int32 pFaction, Mob* tnpc)
{
	_ZP(Client_GetFactionLevel);

	if (pFaction < 0)
		return GetSpecialFactionCon(tnpc);
	FACTION_VALUE fac = FACTION_INDIFFERENT;
	//int32 pFacValue; -Trumpcard: commenting. Not currently used.
	int32 tmpFactionValue;
	FactionMods fmods;

	// neotokyo: few optimizations
	if (GetFeigned())
		return FACTION_INDIFFERENT;
	if (invisible_undead && tnpc && !tnpc->SeeInvisibleUndead())
		return FACTION_INDIFFERENT;
	if (IsInvisible(tnpc))
		return FACTION_INDIFFERENT;
	if (tnpc && tnpc->GetOwnerID() != 0) // pets con amiably to owner and indiff to rest
		if (char_id == tnpc->GetOwner()->CastToClient()->CharacterID())
			return FACTION_AMIABLE;
		else
			return FACTION_INDIFFERENT;

	//First get the NPC's Primary faction
	if(pFaction > 0)
	{
		//Get the faction data from the database
		if(database.GetFactionData(&fmods, p_class, p_race, p_deity, pFaction))
		{
			//Get the players current faction with pFaction
			tmpFactionValue = GetCharacterFactionLevel(pFaction);
			// Everhood - tack on any bonuses from Alliance type spell effects
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

//o--------------------------------------------------------------
//| Name: SetFactionLevel; rembrant, Dec. 20, 2001
//o--------------------------------------------------------------
//| Notes: Sets the characters faction standing with the specified NPC.
//o--------------------------------------------------------------
void Client::SetFactionLevel(uint32 char_id, uint32 npc_id, uint8 char_class, uint8 char_race, uint8 char_deity)
{
	_ZP(Client_SetFactionLevel);
	int32 faction_id[MAX_NPC_FACTIONS]={ 0,0,0,0,0,0,0,0,0,0 };
	int32 npc_value[MAX_NPC_FACTIONS]={ 0,0,0,0,0,0,0,0,0,0 };
	uint8 temp[MAX_NPC_FACTIONS]={ 0,0,0,0,0,0,0,0,0,0 };
	int32 mod;
	int32 t;
	int32 tmpValue;
	int32 current_value;
	FactionMods fm;
	// Get the npc faction list
	if(!database.GetNPCFactionList(npc_id, faction_id, npc_value, temp))
		return;
	for(int i = 0;i<MAX_NPC_FACTIONS;i++)
	{
		if(faction_id[i] <= 0)
			continue;

		// Get the faction modifiers
		if(database.GetFactionData(&fm,char_class,char_race,char_deity,faction_id[i]))
		{
			// Get the characters current value with that faction
			current_value = GetCharacterFactionLevel(faction_id[i]);

			if(this->itembonuses.HeroicCHA) {
				int faction_mod = itembonuses.HeroicCHA / 5;
				// If our result isn't truncated, then just do that
				if(npc_value[i] * faction_mod / 100 != 0)
					npc_value[i] += npc_value[i] * faction_mod / 100;
				// If our result is truncated, then double a mob's value every once and a while to equal what they would have got
				else {
					if(MakeRandomInt(0, 100) < faction_mod)
						npc_value[i] *= 2;
				}
			}
			//figure out their modifier
			mod = fm.base + fm.class_mod + fm.race_mod + fm.deity_mod;
			if(mod > MAX_FACTION)
				mod = MAX_FACTION;
			else if(mod < MIN_FACTION)
				mod = MIN_FACTION;

			// Calculate the faction
			if(npc_value[i] != 0) {
				tmpValue = current_value + mod + npc_value[i];

				// Make sure faction hits don't go to GMs...
				if (m_pp.gm==1 && (tmpValue < current_value)) {
					tmpValue = current_value;
				}

				// Make sure we dont go over the min/max faction limits
				if(tmpValue >= MAX_FACTION)
				{
					t = MAX_FACTION - mod;
					if(current_value == t) {
						//do nothing, it is already maxed out
					} else if(!(database.SetCharacterFactionLevel(char_id, faction_id[i], t, temp[i], factionvalues)))
					{
						return;
					}
				}
				else if(tmpValue <= MIN_FACTION)
				{
					t = MIN_FACTION - mod;
					if(current_value == t) {
						//do nothing, it is already maxed out
					} else if(!(database.SetCharacterFactionLevel(char_id, faction_id[i], t, temp[i], factionvalues)))
					{
						return;
					}
				}
				else
				{
					if(!(database.SetCharacterFactionLevel(char_id, faction_id[i], current_value + npc_value[i], temp[i], factionvalues)))
					{
						return;
					}
				}
				if(tmpValue <= MIN_FACTION)
					tmpValue = MIN_FACTION;

				char* msg = BuildFactionMessage(npc_value[i],faction_id[i],tmpValue,temp[i]);
				if (msg != 0)
					Message(0, msg);
				safe_delete_array(msg);
			}
		}
	}
	return;
}

void Client::SetFactionLevel2(uint32 char_id, int32 faction_id, uint8 char_class, uint8 char_race, uint8 char_deity, int32 value, uint8 temp)
{
	_ZP(Client_SetFactionLevel2);
	int32 current_value;
	//Get the npc faction list
	if(faction_id > 0 && value != 0) {
		//Get the faction modifiers
		current_value = GetCharacterFactionLevel(faction_id) + value;
		if(!(database.SetCharacterFactionLevel(char_id, faction_id, current_value, temp, factionvalues)))
			return;

		char* msg = BuildFactionMessage(value, faction_id, current_value, temp);
		if (msg != 0)
			Message(0, msg);
		safe_delete(msg);

	}
	return;
}

int32 Client::GetCharacterFactionLevel(int32 faction_id)
{
	if (faction_id <= 0)
		return 0;
	faction_map::iterator res;
	res = factionvalues.find(faction_id);
	if(res == factionvalues.end())
		return(0);
	return(res->second);
}

// returns the character's faction level, adjusted for racial, class, and deity modifiers
int32 Client::GetModCharacterFactionLevel(int32 faction_id) {
	int32 Modded = GetCharacterFactionLevel(faction_id);
	FactionMods fm;
	if(database.GetFactionData(&fm,GetClass(),GetRace(),GetDeity(),faction_id))
		Modded += fm.base + fm.class_mod + fm.race_mod + fm.deity_mod;
	if (Modded > MAX_FACTION)
		Modded = MAX_FACTION;

	return Modded;
}

bool Client::HatedByClass(uint32 p_race, uint32 p_class, uint32 p_deity, int32 pFaction)
{

	bool Result = false;
	_ZP(Client_GetFactionLevel);

	int32 tmpFactionValue;
	FactionMods fmods;

	//First get the NPC's Primary faction
	if(pFaction > 0)
	{
		//Get the faction data from the database
		if(database.GetFactionData(&fmods, p_class, p_race, p_deity, pFaction))
		{
			tmpFactionValue = GetCharacterFactionLevel(pFaction);
			tmpFactionValue += GetFactionBonus(pFaction);
			tmpFactionValue += GetItemFactionBonus(pFaction);
			CalculateFaction(&fmods, tmpFactionValue);
			if(fmods.class_mod < fmods.race_mod)
				Result = true;
		}
	}
	return Result;
}

//o--------------------------------------------------------------
//| Name: BuildFactionMessage; rembrant, Dec. 16, 2001
//o--------------------------------------------------------------
//| Purpose: duh?
//o--------------------------------------------------------------
char* Client::BuildFactionMessage(int32 tmpvalue, int32 faction_id, int32 totalvalue, uint8 temp)
{
/*

This should be replaced to send string-ID based messages using:
#define FACTION_WORST 469 //Your faction standing with %1 could not possibly get any worse.
#define FACTION_WORSE 470 //Your faction standing with %1 got worse.
#define FACTION_BEST 471 //Your faction standing with %1 could not possibly get any better.
#define FACTION_BETTER 472 //Your faction standing with %1 got better.

some day.

*/
	//tmpvalue is the change as best I can tell.
	char *faction_message = 0;

	char name[50];

	if(database.GetFactionName(faction_id, name, sizeof(name)) == false) {
		snprintf(name, sizeof(name),"Faction%i",faction_id);
	}

	if(tmpvalue == 0 || temp == 1 || temp == 2) {
		return 0;
	}
	else if (totalvalue >= MAX_FACTION) {
		MakeAnyLenString(&faction_message, "Your faction standing with %s could not possibly get any better!", name);
		return faction_message;
	}
	else if(tmpvalue > 0 && totalvalue < MAX_FACTION) {
		MakeAnyLenString(&faction_message, "Your faction standing with %s has gotten better!", name);
		return faction_message;
	}
	else if(tmpvalue < 0 && totalvalue > MIN_FACTION) {
		MakeAnyLenString(&faction_message, "Your faction standing with %s has gotten worse!", name);
		return faction_message;
	}
	else if(totalvalue <= MIN_FACTION) {
		MakeAnyLenString(&faction_message, "Your faction standing with %s could not possibly get any worse!", name);
		return faction_message;
	}
	return 0;
}

void Client::LoadAccountFlags()
{
    char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;

    accountflags.clear();
    MakeAnyLenString(&query, "SELECT p_flag, p_value FROM account_flags WHERE p_accid = '%d'", account_id);
    if(database.RunQuery(query, strlen(query), errbuf, &result))
    {
        while(row = mysql_fetch_row(result))
        {
            std::string fname(row[0]);
            std::string fval(row[1]);
            accountflags[fname] = fval;
        }
        mysql_free_result(result);
    }
    else
    {
        std::cerr << "Error in LoadAccountFlags query '" << query << "' " << errbuf << std::endl;
    }
    safe_delete_array(query);
}

void Client::SetAccountFlag(std::string flag, std::string val)
{
    char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;

    MakeAnyLenString(&query, "REPLACE INTO account_flags (p_accid, p_flag, p_value) VALUES( '%d', '%s', '%s')", account_id, flag.c_str(), val.c_str());
    if(!database.RunQuery(query, strlen(query), errbuf))
    {
        std::cerr << "Error in SetAccountFlags query '" << query << "' " << errbuf << std::endl;
    }
    safe_delete_array(query);

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

    //Scan equip slots for items
    for(i = 0; i <= 21; i++)
    {
		TryItemTick(i);
    }
    //Scan main inventory + cursor
    for(i = 22; i < 31; i++)
    {
		TryItemTick(i);
    }
    //Scan bags
    for(i = 251; i < 340; i++)
    {
		TryItemTick(i);
    }
}

void Client::TryItemTick(int slot)
{
	int iid = 0;
    const ItemInst* inst = m_inv[slot];
    if(inst == 0) { return; }

    iid = inst->GetID();

    if(zone->tick_items.count(iid) > 0)
    {
        if( GetLevel() >= zone->tick_items[iid].level && MakeRandomInt(0, 100) >= (100 - zone->tick_items[iid].chance) && (zone->tick_items[iid].bagslot || slot < 22) )
        {
            ItemInst* e_inst = (ItemInst*)inst;
            parse->EventItem(EVENT_ITEM_TICK, this, e_inst, e_inst->GetID(), slot);
        }
    }

	//Only look at augs in main inventory
	if(slot > 21) { return; }

    for(int x = 0; x < MAX_AUGMENT_SLOTS; ++x)
    {
        ItemInst * a_inst = inst->GetAugment(x);
        if(!a_inst) { continue; }

        iid = a_inst->GetID();

        if(zone->tick_items.count(iid) > 0)
        {
            if( GetLevel() >= zone->tick_items[iid].level && MakeRandomInt(0, 100) >= (100 - zone->tick_items[iid].chance) )
            {
                ItemInst* e_inst = (ItemInst*)a_inst;
                parse->EventItem(EVENT_ITEM_TICK, this, e_inst, e_inst->GetID(), slot);
            }
        }
    }
}
