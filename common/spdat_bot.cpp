#include "spdat.h"

bool IsBotSpellTypeDetrimental(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::Nuke:
		case BotSpellTypes::Root:
		case BotSpellTypes::Lifetap:
		case BotSpellTypes::Snare:
		case BotSpellTypes::DOT:
		case BotSpellTypes::Dispel:
		case BotSpellTypes::Mez:
		case BotSpellTypes::Charm:
		case BotSpellTypes::Slow:
		case BotSpellTypes::Debuff:
		case BotSpellTypes::HateRedux:
		case BotSpellTypes::Fear:
		case BotSpellTypes::Stun:
		case BotSpellTypes::AENukes:
		case BotSpellTypes::AERains:
		case BotSpellTypes::AEMez:
		case BotSpellTypes::AEStun:
		case BotSpellTypes::AEDebuff:
		case BotSpellTypes::AESlow:
		case BotSpellTypes::AESnare:
		case BotSpellTypes::AEFear:
		case BotSpellTypes::AEDispel:
		case BotSpellTypes::AERoot:
		case BotSpellTypes::AEDoT:
		case BotSpellTypes::AELifetap:
		case BotSpellTypes::PBAENuke:
		case BotSpellTypes::Lull:
		case BotSpellTypes::AELull:
		case BotSpellTypes::HateLine:
		case BotSpellTypes::AEHateLine:
			return true;
		default:
			return false;
	}

	return false;
}

bool IsBotSpellTypeBeneficial(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::RegularHeal:
		case BotSpellTypes::CompleteHeal:
		case BotSpellTypes::GroupCompleteHeals:
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
		case BotSpellTypes::Buff:
		case BotSpellTypes::Cure:
		case BotSpellTypes::GroupCures:
		case BotSpellTypes::PetCures:
		case BotSpellTypes::DamageShields:
		case BotSpellTypes::InCombatBuffSong:
		case BotSpellTypes::OutOfCombatBuffSong:
		case BotSpellTypes::Pet:
		case BotSpellTypes::PetBuffs:
		case BotSpellTypes::PreCombatBuff:
		case BotSpellTypes::PreCombatBuffSong:
		case BotSpellTypes::PetDamageShields:
		case BotSpellTypes::PetResistBuffs:
		case BotSpellTypes::ResistBuffs:
		case BotSpellTypes::Resurrect:
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
		case BotSpellTypes::SummonCorpse:
			return true;
		default:
			return false;
	}

	return false;
}

bool BotSpellTypeUsesTargetSettings(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::RegularHeal:
		case BotSpellTypes::CompleteHeal:
		case BotSpellTypes::GroupCompleteHeals:
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
		case BotSpellTypes::Buff:
		case BotSpellTypes::Cure:
		case BotSpellTypes::GroupCures:
		case BotSpellTypes::PetCures:
		case BotSpellTypes::DamageShields:
		case BotSpellTypes::PetDamageShields:
		case BotSpellTypes::PetBuffs:
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
		case BotSpellTypes::SummonCorpse:
			return true;
		default:
			return false;
	}

	return false;
}

bool IsBotSpellTypeInnate(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::AENukes:
		case BotSpellTypes::AERains:
		case BotSpellTypes::PBAENuke:
		case BotSpellTypes::Nuke:
		case BotSpellTypes::AEDispel:
		case BotSpellTypes::Dispel:
		case BotSpellTypes::AERoot:
		case BotSpellTypes::Root:
		case BotSpellTypes::AESlow:
		case BotSpellTypes::Slow:
		case BotSpellTypes::Charm:
		case BotSpellTypes::AEDebuff:
		case BotSpellTypes::Debuff:
		case BotSpellTypes::AEDoT:
		case BotSpellTypes::DOT:
		case BotSpellTypes::AELifetap:
		case BotSpellTypes::Lifetap:
		case BotSpellTypes::AEStun:
		case BotSpellTypes::Stun:
		case BotSpellTypes::AEMez:
		case BotSpellTypes::Mez:
		case BotSpellTypes::Lull:
		case BotSpellTypes::AELull:
		case BotSpellTypes::HateLine:
		case BotSpellTypes::AEHateLine:
			return true;
		default:
			return false;
	}

	return false;
}

bool IsAEBotSpellType(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::AEDebuff:
		case BotSpellTypes::AEFear:
		case BotSpellTypes::AEMez:
		case BotSpellTypes::AENukes:
		case BotSpellTypes::AERains:
		case BotSpellTypes::AESlow:
		case BotSpellTypes::AESnare:
		case BotSpellTypes::AEStun:
		case BotSpellTypes::AEDispel:
		case BotSpellTypes::AEDoT:
		case BotSpellTypes::PBAENuke:
		case BotSpellTypes::AELifetap:
		case BotSpellTypes::AERoot:
		case BotSpellTypes::AEHateLine:
		case BotSpellTypes::AELull:
			return true;
		default:
			return false;
	}

	return false;
}

bool IsGroupBotSpellType(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::GroupCures:
		case BotSpellTypes::GroupCompleteHeals:
		case BotSpellTypes::GroupHeals:
		case BotSpellTypes::GroupHoTHeals:
			return true;
		default:
			return false;
	}

	return false;
}

bool IsGroupTargetOnlyBotSpellType(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::GroupCures:
		case BotSpellTypes::GroupCompleteHeals:
		case BotSpellTypes::GroupHeals:
			return true;
		default:
			return false;
	}

	return false;
}

bool IsPetBotSpellType(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::PetBuffs:
		case BotSpellTypes::PetRegularHeals:
		case BotSpellTypes::PetCompleteHeals:
		case BotSpellTypes::PetFastHeals:
		case BotSpellTypes::PetVeryFastHeals:
		case BotSpellTypes::PetHoTHeals:
		case BotSpellTypes::PetDamageShields:
		case BotSpellTypes::PetResistBuffs:
		case BotSpellTypes::PetCures:
			return true;
		default:
			return false;
	}

	return false;
}

bool IsClientBotSpellType(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::RegularHeal:
		case BotSpellTypes::CompleteHeal:
		case BotSpellTypes::GroupCompleteHeals:
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
		case BotSpellTypes::Buff:
		case BotSpellTypes::Cure:
		case BotSpellTypes::GroupCures:
		case BotSpellTypes::PetCures:
		case BotSpellTypes::DamageShields:
		case BotSpellTypes::PetDamageShields:
		case BotSpellTypes::PetBuffs:
		case BotSpellTypes::ResistBuffs:
		case BotSpellTypes::PetResistBuffs:
			return true;
		default:
			return false;
	}

	return false;
}

bool IsHealBotSpellType(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::VeryFastHeals:
		case BotSpellTypes::FastHeals:
		case BotSpellTypes::RegularHeal:
		case BotSpellTypes::GroupHeals:
		case BotSpellTypes::CompleteHeal:
		case BotSpellTypes::GroupCompleteHeals:
		case BotSpellTypes::HoTHeals:
		case BotSpellTypes::GroupHoTHeals:
		case BotSpellTypes::PetRegularHeals:
		case BotSpellTypes::PetCompleteHeals:
		case BotSpellTypes::PetFastHeals:
		case BotSpellTypes::PetVeryFastHeals:
		case BotSpellTypes::PetHoTHeals:
			return true;
		default:
			return false;
	}

	return false;
}

bool BotSpellTypeRequiresLoS(uint16 spell_type) {
	if (IsAEBotSpellType(spell_type)) { // These gather their own targets later
		return false;
	}

	switch (spell_type) {
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
		case BotSpellTypes::InCombatBuff:
			return false;
		default:
			return true;
	}

	return true;
}

bool BotSpellTypeRequiresTarget(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::Pet:
		case BotSpellTypes::Succor:
			return false;
		default:
			return true;
	}

	return true;
}

bool BotSpellTypeRequiresAEChecks(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::AEMez:
			return false;
		default:
			return true;
	}

	return true;
}

bool RequiresStackCheck(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::VeryFastHeals:
		case BotSpellTypes::PetVeryFastHeals:
		case BotSpellTypes::FastHeals:
		case BotSpellTypes::PetFastHeals:
		case BotSpellTypes::RegularHeal:
		case BotSpellTypes::PetRegularHeals:
		case BotSpellTypes::CompleteHeal:
		case BotSpellTypes::PetCompleteHeals:
		case BotSpellTypes::GroupCompleteHeals:
			return false;
		default:
			return true;
	}

	return true;
}

bool IsCommandedBotSpellType(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::Charm:
		case BotSpellTypes::AEFear:
		case BotSpellTypes::Fear:
		case BotSpellTypes::Resurrect:
		case BotSpellTypes::AELull:
		case BotSpellTypes::Lull:
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
		case BotSpellTypes::SummonCorpse:
			return true;
		default:
			return false;
	}

	return false;
}

bool IsPullingBotSpellType(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::Nuke:
		case BotSpellTypes::Lifetap:
		case BotSpellTypes::Snare:
		case BotSpellTypes::DOT:
		case BotSpellTypes::Dispel:
		case BotSpellTypes::Slow:
		case BotSpellTypes::Debuff:
		case BotSpellTypes::Stun:
		case BotSpellTypes::HateLine:
			return true;
		default:
			return false;
	}

	return false;
}

uint16 GetCorrectBotSpellType(uint16 spell_type, uint16 spell_id) {
	if (!IsValidSpell(spell_id)) {
		return UINT16_MAX;
	}

	uint16 correct_type = UINT16_MAX;
	SPDat_Spell_Struct spell = spells[spell_id];
	std::string teleport_zone = spell.teleport_zone;

	if (IsCharmSpell(spell_id)) {
		correct_type = BotSpellTypes::Charm;
	}
	else if (IsFearSpell(spell_id)) {
		correct_type = BotSpellTypes::Fear;
	}
	else if (IsEffectInSpell(spell_id, SE_Revive)) {
		correct_type = BotSpellTypes::Resurrect;
	}
	else if (IsHarmonySpell(spell_id)) {
		correct_type = BotSpellTypes::Lull;
	}
	else if (
		teleport_zone.compare("") && 
		!IsEffectInSpell(spell_id, SE_GateToHomeCity) && 
		IsBeneficialSpell(spell_id) && 
		(IsEffectInSpell(spell_id, SE_Teleport) || IsEffectInSpell(spell_id, SE_Translocate))
	) {
		correct_type = BotSpellTypes::Teleport;
	}
	else if (
		IsBeneficialSpell(spell_id) && 
		IsEffectInSpell(spell_id, SE_Succor)
	) {
		correct_type = BotSpellTypes::Succor;
	}
	else if (IsEffectInSpell(spell_id, SE_BindAffinity)) {
		correct_type = BotSpellTypes::BindAffinity;
	}
	else if (IsEffectInSpell(spell_id, SE_Identify)) {
		correct_type = BotSpellTypes::Identify;
	}
	else if (
		spell_type == BotSpellTypes::Levitate && 
		IsBeneficialSpell(spell_id) && 
		IsEffectInSpell(spell_id, SE_Levitate)
	) {
		correct_type = BotSpellTypes::Levitate;
	}
	else if (
		spell_type == BotSpellTypes::Rune && 
		IsBeneficialSpell(spell_id) && 
		(IsEffectInSpell(spell_id, SE_AbsorbMagicAtt) || IsEffectInSpell(spell_id, SE_Rune))
	) {
		correct_type = BotSpellTypes::Rune;
	}
	else if (
		spell_type == BotSpellTypes::WaterBreathing && 
		IsBeneficialSpell(spell_id) && 
		IsEffectInSpell(spell_id, SE_WaterBreathing)
	) {
		correct_type = BotSpellTypes::WaterBreathing;
	}
	else if (
		spell_type == BotSpellTypes::Size && 
		IsBeneficialSpell(spell_id) && 
		(IsEffectInSpell(spell_id, SE_ModelSize) || IsEffectInSpell(spell_id, SE_ChangeHeight))
	) {
		correct_type = BotSpellTypes::Size;
	}
	else if (
		spell_type == BotSpellTypes::Invisibility && 
		IsBeneficialSpell(spell_id) && 
		(IsEffectInSpell(spell_id, SE_SeeInvis) || IsInvisibleSpell(spell_id))
	) {
		correct_type = BotSpellTypes::Invisibility;
	}
	else if (
		spell_type == BotSpellTypes::MovementSpeed && 
		IsBeneficialSpell(spell_id) && 
		IsEffectInSpell(spell_id, SE_MovementSpeed)
	) {
		correct_type = BotSpellTypes::MovementSpeed;
	}
	else if (
		!teleport_zone.compare("") && 
		IsBeneficialSpell(spell_id) && 
		(IsEffectInSpell(spell_id, SE_Translocate) || IsEffectInSpell(spell_id, SE_GateToHomeCity))
	) {
		correct_type = BotSpellTypes::SendHome;
	}
	else if (IsEffectInSpell(spell_id, SE_SummonCorpse)) {
		correct_type = BotSpellTypes::SummonCorpse;
	}

	if (correct_type == UINT16_MAX) {
		if (
			IsSummonPetSpell(spell_id) || 
			IsEffectInSpell(spell_id, SE_TemporaryPets)
		) {
			correct_type = BotSpellTypes::Pet;
		}
		else if (IsMesmerizeSpell(spell_id)) {
			correct_type = BotSpellTypes::Mez;
		}
		else if (IsEscapeSpell(spell_id)) {
			correct_type = BotSpellTypes::Escape;
		}
		else if (
			IsDetrimentalSpell(spell_id) && 
			IsEffectInSpell(spell_id, SE_Root)
		) {
			if (IsAnyAESpell(spell_id)) {
				correct_type = BotSpellTypes::AERoot;
			}
			else {
				correct_type = BotSpellTypes::Root;
			}
		}
		else if (
			IsDetrimentalSpell(spell_id) && 
			IsLifetapSpell(spell_id)
		) {
			correct_type = BotSpellTypes::Lifetap;
		}
		else if (
			IsDetrimentalSpell(spell_id) && 
			IsEffectInSpell(spell_id, SE_MovementSpeed)
		) {
			correct_type = BotSpellTypes::Snare;
		}
		else if (
			IsDetrimentalSpell(spell_id) && 
			(IsStackableDOT(spell_id) || IsDamageOverTimeSpell(spell_id))
		) {
			correct_type = BotSpellTypes::DOT;
		}
		else if (IsDispelSpell(spell_id)) {
			correct_type = BotSpellTypes::Dispel;
		}
		else if (
			IsDetrimentalSpell(spell_id) && 
			IsSlowSpell(spell_id)
		) {
			correct_type = BotSpellTypes::Slow;
		}
		else if (
			IsDebuffSpell(spell_id) && 
			!IsHateReduxSpell(spell_id) && 
			!IsHateSpell(spell_id)
		) {
			correct_type = BotSpellTypes::Debuff;
		}
		else if (IsHateReduxSpell(spell_id)) {
			correct_type = BotSpellTypes::HateRedux;
		}
		else if (
			IsDetrimentalSpell(spell_id) && 
			IsHateSpell(spell_id)
		) {
			correct_type = BotSpellTypes::HateLine;
		}
		else if (
			IsBuffSpell(spell_id) &&
			IsBeneficialSpell(spell_id) &&
			IsBardSong(spell_id)
		) {
			if (
				spell_type == BotSpellTypes::InCombatBuffSong ||
				spell_type == BotSpellTypes::OutOfCombatBuffSong ||
				spell_type == BotSpellTypes::PreCombatBuffSong
			) {
				correct_type = spell_type;
			}
			else {
				correct_type = BotSpellTypes::OutOfCombatBuffSong;
			}
		}
		else if (
			!IsBardSong(spell_id) &&
			(
				(IsSelfConversionSpell(spell_id) && spell.buff_duration < 1) ||
				(spell_type == BotSpellTypes::InCombatBuff && IsAnyBuffSpell(spell_id))
			)
		) {
			correct_type = BotSpellTypes::InCombatBuff;
		}
		else if (
			spell_type == BotSpellTypes::PreCombatBuff &&
			IsAnyBuffSpell(spell_id) &&
			!IsBardSong(spell_id)
		) {
			correct_type = BotSpellTypes::PreCombatBuff;
		}
		else if (
			(IsCureSpell(spell_id) && spell_type == BotSpellTypes::Cure) ||
			(IsCureSpell(spell_id) && !IsAnyHealSpell(spell_id))
		) {
			correct_type = BotSpellTypes::Cure;
		}
		else if (IsAnyNukeOrStunSpell(spell_id)) {
			if (IsAnyAESpell(spell_id)) {
				if (IsAERainSpell(spell_id)) {
					correct_type = BotSpellTypes::AERains;
				}
				else if (IsPBAENukeSpell(spell_id)) {
					correct_type = BotSpellTypes::PBAENuke;
				}
				else if (IsStunSpell(spell_id)) {
					correct_type = BotSpellTypes::AEStun;
				}
				else {
					correct_type = BotSpellTypes::AENukes;
				}
			}
			else if (IsStunSpell(spell_id)) {
				correct_type = BotSpellTypes::Stun;
			}
			else {
				correct_type = BotSpellTypes::Nuke;
			}
		}
		else if (IsAnyHealSpell(spell_id)) {
			if (IsGroupSpell(spell_id)) {
				if (IsGroupCompleteHealSpell(spell_id)) {
					correct_type = BotSpellTypes::GroupCompleteHeals;
				}
				else if (IsGroupHealOverTimeSpell(spell_id)) {
					correct_type = BotSpellTypes::GroupHoTHeals;
				}
				else if (IsRegularGroupHealSpell(spell_id)) {
					correct_type = BotSpellTypes::GroupHeals;
				}

				return correct_type;
			}

			if (IsVeryFastHealSpell(spell_id)) {
				correct_type = BotSpellTypes::VeryFastHeals;
			}
			else if (IsFastHealSpell(spell_id)) {
				correct_type = BotSpellTypes::FastHeals;
			}
			else if (IsCompleteHealSpell(spell_id)) {
				correct_type = BotSpellTypes::CompleteHeal;
			}
			else if (IsHealOverTimeSpell(spell_id)) {
				correct_type = BotSpellTypes::HoTHeals;
			}
			else if (IsRegularSingleTargetHealSpell(spell_id)) {
				correct_type = BotSpellTypes::RegularHeal;
			}
			else if (IsRegularPetHealSpell(spell_id)) {
				correct_type = BotSpellTypes::RegularHeal;
			}
		}
		else if (IsAnyBuffSpell(spell_id)) {
			correct_type = BotSpellTypes::Buff;

			if (IsResistanceOnlySpell(spell_id)) {
				correct_type = BotSpellTypes::ResistBuffs;
			}
			else if (IsDamageShieldOnlySpell(spell_id)) {
				correct_type = BotSpellTypes::DamageShields;
			}
		}
	}

	return correct_type;
}

uint16 GetPetBotSpellType(uint16 spell_type) {
	switch (spell_type) {
		case BotSpellTypes::Buff:
			return BotSpellTypes::PetBuffs;
		case BotSpellTypes::RegularHeal:
			return BotSpellTypes::PetRegularHeals;
		case BotSpellTypes::CompleteHeal:
			return BotSpellTypes::PetCompleteHeals;
		case BotSpellTypes::FastHeals:
			return BotSpellTypes::PetFastHeals;
		case BotSpellTypes::VeryFastHeals:
			return BotSpellTypes::PetVeryFastHeals;
		case BotSpellTypes::HoTHeals:
			return BotSpellTypes::PetHoTHeals;
		case BotSpellTypes::Cure:
			return BotSpellTypes::PetCures;
		case BotSpellTypes::DamageShields:
			return BotSpellTypes::PetDamageShields;
		case BotSpellTypes::ResistBuffs:
			return BotSpellTypes::PetResistBuffs;
		default:
			return spell_type;
	}

	return spell_type;
}
