/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemulator.net)

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

#if EQDEBUG >= 5
//#define TUNE_DEBUG 20
#endif

#include "../common/global_define.h"
#include "../common/eq_constants.h"
#include "../common/eq_packet_structs.h"
#include "../common/rulesys.h"
#include "../common/skills.h"
#include "../common/spdat.h"
#include "../common/string_util.h"
#include "../common/data_verification.h"
#include "../common/misc_functions.h"
#include "queryserv.h"
#include "quest_parser_collection.h"
#include "string_ids.h"
#include "water_map.h"
#include "worldserver.h"
#include "zone.h"
#include "lua_parser.h"
#include "fastmath.h"
#include "mob.h"
#include "npc.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <boost/concept_check.hpp>

#ifdef BOTS
#include "bot.h"
#endif

extern QueryServ* QServ;
extern WorldServer worldserver;
extern FastMath g_Math;

#ifdef _WINDOWS
#define snprintf	_snprintf
#define strncasecmp	_strnicmp
#define strcasecmp	_stricmp
#endif

extern EntityList entity_list;
extern Zone* zone;


void Mob::Tune_FindATKByPctMitigation(Mob* defender,Mob *attacker, float pct_mitigation, int interval, int max_loop, int ac_override, int Msg)
{
	/*Find the amount of 'ATTACK' stat that has to be added/subtracted FROM ATTACKER to reach a specific average mitigation value on the TARGET.
	Can use ac_override to find the value verse a hypothetical amount of worn AC */

	int atk = 0;
	uint32 total_damage = 0;
	int32 damage = 0;
	uint32 minhit = 0;
	int mean_dmg = 0;
	float tmp_pct_mitigated = 0.0f;
	int end = 0;

	if (attacker->IsNPC())
	{
		damage = static_cast<int32>(attacker->CastToNPC()->GetMaxDMG());
		minhit = attacker->CastToNPC()->GetMinDMG();
	}
	else if (attacker->IsClient())
	{
		damage = static_cast<int32>(attacker->CastToClient()->GetMeleeDamage(this));
		minhit = attacker->CastToClient()->GetMeleeDamage(this, true);
	}

	if (damage == 0 || minhit == 0)
	{
		Message(0, "#Tune - Processing... Abort! Damage not found! [MaxDMG %i MinDMG %i]", damage,minhit);
		return;
	}

	mean_dmg = defender->Tune_GetMeanDamage(this, attacker, damage, minhit, nullptr, 0, ac_override, 0, 0,atk);
	tmp_pct_mitigated = 100.0f - static_cast<float>( mean_dmg * 100 /damage);

	Message(0, "#Tune - Begin Parse [Interval %i Max Loop Iterations %i]", interval, max_loop);
	Message(0, "#Tune - Processing... Find ATK for attacker Mitigation (%.0f) pct on defender [MaxDMG %i MinDMG %i Current Mitigation %.2f]", pct_mitigation, damage, minhit,tmp_pct_mitigated);

	if (tmp_pct_mitigated < pct_mitigation)
		interval = interval * -1;

	for (int j=0; j < max_loop; j++) 
	{
		mean_dmg = defender->Tune_GetMeanDamage(this, attacker, damage, minhit, nullptr, 0, ac_override,0, 0,atk);
		tmp_pct_mitigated = 100.0f - ( static_cast<float>(mean_dmg) * 100.0f /static_cast<float>(damage));

		if (Msg >= 3)
			Message(0, "#Tune - Processing... [%i] [ATK %i] Average Melee Hit  %i | Pct Mitigated %.2f ",j,atk, mean_dmg, tmp_pct_mitigated);

		if (interval > 0 && tmp_pct_mitigated <= pct_mitigation)
			end = 1;
				
		else if (interval < 0 && tmp_pct_mitigated >= pct_mitigation)
			end = 1;
		
		else if (interval < 0 && mean_dmg == minhit)
			end = 2;

		if (end >= 1){

			defender->Tune_MeleeMitigation(this, attacker, damage, minhit, nullptr,Msg,ac_override, 0, 0, atk);
			
			if (end == 2)
				Message(0, "#Tune - [WARNING] Mitigation can not be further decreased due to minium hit value (%i).",minhit);

			if (attacker->IsNPC()){
				Message(0, "#Tune - Recommended NPC ATK ADJUSTMENT ( %i ) on ' %s ' average mitigation of (%.0f) pct verse ' %s '. ",atk, attacker->GetCleanName(), pct_mitigation, defender->GetCleanName());
				Message(0, "#SET: [NPC Attack STAT] = [%i]",atk + defender->CastToNPC()->ATK);
			}
			if (attacker->IsClient()){
				Message(0, "#Tune - Recommended CLIENT ATK ADJUSTMENT ( %i ) on ' %s ' average mitigation of (%.0f) pct verse ' %s '. ", atk, attacker->GetCleanName(), pct_mitigation, defender->GetCleanName());
				Message(0, "#Modify (+/-): [Client Attack STAT/SE_ATK(2)] [%i]",atk);
			}

			return;
		}

		atk = atk + interval;
	}

	Message(0, "#Tune - Error: Unable to find desired result for (%.0f) pct - Increase interval (%i) AND/OR max loop value (%i) and run again.", pct_mitigation, interval, max_loop);
	Message(0, "#Tune - Parse ended at ATK ADJUSTMENT ( %i ) average target mitigation of (%.0f) pct.",atk,tmp_pct_mitigated);
}

void Mob::Tune_FindACByPctMitigation(Mob* defender, Mob *attacker, float pct_mitigation, int interval, int max_loop, int atk_override, int Msg)
{

	/*Find the amount of AC stat that has to be added/subtracted from TARGET to reach a specific average mitigation value based on ATTACKER's statistics.
	Can use ac_override to find the value verse a hypothetical amount of worn AC */

	int add_ac = 0;
	uint32 total_damage = 0;
	int32 damage = 0;
	uint32 minhit = 0;
	int mean_dmg = 0;
	float tmp_pct_mitigated = 0.0f;
	int end = 0;


	if (attacker->IsNPC())
	{
		damage = static_cast<int32>(attacker->CastToNPC()->GetMaxDMG());
		minhit = attacker->CastToNPC()->GetMinDMG();
	}
	else if (attacker->IsClient())
	{
		damage = static_cast<int32>(attacker->CastToClient()->GetMeleeDamage(this));
		minhit = attacker->CastToClient()->GetMeleeDamage(this, true);
	}

	if (damage == 0 || minhit == 0)
	{
		Message(0, "#Tune - Processing... Abort! Damage not found! [MaxDMG %i MinDMG %i]", damage,minhit);
		return;
	}

	mean_dmg = defender->Tune_GetMeanDamage(this, attacker, damage, minhit, nullptr, 0, 0, atk_override);
	tmp_pct_mitigated = 100.0f - static_cast<float>( mean_dmg * 100 /damage);

	Message(0, "#Tune - Begin Parse [Interval %i Max Loop Iterations %i]", interval, max_loop);
	Message(0, "#Tune - Processing... Find AC for defender Mitigation (%.0f) pct from attacker [MaxDMG %i MinDMG %i Current Mitigation %.2f]", pct_mitigation, damage, minhit,tmp_pct_mitigated);


	if (tmp_pct_mitigated > pct_mitigation)
		interval = interval * -1;

	for (int j=0; j < max_loop; j++) 
	{
		mean_dmg = defender->Tune_GetMeanDamage(this, attacker, damage, minhit, nullptr, 0, 0,atk_override, add_ac, 0);
		tmp_pct_mitigated = 100.0f - ( static_cast<float>(mean_dmg) * 100.0f /static_cast<float>(damage));

		if (Msg >= 3)
			Message(0, "#Tune - Processing... [%i] [AC %i] Average Melee Hit  %i | Pct Mitigated %.2f ",j,add_ac, mean_dmg, tmp_pct_mitigated);

		if (interval > 0 && tmp_pct_mitigated >= pct_mitigation)
			end = 1;

		else if (interval < 0 && tmp_pct_mitigated <= pct_mitigation)
			end = 1;

		else if (interval < 0 && mean_dmg == minhit)
			end = 2;

		if (end >= 1){

			defender->Tune_MeleeMitigation(this, attacker, damage, minhit, nullptr,Msg,0,atk_override, add_ac, 0);
			
			if (end == 2)
				Message(0, "#Tune - [WARNING] Mitigation can not be further decreased due to minium hit value (%i).",minhit);

			if (defender->IsNPC()){
				Message(Chat::LightGray, "#Tune - Recommended NPC AC ADJUSTMENT ( %i ) on ' %s ' for an average mitigation of (+ %.0f) pct from attacker ' %s '.",add_ac,defender->GetCleanName(), pct_mitigation, attacker->GetCleanName());
				Message(0, "#SET: [NPC Attack STAT] = [%i]",add_ac + defender->CastToNPC()->GetRawAC());
			}
			if (defender->IsClient()){
				Message(Chat::LightGray, "#Tune - Recommended CLIENT AC ADJUSTMENT ( %i ) on ' %s ' for an average mitigation of (+ %.0f) pct from attacker ' %s '.",add_ac,defender->GetCleanName(), pct_mitigation, attacker->GetCleanName());
				Message(0, "#Modify (+/-): [Client AC STAT/SE_AC(1)] [%i]",add_ac);
			}

			return;
		}



		add_ac = add_ac + interval;
	}

	Message(0, "#Tune - Error: Unable to find desired result for (%.0f) pct - Increase interval (%i) AND/OR max loop value (%i) and run again.", pct_mitigation, interval, max_loop);
	Message(0, "#Tune - Parse ended at AC ADJUSTMENT ( %i ) at average mitigation of (%.0f) / (%.0f) pct.",add_ac,tmp_pct_mitigated / pct_mitigation);
}

uint32 Mob::Tune_GetMeanDamage(Mob* GM, Mob *attacker, int32 damage, int32 minhit, ExtraAttackOptions *opts, int Msg,
							   int ac_override, int atk_override, int add_ac, int add_atk)
{
	uint32 total_damage = 0;
	int loop_max = 1000;

	for (int i=0; i < loop_max ; i++) 
	{
		total_damage += Tune_MeleeMitigation(GM, attacker, damage, minhit, nullptr,0,ac_override, atk_override, add_ac, add_atk);
	}

	return(total_damage/loop_max);
}

int32 Mob::Tune_MeleeMitigation(Mob* GM, Mob *attacker, int32 damage, int32 minhit, ExtraAttackOptions *opts, int Msg, 
								int ac_override, int atk_override, int add_ac, int add_atk)
{
	if (damage <= 0)
		return 0;

	Mob* defender = this;
	float aa_mit = (aabonuses.CombatStability + itembonuses.CombatStability +
			spellbonuses.CombatStability) / 100.0f;

	if (Msg){
		
		GM->Message(Chat::White, "######### Melee Mitigation Report: Start [Detail Level %i]#########", Msg);
		GM->Message(Chat::White, "#ATTACKER: %s", attacker->GetCleanName());
		GM->Message(Chat::White, "#DEFENDER: %s", defender->GetCleanName());
	}

	if (RuleB(Combat, UseIntervalAC)) {
		float softcap = (GetSkill(EQ::skills::SkillDefense) + GetLevel()) *
			RuleR(Combat, SoftcapFactor) * (1.0 + aa_mit);
		float mitigation_rating = 0.0;
		float attack_rating = 0.0;
		int shield_ac = 0;
		int armor = 0;
		float weight = 0.0;

		if (Msg >= 2){
			GM->Message(Chat::White, " ");
			GM->Message(Chat::White, "### Calculate Mitigation Rating ###");
			if (aabonuses.CombatStability)
				GM->Message(Chat::White, "# %i #### DEFENDER SE_CombatStability(259) AA Bonus", aabonuses.CombatStability);
			if (spellbonuses.CombatStability)
				GM->Message(Chat::White, "# %i #### DEFENDER SE_CombatStability(259) Spell Bonus", spellbonuses.CombatStability);
			if (itembonuses.CombatStability)
				GM->Message(Chat::White, "# %i #### DEFENDER SE_CombatStability(259) Worn Bonus", itembonuses.CombatStability);

			GM->Message(Chat::White, "# %.2f #### DEFENDER Base Soft Cap", softcap);
		}

		float monkweight = RuleI(Combat, MonkACBonusWeight);
		monkweight = mod_monk_weight(monkweight, attacker);

		if (IsClient()) {
			armor = CastToClient()->GetRawACNoShield(shield_ac) + add_ac;
			weight = (CastToClient()->CalcCurrentWeight() / 10.0);

			if (ac_override)
				armor = ac_override;
			
			if (Msg >=2 ){
				GM->Message(Chat::White, "# %i #### DEFENDER AC Equiped/Worn Bonus", itembonuses.AC);
				GM->Message(Chat::White, "# %i #### DEFENDER SE_ArmorClass(1) AA Bonus", aabonuses.AC);
				GM->Message(Chat::White, "# %i #### DEFENDER SE_ArmorClass(1) Spell Bonus", spellbonuses.AC);
				GM->Message(Chat::White, "# %i #### DEFENDER Shield AC", shield_ac);
				GM->Message(Chat::White, "# %i #### DEFENDER Total Client Armor - NO shield", armor);
			}
			
		} else if (IsNPC()) {
			armor = CastToNPC()->GetRawAC() + add_ac;

			if (ac_override)
				armor = ac_override;

			if (Msg >=2 ){
				GM->Message(Chat::White, "# %i #### DEFENDER AC Equiped/Worn Bonus", itembonuses.AC);
				GM->Message(Chat::White, "# %i #### DEFENDER SE_ArmorClass(1) Spell Bonus", spellbonuses.AC);
				GM->Message(Chat::White, "# %i #### DEFENDER NPC AC Stat", CastToNPC()->GetRawAC());
			}

			int PetACBonus = 0;

			if (!IsPet()){
				armor = (armor / RuleR(Combat, NPCACFactor));
				if (Msg >=2 )
					GM->Message(Chat::White, "# %i #### DEFENDER NPC Armor after RuleR(Combat, NPCACFactor) %.2f", armor, RuleR(Combat, NPCACFactor));
			}

			Mob *owner = nullptr;
			if (IsPet())
				owner = GetOwner();
			else if ((CastToNPC()->GetSwarmOwner()))
				owner = entity_list.GetMobID(CastToNPC()->GetSwarmOwner());

			if (owner){
				PetACBonus = owner->aabonuses.PetMeleeMitigation + owner->itembonuses.PetMeleeMitigation + owner->spellbonuses.PetMeleeMitigation;

				if (Msg >=2 ){
				if (owner->aabonuses.PetMeleeMitigation)
					GM->Message(Chat::White, "# %i #### DEFENDER Pet Owner SE_PetMeleeMitigation(379) AA Bonus", owner->aabonuses.PetMeleeMitigation);
				if (owner->spellbonuses.PetMeleeMitigation)
					GM->Message(Chat::White, "# %i #### DEFENDER Pet Owner SE_PetMeleeMitigation(379) Spell Bonus",owner->spellbonuses.PetMeleeMitigation);
				if (owner->itembonuses.PetMeleeMitigation)
					GM->Message(Chat::White, "# %i #### DEFENDER Pet Owner SE_PetMeleeMitigation(379) Worn Bonus", owner->itembonuses.PetMeleeMitigation);
				}
			}

			armor += spellbonuses.AC + itembonuses.AC + PetACBonus + 1;

			if (Msg >= 2)
				GM->Message(Chat::White, "# %i #### DEFENDER NPC Total Base Armor",armor);
		}

		if (opts) {
			armor *= (1.0f - opts->armor_pen_percent);
			armor -= opts->armor_pen_flat;
		}

		if (RuleB(Combat, OldACSoftcapRules)) {
			if (GetClass() == WIZARD || GetClass() == MAGICIAN ||
					GetClass() == NECROMANCER || GetClass() == ENCHANTER)
				softcap = RuleI(Combat, ClothACSoftcap);
			else if (GetClass() == MONK && weight <= monkweight)
				softcap = RuleI(Combat, MonkACSoftcap);
			else if(GetClass() == DRUID || GetClass() == BEASTLORD || GetClass() == MONK)
				softcap = RuleI(Combat, LeatherACSoftcap);
			else if(GetClass() == SHAMAN || GetClass() == ROGUE ||
					GetClass() == BERSERKER || GetClass() == RANGER)
				softcap = RuleI(Combat, ChainACSoftcap);
			else
				softcap = RuleI(Combat, PlateACSoftcap);
		}
		softcap += shield_ac;
		armor += shield_ac;

		if (RuleB(Combat, OldACSoftcapRules))
			softcap += (softcap * (aa_mit * RuleR(Combat, AAMitigationACFactor)));
		if (armor > softcap) {
			int softcap_armor = armor - softcap;
			if (RuleB(Combat, OldACSoftcapRules)) {
				if (GetClass() == WARRIOR)
					softcap_armor = softcap_armor * RuleR(Combat, WarriorACSoftcapReturn);
				else if (GetClass() == SHADOWKNIGHT || GetClass() == PALADIN ||
						(GetClass() == MONK && weight <= monkweight))
					softcap_armor = softcap_armor * RuleR(Combat, KnightACSoftcapReturn);
				else if (GetClass() == CLERIC || GetClass() == BARD ||
						GetClass() == BERSERKER || GetClass() == ROGUE ||
						GetClass() == SHAMAN || GetClass() == MONK)
					softcap_armor = softcap_armor * RuleR(Combat, LowPlateChainACSoftcapReturn);
				else if (GetClass() == RANGER || GetClass() == BEASTLORD)
					softcap_armor = softcap_armor * RuleR(Combat, LowChainLeatherACSoftcapReturn);
				else if (GetClass() == WIZARD || GetClass() == MAGICIAN ||
						GetClass() == NECROMANCER || GetClass() == ENCHANTER ||
						GetClass() == DRUID)
					softcap_armor = softcap_armor * RuleR(Combat, CasterACSoftcapReturn);
				else
					softcap_armor = softcap_armor * RuleR(Combat, MiscACSoftcapReturn);
			} else {
				if (GetClass() == WARRIOR)
					softcap_armor *= RuleR(Combat, WarACSoftcapReturn);
				else if (GetClass() == PALADIN || GetClass() == SHADOWKNIGHT)
					softcap_armor *= RuleR(Combat, PalShdACSoftcapReturn);
				else if (GetClass() == CLERIC || GetClass() == RANGER ||
						GetClass() == MONK || GetClass() == BARD)
					softcap_armor *= RuleR(Combat, ClrRngMnkBrdACSoftcapReturn);
				else if (GetClass() == DRUID || GetClass() == NECROMANCER ||
						GetClass() == WIZARD || GetClass() == ENCHANTER ||
						GetClass() == MAGICIAN)
					softcap_armor *= RuleR(Combat, DruNecWizEncMagACSoftcapReturn);
				else if (GetClass() == ROGUE || GetClass() == SHAMAN ||
						GetClass() == BEASTLORD || GetClass() == BERSERKER)
					softcap_armor *= RuleR(Combat, RogShmBstBerACSoftcapReturn);
				else
					softcap_armor *= RuleR(Combat, MiscACSoftcapReturn);
			}
			
			
			armor = softcap + softcap_armor;
			if (Msg >= 2)
				GM->Message(Chat::White, "# %i #### DEFENDER Final Armor [Soft Cap %i Soft Cap Armor %i]",armor, softcap,softcap_armor);
		}
		int tmp_armor = armor;
		if (GetClass() == WIZARD || GetClass() == MAGICIAN ||
				GetClass() == NECROMANCER || GetClass() == ENCHANTER){
			mitigation_rating = ((GetSkill(EQ::skills::SkillDefense) + itembonuses.HeroicAGI / 10) / 4.0) + armor + 1;
			if (Msg >= 2)
				GM->Message(Chat::White, "# + %.2f #### DEFENDER Armor Bonus [Defense Skill %i Heroic Agi %i]", mitigation_rating - tmp_armor, GetSkill(EQ::skills::SkillDefense), itembonuses.HeroicAGI);
		}
		else{
			mitigation_rating = ((GetSkill(EQ::skills::SkillDefense) + itembonuses.HeroicAGI / 10) / 3.0) + (armor * 1.333333) + 1;
			if (Msg >= 2)
				GM->Message(Chat::White, "# + %.2f #### DEFENDER Armor Bonus [Defense Skill %i Heroic Agi %i]", mitigation_rating - tmp_armor, GetSkill(EQ::skills::SkillDefense), itembonuses.HeroicAGI);

		}
		mitigation_rating *= 0.847;

		if (Msg >= 1)
			GM->Message(Chat::White, "# %.2f #### DEFENDER Final Mitigation Rating", mitigation_rating);

		
		if (Msg >= 2){
			GM->Message(Chat::White, " ");
			GM->Message(Chat::White, "### Mitigation Bonus Effects ###");
			if (itembonuses.MeleeMitigation)
				GM->Message(Chat::White, "# %i #### DEFENDER Item Mod2 Shielding", itembonuses.MeleeMitigation);
			if (aabonuses.MeleeMitigationEffect)
				GM->Message(Chat::White, "# %i #### DEFENDER SE_MeleeMitigation(168) AA Bonus", aabonuses.MeleeMitigationEffect);
			if (spellbonuses.MeleeMitigationEffect)
				GM->Message(Chat::White, "# %i #### DEFENDER SE_MeleeMitigation(168) Spell Bonus", spellbonuses.MeleeMitigationEffect);
			if (itembonuses.MeleeMitigationEffect)
				GM->Message(Chat::White, "# %i #### DEFENDER SE_MeleeMitigation(168) Worn Bonus", itembonuses.MeleeMitigationEffect);
		}

		mitigation_rating = mod_mitigation_rating(mitigation_rating, attacker);

		if (attacker->IsClient()){
			if (atk_override)
				attack_rating = (atk_override + ((attacker->GetSTR() - 66) * 0.9) + (attacker->GetSkill(EQ::skills::SkillOffense)*1.345));
			else
				attack_rating = ((attacker->CastToClient()->CalcATK() + add_atk) + ((attacker->GetSTR() - 66) * 0.9) + (attacker->GetSkill(EQ::skills::SkillOffense)*1.345));

		}
		else{
			if (atk_override)
				attack_rating = (atk_override + (attacker->GetSkill(EQ::skills::SkillOffense)*1.345) + ((attacker->GetSTR() - 66) * 0.9));
			else
				attack_rating = ((attacker->GetATK() + add_atk) + (attacker->GetSkill(EQ::skills::SkillOffense)*1.345) + ((attacker->GetSTR() - 66) * 0.9));
		}

		attack_rating = attacker->mod_attack_rating(attack_rating, this);

		if (Msg >= 2){
			GM->Message(Chat::White, " ");
			GM->Message(Chat::White, "### Calculate Attack Rating ###");
			if (attacker->IsClient()){
				GM->Message(Chat::White, "# %i #### ATTACKER Worn/Equip ATK Bonus", attacker->itembonuses.ATK);
				GM->Message(Chat::White, "# %i #### ATTACKER SE_ATK(2) AA Bonus", attacker->aabonuses.ATK);
				GM->Message(Chat::White, "# %i #### ATTACKER SE_ATK(2) spell Bonus", attacker->spellbonuses.ATK);
				GM->Message(Chat::White, "# %i #### ATTACKER Leadership Bonus", attacker->CastToClient()->GroupLeadershipAAOffenseEnhancement());
				GM->Message(Chat::White, "# %i #### ATTACKER Worn/Equip ATK Bonus", attacker->itembonuses.ATK);
				GM->Message(Chat::White, "# %i #### ATTACKER Worn/Equip ATK Bonus", attacker->itembonuses.ATK);
				GM->Message(Chat::White, "# %.2f #### ATTACKER Strength Stat ATK Bonus [Stat Amt: %i]", ((attacker->GetSTR()-66) * 0.9),attacker->GetSTR());
				GM->Message(Chat::White, "# %.2f #### ATTACKER Offensive Skill ATK Bonus [Stat Amt: %i]", (attacker->GetSkill(EQ::skills::SkillOffense)*1.345), attacker->GetSkill(EQ::skills::SkillOffense));
			}
				
			else{
				GM->Message(Chat::White, "# %i #### ATTACKER Worn/Equip ATK Bonus", attacker->itembonuses.ATK);
				GM->Message(Chat::White, "# %i #### ATTACKER SE_ATK(2) spell Bonus", attacker->spellbonuses.ATK);
				GM->Message(Chat::White, "# %i #### ATTACKER NPC ATK Stat", attacker->CastToNPC()->ATK);
				GM->Message(Chat::White, "# %.2f #### ATTACKER Strength Stat ATK Bonus [Stat Amt: %i]", ((attacker->GetSTR()-66) * 0.9),attacker->GetSTR());
				GM->Message(Chat::White, "# %.2f #### ATTACKER Offensive Skill ATK Bonus [Stat Amt: %i]", (attacker->GetSkill(EQ::skills::SkillOffense)*1.345), attacker->GetSkill(EQ::skills::SkillOffense));
			}
		}

		if (Msg >= 1){
			GM->Message(Chat::White, "# %.2f #### ATTACKER Final Attack Rating", attack_rating);
			GM->Message(Chat::White, "######### Melee Mitigation Report: Complete #########", Msg);
		}


		//damage = GetMeleeMitDmg(attacker, damage, minhit, mitigation_rating, attack_rating);
	} 

	if (damage < 0)
		damage = 0;

	return damage;
}

// This is called when the Mob is the one being hit
int32 Mob::Tune_GetMeleeMitDmg(Mob* GM, Mob *attacker, int32 damage, int32 minhit,
		float mit_rating, float atk_rating)
{
	float d = 10.0;
	float mit_roll = zone->random.Real(0, mit_rating);
	float atk_roll = zone->random.Real(0, atk_rating);

	if (atk_roll > mit_roll) {
		float a_diff = atk_roll - mit_roll;
		float thac0 = atk_rating * RuleR(Combat, ACthac0Factor);
		float thac0cap = attacker->GetLevel() * 9 + 20;
		if (thac0 > thac0cap)
			thac0 = thac0cap;

		d -= 10.0 * (a_diff / thac0);
	} else if (mit_roll > atk_roll) {
		float m_diff = mit_roll - atk_roll;
		float thac20 = mit_rating * RuleR(Combat, ACthac20Factor);
		float thac20cap = GetLevel() * 9 + 20;
		if (thac20 > thac20cap)
			thac20 = thac20cap;

		d += 10.0 * (m_diff / thac20);
	}

	if (d < 0.0)
		d = 0.0;
	else if (d > 20.0)
		d = 20.0;

	float interval = (damage - minhit) / 20.0;
	damage -= ((int)d * interval);

	damage -= (minhit * itembonuses.MeleeMitigation / 100);
	damage -= (damage *  (spellbonuses.MeleeMitigationEffect + itembonuses.MeleeMitigationEffect + aabonuses.MeleeMitigationEffect) / 100);
	return damage;
}

// This is called when the Client is the one being hit
int32 Client::Tune_GetMeleeMitDmg(Mob* GM, Mob *attacker, int32 damage, int32 minhit,
		float mit_rating, float atk_rating)
{
	if (!attacker->IsNPC() || RuleB(Combat, UseOldDamageIntervalRules))
		return 0; //Mob::GetMeleeMitDmg(attacker, damage, minhit, mit_rating, atk_rating);
	int d = 10;
	// floats for the rounding issues
	float dmg_interval = (damage - minhit) / 19.0;
	float dmg_bonus = minhit - dmg_interval;
	float spellMeleeMit =  (spellbonuses.MeleeMitigationEffect + itembonuses.MeleeMitigationEffect + aabonuses.MeleeMitigationEffect) / 100.0;
	if (GetClass() == WARRIOR)
		spellMeleeMit += 0.05;
	dmg_bonus -= dmg_bonus * (itembonuses.MeleeMitigation / 100.0);
	dmg_interval -= dmg_interval * spellMeleeMit;

	float mit_roll = zone->random.Real(0, mit_rating);
	float atk_roll = zone->random.Real(0, atk_rating);

	if (atk_roll > mit_roll) {
		float a_diff = atk_roll - mit_roll;
		float thac0 = atk_rating * RuleR(Combat, ACthac0Factor);
		float thac0cap = attacker->GetLevel() * 9 + 20;
		if (thac0 > thac0cap)
			thac0 = thac0cap;

		d += 10 * (a_diff / thac0);
	} else if (mit_roll > atk_roll) {
		float m_diff = mit_roll - atk_roll;
		float thac20 = mit_rating * RuleR(Combat, ACthac20Factor);
		float thac20cap = GetLevel() * 9 + 20;
		if (thac20 > thac20cap)
			thac20 = thac20cap;

		d -= 10 * (m_diff / thac20);
	}

	if (d < 1)
		d = 1;
	else if (d > 20)
		d = 20;

	return static_cast<int32>((dmg_bonus + dmg_interval * d));
}


int32 Client::GetMeleeDamage(Mob* other, bool GetMinDamage)
{
	int Hand = EQ::invslot::slotPrimary;

	if (!other) 
		return 0;
	
	EQ::ItemInstance* weapon;
	weapon = GetInv().GetItem(EQ::invslot::slotPrimary);
	
	if(weapon != nullptr) {
		if (!weapon->IsWeapon()) {
			return(0);
		}
	} 

	EQ::skills::SkillType skillinuse = AttackAnimation(Hand, weapon);

	int damage = 0;
	uint8 mylevel = GetLevel() ? GetLevel() : 1;
	uint32 hate = 0;
	if (weapon) hate = weapon->GetItem()->Damage + weapon->GetItem()->ElemDmgAmt;
	int weapon_damage = GetWeaponDamage(other, weapon, &hate);
	if (hate == 0 && weapon_damage > 1) hate = weapon_damage;

	if(weapon_damage > 0){
		if(IsBerserk() && GetClass() == BERSERKER){
			int bonus = 3 + GetLevel()/10;		//unverified
			weapon_damage = weapon_damage * (100+bonus) / 100;
		}

		int min_hit = 1;
		int max_hit = 2;//(2*weapon_damage*GetDamageTable(skillinuse)) / 100;

		if(GetLevel() < 10 && max_hit > RuleI(Combat, HitCapPre10))
			max_hit = (RuleI(Combat, HitCapPre10));
		else if(GetLevel() < 20 && max_hit > RuleI(Combat, HitCapPre20))
			max_hit = (RuleI(Combat, HitCapPre20));

		CheckIncreaseSkill(skillinuse, other, -15);
		CheckIncreaseSkill(EQ::skills::SkillOffense, other, -15);


#ifndef EQEMU_NO_WEAPON_DAMAGE_BONUS

		int ucDamageBonus = 0;

		if (Hand == EQ::invslot::slotPrimary && GetLevel() >= 28 && IsWarriorClass())
		{
			ucDamageBonus = GetWeaponDamageBonus(weapon ? weapon->GetItem() : (const EQ::ItemData*) nullptr);

			min_hit += (int) ucDamageBonus;
			max_hit += (int) ucDamageBonus;
			hate += ucDamageBonus;
		}
#endif
		min_hit += min_hit * GetMeleeMinDamageMod_SE(skillinuse) / 100;

		if(max_hit < min_hit)
			max_hit = min_hit;

		if (GetMinDamage)
			return min_hit;
		else
			return max_hit;
	}

	return 0;
}

void Mob::Tune_FindAccuaryByHitChance(Mob* defender, Mob *attacker, float hit_chance, int interval, int max_loop, int avoid_override, int Msg)
{

	int add_acc = 0;
	float tmp_hit_chance = 0.0f;
	bool end = false;

	EQ::skills::SkillType skillinuse = EQ::skills::SkillHandtoHand;
	if (attacker->IsClient())
	{//Will check first equiped weapon for skill. Ie. remove wepaons to assess bow.
		EQ::ItemInstance* weapon;
		weapon = attacker->CastToClient()->GetInv().GetItem(EQ::invslot::slotPrimary);
			
		if(weapon && weapon->IsWeapon()){
			skillinuse = attacker->CastToClient()->AttackAnimation(EQ::invslot::slotPrimary, weapon);
		}
		else {
			weapon = attacker->CastToClient()->GetInv().GetItem(EQ::invslot::slotSecondary);
			if (weapon && weapon->IsWeapon()) 
				skillinuse = attacker->CastToClient()->AttackAnimation(EQ::invslot::slotSecondary, weapon);
			else {
				weapon = attacker->CastToClient()->GetInv().GetItem(EQ::invslot::slotRange);
				if (weapon && weapon->IsWeapon()) 
					skillinuse = attacker->CastToClient()->AttackAnimation(EQ::invslot::slotRange, weapon);
			}
		}
	}

	tmp_hit_chance = Tune_CheckHitChance(defender, attacker, skillinuse, EQ::invslot::slotPrimary, 0, 0, 0, avoid_override);

	
	Message(0, "#Tune - Begin Parse [Interval %i Max Loop Iterations %i]", interval, max_loop);
	Message(0, "#Tune - Processing... Find Accuracy for hit chance on attacker of (%.0f) pct on defender [Current Hit Chance %.2f]", hit_chance, tmp_hit_chance);


	if (tmp_hit_chance > hit_chance)
		interval = interval * -1;

	for (int j=0; j < max_loop; j++)        
	{
		tmp_hit_chance = Tune_CheckHitChance(defender, attacker, skillinuse, EQ::invslot::slotPrimary, 0, false, 0, avoid_override, add_acc);

		if (Msg >= 3)
			Message(Chat::Yellow, "#Tune - Processing... [%i] [ACCURACY %i] Hit Chance %.2f ",j,add_acc,tmp_hit_chance);

		if (interval > 0 && tmp_hit_chance >= hit_chance){
			end = true;
		}

		else if (interval < 0 && tmp_hit_chance <= hit_chance){
			end = true;
		}

		if (end){
			
			Tune_CheckHitChance(defender, attacker, skillinuse, EQ::invslot::slotPrimary, 0, Msg, 0, avoid_override);//Display Stat Report
			
			Message(0, " ");

			if (attacker->IsNPC()){
				Message(0, "#Recommended NPC Accuracy Statistic adjustment of ( %i ) on ' %s ' for a hit chance of (+ %.0f) pct verse ' %s '. ",add_acc,defender->GetCleanName(), hit_chance, attacker->GetCleanName());
				Message(0, "#SET: [NPC Avoidance] = [%i]",add_acc + defender->CastToNPC()->GetAccuracyRating());
			}
			else if (attacker->IsClient()){
				Message(0, "#Recommended Client Accuracy Bonus adjustment of ( %i ) on ' %s ' for a hit chance of (+ %.0f) pct verse ' %s '. ",add_acc,defender->GetCleanName(), hit_chance, attacker->GetCleanName());
				Message(0, "#Modify (+/-): [Item Mod2 Accuracy]     [%i]",add_acc);
				Message(0, "#Modify (+/-): [SE_Accuracy(216)]       [%i]",add_acc);
				Message(0, "#Modify (+/-): [SE_HitChance(184)]      [%i]",add_acc / 15);
			}

			return;
		}


		add_acc = add_acc + interval;
	}

	Message(Chat::LightGray, "#Tune - Error: Unable to find desired result for (%.0f) pct - Increase interval (%i) AND/OR max loop value (%i) and run again.", hit_chance, interval, max_loop);
	Message(Chat::LightGray, "#Tune - Parse ended at ACCURACY ADJUSTMENTT ( %i ) at hit chance of (%.0f) / (%.0f) pct.",add_acc,tmp_hit_chance / hit_chance);
}

void Mob::Tune_FindAvoidanceByHitChance(Mob* defender, Mob *attacker, float hit_chance, int interval, int max_loop, int acc_override, int Msg)
{
	int add_avoid = 0;
	float tmp_hit_chance = 0.0f;
	bool end = false;

	EQ::skills::SkillType skillinuse = EQ::skills::SkillHandtoHand;
	if (attacker->IsClient())
	{//Will check first equiped weapon for skill. Ie. remove wepaons to assess bow.
		EQ::ItemInstance* weapon;
		weapon = attacker->CastToClient()->GetInv().GetItem(EQ::invslot::slotPrimary);
			
		if(weapon && weapon->IsWeapon()){
			skillinuse = attacker->CastToClient()->AttackAnimation(EQ::invslot::slotPrimary, weapon);
		}
		else {
			weapon = attacker->CastToClient()->GetInv().GetItem(EQ::invslot::slotSecondary);
			if (weapon && weapon->IsWeapon()) 
				skillinuse = attacker->CastToClient()->AttackAnimation(EQ::invslot::slotSecondary, weapon);
			else {
				weapon = attacker->CastToClient()->GetInv().GetItem(EQ::invslot::slotRange);
				if (weapon && weapon->IsWeapon()) 
					skillinuse = attacker->CastToClient()->AttackAnimation(EQ::invslot::slotRange, weapon);
			}
		}
	}

	tmp_hit_chance = Tune_CheckHitChance(defender, attacker, skillinuse, EQ::invslot::slotPrimary, 0, 0, acc_override, 0);

	Message(0, "#Tune - Begin Parse [Interval %i Max Loop Iterations %i]", interval, max_loop);
	Message(0, "#Tune - Processing... Find Avoidance for hit chance on defender of (%.0f) pct from attacker. [Current Hit Chance %.2f]", hit_chance, tmp_hit_chance);

	if (tmp_hit_chance < hit_chance)
		interval = interval * -1;

	for (int j=0; j < max_loop; j++)        
	{
		tmp_hit_chance = Tune_CheckHitChance(defender, attacker, skillinuse, EQ::invslot::slotPrimary, 0, 0, acc_override, 0, 0, add_avoid);

		if (Msg >= 3)
			Message(0, "#Tune - Processing... [%i] [AVOIDANCE %i] Hit Chance %.2f ",j,add_avoid,tmp_hit_chance);

		if (interval > 0 && tmp_hit_chance <= hit_chance){
			end = true;
		}

		else if (interval < 0 && tmp_hit_chance >= hit_chance){
			end = true;
		}

		if (end){
			
			Tune_CheckHitChance(defender, attacker, skillinuse, EQ::invslot::slotPrimary, 0, Msg, acc_override, 0);//Display Stat Report
			
			Message(0, " ");

			if (defender->IsNPC()){
				Message(0, "#Recommended NPC Avoidance Statistic adjustment of ( %i ) on ' %s ' for a hit chance of ( %.0f) pct from ' %s '. ",add_avoid,defender->GetCleanName(), hit_chance, attacker->GetCleanName());
				Message(0, "#SET: [NPC Avoidance] = [%i]",add_avoid + defender->CastToNPC()->GetAvoidanceRating());
			}
			else if (defender->IsClient()){
				Message(0, "#Recommended Client Avoidance Bonus adjustment of ( %i ) on ' %s ' for a hit chance of ( %.0f) pct from ' %s '. ",add_avoid,defender->GetCleanName(), hit_chance, attacker->GetCleanName());
				Message(0, "#Modify (+/-): [Item Mod2 Avoidance] [%i]",add_avoid);
				Message(0, "#Modify (+/-): [SE_AvoidMeleeChance(172)]  [%i]",add_avoid / 10);
			}

			return;
		}

		add_avoid = add_avoid + interval;
	}

	Message(0, "#Tune - Error: Unable to find desired result for (%.0f) pct - Increase interval (%i) AND/OR max loop value (%i) and run again.", hit_chance, interval, max_loop);
	Message(0, "#Tune - Parse ended at AVOIDANCE ADJUSTMENT ( %i ) at hit chance of (%.0f) / (%.0f) pct.",add_avoid,tmp_hit_chance / hit_chance);
}


float Mob::Tune_CheckHitChance(Mob* defender, Mob* attacker, EQ::skills::SkillType skillinuse, int Hand, int16 chance_mod, int Msg, int acc_override, int avoid_override, int add_acc, int add_avoid)
{

	float chancetohit = RuleR(Combat, BaseHitChance);

	if(attacker->IsNPC() && !attacker->IsPet())
		chancetohit += RuleR(Combat, NPCBonusHitChance);

	if (Msg){
		
		Message(0, "######### Hit Chance Report: Start [Detail Level %i]#########", Msg);
		Message(0, "#ATTACKER: %s", attacker->GetCleanName());
		Message(0, "#DEFENDER: %s", defender->GetCleanName());
		if (Msg >= 2){ 
			Message(0, " "); 
			Message(0, "### Calculate Base Hit Chance ###"); 
			Message(0, "# + %.2f Total: %.2f #### RuleR(Combat, BaseHitChance)", RuleR(Combat, BaseHitChance), RuleR(Combat, BaseHitChance));
			if (attacker->IsNPC())
				Message(0, "# + %.2f Total: %.2f #### RuleR(Combat, NPCBonusHitChance)", RuleR(Combat, NPCBonusHitChance), chancetohit);
		}
	}

	float temp_chancetohit = chancetohit;

	bool pvpmode = false;
	if(IsClient() && attacker->IsClient())
		pvpmode = true;

	if (chance_mod >= 10000)
		return true;

	float avoidanceBonus = 0;
	float hitBonus = 0;

	////////////////////////////////////////////////////////
	// To hit calcs go here
	////////////////////////////////////////////////////////

	uint8 attacker_level = attacker->GetLevel() ? attacker->GetLevel() : 1;
	uint8 defender_level = defender->GetLevel() ? defender->GetLevel() : 1;

	//Calculate the level difference

	double level_difference = attacker_level - defender_level;
	double range = defender->GetLevel();
	range = ((range / 4) + 3);

	if(level_difference < 0)
	{
		if(level_difference >= -range)
		{
			chancetohit += (level_difference / range) * RuleR(Combat,HitFalloffMinor); //5
		}
		else if (level_difference >= -(range+3.0))
		{
			chancetohit -= RuleR(Combat,HitFalloffMinor);
			chancetohit += ((level_difference+range) / (3.0)) * RuleR(Combat,HitFalloffModerate); //7
		}
		else
		{
			chancetohit -= (RuleR(Combat,HitFalloffMinor) + RuleR(Combat,HitFalloffModerate));
			chancetohit += ((level_difference+range+3.0)/12.0) * RuleR(Combat,HitFalloffMajor); //50
		}
	}
	else
	{
		chancetohit += (RuleR(Combat,HitBonusPerLevel) * level_difference);
	}

	if (Msg >= 2)
		Message(0, "# + %.2f Total: %.2f #### Level Modifers",  chancetohit - temp_chancetohit, chancetohit);
	
	temp_chancetohit = chancetohit;

	chancetohit -= ((float)defender->GetAGI() * RuleR(Combat, AgiHitFactor));

	if (Msg >= 2)
		Message(0, "# - %.2f Total: %.2f #### DEFENDER Agility",  ((float)defender->GetAGI() * RuleR(Combat, AgiHitFactor)), chancetohit);
	
	if(attacker->IsClient())
	{
		chancetohit -= (RuleR(Combat,WeaponSkillFalloff) * (attacker->CastToClient()->MaxSkill(skillinuse) - attacker->GetSkill(skillinuse)));
		if (Msg >= 2)
			Message(0, "# - %.2f Total: %.2f ##### ATTACKER Wpn Skill Mod: ",  (RuleR(Combat,WeaponSkillFalloff) * (attacker->CastToClient()->MaxSkill(skillinuse) - attacker->GetSkill(skillinuse))), chancetohit);
	}
		
	if(defender->IsClient())
	{
		chancetohit += (RuleR(Combat, WeaponSkillFalloff) * (defender->CastToClient()->MaxSkill(EQ::skills::SkillDefense) - defender->GetSkill(EQ::skills::SkillDefense)));
		if (Msg >= 2)
			Message(0, "# + %.2f Total: %.2f #### DEFENDER Defense Skill Mod", (RuleR(Combat, WeaponSkillFalloff) * (defender->CastToClient()->MaxSkill(EQ::skills::SkillDefense) - defender->GetSkill(EQ::skills::SkillDefense))), chancetohit);
	}
	

	//I dont think this is 100% correct, but at least it does something...
	if(attacker->spellbonuses.MeleeSkillCheckSkill == skillinuse || attacker->spellbonuses.MeleeSkillCheckSkill == 255) {
		chancetohit += attacker->spellbonuses.MeleeSkillCheck;
		if (Msg >= 2)
			Message(0, "# + %.2f Total: %.2f #### ATTACKER SE_MeleeSkillCheck(183) Spell Bonus", attacker->spellbonuses.MeleeSkillCheck , chancetohit);
	}
	if(attacker->itembonuses.MeleeSkillCheckSkill == skillinuse || attacker->itembonuses.MeleeSkillCheckSkill == 255) {
		chancetohit += attacker->itembonuses.MeleeSkillCheck;
		if (Msg >= 2)
			Message(0, "# + %.2f Total: %.2f #### ATTACKER SE_MeleeSkillCheck(183) Worn Bonus", attacker->itembonuses.MeleeSkillCheck , chancetohit);
	}

	if (Msg)
		Message(0, "#FINAL Base Hit Chance: %.2f percent", chancetohit);

	if (Msg >= 2){
		Message(0, " ");
		Message(0, "######### Calculate Avoidance Bonuses  #########");
	}

	//Avoidance Bonuses on defender decreases baseline hit chance by percent.
	avoidanceBonus = defender->spellbonuses.AvoidMeleeChanceEffect +
				defender->itembonuses.AvoidMeleeChanceEffect +
				defender->aabonuses.AvoidMeleeChanceEffect +
				(defender->itembonuses.AvoidMeleeChance / 10.0f); //Item Mod 'Avoidence'

	if (Msg >= 2){
		if (defender->aabonuses.AvoidMeleeChanceEffect)
			Message(0, "# %i #### DEFENDER SE_AvoidMeleeChance(172) AA Bonus", defender->aabonuses.AvoidMeleeChanceEffect);
		if (defender->spellbonuses.AvoidMeleeChanceEffect)
			Message(0, "# %i #### DEFENDER SE_AvoidMeleeChance(172) Spell Bonus", defender->spellbonuses.AvoidMeleeChanceEffect);
		if (defender->itembonuses.AvoidMeleeChanceEffect)
			Message(0, "# %i #### DEFENDER SE_AvoidMeleeChance(172) Worn Bonus", defender->itembonuses.AvoidMeleeChanceEffect);
		if (defender->itembonuses.AvoidMeleeChance)
			Message(0, "# %i #### DEFENDER Avoidance Item Mod2 Bonus[Amt: %i] ", defender->itembonuses.AvoidMeleeChance / 10.0f,defender->itembonuses.AvoidMeleeChance);
	}


	Mob *owner = nullptr;
	if (defender->IsPet())
		owner = defender->GetOwner();
	else if ((defender->IsNPC() && defender->CastToNPC()->GetSwarmOwner()))
		owner = entity_list.GetMobID(defender->CastToNPC()->GetSwarmOwner());

	if (owner){
		avoidanceBonus += owner->aabonuses.PetAvoidance + owner->spellbonuses.PetAvoidance + owner->itembonuses.PetAvoidance;
	
		if (Msg >= 2){
			if (owner->aabonuses.PetAvoidance)
				Message(0, "# %i #### DEFENDER SE_PetAvoidance(215) AA Bonus", owner->aabonuses.PetAvoidance);
			if (owner->aabonuses.PetAvoidance)
				Message(0, "# %i #### DEFENDER SE_PetAvoidance(215) Spell Bonus", owner->itembonuses.PetAvoidance);
			if (owner->aabonuses.PetAvoidance)
				Message(0, "# %i #### DEFENDER SE_PetAvoidance(215) Worn Bonus", owner->spellbonuses.PetAvoidance);
		}
	}

	if(defender->IsNPC()){
		avoidanceBonus += ((defender->CastToNPC()->GetAvoidanceRating() + add_avoid) / 10.0f); //Modifier from database
		if (Msg >= 2)
			Message(0, "# + %.2f #### DEFENDER NPC AVOIDANCE STAT [Stat Amt: %i] ", ((defender->CastToNPC()->GetAvoidanceRating() + add_avoid) / 10.0f),defender->CastToNPC()->GetAvoidanceRating());
	}
	else if(defender->IsClient()){
		avoidanceBonus += (add_avoid / 10.0f); //Avoidance Item Mod
	}

	//#tune override value
	if (avoid_override){
		avoidanceBonus = (avoid_override / 10.0f);
		if (Msg >= 2)
			Message(0, "%.2f #### DEFENDER 'AVOIDANCE OVERRIDE'", avoidanceBonus);
	}

	if (Msg)
		Message(0, "#FINAL Avoidance Bonus': %.2f percent ", avoidanceBonus);

	if (Msg >= 2){
		Message(0, " ");
		Message(0, "######### Calculate Accuracy Bonuses #########");
	}

	//Hit Chance Bonuses on attacker increases baseline hit chance by percent.
	hitBonus +=	attacker->itembonuses.HitChanceEffect[skillinuse] +
				attacker->spellbonuses.HitChanceEffect[skillinuse]+
				attacker->aabonuses.HitChanceEffect[skillinuse]+
				attacker->itembonuses.HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1] +
				attacker->spellbonuses.HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1] +
				attacker->aabonuses.HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1];

	if (Msg >= 2){
		if (attacker->aabonuses.HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1])
			Message(0, "# %i #### ATTACKER SE_HitChance(184) AA Bonus [All Skills]", attacker->aabonuses.HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1]);
		if (attacker->spellbonuses.HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1])
			Message(0, "# %i #### ATTACKER SE_HitChance(184) Spell Bonus [All Skills]", attacker->spellbonuses.HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1]);
		if (attacker->itembonuses.HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1])
			Message(0, "# %i #### ATTACKER SE_HitChance(184) Worn Bonus [All Skills]", attacker->itembonuses.HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1]);
		if (attacker->itembonuses.HitChanceEffect[skillinuse])
			Message(0, "# %i #### ATTACKER SE_HitChance(184) AA Bonus [Skill]", attacker->aabonuses.HitChanceEffect[skillinuse]);
		if (attacker->spellbonuses.HitChanceEffect[skillinuse])
			Message(0, "# %i #### ATTACKER SE_HitChance(184) Spell Bonus [Skill]", attacker->spellbonuses.HitChanceEffect[skillinuse]);
		if (attacker->itembonuses.HitChanceEffect[skillinuse])
			Message(0, "# %i #### ATTACKER SE_HitChance(184) Worn Bonus [Skill]", attacker->itembonuses.HitChanceEffect[skillinuse]);
	}

	//Accuracy = Spell Effect , HitChance = 'Accuracy' from Item Effect
	//Only AA derived accuracy can be skill limited. ie (Precision of the Pathfinder, Dead Aim)
	hitBonus += (attacker->itembonuses.Accuracy[EQ::skills::HIGHEST_SKILL + 1] +
				attacker->spellbonuses.Accuracy[EQ::skills::HIGHEST_SKILL + 1] +
				attacker->aabonuses.Accuracy[EQ::skills::HIGHEST_SKILL + 1] +
				attacker->aabonuses.Accuracy[skillinuse] +
				attacker->itembonuses.HitChance) / 15.0f; //Item Mod 'Accuracy'

	if (Msg >= 2) {
		if (attacker->aabonuses.Accuracy[EQ::skills::HIGHEST_SKILL + 1])
			Message(0, "# %.2f #### ATTACKER SE_Accuracy(216) AA Bonus [All Skills] [Stat Amt: %i]", static_cast<float>(attacker->aabonuses.Accuracy[EQ::skills::HIGHEST_SKILL + 1]) / 15.0f, attacker->aabonuses.Accuracy[EQ::skills::HIGHEST_SKILL + 1]);
		if (attacker->spellbonuses.Accuracy[EQ::skills::HIGHEST_SKILL + 1])
			Message(0, "# %.2f #### ATTACKER SE_Accuracy(216) Spell Bonus [All Skills] [Stat Amt: %i]", static_cast<float>(attacker->spellbonuses.Accuracy[EQ::skills::HIGHEST_SKILL + 1]) / 15.0f, attacker->spellbonuses.Accuracy[EQ::skills::HIGHEST_SKILL + 1]);
		if (attacker->itembonuses.Accuracy[EQ::skills::HIGHEST_SKILL + 1])
			Message(0, "# %.2f #### ATTACKER SE_Accuracy(216) Worn Bonus [All Skills] [Stat Amt: %i]", static_cast<float>(attacker->itembonuses.Accuracy[EQ::skills::HIGHEST_SKILL + 1]) / 15.0f, attacker->itembonuses.Accuracy[EQ::skills::HIGHEST_SKILL + 1]);
		if (attacker->aabonuses.Accuracy[skillinuse])
			Message(0, "# %.2f #### ATTACKER SE_Accuracy(216) AA Bonus [Skill] [Stat Amt: %i]", static_cast<float>(attacker->aabonuses.Accuracy[skillinuse])/15.0f,attacker->aabonuses.Accuracy[skillinuse]);
		if (attacker->itembonuses.HitChance)
			Message(0, "# %.2f #### ATTACKER Accuracy Item Mod2 Bonus [Stat Amt: %i]", static_cast<float>(attacker->itembonuses.HitChance)/15.0f,attacker->itembonuses.HitChance);
	}

	hitBonus += chance_mod; //Modifier applied from casted/disc skill attacks.

	if(attacker->IsNPC()){
		if (acc_override){
			hitBonus = (acc_override / 10.0f);
			if (Msg >= 2)
				Message(0, "# %.2f #### ATTACKER 'ACCURACY OVERRIDE'", hitBonus);
		}
		else {
			hitBonus += ((attacker->CastToNPC()->GetAccuracyRating() + add_acc) / 10.0f); //Modifier from database
			if (Msg >= 2){
				Message(0, "# %.2f #### ATTACKER NPC ACCURACY STAT [Stat Amt: %i] ", ((attacker->CastToNPC()->GetAccuracyRating() + add_avoid) / 10.0f),attacker->CastToNPC()->GetAccuracyRating());
			}
		}
	}
	else if(attacker->IsClient()){
		if (acc_override){
			hitBonus = (acc_override / 15.0f); 
			if (Msg >= 2)
				Message(0, "# %.2f #### ATTACKER 'ACCURACY OVERRIDE': %.2f ");
		}
		else
			hitBonus += (add_acc / 15.0f); //Modifier from database
	}

	if (skillinuse == EQ::skills::SkillArchery){
		hitBonus -= hitBonus*RuleR(Combat, ArcheryHitPenalty);
		if (Msg >= 2)
			Message(0, "# %.2f pct #### RuleR(Combat, ArcheryHitPenalty) ", RuleR(Combat, ArcheryHitPenalty));
	}

	//Calculate final chance to hit
	chancetohit += ((chancetohit * (hitBonus - avoidanceBonus)) / 100.0f);

	if (Msg){
		Message(0, "#FINAL Accuracy Bonus': %.2f percent", hitBonus);

		if (Msg >= 2)
			Message(0, " ");

		Message(0, "#FINAL Hit Chance: %.2f percent [Max: %.2f Min: %.2f] ", chancetohit, RuleR(Combat,MaxChancetoHit), RuleR(Combat,MinChancetoHit) );
		Message(0, "######### Hit Chance Report: Completed #########");
	}

	chancetohit = mod_hit_chance(chancetohit, skillinuse, attacker);

	// Chance to hit;   Max 95%, Min 5% DEFAULTS
	if(chancetohit > 1000 || chancetohit < -1000) {
		//if chance to hit is crazy high, that means a discipline is in use, and let it stay there
	}
	else if(chancetohit > RuleR(Combat,MaxChancetoHit)) {
		chancetohit = RuleR(Combat,MaxChancetoHit);
	}
	else if(chancetohit < RuleR(Combat,MinChancetoHit)) {
		chancetohit = RuleR(Combat,MinChancetoHit);
	}

	return(chancetohit);
}


void Mob::Tune_GetACByPctMitigation(Mob* defender, Mob *attacker, float pct_mitigation, int interval, int max_loop, int atk_override, int Msg)
{

	/*
		Find the amount of AC stat that has to be added/subtracted from DEFENDER to reach a specific average mitigation value based on ATTACKER's offense statistics.
		Can use atk_override to find the value verse a hypothetical amount of worn ATK
	*/

	int max_damage = 0;
	int min_damage = 0;
	int mean_dmg = 0;
	float tmp_pct_mitigated = 0.0f;
	int loop_add_ac = 0;
	int end = 0;

	int offense_rating = attacker->Tune_ClientAttack(defender, true, 0, 0, 0, 0, 0, true);

	if (attacker->IsNPC())
	{
		max_damage = static_cast<int32>(attacker->CastToNPC()->GetMaxDMG());
		min_damage = attacker->CastToNPC()->GetMinDMG();
	}
	else if (attacker->IsClient())
	{
		max_damage = attacker->Tune_ClientGetMaxDamage(defender);
		min_damage = attacker->Tune_ClientGetMinDamage(defender, max_damage);
		Shout("Min %i Max %i", min_damage, max_damage);
	}

	if (!max_damage)
	{
		Message(0, "#Tune - Processing... Abort! Damage not found! [MaxDMG %i MinDMG %i]", max_damage, min_damage);
		return;
	}

	//Obtain baseline mitigation for current stats
	mean_dmg = attacker->Tune_ClientGetMeanDamage(defender);
	tmp_pct_mitigated = 100.0f - (static_cast<float>(mean_dmg) * 100.0f / static_cast<float>(max_damage));

	Message(0, "<########################################################>");
	Message(0, "[#Tune] Begin Parse [Interval %i Max Loop Iterations %i]", interval, max_loop);
	Message(0, "[#Tune] Defender [%s] [AC Mitigation pct %i] [Total AC %i]", attacker->GetCleanName(), tmp_pct_mitigated, defender->Tune_ACSum());
	Message(0, "[#Tune] Attacker [%s] [Max Damage %i Min Damage %i] [Total Offense: %i]", attacker->GetCleanName(), max_damage, min_damage, offense_rating);
	Message(0, "[#Tune] Processing... Find AC for defender to have Mitigation of (%.0f) pct agianst this attacker.", pct_mitigation);
	

	//Set to decrease AC till you reach goal
	if (tmp_pct_mitigated > pct_mitigation) 
	{
		interval = interval * -1; //Interval is how much AC is modified each round. Larger the interval less accurate result is potentially.
	}

	for (int j = 0; j < max_loop; j++)
	{
		mean_dmg = attacker->Tune_ClientGetMeanDamage(defender, 0,0, loop_add_ac);
		tmp_pct_mitigated = 100.0f - (static_cast<float>(mean_dmg) * 100.0f / static_cast<float>(max_damage));

		Message(0, "#Tune - Processing... [%i] [AC %i] Average Melee Hit  %i | Pct Mitigated %.2f ", j, loop_add_ac, mean_dmg, tmp_pct_mitigated);

		if (Msg >= 3) 
		{
			Message(0, "#Tune - Processing... [%i] [AC %i] Average Melee Hit  %i | Pct Mitigated %.2f ", j, loop_add_ac, mean_dmg, tmp_pct_mitigated);
		}

		if (interval > 0 && tmp_pct_mitigated >= pct_mitigation) 
		{
			end = 1;
		}

		else if (interval < 0 && tmp_pct_mitigated <= pct_mitigation) 
		{
			end = 1;
		}

		else if (interval < 0 && mean_dmg == min_damage) 
		{
			end = 2;
		}

		if (end >= 1) {

			//defender->Tune_MeleeMitigation(this, attacker, damage, minhit, nullptr, Msg, 0, atk_override, loop_add_ac, 0);//?Prob for message

			if (end == 2) 
			{
				Message(0, "#Tune - [WARNING] Mitigation can not be further decreased due to minium hit value (%i).", min_damage);
			}

			if (defender->IsNPC()) 
			{
				Message(Chat::LightGray, "[#Tune] Recommended NPC AC ADJUSTMENT ( %i ) on ' %s ' to achieve an average mitigation of ( %.0f ) pct agianst attacker ' %s '.", loop_add_ac, defender->GetCleanName(), pct_mitigation, attacker->GetCleanName());
				Message(0, "[#Tune] SET NPC 'AC' stat value = [%i]", loop_add_ac + defender->CastToNPC()->GetRawAC());
				Message(0, ">########################################################<");
			}
			if (defender->IsClient()) 
			{
				Message(Chat::LightGray, "#Tune - Recommended CLIENT AC ADJUSTMENT ( %i ) on ' %s ' for an average mitigation of (+ %.0f) pct from attacker ' %s '.", loop_add_ac, defender->GetCleanName(), pct_mitigation, attacker->GetCleanName());
				Message(0, "#Modify (+/-): [Client AC STAT/SE_AC(1)] [%i]", loop_add_ac);
				Message(0, "########################################################");
			}

			return;
		}

		loop_add_ac = loop_add_ac + interval;
	}

	Message(0, "#Tune - Error: Unable to find desired result for (%.0f) pct - Increase interval (%i) AND/OR max loop value (%i) and run again.", pct_mitigation, interval, max_loop);
	Message(0, "#Tune - Parse ended at AC ADJUSTMENT ( %i ) at average mitigation of (%.0f) pct.", loop_add_ac, (tmp_pct_mitigated / pct_mitigation));
	return;
}

void Mob::Tune_GetATKByPctMitigation(Mob* defender, Mob *attacker, float pct_mitigation, int interval, int max_loop, int atk_override, int Msg)
{

	/*
		Find the amount of ATK stat that has to be added/subtracted from ATTACKER to reach a specific average mitigation value based on DEFENDERS's mitigation statistics.
		Can use ac_override to find the value verse a hypothetical amount of worn AC
	*/


	int max_damage = 0;
	int min_damage = 0;
	int mean_dmg = 0;
	float tmp_pct_mitigated = 0.0f;
	int loop_add_atk = 0;
	int end = 0;

	int offense_rating = attacker->Tune_ClientAttack(defender, true, 0, 0, 0, 0, 0, true);

	if (attacker->IsNPC())
	{
		max_damage = static_cast<int32>(attacker->CastToNPC()->GetMaxDMG());
		min_damage = attacker->CastToNPC()->GetMinDMG();
	}
	else if (attacker->IsClient())
	{
		max_damage = attacker->Tune_ClientGetMaxDamage(defender);
		min_damage = attacker->Tune_ClientGetMinDamage(defender, max_damage);
		Shout("Min %i Max %i", min_damage, max_damage);
	}

	if (!max_damage)
	{
		Message(0, "#Tune - Processing... Abort! Damage not found! [MaxDMG %i MinDMG %i]", max_damage, min_damage);
		return;
	}

	//Obtain baseline mitigation for current stats
	mean_dmg = attacker->Tune_ClientGetMeanDamage(defender);
	tmp_pct_mitigated = 100.0f - (static_cast<float>(mean_dmg) * 100.0f / static_cast<float>(max_damage));

	Message(0, "<########################################################>");
	Message(0, "[#Tune] Begin Parse [Interval %i Max Loop Iterations %i]", interval, max_loop);
	Message(0, "[#Tune] Defender [%s] [AC Mitigation pct %i] [Total AC %i]", attacker->GetCleanName(), tmp_pct_mitigated, defender->Tune_ACSum());
	Message(0, "[#Tune] Attacker [%s] [Max Damage %i Min Damage %i] [Total Offense: %i]", attacker->GetCleanName(), max_damage, min_damage, offense_rating);
	Message(0, "[#Tune] Processing... Find ATK on attacker for defender to have Mitigation of (%.0f) pct agianst this attacker.", pct_mitigation);


	//Set to decrease AC till you reach goal
	if (tmp_pct_mitigated > pct_mitigation)
	{
		interval = interval * -1; //Interval is how much AC is modified each round. Larger the interval less accurate result is potentially.
	}

	for (int j = 0; j < max_loop; j++)
	{
		mean_dmg = attacker->Tune_ClientGetMeanDamage(defender, 0, 0, loop_add_atk);
		tmp_pct_mitigated = 100.0f - (static_cast<float>(mean_dmg) * 100.0f / static_cast<float>(max_damage));

		Message(0, "#Tune - Processing... [%i] [ATK %i] Average Melee Hit  %i | Pct Mitigated %.2f ", j, loop_add_atk, mean_dmg, tmp_pct_mitigated);

		if (Msg >= 3)
		{
			Message(0, "#Tune - Processing... [%i] [ATK %i] Average Melee Hit  %i | Pct Mitigated %.2f ", j, loop_add_atk, mean_dmg, tmp_pct_mitigated);
		}

		if (interval > 0 && tmp_pct_mitigated >= pct_mitigation)
		{
			end = 1;
		}

		else if (interval < 0 && tmp_pct_mitigated <= pct_mitigation)
		{
			end = 1;
		}

		else if (interval < 0 && mean_dmg == min_damage)
		{
			end = 2;
		}

		if (end >= 1) {

			//defender->Tune_MeleeMitigation(this, attacker, damage, minhit, nullptr, Msg, 0, atk_override, loop_add_atk, 0);//?Prob for message

			if (end == 2)
			{
				Message(0, "#Tune - [WARNING] Mitigation can not be further decreased due to minium hit value (%i).", min_damage);
			}

			if (attacker->IsNPC()) {
				Message(0, "#Tune - Recommended NPC ATK ADJUSTMENT ( %i ) on ' %s ' average mitigation of (%.0f) pct verse ' %s '. ", loop_add_atk, attacker->GetCleanName(), pct_mitigation, defender->GetCleanName());
				Message(0, "#SET: [NPC Attack STAT] = [%i]", loop_add_atk + defender->CastToNPC()->ATK);
			}
			if (attacker->IsClient()) {
				Message(0, "#Tune - Recommended CLIENT ATK ADJUSTMENT ( %i ) on ' %s ' average mitigation of (%.0f) pct verse ' %s '. ", loop_add_atk, attacker->GetCleanName(), pct_mitigation, defender->GetCleanName());
				Message(0, "#Modify (+/-): [Client Attack STAT/SE_ATK(2)] [%i]", loop_add_atk);
			}

			return;
		}

		loop_add_atk = loop_add_atk + interval;
	}

	Message(0, "#Tune - Error: Unable to find desired result for (%.0f) pct - Increase interval (%i) AND/OR max loop value (%i) and run again.", pct_mitigation, interval, max_loop);
	Message(0, "#Tune - Parse ended at ATK ADJUSTMENT ( %i ) average target mitigation of (%.0f) pct.", loop_add_atk, tmp_pct_mitigated);
	return;
}

/*
	Tune support functions
*/


int Mob::Tune_ClientGetMeanDamage(Mob* other, int ac_override, int atk_override, int add_ac, int add_atk)
{
	uint32 total_damage = 0;
	int loop_max = 1000;

	for (int i = 0; i < loop_max; i++)
	{
		total_damage += Tune_ClientAttack(other, true, 10000, ac_override, atk_override, add_ac, add_atk);
	}

	return(total_damage / loop_max);
}

int Mob::Tune_ClientGetMaxDamage(Mob* other)
{
	uint32 max_hit = 0;
	uint32 current_hit = 0;
	int loop_max = 1000;

	for (int i = 0; i < loop_max; i++)
	{
		current_hit = Tune_ClientAttack(other, true, 10000, 1, 10000);
		if (current_hit > max_hit) {
			max_hit = current_hit;
		}
	}
	return(max_hit);
}

int Mob::Tune_ClientGetMinDamage(Mob* other, int max_hit)
{
	uint32 min_hit = max_hit;
	uint32 current_hit = 0;
	int loop_max = 1000;

	for (int i = 0; i < loop_max; i++)
	{
		current_hit = Tune_ClientAttack(other, true, 10000, 10000, 1);
		if (current_hit < min_hit) {
			min_hit = current_hit;
		}
	}
	return(min_hit);
}

float Mob::Tune_GetACMitigationPct(Mob* defender, Mob *attacker) {

	int max_damage = 0;
	int min_damage = 0;

	if (attacker->IsNPC())
	{
		max_damage = static_cast<int32>(attacker->CastToNPC()->GetMaxDMG());
		min_damage = attacker->CastToNPC()->GetMinDMG();
	}
	else if (attacker->IsClient())
	{
		max_damage = attacker->Tune_ClientGetMaxDamage(defender);
		min_damage = attacker->Tune_ClientGetMinDamage(defender, max_damage);
	}

	if (!max_damage)
	{
		Message(0, "[#Tune] Calculation Failure. Error: [Mob::Tune_GetACMitigationPct] No max damage found");
		return max_damage;
	}

	//Obtain baseline mitigation for current stats
	int mean_dmg = attacker->Tune_ClientGetMeanDamage(defender);
	float tmp_pct_mitigated = 100.0f - (static_cast<float>(mean_dmg) * 100.0f / static_cast<float>(max_damage));

	return tmp_pct_mitigated;
}

int Mob::Tune_GetAccuracy(Mob* defender, Mob *attacker)
{
	int accuracy = attacker->Tune_ClientAttack(defender, true, 0, 0, 0, 0, 0, false, true);
	return accuracy;
}

int Mob::Tune_GetAvoidance(Mob* defender, Mob *attacker)
{
	return defender->GetTotalDefense();
}

/*
	Calculate from modified attack.cpp functions.
*/

int Mob::Tune_ClientAttack(Mob* other, bool no_avoid, int hit_chance_bonus, int ac_override, int atk_override, int add_ac, int add_atk, bool get_offense, bool get_accuracy,
	int avoidance_override, int accuracy_override, int add_avoidance, int add_accuracy)
{
	if (!IsClient()) {
		Message(Chat::Red, "#Tune Failure:  A null NON CLIENT object was passed to Tune_ClientAttack() for evaluation!");
		return false;
	}

	if (!other) {
		Message(Chat::Red, "#Tune Failure:  A null Mob object was passed to Tune_ClientAttack() for evaluation!");
		return false;
	}

	int Hand = EQ::invslot::slotPrimary;

	EQ::ItemInstance* weapon = nullptr;
	weapon = CastToClient()->GetInv().GetItem(EQ::invslot::slotPrimary);
	OffHandAtk(false);

	if (weapon != nullptr) {
		if (!weapon->IsWeapon()) {
			Message(Chat::Red, "#Tune Failure: Attack cancelled, Item %s is not a weapon!", weapon->GetItem()->Name);
			return(false);
		}
	}

	DamageHitInfo my_hit;
	my_hit.skill = Tune_AttackAnimation(Hand, weapon);

	// Now figure out damage
	my_hit.damage_done = 1;
	my_hit.min_damage = 0;
	uint8 mylevel = GetLevel() ? GetLevel() : 1;
	uint32 hate = 0;
	if (weapon)
		hate = (weapon->GetItem()->Damage + weapon->GetItem()->ElemDmgAmt);

	my_hit.base_damage = GetWeaponDamage(other, weapon, &hate);
	if (hate == 0 && my_hit.base_damage > 1)
		hate = my_hit.base_damage;

	//if weapon damage > 0 then we know we can hit the target with this weapon
	//otherwise we cannot and we set the damage to -5 later on
	if (my_hit.base_damage > 0) {
		// if we revamp this function be more general, we will have to make sure this isn't
		// executed for anything BUT normal melee damage weapons from auto attack
		if (Hand == EQ::invslot::slotPrimary || Hand == EQ::invslot::slotSecondary)
			my_hit.base_damage = CastToClient()->DoDamageCaps(my_hit.base_damage);
		auto shield_inc = spellbonuses.ShieldEquipDmgMod + itembonuses.ShieldEquipDmgMod + aabonuses.ShieldEquipDmgMod;
		if (shield_inc > 0 && HasShieldEquiped() && Hand == EQ::invslot::slotPrimary) {
			my_hit.base_damage = my_hit.base_damage * (100 + shield_inc) / 100;
			hate = hate * (100 + shield_inc) / 100;
		}

		// ***************************************************************
		// *** Calculate the damage bonus, if applicable, for this hit ***
		// ***************************************************************

		int ucDamageBonus = 0;

		if (Hand == EQ::invslot::slotPrimary && GetLevel() >= 28 && IsWarriorClass())
		{
			// Damage bonuses apply only to hits from the main hand (Hand == MainPrimary) by characters level 28 and above
			// who belong to a melee class. If we're here, then all of these conditions apply.

			ucDamageBonus = GetWeaponDamageBonus(weapon ? weapon->GetItem() : (const EQ::ItemData*) nullptr);

			my_hit.min_damage = ucDamageBonus;
			hate += ucDamageBonus;
		}

		my_hit.offense = Tune_offense(my_hit.skill, atk_override, add_atk); // we need this a few times
		if (get_offense) {
			return my_hit.offense;
		}
		my_hit.hand = Hand;
		
		my_hit.tohit = GetTotalToHit(my_hit.skill, hit_chance_bonus);
		if (get_accuracy) {
			return my_hit.tohit;
		}
		Tune_DoAttack(other, my_hit, nullptr, no_avoid, ac_override, add_ac);
	}
	else {
		my_hit.damage_done = DMG_INVULNERABLE;
	}

	///////////////////////////////////////////////////////////
	////// Send Attack Damage
	///////////////////////////////////////////////////////////

	//other->Damage(this, my_hit.damage_done, SPELL_UNKNOWN, my_hit.skill, true, -1, false, m_specialattacks);

	return my_hit.damage_done;
}

void Mob::Tune_DoAttack(Mob *other, DamageHitInfo &hit, ExtraAttackOptions *opts, bool no_avoid, int ac_override, int add_ac,
	int avoidance_override, int accuracy_override, int add_avoidance, int add_accuracy)
{
	if (!other)
		return;
	LogCombat("[{}]::DoAttack vs [{}] base [{}] min [{}] offense [{}] tohit [{}] skill [{}]", GetName(),
		other->GetName(), hit.base_damage, hit.min_damage, hit.offense, hit.tohit, hit.skill);

	// check to see if we hit..
	if (no_avoid || (!no_avoid && other->AvoidDamage(this, hit))) {
		int strike_through = itembonuses.StrikeThrough + spellbonuses.StrikeThrough + aabonuses.StrikeThrough;
		if (strike_through && zone->random.Roll(strike_through)) {
			MessageString(Chat::StrikeThrough,
				STRIKETHROUGH_STRING); // You strike through your opponents defenses!
			hit.damage_done = 1;			// set to one, we will check this to continue
		}
		// I'm pretty sure you can riposte a riposte
		if (hit.damage_done == DMG_RIPOSTED) {
			//DoRiposte(other); //Disabled for TUNE
			//if (IsDead())
			return;
		}
		LogCombat("Avoided/strikethrough damage with code [{}]", hit.damage_done);
	}

	if (hit.damage_done >= 0) {
		if (no_avoid || (!no_avoid && other->CheckHitChance(this, hit))) {
			other->Tune_MeleeMitigation(this, hit, ac_override, add_ac);
			if (hit.damage_done > 0) {
				ApplyDamageTable(hit);
				CommonOutgoingHitSuccess(other, hit, opts);
			}
			LogCombat("Final damage after all reductions: [{}]", hit.damage_done);
		}
		else {
			LogCombat("Attack missed. Damage set to 0");
			hit.damage_done = 0;
		}
	}
}

void Mob::Tune_MeleeMitigation(Mob *attacker, DamageHitInfo &hit, int ac_override, int add_ac)
{
	if (hit.damage_done < 0 || hit.base_damage == 0)
		return;

	Mob* defender = this;
	//auto mitigation = defender->GetMitigationAC();
	auto mitigation = defender->Tune_ACSum(false, ac_override, add_ac);
	if (IsClient() && attacker->IsClient())
		mitigation = mitigation * 80 / 100; // 2004 PvP changes

	auto roll = RollD20(hit.offense, mitigation);

	// +0.5 for rounding, min to 1 dmg
	hit.damage_done = std::max(static_cast<int>(roll * static_cast<double>(hit.base_damage) + 0.5), 1);

	//Shout("mitigation %d vs offense %d. base %d rolled %f damage %d", mitigation, hit.offense, hit.base_damage, roll, hit.damage_done);
}

int Mob::Tune_ACSum(bool skip_caps, int ac_override, int add_ac)
{
	int ac = 0; // this should be base AC whenever shrouds come around
	ac += itembonuses.AC; // items + food + tribute

	if (IsClient() && ac_override) {
		ac = ac_override;
		ac += add_ac;
	}

	int shield_ac = 0;
	if (HasShieldEquiped() && IsClient()) {
		auto client = CastToClient();
		auto inst = client->GetInv().GetItem(EQ::invslot::slotSecondary);
		if (inst) {
			if (inst->GetItemRecommendedLevel(true) <= GetLevel())
				shield_ac = inst->GetItemArmorClass(true);
			else
				shield_ac = client->CalcRecommendedLevelBonus(GetLevel(), inst->GetItemRecommendedLevel(true), inst->GetItemArmorClass(true));
		}
		shield_ac += client->GetHeroicSTR() / 10;
	}
	// EQ math
	ac = (ac * 4) / 3;
	// anti-twink
	if (!skip_caps && IsClient() && GetLevel() < RuleI(Combat, LevelToStopACTwinkControl))
		ac = std::min(ac, 25 + 6 * GetLevel());
	ac = std::max(0, ac + GetClassRaceACBonus());
	if (IsNPC()) {
		// This is the developer tweaked number
		// for the VAST amount of NPCs in EQ this number didn't exceed 600 until recently (PoWar)
		// According to the guild hall Combat Dummies, a level 50 classic EQ mob it should be ~115
		// For a 60 PoP mob ~120, 70 OoW ~120
		

		if (ac_override) {
			ac += ac_override;
		}
		else {
			ac += GetAC();
		}
		ac += add_ac;

		ac += GetPetACBonusFromOwner();
		auto spell_aa_ac = aabonuses.AC + spellbonuses.AC;
		ac += GetSkill(EQ::skills::SkillDefense) / 5;
		if (EQ::ValueWithin(static_cast<int>(GetClass()), NECROMANCER, ENCHANTER))
			ac += spell_aa_ac / 3;
		else
			ac += spell_aa_ac / 4;
	}
	else { // TODO: so we can't set NPC skills ... so the skill bonus ends up being HUGE so lets nerf them a bit
		auto spell_aa_ac = aabonuses.AC + spellbonuses.AC;
		if (EQ::ValueWithin(static_cast<int>(GetClass()), NECROMANCER, ENCHANTER))
			ac += GetSkill(EQ::skills::SkillDefense) / 2 + spell_aa_ac / 3;
		else
			ac += GetSkill(EQ::skills::SkillDefense) / 3 + spell_aa_ac / 4;
	}

	if (GetAGI() > 70)
		ac += GetAGI() / 20;
	if (ac < 0)
		ac = 0;

	if (!skip_caps && (IsClient())) {
		auto softcap = GetACSoftcap();
		auto returns = GetSoftcapReturns();
		int total_aclimitmod = aabonuses.CombatStability + itembonuses.CombatStability + spellbonuses.CombatStability;
		if (total_aclimitmod)
			softcap = (softcap * (100 + total_aclimitmod)) / 100;
		softcap += shield_ac;
		if (ac > softcap) {
			auto over_cap = ac - softcap;
			ac = softcap + (over_cap * returns);
		}
		LogCombatDetail("ACSum ac [{}] softcap [{}] returns [{}]", ac, softcap, returns);
	}
	else {
		LogCombatDetail("ACSum ac [{}]", ac);
	}
	
	return ac;
}

int Mob::Tune_offense(EQ::skills::SkillType skill, int atk_override, int add_atk)
{
	int offense = GetSkill(skill);
	int stat_bonus = GetSTR();

	switch (skill) {
	case EQ::skills::SkillArchery:
	case EQ::skills::SkillThrowing:
		stat_bonus = GetDEX();
		break;

		// Mobs with no weapons default to H2H.
		// Since H2H is capped at 100 for many many classes,
		// lets not handicap mobs based on not spawning with a
		// weapon.
		//
		// Maybe we tweak this if Disarm is actually implemented.

	case EQ::skills::SkillHandtoHand:
		offense = GetBestMeleeSkill();
		break;
	}

	if (stat_bonus >= 75)
		offense += (2 * stat_bonus - 150) / 3;

	int32 tune_atk = GetATK();
	if (atk_override) {
		tune_atk = atk_override;
	}

	tune_atk += add_atk;

	offense += tune_atk + GetPetATKBonusFromOwner();
	return offense;
}

EQ::skills::SkillType Mob::Tune_AttackAnimation(int Hand, const EQ::ItemInstance* weapon, EQ::skills::SkillType skillinuse)
{
	// Determine animation
	int type = 0;
	if (weapon && weapon->IsClassCommon()) {
		const EQ::ItemData* item = weapon->GetItem();

		switch (item->ItemType) {
		case EQ::item::ItemType1HSlash: // 1H Slashing
			skillinuse = EQ::skills::Skill1HSlashing;
			type = anim1HWeapon;
			break;
		case EQ::item::ItemType2HSlash: // 2H Slashing
			skillinuse = EQ::skills::Skill2HSlashing;
			type = anim2HSlashing;
			break;
		case EQ::item::ItemType1HPiercing: // Piercing
			skillinuse = EQ::skills::Skill1HPiercing;
			type = anim1HPiercing;
			break;
		case EQ::item::ItemType1HBlunt: // 1H Blunt
			skillinuse = EQ::skills::Skill1HBlunt;
			type = anim1HWeapon;
			break;
		case EQ::item::ItemType2HBlunt: // 2H Blunt
			skillinuse = EQ::skills::Skill2HBlunt;
			type = RuleB(Combat, Classic2HBAnimation) ? anim2HWeapon : anim2HSlashing;
			break;
		case EQ::item::ItemType2HPiercing: // 2H Piercing
			if (IsClient() && CastToClient()->ClientVersion() < EQ::versions::ClientVersion::RoF2)
				skillinuse = EQ::skills::Skill1HPiercing;
			else
				skillinuse = EQ::skills::Skill2HPiercing;
			type = anim2HWeapon;
			break;
		case EQ::item::ItemTypeMartial:
			skillinuse = EQ::skills::SkillHandtoHand;
			type = animHand2Hand;
			break;
		default:
			skillinuse = EQ::skills::SkillHandtoHand;
			type = animHand2Hand;
			break;
		}// switch
	}
	else if (IsNPC()) {
		switch (skillinuse) {
		case EQ::skills::Skill1HSlashing: // 1H Slashing
			type = anim1HWeapon;
			break;
		case EQ::skills::Skill2HSlashing: // 2H Slashing
			type = anim2HSlashing;
			break;
		case EQ::skills::Skill1HPiercing: // Piercing
			type = anim1HPiercing;
			break;
		case EQ::skills::Skill1HBlunt: // 1H Blunt
			type = anim1HWeapon;
			break;
		case EQ::skills::Skill2HBlunt: // 2H Blunt
			type = anim2HSlashing; //anim2HWeapon
			break;
		case EQ::skills::Skill2HPiercing: // 2H Piercing
			type = anim2HWeapon;
			break;
		case EQ::skills::SkillHandtoHand:
			type = animHand2Hand;
			break;
		default:
			type = animHand2Hand;
			break;
		}// switch
	}
	else {
		skillinuse = EQ::skills::SkillHandtoHand;
		type = animHand2Hand;
	}

	// If we're attacking with the secondary hand, play the dual wield anim
	if (Hand == EQ::invslot::slotSecondary)	// DW anim
		type = animDualWield;

	return skillinuse;
}

int Mob::Tune_compute_tohit(EQ::skills::SkillType skillinuse, int accuracy_override, int add_accuracy)
{
	int tohit = GetSkill(EQ::skills::SkillOffense) + 7;
	tohit += GetSkill(skillinuse);
	if (IsNPC()) {
		if (accuracy_override) {
			tohit += accuracy_override;
		}
		else {
			tohit += CastToNPC()->GetAccuracyRating();
		}
		tohit += add_accuracy;
	}
	if (IsClient()) {
		double reduction = CastToClient()->m_pp.intoxication / 2.0;
		if (reduction > 20.0) {
			reduction = std::min((110 - reduction) / 100.0, 1.0);
			tohit = reduction * static_cast<double>(tohit);
		}
		else if (IsBerserk()) {
			tohit += (GetLevel() * 2) / 5;
		}
	}
	return std::max(tohit, 1);
}

// return -1 in cases that always hit
int Mob::Tune_GetTotalToHit(EQ::skills::SkillType skill, int chance_mod, int accuracy_override, int add_accuracy)
{
	if (chance_mod >= 10000) // override for stuff like SE_SkillAttack
		return -1;

	// calculate attacker's accuracy
	auto accuracy = Tune_compute_tohit(skill, accuracy_override, add_accuracy) + 10; // add 10 in case the NPC's stats are fucked
	if (chance_mod > 0) // multiplier
		accuracy *= chance_mod;

	// Torven parsed an apparent constant of 1.2 somewhere in here * 6 / 5 looks eqmathy to me!
	// new test clients have 121 / 100
	accuracy = (accuracy * 121) / 100;

	// unsure on the stacking order of these effects, rather hard to parse
	// item mod2 accuracy isn't applied to range? Theory crafting and parses back it up I guess
	// mod2 accuracy -- flat bonus
	if (skill != EQ::skills::SkillArchery && skill != EQ::skills::SkillThrowing)
		accuracy += itembonuses.HitChance;

	//518 Increase ATK accuracy by percentage, stackable
	auto atkhit_bonus = itembonuses.Attack_Accuracy_Max_Percent + aabonuses.Attack_Accuracy_Max_Percent + spellbonuses.Attack_Accuracy_Max_Percent;
	if (atkhit_bonus)
		accuracy += round(static_cast<double>(accuracy) * static_cast<double>(atkhit_bonus) * 0.0001);

	// 216 Melee Accuracy Amt aka SE_Accuracy -- flat bonus
	accuracy += itembonuses.Accuracy[EQ::skills::HIGHEST_SKILL + 1] +
		aabonuses.Accuracy[EQ::skills::HIGHEST_SKILL + 1] +
		spellbonuses.Accuracy[EQ::skills::HIGHEST_SKILL + 1] +
		itembonuses.Accuracy[skill] +
		aabonuses.Accuracy[skill] +
		spellbonuses.Accuracy[skill];

	// auto hit discs (and looks like there are some autohit AAs)
	if (spellbonuses.HitChanceEffect[skill] >= 10000 || aabonuses.HitChanceEffect[skill] >= 10000)
		return -1;

	if (spellbonuses.HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1] >= 10000)
		return -1;

	// 184 Accuracy % aka SE_HitChance -- percentage increase
	auto hit_bonus = itembonuses.HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1] +
		aabonuses.HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1] +
		spellbonuses.HitChanceEffect[EQ::skills::HIGHEST_SKILL + 1] +
		itembonuses.HitChanceEffect[skill] +
		aabonuses.HitChanceEffect[skill] +
		spellbonuses.HitChanceEffect[skill];

	accuracy = (accuracy * (100 + hit_bonus)) / 100;

	// TODO: April 2003 added an archery/throwing PVP accuracy penalty while moving, should be in here some where,
	// but PVP is less important so I haven't tried parsing it at all

	// There is also 110 Ranger Archery Accuracy % which should probably be in here some where, but it's not in any spells/aas
	// Name implies it's a percentage increase, if one wishes to implement, do it like the hit_bonus above but limited to ranger archery

	// There is also 183 UNUSED - Skill Increase Chance which devs say isn't used at all in code, but some spells reference it
	// I do not recommend implementing this once since there are spells that use it which would make this not live-like with default spell files
	return accuracy;
}

// return -1 in cases that always miss
int Mob::Tune_GetTotalDefense()
{
	auto avoidance = compute_defense() + 10; // add 10 in case the NPC's stats are fucked
	auto evasion_bonus = spellbonuses.AvoidMeleeChanceEffect; // we check this first since it has a special case
	if (evasion_bonus >= 10000)
		return -1;

	// 515 SE_AC_Avoidance_Max_Percent
	auto ac_aviodance_bonus = itembonuses.AC_Avoidance_Max_Percent + aabonuses.AC_Avoidance_Max_Percent + spellbonuses.AC_Avoidance_Max_Percent;
	if (ac_aviodance_bonus)
		avoidance += round(static_cast<double>(avoidance) * static_cast<double>(ac_aviodance_bonus) * 0.0001);

	// 172 Evasion aka SE_AvoidMeleeChance
	evasion_bonus += itembonuses.AvoidMeleeChanceEffect + aabonuses.AvoidMeleeChanceEffect; // item bonus here isn't mod2 avoidance

	// 215 Pet Avoidance % aka SE_PetAvoidance
	evasion_bonus += GetPetAvoidanceBonusFromOwner();

	// Evasion is a percentage bonus according to AA descriptions
	if (evasion_bonus)
		avoidance = (avoidance * (100 + evasion_bonus)) / 100;

	return avoidance;
}

// called when a mob is attacked, does the checks to see if it's a hit
// and does other mitigation checks. 'this' is the mob being attacked.
bool Mob::Tune_CheckHitChance(Mob* other, DamageHitInfo &hit)
{

	Mob *attacker = other;
	Mob *defender = this;
	Log(Logs::Detail, Logs::Attack, "CheckHitChance(%s) attacked by %s", defender->GetName(), attacker->GetName());

	if (defender->IsClient() && defender->CastToClient()->IsSitting())
		return true;

	auto avoidance = defender->GetTotalDefense();
	if (avoidance == -1) // some sort of auto avoid disc
		return false;

	auto accuracy = hit.tohit;
	if (accuracy == -1)
		return true;

	// so now we roll!
	// relevant dev quote:
	// Then your chance to simply avoid the attack is checked (defender's avoidance roll beat the attacker's accuracy roll.)
	int tohit_roll = zone->random.Roll0(accuracy);
	int avoid_roll = zone->random.Roll0(avoidance);
	Log(Logs::Detail, Logs::Attack, "CheckHitChance accuracy(%d => %d) avoidance(%d => %d)", accuracy, tohit_roll, avoidance, avoid_roll);

	// tie breaker? Don't want to be biased any one way
	if (tohit_roll == avoid_roll)
		return zone->random.Roll(50);
	return tohit_roll > avoid_roll;
}
