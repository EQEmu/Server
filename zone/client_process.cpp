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
#include "../common/debug.h"
#include <iostream>
#include <iomanip>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <zlib.h>
#include <assert.h>

#ifdef _WINDOWS
	#include <windows.h>
	#include <winsock.h>
	#define snprintf	_snprintf
	#define strncasecmp	_strnicmp
	#define strcasecmp	_stricmp
#else
	#include <pthread.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <unistd.h>
#endif

#include "masterentity.h"
#include "zonedb.h"
#include "../common/packet_functions.h"
#include "../common/packet_dump.h"
#include "worldserver.h"
#include "../common/packet_dump_file.h"
#include "../common/StringUtil.h"
#include "../common/spdat.h"
#include "petitions.h"
#include "NpcAI.h"
#include "../common/skills.h"
#include "forage.h"
#include "zone.h"
#include "event_codes.h"
#include "../common/faction.h"
#include "../common/crc32.h"
#include "../common/rulesys.h"
#include "StringIDs.h"
#include "map.h"
#include "guild_mgr.h"
#include <string>
#include "QuestParserCollection.h"

extern Zone* zone;
extern volatile bool ZoneLoaded;
extern WorldServer worldserver;
extern PetitionList petition_list;
extern EntityList entity_list;

bool Client::Process() {
	bool ret = true;

	if(Connected() || IsLD())
	{
		// try to send all packets that weren't sent before
		if(!IsLD() && zoneinpacket_timer.Check()){
			SendAllPackets();
		}

		if(adventure_request_timer)
		{
			if(adventure_request_timer->Check())
			{
				safe_delete(adventure_request_timer);
			}
		}

		if(adventure_create_timer)
		{
			if(adventure_create_timer->Check())
			{
				safe_delete(adventure_create_timer);
			}
		}

		if(adventure_leave_timer)
		{
			if(adventure_leave_timer->Check())
			{
				safe_delete(adventure_leave_timer);
			}
		}

		if(adventure_door_timer)
		{
			if(adventure_door_timer->Check())
			{
				safe_delete(adventure_door_timer);
			}
		}

		if(adventure_stats_timer)
		{
			if(adventure_stats_timer->Check())
			{
				safe_delete(adventure_stats_timer);
			}
		}

		if(adventure_leaderboard_timer)
		{
			if(adventure_leaderboard_timer->Check())
			{
				safe_delete(adventure_leaderboard_timer);
			}
		}

		if(dead)
		{
			SetHP(-100);
			if(RespawnFromHoverTimer.Check())
				HandleRespawnFromHover(0);
		}

		if(IsTracking() && (GetClientVersion() >= EQClientSoD) && TrackingTimer.Check())
			DoTracking();

		if(hpupdate_timer.Check())
			SendHPUpdate();

		if(mana_timer.Check())
			SendManaUpdatePacket();
		if(dead && dead_timer.Check()) {
			database.MoveCharacterToZone(GetName(),database.GetZoneName(m_pp.binds[0].zoneId));
			m_pp.zone_id = m_pp.binds[0].zoneId;
			m_pp.zoneInstance = 0;
			m_pp.x = m_pp.binds[0].x;
			m_pp.y = m_pp.binds[0].y;
			m_pp.z = m_pp.binds[0].z;
			Save();

			Group *mygroup = GetGroup();
			if (mygroup)
			{
				entity_list.MessageGroup(this,true,15,"%s died.", GetName());
				mygroup->MemberZoned(this);
			}
			Raid *myraid = entity_list.GetRaidByClient(this);
			if (myraid)
			{
				myraid->MemberZoned(this);
			}
			return(false);
		}

		if(charm_update_timer.Check())
		{
			CalcItemScale();
		}

		if(TaskPeriodic_Timer.Check() && taskstate)
			taskstate->TaskPeriodicChecks(this);

		if(linkdead_timer.Check()){
			Save();
			if (GetMerc())
			{
				GetMerc()->Save();
				GetMerc()->Depop();
			}
			LeaveGroup();
			Raid *myraid = entity_list.GetRaidByClient(this);
			if (myraid)
			{
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

		if (IsStunned() && stunned_timer.Check()) {
			this->stunned = false;
			this->stunned_timer.Disable();
		}

		if(!m_CheatDetectMoved)
		{
			m_TimeSinceLastPositionCheck = Timer::GetCurrentTime();
		}

		if (bardsong_timer.Check() && bardsong != 0) {
			//NOTE: this is kinda a heavy-handed check to make sure the mob still exists before
			//doing the next pulse on them...
			Mob *song_target;
			if(bardsong_target_id == GetID()) {
				song_target = this;
			} else {
				song_target = entity_list.GetMob(bardsong_target_id);
			}

			if (song_target == nullptr) {
				InterruptSpell(SONG_ENDS_ABRUPTLY, 0x121, bardsong);
			} else {
				if(!ApplyNextBardPulse(bardsong, song_target, bardsong_slot))
					InterruptSpell(SONG_ENDS_ABRUPTLY, 0x121, bardsong);
//				SpellFinished(bardsong, bardsong_target, bardsong_slot, spells[bardsong].mana);
			}
		}

		if(GetMerc())
		{
				UpdateMercTimer();
		}

		if(GetMercInfo().MercTemplateID != 0 && GetMercInfo().IsSuspended)
		{
			if(p_timers.Expired(&database, pTimerMercSuspend, false)) {
					CheckMercSuspendTimer();
				}
		}

		if(IsAIControlled())
			AI_Process();

		if (bindwound_timer.Check() && bindwound_target != 0) {
			BindWound(bindwound_target, false);
		}

		if(KarmaUpdateTimer)
		{
			if(KarmaUpdateTimer->Check(false))
			{
				KarmaUpdateTimer->Start(RuleI(Chat, KarmaUpdateIntervalMS));
				database.UpdateKarma(AccountID(), ++TotalKarma);
			}
		}

		if(qGlobals)
		{
			if(qglobal_purge_timer.Check())
			{
				qGlobals->PurgeExpiredGlobals();
			}
		}

		bool may_use_attacks = false;
		/*
			Things which prevent us from attacking:
				- being under AI control, the AI does attacks
				- being dead
				- casting a spell (not sure what the rest is doing, prolly bard)
				- not having a target
				- being stunned or mezzed
				- having used a ranged weapon recently
		*/
		if(auto_attack) {
			if(!IsAIControlled() && !dead
				&& !(spellend_timer.Enabled() && (spells[casting_spell_id].classes[7] < 1 && spells[casting_spell_id].classes[7] > 65))
				&& !IsStunned() && !IsFeared() && !IsMezzed() && GetAppearance() != eaDead && !IsMeleeDisabled()
				)
				may_use_attacks = true;

			if(may_use_attacks && ranged_timer.Enabled()) {
				//if the range timer is enabled, we need to consider it
				if(!ranged_timer.Check(false)) {
					//the ranged timer has not elapsed, cannot attack.
					may_use_attacks = false;
				}
			}
		}

		if(AutoFireEnabled()){
			ItemInst *ranged = GetInv().GetItem(SLOT_RANGE);
			if(ranged)
			{
				if(ranged->GetItem() && ranged->GetItem()->ItemType == ItemTypeBow){
					if(ranged_timer.Check(false)){
						if(GetTarget() && (GetTarget()->IsNPC() || GetTarget()->IsClient())){
							if(!GetTarget()->BehindMob(this, GetTarget()->GetX(), GetTarget()->GetY())){
								if(CheckLosFN(GetTarget())){
									//client has built in los check, but auto fire does not.. done last.
									RangedAttack(GetTarget());
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
				else if(ranged->GetItem() && (ranged->GetItem()->ItemType == ItemTypeLargeThrowing || ranged->GetItem()->ItemType == ItemTypeSmallThrowing)){
					if(ranged_timer.Check(false)){
						if(GetTarget() && (GetTarget()->IsNPC() || GetTarget()->IsClient())){
							if(!GetTarget()->BehindMob(this, GetTarget()->GetX(), GetTarget()->GetY())){
								if(CheckLosFN(GetTarget())){
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
			if(aa_los_them_mob)
			{
				if(auto_attack_target != aa_los_them_mob ||
					aa_los_me.x != GetX() ||
					aa_los_me.y != GetY() ||
					aa_los_me.z != GetZ() ||
					aa_los_them.x != aa_los_them_mob->GetX() ||
					aa_los_them.y != aa_los_them_mob->GetY() ||
					aa_los_them.z != aa_los_them_mob->GetZ())
				{
					aa_los_them_mob = auto_attack_target;
					aa_los_me.x = GetX();
					aa_los_me.y = GetY();
					aa_los_me.z = GetZ();
					aa_los_them.x = aa_los_them_mob->GetX();
					aa_los_them.y = aa_los_them_mob->GetY();
					aa_los_them.z = aa_los_them_mob->GetZ();
					if(CheckLosFN(auto_attack_target))
						los_status = true;
					else
						los_status = false;
				}
			}
			else
			{
				aa_los_them_mob = auto_attack_target;
				aa_los_me.x = GetX();
				aa_los_me.y = GetY();
				aa_los_me.z = GetZ();
				aa_los_them.x = aa_los_them_mob->GetX();
				aa_los_them.y = aa_los_them_mob->GetY();
				aa_los_them.z = aa_los_them_mob->GetZ();
				if(CheckLosFN(auto_attack_target))
					los_status = true;
				else
					los_status = false;
			}

			if (!CombatRange(auto_attack_target))
			{
				//duplicate message not wanting to see it.
				//Message_StringID(MT_TooFarAway,TARGET_TOO_FAR);
			}
			else if (auto_attack_target == this)
			{
				Message_StringID(MT_TooFarAway,TRY_ATTACKING_SOMEONE);
			}
			else if (!los_status)
			{
				//you can't see your target
			}
			else if (auto_attack_target->GetHP() > -10) // -10 so we can watch people bleed in PvP
			{
				if(CheckAAEffect(aaEffectRampage))
				{
					entity_list.AEAttack(this, 30);
				} else {
					Attack(auto_attack_target, 13); // Kaiyodo - added attacking hand to arguments
				}
				ItemInst *wpn = GetInv().GetItem(SLOT_PRIMARY);
				TryWeaponProc(wpn, auto_attack_target, 13);

				bool tripleAttackSuccess = false;
				if( auto_attack_target && CanThisClassDoubleAttack() ) {

					CheckIncreaseSkill(SkillDoubleAttack, auto_attack_target, -10);
					if(CheckDoubleAttack()) {
						//should we allow rampage on double attack?
						if(CheckAAEffect(aaEffectRampage)) {
							entity_list.AEAttack(this, 30);
						} else {
							Attack(auto_attack_target, 13, false);
						}
					}

					//triple attack: rangers, monks, warriors, berserkers over level 60
					if((((GetClass() == MONK || GetClass() == WARRIOR || GetClass() == RANGER || GetClass() == BERSERKER)
						&& GetLevel() >= 60) || GetSpecialAbility(SPECATK_TRIPLE))
						&& CheckDoubleAttack(true))
					{
						tripleAttackSuccess = true;
						Attack(auto_attack_target, 13, false);
					}

					//quad attack, does this belong here??
					if(GetSpecialAbility(SPECATK_QUAD) && CheckDoubleAttack(true))
					{
						Attack(auto_attack_target, 13, false);
					}
				}

				//Live AA - Flurry, Rapid Strikes ect (Flurry does not require Triple Attack).
				int16 flurrychance = aabonuses.FlurryChance + spellbonuses.FlurryChance + itembonuses.FlurryChance;

				if (auto_attack_target && flurrychance)
				{
					if(MakeRandomInt(0, 100) < flurrychance)
					{
						Message_StringID(MT_NPCFlurry, 128);
						Attack(auto_attack_target, 13, false);
						Attack(auto_attack_target, 13, false);
					}
				}

				int16 ExtraAttackChanceBonus = spellbonuses.ExtraAttackChance + itembonuses.ExtraAttackChance + aabonuses.ExtraAttackChance;

				if (auto_attack_target && ExtraAttackChanceBonus) {
					ItemInst *wpn = GetInv().GetItem(SLOT_PRIMARY);
					if(wpn){
						if(wpn->GetItem()->ItemType == ItemType2HSlash ||
							wpn->GetItem()->ItemType == ItemType2HBlunt ||
							wpn->GetItem()->ItemType == ItemType2HPiercing )
						{
							if(MakeRandomInt(0, 100) < ExtraAttackChanceBonus)
							{
								Attack(auto_attack_target, 13, false);
							}
						}
					}
				}
			}
		}

		if (GetClass() == WARRIOR || GetClass() == BERSERKER) {
			if(!dead && !berserk && this->GetHPRatio() < 30) {
				entity_list.MessageClose_StringID(this, false, 200, 0, BERSERK_START, GetName());
				this->berserk = true;
			}
			if (berserk && this->GetHPRatio() > 30) {
				entity_list.MessageClose_StringID(this, false, 200, 0, BERSERK_END, GetName());
				this->berserk = false;
			}
		}

		if(auto_attack && may_use_attacks && auto_attack_target != nullptr
			&& CanThisClassDualWield() && attack_dw_timer.Check())
		{
			// Range check
			if(!CombatRange(auto_attack_target)) {
				// this is a duplicate message don't use it.
				Message_StringID(MT_TooFarAway,TARGET_TOO_FAR);
			}
			// Don't attack yourself
			else if(auto_attack_target == this) {
				Message_StringID(MT_TooFarAway,TRY_ATTACKING_SOMEONE);
			}
			else if (!los_status)
			{
				//you can't see your target
			}
			else if(auto_attack_target->GetHP() > -10) {
				float DualWieldProbability = 0.0f;

				int16 Ambidexterity = aabonuses.Ambidexterity + spellbonuses.Ambidexterity + itembonuses.Ambidexterity;
				DualWieldProbability = (GetSkill(SkillDualWield) + GetLevel() + Ambidexterity) / 400.0f; // 78.0 max
				int16 DWBonus = spellbonuses.DualWieldChance + itembonuses.DualWieldChance;
				DualWieldProbability += DualWieldProbability*float(DWBonus)/ 100.0f;

				float random = MakeRandomFloat(0, 1);
				CheckIncreaseSkill(SkillDualWield, auto_attack_target, -10);
				if (random < DualWieldProbability){ // Max 78% of DW
					if(CheckAAEffect(aaEffectRampage)) {
						entity_list.AEAttack(this, 30, 14);
					} else {
						Attack(auto_attack_target, 14);	// Single attack with offhand
					}
					ItemInst *wpn = GetInv().GetItem(SLOT_SECONDARY);
					TryWeaponProc(wpn, auto_attack_target, 14);

					if( CanThisClassDoubleAttack() && CheckDoubleAttack()) {
						if(CheckAAEffect(aaEffectRampage)) {
							entity_list.AEAttack(this, 30, 14);
						} else {
							if(auto_attack_target && auto_attack_target->GetHP() > -10)
								Attack(auto_attack_target, 14);	// Single attack with offhand
						}
					}
				}
			}
		}

		if (position_timer.Check()) {
			if (IsAIControlled())
			{
				if(IsMoving())
					SendPosUpdate(2);
				else
				{
					animation = 0;
					delta_x = 0;
					delta_y = 0;
					delta_z = 0;
					SendPosUpdate(2);
				}
			}

			// Send a position packet every 8 seconds - if not done, other clients
			// see this char disappear after 10-12 seconds of inactivity
			if (position_timer_counter >= 36) { // Approx. 4 ticks per second
				entity_list.SendPositionUpdates(this, pLastUpdateWZ, 500, GetTarget(), true);
				pLastUpdate = Timer::GetCurrentTime();
				pLastUpdateWZ = pLastUpdate;
				position_timer_counter = 0;
			}
			else {
				pLastUpdate = Timer::GetCurrentTime();
				position_timer_counter++;
			}
		}

		if(HasVirus()) {
			if(viral_timer.Check()) {
				viral_timer_counter++;
				for(int i = 0; i < MAX_SPELL_TRIGGER*2; i+=2) {
					if(viral_spells[i])	{
						if(viral_timer_counter % spells[viral_spells[i]].viral_timer == 0) {
							SpreadVirus(viral_spells[i], viral_spells[i+1]);
						}
					}
				}
			}
			if(viral_timer_counter > 999)
				viral_timer_counter = 0;
		}

		if(spellbonuses.GravityEffect == 1) {
			if(gravity_timer.Check())
				DoGravityEffect();
		}

		if (shield_timer.Check())
		{
			if (shield_target)
			{
				if (!CombatRange(shield_target))
				{
					entity_list.MessageClose(this,false,100,0,"%s ceases shielding %s.",GetCleanName(),shield_target->GetCleanName());
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
		if (endupkeep_timer.Check() && !dead){
			DoEnduranceUpkeep();
		}

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
			DoStaminaUpdate();

			if(tribute_timer.Check()) {
				ToggleTribute(true);	//re-activate the tribute.
			}

			if (fishing_timer.Check()) {
				GoFish();
			}

			if (autosave_timer.Check()) {
				Save(0);
			}

			if(m_pp.intoxication > 0)
			{
				--m_pp.intoxication;
				CalcBonuses();
			}

			if(ItemTickTimer.Check())
			{
				TickItemCheck();
			}

			if(ItemQuestTimer.Check())
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
		std::cout << "Client linkdead: " << name << std::endl;

		if (GetGM()) {
			if (GetMerc())
			{
				GetMerc()->Save();
				GetMerc()->Depop();
			}
			return false;
		}
		else if(!linkdead_timer.Enabled()){
			linkdead_timer.Start(RuleI(Zone,ClientLinkdeadMS));
			client_state = CLIENT_LINKDEAD;
			AI_Start(CLIENT_LD_TIMEOUT);
			SendAppearancePacket(AT_Linkdead, 1);
		}
	}


	/************ Get all packets from packet manager out queue and process them ************/
	EQApplicationPacket *app = nullptr;
	if(!eqs->CheckState(CLOSING))
	{
		while(ret && (app = (EQApplicationPacket *)eqs->PopPacket())) {
			if(app)
				ret = HandlePacket(app);
			safe_delete(app);
		}
	}

#ifdef REVERSE_AGGRO
	//At this point, we are still connected, everything important has taken
	//place, now check to see if anybody wants to aggro us.
	// only if client is not feigned
	if(ret && !GetFeigned() && scanarea_timer.Check()) {
		entity_list.CheckClientAggro(this);
	}
#endif

	if (client_state != CLIENT_LINKDEAD && (client_state == CLIENT_ERROR || client_state == DISCONNECTED || client_state == CLIENT_KICKED || !eqs->CheckState(ESTABLISHED))) {
		//client logged out or errored out
		//ResetTrade();
		if (client_state != CLIENT_KICKED) {
			Save();
		}
		if (GetMerc())
		{
			GetMerc()->Depop();
		}

		client_state = CLIENT_LINKDEAD;
		if (zoning || instalog || GetGM())
		{
			Group *mygroup = GetGroup();
			if (mygroup)
			{
				if (!zoning) {
					entity_list.MessageGroup(this, true, 15, "%s logged out.", GetName());
					mygroup->DelMember(this);
				} else {
					entity_list.MessageGroup(this, true, 15, "%s left the zone.", GetName());
					mygroup->MemberZoned(this);
				}

			}
			Raid *myraid = entity_list.GetRaidByClient(this);
			if (myraid)
			{
				if (!zoning) {
					//entity_list.MessageGroup(this,true,15,"%s logged out.",GetName());
					//mygroup->DelMember(this);
					myraid->MemberZoned(this);
				} else {
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

	return ret;
}

//just a set of actions preformed all over in Client::Process
void Client::OnDisconnect(bool hard_disconnect) {
	if(hard_disconnect) {
		LeaveGroup();

		Raid *MyRaid = entity_list.GetRaidByClient(this);

		if (MyRaid)
			MyRaid->MemberZoned(this);

		parse->EventPlayer(EVENT_DISCONNECT, this, "", 0);
	}

	Mob *Other = trade->With();

	if(Other)
	{
		mlog(TRADING__CLIENT, "Client disconnected during a trade. Returning their items.");

		FinishTrade(this);

		if(Other->IsClient())
			Other->CastToClient()->FinishTrade(Other);

		trade->Reset();

		Other->trade->Reset();
	}

	database.SetFirstLogon(CharacterID(), 0); //We change firstlogon status regardless of if a player logs out to zone or not, because we only want to trigger it on their first login from world.

	//remove ourself from all proximities
	ClearAllProximities();

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_LogoutReply);
	FastQueuePacket(&outapp);

	Disconnect();
}

// Sends the client complete inventory used in character login

// DO WE STILL NEED THE 'ITEMCOMBINED' CONDITIONAL CODE? -U

//#ifdef ITEMCOMBINED
void Client::BulkSendInventoryItems() {
	// For future reference: Only the parent item needs to be sent..the ItemInst already contains child ItemInst information
	int16 slot_id = 0;

	// LINKDEAD TRADE ITEMS
	// Move trade slot items back into normal inventory..need them there now for the proceeding validity checks -U
	for(slot_id = 3000; slot_id <= 3007; slot_id++) {
		ItemInst* inst = m_inv.PopItem(slot_id);
		if(inst) {
			bool is_arrow = (inst->GetItem()->ItemType == ItemTypeArrow) ? true : false;
			int16 free_slot_id = m_inv.FindFreeSlot(inst->IsType(ItemClassContainer), true, inst->GetItem()->Size, is_arrow);
			mlog(INVENTORY__ERROR, "Incomplete Trade Transaction: Moving %s from slot %i to %i", inst->GetItem()->Name, slot_id, free_slot_id);
			PutItemInInventory(free_slot_id, *inst, false);
			database.SaveInventory(character_id, nullptr, slot_id);
			safe_delete(inst);
		}
	}

	// Where are cursor buffer items processed? They need to be validated as well... -U

	bool deletenorent = database.NoRentExpired(GetName());
	if(deletenorent){ RemoveNoRent(false); } //client was offline for more than 30 minutes, delete no rent items

	RemoveDuplicateLore(false);
	MoveSlotNotAllowed(false);

	// The previous three method calls took care of moving/removing expired/illegal item placements -U

	//TODO: this function is just retarded... it re-allocates the buffer for every
	//new item. It should be changed to loop through once, gather the
	//lengths, and item packet pointers into an array (fixed length), and
	//then loop again to build the packet.
	//EQApplicationPacket *packets[50];
	//unsigned long buflen = 0;
	//unsigned long pos = 0;
	//memset(packets, 0, sizeof(packets));
	//foreach item in the invendor sections
	//	packets[pos++] = ReturnItemPacket(...)
	//	buflen += temp->size
	//...
	//allocat the buffer
	//for r from 0 to pos
	//	put pos[r]->pBuffer into the buffer
	//for r from 0 to pos
	//	safe_delete(pos[r]);

	uint32 size = 0;
	uint16 i = 0;
	std::map<uint16, std::string> ser_items;
	std::map<uint16, std::string>::iterator itr;

	//Inventory items
	for(slot_id = 0; slot_id <= 30; slot_id++) {
		const ItemInst* inst = m_inv[slot_id];
		if(inst) {
			std::string packet = inst->Serialize(slot_id);
			ser_items[i++] = packet;
			size += packet.length();
		}
	}

	// Power Source
	if(GetClientVersion() >= EQClientSoF) {
		const ItemInst* inst = m_inv[9999];
		if(inst) {
			std::string packet = inst->Serialize(9999);
			ser_items[i++] = packet;
			size += packet.length();
		}
	}

	// Bank items
	for(slot_id = 2000; slot_id <= 2023; slot_id++) {
		const ItemInst* inst = m_inv[slot_id];
		if(inst) {
			std::string packet = inst->Serialize(slot_id);
			ser_items[i++] = packet;
			size += packet.length();
		}
	}

	// Shared Bank items
	for(slot_id = 2500; slot_id <= 2501; slot_id++) {
		const ItemInst* inst = m_inv[slot_id];
		if(inst) {
			std::string packet = inst->Serialize(slot_id);
			ser_items[i++] = packet;
			size += packet.length();
		}
	}

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_CharInventory, size);
	uchar* ptr = outapp->pBuffer;
	for(itr = ser_items.begin(); itr != ser_items.end(); itr++){
		int length = itr->second.length();
		if(length > 5) {
			memcpy(ptr, itr->second.c_str(), length);
			ptr += length;
		}
	}
	QueuePacket(outapp);
	safe_delete(outapp);
}
/*#else
void Client::BulkSendInventoryItems()
{
	// Search all inventory buckets for items
	bool deletenorent=database.NoRentExpired(GetName());
	// Worn items and Inventory items
	int16 slot_id = 0;
	if(deletenorent){//client was offline for more than 30 minutes, delete no rent items
		RemoveNoRent();
	}
	for (slot_id=0; slot_id<=30; slot_id++) {
		const ItemInst* inst = m_inv[slot_id];
		if (inst){
			SendItemPacket(slot_id, inst, ItemPacketCharInventory);
		}
	}
	// Bank items
	for (slot_id=2000; slot_id<=2015; slot_id++) {
		const ItemInst* inst = m_inv[slot_id];
		if (inst){
			SendItemPacket(slot_id, inst, ItemPacketCharInventory);
		}
	}

	// Shared Bank items
	for (slot_id=2500; slot_id<=2501; slot_id++) {
		const ItemInst* inst = m_inv[slot_id];
		if (inst){
			SendItemPacket(slot_id, inst, ItemPacketCharInventory);
		}
	}

	// LINKDEAD TRADE ITEMS
	// If player went LD during a trade, they have items in the trade inventory
	// slots. These items are now being put into their inventory (then queue up on cursor)
	for (int16 trade_slot_id=3000; trade_slot_id<=3007; trade_slot_id++) {
		const ItemInst* inst = m_inv[slot_id];
		if (inst) {
			int16 free_slot_id = m_inv.FindFreeSlot(inst->IsType(ItemClassContainer), true, inst->GetItem()->Size);
			DeleteItemInInventory(trade_slot_id, 0, false);
			PutItemInInventory(free_slot_id, *inst, true);
		}
	}
}
#endif*/

void Client::BulkSendMerchantInventory(int merchant_id, int npcid) {
	const Item_Struct* handyitem = nullptr;
	uint32 numItemSlots=80; //The max number of items passed in the transaction.
	const Item_Struct *item;
	std::list<MerchantList> merlist = zone->merchanttable[merchant_id];
	std::list<MerchantList>::const_iterator itr;
	Mob* merch = entity_list.GetMobByNpcTypeID(npcid);
	if(merlist.size()==0){ //Attempt to load the data, it might have been missed if someone spawned the merchant after the zone was loaded
		zone->LoadNewMerchantData(merchant_id);
		merlist = zone->merchanttable[merchant_id];
		if(merlist.size()==0)
			return;
	}
	std::list<TempMerchantList> tmp_merlist = zone->tmpmerchanttable[npcid];
	std::list<TempMerchantList>::iterator tmp_itr;

	uint32 i=1;
	uint8 handychance = 0;
	for(itr = merlist.begin();itr != merlist.end() && i<numItemSlots;itr++){
		MerchantList ml = *itr;
		if(GetLevel() < ml.level_required) {
			continue;
		}

		int32 fac = merch ? merch->GetPrimaryFaction() : 0;
		if(fac != 0 && GetModCharacterFactionLevel(fac) < ml.faction_required) {
			continue;
		}

		handychance = MakeRandomInt(0, merlist.size() + tmp_merlist.size() - 1 );

		item = database.GetItem(ml.item);
		if(item) {
			if(handychance==0)
				handyitem=item;
			else
				handychance--;
			int charges=1;
			if(item->ItemClass==ItemClassCommon)
				charges=item->MaxCharges;
			ItemInst* inst = database.CreateItem(item, charges);
			if (inst) {
				if (RuleB(Merchant, UsePriceMod)){
				inst->SetPrice((item->Price*(RuleR(Merchant, SellCostMod))*item->SellRate*Client::CalcPriceMod(merch,false)));
				}
				else
					inst->SetPrice((item->Price*(RuleR(Merchant, SellCostMod))*item->SellRate));
				inst->SetMerchantSlot(ml.slot);
				inst->SetMerchantCount(-1);		//unlimited
				if(charges > 0)
					inst->SetCharges(charges);
				else
					inst->SetCharges(1);

				SendItemPacket(ml.slot-1, inst, ItemPacketMerchant);
				safe_delete(inst);
			}
		}
		// Account for merchant lists with gaps.
		if(ml.slot >= i)
			i = ml.slot + 1;
	}
	std::list<TempMerchantList> origtmp_merlist = zone->tmpmerchanttable[npcid];
	tmp_merlist.clear();
	for(tmp_itr = origtmp_merlist.begin();tmp_itr != origtmp_merlist.end() && i<numItemSlots;tmp_itr++){
		TempMerchantList ml = *tmp_itr;
		item=database.GetItem(ml.item);
		ml.slot=i;
		if (item) {
			if(handychance==0)
				handyitem=item;
			else
				handychance--;
			int charges=1;
			//if(item->ItemClass==ItemClassCommon && (int16)ml.charges <= item->MaxCharges)
			//	charges=ml.charges;
			//else
			charges = item->MaxCharges;
			ItemInst* inst = database.CreateItem(item, charges);
			if (inst) {
				if (RuleB(Merchant, UsePriceMod)){
				inst->SetPrice((item->Price*(RuleR(Merchant, SellCostMod))*item->SellRate*Client::CalcPriceMod(merch,false)));
				}
				else
					inst->SetPrice((item->Price*(RuleR(Merchant, SellCostMod))*item->SellRate));
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
	if(merch != nullptr && handyitem){
		char handy_id[8]={0};
		int greeting=MakeRandomInt(0, 4);
		int greet_id=0;
		switch(greeting){
			case 1:
				greet_id=MERCHANT_GREETING;
				break;
			case 2:
				greet_id=MERCHANT_HANDY_ITEM1;
				break;
			case 3:
				greet_id=MERCHANT_HANDY_ITEM2;
				break;
			case 4:
				greet_id=MERCHANT_HANDY_ITEM3;
				break;
			default:
				greet_id=MERCHANT_HANDY_ITEM4;
		}
		sprintf(handy_id,"%i",greet_id);

		if(greet_id!=MERCHANT_GREETING)
			Message_StringID(10,GENERIC_STRINGID_SAY,merch->GetCleanName(),handy_id,this->GetName(),handyitem->Name);
		else
			Message_StringID(10,GENERIC_STRINGID_SAY,merch->GetCleanName(),handy_id,this->GetName());

		merch->CastToNPC()->FaceTarget(this->CastToMob());
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
		_log(TRADING__CLIENT, "Previous customer has gone away.");
		CustomerID = NewCustomer;
		return 1;
	}

	return 0;
}

void Client::OPRezzAnswer(uint32 Action, uint32 SpellID, uint16 ZoneID, uint16 InstanceID, float x, float y, float z)
{
	if(PendingRezzXP < 0) {
		// pendingrezexp is set to -1 if we are not expecting an OP_RezzAnswer
		_log(SPELLS__REZ, "Unexpected OP_RezzAnswer. Ignoring it.");
		Message(13, "You have already been resurrected.\n");
		return;
	}

	if (Action == 1)
	{
		// Mark the corpse as rezzed in the database, just in case the corpse has buried, or the zone the
		// corpse is in has shutdown since the rez spell was cast.
		database.MarkCorpseAsRezzed(PendingRezzDBID);
		_log(SPELLS__REZ, "Player %s got a %i Rezz, spellid %i in zone%i, instance id %i",
				this->name, (uint16)spells[SpellID].base[0],
				SpellID, ZoneID, InstanceID);

		this->BuffFadeAll();
		int SpellEffectDescNum = GetSpellEffectDescNum(SpellID);
		// Rez spells with Rez effects have this DescNum (first is Titanium, second is 6.2 Client)
		if((SpellEffectDescNum == 82) || (SpellEffectDescNum == 39067)) {
			SetMana(0);
			SetHP(GetMaxHP()/5);
			SpellOnTarget(756, this); // Rezz effects
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
		LogFile->write(EQEMuLog::Error,"Wrong size on OP_MemorizeSpell. Got: %i, Expected: %i", app->size, sizeof(MemorizeSpell_Struct));
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
			const ItemInst* inst = m_inv[SLOT_CURSOR];

			if(inst && inst->IsType(ItemClassCommon))
			{
				const Item_Struct* item = inst->GetItem();

				if(item && item->Scroll.Effect == (int32)(memspell->spell_id))
				{
					ScribeSpell(memspell->spell_id, memspell->slot);
					DeleteItemInInventory(SLOT_CURSOR, 1, true);
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

void Client::BreakInvis()
{
	if (invisible)
	{
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
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

		EQApplicationPacket* outapp = new EQApplicationPacket(OP_TradeCoins,sizeof(TradeCoin_Struct));
		TradeCoin_Struct* tcs = (TradeCoin_Struct*)outapp->pBuffer;
		tcs->trader = trader->GetID();
		tcs->slot = mc->cointype2;
		tcs->unknown5 = 0x4fD2;
		tcs->unknown7 = 0;
		tcs->amount = amount_to_add;
		recipient->QueuePacket(outapp);
		safe_delete(outapp);
	}

	Save();
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
	if(DistNoRoot(*pTrainer) > USE_NPC_RANGE2)
		return;

	SkillUseTypes sk;
	for (sk = Skill1HBlunt; sk <= HIGHEST_SKILL; sk = (SkillUseTypes)(sk+1)) {
		if(sk == SkillTinkering && GetRace() != GNOME) {
			gmtrain->skills[sk] = 0; //Non gnomes can't tinker!
		} else {
			gmtrain->skills[sk] = GetMaxSkillAfterSpecializationRules(sk, MaxSkill(sk, GetClass(), RuleI(Character, MaxLevel)));
			//this is the highest level that the trainer can train you to, this is enforced clientside so we can't just
			//Set it to 1 with CanHaveSkill or you wont be able to train past 1.
		}
	}

	uchar ending[]={0x34,0x87,0x8a,0x3F,0x01
		,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9
		,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9,0xC9
		,0x76,0x75,0x3f};
	memcpy(&outapp->pBuffer[outapp->size-40],ending,sizeof(ending));
	FastQueuePacket(&outapp);

	// welcome message
	if (pTrainer && pTrainer->IsNPC())
	{
		pTrainer->Say_StringID(MakeRandomInt(1204, 1207), GetCleanName());
	}
}

void Client::OPGMEndTraining(const EQApplicationPacket *app)
{
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_GMEndTrainingResponse, 0);
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
	if(DistNoRoot(*pTrainer) > USE_NPC_RANGE2)
		return;

	// goodbye message
	if (pTrainer->IsNPC())
	{
		pTrainer->Say_StringID(MakeRandomInt(1208, 1211), GetCleanName());
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
	if(DistNoRoot(*pTrainer) > USE_NPC_RANGE2)
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
		if (gmskill->skill_id > HIGHEST_SKILL)
		{
			std::cout << "Wrong Training Skill (abilities)" << std::endl;
			DumpPacket(app);
			return;
		}

		SkillUseTypes skill = (SkillUseTypes) gmskill->skill_id;

		if(!CanHaveSkill(skill)) {
			mlog(CLIENT__ERROR, "Tried to train skill %d, which is not allowed.", skill);
			return;
		}

		if(MaxSkill(skill) == 0) {
			mlog(CLIENT__ERROR, "Tried to train skill %d, but training is not allowed at this level.", skill);
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
			case SkillBrewing:
			case SkillMakePoison:
			case SkillTinkering:
			case SkillResearch:
			case SkillAlchemy:
			case SkillBaking:
			case SkillTailoring:
			case SkillBlacksmithing:
			case SkillFletching:
			case SkillJewelryMaking:
			case SkillPottery:
				if(skilllevel >= RuleI(Skills, MaxTrainTradeskills)) {
					Message_StringID(13, MORE_SKILLED_THAN_I, pTrainer->GetCleanName());
					return;
				}
				break;
			case SkillSpecializeAbjure:
			case SkillSpecializeAlteration:
			case SkillSpecializeConjuration:
			case SkillSpecializeDivination:
			case SkillSpecializeEvocation:
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

			if(gmskill->skill_id >= SkillSpecializeAbjure && gmskill->skill_id <= SkillSpecializeEvocation)
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

	if(GetClientVersion() >= EQClientSoF) {
		// The following packet decreases the skill points left in the Training Window and
		// produces the 'You have increased your skill / learned the basics of' message.
		//
		EQApplicationPacket* outapp = new EQApplicationPacket(OP_GMTrainSkillConfirm, sizeof(GMTrainSkillConfirm_Struct));

		GMTrainSkillConfirm_Struct *gmtsc = (GMTrainSkillConfirm_Struct *)outapp->pBuffer;
		gmtsc->SkillID = gmskill->skill_id;

		if(gmskill->skillbank == 1) {
			gmtsc->NewSkill = (GetLanguageSkill(gmtsc->SkillID) == 1);
			gmtsc->SkillID += 100;
		}
		else
			gmtsc->NewSkill = (GetRawSkill((SkillUseTypes)gmtsc->SkillID) == 1);

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
				ServerPacket* pack = new ServerPacket(ServerOP_ZonePlayer, sizeof(ServerZonePlayer_Struct));
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
	SetHP(GetHP() + CalcHPRegen() + RestRegenHP);
	SendHPUpdate();
}

void Client::DoManaRegen() {
	if (GetMana() >= max_mana)
		return;

	SetMana(GetMana() + CalcManaRegen() + RestRegenMana);
	SendManaUpdatePacket();
}


void Client::DoStaminaUpdate() {
	if(!stamina_timer.Check())
		return;

	EQApplicationPacket* outapp = new EQApplicationPacket(OP_Stamina, sizeof(Stamina_Struct));
	Stamina_Struct* sta = (Stamina_Struct*)outapp->pBuffer;

	if(zone->GetZoneID() != 151) {
		int loss = RuleI(Character, FoodLossPerUpdate);
		if (m_pp.hunger_level > 0)
			m_pp.hunger_level-=loss;
		if (m_pp.thirst_level > 0)
			m_pp.thirst_level-=loss;
		sta->food = m_pp.hunger_level > 6000 ? 6000 : m_pp.hunger_level;
		sta->water = m_pp.thirst_level> 6000 ? 6000 : m_pp.thirst_level;
	}
	else {
		// No auto food/drink consumption in the Bazaar
		sta->food = 6000;
		sta->water = 6000;
	}
	FastQueuePacket(&outapp);
}

void Client::DoEnduranceRegen()
{
	if(GetEndurance() >= GetMaxEndurance())
		return;

	SetEndurance(GetEndurance() + CalcEnduranceRegen() + RestRegenEndurance);
}

void Client::DoEnduranceUpkeep() {
	int upkeep_sum = 0;

	int cost_redux = spellbonuses.EnduranceReduction + itembonuses.EnduranceReduction;

	uint32 buffs_i;
	uint32 buff_count = GetMaxTotalSlots();
	for (buffs_i = 0; buffs_i < buff_count; buffs_i++) {
		if (buffs[buffs_i].spellid != SPELL_UNKNOWN) {
			int upkeep = spells[buffs[buffs_i].spellid].EndurUpkeep;
			if(upkeep > 0) {
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

	if(upkeep_sum != 0)
		SetEndurance(GetEndurance() - upkeep_sum);
}

void Client::CalcRestState() {

	// This method calculates rest state HP and mana regeneration.
	// The client must have been out of combat for RuleI(Character, RestRegenTimeToActivate) seconds,
	// must be sitting down, and must not have any detrimental spells affecting them.
	//
	if(!RuleI(Character, RestRegenPercent))
		return;

	RestRegenHP = RestRegenMana = RestRegenEndurance = 0;

	if(AggroCount || !IsSitting())
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

	RestRegenHP = (GetMaxHP() * RuleI(Character, RestRegenPercent) / 100);

	RestRegenMana = (GetMaxMana() * RuleI(Character, RestRegenPercent) / 100);

	if(RuleB(Character, RestRegenEndurance))
		RestRegenEndurance = (GetMaxEndurance() * RuleI(Character, RestRegenPercent) / 100);
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
		default_to_bind->zoneid = b->zoneId;
		default_to_bind->x = b->x;
		default_to_bind->y = b->y;
		default_to_bind->z = b->z;
		default_to_bind->heading = b->heading;
		chosen = default_to_bind;
		is_rez = false;
	}

	if (chosen->zoneid == zone->GetZoneID()) //If they should respawn in the current zone...
	{
		if (is_rez)
		{
			if (PendingRezzXP < 0 || PendingRezzSpellID == 0)
			{
				_log(SPELLS__REZ, "Unexpected Rezz from hover request.");
				return;
			}
			SetHP(GetMaxHP() / 5);

			Corpse* corpse = entity_list.GetCorpseByName(PendingRezzCorpseName.c_str());

			if (corpse)
			{
				x_pos = corpse->GetX();
				y_pos = corpse->GetY();
				z_pos = corpse->GetZ();
			}

			EQApplicationPacket* outapp = new EQApplicationPacket(OP_ZonePlayerToBind, sizeof(ZonePlayerToBind_Struct) + 10);
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
				_log(SPELLS__REZ, "Hover Rez in zone %s for corpse %s",
						zone->GetShortName(), PendingRezzCorpseName.c_str());

				_log(SPELLS__REZ, "Found corpse. Marking corpse as rezzed.");

				corpse->Rezzed(true);
				corpse->CompleteRezz();
			}
		}
		else //Not rez
		{
			PendingRezzSpellID = 0;

			EQApplicationPacket* outapp = new EQApplicationPacket(OP_ZonePlayerToBind, sizeof(ZonePlayerToBind_Struct) + chosen->name.length() + 1);
			ZonePlayerToBind_Struct* gmg = (ZonePlayerToBind_Struct*) outapp->pBuffer;

			gmg->bind_zone_id = zone->GetZoneID();
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

			x_pos = chosen->x;
			y_pos = chosen->y;
			z_pos = chosen->z;
			heading = chosen->heading;

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

		m_pp.zone_id = chosen->zoneid;
		m_pp.zoneInstance = 0;
		database.MoveCharacterToZone(this->CharacterID(), database.GetZoneName(chosen->zoneid));

		Save();

		MovePC(chosen->zoneid,chosen->x,chosen->y,chosen->z,chosen->heading,1);
	}

	safe_delete(default_to_bind);
}

void Client::ClearHover()
{
	// Our Entity ID is currently zero, set in Client::Death
	SetID(entity_list.GetFreeID());

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_ZoneEntry, sizeof(ServerZoneEntry_Struct));
	ServerZoneEntry_Struct* sze = (ServerZoneEntry_Struct*)outapp->pBuffer;

	FillSpawnStruct(&sze->player,CastToMob());

	sze->player.spawn.NPC = 0;
	sze->player.spawn.z += 6;	//arbitrary lift, seems to help spawning under zone.

	entity_list.QueueClients(this, outapp, false);
	safe_delete(outapp);

	if(IsClient() && CastToClient()->GetClientVersionBit() & BIT_UnderfootAndLater)
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

			EQApplicationPacket *outapp = new EQApplicationPacket(OP_LFGuild, PacketSize);
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
			EQApplicationPacket *outapp = new EQApplicationPacket(OP_LFGuild, sizeof(LFGuild_PlayerToggle_Struct));
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

			EQApplicationPacket *outapp = new EQApplicationPacket(OP_LFGuild, PacketSize);
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

			EQApplicationPacket *outapp = new EQApplicationPacket(OP_LFGuild, sizeof(LFGuild_GuildToggle_Struct));

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
	ServerPacket* pack = new ServerPacket(ServerOP_QueryServGeneric, strlen(GetName()) + 17);

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
	ServerPacket* pack = new ServerPacket(ServerOP_QueryServGeneric, strlen(GetName()) + +strlen(guild_mgr.GetGuildName(GuildID())) + 18);

	pack->WriteUInt32(zone->GetZoneID());
	pack->WriteUInt32(zone->GetInstanceID());
	pack->WriteString(GetName());
	pack->WriteUInt32(QSG_LFGuild);
	pack->WriteUInt32(QSG_LFGuild_RequestGuildInfo);
	pack->WriteString(guild_mgr.GetGuildName(GuildID()));

	worldserver.SendPacket(pack);
	safe_delete(pack);
}
