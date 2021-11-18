#include "../common/rulesys.h"
#include "../common/global_define.h"
#include "../common/eqemu_logsys.h"

#include "mob.h"
#include "elixir.h"
#include "zone.h"
#include "groups.h"

extern Zone* zone;

// ElixirCastSpell determines if a spell can be casted by Mob.
// If 0 is returned, spell is valid and no target changing is required
// If a negative value is returned, an error occured. See elixir.h ELIXIR_ prefix const error lookup of reasons
// If 1 is returned, outMob is set to the suggested mob entity
int8 Mob::ElixirCastSpellCheck(uint16 spellID, Mob** outMob)
{
	int manaCurrent = GetMana();
	int manaMax = GetMaxMana();
	int hpCurrent = GetHP();
	int hpMax = GetMaxHP();
	int endCurrent = GetEndurance();
	int endMax = GetMaxEndurance();

	int healPercent = 90;
	if (IsMerc()) healPercent = RuleI(Mercs, MercsElixirHealPercent);
#ifdef BOTS
	if (IsBot()) healPercent = RuleI(Bots, BotsElixirHealPercent);
#endif
	if (healPercent < 1) healPercent = 1;
	if (healPercent > 99) healPercent = 99;

	int aeMinimum = 3;
	if (IsMerc()) aeMinimum = RuleI(Mercs, MercsElixirAEMinimum);
#ifdef BOTS
	if (IsBot()) aeMinimum = RuleI(Bots, BotsElixirAEMinimum);
#endif
	if (aeMinimum < 1) aeMinimum = 1;
	if (aeMinimum > 99) aeMinimum = 99;

	if (IsCorpse()) return ELIXIR_CANNOT_CAST_BAD_STATE;

	bool isHeal = false;
	bool isDebuff = false;
	bool isBuff = false;
	bool isLifetap = false;
	bool isMana = false;
	bool isCharm = false;
	bool isSnare = false;
	bool isSow = false;
	bool isTaunt = false;
	bool isSingleTargetSpell = false;
	bool isPetSummon = false;
	bool isTransport = false;
	bool isGroupSpell = false;
	bool isBardSong = false;
	bool isBeneficial = false;
	bool isStun = false;
	bool isMez = false;
	bool isLull = false;
	long stunDuration = 0;
	long damageAmount = 0;
	long healAmount = 0;
	bool isGroupHated = false;

	int skillID = 0;

	bodyType targetBodyType = BT_NoTarget2;

	const SPDat_Spell_Struct& spDat = spells[spellID];

	Log(Logs::General, Logs::Mercenaries, "%s checking %s vs %s", GetName(), spDat.name, target == nullptr ? "no target" : target->GetCleanName());

	int spellgroup = spDat.type_description_id;
	uint32 ticks = spDat.buff_duration;
	int targets = spDat.aoe_max_targets;
	SpellTargetType targettype = spDat.target_type;
	EQ::skills::SkillType skill = spDat.skill;
	uint16 recourseID = spDat.recourse_link;
	int category = spDat.spell_category;
	int subcategory = spDat.effect_description_id;

	uint32 buffCount = 0;
	Group* grp = GetGroup();
	Raid* raid = GetRaid();

	for (int i = 0; i < EFFECT_COUNT; i++) {
		if (IsBlankSpellEffect(spellID, i)) continue;

		int attr = spDat.effect_id[i];
		int base = spDat.base_value[i];
		int base2 = spDat.limit_value[i];
		int max = spDat.max_value[i];
		int calc = spDat.formula[i];

		if (attr == SE_CurrentHP) { //0
			if (max > 0) { //Heal / HoT
				if (ticks < 5 && base > 0) { //regen
					isHeal = true;
					healAmount = base;
				}
				if (ticks > 0) {
					isBuff = true;
				}
				if (category == 114) { //taps like touch of zlandicar
					isLifetap = true;
				}
			}
			if (base < 0 && damageAmount == 0) {
				damageAmount = -base;
			}
			if (max < 0) { //Nuke / DoT
				damageAmount = -max;
			}
		}

		if (attr == SE_ArmorClass || // 1 ac
			attr == SE_ATK || //2 attack
			attr == SE_STR || //4 str
			attr == SE_DEX || //5 dex
			attr == SE_AGI || //6 agi
			attr == SE_STA || //7 sta
			attr == SE_INT || //8 int
			attr == SE_WIS //9 wis
			) {
			if (base > 0) { //+stat
				isBuff = true;
			}
			if (base < 0) { //-stat
				isDebuff = true;
			}
		}

		if (attr == SE_MovementSpeed) { //3
			if (base > 0) { //+Movement
				isBuff = true;
				isSow = true;
			}
			if (base < 0) { //-Movement
				isDebuff = true;
				isSnare = true;
			}
		}

		if (attr == SE_CHA) { //10 CHA
			if (base > 0 && base < 254) { //+CHA
				isBuff = true;
			}
			if (base < 0) { //-CHA
				isDebuff = true;
			}
		}

		if (attr == SE_AttackSpeed) { //11 attackspeed
			if (base > 0) { //+Haste
				isBuff = true;
			}
			if (base < 0) { //-Haste
				isDebuff = true;
			}
		}

		if (attr == SE_CurrentMana) { //15 Mana
			isMana = true;
		}
		if (attr == SE_Lull) { // pacify (lull)
			isLull = true;
		}
		if (attr == SE_Stun) { //21 stun
			stunDuration = base2;
			if (targettype == ST_AEClientV1 || targettype == ST_AECaster) { // 2 or 
				isSingleTargetSpell = true; //hack to make ae stuns work
			}
		}
		if (attr == SE_Charm) { //23 charm
			isCharm = true;
		}

		if (attr == SE_Gate) { //26 Gate
			isTransport = true;
		}

		if (attr == SE_ChangeFrenzyRad) { //30 frenzy radius reduction (lull)
			isLull = true;
		}

		if (attr == SE_Mez) { //31 Mesmerization
			isMez = true;
		}

		if (attr == SE_SummonPet) { //33 Summon Elemental Pet
			isPetSummon = true;
		}

		if (attr == SE_NecPet) { //71 Summon Skeleton Pet
			isPetSummon = true;
		}

		if (attr == SE_Teleport) { //83 Transport
			isTransport = true;
		}

		if (attr == SE_Harmony) { //86 reaction radius reduction (lull)
			isLull = true;
		}
		if (attr == SE_Succor) { //88 Evac
			isTransport = true;
		}


		if (attr == SE_HealOverTime) { //100 Heal over times
			isHeal = true;
		}

		if (attr == SE_Familiar) { //108 Summon Familiar
			isPetSummon = true;
		}

		if (attr == SE_Hate) { //192 taunt
			isTaunt = true;
		}

		if (attr == SE_SkillAttack) { //193 skill attack
			skillID = spDat.skill;
		}
	}

	if (subcategory == 43 && ticks < 10) { //Health
		isHeal = true;
	}

	if (category == 126) { //Taps
		if (subcategory == 43) { //Health
			isLifetap = true;
		}
	}
	isBeneficial = IsBeneficialSpell(spellID);
	isStun = IsStunSpell(spellID);

	/*
	//TODO TargetTypes:
	case 40: return "AE PC v2";
	case 25: return "AE Summoned";
	case 24: return "AE Undead";
	case 20: return "Targetted AE Tap";
	case  8: return "Targetted AE";
	case  2: return "AE PC v1";
	case  1: return "Line of Sight";
	*/


	if (targettype == ST_Group) { // 41 Group v2
		isGroupSpell = true;
	}
	if (targettype == ST_GroupTeleport) { //3 Group v1
		isGroupSpell = true;
	}

	if (isGroupSpell && IsBardSong(spellID)) {
		isBardSong = true;
	}

	if (targettype == ST_Target) { //5 Single
		isSingleTargetSpell = true;
	}

	if (GetTarget()) {
		targetBodyType = target->bodytype;
	}

	if (targettype == ST_Animal) { //9 Animal
		isSingleTargetSpell = true;
	}

	if (targettype == ST_Undead) { //10 Undead
		isSingleTargetSpell = true;
	}

	if (targettype == ST_Summoned) { //11 Summoned
		isSingleTargetSpell = true;
	}

	if (targettype == ST_Tap) { //13 Lifetap
		isSingleTargetSpell = true;
	}

	if (targettype == ST_Pet) { //14 Pet
		isSingleTargetSpell = true;
	}

	if (targettype == ST_Corpse) { //15 Corpse
		isSingleTargetSpell = true;
	}

	if (targettype == ST_Plant) { //16 Plant
		isSingleTargetSpell = true;
	}

	if (targettype == ST_Giant) { //17 Uber Giants
		isSingleTargetSpell = true;
	}

	if (targettype == ST_Dragon) { //18 Uber Dragons
		isSingleTargetSpell = true;
	}

	if (spDat.mana > 0 && manaCurrent < GetActSpellCost(spellID, spDat.mana)) return ELIXIR_NOT_ENOUGH_MANA;
	if (spDat.endurance_cost > 0 && endCurrent < spDat.endurance_cost) return ELIXIR_NOT_ENOUGH_ENDURANCE;

	if (isLull) return ELIXIR_LULL_IGNORED;
	if (isMez) return ELIXIR_MEZ_IGNORED;
	if (isCharm) return ELIXIR_CHARM_IGNORED;

	if (targettype == ST_Animal) { //16 Animal
		if (target == nullptr) return ELIXIR_NO_TARGET;
		if (targetBodyType != BT_Animal) return ELIXIR_INVALID_TARGET_BODYTYPE;
	}

	if (targettype == ST_Undead) { //10 Undead
		if (target == nullptr) return ELIXIR_NO_TARGET;
		if (targetBodyType != BT_Undead) return ELIXIR_INVALID_TARGET_BODYTYPE;
	}

	if (targettype == ST_Summoned) { //11 Summoned
		if (target == nullptr) return ELIXIR_NO_TARGET;
		if (targetBodyType != BT_Summoned) return ELIXIR_INVALID_TARGET_BODYTYPE;
	}

	if (targettype == ST_Plant) { //Plant
		if (target == nullptr) return ELIXIR_NO_TARGET;
		if (targetBodyType != BT_Plant) return ELIXIR_INVALID_TARGET_BODYTYPE;
	}

	if (isTransport) return ELIXIR_TRANSPORT_IGNORED;

	if (spDat.npc_no_los == 0 && target && isSingleTargetSpell && !isBeneficial && !CheckLosFN(target)) return ELIXIR_NOT_LINE_OF_SIGHT;

	for (int i = 0; i < 4; i++) { // Reagent check
		if (spDat.component[i] < 1) continue;
		if (spDat.component_count[i] < 1) continue;
		if (IsMerc()) continue; //mercs don't have inventory nor require reagents
#ifdef BOTS
		if (IsBot()) continue; //bots don't have inventory nor require reagents
#endif
		return ELIXIR_COMPONENT_REQUIRED;
		//TODO: teach elixir how to check inventory for component in cases it's a client calling this function
	}

	//TODO: CasterRequirement logic
	//DWORD ReqID = pSpell->CasterRequirementID;
	//if (ReqID == 518 && SpawnPctHPs(pChar->pSpawn) > 89) return "not < 90% hp";


	if (skillID == EQ::skills::SkillBackstab) { // 8 backstab
		if (target == nullptr) {
			return ELIXIR_NO_TARGET;
		}
		if (!BehindMob(target)) {
			return ELIXIR_NOT_BEHIND_MOB;
		}
		if (!IsWithinSpellRange(target, spDat.range, spellID)) {
			return ELIXIR_OUT_OF_RANGE;
		}
		return ELIXIR_OK;
	}

	if (recourseID > 0) { //recourse buff attached
		const SPDat_Spell_Struct& spDatRecourse = spells[recourseID];
		if (spDatRecourse.buff_duration > 0) {
			buffCount = GetMaxTotalSlots();
			for (uint32 i = 0; i < buffCount; i++) {
				auto buff = buffs[i];
				if (buff.spellid == SPELL_UNKNOWN) continue;
				if (spells[buff.spellid].buff_duration_formula == DF_Permanent) continue;
				if (buff.ticsremaining < 2) continue;
				if (buff.spellid == recourseID) {
					return ELIXIR_ALREADY_HAVE_BUFF;
				}
				int stackResult = CheckStackConflict(buff.spellid, buff.casterlevel, recourseID, GetLevel(), entity_list.GetMobID(buff.casterid), this, i);
				if (stackResult == -1) {
					return ELIXIR_ALREADY_HAVE_BUFF;
				}
			}
		}
	}

	if (ticks > 0 && !isBeneficial && targettype == ST_Target) { // debuff
		if (target == nullptr) {
			return ELIXIR_NO_TARGET;
		}

		buffCount = target->GetMaxTotalSlots();
		for (uint32 i = 0; i < buffCount; i++) {
			auto buff = target->buffs[i];
			if (buff.spellid == SPELL_UNKNOWN) continue;
			if (spells[buff.spellid].buff_duration_formula == DF_Permanent) continue;
			if (buff.ticsremaining < 2) continue;
			if (buff.spellid == spellID) {
				return ELIXIR_ALREADY_HAVE_BUFF;
			}
			int stackResult = CheckStackConflict(buff.spellid, buff.casterlevel, spellID, GetLevel(), entity_list.GetMobID(buff.casterid), this, i);
			if (stackResult == -1) {
				return ELIXIR_ALREADY_HAVE_BUFF;
			}
		}
	}

	if (spDat.zone_type == 1 && !zone->CanCastOutdoor()) {
		return ELIXIR_ZONETYPE_FAIL;
	}

	if (target) { //do aggro check
		if (target->GetHateAmount(this) > 0) isGroupHated = true;
		if (!isGroupHated) {
			for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
				if (!grp) break;
				if (!grp->members[i]) continue;
				if (target->GetHateAmount(grp->members[i]) == 0) continue;
				isGroupHated = true;
				break;
			}
		}
	}
	//TODO: zone_type 2 check (can't cast outdoors indoor only)

	if (IsEffectInSpell(spellID, SE_Levitate) && !zone->CanLevitate()) {
		return ELIXIR_ZONETYPE_FAIL;
	}

	if (!spDat.can_cast_in_combat) {
		if (IsEngaged()) {
			return ELIXIR_CANNOT_USE_IN_COMBAT;
		}
		if (target == nullptr) {
			return ELIXIR_NO_TARGET;
		}
		buffCount = target->GetMaxTotalSlots();
		for (uint32 i = 0; i < buffCount; i++) {
			auto buff = target->buffs[i];
			if (buff.spellid == SPELL_UNKNOWN) continue;
			if (IsDetrimentalSpell(buff.spellid) && buff.ticsremaining > 0 && !DetrimentalSpellAllowsRest(buff.spellid)) {
				return ELIXIR_CANNOT_USE_IN_COMBAT;
			}
		}
	}

	if (IsDisciplineBuff(spellID)) {
		buffCount = GetMaxTotalSlots();
		for (uint32 i = 0; i < buffCount; i++) {
			auto buff = buffs[i];
			if (buff.spellid == SPELL_UNKNOWN) continue;
			if (spells[buff.spellid].buff_duration_formula == DF_Permanent) continue;
			if (buff.ticsremaining < 2) continue;
			if (buff.spellid == spellID) {
				return ELIXIR_ALREADY_HAVE_BUFF;
			}
			int stackResult = CheckStackConflict(buff.spellid, buff.casterlevel, spellID, GetLevel(), entity_list.GetMobID(buff.casterid), this, i);
			if (stackResult == -1) {
				return ELIXIR_ALREADY_HAVE_BUFF;
			}
		}
	}


	if (isPetSummon) {
		if (HasPet()) return ELIXIR_ALREADY_HAVE_PET;
		return ELIXIR_OK;
	}

	if (targettype == ST_Pet && isHeal) {
		if (!HasPet()) {
			return ELIXIR_NO_PET;
		}
		if (GetPet()->GetHPRatio() > healPercent) {
			return ELIXIR_HP_NOT_LOW;
		}
		return ELIXIR_OK;
	}


	if (isBuff && targettype == ST_Pet && isBeneficial) {
		if (!HasPet()) {
			return ELIXIR_NO_PET;
		}

		buffCount = GetPet()->GetMaxTotalSlots();
		for (uint32 i = 0; i < buffCount; i++) {
			auto buff = GetPet()->buffs[i];
			if (buff.spellid == SPELL_UNKNOWN) continue;
			if (spells[buff.spellid].buff_duration_formula == DF_Permanent) continue;
			if (buff.ticsremaining < 2) continue;
			if (buff.spellid == spellID) {
				return ELIXIR_ALREADY_HAVE_BUFF;
			}
			int stackResult = CheckStackConflict(buff.spellid, buff.casterlevel, spellID, GetLevel(), entity_list.GetMobID(buff.casterid), this, i);
			if (stackResult == -1) {
				return ELIXIR_ALREADY_HAVE_BUFF;
			}
			if (!IsWithinSpellRange(GetPet(), spDat.range, spellID)) {
				return ELIXIR_OUT_OF_RANGE;
			}
		}
		return ELIXIR_OK;
	}

	if (isMana && targettype == ST_Self && ticks <= 0 && !isPetSummon) { // self only regen, like harvest, canni
		if (stunDuration > 0 && isGroupHated) {
			return ELIXIR_CANNOT_USE_IN_COMBAT;
		}

		if (GetManaRatio() > 50) {
			return ELIXIR_MANA_NOT_LOW;
		}

		return ELIXIR_OK;
	}

	if (isLifetap && GetHPRatio() > healPercent) {
		return ELIXIR_HP_NOT_LOW;
	}

	if (isHeal) { //heal logic
		int groupHealCount = 0;
		int healIDPercent = 100;
		// figure out who is lowest HP party member
		if (GetHPRatio() <= healPercent) {
			if (ticks == 0) { // instant heal, just apply				
				*outMob = this;
				healIDPercent = GetHPRatio();
			}
			else { // it's a heal buff, check if player already has it
				bool isBuffNeeded = true;
				buffCount = GetMaxTotalSlots();
				for (uint32 i = 0; i < buffCount; i++) {
					auto buff = buffs[i];
					if (buff.spellid == SPELL_UNKNOWN) continue;
					if (spells[buff.spellid].buff_duration_formula == DF_Permanent) continue;
					if (buff.spellid == spellID) {
						isBuffNeeded = false;
						break;
					}
					int stackResult = CheckStackConflict(buff.spellid, buff.casterlevel, spellID, GetLevel(), entity_list.GetMobID(buff.casterid), this, i);
					if (stackResult == -1) {
						isBuffNeeded = false;
						break;
					}
				}
				if (isBuffNeeded) {
					*outMob = this;
					healIDPercent = GetHPRatio();
				}
			}
		}

		for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
			if (!grp) break;
			if (!grp->members[i]) continue;
			if (grp->members[i]->GetHPRatio() > healPercent) continue;
			if (grp->members[i]->GetHPRatio() > healIDPercent) continue;
			if (!IsWithinSpellRange(grp->members[i], spDat.range, spellID)) continue;

			groupHealCount++;
			if (ticks == 0) { // instant heal, just apply
				*outMob = grp->members[i];
				healIDPercent = grp->members[i]->GetHPRatio();
			}
			else { // it's a heal buff, check if player already has it
				bool isBuffNeeded = true;
				buffCount = grp->members[i]->GetMaxTotalSlots();
				for (uint32 j = 0; j < buffCount; j++) {
					auto buff = grp->members[i]->buffs[j];
					if (buff.spellid == SPELL_UNKNOWN) continue;
					if (spells[buff.spellid].buff_duration_formula == DF_Permanent) continue;
					if (buff.spellid == spellID) {
						isBuffNeeded = false;
						break;
					}
					int stackResult = CheckStackConflict(buff.spellid, buff.casterlevel, spellID, GetLevel(), entity_list.GetMobID(buff.casterid), this, i);
					if (stackResult == -1) {
						isBuffNeeded = false;
						break;
					}
					// TODO: Immune Check						
				}
				if (isBuffNeeded) {
					*outMob = grp->members[i];
					healIDPercent = GetHPRatio();
				}
			}
		}

		if (isGroupSpell && groupHealCount < aeMinimum) {
			return ELIXIR_AE_LIMIT_NOT_MET;
		}

		if (!*outMob) {
			return ELIXIR_NOT_NEEDED;
		}
		return ELIXIR_TARGET_CHANGE;
	}


	if ((targettype == ST_Self || isGroupSpell) && isBeneficial) { //self/group beneficial spell
		if (isGroupHated && !isBardSong) {
			return ELIXIR_CANNOT_USE_IN_COMBAT;
		}
		
		if (ticks <= 4 && GetClass() != BARD) { //don't bother with short duration buffs
			return ELIXIR_NOT_NEEDED;
		}

		if (ticks == 0) {
			return ELIXIR_OK;
		}
		bool isBuffNeeded = true;

		buffCount = GetMaxTotalSlots();
		for (uint32 i = 0; i < buffCount; i++) {
			auto buff = buffs[i];
			if (buff.spellid == SPELL_UNKNOWN) continue;
			if (spells[buff.spellid].buff_duration_formula == DF_Permanent) continue;
			if (buff.ticsremaining < 2) continue;
			if (buff.spellid == spellID) {
				isBuffNeeded = false;
				break;
			}
			int stackResult = CheckStackConflict(buff.spellid, buff.casterlevel, spellID, GetLevel(), entity_list.GetMobID(buff.casterid), this, i);
			if (stackResult == -1) {
				isBuffNeeded = false;
				break;
			}
		}
		if (isBuffNeeded) return ELIXIR_OK;
		if (!isGroupSpell) return ELIXIR_NOT_NEEDED;

		isBuffNeeded = true;
		for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {

			if (!grp) break;
			if (!grp->members[i]) continue;
			buffCount = grp->members[i]->GetMaxTotalSlots();
			for (uint32 j = 0; j < buffCount; j++) {
				auto buff = grp->members[i]->buffs[j];
				if (buff.spellid == SPELL_UNKNOWN) continue;
				if (spells[buff.spellid].buff_duration_formula == DF_Permanent) continue;
				if (buff.ticsremaining < 2) continue;
				if (buff.spellid == spellID) {
					isBuffNeeded = false;
					break;
				}
				int stackResult = CheckStackConflict(buff.spellid, buff.casterlevel, spellID, GetLevel(), entity_list.GetMobID(buff.casterid), this, i);
				if (stackResult == -1) {
					isBuffNeeded = false;
					break;
				}
			}
			if (!isBuffNeeded) continue;
			*outMob = grp->members[i];
			return ELIXIR_TARGET_CHANGE;
		}
		return ELIXIR_NOT_NEEDED;

	}

	if (targettype == ST_Target && isBeneficial && damageAmount == 0) { //single target beneficial spell

		// TODO: add exceptions for combat buffs situation
		bool isCombatBuff = false;

		if (isGroupHated && !isCombatBuff) {
			return ELIXIR_NOT_NEEDED;
		}

		// for the time being, any beneficial non-heal single target spells without a duration are skipped
		// later, we need to add in things like necro mana flow, etc
		if (ticks == 0) {
			return ELIXIR_NOT_NEEDED;
		}
		if (ticks <= 4 && GetClass() != BARD) { //don't bother with short duration buffs
			return ELIXIR_NOT_NEEDED;
		}
		// always self buff first
		bool isBuffNeeded = true;
		buffCount = GetMaxBuffSlots();
		for (uint32 i = 0; i < buffCount; i++) {
			auto buff = buffs[i];
			if (buff.spellid == SPELL_UNKNOWN) continue;
			if (spells[buff.spellid].buff_duration_formula == DF_Permanent) continue;
			if (buff.spellid == spellID) {
				isBuffNeeded = false;
				break;
			}
			int stackResult = CheckStackConflict(buff.spellid, buff.casterlevel, spellID, GetLevel(), entity_list.GetMobID(buff.casterid), this, i);
			if (stackResult == -1) {
				isBuffNeeded = false;
				break;
			}
			// TODO: Immune Check
		}
		if (isBuffNeeded) {
			if (target && target->GetID() == GetID()) {
				return ELIXIR_OK;
			}
			*outMob = this;
			return ELIXIR_TARGET_CHANGE;
		}

		for (int i = 0; i < MAX_GROUP_MEMBERS; i++) {
			if (!grp) break;
			if (!grp->members[i]) continue;
			if (!IsWithinSpellRange(grp->members[i], spDat.range, spellID)) continue;

			bool isBuffNeeded = true;
			buffCount = grp->members[i]->GetMaxTotalSlots();
			for (uint32 j = 0; j < buffCount; j++) {
				auto buff = grp->members[i]->buffs[j];
				if (buff.spellid == SPELL_UNKNOWN) continue;
				if (spells[buff.spellid].buff_duration_formula == DF_Permanent) continue;
				if (buff.spellid == spellID) {
					isBuffNeeded = false;
					break;
				}
				int stackResult = CheckStackConflict(buff.spellid, buff.casterlevel, spellID, GetLevel(), entity_list.GetMobID(buff.casterid), this, i);
				if (stackResult == -1) {
					isBuffNeeded = false;
					break;
				}
				// TODO: Immune Check
			}
			if (isBuffNeeded) {
				if (target && target->GetID() == grp->members[i]->GetID()) {
					return ELIXIR_OK;
				}
				*outMob = grp->members[i];
				return ELIXIR_TARGET_CHANGE;
			}
		}

		return ELIXIR_NOT_NEEDED;
	}

	if (isStun && (targettype == ST_AEClientV1 || targettype == ST_AreaClientOnly || targettype == ST_AECaster)) { // PB AE stun
		int targetCount = 0;
		float sqDistance = spDat.aoe_range * spDat.aoe_range;
		auto hates = GetHateList();
		auto iter = hates.begin();
		for (auto iter : hates) {
			if (sqDistance > 0 && DistanceSquaredNoZ(GetPosition(), iter->entity_on_hatelist->GetPosition()) > sqDistance) continue;
			if (iter->entity_on_hatelist->IsStunned()) continue;
			targetCount++;
		}
		if (targetCount < aeMinimum) {
			return ELIXIR_AE_LIMIT_NOT_MET;
		}
		return ELIXIR_OK;
	}

	if (damageAmount > 0 && (targettype == ST_AEClientV1 || targettype == ST_AreaClientOnly || targettype == ST_AECaster)) { // PB AE DD
		int targetCount = 0;
		float sqDistance = spDat.aoe_range * spDat.aoe_range;
		auto hates = GetHateList();
		auto iter = hates.begin();
		for (auto iter : hates) {
			if (sqDistance > 0 && DistanceSquaredNoZ(GetPosition(), iter->entity_on_hatelist->GetPosition()) > sqDistance) continue;
			targetCount++;
		}
		if (targetCount < aeMinimum) {
			return ELIXIR_AE_LIMIT_NOT_MET;
		}

		return ELIXIR_OK;
	}


	if (damageAmount > 0 && (targettype == ST_TargetAETap || targettype == ST_AETarget)) { // Target AE DD
		if (target == nullptr) {
			return ELIXIR_NO_TARGET;
		}
		if (!IsWithinSpellRange(target, spDat.range, spellID)) {
			return ELIXIR_OUT_OF_RANGE;
		}

		int targetCount = 0;
		float sqDistance = spDat.aoe_range * spDat.aoe_range;
		auto hates = GetHateList();
		auto iter = hates.begin();
		for (auto iter : hates) {
			if (sqDistance > 0 && DistanceSquaredNoZ(target->GetPosition(), iter->entity_on_hatelist->GetPosition()) > sqDistance) continue;
			targetCount++;
		}
		if (targetCount < aeMinimum) {
			return ELIXIR_AE_LIMIT_NOT_MET;
		}

		return ELIXIR_OK;
	}


	if ((targettype == ST_Target || targettype == ST_Tap) && !isBeneficial) { // single target detrimental spell
		if (target == nullptr) {
			return ELIXIR_NO_TARGET;
		}

		if (!isGroupHated) {
			return ELIXIR_NO_HATE;
		}

		if (target->GetHPRatio() <= 0) {
			return ELIXIR_HP_NOT_LOW;
		}

		if (!IsWithinSpellRange(target, spDat.range, spellID)) {
			return ELIXIR_OUT_OF_RANGE;
		}

		if (target->IsMezzed()) {
			return ELIXIR_TARGET_MEZZED;
		}

		if (!IsAttackAllowed(target)) {
			return ELIXIR_ATTACK_NOT_ALLOWED;
		}

		if (isStun) {
			if (target->IsStunned()) {
				return ELIXIR_TARGET_ALREADY_STUNNED;
			}
			return ELIXIR_OK;
		}

		if (ticks == 0) {
			return ELIXIR_OK;
		}
		buffCount = target->GetMaxTotalSlots();
		for (uint32 i = 0; i < buffCount; i++) {
			auto buff = target->buffs[i];
			if (buff.spellid == SPELL_UNKNOWN) continue;
			if (spells[buff.spellid].buff_duration_formula == DF_Permanent) continue;
			if (buff.spellid == spellID) {
				return ELIXIR_NOT_NEEDED;
			}
			int stackResult = CheckStackConflict(buff.spellid, buff.casterlevel, spellID, GetLevel(), entity_list.GetMobID(buff.casterid), this, i);
			if (stackResult == -1) {
				return ELIXIR_NOT_NEEDED;
			}
			// TODO: Immune Check
		}
		return ELIXIR_OK;
	}

	return ELIXIR_UNHANDLED_SPELL;
}

/*
	for (int i = 0; i < MAX_RAID_MEMBERS; i++) {
		if (!raid) break;
		if (!raid->members[i]) continue;
		if (!raid->members[i].member) continue;
		//raid->members[i].GroupNumber == gid
	}
*/
