/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2003 EQEMu Development Team (http://eqemulator.net)

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

	client_process.cpp:
	Handles client login sequence and packets sent from client to zone
*/

#include "../common/eqemu_logsys.h"
#include "../common/global_define.h"
#include <iostream>
#include <stdio.h>
#include <zlib.h>

#ifdef _WINDOWS
	#include <winsock2.h>
	#include <windows.h>
	#define snprintf	_snprintf
	#define strncasecmp	_strnicmp
	#define strcasecmp	_stricmp
#else
	#include <pthread.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <unistd.h>
#endif

#include "../common/data_verification.h"
#include "../common/rulesys.h"
#include "../common/skills.h"
#include "../common/spdat.h"
#include "../common/string_util.h"
#include "event_codes.h"
#include "guild_mgr.h"
#include "map.h"
#include "petitions.h"
#include "queryserv.h"
#include "quest_parser_collection.h"
#include "string_ids.h"
#include "worldserver.h"
#include "zone.h"
#include "zonedb.h"

extern QueryServ* QServ;
extern Zone* zone;
extern volatile bool is_zone_loaded;
extern WorldServer worldserver;
extern PetitionList petition_list;
extern EntityList entity_list;

bool Client::Process() {
	bool ret = true;

	if (Connected() || IsLD()) {
		// try to send all packets that weren't sent before
		if (!IsLD() && zoneinpacket_timer.Check()) {
			SendAllPackets();
		}

		if (adventure_request_timer) {
			if (adventure_request_timer->Check()) {
				safe_delete(adventure_request_timer);
			}
		}

		if (adventure_create_timer) {
			if (adventure_create_timer->Check()) {
				safe_delete(adventure_create_timer);
			}
		}

		if (adventure_leave_timer) {
			if (adventure_leave_timer->Check()) {
				safe_delete(adventure_leave_timer);
			}
		}

		if (adventure_door_timer) {
			if (adventure_door_timer->Check()) {
				safe_delete(adventure_door_timer);
			}
		}

		if (adventure_stats_timer) {
			if (adventure_stats_timer->Check()) {
				safe_delete(adventure_stats_timer);
			}
		}

		if (adventure_leaderboard_timer) {
			if (adventure_leaderboard_timer->Check()) {
				safe_delete(adventure_leaderboard_timer);
			}
		}

		if (dead) {
			SetHP(-100);
			if (RespawnFromHoverTimer.Check())
				HandleRespawnFromHover(0);
		}

		if (IsTracking() && (ClientVersion() >= EQEmu::versions::ClientVersion::SoD) && TrackingTimer.Check())
			DoTracking();

		// SendHPUpdate calls hpupdate_timer.Start so it can delay this timer, so lets not reset with the check
		// since the function will anyways
		if (hpupdate_timer.Check(false))
			SendHPUpdate();

		/* I haven't naturally updated my position in 10 seconds, updating manually */
		if (!is_client_moving && position_update_timer.Check()) {
			SendPositionUpdate();
		}

		if (mana_timer.Check())
			CheckManaEndUpdate();

		if (dead && dead_timer.Check()) {
			database.MoveCharacterToZone(GetName(), database.GetZoneName(m_pp.binds[0].zoneId));

			m_pp.zone_id = m_pp.binds[0].zoneId;
			m_pp.zoneInstance = m_pp.binds[0].instance_id;
			m_pp.x = m_pp.binds[0].x;
			m_pp.y = m_pp.binds[0].y;
			m_pp.z = m_pp.binds[0].z;
			Save();

			Group *mygroup = GetGroup();
			if (mygroup)
			{
				entity_list.MessageGroup(this, true, 15, "%s died.", GetName());
				mygroup->MemberZoned(this);
			}
			Raid *myraid = entity_list.GetRaidByClient(this);
			if (myraid)
			{
				myraid->MemberZoned(this);
			}
			return(false);
		}

		if (charm_update_timer.Check()) {
			CalcItemScale();
		}

		if (TaskPeriodic_Timer.Check() && taskstate)
			taskstate->TaskPeriodicChecks(this);

		if (linkdead_timer.Check()) {
			LeaveGroup();
			Save();
			if (GetMerc()) {
				GetMerc()->Save();
				GetMerc()->Depop();
			}

			Raid *myraid = entity_list.GetRaidByClient(this);
			if (myraid) {
				myraid->MemberZoned(this);
			}
			return false; //delete client
		}

		if (camp_timer.Check()) {
			LeaveGroup();
			Save();
			if (GetMerc())
			{
				GetMerc()->Save();
				GetMerc()->Depop();
			}
			instalog = true;
		}

		if (IsStunned() && stunned_timer.Check())
			Mob::UnStun();

		if (!m_CheatDetectMoved) {
			m_TimeSinceLastPositionCheck = Timer::GetCurrentTime();
		}

		if (bardsong_timer.Check() && bardsong != 0) {
			//NOTE: this is kinda a heavy-handed check to make sure the mob still exists before
			//doing the next pulse on them...
			Mob *song_target = nullptr;
			if (bardsong_target_id == GetID()) {
				song_target = this;
			}
			else {
				song_target = entity_list.GetMob(bardsong_target_id);
			}

			if (song_target == nullptr) {
				InterruptSpell(SONG_ENDS_ABRUPTLY, 0x121, bardsong);
			}
			else {
				if (!ApplyNextBardPulse(bardsong, song_target, bardsong_slot))
					InterruptSpell(SONG_ENDS_ABRUPTLY, 0x121, bardsong);
				//SpellFinished(bardsong, bardsong_target, bardsong_slot, spells[bardsong].mana);
			}
		}

		if (GetMerc()) {
			UpdateMercTimer();
		}

		if (GetMercInfo().MercTemplateID != 0 && GetMercInfo().IsSuspended) {
			CheckMercSuspendTimer();
		}

		if (IsAIControlled())
			AI_Process();

		// Don't reset the bindwound timer so we can check it in BindWound as well.
		if (bindwound_timer.Check(false) && bindwound_target != 0) {
			BindWound(bindwound_target, false);
		}

		if (KarmaUpdateTimer) {
			if (KarmaUpdateTimer->Check(false)) {
				KarmaUpdateTimer->Start(RuleI(Chat, KarmaUpdateIntervalMS));
				database.UpdateKarma(AccountID(), ++TotalKarma);
			}
		}

		if (qGlobals) {
			if (qglobal_purge_timer.Check()) {
				qGlobals->PurgeExpiredGlobals();
			}
		}

		/* Build a close range list of NPC's  */
		if (npc_close_scan_timer.Check()) {
			close_mobs.clear();

			/* Force spawn updates when traveled far */
			bool force_spawn_updates = false;
			float client_update_range = (RuleI(Range, ClientForceSpawnUpdateRange) *  RuleI(Range, ClientForceSpawnUpdateRange));
			if (DistanceSquared(last_major_update_position, m_Position) >= client_update_range) {
				last_major_update_position = m_Position;
				force_spawn_updates = true;
			}

			float scan_range = (RuleI(Range, ClientNPCScan) * RuleI(Range, ClientNPCScan));
			auto &mob_list = entity_list.GetMobList();
			for (auto itr = mob_list.begin(); itr != mob_list.end(); ++itr) {
				Mob* mob = itr->second;

				float distance = DistanceSquared(m_Position, mob->GetPosition());
				if (mob->IsNPC()) {
					if (distance <= scan_range) {
						close_mobs.insert(std::pair<Mob *, float>(mob, distance));
					}
					else if (mob->GetAggroRange() > scan_range) {
						close_mobs.insert(std::pair<Mob *, float>(mob, distance));
					}
				}

				if (force_spawn_updates && mob != this) {

					if (mob->is_distance_roamer) {
						mob->SendPositionUpdateToClient(this);
						continue;
					}

					if (distance <= client_update_range)
						mob->SendPositionUpdateToClient(this);
				}

			}
		}

		bool may_use_attacks = false;
		/*
			Things which prevent us from attacking:
				- being under AI control, the AI does attacks
				- being dead
				- casting a spell and bard check
				- not having a target
				- being stunned or mezzed
				- having used a ranged weapon recently
		*/
		if (auto_attack) {
			if (!IsAIControlled() && !dead
				&& !(spellend_timer.Enabled() && casting_spell_id && !IsBardSong(casting_spell_id))
				&& !IsStunned() && !IsFeared() && !IsMezzed() && GetAppearance() != eaDead && !IsMeleeDisabled()
				)
				may_use_attacks = true;

			if (may_use_attacks && ranged_timer.Enabled()) {
				//if the range timer is enabled, we need to consider it
				if (!ranged_timer.Check(false)) {
					//the ranged timer has not elapsed, cannot attack.
					may_use_attacks = false;
				}
			}
		}

		if (AutoFireEnabled()) {
			EQEmu::ItemInstance *ranged = GetInv().GetItem(EQEmu::inventory::slotRange);
			if (ranged)
			{
				if (ranged->GetItem() && ranged->GetItem()->ItemType == EQEmu::item::ItemTypeBow) {
					if (ranged_timer.Check(false)) {
						if (GetTarget() && (GetTarget()->IsNPC() || GetTarget()->IsClient())) {
							if (GetTarget()->InFrontMob(this, GetTarget()->GetX(), GetTarget()->GetY())) {
								if (CheckLosFN(GetTarget())) {
									//client has built in los check, but auto fire does not.. done last.
									RangedAttack(GetTarget());
									if (CheckDoubleRangedAttack())
										RangedAttack(GetTarget(), true);
								}
								else
									ranged_timer.Start();
							}
							else
								ranged_timer.Start();
						}
						else
							ranged_timer.Start();
					}
				}
				else if (ranged->GetItem() && (ranged->GetItem()->ItemType == EQEmu::item::ItemTypeLargeThrowing || ranged->GetItem()->ItemType == EQEmu::item::ItemTypeSmallThrowing)) {
					if (ranged_timer.Check(false)) {
						if (GetTarget() && (GetTarget()->IsNPC() || GetTarget()->IsClient())) {
							if (GetTarget()->InFrontMob(this, GetTarget()->GetX(), GetTarget()->GetY())) {
								if (CheckLosFN(GetTarget())) {
									//client has built in los check, but auto fire does not.. done last.
									ThrowingAttack(GetTarget());
								}
								else
									ranged_timer.Start();
							}
							else
								ranged_timer.Start();
						}
						else
							ranged_timer.Start();
					}
				}
			}
		}

		Mob *auto_attack_target = GetTarget();
		if (auto_attack && auto_attack_target != nullptr && may_use_attacks && attack_timer.Check())
		{
			//check if change
			//only check on primary attack.. sorry offhand you gotta wait!
			if (aa_los_them_mob)
			{
				if (auto_attack_target != aa_los_them_mob ||
					m_AutoAttackPosition.x != GetX() ||
					m_AutoAttackPosition.y != GetY() ||
					m_AutoAttackPosition.z != GetZ() ||
					m_AutoAttackTargetLocation.x != aa_los_them_mob->GetX() ||
					m_AutoAttackTargetLocation.y != aa_los_them_mob->GetY() ||
					m_AutoAttackTargetLocation.z != aa_los_them_mob->GetZ())
				{
					aa_los_them_mob = auto_attack_target;
					m_AutoAttackPosition = GetPosition();
					m_AutoAttackTargetLocation = glm::vec3(aa_los_them_mob->GetPosition());
					los_status = CheckLosFN(auto_attack_target);
					los_status_facing = IsFacingMob(aa_los_them_mob);
				}
				// If only our heading changes, we can skip the CheckLosFN call
				// but above we still need to update los_status_facing
				if (m_AutoAttackPosition.w != GetHeading()) {
					m_AutoAttackPosition.w = GetHeading();
					los_status_facing = IsFacingMob(aa_los_them_mob);
				}
			}
			else
			{
				aa_los_them_mob = auto_attack_target;
				m_AutoAttackPosition = GetPosition();
				m_AutoAttackTargetLocation = glm::vec3(aa_los_them_mob->GetPosition());
				los_status = CheckLosFN(auto_attack_target);
				los_status_facing = IsFacingMob(aa_los_them_mob);
			}

			if (!CombatRange(auto_attack_target))
			{
				Message_StringID(MT_TooFarAway, TARGET_TOO_FAR);
			}
			else if (auto_attack_target == this)
			{
				Message_StringID(MT_TooFarAway, TRY_ATTACKING_SOMEONE);
			}
			else if (!los_status || !los_status_facing)
			{
				//you can't see your target
			}
			else if (auto_attack_target->GetHP() > -10) // -10 so we can watch people bleed in PvP
			{
				EQEmu::ItemInstance *wpn = GetInv().GetItem(EQEmu::inventory::slotPrimary);
				TryWeaponProc(wpn, auto_attack_target, EQEmu::inventory::slotPrimary);
				TriggerDefensiveProcs(auto_attack_target, EQEmu::inventory::slotPrimary, false);

				DoAttackRounds(auto_attack_target, EQEmu::inventory::slotPrimary);
				if (CheckAATimer(aaTimerRampage))
					entity_list.AEAttack(this, 30);
			}
		}

		if (GetClass() == WARRIOR || GetClass() == BERSERKER) {
			if (!dead && !IsBerserk() && GetHPRatio() < RuleI(Combat, BerserkerFrenzyStart)) {
				entity_list.MessageClose_StringID(this, false, 200, 0, BERSERK_START, GetName());
				berserk = true;
			}
			if (IsBerserk() && GetHPRatio() > RuleI(Combat, BerserkerFrenzyEnd)) {
				entity_list.MessageClose_StringID(this, false, 200, 0, BERSERK_END, GetName());
				berserk = false;
			}
		}

		if (auto_attack && may_use_attacks && auto_attack_target != nullptr
			&& CanThisClassDualWield() && attack_dw_timer.Check())
		{
			// Range check
			if (!CombatRange(auto_attack_target)) {
				// this is a duplicate message don't use it.
				//Message_StringID(MT_TooFarAway,TARGET_TOO_FAR);
			}
			// Don't attack yourself
			else if (auto_attack_target == this) {
				//Message_StringID(MT_TooFarAway,TRY_ATTACKING_SOMEONE);
			}
			else if (!los_status || !los_status_facing)
			{
				//you can't see your target
			}
			else if (auto_attack_target->GetHP() > -10) {
				CheckIncreaseSkill(EQEmu::skills::SkillDualWield, auto_attack_target, -10);
				if (CheckDualWield()) {
					EQEmu::ItemInstance *wpn = GetInv().GetItem(EQEmu::inventory::slotSecondary);
					TryWeaponProc(wpn, auto_attack_target, EQEmu::inventory::slotSecondary);

					DoAttackRounds(auto_attack_target, EQEmu::inventory::slotSecondary);
				}
			}
		}

		if (position_timer.Check()) {
			if (IsAIControlled())
			{
				if (!IsMoving())
				{
					animation = 0;
					m_Delta = glm::vec4(0.0f, 0.0f, 0.0f, m_Delta.w);
					SendPositionUpdate(2);
				}
			}

			// Send a position packet every 8 seconds - if not done, other clients
			// see this char disappear after 10-12 seconds of inactivity
			if (position_timer_counter >= 36) { // Approx. 4 ticks per second
				entity_list.SendPositionUpdates(this, pLastUpdateWZ, RuleI(Range, MobPositionUpdates), GetTarget(), true);
				pLastUpdate = Timer::GetCurrentTime();
				pLastUpdateWZ = pLastUpdate;
				position_timer_counter = 0;
			}
			else {
				pLastUpdate = Timer::GetCurrentTime();
				position_timer_counter++;
			}
		}

		if (HasVirus()) {
			if (viral_timer.Check()) {
				viral_timer_counter++;
				for (int i = 0; i < MAX_SPELL_TRIGGER * 2; i += 2) {
					if (viral_spells[i]) {
						if (viral_timer_counter % spells[viral_spells[i]].viral_timer == 0) {
							SpreadVirus(viral_spells[i], viral_spells[i + 1]);
						}
					}
				}
			}
			if (viral_timer_counter > 999)
				viral_timer_counter = 0;
		}

		ProjectileAttack();

		if (spellbonuses.GravityEffect == 1) {
			if (gravity_timer.Check())
				DoGravityEffect();
		}

		if (shield_timer.Check())
		{
			if (shield_target)
			{
				if (!CombatRange(shield_target))
				{
					entity_list.MessageClose_StringID(this, false, 100, 0,
						END_SHIELDING, GetCleanName(), shield_target->GetCleanName());
					for (int y = 0; y < 2; y++)
					{
						if (shield_target->shielder[y].shielder_id == GetID())
						{
							shield_target->shielder[y].shielder_id = 0;
							shield_target->shielder[y].shielder_bonus = 0;
						}
					}
					shield_target = 0;
					shield_timer.Disable();
				}
			}
			else
			{
				shield_target = 0;
				shield_timer.Disable();
			}
		}

		SpellProcess();
		if (endupkeep_timer.Check() && !dead) {
			DoEnduranceUpkeep();
		}

		// this is independent of the tick timer
		if (consume_food_timer.Check())
			DoStaminaHungerUpdate();

		if (tic_timer.Check() && !dead) {
			CalcMaxHP();
			CalcMaxMana();
			CalcATK();
			CalcMaxEndurance();
			CalcRestState();
			DoHPRegen();
			DoManaRegen();
			DoEnduranceRegen();
			BuffProcess();

			if (tribute_timer.Check()) {
				ToggleTribute(true);	//re-activate the tribute.
			}

			if (fishing_timer.Check()) {
				GoFish();
			}

			if (autosave_timer.Check()) {
				Save(0);
			}

			if (m_pp.intoxication > 0)
			{
				--m_pp.intoxication;
				CalcBonuses();
			}

			if (ItemTickTimer.Check())
			{
				TickItemCheck();
			}

			if (ItemQuestTimer.Check())
			{
				ItemTimerCheck();
			}
		}
	}

	if (client_state == CLIENT_KICKED) {
		Save();
		OnDisconnect(true);
		std::cout << "Client disconnected (cs=k): " << GetName() << std::endl;
		return false;
	}

	if (client_state == DISCONNECTED) {
		OnDisconnect(true);
		std::cout << "Client disconnected (cs=d): " << GetName() << std::endl;
		database.SetMQDetectionFlag(this->AccountName(), GetName(), "/MQInstantCamp: Possible instant camp disconnect.", zone->GetShortName());
		return false;
	}

	if (client_state == CLIENT_ERROR) {
		OnDisconnect(true);
		std::cout << "Client disconnected (cs=e): " << GetName() << std::endl;
		return false;
	}

	if (client_state != CLIENT_LINKDEAD && !eqs->CheckState(ESTABLISHED)) {
		OnDisconnect(true);
		Log(Logs::General, Logs::Zone_Server, "Client linkdead: %s", name);

		if (GetGM()) {
			if (GetMerc())
			{
				GetMerc()->Save();
				GetMerc()->Depop();
			}
			return false;
		}
		else if (!linkdead_timer.Enabled()) {
			linkdead_timer.Start(RuleI(Zone, ClientLinkdeadMS));
			client_state = CLIENT_LINKDEAD;
			AI_Start(CLIENT_LD_TIMEOUT);
			SendAppearancePacket(AT_Linkdead, 1);
		}
	}


	/************ Get all packets from packet manager out queue and process them ************/
	EQApplicationPacket *app = nullptr;
	if (!eqs->CheckState(CLOSING))
	{
		while (ret && (app = (EQApplicationPacket *)eqs->PopPacket())) {
			if (app)
				ret = HandlePacket(app);
			safe_delete(app);
		}
	}

	//At this point, we are still connected, everything important has taken
	//place, now check to see if anybody wants to aggro us.
	// only if client is not feigned
	if (zone->CanDoCombat() && ret && !GetFeigned() && client_scan_npc_aggro_timer.Check()) {
		int npc_scan_count = 0;
		for (auto it = close_mobs.begin(); it != close_mobs.end(); ++it) {
			Mob *mob = it->first;

			if (!mob)
				continue;

			if (mob->IsClient())
				continue;

			if (mob->CheckWillAggro(this) && !mob->CheckAggro(this)) {
				mob->AddToHateList(this, 25);
			}
			npc_scan_count++;
		}
		Log(Logs::General, Logs::Aggro, "Checking Reverse Aggro (client->npc) scanned_npcs (%i)", npc_scan_count);
	}

	if (client_state != CLIENT_LINKDEAD && (client_state == CLIENT_ERROR || client_state == DISCONNECTED || client_state == CLIENT_KICKED || !eqs->CheckState(ESTABLISHED)))
	{
		//client logged out or errored out
		//ResetTrade();
		if (client_state != CLIENT_KICKED && !zoning && !instalog) {
			Save();
		}

		client_state = CLIENT_LINKDEAD;
		if (zoning || instalog || GetGM())
		{
			Group *mygroup = GetGroup();
			if (mygroup)
			{
				if (!zoning)
				{
					entity_list.MessageGroup(this, true, 15, "%s logged out.", GetName());
					LeaveGroup();
				}
				else
				{
					entity_list.MessageGroup(this, true, 15, "%s left the zone.", GetName());
					mygroup->MemberZoned(this);
					if (GetMerc() && GetMerc()->HasGroup())
					{
						GetMerc()->RemoveMercFromGroup(GetMerc(), GetMerc()->GetGroup());
					}
				}

			}
			Raid *myraid = entity_list.GetRaidByClient(this);
			if (myraid)
			{
				if (!zoning)
				{
					//entity_list.MessageGroup(this,true,15,"%s logged out.",GetName());
					myraid->MemberZoned(this);
				}
				else
				{
					//entity_list.MessageGroup(this,true,15,"%s left the zone.",GetName());
					myraid->MemberZoned(this);
				}
			}
			OnDisconnect(false);
			return false;
		}
		else
		{
			LinkDead();
		}
		OnDisconnect(true);
	}
	// Feign Death 2 minutes and zone forgets you
	if (forget_timer.Check()) {
		forget_timer.Disable();
		entity_list.ClearZoneFeignAggro(this);
		Message(0, "Your enemies have forgotten you!");
	}

	if (client_state == CLIENT_CONNECTED) {
		if (m_dirtyautohaters)
			ProcessXTargetAutoHaters();
		if (aggro_meter_timer.Check())
			ProcessAggroMeter();
	}

	return ret;
}

/* Just a set of actions preformed all over in Client::Process */
void Client::OnDisconnect(bool hard_disconnect) {
	if(hard_disconnect)
	{
		LeaveGroup();
		if (GetMerc())
		{
			GetMerc()->Save();
			GetMerc()->Depop();
		}
		Raid *MyRaid = entity_list.GetRaidByClient(this);

		if (MyRaid)
			MyRaid->MemberZoned(this);

		parse->EventPlayer(EVENT_DISCONNECT, this, "", 0);

		/* QS: PlayerLogConnectDisconnect */
		if (RuleB(QueryServ, PlayerLogConnectDisconnect)){
			std::string event_desc = StringFormat("Disconnect :: in zoneid:%i instid:%i", this->GetZoneID(), this->GetInstanceID());
			QServ->PlayerLogEvent(Player_Log_Connect_State, this->CharacterID(), event_desc);
		}
	}

	RemoveAllAuras();

	Mob *Other = trade->With();
	if(Other)
	{
		Log(Logs::Detail, Logs::Trading, "Client disconnected during a trade. Returning their items.");
		FinishTrade(this);

		if(Other->IsClient())
			Other->CastToClient()->FinishTrade(Other);

		/* Reset both sides of the trade */
		trade->Reset();
		Other->trade->Reset();
	}

	database.SetFirstLogon(CharacterID(), 0); //We change firstlogon status regardless of if a player logs out to zone or not, because we only want to trigger it on their first login from world.

	/* Remove ourself from all proximities */
	ClearAllProximities();

	auto outapp = new EQApplicationPacket(OP_LogoutReply);
	FastQueuePacket(&outapp);

	Disconnect();
}

// Sends the client complete inventory used in character login
void Client::BulkSendInventoryItems()
{
	// LINKDEAD TRADE ITEMS
	// Move trade slot items back into normal inventory..need them there now for the proceeding validity checks
	for (int16 slot_id = EQEmu::legacy::TRADE_BEGIN; slot_id <= EQEmu::legacy::TRADE_END; slot_id++) {
		EQEmu::ItemInstance* inst = m_inv.PopItem(slot_id);
		if(inst) {
			bool is_arrow = (inst->GetItem()->ItemType == EQEmu::item::ItemTypeArrow) ? true : false;
			int16 free_slot_id = m_inv.FindFreeSlot(inst->IsClassBag(), true, inst->GetItem()->Size, is_arrow);
			Log(Logs::Detail, Logs::Inventory, "Incomplete Trade Transaction: Moving %s from slot %i to %i", inst->GetItem()->Name, slot_id, free_slot_id);
			PutItemInInventory(free_slot_id, *inst, false);
			database.SaveInventory(character_id, nullptr, slot_id);
			safe_delete(inst);
		}
	}

	bool deletenorent = database.NoRentExpired(GetName());
	if (deletenorent) { //client was offline for more than 30 minutes, delete no rent items
		if (RuleB(Inventory, TransformSummonedBags))
			DisenchantSummonedBags(false);
		RemoveNoRent(false);
	}

	RemoveDuplicateLore(false);
	MoveSlotNotAllowed(false);

	EQEmu::OutBuffer ob;
	EQEmu::OutBuffer::pos_type last_pos = ob.tellp();

	// Possessions items
	for (int16 slot_id = EQEmu::inventory::slotBegin; slot_id < EQEmu::legacy::TYPE_POSSESSIONS_SIZE; slot_id++) {
		const EQEmu::ItemInstance* inst = m_inv[slot_id];
		if (!inst)
			continue;

		inst->Serialize(ob, slot_id);

		if (ob.tellp() == last_pos)
			Log(Logs::General, Logs::Inventory, "Serialization failed on item slot %d during BulkSendInventoryItems.  Item skipped.", slot_id);
		
		last_pos = ob.tellp();
	}

	// PowerSource item
	if (ClientVersion() >= EQEmu::versions::ClientVersion::SoF) {
		const EQEmu::ItemInstance* inst = m_inv[EQEmu::inventory::slotPowerSource];
		if (inst) {
			inst->Serialize(ob, EQEmu::inventory::slotPowerSource);

			if (ob.tellp() == last_pos)
				Log(Logs::General, Logs::Inventory, "Serialization failed on item slot %d during BulkSendInventoryItems.  Item skipped.", EQEmu::inventory::slotPowerSource);

			last_pos = ob.tellp();
		}
	}

	// Bank items
	for (int16 slot_id = EQEmu::legacy::BANK_BEGIN; slot_id <= EQEmu::legacy::BANK_END; slot_id++) {
		const EQEmu::ItemInstance* inst = m_inv[slot_id];
		if (!inst)
			continue;

		inst->Serialize(ob, slot_id);

		if (ob.tellp() == last_pos)
			Log(Logs::General, Logs::Inventory, "Serialization failed on item slot %d during BulkSendInventoryItems.  Item skipped.", slot_id);

		last_pos = ob.tellp();
	}

	// SharedBank items
	for (int16 slot_id = EQEmu::legacy::SHARED_BANK_BEGIN; slot_id <= EQEmu::legacy::SHARED_BANK_END; slot_id++) {
		const EQEmu::ItemInstance* inst = m_inv[slot_id];
		if (!inst)
			continue;

		inst->Serialize(ob, slot_id);

		if (ob.tellp() == last_pos)
			Log(Logs::General, Logs::Inventory, "Serialization failed on item slot %d during BulkSendInventoryItems.  Item skipped.", slot_id);

		last_pos = ob.tellp();
	}

	auto outapp = new EQApplicationPacket(OP_CharInventory);
	outapp->size = ob.size();
	outapp->pBuffer = ob.detach();
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::BulkSendMerchantInventory(int merchant_id, int npcid) {
	const EQEmu::ItemData* handyitem = nullptr;
	uint32 numItemSlots = 80; //The max number of items passed in the transaction.
	if (m_ClientVersionBit & EQEmu::versions::bit_RoFAndLater) { // RoF+ can send 200 items
		numItemSlots = 200;
	}
	const EQEmu::ItemData *item = nullptr;
	std::list<MerchantList> merlist = zone->merchanttable[merchant_id];
	std::list<MerchantList>::const_iterator itr;
	Mob* merch = entity_list.GetMobByNpcTypeID(npcid);
	if (merlist.size() == 0) { //Attempt to load the data, it might have been missed if someone spawned the merchant after the zone was loaded
		zone->LoadNewMerchantData(merchant_id);
		merlist = zone->merchanttable[merchant_id];
		if (merlist.size() == 0)
			return;
	}
	std::list<TempMerchantList> tmp_merlist = zone->tmpmerchanttable[npcid];
	std::list<TempMerchantList>::iterator tmp_itr;

	uint32 i = 1;
	uint8 handychance = 0;
	for (itr = merlist.begin(); itr != merlist.end() && i <= numItemSlots; ++itr) {
		MerchantList ml = *itr;
		if (merch->CastToNPC()->GetMerchantProbability() > ml.probability)
			continue;

		if (GetLevel() < ml.level_required)
			continue;

		if (!(ml.classes_required & (1 << (GetClass() - 1))))
			continue;

		int32 fac = merch ? merch->GetPrimaryFaction() : 0;
		int32 cur_fac_level;
		if (fac == 0 || sneaking) {
			cur_fac_level = 0;
		}
		else {
			cur_fac_level = GetModCharacterFactionLevel(fac);
		}
			
		if (cur_fac_level < ml.faction_required)
			continue;

		handychance = zone->random.Int(0, merlist.size() + tmp_merlist.size() - 1);

		item = database.GetItem(ml.item);
		if (item) {
			if (handychance == 0)
				handyitem = item;
			else
				handychance--;
			int charges = 1;
			if (item->IsClassCommon())
				charges = item->MaxCharges;
			EQEmu::ItemInstance* inst = database.CreateItem(item, charges);
			if (inst) {
				if (RuleB(Merchant, UsePriceMod)) {
					inst->SetPrice((item->Price * (RuleR(Merchant, SellCostMod)) * item->SellRate * Client::CalcPriceMod(merch, false)));
				}
				else
					inst->SetPrice((item->Price * (RuleR(Merchant, SellCostMod)) * item->SellRate));
				inst->SetMerchantSlot(ml.slot);
				inst->SetMerchantCount(-1);		//unlimited
				if (charges > 0)
					inst->SetCharges(charges);
				else
					inst->SetCharges(1);

				SendItemPacket(ml.slot - 1, inst, ItemPacketMerchant);
				safe_delete(inst);
			}
		}
		// Account for merchant lists with gaps.
		if (ml.slot >= i) {
			if (ml.slot > i)
				Log(Logs::General, Logs::None, "(WARNING) Merchantlist contains gap at slot %d. Merchant: %d, NPC: %d", i, merchant_id, npcid);
			i = ml.slot + 1;
		}
	}
	std::list<TempMerchantList> origtmp_merlist = zone->tmpmerchanttable[npcid];
	tmp_merlist.clear();
	for (tmp_itr = origtmp_merlist.begin(); tmp_itr != origtmp_merlist.end() && i <= numItemSlots; ++tmp_itr) {
		TempMerchantList ml = *tmp_itr;
		item = database.GetItem(ml.item);
		ml.slot = i;
		if (item) {
			if (handychance == 0)
				handyitem = item;
			else
				handychance--;
			int charges = 1;
			//if(item->ItemClass==ItemClassCommon && (int16)ml.charges <= item->MaxCharges)
			//	charges=ml.charges;
			//else
			charges = item->MaxCharges;
			EQEmu::ItemInstance* inst = database.CreateItem(item, charges);
			if (inst) {
				if (RuleB(Merchant, UsePriceMod)) {
					inst->SetPrice((item->Price * (RuleR(Merchant, SellCostMod)) * item->SellRate * Client::CalcPriceMod(merch, false)));
				}
				else
					inst->SetPrice((item->Price * (RuleR(Merchant, SellCostMod)) * item->SellRate));
				inst->SetMerchantSlot(ml.slot);
				inst->SetMerchantCount(ml.charges);
				if(charges > 0)
					inst->SetCharges(item->MaxCharges);//inst->SetCharges(charges);
				else
					inst->SetCharges(1);
				SendItemPacket(ml.slot-1, inst, ItemPacketMerchant);
				safe_delete(inst);
			}
		}
		tmp_merlist.push_back(ml);
		i++;
	}
	//this resets the slot
	zone->tmpmerchanttable[npcid] = tmp_merlist;
	if (merch != nullptr && handyitem) {
		char handy_id[8] = { 0 };
		int greeting = zone->random.Int(0, 4);
		int greet_id = 0;
		switch (greeting) {
			case 1:
				greet_id = MERCHANT_GREETING;
				break;
			case 2:
				greet_id = MERCHANT_HANDY_ITEM1;
				break;
			case 3:
				greet_id = MERCHANT_HANDY_ITEM2;
				break;
			case 4:
				greet_id = MERCHANT_HANDY_ITEM3;
				break;
			default:
				greet_id = MERCHANT_HANDY_ITEM4;
		}
		sprintf(handy_id, "%i", greet_id);

		if (greet_id != MERCHANT_GREETING)
			Message_StringID(10, GENERIC_STRINGID_SAY, merch->GetCleanName(), handy_id, this->GetName(), handyitem->Name);
		else
			Message_StringID(10, GENERIC_STRINGID_SAY, merch->GetCleanName(), handy_id, this->GetName());
	}

//		safe_delete_array(cpi);
}

uint8 Client::WithCustomer(uint16 NewCustomer){

	if(NewCustomer == 0) {
		CustomerID = 0;
		return 0;
	}

	if(CustomerID == 0) {
		CustomerID = NewCustomer;
		return 1;
	}

	// Check that the player browsing our wares hasn't gone away.

	Client* c = entity_list.GetClientByID(CustomerID);

	if(!c) {
		Log(Logs::Detail, Logs::Trading, "Previous customer has gone away.");
		CustomerID = NewCustomer;
		return 1;
	}

	return 0;
}

void Client::OPRezzAnswer(uint32 Action, uint32 SpellID, uint16 ZoneID, uint16 InstanceID, float x, float y, float z)
{
	if(PendingRezzXP < 0) {
		// pendingrezexp is set to -1 if we are not expecting an OP_RezzAnswer
		Log(Logs::Detail, Logs::Spells, "Unexpected OP_RezzAnswer. Ignoring it.");
		Message(13, "You have already been resurrected.\n");
		return;
	}

	if (Action == 1)
	{
		// Mark the corpse as rezzed in the database, just in case the corpse has buried, or the zone the
		// corpse is in has shutdown since the rez spell was cast.
		database.MarkCorpseAsRezzed(PendingRezzDBID);
		Log(Logs::Detail, Logs::Spells, "Player %s got a %i Rezz, spellid %i in zone%i, instance id %i",
				this->name, (uint16)spells[SpellID].base[0],
				SpellID, ZoneID, InstanceID);

		this->BuffFadeNonPersistDeath();
		int SpellEffectDescNum = GetSpellEffectDescNum(SpellID);
		// Rez spells with Rez effects have this DescNum (first is Titanium, second is 6.2 Client)
		if((SpellEffectDescNum == 82) || (SpellEffectDescNum == 39067)) {
			SetMana(0);
			SetHP(GetMaxHP()/5);
			int rez_eff = 756;
			if (RuleB(Character, UseOldRaceRezEffects) &&
			    (GetRace() == BARBARIAN || GetRace() == DWARF || GetRace() == TROLL || GetRace() == OGRE))
				rez_eff = 757;
			SpellOnTarget(rez_eff, this); // Rezz effects
		}
		else {
			SetMana(GetMaxMana());
			SetHP(GetMaxHP());
		}
		if(spells[SpellID].base[0] < 100 && spells[SpellID].base[0] > 0 && PendingRezzXP > 0)
		{
				SetEXP(((int)(GetEXP()+((float)((PendingRezzXP / 100) * spells[SpellID].base[0])))),
						GetAAXP(),true);
		}
		else if (spells[SpellID].base[0] == 100 && PendingRezzXP > 0) {
			SetEXP((GetEXP() + PendingRezzXP), GetAAXP(), true);
		}

		//Was sending the packet back to initiate client zone...
		//but that could be abusable, so lets go through proper channels
		MovePC(ZoneID, InstanceID, x, y, z, GetHeading(), 0, ZoneSolicited);
		entity_list.RefreshClientXTargets(this);
	}
	PendingRezzXP = -1;
	PendingRezzSpellID = 0;
}

void Client::OPTGB(const EQApplicationPacket *app)
{
	if(!app) return;
	if(!app->pBuffer) return;

	uint32 tgb_flag = *(uint32 *)app->pBuffer;
	if(tgb_flag == 2)
		Message_StringID(0, TGB() ? TGB_ON : TGB_OFF);
	else
		tgb = tgb_flag;
}

void Client::OPMemorizeSpell(const EQApplicationPacket* app)
{
	if(app->size != sizeof(MemorizeSpell_Struct))
	{
		Log(Logs::General, Logs::Error, "Wrong size on OP_MemorizeSpell. Got: %i, Expected: %i", app->size, sizeof(MemorizeSpell_Struct));
		DumpPacket(app);
		return;
	}

	const MemorizeSpell_Struct* memspell = (const MemorizeSpell_Struct*) app->pBuffer;

	if(!IsValidSpell(memspell->spell_id))
	{
		Message(13, "Unexpected error: spell id out of range");
		return;
	}

	if
	(
		GetClass() > 16 ||
		GetLevel() < spells[memspell->spell_id].classes[GetClass()-1]
	)
	{
		char val1[20]={0};
		Message_StringID(13,SPELL_LEVEL_TO_LOW,ConvertArray(spells[memspell->spell_id].classes[GetClass()-1],val1),spells[memspell->spell_id].name);
		//Message(13, "Unexpected error: Class cant use this spell at your level!");
		return;
	}

	switch(memspell->scribing)
	{
		case memSpellScribing:	{	// scribing spell to book
			const EQEmu::ItemInstance* inst = m_inv[EQEmu::inventory::slotCursor];

			if (inst && inst->IsClassCommon())
			{
				const EQEmu::ItemData* item = inst->GetItem();

				if (RuleB(Character, RestrictSpellScribing) && !item->IsEquipable(GetRace(), GetClass())) {
					Message_StringID(13, CANNOT_USE_ITEM);
					break;
				}

				if(item && item->Scroll.Effect == (int32)(memspell->spell_id))
				{
					ScribeSpell(memspell->spell_id, memspell->slot);
					DeleteItemInInventory(EQEmu::inventory::slotCursor, 1, true);
				}
				else
					Message(0,"Scribing spell: inst exists but item does not or spell ids do not match.");
			}
			else
				Message(0,"Scribing a spell without an inst on your cursor?");
			break;
		}
		case memSpellMemorize:	{	// memming spell
			if(HasSpellScribed(memspell->spell_id))
			{
				MemSpell(memspell->spell_id, memspell->slot);
			}
			else
			{
				database.SetMQDetectionFlag(AccountName(), GetName(), "OP_MemorizeSpell but we don't have this spell scribed...", zone->GetShortName());
			}
			break;
		}
		case memSpellForget:	{	// unmemming spell
			UnmemSpell(memspell->slot);
			break;
		}
	}

	Save();
}

void Client::CancelSneakHide()
{
	if (hidden || improved_hidden) {
		auto app = new EQApplicationPacket(OP_CancelSneakHide, 0);
		FastQueuePacket(&app);
		// SoF and Tit send back a OP_SpawnAppearance turning off AT_Invis
		// so we need to handle our sneaking flag only
		// The later clients send back a OP_Hide (this has a size but data is 0)
		// as well as OP_SpawnAppearance with AT_Invis and one with AT_Sneak
		// So we don't have to handle any of those flags
		if (ClientVersionBit() & EQEmu::versions::bit_SoFAndEarlier)
			sneaking = false;
	}
}

void Client::BreakInvis()
{
	if (invisible)
	{
		auto outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
		SpawnAppearance_Struct* sa_out = (SpawnAppearance_Struct*)outapp->pBuffer;
		sa_out->spawn_id = GetID();
		sa_out->type = 0x03;
		sa_out->parameter = 0;
		entity_list.QueueClients(this, outapp, true);
		safe_delete(outapp);
		invisible = false;
		invisible_undead = false;
		invisible_animals = false;
		hidden = false;
		improved_hidden = false;
	}
}

static uint64 CoinTypeCoppers(uint32 type) {
	switch(type) {
	case COINTYPE_PP:
		return(1000);
	case COINTYPE_GP:
		return(100);
	case COINTYPE_SP:
		return(10);
	case COINTYPE_CP:
	default:
		break;
	}
	return(1);
}

void Client::OPMoveCoin(const EQApplicationPacket* app)
{
	MoveCoin_Struct* mc = (MoveCoin_Struct*)app->pBuffer;
	uint64 value = 0, amount_to_take = 0, amount_to_add = 0;
	int32 *from_bucket = 0, *to_bucket = 0;
	Mob* trader = trade->With();

	// if amount < 0, client is sending a malicious packet
	if (mc->amount < 0)
	{
		return;
	}
	
	// could just do a range, but this is clearer and explicit
	if
	(
		(
			mc->cointype1 != COINTYPE_PP &&
			mc->cointype1 != COINTYPE_GP &&
			mc->cointype1 != COINTYPE_SP &&
			mc->cointype1 != COINTYPE_CP
		) ||
		(
			mc->cointype2 != COINTYPE_PP &&
			mc->cointype2 != COINTYPE_GP &&
			mc->cointype2 != COINTYPE_SP &&
			mc->cointype2 != COINTYPE_CP
		)
	)
	{
		return;
	}

	switch(mc->from_slot)
	{
		case -1:	// destroy
		{
			// I don't think you can move coin from the void,
			// but need to check this
			break;
		}
		case 0:	// cursor
		{
			switch(mc->cointype1)
			{
				case COINTYPE_PP:
					from_bucket = (int32 *) &m_pp.platinum_cursor; break;
				case COINTYPE_GP:
					from_bucket = (int32 *) &m_pp.gold_cursor; break;
				case COINTYPE_SP:
					from_bucket = (int32 *) &m_pp.silver_cursor; break;
				case COINTYPE_CP:
					from_bucket = (int32 *) &m_pp.copper_cursor; break;
			}
			break;
		}
		case 1:	// inventory
		{
			switch(mc->cointype1)
			{
				case COINTYPE_PP:
					from_bucket = (int32 *) &m_pp.platinum; break;
				case COINTYPE_GP:
					from_bucket = (int32 *) &m_pp.gold; break;
				case COINTYPE_SP:
					from_bucket = (int32 *) &m_pp.silver; break;
				case COINTYPE_CP:
					from_bucket = (int32 *) &m_pp.copper; break;
			}
			break;
		}
		case 2:	// bank
		{
			uint32 distance = 0;
			NPC *banker = entity_list.GetClosestBanker(this, distance);
			if(!banker || distance > USE_NPC_RANGE2)
			{
				char *hacked_string = nullptr;
				MakeAnyLenString(&hacked_string, "Player tried to make use of a banker(coin move) but %s is non-existant or too far away (%u units).",
					banker ? banker->GetName() : "UNKNOWN NPC", distance);
				database.SetMQDetectionFlag(AccountName(), GetName(), hacked_string, zone->GetShortName());
				safe_delete_array(hacked_string);
				return;
			}

			switch(mc->cointype1)
			{
				case COINTYPE_PP:
					from_bucket = (int32 *) &m_pp.platinum_bank; break;
				case COINTYPE_GP:
					from_bucket = (int32 *) &m_pp.gold_bank; break;
				case COINTYPE_SP:
					from_bucket = (int32 *) &m_pp.silver_bank; break;
				case COINTYPE_CP:
					from_bucket = (int32 *) &m_pp.copper_bank; break;
			}
			break;
		}
		case 3:	// trade
		{
			// can't move coin from trade
			break;
		}
		case 4:	// shared bank
		{
			uint32 distance = 0;
			NPC *banker = entity_list.GetClosestBanker(this, distance);
			if(!banker || distance > USE_NPC_RANGE2)
			{
				char *hacked_string = nullptr;
				MakeAnyLenString(&hacked_string, "Player tried to make use of a banker(shared coin move) but %s is non-existant or too far away (%u units).",
					banker ? banker->GetName() : "UNKNOWN NPC", distance);
				database.SetMQDetectionFlag(AccountName(), GetName(), hacked_string, zone->GetShortName());
				safe_delete_array(hacked_string);
				return;
			}
			if(mc->cointype1 == COINTYPE_PP)	// there's only platinum here
				from_bucket = (int32 *) &m_pp.platinum_shared;
			break;
		}
	}

	switch(mc->to_slot)
	{
		case -1:	// destroy
		{
			// no action required
			break;
		}
		case 0:	// cursor
		{
			switch(mc->cointype2)
			{
				case COINTYPE_PP:
					to_bucket = (int32 *) &m_pp.platinum_cursor; break;
				case COINTYPE_GP:
					to_bucket = (int32 *) &m_pp.gold_cursor; break;
				case COINTYPE_SP:
					to_bucket = (int32 *) &m_pp.silver_cursor; break;
				case COINTYPE_CP:
					to_bucket = (int32 *) &m_pp.copper_cursor; break;
			}
			break;
		}
		case 1:	// inventory
		{
			switch(mc->cointype2)
			{
				case COINTYPE_PP:
					to_bucket = (int32 *) &m_pp.platinum; break;
				case COINTYPE_GP:
					to_bucket = (int32 *) &m_pp.gold; break;
				case COINTYPE_SP:
					to_bucket = (int32 *) &m_pp.silver; break;
				case COINTYPE_CP:
					to_bucket = (int32 *) &m_pp.copper; break;
			}
			break;
		}
		case 2:	// bank
		{
			uint32 distance = 0;
			NPC *banker = entity_list.GetClosestBanker(this, distance);
			if(!banker || distance > USE_NPC_RANGE2)
			{
				char *hacked_string = nullptr;
				MakeAnyLenString(&hacked_string, "Player tried to make use of a banker(coin move) but %s is non-existant or too far away (%u units).",
					banker ? banker->GetName() : "UNKNOWN NPC", distance);
				database.SetMQDetectionFlag(AccountName(), GetName(), hacked_string, zone->GetShortName());
				safe_delete_array(hacked_string);
				return;
			}
			switch(mc->cointype2)
			{
				case COINTYPE_PP:
					to_bucket = (int32 *) &m_pp.platinum_bank; break;
				case COINTYPE_GP:
					to_bucket = (int32 *) &m_pp.gold_bank; break;
				case COINTYPE_SP:
					to_bucket = (int32 *) &m_pp.silver_bank; break;
				case COINTYPE_CP:
					to_bucket = (int32 *) &m_pp.copper_bank; break;
			}
			break;
		}
		case 3:	// trade
		{
			if(trader)
			{
				switch(mc->cointype2)
				{
					case COINTYPE_PP:
						to_bucket = (int32 *) &trade->pp; break;
					case COINTYPE_GP:
						to_bucket = (int32 *) &trade->gp; break;
					case COINTYPE_SP:
						to_bucket = (int32 *) &trade->sp; break;
					case COINTYPE_CP:
						to_bucket = (int32 *) &trade->cp; break;
				}
			}
			break;
		}
		case 4:	// shared bank
		{
			uint32 distance = 0;
			NPC *banker = entity_list.GetClosestBanker(this, distance);
			if(!banker || distance > USE_NPC_RANGE2)
			{
				char *hacked_string = nullptr;
				MakeAnyLenString(&hacked_string, "Player tried to make use of a banker(shared coin move) but %s is non-existant or too far away (%u units).",
					banker ? banker->GetName() : "UNKNOWN NPC", distance);
				database.SetMQDetectionFlag(AccountName(), GetName(), hacked_string, zone->GetShortName());
				safe_delete_array(hacked_string);
				return;
			}
			if(mc->cointype2 == COINTYPE_PP)	// there's only platinum here
				to_bucket = (int32 *) &m_pp.platinum_shared;
			break;
		}
	}

	if(!from_bucket)
	{
		return;
	}

	// don't allow them to go into negatives (from our point of view)
	amount_to_take = *from_bucket < mc->amount ? *from_bucket : mc->amount;

	// if you move 11 gold into a bank platinum location, the packet
	// will say 11, but the client will have 1 left on their cursor, so we have
	// to figure out the conversion ourselves

	amount_to_add = amount_to_take * ((float)CoinTypeCoppers(mc->cointype1) / (float)CoinTypeCoppers(mc->cointype2));

	// the amount we're adding could be different than what was requested, so
	// we have to adjust the amount we take as well
	amount_to_take = amount_to_add * ((float)CoinTypeCoppers(mc->cointype2) / (float)CoinTypeCoppers(mc->cointype1));

	// now we should have a from_bucket, a to_bucket, an amount_to_take
	// and an amount_to_add

	// now we actually take it from the from bucket. if there's an error
	// with the destination slot, they lose their money
	*from_bucket -= amount_to_take;
	// why are intentionally inducing a crash here rather than letting the code attempt to stumble on?
	// assert(*from_bucket >= 0);

	if(to_bucket)
	{
		if(*to_bucket + amount_to_add > *to_bucket)	// overflow check
			*to_bucket += amount_to_add;

		//shared bank plat
		if (RuleB(Character, SharedBankPlat))
		{
			if (to_bucket == &m_pp.platinum_shared || from_bucket == &m_pp.platinum_shared)
			{
				if (from_bucket == &m_pp.platinum_shared)
					amount_to_add = 0 - amount_to_take;

				database.SetSharedPlatinum(AccountID(),amount_to_add);
			}
		}
		else{
			if (to_bucket == &m_pp.platinum_shared || from_bucket == &m_pp.platinum_shared){
				this->Message(13, "::: WARNING! ::: SHARED BANK IS DISABLED AND YOUR PLATINUM WILL BE DESTROYED IF YOU PUT IT HERE");
			}
		}
	}

	// if this is a trade move, inform the person being traded with
	if(mc->to_slot == 3 && trader && trader->IsClient())
	{

		// If one party accepted the trade then some coin was added, their state needs to be reset
		trade->state = Trading;
		Mob* with = trade->With();
		if (with)
			with->trade->state = Trading;

		Client* recipient = trader->CastToClient();
		recipient->Message(15, "%s adds some coins to the trade.", GetName());
		recipient->Message(15, "The total trade is: %i PP, %i GP, %i SP, %i CP",
			trade->pp, trade->gp,
			trade->sp, trade->cp
		);

		auto outapp = new EQApplicationPacket(OP_TradeCoins, sizeof(TradeCoin_Struct));
		TradeCoin_Struct* tcs = (TradeCoin_Struct*)outapp->pBuffer;
		tcs->trader = trader->GetID();
		tcs->slot = mc->cointype2;
		tcs->unknown5 = 0x4fD2;
		tcs->unknown7 = 0;
		tcs->amount = amount_to_add;
		recipient->QueuePacket(outapp);
		safe_delete(outapp);
	}

	SaveCurrency();
}

void Client::OPGMTraining(const EQApplicationPacket *app)
{

	EQApplicationPacket* outapp = app->Copy();
	GMTrainee_Struct* gmtrain = (GMTrainee_Struct*) outapp->pBuffer;

	Mob* pTrainer = entity_list.GetMob(gmtrain->npcid);

	if(!pTrainer || !pTrainer->IsNPC() || pTrainer->GetClass() < WARRIORGM || pTrainer->GetClass() > BERSERKERGM)
		return;

	//you can only use your own trainer, client enforces this, but why trust it
	int trains_class = pTrainer->GetClass() - (WARRIORGM - WARRIOR);
	if(GetClass() != trains_class)
		return;

	//you have to be somewhat close to a trainer to be properly using them
	if(DistanceSquared(m_Position,pTrainer->GetPosition()) > USE_NPC_RANGE2)
		return;

	// if this for-loop acts up again (crashes linux), try enabling the before and after #pragmas
//#pragma GCC push_options
//#pragma GCC optimize ("O0")
	for (int sk = EQEmu::skills::Skill1HBlunt; sk <= EQEmu::skills::HIGHEST_SKILL; ++sk) {
		if (sk == EQEmu::skills::SkillTinkering && GetRace() != GNOME) {
			gmtrain->skills[sk] = 0; //Non gnomes can't tinker!
		} else {
			gmtrain->skills[sk] = GetMaxSkillAfterSpecializationRules((EQEmu::skills::SkillType)sk, MaxSkill((EQEmu::skills::SkillType)sk, GetClass(), RuleI(Character, MaxLevel)));
			//this is the highest level that the trainer can train you to, this is enforced clientside so we can't just
			//Set it to 1 with CanHaveSkill or you wont be able to train past 1.
		}
	}

	if (ClientVersion() < EQEmu::versions::ClientVersion::RoF2 && GetClass() == BERSERKER) {
		gmtrain->skills[EQEmu::skills::Skill1HPiercing] = gmtrain->skills[EQEmu::skills::Skill2HPiercing];
		gmtrain->skills[EQEmu::skills::Skill2HPiercing] = 0;
	}
//#pragma GCC pop_options

	uchar ending[]={0x34,0x87,0x8a,0x3F,0x01
		,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9
		,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9
		,0x76,0x75,0x3f};
	memcpy(&outapp->pBuffer[outapp->size-40],ending,sizeof(ending));
	FastQueuePacket(&outapp);

	// welcome message
	if (pTrainer && pTrainer->IsNPC())
	{
		pTrainer->Say_StringID(zone->random.Int(1204, 1207), GetCleanName());
	}
}

void Client::OPGMEndTraining(const EQApplicationPacket *app)
{
	auto outapp = new EQApplicationPacket(OP_GMEndTrainingResponse, 0);
	GMTrainEnd_Struct *p = (GMTrainEnd_Struct *)app->pBuffer;

	FastQueuePacket(&outapp);

	Mob* pTrainer = entity_list.GetMob(p->npcid);
	if(!pTrainer || !pTrainer->IsNPC() || pTrainer->GetClass() < WARRIORGM || pTrainer->GetClass() > BERSERKERGM)
		return;

	//you can only use your own trainer, client enforces this, but why trust it
	int trains_class = pTrainer->GetClass() - (WARRIORGM - WARRIOR);
	if(GetClass() != trains_class)
		return;

	//you have to be somewhat close to a trainer to be properly using them
	if(DistanceSquared(m_Position, pTrainer->GetPosition()) > USE_NPC_RANGE2)
		return;

	// goodbye message
	if (pTrainer->IsNPC())
	{
		pTrainer->Say_StringID(zone->random.Int(1208, 1211), GetCleanName());
	}
}

void Client::OPGMTrainSkill(const EQApplicationPacket *app)
{
	if(!m_pp.points)
		return;

	int Cost = 0;

	GMSkillChange_Struct* gmskill = (GMSkillChange_Struct*) app->pBuffer;

	Mob* pTrainer = entity_list.GetMob(gmskill->npcid);
	if(!pTrainer || !pTrainer->IsNPC() || pTrainer->GetClass() < WARRIORGM || pTrainer->GetClass() > BERSERKERGM)
		return;

	//you can only use your own trainer, client enforces this, but why trust it
	int trains_class = pTrainer->GetClass() - (WARRIORGM - WARRIOR);
	if(GetClass() != trains_class)
		return;

	//you have to be somewhat close to a trainer to be properly using them
	if(DistanceSquared(m_Position, pTrainer->GetPosition()) > USE_NPC_RANGE2)
		return;

	if (gmskill->skillbank == 0x01)
	{
		// languages go here
		if (gmskill->skill_id > 25)
		{
			std::cout << "Wrong Training Skill (languages)" << std::endl;
			DumpPacket(app);
			return;
		}
		int AdjustedSkillLevel = GetLanguageSkill(gmskill->skill_id) - 10;
		if(AdjustedSkillLevel > 0)
			Cost = AdjustedSkillLevel * AdjustedSkillLevel * AdjustedSkillLevel / 100;

		IncreaseLanguageSkill(gmskill->skill_id);
	}
	else if (gmskill->skillbank == 0x00)
	{
		// normal skills go here
		if (gmskill->skill_id > EQEmu::skills::HIGHEST_SKILL)
		{
			std::cout << "Wrong Training Skill (abilities)" << std::endl;
			DumpPacket(app);
			return;
		}

		EQEmu::skills::SkillType skill = (EQEmu::skills::SkillType)gmskill->skill_id;

		if(!CanHaveSkill(skill)) {
			Log(Logs::Detail, Logs::Skills, "Tried to train skill %d, which is not allowed.", skill);
			return;
		}

		if(MaxSkill(skill) == 0) {
			Log(Logs::Detail, Logs::Skills, "Tried to train skill %d, but training is not allowed at this level.", skill);
			return;
		}

		uint16 skilllevel = GetRawSkill(skill);

		if(skilllevel == 0) {
			//this is a new skill..
			uint16 t_level = SkillTrainLevel(skill, GetClass());
			if (t_level == 0)
			{
				return;
			}

			SetSkill(skill, t_level);
		} else {
			switch(skill) {
			case EQEmu::skills::SkillBrewing:
			case EQEmu::skills::SkillMakePoison:
			case EQEmu::skills::SkillTinkering:
			case EQEmu::skills::SkillResearch:
			case EQEmu::skills::SkillAlchemy:
			case EQEmu::skills::SkillBaking:
			case EQEmu::skills::SkillTailoring:
			case EQEmu::skills::SkillBlacksmithing:
			case EQEmu::skills::SkillFletching:
			case EQEmu::skills::SkillJewelryMaking:
			case EQEmu::skills::SkillPottery:
				if(skilllevel >= RuleI(Skills, MaxTrainTradeskills)) {
					Message_StringID(13, MORE_SKILLED_THAN_I, pTrainer->GetCleanName());
					return;
				}
				break;
			case EQEmu::skills::SkillSpecializeAbjure:
			case EQEmu::skills::SkillSpecializeAlteration:
			case EQEmu::skills::SkillSpecializeConjuration:
			case EQEmu::skills::SkillSpecializeDivination:
			case EQEmu::skills::SkillSpecializeEvocation:
				if(skilllevel >= RuleI(Skills, MaxTrainSpecializations)) {
					Message_StringID(13, MORE_SKILLED_THAN_I, pTrainer->GetCleanName());
					return;
				}
			default:
				break;
			}

			int MaxSkillValue = MaxSkill(skill);
			if (skilllevel >= MaxSkillValue)
			{
				// Don't allow training over max skill level
				Message_StringID(13, MORE_SKILLED_THAN_I, pTrainer->GetCleanName());
				return;
			}

			if (gmskill->skill_id >= EQEmu::skills::SkillSpecializeAbjure && gmskill->skill_id <= EQEmu::skills::SkillSpecializeEvocation)
			{
				int MaxSpecSkill = GetMaxSkillAfterSpecializationRules(skill, MaxSkillValue);
				if (skilllevel >= MaxSpecSkill)
				{
					// Restrict specialization training to follow the rules
					Message_StringID(13, MORE_SKILLED_THAN_I, pTrainer->GetCleanName());
					return;
				}
			}

			// Client train a valid skill
			//
			int AdjustedSkillLevel = skilllevel - 10;

			if(AdjustedSkillLevel > 0)
				Cost = AdjustedSkillLevel * AdjustedSkillLevel * AdjustedSkillLevel / 100;

			SetSkill(skill, skilllevel + 1);


		}
	}

	if (ClientVersion() >= EQEmu::versions::ClientVersion::SoF) {
		// The following packet decreases the skill points left in the Training Window and
		// produces the 'You have increased your skill / learned the basics of' message.
		//
		auto outapp = new EQApplicationPacket(OP_GMTrainSkillConfirm, sizeof(GMTrainSkillConfirm_Struct));

		GMTrainSkillConfirm_Struct *gmtsc = (GMTrainSkillConfirm_Struct *)outapp->pBuffer;
		gmtsc->SkillID = gmskill->skill_id;

		if(gmskill->skillbank == 1) {
			gmtsc->NewSkill = (GetLanguageSkill(gmtsc->SkillID) == 1);
			gmtsc->SkillID += 100;
		}
		else
			gmtsc->NewSkill = (GetRawSkill((EQEmu::skills::SkillType)gmtsc->SkillID) == 1);

		gmtsc->Cost = Cost;

		strcpy(gmtsc->TrainerName, pTrainer->GetCleanName());
		QueuePacket(outapp);
		safe_delete(outapp);
	}

	if(Cost)
		TakeMoneyFromPP(Cost);

	m_pp.points--;
}

// this is used for /summon and /corpse
void Client::OPGMSummon(const EQApplicationPacket *app)
{
	GMSummon_Struct* gms = (GMSummon_Struct*) app->pBuffer;
	Mob* st = entity_list.GetMob(gms->charname);

	if(st && st->IsCorpse())
	{
		st->CastToCorpse()->Summon(this, false, true);
	}
	else
	{
		if(admin < 80)
		{
			return;
		}
		if(st)
		{
			Message(0, "Local: Summoning %s to %f, %f, %f", gms->charname, gms->x, gms->y, gms->z);
			if (st->IsClient() && (st->CastToClient()->GetAnon() != 1 || this->Admin() >= st->CastToClient()->Admin()))
				st->CastToClient()->MovePC(zone->GetZoneID(), zone->GetInstanceID(), (float)gms->x, (float)gms->y, (float)gms->z, this->GetHeading(), true);
			else
				st->GMMove(this->GetX(), this->GetY(), this->GetZ(),this->GetHeading());
		}
		else
		{
			uint8 tmp = gms->charname[strlen(gms->charname)-1];
			if (!worldserver.Connected())
			{
				Message(0, "Error: World server disconnected");
			}
			else if (tmp < '0' || tmp > '9') // dont send to world if it's not a player's name
			{
				auto pack = new ServerPacket(ServerOP_ZonePlayer, sizeof(ServerZonePlayer_Struct));
				ServerZonePlayer_Struct* szp = (ServerZonePlayer_Struct*) pack->pBuffer;
				strcpy(szp->adminname, this->GetName());
				szp->adminrank = this->Admin();
				strcpy(szp->name, gms->charname);
				strcpy(szp->zone, zone->GetShortName());
				szp->x_pos = (float)gms->x;
				szp->y_pos = (float)gms->y;
				szp->z_pos = (float)gms->z;
				szp->ignorerestrictions = 2;
				worldserver.SendPacket(pack);
				safe_delete(pack);
			}
			else {
				//all options have been exhausted
				//summon our target...
				if(GetTarget() && GetTarget()->IsCorpse()){
					GetTarget()->CastToCorpse()->Summon(this, false, true);
				}
			}
		}
	}
}

void Client::DoHPRegen() {
	SetHP(GetHP() + CalcHPRegen());
	SendHPUpdate();
}

void Client::DoManaRegen() {
	if (GetMana() >= max_mana && spellbonuses.ManaRegen >= 0)
		return;

	if (GetMana() < max_mana && (IsSitting() || CanMedOnHorse()) && HasSkill(EQEmu::skills::SkillMeditate))
		CheckIncreaseSkill(EQEmu::skills::SkillMeditate, nullptr, -5);

	SetMana(GetMana() + CalcManaRegen());
	CheckManaEndUpdate();
}

void Client::DoStaminaHungerUpdate()
{
	auto outapp = new EQApplicationPacket(OP_Stamina, sizeof(Stamina_Struct));
	Stamina_Struct *sta = (Stamina_Struct *)outapp->pBuffer;

	Log(Logs::General, Logs::Food, "Client::DoStaminaHungerUpdate() hunger_level: %i thirst_level: %i before loss",
	    m_pp.hunger_level, m_pp.thirst_level);

	if (zone->GetZoneID() != 151 && !GetGM()) {
		int loss = RuleI(Character, FoodLossPerUpdate);
		if (GetHorseId() != 0)
			loss *= 3;

		m_pp.hunger_level = EQEmu::Clamp(m_pp.hunger_level - loss, 0, 6000);
		m_pp.thirst_level = EQEmu::Clamp(m_pp.thirst_level - loss, 0, 6000);
		if (spellbonuses.hunger) {
			m_pp.hunger_level = EQEmu::ClampLower(m_pp.hunger_level, 3500);
			m_pp.thirst_level = EQEmu::ClampLower(m_pp.thirst_level, 3500);
		}
		sta->food = m_pp.hunger_level;
		sta->water = m_pp.thirst_level;
	} else {
		// No auto food/drink consumption in the Bazaar
		sta->food = 6000;
		sta->water = 6000;
	}

	Log(Logs::General, Logs::Food,
	    "Client::DoStaminaHungerUpdate() Current hunger_level: %i = (%i minutes left) thirst_level: %i = (%i "
	    "minutes left) - after loss",
	    m_pp.hunger_level, m_pp.hunger_level, m_pp.thirst_level, m_pp.thirst_level);

	FastQueuePacket(&outapp);
}

void Client::DoEnduranceRegen()
{
	// endurance has some negative mods that could result in a negative regen when starved
	int regen = CalcEnduranceRegen();

	if (regen < 0 || (regen > 0 && GetEndurance() < GetMaxEndurance()))
		SetEndurance(GetEndurance() + regen);
}

void Client::DoEnduranceUpkeep() {

	if (!HasEndurUpkeep())
		return;

	int upkeep_sum = 0;
	int cost_redux = spellbonuses.EnduranceReduction + itembonuses.EnduranceReduction + aabonuses.EnduranceReduction;

	bool has_effect = false;
	uint32 buffs_i;
	uint32 buff_count = GetMaxTotalSlots();
	for (buffs_i = 0; buffs_i < buff_count; buffs_i++) {
		if (buffs[buffs_i].spellid != SPELL_UNKNOWN) {
			int upkeep = spells[buffs[buffs_i].spellid].EndurUpkeep;
			if(upkeep > 0) {
				has_effect = true;
				if(cost_redux > 0) {
					if(upkeep <= cost_redux)
						continue;	//reduced to 0
					upkeep -= cost_redux;
				}
				if((upkeep+upkeep_sum) > GetEndurance()) {
					//they do not have enough to keep this one going.
					BuffFadeBySlot(buffs_i);
				} else {
					upkeep_sum += upkeep;
				}
			}
		}
	}

	if(upkeep_sum != 0){
		SetEndurance(GetEndurance() - upkeep_sum);
		TryTriggerOnValueAmount(false, false, true);
	}

	if (!has_effect)
		SetEndurUpkeep(false);
}

void Client::CalcRestState() {

	// This method calculates rest state HP and mana regeneration.
	// The client must have been out of combat for RuleI(Character, RestRegenTimeToActivate) seconds,
	// must be sitting down, and must not have any detrimental spells affecting them.
	//
	if(!RuleB(Character, RestRegenEnabled))
		return;

	ooc_regen = false;

	if(AggroCount || !(IsSitting() || CanMedOnHorse()))
		return;

	if(!rest_timer.Check(false))
		return;

	uint32 buff_count = GetMaxTotalSlots();
	for (unsigned int j = 0; j < buff_count; j++) {
		if(buffs[j].spellid != SPELL_UNKNOWN) {
			if(IsDetrimentalSpell(buffs[j].spellid) && (buffs[j].ticsremaining > 0))
				if(!DetrimentalSpellAllowsRest(buffs[j].spellid))
					return;
		}
	}

	ooc_regen = true;

}

void Client::DoTracking()
{
	if(TrackingID == 0)
		return;

	Mob *m = entity_list.GetMob(TrackingID);

	if(!m || m->IsCorpse())
	{
		Message_StringID(MT_Skills, TRACK_LOST_TARGET);

		TrackingID = 0;

		return;
	}

	float RelativeHeading = GetHeading() - CalculateHeadingToTarget(m->GetX(), m->GetY());

	if(RelativeHeading < 0)
		RelativeHeading += 256;

	if((RelativeHeading <= 16) || (RelativeHeading >= 240))
	{
		Message_StringID(MT_Skills, TRACK_STRAIGHT_AHEAD, m->GetCleanName());
	}
	else if((RelativeHeading > 16) && (RelativeHeading <= 48))
	{
		Message_StringID(MT_Skills, TRACK_AHEAD_AND_TO, m->GetCleanName(), "right");
	}
	else if((RelativeHeading > 48) && (RelativeHeading <= 80))
	{
		Message_StringID(MT_Skills, TRACK_TO_THE, m->GetCleanName(), "right");
	}
	else if((RelativeHeading > 80) && (RelativeHeading <= 112))
	{
		Message_StringID(MT_Skills, TRACK_BEHIND_AND_TO, m->GetCleanName(), "right");
	}
	else if((RelativeHeading > 112) && (RelativeHeading <= 144))
	{
		Message_StringID(MT_Skills, TRACK_BEHIND_YOU, m->GetCleanName());
	}
	else if((RelativeHeading > 144) && (RelativeHeading <= 176))
	{
		Message_StringID(MT_Skills, TRACK_BEHIND_AND_TO, m->GetCleanName(), "left");
	}
	else if((RelativeHeading > 176) && (RelativeHeading <= 208))
	{
		Message_StringID(MT_Skills, TRACK_TO_THE, m->GetCleanName(), "left");
	}
	else if((RelativeHeading > 208) && (RelativeHeading < 240))
	{
		Message_StringID(MT_Skills, TRACK_AHEAD_AND_TO, m->GetCleanName(), "left");
	}
}

void Client::HandleRespawnFromHover(uint32 Option)
{
	RespawnFromHoverTimer.Disable();

	RespawnOption* chosen = nullptr;
	bool is_rez = false;

	//Find the selected option
	if (Option == 0)
	{
		chosen = &respawn_options.front();
	}
	else if (Option == (respawn_options.size() - 1))
	{
		chosen = &respawn_options.back();
		is_rez = true; //Rez must always be the last option
	}
	else
	{
		std::list<RespawnOption>::iterator itr;
		uint32 pos = 0;
		for (itr = respawn_options.begin(); itr != respawn_options.end(); ++itr)
		{
			if (pos++ == Option)
			{
				chosen = &(*itr);
				break;
			}
		}
	}

	//If they somehow chose an option they don't have, just send them to bind
	RespawnOption* default_to_bind = nullptr;
	if (!chosen)
	{
		/* put error logging here */
		BindStruct* b = &m_pp.binds[0];
		default_to_bind = new RespawnOption;
		default_to_bind->name = "Bind Location";
		default_to_bind->zone_id = b->zoneId;
		default_to_bind->instance_id = b->instance_id;
		default_to_bind->x = b->x;
		default_to_bind->y = b->y;
		default_to_bind->z = b->z;
		default_to_bind->heading = b->heading;
		chosen = default_to_bind;
		is_rez = false;
	}

	if (chosen->zone_id == zone->GetZoneID() && chosen->instance_id == zone->GetInstanceID()) //If they should respawn in the current zone...
	{
		if (is_rez)
		{
			if (PendingRezzXP < 0 || PendingRezzSpellID == 0)
			{
				Log(Logs::Detail, Logs::Spells, "Unexpected Rezz from hover request.");
				return;
			}
			SetHP(GetMaxHP() / 5);

			Corpse* corpse = entity_list.GetCorpseByName(PendingRezzCorpseName.c_str());

			if (corpse)
			{
				m_Position.x = corpse->GetX();
				m_Position.y = corpse->GetY();
				m_Position.z = corpse->GetZ();
			}

			auto outapp =
			    new EQApplicationPacket(OP_ZonePlayerToBind, sizeof(ZonePlayerToBind_Struct) + 10);
			ZonePlayerToBind_Struct* gmg = (ZonePlayerToBind_Struct*) outapp->pBuffer;

			gmg->bind_zone_id = zone->GetZoneID();
			gmg->bind_instance_id = zone->GetInstanceID();
			gmg->x = GetX();
			gmg->y = GetY();
			gmg->z = GetZ();
			gmg->heading = GetHeading();
			strcpy(gmg->zone_name, "Resurrect");

			FastQueuePacket(&outapp);

			ClearHover();
			SendHPUpdate();
			OPRezzAnswer(1, PendingRezzSpellID, zone->GetZoneID(), zone->GetInstanceID(), GetX(), GetY(), GetZ());

			if (corpse && corpse->IsCorpse())
			{
				Log(Logs::Detail, Logs::Spells, "Hover Rez in zone %s for corpse %s",
						zone->GetShortName(), PendingRezzCorpseName.c_str());

				Log(Logs::Detail, Logs::Spells, "Found corpse. Marking corpse as rezzed.");

				corpse->IsRezzed(true);
				corpse->CompleteResurrection();
			}
		}
		else //Not rez
		{
			PendingRezzSpellID = 0;

			auto outapp = new EQApplicationPacket(OP_ZonePlayerToBind, sizeof(ZonePlayerToBind_Struct) +
										       chosen->name.length() + 1);
			ZonePlayerToBind_Struct* gmg = (ZonePlayerToBind_Struct*) outapp->pBuffer;

			gmg->bind_zone_id = zone->GetZoneID();
			gmg->bind_instance_id = chosen->instance_id;
			gmg->x = chosen->x;
			gmg->y = chosen->y;
			gmg->z = chosen->z;
			gmg->heading = chosen->heading;
			strcpy(gmg->zone_name, chosen->name.c_str());

			FastQueuePacket(&outapp);

			CalcBonuses();
			SetHP(GetMaxHP());
			SetMana(GetMaxMana());
			SetEndurance(GetMaxEndurance());

			m_Position.x = chosen->x;
			m_Position.y = chosen->y;
			m_Position.z = chosen->z;
			m_Position.w = chosen->heading;

			ClearHover();
			entity_list.RefreshClientXTargets(this);
			SendHPUpdate();
		}

		//After they've respawned into the same zone, trigger EVENT_RESPAWN
		parse->EventPlayer(EVENT_RESPAWN, this, static_cast<std::string>(itoa(Option)), is_rez ? 1 : 0);

		//Pop Rez option from the respawn options list;
		//easiest way to make sure it stays at the end and
		//doesn't disrupt adding/removing scripted options
		respawn_options.pop_back();
	}
	else
	{
		//Heading to a different zone
		if(isgrouped)
		{
			Group *g = GetGroup();
			if(g)
				g->MemberZoned(this);
		}

		Raid* r = entity_list.GetRaidByClient(this);
		if(r)
			r->MemberZoned(this);

		m_pp.zone_id = chosen->zone_id;
		m_pp.zoneInstance = chosen->instance_id;
		database.MoveCharacterToZone(CharacterID(), database.GetZoneName(chosen->zone_id));

		Save();

		MovePC(chosen->zone_id, chosen->instance_id, chosen->x, chosen->y, chosen->z, chosen->heading, 1);
	}

	safe_delete(default_to_bind);
}

void Client::ClearHover()
{
	// Our Entity ID is currently zero, set in Client::Death
	SetID(entity_list.GetFreeID());

	auto outapp = new EQApplicationPacket(OP_ZoneEntry, sizeof(ServerZoneEntry_Struct));
	ServerZoneEntry_Struct* sze = (ServerZoneEntry_Struct*)outapp->pBuffer;

	FillSpawnStruct(&sze->player,CastToMob());

	sze->player.spawn.NPC = 0;
	sze->player.spawn.z += 6;	//arbitrary lift, seems to help spawning under zone.

	entity_list.QueueClients(this, outapp, false);
	safe_delete(outapp);

	if (IsClient() && CastToClient()->ClientVersionBit() & EQEmu::versions::bit_UFAndLater)
	{
		EQApplicationPacket *outapp = MakeBuffsPacket(false);
		CastToClient()->FastQueuePacket(&outapp);
	}

	dead = false;
}

void Client::HandleLFGuildResponse(ServerPacket *pack)
{
	pack->SetReadPosition(8);

	char Tmp[257];

	pack->ReadString(Tmp);

	pack->ReadSkipBytes(4);
	uint32 SubType, NumberOfMatches;

	SubType = pack->ReadUInt32();

	switch(SubType)
	{
		case QSG_LFGuild_PlayerMatches:
		{
			NumberOfMatches = pack->ReadUInt32();
			uint32 StartOfMatches = pack->GetReadPosition();
			uint32 i = NumberOfMatches;
			uint32 PacketSize = 12;
			while(i > 0)
			{
				pack->ReadString(Tmp);
				PacketSize += strlen(Tmp) + 1;
				pack->ReadString(Tmp);
				PacketSize += strlen(Tmp) + 1;
				PacketSize += 16;
				pack->ReadSkipBytes(16);
				--i;
			}

			auto outapp = new EQApplicationPacket(OP_LFGuild, PacketSize);
			outapp->WriteUInt32(3);
			outapp->WriteUInt32(0xeb63);	// Don't know the significance of this value.
			outapp->WriteUInt32(NumberOfMatches);
			pack->SetReadPosition(StartOfMatches);

			while(NumberOfMatches > 0)
			{
				pack->ReadString(Tmp);
				outapp->WriteString(Tmp);
				pack->ReadString(Tmp);
				uint32 Level = pack->ReadUInt32();
				uint32 Class = pack->ReadUInt32();
				uint32 AACount = pack->ReadUInt32();
				uint32 TimeZone = pack->ReadUInt32();
				outapp->WriteUInt32(Level);
				outapp->WriteUInt32(Class);
				outapp->WriteUInt32(AACount);
				outapp->WriteUInt32(TimeZone);
				outapp->WriteString(Tmp);
				--NumberOfMatches;
			}

			FastQueuePacket(&outapp);
			break;
		}
		case QSG_LFGuild_RequestPlayerInfo:
		{
			auto outapp = new EQApplicationPacket(OP_LFGuild, sizeof(LFGuild_PlayerToggle_Struct));
			LFGuild_PlayerToggle_Struct *pts = (LFGuild_PlayerToggle_Struct *)outapp->pBuffer;

			pts->Command = 0;
			pack->ReadString(pts->Comment);
			pts->TimeZone = pack->ReadUInt32();
			pts->TimePosted = pack->ReadUInt32();
			pts->Toggle = pack->ReadUInt32();

			FastQueuePacket(&outapp);

			break;
		}
		case QSG_LFGuild_GuildMatches:
		{
			NumberOfMatches = pack->ReadUInt32();
			uint32 StartOfMatches = pack->GetReadPosition();
			uint32 i = NumberOfMatches;
			uint32 PacketSize = 12;
			while(i > 0)
			{
				pack->ReadString(Tmp);
				PacketSize += strlen(Tmp) + 1;
				pack->ReadSkipBytes(4);
				pack->ReadString(Tmp);
				PacketSize += strlen(Tmp) + 1;
				PacketSize += 4;
				--i;
			}

			auto outapp = new EQApplicationPacket(OP_LFGuild, PacketSize);
			outapp->WriteUInt32(4);
			outapp->WriteUInt32(0xeb63);
			outapp->WriteUInt32(NumberOfMatches);
			pack->SetReadPosition(StartOfMatches);

			while(NumberOfMatches > 0)
			{
				pack->ReadString(Tmp);
				uint32 TimeZone = pack->ReadUInt32();
				outapp->WriteString(Tmp);
				outapp->WriteUInt32(TimeZone);
				pack->ReadString(Tmp);
				outapp->WriteString(Tmp);
				--NumberOfMatches;
			}
			FastQueuePacket(&outapp);

			break;
		}
		case QSG_LFGuild_RequestGuildInfo:
		{

			char Comments[257];
			uint32 FromLevel, ToLevel, Classes, AACount, TimeZone, TimePosted;

			pack->ReadString(Comments);
			FromLevel = pack->ReadUInt32();
			ToLevel = pack->ReadUInt32();
			Classes = pack->ReadUInt32();
			AACount = pack->ReadUInt32();
			TimeZone = pack->ReadUInt32();
			TimePosted = pack->ReadUInt32();

			auto outapp = new EQApplicationPacket(OP_LFGuild, sizeof(LFGuild_GuildToggle_Struct));

			LFGuild_GuildToggle_Struct *gts = (LFGuild_GuildToggle_Struct *)outapp->pBuffer;
			gts->Command = 1;
			strcpy(gts->Comment, Comments);
			gts->FromLevel = FromLevel;
			gts->ToLevel = ToLevel;
			gts->Classes = Classes;
			gts->AACount = AACount;
			gts->TimeZone = TimeZone;
			gts->Toggle = 1;
			gts->TimePosted = TimePosted;
			gts->Name[0] = 0;

			FastQueuePacket(&outapp);

			break;
		}

		default:
			break;
	}

}

void Client::SendLFGuildStatus()
{
	auto pack = new ServerPacket(ServerOP_QueryServGeneric, strlen(GetName()) + 17);

	pack->WriteUInt32(zone->GetZoneID());
	pack->WriteUInt32(zone->GetInstanceID());
	pack->WriteString(GetName());
	pack->WriteUInt32(QSG_LFGuild);
	pack->WriteUInt32(QSG_LFGuild_RequestPlayerInfo);

	worldserver.SendPacket(pack);
	safe_delete(pack);

}

void Client::SendGuildLFGuildStatus()
{
	auto pack = new ServerPacket(ServerOP_QueryServGeneric,
				     strlen(GetName()) + +strlen(guild_mgr.GetGuildName(GuildID())) + 18);

	pack->WriteUInt32(zone->GetZoneID());
	pack->WriteUInt32(zone->GetInstanceID());
	pack->WriteString(GetName());
	pack->WriteUInt32(QSG_LFGuild);
	pack->WriteUInt32(QSG_LFGuild_RequestGuildInfo);
	pack->WriteString(guild_mgr.GetGuildName(GuildID()));

	worldserver.SendPacket(pack);
	safe_delete(pack);
}
