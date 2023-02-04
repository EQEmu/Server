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

				Bot::BotGroupSay(this, "Pulling %s to the group..", pull_target->GetCleanName());
				//raid->RaidBotGroupSay(this, 0, 100, "Pulling %s to the group..", pull_target->GetCleanName());
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

					Mob* bg_member = iter->member;// bot_group->members[counter];
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

bool Bot::AICastSpell_Raid(Mob* tar, uint8 iChance, uint32 iSpellTypes) {

	// Bot AI Raid

	Raid* raid = entity_list.GetRaidByBotName(GetName());
	uint32 r_group = raid->GetGroup(GetName());
	if (!raid)
		return false;

	if (!tar) {
		return false;
	}

	if (!AI_HasSpells())
		return false;

	if (iChance < 100) {
		if (zone->random.Int(0, 100) > iChance) {
			return false;
		}
	}

	if (tar->GetAppearance() == eaDead) {
		if ((tar->IsClient() && tar->CastToClient()->GetFeigned()) || tar->IsBot()) {
			// do nothing
		}
		else {
			return false;
		}
	}

	uint8 botClass = GetClass();
	uint8 botLevel = GetLevel();

	bool checked_los = false;	//we do not check LOS until we are absolutely sure we need to, and we only do it once.

	bool castedSpell = false;

	BotSpell botSpell;
	botSpell.SpellId = 0;
	botSpell.SpellIndex = 0;
	botSpell.ManaCost = 0;

	switch (iSpellTypes) {
	case SpellType_Mez: {
		if (tar->GetBodyType() != BT_Giant) {
			if (!checked_los) {
				if (!CheckLosFN(tar))
					break;	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call

				checked_los = true;
			}

			//TODO
			//Check if single target or AoE mez is best
			//if (TARGETS ON MT IS => 3 THEN botSpell = AoEMez)
			//if (TARGETS ON MT IS <= 2 THEN botSpell = BestMez)

			botSpell = GetBestBotSpellForMez(this);

			if (botSpell.SpellId == 0)
				break;

			Mob* addMob = GetFirstIncomingMobToMez(this, botSpell);

			if (!addMob) {
				//Say("!addMob.");
				break;
			}

			if (!(!addMob->IsImmuneToSpell(botSpell.SpellId, this) && addMob->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0))
				break;

			castedSpell = AIDoSpellCast(botSpell.SpellIndex, addMob, botSpell.ManaCost);

			if (castedSpell)
				BotGroupSay(this, "Attempting to mez %s.", addMob->GetCleanName());
				//raid->RaidBotGroupSay(this, 0, 100, "Attempting to mez %s.", addMob->GetCleanName());
		}
		break;
	}
	case SpellType_Heal: {
		if (tar->DontHealMeBefore() < Timer::GetCurrentTime()) {
			uint8 hpr = (uint8)tar->GetHPRatio();
			bool hasAggro = false;
			bool isPrimaryHealer = false;

			if (IsRaidGrouped()) {
				isPrimaryHealer = IsGroupHealer();
			}

			if (hpr < 95 || (tar->IsClient() && (hpr < 95)) || (botClass == BARD)) {
				if (tar->GetClass() == NECROMANCER) {
					// Give necromancers a chance to go lifetap something or cleric can spend too much mana on a necro
					if (hpr >= 40) {
						break;
					}
				}

				if (tar->GetClass() == SHAMAN) {
					// Give shaman the chance to canni without wasting the cleric's mana
					if (hpr >= 80) {
						break;
					}
				}

				// Evaluate the situation
				if ((IsEngaged()) && ((botClass == CLERIC) || (botClass == DRUID) || (botClass == SHAMAN) || (botClass == PALADIN))) {
					if (tar->GetTarget() && tar->GetTarget()->GetHateTop() && tar->GetTarget()->GetHateTop() == tar) {
						hasAggro = true;
					}

					if (hpr < 35) {
						botSpell = GetBestBotSpellForFastHeal(this);
					}
					else if (hpr >= 35 && hpr < 70) {
						if (GetNumberNeedingHealedInRaidGroup(60, false) >= 3)
							botSpell = GetBestBotSpellForGroupHeal(this);

						if (botSpell.SpellId == 0)
							botSpell = GetBestBotSpellForPercentageHeal(this);
					}
					else if (hpr >= 70 && hpr < 95) {
						if (GetNumberNeedingHealedInRaidGroup(80, false) >= 3)
							botSpell = GetBestBotSpellForGroupHealOverTime(this);

						if (hasAggro)
							botSpell = GetBestBotSpellForPercentageHeal(this);
					}
					else {
						if (!tar->FindType(SE_HealOverTime))
							botSpell = GetBestBotSpellForHealOverTime(this);
					}
				}
				else if ((botClass == CLERIC) || (botClass == DRUID) || (botClass == SHAMAN) || (botClass == PALADIN)) {
					if (GetNumberNeedingHealedInRaidGroup(40, true) >= 2) {
						botSpell = GetBestBotSpellForGroupCompleteHeal(this);

						if (botSpell.SpellId == 0)
							botSpell = GetBestBotSpellForGroupHeal(this);

						if (botSpell.SpellId == 0)
							botSpell = GetBestBotSpellForGroupHealOverTime(this);

						if (hpr < 40) {
							if (botSpell.SpellId == 0)
								botSpell = GetBestBotSpellForPercentageHeal(this);
						}
					}
					else if (GetNumberNeedingHealedInRaidGroup(60, true) >= 2) {
						botSpell = GetBestBotSpellForGroupHeal(this);

						if (botSpell.SpellId == 0)
							botSpell = GetBestBotSpellForGroupHealOverTime(this);

						if (hpr < 40) {
							if (botSpell.SpellId == 0)
								botSpell = GetBestBotSpellForPercentageHeal(this);
						}
					}
					else if (hpr < 40)
						botSpell = GetBestBotSpellForPercentageHeal(this);
					else if (hpr >= 40 && hpr < 75)
						botSpell = GetBestBotSpellForRegularSingleTargetHeal(this);
					else {
						if (hpr < 90 && !tar->FindType(SE_HealOverTime))
							botSpell = GetBestBotSpellForHealOverTime(this);
					}
				}
				else {
					float hpRatioToCast = 0.0f;

					switch (GetBotStance()) {
					case EQ::constants::stanceEfficient:
					case EQ::constants::stanceAggressive:
						hpRatioToCast = isPrimaryHealer ? 90.0f : 50.0f;
						break;
					case EQ::constants::stanceBalanced:
						hpRatioToCast = isPrimaryHealer ? 95.0f : 75.0f;
						break;
					case EQ::constants::stanceReactive:
						hpRatioToCast = isPrimaryHealer ? 100.0f : 90.0f;
						break;
					case EQ::constants::stanceBurn:
					case EQ::constants::stanceBurnAE:
						hpRatioToCast = isPrimaryHealer ? 75.0f : 25.0f;
						break;
					default:
						hpRatioToCast = isPrimaryHealer ? 100.0f : 0.0f;
						break;
					}

					//If we're at specified mana % or below, don't heal as hybrid
					if (tar->GetHPRatio() <= hpRatioToCast)
						botSpell = GetBestBotSpellForRegularSingleTargetHeal(this);
				}

				if (botSpell.SpellId == 0)
					botSpell = GetBestBotSpellForRegularSingleTargetHeal(this);

				if (botSpell.SpellId == 0)
					botSpell = GetFirstBotSpellForSingleTargetHeal(this);

				if (botSpell.SpellId == 0 && botClass == BARD) {
					botSpell = GetFirstBotSpellBySpellType(this, SpellType_Heal);
				}

				// If there is still no spell id, then there isn't going to be one so we are done
				if (botSpell.SpellId == 0)
					break;

				// Can we cast this spell on this target?
				if (!(spells[botSpell.SpellId].target_type == ST_GroupTeleport || spells[botSpell.SpellId].target_type == ST_Target || tar == this)
					&& !(tar->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0))
					break;

				uint32 TempDontHealMeBeforeTime = tar->DontHealMeBefore();

				castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost, &TempDontHealMeBeforeTime);

				if (castedSpell) {
					/*if(TempDontHealMeBeforeTime != tar->DontHealMeBefore())
						tar->SetDontHealMeBefore(TempDontHealMeBeforeTime);

					// For non-HoT heals, do a 4 second delay
					// TODO: Replace this code with logic that calculates the delay based on number of clerics in rotation
					//			and ignores heals for anyone except the main tank
					if(!IsHealOverTimeSpell(botSpell.SpellId)) {
						if(IsCompleteHealSpell(botSpell.SpellId)) {
							// Complete Heal 4 second rotation
							tar->SetDontHealMeBefore(Timer::GetCurrentTime() + 4000);
						}
						else {
							tar->SetDontHealMeBefore(Timer::GetCurrentTime() + 1000);
						}
					}*/
					if (botClass != BARD) {
						if (IsGroupSpell(botSpell.SpellId)) {
							if (IsRaidGrouped()) {
								uint32 r_group = raid->GetGroup(GetName());
								BotGroupSay(this, "Casting %s.", spells[botSpell.SpellId].name);
								//raid->RaidBotGroupSay(this, 0, 100, "Casting %s.", spells[botSpell.SpellId].name);
								std::vector<RaidMember> raid_group_members = raid->GetRaidGroupMembers(r_group);
								//for (std::vector<RaidMember>::iterator iter = raid_group_members.begin(); iter != raid_group_members.end(); ++iter) {
								for (int i = 0; i < raid_group_members.size(); ++i){
									if (raid_group_members.at(i).member && !raid_group_members.at(i).member->qglobal) {
										raid_group_members.at(i).member->SetDontHealMeBefore(Timer::GetCurrentTime() + 1000);
									}
								}
							}
						}
						else {
							if (tar != this)		//we don't need spam of bots healing themselves
								BotGroupSay(this, "Casting %s on %s", spells[botSpell.SpellId].name, tar->GetCleanName());
							tar->SetDontHealMeBefore(Timer::GetCurrentTime() + 2000);
						}
					}
				}
			}
		}
		break;
	}
	case SpellType_Root: {
		if (!tar->IsRooted() && tar->DontRootMeBefore() < Timer::GetCurrentTime()) {
			if (!checked_los) {
				if (!CheckLosFN(tar))
					break;	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call

				checked_los = true;
			}

			// TODO: If there is a ranger in the group then don't allow root spells

			botSpell = GetFirstBotSpellBySpellType(this, iSpellTypes);

			if (botSpell.SpellId == 0)
				break;

			if (tar->CanBuffStack(botSpell.SpellId, botLevel, true) == 0)
				break;

			uint32 TempDontRootMeBefore = tar->DontRootMeBefore();

			castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost, &TempDontRootMeBefore);

			if (TempDontRootMeBefore != tar->DontRootMeBefore())
				tar->SetDontRootMeBefore(TempDontRootMeBefore);
		}
		break;
	}
	case SpellType_Buff: {
		if (tar->DontBuffMeBefore() < Timer::GetCurrentTime()) {
			std::list<BotSpell> buffSpellList = GetBotSpellsBySpellType(this, SpellType_Buff);

			for (std::list<BotSpell>::iterator itr = buffSpellList.begin(); itr != buffSpellList.end(); ++itr) {
				BotSpell selectedBotSpell = *itr;

				if (selectedBotSpell.SpellId == 0)
					continue;

				// no buffs with illusions.. use #bot command to cast illusions
				if (IsEffectInSpell(selectedBotSpell.SpellId, SE_Illusion) && tar != this)
					continue;

				//no teleport spells use #bot command to cast teleports
				if (IsEffectInSpell(selectedBotSpell.SpellId, SE_Teleport) || IsEffectInSpell(selectedBotSpell.SpellId, SE_Succor))
					continue;

				// can not cast buffs for your own pet only on another pet that isn't yours
				if ((spells[selectedBotSpell.SpellId].target_type == ST_Pet) && (tar != GetPet()))
					continue;

				// Validate target

				if (!((spells[selectedBotSpell.SpellId].target_type == ST_Target || spells[selectedBotSpell.SpellId].target_type == ST_Pet || tar == this ||
					spells[selectedBotSpell.SpellId].target_type == ST_Group || spells[selectedBotSpell.SpellId].target_type == ST_GroupTeleport ||
					(botClass == BARD && spells[selectedBotSpell.SpellId].target_type == ST_AEBard))
					&& !tar->IsImmuneToSpell(selectedBotSpell.SpellId, this)
					&& (tar->CanBuffStack(selectedBotSpell.SpellId, botLevel, true) >= 0))) {
					continue;
				}

				// Put the zone levitate and movement check here since bots are able to bypass the client casting check
				if ((IsEffectInSpell(selectedBotSpell.SpellId, SE_Levitate) && !zone->CanLevitate())
					|| (IsEffectInSpell(selectedBotSpell.SpellId, SE_MovementSpeed) && !zone->CanCastOutdoor())) {
					if (botClass != BARD || !IsSpellUsableThisZoneType(selectedBotSpell.SpellId, zone->GetZoneType())) {
						continue;
					}
				}

				switch (tar->GetArchetype())
				{
				case ARCHETYPE_CASTER:
					//TODO: probably more caster specific spell effects in here
					if (IsEffectInSpell(selectedBotSpell.SpellId, SE_AttackSpeed) || IsEffectInSpell(selectedBotSpell.SpellId, SE_ATK) ||
						IsEffectInSpell(selectedBotSpell.SpellId, SE_STR) || IsEffectInSpell(selectedBotSpell.SpellId, SE_ReverseDS))
					{
						continue;
					}
					break;
				case ARCHETYPE_MELEE:
					if (IsEffectInSpell(selectedBotSpell.SpellId, SE_IncreaseSpellHaste) || IsEffectInSpell(selectedBotSpell.SpellId, SE_ManaPool) ||
						IsEffectInSpell(selectedBotSpell.SpellId, SE_CastingLevel) || IsEffectInSpell(selectedBotSpell.SpellId, SE_ManaRegen_v2) ||
						IsEffectInSpell(selectedBotSpell.SpellId, SE_CurrentMana))
					{
						continue;
					}
					break;
				case ARCHETYPE_HYBRID:
					//Hybrids get all buffs
				default:
					break;
				}

				if (botClass == ENCHANTER && IsEffectInSpell(selectedBotSpell.SpellId, SE_Rune))
				{
					float manaRatioToCast = 75.0f;

					switch (GetBotStance()) {
					case EQ::constants::stanceEfficient:
						manaRatioToCast = 90.0f;
						break;
					case EQ::constants::stanceBalanced:
					case EQ::constants::stanceAggressive:
						manaRatioToCast = 75.0f;
						break;
					case EQ::constants::stanceReactive:
					case EQ::constants::stanceBurn:
					case EQ::constants::stanceBurnAE:
						manaRatioToCast = 50.0f;
						break;
					default:
						manaRatioToCast = 75.0f;
						break;
					}

					//If we're at specified mana % or below, don't rune as enchanter
					if (GetManaRatio() <= manaRatioToCast)
						break;
				}

				if (CheckSpellRecastTimers(this, itr->SpellIndex))
				{

					uint32 TempDontBuffMeBefore = tar->DontBuffMeBefore();

					castedSpell = AIDoSpellCast(selectedBotSpell.SpellIndex, tar, selectedBotSpell.ManaCost, &TempDontBuffMeBefore);

					if (TempDontBuffMeBefore != tar->DontBuffMeBefore())
						tar->SetDontBuffMeBefore(TempDontBuffMeBefore);
				}

				if (castedSpell)
					break;
			}
		}
		break;
	}
	case SpellType_Escape: {
		uint8 hpr = (uint8)GetHPRatio();
		bool mayGetAggro = false;

#ifdef IPC
		if (hpr <= 5 || (IsNPC() && CastToNPC()->IsInteractive() && tar != this))
#else
		if (hpr > 15 && ((botClass == WIZARD) || (botClass == ENCHANTER) || (botClass == RANGER)))
			mayGetAggro = HasOrMayGetAggro(); //classes have hate reducing spells

		if (hpr <= 15 || mayGetAggro)
#endif
		{
			botSpell = GetFirstBotSpellBySpellType(this, iSpellTypes);

			if (botSpell.SpellId == 0)
				break;

			if (IsInvulnerabilitySpell(botSpell.SpellId))
				tar = this; //target self for invul type spells

			castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost);
		}
		break;
	}
	case SpellType_Nuke: {
		if ((tar->GetHPRatio() <= 95.0f) || ((botClass == BARD) || (botClass == SHAMAN) || (botClass == ENCHANTER)))
		{
			if (!checked_los) {
				if (!CheckLosFN(tar))
					break;	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call

				checked_los = true;
			}

			if (botClass == CLERIC || botClass == ENCHANTER)
			{
				float manaRatioToCast = 75.0f;

				switch (GetBotStance()) {
				case EQ::constants::stanceEfficient:
					manaRatioToCast = 90.0f;
					break;
				case EQ::constants::stanceBalanced:
					manaRatioToCast = 75.0f;
					break;
				case EQ::constants::stanceReactive:
				case EQ::constants::stanceAggressive:
					manaRatioToCast = 50.0f;
					break;
				case EQ::constants::stanceBurn:
				case EQ::constants::stanceBurnAE:
					manaRatioToCast = 25.0f;
					break;
				default:
					manaRatioToCast = 50.0f;
					break;
				}

				//If we're at specified mana % or below, don't nuke as cleric or enchanter
				if (GetManaRatio() <= manaRatioToCast)
					break;
			}

			if (botClass == MAGICIAN || botClass == SHADOWKNIGHT || botClass == NECROMANCER || botClass == PALADIN || botClass == RANGER || botClass == DRUID || botClass == CLERIC) {
				if (tar->GetBodyType() == BT_Undead || tar->GetBodyType() == BT_SummonedUndead || tar->GetBodyType() == BT_Vampire)
					botSpell = GetBestBotSpellForNukeByTargetType(this, ST_Undead);
				else if (tar->GetBodyType() == BT_Summoned || tar->GetBodyType() == BT_Summoned2 || tar->GetBodyType() == BT_Summoned3)
					botSpell = GetBestBotSpellForNukeByTargetType(this, ST_Summoned);
			}

			if (botClass == PALADIN || botClass == DRUID || botClass == CLERIC || botClass == ENCHANTER || botClass == WIZARD) {
				if (botSpell.SpellId == 0) {
					uint8 stunChance = (tar->IsCasting() ? 30 : 15);

					if (botClass == PALADIN)
						stunChance = 50;

					if (!tar->GetSpecialAbility(UNSTUNABLE) && !tar->IsStunned() && (zone->random.Int(1, 100) <= stunChance)) {
						botSpell = GetBestBotSpellForStunByTargetType(this, ST_Target);
					}
				}
			}

			if (botClass == WIZARD && botSpell.SpellId == 0) {
				botSpell = GetBestBotWizardNukeSpellByTargetResists(this, tar);
			}

			if (botSpell.SpellId == 0)
				botSpell = GetBestBotSpellForNukeByTargetType(this, ST_Target);

			if (botSpell.SpellId == 0)
				break;

			if (!(!tar->IsImmuneToSpell(botSpell.SpellId, this) && (tar->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0)))
				break;

			if (IsFearSpell(botSpell.SpellId)) {
				// don't let fear cast if the npc isn't snared or rooted
				if (tar->GetSnaredAmount() == -1) {
					if (!tar->IsRooted())
						break;
				}
			}

			castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost);
		}
		break;
	}
	case SpellType_Dispel: {
		if (tar->GetHPRatio() > 95.0f) {
			if (!checked_los) {
				if (!CheckLosFN(tar))
					break;	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call

				checked_los = true;
			}

			botSpell = GetFirstBotSpellBySpellType(this, iSpellTypes);

			if (botSpell.SpellId == 0)
				break;

			// TODO: Check target to see if there is anything to dispel

			if (tar->CountDispellableBuffs() > 0) {
				castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost);
			}
		}
		break;
	}
	case SpellType_Pet: {
		//keep mobs from recasting pets when they have them.
		if (!IsPet() && !GetPetID() && !IsBotCharmer()) {
			if (botClass == WIZARD) {
				auto buffs_max = GetMaxBuffSlots();
				auto my_buffs = GetBuffs();
				int familiar_buff_slot = -1;
				if (buffs_max && my_buffs) {
					for (int index = 0; index < buffs_max; ++index) {
						if (IsEffectInSpell(my_buffs[index].spellid, SE_Familiar)) {
							MakePet(my_buffs[index].spellid, spells[my_buffs[index].spellid].teleport_zone);
							familiar_buff_slot = index;
							break;
						}
					}
				}
				if (GetPetID())
					break;

				if (familiar_buff_slot >= 0) {
					BuffFadeBySlot(familiar_buff_slot);
					break;
				}

				botSpell = GetFirstBotSpellBySpellType(this, SpellType_Pet);
			}
			else if (botClass == MAGICIAN) {
				botSpell = GetBestBotMagicianPetSpell(this);
			}
			else {
				botSpell = GetFirstBotSpellBySpellType(this, SpellType_Pet);
			}

			if (botSpell.SpellId == 0)
				break;

			castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost);
		}
		break;
	}
	case SpellType_InCombatBuff: {

		if (botClass == SHAMAN) {
			checked_los = true;

			std::list<BotSpell> inCombatBuffList = GetBotSpellsBySpellType(this, SpellType_InCombatBuff);

			for (std::list<BotSpell>::iterator itr = inCombatBuffList.begin(); itr != inCombatBuffList.end(); ++itr) {
				BotSpell selectedBotSpell = *itr;

				if (selectedBotSpell.SpellId == 0)
					continue;

				if (CheckSpellRecastTimers(this, itr->SpellIndex))
				{
					if (!(!tar->IsImmuneToSpell(selectedBotSpell.SpellId, this) && (spells[selectedBotSpell.SpellId].buff_duration < 1 || tar->CanBuffStack(selectedBotSpell.SpellId, botLevel, true) >= 0)))
						continue;

					//short duration buffs or other buffs only to be cast during combat.
					if (IsSelfConversionSpell(selectedBotSpell.SpellId)) {
						if (GetManaRatio() > 90.0f || GetHPRatio() < 50.0f || GetHPRatio() < (GetManaRatio() + 10.0f))
							break; //don't cast if low hp, lots of mana, or if mana is higher than hps
					}

					castedSpell = AIDoSpellCast(selectedBotSpell.SpellIndex, tar, selectedBotSpell.ManaCost);
				}

				if (castedSpell)
					break;
			}
		}
		else if (botClass == BARD) {
			if (tar->DontBuffMeBefore() < Timer::GetCurrentTime()) {
				std::list<BotSpell> inCombatBuffList = GetBotSpellsBySpellType(this, SpellType_InCombatBuff);

				for (std::list<BotSpell>::iterator itr = inCombatBuffList.begin(); itr != inCombatBuffList.end(); ++itr) {
					BotSpell selectedBotSpell = *itr;

					if (selectedBotSpell.SpellId == 0)
						continue;

					if (CheckSpellRecastTimers(this, itr->SpellIndex)) {
						uint32 TempDontBuffMeBefore = tar->DontBuffMeBefore();

						// no buffs with illusions.. use #bot command to cast illusions
						if (IsEffectInSpell(selectedBotSpell.SpellId, SE_Illusion) && tar != this)
							continue;

						//no teleport spells use #bot command to cast teleports
						if (IsEffectInSpell(selectedBotSpell.SpellId, SE_Teleport) || IsEffectInSpell(selectedBotSpell.SpellId, SE_Succor))
							continue;

						// can not cast buffs for your own pet only on another pet that isn't yours
						if ((spells[selectedBotSpell.SpellId].target_type == ST_Pet) && (tar != GetPet()))
							continue;

						// Validate target

						if (!((spells[selectedBotSpell.SpellId].target_type == ST_Target || spells[selectedBotSpell.SpellId].target_type == ST_Pet || tar == this ||
							spells[selectedBotSpell.SpellId].target_type == ST_Group || spells[selectedBotSpell.SpellId].target_type == ST_GroupTeleport ||
							(botClass == BARD && spells[selectedBotSpell.SpellId].target_type == ST_AEBard))
							&& !tar->IsImmuneToSpell(selectedBotSpell.SpellId, this)
							&& (tar->CanBuffStack(selectedBotSpell.SpellId, botLevel, true) >= 0))) {
							continue;
						}

						// Put the zone levitate and movement check here since bots are able to bypass the client casting check
						if ((IsEffectInSpell(selectedBotSpell.SpellId, SE_Levitate) && !zone->CanLevitate())
							|| (IsEffectInSpell(selectedBotSpell.SpellId, SE_MovementSpeed) && !zone->CanCastOutdoor())) {
							if (!IsSpellUsableThisZoneType(selectedBotSpell.SpellId, zone->GetZoneType())) {
								continue;
							}
						}

						if (!IsGroupSpell(selectedBotSpell.SpellId)) {
							//Only check archetype if song is not a group spell
							switch (tar->GetArchetype()) {
							case ARCHETYPE_CASTER:
								//TODO: probably more caster specific spell effects in here
								if (IsEffectInSpell(selectedBotSpell.SpellId, SE_AttackSpeed) || IsEffectInSpell(selectedBotSpell.SpellId, SE_ATK) ||
									IsEffectInSpell(selectedBotSpell.SpellId, SE_STR) || IsEffectInSpell(selectedBotSpell.SpellId, SE_ReverseDS))
								{
									continue;
								}
								break;
							case ARCHETYPE_MELEE:
								if (IsEffectInSpell(selectedBotSpell.SpellId, SE_IncreaseSpellHaste) || IsEffectInSpell(selectedBotSpell.SpellId, SE_ManaPool) ||
									IsEffectInSpell(selectedBotSpell.SpellId, SE_CastingLevel) || IsEffectInSpell(selectedBotSpell.SpellId, SE_ManaRegen_v2) ||
									IsEffectInSpell(selectedBotSpell.SpellId, SE_CurrentMana))
								{
									continue;
								}
								break;
							case ARCHETYPE_HYBRID:
								//Hybrids get all buffs
							default:
								break;
							}
						}

						castedSpell = AIDoSpellCast(selectedBotSpell.SpellIndex, tar, selectedBotSpell.ManaCost, &TempDontBuffMeBefore);

						if (TempDontBuffMeBefore != tar->DontBuffMeBefore())
							tar->SetDontBuffMeBefore(TempDontBuffMeBefore);
					}

					if (castedSpell)
						break;
				}
			}
		}
		break;
	}
	case SpellType_Lifetap: {
		if (GetHPRatio() < 90.0f) {
			if (!checked_los) {
				if (!CheckLosFN(tar))
					break;	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call

				checked_los = true;
			}

			botSpell = GetFirstBotSpellBySpellType(this, iSpellTypes);

			if (botSpell.SpellId == 0)
				break;

			if (!(!tar->IsImmuneToSpell(botSpell.SpellId, this) && (tar->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0)))
				break;

			castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost);
		}
		break;
	}
	case SpellType_Snare: {
		if (tar->DontSnareMeBefore() < Timer::GetCurrentTime()) {
			if (!checked_los) {
				if (!CheckLosFN(tar))
					break;	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call

				checked_los = true;
			}

			botSpell = GetFirstBotSpellBySpellType(this, iSpellTypes);

			if (botSpell.SpellId == 0)
				break;

			if (!(!tar->IsImmuneToSpell(botSpell.SpellId, this) && tar->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0))
				break;

			uint32 TempDontSnareMeBefore = tar->DontSnareMeBefore();

			castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost, &TempDontSnareMeBefore);

			if (TempDontSnareMeBefore != tar->DontSnareMeBefore())
				tar->SetDontSnareMeBefore(TempDontSnareMeBefore);
		}
		break;
	}
	case SpellType_DOT: {
		if ((tar->GetHPRatio() <= 98.0f) && (tar->DontDotMeBefore() < Timer::GetCurrentTime()) && (tar->GetHPRatio() > 15.0f)) {
			if (!checked_los) {
				if (!CheckLosFN(tar))
					break;	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call

				checked_los = true;
			}

			if (GetClass() == BARD) {
				std::list<BotSpell_wPriority> dotList = GetPrioritizedBotSpellsBySpellType(this, SpellType_DOT);

				const int maxDotSelect = 5;
				int dotSelectCounter = 0;

				for (std::list<BotSpell_wPriority>::iterator itr = dotList.begin(); itr != dotList.end(); ++itr) {
					BotSpell selectedBotSpell = *itr;

					if (selectedBotSpell.SpellId == 0)
						continue;

					if (CheckSpellRecastTimers(this, itr->SpellIndex))
					{

						if (!(!tar->IsImmuneToSpell(selectedBotSpell.SpellId, this) && tar->CanBuffStack(selectedBotSpell.SpellId, botLevel, true) >= 0))
							continue;

						uint32 TempDontDotMeBefore = tar->DontDotMeBefore();

						castedSpell = AIDoSpellCast(selectedBotSpell.SpellIndex, tar, selectedBotSpell.ManaCost, &TempDontDotMeBefore);

						if (TempDontDotMeBefore != tar->DontDotMeBefore())
							tar->SetDontDotMeBefore(TempDontDotMeBefore);
					}

					dotSelectCounter++;

					if ((dotSelectCounter == maxDotSelect) || castedSpell)
						break;
				}
			}
			else {
				std::list<BotSpell> dotList = GetBotSpellsBySpellType(this, SpellType_DOT);

				const int maxDotSelect = 5;
				int dotSelectCounter = 0;

				for (std::list<BotSpell>::iterator itr = dotList.begin(); itr != dotList.end(); ++itr) {
					BotSpell selectedBotSpell = *itr;

					if (selectedBotSpell.SpellId == 0)
						continue;

					if (CheckSpellRecastTimers(this, itr->SpellIndex))
					{

						if (!(!tar->IsImmuneToSpell(selectedBotSpell.SpellId, this) && tar->CanBuffStack(selectedBotSpell.SpellId, botLevel, true) >= 0))
							continue;

						uint32 TempDontDotMeBefore = tar->DontDotMeBefore();

						castedSpell = AIDoSpellCast(selectedBotSpell.SpellIndex, tar, selectedBotSpell.ManaCost, &TempDontDotMeBefore);

						if (TempDontDotMeBefore != tar->DontDotMeBefore())
							tar->SetDontDotMeBefore(TempDontDotMeBefore);
					}

					dotSelectCounter++;

					if ((dotSelectCounter == maxDotSelect) || castedSpell)
						break;
				}
			}
		}
		break;
	}
	case SpellType_Slow: {
		if (tar->GetHPRatio() <= 99.0f) {

			if (!checked_los) {
				if (!CheckLosFN(tar))
					break;	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call

				checked_los = true;
			}

			switch (botClass) {
			case BARD: {
				// probably needs attackable check
				std::list<BotSpell_wPriority> botSongList = GetPrioritizedBotSpellsBySpellType(this, SpellType_Slow);
				for (auto iter : botSongList) {
					if (!iter.SpellId)
						continue;
					if (!CheckSpellRecastTimers(this, iter.SpellIndex))
						continue;
					if (spells[iter.SpellId].zone_type != -1 && zone->GetZoneType() != -1 && spells[iter.SpellId].zone_type != zone->GetZoneType()) // is this bit or index?
						continue;
					if (spells[iter.SpellId].target_type != ST_Target)
						continue;
					if (tar->CanBuffStack(iter.SpellId, botLevel, true) < 0)
						continue;

					castedSpell = AIDoSpellCast(iter.SpellIndex, tar, iter.ManaCost);
					if (castedSpell)
						break;
				}

				break;
			}
			case ENCHANTER: {
				botSpell = GetBestBotSpellForMagicBasedSlow(this);
				break;
			}
			case SHAMAN:
			case BEASTLORD: {
				botSpell = GetBestBotSpellForDiseaseBasedSlow(this);

				if (botSpell.SpellId == 0 || ((tar->GetMR() - 50) < (tar->GetDR() + spells[botSpell.SpellId].resist_difficulty)))
					botSpell = GetBestBotSpellForMagicBasedSlow(this);
				break;
			}
			}

			if (botSpell.SpellId == 0)
				break;

			if (!(!tar->IsImmuneToSpell(botSpell.SpellId, this) && tar->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0))
				break;

			castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost);

			if (castedSpell && GetClass() != BARD)
				BotGroupSay(this, "Attempting to slow %s.", tar->GetCleanName());
		}
		break;
	}
	case SpellType_Debuff: {
		if ((tar->GetHPRatio() <= 99.0f) || ((botClass == BARD) || (botClass == SHAMAN) || (botClass == ENCHANTER) || (botClass == DRUID)) && (tar->GetHPRatio() > 40.0f))
		{
			if (!checked_los) {
				if (!CheckLosFN(tar))
					break;	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call

				checked_los = true;
			}

			botSpell = GetBestBotSpellForResistDebuff(this, tar);

			if (botSpell.SpellId == 0)
				botSpell = GetDebuffBotSpell(this, tar);

			if (botSpell.SpellId == 0)
				break;

			if (!(!tar->IsImmuneToSpell(botSpell.SpellId, this) && (tar->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0)))
				break;

			castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost);
		}
		break;
	}
	case SpellType_Cure: {
		if (GetNeedsCured(tar) && (tar->DontCureMeBefore() < Timer::GetCurrentTime()) && !(GetNumberNeedingHealedInRaidGroup(25, false) > 0) && !(GetNumberNeedingHealedInRaidGroup(40, false) > 2))
		{
			botSpell = GetBestBotSpellForCure(this, tar);

			if (botSpell.SpellId == 0)
				break;

			uint32 TempDontCureMeBeforeTime = tar->DontCureMeBefore();

			castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost, &TempDontCureMeBeforeTime);

			if (castedSpell) {
				if (botClass != BARD) {
					if (IsGroupSpell(botSpell.SpellId)) {
						if (IsRaidGrouped()) {
							if (r_group) {
								std::vector<RaidMember> raid_group_members = raid->GetRaidGroupMembers(r_group);
								for (std::vector<RaidMember>::iterator iter = raid_group_members.begin(); iter != raid_group_members.end(); ++iter) {
									if (iter->member && !iter->member->qglobal) {
										if (TempDontCureMeBeforeTime != tar->DontCureMeBefore())
											iter->member->SetDontCureMeBefore(Timer::GetCurrentTime() + 4000);
									}
								}
							}
						}
					}
					else {
						if (TempDontCureMeBeforeTime != tar->DontCureMeBefore())
							tar->SetDontCureMeBefore(Timer::GetCurrentTime() + 4000);
					}
				}
			}
		}
		break;
	}
	case SpellType_Resurrect:
		break;
	case SpellType_HateRedux: {
		// assumed group member at this point
		if (GetClass() == BARD) {
			std::list<BotSpell_wPriority> botSongList = GetPrioritizedBotSpellsBySpellType(this, SpellType_HateRedux);
			for (auto iter : botSongList) {
				if (!iter.SpellId)
					continue;
				if (!CheckSpellRecastTimers(this, iter.SpellIndex))
					continue;
				if (spells[iter.SpellId].zone_type != -1 && zone->GetZoneType() != -1 && spells[iter.SpellId].zone_type != zone->GetZoneType()) // is this bit or index?
					continue;
				if (spells[iter.SpellId].target_type != ST_Target)
					continue;
				if (tar->CanBuffStack(iter.SpellId, botLevel, true) < 0)
					continue;

				castedSpell = AIDoSpellCast(iter.SpellIndex, tar, iter.ManaCost);
				if (castedSpell) {
					BotGroupSay(this, "Attempting to reduce hate on %s.", tar->GetCleanName());
					break;
				}
			}
		}

		break;
	}
	case SpellType_InCombatBuffSong: {
		if (GetClass() != BARD || tar != this) // In-Combat songs can be cast Out-of-Combat in preparation for battle
			break;

		std::list<BotSpell_wPriority> botSongList = GetPrioritizedBotSpellsBySpellType(this, SpellType_InCombatBuffSong);
		for (auto iter : botSongList) {
			if (!iter.SpellId)
				continue;
			if (!CheckSpellRecastTimers(this, iter.SpellIndex))
				continue;
			if (spells[iter.SpellId].zone_type != -1 && zone->GetZoneType() != -1 && spells[iter.SpellId].zone_type != zone->GetZoneType()) // is this bit or index?
				continue;
			switch (spells[iter.SpellId].target_type) {
			case ST_AEBard:
			case ST_AECaster:
			case ST_GroupTeleport:
			case ST_Group:
			case ST_Self:
				break;
			default:
				continue;
			}
			if (tar->CanBuffStack(iter.SpellId, botLevel, true) < 0)
				continue;

			castedSpell = AIDoSpellCast(iter.SpellIndex, tar, iter.ManaCost);
			if (castedSpell)
				break;
		}

		break;
	}
	case SpellType_OutOfCombatBuffSong: {
		if (GetClass() != BARD || tar != this || IsEngaged()) // Out-of-Combat songs can not be cast in combat
			break;

		std::list<BotSpell_wPriority> botSongList = GetPrioritizedBotSpellsBySpellType(this, SpellType_OutOfCombatBuffSong);
		for (auto iter : botSongList) {
			if (!iter.SpellId)
				continue;
			if (!CheckSpellRecastTimers(this, iter.SpellIndex))
				continue;
			if (spells[iter.SpellId].zone_type != -1 && zone->GetZoneType() != -1 && spells[iter.SpellId].zone_type != zone->GetZoneType()) // is this bit or index?
				continue;
			switch (spells[iter.SpellId].target_type) {
			case ST_AEBard:
			case ST_AECaster:
			case ST_GroupTeleport:
			case ST_Group:
			case ST_Self:
				break;
			default:
				continue;
			}
			if (tar->CanBuffStack(iter.SpellId, botLevel, true) < 0)
				continue;

			castedSpell = AIDoSpellCast(iter.SpellIndex, tar, iter.ManaCost);
			if (castedSpell)
				break;
		}

		break;
	}
	case SpellType_PreCombatBuff: {
		break;
	}
	case SpellType_PreCombatBuffSong: {
		break;
	}
	default:
		break;
	}

	return castedSpell;
}

void Raid::RaidBotGroupSay(Bot* b, uint8 language, uint8 lang_skill, const char* msg, ...)
{
	if (!b)
		return;

	char buf[1000];
	va_list ap;
	va_start(ap, msg);
	vsnprintf(buf, 1000, msg, ap);
	va_end(ap);

	uint32 groupToUse = GetGroup(b->GetName());

	if (groupToUse > 11)
		return;

	auto pack = new ServerPacket(ServerOP_RaidGroupSay, sizeof(ServerRaidMessage_Struct) + strlen(msg) + 1);
	ServerRaidMessage_Struct* rga = (ServerRaidMessage_Struct*)pack->pBuffer;
	rga->rid = GetID();
	rga->gid = groupToUse;
	rga->language = language;
	rga->lang_skill = lang_skill;
	strn0cpy(rga->from, b->GetName(), 64);

	strcpy(rga->message, buf); // this is safe because we are allocating enough space for the entire msg above

	worldserver.SendPacket(pack);
	//safe_delete(pack);
}

uint8 Bot::GetNumberNeedingHealedInRaidGroup(uint8 hpr, bool includePets) {
	uint8 needHealed = 0;
	Raid* raid = nullptr;
	raid = entity_list.GetRaidByBotName(GetName());
	uint32 r_group = raid->GetGroup(GetName());
	std::vector<RaidMember> raid_group_members = raid->GetRaidGroupMembers(r_group);
	//for (std::vector<RaidMember>::iterator iter = raid_group_members.begin(); iter != raid_group_members.end(); ++iter) {
	for (int i = 0; i< raid_group_members.size(); ++i) {
		if (raid_group_members.at(i).member && !raid_group_members.at(i).member->qglobal) {
			if (raid_group_members.at(i).member->GetHPRatio() <= hpr)
				needHealed++;

				if (includePets) {
					if (raid_group_members.at(i).member->GetPet() && raid_group_members.at(i).member->GetPet()->GetHPRatio() <= hpr)
						needHealed++;
				}
			}
		}
	return needHealed;
}
