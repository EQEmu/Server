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

bool Bot::AICastSpell(Mob* tar, uint8 iChance, uint32 iSpellTypes) {

	// Bot AI
	Raid* raid = entity_list.GetRaidByBotName(GetName());

	if (!tar) {
		return false;
	}

	if (!AI_HasSpells()) {
		return false;
	}

	if (iChance < 100 && zone->random.Int(0, 100) > iChance) {
		return false;
	}

	if (tar->GetAppearance() == eaDead && ((tar->IsClient() && !tar->CastToClient()->GetFeigned()) || tar->IsBot())) {
		return false;
	}

	uint8 botClass = GetClass();
	uint8 botLevel = GetLevel();

	bool checked_los = false;	//we do not check LOS until we are absolutely sure we need to, and we only do it once.

	BotSpell botSpell;
	botSpell.SpellId = 0;
	botSpell.SpellIndex = 0;
	botSpell.ManaCost = 0;

	switch (iSpellTypes) {
		case SpellType_Mez:
			return BotCastMez(tar, botLevel, checked_los, botSpell, raid);
		case SpellType_Heal:
			return BotCastHeal(tar, botLevel, botClass, botSpell, raid);
		case SpellType_Root:
			return BotCastRoot(tar, botLevel, iSpellTypes, botSpell, checked_los);
		case SpellType_Buff:
			return BotCastBuff(tar, botLevel, botClass);
		case SpellType_Escape:
			return BotCastEscape(tar, botClass, botSpell, iSpellTypes);
		case SpellType_Nuke:
			return BotCastNuke(tar, botLevel, botClass, botSpell, checked_los);
		case SpellType_Dispel:
			return BotCastDispel(tar, botSpell, iSpellTypes, checked_los);
		case SpellType_Pet:
			return BotCastPet(tar, botClass, botSpell);
		case SpellType_InCombatBuff:
			return BotCastCombatBuff(tar, botLevel, botClass);
		case SpellType_Lifetap:
			return BotCastLifetap(tar, botLevel, botSpell, checked_los, iSpellTypes);
		case SpellType_Snare:
			return BotCastSnare(tar, botLevel, botSpell, checked_los, iSpellTypes);
		case SpellType_DOT:
			return BotCastDOT(tar, botLevel, botSpell, checked_los);
		case SpellType_Slow:
			return BotCastSlow(tar, botLevel, botClass, botSpell, checked_los, raid);
		case SpellType_Debuff:
			return BotCastDebuff(tar, botLevel, botSpell, checked_los);
		case SpellType_Cure:
			return BotCastCure(tar, botClass, botSpell, raid);
		case SpellType_HateRedux:
			return BotCastHateReduction(tar, botLevel, botSpell);
		case SpellType_InCombatBuffSong:
			return BotCastCombatSong(tar, botLevel);
		case SpellType_OutOfCombatBuffSong:
			return BotCastSong(tar, botLevel);
		case SpellType_Resurrect:
		case SpellType_PreCombatBuff:
		case SpellType_PreCombatBuffSong:
		default:
			return false;
	}

	return false;
}

bool Bot::BotCastSong(Mob* tar, uint8 botLevel) {
	bool casted_spell = false;
	if (GetClass() != Class::Bard || tar != this || IsEngaged()) // Out-of-Combat songs can not be cast in combat
		return casted_spell;

	for (auto botSongList = GetPrioritizedBotSpellsBySpellType(this, SpellType_OutOfCombatBuffSong);
			auto iter : botSongList) {
		if (!iter.SpellId)
			continue;
		if (!CheckSpellRecastTimer(iter.SpellId))
			continue;
		if (!IsSpellUsableInThisZoneType(iter.SpellId, zone->GetZoneType()))
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

		casted_spell = AIDoSpellCast(iter.SpellIndex, tar, iter.ManaCost);
		if (casted_spell)
			break;
	}

	return casted_spell;
}

bool Bot::BotCastCombatSong(Mob* tar, uint8 botLevel) {
	bool casted_spell = false;
	if (tar != this) { // In-Combat songs can be cast Out-of-Combat in preparation for battle
		return casted_spell;
	}
	for (auto botSongList = GetPrioritizedBotSpellsBySpellType(this, SpellType_InCombatBuffSong);
		auto iter : botSongList) {
		if (!iter.SpellId)
			continue;
		if (!CheckSpellRecastTimer(iter.SpellId))
			continue;
		if (!IsSpellUsableInThisZoneType(iter.SpellId, zone->GetZoneType()))
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

		casted_spell = AIDoSpellCast(iter.SpellIndex, tar, iter.ManaCost);
		if (casted_spell)
			break;
	}

	return casted_spell;
}

bool Bot::BotCastHateReduction(Mob* tar, uint8 botLevel, const BotSpell& botSpell) {
	bool casted_spell = false;
	if (GetClass() == Class::Bard) {
		std::list<BotSpell_wPriority> botSongList = GetPrioritizedBotSpellsBySpellType(this, SpellType_HateRedux);
		for (auto iter : botSongList) {
			if (!iter.SpellId)
				continue;
			if (!CheckSpellRecastTimer(iter.SpellId))
				continue;
			if (!IsSpellUsableInThisZoneType(iter.SpellId, zone->GetZoneType()))
				continue;
			if (spells[iter.SpellId].target_type != ST_Target)
				continue;
			if (tar->CanBuffStack(iter.SpellId, botLevel, true) < 0)
				continue;

			if (IsValidSpellRange(iter.SpellId, tar)) {
				casted_spell = AIDoSpellCast(iter.SpellIndex, tar, iter.ManaCost);
			}
			if (casted_spell) {
				BotGroupSay(
					this,
					fmt::format(
						"Attempting to reduce hate on {} with {}.",
						tar->GetCleanName(),
						spells[iter.SpellId].name
					).c_str()
				);
			}
		}
	}

	return casted_spell;
}

bool Bot::BotCastCure(Mob* tar, uint8 botClass, BotSpell& botSpell, Raid* raid) {
	bool casted_spell = false;
	if (
		GetNeedsCured(tar) &&
		(tar->DontCureMeBefore() < Timer::GetCurrentTime()) &&
		GetNumberNeedingHealedInGroup(25, false, raid) <= 0 &&
		GetNumberNeedingHealedInGroup(40, false, raid) <= 2
	) {
		botSpell = GetBestBotSpellForCure(this, tar);

		if (!IsValidSpell(botSpell.SpellId)) {
			return casted_spell;
		}

		uint32 TempDontCureMeBeforeTime = tar->DontCureMeBefore();

		casted_spell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost, &TempDontCureMeBeforeTime);

		if (casted_spell && botClass != Class::Bard) {
			if (IsGroupSpell(botSpell.SpellId)) {
				if (HasGroup()) {
					Group const* group = GetGroup();
					if (group) {
						for (auto member : group->members) {
							if (
								member &&
								!member->qglobal &&
								TempDontCureMeBeforeTime != tar->DontCureMeBefore()
							) {
								member->SetDontCureMeBefore(Timer::GetCurrentTime() + 4000);
							}
						}
					}
				} else if (IsRaidGrouped()) {
					uint32 r_group = raid->GetGroup(GetName());
					if (r_group) {
						std::vector<RaidMember> raid_group_members = raid->GetRaidGroupMembers(r_group);
						for (auto& iter: raid_group_members) {
							if (
								iter.member &&
								!iter.member->qglobal &&
								TempDontCureMeBeforeTime != tar->DontCureMeBefore()
							) {
								iter.member->SetDontCureMeBefore(Timer::GetCurrentTime() + 4000);
							}
						}
					}
				}
			} else if (TempDontCureMeBeforeTime != tar->DontCureMeBefore()) {
				tar->SetDontCureMeBefore(Timer::GetCurrentTime() + 4000);
			}
		}
	}
	return casted_spell;
}

bool Bot::BotCastDebuff(Mob* tar, uint8 botLevel, BotSpell& botSpell, bool checked_los) {
	bool casted_spell = false;
	if ((tar->GetHPRatio() <= 99.0f) && (tar->GetHPRatio() > 20.0f))
	{
		if (!checked_los && (!CheckLosFN(tar) || !CheckWaterLoS(tar))) {
			return casted_spell;
		}

		botSpell = GetBestBotSpellForResistDebuff(this, tar);

		if (!IsValidSpell(botSpell.SpellId)) {
			botSpell = GetDebuffBotSpell(this, tar);
		}

		if (!IsValidSpell(botSpell.SpellId)) {
			return casted_spell;
		}

		if (!
			(
				!tar->IsImmuneToSpell(botSpell.SpellId, this) &&
				(tar->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0)
			)
		) {
			return casted_spell;
		}

		if (IsValidSpellRange(botSpell.SpellId, tar)) {
			casted_spell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost);
		}
	}
	return casted_spell;
}

bool Bot::BotCastSlow(Mob* tar, uint8 botLevel, uint8 botClass, BotSpell& botSpell, const bool& checked_los, Raid* raid) {
	bool casted_spell = false;
	if (tar->GetHPRatio() <= 99.0f) {

		if (!checked_los && (!CheckLosFN(tar) || !CheckWaterLoS(tar))) {
			return casted_spell;
		}

		switch (botClass) {
			case Class::Bard: {
				// probably needs attackable check
				for (
					auto botSongList = GetPrioritizedBotSpellsBySpellType(this, SpellType_Slow);
					auto iter : botSongList
				) {

					if (!iter.SpellId) {
						continue;
					}

					if (!CheckSpellRecastTimer(iter.SpellId)) {
						continue;
					}

					if (!IsSpellUsableInThisZoneType(iter.SpellId, zone->GetZoneType())) {
						continue;
					}

					if (spells[iter.SpellId].target_type != ST_Target) {
						continue;
					}

					if (tar->CanBuffStack(iter.SpellId, botLevel, true) < 0) {
						continue;
					}

					if (IsValidSpellRange(iter.SpellId, tar)) {
						casted_spell = AIDoSpellCast(iter.SpellIndex, tar, iter.ManaCost);
					}

					if (casted_spell) {
						return casted_spell;
					}
				}

				break;
			}
			case Class::Enchanter: {
				botSpell = GetBestBotSpellForMagicBasedSlow(this);
				break;
			}
			case Class::Shaman:
			case Class::Beastlord: {
				botSpell = GetBestBotSpellForDiseaseBasedSlow(this);

				if (botSpell.SpellId == 0 || ((tar->GetMR() - 50) < (tar->GetDR() + spells[botSpell.SpellId].resist_difficulty)))
					botSpell = GetBestBotSpellForMagicBasedSlow(this);
				break;
			}
		}

		if (!IsValidSpell(botSpell.SpellId)) {
			return casted_spell;
		}

		if (!(!tar->IsImmuneToSpell(botSpell.SpellId, this) && tar->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0)) {
			return casted_spell;
		}
		if (IsValidSpellRange(botSpell.SpellId, tar)) {
			casted_spell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost);
		}

		if (casted_spell && GetClass() != Class::Bard) {
			if (raid) {
				const auto msg = fmt::format("Attempting to slow {}.", tar->GetCleanName());
				raid->RaidSay(msg.c_str(), GetCleanName(), Language::CommonTongue, Language::MaxValue);
			} else {
				BotGroupSay(
					this,
					fmt::format(
						"Attempting to slow {} with {}.",
						tar->GetCleanName(),
						spells[botSpell.SpellId].name
					).c_str()
				);
			}
		}
	}
	return casted_spell;
}

bool Bot::BotCastDOT(Mob* tar, uint8 botLevel, const BotSpell& botSpell, const bool& checked_los) {
	bool casted_spell = false;

	if ((tar->GetHPRatio() <= 98.0f) && (tar->DontDotMeBefore() < Timer::GetCurrentTime()) && (tar->GetHPRatio() > 15.0f)) {
		if (!checked_los && (!CheckLosFN(tar) || !CheckWaterLoS(tar))) {
			return casted_spell;
		}

		if (GetClass() == Class::Bard) {
			std::list<BotSpell_wPriority> dotList = GetPrioritizedBotSpellsBySpellType(this, SpellType_DOT);

			const int maxDotSelect = 5;
			int dotSelectCounter = 0;

			for (const auto& s : dotList) {

				if (!IsValidSpell(s.SpellId)) {
					continue;
				}

				if (CheckSpellRecastTimer(s.SpellId))
				{

					if (!(!tar->IsImmuneToSpell(s.SpellId, this) && tar->CanBuffStack(s.SpellId, botLevel, true) >= 0)) {
						continue;
					}

					uint32 TempDontDotMeBefore = tar->DontDotMeBefore();

					casted_spell = AIDoSpellCast(s.SpellIndex, tar, s.ManaCost, &TempDontDotMeBefore);

					if (TempDontDotMeBefore != tar->DontDotMeBefore()) {
						tar->SetDontDotMeBefore(TempDontDotMeBefore);
					}
				}

				dotSelectCounter++;

				if ((dotSelectCounter == maxDotSelect) || casted_spell) {
					break;
				}
			}
		}
		else {
			std::list<BotSpell> dotList = GetBotSpellsBySpellType(this, SpellType_DOT);

			const int maxDotSelect = 5;
			int dotSelectCounter = 0;

			for (const auto& s : dotList) {

				if (!IsValidSpell(s.SpellId)) {
					continue;
				}

				if (CheckSpellRecastTimer(s.SpellId)) {

					if (!(!tar->IsImmuneToSpell(s.SpellId, this) &&
						  tar->CanBuffStack(s.SpellId, botLevel, true) >= 0)) {
						continue;
					}

					uint32 TempDontDotMeBefore = tar->DontDotMeBefore();

					if (IsValidSpellRange(s.SpellId, tar)) {
						casted_spell = AIDoSpellCast(s.SpellIndex, tar, s.ManaCost, &TempDontDotMeBefore);
					}

					if (TempDontDotMeBefore != tar->DontDotMeBefore()) {
						tar->SetDontDotMeBefore(TempDontDotMeBefore);
					}
				}

				dotSelectCounter++;

				if ((dotSelectCounter == maxDotSelect) || casted_spell) {
					return casted_spell;
				}
			}
		}
	}
	return casted_spell;
}

bool Bot::BotCastSnare(Mob* tar, uint8 botLevel, BotSpell& botSpell, const bool& checked_los, uint32 iSpellTypes) {
	bool casted_spell = false;
	if (tar->DontSnareMeBefore() < Timer::GetCurrentTime()) {
		if (!checked_los && (!CheckLosFN(tar) || !CheckWaterLoS(tar))) {
			return casted_spell;
		}


		botSpell = GetFirstBotSpellBySpellType(this, iSpellTypes);

		if (!IsValidSpell(botSpell.SpellId)) {
			return casted_spell;
		}

		if (!(!tar->IsImmuneToSpell(botSpell.SpellId, this) && tar->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0)) {
			return casted_spell;
		}

		uint32 TempDontSnareMeBefore = tar->DontSnareMeBefore();

		if (IsValidSpellRange(botSpell.SpellId, tar)) {
			casted_spell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost, &TempDontSnareMeBefore);
		}

		if (TempDontSnareMeBefore != tar->DontSnareMeBefore()) {
			tar->SetDontSnareMeBefore(TempDontSnareMeBefore);
		}
	}
	return casted_spell;
}

bool Bot::BotCastLifetap(Mob* tar, uint8 botLevel, BotSpell& botSpell, const bool& checked_los, uint32 iSpellTypes) {
	bool casted_spell = false;
	if (GetHPRatio() < 90.0f) {
		if (!checked_los && (!CheckLosFN(tar) || !CheckWaterLoS(tar))) {
			return casted_spell;
		}


		botSpell = GetFirstBotSpellBySpellType(this, iSpellTypes);

		if (!IsValidSpell(botSpell.SpellId)) {
			return casted_spell;
		}

		if (!(!tar->IsImmuneToSpell(botSpell.SpellId, this) && (tar->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0))) {
			return casted_spell;
		}

		if (IsValidSpellRange(botSpell.SpellId, tar)) {
			casted_spell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost);
		}
	}
	return casted_spell;
}

bool Bot::BotCastCombatBuff(Mob* tar, uint8 botLevel, uint8 botClass) {

	bool casted_spell = false;
	if (tar->DontBuffMeBefore() < Timer::GetCurrentTime()) {
		std::list<BotSpell> buffSpellList = GetBotSpellsBySpellType(this, SpellType_InCombatBuff);

		for (const auto& s : buffSpellList) {

			if (!IsValidSpell(s.SpellId)) {
				continue;
			}
			// no buffs with illusions.. use #bot command to cast illusions
			if (IsEffectInSpell(s.SpellId, SE_Illusion) && tar != this) {
				continue;
			}
			//no teleport spells use #bot command to cast teleports
			if (IsEffectInSpell(s.SpellId, SE_Teleport) || IsEffectInSpell(s.SpellId, SE_Succor)) {
				continue;
			}
			// can not cast buffs for your own pet only on another pet that isn't yours
			if ((spells[s.SpellId].target_type == ST_Pet) && (tar != GetPet())) {
				continue;
			}

			//Conversion Spells
			if (
				IsSelfConversionSpell(s.SpellId) &&
				(
					GetManaRatio() > 90.0f ||
					GetHPRatio() < 50.0f ||
					GetHPRatio() < (GetManaRatio() + 10.0f)
					)
			) {
				break; //don't cast if low hp, lots of mana, or if mana is higher than hps
			}

			// Validate target
			// TODO: Add ST_TargetsTarget support for Buffing.
			if (
				!(
					(
						spells[s.SpellId].target_type == ST_Target ||
						spells[s.SpellId].target_type == ST_Pet ||
						(tar == this && spells[s.SpellId].target_type != ST_TargetsTarget) ||
						spells[s.SpellId].target_type == ST_Group ||
						spells[s.SpellId].target_type == ST_GroupTeleport ||
						(botClass == Class::Bard && spells[s.SpellId].target_type == ST_AEBard)
					) &&
					!tar->IsImmuneToSpell(s.SpellId, this) &&
					tar->CanBuffStack(s.SpellId, botLevel, true) >= 0
				)
			) {
				continue;
			}

			// Put the zone levitate and movement check here since bots are able to bypass the client casting check
			if (
				((IsEffectInSpell(s.SpellId, SE_Levitate) && !zone->CanLevitate()) ||
				(IsEffectInSpell(s.SpellId, SE_MovementSpeed) && !zone->CanCastOutdoor())) &&
				(botClass != Class::Bard || !IsSpellUsableInThisZoneType(s.SpellId, zone->GetZoneType()))
			) {
				continue;
			}

			if (!IsGroupSpell(s.SpellId)) {
			//Only check archetype if spell is not a group spell
			//Hybrids get all buffs
				switch (tar->GetArchetype()) {
					case Archetype::Caster:
						//TODO: probably more caster specific spell effects in here
						if (
							(
								IsEffectInSpell(s.SpellId, SE_AttackSpeed) ||
								IsEffectInSpell(s.SpellId, SE_ATK) ||
								IsEffectInSpell(s.SpellId, SE_STR) ||
								IsEffectInSpell(s.SpellId, SE_ReverseDS) ||
								IsEffectInSpell(s.SpellId, SE_DamageShield)
							) &&
							spells[s.SpellId].target_type != ST_Self
						) {
							continue;
						}
						break;
					case Archetype::Melee:
						if (
							(
								IsEffectInSpell(s.SpellId, SE_IncreaseSpellHaste) ||
								IsEffectInSpell(s.SpellId, SE_ManaPool) ||
								IsEffectInSpell(s.SpellId, SE_CastingLevel) ||
								IsEffectInSpell(s.SpellId, SE_ManaRegen_v2) ||
								IsEffectInSpell(s.SpellId, SE_CurrentMana)
							) &&
							spells[s.SpellId].target_type != ST_Self
						) {
							continue;
						}
						break;
					default:
						break;
				}
			}
			// TODO: Add TriggerSpell Support for Exchanter Runes
			if (botClass == Class::Enchanter && IsEffectInSpell(s.SpellId, SE_Rune)) {
				float manaRatioToCast = 75.0f;

				switch(GetBotStance()) {
				case Stance::Efficient:
					manaRatioToCast = 90.0f;
					break;
				case Stance::Balanced:
				case Stance::Aggressive:
					manaRatioToCast = 75.0f;
					break;
				case Stance::Reactive:
				case Stance::Burn:
				case Stance::AEBurn:
					manaRatioToCast = 50.0f;
					break;
				default:
					manaRatioToCast = 75.0f;
					break;
				}

				//If we're at specified mana % or below, don't rune as enchanter
				if (GetManaRatio() <= manaRatioToCast) {
					break;
				}
			}

			if (CheckSpellRecastTimer(s.SpellId)) {
				uint32 TempDontBuffMeBefore = tar->DontBuffMeBefore();
				casted_spell = AIDoSpellCast(s.SpellIndex, tar, s.ManaCost, &TempDontBuffMeBefore);
				if (TempDontBuffMeBefore != tar->DontBuffMeBefore()) {
					tar->SetDontBuffMeBefore(TempDontBuffMeBefore);
				}
			}

			if (casted_spell) {
				return casted_spell;
			}
		}
	}
	return casted_spell;
}

bool Bot::BotCastPet(Mob* tar, uint8 botClass, BotSpell& botSpell) {
	bool casted_spell = false;
	if (!IsPet() && !GetPetID() && !IsBotCharmer()) {
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
				return casted_spell;
			}
			if (familiar_buff_slot >= 0) {
				BuffFadeBySlot(familiar_buff_slot);
				return casted_spell;
			}

			botSpell = GetFirstBotSpellBySpellType(this, SpellType_Pet);
		}
		else if (botClass == Class::Magician) {
			botSpell = GetBestBotMagicianPetSpell(this);
		}
		else {
			botSpell = GetFirstBotSpellBySpellType(this, SpellType_Pet);
		}

		if (!IsValidSpell(botSpell.SpellId)) {
			return casted_spell;
		}

		casted_spell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost);
	}
	return casted_spell;
}

bool Bot::BotCastDispel(Mob* tar, BotSpell& botSpell, uint32 iSpellTypes, const bool& checked_los) {

	bool casted_spell = false;
	if (tar->GetHPRatio() > 95.0f) {
		if (!checked_los && (!CheckLosFN(tar) || !CheckWaterLoS(tar))) {
			return casted_spell;
		}

		botSpell = GetFirstBotSpellBySpellType(this, iSpellTypes);

		if (!IsValidSpell(botSpell.SpellId)) {
			return casted_spell;
		}
		// TODO: Check target to see if there is anything to dispel

		if (tar->CountDispellableBuffs() > 0 && IsValidSpellRange(botSpell.SpellId, tar)) {
			casted_spell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost);
		}
	}
	return casted_spell;
}

bool Bot::BotCastNuke(Mob* tar, uint8 botLevel, uint8 botClass, BotSpell& botSpell, const bool& checked_los) {

	bool casted_spell = false;
	if ((tar->GetHPRatio() <= 95.0f) || ((botClass == Class::Bard) || (botClass == Class::Shaman) || (botClass == Class::Enchanter) || (botClass == Class::Paladin) || (botClass == Class::ShadowKnight) || (botClass == Class::Warrior)))
	{
		if (!checked_los && (!CheckLosFN(tar) || !CheckWaterLoS(tar))) {
			return casted_spell;
		}

		if (botClass == Class::Cleric || botClass == Class::Enchanter)
		{
			float manaRatioToCast = 75.0f;

			switch(GetBotStance()) {
			case Stance::Efficient:
				manaRatioToCast = 90.0f;
				break;
			case Stance::Balanced:
				manaRatioToCast = 75.0f;
				break;
			case Stance::Reactive:
			case Stance::Aggressive:
				manaRatioToCast = 50.0f;
				break;
			case Stance::Burn:
			case Stance::AEBurn:
				manaRatioToCast = 25.0f;
				break;
			default:
				manaRatioToCast = 50.0f;
				break;
			}

			//If we're at specified mana % or below, don't nuke as cleric or enchanter
			if (GetManaRatio() <= manaRatioToCast)
				return casted_spell;
		}

		if (botClass == Class::Magician || botClass == Class::ShadowKnight || botClass == Class::Necromancer || botClass == Class::Paladin || botClass == Class::Ranger || botClass == Class::Druid || botClass == Class::Cleric) {
			if (tar->GetBodyType() == BodyType::Undead || tar->GetBodyType() == BodyType::SummonedUndead || tar->GetBodyType() == BodyType::Vampire)
				botSpell = GetBestBotSpellForNukeByTargetType(this, ST_Undead);
			else if (tar->GetBodyType() == BodyType::Summoned || tar->GetBodyType() == BodyType::Summoned2 || tar->GetBodyType() == BodyType::Summoned3)
				botSpell = GetBestBotSpellForNukeByTargetType(this, ST_Summoned);
		}

		if ((botClass == Class::Paladin || botClass == Class::Druid || botClass == Class::Cleric || botClass == Class::Enchanter || botClass == Class::Wizard) && !IsValidSpell(botSpell.SpellId)) {
			uint8 stunChance = (tar->IsCasting() ? 30: 15);

			if (botClass == Class::Paladin) {
				stunChance = 50;
			}

			if (!tar->GetSpecialAbility(SpecialAbility::StunImmunity) && !tar->IsStunned() && (zone->random.Int(1, 100) <= stunChance)) {
				botSpell = GetBestBotSpellForStunByTargetType(this, ST_Target);
			}
		}

		if (botClass == Class::Wizard && botSpell.SpellId == 0) {
			botSpell = GetBestBotWizardNukeSpellByTargetResists(this, tar);
		}

		if (!IsValidSpell(botSpell.SpellId)) {
			botSpell = GetBestBotSpellForNukeByTargetType(this, ST_Target);
		}
		if (!IsValidSpell(botSpell.SpellId)) {
			return casted_spell;
		}
		if (!(!tar->IsImmuneToSpell(botSpell.SpellId, this) && (tar->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0))) {
			return casted_spell;
		}
		if (IsFearSpell(botSpell.SpellId) && tar->GetSnaredAmount() == -1 && !tar->IsRooted()) {
			return casted_spell;
		}

		if (IsValidSpellRange(botSpell.SpellId, tar)) {
			casted_spell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost);
		}
	}
	return casted_spell;
}

bool Bot::BotCastEscape(Mob*& tar, uint8 botClass, BotSpell& botSpell, uint32 iSpellTypes) {

	bool casted_spell = false;
	auto hpr = (uint8) GetHPRatio();
	bool mayGetAggro = false;

	if (hpr > 15 && ((botClass == Class::Wizard) || (botClass == Class::Enchanter) || (botClass == Class::Ranger))) {
		mayGetAggro = HasOrMayGetAggro();
	}

	if (hpr <= 15 || mayGetAggro)
	{
		botSpell = GetFirstBotSpellBySpellType(this, iSpellTypes);

		if (!IsValidSpell(botSpell.SpellId)) {
			return casted_spell;
		}

		if (IsInvulnerabilitySpell(botSpell.SpellId)) {
			tar = this; //target self for invul type spells
		}

		if (IsValidSpellRange(botSpell.SpellId, tar) || botClass == Class::Bard) {
			casted_spell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost);
		}
	}
	return casted_spell;
}

bool Bot::BotCastBuff(Mob* tar, uint8 botLevel, uint8 botClass) {
	bool casted_spell = false;
	if (tar->DontBuffMeBefore() < Timer::GetCurrentTime()) {
		std::list<BotSpell> buffSpellList = GetBotSpellsBySpellType(this, SpellType_Buff);

		for(const auto& s : buffSpellList) {

			if (!IsValidSpell(s.SpellId))  {
				continue;
			}

			// no buffs with illusions.. use #bot command to cast illusions
			if (IsEffectInSpell(s.SpellId, SE_Illusion) && tar != this) {
				continue;
			}

			//no teleport spells use #bot command to cast teleports
			if (IsEffectInSpell(s.SpellId, SE_Teleport) || IsEffectInSpell(s.SpellId, SE_Succor)) {
				continue;
			}
			// can not cast buffs for your own pet only on another pet that isn't yours
			if ((spells[s.SpellId].target_type == ST_Pet) && (tar != GetPet())) {
				continue;
			}

			// Validate target
			// TODO: Add ST_TargetsTarget support for Buffing.
			if (
				!(
					(
						spells[s.SpellId].target_type == ST_Target ||
						spells[s.SpellId].target_type == ST_Pet ||
						(tar == this && spells[s.SpellId].target_type != ST_TargetsTarget) ||
						spells[s.SpellId].target_type == ST_Group ||
						spells[s.SpellId].target_type == ST_GroupTeleport ||
						(botClass == Class::Bard && spells[s.SpellId].target_type == ST_AEBard)
					) &&
					!tar->IsImmuneToSpell(s.SpellId, this) &&
					tar->CanBuffStack(s.SpellId, botLevel, true) >= 0
				)
			) {
				continue;
			}

			// Put the zone levitate and movement check here since bots are able to bypass the client casting check
			if (
				(
					(IsEffectInSpell(s.SpellId, SE_Levitate) && !zone->CanLevitate()) ||
					(IsEffectInSpell(s.SpellId, SE_MovementSpeed) && !zone->CanCastOutdoor())
				) &&
				(botClass != Class::Bard || !IsSpellUsableInThisZoneType(s.SpellId, zone->GetZoneType()))
			) {
				continue;
			}

			switch (tar->GetArchetype())
			{
				case Archetype::Caster:
					//TODO: probably more caster specific spell effects in here
					if (IsEffectInSpell(s.SpellId, SE_AttackSpeed) || IsEffectInSpell(s.SpellId, SE_ATK) ||
						IsEffectInSpell(s.SpellId, SE_STR) || IsEffectInSpell(s.SpellId, SE_ReverseDS))
					{
						continue;
					}
					break;
				case Archetype::Melee:
					if (IsEffectInSpell(s.SpellId, SE_IncreaseSpellHaste) || IsEffectInSpell(s.SpellId, SE_ManaPool) ||
						IsEffectInSpell(s.SpellId, SE_CastingLevel) || IsEffectInSpell(s.SpellId, SE_ManaRegen_v2) ||
						IsEffectInSpell(s.SpellId, SE_CurrentMana))
					{
						continue;
					}
					break;
				default: //Hybrids get all buffs
					break;
			}

			if (botClass == Class::Enchanter && IsEffectInSpell(s.SpellId, SE_Rune))
			{
				float manaRatioToCast = 75.0f;

				switch (GetBotStance()) {
				case Stance::Efficient:
					manaRatioToCast = 90.0f;
						break;
				case Stance::Balanced:
				case Stance::Aggressive:
					manaRatioToCast = 75.0f;
						break;
				case Stance::Reactive:
				case Stance::Burn:
				case Stance::AEBurn:
					manaRatioToCast = 50.0f;
						break;
				default:
					manaRatioToCast = 75.0f;
						break;
				}

				//If we're at specified mana % or below, don't rune as enchanter
				if (GetManaRatio() <= manaRatioToCast) {
					return casted_spell;
				}
			}

			if (CheckSpellRecastTimer(s.SpellId))
			{
				uint32 TempDontBuffMeBefore = tar->DontBuffMeBefore();

				casted_spell = AIDoSpellCast(s.SpellIndex, tar, s.ManaCost, &TempDontBuffMeBefore);

				if (TempDontBuffMeBefore != tar->DontBuffMeBefore()) {
					tar->SetDontBuffMeBefore(TempDontBuffMeBefore);
				}
			}
		}
	}
	return casted_spell;
}

bool Bot::BotCastRoot(Mob* tar, uint8 botLevel, uint32 iSpellTypes, BotSpell& botSpell, const bool& checked_los) {
	bool casted_spell = false;
	if (!tar->IsRooted() && tar->DontRootMeBefore() < Timer::GetCurrentTime()) {
		if (!checked_los && (!CheckLosFN(tar) || !CheckWaterLoS(tar))) {
			return casted_spell;
		}

		// TODO: If there is a ranger in the group then don't allow root spells

		botSpell = GetFirstBotSpellBySpellType(this, iSpellTypes);

		if (!IsValidSpell(botSpell.SpellId)) {
			return casted_spell;
		}
		if (tar->CanBuffStack(botSpell.SpellId, botLevel, true) == 0) {
			return casted_spell;
		}
		uint32 TempDontRootMeBefore = tar->DontRootMeBefore();

		if (IsValidSpellRange(botSpell.SpellId, tar)) {
			casted_spell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost, &TempDontRootMeBefore);
		}

		if (TempDontRootMeBefore != tar->DontRootMeBefore()) {
			tar->SetDontRootMeBefore(TempDontRootMeBefore);
		}
	}
	return casted_spell;
}

bool Bot::BotCastHeal(Mob* tar, uint8 botLevel, uint8 botClass, BotSpell& botSpell, Raid* raid) {
	bool casted_spell = false;
	if (tar->DontHealMeBefore() < Timer::GetCurrentTime()) {
		auto hpr = (uint8)tar->GetHPRatio();
		bool hasAggro = false;
		bool isPrimaryHealer = false;

		if (HasGroup() || IsRaidGrouped()) {
			isPrimaryHealer = IsGroupHealer();
		}

		if (hpr < 95 || tar->IsClient() || botClass == Class::Bard) {
			if (tar->GetClass() == Class::Necromancer && hpr >= 40) {
				return false;
			}

			if (tar->GetClass() == Class::Shaman && hpr >= 80) {
				return false;
			}

			// Evaluate the situation
			if ((IsEngaged()) && ((botClass == Class::Cleric) || (botClass == Class::Druid) || (botClass == Class::Shaman) || (botClass == Class::Paladin))) {
				if (tar->GetTarget() && tar->GetTarget()->GetHateTop() && tar->GetTarget()->GetHateTop() == tar) {
					hasAggro = true;
				}

				if (hpr < 35) {
					botSpell = GetBestBotSpellForFastHeal(this);
				}
				else if (hpr < 70) {
					if (GetNumberNeedingHealedInGroup(60, false, raid) >= 3) {
						botSpell = GetBestBotSpellForGroupHeal(this);
					}

					if (!IsValidSpell(botSpell.SpellId)) {
						botSpell = GetBestBotSpellForPercentageHeal(this);
					}
				}
				else if (hpr < 95) {
					if (GetNumberNeedingHealedInGroup(80, false, raid) >= 3) {
						botSpell = GetBestBotSpellForGroupHealOverTime(this);
					}

					if (hasAggro) {
						botSpell = GetBestBotSpellForPercentageHeal(this);
					}
				}
				else {
					if (!tar->FindType(SE_HealOverTime)) {
						botSpell = GetBestBotSpellForHealOverTime(this);
					}
				}
			}
			else if ((botClass == Class::Cleric) || (botClass == Class::Druid) || (botClass == Class::Shaman) || (botClass == Class::Paladin)) {
				if (GetNumberNeedingHealedInGroup(40, true, raid) >= 2) {
					botSpell = GetBestBotSpellForGroupCompleteHeal(this);

					if (!IsValidSpell(botSpell.SpellId)) {
						botSpell = GetBestBotSpellForGroupHeal(this);
					}

					if (!IsValidSpell(botSpell.SpellId)) {
						botSpell = GetBestBotSpellForGroupHealOverTime(this);
					}

					if (hpr < 40 && !IsValidSpell(botSpell.SpellId)) {
						botSpell = GetBestBotSpellForPercentageHeal(this);
					}
				}
				else if (GetNumberNeedingHealedInGroup(60, true, raid) >= 2) {
					botSpell = GetBestBotSpellForGroupHeal(this);

					if (!IsValidSpell(botSpell.SpellId)) {
						botSpell = GetBestBotSpellForGroupHealOverTime(this);
					}

					if (hpr < 40 && !IsValidSpell(botSpell.SpellId)) {
						botSpell = GetBestBotSpellForPercentageHeal(this);
					}
				}
				else if (hpr < 40) {
					botSpell = GetBestBotSpellForPercentageHeal(this);
				}
				else if (hpr < 75) {
					botSpell = GetBestBotSpellForRegularSingleTargetHeal(this);
				}
				else {
					if (hpr < 90 && !tar->FindType(SE_HealOverTime)) {
						botSpell = GetBestBotSpellForHealOverTime(this);
					}
				}
			}
			else {
				float hpRatioToCast = 0.0f;

				switch (GetBotStance()) {
					case Stance::Efficient:
					case Stance::Aggressive:
						hpRatioToCast = isPrimaryHealer ? 90.0f : 50.0f;
						break;
					case Stance::Balanced:
						hpRatioToCast = isPrimaryHealer ? 95.0f : 75.0f;
						break;
					case Stance::Reactive:
						hpRatioToCast = isPrimaryHealer ? 100.0f : 90.0f;
						break;
					case Stance::Burn:
					case Stance::AEBurn:
						hpRatioToCast = isPrimaryHealer ? 75.0f : 25.0f;
						break;
					default:
						hpRatioToCast = isPrimaryHealer ? 100.0f : 0.0f;
						break;
				}

				//If we're at specified mana % or below, don't heal as hybrid
				if (tar->GetHPRatio() <= hpRatioToCast) {
					botSpell = GetBestBotSpellForRegularSingleTargetHeal(this);
				}
			}

			if (!IsValidSpell(botSpell.SpellId)) {
				botSpell = GetBestBotSpellForRegularSingleTargetHeal(this);
			}
			if (!IsValidSpell(botSpell.SpellId)) {
				botSpell = GetFirstBotSpellForSingleTargetHeal(this);
			}
			if (botSpell.SpellId == 0 && botClass == Class::Bard) {
				botSpell = GetFirstBotSpellBySpellType(this, SpellType_Heal);
			}

			if (!IsValidSpell(botSpell.SpellId)) {
				return false;
			}
			// Can we cast this spell on this target?
			if (!(spells[botSpell.SpellId].target_type==ST_GroupTeleport || spells[botSpell.SpellId].target_type == ST_Target || tar == this)
			    && tar->CanBuffStack(botSpell.SpellId, botLevel, true) < 0) {
				return false;
			}

			uint32 TempDontHealMeBeforeTime = tar->DontHealMeBefore();

			if (IsValidSpellRange(botSpell.SpellId, tar) || botClass == Class::Bard) {
				casted_spell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost, &TempDontHealMeBeforeTime);
			}

			if (casted_spell && botClass != Class::Bard) {
				if (IsGroupSpell(botSpell.SpellId)) {
					if (HasGroup()) {
						Group *group = GetGroup();
						if (group) {
							BotGroupSay(
								this,
								fmt::format(
									"Casting {}.",
									spells[botSpell.SpellId].name
								).c_str()
							);

							for (const auto& member : group->members) {
								if (member && !member->qglobal) {
									member->SetDontHealMeBefore(Timer::GetCurrentTime() + 1000);
								}
							}
						} else if (IsRaidGrouped()) {
							uint32 r_group = raid->GetGroup(GetName());
							const auto msg = fmt::format("Casting {}.", spells[botSpell.SpellId].name);
							raid->RaidGroupSay(msg.c_str(), GetCleanName(), Language::CommonTongue, Language::MaxValue);
							std::vector<RaidMember> raid_group_members = raid->GetRaidGroupMembers(r_group);
							for (const auto& rgm : raid_group_members) {
								if (rgm.member && !rgm.member->qglobal) {
									rgm.member->SetDontHealMeBefore(Timer::GetCurrentTime() + 1000);
								}
							}
						}
					} else {
						if (tar != this) { //we don't need spam of bots healing themselves
							BotGroupSay(
								this,
								fmt::format(
									"Casting {} on {}.",
									spells[botSpell.SpellId].name,
									tar->GetCleanName()
								).c_str()
							);
						}
					}
				}
				tar->SetDontHealMeBefore(Timer::GetCurrentTime() + 2000);
			}
		}
	}
	return casted_spell;
}

bool Bot::BotCastMez(Mob* tar, uint8 botLevel, bool checked_los, BotSpell& botSpell, Raid* raid) {
	bool casted_spell = false;
	if (!checked_los && (!CheckLosFN(tar) || !CheckWaterLoS(tar))) {
		return false;
	}

	//TODO
	//Check if single target or AoE mez is best
	//if (TARGETS ON MT IS => 3 THEN botSpell = AoEMez)
	//if (TARGETS ON MT IS <= 2 THEN botSpell = BestMez)

	botSpell = GetBestBotSpellForMez(this);

	if (!IsValidSpell(botSpell.SpellId)) {
		return false;
	}

	Mob* addMob = GetFirstIncomingMobToMez(this, botSpell);

	if (!addMob) {
		return false;
	}

	if (!(!addMob->IsImmuneToSpell(botSpell.SpellId, this) && addMob->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0)) {
		return false;
	}

	if (IsValidSpellRange(botSpell.SpellId, addMob)) {
		casted_spell = AIDoSpellCast(botSpell.SpellIndex, addMob, botSpell.ManaCost);
	}
	if (casted_spell) {
		if (raid) {
			raid->RaidSay(
				GetCleanName(),
				fmt::format(
					"Attempting to mesmerize {} with {}.",
					addMob->GetCleanName(),
					spells[botSpell.SpellId].name
				).c_str(),
				0,
				100
			);
		} else {
			BotGroupSay(
				this,
				fmt::format(
					"Attempting to mesmerize {} with {}.",
					addMob->GetCleanName(),
					spells[botSpell.SpellId].name
				).c_str()
			);
		}
	}
	return casted_spell;
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
		manaCost > hasMana && AIBot_spells[i].type & SpellType_Buff
	) {
		SetMana(manaCost);
	}

	float dist2 = 0;

	if (AIBot_spells[i].type & SpellType_Escape) {
		dist2 = 0;
	} else
		dist2 = DistanceSquared(m_Position, tar->GetPosition());

	if (
		(
			(
				(
					(spells[AIBot_spells[i].spellid].target_type==ST_GroupTeleport && AIBot_spells[i].type == SpellType_Heal) ||
					spells[AIBot_spells[i].spellid].target_type ==ST_AECaster ||
					spells[AIBot_spells[i].spellid].target_type ==ST_Group ||
					spells[AIBot_spells[i].spellid].target_type ==ST_AEBard ||
					(
						tar == this && spells[AIBot_spells[i].spellid].target_type != ST_TargetsTarget
					)
				) &&
				dist2 <= spells[AIBot_spells[i].spellid].aoe_range*spells[AIBot_spells[i].spellid].aoe_range
			) ||
			dist2 <= GetActSpellRange(AIBot_spells[i].spellid, spells[AIBot_spells[i].spellid].range)*GetActSpellRange(AIBot_spells[i].spellid, spells[AIBot_spells[i].spellid].range)
		) &&
		(
			mana_cost <= GetMana() ||
			IsBotNonSpellFighter()
		)
	) {
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
	else {
		if (CalcSpellRecastTimer(AIBot_spells[i].spellid) > 0) {
			SetSpellRecastTimer(AIBot_spells[i].spellid);
		}
	}

	return result;
}

bool Bot::AI_PursueCastCheck() {
	bool result = false;

	if (AIautocastspell_timer->Check(false)) {

		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.

		LogAIDetail("Bot Engaged (pursuing) autocast check triggered. Trying to cast offensive spells");

		if (!AICastSpell(GetTarget(), 100, SpellType_Snare)) {
			if (!AICastSpell(GetTarget(), 100, SpellType_Lifetap) && !AICastSpell(GetTarget(), 100, SpellType_Nuke)) {
			}
			result = true;
		}

		if (!AIautocastspell_timer->Enabled()) {
			AIautocastspell_timer->Start(RandomTimer(100, 250), false);
		}
	}

	return result;
}

bool Bot::AI_IdleCastCheck() {
	bool result = false;

	if (AIautocastspell_timer->Check(false)) {
		LogAIDetail("Bot Non-Engaged autocast check triggered: [{}]", GetCleanName());
		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.

		bool pre_combat = false;
		Client* test_against = nullptr;

		if (HasGroup() && GetGroup()->GetLeader() && GetGroup()->GetLeader()->IsClient()) {
			test_against = GetGroup()->GetLeader()->CastToClient();
		} else if (GetOwner() && GetOwner()->IsClient()) {
			test_against = GetOwner()->CastToClient();
		}

		if (test_against) {
			pre_combat = test_against->GetBotPrecombat();
		}

		//Ok, IdleCastCheck depends of class.
		switch (GetClass()) {
		// Healers WITHOUT pets will check if a heal is needed before buffing.
		case Class::Cleric:
		case Class::Paladin:
		case Class::Ranger: {
			if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_Cure)) {
				if (!AICastSpell(this, 100, SpellType_Heal)) {
					if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_Heal)) {
						if (!AICastSpell(this, 100, SpellType_Buff)) {
							if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_Buff)) {
							}
						}
					}
				}
			}

			result = true;
			break;
		}
		case Class::Monk:
		case Class::Rogue:
		case Class::Warrior:
		case Class::Berserker: {
			if (!AICastSpell(this, 100, SpellType_Cure)) {
				if (!AICastSpell(this, 100, SpellType_Heal)) {
					if (!AICastSpell(this, 100, SpellType_Buff)) {
						if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_Buff)) {
						}
					}
				}
			}

			result = true;
			break;
		}
		// Pets class will first cast their pet, then buffs

		case Class::Magician:
		case Class::ShadowKnight:
		case Class::Necromancer:
		case Class::Enchanter: {
			if (!AICastSpell(this, 100, SpellType_Pet)) {
				if (!AICastSpell(this, 100, SpellType_Cure)) {
					if (!AICastSpell(GetPet(), 100, SpellType_Cure)) {
						if (!AICastSpell(this, 100, SpellType_Buff)) {
							if (!AICastSpell(GetPet(), 100, SpellType_Heal)) {
								if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_Buff)) {
								}
							}
						}
					}
				}
			}

			result = true;
			break;
		}
		case Class::Druid:
		case Class::Shaman:
		case Class::Beastlord: {
			if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_Cure)) {
				if (!AICastSpell(this, 100, SpellType_Pet)) {
					if (!AICastSpell(this, 100, SpellType_Heal)) {
						if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_Heal)) {
							if (!AICastSpell(this, 100, SpellType_Buff)) {
								if (!AICastSpell(GetPet(), 100, SpellType_Heal)) {
									if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_Buff)) {
									}
								}
							}
						}
					}
				}
			}

			result = true;
			break;
		}
		case Class::Wizard: { // This can eventually be move into the Class::Beastlord case handler once pre-combat is fully implemented
			if (pre_combat) {
				if (!AICastSpell(this, 100, SpellType_Pet)) {
					if (!AICastSpell(this, 100, SpellType_Cure)) {
						if (!AICastSpell(this, 100, SpellType_Heal)) {
							if (!AICastSpell(this, 100, SpellType_Buff)) {
								if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_PreCombatBuff)) {
									if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_Buff)) {
									}
								}
							}
						}
					}
				}
			}
			else {
				if (!AICastSpell(this, 100, SpellType_Cure)) {
					if (!AICastSpell(this, 100, SpellType_Pet)) {
						if (!AICastSpell(this, 100, SpellType_Heal)) {
							if (!AICastSpell(this, 100, SpellType_Buff)) {
								if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_Buff)) {
								}
							}
						}
					}
				}
			}

			result = true;
			break;
		}
		case Class::Bard: {
			if (pre_combat) {
				if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_Cure)) {
					if (!AICastSpell(this, 100, SpellType_Buff)) {
						if (!AICastSpell(this, 100, SpellType_PreCombatBuffSong)) {
							if (!AICastSpell(this, 100, SpellType_InCombatBuffSong)) {
							}
						}
					}
				}
			}
			else {
				if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_Cure)) {
					if (!AICastSpell(this, 100, SpellType_Buff)) {
						if (!AICastSpell(this, 100, SpellType_OutOfCombatBuffSong)) {
							if (!AICastSpell(this, 100, SpellType_InCombatBuffSong)) {
							}
						}
					}
				}
			}

			result = true;
			break;
		}
		default:
			break;
		}

		if (!AIautocastspell_timer->Enabled())
			AIautocastspell_timer->Start(RandomTimer(500, 2000), false); // avg human response is much less than 5 seconds..even for non-combat situations...
	}

	return result;
}

bool Bot::AI_EngagedCastCheck() {
	bool result = false;
	bool failedToCast = false;

	if (GetTarget() && AIautocastspell_timer->Check(false)) {

		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.

		uint8 botClass = GetClass();
		bool mayGetAggro = HasOrMayGetAggro();

		LogAIDetail("Engaged autocast check triggered (BOTS). Trying to cast healing spells then maybe offensive spells");

		if (botClass == Class::Cleric) {
			if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Escape), SpellType_Escape)) {
				if (!AICastSpell(this, GetChanceToCastBySpellType(SpellType_Heal), SpellType_Heal)) {
					if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_Heal), BotAISpellRange, SpellType_Heal)) {
						if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Debuff), SpellType_Debuff)) {
							if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_InCombatBuff), BotAISpellRange, SpellType_InCombatBuff)) {
								if (!AICastSpell(GetTarget(), mayGetAggro?0:GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
									//AIautocastspell_timer->Start(RandomTimer(100, 250), false);		// Do not give healer classes a lot of time off or your tank's die
									failedToCast = true;
								}
							}
						}
					}
				}
			}
		}
		else if (botClass == Class::Druid) {
			if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Escape), SpellType_Escape)) {
				if (!AICastSpell(this, GetChanceToCastBySpellType(SpellType_Heal), SpellType_Heal)) {
					if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_Heal), BotAISpellRange, SpellType_Heal)) {
						if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Debuff), SpellType_Debuff)) {
							if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_InCombatBuff), BotAISpellRange, SpellType_InCombatBuff)) {
								if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_DOT), SpellType_DOT)) {
									if (!AICastSpell(GetTarget(), mayGetAggro?0:GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
										//AIautocastspell_timer->Start(RandomTimer(100, 250), false);		// Do not give healer classes a lot of time off or your tank's die
										failedToCast = true;
									}
								}
							}
						}
					}
				}
			}
		}
		else if (botClass == Class::Shaman) {
			if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Escape), SpellType_Escape)) {
				if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Slow), SpellType_Slow)) {
					if (!AICastSpell(this, GetChanceToCastBySpellType(SpellType_Heal), SpellType_Heal)) {
						if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_Heal), BotAISpellRange, SpellType_Heal)) {
							if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Debuff), SpellType_Debuff)) {
								if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_InCombatBuff), BotAISpellRange, SpellType_InCombatBuff)) {
									if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_DOT), SpellType_DOT)) {
										if (!AICastSpell(GetTarget(), mayGetAggro?0:GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
											if (!AICastSpell(GetPet(), GetChanceToCastBySpellType(SpellType_Heal), SpellType_Heal)) {
												failedToCast = true;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		else if (botClass == Class::Ranger) {
			if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Escape), SpellType_Escape)) {
				if (!AICastSpell(this, GetChanceToCastBySpellType(SpellType_Heal), SpellType_Heal)) {
					if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_InCombatBuff), BotAISpellRange, SpellType_InCombatBuff)) {
						if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_DOT), SpellType_DOT)) {
							if (!AICastSpell(GetTarget(), mayGetAggro?0:GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
								if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_Heal), BotAISpellRange, SpellType_Heal)) {
									failedToCast = true;
								}
							}
						}
					}
				}
			}
		}
		else if (botClass == Class::Beastlord) {
			if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Slow), SpellType_Slow)) {
				if (!AICastSpell(this, GetChanceToCastBySpellType(SpellType_Heal), SpellType_Heal)) {
					if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Debuff), SpellType_Debuff)) {
						if (!AICastSpell(this, GetChanceToCastBySpellType(SpellType_Pet), SpellType_Pet)) {
							if (!AICastSpell(GetPet(), GetChanceToCastBySpellType(SpellType_Heal), SpellType_Heal)) {
								if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_InCombatBuff), BotAISpellRange, SpellType_InCombatBuff)) {
									if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_DOT), SpellType_DOT)) {
										if (!AICastSpell(GetTarget(), mayGetAggro?0:GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
											if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_Heal), BotAISpellRange, SpellType_Heal)) {
												failedToCast = true;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		else if (botClass == Class::Wizard) {
			if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Escape), SpellType_Escape)) {
				if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Debuff), SpellType_Debuff)) {
					if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_InCombatBuff), BotAISpellRange, SpellType_InCombatBuff)) {
						if (!AICastSpell(GetTarget(), mayGetAggro?0:GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
							failedToCast = true;
						}
					}
				}
			}
		}
		else if (botClass == Class::Paladin) {
			if (!AICastSpell(this, GetChanceToCastBySpellType(SpellType_Heal), SpellType_Heal)) {
				if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_InCombatBuff), BotAISpellRange, SpellType_InCombatBuff)) {
					if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Debuff), SpellType_Debuff)) {
						if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_Heal), BotAISpellRange, SpellType_Heal)) {
							if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
								failedToCast = true;
							}
						}
					}
				}
			}
		}
		else if (botClass == Class::ShadowKnight) {
			if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Lifetap), SpellType_Lifetap)) {
				if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_InCombatBuff), BotAISpellRange, SpellType_InCombatBuff)) {
					if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Debuff), SpellType_Debuff)) {
						if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_DOT), SpellType_DOT)) {
							if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
								failedToCast = true;
							}
						}
					}
				}
			}
		}
		else if (botClass == Class::Magician) {
			if (!AICastSpell(this, GetChanceToCastBySpellType(SpellType_Pet), SpellType_Pet)) {
				if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Debuff), SpellType_Debuff)) {
					if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_InCombatBuff), BotAISpellRange, SpellType_InCombatBuff)) {
						if (!AICastSpell(GetPet(), GetChanceToCastBySpellType(SpellType_Heal), SpellType_Heal)) {
							if (!AICastSpell(GetTarget(), mayGetAggro?0:GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
								failedToCast = true;
							}
						}
					}
				}
			}
		}
		else if (botClass == Class::Necromancer) {
			if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Escape), SpellType_Escape)) {
				if (!AICastSpell(this, GetChanceToCastBySpellType(SpellType_Pet), SpellType_Pet)) {
					if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Debuff), SpellType_Debuff)) {
						if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Lifetap), SpellType_Lifetap)) {
							if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_InCombatBuff), BotAISpellRange, SpellType_InCombatBuff)) {
								if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_DOT), SpellType_DOT)) {
									if (!AICastSpell(GetPet(), GetChanceToCastBySpellType(SpellType_Heal), SpellType_Heal)) {
										if (!AICastSpell(GetTarget(), mayGetAggro?0:GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
											failedToCast = true;
										}
									}
								}
							}
						}
					}
				}
			}
		}
		else if (botClass == Class::Enchanter) {
			if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Mez), SpellType_Mez)) {
				if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Escape), SpellType_Escape)) {
					if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Slow), SpellType_Slow)) {
						if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Debuff), SpellType_Debuff)) {
							if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_InCombatBuff), BotAISpellRange, SpellType_InCombatBuff)) {
								if (!AICastSpell(GetTarget(), mayGetAggro?0:GetChanceToCastBySpellType(SpellType_DOT), SpellType_DOT)) {
									if (!AICastSpell(GetTarget(), mayGetAggro?0:GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
										failedToCast = true;
									}
								}
							}
						}
					}
				}
			}
		}
		else if (botClass == Class::Bard) {
			if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Escape), SpellType_Escape)) {// Bards will use their escape songs
				if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_HateRedux), BotAISpellRange, SpellType_HateRedux)) {
					if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Slow), SpellType_Slow)) {
						if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Debuff), SpellType_Debuff)) {
							if (!AICastSpell(this, GetChanceToCastBySpellType(SpellType_InCombatBuffSong), SpellType_InCombatBuffSong)) {
								if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_InCombatBuff), BotAISpellRange, SpellType_InCombatBuff)) {
									if (!AICastSpell(GetTarget(), mayGetAggro ? 0 : GetChanceToCastBySpellType(SpellType_DOT), SpellType_DOT)) {// Bards will use their dot songs
										if (!AICastSpell(GetTarget(), mayGetAggro ? 0 : GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {// Bards will use their nuke songs
											failedToCast = true;
										}
									}
								}
							}
						}
					}
				}
			}
		}
		else if (botClass == Class::Berserker) {
			if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Escape), SpellType_Escape)) {
				if (!AICastSpell(this, GetChanceToCastBySpellType(SpellType_Heal), SpellType_Heal)) {
					if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Debuff), SpellType_Debuff)) {
						if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_InCombatBuff), BotAISpellRange, SpellType_InCombatBuff)) {
							if (!AICastSpell(this, GetChanceToCastBySpellType(SpellType_InCombatBuffSong), SpellType_InCombatBuffSong)) {
								if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_DOT), SpellType_DOT)) {
									if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
										failedToCast = true;
									}
								}
							}
						}
					}
				}
			}
		}
		else if (botClass == Class::Monk) {
			if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Escape), SpellType_Escape)) {
				if (!AICastSpell(this, GetChanceToCastBySpellType(SpellType_Heal), SpellType_Heal)) {
					if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Debuff), SpellType_Debuff)) {
						if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_InCombatBuff), BotAISpellRange, SpellType_InCombatBuff)) {
							if (!AICastSpell(this, GetChanceToCastBySpellType(SpellType_InCombatBuffSong), SpellType_InCombatBuffSong)) {
								if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_DOT), SpellType_DOT)) {
									if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
										failedToCast = true;
									}
								}
							}
						}
					}
				}
			}
		}
		else if (botClass == Class::Rogue) {
			if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Escape), SpellType_Escape)) {
				if (!AICastSpell(this, GetChanceToCastBySpellType(SpellType_Heal), SpellType_Heal)) {
					if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Debuff), SpellType_Debuff)) {
						if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_InCombatBuff), BotAISpellRange, SpellType_InCombatBuff)) {
							if (!AICastSpell(this, GetChanceToCastBySpellType(SpellType_InCombatBuffSong), SpellType_InCombatBuffSong)) {
								if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_DOT), SpellType_DOT)) {
									if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
										failedToCast = true;
									}
								}
							}
						}
					}
				}
			}
		}
		else if (botClass == Class::Warrior) {
			if (!AICastSpell(this, GetChanceToCastBySpellType(SpellType_Heal), SpellType_Heal)) {
				if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_InCombatBuff), BotAISpellRange, SpellType_InCombatBuff)) {
					if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Debuff), SpellType_Debuff)) {
						if (!AICastSpell(this, GetChanceToCastBySpellType(SpellType_InCombatBuffSong), SpellType_InCombatBuffSong)) {
							if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_DOT), SpellType_DOT)) {
								if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
									failedToCast = true;
								}
							}
						}
					}
				}
			}
		}

		if (!AIautocastspell_timer->Enabled()) {
			AIautocastspell_timer->Start(RandomTimer(150, 300), false);
		}

		if (!failedToCast) {
			result = true;
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
		botSpell = GetBestBotSpellForRegularSingleTargetHeal(this);

		if (!IsValidSpell(botSpell.SpellId))
			botSpell = GetBestBotSpellForFastHeal(this);
	}
	else {
		botSpell = GetBestBotSpellForPercentageHeal(this);

		if (!IsValidSpell(botSpell.SpellId)) {
			botSpell = GetBestBotSpellForRegularSingleTargetHeal(this);
		}
		if (!IsValidSpell(botSpell.SpellId)) {
			botSpell = GetFirstBotSpellForSingleTargetHeal(this);
		}
		if (!IsValidSpell(botSpell.SpellId)) {
			botSpell = GetFirstBotSpellBySpellType(this, SpellType_Heal);
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

std::list<BotSpell> Bot::GetBotSpellsForSpellEffect(Bot* botCaster, int spellEffect) {
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
			if (!IsValidSpell(botSpellList[i].spellid)) {
				// this is both to quit early to save cpu and to avoid casting bad spells
				// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
				continue;
			}

			if (IsEffectInSpell(botSpellList[i].spellid, spellEffect) || GetSpellTriggerSpellID(botSpellList[i].spellid, spellEffect)) {
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

std::list<BotSpell> Bot::GetBotSpellsForSpellEffectAndTargetType(Bot* botCaster, int spellEffect, SpellTargetType targetType) {
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
			if (!IsValidSpell(botSpellList[i].spellid)) {
				// this is both to quit early to save cpu and to avoid casting bad spells
				// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
				continue;
			}

			if (
				(
					IsEffectInSpell(botSpellList[i].spellid, spellEffect) ||
					GetSpellTriggerSpellID(botSpellList[i].spellid, spellEffect)
				) &&
				spells[botSpellList[i].spellid].target_type == targetType
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

std::list<BotSpell> Bot::GetBotSpellsBySpellType(Bot* botCaster, uint32 spellType) {
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
			if (!IsValidSpell(botSpellList[i].spellid)) {
				// this is both to quit early to save cpu and to avoid casting bad spells
				// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
				continue;
			}

			if (botSpellList[i].type & spellType) {
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

std::list<BotSpell_wPriority> Bot::GetPrioritizedBotSpellsBySpellType(Bot* botCaster, uint32 spellType) {
	std::list<BotSpell_wPriority> result;

	if (botCaster && botCaster->AI_HasSpells()) {
		std::vector<BotSpells_Struct> botSpellList = botCaster->AIBot_spells;

		for (int i = botSpellList.size() - 1; i >= 0; i--) {
			if (!IsValidSpell(botSpellList[i].spellid)) {
				// this is both to quit early to save cpu and to avoid casting bad spells
				// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
				continue;
			}

			if (botSpellList[i].type & spellType) {
				BotSpell_wPriority botSpell;
				botSpell.SpellId = botSpellList[i].spellid;
				botSpell.SpellIndex = i;
				botSpell.ManaCost = botSpellList[i].manacost;
				botSpell.Priority = botSpellList[i].priority;

				result.push_back(botSpell);
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

BotSpell Bot::GetFirstBotSpellBySpellType(Bot* botCaster, uint32 spellType) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster && botCaster->AI_HasSpells()) {
		std::vector<BotSpells_Struct> botSpellList = botCaster->AIBot_spells;

		for (int i = botSpellList.size() - 1; i >= 0; i--) {
			if (!IsValidSpell(botSpellList[i].spellid)) {
				// this is both to quit early to save cpu and to avoid casting bad spells
				// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
				continue;
			}

			if ((botSpellList[i].type & spellType) && botCaster->CheckSpellRecastTimer(botSpellList[i].spellid)) {
				result.SpellId = botSpellList[i].spellid;
				result.SpellIndex = i;
				result.ManaCost = botSpellList[i].manacost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForFastHeal(Bot *botCaster) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, SE_CurrentHP);

		for (auto botSpellListItr : botSpellList) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (IsFastHealSpell(botSpellListItr.SpellId) && botCaster->CheckSpellRecastTimer(botSpellListItr.SpellId)) {
				result.SpellId = botSpellListItr.SpellId;
				result.SpellIndex = botSpellListItr.SpellIndex;
				result.ManaCost = botSpellListItr.ManaCost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForHealOverTime(Bot* botCaster) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster) {
		std::list<BotSpell> botHoTSpellList = GetBotSpellsForSpellEffect(botCaster, SE_HealOverTime);
		std::vector<BotSpells_Struct> botSpellList = botCaster->AIBot_spells;

		for (auto botSpellListItr : botHoTSpellList) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (IsHealOverTimeSpell(botSpellListItr.SpellId)) {
				for (int i = botSpellList.size() - 1; i >= 0; i--) {
					if (!IsValidSpell(botSpellList[i].spellid)) {
						// this is both to quit early to save cpu and to avoid casting bad spells
						// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
						continue;
					}

					if (
						botSpellList[i].spellid == botSpellListItr.SpellId &&
						(botSpellList[i].type & SpellType_Heal) &&
						botCaster->CheckSpellRecastTimer(botSpellListItr.SpellId)
					) {
						result.SpellId = botSpellListItr.SpellId;
						result.SpellIndex = botSpellListItr.SpellIndex;
						result.ManaCost = botSpellListItr.ManaCost;
					}
				}

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForPercentageHeal(Bot *botCaster) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster && botCaster->AI_HasSpells()) {
		std::vector<BotSpells_Struct> botSpellList = botCaster->AIBot_spells;

		for (int i = botSpellList.size() - 1; i >= 0; i--) {
			if (!IsValidSpell(botSpellList[i].spellid)) {
				// this is both to quit early to save cpu and to avoid casting bad spells
				// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
				continue;
			}

			if (IsCompleteHealSpell(botSpellList[i].spellid) && botCaster->CheckSpellRecastTimer(botSpellList[i].spellid)) {
				result.SpellId = botSpellList[i].spellid;
				result.SpellIndex = i;
				result.ManaCost = botSpellList[i].manacost;
				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForRegularSingleTargetHeal(Bot* botCaster) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, SE_CurrentHP);

		for (std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (IsRegularSingleTargetHealSpell(botSpellListItr->SpellId) && botCaster->CheckSpellRecastTimer(botSpellListItr->SpellId)) {
				result.SpellId = botSpellListItr->SpellId;
				result.SpellIndex = botSpellListItr->SpellIndex;
				result.ManaCost = botSpellListItr->ManaCost;
				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetFirstBotSpellForSingleTargetHeal(Bot* botCaster) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, SE_CurrentHP);

		for (std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (
				(
					IsRegularSingleTargetHealSpell(botSpellListItr->SpellId) ||
					IsFastHealSpell(botSpellListItr->SpellId)
				) &&
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

BotSpell Bot::GetBestBotSpellForGroupHeal(Bot* botCaster) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, SE_CurrentHP);

		for (std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (
				IsRegularGroupHealSpell(botSpellListItr->SpellId) &&
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

BotSpell Bot::GetBestBotSpellForGroupHealOverTime(Bot* botCaster) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster) {
		std::list<BotSpell> botHoTSpellList = GetBotSpellsForSpellEffect(botCaster, SE_HealOverTime);
		std::vector<BotSpells_Struct> botSpellList = botCaster->AIBot_spells;

		for (std::list<BotSpell>::iterator botSpellListItr = botHoTSpellList.begin(); botSpellListItr != botHoTSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (IsGroupHealOverTimeSpell(botSpellListItr->SpellId)) {

				for (int i = botSpellList.size() - 1; i >= 0; i--) {
					if (!IsValidSpell(botSpellList[i].spellid)) {
						// this is both to quit early to save cpu and to avoid casting bad spells
						// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
						continue;
					}

					if (
						botSpellList[i].spellid == botSpellListItr->SpellId &&
						(botSpellList[i].type & SpellType_Heal) &&
						botCaster->CheckSpellRecastTimer(botSpellListItr->SpellId)
					) {
						result.SpellId = botSpellListItr->SpellId;
						result.SpellIndex = botSpellListItr->SpellIndex;
						result.ManaCost = botSpellListItr->ManaCost;
					}
				}

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForGroupCompleteHeal(Bot* botCaster) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, SE_CompleteHeal);

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (
				IsGroupCompleteHealSpell(botSpellListItr->SpellId) &&
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

BotSpell Bot::GetBestBotSpellForMez(Bot* botCaster) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, SE_Mez);

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

BotSpell Bot::GetBestBotSpellForMagicBasedSlow(Bot* botCaster) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, SE_AttackSpeed);

		for (std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (
				IsSlowSpell(botSpellListItr->SpellId) &&
				spells[botSpellListItr->SpellId].resist_type == RESIST_MAGIC &&
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

BotSpell Bot::GetBestBotSpellForDiseaseBasedSlow(Bot* botCaster) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, SE_AttackSpeed);

		for (std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (
				IsSlowSpell(botSpellListItr->SpellId) &&
				spells[botSpellListItr->SpellId].resist_type == RESIST_DISEASE &&
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

Mob* Bot::GetFirstIncomingMobToMez(Bot* botCaster, BotSpell botSpell) {
	Mob* result = 0;

	if (botCaster && IsMesmerizeSpell(botSpell.SpellId)) {

		std::list<NPC*> npc_list;
		entity_list.GetNPCList(npc_list);

		for(std::list<NPC*>::iterator itr = npc_list.begin(); itr != npc_list.end(); ++itr) {
			NPC* npc = *itr;

			if (DistanceSquaredNoZ(npc->GetPosition(), botCaster->GetPosition()) <= botCaster->GetActSpellRange(botSpell.SpellId, spells[botSpell.SpellId].range)) {
				if (!npc->IsMezzed()) {
					if (botCaster->HasGroup()) {
						Group* g = botCaster->GetGroup();

						if (g) {
							for (int counter = 0; counter < g->GroupCount(); counter++) {
								if (
									npc->IsOnHatelist(g->members[counter]) &&
									g->members[counter]->GetTarget() != npc && g->members[counter]->IsEngaged()) {
									result = npc;
									break;
								}
							}
						}
					}
				}
			}

			if (result)
				break;
		}
	}

	return result;
}

BotSpell Bot::GetBestBotMagicianPetSpell(Bot *botCaster) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, SE_SummonPet);

		std::string petType = GetBotMagicianPetType(botCaster);

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (IsSummonPetSpell(botSpellListItr->SpellId) && botCaster->CheckSpellRecastTimer(botSpellListItr->SpellId)) {
				if (!strncmp(spells[botSpellListItr->SpellId].teleport_zone, petType.c_str(), petType.length())) {
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

std::string Bot::GetBotMagicianPetType(Bot* botCaster) {
	std::string result;

	if (botCaster) {
		if (botCaster->IsPetChooser()) {
			switch(botCaster->GetPetChooserID()) {
				case 0:
					result = std::string("SumWater");
					break;
				case 1:
					result = std::string("SumFire");
					break;
				case 2:
					result = std::string("SumAir");
					break;
				case 3:
					result = std::string("SumEarth");
					break;
				default:
					result = std::string("MonsterSum");
					break;
			}
		}
		else {
			if (botCaster->GetLevel() == 2)
				result = std::string("SumWater");
			else if (botCaster->GetLevel() == 3)
				result = std::string("SumFire");
			else if (botCaster->GetLevel() == 4)
				result = std::string("SumAir");
			else if (botCaster->GetLevel() == 5)
				result = std::string("SumEarth");
			else if (botCaster->GetLevel() < 30) {
				// Under level 30
				int counter = zone->random.Int(0, 3);

				switch(counter) {
					case 0:
					result = std::string("SumWater");
					break;
				case 1:
					result = std::string("SumFire");
					break;
				case 2:
					result = std::string("SumAir");
					break;
				case 3:
					result = std::string("SumEarth");
					break;
				default:
					result = std::string("MonsterSum");
					break;
				}
			}
			else {
				// Over level 30
				int counter = zone->random.Int(0, 4);

				switch(counter) {
					case 0:
					result = std::string("SumWater");
					break;
				case 1:
					result = std::string("SumFire");
					break;
				case 2:
					result = std::string("SumAir");
					break;
				case 3:
					result = std::string("SumEarth");
					break;
				default:
					result = std::string("MonsterSum");
					break;
				}
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForNukeByTargetType(Bot* botCaster, SpellTargetType targetType) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffectAndTargetType(botCaster, SE_CurrentHP, targetType);

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if ((IsPureNukeSpell(botSpellListItr->SpellId) || IsDamageSpell(botSpellListItr->SpellId)) && botCaster->CheckSpellRecastTimer(botSpellListItr->SpellId)) {
				result.SpellId = botSpellListItr->SpellId;
				result.SpellIndex = botSpellListItr->SpellIndex;
				result.ManaCost = botSpellListItr->ManaCost;

				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotSpellForStunByTargetType(Bot* botCaster, SpellTargetType targetType)
{
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster)
	{
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffectAndTargetType(botCaster, SE_Stun, targetType);

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr)
		{
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (IsStunSpell(botSpellListItr->SpellId) && botCaster->CheckSpellRecastTimer(botSpellListItr->SpellId))
			{
				result.SpellId = botSpellListItr->SpellId;
				result.SpellIndex = botSpellListItr->SpellIndex;
				result.ManaCost = botSpellListItr->ManaCost;
				break;
			}
		}
	}

	return result;
}

BotSpell Bot::GetBestBotWizardNukeSpellByTargetResists(Bot* botCaster, Mob* target) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (botCaster && target) {
		const int lureResisValue = -100;
		const int maxTargetResistValue = 300;
		bool selectLureNuke = false;

		if ((target->GetMR() > maxTargetResistValue) && (target->GetCR() > maxTargetResistValue) && (target->GetFR() > maxTargetResistValue))
			selectLureNuke = true;


		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffectAndTargetType(botCaster, SE_CurrentHP, ST_Target);

		BotSpell firstWizardMagicNukeSpellFound;
		firstWizardMagicNukeSpellFound.SpellId = 0;
		firstWizardMagicNukeSpellFound.SpellIndex = 0;
		firstWizardMagicNukeSpellFound.ManaCost = 0;

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			bool spellSelected = false;

			if (botCaster->CheckSpellRecastTimer(botSpellListItr->SpellId)) {
				if (selectLureNuke && (spells[botSpellListItr->SpellId].resist_difficulty < lureResisValue)) {
					spellSelected = true;
				}
				else if (IsPureNukeSpell(botSpellListItr->SpellId)) {
					if (((target->GetMR() < target->GetCR()) || (target->GetMR() < target->GetFR())) && (GetSpellResistType(botSpellListItr->SpellId) == RESIST_MAGIC)
						&& (spells[botSpellListItr->SpellId].resist_difficulty > lureResisValue))
					{
						spellSelected = true;
					}
					else if (((target->GetCR() < target->GetMR()) || (target->GetCR() < target->GetFR())) && (GetSpellResistType(botSpellListItr->SpellId) == RESIST_COLD)
						&& (spells[botSpellListItr->SpellId].resist_difficulty > lureResisValue))
					{
						spellSelected = true;
					}
					else if (((target->GetFR() < target->GetCR()) || (target->GetFR() < target->GetMR())) && (GetSpellResistType(botSpellListItr->SpellId) == RESIST_FIRE)
						&& (spells[botSpellListItr->SpellId].resist_difficulty > lureResisValue))
					{
						spellSelected = true;
					}
					else if ((GetSpellResistType(botSpellListItr->SpellId) == RESIST_MAGIC) && (spells[botSpellListItr->SpellId].resist_difficulty > lureResisValue) && !IsStunSpell(botSpellListItr->SpellId)) {
						firstWizardMagicNukeSpellFound.SpellId = botSpellListItr->SpellId;
						firstWizardMagicNukeSpellFound.SpellIndex = botSpellListItr->SpellIndex;
						firstWizardMagicNukeSpellFound.ManaCost = botSpellListItr->ManaCost;
					}
				}
			}

			if (spellSelected) {
				result.SpellId = botSpellListItr->SpellId;
				result.SpellIndex = botSpellListItr->SpellIndex;
				result.ManaCost = botSpellListItr->ManaCost;

				break;
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
			if (!IsValidSpell(botSpellList[i].spellid)) {
				// this is both to quit early to save cpu and to avoid casting bad spells
				// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
				continue;
			}

			if (((botSpellList[i].type & SpellType_Debuff) || IsDebuffSpell(botSpellList[i].spellid))
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
			if (!IsValidSpell(botSpellList[i].spellid)) {
				// this is both to quit early to save cpu and to avoid casting bad spells
				// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
				continue;
			}

			if (((botSpellList[i].type & SpellType_Debuff) || IsResistDebuffSpell(botSpellList[i].spellid))
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

BotSpell Bot::GetBestBotSpellForCure(Bot* botCaster, Mob *tar) {
	BotSpell_wPriority result;
	bool spellSelected = false;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if (!tar)
		return result;

	int countNeedsCured = 0;
	bool isPoisoned = tar->FindType(SE_PoisonCounter);
	bool isDiseased = tar->FindType(SE_DiseaseCounter);
	bool isCursed = tar->FindType(SE_CurseCounter);
	bool isCorrupted = tar->FindType(SE_CorruptionCounter);

	if (botCaster && botCaster->AI_HasSpells()) {
		std::list<BotSpell_wPriority> cureList = GetPrioritizedBotSpellsBySpellType(botCaster, SpellType_Cure);

		if (tar->HasGroup()) {
			Group *g = tar->GetGroup();

			if (g) {
				for( int i = 0; i<MAX_GROUP_MEMBERS; i++) {
					if (g->members[i] && !g->members[i]->qglobal) {
						if (botCaster->GetNeedsCured(g->members[i]))
							countNeedsCured++;
					}
				}
			}
		}

		//Check for group cure first
		if (countNeedsCured > 2) {
			for (std::list<BotSpell_wPriority>::iterator itr = cureList.begin(); itr != cureList.end(); ++itr) {
				BotSpell selectedBotSpell = *itr;

				if (IsGroupSpell(itr->SpellId) && botCaster->CheckSpellRecastTimer(selectedBotSpell.SpellId)) {
					if (selectedBotSpell.SpellId == 0)
						continue;

					if (isPoisoned && IsEffectInSpell(selectedBotSpell.SpellId, SE_PoisonCounter)) {
						spellSelected = true;
					}
					else if (isDiseased && IsEffectInSpell(selectedBotSpell.SpellId, SE_DiseaseCounter)) {
						spellSelected = true;
					}
					else if (isCursed && IsEffectInSpell(selectedBotSpell.SpellId, SE_CurseCounter)) {
						spellSelected = true;
					}
					else if (isCorrupted && IsEffectInSpell(selectedBotSpell.SpellId, SE_CorruptionCounter)) {
						spellSelected = true;
					}
					else if (IsEffectInSpell(selectedBotSpell.SpellId, SE_DispelDetrimental)) {
						spellSelected = true;
					}

					if (spellSelected)
					{
						result.SpellId = selectedBotSpell.SpellId;
						result.SpellIndex = selectedBotSpell.SpellIndex;
						result.ManaCost = selectedBotSpell.ManaCost;

						break;
					}
				}
			}
		}

		//no group cure for target- try to find single target spell
		if (!spellSelected) {
			for(std::list<BotSpell_wPriority>::iterator itr = cureList.begin(); itr != cureList.end(); ++itr) {
				BotSpell selectedBotSpell = *itr;

				if (botCaster->CheckSpellRecastTimer(selectedBotSpell.SpellId)) {
					if (selectedBotSpell.SpellId == 0)
						continue;

					if (isPoisoned && IsEffectInSpell(selectedBotSpell.SpellId, SE_PoisonCounter)) {
						spellSelected = true;
					}
					else if (isDiseased && IsEffectInSpell(selectedBotSpell.SpellId, SE_DiseaseCounter)) {
						spellSelected = true;
					}
					else if (isCursed && IsEffectInSpell(selectedBotSpell.SpellId, SE_CurseCounter)) {
						spellSelected = true;
					}
					else if (isCorrupted && IsEffectInSpell(selectedBotSpell.SpellId, SE_CorruptionCounter)) {
						spellSelected = true;
					}
					else if (IsEffectInSpell(selectedBotSpell.SpellId, SE_DispelDetrimental)) {
						spellSelected = true;
					}

					if (spellSelected)
					{
						result.SpellId = selectedBotSpell.SpellId;
						result.SpellIndex = selectedBotSpell.SpellIndex;
						result.ManaCost = selectedBotSpell.ManaCost;

						break;
					}
				}
			}
		}
	}

	return result;
}

uint8 Bot::GetChanceToCastBySpellType(uint32 spellType)
{
	uint8 spell_type_index = SPELL_TYPE_COUNT;
	switch (spellType) {
	case SpellType_Nuke:
		spell_type_index = spellTypeIndexNuke;
		break;
	case SpellType_Heal:
		spell_type_index = spellTypeIndexHeal;
		break;
	case SpellType_Root:
		spell_type_index = spellTypeIndexRoot;
		break;
	case SpellType_Buff:
		spell_type_index = spellTypeIndexBuff;
		break;
	case SpellType_Escape:
		spell_type_index = spellTypeIndexEscape;
		break;
	case SpellType_Pet:
		spell_type_index = spellTypeIndexPet;
		break;
	case SpellType_Lifetap:
		spell_type_index = spellTypeIndexLifetap;
		break;
	case SpellType_Snare:
		spell_type_index = spellTypeIndexSnare;
		break;
	case SpellType_DOT:
		spell_type_index = spellTypeIndexDot;
		break;
	case SpellType_Dispel:
		spell_type_index = spellTypeIndexDispel;
		break;
	case SpellType_InCombatBuff:
		spell_type_index = spellTypeIndexInCombatBuff;
		break;
	case SpellType_Mez:
		spell_type_index = spellTypeIndexMez;
		break;
	case SpellType_Charm:
		spell_type_index = spellTypeIndexCharm;
		break;
	case SpellType_Slow:
		spell_type_index = spellTypeIndexSlow;
		break;
	case SpellType_Debuff:
		spell_type_index = spellTypeIndexDebuff;
		break;
	case SpellType_Cure:
		spell_type_index = spellTypeIndexCure;
		break;
	case SpellType_Resurrect:
		spell_type_index = spellTypeIndexResurrect;
		break;
	case SpellType_HateRedux:
		spell_type_index = spellTypeIndexHateRedux;
		break;
	case SpellType_InCombatBuffSong:
		spell_type_index = spellTypeIndexInCombatBuffSong;
		break;
	case SpellType_OutOfCombatBuffSong:
		spell_type_index = spellTypeIndexOutOfCombatBuffSong;
		break;
	case SpellType_PreCombatBuff:
		spell_type_index = spellTypeIndexPreCombatBuff;
		break;
	case SpellType_PreCombatBuffSong:
		spell_type_index = spellTypeIndexPreCombatBuffSong;
		break;
	default:
		break;
	}

	if (spell_type_index >= SPELL_TYPE_COUNT)
		return 0;

	uint8 class_index = GetClass();
	if (class_index > Class::Berserker || class_index < Class::Warrior)
		return 0;
	--class_index;

	uint32 stance_id = GetBotStance();
	if (!Stance::IsValid(stance_id)) {
		return 0;
	}

	uint8 stance_index = Stance::GetIndex(stance_id);
	uint8 type_index = nHSND;

	if (HasGroup()) {
		if (IsGroupHealer()/* || IsRaidHealer()*/)
			type_index |= pH;
		if (IsGroupSlower()/* || IsRaidSlower()*/)
			type_index |= pS;
		if (IsGroupNuker()/* || IsRaidNuker()*/)
			type_index |= pN;
		if (IsGroupDoter()/* || IsRaidDoter()*/)
			type_index |= pD;
	}

	return database.botdb.GetSpellCastingChance(spell_type_index, class_index, stance_index, type_index);
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
				if (!(entry.type & SPELL_TYPES_INNATE) && entry.priority == 0) {
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

bool Bot::HasBotSpellEntry(uint16 spellid) {
	auto* spell_list = content_db.GetBotSpells(GetBotSpellID());

	if (!spell_list) {
		return false;
	}

	// Check if Spell ID is found in Bot Spell Entries
	for (auto& e : spell_list->entries) {
		if (spellid == e.spellid) {
			return true;
		}
	}

	return false;
}

bool Bot::IsValidSpellRange(uint16 spell_id, Mob const* tar) {
	if (!IsValidSpell(spell_id)) {
		return false;
	}

	if (tar) {
		int spellrange = (GetActSpellRange(spell_id, spells[spell_id].range) * GetActSpellRange(spell_id, spells[spell_id].range));
		if (spellrange >= DistanceSquared(m_Position, tar->GetPosition())) {
			return true;
		}
	}
	return false;
}
