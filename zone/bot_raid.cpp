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

#include "bot.h"
#include "object.h"
#include "raids.h"
#include "doors.h"
#include "quest_parser_collection.h"
#include "lua_parser.h"
#include "../common/strings.h"
#include "../common/say_link.h"
#include "../common/repositories/bot_spell_settings_repository.h"
#include "../common/data_verification.h"

extern volatile bool is_zone_loaded;
extern bool Critical; 

// AI Processing for the Bot object

constexpr float MAX_CASTER_DISTANCE[PLAYER_CLASS_COUNT] = {
	0, (34 * 34), (24 * 24), (28 * 28), (26 * 26), (42 * 42), 0, (30 * 30), 0, (38 * 38), (54 * 54), (48 * 48), (52 * 52), (50 * 50), (32 * 32), 0
	//  W      C          P          R          S          D      M      B      R      S          N          W          M          E          B      B
	//  A      L          A          N          H          R      N      R      O      H          E          I          A          N          S      E
	//  R      R          L          G          D          U      K      D      G      M          C          Z          G          C          T      R
};

void Bot::AI_Process_Raid()
{
#define TEST_COMBATANTS() if (!GetTarget() || GetAppearance() == eaDead) { return; }
#define PULLING_BOT (GetPullingFlag() || GetReturningFlag())
#define NOT_PULLING_BOT (!GetPullingFlag() && !GetReturningFlag())
#define GUARDING (GetGuardFlag())
#define NOT_GUARDING (!GetGuardFlag())
#define HOLDING (GetHoldFlag())
#define NOT_HOLDING (!GetHoldFlag())
#define PASSIVE (GetBotStance() == EQ::constants::stancePassive)
#define NOT_PASSIVE (GetBotStance() != EQ::constants::stancePassive)

	Raid* raid = entity_list.GetRaidByBotName(GetName());
	Client* bot_owner = (GetBotOwner() && GetBotOwner()->IsClient() ? GetBotOwner()->CastToClient() : nullptr);
	uint32 r_group = raid->GetGroup(GetName());

	LogAI("Bot_Raid: Entered Raid Process() for [{}].", GetCleanName());

	//#pragma region PRIMARY AI SKIP CHECKS

		// Primary reasons for not processing AI
	if (!bot_owner || (!raid) || !IsAIControlled()) {
		return;
	}

	if (bot_owner->IsDead()) {

		SetTarget(nullptr);
		SetBotOwner(nullptr);

		return;
	}

	// We also need a leash owner and follow mob (subset of primary AI criteria)
	Client* leash_owner = nullptr;
	if (r_group < MAX_RAID_GROUPS && raid->IsGroupLeader(GetName())) {
		leash_owner = bot_owner;
		SetFollowID(leash_owner->GetID());
	}
	else if (r_group < MAX_RAID_GROUPS && raid->GetGroupLeader(r_group) && raid->GetGroupLeader(r_group)->IsClient()) {
		if (raid->GetGroupLeader(r_group)) {
			leash_owner = raid->GetGroupLeader(r_group);
			SetFollowID(leash_owner->GetID());
		}
	}
	else {
		leash_owner = bot_owner;
		SetFollowID(leash_owner->GetID());
	}

	if (!leash_owner) {
		return;
	}

	//#pragma endregion

	Mob* follow_mob = entity_list.GetMob(GetFollowID());
	if (!follow_mob) {

		follow_mob = leash_owner;
		SetFollowID(leash_owner->GetID());
	}
	
	if (mana_timer.Check(false)) {
		raid->SendHPManaEndPacketsFrom(this);
	}
	if (send_hp_update_timer.Check(false)) {

		raid->SendHPManaEndPacketsFrom(this);

	}
	// Berserk updates should occur if primary AI criteria are met
	if (GetClass() == WARRIOR || GetClass() == BERSERKER) {

		if (!berserk && GetHP() > 0 && GetHPRatio() < 30.0f) {

			entity_list.MessageCloseString(this, false, 200, 0, BERSERK_START, GetName());
			berserk = true;
		}

		if (berserk && GetHPRatio() >= 30.0f) {

			entity_list.MessageCloseString(this, false, 200, 0, BERSERK_END, GetName());
			berserk = false;
		}
	}

	//#pragma region SECONDARY AI SKIP CHECKS

		// Secondary reasons for not processing AI
	if (GetPauseAI() || IsStunned() || IsMezzed() || (GetAppearance() == eaDead)) {

		if (IsCasting()) {
			InterruptSpell();
		}

		if (IsMyHealRotationSet() || (AmICastingForHealRotation() && m_member_of_heal_rotation->CastingMember() == this)) {

			AdvanceHealRotation(false);
			m_member_of_heal_rotation->SetMemberIsCasting(this, false);
		}

		return;
	}

	//#pragma endregion

	float fm_distance = DistanceSquared(m_Position, follow_mob->GetPosition());
	float lo_distance = DistanceSquared(m_Position, leash_owner->GetPosition());
	float leash_distance = RuleR(Bots, LeashDistance);

	//#pragma region CURRENTLY CASTING CHECKS

	if (IsCasting()) {

		if (IsHealRotationMember() &&
			m_member_of_heal_rotation->CastingOverride() &&
			m_member_of_heal_rotation->CastingTarget() != nullptr &&
			m_member_of_heal_rotation->CastingReady() &&
			m_member_of_heal_rotation->CastingMember() == this &&
			!m_member_of_heal_rotation->MemberIsCasting(this))
		{
			InterruptSpell();
		}
		else if (AmICastingForHealRotation() && m_member_of_heal_rotation->CastingMember() == this) {

			AdvanceHealRotation(false);
			return;
		}
		else if (GetClass() != BARD) {

			if (IsEngaged()) {
				return;
			}

			if (
				(NOT_GUARDING && fm_distance > GetFollowDistance()) || // Cancel out-of-combat casting if movement to follow mob is required
				(GUARDING && DistanceSquared(GetPosition(), GetGuardPoint()) > GetFollowDistance()) // Cancel out-of-combat casting if movement to guard point is required
				) {
				InterruptSpell();
			}

			return;
		}
	}
	else if (IsHealRotationMember()) {
		m_member_of_heal_rotation->SetMemberIsCasting(this, false);
	}

	//#pragma endregion

		// Can't move if rooted...
	if (IsRooted() && IsMoving()) {

		StopMoving();
		return;
	}

	//#pragma region HEAL ROTATION CASTING CHECKS

	if (IsMyHealRotationSet()) {

		if (AIHealRotation(HealRotationTarget(), UseHealRotationFastHeals())) {

			m_member_of_heal_rotation->SetMemberIsCasting(this);
			m_member_of_heal_rotation->UpdateTargetHealingStats(HealRotationTarget());
			AdvanceHealRotation();
		}
		else {

			m_member_of_heal_rotation->SetMemberIsCasting(this, false);
			AdvanceHealRotation(false);
		}
	}

	//#pragma endregion

	bool bo_alt_combat = (RuleB(Bots, AllowOwnerOptionAltCombat) && bot_owner->GetBotOption(Client::booAltCombat));

	//#pragma region ATTACK FLAG

	if (GetAttackFlag()) { // Push owner's target onto our hate list

		if (GetPet() && PULLING_BOT) {
			GetPet()->SetPetOrder(m_previous_pet_order);
		}

		SetAttackFlag(false);
		SetAttackingFlag(false);
		SetPullFlag(false);
		SetPullingFlag(false);
		SetReturningFlag(false);
		bot_owner->SetBotPulling(false);

		if (NOT_HOLDING && NOT_PASSIVE) {

			auto attack_target = bot_owner->GetTarget();

			if (attack_target) {

				InterruptSpell();
				WipeHateList();
				AddToHateList(attack_target, 1);
				SetTarget(attack_target);
				SetAttackingFlag();
				if (HasPet() && (GetClass() != ENCHANTER || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 2)) {

					GetPet()->WipeHateList();
					GetPet()->AddToHateList(attack_target, 1);
					GetPet()->SetTarget(attack_target);
				}
			}
		}
	}

	//#pragma endregion

	//#pragma region PULL FLAG

	else if (GetPullFlag()) { // Push owner's target onto our hate list and set flags so other bots do not aggro

		SetAttackFlag(false);
		SetAttackingFlag(false);
		SetPullFlag(false);
		SetPullingFlag(false);
		SetReturningFlag(false);
		bot_owner->SetBotPulling(false);

		if (NOT_HOLDING && NOT_PASSIVE) {

			auto pull_target = bot_owner->GetTarget();
			if (pull_target) {
				const auto msg = fmt::format("Pulling {} to the group..", pull_target->GetCleanName());
				raid->RaidSay(msg.c_str(), GetCleanName(), 0, 100);
				InterruptSpell();
				WipeHateList();
				AddToHateList(pull_target, 1);
				SetTarget(pull_target);
				SetPullingFlag();
				bot_owner->SetBotPulling();
				if (HasPet() && (GetClass() != ENCHANTER || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 1)) {

					GetPet()->WipeHateList();
					GetPet()->SetTarget(nullptr);
					m_previous_pet_order = GetPet()->GetPetOrder();
					GetPet()->SetPetOrder(SPO_Guard);
				}
			}
		}
	}

	//#pragma endregion

	//#pragma region ALT COMBAT (ACQUIRE HATE)

	else if (bo_alt_combat && m_alt_combat_hate_timer.Check(false)) { // 'Alt Combat' gives some more 'control' options on how bots process aggro

		// Empty hate list - let's find some aggro
		if (!IsEngaged() && NOT_HOLDING && NOT_PASSIVE && (!bot_owner->GetBotPulling() || NOT_PULLING_BOT)) {

			Mob* lo_target = leash_owner->GetTarget();
			if (lo_target &&
				lo_target->IsNPC() &&
				!lo_target->IsMezzed() &&
				((bot_owner->GetBotOption(Client::booAutoDefend) && lo_target->GetHateAmount(leash_owner)) || leash_owner->AutoAttackEnabled()) &&
				lo_distance <= leash_distance &&
				DistanceSquared(m_Position, lo_target->GetPosition()) <= leash_distance &&
				(CheckLosFN(lo_target) || leash_owner->CheckLosFN(lo_target)) &&
				IsAttackAllowed(lo_target))
			{
				AddToHateList(lo_target, 1);
				if (HasPet() && (GetClass() != ENCHANTER || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 2)) {

					GetPet()->AddToHateList(lo_target, 1);
					GetPet()->SetTarget(lo_target);
				}
			}
			else {

				std::vector<RaidMember> raid_group_members = raid->GetRaidGroupMembers(r_group);
				for (std::vector<RaidMember>::iterator iter = raid_group_members.begin(); iter != raid_group_members.end(); ++iter) {

					Mob* bg_member = iter->member;
					if (!bg_member) {
						continue;
					}

					Mob* bgm_target = bg_member->GetTarget();
					if (bgm_target) {
						if (!bgm_target || !bgm_target->IsNPC()) {
							continue;
						}

						if (!bgm_target->IsMezzed() &&
							((bot_owner->GetBotOption(Client::booAutoDefend) && bgm_target->GetHateAmount(bg_member)) || leash_owner->AutoAttackEnabled()) &&
							lo_distance <= leash_distance &&
							DistanceSquared(m_Position, bgm_target->GetPosition()) <= leash_distance &&
							(CheckLosFN(bgm_target) || leash_owner->CheckLosFN(bgm_target)) &&
							IsAttackAllowed(bgm_target))
						{
							AddToHateList(bgm_target, 1);
							if (HasPet() && (GetClass() != ENCHANTER || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 2)) {

								GetPet()->AddToHateList(bgm_target, 1);
								GetPet()->SetTarget(bgm_target);
							}

							break;
						}
					}
				}
			}
		}
	}

	//#pragma endregion

	glm::vec3 Goal(0, 0, 0);

	// We have aggro to choose from
	if (IsEngaged()) {

		if (rest_timer.Enabled()) {
			rest_timer.Disable();
		}
//		AddToHateList(GetTarget(), 1);
		
		//#pragma region PULLING FLAG (TARGET VALIDATION)

		if (GetPullingFlag()) {

			if (!GetTarget()) {

				WipeHateList();
				SetTarget(nullptr);
				SetPullingFlag(false);
				SetReturningFlag(false);
				bot_owner->SetBotPulling(false);
				if (GetPet()) {
					GetPet()->SetPetOrder(m_previous_pet_order);
				}

				return;
			}
			else if (GetTarget()->GetHateList().size()) {

				WipeHateList();
				SetTarget(nullptr);
				SetPullingFlag(false);
				SetReturningFlag();

				return;
			}
			else {
				// Default action is to aggress towards enemy
			}
		}

		//#pragma endregion

		//#pragma region RETURNING FLAG

		else if (GetReturningFlag()) {

			// Need to make it back to group before clearing return flag
			if (fm_distance <= GetFollowDistance()) {

				// Once we're back, clear blocking flags so everyone else can join in
				SetReturningFlag(false);
				bot_owner->SetBotPulling(false);
				if (GetPet()) {
					GetPet()->SetPetOrder(m_previous_pet_order);
				}
			}

			// Need to keep puller out of combat until they reach their 'return to' destination
			if (HasTargetReflection()) {

				SetTarget(nullptr);
				WipeHateList();

				return;
			}
		}

		//#pragma endregion

		//#pragma region ALT COMBAT (ACQUIRE TARGET)

		else if (bo_alt_combat && m_alt_combat_hate_timer.Check()) { // Find a mob from hate list to target

			// Raid Group roles can be expounded upon in the future
			//r_group is the uint32 group id
			auto assist_mob = raid->GetRaidMainAssistOneByName(GetName());
			bool find_target = true;

			//if (!assist_mob) {
			//	bot_owner->Message(Chat::Yellow, "Assist Mob is nullptr");
			//}

			if (assist_mob) {

				if (assist_mob->GetTarget()) {

					if (assist_mob != this) {

						SetTarget(assist_mob->GetTarget());
						if (HasPet() && (GetClass() != ENCHANTER || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 2)) {

							// This artificially inflates pet's target aggro..but, less expensive than checking hate each AI process
							GetPet()->AddToHateList(assist_mob->GetTarget(), 1);
							GetPet()->SetTarget(assist_mob->GetTarget());
						}
					}

					find_target = false;
				}
				else if (assist_mob != this) {

					SetTarget(nullptr);
					if (HasPet() && (GetClass() != ENCHANTER || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 1)) {

						GetPet()->WipeHateList();
						GetPet()->SetTarget(nullptr);
					}

					find_target = false;
				}
			}

			if (find_target) {

				if (IsRooted()) {
					SetTarget(hate_list.GetClosestEntOnHateList(this, true));
				}
				else {

					// This will keep bots on target for now..but, future updates will allow for rooting/stunning
					SetTarget(hate_list.GetEscapingEntOnHateList(leash_owner, leash_distance));
					if (!GetTarget()) {
						SetTarget(hate_list.GetEntWithMostHateOnList(this, nullptr, true));
					}
				}
			}
		}

		//#pragma endregion

		//#pragma region DEFAULT (ACQUIRE TARGET)

		else {

			// Default behavior doesn't have a means of acquiring a target from the bot's hate list..
			// ..that action occurs through commands or out-of-combat checks
			// (Use current target, if already in combat)
		}

		//#pragma endregion

		//#pragma region VERIFY TARGET AND STANCE

		Mob* tar = GetTarget(); // We should have a target..if not, we're awaiting new orders
		if (!tar || PASSIVE) {

			SetTarget(nullptr);
			WipeHateList();
			SetAttackFlag(false);
			SetAttackingFlag(false);
			if (PULLING_BOT) {

				// 'Flags' should only be set on the bot that is pulling
				SetPullingFlag(false);
				SetReturningFlag(false);
				bot_owner->SetBotPulling(false);
				if (GetPet()) {
					GetPet()->SetPetOrder(m_previous_pet_order);
				}
			}

			if (GetArchetype() == ARCHETYPE_CASTER) {
				BotMeditate(true);
			}

			return;
		}

		//#pragma endregion

		//#pragma region ATTACKING FLAG (HATE VALIDATION)

		if (GetAttackingFlag() && tar->CheckAggro(this)) {
			SetAttackingFlag(false);
		}

		//#pragma endregion

		float tar_distance = DistanceSquared(m_Position, tar->GetPosition());

		//#pragma region TARGET VALIDATION

				// DOUBLE-CHECK THIS CRITERIA

				// Verify that our target has attackable criteria
		if (HOLDING ||
			!tar->IsNPC() ||
			tar->IsMezzed() ||
			lo_distance > leash_distance ||
			tar_distance > leash_distance ||
			(!GetAttackingFlag() && !CheckLosFN(tar) && !leash_owner->CheckLosFN(tar)) || // This is suppose to keep bots from attacking things behind walls
			!IsAttackAllowed(tar) ||
			(bo_alt_combat &&
				(!GetAttackingFlag() && NOT_PULLING_BOT && !leash_owner->AutoAttackEnabled() && !tar->GetHateAmount(this) && !tar->GetHateAmount(leash_owner))
				)
			)
		{
			// Normally, we wouldn't want to do this without class checks..but, too many issues can arise if we let enchanter animation pets run rampant
			if (HasPet()) {

				GetPet()->RemoveFromHateList(tar);
				GetPet()->SetTarget(nullptr);
			}

			RemoveFromHateList(tar);
			SetTarget(nullptr);

			SetAttackFlag(false);
			SetAttackingFlag(false);
			if (PULLING_BOT) {

				SetPullingFlag(false);
				SetReturningFlag(false);
				bot_owner->SetBotPulling(false);
				if (GetPet()) {
					GetPet()->SetPetOrder(m_previous_pet_order);
				}
			}

			if (IsMoving()) {
				StopMoving();
			}

			return;
		}

		//#pragma endregion

				// This causes conflicts with default pet handler (bounces between targets)
		if (NOT_PULLING_BOT && HasPet() && (GetClass() != ENCHANTER || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 2)) {

			// We don't add to hate list here because it's assumed to already be on the list
			GetPet()->SetTarget(tar);
		}

		if (DivineAura()) {
			return;
		}

		if (!(m_PlayerState & static_cast<uint32>(PlayerState::Aggressive))) {
			SendAddPlayerState(PlayerState::Aggressive);
		}

		//#pragma region PULLING FLAG (ACTIONABLE RANGE)

		if (GetPullingFlag()) {

			constexpr size_t PULL_AGGRO = 5225; // spells[5225]: 'Throw Stone' - 0 cast time

			if (tar_distance <= (spells[PULL_AGGRO].range * spells[PULL_AGGRO].range)) {

				StopMoving();
				CastSpell(PULL_AGGRO, tar->GetID());
				return;
			}
		}

		//#pragma endregion

		//#pragma region COMBAT RANGE CALCS

		bool atCombatRange = false;

		const auto* p_item = GetBotItem(EQ::invslot::slotPrimary);
		const auto* s_item = GetBotItem(EQ::invslot::slotSecondary);

		bool behind_mob = false;
		bool backstab_weapon = false;
		if (GetClass() == ROGUE) {

			behind_mob = BehindMob(tar, GetX(), GetY()); // Can be separated for other future use
			backstab_weapon = p_item && p_item->GetItemBackstabDamage();
		}

		// Calculate melee distances
		float melee_distance_max = 0.0f;
		float melee_distance = 0.0f;
		{
			float size_mod = GetSize();
			float other_size_mod = tar->GetSize();

			if (GetRace() == RT_DRAGON || GetRace() == RT_WURM || GetRace() == RT_DRAGON_7) { // For races with a fixed size
				size_mod = 60.0f;
			}
			else if (size_mod < 6.0f) {
				size_mod = 8.0f;
			}

			if (tar->GetRace() == RT_DRAGON || tar->GetRace() == RT_WURM || tar->GetRace() == RT_DRAGON_7) { // For races with a fixed size
				other_size_mod = 60.0f;
			}
			else if (other_size_mod < 6.0f) {
				other_size_mod = 8.0f;
			}

			if (other_size_mod > size_mod) {
				size_mod = other_size_mod;
			}

			if (size_mod > 29.0f) {
				size_mod *= size_mod;
			}
			else if (size_mod > 19.0f) {
				size_mod *= (size_mod * 2.0f);
			}
			else {
				size_mod *= (size_mod * 4.0f);
			}

			// Prevention of ridiculously sized hit boxes
			if (size_mod > 10000.0f) {
				size_mod = (size_mod / 7.0f);
			}

			melee_distance_max = size_mod;

			switch (GetClass()) {
			case WARRIOR:
			case PALADIN:
			case SHADOWKNIGHT:
				if (p_item && p_item->GetItem()->IsType2HWeapon()) {
					melee_distance = melee_distance_max * 0.45f;
				}
				else if ((s_item && s_item->GetItem()->IsTypeShield()) || (!p_item && !s_item)) {
					melee_distance = melee_distance_max * 0.35f;
				}
				else {
					melee_distance = melee_distance_max * 0.40f;
				}

				break;
			case NECROMANCER:
			case WIZARD:
			case MAGICIAN:
			case ENCHANTER:
				if (p_item && p_item->GetItem()->IsType2HWeapon()) {
					melee_distance = melee_distance_max * 0.95f;
				}
				else {
					melee_distance = melee_distance_max * 0.75f;
				}

				break;
			case ROGUE:
				if (behind_mob && backstab_weapon) {
					if (p_item->GetItem()->IsType2HWeapon()) { // 'p_item' tested in 'backstab_weapon' check above
						melee_distance = melee_distance_max * 0.30f;
					}
					else {
						melee_distance = melee_distance_max * 0.25f;
					}

					break;
				}
				// Fall-through
			default:
				if (p_item && p_item->GetItem()->IsType2HWeapon()) {
					melee_distance = melee_distance_max * 0.70f;
				}
				else {
					melee_distance = melee_distance_max * 0.50f;
				}

				break;
			}
		}
		float melee_distance_min = melee_distance / 2.0f;

		// Calculate caster distances
		float caster_distance_max = 0.0f;
		float caster_distance_min = 0.0f;
		float caster_distance = 0.0f;
		{
			if (GetLevel() >= GetStopMeleeLevel() && GetClass() >= WARRIOR && GetClass() <= BERSERKER) {
				caster_distance_max = MAX_CASTER_DISTANCE[(GetClass() - 1)];
			}

			if (caster_distance_max) {

				caster_distance_min = melee_distance_max;
				if (caster_distance_max <= caster_distance_min) {
					caster_distance_max = caster_distance_min * 1.25f;
				}

				caster_distance = ((caster_distance_max + caster_distance_min) / 2);
			}
		}

		bool atArcheryRange = IsArcheryRange(tar);

		if (GetRangerAutoWeaponSelect()) {

			bool changeWeapons = false;

			if (atArcheryRange && !IsBotArcher()) {

				SetBotArcherySetting(true);
				changeWeapons = true;
			}
			else if (!atArcheryRange && IsBotArcher()) {

				SetBotArcherySetting(false);
				changeWeapons = true;
			}

			if (changeWeapons) {
				ChangeBotArcherWeapons(IsBotArcher());
			}
		}

		if (IsBotArcher() && atArcheryRange) {
			atCombatRange = true;
		}
		else if (caster_distance_max && tar_distance <= caster_distance_max) {
			atCombatRange = true;
		}
		else if (tar_distance <= melee_distance) {
			atCombatRange = true;
		}

		//#pragma endregion

		//#pragma region ENGAGED AT COMBAT RANGE

				// We can fight
		if (atCombatRange) {

			//if (IsMoving() || GetCombatJitterFlag()) { // StopMoving() needs to be called so that the jitter timer can be reset
			if (IsMoving()) {

				// Since we're using a pseudo-shadowstep for jitter, disregard the combat jitter flag
				//if (!GetCombatJitterFlag()) {
				StopMoving(CalculateHeadingToTarget(tar->GetX(), tar->GetY()));
				//}

				return;
			}

			// Combat 'jitter' code
			// Note: Combat Jitter is disabled until a working movement solution can be found
			if (AI_movement_timer->Check() && (!spellend_timer.Enabled() || GetClass() == BARD)) {

				if (!IsRooted()) {

					if (HasTargetReflection()) {

						if (!tar->IsFeared() && !tar->IsStunned()) {

							if (GetClass() == ROGUE) {

								if (m_evade_timer.Check(false)) { // Attempt to evade

									int timer_duration = (HideReuseTime - GetSkillReuseTime(EQ::skills::SkillHide)) * 1000;
									if (timer_duration < 0) {
										timer_duration = 0;
									}

									m_evade_timer.Start(timer_duration);
									if (zone->random.Int(0, 260) < (int)GetSkill(EQ::skills::SkillHide)) {
										RogueEvade(tar);
									}

									return;
								}
							}
						}
					}
				}
				else {

					if (!IsSitting() && !IsFacingMob(tar)) {

						FaceTarget(tar);
						return;
					}
				}
			}

			if (!IsBotNonSpellFighter() && AI_EngagedCastCheck()) {
				return;
			}

			// Up to this point, GetTarget() has been safe to dereference since the initial
			// TEST_COMBATANTS() call. Due to the chance of the target dying and our pointer
			// being nullified, we need to test it before dereferencing to avoid crashes

			if (IsBotArcher() && ranged_timer.Check(false)) { // Can shoot mezzed, stunned and dead!?

				TEST_COMBATANTS();
				if (GetTarget()->GetHPRatio() <= 99.0f) {
					BotRangedAttack(tar);
				}
			}
			else if (!IsBotArcher() && GetLevel() < GetStopMeleeLevel()) {

				// We can't fight if we don't have a target, are stun/mezzed or dead..
				// Stop attacking if the target is enraged
				TEST_COMBATANTS();
				if (tar->IsEnraged() && !BehindMob(tar, GetX(), GetY())) {
					return;
				}

				// First, special attack per class (kick, backstab etc..)
				TEST_COMBATANTS();
				DoClassAttacks(tar);

				TEST_COMBATANTS();
				if (attack_timer.Check()) { // Process primary weapon attacks

					Attack(tar, EQ::invslot::slotPrimary);

					TEST_COMBATANTS();
					TriggerDefensiveProcs(tar, EQ::invslot::slotPrimary, false);

					TEST_COMBATANTS();
					//TryWeaponProc(p_item, tar, EQ::invslot::slotPrimary);
					TryCombatProcs(p_item, tar, EQ::invslot::slotPrimary);
					// bool tripleSuccess = false;

					TEST_COMBATANTS();
					if (CanThisClassDoubleAttack()) {

						if (CheckBotDoubleAttack()) {
							Attack(tar, EQ::invslot::slotPrimary, true);
						}

						TEST_COMBATANTS();
						if (GetSpecialAbility(SPECATK_TRIPLE) && CheckBotDoubleAttack(true)) {
							// tripleSuccess = true;
							Attack(tar, EQ::invslot::slotPrimary, true);
						}

						TEST_COMBATANTS();
						// quad attack, does this belong here??
						if (GetSpecialAbility(SPECATK_QUAD) && CheckBotDoubleAttack(true)) {
							Attack(tar, EQ::invslot::slotPrimary, true);
						}
					}

					TEST_COMBATANTS();
					// Live AA - Flurry, Rapid Strikes ect (Flurry does not require Triple Attack).
					int32 flurrychance = (aabonuses.FlurryChance + spellbonuses.FlurryChance + itembonuses.FlurryChance);
					if (flurrychance) {

						if (zone->random.Int(0, 100) < flurrychance) {

							MessageString(Chat::NPCFlurry, YOU_FLURRY);
							Attack(tar, EQ::invslot::slotPrimary, false);

							TEST_COMBATANTS();
							Attack(tar, EQ::invslot::slotPrimary, false);
						}
					}

					TEST_COMBATANTS();
					//int32 ExtraAttackChanceBonus = (spellbonuses.ExtraAttackChance + itembonuses.ExtraAttackChance + aabonuses.ExtraAttackChance);
					auto ExtraAttackChanceBonus =
						(spellbonuses.ExtraAttackChance[0] + itembonuses.ExtraAttackChance[0] +
							aabonuses.ExtraAttackChance[0]);
					if (ExtraAttackChanceBonus) {

						if (p_item && p_item->GetItem()->IsType2HWeapon()) {

							if (zone->random.Int(0, 100) < ExtraAttackChanceBonus) {
								Attack(tar, EQ::invslot::slotPrimary, false);
							}
						}
					}
				}

				TEST_COMBATANTS();
				if (attack_dw_timer.Check() && CanThisClassDualWield()) { // Process secondary weapon attacks

					const EQ::ItemData* s_itemdata = nullptr;
					// Can only dual wield without a weapon if you're a monk
					if (s_item || (GetClass() == MONK)) {

						if (s_item) {
							s_itemdata = s_item->GetItem();
						}

						int weapon_type = 0; // No weapon type.
						bool use_fist = true;
						if (s_itemdata) {

							weapon_type = s_itemdata->ItemType;
							use_fist = false;
						}

						if (use_fist || !s_itemdata->IsType2HWeapon()) {

							float DualWieldProbability = 0.0f;

							int32 Ambidexterity = (aabonuses.Ambidexterity + spellbonuses.Ambidexterity + itembonuses.Ambidexterity);
							DualWieldProbability = ((GetSkill(EQ::skills::SkillDualWield) + GetLevel() + Ambidexterity) / 400.0f); // 78.0 max

							int32 DWBonus = (spellbonuses.DualWieldChance + itembonuses.DualWieldChance);
							DualWieldProbability += (DualWieldProbability * float(DWBonus) / 100.0f);

							float random = zone->random.Real(0, 1);
							if (random < DualWieldProbability) { // Max 78% of DW

								Attack(tar, EQ::invslot::slotSecondary);	// Single attack with offhand

								TEST_COMBATANTS();
								TryCombatProcs(s_item, tar, EQ::invslot::slotSecondary);
								//TryWeaponProc(s_item, tar, EQ::invslot::slotSecondary);

								TEST_COMBATANTS();
								if (CanThisClassDoubleAttack() && CheckBotDoubleAttack()) {

									if (tar->GetHP() > -10) {
										Attack(tar, EQ::invslot::slotSecondary);	// Single attack with offhand
									}
								}
							}
						}
					}
				}
			}

			if (GetAppearance() == eaDead) {
				return;
			}
		}

		//#pragma endregion

		//#pragma region ENGAGED NOT AT COMBAT RANGE

		else { // To far away to fight (GetTarget() validity can be iffy below this point - including outer scopes)

			// This code actually gets processed when we are too far away from target and have not engaged yet, too
			if (/*!GetCombatJitterFlag() && */AI_movement_timer->Check() && (!spellend_timer.Enabled() || GetClass() == BARD)) { // Pursue processing

				if (GetTarget() && !IsRooted()) {

					LogAI("Pursuing [{}] while engaged", GetTarget()->GetCleanName());
					Goal = GetTarget()->GetPosition();
					if (DistanceSquared(m_Position, Goal) <= leash_distance) {
						RunTo(Goal.x, Goal.y, Goal.z);
					}
					else {

						WipeHateList();
						SetTarget(nullptr);
						if (HasPet() && (GetClass() != ENCHANTER || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 2)) {

							GetPet()->WipeHateList();
							GetPet()->SetTarget(nullptr);
						}
					}

					return;
				}
				else {

					if (IsMoving()) {
						StopMoving();
					}
					return;
				}
			}

			if (GetTarget() && GetTarget()->IsFeared() && !spellend_timer.Enabled() && AI_think_timer->Check()) {

				if (!IsFacingMob(GetTarget())) {
					FaceTarget(GetTarget());
				}

				// This is a mob that is fleeing either because it has been feared or is low on hitpoints
				AI_PursueCastCheck(); // This appears to always return true..can't trust for success/fail

				return;
			}
		} // End not in combat range

//#pragma endregion

		if (!IsMoving() && !spellend_timer.Enabled()) { // This may actually need work...

			if (GetTarget() && AI_EngagedCastCheck()) {
				BotMeditate(false);
			}
			else if (GetArchetype() == ARCHETYPE_CASTER) {
				BotMeditate(true);
			}

			return;
		}
	}
	else { // Out-of-combat behavior

		SetAttackFlag(false);
		SetAttackingFlag(false);
		if (!bot_owner->GetBotPulling()) {

			SetPullingFlag(false);
			SetReturningFlag(false);
		}

		//#pragma region AUTO DEFEND

				// This is as close as I could get without modifying the aggro mechanics and making it an expensive process...
				// 'class Client' doesn't make use of hate_list...
		if (RuleB(Bots, AllowOwnerOptionAutoDefend) && bot_owner->GetBotOption(Client::booAutoDefend)) {

			if (!m_auto_defend_timer.Enabled()) {

				m_auto_defend_timer.Start(zone->random.Int(250, 1250)); // random timer to simulate 'awareness' (cuts down on scanning overhead)
				return;
			}

			if (m_auto_defend_timer.Check() && bot_owner->GetAggroCount()) {

				if (NOT_HOLDING && NOT_PASSIVE) {

					auto xhaters = bot_owner->GetXTargetAutoMgr();
					if (xhaters && !xhaters->empty()) {

						for (auto hater_iter : xhaters->get_list()) {

							if (!hater_iter.spawn_id) {
								continue;
							}

							if (bot_owner->GetBotPulling() && bot_owner->GetTarget() && hater_iter.spawn_id == bot_owner->GetTarget()->GetID()) {
								continue;
							}

							auto hater = entity_list.GetMob(hater_iter.spawn_id);
							if (hater && !hater->IsMezzed() && DistanceSquared(hater->GetPosition(), bot_owner->GetPosition()) <= leash_distance) {

								// This is roughly equivilent to npc attacking a client pet owner
								AddToHateList(hater, 1);
								SetTarget(hater);
								SetAttackingFlag();
								if (HasPet() && (GetClass() != ENCHANTER || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 2)) {

									GetPet()->AddToHateList(hater, 1);
									GetPet()->SetTarget(hater);
								}

								m_auto_defend_timer.Disable();

								return;
							}
						}
					}
				}
			}
		}

		//#pragma endregion

		SetTarget(nullptr);

		if (HasPet() && (GetClass() != ENCHANTER || GetPet()->GetPetType() != petAnimation || GetAA(aaAnimationEmpathy) >= 1)) {

			GetPet()->WipeHateList();
			GetPet()->SetTarget(nullptr);
		}

		if (m_PlayerState & static_cast<uint32>(PlayerState::Aggressive)) {
			SendRemovePlayerState(PlayerState::Aggressive);
		}

		//#pragma region OK TO IDLE

				// Ok to idle
		if ((NOT_GUARDING && fm_distance <= GetFollowDistance()) || (GUARDING && DistanceSquared(GetPosition(), GetGuardPoint()) <= GetFollowDistance())) {

			if (!IsMoving() && AI_think_timer->Check() && !spellend_timer.Enabled()) {

				if (NOT_PASSIVE) {

					if (!AI_IdleCastCheck() && !IsCasting() && GetClass() != BARD) {
						BotMeditate(true);
					}
				}
				else {

					if (GetClass() != BARD) {
						BotMeditate(true);
					}
				}

				return;
			}
		}

		// Non-engaged movement checks
		if (AI_movement_timer->Check() && (!IsCasting() || GetClass() == BARD)) {

			if (GUARDING) {
				Goal = GetGuardPoint();
			}
			else {
				Goal = follow_mob->GetPosition();
			}
			float destination_distance = DistanceSquared(GetPosition(), Goal);

			if ((!bot_owner->GetBotPulling() || PULLING_BOT) && (destination_distance > GetFollowDistance())) {

				if (!IsRooted()) {

					if (rest_timer.Enabled()) {
						rest_timer.Disable();
					}

					bool running = true;

					if (destination_distance < GetFollowDistance() + BOT_FOLLOW_DISTANCE_WALK) {
						running = false;
					}

					if (running) {
						RunTo(Goal.x, Goal.y, Goal.z);
					}
					else {
						WalkTo(Goal.x, Goal.y, Goal.z);
					}

					return;
				}
			}
			else {

				if (IsMoving()) {

					StopMoving();
					return;
				}
			}
		}

		// Basically, bard bots get a chance to cast idle spells while moving
		if (GetClass() == BARD && IsMoving() && NOT_PASSIVE) {

			if (!spellend_timer.Enabled() && AI_think_timer->Check()) {

				AI_IdleCastCheck();
				return;
			}
		}

		//#pragma endregion

	}

#undef TEST_COMBATANTS
#undef PULLING_BOT
#undef NOT_PULLING_BOT
#undef GUARDING
#undef NOT_GUARDING
#undef HOLDING
#undef NOT_HOLDING
#undef PASSIVE
#undef NOT_PASSIVE
}

// AI Processing for a Bot object's pet if Bot is a member of a raid
void Bot::PetAIProcess_Raid() {
	if (!HasPet() || !GetPet() || !GetPet()->IsNPC())
		return;

	Mob* BotOwner = GetBotOwner();
	NPC* botPet = GetPet()->CastToNPC();
	if (!botPet->GetOwner() || !botPet->GetID() || !botPet->GetOwnerID()) {
		Kill();
		return;
	}

	if (!botPet->IsAIControlled() || botPet->GetAttackTimer().Check(false) || botPet->IsCasting() || !botPet->GetOwner()->IsBot())
		return;

	if (IsEngaged()) {
		if (botPet->IsRooted())
			botPet->SetTarget(hate_list.GetClosestEntOnHateList(botPet));
		else
			botPet->SetTarget(hate_list.GetEntWithMostHateOnList(botPet));

		// Let's check if we have a los with our target.
		// If we don't, our hate_list is wiped.
		// It causes some cpu stress but without it, it was causing the bot/pet to aggro behind wall, floor etc...
		if (!botPet->CheckLosFN(botPet->GetTarget()) || botPet->GetTarget()->IsMezzed() || !botPet->IsAttackAllowed(GetTarget())) {
			botPet->WipeHateList();
			botPet->SetTarget(botPet->GetOwner());
			return;
		}

		botPet->FaceTarget(botPet->GetTarget());
		bool is_combat_range = botPet->CombatRange(botPet->GetTarget());
		// Ok, we're engaged, each class type has a special AI
		// Only melee class will go to melee. Casters and healers will stay behind, following the leader by default.
		// I should probably make the casters staying in place so they can cast..

		// Ok, we 're a melee or any other class lvl<12. Yes, because after it becomes hard to go in melee for casters.. even for bots..
		if (is_combat_range) {
			botPet->GetAIMovementTimer()->Check();
			if (botPet->IsMoving()) {
				botPet->SetHeading(botPet->GetTarget()->GetHeading());
				if (moved) {
					moved = false;
					botPet->SetRunAnimSpeed(0);
				}
			}

			if (!botPet->IsMoving()) {
				float newX = 0;
				float newY = 0;
				float newZ = 0;
				bool petHasAggro = false;
				if (botPet->GetTarget() && botPet->GetTarget()->GetHateTop() && botPet->GetTarget()->GetHateTop() == botPet)
					petHasAggro = true;

				if (botPet->GetClass() == ROGUE && !petHasAggro && !botPet->BehindMob(botPet->GetTarget(), botPet->GetX(), botPet->GetY())) {
					// Move the rogue to behind the mob
					if (botPet->PlotPositionAroundTarget(botPet->GetTarget(), newX, newY, newZ)) {
						botPet->RunTo(newX, newY, newZ);
						return;
					}
				}
				else if (GetTarget() == botPet->GetTarget() && !petHasAggro && !botPet->BehindMob(botPet->GetTarget(), botPet->GetX(), botPet->GetY())) {
					// If the bot owner and the bot are fighting the same mob, then move the pet to the rear arc of the mob
					if (botPet->PlotPositionAroundTarget(botPet->GetTarget(), newX, newY, newZ)) {
						botPet->RunTo(newX, newY, newZ);
						return;
					}
				}
				else if (DistanceSquaredNoZ(botPet->GetPosition(), botPet->GetTarget()->GetPosition()) < botPet->GetTarget()->GetSize()) {
					// Let's try to adjust our melee range so we don't appear to be bunched up
					bool isBehindMob = false;
					bool moveBehindMob = false;
					if (botPet->BehindMob(botPet->GetTarget(), botPet->GetX(), botPet->GetY()))
						isBehindMob = true;

					if (!isBehindMob && !petHasAggro)
						moveBehindMob = true;

					if (botPet->PlotPositionAroundTarget(botPet->GetTarget(), newX, newY, newZ, moveBehindMob)) {
						botPet->RunTo(newX, newY, newZ);
						return;
					}
				}
			}

			// we can't fight if we don't have a target, are stun/mezzed or dead..
			if (botPet->GetTarget() && !botPet->IsStunned() && !botPet->IsMezzed() && (botPet->GetAppearance() != eaDead)) {
				// check the delay on the attack
				if (botPet->GetAttackTimer().Check()) {
					// Stop attacking while we are on a front arc and the target is enraged
					if (!botPet->BehindMob(botPet->GetTarget(), botPet->GetX(), botPet->GetY()) && botPet->GetTarget()->IsEnraged())
						return;

					if (botPet->Attack(GetTarget(), EQ::invslot::slotPrimary))	// try the main hand
						if (botPet->GetTarget()) {
							// We're a pet so we re able to dual attack
							int32 RandRoll = zone->random.Int(0, 99);
							if (botPet->CanThisClassDoubleAttack() && (RandRoll < (botPet->GetLevel() + NPCDualAttackModifier))) {
								if (botPet->Attack(botPet->GetTarget(), EQ::invslot::slotPrimary)) {}
							}
						}

					if (botPet->GetOwner()->IsBot()) {
						int aa_chance = 0;
						int aa_skill = 0;
						// Magician AA
						aa_skill += botPet->GetOwner()->GetAA(aaElementalAlacrity);
						// Necromancer AA
						aa_skill += botPet->GetOwner()->GetAA(aaQuickeningofDeath);
						// Beastlord AA
						aa_skill += botPet->GetOwner()->GetAA(aaWardersAlacrity);
						if (aa_skill >= 1)
							aa_chance += ((aa_skill > 5 ? 5 : aa_skill) * 4);

						if (aa_skill >= 6)
							aa_chance += ((aa_skill - 5 > 3 ? 3 : aa_skill - 5) * 7);

						if (aa_skill >= 9)
							aa_chance += ((aa_skill - 8 > 3 ? 3 : aa_skill - 8) * 3);

						if (aa_skill >= 12)
							aa_chance += ((aa_skill - 11) * 1);


						//aa_chance += botPet->GetOwner()->GetAA(aaCompanionsAlacrity) * 3;

						if (zone->random.Int(1, 100) < aa_chance)
							Flurry(nullptr);
					}

					// Ok now, let's check pet's offhand.
					if (botPet->GetAttackDWTimer().Check() && botPet->GetOwnerID() && botPet->GetOwner() && ((botPet->GetOwner()->GetClass() == MAGICIAN) || (botPet->GetOwner()->GetClass() == NECROMANCER) || (botPet->GetOwner()->GetClass() == SHADOWKNIGHT) || (botPet->GetOwner()->GetClass() == BEASTLORD))) {
						if (botPet->GetOwner()->GetLevel() >= 24) {
							float DualWieldProbability = ((botPet->GetSkill(EQ::skills::SkillDualWield) + botPet->GetLevel()) / 400.0f);
							DualWieldProbability -= zone->random.Real(0, 1);
							if (DualWieldProbability < 0) {
								botPet->Attack(botPet->GetTarget(), EQ::invslot::slotSecondary);
								if (botPet->CanThisClassDoubleAttack()) {
									int32 RandRoll = zone->random.Int(0, 99);
									if (RandRoll < (botPet->GetLevel() + 20))
										botPet->Attack(botPet->GetTarget(), EQ::invslot::slotSecondary);
								}
							}
						}
					}
					if (!botPet->GetOwner())
						return;

					// Special attack
					botPet->DoClassAttacks(botPet->GetTarget());
				}
				// See if the pet can cast any spell
				botPet->AI_EngagedCastCheck();
			}
		}
		else {
			// Now, if we cannot reach our target
			if (!botPet->HateSummon()) {
				if (botPet->GetTarget() && botPet->AI_PursueCastCheck()) {}
				else if (botPet->GetTarget() && botPet->GetAIMovementTimer()->Check()) {
					botPet->SetRunAnimSpeed(0);
					if (!botPet->IsRooted()) {
						LogAI("Pursuing [{}] while engaged", botPet->GetTarget()->GetCleanName());
						botPet->RunTo(botPet->GetTarget()->GetX(), botPet->GetTarget()->GetY(), botPet->GetTarget()->GetZ());
						return;
					}
					else {
						botPet->SetHeading(botPet->GetTarget()->GetHeading());
						if (moved) {
							moved = false;
							StopNavigation();
							botPet->StopNavigation();
						}
					}
				}
			}
		}
	}
	else {
		// Ok if we're not engaged, what's happening..
		if (botPet->GetTarget() != botPet->GetOwner())
			botPet->SetTarget(botPet->GetOwner());

		if (!IsMoving())
			botPet->AI_IdleCastCheck();

		if (botPet->GetAIMovementTimer()->Check()) {
			switch (pStandingPetOrder) {
			case SPO_Follow: {
				float dist = DistanceSquared(botPet->GetPosition(), botPet->GetTarget()->GetPosition());
				botPet->SetRunAnimSpeed(0);
				if (dist > 184) {
					botPet->RunTo(botPet->GetTarget()->GetX(), botPet->GetTarget()->GetY(), botPet->GetTarget()->GetZ());
					return;
				}
				else {
					botPet->SetHeading(botPet->GetTarget()->GetHeading());
					if (moved) {
						moved = false;
						StopNavigation();
						botPet->StopNavigation();
					}
				}
				break;
			}
			case SPO_Sit:
				botPet->SetAppearance(eaSitting);
				break;
			case SPO_Guard:
				botPet->NextGuardPosition();
				break;
			}
		}
	}
}

std::vector<RaidMember> Raid::GetRaidGroupMembers(uint32 gid) 
{
	std::vector<RaidMember> raid_group_members;
	raid_group_members.clear();

	for (int i = 0; i < MAX_RAID_MEMBERS; ++i)
	{
		if (members[i].member && members[i].GroupNumber == gid)
		{
			raid_group_members.push_back(members[i]);
		}
	}
	return raid_group_members;
}

// Returns Bot members that are in the raid
// passing in owner will return Bots that have the same owner.
std::vector<Bot*> Raid::GetRaidBotMembers(uint32 owner)
{
	std::vector<Bot*> raid_members_bots;
	raid_members_bots.clear();

	for (int i = 0; i < MAX_RAID_MEMBERS; i++) {
		if (
			members[i].member &&
			members[i].member->IsBot()
			) {
			auto b_member = members[i].member->CastToBot();
			if (owner && b_member->GetBotOwnerCharacterID() == owner) {
				raid_members_bots.emplace_back(b_member);
			} else if (!owner) {
				raid_members_bots.emplace_back(b_member);
			}
		}
	}

	return raid_members_bots;
}

// Returns Bot members that are in the group specified
// passing in owner will return only Bots that have the same owner.
std::vector<Bot*> Raid::GetRaidGroupBotMembers(uint32 gid)
{
	std::vector<Bot*> raid_members_bots;
	raid_members_bots.clear();

	for (int i = 0; i < MAX_RAID_MEMBERS; i++) {
		if (
			members[i].member &&
			members[i].member->IsBot() &&
			members[i].GroupNumber == gid
		) {
			auto b_member = members[i].member->CastToBot();
			raid_members_bots.emplace_back(b_member);
			raid_members_bots.emplace_back(b_member);
		}
	}

	return raid_members_bots;
}

void Raid::HandleBotGroupDisband(uint32 owner, uint32 gid)
{
	auto raid_members_bots = gid != RAID_GROUPLESS ? GetRaidGroupBotMembers(gid) : GetRaidBotMembers(owner);

	// If any of the bots are a group leader then re-create the botgroup on disband, dropping any clients
	for (auto& bot_iter: raid_members_bots) {

		// Remove the entire BOT group in this case
		if (
			bot_iter &&
			gid != RAID_GROUPLESS &&
			IsRaidMember(bot_iter->GetName()) &&
			IsGroupLeader(bot_iter->GetName())
			) {
			auto r_group_members = GetRaidGroupMembers(GetGroup(bot_iter->GetName()));
			auto group_inst = new Group(bot_iter);
			entity_list.AddGroup(group_inst);
			database.SetGroupID(bot_iter->GetCleanName(), group_inst->GetID(), bot_iter->GetBotID());
			database.SetGroupLeaderName(group_inst->GetID(), bot_iter->GetName());

			for (auto member_iter: r_group_members) {
				if (member_iter.member->IsBot()) {
					auto b_member = member_iter.member->CastToBot();
					if (strcmp(b_member->GetName(), bot_iter->GetName()) == 0) {
						bot_iter->SetFollowID(owner);
					} else {
						Bot::AddBotToGroup(b_member, group_inst);
					}
					Bot::RemoveBotFromRaid(b_member);
				}
			}
		} else {
			Bot::RemoveBotFromRaid(bot_iter);
		}
	}
}

uint8 Bot::GetNumberNeedingHealedInRaidGroup(uint8& need_healed, uint8 hpr, bool includePets, Raid* raid) {

	if (raid) {
		uint32 r_group = raid->GetGroup(GetName());
		auto raid_group_members = raid->GetRaidGroupMembers(r_group);

		for (auto& m: raid_group_members) {
			if (m.member && !m.member->qglobal) {
				if (m.member->GetHPRatio() <= hpr) {
					need_healed++;
				}

				if (includePets) {
					if (m.member->GetPet() && m.member->GetPet()->GetHPRatio() <= hpr) {
						need_healed++;
					}
				}
			}
		}
	}
	return need_healed;
}

void Bot::ProcessRaidInvite(Mob* invitee, Client* invitor, bool group_invite) {

	if (!invitee || !invitor) {
		return;
	}

	if (invitee->IsBot() && invitee->CastToBot()->GetBotOwnerCharacterID() != invitor->CharacterID()) {
		invitor->Message(
			Chat::Red,
			fmt::format(
				"{} is not your Bot. You can only invite your Bots, or players grouped with bots.",
				invitee->GetCleanName()
			).c_str()
		);
		return;
	}

	Raid* raid = entity_list.GetRaidByClient(invitor);

	Bot::CreateBotRaid(invitee, invitor, group_invite, raid);
}

void Bot::CreateBotRaid(Mob* invitee, Client* invitor, bool group_invite, Raid* raid) {

	Group* g_invitee = invitee->GetGroup();
	Group* g_invitor = invitor->GetGroup();

	if (g_invitee && invitor->IsClient()) {
		if (!g_invitee->IsLeader(invitee)) {
			invitor->Message(
				Chat::Red,
				fmt::format(
					"You can only invite group leaders or ungrouped bots. Try {} instead.",
					g_invitee->GetLeader()->GetCleanName()
				).c_str()
			);
			return;
		}
	}

	bool new_raid = false;
	if (!raid) {
		new_raid = true;
		raid = new Raid(invitor);
		entity_list.AddRaid(raid);
		raid->SetRaidDetails();
	}

	// Add Invitor to new raid
	if (new_raid) {
		if (g_invitor) {
			ProcessBotGroupAdd(g_invitor, raid, true);
		} else {
			raid->SendRaidCreate(invitor);
			raid->AddMember(invitor, 0xFFFFFFFF, true, false, true);
			raid->SendMakeLeaderPacketTo(invitor->GetName(), invitor);
			if (raid->IsLocked()) {
				raid->SendRaidLockTo(invitor);
			}
		}
	}

	// Add Bot Group, or Individual Bot to Raid
	if (invitee->IsBot()) {
		if (g_invitee) {
			ProcessBotGroupAdd(g_invitee, raid);
		} else {
			auto gid = raid->GetGroup(invitor->GetName());
			auto b = invitee->CastToBot();

			// gives us a choice to either invite directly into the clients Raid Group, or just into the Raid
			if (group_invite && raid->GroupCount(gid) < MAX_GROUP_MEMBERS) {
				raid->AddBot(b, gid);
			} else {
				raid->AddBot(b);
			}

			if (new_raid) {
				invitee->SetFollowID(invitor->GetID());
			}
		}
	}
	// Add Client invitee if they have Bots in their group
	else if (g_invitee && invitee->IsClient()) {
		ProcessBotGroupAdd(g_invitee, raid);
	}
}

void Bot::ProcessBotGroupAdd(Group *group, Raid *raid, bool new_raid) {

	uint32 raid_free_group_id = raid->GetFreeGroup();
	for (int x = 0; x < MAX_GROUP_MEMBERS; x++) {
		if (group->members[x]) {
			Client* c = nullptr;
			Bot* b = nullptr;

			if (group->members[x] && group->members[x]->IsBot()) {
				b = group->members[x]->CastToBot();
				raid->AddBot(b, raid_free_group_id, false, x == 0, false);
			}

			else if (group->members[x] && group->members[x]->IsClient()) {
				c = group->members[x]->CastToClient();
				raid->SendRaidCreate(c);
				raid->AddMember(
					c,
					raid_free_group_id,
					new_raid,
					x == 0,
					false
				);
				raid->SendMakeLeaderPacketTo(raid->leadername, c);
				raid->SendBulkRaid(c);
			}
		}
	}

	group->JoinRaidXTarget(raid);
	group->DisbandGroup(true);
	raid->GroupUpdate(raid_free_group_id);
}


