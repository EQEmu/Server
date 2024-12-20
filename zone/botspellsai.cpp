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
#include "../common/data_verification.h"
#include "../common/repositories/bot_spells_entries_repository.h"
#include "../common/repositories/npc_spells_repository.h"

bool Bot::AICastSpell(Mob* tar, uint8 iChance, uint16 spellType, uint16 subTargetType, uint16 subType) {
	if (!tar) {
		return false;
	}

	LogBotPreChecksDetail("{} says, 'Attempting {} AICastSpell on {}.'", GetCleanName(), GetSpellTypeNameByID(spellType), tar->GetCleanName()); //deleteme

	if (
		!AI_HasSpells() ||
		(spellType == BotSpellTypes::Pet && tar != this) ||
		(IsPetBotSpellType(spellType) && !tar->IsPet()) ||
		(spellType == BotSpellTypes::Buff && tar->IsPet()) ||
		(spellType == BotSpellTypes::InCombatBuffSong && tar->IsPet()) ||
		(spellType == BotSpellTypes::OutOfCombatBuffSong && tar->IsPet()) ||
		(spellType == BotSpellTypes::PreCombatBuffSong && tar->IsPet()) ||
		(!RuleB(Bots, AllowBuffingHealingFamiliars) && tar->IsFamiliar()) ||
		(tar->IsPet() && tar->IsCharmed() && spellType == BotSpellTypes::PetBuffs && !RuleB(Bots, AllowCharmedPetBuffs)) ||
		(tar->IsPet() && tar->IsCharmed() && (spellType == BotSpellTypes::Cure || spellType == BotSpellTypes::GroupCures) && !RuleB(Bots, AllowCharmedPetCures)) ||
		(tar->IsPet() && tar->IsCharmed() && IsHealBotSpellType(spellType) && !RuleB(Bots, AllowCharmedPetHeals))
	) {
		return false;
	}
	
	if (iChance < 100 && zone->random.Int(0, 100) > iChance) {
		return false;
	}

	if ((spellType != BotSpellTypes::Resurrect && spellType != BotSpellTypes::SummonCorpse) && tar->GetAppearance() == eaDead) {
		if (!((tar->IsClient() && tar->CastToClient()->GetFeigned()) || tar->IsBot())) {
			return false;
		}
	}

	uint8 botClass = GetClass();

	SetCastedSpellType(UINT16_MAX);

	BotSpell botSpell;
	botSpell.SpellId = 0;
	botSpell.SpellIndex = 0;
	botSpell.ManaCost = 0;

	if (SpellTypeRequiresLoS(spellType) && tar != this) {
		SetHasLoS(DoLosChecks(this, tar));
	}
	else {
		SetHasLoS(true);
	}

	switch (spellType) {
		case BotSpellTypes::Slow:
			if (tar->GetSpecialAbility(SpecialAbility::SlowImmunity)) {
				return false;
			}

			break;
		case BotSpellTypes::Snare:
			if (tar->GetSpecialAbility(SpecialAbility::SnareImmunity)) {
				return false;
			}

			break;
		case BotSpellTypes::AELull:
		case BotSpellTypes::Lull:
			if (tar->GetSpecialAbility(SpecialAbility::PacifyImmunity)) {
				return false;
			}

			break;
		case BotSpellTypes::Fear:
			if (tar->GetSpecialAbility(SpecialAbility::FearImmunity)) {
				return false;
			}

			if (!IsCommandedSpell() && (tar->IsRooted() || tar->GetSnaredAmount() == -1)) {
				return false;
			}

			break;
		case BotSpellTypes::Dispel:
			if (tar->GetSpecialAbility(SpecialAbility::DispellImmunity)) {
				return false;
			}

			if (!IsCommandedSpell() && tar->CountDispellableBuffs() <= 0) {
				return false;
			}

			break;
		case BotSpellTypes::HateRedux:
			if (!IsCommandedSpell() && !GetNeedsHateRedux(tar)) {
				return false;
			}

			break;
		case BotSpellTypes::InCombatBuff:
			if (!IsCommandedSpell() && GetClass() != Class::Shaman && spellType == BotSpellTypes::InCombatBuff && IsCasterClass(GetClass()) && GetLevel() >= GetStopMeleeLevel()) {
				return false;
			}

			break;
		case BotSpellTypes::HateLine:
			if (!tar->IsNPC()) {
				return false;
			}

			break;
		case BotSpellTypes::InCombatBuffSong:
		case BotSpellTypes::Buff:
		case BotSpellTypes::PetBuffs:
		case BotSpellTypes::PreCombatBuff:
		case BotSpellTypes::DamageShields:
		case BotSpellTypes::PetDamageShields:
		case BotSpellTypes::ResistBuffs:
		case BotSpellTypes::PetResistBuffs:
		case BotSpellTypes::Teleport:
		case BotSpellTypes::Succor:
		case BotSpellTypes::BindAffinity:
		case BotSpellTypes::Identify:
		case BotSpellTypes::Levitate:
		case BotSpellTypes::Rune:
		case BotSpellTypes::WaterBreathing:
		case BotSpellTypes::Size:
		case BotSpellTypes::Invisibility:
		case BotSpellTypes::MovementSpeed:
		case BotSpellTypes::SendHome:
			if (!tar->IsOfClientBot() && !(tar->IsPet() && tar->GetOwner() && tar->GetOwner()->IsOfClientBot())) {
				return false;
			}

			break;
		case BotSpellTypes::PreCombatBuffSong:
		case BotSpellTypes::OutOfCombatBuffSong:
			if (!IsCommandedSpell() && (IsEngaged() || tar->IsEngaged())) { // Out-of-Combat songs can not be cast in combat
				return false;
			}

			break;
		case BotSpellTypes::AEMez:
		case BotSpellTypes::Mez:
			return BotCastMez(tar, botClass, botSpell, spellType);
		case BotSpellTypes::AENukes:
		case BotSpellTypes::AERains:
		case BotSpellTypes::PBAENuke:
		case BotSpellTypes::Nuke:
		case BotSpellTypes::AEStun:
		case BotSpellTypes::Stun:
			return BotCastNuke(tar, botClass, botSpell, spellType);
		case BotSpellTypes::RegularHeal:
		case BotSpellTypes::GroupCompleteHeals:
		case BotSpellTypes::CompleteHeal:
		case BotSpellTypes::FastHeals:
		case BotSpellTypes::VeryFastHeals:
		case BotSpellTypes::GroupHeals:
		case BotSpellTypes::GroupHoTHeals:
		case BotSpellTypes::HoTHeals:
		case BotSpellTypes::PetRegularHeals:
		case BotSpellTypes::PetCompleteHeals:
		case BotSpellTypes::PetFastHeals:
		case BotSpellTypes::PetVeryFastHeals:
		case BotSpellTypes::PetHoTHeals:
			if (!tar->IsOfClientBot() && !(tar->IsPet() && tar->GetOwner() && tar->GetOwner()->IsOfClientBot())) {
				return false;
			}

			return BotCastHeal(tar, botClass, botSpell, spellType);
		case BotSpellTypes::GroupCures:
		case BotSpellTypes::Cure:
			if (!tar->IsOfClientBot() && !(tar->IsPet() && tar->GetOwner() && tar->GetOwner()->IsOfClientBot())) {
				return false;
			}

			return BotCastCure(tar, botClass, botSpell, spellType);
		case BotSpellTypes::Pet:
			if (HasPet() || IsBotCharmer()) {
				return false;
			}

			return BotCastPet(tar, botClass, botSpell, spellType);
		case BotSpellTypes::Resurrect:
			if (!tar->IsCorpse() || !tar->CastToCorpse()->IsPlayerCorpse()) {
				return false;
			}

			break;
		case BotSpellTypes::Charm:
			if (HasPet() || tar->IsCharmed() || !tar->IsNPC() || tar->GetSpecialAbility(SpecialAbility::CharmImmunity)) {
				return false;
			}

			break;
		default:
			break;
	}

	std::list<BotSpell_wPriority> botSpellList = GetPrioritizedBotSpellsBySpellType(this, spellType, tar, (IsAEBotSpellType(spellType) || subTargetType == CommandedSubTypes::AETarget), subTargetType, subType);

	for (const auto& s : botSpellList) {

		if (!IsValidSpellAndLoS(s.SpellId, HasLoS())) {
			continue;
		}

		if (IsInvulnerabilitySpell(s.SpellId)) {
			tar = this; //target self for invul type spells
		}

		if (IsCommandedSpell() && IsCasting()) {
			BotGroupSay(
				this,
				fmt::format(
					"Interrupting {}. I have been commanded to try to cast a [{}] spell, {} on {}.",
					CastingSpellID() ? spells[CastingSpellID()].name : "my spell",
					GetSpellTypeNameByID(spellType),
					spells[s.SpellId].name,
					tar->GetCleanName()
				).c_str()
			);
		
			InterruptSpell();
		}

		if (AIDoSpellCast(s.SpellIndex, tar, s.ManaCost)) {
			if (IsBotSpellTypeOtherBeneficial(spellType)) {
				SetCastedSpellType(UINT16_MAX);

				if (!IsCommandedSpell()) {
					SetBotSpellRecastTimer(spellType, tar, true);
				}
			}
			else {
				SetCastedSpellType(spellType);
			}

			if (botClass != Class::Bard || RuleB(Bots, BardsAnnounceCasts)) {
				BotGroupSay(
					this,
					fmt::format(
						"Casting {} [{}] on {}.",
						GetSpellName(s.SpellId),
						GetSpellTypeNameByID(spellType),
						(tar == this ? "myself" : tar->GetCleanName())
					).c_str()
				);
			}

			return true;
		}
		else { LogTestDebug("{} says, '{} [#{}] - [{}] FAILED AIDoSpellCast on {}.'", GetCleanName(), spells[s.SpellId].name, s.SpellId, GetSpellTypeNameByID(spellType), tar->GetCleanName()); } //deleteme
	}

	return false;
}

bool Bot::BotCastMez(Mob* tar, uint8 botClass, BotSpell& botSpell, uint16 spellType) {
	std::list<BotSpell_wPriority> botSpellList = GetPrioritizedBotSpellsBySpellType(this, spellType, tar, IsAEBotSpellType(spellType));

	for (const auto& s : botSpellList) {
		if (!IsValidSpellAndLoS(s.SpellId, HasLoS())) {
			continue;
		}

		if (!IsCommandedSpell()) {
			Mob* addMob = GetFirstIncomingMobToMez(this, s.SpellId, spellType, IsAEBotSpellType(spellType));

			if (!addMob) {
				return false;
			}

			tar = addMob;
		}

		if (AIDoSpellCast(s.SpellIndex, tar, s.ManaCost)) {
			if (IsBotSpellTypeOtherBeneficial(spellType)) {
				SetCastedSpellType(UINT16_MAX);
				
				if (!IsCommandedSpell()) {
					SetBotSpellRecastTimer(spellType, tar, true);
				}
			}
			else {
				SetCastedSpellType(spellType);
			}

			BotGroupSay(
				this,
				fmt::format(
					"Casting {} [{}] on {}.",
					GetSpellName(s.SpellId),
					GetSpellTypeNameByID(spellType),
					(tar == this ? "myself" : tar->GetCleanName())
				).c_str()
			);

			return true;
		}
	}

	return false;
}

bool Bot::BotCastCure(Mob* tar, uint8 botClass, BotSpell& botSpell, uint16 spellType) {
	uint32 currentTime = Timer::GetCurrentTime();
	uint32 nextCureTime = tar->DontCureMeBefore();

	if (!IsCommandedSpell()) {
		if ((nextCureTime > currentTime) || !GetNeedsCured(tar)) {
			return false;
		}
	}

	botSpell = GetBestBotSpellForCure(this, tar, spellType);

	if (!IsValidSpellAndLoS(botSpell.SpellId, HasLoS())) {
		return false;
	}

	if (AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost)) {
		if (botClass != Class::Bard || RuleB(Bots, BardsAnnounceCasts)) {
			if (IsGroupSpell(botSpell.SpellId)) {
				BotGroupSay(
					this,
					fmt::format(
						"Curing the group with {}.",
						GetSpellName(botSpell.SpellId)
					).c_str()
				);

				const std::vector<Mob*> v = GatherGroupSpellTargets(tar);


				if (!IsCommandedSpell()) {
					for (Mob* m : v) {
						SetBotSpellRecastTimer(spellType, m, true);
					}
				}
			}
			else {
				BotGroupSay(
					this,
					fmt::format(
						"Curing {} with {}.",
						(tar == this ? "myself" : tar->GetCleanName()),
						GetSpellName(botSpell.SpellId)
					).c_str()
				);

				if (!IsCommandedSpell()) {
					SetBotSpellRecastTimer(spellType, tar, true);
				}

				return true;
			}
		}
	}

	return false;
}

bool Bot::BotCastPet(Mob* tar, uint8 botClass, BotSpell& botSpell, uint16 spellType) {
	if (botClass == Class::Wizard) {
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
		if (GetPetID()) {
			return false;
		}
		if (familiar_buff_slot >= 0) {
			BuffFadeBySlot(familiar_buff_slot);
			return false;
		}

		botSpell = GetFirstBotSpellBySpellType(this, spellType);
	}
	else if (botClass == Class::Magician) {
		botSpell = GetBestBotMagicianPetSpell(this, spellType);
	}
	else {
		botSpell = GetFirstBotSpellBySpellType(this, spellType);
	}

	if (!IsValidSpellAndLoS(botSpell.SpellId, HasLoS())) {
		return false;
	}

	if (AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost)) {
		SetCastedSpellType(spellType);
		BotGroupSay(
			this,
			fmt::format(
				"Summoning a pet [{}].",
				GetSpellName(botSpell.SpellId)
			).c_str()
		);

		return true;
	}

	return false;
}

bool Bot::BotCastNuke(Mob* tar, uint8 botClass, BotSpell& botSpell, uint16 spellType) {
	if (spellType == BotSpellTypes::Stun || spellType == BotSpellTypes::AEStun) {
		uint8 stunChance = (tar->IsCasting() ? RuleI(Bots, StunCastChanceIfCasting) : RuleI(Bots, StunCastChanceNormal));

		if (botClass == Class::Paladin) {
			stunChance = RuleI(Bots, StunCastChancePaladins);
		}

		if (
			!tar->GetSpecialAbility(SpecialAbility::StunImmunity) && 
			(
				IsCommandedSpell() || 
				(!tar->IsStunned() && (zone->random.Int(1, 100) <= stunChance))
			) 
		) {
			botSpell = GetBestBotSpellForStunByTargetType(this, ST_TargetOptional, spellType, IsAEBotSpellType(spellType), tar);
		}
		
		if (!IsValidSpellAndLoS(botSpell.SpellId, HasLoS())) {
			return false;
		}
	}

	if (!IsValidSpellAndLoS(botSpell.SpellId, HasLoS())) {
		botSpell = GetBestBotSpellForNukeByBodyType(this, tar->GetBodyType(), spellType, IsAEBotSpellType(spellType), tar);
	}

	if (!IsValidSpellAndLoS(botSpell.SpellId, HasLoS()) && spellType == BotSpellTypes::Nuke && botClass == Class::Wizard) {
		botSpell = GetBestBotWizardNukeSpellByTargetResists(this, tar, spellType);
	}

	if (!IsValidSpellAndLoS(botSpell.SpellId, HasLoS())) {
		std::list<BotSpell_wPriority> botSpellList = GetPrioritizedBotSpellsBySpellType(this, spellType, tar, IsAEBotSpellType(spellType));

		for (const auto& s : botSpellList) {
			if (!IsValidSpellAndLoS(s.SpellId, HasLoS())) {
				continue;
			}

			if (AIDoSpellCast(s.SpellIndex, tar, s.ManaCost)) {
				SetCastedSpellType(spellType);

				if (botClass != Class::Bard || RuleB(Bots, BardsAnnounceCasts)) {
					BotGroupSay(
						this,
						fmt::format(
							"Casting {} [{}] on {}.",
							GetSpellName(s.SpellId),
							GetSpellTypeNameByID(spellType),
							tar->GetCleanName()
						).c_str()
					);
				}

				return true;
			}
		}
	}
	else {
		if (AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost)) {
			SetCastedSpellType(spellType);

			if (botClass != Class::Bard || RuleB(Bots, BardsAnnounceCasts)) {
				BotGroupSay(
					this,
					fmt::format(
						"Casting {} [{}] on {}.",
						GetSpellName(botSpell.SpellId),
						GetSpellTypeNameByID(spellType),
						tar->GetCleanName()
					).c_str()
				);
			}

			return true;
		}
	}

	return false;
}

bool Bot::BotCastHeal(Mob* tar, uint8 botClass, BotSpell& botSpell, uint16 spellType) {
	botSpell = GetSpellByHealType(spellType, tar);

	if (!IsValidSpell(botSpell.SpellId)) {
		return false;
	}

	if (AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost)) {
		if (botClass != Class::Bard || RuleB(Bots, BardsAnnounceCasts)) {
			if (IsGroupSpell(botSpell.SpellId)) {
				BotGroupSay(
					this,
					fmt::format(
						"Healing the group with {} [{}].",
						GetSpellName(botSpell.SpellId),
						GetSpellTypeNameByID(spellType)
					).c_str()
				);

				if (botClass != Class::Bard) {
					const std::vector<Mob*> v = GatherGroupSpellTargets(tar);

					if (!IsCommandedSpell()) {
						for (Mob* m : v) {
							SetBotSpellRecastTimer(spellType, m, true);
						}
					}
				}

			}
			else {
				BotGroupSay(
					this,
					fmt::format(
						"Healing {} with {} [{}].",
						(tar == this ? "myself" : tar->GetCleanName()), 
						GetSpellName(botSpell.SpellId), 
						GetSpellTypeNameByID(spellType)
					).c_str()
				);

				if (botClass != Class::Bard) {
					if (!IsCommandedSpell()) {
						SetBotSpellRecastTimer(spellType, tar, true);
					}
				}
			}
		}

		return true;
	}

	return false;
}

bool Bot::AIDoSpellCast(int32 i, Mob* tar, int32 mana_cost, uint32* oDontDoAgainBefore) {
	bool result = false;

	// manacost has special values, -1 is no mana cost, -2 is instant cast (no mana)
	int32 manaCost = mana_cost;

	if (manaCost == -1) {
		manaCost = spells[AIBot_spells[i].spellid].mana;
	} else if (manaCost == -2) {
		manaCost = 0;
	}

	int64 hasMana = GetMana();

	// Allow bots to cast buff spells even if they are out of mana
	if (
		RuleB(Bots, FinishBuffing) &&
		manaCost > hasMana && AIBot_spells[i].type == BotSpellTypes::Buff
	) {
		SetMana(manaCost);
	}

	float dist2 = 0;

	if (AIBot_spells[i].type == BotSpellTypes::Escape) {
		dist2 = 0;
	} else
		dist2 = DistanceSquared(m_Position, tar->GetPosition());

	if (IsValidSpellRange(AIBot_spells[i].spellid, tar) && (mana_cost <= GetMana() || IsBotNonSpellFighter())) {
		casting_spell_AIindex = i;
		LogAI("spellid [{}] tar [{}] mana [{}] Name [{}]", AIBot_spells[i].spellid, tar->GetName(), mana_cost, spells[AIBot_spells[i].spellid].name);
		result = Mob::CastSpell(AIBot_spells[i].spellid, tar->GetID(), EQ::spells::CastingSlot::Gem2, spells[AIBot_spells[i].spellid].cast_time, AIBot_spells[i].manacost == -2 ? 0 : mana_cost, oDontDoAgainBefore, -1, -1, 0, &(AIBot_spells[i].resist_adjust));

		if (IsCasting() && IsSitting())
			Stand();
	}

	// if the spell wasn't casted, then take back any extra mana that was given to the bot to cast that spell
	if (!result) {
		SetMana(hasMana);
	}

	return result;
}

bool Bot::AI_PursueCastCheck() {
	if (GetAppearance() == eaDead || delaytimer || spellend_timer.Enabled() || IsFeared() || IsSilenced() || IsAmnesiad() || GetHP() < 0) {
		return false;
	}

	bool result = false;

	if (GetTarget() && AIautocastspell_timer->Check(false)) {

		LogAIDetail("Bot Pursue autocast check triggered: [{}]", GetCleanName());
		LogBotPreChecksDetail("{} says, 'AI_PursueCastCheck started.'", GetCleanName()); //deleteme

		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.

		if (!IsAttackAllowed(GetTarget())) {
			return false;
		}

		auto castOrder = GetSpellTypesPrioritized(BotPriorityCategories::Pursue);

		for (auto& currentCast : castOrder) {
			if (currentCast.priority == 0) {
				LogBotPreChecksDetail("{} says, '[{}] is priority 0, skipping.'", GetCleanName(), GetSpellTypeNameByID(currentCast.spellType)); //deleteme
				continue;
			}

			if (!RuleB(Bots, AllowAIMez) && (currentCast.spellType == BotSpellTypes::AEMez || currentCast.spellType == BotSpellTypes::Mez)) {
				continue;
			}

			if (IsCommandedSpellType(currentCast.spellType)) { // Unsupported by AI currently.
				continue;
			}

			result = AttemptAICastSpell(currentCast.spellType);

			if (result) {
				break;
			}
		}

		if (!AIautocastspell_timer->Enabled()) {
			AIautocastspell_timer->Start(RandomTimer(RuleI(Bots, MinDelayBetweenInCombatCastAttempts), RuleI(Bots, MaxDelayBetweenInCombatCastAttempts)), false);
		}
	}

	return result;
}

bool Bot::AI_IdleCastCheck() {
	if (GetAppearance() == eaDead || delaytimer || spellend_timer.Enabled() || IsFeared() || IsSilenced() || IsAmnesiad() || GetHP() < 0) {
		return false;
	}

	bool result = false;

	if (AIautocastspell_timer->Check(false)) {

		LogAIDetail("Bot Non-Engaged autocast check triggered: [{}]", GetCleanName());
		LogBotPreChecksDetail("{} says, 'AI_IdleCastCheck started.'", GetCleanName()); //deleteme

		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.

		bool preCombat = false;
		Client* test_against = nullptr;

		if (HasGroup() && GetGroup()->GetLeader() && GetGroup()->GetLeader()->IsClient()) {
			test_against = GetGroup()->GetLeader()->CastToClient();
		}
		else if (GetOwner() && GetOwner()->IsClient()) {
			test_against = GetOwner()->CastToClient();
		}

		if (test_against) {
			preCombat = test_against->GetBotPrecombat();
		}

		auto castOrder = GetSpellTypesPrioritized(BotPriorityCategories::Idle);

		for (auto& currentCast : castOrder) {
			if (currentCast.priority == 0) {
				LogBotPreChecksDetail("{} says, '[{}] is priority 0, skipping.'", GetCleanName(), GetSpellTypeNameByID(currentCast.spellType)); //deleteme
				continue;
			}

			if (!preCombat && (currentCast.spellType == BotSpellTypes::PreCombatBuff || currentCast.spellType == BotSpellTypes::PreCombatBuffSong)) {
				continue;
			}

			if (!RuleB(Bots, AllowAIMez) && (currentCast.spellType == BotSpellTypes::AEMez || currentCast.spellType == BotSpellTypes::Mez)) {
				continue;
			}

			if (IsCommandedSpellType(currentCast.spellType)) { // Unsupported by AI currently.
				continue;
			}

			result = AttemptAICastSpell(currentCast.spellType);

			if (result) {
				break;
			}
		}

		if (!AIautocastspell_timer->Enabled()) {
			AIautocastspell_timer->Start(RandomTimer(RuleI(Bots, MinDelayBetweenOutCombatCastAttempts), RuleI(Bots, MaxDelayBetweenOutCombatCastAttempts)), false);
		}
	}

	return result;
}

bool Bot::AI_EngagedCastCheck() {
	if (GetAppearance() == eaDead || delaytimer || spellend_timer.Enabled() || IsFeared() || IsSilenced() || IsAmnesiad() || GetHP() < 0) {
		return false;
	}

	bool result = false;
	bool failedToCast = false;

	if (GetTarget() && AIautocastspell_timer->Check(false)) {

		LogAIDetail("Bot Engaged autocast check triggered: [{}]", GetCleanName());
		LogBotPreChecksDetail("{} says, 'AI_EngagedCastCheck started.'", GetCleanName()); //deleteme

		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.

		if (!IsAttackAllowed(GetTarget())) {
			return false;
		}

		auto castOrder = GetSpellTypesPrioritized(BotPriorityCategories::Engaged);

		for (auto& currentCast : castOrder) {
			if (currentCast.priority == 0) {
				LogBotPreChecksDetail("{} says, '[{}] is priority 0, skipping.'", GetCleanName(), GetSpellTypeNameByID(currentCast.spellType)); //deleteme
				continue;
			}

			if (!RuleB(Bots, AllowAIMez) && (currentCast.spellType == BotSpellTypes::AEMez || currentCast.spellType == BotSpellTypes::Mez)) {
				continue;
			}

			if (IsCommandedSpellType(currentCast.spellType)) { // Unsupported by AI currently.
				continue;
			}

			result = AttemptAICastSpell(currentCast.spellType);

			if (result) {
				break;
			}
		}

		if (!AIautocastspell_timer->Enabled()) {
			AIautocastspell_timer->Start(RandomTimer(RuleI(Bots, MinDelayBetweenInCombatCastAttempts), RuleI(Bots, MaxDelayBetweenInCombatCastAttempts)), false);
		}
	}

	return result;
}

bool Bot::AIHealRotation(Mob* tar, bool useFastHeals) {

	if (!tar) {
		return false;
	}

	if (!AI_HasSpells())
		return false;

	if (tar->GetAppearance() == eaDead) {
		if ((tar->IsClient() && tar->CastToClient()->GetFeigned()) || tar->IsBot()) {
			// do nothing
		}
		else {
			return false;
		}
	}

	uint8 botLevel = GetLevel();

	bool castedSpell = false;

	BotSpell botSpell;
	botSpell.SpellId = 0;
	botSpell.SpellIndex = 0;
	botSpell.ManaCost = 0;

	if (useFastHeals) {
		botSpell = GetBestBotSpellForRegularSingleTargetHeal(this, tar);

		if (!IsValidSpell(botSpell.SpellId))
			botSpell = GetBestBotSpellForFastHeal(this, tar);
	}
	else {
		botSpell = GetBestBotSpellForPercentageHeal(this, tar);

		if (!IsValidSpell(botSpell.SpellId)) {
			botSpell = GetBestBotSpellForRegularSingleTargetHeal(this, tar);
		}
		if (!IsValidSpell(botSpell.SpellId)) {
			botSpell = GetFirstBotSpellForSingleTargetHeal(this, tar);
		}
		if (!IsValidSpell(botSpell.SpellId)) {
			botSpell = GetFirstBotSpellBySpellType(this, BotSpellTypes::RegularHeal);
		}
	}

	LogAIDetail("heal spellid [{}] fastheals [{}] casterlevel [{}]",
		botSpell.SpellId, ((useFastHeals) ? ('T') : ('F')), GetLevel());

	LogAIDetail("target [{}] current_time [{}] donthealmebefore [{}]", tar->GetCleanName(), Timer::GetCurrentTime(), tar->DontHealMeBefore());

	// If there is still no spell id, then there isn't going to be one so we are done
	if (!IsValidSpell(botSpell.SpellId)) {
		return false;
	}

	// Can we cast this spell on this target?
	if (!
		(
			spells[botSpell.SpellId].target_type == ST_GroupTeleport ||
			spells[botSpell.SpellId].target_type == ST_Target ||
			tar == this
		) &&
		tar->CanBuffStack(botSpell.SpellId, botLevel, true) < 0
	) {
		return false;
	}

	uint32 TempDontHealMeBeforeTime = tar->DontHealMeBefore();
	if (IsValidSpellRange(botSpell.SpellId, tar)) {
		castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost, &TempDontHealMeBeforeTime);
	}

	if (castedSpell) {
		BotGroupSay(
			this,
			fmt::format(
				"Casting {} on {}, please stay in range!",
				spells[botSpell.SpellId].name,
				tar->GetCleanName()
			).c_str()
		);
	}

	return castedSpell;
}

std::list<BotSpell> Bot::GetBotSpellsForSpellEffect(Bot* botCaster, uint16 spellType, int spellEffect) {
	std::list<BotSpell> result;

	if (!botCaster) {
		return result;
	}

	if (auto bot_owner = botCaster->GetBotOwner(); !bot_owner) {
		return result;
	}

	if (botCaster->AI_HasSpells()) {
		std::vector<BotSpells_Struct> botSpellList = botCaster->AIBot_spells;

		for (int i = botSpellList.size() - 1; i >= 0; i--) {
			if (!IsValidSpellAndLoS(botSpellList[i].spellid, botCaster->HasLoS())) {
				continue;
			}

			if (
				botCaster->CheckSpellRecastTimer(botSpellList[i].spellid) &&
				(botSpellList[i].type == spellType || botSpellList[i].type == botCaster->GetSpellListSpellType(spellType)) &&
				botCaster->IsValidSpellTypeBySpellID(spellType, botSpellList[i].spellid) &&
				(IsEffectInSpell(botSpellList[i].spellid, spellEffect) || GetSpellTriggerSpellID(botSpellList[i].spellid, spellEffect))
			) {
				BotSpell botSpell;
				botSpell.SpellId = botSpellList[i].spellid;
				botSpell.SpellIndex = i;
				botSpell.ManaCost = botSpellList[i].manacost;

				result.push_back(botSpell);
			}
		}
	}

	return result;
}

std::list<BotSpell> Bot::GetBotSpellsForSpellEffectAndTargetType(Bot* botCaster, uint16 spellType, int spellEffect, SpellTargetType targetType) {
	std::list<BotSpell> result;

	if (!botCaster) {
		return result;
	}

	if (auto bot_owner = botCaster->GetBotOwner(); !bot_owner) {
		return result;
	}

	if (botCaster->AI_HasSpells()) {
		std::vector<BotSpells_Struct> botSpellList = botCaster->AIBot_spells;

		for (int i = botSpellList.size() - 1; i >= 0; i--) {
			if (!IsValidSpellAndLoS(botSpellList[i].spellid, botCaster->HasLoS())) {
				continue;
			}

			if (
				botCaster->CheckSpellRecastTimer(botSpellList[i].spellid) && 
				(botSpellList[i].type == spellType || botSpellList[i].type == botCaster->GetSpellListSpellType(spellType)) &&
				botCaster->IsValidSpellTypeBySpellID(spellType, botSpellList[i].spellid) &&
				(
					IsEffectInSpell(botSpellList[i].spellid, spellEffect) ||
					GetSpellTriggerSpellID(botSpellList[i].spellid, spellEffect)
				) &&
				(targetType == ST_TargetOptional || spells[botSpellList[i].spellid].target_type == targetType)
			) {
				BotSpell botSpell;
				botSpell.SpellId = botSpellList[i].spellid;
				botSpell.SpellIndex = i;
				botSpell.ManaCost = botSpellList[i].manacost;
				result.push_back(botSpell);
			}
		}
	}

	return result;
}

std::list<BotSpell> Bot::GetBotSpellsBySpellType(Bot* botCaster, uint16 spellType) {
	std::list<BotSpell> result;

	if (!botCaster) {
		return result;
	}

	if (auto bot_owner = botCaster->GetBotOwner(); !bot_owner) {
		return result;
	}

	if (botCaster->AI_HasSpells()) {
		std::vector<BotSpells_Struct> botSpellList = botCaster->AIBot_spells;

		for (int i = botSpellList.size() - 1; i >= 0; i--) {
			if (!IsValidSpellAndLoS(botSpellList[i].spellid, botCaster->HasLoS())) {
				continue;
			}

			if (
				botCaster->CheckSpellRecastTimer(botSpellList[i].spellid) && 
				(botSpellList[i].type == spellType || botSpellList[i].type == botCaster->GetSpellListSpellType(spellType)) &&
				botCaster->IsValidSpellTypeBySpellID(spellType, botSpellList[i].spellid)
			) {
				BotSpell botSpell;
				botSpell.SpellId = botSpellList[i].spellid;
				botSpell.SpellIndex = i;
				botSpell.ManaCost = botSpellList[i].manacost;

				result.push_back(botSpell);
			}
		}
	}

	return result;
}

std::list<BotSpell_wPriority> Bot::GetPrioritizedBotSpellsBySpellType(Bot* botCaster, uint16 spellType, Mob* tar, bool AE, uint16 subTargetType, uint16 subType) {
	std::list<BotSpell_wPriority> result;

	if (botCaster && botCaster->AI_HasSpells()) {
		std::vector<BotSpells_Struct> botSpellList = botCaster->AIBot_spells;

		for (int i = botSpellList.size() - 1; i >= 0; i--) {
			if (!IsValidSpellAndLoS(botSpellList[i].spellid, botCaster->HasLoS())) {
				continue;
			}

			if (spellType == BotSpellTypes::HateRedux && botCaster->GetClass() == Class::Bard) {
				if (spells[botSpellList[i].spellid].target_type != ST_Target) {
					continue;
				}
			}

			if (
				botCaster->CheckSpellRecastTimer(botSpellList[i].spellid) &&
				(botSpellList[i].type == spellType || botSpellList[i].type == botCaster->GetSpellListSpellType(spellType)) &&
				botCaster->IsValidSpellTypeBySpellID(spellType, botSpellList[i].spellid)
			) {
				if (
					botCaster->IsCommandedSpell() &&
					(
						!botCaster->IsValidSpellTypeSubType(spellType, subTargetType, botSpellList[i].spellid) ||
						!botCaster->IsValidSpellTypeSubType(spellType, subType, botSpellList[i].spellid)
					)
				) {
					continue;
				}

				if (!AE && IsAnyAESpell(botSpellList[i].spellid) && !IsGroupSpell(botSpellList[i].spellid)) {
					continue;
				}
				else if (AE && !IsAnyAESpell(botSpellList[i].spellid)) {
					continue;
				}

				if (
					!RuleB(Bots, EnableBotTGB) &&
					IsGroupSpell(botSpellList[i].spellid) &&
					!IsTGBCompatibleSpell(botSpellList[i].spellid) &&
					!botCaster->IsInGroupOrRaid(tar, true)
				) {					
					continue;
				}

				if (!IsPBAESpell(botSpellList[i].spellid) && !botCaster->CastChecks(botSpellList[i].spellid, tar, spellType, false, IsAEBotSpellType(spellType))) {					
					continue;
				}

				if (
					botCaster->IsCommandedSpell() || 
					!AE ||
					(
						SpellTypeRequiresAEChecks(spellType) && 
						botCaster->HasValidAETarget(botCaster, botSpellList[i].spellid, spellType, tar)
					)
				) {
					BotSpell_wPriority botSpell;
					botSpell.SpellId = botSpellList[i].spellid;
					botSpell.SpellIndex = i;
					botSpell.ManaCost = botSpellList[i].manacost;
					botSpell.Priority = botSpellList[i].priority;

					result.push_back(botSpell);
				}
			}
		}

		if (result.size() > 1) {
			result.sort(
				[](BotSpell_wPriority const& l, BotSpell_wPriority const& r) {
					return l.Priority < r.Priority;
				}
			);
		}
	}

	return result;
}

BotSpell Bot::GetFirstBotSpellBySpellType(Bot* botCaster, uint16 spellType) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster && botCaster->AI_HasSpells()) {
		std::vector<BotSpells_Struct> botSpellList = botCaster->AIBot_spells;

		for (int i = botSpellList.size() - 1; i >= 0; i--) {
			if (!IsValidSpellAndLoS(botSpellList[i].spellid, botCaster->HasLoS())) {
				continue;
			}

			if (
				botCaster->CheckSpellRecastTimer(botSpellList[i].spellid) &&
				(botSpellList[i].type == spellType || botSpellList[i].type == botCaster->GetSpellListSpellType(spellType)) &&
				botCaster->IsValidSpellTypeBySpellID(spellType, botSpellList[i].spellid)
			) {
				result.SpellId = botSpellList[i].spellid;
				result.SpellIndex = i;
				result.ManaCost = botSpellList[i].manacost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForVeryFastHeal(Bot* botCaster, Mob* tar, uint16 spellType) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, spellType, SE_CurrentHP);

		for (auto botSpellListItr : botSpellList) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (
				IsVeryFastHealSpell(botSpellListItr.SpellId) && botCaster->CastChecks(botSpellListItr.SpellId, tar, spellType)) {
				result.SpellId = botSpellListItr.SpellId;
				result.SpellIndex = botSpellListItr.SpellIndex;
				result.ManaCost = botSpellListItr.ManaCost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForFastHeal(Bot *botCaster, Mob* tar, uint16 spellType) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, spellType, SE_CurrentHP);

		for (auto botSpellListItr : botSpellList) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (IsFastHealSpell(botSpellListItr.SpellId) && botCaster->CastChecks(botSpellListItr.SpellId, tar, spellType)) {
				result.SpellId = botSpellListItr.SpellId;
				result.SpellIndex = botSpellListItr.SpellIndex;
				result.ManaCost = botSpellListItr.ManaCost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForHealOverTime(Bot* botCaster, Mob* tar, uint16 spellType) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, spellType, SE_HealOverTime);

		for (auto botSpellListItr : botSpellList) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (IsHealOverTimeSpell(botSpellListItr.SpellId) && botCaster->CastChecks(botSpellListItr.SpellId, tar, spellType)) {
				result.SpellId = botSpellListItr.SpellId;
				result.SpellIndex = botSpellListItr.SpellIndex;
				result.ManaCost = botSpellListItr.ManaCost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForPercentageHeal(Bot *botCaster, Mob* tar, uint16 spellType) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster && botCaster->AI_HasSpells()) {
		std::vector<BotSpells_Struct> botSpellList = botCaster->AIBot_spells;
		for (int i = botSpellList.size() - 1; i >= 0; i--) {
			if (!IsValidSpell(botSpellList[i].spellid)) {
				continue;
			}

			if (
				(botSpellList[i].type == spellType || botSpellList[i].type == botCaster->GetSpellListSpellType(spellType)) &&
				botCaster->IsValidSpellTypeBySpellID(spellType, botSpellList[i].spellid) &&
				IsCompleteHealSpell(botSpellList[i].spellid) &&
				botCaster->CastChecks(botSpellList[i].spellid, tar, spellType)
			) {
				result.SpellId = botSpellList[i].spellid;
				result.SpellIndex = i;
				result.ManaCost = botSpellList[i].manacost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForRegularSingleTargetHeal(Bot* botCaster, Mob* tar, uint16 spellType) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, spellType, SE_CurrentHP);

		for (std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (IsRegularSingleTargetHealSpell(botSpellListItr->SpellId) && botCaster->CastChecks(botSpellListItr->SpellId, tar, spellType)) {
				result.SpellId = botSpellListItr->SpellId;
				result.SpellIndex = botSpellListItr->SpellIndex;
				result.ManaCost = botSpellListItr->ManaCost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetFirstBotSpellForSingleTargetHeal(Bot* botCaster, Mob* tar, uint16 spellType) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, spellType, SE_CurrentHP);

		for (std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (IsRegularSingleTargetHealSpell(botSpellListItr->SpellId) && botCaster->CastChecks(botSpellListItr->SpellId, tar, spellType)) {
				result.SpellId = botSpellListItr->SpellId;
				result.SpellIndex = botSpellListItr->SpellIndex;
				result.ManaCost = botSpellListItr->ManaCost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForGroupHeal(Bot* botCaster, Mob* tar, uint16 spellType) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, spellType, SE_CurrentHP);
		std::vector<Mob*> v;

		if (RuleB(Bots, CrossRaidBuffingAndHealing)) {
			v = botCaster->GatherSpellTargets(true);
		}
		else {
			v = botCaster->GatherGroupSpellTargets();
		}

		int targetCount = 0;

		for (std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (IsRegularGroupHealSpell(botSpellListItr->SpellId)) {
				if (!botCaster->IsCommandedSpell()) {
					targetCount = 0;

					for (Mob* m : v) {
						if (botCaster->IsValidSpellRange(botSpellListItr->SpellId, m) && botCaster->CastChecks(botSpellListItr->SpellId, m, spellType, true, IsGroupBotSpellType(spellType))) {
							++targetCount;
						}
					}

					if (targetCount < botCaster->GetSpellTypeAEOrGroupTargetCount(spellType)) {
						continue;
					}
				}

				result.SpellId = botSpellListItr->SpellId;
				result.SpellIndex = botSpellListItr->SpellIndex;
				result.ManaCost = botSpellListItr->ManaCost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForGroupHealOverTime(Bot* botCaster, Mob* tar, uint16 spellType) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, spellType, SE_HealOverTime);
		std::vector<Mob*> v;

		if (RuleB(Bots, CrossRaidBuffingAndHealing)) {
			v = botCaster->GatherSpellTargets(true);
		}
		else {
			v = botCaster->GatherGroupSpellTargets();
		}

		int targetCount = 0;

		for (std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (IsGroupHealOverTimeSpell(botSpellListItr->SpellId)) {
				if (!botCaster->IsCommandedSpell()) {
					targetCount = 0;

					for (Mob* m : v) {
						if (botCaster->IsValidSpellRange(botSpellListItr->SpellId, m) && botCaster->CastChecks(botSpellListItr->SpellId, m, spellType, true, IsGroupBotSpellType(spellType))) {
							++targetCount;
						}
					}

					if (targetCount < botCaster->GetSpellTypeAEOrGroupTargetCount(spellType)) {
						continue;
					}
				}

				result.SpellId = botSpellListItr->SpellId;
				result.SpellIndex = botSpellListItr->SpellIndex;
				result.ManaCost = botSpellListItr->ManaCost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForGroupCompleteHeal(Bot* botCaster, Mob* tar, uint16 spellType) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, spellType, SE_CompleteHeal);
		std::vector<Mob*> v;

		if (RuleB(Bots, CrossRaidBuffingAndHealing)) {
			v = botCaster->GatherSpellTargets(true);
		}
		else {
			v = botCaster->GatherGroupSpellTargets();
		}

		int targetCount = 0;

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (IsGroupCompleteHealSpell(botSpellListItr->SpellId)) {
				if (!botCaster->IsCommandedSpell()) {
					targetCount = 0;

					for (Mob* m : v) {
						if (botCaster->IsValidSpellRange(botSpellListItr->SpellId, m) && botCaster->CastChecks(botSpellListItr->SpellId, m, spellType, true, IsGroupBotSpellType(spellType))) {
							++targetCount;
						}
					}

					if (targetCount < botCaster->GetSpellTypeAEOrGroupTargetCount(spellType)) {
						continue;
					}
				}

				result.SpellId = botSpellListItr->SpellId;
				result.SpellIndex = botSpellListItr->SpellIndex;
				result.ManaCost = botSpellListItr->ManaCost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForMez(Bot* botCaster, uint16 spellType) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, spellType, SE_Mez);

		for (std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (
				IsMesmerizeSpell(botSpellListItr->SpellId) &&
				botCaster->CheckSpellRecastTimer(botSpellListItr->SpellId)
			) {
				result.SpellId = botSpellListItr->SpellId;
				result.SpellIndex = botSpellListItr->SpellIndex;
				result.ManaCost = botSpellListItr->ManaCost;

				break;
			}
		}
	}

	return result;
}

Mob* Bot::GetFirstIncomingMobToMez(Bot* botCaster, int16 spell_id, uint16 spellType, bool AE) {
	Mob* result = nullptr;

	if (botCaster && botCaster->GetOwner()) {
		int spellRange = (!AE ? botCaster->GetActSpellRange(spell_id, spells[spell_id].range) : botCaster->GetActSpellRange(spell_id, spells[spell_id].aoe_range));
		int buff_count = 0;
		NPC* npc = nullptr;

		for (auto& close_mob : botCaster->m_close_mobs) {
			buff_count = 0;
			npc = close_mob.second->CastToNPC();

			if (!npc) {
				continue;
			}

			if (!botCaster->IsValidMezTarget(botCaster->GetOwner(), npc, spell_id)) {
				continue;
			}

			if (AE) {
				int targetCount = 0;

				for (auto& close_mob : botCaster->m_close_mobs) {
					Mob* m = close_mob.second;

					if (npc == m) {
						continue;
					}

					if (!botCaster->IsValidMezTarget(botCaster->GetOwner(), m, spell_id)) {
						continue;
					}

					if (IsPBAESpell(spell_id)) {
						if (spellRange < Distance(botCaster->GetPosition(), m->GetPosition())) {
							continue;							
						}
					}
					else {
						if (spellRange < Distance(m->GetPosition(), npc->GetPosition())) {
							continue;
						}
					}

					if (botCaster->CastChecks(spell_id, m, spellType, true, true)) {
						++targetCount;
					}

					if (targetCount >= botCaster->GetSpellTypeAEOrGroupTargetCount(spellType)) {
						break;
					}
				}

				if (targetCount < botCaster->GetSpellTypeAEOrGroupTargetCount(spellType)) {
					continue;
				}

				if (zone->random.Int(1, 100) < RuleI(Bots, AEMezChance)) {
					botCaster->SetSpellTypeRecastTimer(spellType, RuleI(Bots, MezFailDelay));
					return result;
				}

				result = npc;
			}
			else {
				if (spellRange < Distance(botCaster->GetPosition(), npc->GetPosition())) {
					continue;
				}

				if (!botCaster->CastChecks(spell_id, npc, spellType, true)) {
					continue;
				}

				if (zone->random.Int(1, 100) < RuleI(Bots, MezChance)) {
					botCaster->SetSpellTypeRecastTimer(spellType, RuleI(Bots, MezAEFailDelay));

					return result;
				}

				result = npc;
			}

			if (result) {
				botCaster->SetHasLoS(true);

				return result;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotMagicianPetSpell(Bot *botCaster, uint16 spellType) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, spellType, SE_SummonPet);
		std::string petType = GetBotMagicianPetType(botCaster);

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (
				IsSummonPetSpell(botSpellListItr->SpellId) &&
				botCaster->CheckSpellRecastTimer(botSpellListItr->SpellId) &&
				!strncmp(spells[botSpellListItr->SpellId].teleport_zone, petType.c_str(), petType.length())
			) {
					result.SpellId = botSpellListItr->SpellId;
					result.SpellIndex = botSpellListItr->SpellIndex;
					result.ManaCost = botSpellListItr->ManaCost;

					break;
			}
		}
	}

	return result;
}

std::string Bot::GetBotMagicianPetType(Bot* botCaster) {
	std::string result;

	if (botCaster) {
		bool epicAllowed = false;
		if (RuleB(Bots, AllowMagicianEpicPet)) {
			if (botCaster->GetLevel() >= RuleI(Bots, AllowMagicianEpicPetLevel)) {
				if (!RuleI(Bots, RequiredMagicianEpicPetItemID)) {
					epicAllowed = true;
				}
				else {
					bool has_item = botCaster->HasBotItem(RuleI(Bots, RequiredMagicianEpicPetItemID)) != INVALID_INDEX;

					if (has_item) {
						epicAllowed = true;
					}
				}
			}
		}

		if (botCaster->GetPetChooserID() > 0) {
			switch (botCaster->GetPetChooserID()) {
				case SumWater:
					result = std::string("SumWater");
					break;
				case SumFire:
					result = std::string("SumFire");
					break;
				case SumAir:
					result = std::string("SumAir");
					break;
				case SumEarth:
					result = std::string("SumEarth");
					break;
				case MonsterSum:
					result = std::string("MonsterSum");
					break;
				case SumMageMultiElement:
					if (epicAllowed) {
						result = std::string(RuleS(Bots, EpicPetSpellName));

						if (result.empty()) {
							result = "SumMageMultiElement";
						}
					}
					else {
						result = std::string("MonsterSum");
					}
					break;
			}
		}
		else {
			if (epicAllowed) {
				result = std::string(RuleS(Bots, EpicPetSpellName));

				if (result.empty()) {
					result = "SumMageMultiElement";
				}
			}
			else {
				if (botCaster->GetLevel() == 2) {
					result = std::string("SumWater");
				}
				else if (botCaster->GetLevel() == 3) {
					result = std::string("SumFire");
				}
				else if (botCaster->GetLevel() == 4) {
					result = std::string("SumAir");
				}
				else if (botCaster->GetLevel() == 5) {
					result = std::string("SumEarth");
				}
				else {
					int counter;

					if (botCaster->GetLevel() < 30) {
						counter = zone->random.Int(1, 4);
					}
					else {
						counter = zone->random.Int(1, 5);
					}

					switch (counter) {
						case 1:
							result = std::string("SumWater");
							break;
						case 2:
							result = std::string("SumFire");
							break;
						case 3:
							result = std::string("SumAir");
							break;
						case 4:
							result = std::string("SumEarth");
							break;
						default:
							result = std::string("MonsterSum");
							break;
					}
				}
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForNukeByTargetType(Bot* botCaster, SpellTargetType targetType, uint16 spellType, bool AE, Mob* tar) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (tar == nullptr) {
		tar = botCaster->GetTarget();
	}

	if (botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffectAndTargetType(botCaster, spellType, SE_CurrentHP, targetType);

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (IsPureNukeSpell(botSpellListItr->SpellId) || IsDamageSpell(botSpellListItr->SpellId)) {
				if (!AE && IsAnyAESpell(botSpellListItr->SpellId) && !IsGroupSpell(botSpellListItr->SpellId)) {
					continue;
				}
				else if (AE && !IsAnyAESpell(botSpellListItr->SpellId)) {
					continue;
				}

				if (!IsPBAESpell(botSpellListItr->SpellId) && !botCaster->CastChecks(botSpellListItr->SpellId, tar, spellType, false, IsAEBotSpellType(spellType))) {
					continue;
				}


				if (
					botCaster->IsCommandedSpell() ||
					!AE ||
					(AE && botCaster->HasValidAETarget(botCaster, botSpellListItr->SpellId, spellType, tar))
				) {
					result.SpellId = botSpellListItr->SpellId;
					result.SpellIndex = botSpellListItr->SpellIndex;
					result.ManaCost = botSpellListItr->ManaCost;

					break;
				}
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForStunByTargetType(Bot* botCaster, SpellTargetType targetType, uint16 spellType, bool AE, Mob* tar)
{
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (tar == nullptr) {
		tar = botCaster->GetTarget();
	}

	if (botCaster)
	{
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffectAndTargetType(botCaster, spellType, SE_Stun, targetType);

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr)
		{
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (IsStunSpell(botSpellListItr->SpellId)) {
				if (!AE && IsAnyAESpell(botSpellListItr->SpellId) && !IsGroupSpell(botSpellListItr->SpellId)) {
					continue;
				}
				else if (AE && !IsAnyAESpell(botSpellListItr->SpellId)) {
					continue;
				}

				if (!IsPBAESpell(botSpellListItr->SpellId) && !botCaster->CastChecks(botSpellListItr->SpellId, tar, spellType, false, IsAEBotSpellType(spellType))) {
					continue;
				}

				if (
					botCaster->IsCommandedSpell() ||
					!AE ||
					(AE && botCaster->HasValidAETarget(botCaster, botSpellListItr->SpellId, spellType, tar))
				) {
					result.SpellId = botSpellListItr->SpellId;
					result.SpellIndex = botSpellListItr->SpellIndex;
					result.ManaCost = botSpellListItr->ManaCost;

					break;
				}
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotWizardNukeSpellByTargetResists(Bot* botCaster, Mob* target, uint16 spellType) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster && target) {

		const int lureResisValue = -100;

		int32 level_mod = (target->GetLevel() - botCaster->GetLevel()) * (target->GetLevel() - botCaster->GetLevel()) / 2;

		if (target->GetLevel() - botCaster->GetLevel() < 0) {
			level_mod = -level_mod;
		}
		const int maxTargetResistValue = botCaster->GetSpellTypeResistLimit(spellType);
		bool selectLureNuke = false;

		if (((target->GetMR() + level_mod) > maxTargetResistValue) && ((target->GetCR() + level_mod) > maxTargetResistValue) && ((target->GetFR() + level_mod) > maxTargetResistValue)) {
			selectLureNuke = true;
		}


		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffectAndTargetType(botCaster, spellType, SE_CurrentHP, ST_Target);

		BotSpell firstWizardMagicNukeSpellFound;
		firstWizardMagicNukeSpellFound.SpellId = 0;
		firstWizardMagicNukeSpellFound.SpellIndex = 0;
		firstWizardMagicNukeSpellFound.ManaCost = 0;
		bool spellSelected = false;

		for (std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (!botCaster->IsValidSpellRange(botSpellListItr->SpellId, target)) {
				continue;
			}

			if (selectLureNuke && (spells[botSpellListItr->SpellId].resist_difficulty < lureResisValue)) {
				if (botCaster->CastChecks(botSpellListItr->SpellId, target, spellType)) {
					spellSelected = true;
				}
			}
			else if (!selectLureNuke && IsPureNukeSpell(botSpellListItr->SpellId)) {
				if (
					((target->GetMR() < target->GetCR()) || (target->GetMR() < target->GetFR())) && 
					(GetSpellResistType(botSpellListItr->SpellId) == RESIST_MAGIC) && 
					(spells[botSpellListItr->SpellId].resist_difficulty > lureResisValue) &&
					botCaster->CastChecks(botSpellListItr->SpellId, target, spellType)
				) {
					spellSelected = true;
				}
				else if (
					((target->GetCR() < target->GetMR()) || (target->GetCR() < target->GetFR())) && 
					(GetSpellResistType(botSpellListItr->SpellId) == RESIST_COLD) && 
					(spells[botSpellListItr->SpellId].resist_difficulty > lureResisValue) && 
					botCaster->CastChecks(botSpellListItr->SpellId, target, spellType)
				) {
					spellSelected = true;
				}
				else if (
					((target->GetFR() < target->GetCR()) || (target->GetFR() < target->GetMR())) && 
					(GetSpellResistType(botSpellListItr->SpellId) == RESIST_FIRE) && 
					(spells[botSpellListItr->SpellId].resist_difficulty > lureResisValue) && 
					botCaster->CastChecks(botSpellListItr->SpellId, target, spellType)
				) {
					spellSelected = true;
				}
				else if (
					(GetSpellResistType(botSpellListItr->SpellId) == RESIST_MAGIC) && 
					(spells[botSpellListItr->SpellId].resist_difficulty > lureResisValue) && 
					botCaster->CastChecks(botSpellListItr->SpellId, target, spellType)
				) {
					firstWizardMagicNukeSpellFound.SpellId = botSpellListItr->SpellId;
					firstWizardMagicNukeSpellFound.SpellIndex = botSpellListItr->SpellIndex;
					firstWizardMagicNukeSpellFound.ManaCost = botSpellListItr->ManaCost;
				}
			}

			if (spellSelected) {
				result.SpellId = botSpellListItr->SpellId;
				result.SpellIndex = botSpellListItr->SpellIndex;
				result.ManaCost = botSpellListItr->ManaCost;

				break;
			}
		}

		if (!spellSelected) {
			for (std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
				// Assuming all the spells have been loaded into this list by level and in descending order

				if (botCaster->CheckSpellRecastTimer(botSpellListItr->SpellId)) {
					if (botCaster->CastChecks(botSpellListItr->SpellId, target, spellType)) {
						spellSelected = true;
					}
				}
				if (spellSelected) {
					result.SpellId = botSpellListItr->SpellId;
					result.SpellIndex = botSpellListItr->SpellIndex;
					result.ManaCost = botSpellListItr->ManaCost;

					break;
				}
			}
		}

		if (result.SpellId == 0) {
			result = firstWizardMagicNukeSpellFound;
		}
	}

	return result;
}

BotSpell Bot::GetDebuffBotSpell(Bot* botCaster, Mob *tar) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (!tar || !botCaster)
		return result;

	if (botCaster->AI_HasSpells()) {
		std::vector<BotSpells_Struct> botSpellList = botCaster->AIBot_spells;

		for (int i = botSpellList.size() - 1; i >= 0; i--) {
			if (!IsValidSpellAndLoS(botSpellList[i].spellid, botCaster->HasLoS())) {
				continue;
			}

			if (((botSpellList[i].type == BotSpellTypes::Debuff) || IsDebuffSpell(botSpellList[i].spellid))
				&& (!tar->IsImmuneToSpell(botSpellList[i].spellid, botCaster)
				&& tar->CanBuffStack(botSpellList[i].spellid, botCaster->GetLevel(), true) >= 0)
				&& botCaster->CheckSpellRecastTimer(botSpellList[i].spellid)) {
				result.SpellId = botSpellList[i].spellid;
				result.SpellIndex = i;
				result.ManaCost = botSpellList[i].manacost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForResistDebuff(Bot* botCaster, Mob *tar) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (!tar || !botCaster) {
		return result;
	}

	int level_mod = (tar->GetLevel() - botCaster->GetLevel())* (tar->GetLevel() - botCaster->GetLevel()) / 2;
	if (tar->GetLevel() - botCaster->GetLevel() < 0) {
		level_mod = -level_mod;
	}

	bool needsMagicResistDebuff = (tar->GetMR() + level_mod) > 100;
	bool needsColdResistDebuff = (tar->GetCR() + level_mod) > 100;
	bool needsFireResistDebuff = (tar->GetFR() + level_mod) > 100;
	bool needsPoisonResistDebuff = (tar->GetPR() + level_mod) > 100;
	bool needsDiseaseResistDebuff = (tar->GetDR() + level_mod) > 100;

	if (botCaster->AI_HasSpells()) {
		std::vector<BotSpells_Struct> botSpellList = botCaster->AIBot_spells;

		for (int i = botSpellList.size() - 1; i >= 0; i--) {
			if (!IsValidSpellAndLoS(botSpellList[i].spellid, botCaster->HasLoS())) {
				continue;
			}

			if (((botSpellList[i].type == BotSpellTypes::Debuff) || IsResistDebuffSpell(botSpellList[i].spellid))
				&& ((needsMagicResistDebuff && (IsEffectInSpell(botSpellList[i].spellid, SE_ResistMagic)) || IsEffectInSpell(botSpellList[i].spellid, SE_ResistAll))
				|| (needsColdResistDebuff && (IsEffectInSpell(botSpellList[i].spellid, SE_ResistCold)) || IsEffectInSpell(botSpellList[i].spellid, SE_ResistAll))
				|| (needsFireResistDebuff && (IsEffectInSpell(botSpellList[i].spellid, SE_ResistFire)) || IsEffectInSpell(botSpellList[i].spellid, SE_ResistAll))
				|| (needsPoisonResistDebuff && (IsEffectInSpell(botSpellList[i].spellid, SE_ResistPoison)) || IsEffectInSpell(botSpellList[i].spellid, SE_ResistAll))
				|| (needsDiseaseResistDebuff && (IsEffectInSpell(botSpellList[i].spellid, SE_ResistDisease)) || IsEffectInSpell(botSpellList[i].spellid, SE_ResistAll)))
				&& (!tar->IsImmuneToSpell(botSpellList[i].spellid, botCaster)
				&& tar->CanBuffStack(botSpellList[i].spellid, botCaster->GetLevel(), true) >= 0)
				&& botCaster->CheckSpellRecastTimer(botSpellList[i].spellid)) {
				result.SpellId = botSpellList[i].spellid;
				result.SpellIndex = i;
				result.ManaCost = botSpellList[i].manacost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForCure(Bot* botCaster, Mob* tar, uint16 spellType) {
	BotSpell_wPriority result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (!tar) {
		return result;
	}

	if (botCaster) {
		std::list<BotSpell_wPriority> botSpellListItr = GetPrioritizedBotSpellsBySpellType(botCaster, spellType, tar);
		
		if (IsGroupBotSpellType(spellType)) {
			const std::vector<Mob*> v = botCaster->GatherGroupSpellTargets(tar);
			int countNeedsCured = 0;
			uint16 countPoisoned = 0;
			uint16 countDiseased = 0;
			uint16 countCursed = 0;
			uint16 countCorrupted = 0;

			for (std::list<BotSpell_wPriority>::iterator itr = botSpellListItr.begin(); itr != botSpellListItr.end(); ++itr) {
				if (!IsValidSpell(itr->SpellId) || !IsGroupSpell(itr->SpellId)) {
					continue;
				}

				for (Mob* m : v) {
					if (botCaster->IsCommandedSpell() || botCaster->GetNeedsCured(m)) {
						if (botCaster->CastChecks(itr->SpellId, m, spellType, true, IsGroupBotSpellType(spellType))) {
							if (m->FindType(SE_PoisonCounter)) {
								++countPoisoned;
							}
							if (m->FindType(SE_DiseaseCounter)) {
								++countDiseased;
							}
							if (m->FindType(SE_CurseCounter)) {
								++countCursed;
							}
							if (m->FindType(SE_CorruptionCounter)) {
								++countCorrupted;
							}
						}
					}
				}

				if (
					(countPoisoned >= botCaster->GetSpellTypeAEOrGroupTargetCount(spellType) && IsEffectInSpell(itr->SpellId, SE_PoisonCounter)) ||
					(countDiseased >= botCaster->GetSpellTypeAEOrGroupTargetCount(spellType) && IsEffectInSpell(itr->SpellId, SE_DiseaseCounter)) ||
					(countCursed >= botCaster->GetSpellTypeAEOrGroupTargetCount(spellType) && IsEffectInSpell(itr->SpellId, SE_CurseCounter)) ||
					(countCorrupted >= botCaster->GetSpellTypeAEOrGroupTargetCount(spellType) && IsEffectInSpell(itr->SpellId, SE_CorruptionCounter))
				) {
					result.SpellId = itr->SpellId;
					result.SpellIndex = itr->SpellIndex;
					result.ManaCost = itr->ManaCost;

					break;
				}
			}
		}
		else {
			for (std::list<BotSpell_wPriority>::iterator itr = botSpellListItr.begin(); itr != botSpellListItr.end(); ++itr) {
				if (!IsValidSpell(itr->SpellId) || IsGroupSpell(itr->SpellId)) {
					continue;
				}

				if (
					tar->FindType(SE_PoisonCounter) && IsEffectInSpell(itr->SpellId, SE_PoisonCounter) ||
					tar->FindType(SE_DiseaseCounter) && IsEffectInSpell(itr->SpellId, SE_DiseaseCounter) ||
					tar->FindType(SE_CurseCounter) && IsEffectInSpell(itr->SpellId, SE_CurseCounter) ||
					tar->FindType(SE_CorruptionCounter) && IsEffectInSpell(itr->SpellId, SE_CorruptionCounter)
				) {
					result.SpellId = itr->SpellId;
					result.SpellIndex = itr->SpellIndex;
					result.ManaCost = itr->ManaCost;
					break;
				}
			}
		}
	}

	return result;
}

uint8 Bot::GetChanceToCastBySpellType(uint16 spellType)
{
	switch (spellType) {
		case BotSpellTypes::AENukes:
		case BotSpellTypes::AERains:
		case BotSpellTypes::AEStun:
		case BotSpellTypes::AESnare:
		case BotSpellTypes::AEMez:
		case BotSpellTypes::AESlow:
		case BotSpellTypes::AEDebuff:
		case BotSpellTypes::AEFear:
		case BotSpellTypes::AEDispel:
		case BotSpellTypes::AEDoT:
		case BotSpellTypes::AELifetap:
		case BotSpellTypes::AERoot:
		case BotSpellTypes::PBAENuke:
		case BotSpellTypes::AEHateLine:
			return RuleI(Bots, PercentChanceToCastAEs);
		case BotSpellTypes::GroupHeals:
		case BotSpellTypes::GroupCompleteHeals:
		case BotSpellTypes::GroupHoTHeals:
		case BotSpellTypes::GroupCures:
			return RuleI(Bots, PercentChanceToCastGroupHeal);
		case BotSpellTypes::Nuke:
			return RuleI(Bots, PercentChanceToCastNuke);
		case BotSpellTypes::Root:
			return RuleI(Bots, PercentChanceToCastRoot);
		case BotSpellTypes::Buff:
		case BotSpellTypes::PetBuffs:
		case BotSpellTypes::ResistBuffs:
		case BotSpellTypes::PetResistBuffs:
		case BotSpellTypes::DamageShields:
		case BotSpellTypes::PetDamageShields:
			return RuleI(Bots, PercentChanceToCastBuff);
		case BotSpellTypes::Escape:
			return RuleI(Bots, PercentChanceToCastEscape);
		case BotSpellTypes::Lifetap:
			return RuleI(Bots, PercentChanceToCastLifetap);
		case BotSpellTypes::Snare:
			return RuleI(Bots, PercentChanceToCastSnare);
		case BotSpellTypes::DOT:
			return RuleI(Bots, PercentChanceToCastDOT);
		case BotSpellTypes::Dispel:
			return RuleI(Bots, PercentChanceToCastDispel);
		case BotSpellTypes::InCombatBuff:
			return RuleI(Bots, PercentChanceToCastInCombatBuff);		
		case BotSpellTypes::HateLine:
			return RuleI(Bots, PercentChanceToCastHateLine);
		case BotSpellTypes::Mez:
			return RuleI(Bots, PercentChanceToCastMez);		
		case BotSpellTypes::Slow:
			return RuleI(Bots, PercentChanceToCastSlow);		
		case BotSpellTypes::Debuff:
			return RuleI(Bots, PercentChanceToCastDebuff);
		case BotSpellTypes::Cure:
			return RuleI(Bots, PercentChanceToCastCure);
		case BotSpellTypes::HateRedux:
			return RuleI(Bots, PercentChanceToCastHateRedux);		
		case BotSpellTypes::Fear:
			return RuleI(Bots, PercentChanceToCastFear);
		case BotSpellTypes::RegularHeal:
		case BotSpellTypes::CompleteHeal:
		case BotSpellTypes::FastHeals:
		case BotSpellTypes::VeryFastHeals:
		case BotSpellTypes::HoTHeals:
		case BotSpellTypes::PetRegularHeals:
		case BotSpellTypes::PetCompleteHeals:
		case BotSpellTypes::PetFastHeals:
		case BotSpellTypes::PetVeryFastHeals:
		case BotSpellTypes::PetHoTHeals:
			return RuleI(Bots, PercentChanceToCastHeal);
		default:
			return RuleI(Bots, PercentChanceToCastOtherType);
	}

	return RuleI(Bots, PercentChanceToCastOtherType);
}

bool Bot::AI_AddBotSpells(uint32 bot_spell_id) {
	// ok, this function should load the list, and the parent list then shove them into the struct and sort
	npc_spells_id = bot_spell_id;
	AIBot_spells.clear();
	AIBot_spells_enforced.clear();
	if (!bot_spell_id) {
		AIautocastspell_timer->Disable();
		return false;
	}

	auto* spell_list = content_db.GetBotSpells(bot_spell_id);
	if (!spell_list) {
		AIautocastspell_timer->Disable();
		return false;
	}

	auto* parentlist = content_db.GetBotSpells(spell_list->parent_list);

	auto debug_msg = fmt::format(
		"Loading Bot spells onto {}: dbspellsid={}, level={}",
		GetName(),
		bot_spell_id,
		GetLevel()
	);

	debug_msg.append(
		fmt::format(
			" (found, {})",
			spell_list->entries.size()
		)
	);

	LogAI("[{}]", debug_msg);
	for (const auto &iter: spell_list->entries) {
		LogAIDetail("([{}]) [{}]", iter.spellid, spells[iter.spellid].name);
	}

	LogAI("fin (spell list)");

	uint16 attack_proc_spell = -1;
	int8 proc_chance = 3;
	uint16 range_proc_spell = -1;
	int16 rproc_chance = 0;
	uint16 defensive_proc_spell = -1;
	int16 dproc_chance = 0;
	uint32 _fail_recast = 0;
	uint32 _engaged_no_sp_recast_min = 0;
	uint32 _engaged_no_sp_recast_max = 0;
	uint8 _engaged_beneficial_self_chance = 0;
	uint8 _engaged_beneficial_other_chance = 0;
	uint8 _engaged_detrimental_chance = 0;
	uint32 _pursue_no_sp_recast_min = 0;
	uint32 _pursue_no_sp_recast_max = 0;
	uint8 _pursue_detrimental_chance = 0;
	uint32 _idle_no_sp_recast_min = 0;
	uint32 _idle_no_sp_recast_max = 0;
	uint8 _idle_beneficial_chance = 0;

	if (parentlist) {
		attack_proc_spell = parentlist->attack_proc;
		proc_chance = parentlist->proc_chance;
		range_proc_spell = parentlist->range_proc;
		rproc_chance = parentlist->rproc_chance;
		defensive_proc_spell = parentlist->defensive_proc;
		dproc_chance = parentlist->dproc_chance;
		_fail_recast = parentlist->fail_recast;
		_engaged_no_sp_recast_min = parentlist->engaged_no_sp_recast_min;
		_engaged_no_sp_recast_max = parentlist->engaged_no_sp_recast_max;
		_engaged_beneficial_self_chance = parentlist->engaged_beneficial_self_chance;
		_engaged_beneficial_other_chance = parentlist->engaged_beneficial_other_chance;
		_engaged_detrimental_chance = parentlist->engaged_detrimental_chance;
		_pursue_no_sp_recast_min = parentlist->pursue_no_sp_recast_min;
		_pursue_no_sp_recast_max = parentlist->pursue_no_sp_recast_max;
		_pursue_detrimental_chance = parentlist->pursue_detrimental_chance;
		_idle_no_sp_recast_min = parentlist->idle_no_sp_recast_min;
		_idle_no_sp_recast_max = parentlist->idle_no_sp_recast_max;
		_idle_beneficial_chance = parentlist->idle_beneficial_chance;
		for (auto &e : parentlist->entries) {
			if (
				EQ::ValueWithin(GetLevel(), e.minlevel, e.maxlevel) &&
				e.spellid &&
				!IsSpellInBotList(spell_list, e.spellid)
			) {
				if (!e.bucket_name.empty() && !e.bucket_value.empty()) {
					if (!CheckDataBucket(e.bucket_name, e.bucket_value, e.bucket_comparison)) {
						continue;
					}
				}

				const auto& bs = GetBotSpellSetting(e.spellid);
				if (bs) {
					if (!bs->is_enabled) {
						continue;
					}

					AddSpellToBotList(
						bs->priority,
						e.spellid,
						e.type,
						e.manacost,
						e.recast_delay,
						e.resist_adjust,
						e.minlevel,
						e.maxlevel,
						bs->min_hp,
						bs->max_hp,
						e.bucket_name,
						e.bucket_value,
						e.bucket_comparison
					);
					continue;
				}

				if (!GetBotEnforceSpellSetting()) {
					AddSpellToBotList(
						e.priority,
						e.spellid,
						e.type,
						e.manacost,
						e.recast_delay,
						e.resist_adjust,
						e.minlevel,
						e.maxlevel,
						e.min_hp,
						e.max_hp,
						e.bucket_name,
						e.bucket_value,
						e.bucket_comparison
					);
				} else {
					AddSpellToBotEnforceList(
						e.priority,
						e.spellid,
						e.type,
						e.manacost,
						e.recast_delay,
						e.resist_adjust,
						e.minlevel,
						e.maxlevel,
						e.min_hp,
						e.max_hp,
						e.bucket_name,
						e.bucket_value,
						e.bucket_comparison
					);
				}
			}
		}
	}

	attack_proc_spell = spell_list->attack_proc;
	proc_chance = spell_list->proc_chance;

	range_proc_spell = spell_list->range_proc;
	rproc_chance = spell_list->rproc_chance;

	defensive_proc_spell = spell_list->defensive_proc;
	dproc_chance = spell_list->dproc_chance;

	//If any casting variables are defined in the current list, ignore those in the parent list.
	if (
		spell_list->fail_recast ||
		spell_list->engaged_no_sp_recast_min ||
		spell_list->engaged_no_sp_recast_max ||
		spell_list->engaged_beneficial_self_chance ||
		spell_list->engaged_beneficial_other_chance ||
		spell_list->engaged_detrimental_chance ||
		spell_list->pursue_no_sp_recast_min ||
		spell_list->pursue_no_sp_recast_max ||
		spell_list->pursue_detrimental_chance ||
		spell_list->idle_no_sp_recast_min ||
		spell_list->idle_no_sp_recast_max ||
		spell_list->idle_beneficial_chance
	) {
		_fail_recast = spell_list->fail_recast;
		_engaged_no_sp_recast_min = spell_list->engaged_no_sp_recast_min;
		_engaged_no_sp_recast_max = spell_list->engaged_no_sp_recast_max;
		_engaged_beneficial_self_chance = spell_list->engaged_beneficial_self_chance;
		_engaged_beneficial_other_chance = spell_list->engaged_beneficial_other_chance;
		_engaged_detrimental_chance = spell_list->engaged_detrimental_chance;
		_pursue_no_sp_recast_min = spell_list->pursue_no_sp_recast_min;
		_pursue_no_sp_recast_max = spell_list->pursue_no_sp_recast_max;
		_pursue_detrimental_chance = spell_list->pursue_detrimental_chance;
		_idle_no_sp_recast_min = spell_list->idle_no_sp_recast_min;
		_idle_no_sp_recast_max = spell_list->idle_no_sp_recast_max;
		_idle_beneficial_chance = spell_list->idle_beneficial_chance;
	}

	for (auto &e : spell_list->entries) {
		if (EQ::ValueWithin(GetLevel(), e.minlevel, e.maxlevel) && e.spellid) {
			if (!e.bucket_name.empty() && !e.bucket_value.empty()) {
				if (!CheckDataBucket(e.bucket_name, e.bucket_value, e.bucket_comparison)) {
					continue;
				}
			}

			const auto& bs = GetBotSpellSetting(e.spellid);
			if (bs) {
				if (!bs->is_enabled) {
					continue;
				}

				AddSpellToBotList(
					bs->priority,
					e.spellid,
					e.type,
					e.manacost,
					e.recast_delay,
					e.resist_adjust,
					e.minlevel,
					e.maxlevel,
					bs->min_hp,
					bs->max_hp,
					e.bucket_name,
					e.bucket_value,
					e.bucket_comparison
				);
				continue;
			}

			if (!GetBotEnforceSpellSetting()) {
				AddSpellToBotList(
					e.priority,
					e.spellid,
					e.type,
					e.manacost,
					e.recast_delay,
					e.resist_adjust,
					e.minlevel,
					e.maxlevel,
					e.min_hp,
					e.max_hp,
					e.bucket_name,
					e.bucket_value,
					e.bucket_comparison
				);
			} else {
				AddSpellToBotEnforceList(
					e.priority,
					e.spellid,
					e.type,
					e.manacost,
					e.recast_delay,
					e.resist_adjust,
					e.minlevel,
					e.maxlevel,
					e.min_hp,
					e.max_hp,
					e.bucket_name,
					e.bucket_value,
					e.bucket_comparison
				);
			}
		}
	}

	std::sort(AIBot_spells.begin(), AIBot_spells.end(), [](const BotSpells_Struct& a, const BotSpells_Struct& b) {
		return a.priority > b.priority;
	});

	if (IsValidSpell(attack_proc_spell)) {
		AddProcToWeapon(attack_proc_spell, true, proc_chance);

		if (RuleB(Spells, NPCInnateProcOverride)) {
			innate_proc_spell_id = attack_proc_spell;
		}
	}

	if (IsValidSpell(range_proc_spell)) {
		AddRangedProc(range_proc_spell, (rproc_chance + 100));
	}

	if (IsValidSpell(defensive_proc_spell)) {
		AddDefensiveProc(defensive_proc_spell, (dproc_chance + 100));
	}

	//Set AI casting variables

	AISpellVar.fail_recast = (_fail_recast) ? _fail_recast : RuleI(Spells, AI_SpellCastFinishedFailRecast);
	AISpellVar.engaged_no_sp_recast_min = (_engaged_no_sp_recast_min) ? _engaged_no_sp_recast_min : RuleI(Spells, AI_EngagedNoSpellMinRecast);
	AISpellVar.engaged_no_sp_recast_max = (_engaged_no_sp_recast_max) ? _engaged_no_sp_recast_max : RuleI(Spells, AI_EngagedNoSpellMaxRecast);
	AISpellVar.engaged_beneficial_self_chance = (_engaged_beneficial_self_chance) ? _engaged_beneficial_self_chance : RuleI(Spells, AI_EngagedBeneficialSelfChance);
	AISpellVar.engaged_beneficial_other_chance = (_engaged_beneficial_other_chance) ? _engaged_beneficial_other_chance : RuleI(Spells, AI_EngagedBeneficialOtherChance);
	AISpellVar.engaged_detrimental_chance = (_engaged_detrimental_chance) ? _engaged_detrimental_chance : RuleI(Spells, AI_EngagedDetrimentalChance);
	AISpellVar.pursue_no_sp_recast_min = (_pursue_no_sp_recast_min) ? _pursue_no_sp_recast_min : RuleI(Spells, AI_PursueNoSpellMinRecast);
	AISpellVar.pursue_no_sp_recast_max = (_pursue_no_sp_recast_max) ? _pursue_no_sp_recast_max : RuleI(Spells, AI_PursueNoSpellMaxRecast);
	AISpellVar.pursue_detrimental_chance = (_pursue_detrimental_chance) ? _pursue_detrimental_chance : RuleI(Spells, AI_PursueDetrimentalChance);
	AISpellVar.idle_no_sp_recast_min = (_idle_no_sp_recast_min) ? _idle_no_sp_recast_min : RuleI(Spells, AI_IdleNoSpellMinRecast);
	AISpellVar.idle_no_sp_recast_max = (_idle_no_sp_recast_max) ? _idle_no_sp_recast_max : RuleI(Spells, AI_IdleNoSpellMaxRecast);
	AISpellVar.idle_beneficial_chance = (_idle_beneficial_chance) ? _idle_beneficial_chance : RuleI(Spells, AI_IdleBeneficialChance);

	if (AIBot_spells.empty()) {
		AIautocastspell_timer->Disable();
	} else {
		AIautocastspell_timer->Trigger();
	}
	return true;
}

bool IsSpellInBotList(DBbotspells_Struct* spell_list, uint16 iSpellID) {
	auto it = std::find_if (
		spell_list->entries.begin(),
		spell_list->entries.end(),
		[iSpellID](const DBbotspells_entries_Struct &a) {
			return a.spellid == iSpellID;
		}
	);

	return it != spell_list->entries.end();
}

DBbotspells_Struct* ZoneDatabase::GetBotSpells(uint32 bot_spell_id)
{
	if (!bot_spell_id) {
		return nullptr;
	}

	auto c = bot_spells_cache.find(bot_spell_id);
	if (c != bot_spells_cache.end()) { // it's in the cache, easy =)
		return &c->second;
	}

	if (!bot_spells_loadtried.count(bot_spell_id)) { // no reason to ask the DB again if we have failed once already
		bot_spells_loadtried.insert(bot_spell_id);

		auto n = NpcSpellsRepository::FindOne(content_db, bot_spell_id);
		if (!n.id) {
			return nullptr;
		}

		DBbotspells_Struct spell_set;

		spell_set.parent_list = n.parent_list;
		spell_set.attack_proc = n.attack_proc;
		spell_set.proc_chance = n.proc_chance;
		spell_set.range_proc = n.range_proc;
		spell_set.rproc_chance = n.rproc_chance;
		spell_set.defensive_proc = n.defensive_proc;
		spell_set.dproc_chance = n.dproc_chance;
		spell_set.fail_recast = n.fail_recast;
		spell_set.engaged_no_sp_recast_min = n.engaged_no_sp_recast_min;
		spell_set.engaged_no_sp_recast_max = n.engaged_no_sp_recast_max;
		spell_set.engaged_beneficial_self_chance = n.engaged_b_self_chance;
		spell_set.engaged_beneficial_other_chance = n.engaged_b_other_chance;
		spell_set.engaged_detrimental_chance = n.engaged_d_chance;
		spell_set.pursue_no_sp_recast_min = n.pursue_no_sp_recast_min;
		spell_set.pursue_no_sp_recast_max = n.pursue_no_sp_recast_max;
		spell_set.pursue_detrimental_chance = n.pursue_d_chance;
		spell_set.idle_no_sp_recast_min = n.idle_no_sp_recast_min;
		spell_set.idle_no_sp_recast_max = n.idle_no_sp_recast_max;
		spell_set.idle_beneficial_chance = n.idle_b_chance;

		auto bse = BotSpellsEntriesRepository::GetWhere(
			content_db,
			fmt::format(
				"npc_spells_id = {}",
				bot_spell_id
			)
		);

		if (!bse.empty()) {
			for (const auto& e : bse) {
				DBbotspells_entries_Struct entry;

				entry.spellid           = e.spell_id;
				entry.type              = e.type;
				entry.minlevel          = e.minlevel;
				entry.maxlevel          = e.maxlevel;
				entry.manacost          = e.manacost;
				entry.recast_delay      = e.recast_delay;
				entry.priority          = e.priority;
				entry.min_hp            = e.min_hp;
				entry.max_hp            = e.max_hp;
				entry.resist_adjust     = e.resist_adjust;
				entry.bucket_name       = e.bucket_name;
				entry.bucket_value      = e.bucket_value;
				entry.bucket_comparison = e.bucket_comparison;

				// some spell types don't make much since to be priority 0, so fix that
				if (!IsBotSpellTypeInnate(entry.type) && entry.priority == 0) {
					entry.priority = 1;
				}

				if (e.resist_adjust) {
					entry.resist_adjust = e.resist_adjust;
				} else if (IsValidSpell(e.spell_id)) {
					entry.resist_adjust = spells[e.spell_id].resist_difficulty;
				}

				spell_set.entries.push_back(entry);
			}
		}

		bot_spells_cache.emplace(std::make_pair(bot_spell_id, spell_set));

		return &bot_spells_cache[bot_spell_id];
	}

	return nullptr;
}

// adds a spell to the list, taking into account priority and resorting list as needed.
void Bot::AddSpellToBotList(
	int16 in_priority,
	uint16 in_spell_id,
	uint32 in_type,
	int16 in_mana_cost,
	int32 in_recast_delay,
	int16 in_resist_adjust,
	uint8 in_min_level,
	uint8 in_max_level,
	int8 in_min_hp,
	int8 in_max_hp,
	std::string in_bucket_name,
	std::string in_bucket_value,
	uint8 in_bucket_comparison
) {
	if (!IsValidSpell(in_spell_id)) {
		return;
	}

	HasAISpell = true;
	BotSpells_Struct t;

	t.priority          = in_priority;
	t.spellid           = in_spell_id;
	t.type              = in_type;
	t.manacost          = in_mana_cost;
	t.recast_delay      = in_recast_delay;
	t.time_cancast      = 0;
	t.resist_adjust     = in_resist_adjust;
	t.minlevel          = in_min_level;
	t.maxlevel          = in_max_level;
	t.min_hp            = in_min_hp;
	t.max_hp            = in_max_hp;
	t.bucket_name       = in_bucket_name;
	t.bucket_value      = in_bucket_value;
	t.bucket_comparison = in_bucket_comparison;

	AIBot_spells.push_back(t);

	// If we're going from an empty list, we need to start the timer
	if (AIBot_spells.empty()) {
		AIautocastspell_timer->Start(RandomTimer(0, 300), false);
	}
}

// adds spells to the list ^spells that are returned if ^enforce is enabled
void Bot::AddSpellToBotEnforceList(
	int16 iPriority,
	uint16 iSpellID,
	uint32 iType,
	int16 iManaCost,
	int32 iRecastDelay,
	int16 iResistAdjust,
	uint8 min_level,
	uint8 max_level,
	int8 min_hp,
	int8 max_hp,
	std::string bucket_name,
	std::string bucket_value,
	uint8 bucket_comparison
) {
	if (!IsValidSpell(iSpellID)) {
		return;
	}

	HasAISpell = true;
	BotSpells_Struct t;

	t.priority = iPriority;
	t.spellid = iSpellID;
	t.type = iType;
	t.manacost = iManaCost;
	t.recast_delay = iRecastDelay;
	t.time_cancast = 0;
	t.resist_adjust = iResistAdjust;
	t.minlevel = min_level;
	t.maxlevel = maxlevel;
	t.min_hp = min_hp;
	t.max_hp = max_hp;
	t.bucket_name = bucket_name;
	t.bucket_value = bucket_value;
	t.bucket_comparison = bucket_comparison;

	AIBot_spells_enforced.push_back(t);
}

//this gets called from InterruptSpell() for failure or SpellFinished() for success
void Bot::AI_Bot_Event_SpellCastFinished(bool iCastSucceeded, uint16 slot) {
	if (slot == 1) {
		uint32 recovery_time = 0;
		if (iCastSucceeded) {
			if (casting_spell_AIindex < AIBot_spells.size()) {
				recovery_time += spells[AIBot_spells[casting_spell_AIindex].spellid].recovery_time;
				if (AIBot_spells[casting_spell_AIindex].recast_delay >= 0) {
					if (AIBot_spells[casting_spell_AIindex].recast_delay < 10000) {
						AIBot_spells[casting_spell_AIindex].time_cancast = Timer::GetCurrentTime() + (AIBot_spells[casting_spell_AIindex].recast_delay*1000);
					}
				}
				else {
					AIBot_spells[casting_spell_AIindex].time_cancast = Timer::GetCurrentTime() + spells[AIBot_spells[casting_spell_AIindex].spellid].recast_time;
				}
			}
			if (recovery_time < AIautocastspell_timer->GetSetAtTrigger()) {
				recovery_time = AIautocastspell_timer->GetSetAtTrigger();
			}
			AIautocastspell_timer->Start(recovery_time, false);
		}
		else {
			AIautocastspell_timer->Start(AISpellVar.fail_recast, false);
		}
		casting_spell_AIindex = AIBot_spells.size();
	}
}

bool Bot::HasBotSpellEntry(uint16 spell_id) {
	auto* spell_list = content_db.GetBotSpells(GetBotSpellID());

	if (!spell_list) {
		return false;
	}

	// Check if Spell ID is found in Bot Spell Entries
	for (auto& e : spell_list->entries) {
		if (spell_id == e.spellid) {
			return true;
		}
	}

	return false;
}

bool Bot::CanUseBotSpell(uint16 spell_id) {
	if (AIBot_spells.empty()) {
		return false;
	}

	for (const auto& s : AIBot_spells) {
		if (!IsValidSpell(s.spellid)) {
			return false;
		}

		if (s.spellid != spell_id) {
			continue;
		}

		if (s.minlevel > GetLevel()) {
			return false;
		}

		return true;
	}

	return false;
}

bool Bot::IsValidSpellRange(uint16 spell_id, Mob* tar) {
	if (!IsValidSpell(spell_id) || !tar) {
		return false;
	}

	float range = spells[spell_id].range + GetRangeDistTargetSizeMod(tar);

	if (IsAnyAESpell(spell_id)) {			
		range = GetAOERange(spell_id);
	}
	
	if (RuleB(Bots, EnableBotTGB) && IsTGBCompatibleSpell(spell_id) && IsGroupSpell(spell_id)) {			
		range = spells[spell_id].aoe_range;
	}

	range = GetActSpellRange(spell_id, range);

	if (IsIllusionSpell(spell_id) && (HasProjectIllusion())) {
		range = 100;
	}

	float dist2 = DistanceSquared(m_Position, tar->GetPosition());
	float range2 = range * range;
	float min_range2 = spells[spell_id].min_range * spells[spell_id].min_range;

	if (dist2 > range2) {
		//target is out of range.
		return false;
	}
	else if (dist2 < min_range2) {
		//target is too close range.
		return false;
	}

	return true;
}

BotSpell Bot::GetBestBotSpellForNukeByBodyType(Bot* botCaster, uint8 bodyType, uint16 spellType, bool AE, Mob* tar) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (!botCaster || !bodyType) {
		return result;
	}

	if (tar == nullptr) {
		tar = botCaster->GetTarget();
	}

	switch (bodyType) {
		case BodyType::Undead:
		case BodyType::SummonedUndead:
		case BodyType::Vampire:
			result = GetBestBotSpellForNukeByTargetType(botCaster, (!AE ? ST_Undead : ST_UndeadAE), spellType, AE, tar);
			break;
		case BodyType::Summoned:
		case BodyType::Summoned2:
		case BodyType::Summoned3:
			result = GetBestBotSpellForNukeByTargetType(botCaster, (!AE ? ST_Summoned : ST_SummonedAE), spellType, AE, tar);
			break;
		case BodyType::Animal:
			result = GetBestBotSpellForNukeByTargetType(botCaster, ST_Animal, spellType, AE, tar);
			break;
		case BodyType::Plant:
			result = GetBestBotSpellForNukeByTargetType(botCaster, ST_Plant, spellType, AE, tar);
			break;
		case BodyType::Giant:
			result = GetBestBotSpellForNukeByTargetType(botCaster, ST_Giant, spellType, AE, tar);
			break;
		case BodyType::Dragon:
			result = GetBestBotSpellForNukeByTargetType(botCaster, ST_Dragon, spellType, AE, tar);
			break;
		default:
			break;
	}

	return result;
}

void Bot::CheckBotSpells() {
	auto spellList = BotSpellsEntriesRepository::All(content_db);
	uint16 spell_id;
	SPDat_Spell_Struct spell;
	uint16 correctType;
	uint16 parentType;

	for (const auto& s : spellList) {
		if (!IsValidSpell(s.spell_id)) {
			LogBotSpellTypeChecks("{} is an invalid spell", s.spell_id); //deleteme
			continue;
		}

		spell = spells[s.spell_id];
		spell_id = spell.id;

		if (spell.classes[s.npc_spells_id - (BOT_CLASS_BASE_ID_PREFIX + 1)] >= 255) {
			LogBotSpellTypeChecks("{} [#{}] is not usable by a {} [#{}].", GetSpellName(spell_id), spell_id, GetClassIDName(s.npc_spells_id - BOT_CLASS_BASE_ID_PREFIX), s.npc_spells_id); //deleteme
		}
		else {
			if (spell.classes[s.npc_spells_id - (BOT_CLASS_BASE_ID_PREFIX + 1)] > s.minlevel) {
				LogBotSpellTypeChecks("{} [#{}] is not usable until level {} for a {} [#{}] and the min level is currently set to {}."
					, GetSpellName(spell_id)
					, spell_id
					, spell.classes[s.npc_spells_id - (BOT_CLASS_BASE_ID_PREFIX + 1)]
					, GetClassIDName(s.npc_spells_id - BOT_CLASS_BASE_ID_PREFIX)
					, s.npc_spells_id
					, s.minlevel
				); //deleteme

				LogBotSpellTypeChecksDetail("UPDATE bot_spells_entries SET `minlevel` = {} WHERE `spellid` = {} AND `npc_spells_id` = {}; -- {} [#{}] from minlevel {} to {} for {} [#{}]"
					, spell.classes[s.npc_spells_id - (BOT_CLASS_BASE_ID_PREFIX + 1)]
					, spell_id
					, s.npc_spells_id
					, GetSpellName(spell_id)
					, spell_id
					, s.minlevel
					, spell.classes[s.npc_spells_id - (BOT_CLASS_BASE_ID_PREFIX + 1)]
					, GetClassIDName(s.npc_spells_id - BOT_CLASS_BASE_ID_PREFIX)
					, s.npc_spells_id
				); //deleteme
			}

			if (spell.classes[s.npc_spells_id - (BOT_CLASS_BASE_ID_PREFIX + 1)] < s.minlevel) {
				LogBotSpellTypeChecks("{} [#{}] could be used starting at level {} for a {} [#{}] instead of the current min level of {}."
					, GetSpellName(spell_id)
					, spell_id
					, spell.classes[s.npc_spells_id - (BOT_CLASS_BASE_ID_PREFIX + 1)]
					, GetClassIDName(s.npc_spells_id - BOT_CLASS_BASE_ID_PREFIX)
					, s.npc_spells_id
					, s.minlevel
				); //deleteme

				LogBotSpellTypeChecksDetail("UPDATE bot_spells_entries SET `minlevel` = {} WHERE `spellid` = {} AND `npc_spells_id` = {}; -- {} [#{}] from minlevel {} to {} for {} [#{}]"
					, spell.classes[s.npc_spells_id - (BOT_CLASS_BASE_ID_PREFIX + 1)]
					, spell_id
					, s.npc_spells_id
					, GetSpellName(spell_id)
					, spell_id
					, s.minlevel
					, spell.classes[s.npc_spells_id - (BOT_CLASS_BASE_ID_PREFIX + 1)]
					, GetClassIDName(s.npc_spells_id - BOT_CLASS_BASE_ID_PREFIX)
					, s.npc_spells_id
				); //deleteme
			}


			if (spell.classes[s.npc_spells_id - (BOT_CLASS_BASE_ID_PREFIX + 1)] > s.maxlevel) {
				LogBotSpellTypeChecks("{} [#{}] is not usable until level {} for a {} [#{}] and the max level is currently set to {}."
					, GetSpellName(spell_id)
					, spell_id
					, spell.classes[s.npc_spells_id - (BOT_CLASS_BASE_ID_PREFIX + 1)]
					, GetClassIDName(s.npc_spells_id - BOT_CLASS_BASE_ID_PREFIX)
					, s.npc_spells_id
					, s.maxlevel
				); //deleteme
			}
		}

		correctType = GetCorrectSpellType(s.type, spell_id);
		parentType = GetSpellListSpellType(correctType);
		
		if (RuleB(Bots, UseParentSpellTypeForChecks)) {
			if (s.type == parentType || s.type == correctType) {
				continue;
			}
		}
		else { 
			if (IsPetBotSpellType(s.type)) {
				correctType = GetPetSpellType(correctType);
			}
		}

		if (correctType == s.type) {
			continue;
		}
		
		if (correctType == UINT16_MAX) {
			LogBotSpellTypeChecks("{} [#{}] is incorrect. It is currently set as {} [#{}] but the correct type is unknown."
				, GetSpellName(spell_id)
				, spell_id
				, GetSpellTypeNameByID(s.type)
				, s.type
			); //deleteme
		}
		else {
			LogBotSpellTypeChecks("{} [#{}] is incorrect. It is currently set as {} [#{}] and should be {} [#{}]"
				, GetSpellName(spell_id)
				, spell_id
				, GetSpellTypeNameByID(s.type)
				, s.type
				, GetSpellTypeNameByID(correctType)
				, correctType
			); //deleteme
			LogBotSpellTypeChecksDetail("UPDATE bot_spells_entries SET `type` = {} WHERE `spell_id` = {}; -- {} [#{}] from {} [#{}] to {} [#{}]"
				, correctType
				, spell_id
				, GetSpellName(spell_id)
				, spell_id
				, GetSpellTypeNameByID(s.type)
				, s.type
				, GetSpellTypeNameByID(correctType)
				, correctType
			); //deleteme
		}
	}
}

BotSpell Bot::GetBestBotSpellForRez(Bot* botCaster, Mob* target, uint16 spellType) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, spellType, SE_Revive);

		for (std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (
				IsResurrectSpell(botSpellListItr->SpellId) &&
				botCaster->CheckSpellRecastTimer(botSpellListItr->SpellId)
			) {
				result.SpellId = botSpellListItr->SpellId;
				result.SpellIndex = botSpellListItr->SpellIndex;
				result.ManaCost = botSpellListItr->ManaCost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForCharm(Bot* botCaster, Mob* target, uint16 spellType) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, spellType, SE_Charm);

		for (std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (
				IsCharmSpell(botSpellListItr->SpellId) &&
				botCaster->CastChecks(botSpellListItr->SpellId, target, spellType)
			) {
				result.SpellId = botSpellListItr->SpellId;
				result.SpellIndex = botSpellListItr->SpellIndex;
				result.ManaCost = botSpellListItr->ManaCost;

				break;
			}
		}
	}

	return result;
}
