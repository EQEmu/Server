
#include "merc.h"
#include "client.h"
#include "corpse.h"
#include "entity.h"
#include "groups.h"
#include "mob.h"

#include "../common/eqemu_logsys.h"
#include "../common/eq_packet_structs.h"
#include "../common/eq_constants.h"
#include "../common/skills.h"
#include "../common/spdat.h"

#include "zone.h"
#include "string_ids.h"

#include "../common/string_util.h"
#include "../common/rulesys.h"

extern volatile bool is_zone_loaded;

#if EQDEBUG >= 12
	#define MercAI_DEBUG_Spells	25
#elif EQDEBUG >= 9
	#define MercAI_DEBUG_Spells	10
#else
	#define MercAI_DEBUG_Spells	-1
#endif

Merc::Merc(const NPCType* d, float x, float y, float z, float heading)
: NPC(d, nullptr, glm::vec4(x, y, z, heading), 0, false), endupkeep_timer(1000), rest_timer(1), confidence_timer(6000), check_target_timer(2000)
{
	base_hp = d->max_hp;
	base_mana = d->Mana;
	_baseAC = d->AC;
	_baseSTR = d->STR;
	_baseSTA = d->STA;
	_baseDEX = d->DEX;
	_baseAGI = d->AGI;
	_baseINT = d->INT;
	_baseWIS = d->WIS;
	_baseCHA = d->CHA;
	_baseATK = d->ATK;
	_baseRace = d->race;
	_baseGender = d->gender;
	_baseMR = d->MR;
	_baseCR = d->CR;
	_baseDR = d->DR;
	_baseFR = d->FR;
	_basePR = d->PR;
	_baseCorrup = d->Corrup;
	_OwnerClientVersion = static_cast<unsigned int>(EQEmu::versions::ClientVersion::Titanium);
	RestRegenHP = 0;
	RestRegenMana = 0;
	RestRegenEndurance = 0;
	cur_end = 0;

	_medding = false;
	_suspended = false;
	p_depop = false;
	_check_confidence = false;
	_lost_confidence = false;
	_hatedCount = 0;

	memset(equipment, 0, sizeof(equipment));

	SetMercID(0);
	SetStance(MercStanceBalanced);
	rest_timer.Disable();

	if (GetClass() == ROGUE)
		evade_timer.Start();

	int r;
	for (r = 0; r <= EQEmu::skills::HIGHEST_SKILL; r++) {
		skills[r] = database.GetSkillCap(GetClass(), (EQEmu::skills::SkillType)r, GetLevel());
	}

	size = d->size;
	CalcBonuses();

	// Class should use npc constructor to set light properties

	SetHP(GetMaxHP());
	SetMana(GetMaxMana());
	SetEndurance(GetMaxEndurance());

	AI_Start();
}

Merc::~Merc() {
	AI_Stop();
	//entity_list.RemoveMerc(this->GetID());
	UninitializeBuffSlots();
}

void Merc::CalcBonuses()
{
	memset(&itembonuses, 0, sizeof(StatBonuses));
	memset(&aabonuses, 0, sizeof(StatBonuses));
	CalcItemBonuses(&itembonuses);

	CalcSpellBonuses(&spellbonuses);

	CalcAC();
	CalcATK();

	CalcSTR();
	CalcSTA();
	CalcDEX();
	CalcAGI();
	CalcINT();
	CalcWIS();
	CalcCHA();

	CalcMR();
	CalcFR();
	CalcDR();
	CalcPR();
	CalcCR();
	CalcCorrup();

	CalcMaxHP();
	CalcMaxMana();
	CalcMaxEndurance();

	rooted = FindType(SE_Root);
}

float Merc::GetDefaultSize() {

	float MercSize = GetSize();

	switch(this->GetRace())
	{
		case 1: // Humans
			MercSize = 6.0;
			break;
		case 2: // Barbarian
			MercSize = 7.0;
			break;
		case 3: // Erudite
			MercSize = 6.0;
			break;
		case 4: // Wood Elf
			MercSize = 5.0;
			break;
		case 5: // High Elf
			MercSize = 6.0;
			break;
		case 6: // Dark Elf
			MercSize = 5.0;
			break;
		case 7: // Half Elf
			MercSize = 5.5;
			break;
		case 8: // Dwarf
			MercSize = 4.0;
			break;
		case 9: // Troll
			MercSize = 8.0;
			break;
		case 10: // Ogre
			MercSize = 9.0;
			break;
		case 11: // Halfling
			MercSize = 3.5;
			break;
		case 12: // Gnome
			MercSize = 3.0;
			break;
		case 128: // Iksar
			MercSize = 6.0;
			break;
		case 130: // Vah Shir
			MercSize = 7.0;
			break;
		case 330: // Froglok
			MercSize = 5.0;
			break;
		case 522: // Drakkin
			MercSize = 5.0;
			break;
		default:
			MercSize = 6.0;
			break;
	}

	return MercSize;
}

int Merc::CalcRecommendedLevelBonus(uint8 level, uint8 reclevel, int basestat)
{
	if( (reclevel > 0) && (level < reclevel) )
	{
		int32 statmod = (level * 10000 / reclevel) * basestat;

		if( statmod < 0 )
		{
			statmod -= 5000;
			return (statmod/10000);
		}
		else
		{
			statmod += 5000;
			return (statmod/10000);
		}
	}

	return 0;
}

void Merc::CalcItemBonuses(StatBonuses* newbon) {
	//memset assumed to be done by caller.


	unsigned int i;
	//should not include 21 (SLOT_AMMO)
	for (i = 0; i < EQEmu::invslot::slotAmmo; i++) {
		if(equipment[i] == 0)
			continue;
		const EQEmu::ItemData * itm = database.GetItem(equipment[i]);
		if(itm)
			AddItemBonuses(itm, newbon);
	}

	//Power Source Slot
	/*if (GetClientVersion() >= EQClientSoF)
	{
	const EQEmu::ItemInstance* inst = m_inv[MainPowerSource];
	if(inst)
	AddItemBonuses(inst, newbon);
	}*/

	// Caps
	if(newbon->HPRegen > CalcHPRegenCap())
		newbon->HPRegen = CalcHPRegenCap();

	if(newbon->ManaRegen > CalcManaRegenCap())
		newbon->ManaRegen = CalcManaRegenCap();

	if(newbon->EnduranceRegen > CalcEnduranceRegenCap())
		newbon->EnduranceRegen = CalcEnduranceRegenCap();

	SetAttackTimer();
}

void Merc::AddItemBonuses(const EQEmu::ItemData *item, StatBonuses* newbon) {

	if(GetLevel() < item->ReqLevel)
	{
		return;
	}

	if(GetLevel() >= item->RecLevel)
	{
		newbon->AC += item->AC;
		newbon->HP += item->HP;
		newbon->Mana += item->Mana;
		newbon->Endurance += item->Endur;
		newbon->STR += (item->AStr + item->HeroicStr);
		newbon->STA += (item->ASta + item->HeroicSta);
		newbon->DEX += (item->ADex + item->HeroicDex);
		newbon->AGI += (item->AAgi + item->HeroicAgi);
		newbon->INT += (item->AInt + item->HeroicInt);
		newbon->WIS += (item->AWis + item->HeroicWis);
		newbon->CHA += (item->ACha + item->HeroicCha);

		newbon->MR += (item->MR + item->HeroicMR);
		newbon->FR += (item->FR + item->HeroicFR);
		newbon->CR += (item->CR + item->HeroicCR);
		newbon->PR += (item->PR + item->HeroicPR);
		newbon->DR += (item->DR + item->HeroicDR);
		newbon->Corrup += (item->SVCorruption + item->HeroicSVCorrup);

		newbon->STRCapMod += item->HeroicStr;
		newbon->STACapMod += item->HeroicSta;
		newbon->DEXCapMod += item->HeroicDex;
		newbon->AGICapMod += item->HeroicAgi;
		newbon->INTCapMod += item->HeroicInt;
		newbon->WISCapMod += item->HeroicWis;
		newbon->CHACapMod += item->HeroicCha;
		newbon->MRCapMod += item->HeroicMR;
		newbon->CRCapMod += item->HeroicFR;
		newbon->FRCapMod += item->HeroicCR;
		newbon->PRCapMod += item->HeroicPR;
		newbon->DRCapMod += item->HeroicDR;
		newbon->CorrupCapMod += item->HeroicSVCorrup;

		newbon->HeroicSTR += item->HeroicStr;
		newbon->HeroicSTA += item->HeroicSta;
		newbon->HeroicDEX += item->HeroicDex;
		newbon->HeroicAGI += item->HeroicAgi;
		newbon->HeroicINT += item->HeroicInt;
		newbon->HeroicWIS += item->HeroicWis;
		newbon->HeroicCHA += item->HeroicCha;
		newbon->HeroicMR += item->HeroicMR;
		newbon->HeroicFR += item->HeroicFR;
		newbon->HeroicCR += item->HeroicCR;
		newbon->HeroicPR += item->HeroicPR;
		newbon->HeroicDR += item->HeroicDR;
		newbon->HeroicCorrup += item->HeroicSVCorrup;

	}
	else
	{
		int lvl = GetLevel();
		int reclvl = item->RecLevel;

		newbon->AC += CalcRecommendedLevelBonus( lvl, reclvl, item->AC );
		newbon->HP += CalcRecommendedLevelBonus( lvl, reclvl, item->HP );
		newbon->Mana += CalcRecommendedLevelBonus( lvl, reclvl, item->Mana );
		newbon->Endurance += CalcRecommendedLevelBonus( lvl, reclvl, item->Endur );
		newbon->STR += CalcRecommendedLevelBonus( lvl, reclvl, (item->AStr + item->HeroicStr) );
		newbon->STA += CalcRecommendedLevelBonus( lvl, reclvl, (item->ASta + item->HeroicSta) );
		newbon->DEX += CalcRecommendedLevelBonus( lvl, reclvl, (item->ADex + item->HeroicDex) );
		newbon->AGI += CalcRecommendedLevelBonus( lvl, reclvl, (item->AAgi + item->HeroicAgi) );
		newbon->INT += CalcRecommendedLevelBonus( lvl, reclvl, (item->AInt + item->HeroicInt) );
		newbon->WIS += CalcRecommendedLevelBonus( lvl, reclvl, (item->AWis + item->HeroicWis) );
		newbon->CHA += CalcRecommendedLevelBonus( lvl, reclvl, (item->ACha + item->HeroicCha) );

		newbon->MR += CalcRecommendedLevelBonus( lvl, reclvl, (item->MR + item->HeroicMR) );
		newbon->FR += CalcRecommendedLevelBonus( lvl, reclvl, (item->FR + item->HeroicFR) );
		newbon->CR += CalcRecommendedLevelBonus( lvl, reclvl, (item->CR + item->HeroicCR) );
		newbon->PR += CalcRecommendedLevelBonus( lvl, reclvl, (item->PR + item->HeroicPR) );
		newbon->DR += CalcRecommendedLevelBonus( lvl, reclvl, (item->DR + item->HeroicDR) );
		newbon->Corrup += CalcRecommendedLevelBonus( lvl, reclvl, (item->SVCorruption + item->HeroicSVCorrup) );

		newbon->STRCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicStr );
		newbon->STACapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicSta );
		newbon->DEXCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicDex );
		newbon->AGICapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicAgi );
		newbon->INTCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicInt );
		newbon->WISCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicWis );
		newbon->CHACapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicCha );
		newbon->MRCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicMR );
		newbon->CRCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicFR );
		newbon->FRCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicCR );
		newbon->PRCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicPR );
		newbon->DRCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicDR );
		newbon->CorrupCapMod += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicSVCorrup );

		newbon->HeroicSTR += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicStr );
		newbon->HeroicSTA += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicSta );
		newbon->HeroicDEX += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicDex );
		newbon->HeroicAGI += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicAgi );
		newbon->HeroicINT += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicInt );
		newbon->HeroicWIS += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicWis );
		newbon->HeroicCHA += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicCha );
		newbon->HeroicMR += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicMR );
		newbon->HeroicFR += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicFR );
		newbon->HeroicCR += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicCR );
		newbon->HeroicPR += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicPR );
		newbon->HeroicDR += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicDR );
		newbon->HeroicCorrup += CalcRecommendedLevelBonus( lvl, reclvl, item->HeroicSVCorrup );
	}

	//FatherNitwit: New style haste, shields, and regens
	if(newbon->haste < (int16)item->Haste) {
		newbon->haste = item->Haste;
	}
	if(item->Regen > 0)
		newbon->HPRegen += item->Regen;

	if(item->ManaRegen > 0)
		newbon->ManaRegen += item->ManaRegen;

	if(item->EnduranceRegen > 0)
		newbon->EnduranceRegen += item->EnduranceRegen;

	if(item->Attack > 0) {

		unsigned int cap = RuleI(Character, ItemATKCap);
		cap += itembonuses.ItemATKCap + spellbonuses.ItemATKCap + aabonuses.ItemATKCap;

		if((newbon->ATK + item->Attack) > cap)
			newbon->ATK = RuleI(Character, ItemATKCap);
		else
			newbon->ATK += item->Attack;
	}
	if(item->DamageShield > 0) {
		if((newbon->DamageShield + item->DamageShield) > RuleI(Character, ItemDamageShieldCap))
			newbon->DamageShield = RuleI(Character, ItemDamageShieldCap);
		else
			newbon->DamageShield += item->DamageShield;
	}
	if(item->SpellShield > 0) {
		if((newbon->SpellShield + item->SpellShield) > RuleI(Character, ItemSpellShieldingCap))
			newbon->SpellShield = RuleI(Character, ItemSpellShieldingCap);
		else
			newbon->SpellShield += item->SpellShield;
	}
	if(item->Shielding > 0) {
		if((newbon->MeleeMitigation + item->Shielding) > RuleI(Character, ItemShieldingCap))
			newbon->MeleeMitigation = RuleI(Character, ItemShieldingCap);
		else
			newbon->MeleeMitigation += item->Shielding;
	}
	if(item->StunResist > 0) {
		if((newbon->StunResist + item->StunResist) > RuleI(Character, ItemStunResistCap))
			newbon->StunResist = RuleI(Character, ItemStunResistCap);
		else
			newbon->StunResist += item->StunResist;
	}
	if(item->StrikeThrough > 0) {
		if((newbon->StrikeThrough + item->StrikeThrough) > RuleI(Character, ItemStrikethroughCap))
			newbon->StrikeThrough = RuleI(Character, ItemStrikethroughCap);
		else
			newbon->StrikeThrough += item->StrikeThrough;
	}
	if(item->Avoidance > 0) {
		if((newbon->AvoidMeleeChance + item->Avoidance) > RuleI(Character, ItemAvoidanceCap))
			newbon->AvoidMeleeChance = RuleI(Character, ItemAvoidanceCap);
		else
			newbon->AvoidMeleeChance += item->Avoidance;
	}
	if(item->Accuracy > 0) {
		if((newbon->HitChance + item->Accuracy) > RuleI(Character, ItemAccuracyCap))
			newbon->HitChance = RuleI(Character, ItemAccuracyCap);
		else
			newbon->HitChance += item->Accuracy;
	}
	if(item->CombatEffects > 0) {
		if((newbon->ProcChance + item->CombatEffects) > RuleI(Character, ItemCombatEffectsCap))
			newbon->ProcChance = RuleI(Character, ItemCombatEffectsCap);
		else
			newbon->ProcChance += item->CombatEffects;
	}
	if(item->DotShielding > 0) {
		if((newbon->DoTShielding + item->DotShielding) > RuleI(Character, ItemDoTShieldingCap))
			newbon->DoTShielding = RuleI(Character, ItemDoTShieldingCap);
		else
			newbon->DoTShielding += item->DotShielding;
	}

	if(item->HealAmt > 0) {
		if((newbon->HealAmt + item->HealAmt) > RuleI(Character, ItemHealAmtCap))
			newbon->HealAmt = RuleI(Character, ItemHealAmtCap);
		else
			newbon->HealAmt += item->HealAmt;
	}
	if(item->SpellDmg > 0) {
		if((newbon->SpellDmg + item->SpellDmg) > RuleI(Character, ItemSpellDmgCap))
			newbon->SpellDmg = RuleI(Character, ItemSpellDmgCap);
		else
			newbon->SpellDmg += item->SpellDmg;
	}
	if(item->Clairvoyance > 0) {
		if((newbon->Clairvoyance + item->Clairvoyance) > RuleI(Character, ItemClairvoyanceCap))
			newbon->Clairvoyance = RuleI(Character, ItemClairvoyanceCap);
		else
			newbon->Clairvoyance += item->Clairvoyance;
	}

	if(item->DSMitigation > 0) {
		if((newbon->DSMitigation + item->DSMitigation) > RuleI(Character, ItemDSMitigationCap))
			newbon->DSMitigation = RuleI(Character, ItemDSMitigationCap);
		else
			newbon->DSMitigation += item->DSMitigation;
	}
	if (item->Worn.Effect>0 && (item->Worn.Type == EQEmu::item::ItemEffectWorn)) { // latent effects
		ApplySpellsBonuses(item->Worn.Effect, item->Worn.Level, newbon, 0, item->Worn.Type);
	}

	if (item->Focus.Effect>0 && (item->Focus.Type == EQEmu::item::ItemEffectFocus)) { // focus effects
		ApplySpellsBonuses(item->Focus.Effect, item->Focus.Level, newbon, 0);
	}

	switch(item->BardType)
	{
	case 51: /* All (e.g. Singing Short Sword) */
		{
			if(item->BardValue > newbon->singingMod)
				newbon->singingMod = item->BardValue;
			if(item->BardValue > newbon->brassMod)
				newbon->brassMod = item->BardValue;
			if(item->BardValue > newbon->stringedMod)
				newbon->stringedMod = item->BardValue;
			if(item->BardValue > newbon->percussionMod)
				newbon->percussionMod = item->BardValue;
			if(item->BardValue > newbon->windMod)
				newbon->windMod = item->BardValue;
			break;
		}
	case 50: /* Singing */
		{
			if(item->BardValue > newbon->singingMod)
				newbon->singingMod = item->BardValue;
			break;
		}
	case 23: /* Wind */
		{
			if(item->BardValue > newbon->windMod)
				newbon->windMod = item->BardValue;
			break;
		}
	case 24: /* stringed */
		{
			if(item->BardValue > newbon->stringedMod)
				newbon->stringedMod = item->BardValue;
			break;
		}
	case 25: /* brass */
		{
			if(item->BardValue > newbon->brassMod)
				newbon->brassMod = item->BardValue;
			break;
		}
	case 26: /* Percussion */
		{
			if(item->BardValue > newbon->percussionMod)
				newbon->percussionMod = item->BardValue;
			break;
		}
	}

	if (item->SkillModValue != 0 && item->SkillModType <= EQEmu::skills::HIGHEST_SKILL){
		if ((item->SkillModValue > 0 && newbon->skillmod[item->SkillModType] < item->SkillModValue) ||
			(item->SkillModValue < 0 && newbon->skillmod[item->SkillModType] > item->SkillModValue))
		{
			newbon->skillmod[item->SkillModType] = item->SkillModValue;
		}
	}

	// Add Item Faction Mods
	if (item->FactionMod1)
	{
		if (item->FactionAmt1 > 0 && item->FactionAmt1 > GetItemFactionBonus(item->FactionMod1))
		{
			AddItemFactionBonus(item->FactionMod1, item->FactionAmt1);
		}
		else if (item->FactionAmt1 < 0 && item->FactionAmt1 < GetItemFactionBonus(item->FactionMod1))
		{
			AddItemFactionBonus(item->FactionMod1, item->FactionAmt1);
		}
	}
	if (item->FactionMod2)
	{
		if (item->FactionAmt2 > 0 && item->FactionAmt2 > GetItemFactionBonus(item->FactionMod2))
		{
			AddItemFactionBonus(item->FactionMod2, item->FactionAmt2);
		}
		else if (item->FactionAmt2 < 0 && item->FactionAmt2 < GetItemFactionBonus(item->FactionMod2))
		{
			AddItemFactionBonus(item->FactionMod2, item->FactionAmt2);
		}
	}
	if (item->FactionMod3)
	{
		if (item->FactionAmt3 > 0 && item->FactionAmt3 > GetItemFactionBonus(item->FactionMod3))
		{
			AddItemFactionBonus(item->FactionMod3, item->FactionAmt3);
		}
		else if (item->FactionAmt3 < 0 && item->FactionAmt3 < GetItemFactionBonus(item->FactionMod3))
		{
			AddItemFactionBonus(item->FactionMod3, item->FactionAmt3);
		}
	}
	if (item->FactionMod4)
	{
		if (item->FactionAmt4 > 0 && item->FactionAmt4 > GetItemFactionBonus(item->FactionMod4))
		{
			AddItemFactionBonus(item->FactionMod4, item->FactionAmt4);
		}
		else if (item->FactionAmt4 < 0 && item->FactionAmt4 < GetItemFactionBonus(item->FactionMod4))
		{
			AddItemFactionBonus(item->FactionMod4, item->FactionAmt4);
		}
	}

	if (item->ExtraDmgSkill != 0 && item->ExtraDmgSkill <= EQEmu::skills::HIGHEST_SKILL) {
		if((newbon->SkillDamageAmount[item->ExtraDmgSkill] + item->ExtraDmgAmt) > RuleI(Character, ItemExtraDmgCap))
			newbon->SkillDamageAmount[item->ExtraDmgSkill] = RuleI(Character, ItemExtraDmgCap);
		else
			newbon->SkillDamageAmount[item->ExtraDmgSkill] += item->ExtraDmgAmt;
	}
}

int Merc::GroupLeadershipAAHealthEnhancement()
{
	Group *g = GetGroup();

	if(!g || (g->GroupCount() < 3))
		return 0;

	switch(g->GetLeadershipAA(groupAAHealthEnhancement))
	{
	case 0:
		return 0;
	case 1:
		return 30;
	case 2:
		return 60;
	case 3:
		return 100;
	}

	return 0;
}

int Merc::GroupLeadershipAAManaEnhancement()
{
	Group *g = GetGroup();

	if(!g || (g->GroupCount() < 3))
		return 0;

	switch(g->GetLeadershipAA(groupAAManaEnhancement))
	{
	case 0:
		return 0;
	case 1:
		return 30;
	case 2:
		return 60;
	case 3:
		return 100;
	}

	return 0;
}

int Merc::GroupLeadershipAAHealthRegeneration()
{
	Group *g = GetGroup();

	if(!g || (g->GroupCount() < 3))
		return 0;

	switch(g->GetLeadershipAA(groupAAHealthRegeneration))
	{
	case 0:
		return 0;
	case 1:
		return 4;
	case 2:
		return 6;
	case 3:
		return 8;
	}

	return 0;
}

int Merc::GroupLeadershipAAOffenseEnhancement()
{
	Group *g = GetGroup();

	if(!g || (g->GroupCount() < 3))
		return 0;

	switch(g->GetLeadershipAA(groupAAOffenseEnhancement))
	{
	case 0:
		return 0;
	case 1:
		return 10;
	case 2:
		return 19;
	case 3:
		return 28;
	case 4:
		return 34;
	case 5:
		return 40;
	}
	return 0;
}

int32 Merc::CalcSTR() {
	int32 val = _baseSTR + itembonuses.STR + spellbonuses.STR;

	int32 mod = aabonuses.STR;

	STR = val + mod;

	if(STR < 1)
		STR = 1;

	return(STR);
}

int32 Merc::CalcSTA() {
	int32 val = _baseSTA + itembonuses.STA + spellbonuses.STA;

	int32 mod = aabonuses.STA;

	STA = val + mod;

	if(STA < 1)
		STA = 1;

	return(STA);
}

int32 Merc::CalcAGI() {
	int32 val = _baseAGI + itembonuses.AGI + spellbonuses.AGI;
	int32 mod = aabonuses.AGI;

	int32 str = GetSTR();

	AGI = val + mod;

	if(AGI < 1)
		AGI = 1;

	return(AGI);
}

int32 Merc::CalcDEX() {
	int32 val = _baseDEX + itembonuses.DEX + spellbonuses.DEX;

	int32 mod = aabonuses.DEX;

	DEX = val + mod;

	if(DEX < 1)
		DEX = 1;

	return(DEX);
}

int32 Merc::CalcINT() {
	int32 val = _baseINT + itembonuses.INT + spellbonuses.INT;

	int32 mod = aabonuses.INT;

	INT = val + mod;

	if(INT < 1)
		INT = 1;

	return(INT);
}

int32 Merc::CalcWIS() {
	int32 val = _baseWIS + itembonuses.WIS + spellbonuses.WIS;

	int32 mod = aabonuses.WIS;

	WIS = val + mod;

	if(WIS < 1)
		WIS = 1;

	return(WIS);
}

int32 Merc::CalcCHA() {
	int32 val = _baseCHA + itembonuses.CHA + spellbonuses.CHA;

	int32 mod = aabonuses.CHA;

	CHA = val + mod;

	if(CHA < 1)
		CHA = 1;

	return(CHA);
}

//The AA multipliers are set to be 5, but were 2 on WR
//The resistant discipline which I think should be here is implemented
//in Mob::ResistSpell
int32 Merc::CalcMR()
{
	MR = _baseMR + itembonuses.MR + spellbonuses.MR + aabonuses.MR;

	if(MR < 1)
		MR = 1;

	return(MR);
}

int32 Merc::CalcFR()
{
	FR = _baseFR + itembonuses.FR + spellbonuses.FR + aabonuses.FR;

	if(FR < 1)
		FR = 1;

	return(FR);
}

int32 Merc::CalcDR()
{
	DR = _baseDR + itembonuses.DR + spellbonuses.DR + aabonuses.DR;

	if(DR < 1)
		DR = 1;

	return(DR);
}

int32 Merc::CalcPR()
{
	PR = _basePR + itembonuses.PR + spellbonuses.PR + aabonuses.PR;

	if(PR < 1)
		PR = 1;

	return(PR);
}

int32 Merc::CalcCR()
{
	CR = _baseCR + itembonuses.CR + spellbonuses.CR + aabonuses.CR;

	if(CR < 1)
		CR = 1;

	return(CR);
}

int32 Merc::CalcCorrup()
{
	Corrup = _baseCorrup + itembonuses.Corrup + spellbonuses.Corrup + aabonuses.Corrup;

	return(Corrup);
}

int32 Merc::CalcATK() {
	ATK = _baseATK + itembonuses.ATK + spellbonuses.ATK + aabonuses.ATK + GroupLeadershipAAOffenseEnhancement();
	return(ATK);
}

int32 Merc::CalcAC() {
	//spell AC bonuses are added directly to natural total
	AC = _baseAC + spellbonuses.AC;
	return(AC);
}

int32 Merc::CalcHPRegen() {
	int32 regen = hp_regen + itembonuses.HPRegen + spellbonuses.HPRegen;

	regen += aabonuses.HPRegen + GroupLeadershipAAHealthRegeneration();

	return (regen * RuleI(Character, HPRegenMultiplier) / 100);
}

int32 Merc::CalcHPRegenCap()
{
	int cap = RuleI(Character, ItemHealthRegenCap) + itembonuses.HeroicSTA/25;

	cap += aabonuses.ItemHPRegenCap + spellbonuses.ItemHPRegenCap + itembonuses.ItemHPRegenCap;

	return (cap * RuleI(Character, HPRegenMultiplier) / 100);
}

int32 Merc::CalcMaxHP() {
	float nd = 10000;
	max_hp = (CalcBaseHP() + itembonuses.HP);

	//The AA desc clearly says it only applies to base hp..
	//but the actual effect sent on live causes the client
	//to apply it to (basehp + itemhp).. I will oblige to the client's whims over
	//the aa description
	nd += aabonuses.MaxHP;  //Natural Durability, Physical Enhancement, Planar Durability

	max_hp = (float)max_hp * (float)nd / (float)10000; //this is to fix the HP-above-495k issue
	max_hp += spellbonuses.HP + aabonuses.HP;

	max_hp += GroupLeadershipAAHealthEnhancement();

	max_hp += max_hp * ((spellbonuses.MaxHPChange + itembonuses.MaxHPChange) / 10000.0f);

	if (cur_hp > max_hp)
		cur_hp = max_hp;

	int hp_perc_cap = spellbonuses.HPPercCap[0];
	if(hp_perc_cap) {
		int curHP_cap = (max_hp * hp_perc_cap) / 100;
		if (cur_hp > curHP_cap || (spellbonuses.HPPercCap[1] && cur_hp > spellbonuses.HPPercCap[1]))
			cur_hp = curHP_cap;
	}

	return max_hp;
}

int32 Merc::CalcBaseHP()
{
	return base_hp;
}

int32 Merc::CalcMaxMana()
{
	switch(GetCasterClass())
	{
	case 'I':
	case 'W': {
		max_mana = (CalcBaseMana() + itembonuses.Mana + spellbonuses.Mana + GroupLeadershipAAManaEnhancement());
		break;
			  }
	case 'N': {
		max_mana = 0;
		break;
			  }
	default: {
		Log(Logs::General, Logs::None, "Invalid Class '%c' in CalcMaxMana", GetCasterClass());
		max_mana = 0;
		break;
			 }
	}
	if (max_mana < 0) {
		max_mana = 0;
	}

	if (current_mana > max_mana) {
		current_mana = max_mana;
	}

	int mana_perc_cap = spellbonuses.ManaPercCap[0];
	if(mana_perc_cap) {
		int curMana_cap = (max_mana * mana_perc_cap) / 100;
		if (current_mana > curMana_cap  || (spellbonuses.ManaPercCap[1] && current_mana > spellbonuses.ManaPercCap[1]))
			current_mana = curMana_cap;
	}

#if EQDEBUG >= 11
	Log(Logs::General, Logs::None, "Merc::CalcMaxMana() called for %s - returning %d", GetName(), max_mana);
#endif
	return max_mana;
}

int32 Merc::CalcBaseMana()
{
	return base_mana;
}

int32 Merc::CalcBaseManaRegen()
{
	uint8 clevel = GetLevel();
	int32 regen = 0;
	if (IsSitting())
	{
		if (HasSkill(EQEmu::skills::SkillMeditate))
			regen = (((GetSkill(EQEmu::skills::SkillMeditate) / 10) + (clevel - (clevel / 4))) / 4) + 4;
		else
			regen = 2;
	}
	else {
		regen = 2;
	}
	return regen;
}

int32 Merc::CalcManaRegen()
{
	int32 regen = 0;
	if (IsSitting())
	{
		BuffFadeBySitModifier();
		if (HasSkill(EQEmu::skills::SkillMeditate)) {
			this->_medding = true;
			regen = ((GetSkill(EQEmu::skills::SkillMeditate) / 10) + mana_regen);
			regen += spellbonuses.ManaRegen + itembonuses.ManaRegen;
		}
		else
			regen = mana_regen + spellbonuses.ManaRegen + itembonuses.ManaRegen;
	}
	else {
		this->_medding = false;
		regen = mana_regen + spellbonuses.ManaRegen + itembonuses.ManaRegen;
	}

	if(GetCasterClass() == 'I')
		regen += (itembonuses.HeroicINT / 25);
	else if(GetCasterClass() == 'W')
		regen += (itembonuses.HeroicWIS / 25);
	else
		regen = 0;

	//AAs
	regen += aabonuses.ManaRegen;

	return (regen * RuleI(Character, ManaRegenMultiplier) / 100);
}

int32 Merc::CalcManaRegenCap()
{
	int32 cap = RuleI(Character, ItemManaRegenCap) + aabonuses.ItemManaRegenCap;
	switch(GetCasterClass())
	{
	case 'I':
		cap += (itembonuses.HeroicINT / 25);
		break;
	case 'W':
		cap += (itembonuses.HeroicWIS / 25);
		break;
	}

	return (cap * RuleI(Character, ManaRegenMultiplier) / 100);
}

void Merc::CalcMaxEndurance()
{
	max_end = CalcBaseEndurance() + spellbonuses.Endurance + itembonuses.Endurance + aabonuses.Endurance;

	if (max_end < 0) {
		max_end = 0;
	}

	if (cur_end > max_end) {
		cur_end = max_end;
	}

	int end_perc_cap = spellbonuses.EndPercCap[0];
	if(end_perc_cap) {
		int curEnd_cap = (max_end * end_perc_cap) / 100;
		if (cur_end > curEnd_cap || (spellbonuses.EndPercCap[1] && cur_end > spellbonuses.EndPercCap[1]))
			cur_end = curEnd_cap;
	}
}

int32 Merc::CalcBaseEndurance()
{
	int32 base_end = 0;
	int32 base_endurance = 0;
	int32 ConvertedStats = 0;
	int32 sta_end = 0;
	int Stats = 0;

	if (GetClientVersion() >= static_cast<unsigned int>(EQEmu::versions::ClientVersion::SoD) && RuleB(Character, SoDClientUseSoDHPManaEnd)) {
		int HeroicStats = 0;

		Stats = ((GetSTR() + GetSTA() + GetDEX() + GetAGI()) / 4);
		HeroicStats = ((GetHeroicSTR() + GetHeroicSTA() + GetHeroicDEX() + GetHeroicAGI()) / 4);

		if (Stats > 100) {
			ConvertedStats = (((Stats - 100) * 5 / 2) + 100);
			if (Stats > 201) {
				ConvertedStats -= ((Stats - 201) * 5 / 4);
			}
		}
		else {
			ConvertedStats = Stats;
		}

		if (GetLevel() < 41) {
			sta_end = (GetLevel() * 75 * ConvertedStats / 1000);
			base_endurance = (GetLevel() * 15);
		}
		else if (GetLevel() < 81) {
			sta_end = ((3 * ConvertedStats) + ((GetLevel() - 40) * 15 * ConvertedStats / 100));
			base_endurance = (600 + ((GetLevel() - 40) * 30));
		}
		else {
			sta_end = (9 * ConvertedStats);
			base_endurance = (1800 + ((GetLevel() - 80) * 18));
		}
		base_end = (base_endurance + sta_end + (HeroicStats * 10));
	}
	else
	{
		Stats = GetSTR()+GetSTA()+GetDEX()+GetAGI();
		int LevelBase = GetLevel() * 15;

		int at_most_800 = Stats;
		if(at_most_800 > 800)
			at_most_800 = 800;

		int Bonus400to800 = 0;
		int HalfBonus400to800 = 0;
		int Bonus800plus = 0;
		int HalfBonus800plus = 0;

		int BonusUpto800 = int( at_most_800 / 4 ) ;
		if(Stats > 400) {
			Bonus400to800 = int( (at_most_800 - 400) / 4 );
			HalfBonus400to800 = int( std::max( ( at_most_800 - 400 ), 0 ) / 8 );

			if(Stats > 800) {
				Bonus800plus = int( (Stats - 800) / 8 ) * 2;
				HalfBonus800plus = int( (Stats - 800) / 16 );
			}
		}
		int bonus_sum = BonusUpto800 + Bonus400to800 + HalfBonus400to800 + Bonus800plus + HalfBonus800plus;

		base_end = LevelBase;

		//take all of the sums from above, then multiply by level*0.075
		base_end += ( bonus_sum * 3 * GetLevel() ) / 40;
	}
	return base_end;
}

int32 Merc::CalcEnduranceRegen() {
	int32 regen = int32(GetLevel() * 4 / 10) + 2;
	regen += aabonuses.EnduranceRegen + spellbonuses.EnduranceRegen + itembonuses.EnduranceRegen;

	return (regen * RuleI(Character, EnduranceRegenMultiplier) / 100);
}

int32 Merc::CalcEnduranceRegenCap() {
	int cap = (RuleI(Character, ItemEnduranceRegenCap) + itembonuses.HeroicSTR/25 + itembonuses.HeroicDEX/25 + itembonuses.HeroicAGI/25 + itembonuses.HeroicSTA/25);

	return (cap * RuleI(Character, EnduranceRegenMultiplier) / 100);
}

void Merc::SetEndurance(int32 newEnd)
{
	/*Endurance can't be less than 0 or greater than max*/
	if(newEnd < 0)
		newEnd = 0;
	else if(newEnd > GetMaxEndurance()){
		newEnd = GetMaxEndurance();
	}

	cur_end = newEnd;
}

void Merc::DoEnduranceUpkeep() {

	if (!HasEndurUpkeep())
		return;

	int upkeep_sum = 0;
	int cost_redux = spellbonuses.EnduranceReduction + itembonuses.EnduranceReduction;

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
						continue;       //reduced to 0
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

	if (!has_effect)
		SetEndurUpkeep(false);
}

void Merc::CalcRestState() {

	// This method calculates rest state HP and mana regeneration.
	// The bot must have been out of combat for RuleI(Character, RestRegenTimeToActivate) seconds,
	// must be sitting down, and must not have any detrimental spells affecting them.
	//
	if(!RuleB(Character, RestRegenEnabled))
		return;

	RestRegenHP = RestRegenMana = RestRegenEndurance = 0;

	if(IsEngaged() || !IsSitting())
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

	RestRegenHP = 6 * (GetMaxHP() / zone->newzone_data.FastRegenHP);

	RestRegenMana = 6 * (GetMaxMana() / zone->newzone_data.FastRegenMana);

	RestRegenEndurance = 6 * (GetMaxEndurance() / zone->newzone_data.FastRegenEndurance);
}

bool Merc::HasSkill(EQEmu::skills::SkillType skill_id) const {
	return((GetSkill(skill_id) > 0) && CanHaveSkill(skill_id));
}

bool Merc::CanHaveSkill(EQEmu::skills::SkillType skill_id) const {
	return(database.GetSkillCap(GetClass(), skill_id, RuleI(Character, MaxLevel)) > 0);
	//if you don't have it by max level, then odds are you never will?
}

uint16 Merc::MaxSkill(EQEmu::skills::SkillType skillid, uint16 class_, uint16 level) const {
	return(database.GetSkillCap(class_, skillid, level));
}

void Merc::FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho) {
	if(ns) {
		Mob::FillSpawnStruct(ns, ForWho);

		ns->spawn.afk = 0;
		ns->spawn.lfg = 0;
		ns->spawn.anon = 0;
		ns->spawn.gm = 0;
		ns->spawn.guildID = 0xFFFFFFFF;         // 0xFFFFFFFF = NO GUILD, 0 = Unknown Guild
		ns->spawn.is_npc = 1;                           // 0=no, 1=yes
		ns->spawn.is_pet = 0;
		ns->spawn.guildrank = 0;
		ns->spawn.showhelm = 1;
		ns->spawn.flymode = 0;
		ns->spawn.NPC = 1;                                      // 0=player,1=npc,2=pc corpse,3=npc corpse
		ns->spawn.IsMercenary = 1;
		ns->spawn.show_name = true;

		UpdateActiveLight();
		ns->spawn.light = m_Light.Type[EQEmu::lightsource::LightActive];

		/*
		// Wear Slots are not setup for Mercs yet
		unsigned int i;
		for (i = 0; i < _MaterialCount; i++)
		{
			if (equipment[i] == 0)
			{
				continue;
			}
			const ItemData* item = database.GetItem(equipment[i]);
			if(item)
			{
				ns->spawn.equipment[i].material = item->Material;
				ns->spawn.equipment[i].elitematerial = item->EliteMaterial;
				ns->spawn.equipment[i].heroforgemodel = item->HerosForgeModel;
				if (armor_tint[i])
				{
					ns->spawn.colors[i].color = armor_tint[i];
				}
				else
				{
					ns->spawn.colors[i].color = item->Color;
				}
			}
		}
		*/
	}
}

bool Merc::Process()
{
	if(IsStunned() && stunned_timer.Check())
		Mob::UnStun();

	if (GetDepop())
	{
		SetMercCharacterID(0);
		SetOwnerID(0);
		return false;
	}

	if(!GetMercOwner()) {
		//p_depop = true;               //this was causing a crash - removed merc from entity list, but not group
		//return false;                 //merc can live after client dies, not sure how long
	}

	if(IsSuspended())
	{
		return false;
	}

	if (HasGroup() && GetMercOwner() && GetFollowID() == 0) {
		SetFollowID(GetMercOwner()->GetID());
	}

	SpellProcess();

	if(tic_timer.Check())
	{
		//6 seconds, or whatever the rule is set to has passed, send this position to everyone to avoid ghosting
		if(!IsMoving() && !IsEngaged())
		{
			SendPosition();
			if(IsSitting()) {
				if(!rest_timer.Enabled()) {
					rest_timer.Start(RuleI(Character, RestRegenTimeToActivate) * 1000);
				}
			}
		}

		BuffProcess();

		CalcRestState();

		if(GetHP() < GetMaxHP())
			SetHP(GetHP() + CalcHPRegen() + RestRegenHP);

		if(GetMana() < GetMaxMana())
			SetMana(GetMana() + CalcManaRegen() + RestRegenMana);

		if(GetEndurance() < GetMaxEndurance())
			SetEndurance(GetEndurance() + CalcEnduranceRegen() + RestRegenEndurance);
	}

	if(confidence_timer.Check()) {
		_check_confidence = true;
	}

	if (send_hp_update_timer.Check()) {
		SendHPUpdate();
	}

	if (endupkeep_timer.Check() && GetHP() > 0){
		DoEnduranceUpkeep();
	}

	if (IsStunned() || IsMezzed())
		return true;

	// Merc AI
	AI_Process();

	return true;
}

bool Merc::IsMercCasterCombatRange(Mob *target) {
	bool result = false;

	if(target) {
		float range = MercAISpellRange;

		range *= range;

		// half the max so the merc doesn't always stop at max range to allow combat movement
		range *= .5;

		float targetDistance = DistanceSquaredNoZ(m_Position, target->GetPosition());

		if(targetDistance > range)
			result = false;
		else
			result = true;
	}

	return result;
}

void Merc::AI_Process() {
	if(!IsAIControlled())
		return;

	if(IsCasting())
		return;

	// A merc wont start its AI if not grouped
	if(!HasGroup()) {
		return;
	}

	Mob* MercOwner = GetOwner();

	if(GetAppearance() == eaDead)
	{
		if(!MercOwner)
		{
			Depop();
		}
		return;
	}

	// The merc needs an owner
	if(!MercOwner) {
		//SetTarget(0);
		//SetOwnerID(0);
		// TODO: Need to wait and try casting rez if merc is a healer with a dead owner
		return;
	}

	/*
	try {
		if(MercOwner->CastToClient()->IsDead()) {
			SetTarget(0);
			SetOwnerID(0);
			return;
		}
	}
	catch(...) {
		SetTarget(0);
		SetOwnerID(0);
		return;
	}
	*/

	if(check_target_timer.Check()) {
		CheckHateList();
	}

	if(IsEngaged())
	{
		if(rest_timer.Enabled())
			rest_timer.Disable();

		if(IsRooted())
			SetTarget(hate_list.GetClosestEntOnHateList(this));
		else
			FindTarget();

		if(!GetTarget())
			return;

		if(HasPet())
			GetPet()->SetTarget(GetTarget());

		if(!IsSitting())
			FaceTarget(GetTarget());

		if(DivineAura())
			return;

		int hateCount = entity_list.GetHatedCount(this, nullptr, false);
		if(GetHatedCount() < hateCount) {
			SetHatedCount(hateCount);

			if(!CheckConfidence()) {
				if(!confidence_timer.Enabled()) {
					confidence_timer.Start(10000);
				}
			}
		}

		//Check specific conditions for merc to lose confidence and flee (or regain confidence once fleeing)
		if(_check_confidence) {
			//not already running
			if(!_lost_confidence) {
				//and fail confidence check
				if(!CheckConfidence()) {
					_lost_confidence = true;

					//move to bottom of hate lists?
					//Iterate though hatelist
					// SetHate(other, hate, damage)

					if(RuleB(Combat, EnableFearPathing)) {
						CalculateNewFearpoint();
						if(currently_fleeing) {
							return;
						}
					}
					else {
						Stun(12000 - (6000 - tic_timer.GetRemainingTime()));
					}
				}
			}
			else { //are fleeing due to lost confidence
				if(CheckConfidence()) { //passed test - regain confidence
					_lost_confidence = false;
				}
			}

			//they are in flee mode
			if(_lost_confidence)
				return;
		}

		// Let's check if we have a los with our target.
		// If we don't, our hate_list is wiped.
		// Else, it was causing the merc to aggro behind wall etc... causing massive trains.
		if(GetTarget()->IsMezzed() || !IsAttackAllowed(GetTarget())) {
			WipeHateList();

			if(IsMoving()) {
				SetHeading(0);
				SetRunAnimSpeed(0);

				if(moved) {
					moved = false;
					SetCurrentSpeed(0);
				}
			}

			return;
		}
		else if (!CheckLosFN(GetTarget())) {
			if (RuleB(Mercs, MercsUsePathing) && zone->pathing) {
				bool WaypointChanged, NodeReached;

				glm::vec3 Goal = UpdatePath(
					GetTarget()->GetX(),
					GetTarget()->GetY(),
					GetTarget()->GetZ(),
					GetRunspeed(),
					WaypointChanged,
					NodeReached
				);

				if (WaypointChanged)
					tar_ndx = 20;

				CalculateNewPosition(Goal.x, Goal.y, Goal.z, GetRunspeed());
			}
			else {
				Mob* follow = entity_list.GetMob(GetFollowID());
				if (follow)
					CalculateNewPosition(follow->GetX(), follow->GetY(), follow->GetZ(), GetRunspeed());
			}

			return;
		}

		if (!(m_PlayerState & static_cast<uint32>(PlayerState::Aggressive)))
			SendAddPlayerState(PlayerState::Aggressive);

		bool atCombatRange = false;

		float meleeDistance = GetMaxMeleeRangeToTarget(GetTarget());

		if(GetClass() == SHADOWKNIGHT || GetClass() == PALADIN || GetClass() == WARRIOR) {
			meleeDistance = meleeDistance * .30;
		}
		else {
			meleeDistance *= (float)zone->random.Real(.50, .85);
		}
		if(IsMercCaster() && GetLevel() > 12) {
			if(IsMercCasterCombatRange(GetTarget()))
				atCombatRange = true;
		}
		else if(DistanceSquared(m_Position, GetTarget()->GetPosition()) <= meleeDistance) {
			atCombatRange = true;
		}

		if(atCombatRange)
		{
			if(IsMoving())
			{
				SetHeading(CalculateHeadingToTarget(GetTarget()->GetX(), GetTarget()->GetY()));
				SetRunAnimSpeed(0);

				if(moved) {
					SetCurrentSpeed(0);
				}
			}

			if(AI_movement_timer->Check()) {
				if (!IsMoving()) {
					if (GetClass() == ROGUE) {
						if (HasTargetReflection() && !GetTarget()->IsFeared() && !GetTarget()->IsStunned()) {
							// Hate redux actions
							if (evade_timer.Check(false)) {
								// Attempt to evade
								int timer_duration = (HideReuseTime - GetSkillReuseTime(EQEmu::skills::SkillHide)) * 1000;
								if (timer_duration < 0)
									timer_duration = 0;
								evade_timer.Start(timer_duration);

								if (zone->random.Int(0, 260) < (int)GetSkill(EQEmu::skills::SkillHide))
									RogueEvade(GetTarget());

								return;
							}
							else if (GetTarget()->IsRooted()) {
								// Move rogue back from rooted mob - out of combat range, if necessary
								float melee_distance = GetMaxMeleeRangeToTarget(GetTarget());
								float current_distance = DistanceSquared(static_cast<glm::vec3>(m_Position), static_cast<glm::vec3>(GetTarget()->GetPosition()));

								if (current_distance <= melee_distance) {
									float newX = 0;
									float newY = 0;
									float newZ = 0;
									FaceTarget(GetTarget());
									if (PlotPositionAroundTarget(this, newX, newY, newZ)) {
										CalculateNewPosition(newX, newY, newZ, GetRunspeed());
										return;
									}
								}
							}
						}
						else if (!BehindMob(GetTarget(), GetX(), GetY())) {
							// Move the rogue to behind the mob
							float newX = 0;
							float newY = 0;
							float newZ = 0;
							if (PlotPositionAroundTarget(GetTarget(), newX, newY, newZ)) {
								CalculateNewPosition(newX, newY, newZ, GetRunspeed());
								return;
							}
						}
					}
					else if (GetClass() != ROGUE && (DistanceSquaredNoZ(m_Position, GetTarget()->GetPosition()) < GetTarget()->GetSize())) {
						// If we are not a rogue trying to backstab, let's try to adjust our melee range so we don't appear to be bunched up
						float newX = 0;
						float newY = 0;
						float newZ = 0;
						if (PlotPositionAroundTarget(GetTarget(), newX, newY, newZ, false) && GetArchetype() != ARCHETYPE_CASTER) {
							CalculateNewPosition(newX, newY, newZ, GetRunspeed());
							return;
						}
					}
				}

				if (IsMoving())
					SendPositionUpdate();
				else
					SendPosition();
			}

			if(!IsMercCaster() && GetTarget() && !IsStunned() && !IsMezzed() && (GetAppearance() != eaDead))
			{
				// we can't fight if we don't have a target, are stun/mezzed or dead..
				// Stop attacking if the target is enraged
				if(IsEngaged() && !BehindMob(GetTarget(), GetX(), GetY()) && GetTarget()->IsEnraged())
					return;
				//TODO: Implement Stances.
				/*if(GetBotStance() == BotStancePassive)
				return;*/

				// First, special attack per class (kick, backstab etc..)
				DoClassAttacks(GetTarget());

				//try main hand first
				if(attack_timer.Check())
				{
					Attack(GetTarget(), EQEmu::invslot::slotPrimary);

					bool tripleSuccess = false;

					if(GetOwner() && GetTarget() && CanThisClassDoubleAttack())
					{
						if(GetOwner()) {
							Attack(GetTarget(), EQEmu::invslot::slotPrimary, true);
						}

						if(GetOwner() && GetTarget() && GetSpecialAbility(SPECATK_TRIPLE)) {
							tripleSuccess = true;
							Attack(GetTarget(), EQEmu::invslot::slotPrimary, true);
						}

						//quad attack, does this belong here??
						if(GetOwner() && GetTarget() && GetSpecialAbility(SPECATK_QUAD)) {
							Attack(GetTarget(), EQEmu::invslot::slotPrimary, true);
						}
					}

					//Live AA - Flurry, Rapid Strikes ect (Flurry does not require Triple Attack).
					int16 flurrychance = aabonuses.FlurryChance + spellbonuses.FlurryChance + itembonuses.FlurryChance;

					if (GetTarget() && flurrychance)
					{
						if(zone->random.Roll(flurrychance))
						{
							Message_StringID(MT_NPCFlurry, YOU_FLURRY);
							Attack(GetTarget(), EQEmu::invslot::slotPrimary, false);
							Attack(GetTarget(), EQEmu::invslot::slotPrimary, false);
						}
					}

					int16 ExtraAttackChanceBonus = spellbonuses.ExtraAttackChance + itembonuses.ExtraAttackChance + aabonuses.ExtraAttackChance;

					if (GetTarget() && ExtraAttackChanceBonus) {
						if(zone->random.Roll(ExtraAttackChanceBonus))
						{
							Attack(GetTarget(), EQEmu::invslot::slotPrimary, false);
						}
					}
				}

				// TODO: Do mercs berserk? Find this out on live...
				//if (GetClass() == WARRIOR || GetClass() == BERSERKER) {
				//      if(GetHP() > 0 && !berserk && this->GetHPRatio() < 30) {
				//              entity_list.MessageClose_StringID(this, false, 200, 0, BERSERK_START, GetName());
				//              this->berserk = true;
				//      }
				//      if (berserk && this->GetHPRatio() > 30) {
				//              entity_list.MessageClose_StringID(this, false, 200, 0, BERSERK_END, GetName());
				//              this->berserk = false;
				//      }
				//}

				//now off hand
				if(GetTarget() && attack_dw_timer.Check() && CanThisClassDualWield())
				{
					int weapontype = 0; // No weapon type
					bool bIsFist = true;

					// why are we checking 'weapontype' when we know it's set to '0' above?
					if (bIsFist || ((weapontype != EQEmu::item::ItemType2HSlash) && (weapontype != EQEmu::item::ItemType2HPiercing) && (weapontype != EQEmu::item::ItemType2HBlunt)))
					{
						float DualWieldProbability = 0.0f;

						int16 Ambidexterity = aabonuses.Ambidexterity + spellbonuses.Ambidexterity + itembonuses.Ambidexterity;
						DualWieldProbability = (GetSkill(EQEmu::skills::SkillDualWield) + GetLevel() + Ambidexterity) / 400.0f; // 78.0 max
						int16 DWBonus = spellbonuses.DualWieldChance + itembonuses.DualWieldChance;
						DualWieldProbability += DualWieldProbability*float(DWBonus)/ 100.0f;

						// Max 78% of DW
						if (zone->random.Roll(DualWieldProbability))
						{
							Attack(GetTarget(), EQEmu::invslot::slotSecondary);     // Single attack with offhand

							if(CanThisClassDoubleAttack()) {
								if(GetTarget() && GetTarget()->GetHP() > -10)
									Attack(GetTarget(), EQEmu::invslot::slotSecondary);     // Single attack with offhand
							}
						}
					}
				}
			}
		}
		else
		{
			if(GetTarget()->IsFeared() && !spellend_timer.Enabled()) {
				// This is a mob that is fleeing either because it has been feared or is low on hitpoints
				//TODO: Implement Stances.
				//if(GetStance() != MercStancePassive)
				AI_PursueCastCheck();
			}

			if (AI_movement_timer->Check())
			{
				if(!IsRooted()) {
					Log(Logs::Detail, Logs::AI, "Pursuing %s while engaged.", GetTarget()->GetCleanName());
					CalculateNewPosition(GetTarget()->GetX(), GetTarget()->GetY(), GetTarget()->GetZ(), GetRunspeed());
					return;
				}

				if(IsMoving())
					SendPositionUpdate();
				else
					SendPosition();
			}
		} // end not in combat range

		if(!IsMoving() && !spellend_timer.Enabled())
		{
			//TODO: Implement Stances.
			//if(GetStance() == MercStancePassive)
			//      return;

			if(AI_EngagedCastCheck()) {
				MercMeditate(false);
			}
			else if(GetArchetype() == ARCHETYPE_CASTER)
				MercMeditate(true);
		}
	}
	else
	{
		// Not engaged in combat
		SetTarget(0);
		SetHatedCount(0);
		confidence_timer.Disable();
		_check_confidence = false;

		if (m_PlayerState & static_cast<uint32>(PlayerState::Aggressive))
			SendRemovePlayerState(PlayerState::Aggressive);

		if(!check_target_timer.Enabled())
			check_target_timer.Start(2000, false);

		if(!IsMoving() && AI_think_timer->Check() && !spellend_timer.Enabled())
		{
			//TODO: Implement passive stances.
			//if(GetStance() != MercStancePassive) {
			if(!AI_IdleCastCheck() && !IsCasting()) {
				if(GetArchetype() == ARCHETYPE_CASTER) {
					MercMeditate(true);
				}
			}
		}

		if(AI_movement_timer->Check()) {
			if(GetFollowID()) {
				Mob* follow = entity_list.GetMob(GetFollowID());

				if (follow) {
					float dist = DistanceSquared(m_Position, follow->GetPosition());
					int speed = GetRunspeed();

					if (dist < GetFollowDistance() + 1000)
						speed = GetWalkspeed();

					SetRunAnimSpeed(0);

					if (dist > GetFollowDistance()) {
						if (RuleB(Mercs, MercsUsePathing) && zone->pathing) {
							bool WaypointChanged, NodeReached;

							glm::vec3 Goal = UpdatePath(follow->GetX(), follow->GetY(), follow->GetZ(),
								speed, WaypointChanged, NodeReached);

							if (WaypointChanged)
								tar_ndx = 20;

							CalculateNewPosition(Goal.x, Goal.y, Goal.z, speed);
						}
						else {
							CalculateNewPosition(follow->GetX(), follow->GetY(), follow->GetZ(), speed);
						}

						if (rest_timer.Enabled())
							rest_timer.Disable();
					}
					else {
						if (moved) {
							moved = false;
							SetCurrentSpeed(0);
						}
					}
				}
			}
		}
	}
}

void Merc::AI_Start(int32 iMoveDelay) {
	if (!pAIControlled)
		return;

	if (merc_spells.empty()) {
		AIautocastspell_timer->SetTimer(1000);
		AIautocastspell_timer->Disable();
	} else {
		AIautocastspell_timer->SetTimer(750);
		AIautocastspell_timer->Start(RandomTimer(0, 2000), false);
	}

	if (NPCTypedata_ours) {
		ProcessSpecialAbilities(NPCTypedata_ours->special_abilities);
	}

	SendTo(GetX(), GetY(), GetZ());
	SetChanged();
	SaveGuardSpot();
}

void Merc::AI_Stop() {
	NPC::AI_Stop();
	Mob::AI_Stop();
}

bool Merc::AI_EngagedCastCheck() {
	bool result = false;
	bool failedToCast = false;

	if (GetTarget() && AIautocastspell_timer->Check(false))
	{
		AIautocastspell_timer->Disable();       //prevent the timer from going off AGAIN while we are casting.

		Log(Logs::Detail, Logs::AI, "Merc Engaged autocast check triggered");

		int8 mercClass = GetClass();

		switch(mercClass)
		{
		case TANK:
			if (!AICastSpell(GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
				if (!AICastSpell(GetChanceToCastBySpellType(SpellType_InCombatBuff), SpellType_InCombatBuff)) {
					failedToCast = true;
				}
			}
			break;
		case HEALER:
			if(!entity_list.Merc_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_Heal), MercAISpellRange, SpellType_Heal)) {
				if(!entity_list.Merc_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_Buff), MercAISpellRange, SpellType_Buff)) {
					failedToCast = true;
				}
			}
			break;
		case MELEEDPS:
			if (!AICastSpell(GetChanceToCastBySpellType(SpellType_Escape), SpellType_Escape)) {
				if (!AICastSpell(GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
					if (!AICastSpell(GetChanceToCastBySpellType(SpellType_InCombatBuff), SpellType_InCombatBuff)) {
						failedToCast = true;
					}
				}
			}
			break;
		case CASTERDPS:
			if (!AICastSpell(GetChanceToCastBySpellType(SpellType_Escape), SpellType_Escape)) {
				if (!AICastSpell(GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
					failedToCast = true;
				}
			}
			break;
		}

		if(!AIautocastspell_timer->Enabled()) {
			AIautocastspell_timer->Start(RandomTimer(100, 250), false);
		}

		if(!failedToCast)
			result = true;
	}

	return result;
}

bool Merc::AI_IdleCastCheck() {
	bool result = false;
	bool failedToCast = false;

	if (AIautocastspell_timer->Check(false)) {
#if MercAI_DEBUG_Spells >= 25
		Log(Logs::Detail, Logs::AI, "Merc Non-Engaged autocast check triggered: %s", this->GetCleanName());
#endif
		AIautocastspell_timer->Disable();       //prevent the timer from going off AGAIN while we are casting.

		//Ok, IdleCastCheck depends of class.
		int8 mercClass = GetClass();

		switch(mercClass)
		{
		case TANK:
			failedToCast = true;
			break;
		case HEALER:
			if(!entity_list.Merc_AICheckCloseBeneficialSpells(this, 100, MercAISpellRange, SpellType_Cure)) {
				if(!entity_list.Merc_AICheckCloseBeneficialSpells(this, 100, MercAISpellRange, SpellType_Heal)) {
					if(!entity_list.Merc_AICheckCloseBeneficialSpells(this, 100, MercAISpellRange, SpellType_Resurrect)) {
						if(!entity_list.Merc_AICheckCloseBeneficialSpells(this, 100, MercAISpellRange, SpellType_Buff)) {
							failedToCast = true;
						}
					}
				}
			}
			result = true;
			break;
		case MELEEDPS:
			if(!entity_list.Merc_AICheckCloseBeneficialSpells(this, 100, MercAISpellRange, SpellType_Buff)) {
				failedToCast = true;
			}
			break;
		case CASTERDPS:
			failedToCast = true;
			break;
		}

		if(!AIautocastspell_timer->Enabled())
			AIautocastspell_timer->Start(RandomTimer(500, 1000), false);

		if(!failedToCast)
			result = true;
	}

	return result;
}

bool EntityList::Merc_AICheckCloseBeneficialSpells(Merc* caster, uint8 iChance, float iRange, uint32 iSpellTypes) {

	if((iSpellTypes&SpellTypes_Detrimental) != 0) {
		//according to live, you can buff and heal through walls...
		//now with PCs, this only applies if you can TARGET the target, but
		// according to Rogean, Live NPCs will just cast through walls/floors, no problem..
		//
		// This check was put in to address an idle-mob CPU issue
		Log(Logs::General, Logs::Error, "Error: detrimental spells requested from AICheckCloseBeneficialSpells!!");
		return(false);
	}

	if(!caster)
		return false;

	if(!caster->AI_HasSpells())
		return false;

	if (iChance < 100) {
		int8 tmp = zone->random.Int(1, 100);
		if (tmp > iChance)
			return false;
	}

	int8 mercCasterClass = caster->GetClass();

	if(caster->HasGroup()) {
		if( mercCasterClass == HEALER) {
			if( iSpellTypes == SpellType_Heal )     {
				if(caster->AICastSpell(100, SpellType_Heal))
					return true;
			}

			if( iSpellTypes == SpellType_Cure )     {
				if(caster->AICastSpell(100, SpellType_Cure))
					return true;
			}

			if( iSpellTypes == SpellType_Resurrect )        {
				if(caster->AICastSpell(100, SpellType_Resurrect))
					return true;
			}
		}

		//Ok for the buffs..
		if( iSpellTypes == SpellType_Buff) {
			if(caster->AICastSpell(100, SpellType_Buff))
				return true;
		}
	}

	return false;
}

bool Merc::AIDoSpellCast(uint16 spellid, Mob* tar, int32 mana_cost, uint32* oDontDoAgainBefore) {
	bool result = false;
	MercSpell mercSpell = GetMercSpellBySpellID(this, spellid);

	// manacost has special values, -1 is no mana cost, -2 is instant cast (no mana)
	int32 manaCost = mana_cost;

	if (manaCost == -1)
		manaCost = spells[spellid].mana;
	else if (manaCost == -2)
		manaCost = 0;

	int32 extraMana = 0;
	int32 hasMana = GetMana();

	float dist2 = 0;

	if (mercSpell.type & SpellType_Escape) {
		dist2 = 0;
	} else
		dist2 = DistanceSquared(m_Position, tar->GetPosition());

	if (((((spells[spellid].targettype==ST_GroupTeleport && mercSpell.type==SpellType_Heal)
		|| spells[spellid].targettype==ST_AECaster
		|| spells[spellid].targettype==ST_Group
		|| spells[spellid].targettype==ST_AEBard)
		&& dist2 <= spells[spellid].aoerange*spells[spellid].aoerange)
		|| dist2 <= GetActSpellRange(spellid, spells[spellid].range)*GetActSpellRange(spellid, spells[spellid].range)) && (mana_cost <= GetMana() || GetMana() == GetMaxMana()))
	{
		SetRunAnimSpeed(0);
		SendPosition();
		SetMoving(false);

		result = CastSpell(spellid, tar->GetID(), EQEmu::CastingSlot::Gem2, -1, mana_cost, oDontDoAgainBefore, -1, -1, 0, 0);

		if(IsCasting() && IsSitting())
			Stand();
	}

	// if the spell wasn't casted, then take back any extra mana that was given to the bot to cast that spell
	if(!result) {
		SetMana(hasMana);
		extraMana = false;
	}
	else { //handle spell recast and recast timers
		SetSpellTimeCanCast(mercSpell.spellid, spells[spellid].recast_time);

		if(spells[spellid].EndurTimerIndex > 0) {
			SetSpellRecastTimer(spells[spellid].EndurTimerIndex, spellid, spells[spellid].recast_time);
		}
	}

	return result;
}

bool Merc::AICastSpell(int8 iChance, uint32 iSpellTypes) {

	if(!AI_HasSpells())
		return false;

	if (iChance < 100) {
		if (zone->random.Int(0, 100) > iChance){
			return false;
		}
	}

	int8 mercClass = GetClass();
	uint8 mercLevel = GetLevel();

	bool checked_los = false;       //we do not check LOS until we are absolutely sure we need to, and we only do it once.
	bool castedSpell = false;
	bool isDiscipline = false;

	if(HasGroup()) {
		Group *g = GetGroup();

		if(g) {
			MercSpell selectedMercSpell;
			selectedMercSpell.spellid = 0;
			selectedMercSpell.stance = 0;
			selectedMercSpell.type = 0;
			selectedMercSpell.slot = 0;
			selectedMercSpell.proc_chance = 0;
			selectedMercSpell.time_cancast = 0;

			switch(mercClass)
			{
			case TANK:
			case MELEEDPS:
				isDiscipline = true;
				break;
			default:
				isDiscipline = false;
				break;
			}

			switch (iSpellTypes) {
							case SpellType_Heal: {
								Mob* tar = nullptr;
								int8 numToHeal = g->GetNumberNeedingHealedInGroup(IsEngaged() ? 75 : 95, true);
								int8 checkHPR = IsEngaged() ? 95 : 99;
								int8 checkPetHPR = IsEngaged() ? 95 : 99;

								//todo: check stance to determine healing spell selection

								for(int i = 0; i < MAX_GROUP_MEMBERS; i++) {
									if(g->members[i] && !g->members[i]->qglobal) {
										int8 hpr = (int8)g->members[i]->GetHPRatio();

										if(g->members[i]->HasPet() && g->members[i]->GetPet()->GetHPRatio() < checkHPR) {
											if(!tar || ((g->members[i]->GetPet()->GetHPRatio() + 25) < tar->GetHPRatio())) {
												tar = g->members[i]->GetPet();
												checkPetHPR = g->members[i]->GetPet()->GetHPRatio() + 25;
											}
										}

										if(hpr > checkHPR) {
											continue;
										}

										if(IsEngaged() && (g->members[i]->GetClass() == NECROMANCER && hpr >= 50)
											|| (g->members[i]->GetClass() == SHAMAN && hpr >= 80)) {
												//allow necros to lifetap & shaman to canni without wasting mana
												continue;
										}

										if(hpr < checkHPR && g->members[i] == GetMercOwner()) {
											if(!tar || (hpr < tar->GetHPRatio() || (tar->IsPet() && hpr < checkPetHPR)))
												tar = g->members[i];            //check owner first
										}
										else if(hpr < checkHPR && g->HasRole(g->members[i], RoleTank)){
											if(!tar || (hpr < tar->GetHPRatio() || (tar->IsPet() && hpr < checkPetHPR)))
												tar = g->members[i];
										}
										else if( hpr < checkHPR && (!tar || (hpr < tar->GetHPRatio() || (tar->IsPet() && hpr < checkPetHPR)))) {
											tar = g->members[i];
										}
									}
								}

								if(numToHeal > 2) {
									selectedMercSpell = GetBestMercSpellForGroupHeal(this);
								}

								if(tar && selectedMercSpell.spellid == 0) {
									if(tar->GetHPRatio() < 15) {
										//check for very fast heals first (casting time < 1 s)
										selectedMercSpell = GetBestMercSpellForVeryFastHeal(this);

										//check for fast heals next (casting time < 2 s)
										if(selectedMercSpell.spellid == 0) {
											selectedMercSpell = GetBestMercSpellForFastHeal(this);
										}

										//get regular heal
										if(selectedMercSpell.spellid == 0) {
											selectedMercSpell = GetBestMercSpellForRegularSingleTargetHeal(this);
										}
									}
									else if (tar->GetHPRatio() < 35) {
										//check for fast heals next (casting time < 2 s)
										selectedMercSpell = GetBestMercSpellForFastHeal(this);

										//get regular heal
										if(selectedMercSpell.spellid == 0) {
											selectedMercSpell = GetBestMercSpellForRegularSingleTargetHeal(this);
										}
									}
									else if (tar->GetHPRatio() < 80) {
										selectedMercSpell = GetBestMercSpellForPercentageHeal(this);

										//get regular heal
										if(selectedMercSpell.spellid == 0) {
											selectedMercSpell = GetBestMercSpellForRegularSingleTargetHeal(this);
										}
									}
									else {
										//check for heal over time. if not present, try it first
										if(!tar->FindType(SE_HealOverTime)) {
											selectedMercSpell = GetBestMercSpellForHealOverTime(this);

											//get regular heal
											if(selectedMercSpell.spellid == 0) {
												selectedMercSpell = GetBestMercSpellForRegularSingleTargetHeal(this);
											}
										}
									}
								}

								if(selectedMercSpell.spellid > 0) {
									castedSpell = AIDoSpellCast(selectedMercSpell.spellid, tar, -1);
								}

								if(castedSpell) {
									char* gmsg = nullptr;

									if(tar != this) {
										//we don't need spam of bots healing themselves
										MakeAnyLenString(&gmsg, "Casting %s on %s.", spells[selectedMercSpell.spellid].name, tar->GetCleanName());
										if(gmsg)
										{
											MercGroupSay(this, gmsg);
											safe_delete_array(gmsg);
										}
									}
								}

								break;
												 }
							case SpellType_Root: {
								break;
												 }
							case SpellType_Buff: {

								if(GetClass() == HEALER && GetManaRatio() < 50) {
									return false;                   //mercs buff when Mana > 50%
								}

								std::list<MercSpell> buffSpellList = GetMercSpellsBySpellType(this, SpellType_Buff);

								for (auto itr = buffSpellList.begin();
								     itr != buffSpellList.end(); ++itr) {
									MercSpell selectedMercSpell = *itr;

									if(!((spells[selectedMercSpell.spellid].targettype == ST_Target || spells[selectedMercSpell.spellid].targettype == ST_Pet ||
										spells[selectedMercSpell.spellid].targettype == ST_Group || spells[selectedMercSpell.spellid].targettype == ST_GroupTeleport ||
										spells[selectedMercSpell.spellid].targettype == ST_Self))) {
											continue;
									}

									if(spells[selectedMercSpell.spellid].targettype == ST_Self) {
										if( !this->IsImmuneToSpell(selectedMercSpell.spellid, this)
											&& (this->CanBuffStack(selectedMercSpell.spellid, mercLevel, true) >= 0)) {

												if( this->GetArchetype() == ARCHETYPE_MELEE && IsEffectInSpell(selectedMercSpell.spellid, SE_IncreaseSpellHaste)) {
													continue;
												}

												uint32 TempDontBuffMeBeforeTime = this->DontBuffMeBefore();

												if(selectedMercSpell.spellid > 0) {
													if(isDiscipline) {
														castedSpell = UseDiscipline(selectedMercSpell.spellid, GetID());
													}
													else {
														castedSpell = AIDoSpellCast(selectedMercSpell.spellid, this, -1, &TempDontBuffMeBeforeTime);

														if(TempDontBuffMeBeforeTime != this->DontBuffMeBefore())
															this->SetDontBuffMeBefore(TempDontBuffMeBeforeTime);
													}
												}
										}
									}
									else {
										for( int i = 0; i < MAX_GROUP_MEMBERS; i++) {
											if(g->members[i]) {
												Mob* tar = g->members[i];

												if( !tar->IsImmuneToSpell(selectedMercSpell.spellid, this)
													&& (tar->CanBuffStack(selectedMercSpell.spellid, mercLevel, true) >= 0)) {

														if( tar->GetArchetype() == ARCHETYPE_MELEE && IsEffectInSpell(selectedMercSpell.spellid, SE_IncreaseSpellHaste)) {
															continue;
														}

														uint32 TempDontBuffMeBeforeTime = tar->DontBuffMeBefore();

														if(selectedMercSpell.spellid > 0) {
															if(isDiscipline) {
																castedSpell = UseDiscipline(selectedMercSpell.spellid, tar->GetID());
															}
															else {
																castedSpell = AIDoSpellCast(selectedMercSpell.spellid, tar, -1, &TempDontBuffMeBeforeTime);

																if(TempDontBuffMeBeforeTime != tar->DontBuffMeBefore())
																	tar->SetDontBuffMeBefore(TempDontBuffMeBeforeTime);
															}
														}
												}

												if(!castedSpell && tar->GetPet()) {

													//don't cast group spells on pets
													if(IsGroupSpell(selectedMercSpell.spellid)
														|| spells[selectedMercSpell.spellid].targettype == ST_Group
														|| spells[selectedMercSpell.spellid].targettype == ST_GroupTeleport ) {
															continue;
													}

													if(!tar->GetPet()->IsImmuneToSpell(selectedMercSpell.spellid, this)
														&& (tar->GetPet()->CanBuffStack(selectedMercSpell.spellid, mercLevel, true) >= 0)) {

															uint32 TempDontBuffMeBeforeTime = tar->DontBuffMeBefore();

															if(selectedMercSpell.spellid > 0) {
																if(isDiscipline) {
																	castedSpell = UseDiscipline(selectedMercSpell.spellid, tar->GetPet()->GetID());
																}
																else {
																	castedSpell = AIDoSpellCast(selectedMercSpell.spellid, tar->GetPet(), -1, &TempDontBuffMeBeforeTime);

																	if(TempDontBuffMeBeforeTime != tar->GetPet()->DontBuffMeBefore())
																		tar->GetPet()->SetDontBuffMeBefore(TempDontBuffMeBeforeTime);
																}
															}
													}
												}
											}
										}
									}
								}
								break;
												 }
							case SpellType_Nuke: {
								switch(mercClass)
								{
								case TANK:
									//check for taunt
									if(CheckAETaunt()) {
										//get AE taunt
										selectedMercSpell = GetBestMercSpellForAETaunt(this);
										Log(Logs::General, Logs::Mercenaries, "%s AE Taunting.", GetName());
									}

									if(selectedMercSpell.spellid == 0 && CheckTaunt()) {
										//get taunt
										selectedMercSpell = GetBestMercSpellForTaunt(this);
									}

									//get hate disc
									if(selectedMercSpell.spellid == 0) {
										selectedMercSpell = GetBestMercSpellForHate(this);
									}

									break;
								case HEALER:
									break;
								case MELEEDPS:
									break;
								case CASTERDPS:
									Mob* tar = GetTarget();

									selectedMercSpell = GetBestMercSpellForAENuke(this, tar);

									if(selectedMercSpell.spellid == 0 && !tar->GetSpecialAbility(UNSTUNABLE) && !tar->IsStunned()) {
										uint8 stunChance = 15;
										if(zone->random.Roll(stunChance)) {
											selectedMercSpell = GetBestMercSpellForStun(this);
										}
									}

									if(selectedMercSpell.spellid == 0) {
										uint8 lureChance = 25;
										if(zone->random.Roll(lureChance)) {
											selectedMercSpell = GetBestMercSpellForNukeByTargetResists(this, tar);
										}
									}

									if(selectedMercSpell.spellid == 0) {
										selectedMercSpell = GetBestMercSpellForNuke(this);
									}

									break;
								}

								if(selectedMercSpell.spellid > 0) {
									if(isDiscipline) {
										castedSpell = UseDiscipline(selectedMercSpell.spellid, GetTarget()->GetID());
									}
									else {
										castedSpell = AIDoSpellCast(selectedMercSpell.spellid, GetTarget(), -1);
									}
								}

								break;
												 }
							case SpellType_InCombatBuff: {
								std::list<MercSpell> buffSpellList = GetMercSpellsBySpellType(this, SpellType_InCombatBuff);
								Mob* tar = this;

								for (auto itr = buffSpellList.begin();
								     itr != buffSpellList.end(); ++itr) {
									MercSpell selectedMercSpell = *itr;

									if(!(spells[selectedMercSpell.spellid].targettype == ST_Self)) {
										continue;
									}

									if (spells[selectedMercSpell.spellid].skill == EQEmu::skills::SkillBackstab && spells[selectedMercSpell.spellid].targettype == ST_Self) {
										if(!hidden) {
											continue;
										}
									}

									if( !tar->IsImmuneToSpell(selectedMercSpell.spellid, this)
										&& (tar->CanBuffStack(selectedMercSpell.spellid, mercLevel, true) >= 0)) {

											uint32 TempDontBuffMeBeforeTime = tar->DontBuffMeBefore();

											if(selectedMercSpell.spellid > 0) {
												if(isDiscipline) {
													castedSpell = UseDiscipline(selectedMercSpell.spellid, GetID());
												}
												else {
													castedSpell = AIDoSpellCast(selectedMercSpell.spellid, this, -1);
												}
											}
									}
								}
								break;
														 }
							case SpellType_Cure: {
								Mob* tar = nullptr;
								for(int i = 0; i < MAX_GROUP_MEMBERS; i++) {
									if(g->members[i] && !g->members[i]->qglobal) {
										if(GetNeedsCured(g->members[i]) && (g->members[i]->DontCureMeBefore() < Timer::GetCurrentTime())) {
											tar = g->members[i];
										}
									}
								}

								if(tar && !(g->GetNumberNeedingHealedInGroup(IsEngaged() ? 25 : 40, false) > 0) && !(g->GetNumberNeedingHealedInGroup(IsEngaged() ? 40 : 60, false) > 2))
								{
									selectedMercSpell = GetBestMercSpellForCure(this, tar);

									if(selectedMercSpell.spellid == 0)
										break;

									uint32 TempDontCureMeBeforeTime = tar->DontCureMeBefore();

									castedSpell = AIDoSpellCast(selectedMercSpell.spellid, tar, spells[selectedMercSpell.spellid].mana, &TempDontCureMeBeforeTime);

									if(castedSpell) {
										if(IsGroupSpell(selectedMercSpell.spellid)){

											if(this->HasGroup()) {
												Group *g = this->GetGroup();

												if(g) {
													for( int i = 0; i<MAX_GROUP_MEMBERS; i++) {
														if(g->members[i] && !g->members[i]->qglobal) {
															if(TempDontCureMeBeforeTime != tar->DontCureMeBefore())
																g->members[i]->SetDontCureMeBefore(Timer::GetCurrentTime() + 4000);
														}
													}
												}
											}
										}
										else {
											if(TempDontCureMeBeforeTime != tar->DontCureMeBefore())
												tar->SetDontCureMeBefore(Timer::GetCurrentTime() + 4000);
										}
									}
								}
								break;
												 }
							case SpellType_Resurrect: {
								Corpse *corpse = GetGroupMemberCorpse();

								if(corpse) {
									selectedMercSpell = GetFirstMercSpellBySpellType(this, SpellType_Resurrect);

									if(selectedMercSpell.spellid == 0)
										break;

									uint32 TempDontRootMeBeforeTime = corpse->DontRootMeBefore();

									castedSpell = AIDoSpellCast(selectedMercSpell.spellid, corpse, spells[selectedMercSpell.spellid].mana, &TempDontRootMeBeforeTime);

									//CastSpell(selectedMercSpell.spellid, corpse->GetID(), 1, -1, -1, &TempDontRootMeBeforeTime);
									corpse->SetDontRootMeBefore(TempDontRootMeBeforeTime);
								}

								break;
													  }
							case SpellType_Escape: {
								Mob* tar = GetTarget();
								uint8 hpr = (uint8)GetHPRatio();
								bool mayGetAggro = false;

								if(tar && (mercClass == CASTERDPS) || (mercClass == MELEEDPS)) {
									mayGetAggro = HasOrMayGetAggro(); //classes have hate reducing spells

									if (mayGetAggro) {
										selectedMercSpell = GetFirstMercSpellBySpellType(this, SpellType_Escape);

										if(selectedMercSpell.spellid == 0)
											break;

										if(isDiscipline) {
											castedSpell = UseDiscipline(selectedMercSpell.spellid, tar->GetID());
										}
										else {
											castedSpell = AIDoSpellCast(selectedMercSpell.spellid, tar, -1);
										}
									}
								}
								break;
												   }
			}
		}
	}

	return castedSpell;
}

void Merc::CheckHateList() {
	if(check_target_timer.Enabled())
		check_target_timer.Disable();

	if(!IsEngaged()) {
		if(GetFollowID()) {
			Group* g = GetGroup();
			if(g) {
				Mob* MercOwner = GetOwner();
				if(MercOwner && MercOwner->GetTarget() && MercOwner->GetTarget()->IsNPC() && (MercOwner->GetTarget()->GetHateAmount(MercOwner) || MercOwner->CastToClient()->AutoAttackEnabled()) && IsAttackAllowed(MercOwner->GetTarget())) {
					float range = g->HasRole(MercOwner, RolePuller) ? RuleI(Mercs, AggroRadiusPuller) : RuleI(Mercs, AggroRadius);
					range = range * range;
					if(DistanceSquaredNoZ(m_Position, MercOwner->GetTarget()->GetPosition()) < range) {
						AddToHateList(MercOwner->GetTarget(), 1);
					}
				}
				else {
					std::list<NPC*> npc_list;
					entity_list.GetNPCList(npc_list);

					for (auto itr = npc_list.begin(); itr != npc_list.end(); ++itr) {
						NPC* npc = *itr;
						float dist = DistanceSquaredNoZ(m_Position, npc->GetPosition());
						int radius = RuleI(Mercs, AggroRadius);
						radius *= radius;
						if(dist <= radius) {

							for(int counter = 0; counter < g->GroupCount(); counter++) {
								Mob* groupMember = g->members[counter];
								if(groupMember) {
									if(npc->IsOnHatelist(groupMember)) {
										if(!hate_list.IsEntOnHateList(npc)) {
											float range = g->HasRole(groupMember, RolePuller) ? RuleI(Mercs, AggroRadiusPuller) : RuleI(Mercs, AggroRadius);
											range *= range;
											if(DistanceSquaredNoZ(m_Position, npc->GetPosition()) < range) {
												hate_list.AddEntToHateList(npc, 1);
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
	}
}

bool Merc::HasOrMayGetAggro() {
	bool mayGetAggro = false;

	if(GetTarget() && GetTarget()->GetHateTop()) {
		Mob *topHate = GetTarget()->GetHateTop();

		if(topHate == this)
			mayGetAggro = true; //I currently have aggro
		else {
			uint32 myHateAmt = GetTarget()->GetHateAmount(this);
			uint32 topHateAmt = GetTarget()->GetHateAmount(topHate);

			if(myHateAmt > 0 && topHateAmt > 0 && (uint8)((myHateAmt/topHateAmt)*100) > 90) //I have 90% as much hate as top, next action may give me aggro
				mayGetAggro = true;
		}
	}

	return mayGetAggro;
}

bool Merc::CheckAENuke(Merc* caster, Mob* tar, uint16 spell_id, uint8 &numTargets) {
	std::list<NPC*> npc_list;
	entity_list.GetNPCList(npc_list);

	for (auto itr = npc_list.begin(); itr != npc_list.end(); ++itr) {
		NPC* npc = *itr;

		if(DistanceSquaredNoZ(npc->GetPosition(), tar->GetPosition()) <= spells[spell_id].aoerange * spells[spell_id].aoerange) {
			if(!npc->IsMezzed()) {
				numTargets++;
			}
			else {
				numTargets = 0;
				return false;
			}
		}
	}

	if(numTargets > 1)
		return true;

	return false;
}

int16 Merc::GetFocusEffect(focusType type, uint16 spell_id) {

	int16 realTotal = 0;
	int16 realTotal2 = 0;
	int16 realTotal3 = 0;
	bool rand_effectiveness = false;

	//Improved Healing, Damage & Mana Reduction are handled differently in that some are random percentages
	//In these cases we need to find the most powerful effect, so that each piece of gear wont get its own chance
	if((type == focusManaCost || type == focusImprovedHeal || type == focusImprovedDamage)
		&& RuleB(Spells, LiveLikeFocusEffects))
	{
		rand_effectiveness = true;
	}

	//Check if item focus effect exists for the client.
	if (itembonuses.FocusEffects[type]){

		const EQEmu::ItemData* TempItem = nullptr;
		const EQEmu::ItemData* UsedItem = nullptr;
		uint16 UsedFocusID = 0;
		int16 Total = 0;
		int16 focus_max = 0;
		int16 focus_max_real = 0;

		//item focus
		for (int x = EQEmu::invslot::EQUIPMENT_BEGIN; x <= EQEmu::invslot::EQUIPMENT_END; ++x)
		{
			TempItem = nullptr;
			if (equipment[x] == 0)
				continue;
			TempItem = database.GetItem(equipment[x]);
			if (TempItem && TempItem->Focus.Effect > 0 && TempItem->Focus.Effect != SPELL_UNKNOWN) {
				if(rand_effectiveness) {
					focus_max = CalcFocusEffect(type, TempItem->Focus.Effect, spell_id, true);
					if (focus_max > 0 && focus_max_real >= 0 && focus_max > focus_max_real) {
						focus_max_real = focus_max;
						UsedItem = TempItem;
						UsedFocusID = TempItem->Focus.Effect;
					} else if (focus_max < 0 && focus_max < focus_max_real) {
						focus_max_real = focus_max;
						UsedItem = TempItem;
						UsedFocusID = TempItem->Focus.Effect;
					}
				}
				else {
					Total = CalcFocusEffect(type, TempItem->Focus.Effect, spell_id);
					if (Total > 0 && realTotal >= 0 && Total > realTotal) {
						realTotal = Total;
						UsedItem = TempItem;
						UsedFocusID = TempItem->Focus.Effect;
					} else if (Total < 0 && Total < realTotal) {
						realTotal = Total;
						UsedItem = TempItem;
						UsedFocusID = TempItem->Focus.Effect;
					}
				}
			}
		}

		if(UsedItem && rand_effectiveness && focus_max_real != 0)
			realTotal = CalcFocusEffect(type, UsedFocusID, spell_id);

		if (realTotal != 0 && UsedItem)
			Message_StringID(MT_Spells, BEGINS_TO_GLOW, UsedItem->Name);
	}

	//Check if spell focus effect exists for the client.
	if (spellbonuses.FocusEffects[type]){

		//Spell Focus
		int16 Total2 = 0;
		int16 focus_max2 = 0;
		int16 focus_max_real2 = 0;

		int buff_tracker = -1;
		int buff_slot = 0;
		uint16 focusspellid = 0;
		uint16 focusspell_tracker = 0;
		uint32 buff_max = GetMaxTotalSlots();
		for (buff_slot = 0; buff_slot < buff_max; buff_slot++) {
			focusspellid = buffs[buff_slot].spellid;
			if (focusspellid == 0 || focusspellid >= SPDAT_RECORDS)
				continue;

			if(rand_effectiveness) {
				focus_max2 = CalcFocusEffect(type, focusspellid, spell_id, true);
				if (focus_max2 > 0 && focus_max_real2 >= 0 && focus_max2 > focus_max_real2) {
					focus_max_real2 = focus_max2;
					buff_tracker = buff_slot;
					focusspell_tracker = focusspellid;
				} else if (focus_max2 < 0 && focus_max2 < focus_max_real2) {
					focus_max_real2 = focus_max2;
					buff_tracker = buff_slot;
					focusspell_tracker = focusspellid;
				}
			}
			else {
				Total2 = CalcFocusEffect(type, focusspellid, spell_id);
				if (Total2 > 0 && realTotal2 >= 0 && Total2 > realTotal2) {
					realTotal2 = Total2;
					buff_tracker = buff_slot;
					focusspell_tracker = focusspellid;
				} else if (Total2 < 0 && Total2 < realTotal2) {
					realTotal2 = Total2;
					buff_tracker = buff_slot;
					focusspell_tracker = focusspellid;
				}
			}
		}

		if(focusspell_tracker && rand_effectiveness && focus_max_real2 != 0)
			realTotal2 = CalcFocusEffect(type, focusspell_tracker, spell_id);

		// For effects like gift of mana that only fire once, save the spellid into an array that consists of all available buff slots.
		if(buff_tracker >= 0 && buffs[buff_tracker].numhits > 0) {
			m_spellHitsLeft[buff_tracker] = focusspell_tracker;
		}
	}


	// AA Focus
	/*if (aabonuses.FocusEffects[type]){

	int16 Total3 = 0;
	uint32 slots = 0;
	uint32 aa_AA = 0;
	uint32 aa_value = 0;

	for (int i = 0; i < MAX_PP_AA_ARRAY; i++)
	{
	aa_AA = this->aa[i]->AA;
	aa_value = this->aa[i]->value;
	if (aa_AA < 1 || aa_value < 1)
	continue;

	Total3 = CalcAAFocus(type, aa_AA, spell_id);
	if (Total3 > 0 && realTotal3 >= 0 && Total3 > realTotal3) {
	realTotal3 = Total3;
	}
	else if (Total3 < 0 && Total3 < realTotal3) {
	realTotal3 = Total3;
	}
	}
	}*/

	if(type == focusReagentCost && IsSummonPetSpell(spell_id) && GetAA(aaElementalPact))
		return 100;

	if(type == focusReagentCost && (IsEffectInSpell(spell_id, SE_SummonItem) || IsSacrificeSpell(spell_id)))
		return 0;
	//Summon Spells that require reagents are typically imbue type spells, enchant metal, sacrifice and shouldn't be affected
	//by reagent conservation for obvious reasons.

	return realTotal + realTotal2 + realTotal3;
}

int32 Merc::GetActSpellCost(uint16 spell_id, int32 cost)
{
	// Formula = Unknown exact, based off a random percent chance up to mana cost(after focuses) of the cast spell
	if(this->itembonuses.Clairvoyance && spells[spell_id].classes[(GetClass()%17) - 1] >= GetLevel() - 5)
	{
		int16 mana_back = this->itembonuses.Clairvoyance * zone->random.Int(1, 100) / 100;
		// Doesnt generate mana, so best case is a free spell
		if(mana_back > cost)
			mana_back = cost;

		cost -= mana_back;
	}

	// This formula was derived from the following resource:
	// http://www.eqsummoners.com/eq1/specialization-library.html
	// WildcardX
	float PercentManaReduction = 0;

	int16 focus_redux = GetFocusEffect(focusManaCost, spell_id);

	if(focus_redux > 0)
	{
		PercentManaReduction += zone->random.Real(1, (double)focus_redux);
	}

	cost -= (cost * (PercentManaReduction / 100));

	// Gift of Mana - reduces spell cost to 1 mana
	if(focus_redux >= 100) {
		uint32 buff_max = GetMaxTotalSlots();
		for (int buffSlot = 0; buffSlot < buff_max; buffSlot++) {
			if (buffs[buffSlot].spellid == 0 || buffs[buffSlot].spellid >= SPDAT_RECORDS)
				continue;

			if(IsEffectInSpell(buffs[buffSlot].spellid, SE_ReduceManaCost)) {
				if(CalcFocusEffect(focusManaCost, buffs[buffSlot].spellid, spell_id) == 100)
					cost = 1;
			}
		}
	}

	if(cost < 0)
		cost = 0;

	return cost;
}

int32 Merc::GetActSpellCasttime(uint16 spell_id, int32 casttime)
{
	int32 cast_reducer = 0;
	cast_reducer += GetFocusEffect(focusSpellHaste, spell_id);

	if (cast_reducer > RuleI(Spells, MaxCastTimeReduction))
		cast_reducer = RuleI(Spells, MaxCastTimeReduction);

	casttime = (casttime*(100 - cast_reducer)/100);

	return casttime;
}

int8 Merc::GetChanceToCastBySpellType(uint32 spellType) {
	int mercStance = (int)GetStance();
	int8 mercClass = GetClass();
	int8 chance = 0;

	switch (spellType) {
			case SpellType_Nuke: {
				switch(mercClass)
				{
				case TANK: {
					chance = 100;
					break;
						   }
				case HEALER:{
					break;
							}
				case MELEEDPS:{
					chance = 100;
					break;
							  }
				case CASTERDPS:{
					chance = 100;
					break;
							   }
				}
				break;
								 }
			case SpellType_Heal: {
				switch(mercClass)
				{
				case TANK: {
					break;
						   }
				case HEALER:{
					chance = 100;
					break;
							}
				case MELEEDPS:{
					break;
							  }
				case CASTERDPS:{
					break;
							   }
				}
				break;
								 }
			case SpellType_Root: {
				switch(mercClass)
				{
				case TANK: {
					break;
						   }
				case HEALER:{
					break;
							}
				case MELEEDPS:{
					break;
							  }
				case CASTERDPS:{
					break;
							   }
				}
				break;
								 }
			case SpellType_Buff: {
				switch(mercClass)
				{
				case TANK: {
					break;
						   }
				case HEALER:{
					chance = IsEngaged() ? 0 : 100;
					break;
							}
				case MELEEDPS:{
					break;
							  }
				case CASTERDPS:{
					break;
							   }
				}
				break;
								 }
			case SpellType_InCombatBuff: {
				switch(mercClass)
				{
				case TANK: {
					chance = 50;
					break;
						   }
				case HEALER:{
					break;
							}
				case MELEEDPS:{
					chance = 50;
					break;
							  }
				case CASTERDPS:{
					break;
							   }
				}
				break;
										 }
			case SpellType_Escape: {
				switch(mercClass)
				{
				case TANK: {
					break;
						   }
				case HEALER:{
					break;
							}
				case MELEEDPS:{
					chance = 100;
					break;
							  }
				case CASTERDPS:{
					chance = 100;
					break;
							   }
				}
				break;
								   }
			default:
				chance = 0;
				break;
	}

	return chance;
}

bool Merc::CheckStance(int16 stance) {

	//checks of current stance matches stances listed as valid for spell in database
	//stance = 0 for all stances, stance # for only that stance & -stance# for all but that stance
	if (stance == 0 || (stance > 0 && stance == GetStance()) || (stance < 0 && std::abs(stance) != GetStance())) {
			return true;
	}

	return false;
}

std::list<MercSpell> Merc::GetMercSpellsBySpellType(Merc* caster, uint32 spellType) {
	std::list<MercSpell> result;

	if(caster && caster->AI_HasSpells()) {
		std::vector<MercSpell> mercSpellList = caster->GetMercSpells();

		for (int i = mercSpellList.size() - 1; i >= 0; i--) {
			if (mercSpellList[i].spellid <= 0 || mercSpellList[i].spellid >= SPDAT_RECORDS) {
				// this is both to quit early to save cpu and to avoid casting bad spells
				// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
				continue;
			}

			if((mercSpellList[i].type & spellType) && caster->CheckStance(mercSpellList[i].stance)) {
				MercSpell mercSpell;
				mercSpell.spellid = mercSpellList[i].spellid;
				mercSpell.stance = mercSpellList[i].stance;
				mercSpell.type = mercSpellList[i].type;
				mercSpell.slot = mercSpellList[i].slot;
				mercSpell.proc_chance = mercSpellList[i].proc_chance;
				mercSpell.time_cancast = mercSpellList[i].time_cancast;

				result.push_back(mercSpell);
			}
		}
	}

	return result;
}

MercSpell Merc::GetFirstMercSpellBySpellType(Merc* caster, uint32 spellType) {
	MercSpell result;

	result.spellid = 0;
	result.stance = 0;
	result.type = 0;
	result.slot = 0;
	result.proc_chance = 0;
	result.time_cancast = 0;

	if(caster && caster->AI_HasSpells()) {
		std::vector<MercSpell> mercSpellList = caster->GetMercSpells();

		for (int i = mercSpellList.size() - 1; i >= 0; i--) {
			if (mercSpellList[i].spellid <= 0 || mercSpellList[i].spellid >= SPDAT_RECORDS) {
				// this is both to quit early to save cpu and to avoid casting bad spells
				// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
				continue;
			}

			if((mercSpellList[i].type & spellType)
				&& caster->CheckStance(mercSpellList[i].stance)
				&& CheckSpellRecastTimers(caster, mercSpellList[i].spellid)) {
					result.spellid = mercSpellList[i].spellid;
					result.stance = mercSpellList[i].stance;
					result.type = mercSpellList[i].type;
					result.slot = mercSpellList[i].slot;
					result.proc_chance = mercSpellList[i].proc_chance;
					result.time_cancast = mercSpellList[i].time_cancast;

					break;
			}
		}
	}

	return result;
}

MercSpell Merc::GetMercSpellBySpellID(Merc* caster, uint16 spellid) {
	MercSpell result;

	result.spellid = 0;
	result.stance = 0;
	result.type = 0;
	result.slot = 0;
	result.proc_chance = 0;
	result.time_cancast = 0;

	if(caster && caster->AI_HasSpells()) {
		std::vector<MercSpell> mercSpellList = caster->GetMercSpells();

		for (int i = mercSpellList.size() - 1; i >= 0; i--) {
			if (mercSpellList[i].spellid <= 0 || mercSpellList[i].spellid >= SPDAT_RECORDS) {
				// this is both to quit early to save cpu and to avoid casting bad spells
				// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
				continue;
			}

			if((mercSpellList[i].spellid == spellid)
				&& caster->CheckStance(mercSpellList[i].stance)) {
					result.spellid = mercSpellList[i].spellid;
					result.stance = mercSpellList[i].stance;
					result.type = mercSpellList[i].type;
					result.slot = mercSpellList[i].slot;
					result.proc_chance = mercSpellList[i].proc_chance;
					result.time_cancast = mercSpellList[i].time_cancast;

					break;
			}
		}
	}

	return result;
}

std::list<MercSpell> Merc::GetMercSpellsForSpellEffect(Merc* caster, int spellEffect) {
	std::list<MercSpell> result;

	if(caster && caster->AI_HasSpells()) {
		std::vector<MercSpell> mercSpellList = caster->GetMercSpells();

		for (int i = mercSpellList.size() - 1; i >= 0; i--) {
			if (mercSpellList[i].spellid <= 0 || mercSpellList[i].spellid >= SPDAT_RECORDS) {
				// this is both to quit early to save cpu and to avoid casting bad spells
				// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
				continue;
			}

			if(IsEffectInSpell(mercSpellList[i].spellid, spellEffect) && caster->CheckStance(mercSpellList[i].stance)) {
				MercSpell MercSpell;
				MercSpell.spellid = mercSpellList[i].spellid;
				MercSpell.stance = mercSpellList[i].stance;
				MercSpell.type = mercSpellList[i].type;
				MercSpell.slot = mercSpellList[i].slot;
				MercSpell.proc_chance = mercSpellList[i].proc_chance;
				MercSpell.time_cancast = mercSpellList[i].time_cancast;

				result.push_back(MercSpell);
			}
		}
	}

	return result;
}

std::list<MercSpell> Merc::GetMercSpellsForSpellEffectAndTargetType(Merc* caster, int spellEffect, SpellTargetType targetType) {
	std::list<MercSpell> result;

	if(caster && caster->AI_HasSpells()) {
		std::vector<MercSpell> mercSpellList = caster->GetMercSpells();

		for (int i = mercSpellList.size() - 1; i >= 0; i--) {
			if (mercSpellList[i].spellid <= 0 || mercSpellList[i].spellid >= SPDAT_RECORDS) {
				// this is both to quit early to save cpu and to avoid casting bad spells
				// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
				continue;
			}

			if(IsEffectInSpell(mercSpellList[i].spellid, spellEffect) && caster->CheckStance(mercSpellList[i].stance)) {
				if(spells[mercSpellList[i].spellid].targettype == targetType) {
					MercSpell MercSpell;
					MercSpell.spellid = mercSpellList[i].spellid;
					MercSpell.stance = mercSpellList[i].stance;
					MercSpell.type = mercSpellList[i].type;
					MercSpell.slot = mercSpellList[i].slot;
					MercSpell.proc_chance = mercSpellList[i].proc_chance;
					MercSpell.time_cancast = mercSpellList[i].time_cancast;

					result.push_back(MercSpell);
				}
			}
		}
	}

	return result;
}

MercSpell Merc::GetBestMercSpellForVeryFastHeal(Merc* caster) {
	MercSpell result;

	result.spellid = 0;
	result.stance = 0;
	result.type = 0;
	result.slot = 0;
	result.proc_chance = 0;
	result.time_cancast = 0;

	if(caster) {
		std::list<MercSpell> mercSpellList = GetMercSpellsForSpellEffect(caster, SE_CurrentHP);

		for (auto mercSpellListItr = mercSpellList.begin(); mercSpellListItr != mercSpellList.end();
		     ++mercSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsVeryFastHealSpell(mercSpellListItr->spellid)
				&& CheckSpellRecastTimers(caster, mercSpellListItr->spellid)) {
					result.spellid = mercSpellListItr->spellid;
					result.stance = mercSpellListItr->stance;
					result.type = mercSpellListItr->type;
					result.slot = mercSpellListItr->slot;
					result.proc_chance = mercSpellListItr->proc_chance;
					result.time_cancast = mercSpellListItr->time_cancast;

					break;
			}
		}
	}

	return result;
}

MercSpell Merc::GetBestMercSpellForFastHeal(Merc* caster) {
	MercSpell result;

	result.spellid = 0;
	result.stance = 0;
	result.type = 0;
	result.slot = 0;
	result.proc_chance = 0;
	result.time_cancast = 0;

	if(caster) {
		std::list<MercSpell> mercSpellList = GetMercSpellsForSpellEffect(caster, SE_CurrentHP);

		for (auto mercSpellListItr = mercSpellList.begin(); mercSpellListItr != mercSpellList.end();
		     ++mercSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsFastHealSpell(mercSpellListItr->spellid)
				&& CheckSpellRecastTimers(caster, mercSpellListItr->spellid)) {
					result.spellid = mercSpellListItr->spellid;
					result.stance = mercSpellListItr->stance;
					result.type = mercSpellListItr->type;
					result.slot = mercSpellListItr->slot;
					result.proc_chance = mercSpellListItr->proc_chance;
					result.time_cancast = mercSpellListItr->time_cancast;

					break;
			}
		}
	}

	return result;
}

MercSpell Merc::GetBestMercSpellForHealOverTime(Merc* caster) {
	MercSpell result;

	result.spellid = 0;
	result.stance = 0;
	result.type = 0;
	result.slot = 0;
	result.proc_chance = 0;
	result.time_cancast = 0;

	if(caster) {
		std::list<MercSpell> mercHoTSpellList = GetMercSpellsForSpellEffect(caster, SE_HealOverTime);

		for (auto mercSpellListItr = mercHoTSpellList.begin(); mercSpellListItr != mercHoTSpellList.end();
		     ++mercSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsHealOverTimeSpell(mercSpellListItr->spellid)) {

				if (mercSpellListItr->spellid <= 0 || mercSpellListItr->spellid >= SPDAT_RECORDS) {
					// this is both to quit early to save cpu and to avoid casting bad spells
					// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
					continue;
				}

				if(CheckSpellRecastTimers(caster, mercSpellListItr->spellid)) {
					result.spellid = mercSpellListItr->spellid;
					result.stance = mercSpellListItr->stance;
					result.type = mercSpellListItr->type;
					result.slot = mercSpellListItr->slot;
					result.proc_chance = mercSpellListItr->proc_chance;
					result.time_cancast = mercSpellListItr->time_cancast;
				}

				break;
			}
		}
	}

	return result;
}

MercSpell Merc::GetBestMercSpellForPercentageHeal(Merc* caster) {
	MercSpell result;

	result.spellid = 0;
	result.stance = 0;
	result.type = 0;
	result.slot = 0;
	result.proc_chance = 0;
	result.time_cancast = 0;

	if(caster && caster->AI_HasSpells()) {
		std::list<MercSpell> mercSpellList = GetMercSpellsForSpellEffect(caster, SE_CurrentHP);

		for (auto mercSpellListItr = mercSpellList.begin(); mercSpellListItr != mercSpellList.end();
		     ++mercSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsCompleteHealSpell(mercSpellListItr->spellid)
				&& CheckSpellRecastTimers(caster, mercSpellListItr->spellid)) {
					result.spellid = mercSpellListItr->spellid;
					result.stance = mercSpellListItr->stance;
					result.type = mercSpellListItr->type;
					result.slot = mercSpellListItr->slot;
					result.proc_chance = mercSpellListItr->proc_chance;
					result.time_cancast = mercSpellListItr->time_cancast;

					break;
			}
		}
	}

	return result;
}

MercSpell Merc::GetBestMercSpellForRegularSingleTargetHeal(Merc* caster) {
	MercSpell result;

	result.spellid = 0;
	result.stance = 0;
	result.type = 0;
	result.slot = 0;
	result.proc_chance = 0;
	result.time_cancast = 0;

	if(caster) {
		std::list<MercSpell> mercSpellList = GetMercSpellsForSpellEffect(caster, SE_CurrentHP);

		for (auto mercSpellListItr = mercSpellList.begin(); mercSpellListItr != mercSpellList.end();
		     ++mercSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsRegularSingleTargetHealSpell(mercSpellListItr->spellid)
				&& CheckSpellRecastTimers(caster, mercSpellListItr->spellid)) {
					result.spellid = mercSpellListItr->spellid;
					result.stance = mercSpellListItr->stance;
					result.type = mercSpellListItr->type;
					result.slot = mercSpellListItr->slot;
					result.proc_chance = mercSpellListItr->proc_chance;
					result.time_cancast = mercSpellListItr->time_cancast;

					break;
			}
		}
	}

	return result;
}

MercSpell Merc::GetFirstMercSpellForSingleTargetHeal(Merc* caster) {
	MercSpell result;

	result.spellid = 0;
	result.stance = 0;
	result.type = 0;
	result.slot = 0;
	result.proc_chance = 0;
	result.time_cancast = 0;

	if(caster) {
		std::list<MercSpell> mercSpellList = GetMercSpellsForSpellEffect(caster, SE_CurrentHP);

		for (auto mercSpellListItr = mercSpellList.begin(); mercSpellListItr != mercSpellList.end();
		     ++mercSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if((IsRegularSingleTargetHealSpell(mercSpellListItr->spellid)
				|| IsFastHealSpell(mercSpellListItr->spellid))
				&& CheckSpellRecastTimers(caster, mercSpellListItr->spellid)) {
					result.spellid = mercSpellListItr->spellid;
					result.stance = mercSpellListItr->stance;
					result.type = mercSpellListItr->type;
					result.slot = mercSpellListItr->slot;
					result.proc_chance = mercSpellListItr->proc_chance;
					result.time_cancast = mercSpellListItr->time_cancast;

					break;
			}
		}
	}

	return result;
}

MercSpell Merc::GetBestMercSpellForGroupHeal(Merc* caster) {
	MercSpell result;

	result.spellid = 0;
	result.stance = 0;
	result.type = 0;
	result.slot = 0;
	result.proc_chance = 0;
	result.time_cancast = 0;

	if(caster) {
		std::list<MercSpell> mercSpellList = GetMercSpellsForSpellEffect(caster, SE_CurrentHP);

		for (auto mercSpellListItr = mercSpellList.begin(); mercSpellListItr != mercSpellList.end();
		     ++mercSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsRegularGroupHealSpell(mercSpellListItr->spellid)
				&& CheckSpellRecastTimers(caster, mercSpellListItr->spellid)) {
					result.spellid = mercSpellListItr->spellid;
					result.stance = mercSpellListItr->stance;
					result.type = mercSpellListItr->type;
					result.slot = mercSpellListItr->slot;
					result.proc_chance = mercSpellListItr->proc_chance;
					result.time_cancast = mercSpellListItr->time_cancast;

					break;
			}
		}
	}

	return result;
}

MercSpell Merc::GetBestMercSpellForGroupHealOverTime(Merc* caster) {
	MercSpell result;

	result.spellid = 0;
	result.stance = 0;
	result.type = 0;
	result.slot = 0;
	result.proc_chance = 0;
	result.time_cancast = 0;

	if(caster) {
		std::list<MercSpell> mercHoTSpellList = GetMercSpellsForSpellEffect(caster, SE_HealOverTime);

		for (auto mercSpellListItr = mercHoTSpellList.begin(); mercSpellListItr != mercHoTSpellList.end();
		     ++mercSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsGroupHealOverTimeSpell(mercSpellListItr->spellid)) {

				if (mercSpellListItr->spellid <= 0 || mercSpellListItr->spellid >= SPDAT_RECORDS) {
					// this is both to quit early to save cpu and to avoid casting bad spells
					// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
					continue;
				}

				if(CheckSpellRecastTimers(caster, mercSpellListItr->spellid)) {
					result.spellid = mercSpellListItr->spellid;
					result.stance = mercSpellListItr->stance;
					result.type = mercSpellListItr->type;
					result.slot = mercSpellListItr->slot;
					result.proc_chance = mercSpellListItr->proc_chance;
					result.time_cancast = mercSpellListItr->time_cancast;
				}

				break;
			}
		}
	}

	return result;
}

MercSpell Merc::GetBestMercSpellForGroupCompleteHeal(Merc* caster) {
	MercSpell result;

	result.spellid = 0;
	result.stance = 0;
	result.type = 0;
	result.slot = 0;
	result.proc_chance = 0;
	result.time_cancast = 0;

	if(caster) {
		std::list<MercSpell> mercSpellList = GetMercSpellsForSpellEffect(caster, SE_CompleteHeal);

		for (auto mercSpellListItr = mercSpellList.begin(); mercSpellListItr != mercSpellList.end();
		     ++mercSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsGroupCompleteHealSpell(mercSpellListItr->spellid)
				&& CheckSpellRecastTimers(caster, mercSpellListItr->spellid)) {
					result.spellid = mercSpellListItr->spellid;
					result.stance = mercSpellListItr->stance;
					result.type = mercSpellListItr->type;
					result.slot = mercSpellListItr->slot;
					result.proc_chance = mercSpellListItr->proc_chance;
					result.time_cancast = mercSpellListItr->time_cancast;

					break;
			}
		}
	}

	return result;
}

MercSpell Merc::GetBestMercSpellForAETaunt(Merc* caster) {
	MercSpell result;

	result.spellid = 0;
	result.stance = 0;
	result.type = 0;
	result.slot = 0;
	result.proc_chance = 0;
	result.time_cancast = 0;

	if(caster) {
		std::list<MercSpell> mercSpellList = GetMercSpellsForSpellEffect(caster, SE_Taunt);

		for (auto mercSpellListItr = mercSpellList.begin(); mercSpellListItr != mercSpellList.end();
		     ++mercSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if((spells[mercSpellListItr->spellid].targettype == ST_AECaster
				|| spells[mercSpellListItr->spellid].targettype == ST_AETarget
				|| spells[mercSpellListItr->spellid].targettype == ST_UndeadAE)
				&& CheckSpellRecastTimers(caster, mercSpellListItr->spellid)) {
					result.spellid = mercSpellListItr->spellid;
					result.stance = mercSpellListItr->stance;
					result.type = mercSpellListItr->type;
					result.slot = mercSpellListItr->slot;
					result.proc_chance = mercSpellListItr->proc_chance;
					result.time_cancast = mercSpellListItr->time_cancast;

					break;
			}
		}
	}

	return result;
}

MercSpell Merc::GetBestMercSpellForTaunt(Merc* caster) {
	MercSpell result;

	result.spellid = 0;
	result.stance = 0;
	result.type = 0;
	result.slot = 0;
	result.proc_chance = 0;
	result.time_cancast = 0;

	if(caster) {
		std::list<MercSpell> mercSpellList = GetMercSpellsForSpellEffect(caster, SE_Taunt);

		for (auto mercSpellListItr = mercSpellList.begin(); mercSpellListItr != mercSpellList.end();
		     ++mercSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if((spells[mercSpellListItr->spellid].targettype == ST_Target)
				&& CheckSpellRecastTimers(caster, mercSpellListItr->spellid)) {
					result.spellid = mercSpellListItr->spellid;
					result.stance = mercSpellListItr->stance;
					result.type = mercSpellListItr->type;
					result.slot = mercSpellListItr->slot;
					result.proc_chance = mercSpellListItr->proc_chance;
					result.time_cancast = mercSpellListItr->time_cancast;

					break;
			}
		}
	}

	return result;
}

MercSpell Merc::GetBestMercSpellForHate(Merc* caster) {
	MercSpell result;

	result.spellid = 0;
	result.stance = 0;
	result.type = 0;
	result.slot = 0;
	result.proc_chance = 0;
	result.time_cancast = 0;

	if(caster) {
		std::list<MercSpell> mercSpellList = GetMercSpellsForSpellEffect(caster, SE_InstantHate);

		for (auto mercSpellListItr = mercSpellList.begin(); mercSpellListItr != mercSpellList.end();
		     ++mercSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(CheckSpellRecastTimers(caster, mercSpellListItr->spellid)) {
				result.spellid = mercSpellListItr->spellid;
				result.stance = mercSpellListItr->stance;
				result.type = mercSpellListItr->type;
				result.slot = mercSpellListItr->slot;
				result.proc_chance = mercSpellListItr->proc_chance;
				result.time_cancast = mercSpellListItr->time_cancast;

				break;
			}
		}
	}

	return result;
}

MercSpell Merc::GetBestMercSpellForCure(Merc* caster, Mob *tar) {
	MercSpell result;
	bool spellSelected = false;

	result.spellid = 0;
	result.stance = 0;
	result.type = 0;
	result.slot = 0;
	result.proc_chance = 0;
	result.time_cancast = 0;

	if(!tar)
		return result;

	int countNeedsCured = 0;
	bool isPoisoned = tar->FindType(SE_PoisonCounter);
	bool isDiseased = tar->FindType(SE_DiseaseCounter);
	bool isCursed = tar->FindType(SE_CurseCounter);
	bool isCorrupted = tar->FindType(SE_CorruptionCounter);

	if(caster && caster->AI_HasSpells()) {
		std::list<MercSpell> cureList = GetMercSpellsBySpellType(caster, SpellType_Cure);

		if(tar->HasGroup()) {
			Group *g = tar->GetGroup();

			if(g) {
				for( int i = 0; i<MAX_GROUP_MEMBERS; i++) {
					if(g->members[i] && !g->members[i]->qglobal) {
						if(caster->GetNeedsCured(g->members[i]))
							countNeedsCured++;
					}
				}
			}
		}

		//Check for group cure first
		if(countNeedsCured > 2) {
			for (auto itr = cureList.begin(); itr != cureList.end(); ++itr) {
				MercSpell selectedMercSpell = *itr;

				if(IsGroupSpell(itr->spellid) && CheckSpellRecastTimers(caster, itr->spellid)) {
					if(selectedMercSpell.spellid == 0)
						continue;

					if(isPoisoned && IsEffectInSpell(itr->spellid, SE_PoisonCounter)) {
						spellSelected = true;
					}
					else if(isDiseased && IsEffectInSpell(itr->spellid, SE_DiseaseCounter)) {
						spellSelected = true;
					}
					else if(isCursed && IsEffectInSpell(itr->spellid, SE_CurseCounter)) {
						spellSelected = true;
					}
					else if(isCorrupted && IsEffectInSpell(itr->spellid, SE_CorruptionCounter)) {
						spellSelected = true;
					}
					else if(IsEffectInSpell(itr->spellid, SE_DispelDetrimental)) {
						spellSelected = true;
					}

					if(spellSelected)
					{
						result.spellid = itr->spellid;
						result.stance = itr->stance;
						result.type = itr->type;
						result.slot = itr->slot;
						result.proc_chance = itr->proc_chance;
						result.time_cancast = itr->time_cancast;

						break;
					}
				}
			}
		}

		//no group cure for target- try to find single target spell
		if(!spellSelected) {
			for (auto itr = cureList.begin(); itr != cureList.end(); ++itr) {
				MercSpell selectedMercSpell = *itr;

				if(CheckSpellRecastTimers(caster, itr->spellid)) {
					if(selectedMercSpell.spellid == 0)
						continue;

					if(isPoisoned && IsEffectInSpell(itr->spellid, SE_PoisonCounter)) {
						spellSelected = true;
					}
					else if(isDiseased && IsEffectInSpell(itr->spellid, SE_DiseaseCounter)) {
						spellSelected = true;
					}
					else if(isCursed && IsEffectInSpell(itr->spellid, SE_CurseCounter)) {
						spellSelected = true;
					}
					else if(isCorrupted && IsEffectInSpell(itr->spellid, SE_CorruptionCounter)) {
						spellSelected = true;
					}
					else if(IsEffectInSpell(itr->spellid, SE_DispelDetrimental)) {
						spellSelected = true;
					}

					if(spellSelected)
					{
						result.spellid = itr->spellid;
						result.stance = itr->stance;
						result.type = itr->type;
						result.slot = itr->slot;
						result.proc_chance = itr->proc_chance;
						result.time_cancast = itr->time_cancast;

						break;
					}
				}
			}
		}
	}

	return result;
}

MercSpell Merc::GetBestMercSpellForStun(Merc* caster) {
	MercSpell result;
	bool spellSelected = false;

	result.spellid = 0;
	result.stance = 0;
	result.type = 0;
	result.slot = 0;
	result.proc_chance = 0;
	result.time_cancast = 0;

	if(caster) {
		std::list<MercSpell> mercSpellList = GetMercSpellsForSpellEffect(caster, SE_Stun);

		for (auto mercSpellListItr = mercSpellList.begin(); mercSpellListItr != mercSpellList.end();
		     ++mercSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(CheckSpellRecastTimers(caster, mercSpellListItr->spellid)) {
				result.spellid = mercSpellListItr->spellid;
				result.stance = mercSpellListItr->stance;
				result.type = mercSpellListItr->type;
				result.slot = mercSpellListItr->slot;
				result.proc_chance = mercSpellListItr->proc_chance;
				result.time_cancast = mercSpellListItr->time_cancast;

				break;
			}
		}
	}

	return result;
}

MercSpell Merc::GetBestMercSpellForAENuke(Merc* caster, Mob* tar) {
	MercSpell result;
	bool spellSelected = false;

	result.spellid = 0;
	result.stance = 0;
	result.type = 0;
	result.slot = 0;
	result.proc_chance = 0;
	result.time_cancast = 0;

	if(caster) {
		uint8 initialCastChance = 0;
		uint8 castChanceFalloff = 75;

		switch(caster->GetStance())
		{
		case MercStanceBurnAE:
			initialCastChance = 50;
			break;
		case MercStanceBalanced:
			initialCastChance = 25;
			break;
		case MercStanceBurn:
			initialCastChance = 0;
			break;
		}

		//check of we even want to cast an AE nuke
		if(zone->random.Roll(initialCastChance)) {

			result = GetBestMercSpellForAERainNuke(caster, tar);

			//check if we have a spell & allow for other AE nuke types
			if(result.spellid == 0 && zone->random.Roll(castChanceFalloff)) {

				result = GetBestMercSpellForPBAENuke(caster, tar);

				//check if we have a spell & allow for other AE nuke types
				if(result.spellid == 0 && zone->random.Roll(castChanceFalloff)) {

					result = GetBestMercSpellForTargetedAENuke(caster, tar);
				}
			}
		}
	}

	return result;
}

MercSpell Merc::GetBestMercSpellForTargetedAENuke(Merc* caster, Mob* tar) {
	MercSpell result;
	int castChance = 50;            //used to cycle through multiple spells (first has 50% overall chance, 2nd has 25%, etc.)
	int numTargetsCheck = 1;        //used to check for min number of targets to use AE
	bool spellSelected = false;

	result.spellid = 0;
	result.stance = 0;
	result.type = 0;
	result.slot = 0;
	result.proc_chance = 0;
	result.time_cancast = 0;

	switch(caster->GetStance())
	{
	case MercStanceBurnAE:
		numTargetsCheck = 1;
		break;
	case MercStanceBalanced:
	case MercStanceBurn:
		numTargetsCheck = 2;
		break;
	}

	if(caster) {
		std::list<MercSpell> mercSpellList = GetMercSpellsBySpellType(caster, SpellType_Nuke);

		for (auto mercSpellListItr = mercSpellList.begin(); mercSpellListItr != mercSpellList.end();
		     ++mercSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsAENukeSpell(mercSpellListItr->spellid) && !IsAERainNukeSpell(mercSpellListItr->spellid)
				&& !IsPBAENukeSpell(mercSpellListItr->spellid) && CheckSpellRecastTimers(caster, mercSpellListItr->spellid)) {
					uint8 numTargets = 0;
					if(CheckAENuke(caster, tar, mercSpellListItr->spellid, numTargets)) {
						if(numTargets >= numTargetsCheck && zone->random.Roll(castChance)) {
							result.spellid = mercSpellListItr->spellid;
							result.stance = mercSpellListItr->stance;
							result.type = mercSpellListItr->type;
							result.slot = mercSpellListItr->slot;
							result.proc_chance = mercSpellListItr->proc_chance;
							result.time_cancast = mercSpellListItr->time_cancast;
						}
					}

					break;
			}
		}
	}

	return result;
}

MercSpell Merc::GetBestMercSpellForPBAENuke(Merc* caster, Mob* tar) {
	MercSpell result;
	int castChance = 50;            //used to cycle through multiple spells (first has 50% overall chance, 2nd has 25%, etc.)
	int numTargetsCheck = 1;        //used to check for min number of targets to use AE
	bool spellSelected = false;

	result.spellid = 0;
	result.stance = 0;
	result.type = 0;
	result.slot = 0;
	result.proc_chance = 0;
	result.time_cancast = 0;

	switch(caster->GetStance())
	{
	case MercStanceBurnAE:
		numTargetsCheck = 2;
		break;
	case MercStanceBalanced:
	case MercStanceBurn:
		numTargetsCheck = 3;
		break;
	}

	if(caster) {
		std::list<MercSpell> mercSpellList = GetMercSpellsBySpellType(caster, SpellType_Nuke);

		for (auto mercSpellListItr = mercSpellList.begin(); mercSpellListItr != mercSpellList.end();
		     ++mercSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsPBAENukeSpell(mercSpellListItr->spellid) && CheckSpellRecastTimers(caster, mercSpellListItr->spellid)) {
				uint8 numTargets = 0;
				if(CheckAENuke(caster, caster, mercSpellListItr->spellid, numTargets)) {
					if(numTargets >= numTargetsCheck && zone->random.Roll(castChance)) {
						result.spellid = mercSpellListItr->spellid;
						result.stance = mercSpellListItr->stance;
						result.type = mercSpellListItr->type;
						result.slot = mercSpellListItr->slot;
						result.proc_chance = mercSpellListItr->proc_chance;
						result.time_cancast = mercSpellListItr->time_cancast;
					}
				}

				break;
			}
		}
	}

	return result;
}

MercSpell Merc::GetBestMercSpellForAERainNuke(Merc* caster, Mob* tar) {
	MercSpell result;
	int castChance = 50;            //used to cycle through multiple spells (first has 50% overall chance, 2nd has 25%, etc.)
	int numTargetsCheck = 1;        //used to check for min number of targets to use AE
	bool spellSelected = false;

	result.spellid = 0;
	result.stance = 0;
	result.type = 0;
	result.slot = 0;
	result.proc_chance = 0;
	result.time_cancast = 0;

	switch(caster->GetStance())
	{
	case MercStanceBurnAE:
		numTargetsCheck = 1;
		break;
	case MercStanceBalanced:
	case MercStanceBurn:
		numTargetsCheck = 2;
		break;
	}

	if(caster) {
		std::list<MercSpell> mercSpellList = GetMercSpellsBySpellType(caster, SpellType_Nuke);

		for (auto mercSpellListItr = mercSpellList.begin(); mercSpellListItr != mercSpellList.end();
		     ++mercSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsAERainNukeSpell(mercSpellListItr->spellid) && zone->random.Roll(castChance) && CheckSpellRecastTimers(caster, mercSpellListItr->spellid)) {
				uint8 numTargets = 0;
				if(CheckAENuke(caster, tar, mercSpellListItr->spellid, numTargets)) {
					if(numTargets >= numTargetsCheck) {
						result.spellid = mercSpellListItr->spellid;
						result.stance = mercSpellListItr->stance;
						result.type = mercSpellListItr->type;
						result.slot = mercSpellListItr->slot;
						result.proc_chance = mercSpellListItr->proc_chance;
						result.time_cancast = mercSpellListItr->time_cancast;
					}
				}

				break;
			}
		}
	}

	return result;
}

MercSpell Merc::GetBestMercSpellForNuke(Merc* caster) {
	MercSpell result;
	int castChance = 50;    //used to cycle through multiple spells (first has 50% overall chance, 2nd has 25%, etc.)
	bool spellSelected = false;

	result.spellid = 0;
	result.stance = 0;
	result.type = 0;
	result.slot = 0;
	result.proc_chance = 0;
	result.time_cancast = 0;

	if(caster) {
		std::list<MercSpell> mercSpellList = GetMercSpellsBySpellType(caster, SpellType_Nuke);

		for (auto mercSpellListItr = mercSpellList.begin(); mercSpellListItr != mercSpellList.end();
		     ++mercSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsPureNukeSpell(mercSpellListItr->spellid) && !IsAENukeSpell(mercSpellListItr->spellid)
				&& zone->random.Roll(castChance) && CheckSpellRecastTimers(caster, mercSpellListItr->spellid)) {
					result.spellid = mercSpellListItr->spellid;
					result.stance = mercSpellListItr->stance;
					result.type = mercSpellListItr->type;
					result.slot = mercSpellListItr->slot;
					result.proc_chance = mercSpellListItr->proc_chance;
					result.time_cancast = mercSpellListItr->time_cancast;

					break;
			}
		}
	}

	return result;
}

MercSpell Merc::GetBestMercSpellForNukeByTargetResists(Merc* caster, Mob* target) {
	MercSpell result;
	bool spellSelected = false;

	result.spellid = 0;
	result.stance = 0;
	result.type = 0;
	result.slot = 0;
	result.proc_chance = 0;
	result.time_cancast = 0;

	if(!target)
		return result;

	if(caster) {
		const int lureResisValue = -100;
		const int maxTargetResistValue = 300;
		bool selectLureNuke = false;

		if((target->GetMR() > maxTargetResistValue) && (target->GetCR() > maxTargetResistValue) && (target->GetFR() > maxTargetResistValue))
			selectLureNuke = true;

		std::list<MercSpell> mercSpellList = GetMercSpellsBySpellType(caster, SpellType_Nuke);

		for (auto mercSpellListItr = mercSpellList.begin(); mercSpellListItr != mercSpellList.end();
		     ++mercSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order

			if(IsPureNukeSpell(mercSpellListItr->spellid) && !IsAENukeSpell(mercSpellListItr->spellid) && CheckSpellRecastTimers(caster, mercSpellListItr->spellid)) {
				if(selectLureNuke && (spells[mercSpellListItr->spellid].ResistDiff < lureResisValue)) {
					spellSelected = true;
				}
				else {
					if(((target->GetMR() < target->GetCR()) || (target->GetMR() < target->GetFR())) && (GetSpellResistType(mercSpellListItr->spellid) == RESIST_MAGIC)
						&& (spells[mercSpellListItr->spellid].ResistDiff > lureResisValue))
					{
						spellSelected = true;
					}
					else if(((target->GetCR() < target->GetMR()) || (target->GetCR() < target->GetFR())) && (GetSpellResistType(mercSpellListItr->spellid) == RESIST_COLD)
						&& (spells[mercSpellListItr->spellid].ResistDiff > lureResisValue))
					{
						spellSelected = true;
					}
					else if(((target->GetFR() < target->GetCR()) || (target->GetFR() < target->GetMR())) && (GetSpellResistType(mercSpellListItr->spellid) == RESIST_FIRE)
						&& (spells[mercSpellListItr->spellid].ResistDiff > lureResisValue))
					{
						spellSelected = true;
					}
				}
			}

			if(spellSelected) {
				result.spellid = mercSpellListItr->spellid;
				result.stance = mercSpellListItr->stance;
				result.type = mercSpellListItr->type;
				result.slot = mercSpellListItr->slot;
				result.proc_chance = mercSpellListItr->proc_chance;
				result.time_cancast = mercSpellListItr->time_cancast;

				break;
			}
		}
	}

	return result;
}

bool Merc::GetNeedsCured(Mob *tar) {
	bool needCured = false;

	if(tar) {
		if(tar->FindType(SE_PoisonCounter) || tar->FindType(SE_DiseaseCounter) || tar->FindType(SE_CurseCounter) || tar->FindType(SE_CorruptionCounter)) {
			uint32 buff_count = tar->GetMaxTotalSlots();
			int buffsWithCounters = 0;
			needCured = true;

			for (unsigned int j = 0; j < buff_count; j++) {
				if(tar->GetBuffs()[j].spellid != SPELL_UNKNOWN) {
					if(CalculateCounters(tar->GetBuffs()[j].spellid) > 0) {
						buffsWithCounters++;

						if(buffsWithCounters == 1 && (tar->GetBuffs()[j].ticsremaining < 2 || (int32)((tar->GetBuffs()[j].ticsremaining * 6) / tar->GetBuffs()[j].counters) < 2)) {
							// Spell has ticks remaining but may have too many counters to cure in the time remaining;
							// We should try to just wait it out. Could spend entire time trying to cure spell instead of healing, buffing, etc.
							// Since this is the first buff with counters, don't try to cure. Cure spell will be wasted, as cure will try to
							// remove counters from the first buff that has counters remaining.
							needCured = false;
							break;
						}
					}
				}
			}
		}
	}

	return needCured;
}

void Merc::MercGroupSay(Mob *speaker, const char *msg, ...)
{

	char buf[1000];
	va_list ap;

	va_start(ap, msg);
	vsnprintf(buf, 1000, msg, ap);
	va_end(ap);

	if(speaker->HasGroup()) {
		Group *g = speaker->GetGroup();

		if(g)
			g->GroupMessage(speaker->CastToMob(), 0, 100, buf);
	}
}

bool Merc::UseDiscipline(int32 spell_id, int32 target) {
	// Dont let client waste a reuse timer if they can't use the disc
	if (IsStunned() || IsFeared() || IsMezzed() || IsAmnesiad())
	{
		return(false);
	}

	//make sure we can use it..
	if(!IsValidSpell(spell_id)) {
		return(false);
	}

	const SPDat_Spell_Struct &spell = spells[spell_id];

	if(spell.recast_time > 0)
	{
		if(CheckDisciplineRecastTimers(this, spell_id, spells[spell_id].EndurTimerIndex)) {
			if(spells[spell_id].EndurTimerIndex > 0) {
				SetDisciplineRecastTimer(spells[spell_id].EndurTimerIndex, spell_id, spell.recast_time);
			}

			SetSpellTimeCanCast(spell_id, spells[spell_id].recast_time);
		}
		else {
			return(false);
		}
	}

	if(GetEndurance() > spell.EndurCost) {
		SetEndurance(GetEndurance() - spell.EndurCost);
	} else {
		//too fatigued to use this skill right now.
		return(false);
	}

	if(IsCasting())
		InterruptSpell();

	CastSpell(spell_id, target, EQEmu::CastingSlot::Discipline);

	return(true);
}

void Merc::SetSpellRecastTimer(uint16 timer_id, uint16 spellid, uint32 recast_delay) {
	if(timer_id > 0) {
		MercTimer timer;
		timer.timerid = timer_id;
		timer.timertype = 1;
		timer.spellid = spellid;
		timer.time_cancast = Timer::GetCurrentTime() + recast_delay;
		timers[timer_id] = timer;
	}
}

int32 Merc::GetSpellRecastTimer(Merc *caster, uint16 timer_id) {
	int32 result = 0;
	if(caster && timer_id > 0) {
		if(caster->timers.find(timer_id) != caster->timers.end()) {
			result = caster->timers[timer_id].time_cancast;
		}
	}
	return result;
}

bool Merc::CheckSpellRecastTimers(Merc *caster, uint16 spell_id) {
	if(caster) {
		MercSpell mercSpell = GetMercSpellBySpellID(caster, spell_id);
		if(mercSpell.spellid > 0 && mercSpell.time_cancast < Timer::GetCurrentTime()) { //checks spell recast
			if(GetSpellRecastTimer(caster, spells[spell_id].EndurTimerIndex) < Timer::GetCurrentTime()) { //checks for spells on the same timer
				return true; //can cast spell
			}
		}
	}
	return false;
}

void Merc::SetDisciplineRecastTimer(uint16 timer_id, uint16 spellid, uint32 recast_delay) {
	if(timer_id > 0) {
		MercTimer timer;
		timer.timerid = timer_id;
		timer.timertype = 2;
		timer.spellid = spellid;
		timer.time_cancast = Timer::GetCurrentTime() + recast_delay;
		timers[timer_id] = timer;
	}
}

int32 Merc::GetDisciplineRecastTimer(Merc *caster, uint16 timer_id) {
	int32 result = 0;
	if(caster && timer_id > 0) {
		if(caster->timers.find(timer_id) != caster->timers.end()) {
			result = caster->timers[timer_id].time_cancast;
		}
	}
	return result;
}

int32 Merc::GetDisciplineRemainingTime(Merc *caster, uint16 timer_id) {
	int32 result = 0;
	if(caster && timer_id > 0) {
		int32 time_cancast = GetDisciplineRecastTimer(caster, timer_id);
		if(time_cancast > Timer::GetCurrentTime())
			result = time_cancast - Timer::GetCurrentTime();
	}
	return result;
}

bool Merc::CheckDisciplineRecastTimers(Merc *caster, uint16 spell_id, uint16 timer_id) {
	if(caster) {
		MercSpell mercSpell = GetMercSpellBySpellID(caster, spell_id);
		if(mercSpell.spellid > 0 && mercSpell.time_cancast < Timer::GetCurrentTime()) { //checks spell recast
			if(timer_id > 0 && !(GetDisciplineRecastTimer(caster, timer_id) < Timer::GetCurrentTime())) { //checks for spells on the same timer
				return false; //can't cast spell
			}
			return true;
		}
	}
	return false;
}

void Merc::SetSpellTimeCanCast(uint16 spellid, uint32 recast_delay) {
	for (int i = 0; i < merc_spells.size(); i++) {
		if(merc_spells[i].spellid == spellid) {
			merc_spells[i].time_cancast = Timer::GetCurrentTime() + recast_delay;
		}
	}
}

bool Merc::CheckTaunt() {
	Mob* tar = GetTarget();
	//Only taunt if we are not top on target's hate list
	//This ensures we have taunt available to regain aggro if needed
	if(tar && tar->GetHateTop() && tar->GetHateTop() != this) {
		return true;
	}
	return false;
}

bool Merc::CheckAETaunt() {
	//need to check area for mobs needing taunted
	MercSpell mercSpell = GetBestMercSpellForAETaunt(this);
	uint8 result = 0;

	if(mercSpell.spellid != 0) {

		std::list<NPC*> npc_list;
		entity_list.GetNPCList(npc_list);

		for (auto itr = npc_list.begin(); itr != npc_list.end(); ++itr) {
			NPC* npc = *itr;
			float dist = DistanceSquaredNoZ(m_Position, npc->GetPosition());
			int range = GetActSpellRange(mercSpell.spellid, spells[mercSpell.spellid].range);
			range *= range;

			if(dist <= range) {
				if(!npc->IsMezzed()) {
					if(HasGroup()) {
						Group* g = GetGroup();

						if(g) {
							for(int i = 0; i < g->GroupCount(); i++) {
								//if(npc->IsOnHatelist(g->members[i]) && g->members[i]->GetTarget() != npc && g->members[i]->IsEngaged()) {
								if(GetTarget() != npc && g->members[i]->GetTarget() != npc && npc->IsOnHatelist(g->members[i])) {
									result++;
								}
							}
						}
					}
				}
			}
		}

		if(result >= 1) {
			Log(Logs::General, Logs::Mercenaries, "%s: Attempting AE Taunt", GetCleanName());
			return true;
		}
	}
	return false;
}

Corpse* Merc::GetGroupMemberCorpse() {
	Corpse* corpse = nullptr;

	if(HasGroup()) {
		Group* g = GetGroup();

		if(g) {
			for(int i = 0; i < g->GroupCount(); i++) {
				if(g->members[i] && g->members[i]->IsClient()) {
					corpse = entity_list.GetCorpseByOwnerWithinRange(g->members[i]->CastToClient(), this, RuleI(Mercs, ResurrectRadius));

					if(corpse && !corpse->IsRezzed()) {
						return corpse;
					}
				}
			}
		}
	}
	return 0;
}

bool Merc::TryHide() {
	if(GetClass() != MELEEDPS) {
		return false;
	}

	//don't hide if already hidden
	if(hidden == true) {
		return false;
	}

	auto outapp = new EQApplicationPacket(OP_SpawnAppearance, sizeof(SpawnAppearance_Struct));
	SpawnAppearance_Struct* sa_out = (SpawnAppearance_Struct*)outapp->pBuffer;
	sa_out->spawn_id = GetID();
	sa_out->type = 0x03;
	sa_out->parameter = 1;
	entity_list.QueueClients(this, outapp, true);
	safe_delete(outapp);
	hidden = true;

	return true;
}

//Checks if Merc still has confidence. Can be checked to begin fleeing, or to regain confidence after confidence loss - true = confident, false = confidence loss
bool Merc::CheckConfidence() {
	bool result = true;
	int ConfidenceLossChance = 0;
	float ConfidenceCheck = 0;
	int ConfidenceRating = 2 * GetProficiencyID();

	std::list<NPC*> npc_list;
	entity_list.GetNPCList(npc_list);

	for (auto itr = npc_list.begin(); itr != npc_list.end(); ++itr) {
		NPC* mob = *itr;
		float ConRating = 1.0;
		int CurrentCon = 0;

		if(!mob) continue;

		if(!mob->IsEngaged()) continue;

		if(mob->IsFeared() || mob->IsMezzed() || mob->IsStunned() || mob->IsRooted() || mob->IsCharmed()) continue;

		if(!mob->CheckAggro(this)) continue;

		float AggroRange = mob->GetAggroRange();

		// Square it because we will be using DistNoRoot

		AggroRange = AggroRange * AggroRange;

		if(DistanceSquared(m_Position, mob->GetPosition()) > AggroRange) continue;

		CurrentCon = this->GetLevelCon(mob->GetLevel());
		switch(CurrentCon) {


					case CON_GRAY: {
						ConRating = 0;
						break;
					}

					case CON_GREEN: {
						ConRating = 0.1;
						break;
									}

					case CON_LIGHTBLUE: {
						ConRating = 0.2;
						break;
										}

					case CON_BLUE: {
						ConRating = 0.6;
						break;
								   }

					case CON_WHITE: {
						ConRating = 1.0;
						break;
									}

					case CON_YELLOW: {
						ConRating = 1.2;
						break;
									 }

					case CON_RED: {
						ConRating = 1.5;
						break;
								  }

					default: {
						ConRating = 0;
						break;
							 }
		}

		ConfidenceCheck += ConRating;
	}

	if(ConfidenceRating < ConfidenceCheck) {
		ConfidenceLossChance = 25 - ( 5 * (GetTierID() - 1));
	}

	if(zone->random.Roll(ConfidenceLossChance)) {
		result = false;
	}

	return result;
}

void Merc::MercMeditate(bool isSitting) {
	// Don't try to meditate if engaged or dead
	if (IsEngaged() || GetAppearance() == eaDead)
	{
		return;
	}
	if(isSitting) {
		// If the merc is a caster and has less than 99% mana while its not engaged, he needs to sit to meditate
		if(GetManaRatio() < 99.0f)
		{
			if(!IsSitting())
				Sit();
		}
		else
		{
			if(IsSitting())
				Stand();
		}
	}
	else
	{
		if(IsSitting())
			Stand();
	}

	if(IsSitting()) {
		if(!rest_timer.Enabled()) {
			rest_timer.Start(RuleI(Character, RestRegenTimeToActivate) * 1000);
		}
	}
	else {
		rest_timer.Disable();
	}
}


void Merc::Sit() {
	if(IsMoving()) {
		moved = false;
		// SetHeading(CalculateHeadingToTarget(GetTarget()->GetX(), GetTarget()->GetY()));
		SendPosition();
		SetMoving(false);
		tar_ndx = 0;
	}

	SetAppearance(eaSitting);
}

void Merc::Stand() {
	SetAppearance(eaStanding);
}

bool Merc::IsSitting() {
	bool result = false;

	if(GetAppearance() == eaSitting && !IsMoving())
		result = true;

	return result;
}

bool Merc::IsStanding() {
	bool result = false;

	if(GetAppearance() == eaStanding)
		result = true;

	return result;
}

float Merc::GetMaxMeleeRangeToTarget(Mob* target) {
	float result = 0;

	if(target) {
		float size_mod = GetSize();
		float other_size_mod = target->GetSize();

		if(GetRace() == 49 || GetRace() == 158 || GetRace() == 196) //For races with a fixed size
			size_mod = 60.0f;
		else if (size_mod < 6.0)
			size_mod = 8.0f;

		if(target->GetRace() == 49 || target->GetRace() == 158 || target->GetRace() == 196) //For races with a fixed size
			other_size_mod = 60.0f;
		else if (other_size_mod < 6.0)
			other_size_mod = 8.0f;

		if (other_size_mod > size_mod) {
			size_mod = other_size_mod;
		}

		// this could still use some work, but for now it's an improvement....

		if (size_mod > 29)
			size_mod *= size_mod;
		else if (size_mod > 19)
			size_mod *= size_mod * 2;
		else
			size_mod *= size_mod * 4;

		// prevention of ridiculously sized hit boxes
		if (size_mod > 10000)
			size_mod = size_mod / 7;

		result = size_mod;
	}

	return result;
}

void Merc::DoClassAttacks(Mob *target) {
	if(target == nullptr)
		return; //gotta have a target for all these

	bool ca_time = classattack_timer.Check(false);

	//only check attack allowed if we are going to do something
	if(ca_time && !IsAttackAllowed(target))
		return;

	if(!ca_time)
		return;

	float HasteModifier = GetHaste() * 0.01f;

	int level = GetLevel();
	int reuse = TauntReuseTime * 1000;      //make this very long since if they dont use it once, they prolly never will
	bool did_attack = false;
	//class specific stuff...
	switch(GetClass()) {
			case MELEEDPS:
				if(level >= 10) {
					reuse = BackstabReuseTime * 1000;
					TryBackstab(target, reuse);
					did_attack = true;
				}
				break;
			case TANK:{
				if(level >= RuleI(Combat, NPCBashKickLevel)){
					if(zone->random.Int(0, 100) > 25) //tested on live, warrior mobs both kick and bash, kick about 75% of the time, casting doesn't seem to make a difference.
					{
						DoAnim(animKick, 0, false);
						int32 dmg = GetBaseSkillDamage(EQEmu::skills::SkillKick);

						if (GetWeaponDamage(target, (const EQEmu::ItemData*)nullptr) <= 0)
							dmg = DMG_INVULNERABLE;

						reuse = KickReuseTime * 1000;
						DoSpecialAttackDamage(target, EQEmu::skills::SkillKick, dmg, 1, -1, reuse);
						did_attack = true;
					}
					else
					{
						DoAnim(animTailRake, 0, false);
						int32 dmg = GetBaseSkillDamage(EQEmu::skills::SkillBash);

						if (GetWeaponDamage(target, (const EQEmu::ItemData*)nullptr) <= 0)
							dmg = DMG_INVULNERABLE;

						reuse = BashReuseTime * 1000;
						DoSpecialAttackDamage(target, EQEmu::skills::SkillBash, dmg, 1, -1, reuse);
						did_attack = true;
					}
				}
				break;
					  }
	}

	classattack_timer.Start(reuse / HasteModifier);
}

bool Merc::Attack(Mob* other, int Hand, bool bRiposte, bool IsStrikethrough, bool IsFromSpell, ExtraAttackOptions *opts)
{
	if (!other) {
		SetTarget(nullptr);
		Log(Logs::General, Logs::Error, "A null Mob object was passed to Merc::Attack() for evaluation!");
		return false;
	}

	return NPC::Attack(other, Hand, bRiposte, IsStrikethrough, IsFromSpell, opts);
}

void Merc::Damage(Mob* other, int32 damage, uint16 spell_id, EQEmu::skills::SkillType attack_skill, bool avoidable, int8 buffslot, bool iBuffTic, eSpecialAttacks special)
{
	if(IsDead() || IsCorpse())
		return;

	if(spell_id==0)
		spell_id = SPELL_UNKNOWN;

	NPC::Damage(other, damage, spell_id, attack_skill, avoidable, buffslot, iBuffTic, special);

	//Not needed since we're using NPC damage.
	//CommonDamage(other, damage, spell_id, attack_skill, avoidable, buffslot, iBuffTic);
}

bool Merc::FindTarget() {
	bool found = false;
	Mob* target = GetHateTop();

	if(target) {
		found = true;
		SetTarget(target);
	}

	return found;
}

void Merc::SetTarget(Mob* mob) {
	NPC::SetTarget(mob);
}

Mob* Merc::GetOwnerOrSelf() {
	Mob* Result = nullptr;

	if(this->GetMercOwner())
		Result = GetMercOwner();
	else
		Result = this;

	return Result;
}

bool Merc::Death(Mob* killerMob, int32 damage, uint16 spell, EQEmu::skills::SkillType attack_skill)
{
	if(!NPC::Death(killerMob, damage, spell, attack_skill))
	{
		return false;
	}

	Save();

	//no corpse, no exp if we're a merc.
	//We'll suspend instead, since that's what live does.
	//Not actually sure live supports 'depopping' merc corpses.
	//if(entity_list.GetCorpseByID(GetID()))
	//      entity_list.GetCorpseByID(GetID())->Depop();

	// If client is in zone, suspend merc, else depop it.
	if (!Suspend())
	{
		Depop();
	}

	return true;
}

Client* Merc::GetMercOwner() {
	Client* mercOwner = nullptr;

	if(GetOwner())
	{
		if(GetOwner()->IsClient())
		{
			mercOwner = GetOwner()->CastToClient();
		}
	}

	return mercOwner;
}

Mob* Merc::GetOwner() {
	Mob* Result = nullptr;

	Result = entity_list.GetMob(GetOwnerID());

	if(!Result) {
		this->SetOwnerID(0);
	}

	return Result->CastToMob();
}

const char* Merc::GetRandomName(){
	// creates up to a 10 char name
	static char name[17];
	char vowels[18]="aeiouyaeiouaeioe";
	char cons[48]="bcdfghjklmnpqrstvwxzybcdgklmnprstvwbcdgkpstrkd";
	char rndname[17]="\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
	char paircons[33]="ngrkndstshthphsktrdrbrgrfrclcr";
	bool valid = false;

	while(!valid) {
		int rndnum=zone->random.Int(0, 75),n=1;
		bool dlc=false;
		bool vwl=false;
		bool dbl=false;
		if (rndnum>63)
		{       // rndnum is 0 - 75 where 64-75 is cons pair, 17-63 is cons, 0-16 is vowel
			rndnum=(rndnum-61)*2;   // name can't start with "ng" "nd" or "rk"
			rndname[0]=paircons[rndnum];
			rndname[1]=paircons[rndnum+1];
			n=2;
		}
		else if (rndnum>16)
		{
			rndnum-=17;
			rndname[0]=cons[rndnum];
		}
		else
		{
			rndname[0]=vowels[rndnum];
			vwl=true;
		}
		int namlen=zone->random.Int(5, 10);
		for (int i=n;i<namlen;i++)
		{
			dlc=false;
			if (vwl)        //last char was a vowel
			{                       // so pick a cons or cons pair
				rndnum=zone->random.Int(0, 62);
				if (rndnum>46)
				{       // pick a cons pair
					if (i>namlen-3) // last 2 chars in name?
					{       // name can only end in cons pair "rk" "st" "sh" "th" "ph" "sk" "nd" or "ng"
						rndnum=zone->random.Int(0, 7)*2;
					}
					else
					{       // pick any from the set
						rndnum=(rndnum-47)*2;
					}
					rndname[i]=paircons[rndnum];
					rndname[i+1]=paircons[rndnum+1];
					dlc=true;       // flag keeps second letter from being doubled below
					i+=1;
				}
				else
				{       // select a single cons
					rndname[i]=cons[rndnum];
				}
			}
			else
			{               // select a vowel
				rndname[i]=vowels[zone->random.Int(0, 16)];
			}
			vwl=!vwl;
			if (!dbl && !dlc)
			{       // one chance at double letters in name
				if (!zone->random.Int(0, i+9))     // chances decrease towards end of name
				{
					rndname[i+1]=rndname[i];
					dbl=true;
					i+=1;
				}
			}
		}

		rndname[0]=toupper(rndname[0]);

		if(!database.CheckNameFilter(rndname)) {
			valid = false;
		}
		else if(rndname[0] < 'A' && rndname[0] > 'Z') {
			//name must begin with an upper-case letter.
			valid = false;
		}
		else if (database.CheckUsedName(rndname)) {
			valid = true;
		}
		else {
			valid = false;
		}
	}

	memset(name, 0, 17);
	strcpy(name, rndname);
	return name;
}

bool Merc::LoadMercSpells() {
	// loads mercs spells into list
	merc_spells.clear();

	std::list<MercSpellEntry> spellList = zone->merc_spells_list[GetClass()];

	if (spellList.size() == 0) {
		AIautocastspell_timer->Disable();
		return false;
	}

	uint8 proficiency_id = GetProficiencyID();
	int16 attack_proc_spell = -1;
	int8 proc_chance = 0;

	for (auto mercSpellEntryItr = spellList.begin(); mercSpellEntryItr != spellList.end(); ++mercSpellEntryItr) {
		if (proficiency_id == mercSpellEntryItr->proficiencyid && GetLevel() >= mercSpellEntryItr->minlevel && GetLevel() <= mercSpellEntryItr->maxlevel && mercSpellEntryItr->spellid > 0) {
			MercSpell mercSpell;

			mercSpell.spellid = mercSpellEntryItr->spellid;
			mercSpell.type = mercSpellEntryItr->type;
			mercSpell.stance = mercSpellEntryItr->stance;
			mercSpell.slot = mercSpellEntryItr->slot;
			mercSpell.proc_chance = mercSpellEntryItr->proc_chance;
			mercSpell.time_cancast = 0;

			merc_spells.push_back(mercSpell);

			if(mercSpellEntryItr->proc_chance > 0)
				AddProcToWeapon(mercSpellEntryItr->spellid, true, mercSpellEntryItr->proc_chance);
		}
	}
	std::sort(merc_spells.begin(), merc_spells.end(), [](const MercSpell& a, const MercSpell& b) {
		return a.slot > b.slot;
	});

	if (merc_spells.empty())
		AIautocastspell_timer->Disable();
	else {
		HasAISpell = true;
		AIautocastspell_timer->Trigger();
	}

	return true;
}

bool Merc::Save() {

	if(database.SaveMerc(this)){
		return true;
	}

	return false;
}

Merc* Merc::LoadMerc(Client *c, MercTemplate* merc_template, uint32 merchant_id, bool updateFromDB) {

	if(c)
	{
		if(c->GetMercID())
		{
			merc_template = zone->GetMercTemplate(c->GetMercInfo().MercTemplateID);
		}
	}

	//get mercenary data
	if(merc_template)
	{
		//TODO: Maybe add a way of updating client merc stats in a seperate function? like, for example, on leveling up.
		const NPCType* npc_type_to_copy = database.GetMercType(merc_template->MercNPCID, merc_template->RaceID, c->GetLevel());
		if(npc_type_to_copy != nullptr)
		{
			//This is actually a very terrible method of assigning stats, and should be changed at some point. See the comment in merc's deconstructor.
			auto npc_type = new NPCType;
			memset(npc_type, 0, sizeof(NPCType));
			memcpy(npc_type, npc_type_to_copy, sizeof(NPCType));
			if(c && !updateFromDB)
			{
				if(c->GetMercInfo().merc_name[0] == 0)
				{
					snprintf(c->GetMercInfo().merc_name, 64, "%s", GetRandomName()); //sanity check.
				}
				snprintf(npc_type->name, 64, "%s", c->GetMercInfo().merc_name);
			}

			npc_type->race = merc_template->RaceID;

			// Use the Gender and Size of the Merchant if possible
			uint8 tmpgender = 0;
			float tmpsize = 6.0f;
			if(merchant_id > 0)
			{
				NPC* tar = entity_list.GetNPCByID(merchant_id);
				if(tar)
				{
					tmpgender = tar->GetGender();
					tmpsize = tar->GetSize();
				}
				else
				{
					tmpgender = Mob::GetDefaultGender(npc_type->race, c->GetMercInfo().Gender);
				}

			}
			else
			{
				tmpgender = c->GetMercInfo().Gender;
				tmpsize = c->GetMercInfo().MercSize;
			}

			sprintf(npc_type->lastname, "%s's Mercenary", c->GetName());
			npc_type->gender = tmpgender;
			npc_type->size = tmpsize;
			npc_type->loottable_id = 0; // Loottable has to be 0, otherwise we'll be leavin' some corpses!
			npc_type->npc_id = 0; //NPC ID has to be 0, otherwise db gets all confuzzled.
			npc_type->class_ = merc_template->ClassID;
			npc_type->maxlevel = 0; //We should hard-set this to override scalerate's functionality in the NPC class when it is constructed.
			npc_type->no_target_hotkey = 1;

			auto merc = new Merc(npc_type, c->GetX(), c->GetY(), c->GetZ(), 0);
			merc->GiveNPCTypeData(npc_type); // for clean up, works a bit like pets

			if(merc)
			{
				merc->SetMercData( merc_template->MercTemplateID );
				database.LoadMercEquipment(merc);
				merc->UpdateMercStats(c, true);

				if(updateFromDB)
				{
					database.LoadCurrentMerc(c);

					merc->SetMercID(c->GetMercInfo().mercid);
					snprintf(merc->name, 64, "%s", c->GetMercInfo().merc_name);
					merc->SetSuspended(c->GetMercInfo().IsSuspended);
					merc->gender = c->GetMercInfo().Gender;
					merc->size = c->GetMercInfo().MercSize;
					merc->SetHP(c->GetMercInfo().hp <= 0 ? merc->GetMaxHP() : c->GetMercInfo().hp);
					merc->SetMana(c->GetMercInfo().hp <= 0 ? merc->GetMaxMana() : c->GetMercInfo().mana);
					merc->SetEndurance(c->GetMercInfo().endurance);
					merc->luclinface = c->GetMercInfo().face;
					merc->hairstyle = c->GetMercInfo().luclinHairStyle;
					merc->haircolor = c->GetMercInfo().luclinHairColor;
					merc->eyecolor1 = c->GetMercInfo().luclinEyeColor;
					merc->eyecolor2 = c->GetMercInfo().luclinEyeColor2;
					merc->beardcolor = c->GetMercInfo().luclinBeardColor;
					merc->beard = c->GetMercInfo().luclinBeard;
					merc->drakkin_heritage = c->GetMercInfo().drakkinHeritage;
					merc->drakkin_tattoo = c->GetMercInfo().drakkinTattoo;
					merc->drakkin_details = c->GetMercInfo().drakkinDetails;
				}
				else
				{
					// Give Random Features to newly hired Mercs
					merc->RandomizeFeatures(false, true);
				}

				if(merc->GetMercID()) {
					database.LoadMercBuffs(merc);
				}

				merc->LoadMercSpells();
			}

			Log(Logs::General, Logs::Mercenaries, "LoadMerc Successful for %s (%s).", merc->GetName(), c->GetName());
			return merc;
		}
	}

	return 0;
}

void Merc::UpdateMercInfo(Client *c) {
	snprintf(c->GetMercInfo().merc_name, 64, "%s", name);
	c->GetMercInfo().mercid = GetMercID();
	c->GetMercInfo().IsSuspended = IsSuspended();
	c->GetMercInfo().Gender = GetGender();
	c->GetMercInfo().MercSize = GetSize();
	c->GetMercInfo().hp = GetHP();
	c->GetMercInfo().mana = GetMana();
	c->GetMercInfo().endurance = GetEndurance();
	c->GetMercInfo().face = luclinface;
	c->GetMercInfo().luclinHairStyle = hairstyle;
	c->GetMercInfo().luclinHairColor = haircolor;
	c->GetMercInfo().luclinEyeColor = eyecolor1;
	c->GetMercInfo().luclinEyeColor2 = eyecolor2;
	c->GetMercInfo().luclinBeardColor = beardcolor;
	c->GetMercInfo().luclinBeard = beard;
	c->GetMercInfo().drakkinHeritage = drakkin_heritage;
	c->GetMercInfo().drakkinTattoo = drakkin_tattoo;
	c->GetMercInfo().drakkinDetails = drakkin_details;
}

void Merc::UpdateMercStats(Client *c, bool setmax)
{
	if (c->GetMercInfo().MercTemplateID > 0) {
		Log(Logs::General, Logs::Mercenaries, "Updating Mercenary Stats for %s (%s).", GetName(),
			c->GetName());
		const NPCType *npc_type = database.GetMercType(
		    zone->GetMercTemplate(c->GetMercInfo().MercTemplateID)->MercNPCID, GetRace(), c->GetLevel());
		if (npc_type) {
			max_hp = npc_type->max_hp;
			base_hp = npc_type->max_hp;
			max_mana = npc_type->Mana;
			base_mana = npc_type->Mana;
			max_end = npc_type->max_hp;  // Hack since Endurance does not exist for NPCType yet
			base_end = npc_type->max_hp; // Hack since Endurance does not exist for NPCType yet
			hp_regen = npc_type->hp_regen;
			mana_regen = npc_type->mana_regen;
			max_dmg = npc_type->max_dmg;
			min_dmg = npc_type->min_dmg;

			_baseAC = npc_type->AC;
			_baseATK = npc_type->ATK;
			_baseSTR = npc_type->STR;
			_baseSTA = npc_type->STA;
			_baseDEX = npc_type->DEX;
			_baseAGI = npc_type->AGI;
			_baseWIS = npc_type->WIS;
			_baseINT = npc_type->INT;
			_baseCHA = npc_type->CHA;
			_baseATK = npc_type->ATK;
			_baseMR = npc_type->MR;
			_baseFR = npc_type->FR;
			_baseDR = npc_type->DR;
			_basePR = npc_type->PR;
			_baseCR = npc_type->CR;
			_baseCorrup = npc_type->Corrup;

			uint32 scalepercent = (int)(npc_type->scalerate * RuleI(Mercs, ScaleRate) / 100);

			ScaleStats(scalepercent, setmax);

			level = npc_type->level;
			attack_count = npc_type->attack_count;
			attack_delay = npc_type->attack_delay;
			spellscale = npc_type->spellscale;
			healscale = npc_type->healscale;

			CalcBonuses();
			CalcMaxHP();
			CalcMaxMana();
			CalcMaxEndurance();
		}
	}
}

void Merc::ScaleStats(int scalepercent, bool setmax) {

	Log(Logs::General, Logs::Mercenaries, "Scaling Mercenary Stats to %d Percent for %s.", scalepercent, GetName());

	if (scalepercent <= 0)
		return;

	float scalerate = (float)scalepercent / 100.0f;

	if ((int)((float)base_hp * scalerate) > 1)
	{
		max_hp = (int)((float)base_hp * scalerate);
		base_hp = max_hp;
		if (setmax)
			cur_hp = max_hp;
	}

	if (base_mana)
	{
		max_mana = (int)((float)base_mana * scalerate);
		base_mana = max_mana;
		if (setmax)
			current_mana = max_mana;
	}

	if (base_end)
	{
		max_end = (int)((float)base_end * scalerate);
		base_end = max_end;
		if (setmax)
			cur_end = max_end;
	}

	if (_baseAC)
	{
		AC = (int)((float)_baseAC * scalerate);
		_baseAC = AC;
	}

	if (_baseATK)
	{
		ATK = (int)((float)_baseATK * scalerate);
		_baseATK = ATK;
	}

	if (_baseSTR)
	{
		STR = (int)((float)_baseSTR * scalerate);
		_baseSTR = STR;
	}
	if (_baseSTA)
	{
		STA = (int)((float)_baseSTA * scalerate);
		_baseSTA = STA;
	}
	if (_baseAGI)
	{
		AGI = (int)((float)_baseAGI * scalerate);
		_baseAGI = AGI;
	}
	if (_baseDEX)
	{
		DEX = (int)((float)_baseDEX * scalerate);
		_baseDEX = DEX;
	}
	if (_baseINT)
	{
		INT = (int)((float)_baseINT * scalerate);
		_baseINT = INT;
	}
	if (_baseWIS)
	{
		WIS = (int)((float)_baseWIS * scalerate);
		_baseWIS = WIS;
	}
	if (_baseCHA)
	{
		CHA = (int)((float)_baseCHA * scalerate);
		_baseCHA = CHA;
	}

	if (_baseMR)
	{
		MR = (int)((float)_baseMR * scalerate);
		_baseMR = MR;
	}
	if (_baseCR)
	{
		CR = (int)((float)_baseCR * scalerate);
		_baseCR = CR;
	}
	if (_baseDR)
	{
		DR = (int)((float)_baseDR * scalerate);
		_baseDR = DR;
	}
	if (_baseFR)
	{
		FR = (int)((float)_baseFR * scalerate);
		_baseFR = FR;
	}
	if (_basePR)
	{
		PR = (int)((float)_basePR * scalerate);
		_basePR = PR;
	}
	if (_baseCorrup)
	{
		Corrup = (int)((float)_baseCorrup * scalerate);
		_baseCorrup = Corrup;
	}

	if (max_dmg)
	{
		max_dmg = (int)((float)max_dmg * scalerate);
	}
	if (min_dmg)
	{
		min_dmg = (int)((float)min_dmg * scalerate);
	}

	return;
}

void Merc::UpdateMercAppearance() {
	// Copied from Bot Code:
	uint32 itemID = 0;
	uint8 materialFromSlot = EQEmu::textures::materialInvalid;
	for (int i = EQEmu::invslot::EQUIPMENT_BEGIN; i <= EQEmu::invslot::EQUIPMENT_END; ++i) {
		itemID = equipment[i];
		if(itemID != 0) {
			materialFromSlot = EQEmu::InventoryProfile::CalcMaterialFromSlot(i);
			if (materialFromSlot != EQEmu::textures::materialInvalid)
				this->SendWearChange(materialFromSlot);
		}
	}

	if (UpdateActiveLight())
		SendAppearancePacket(AT_Light, GetActiveLightType());
}

void Merc::UpdateEquipmentLight()
{
	m_Light.Type[EQEmu::lightsource::LightEquipment] = 0;
	m_Light.Level[EQEmu::lightsource::LightEquipment] = 0;

	for (int index = EQEmu::invslot::EQUIPMENT_BEGIN; index <= EQEmu::invslot::EQUIPMENT_END; ++index) {
		if (index == EQEmu::invslot::slotAmmo) { continue; }

		auto item = database.GetItem(equipment[index]);
		if (item == nullptr) { continue; }

		if (EQEmu::lightsource::IsLevelGreater(item->Light, m_Light.Type[EQEmu::lightsource::LightEquipment])) {
			m_Light.Type[EQEmu::lightsource::LightEquipment] = item->Light;
			m_Light.Level[EQEmu::lightsource::LightEquipment] = EQEmu::lightsource::TypeToLevel(m_Light.Type[EQEmu::lightsource::LightEquipment]);
		}
	}

	uint8 general_light_type = 0;
	for (auto iter = itemlist.begin(); iter != itemlist.end(); ++iter) {
		auto item = database.GetItem((*iter)->item_id);
		if (item == nullptr) { continue; }

		if (!item->IsClassCommon()) { continue; }
		if (item->Light < 9 || item->Light > 13) { continue; }

		if (EQEmu::lightsource::TypeToLevel(item->Light))
			general_light_type = item->Light;
	}

	if (EQEmu::lightsource::IsLevelGreater(general_light_type, m_Light.Type[EQEmu::lightsource::LightEquipment]))
		m_Light.Type[EQEmu::lightsource::LightEquipment] = general_light_type;

	m_Light.Level[EQEmu::lightsource::LightEquipment] = EQEmu::lightsource::TypeToLevel(m_Light.Type[EQEmu::lightsource::LightEquipment]);
}

void Merc::AddItem(uint8 slot, uint32 item_id) {
	equipment[slot] = item_id;
	UpdateEquipmentLight();
}

bool Merc::Spawn(Client *owner) {

	if(!owner)
		return false;

	MercTemplate* merc_template = zone->GetMercTemplate(GetMercTemplateID());

	if(!merc_template)
		return false;

	entity_list.AddMerc(this, true, true);

	SendPosition();

	Log(Logs::General, Logs::Mercenaries, "Spawn Mercenary %s.", GetName());

	//UpdateMercAppearance();


	return true;
}

void Client::SendMercResponsePackets(uint32 ResponseType)
{
	switch (ResponseType)
	{
	case 0: // Mercenary Spawned Successfully?
		SendMercMerchantResponsePacket(0);
		break;
	case 1: //You do not have enough funds to make that purchase!
		SendMercMerchantResponsePacket(1);
		break;
	case 2: //Mercenary does not exist!
		SendMercMerchantResponsePacket(2);
		break;
	case 3: //Mercenary failed to spawn!
		SendMercMerchantResponsePacket(3);
		break;
	case 4: //Mercenaries are not allowed in raids!
		SendMercMerchantResponsePacket(4);
		break;
	case 5: //You already have a pending mercenary purchase!
		SendMercMerchantResponsePacket(5);
		break;
	case 6: //You have the maximum number of mercenaries.  You must dismiss one before purchasing a new one!
		SendMercMerchantResponsePacket(6);
		break;
	case 7: //You must dismiss your suspended mercenary before purchasing a new one!
		if (ClientVersion() < EQEmu::versions::ClientVersion::RoF)
			SendMercMerchantResponsePacket(7);
		else
			//You have the maximum number of mercenaries.  You must dismiss one before purchasing a new one!
			SendMercMerchantResponsePacket(6);
		break;
	case 8: //You can not purchase a mercenary because your group is full!
		if (ClientVersion() < EQEmu::versions::ClientVersion::RoF)
			SendMercMerchantResponsePacket(8);
		else
			SendMercMerchantResponsePacket(7);
		break;
	case 9: //You can not purchase a mercenary because you are in combat!
		if (ClientVersion() < EQEmu::versions::ClientVersion::RoF)
			//Mercenary failed to spawn!
			SendMercMerchantResponsePacket(3);
		else
			SendMercMerchantResponsePacket(8);
		break;
	case 10: //You have recently dismissed a mercenary and must wait a few more seconds before you can purchase a new one!
		if (ClientVersion() < EQEmu::versions::ClientVersion::RoF)
			//Mercenary failed to spawn!
			SendMercMerchantResponsePacket(3);
		else
			SendMercMerchantResponsePacket(9);
		break;
	case 11: //An error occurred created your mercenary!
		if (ClientVersion() < EQEmu::versions::ClientVersion::RoF)
			SendMercMerchantResponsePacket(9);
		else
			SendMercMerchantResponsePacket(10);
		break;
	case 12: //Upkeep Charge Message
		if (ClientVersion() < EQEmu::versions::ClientVersion::RoF)
			SendMercMerchantResponsePacket(10);
		else
			SendMercMerchantResponsePacket(11);
		break;
	case 13: // ???
		if (ClientVersion() < EQEmu::versions::ClientVersion::RoF)
			SendMercMerchantResponsePacket(11);
		else
			SendMercMerchantResponsePacket(12);
		break;
	case 14: //You ran out of funds to pay for your mercenary!
		if (ClientVersion() < EQEmu::versions::ClientVersion::RoF)
			SendMercMerchantResponsePacket(12);
		else
			SendMercMerchantResponsePacket(13);
		break;
	case 15: // ???
		if (ClientVersion() < EQEmu::versions::ClientVersion::RoF)
			SendMercMerchantResponsePacket(13);
		else
			SendMercMerchantResponsePacket(14);
		break;
	case 16: //Your mercenary is about to be suspended due to insufficient funds!
		if (ClientVersion() < EQEmu::versions::ClientVersion::RoF)
			SendMercMerchantResponsePacket(14);
		else
			SendMercMerchantResponsePacket(15);
		break;
	case 17: //There is no mercenary liaison nearby!
		if (ClientVersion() < EQEmu::versions::ClientVersion::RoF)
			SendMercMerchantResponsePacket(15);
		else
			SendMercMerchantResponsePacket(16);
		break;
	case 18: //You are too far from the liaison!
		if (ClientVersion() < EQEmu::versions::ClientVersion::RoF)
			SendMercMerchantResponsePacket(16);
		else
			SendMercMerchantResponsePacket(17);
		break;
	case 19: //You do not meet the requirements for that mercenary!
		if (ClientVersion() < EQEmu::versions::ClientVersion::RoF)
			SendMercMerchantResponsePacket(17);
		else
			SendMercMerchantResponsePacket(18);
		break;
	case 20: //You are unable to interact with the liaison!
		if (ClientVersion() < EQEmu::versions::ClientVersion::RoF)
			//You are too far from the liaison!
			SendMercMerchantResponsePacket(16);
		else
			SendMercMerchantResponsePacket(19);
		break;
	case 21: //You do not have a high enough membership level to purchase this mercenary!
		if (ClientVersion() < EQEmu::versions::ClientVersion::RoF)
			//You do not meet the requirements for that mercenary!
			SendMercMerchantResponsePacket(17);
		else
			SendMercMerchantResponsePacket(20);
		break;
	case 22: //Your purchase has failed because this mercenary requires a Gold membership!
		if (ClientVersion() < EQEmu::versions::ClientVersion::RoF)
			//You do not meet the requirements for that mercenary!
			SendMercMerchantResponsePacket(17);
		else
			SendMercMerchantResponsePacket(21);
		break;
	case 23: //Your purchase has failed because this mercenary requires at least a Silver membership!
		if (ClientVersion() < EQEmu::versions::ClientVersion::RoF)
			//You do not meet the requirements for that mercenary!
			SendMercMerchantResponsePacket(17);
		else
			SendMercMerchantResponsePacket(22);
		break;
	default: //Mercenary failed to spawn!
		SendMercMerchantResponsePacket(3);
		break;
	}
	Log(Logs::General, Logs::Mercenaries, "SendMercResponsePackets %i for %s.", ResponseType, GetName());

}

void Client::UpdateMercTimer()
{
	Merc *merc = GetMerc();

	if(merc && !merc->IsSuspended())
	{
		if(GetMercTimer()->Check())
		{
			uint32 upkeep = merc->CalcUpkeepCost(merc->GetMercTemplateID(), GetLevel());

			if(CheckCanRetainMerc(upkeep))
			{
				if(RuleB(Mercs, ChargeMercUpkeepCost))
				{
					TakeMoneyFromPP((upkeep * 100), true);
				}
			}
			else
			{
				merc->Suspend();
				return;
			}

			// Reset the upkeep timer
			GetMercInfo().MercTimerRemaining = RuleI(Mercs, UpkeepIntervalMS);
			SendMercTimer(merc);
			GetMercTimer()->Start(RuleI(Mercs, UpkeepIntervalMS));
			GetMercTimer()->SetTimer(GetMercInfo().MercTimerRemaining);

			// Send upkeep charge message
			SendMercResponsePackets(12);

			// Warn that mercenary is about to be suspended due to insufficient funds (on next upkeep)
			if (RuleB(Mercs, ChargeMercUpkeepCost) && upkeep > 0 && !HasMoney(upkeep * 100))
			{
				SendMercResponsePackets(16);
			}

			Log(Logs::General, Logs::Mercenaries, "UpdateMercTimer Complete for %s.", GetName());

			// Normal upkeep charge message
			//Message(7, "You have been charged a mercenary upkeep cost of %i plat, and %i gold and your mercenary upkeep cost timer has been reset to 15 minutes.", upkeep_plat, upkeep_gold, (int)(RuleI(Mercs, UpkeepIntervalMS) / 1000 / 60));

			// Message below given when too low level to be charged
			//Message(7, "Your mercenary waived an upkeep cost of %i plat, and %i gold or %i %s and your mercenary upkeep cost timer has been reset to %i minutes", upkeep_plat, upkeep_gold, 1, "Bayle Marks", (int)(RuleI(Mercs, UpkeepIntervalMS) / 1000 / 60));
		}
	}
}

bool Client::CheckCanHireMerc(Mob* merchant, uint32 template_id) {

	if (!CheckCanSpawnMerc(template_id))
	{
		return false;
	}

	MercTemplate* mercTemplate = zone->GetMercTemplate(template_id);

	//check for suspended merc
	if(GetMercInfo().mercid != 0 && GetMercInfo().IsSuspended) {
		SendMercResponsePackets(6);
		return false;
	}

	// Check if max number of mercs is already reached
	if(GetNumMercs() >= MAXMERCS) {
		SendMercResponsePackets(6);
		return false;
	}

	//check for valid merchant
	if(!merchant) {
		SendMercResponsePackets(17);
		return false;
	}

	//check for merchant too far away
	if(DistanceSquared(m_Position, merchant->GetPosition()) > USE_NPC_RANGE2) {
		SendMercResponsePackets(18);
		return false;
	}

	//check for sufficient funds and remove them last
	if(RuleB(Mercs, ChargeMercPurchaseCost)) {
		uint32 cost = Merc::CalcPurchaseCost(template_id, GetLevel()) * 100;  // Cost is in gold
		if(cost > 0 && !HasMoney(cost)) {
			SendMercResponsePackets(1);
			return false;
		}
	}

	Log(Logs::General, Logs::Mercenaries, "CheckCanHireMerc True for %s.", GetName());

	return true;
}

bool Client::CheckCanRetainMerc(uint32 upkeep) {
	Merc* merc = GetMerc();

	//check for sufficient funds
	if(RuleB(Mercs, ChargeMercPurchaseCost)) {
		if(merc) {
			if(upkeep > 0 && !HasMoney(upkeep * 100)) {
				SendMercResponsePackets(14);
				return false;
			}
		}
	}

	return true;
}

bool Client::CheckCanSpawnMerc(uint32 template_id) {

	// Check if mercs are enabled globally
	if(!RuleB(Mercs, AllowMercs))
	{
		return false;
	}

	// Check if zone allows mercs
	if(!zone->AllowMercs())
	{
		SendMercResponsePackets(3);
		return false;
	}

	MercTemplate* mercTemplate = zone->GetMercTemplate(template_id);

	// Invalid merc data
	if(!mercTemplate)
	{
		SendMercResponsePackets(11);
		return false;
	}

	// Check client version
	if(static_cast<unsigned int>(ClientVersion()) < mercTemplate->ClientVersion)
	{
		SendMercResponsePackets(3);
		return false;
	}

	// Check for raid
	if(HasRaid())
	{
		SendMercResponsePackets(4);
		return false;
	}

	// Check group size
	if(GetGroup() &&  GetGroup()->GroupCount() >= MAX_GROUP_MEMBERS)	// database.GroupCount(GetGroup()->GetID())
	{
		SendMercResponsePackets(8);
		return false;
	}

	// Check in combat
	if(GetAggroCount() > 0)
	{
		SendMercResponsePackets(9);
		return false;
	}

	Log(Logs::General, Logs::Mercenaries, "CheckCanSpawnMerc True for %s.", GetName());

	return true;
}

bool Client::CheckCanUnsuspendMerc() {

	if (!CheckCanSpawnMerc(GetMercInfo().MercTemplateID))
	{
		return false;
	}

	MercTemplate* mercTemplate = zone->GetMercTemplate(GetMercInfo().MercTemplateID);

	if(!GetPTimers().Expired(&database, pTimerMercSuspend, false))
	{
		SendMercResponsePackets(10);
		//TODO: find this packet response and tell them properly.
		Message(0, "You must wait %i seconds before unsuspending your mercenary.", GetPTimers().GetRemainingTime(pTimerMercSuspend));
		return false;
	}

	Log(Logs::General, Logs::Mercenaries, "CheckCanUnsuspendMerc True for %s.", GetName());

	return true;
}

void Client::CheckMercSuspendTimer() {

	if(GetMercInfo().SuspendedTime != 0)
	{
		//if(time(nullptr) >= GetMercInfo().SuspendedTime)
		if (p_timers.Expired(&database, pTimerMercSuspend, false))
		{
			GetMercInfo().SuspendedTime = 0;
			SendMercResponsePackets(0);
			SendMercSuspendResponsePacket(GetMercInfo().SuspendedTime);
			Log(Logs::General, Logs::Mercenaries, "CheckMercSuspendTimer Ready for %s.", GetName());
		}
	}
}

void Client::SuspendMercCommand() {

	if(GetMercInfo().MercTemplateID != 0)
	{
		if(GetMercInfo().IsSuspended)
		{
			if(!CheckCanUnsuspendMerc())
			{
				Log(Logs::General, Logs::Mercenaries, "SuspendMercCommand Unable to Unsuspend Merc for %s.", GetName());

				return;
			}

			// Get merc, assign it to client & spawn
			Merc* merc = Merc::LoadMerc(this, &zone->merc_templates[GetMercInfo().MercTemplateID], 0, true);
			if(merc)
			{
				SpawnMerc(merc, false);
				Log(Logs::General, Logs::Mercenaries, "SuspendMercCommand Successful Unsuspend for %s.", GetName());
			}
			else
			{
				//merc failed to spawn
				SendMercResponsePackets(3);
				Log(Logs::General, Logs::Mercenaries, "SuspendMercCommand Failed to Spawn Merc for %s.", GetName());
			}
		}
		else
		{
			Merc* CurrentMerc = GetMerc();


			if (!RuleB(Mercs, AllowMercSuspendInCombat))
			{
				if (!CheckCanSpawnMerc(GetMercInfo().MercTemplateID))
				{
					return;
				}
			}

			if(CurrentMerc && GetMercID())
			{
				CurrentMerc->Suspend();
				Log(Logs::General, Logs::Mercenaries, "SuspendMercCommand Successful Suspend for %s.", GetName());
			}
			else
			{
				// Reset Merc Suspend State
				GetMercInfo().IsSuspended = true;
				//GetMercInfo().SuspendedTime = time(nullptr) + RuleI(Mercs, SuspendIntervalS);
				//GetMercInfo().MercTimerRemaining = GetMercTimer()->GetRemainingTime();
				//GetMercInfo().Stance = GetStance();
				GetMercTimer()->Disable();
				SendMercSuspendResponsePacket(GetMercInfo().SuspendedTime);
				SendMercTimer(nullptr);
				Log(Logs::General, Logs::Mercenaries, "SuspendMercCommand Failed to Get Merc to Suspend. Resetting Suspend State for %s.", GetName());
			}
		}
	}
	else
	{
		SpawnMercOnZone();
		Log(Logs::General, Logs::Mercenaries, "SuspendMercCommand Request Failed to Load Merc for %s.  Trying SpawnMercOnZone.", GetName());
	}
}


// Handles all client zone change event
void Merc::ProcessClientZoneChange(Client* mercOwner) {

	if(mercOwner)
	{
		Zone();
	}
}

void Client::SpawnMercOnZone() {

	if(!RuleB(Mercs, AllowMercs))
		return;

	if (GetMerc())
		return;

	if(database.LoadMercInfo(this))
	{
		if(!GetMercInfo().IsSuspended)
		{
			GetMercInfo().SuspendedTime = 0;
			// Get merc, assign it to client & spawn
			Merc* merc = Merc::LoadMerc(this, &zone->merc_templates[GetMercInfo().MercTemplateID], 0, true);
			if(merc)
			{
				SpawnMerc(merc, false);
			}
			Log(Logs::General, Logs::Mercenaries, "SpawnMercOnZone Normal Merc for %s.", GetName());
		}
		else
		{
			int32 TimeDiff = GetMercInfo().SuspendedTime - time(nullptr);
			if (TimeDiff > 0)
			{
				if (!GetPTimers().Enabled(pTimerMercSuspend))
				{
					// Start the timer to send the packet that refreshes the Unsuspend Button
					GetPTimers().Start(pTimerMercSuspend, TimeDiff);
				}
			}
			// Send Mercenary Status/Timer packet
			SendMercTimer(GetMerc());

			Log(Logs::General, Logs::Mercenaries, "SpawnMercOnZone Suspended Merc for %s.", GetName());
		}
	}
	else
	{
		// No Merc Hired
		// RoF+ displays a message from the following packet, which seems useless
		//SendClearMercInfo();
		Log(Logs::General, Logs::Mercenaries, "SpawnMercOnZone Failed to load Merc Info from the Database for %s.", GetName());
	}
}

void Client::SendMercTimer(Merc* merc) {

	if (GetMercInfo().mercid == 0)
	{
		return;
	}

	if (!merc)
	{
		SendMercTimerPacket(NO_MERC_ID, MERC_STATE_SUSPENDED, GetMercInfo().SuspendedTime, GetMercInfo().MercTimerRemaining, RuleI(Mercs, SuspendIntervalMS));
		Log(Logs::General, Logs::Mercenaries, "SendMercTimer No Merc for %s.", GetName());
	}
	else if (merc->IsSuspended())
	{
		SendMercTimerPacket(NO_MERC_ID, MERC_STATE_SUSPENDED, GetMercInfo().SuspendedTime, GetMercInfo().MercTimerRemaining, RuleI(Mercs, SuspendIntervalMS));
		Log(Logs::General, Logs::Mercenaries, "SendMercTimer Suspended Merc for %s.", GetName());
	}
	else
	{
		SendMercTimerPacket(merc->GetID(), MERC_STATE_NORMAL, NOT_SUSPENDED_TIME, GetMercInfo().MercTimerRemaining, RuleI(Mercs, SuspendIntervalMS));
		Log(Logs::General, Logs::Mercenaries, "SendMercTimer Normal Merc for %s.", GetName());
	}

}

void Client::SpawnMerc(Merc* merc, bool setMaxStats) {

	if (!merc || !CheckCanSpawnMerc(merc->GetMercTemplateID()))
	{
		if (merc)
		{
			merc->Suspend();
		}
		return;
	}

	merc->Spawn(this);
	merc->SetSuspended(false);
	SetMerc(merc);
	merc->Unsuspend(setMaxStats);
	merc->SetStance(GetMercInfo().Stance);

	Log(Logs::General, Logs::Mercenaries, "SpawnMerc Success for %s.", GetName());

	return;

}

bool Merc::Suspend() {

	Client* mercOwner = GetMercOwner();

	if(!mercOwner)
		return false;

	SetSuspended(true);

	mercOwner->GetMercInfo().IsSuspended = true;
	mercOwner->GetMercInfo().SuspendedTime = time(nullptr) + RuleI(Mercs, SuspendIntervalS);
	mercOwner->GetMercInfo().MercTimerRemaining = mercOwner->GetMercTimer()->GetRemainingTime();
	mercOwner->GetMercInfo().Stance = GetStance();
	Save();
	mercOwner->GetMercTimer()->Disable();
	mercOwner->SendMercSuspendResponsePacket(mercOwner->GetMercInfo().SuspendedTime);
	mercOwner->SendMercTimer(this);

	Depop();

	// Start the timer to send the packet that refreshes the Unsuspend Button
	mercOwner->GetPTimers().Start(pTimerMercSuspend, RuleI(Mercs, SuspendIntervalS));

	Log(Logs::General, Logs::Mercenaries, "Suspend Complete for %s.", mercOwner->GetName());

	return true;
}

bool Client::MercOnlyOrNoGroup() {

	if (!GetGroup())
	{
		return true;
	}
	if (GetMerc())
	{
		if (GetMerc()->GetGroup() == GetGroup())
		{
			if (GetGroup()->GroupCount() < 3)
			{
				return true;
			}
		}
	}
	return false;
}

bool Merc::Unsuspend(bool setMaxStats) {

	Client* mercOwner = nullptr;

	if(GetMercOwner()) {
		mercOwner = GetMercOwner();
	}

	if(!mercOwner)
		return false;

	if(GetID())
	{
		// Set time remaining to max on unsuspend - there is a charge for unsuspending as well
		SetSuspended(false);

		mercOwner->GetMercInfo().mercid = GetMercID();
		mercOwner->GetMercInfo().IsSuspended = false;

		mercOwner->SendMercenaryUnsuspendPacket(0);
		mercOwner->SendMercenaryUnknownPacket(1);
		mercOwner->GetMercInfo().SuspendedTime = 0;
		// Reset the upkeep timer
		mercOwner->GetMercInfo().MercTimerRemaining = RuleI(Mercs, UpkeepIntervalMS);
		mercOwner->GetMercTimer()->Start(RuleI(Mercs, UpkeepIntervalMS));
		//mercOwner->GetMercTimer()->SetTimer(mercOwner->GetMercInfo().MercTimerRemaining);
		mercOwner->SendMercTimer(this);
		if(!mercOwner->GetPTimers().Expired(&database, pTimerMercSuspend, false))
			mercOwner->GetPTimers().Clear(&database, pTimerMercSuspend);

		if (MercJoinClientGroup())
		{
			if(setMaxStats)
			{
				SetHP(GetMaxHP());
				SetMana(GetMaxMana());
				SetEndurance(GetMaxEndurance());
			}

			//check for sufficient funds and remove them last
			if(RuleB(Mercs, ChargeMercUpkeepCost))
			{
				uint32 cost = CalcUpkeepCost(GetMercTemplateID(), GetLevel()) * 100;    // Cost is in gold
				if(cost > 0 && !mercOwner->HasMoney(cost))
				{
					mercOwner->SendMercResponsePackets(1);
					Suspend();
					return false;
				}
			}
			Save();
		}
	}

	return true;
}

bool Client::DismissMerc(uint32 MercID) {

	bool Dismissed = true;
	if (!database.DeleteMerc(MercID))
	{
		Log(Logs::General, Logs::Mercenaries, "Dismiss Failed Database Query for MercID: %i, Client: %s.", MercID, GetName());
		Dismissed = false;
	}
	else
	{
		Log(Logs::General, Logs::Mercenaries, "Dismiss Successful for %s.", GetName());
	}

	if (GetMerc())
	{
		GetMerc()->Depop();
	}

	SendClearMercInfo();
	SetMerc(nullptr);

	return Dismissed;
}

void Merc::Zone() {
	Save();
	Depop();
}

void Merc::Depop() {

	WipeHateList();

	if(IsCasting())
	{
		InterruptSpell();
	}

	entity_list.RemoveFromHateLists(this);

	if(GetGroup())
	{
		RemoveMercFromGroup(this, GetGroup());
	}

	entity_list.RemoveMerc(this->GetID());

	if(HasPet())
	{
		GetPet()->Depop();
	}

	p_depop = true;

	NPC::Depop(false);
}

bool Merc::RemoveMercFromGroup(Merc* merc, Group* group) {

	bool Result = false;

	if(merc && group)
	{
		uint32 groupID = group->GetID();
		if(merc->HasGroup())
		{
			if(!group->IsLeader(merc))
			{
				merc->SetFollowID(0);

				if (group->GroupCount() <= 2 && merc->GetGroup() == group && is_zone_loaded)
				{
					group->DisbandGroup();
				}
				else if(group->DelMember(merc, true))
				{
					if(merc->GetMercCharacterID() != 0)
					{
						database.SetGroupID(merc->GetName(), 0, merc->GetMercCharacterID(), true);
					}
				}
			}
			else
			{
				// A merc is group leader - Disband and re-group each member with their mercs
				for(int i = 0; i < MAX_GROUP_MEMBERS; i++)
				{
					if(!group->members[i])
						continue;

					if(!group->members[i]->IsClient())
						continue;

					Client *groupMember = group->members[i]->CastToClient();
					groupMember->LeaveGroup();
					if (groupMember->GetMerc())
					{
						groupMember->GetMerc()->MercJoinClientGroup();
					}
				}
				// Group should be removed by now, but just in case:
				Group *oldGroup = entity_list.GetGroupByID(groupID);
				if (oldGroup != nullptr)
				{
					oldGroup->DisbandGroup();
				}
			}

			Result = true;
		}
	}

	return Result;
}

bool Merc::MercJoinClientGroup() {

	Client* mercOwner = nullptr;

	if(GetMercOwner())
	{
		mercOwner = GetMercOwner();
	}

	if(!mercOwner)
	{
		Suspend();
		return false;
	}

	if(GetID())
	{
		if (HasGroup())
		{
			RemoveMercFromGroup(this, GetGroup());
		}

		Group* g = entity_list.GetGroupByClient(mercOwner);

		//nobody from our group is here... start a new group
		if(!g)
		{
			g = new Group(mercOwner);

			if(!g)
			{
				delete g;
				g = nullptr;
				return false;
			}

			entity_list.AddGroup(g);

			if(g->GetID() == 0)
			{

				delete g;
				g = nullptr;
				return false;
			}

			if (AddMercToGroup(this, g))
			{
				database.SetGroupID(mercOwner->GetName(), g->GetID(), mercOwner->CharacterID(), false);
				database.SetGroupLeaderName(g->GetID(), mercOwner->GetName());
				database.RefreshGroupFromDB(mercOwner);
				g->SaveGroupLeaderAA();
				Log(Logs::General, Logs::Mercenaries, "Mercenary joined new group: %s (%s).", GetName(), mercOwner->GetName());
			}
			else
			{
				g->DisbandGroup();
				Suspend();
				Log(Logs::General, Logs::Mercenaries, "Mercenary disbanded new group: %s (%s).", GetName(), mercOwner->GetName());
			}

		}
		else if (AddMercToGroup(this, mercOwner->GetGroup()))
		{
			// Group already exists
			database.RefreshGroupFromDB(mercOwner);
			// Update members that are out of zone
			GetGroup()->SendGroupJoinOOZ(this);
			Log(Logs::General, Logs::Mercenaries, "Mercenary %s joined existing group with %s.", GetName(), mercOwner->GetName());
		}
		else
		{
			Suspend();
			Log(Logs::General, Logs::Mercenaries, "Mercenary failed to join the group - Suspending %s for (%s).", GetName(), mercOwner->GetName());
		}
	}

	return true;
}

bool Merc::AddMercToGroup(Merc* merc, Group* group) {
	bool Result = false;

	if(merc && group) {
		// Remove merc from current group if it's not the destination group
		if(merc->HasGroup())
		{
			if(merc->GetGroup() == group && merc->GetMercOwner())
			{
				// Merc is already in the destination group
				merc->SetFollowID(merc->GetMercOwner()->GetID());
				return true;
			}
			merc->RemoveMercFromGroup(merc, merc->GetGroup());
		}
		//Try and add the member, followed by checking if the merc owner exists.
		if(group->AddMember(merc) && merc->GetMercOwner())
		{
			merc->SetFollowID(merc->GetMercOwner()->GetID());
			Result = true;
		}
		else
		{
			//Suspend it if the member is not added or the merc's owner is not valid.
			merc->Suspend();
		}
	}

	return Result;
}

void Client::InitializeMercInfo() {

	for(int i=0; i<MAXMERCS; i++)
	{
		m_mercinfo[i] = MercInfo();
	}

}

Merc* Client::GetMerc() {

	if(GetMercID() == 0)
	{
		Log(Logs::Detail, Logs::Mercenaries, "GetMerc - GetMercID: 0 for %s.", GetName());
		return (nullptr);
	}

	Merc* tmp = entity_list.GetMercByID(GetMercID());
	if(tmp == nullptr)
	{
		SetMercID(0);
		Log(Logs::Detail, Logs::Mercenaries, "GetMerc No Merc for %s.", GetName());
		return (nullptr);
	}

	if(tmp->GetOwnerID() != GetID())
	{
		SetMercID(0);
		Log(Logs::Detail, Logs::Mercenaries, "GetMerc Owner Mismatch - OwnerID: %d, ClientID: %d, Client: %s.", tmp->GetOwnerID(), GetID(), GetName());
		return (nullptr);
	}

	return (tmp);
}

uint8 Client::GetNumMercs() {

	uint8 numMercs = 0;

	for(int i=0; i<MAXMERCS; i++)
	{
		if(m_mercinfo[i].mercid != 0)
		{
			numMercs++;
		}
	}
	Log(Logs::General, Logs::Mercenaries, "GetNumMercs Number: %i for %s.", numMercs, GetName());

	return numMercs;
}

void Merc::SetMercData( uint32 template_id ) {

	MercTemplate* merc_template = zone->GetMercTemplate(template_id);
	SetMercTemplateID( merc_template->MercTemplateID );
	SetMercType( merc_template->MercType );
	SetMercSubType( merc_template->MercSubType );
	SetProficiencyID( merc_template->ProficiencyID );
	SetTierID( merc_template->TierID );
	SetCostFormula( merc_template->CostFormula );
	SetMercNameType( merc_template->MercNameType );

}

MercTemplate* Zone::GetMercTemplate( uint32 template_id ) {
	return &merc_templates[template_id];
}

void Client::SetMerc(Merc* newmerc) {

	Merc* oldmerc = GetMerc();
	if (oldmerc)
	{
		oldmerc->SetOwnerID(0);
	}

	if (!newmerc)
	{
		SetMercID(0);
		GetMercInfo().mercid = 0;
		GetMercInfo().MercTemplateID = 0;
		GetMercInfo().myTemplate = nullptr;
		GetMercInfo().IsSuspended = false;
		GetMercInfo().SuspendedTime = 0;
		GetMercInfo().Gender = 0;
		GetMercInfo().State = 0;
		memset(GetMercInfo().merc_name, 0, 64);
		Log(Logs::General, Logs::Mercenaries, "SetMerc No Merc for %s.", GetName());
	}
	else
	{
		SetMercID(newmerc->GetID());
		//Client* oldowner = entity_list.GetClientByID(newmerc->GetOwnerID());
		newmerc->SetOwnerID(this->GetID());
		newmerc->SetMercCharacterID(this->CharacterID());
		newmerc->SetClientVersion((uint8)this->ClientVersion());
		GetMercInfo().mercid = newmerc->GetMercID();
		GetMercInfo().MercTemplateID = newmerc->GetMercTemplateID();
		GetMercInfo().myTemplate = zone->GetMercTemplate(GetMercInfo().MercTemplateID);
		GetMercInfo().IsSuspended = newmerc->IsSuspended();
		GetMercInfo().SuspendedTime = 0;
		GetMercInfo().Gender = newmerc->GetGender();
		GetMercInfo().State = newmerc->IsSuspended() ? MERC_STATE_SUSPENDED : MERC_STATE_NORMAL;
		snprintf(GetMercInfo().merc_name, 64, "%s", newmerc->GetName());
		Log(Logs::General, Logs::Mercenaries, "SetMerc New Merc for %s.", GetName());
	}
}

void Client::UpdateMercLevel() {
	Merc* merc = GetMerc();
	if (merc)
	{
		merc->UpdateMercStats(this, false);
		merc->SendAppearancePacket(AT_WhoLevel, GetLevel(), true, true);
	}
}

void Client::SendMercMerchantResponsePacket(int32 response_type) {
	// This response packet brings up the Mercenary Manager window
	if (ClientVersion() >= EQEmu::versions::ClientVersion::SoD)
	{
		auto outapp = new EQApplicationPacket(OP_MercenaryHire, sizeof(MercenaryMerchantResponse_Struct));
		MercenaryMerchantResponse_Struct* mmr = (MercenaryMerchantResponse_Struct*)outapp->pBuffer;
		mmr->ResponseType = response_type;              // send specified response type
		FastQueuePacket(&outapp);
		Log(Logs::Moderate, Logs::Mercenaries, "Sent SendMercMerchantResponsePacket ResponseType: %i, Client: %s.", response_type, GetName());
	}
}

void Client::SendMercenaryUnknownPacket(uint8 type) {

	auto outapp = new EQApplicationPacket(OP_MercenaryUnknown1, 1);
	outapp->WriteUInt8(type);
	FastQueuePacket(&outapp);
	Log(Logs::Moderate, Logs::Mercenaries, "Sent SendMercenaryUnknownPacket Type: %i, Client: %s.", type, GetName());

}

void Client::SendMercenaryUnsuspendPacket(uint8 type) {

	auto outapp = new EQApplicationPacket(OP_MercenaryUnsuspendResponse, 1);
	outapp->WriteUInt8(type);
	FastQueuePacket(&outapp);
	Log(Logs::Moderate, Logs::Mercenaries, "Sent SendMercenaryUnsuspendPacket Type: %i, Client: %s.", type, GetName());

}

void Client::SendMercSuspendResponsePacket(uint32 suspended_time) {

	auto outapp = new EQApplicationPacket(OP_MercenarySuspendResponse, sizeof(SuspendMercenaryResponse_Struct));
	SuspendMercenaryResponse_Struct* smr = (SuspendMercenaryResponse_Struct*)outapp->pBuffer;
	smr->SuspendTime = suspended_time;              // Seen 0 (not suspended) or c9 c2 64 4f (suspended on Sat Mar 17 11:58:49 2012) - Unix Timestamp
	FastQueuePacket(&outapp);
	Log(Logs::Moderate, Logs::Mercenaries, "Sent SendMercSuspendResponsePacket Time: %i, Client: %s.", suspended_time, GetName());

}

void Client::SendMercTimerPacket(int32 entity_id, int32 merc_state, int32 suspended_time, int32 update_interval, int32 unk01) {

	// Send Mercenary Status/Timer packet
	auto outapp = new EQApplicationPacket(OP_MercenaryTimer, sizeof(MercenaryStatus_Struct));
	MercenaryStatus_Struct* mss = (MercenaryStatus_Struct*)outapp->pBuffer;
	mss->MercEntityID = entity_id; // Seen 0 (no merc spawned) or unknown value when merc is spawned
	mss->MercState = merc_state; // Seen 5 (normal) or 1 (suspended)
	mss->SuspendedTime = suspended_time; // Seen 0 for not suspended or Unix Timestamp for suspended merc
	mss->UpdateInterval = update_interval; // Seen 900000 - 15 minutes in ms
	mss->MercUnk01 = unk01; // Seen 180000 - 3 minutes in ms - Used for the unsuspend button refresh timer
	FastQueuePacket(&outapp);
	Log(Logs::Moderate, Logs::Mercenaries, "Sent SendMercTimerPacket EndID: %i, State: %i, SuspendTime: %i, Interval: %i, Unk1: %i, Client: %s.", entity_id, merc_state, suspended_time, update_interval, unk01, GetName());

}

void Client::SendMercAssignPacket(uint32 entityID, uint32 unk01, uint32 unk02) {
	auto outapp = new EQApplicationPacket(OP_MercenaryAssign, sizeof(MercenaryAssign_Struct));
	MercenaryAssign_Struct* mas = (MercenaryAssign_Struct*)outapp->pBuffer;
	mas->MercEntityID = entityID;
	mas->MercUnk01 = unk01;
	mas->MercUnk02 = unk02;
	FastQueuePacket(&outapp);
	Log(Logs::Moderate, Logs::Mercenaries, "Sent SendMercAssignPacket EndID: %i, Unk1: %i, Unk2: %i, Client: %s.", entityID, unk01, unk02, GetName());
}

void NPC::LoadMercTypes() {

	std::string query = StringFormat("SELECT DISTINCT MTyp.dbstring, MTyp.clientversion "
		"FROM merc_merchant_entries MME, merc_merchant_template_entries MMTE, "
		"merc_types MTyp, merc_templates MTem "
		"WHERE MME.merchant_id = %i "
		"AND MME.merc_merchant_template_id = MMTE.merc_merchant_template_id "
		"AND MMTE.merc_template_id = MTem.merc_template_id "
		"AND MTem.merc_type_id = MTyp.merc_type_id;", GetNPCTypeID());
	auto results = database.QueryDatabase(query);
	if (!results.Success())
	{
		Log(Logs::General, Logs::Error, "Error in NPC::LoadMercTypes()");
		return;
	}

	for (auto row = results.begin(); row != results.end(); ++row)
	{
		MercType tempMercType;

		tempMercType.Type = atoi(row[0]);
		tempMercType.ClientVersion = atoi(row[1]);

		mercTypeList.push_back(tempMercType);
	}

}

void NPC::LoadMercs() {

	std::string query = StringFormat("SELECT DISTINCT MTem.merc_template_id, MTyp.dbstring AS merc_type_id, "
		"MTem.dbstring AS merc_subtype_id, 0 AS CostFormula, "
		"CASE WHEN MTem.clientversion > MTyp.clientversion "
		"THEN MTem.clientversion "
		"ELSE MTyp.clientversion END AS clientversion, MTem.merc_npc_type_id "
		"FROM merc_merchant_entries MME, merc_merchant_template_entries MMTE, "
		"merc_types MTyp, merc_templates MTem "
		"WHERE MME.merchant_id = %i AND "
		"MME.merc_merchant_template_id = MMTE.merc_merchant_template_id "
		"AND MMTE.merc_template_id = MTem.merc_template_id "
		"AND MTem.merc_type_id = MTyp.merc_type_id;", GetNPCTypeID());
	auto results = database.QueryDatabase(query);

	if (!results.Success())
	{
		Log(Logs::General, Logs::Error, "Error in NPC::LoadMercTypes()");
		return;
	}

	for (auto row = results.begin(); row != results.end(); ++row)
	{
		MercData tempMerc;

		tempMerc.MercTemplateID = atoi(row[0]);
		tempMerc.MercType = atoi(row[1]);
		tempMerc.MercSubType = atoi(row[2]);
		tempMerc.CostFormula = atoi(row[3]);
		tempMerc.ClientVersion = atoi(row[4]);
		tempMerc.NPCID = atoi(row[5]);

		mercDataList.push_back(tempMerc);
	}

}

int NPC::GetNumMercTypes(uint32 clientVersion) {

	int count = 0;
	std::list<MercType> mercTypeList = GetMercTypesList();

	for (auto mercTypeListItr = mercTypeList.begin(); mercTypeListItr != mercTypeList.end(); ++mercTypeListItr) {
		if(mercTypeListItr->ClientVersion <= clientVersion)
			count++;
	}

	return count;
}

int NPC::GetNumMercs(uint32 clientVersion) {

	int count = 0;
	std::list<MercData> mercDataList = GetMercsList();

	for (auto mercListItr = mercDataList.begin(); mercListItr != mercDataList.end(); ++mercListItr) {
		if(mercListItr->ClientVersion <= clientVersion)
			count++;
	}

	return count;
}

std::list<MercType> NPC::GetMercTypesList(uint32 clientVersion) {

	std::list<MercType> result;

	if(GetNumMercTypes() > 0)
	{
		for (auto mercTypeListItr = mercTypeList.begin(); mercTypeListItr != mercTypeList.end();
		     ++mercTypeListItr) {
			if(mercTypeListItr->ClientVersion <= clientVersion)
			{
				MercType mercType;
				mercType.Type = mercTypeListItr->Type;
				mercType.ClientVersion = mercTypeListItr->ClientVersion;
				result.push_back(mercType);
			}
		}
	}

	return result;
}

std::list<MercData> NPC::GetMercsList(uint32 clientVersion) {

	std::list<MercData> result;

	if(GetNumMercs() > 0)
	{
		for (auto mercListItr = mercDataList.begin(); mercListItr != mercDataList.end(); ++mercListItr) {
			if(mercListItr->ClientVersion <= clientVersion)
			{
				MercTemplate *merc_template = zone->GetMercTemplate(mercListItr->MercTemplateID);

				if(merc_template)
				{
					MercData mercData;
					mercData.MercTemplateID = mercListItr->MercTemplateID;
					mercData.MercType = merc_template->MercType;
					mercData.MercSubType = merc_template->MercSubType;
					mercData.CostFormula = merc_template->CostFormula;
					mercData.ClientVersion = merc_template->ClientVersion;
					mercData.NPCID = merc_template->MercNPCID;
					result.push_back(mercData);
				}
			}
		}
	}

	return result;
}

uint32 Merc::CalcPurchaseCost(uint32 templateID , uint8 level, uint8 currency_type) {

	uint32 cost = 0;

	MercTemplate *mercData = zone->GetMercTemplate(templateID);

	if(mercData)
	{
		//calculate cost in coin - cost in gold
		if(currency_type == 0)
		{
			int levels_above_cutoff;
			switch (mercData->CostFormula)
			{
			case 0:
				levels_above_cutoff = level > 10 ? (level - 10) : 0;
				cost = levels_above_cutoff * 300;
				cost += level >= 10 ? 100 : 0;
				cost /= 100;
				break;
			default:
				break;
			}
		}
		else if(currency_type == 19)
		{
			// cost in Bayle Marks
			cost = 1;
		}
	}

	return cost;
}

uint32 Merc::CalcUpkeepCost(uint32 templateID , uint8 level, uint8 currency_type) {

	uint32 cost = 0;

	MercTemplate *mercData = zone->GetMercTemplate(templateID);

	if(mercData)
	{
		//calculate cost in coin - cost in gold
		if(currency_type == 0)
		{
			int levels_above_cutoff;
			switch (mercData->CostFormula)
			{
			case 0:
				levels_above_cutoff = level > 10 ? (level - 10) : 0;
				cost = levels_above_cutoff * 300;
				cost += level >= 10 ? 100 : 0;
				cost /= 100;
				break;
			default:
				break;
			}
		}
		else if(currency_type == 19)
		{
			// cost in Bayle Marks
			cost = 1;
		}
	}

	return cost;
}
