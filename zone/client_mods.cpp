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
*/
#include "../common/debug.h"
#include "masterentity.h"
#include "worldserver.h"
#include "zonedb.h"
#include "../common/spdat.h"
#include "../common/packet_dump.h"
#include "../common/packet_functions.h"
#include "petitions.h"
#include "../common/serverinfo.h"
#include "../common/ZoneNumbers.h"
#include "../common/moremath.h"
#include "../common/guilds.h"
#include "../common/logsys.h"
#include "StringIDs.h"
#include "NpcAI.h"


// Return max stat value for level
int16 Client::GetMaxStat() const {

	if((RuleI(Character, StatCap)) > 0)
		return (RuleI(Character, StatCap));

	int level = GetLevel();

	int16 base = 0;

	if (level < 61) {
		base = 255;
	}
	else if (GetClientVersion() >= EQClientSoF) {
		base = 255 + 5 * (level - 60);
	}
	else if (level < 71) {
		base = 255 + 5 * (level - 60);
	}
	else {
		base = 330;
	}

	return(base);
}

int16 Client::GetMaxResist() const
{
	int level = GetLevel();

	int16 base = 500;

	if(level > 60)
		base += ((level - 60) * 5);

	return base;
}

int16 Client::GetMaxSTR() const {
	return GetMaxStat()
		+ itembonuses.STRCapMod
		+ spellbonuses.STRCapMod
		+ aabonuses.STRCapMod;
}
int16 Client::GetMaxSTA() const {
	return GetMaxStat()
		+ itembonuses.STACapMod
		+ spellbonuses.STACapMod
		+ aabonuses.STACapMod;
}
int16 Client::GetMaxDEX() const {
	return GetMaxStat()
		+ itembonuses.DEXCapMod
		+ spellbonuses.DEXCapMod
		+ aabonuses.DEXCapMod;
}
int16 Client::GetMaxAGI() const {
	return GetMaxStat()
		+ itembonuses.AGICapMod
		+ spellbonuses.AGICapMod
		+ aabonuses.AGICapMod;
}
int16 Client::GetMaxINT() const {
	return GetMaxStat()
		+ itembonuses.INTCapMod
		+ spellbonuses.INTCapMod
		+ aabonuses.INTCapMod;
}
int16 Client::GetMaxWIS() const {
	return GetMaxStat()
		+ itembonuses.WISCapMod
		+ spellbonuses.WISCapMod
		+ aabonuses.WISCapMod;
}
int16 Client::GetMaxCHA() const {
	return GetMaxStat()
		+ itembonuses.CHACapMod
		+ spellbonuses.CHACapMod
		+ aabonuses.CHACapMod;
}
int16 Client::GetMaxMR() const {
	return GetMaxResist()
		+ itembonuses.MRCapMod
		+ spellbonuses.MRCapMod
		+ aabonuses.MRCapMod;
}
int16 Client::GetMaxPR() const {
	return GetMaxResist()
		+ itembonuses.PRCapMod
		+ spellbonuses.PRCapMod
		+ aabonuses.PRCapMod;
}
int16 Client::GetMaxDR() const {
	return GetMaxResist()
		+ itembonuses.DRCapMod
		+ spellbonuses.DRCapMod
		+ aabonuses.DRCapMod;
}
int16 Client::GetMaxCR() const {
	return GetMaxResist()
		+ itembonuses.CRCapMod
		+ spellbonuses.CRCapMod
		+ aabonuses.CRCapMod;
}
int16 Client::GetMaxFR() const {
	return GetMaxResist()
		+ itembonuses.FRCapMod
		+ spellbonuses.FRCapMod
		+ aabonuses.FRCapMod;
}
int16 Client::GetMaxCorrup() const {
	return GetMaxResist()
		+ itembonuses.CorrupCapMod
		+ spellbonuses.CorrupCapMod
		+ aabonuses.CorrupCapMod;
}
int32 Client::LevelRegen()
{
	bool sitting = IsSitting();
	bool feigned = GetFeigned();
	int level = GetLevel();
	bool bonus = GetRaceBitmask(GetBaseRace()) & RuleI(Character, BaseHPRegenBonusRaces);
	uint8 multiplier1 = bonus ? 2 : 1;
	int32 hp = 0;

	//these calculations should match up with the info from Monkly Business, which was last updated ~05/2008: http://www.monkly-business.net/index.php?pageid=abilities
	if (level < 51) {
		if (sitting) {
			if (level < 20)
				hp += 2 * multiplier1;
			else if (level < 50)
				hp += 3 * multiplier1;
			else	//level == 50
				hp += 4 * multiplier1;
		}
		else	//feigned or standing
			hp += 1 * multiplier1;
	}
	//there may be an easier way to calculate this next part, but I don't know what it is
	else {	//level >= 51
		int32 tmp = 0;
		float multiplier2 = 1;
		if (level < 56) {
			tmp = 2;
			if (bonus)
				multiplier2 = 3;
		}
		else if (level < 60) {
			tmp = 3;
			if (bonus)
				multiplier2 = 3.34;
		}
		else if (level < 61) {
			tmp = 4;
			if (bonus)
				multiplier2 = 3;
		}
		else if (level < 63) {
			tmp = 5;
			if (bonus)
				multiplier2 = 2.8;
		}
		else if (level < 65) {
			tmp = 6;
			if (bonus)
				multiplier2 = 2.67;
		}
		else {	//level >= 65
			tmp = 7;
			if (bonus)
				multiplier2 = 2.58;
		}

		hp += int32(float(tmp) * multiplier2);

		if (sitting)
			hp += 3 * multiplier1;
		else if (feigned)
			hp += 1 * multiplier1;
	}

	return hp;
}

int32 Client::CalcHPRegen() {
	int32 regen = LevelRegen() + itembonuses.HPRegen + spellbonuses.HPRegen;

	regen += aabonuses.HPRegen + GroupLeadershipAAHealthRegeneration();

	return (regen * RuleI(Character, HPRegenMultiplier) / 100);
}

int32 Client::CalcHPRegenCap()
{
	int cap = RuleI(Character, ItemHealthRegenCap) + itembonuses.HeroicSTA/25;

	cap += aabonuses.ItemHPRegenCap + spellbonuses.ItemHPRegenCap + itembonuses.ItemHPRegenCap;

	return (cap * RuleI(Character, HPRegenMultiplier) / 100);
}

int32 Client::CalcMaxHP() {
	float nd = 10000;
	max_hp = (CalcBaseHP() + itembonuses.HP);

	//The AA desc clearly says it only applies to base hp..
	//but the actual effect sent on live causes the client
	//to apply it to (basehp + itemhp).. I will oblige to the client's whims over
	//the aa description
	nd += aabonuses.MaxHP;	//Natural Durability, Physical Enhancement, Planar Durability

	max_hp = (float)max_hp * (float)nd / (float)10000; //this is to fix the HP-above-495k issue
	max_hp += spellbonuses.HP + aabonuses.HP;

	max_hp += GroupLeadershipAAHealthEnhancement();

	max_hp += max_hp * (spellbonuses.MaxHPChange + itembonuses.MaxHPChange) / 10000;

	if (cur_hp > max_hp)
		cur_hp = max_hp;

	int hp_perc_cap = spellbonuses.HPPercCap;
	if(hp_perc_cap) {
		int curHP_cap = (max_hp * hp_perc_cap) / 100;
		if (cur_hp > curHP_cap)
			cur_hp = curHP_cap;
	}

	return max_hp;
}

uint16 Mob::GetClassLevelFactor(){
	uint16 multiplier = 0;
	uint8 mlevel=GetLevel();
	switch(GetClass())
	{
		case WARRIOR:{
			if (mlevel < 20)
				multiplier = 220;
			else if (mlevel < 30)
				multiplier = 230;
			else if (mlevel < 40)
				multiplier = 250;
			else if (mlevel < 53)
				multiplier = 270;
			else if (mlevel < 57)
				multiplier = 280;
			else if (mlevel < 60)
				multiplier = 290;
			else if (mlevel < 70)
				multiplier = 300;
			else
				multiplier = 311;
			break;
		}
		case DRUID:
		case CLERIC:
		case SHAMAN:{
			if (mlevel < 70)
				multiplier = 150;
			else
				multiplier = 157;
			break;
		}
		case BERSERKER:
		case PALADIN:
		case SHADOWKNIGHT:{
			if (mlevel < 35)
				multiplier = 210;
			else if (mlevel < 45)
				multiplier = 220;
			else if (mlevel < 51)
				multiplier = 230;
			else if (mlevel < 56)
				multiplier = 240;
			else if (mlevel < 60)
				multiplier = 250;
			else if (mlevel < 68)
				multiplier = 260;
			else
				multiplier = 270;
			break;
		}
		case MONK:
		case BARD:
		case ROGUE:
		case BEASTLORD:{
			if (mlevel < 51)
				multiplier = 180;
			else if (mlevel < 58)
				multiplier = 190;
			else if (mlevel < 70)
				multiplier = 200;
			else
				multiplier = 210;
			break;
		}
		case RANGER:{
			if (mlevel < 58)
				multiplier = 200;
			else if (mlevel < 70)
				multiplier = 210;
			else
				multiplier = 220;
			break;
		}
		case MAGICIAN:
		case WIZARD:
		case NECROMANCER:
		case ENCHANTER:{
			if (mlevel < 70)
				multiplier = 120;
			else
				multiplier = 127;
			break;
		}
		default:{
			if (mlevel < 35)
				multiplier = 210;
			else if (mlevel < 45)
				multiplier = 220;
			else if (mlevel < 51)
				multiplier = 230;
			else if (mlevel < 56)
				multiplier = 240;
			else if (mlevel < 60)
				multiplier = 250;
			else
				multiplier = 260;
			break;
		}
	}
	return multiplier;
}

int32 Client::CalcBaseHP()
{
	if(GetClientVersion() >= EQClientSoD && RuleB(Character, SoDClientUseSoDHPManaEnd)) {
		float SoDPost255;
		uint16 NormalSTA = GetSTA();

		if(((NormalSTA - 255) / 2) > 0)
			SoDPost255 = ((NormalSTA - 255) / 2);
		else
			SoDPost255 = 0;

		int hp_factor = GetClassHPFactor();

		if (level < 41) {
			base_hp = (5 + (GetLevel() * hp_factor / 12) +
				((NormalSTA - SoDPost255) * GetLevel() * hp_factor / 3600));
		}
		else if (level < 81) {
			base_hp = (5 + (40 * hp_factor / 12) + ((GetLevel() - 40) * hp_factor / 6) +
				((NormalSTA - SoDPost255) * hp_factor / 90) +
				((NormalSTA - SoDPost255) * (GetLevel() - 40) * hp_factor / 1800));
		}
		else {
			base_hp = (5 + (80 * hp_factor / 8) + ((GetLevel() - 80) * hp_factor / 10) +
				((NormalSTA - SoDPost255) * hp_factor / 90) +
				((NormalSTA - SoDPost255) * hp_factor / 45));
		}

		base_hp += (GetHeroicSTA() * 10);

	}
	else {
		uint16 Post255;
		uint16 lm=GetClassLevelFactor();
		if((GetSTA()-255)/2 > 0)
			Post255 = (GetSTA()-255)/2;
		else
			Post255 = 0;

		base_hp = (5)+(GetLevel()*lm/10) + (((GetSTA()-Post255)*GetLevel()*lm/3000)) + ((Post255*GetLevel())*lm/6000);
	}
	return base_hp;
}

// This is for calculating Base HPs + STA bonus for SoD or later clients.
uint32 Client::GetClassHPFactor() {

	int factor;

	// Note: Base HP factor under level 41 is equal to factor / 12, and from level 41 to 80 is factor / 6.
	// Base HP over level 80 is factor / 10
	// HP per STA point per level is factor / 30 for level 80+
	// HP per STA under level 40 is the level 80 HP Per STA / 120, and for over 40 it is / 60.

	switch(GetClass())
	{
		case DRUID:
		case ENCHANTER:
		case NECROMANCER:
		case MAGICIAN:
		case WIZARD:
			factor = 240;
			break;
		case BEASTLORD:
		case BERSERKER:
		case MONK:
		case ROGUE:
		case SHAMAN:
			factor = 255;
			break;
		case BARD:
		case CLERIC:
			factor = 264;
			break;
		case SHADOWKNIGHT:
		case PALADIN:
			factor = 288;
			break;
		case RANGER:
			factor = 276;
			break;
		case WARRIOR:
			factor = 300;
			break;
		default:
			factor = 240;
			break;
	}
	return factor;
}

// This should return the combined AC of all the items the player is wearing.
int16 Client::GetRawItemAC() {
	int16 Total = 0;

	for (int16 slot_id=0; slot_id<21; slot_id++) {
		const ItemInst* inst = m_inv[slot_id];
		if (inst && inst->IsType(ItemClassCommon)) {
			Total += inst->GetItem()->AC;
		}
	}

	return Total;
}

int16 Client::acmod() {
	int agility = GetAGI();
	int level = GetLevel();
	if(agility < 1 || level < 1)
		return(0);

	if (agility <=74){
		if (agility == 1)
			return -24;
		else if (agility <=3)
			return -23;
		else if (agility == 4)
			return -22;
		else if (agility <=6)
			return -21;
		else if (agility <=8)
			return -20;
		else if (agility == 9)
			return -19;
		else if (agility <=11)
			return -18;
		else if (agility == 12)
			return -17;
		else if (agility <=14)
			return -16;
		else if (agility <=16)
			return -15;
		else if (agility == 17)
			return -14;
		else if (agility <=19)
			return -13;
		else if (agility == 20)
			return -12;
		else if (agility <=22)
			return -11;
		else if (agility <=24)
			return -10;
		else if (agility == 25)
			return -9;
		else if (agility <=27)
			return -8;
		else if (agility == 28)
			return -7;
		else if (agility <=30)
			return -6;
		else if (agility <=32)
			return -5;
		else if (agility == 33)
			return -4;
		else if (agility <=35)
			return -3;
		else if (agility == 36)
			return -2;
		else if (agility <=38)
			return -1;
		else if (agility <=65)
			return 0;
		else if (agility <=70)
			return 1;
		else if (agility <=74)
			return 5;
	}
	else if(agility <= 137) {
		if (agility == 75){
			if (level <= 6)
				return 9;
			else if (level <= 19)
				return 23;
			else if (level <= 39)
				return 33;
			else
				return 39;
		}
		else if (agility >= 76 && agility <= 79){
			if (level <= 6)
				return 10;
			else if (level <= 19)
				return 23;
			else if (level <= 39)
				return 33;
			else
				return 40;
		}
		else if (agility == 80){
			if (level <= 6)
				return 11;
			else if (level <= 19)
				return 24;
			else if (level <= 39)
				return 34;
			else
				return 41;
		}
		else if (agility >= 81 && agility <= 85){
			if (level <= 6)
				return 12;
			else if (level <= 19)
				return 25;
			else if (level <= 39)
				return 35;
			else
				return 42;
		}
		else if (agility >= 86 && agility <= 90){
			if (level <= 6)
				return 12;
			else if (level <= 19)
				return 26;
			else if (level <= 39)
				return 36;
			else
				return 42;
		}
		else if (agility >= 91 && agility <= 95){
			if (level <= 6)
				return 13;
			else if (level <= 19)
				return 26;
			else if (level <= 39)
				return 36;
			else
				return 43;
		}
		else if (agility >= 96 && agility <= 99){
			if (level <= 6)
				return 14;
			else if (level <= 19)
				return 27;
			else if (level <= 39)
				return 37;
			else
				return 44;
		}
		else if (agility == 100 && level >= 7){
			if (level <= 19)
				return 28;
			else if (level <= 39)
				return 38;
			else
				return 45;
		}
		else if (level <= 6) {
			return 15;
		}
		//level is >6
		else if (agility >= 101 && agility <= 105){
			if (level <= 19)
				return 29;
			else if (level <= 39)
				return 39;// not verified
			else
				return 45;
		}
		else if (agility >= 106 && agility <= 110){
			if (level <= 19)
				return 29;
			else if (level <= 39)
				return 39;// not verified
			else
				return 46;
		}
		else if (agility >= 111 && agility <= 115){
			if (level <= 19)
				return 30;
			else if (level <= 39)
				return 40;// not verified
			else
				return 47;
		}
		else if (agility >= 116 && agility <= 119){
			if (level <= 19)
				return 31;
			else if (level <= 39)
				return 41;
			else
				return 47;
		}
		else if (level <= 19) {
				return 32;
		}
		//level is > 19
		else if (agility == 120){
			if (level <= 39)
				return 42;
			else
				return 48;
		}
		else if (agility <= 125){
			if (level <= 39)
				return 42;
			else
				return 49;
		}
		else if (agility <= 135){
			if (level <= 39)
				return 42;
			else
				return 50;
		}
		else {
			if (level <= 39)
				return 42;
			else
				return 51;
		}
	} else if(agility <= 300) {
		if(level <= 6) {
			if(agility <= 139)
				return(21);
			else if(agility == 140)
				return(22);
			else if(agility <= 145)
				return(23);
			else if(agility <= 150)
				return(23);
			else if(agility <= 155)
				return(24);
			else if(agility <= 159)
				return(25);
			else if(agility == 160)
				return(26);
			else if(agility <= 165)
				return(26);
			else if(agility <= 170)
				return(27);
			else if(agility <= 175)
				return(28);
			else if(agility <= 179)
				return(28);
			else if(agility == 180)
				return(29);
			else if(agility <= 185)
				return(30);
			else if(agility <= 190)
				return(31);
			else if(agility <= 195)
				return(31);
			else if(agility <= 199)
				return(32);
			else if(agility <= 219)
				return(33);
			else if(agility <= 239)
				return(34);
			else
				return(35);
		} else if(level <= 19) {
			if(agility <= 139)
				return(34);
			else if(agility == 140)
				return(35);
			else if(agility <= 145)
				return(36);
			else if(agility <= 150)
				return(37);
			else if(agility <= 155)
				return(37);
			else if(agility <= 159)
				return(38);
			else if(agility == 160)
				return(39);
			else if(agility <= 165)
				return(40);
			else if(agility <= 170)
				return(40);
			else if(agility <= 175)
				return(41);
			else if(agility <= 179)
				return(42);
			else if(agility == 180)
				return(43);
			else if(agility <= 185)
				return(43);
			else if(agility <= 190)
				return(44);
			else if(agility <= 195)
				return(45);
			else if(agility <= 199)
				return(45);
			else if(agility <= 219)
				return(46);
			else if(agility <= 239)
				return(47);
			else
				return(48);
		} else if(level <= 39) {
			if(agility <= 139)
				return(44);
			else if(agility == 140)
				return(45);
			else if(agility <= 145)
				return(46);
			else if(agility <= 150)
				return(47);
			else if(agility <= 155)
				return(47);
			else if(agility <= 159)
				return(48);
			else if(agility == 160)
				return(49);
			else if(agility <= 165)
				return(50);
			else if(agility <= 170)
				return(50);
			else if(agility <= 175)
				return(51);
			else if(agility <= 179)
				return(52);
			else if(agility == 180)
				return(53);
			else if(agility <= 185)
				return(53);
			else if(agility <= 190)
				return(54);
			else if(agility <= 195)
				return(55);
			else if(agility <= 199)
				return(55);
			else if(agility <= 219)
				return(56);
			else if(agility <= 239)
				return(57);
			else
				return(58);
		} else {	//lvl >= 40
			if(agility <= 139)
				return(51);
			else if(agility == 140)
				return(52);
			else if(agility <= 145)
				return(53);
			else if(agility <= 150)
				return(53);
			else if(agility <= 155)
				return(54);
			else if(agility <= 159)
				return(55);
			else if(agility == 160)
				return(56);
			else if(agility <= 165)
				return(56);
			else if(agility <= 170)
				return(57);
			else if(agility <= 175)
				return(58);
			else if(agility <= 179)
				return(58);
			else if(agility == 180)
				return(59);
			else if(agility <= 185)
				return(60);
			else if(agility <= 190)
				return(61);
			else if(agility <= 195)
				return(61);
			else if(agility <= 199)
				return(62);
			else if(agility <= 219)
				return(63);
			else if(agility <= 239)
				return(64);
			else
				return(65);
		}
	}
	else{
		//seems about 21 agil per extra AC pt over 300...
	return (65 + ((agility-300) / 21));
	}
#if EQDEBUG >= 11
	LogFile->write(EQEMuLog::Error, "Error in Client::acmod(): Agility: %i, Level: %i",agility,level);
#endif
	return 0;
};

// This is a testing formula for AC, the value this returns should be the same value as the one the client shows...
// ac1 and ac2 are probably the damage migitation and damage avoidance numbers, not sure which is which.
// I forgot to include the iksar defense bonus and i cant find my notes now...
// AC from spells are not included (cant even cast spells yet..)
int16 Client::CalcAC() {

	// new formula
	int avoidance = (acmod() + ((GetSkill(DEFENSE) + itembonuses.HeroicAGI/10)*16)/9);
	if (avoidance < 0)
		avoidance = 0;

	int mitigation = 0;
	if (m_pp.class_ == WIZARD || m_pp.class_ == MAGICIAN || m_pp.class_ == NECROMANCER || m_pp.class_ == ENCHANTER) {
		//something is wrong with this, naked casters have the wrong natural AC
//		mitigation = (spellbonuses.AC/3) + (GetSkill(DEFENSE)/2) + (itembonuses.AC+1);
		mitigation = (GetSkill(DEFENSE) + itembonuses.HeroicAGI/10)/4 + (itembonuses.AC+1);
		//this might be off by 4..
		mitigation -= 4;
	} else {
//		mitigation = (spellbonuses.AC/4) + (GetSkill(DEFENSE)/3) + ((itembonuses.AC*4)/3);
		mitigation = (GetSkill(DEFENSE) + itembonuses.HeroicAGI/10)/3 + ((itembonuses.AC*4)/3);
		if(m_pp.class_ == MONK)
			mitigation += GetLevel() * 13/10;	//the 13/10 might be wrong, but it is close...
	}
	int displayed = 0;
	displayed += ((avoidance+mitigation)*1000)/847;	//natural AC

	//Iksar AC, untested
	if (GetRace() == IKSAR) {
		displayed += 12;
		int iksarlevel = GetLevel();
		iksarlevel -= 10;
		if (iksarlevel > 25)
			iksarlevel = 25;
		if (iksarlevel > 0)
			displayed += iksarlevel * 12 / 10;
	}

	// Shield AC bonus for HeroicSTR
	if(itembonuses.HeroicSTR) {
		bool equiped = CastToClient()->m_inv.GetItem(14);
		if(equiped) {
			uint8 shield = CastToClient()->m_inv.GetItem(14)->GetItem()->ItemType;
			if(shield == ItemTypeShield)
				displayed += itembonuses.HeroicSTR/2;
		}
	}

	//spell AC bonuses are added directly to natural total
	displayed += spellbonuses.AC;

	AC = displayed;
	return(AC);
}

int16 Client::GetACMit() {

	int mitigation = 0;
	if (m_pp.class_ == WIZARD || m_pp.class_ == MAGICIAN || m_pp.class_ == NECROMANCER || m_pp.class_ == ENCHANTER) {
		mitigation = (GetSkill(DEFENSE) + itembonuses.HeroicAGI/10)/4 + (itembonuses.AC+1);
		mitigation -= 4;
	}
	else {
		mitigation = (GetSkill(DEFENSE) + itembonuses.HeroicAGI/10)/3 + ((itembonuses.AC*4)/3);
		if(m_pp.class_ == MONK)
			mitigation += GetLevel() * 13/10;	//the 13/10 might be wrong, but it is close...
	}

	// Shield AC bonus for HeroicSTR
	if(itembonuses.HeroicSTR) {
		bool equiped = CastToClient()->m_inv.GetItem(14);
		if(equiped) {
			uint8 shield = CastToClient()->m_inv.GetItem(14)->GetItem()->ItemType;
			if(shield == ItemTypeShield)
				mitigation += itembonuses.HeroicSTR/2;
		}
	}

	return(mitigation*1000/847);
}

int16 Client::GetACAvoid() {

	int avoidance = (acmod() + ((GetSkill(DEFENSE) + itembonuses.HeroicAGI/10)*16)/9);
	if (avoidance < 0)
		avoidance = 0;

	return(avoidance*1000/847);
}

int32 Client::CalcMaxMana()
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
			LogFile->write(EQEMuLog::Debug, "Invalid Class '%c' in CalcMaxMana", GetCasterClass());
			max_mana = 0;
			break;
		}
	}
	if (max_mana < 0) {
		max_mana = 0;
	}

	if (cur_mana > max_mana) {
		cur_mana = max_mana;
	}

	int mana_perc_cap = spellbonuses.ManaPercCap;
	if(mana_perc_cap) {
		int curMana_cap = (max_mana * mana_perc_cap) / 100;
		if (cur_mana > curMana_cap)
			cur_mana = curMana_cap;
	}

#if EQDEBUG >= 11
	LogFile->write(EQEMuLog::Debug, "Client::CalcMaxMana() called for %s - returning %d", GetName(), max_mana);
#endif
	return max_mana;
}

int32 Client::CalcBaseMana()
{
	int WisInt = 0;
	int MindLesserFactor, MindFactor;
	int32 max_m = 0;
	int wisint_mana = 0;
	int base_mana = 0;
	int ConvertedWisInt = 0;
	switch(GetCasterClass())
	{
		case 'I':
			WisInt = GetINT();

			if (GetClientVersion() >= EQClientSoD && RuleB(Character, SoDClientUseSoDHPManaEnd)) {

				if (WisInt > 100) {
					ConvertedWisInt = (((WisInt - 100) * 5 / 2) + 100);
					if (WisInt > 201) {
						ConvertedWisInt -= ((WisInt - 201) * 5 / 4);
					}
				}
				else {
					ConvertedWisInt = WisInt;
				}

				if (GetLevel() < 41) {
					wisint_mana = (GetLevel() * 75 * ConvertedWisInt / 1000);
					base_mana = (GetLevel() * 15);
				}
				else if (GetLevel() < 81) {
					wisint_mana = ((3 * ConvertedWisInt) + ((GetLevel() - 40) * 15 * ConvertedWisInt / 100));
					base_mana = (600 + ((GetLevel() - 40) * 30));
				}
				else {
					wisint_mana = (9 * ConvertedWisInt);
					base_mana = (1800 + ((GetLevel() - 80) * 18));
				}
				max_m = base_mana + wisint_mana + (GetHeroicINT() * 10);
			}
			else
			{
				if((( WisInt - 199 ) / 2) > 0)
					MindLesserFactor = ( WisInt - 199 ) / 2;
				else
					MindLesserFactor = 0;

				MindFactor = WisInt - MindLesserFactor;
				if(WisInt > 100)
					max_m = (((5 * (MindFactor + 20)) / 2) * 3 * GetLevel() / 40);
				else
					max_m = (((5 * (MindFactor + 200)) / 2) * 3 * GetLevel() / 100);
			}
			break;

		case 'W':
			WisInt = GetWIS();

			if (GetClientVersion() >= EQClientSoD && RuleB(Character, SoDClientUseSoDHPManaEnd)) {

				if (WisInt > 100) {
					ConvertedWisInt = (((WisInt - 100) * 5 / 2) + 100);
					if (WisInt > 201) {
						ConvertedWisInt -= ((WisInt - 201) * 5 / 4);
					}
				}
				else {
					ConvertedWisInt = WisInt;
				}

				if (GetLevel() < 41) {
					wisint_mana = (GetLevel() * 75 * ConvertedWisInt / 1000);
					base_mana = (GetLevel() * 15);
				}
				else if (GetLevel() < 81) {
					wisint_mana = ((3 * ConvertedWisInt) + ((GetLevel() - 40) * 15 * ConvertedWisInt / 100));
					base_mana = (600 + ((GetLevel() - 40) * 30));
				}
				else {
					wisint_mana = (9 * ConvertedWisInt);
					base_mana = (1800 + ((GetLevel() - 80) * 18));
				}
				max_m = base_mana + wisint_mana + (GetHeroicWIS() * 10);
			}
			else
			{
				if((( WisInt - 199 ) / 2) > 0)
					MindLesserFactor = ( WisInt - 199 ) / 2;
				else
					MindLesserFactor = 0;

				MindFactor = WisInt - MindLesserFactor;
				if(WisInt > 100)
					max_m = (((5 * (MindFactor + 20)) / 2) * 3 * GetLevel() / 40);
				else
					max_m = (((5 * (MindFactor + 200)) / 2) * 3 * GetLevel() / 100);
			}
			break;

		case 'N': {
			max_m = 0;
			break;
		}
		default: {
			LogFile->write(EQEMuLog::Debug, "Invalid Class '%c' in CalcMaxMana", GetCasterClass());
			max_m = 0;
			break;
		}
	}

#if EQDEBUG >= 11
	LogFile->write(EQEMuLog::Debug, "Client::CalcBaseMana() called for %s - returning %d", GetName(), max_m);
#endif
	return max_m;
}

int32 Client::CalcBaseManaRegen()
{
	uint8 clevel = GetLevel();
	int32 regen = 0;
	if (IsSitting() || (GetHorseId() != 0))
	{
		if(HasSkill(MEDITATE))
			regen = (((GetSkill(MEDITATE) / 10) + (clevel - (clevel / 4))) / 4) + 4;
		else
			regen = 2;
	}
	else {
		regen = 2;
	}
	return regen;
}

int32 Client::CalcManaRegen()
{
	uint8 clevel = GetLevel();
	int32 regen = 0;
	//this should be changed so we dont med while camping, etc...
	if (IsSitting() || (GetHorseId() != 0))
	{
		BuffFadeBySitModifier();
		if(HasSkill(MEDITATE)) {
			this->medding = true;
			regen = (((GetSkill(MEDITATE) / 10) + (clevel - (clevel / 4))) / 4) + 4;
			regen += spellbonuses.ManaRegen + itembonuses.ManaRegen;
			CheckIncreaseSkill(MEDITATE, nullptr, -5);
		}
		else
			regen = 2 + spellbonuses.ManaRegen + itembonuses.ManaRegen;
	}
	else {
		this->medding = false;
		regen = 2 + spellbonuses.ManaRegen + itembonuses.ManaRegen;
	}

	//AAs
	regen += aabonuses.ManaRegen;

	return (regen * RuleI(Character, ManaRegenMultiplier) / 100);
}

int32 Client::CalcManaRegenCap()
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

uint32 Client::CalcCurrentWeight() {

	const Item_Struct* TempItem = 0;
	ItemInst* ins;
	uint32 Total = 0;
	int x;
	for(x = 0; x <= 30; x++)
	{
		TempItem = 0;
		ins = GetInv().GetItem(x);
		if (ins)
			TempItem = ins->GetItem();
		if (TempItem)
			Total += TempItem->Weight;
	}
	for (x = 251; x < 331; x++)
	{
		int TmpWeight = 0;
		TempItem = 0;
		ins = GetInv().GetItem(x);
		if (ins)
			TempItem = ins->GetItem();
		if (TempItem)
			TmpWeight = TempItem->Weight;
		if (TmpWeight > 0)
		{
			int bagslot = 22;
			int reduction = 0;
			for (int m = 261; m < 331; m += 10)
			{
				if (x >= m)
					bagslot += 1;
			}
			ItemInst* baginst = GetInv().GetItem(bagslot);
			if (baginst && baginst->GetItem() && baginst->IsType(ItemClassContainer))
				reduction = baginst->GetItem()->BagWR;
			if (reduction > 0)
				TmpWeight -= TmpWeight*reduction/100;
			Total += TmpWeight;
		}
	}

	//TODO: coin weight reduction (from purses, etc), since client already calculates it
	/*From the Wiki http://www.eqemulator.net/wiki/wikka.php?wakka=EQEmuDBSchemaitems under bagwr (thanks Trevius):
	Interestingly, you can also have bags that reduce coin weight. However, in order to set bags to reduce coin weight, you MUST set the Item ID somewhere between 17201 and 17230. This is hard coded into the client.
	The client is set to have certain coin weight reduction on a per Item ID basis within this range. The best way to create an new item to reduce coin weight is to examine existing bags in this range.
	Search for the words "coin purse" with the #finditem command in game and the Bag WR setting on those bags is the amount they will reduce coin weight. It is easiest to overwrite one of those bags if you wish to create one with the
	same weight reduction amount for coins. You can use other Item IDs in this range for setting coin weight reduction, but by using an existing item, at least you will know the amount the client will reduce it by before you create it.
	This is the ONLY instance I have seen where the client is hard coded to particular Item IDs to set a certain property for an item. It is very odd.
	*/

	// SoD client has no weight for coin
	if (GetClientVersion() < EQClientSoD) {
		Total += (m_pp.platinum + m_pp.gold + m_pp.silver + m_pp.copper) / 4;
	}

	float Packrat = (float)spellbonuses.Packrat + (float)aabonuses.Packrat;
	if (Packrat > 0)
		Total = (uint32)((float)Total * (1.0f - ((Packrat * 1.0f) / 100.0f)));	//AndMetal: 1% per level, up to 5% (calculated from Titanium client). verified thru client that it reduces coin weight by the same %
																				//without casting to float & back to uint32, this didn't work right
	return Total;
}

int16 Client::CalcAlcoholPhysicalEffect()
{
	if(m_pp.intoxication <= 55)
		return 0;

	return (m_pp.intoxication - 40) / 16;
}

int16 Client::CalcSTR() {
	int16 val = m_pp.STR + itembonuses.STR + spellbonuses.STR + CalcAlcoholPhysicalEffect();

	int16 mod = aabonuses.STR;

	if(val>255 && GetLevel() <= 60)
		val = 255;
	STR = val + mod;

	if(STR < 1)
		STR = 1;

	int m = GetMaxSTR();
	if(STR > m)
		STR = m;

	return(STR);
}

int16 Client::CalcSTA() {
	int16 val = m_pp.STA + itembonuses.STA + spellbonuses.STA + CalcAlcoholPhysicalEffect();;

	int16 mod = aabonuses.STA;

	if(val>255 && GetLevel() <= 60)
		val = 255;
	STA = val + mod;

	if(STA < 1)
		STA = 1;

	int m = GetMaxSTA();
	if(STA > m)
		STA = m;

	return(STA);
}

int16 Client::CalcAGI() {
	int16 val = m_pp.AGI + itembonuses.AGI + spellbonuses.AGI - CalcAlcoholPhysicalEffect();;
	int16 mod = aabonuses.AGI;

	if(val>255 && GetLevel() <= 60)
		val = 255;

	int16 str = GetSTR();

	//Encumbered penalty
	if(weight > (str * 10)) {
		//AGI is halved when we double our weight, zeroed (defaults to 1) when we triple it. this includes AGI from AAs
		float total_agi = float(val + mod);
		float str_float = float(str);
		AGI = (int16)(((-total_agi) / (str_float * 2)) * (((float)weight / 10) - str_float) + total_agi);	//casting to an int assumes this will be floor'd. without using floats & casting to int16, the calculation doesn't work right
	} else
		AGI = val + mod;

	if(AGI < 1)
		AGI = 1;

	int m = GetMaxAGI();
	if(AGI > m)
		AGI = m;

	return(AGI);
}

int16 Client::CalcDEX() {
	int16 val = m_pp.DEX + itembonuses.DEX + spellbonuses.DEX - CalcAlcoholPhysicalEffect();;

	int16 mod = aabonuses.DEX;

	if(val>255 && GetLevel() <= 60)
		val = 255;
	DEX = val + mod;

	if(DEX < 1)
		DEX = 1;

	int m = GetMaxDEX();
	if(DEX > m)
		DEX = m;

	return(DEX);
}

int16 Client::CalcINT() {
	int16 val = m_pp.INT + itembonuses.INT + spellbonuses.INT;

	int16 mod = aabonuses.INT;

	if(val>255 && GetLevel() <= 60)
		val = 255;
	INT = val + mod;

	if(m_pp.intoxication)
	{
		int16 AlcINT = INT - (int16)((float)m_pp.intoxication / 200.0f * (float)INT) - 1;

		if((AlcINT < (int)(0.2 * INT)))
			INT = (int)(0.2f * (float)INT);
		else
			INT = AlcINT;
	}

	if(INT < 1)
		INT = 1;

	int m = GetMaxINT();
	if(INT > m)
		INT = m;

	return(INT);
}

int16 Client::CalcWIS() {
	int16 val = m_pp.WIS + itembonuses.WIS + spellbonuses.WIS;

	int16 mod = aabonuses.WIS;

	if(val>255 && GetLevel() <= 60)
		val = 255;
	WIS = val + mod;

	if(m_pp.intoxication)
	{
		int16 AlcWIS = WIS - (int16)((float)m_pp.intoxication / 200.0f * (float)WIS) - 1;

		if((AlcWIS < (int)(0.2 * WIS)))
			WIS = (int)(0.2f * (float)WIS);
		else
			WIS = AlcWIS;
	}

	if(WIS < 1)
		WIS = 1;

	int m = GetMaxWIS();
	if(WIS > m)
		WIS = m;

	return(WIS);
}

int16 Client::CalcCHA() {
	int16 val = m_pp.CHA + itembonuses.CHA + spellbonuses.CHA;

	int16 mod = aabonuses.CHA;

	if(val>255 && GetLevel() <= 60)
		val = 255;
	CHA = val + mod;

	if(CHA < 1)
		CHA = 1;

	int m = GetMaxCHA();
	if(CHA > m)
		CHA = m;

	return(CHA);
}

int Client::CalcHaste() {
	int h = spellbonuses.haste + spellbonuses.hastetype2 + itembonuses.haste;
	int cap = 0;
	int level = GetLevel();
	/*
	if(disc_inuse == discBlindingSpeed) {
		if(!disc_elapse.Check(false)) {
			h += 20;		//this ammount is completely unknown
		} else {
			disc_inuse = discNone;
		}
	} */

	if(level < 30) { // Rogean: Are these caps correct? Will use for now.
		cap = 50;
	} else if(level < 50) {
		cap = 74;
	} else if(level < 55) {
		cap = 84;
	} else if(level < 60) {
		cap = 94;
	} else {
		cap = RuleI(Character, HasteCap);
	}

	if(h > cap) h = cap;

	h += spellbonuses.hastetype3;
	h += ExtraHaste;	//GM granted haste.

	h = mod_client_haste(h);

	if (spellbonuses.inhibitmelee){
		if (h >= 0)
			h -= spellbonuses.inhibitmelee;

		else
			h -=((100+h)*spellbonuses.inhibitmelee/100);
	}

	Haste = h;
	return(Haste);
}

//The AA multipliers are set to be 5, but were 2 on WR
//The resistant discipline which I think should be here is implemented
//in Mob::ResistSpell
int16	Client::CalcMR()
{
	//racial bases
	switch(GetBaseRace()) {
		case HUMAN:
			MR = 25;
			break;
		case BARBARIAN:
			MR = 25;
			break;
		case ERUDITE:
			MR = 30;
			break;
		case WOOD_ELF:
			MR = 25;
			break;
		case HIGH_ELF:
			MR = 25;
			break;
		case DARK_ELF:
			MR = 25;
			break;
		case HALF_ELF:
			MR = 25;
			break;
		case DWARF:
			MR = 30;
			break;
		case TROLL:
			MR = 25;
			break;
		case OGRE:
			MR = 25;
			break;
		case HALFLING:
			MR = 25;
			break;
		case GNOME:
			MR = 25;
			break;
		case IKSAR:
			MR = 25;
			break;
		case VAHSHIR:
			MR = 25;
			break;
		case FROGLOK:
			MR = 30;
			break;
		case DRAKKIN:
			MR = 35;
			break;
		default:
			MR = 20;
	}

	MR += itembonuses.MR + spellbonuses.MR + aabonuses.MR;

	if(GetClass() == WARRIOR)
		MR += GetLevel() / 2;

	if(MR < 1)
		MR = 1;

	if(MR > GetMaxMR())
		MR = GetMaxMR();

	return(MR);
}

int16	Client::CalcFR()
{
	//racial bases
	switch(GetBaseRace()) {
		case HUMAN:
			FR = 25;
			break;
		case BARBARIAN:
			FR = 25;
			break;
		case ERUDITE:
			FR = 25;
			break;
		case WOOD_ELF:
			FR = 25;
			break;
		case HIGH_ELF:
			FR = 25;
			break;
		case DARK_ELF:
			FR = 25;
			break;
		case HALF_ELF:
			FR = 25;
			break;
		case DWARF:
			FR = 25;
			break;
		case TROLL:
			FR = 5;
			break;
		case OGRE:
			FR = 25;
			break;
		case HALFLING:
			FR = 25;
			break;
		case GNOME:
			FR = 25;
			break;
		case IKSAR:
			FR = 30;
			break;
		case VAHSHIR:
			FR = 25;
			break;
		case FROGLOK:
			FR = 25;
			break;
		case DRAKKIN:
			FR = 25;
			break;
		default:
			FR = 20;
	}

	int c = GetClass();
	if(c == RANGER) {
		FR += 4;

		int l = GetLevel();
		if(l > 49)
			FR += l - 49;
	}

	FR += itembonuses.FR + spellbonuses.FR + aabonuses.FR;

	if(FR < 1)
		FR = 1;

	if(FR > GetMaxFR())
		FR = GetMaxFR();

	return(FR);
}

int16	Client::CalcDR()
{
	//racial bases
	switch(GetBaseRace()) {
		case HUMAN:
			DR = 15;
			break;
		case BARBARIAN:
			DR = 15;
			break;
		case ERUDITE:
			DR = 10;
			break;
		case WOOD_ELF:
			DR = 15;
			break;
		case HIGH_ELF:
			DR = 15;
			break;
		case DARK_ELF:
			DR = 15;
			break;
		case HALF_ELF:
			DR = 15;
			break;
		case DWARF:
			DR = 15;
			break;
		case TROLL:
			DR = 15;
			break;
		case OGRE:
			DR = 15;
			break;
		case HALFLING:
			DR = 20;
			break;
		case GNOME:
			DR = 15;
			break;
		case IKSAR:
			DR = 15;
			break;
		case VAHSHIR:
			DR = 15;
			break;
		case FROGLOK:
			DR = 15;
			break;
		case DRAKKIN:
			DR = 15;
			break;
		default:
			DR = 15;
	}

	int c = GetClass();
	if(c == PALADIN) {
		DR += 8;

		int l = GetLevel();
		if(l > 49)
			DR += l - 49;

	} else if(c == SHADOWKNIGHT) {
		DR += 4;

		int l = GetLevel();
		if(l > 49)
			DR += l - 49;
	}

	DR += itembonuses.DR + spellbonuses.DR + aabonuses.DR;

	if(DR < 1)
		DR = 1;

	if(DR > GetMaxDR())
		DR = GetMaxDR();

	return(DR);
}

int16	Client::CalcPR()
{
	//racial bases
	switch(GetBaseRace()) {
		case HUMAN:
			PR = 15;
			break;
		case BARBARIAN:
			PR = 15;
			break;
		case ERUDITE:
			PR = 15;
			break;
		case WOOD_ELF:
			PR = 15;
			break;
		case HIGH_ELF:
			PR = 15;
			break;
		case DARK_ELF:
			PR = 15;
			break;
		case HALF_ELF:
			PR = 15;
			break;
		case DWARF:
			PR = 20;
			break;
		case TROLL:
			PR = 15;
			break;
		case OGRE:
			PR = 15;
			break;
		case HALFLING:
			PR = 20;
			break;
		case GNOME:
			PR = 15;
			break;
		case IKSAR:
			PR = 15;
			break;
		case VAHSHIR:
			PR = 15;
			break;
		case FROGLOK:
			PR = 30;
			break;
		case DRAKKIN:
			PR = 15;
			break;
		default:
			PR = 15;
	}

	int c = GetClass();
	if(c == ROGUE) {
		PR += 8;

		int l = GetLevel();
		if(l > 49)
			PR += l - 49;

	} else if(c == SHADOWKNIGHT) {
		PR += 4;

		int l = GetLevel();
		if(l > 49)
			PR += l - 49;
	}

	PR += itembonuses.PR + spellbonuses.PR + aabonuses.PR;

	if(PR < 1)
		PR = 1;

	if(PR > GetMaxPR())
		PR = GetMaxPR();

	return(PR);
}

int16	Client::CalcCR()
{
	//racial bases
	switch(GetBaseRace()) {
		case HUMAN:
			CR = 25;
			break;
		case BARBARIAN:
			CR = 35;
			break;
		case ERUDITE:
			CR = 25;
			break;
		case WOOD_ELF:
			CR = 25;
			break;
		case HIGH_ELF:
			CR = 25;
			break;
		case DARK_ELF:
			CR = 25;
			break;
		case HALF_ELF:
			CR = 25;
			break;
		case DWARF:
			CR = 25;
			break;
		case TROLL:
			CR = 25;
			break;
		case OGRE:
			CR = 25;
			break;
		case HALFLING:
			CR = 25;
			break;
		case GNOME:
			CR = 25;
			break;
		case IKSAR:
			CR = 15;
			break;
		case VAHSHIR:
			CR = 25;
			break;
		case FROGLOK:
			CR = 25;
			break;
		case DRAKKIN:
			CR = 25;
			break;
		default:
			CR = 25;
	}

	int c = GetClass();
	if(c == RANGER) {
		CR += 4;

		int l = GetLevel();
		if(l > 49)
			CR += l - 49;
	}

	CR += itembonuses.CR + spellbonuses.CR + aabonuses.CR;

	if(CR < 1)
		CR = 1;

	if(CR > GetMaxCR())
		CR = GetMaxCR();

	return(CR);
}

int16	Client::CalcCorrup()
{
	Corrup = GetBaseCorrup() + itembonuses.Corrup + spellbonuses.Corrup + aabonuses.Corrup;

	if(Corrup > GetMaxCorrup())
		Corrup = GetMaxCorrup();

	return(Corrup);
}

int16 Client::CalcATK() {
	ATK = itembonuses.ATK + spellbonuses.ATK + aabonuses.ATK + GroupLeadershipAAOffenseEnhancement();
	return(ATK);
}

uint16 Mob::GetInstrumentMod(uint16 spell_id) const {
	if(GetClass() != BARD)
		return(10);

	uint16 effectmod = 10;

	//this should never use spell modifiers...
	//if a spell grants better modifers, they are copied into the item mods
	//because the spells are supposed to act just like having the intrument.

	//item mods are in 10ths of percent increases
	switch(spells[spell_id].skill) {
		case PERCUSSION_INSTRUMENTS:
			if(itembonuses.percussionMod == 0 && spellbonuses.percussionMod == 0)
				effectmod = 10;
			else if(GetSkill(PERCUSSION_INSTRUMENTS) == 0)
				effectmod = 10;
			else if(itembonuses.percussionMod > spellbonuses.percussionMod)
				effectmod = itembonuses.percussionMod;
			else
				effectmod = spellbonuses.percussionMod;
			break;
		case STRINGED_INSTRUMENTS:
			if(itembonuses.stringedMod == 0 && spellbonuses.stringedMod == 0)
				effectmod = 10;
			else if(GetSkill(STRINGED_INSTRUMENTS) == 0)
				effectmod = 10;
			else if(itembonuses.stringedMod > spellbonuses.stringedMod)
				effectmod = itembonuses.stringedMod;
			else
				effectmod = spellbonuses.stringedMod;
			break;
		case WIND_INSTRUMENTS:
			if(itembonuses.windMod == 0 && spellbonuses.windMod == 0)
				effectmod = 10;
			else if(GetSkill(WIND_INSTRUMENTS) == 0)
				effectmod = 10;
			else if(itembonuses.windMod > spellbonuses.windMod)
				effectmod = itembonuses.windMod;
			else
				effectmod = spellbonuses.windMod;
			break;
		case BRASS_INSTRUMENTS:
			if(itembonuses.brassMod == 0 && spellbonuses.brassMod == 0)
				effectmod = 10;
			else if(GetSkill(BRASS_INSTRUMENTS) == 0)
				effectmod = 10;
			else if(itembonuses.brassMod > spellbonuses.brassMod)
				effectmod = itembonuses.brassMod;
			else
				effectmod = spellbonuses.brassMod;
			break;
		case SINGING:
			if(itembonuses.singingMod == 0 && spellbonuses.singingMod == 0)
				effectmod = 10;
			else if(itembonuses.singingMod > spellbonuses.singingMod)
				effectmod = itembonuses.singingMod;
			else
				effectmod = spellbonuses.singingMod;
			break;
		default:
			effectmod = 10;
			break;
	}

	if(spells[spell_id].skill == SINGING)
	{
		effectmod += 2*GetAA(aaSingingMastery);
		effectmod += 2*GetAA(aaImprovedSingingMastery);
	}
	else
	{
		effectmod += 2*GetAA(aaInstrumentMastery);
		effectmod += 2*GetAA(aaImprovedInstrumentMastery);
	}
	effectmod += 2*GetAA(aaAyonaesTutelage); //singing & instruments
	effectmod += 2*GetAA(aaEchoofTaelosia); //singing & instruments


	if(effectmod < 10)
		effectmod = 10;

	_log(SPELLS__BARDS, "%s::GetInstrumentMod() spell=%d mod=%d\n", GetName(), spell_id, effectmod);

	return(effectmod);
}

void Client::CalcMaxEndurance()
{
	max_end = CalcBaseEndurance() + spellbonuses.Endurance + itembonuses.Endurance;

	if (max_end < 0) {
		max_end = 0;
	}

	if (cur_end > max_end) {
		cur_end = max_end;
	}

	int end_perc_cap = spellbonuses.EndPercCap;
	if(end_perc_cap) {
		int curEnd_cap = (max_end * end_perc_cap) / 100;
		if (cur_end > curEnd_cap)
			cur_end = curEnd_cap;
	}
}

int32 Client::CalcBaseEndurance()
{
	int32 base_end = 0;
	int32 base_endurance = 0;
	int32 ConvertedStats = 0;
	int32 sta_end = 0;
	int Stats = 0;

	if(GetClientVersion() >= EQClientSoD && RuleB(Character, SoDClientUseSoDHPManaEnd)) {
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
			HalfBonus400to800 = int( max( ( at_most_800 - 400 ), 0 ) / 8 );

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

int32 Client::CalcEnduranceRegen() {
	int32 regen = int32(GetLevel() * 4 / 10) + 2;
	regen += aabonuses.EnduranceRegen + spellbonuses.EnduranceRegen + itembonuses.EnduranceRegen;

	return (regen * RuleI(Character, EnduranceRegenMultiplier) / 100);
}

int32 Client::CalcEnduranceRegenCap() {
	int cap = (RuleI(Character, ItemEnduranceRegenCap) + itembonuses.HeroicSTR/25 + itembonuses.HeroicDEX/25 + itembonuses.HeroicAGI/25 + itembonuses.HeroicSTA/25);

	return (cap * RuleI(Character, EnduranceRegenMultiplier) / 100);
}

int Client::GetRawACNoShield(int &shield_ac) const
{
	int ac = itembonuses.AC + spellbonuses.AC;
	shield_ac = 0;
	const ItemInst *inst = m_inv.GetItem(SLOT_SECONDARY);
	if(inst)
	{
		if(inst->GetItem()->ItemType == ItemTypeShield)
		{
			ac -= inst->GetItem()->AC;
			shield_ac = inst->GetItem()->AC;
			for(uint8 i = 0; i < MAX_AUGMENT_SLOTS; i++)
			{
				if(inst->GetAugment(i))
				{
					ac -= inst->GetAugment(i)->GetItem()->AC;
					shield_ac += inst->GetAugment(i)->GetItem()->AC;
				}
			}
		}
	}
	return ac;
}
