/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

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

#include "../common/global_define.h"
#include "../common/eqemu_logsys.h"

#include "../common/rulesys.h"
#include "../common/spdat.h"

#include "client.h"
#include "mob.h"

#ifdef BOTS
	#include "bot.h"
#endif

#include <algorithm>


int32 Client::GetMaxStat() const
{
	if ((RuleI(Character, StatCap)) > 0) {
		return (RuleI(Character, StatCap));
	}
	int level = GetLevel();
	int32 base = 0;
	if (level < 61) {
		base = 255;
	}
	else if (ClientVersion() >= EQEmu::versions::ClientVersion::SoF) {
		base = 255 + 5 * (level - 60);
	}
	else if (level < 71) {
		base = 255 + 5 * (level - 60);
	}
	else {
		base = 330;
	}
	return (base);
}

int32 Client::GetMaxResist() const
{
	int level = GetLevel();
	int32 base = 500;
	if (level > 65) {
		base += ((level - 65) * 5);
	}
	return base;
}

int32 Client::GetMaxSTR() const
{
	return GetMaxStat()
	       + itembonuses.STRCapMod
	       + spellbonuses.STRCapMod
	       + aabonuses.STRCapMod;
}
int32 Client::GetMaxSTA() const
{
	return GetMaxStat()
	       + itembonuses.STACapMod
	       + spellbonuses.STACapMod
	       + aabonuses.STACapMod;
}
int32 Client::GetMaxDEX() const
{
	return GetMaxStat()
	       + itembonuses.DEXCapMod
	       + spellbonuses.DEXCapMod
	       + aabonuses.DEXCapMod;
}
int32 Client::GetMaxAGI() const
{
	return GetMaxStat()
	       + itembonuses.AGICapMod
	       + spellbonuses.AGICapMod
	       + aabonuses.AGICapMod;
}
int32 Client::GetMaxINT() const
{
	return GetMaxStat()
	       + itembonuses.INTCapMod
	       + spellbonuses.INTCapMod
	       + aabonuses.INTCapMod;
}
int32 Client::GetMaxWIS() const
{
	return GetMaxStat()
	       + itembonuses.WISCapMod
	       + spellbonuses.WISCapMod
	       + aabonuses.WISCapMod;
}
int32 Client::GetMaxCHA() const
{
	return GetMaxStat()
	       + itembonuses.CHACapMod
	       + spellbonuses.CHACapMod
	       + aabonuses.CHACapMod;
}
int32 Client::GetMaxMR() const
{
	return GetMaxResist()
	       + itembonuses.MRCapMod
	       + spellbonuses.MRCapMod
	       + aabonuses.MRCapMod;
}
int32 Client::GetMaxPR() const
{
	return GetMaxResist()
	       + itembonuses.PRCapMod
	       + spellbonuses.PRCapMod
	       + aabonuses.PRCapMod;
}
int32 Client::GetMaxDR() const
{
	return GetMaxResist()
	       + itembonuses.DRCapMod
	       + spellbonuses.DRCapMod
	       + aabonuses.DRCapMod;
}
int32 Client::GetMaxCR() const
{
	return GetMaxResist()
	       + itembonuses.CRCapMod
	       + spellbonuses.CRCapMod
	       + aabonuses.CRCapMod;
}
int32 Client::GetMaxFR() const
{
	return GetMaxResist()
	       + itembonuses.FRCapMod
	       + spellbonuses.FRCapMod
	       + aabonuses.FRCapMod;
}
int32 Client::GetMaxCorrup() const
{
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
	bool bonus = GetPlayerRaceBit(GetBaseRace()) & RuleI(Character, BaseHPRegenBonusRaces);
	uint8 multiplier1 = bonus ? 2 : 1;
	int32 hp = 0;
	//these calculations should match up with the info from Monkly Business, which was last updated ~05/2008: http://www.monkly-business.net/index.php?pageid=abilities
	if (level < 51) {
		if (sitting) {
			if (level < 20) {
				hp += 2 * multiplier1;
			}
			else if (level < 50) {
				hp += 3 * multiplier1;
			}
			else {	//level == 50
				hp += 4 * multiplier1;
			}
		}
		else {	//feigned or standing
			hp += 1 * multiplier1;
		}
	}
	//there may be an easier way to calculate this next part, but I don't know what it is
	else {	//level >= 51
		int32 tmp = 0;
		float multiplier2 = 1;
		if (level < 56) {
			tmp = 2;
			if (bonus) {
				multiplier2 = 3;
			}
		}
		else if (level < 60) {
			tmp = 3;
			if (bonus) {
				multiplier2 = 3.34;
			}
		}
		else if (level < 61) {
			tmp = 4;
			if (bonus) {
				multiplier2 = 3;
			}
		}
		else if (level < 63) {
			tmp = 5;
			if (bonus) {
				multiplier2 = 2.8;
			}
		}
		else if (level < 65) {
			tmp = 6;
			if (bonus) {
				multiplier2 = 2.67;
			}
		}
		else {	//level >= 65
			tmp = 7;
			if (bonus) {
				multiplier2 = 2.58;
			}
		}
		hp += int32(float(tmp) * multiplier2);
		if (sitting) {
			hp += 3 * multiplier1;
		}
		else if (feigned) {
			hp += 1 * multiplier1;
		}
	}
	return hp;
}

int32 Client::CalcHPRegen()
{
	int32 regen = LevelRegen() + itembonuses.HPRegen + spellbonuses.HPRegen;
	regen += aabonuses.HPRegen + GroupLeadershipAAHealthRegeneration();
	return (regen * RuleI(Character, HPRegenMultiplier) / 100);
}

int32 Client::CalcHPRegenCap()
{
	int cap = RuleI(Character, ItemHealthRegenCap) + itembonuses.HeroicSTA / 25;
	cap += aabonuses.ItemHPRegenCap + spellbonuses.ItemHPRegenCap + itembonuses.ItemHPRegenCap;
	return (cap * RuleI(Character, HPRegenMultiplier) / 100);
}

int32 Client::CalcMaxHP()
{
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
	max_hp += max_hp * ((spellbonuses.MaxHPChange + itembonuses.MaxHPChange) / 10000.0f);
	if (cur_hp > max_hp) {
		cur_hp = max_hp;
	}
	int hp_perc_cap = spellbonuses.HPPercCap[0];
	if (hp_perc_cap) {
		int curHP_cap = (max_hp * hp_perc_cap) / 100;
		if (cur_hp > curHP_cap || (spellbonuses.HPPercCap[1] && cur_hp > spellbonuses.HPPercCap[1])) {
			cur_hp = curHP_cap;
		}
	}
	return max_hp;
}

uint32 Mob::GetClassLevelFactor()
{
	uint32 multiplier = 0;
	uint8 mlevel = GetLevel();
	switch (GetClass()) {
		case WARRIOR: {
				if (mlevel < 20) {
					multiplier = 220;
				}
				else if (mlevel < 30) {
					multiplier = 230;
				}
				else if (mlevel < 40) {
					multiplier = 250;
				}
				else if (mlevel < 53) {
					multiplier = 270;
				}
				else if (mlevel < 57) {
					multiplier = 280;
				}
				else if (mlevel < 60) {
					multiplier = 290;
				}
				else if (mlevel < 70) {
					multiplier = 300;
				}
				else {
					multiplier = 311;
				}
				break;
			}
		case DRUID:
		case CLERIC:
		case SHAMAN: {
				if (mlevel < 70) {
					multiplier = 150;
				}
				else {
					multiplier = 157;
				}
				break;
			}
		case BERSERKER:
		case PALADIN:
		case SHADOWKNIGHT: {
				if (mlevel < 35) {
					multiplier = 210;
				}
				else if (mlevel < 45) {
					multiplier = 220;
				}
				else if (mlevel < 51) {
					multiplier = 230;
				}
				else if (mlevel < 56) {
					multiplier = 240;
				}
				else if (mlevel < 60) {
					multiplier = 250;
				}
				else if (mlevel < 68) {
					multiplier = 260;
				}
				else {
					multiplier = 270;
				}
				break;
			}
		case MONK:
		case BARD:
		case ROGUE:
		case BEASTLORD: {
				if (mlevel < 51) {
					multiplier = 180;
				}
				else if (mlevel < 58) {
					multiplier = 190;
				}
				else if (mlevel < 70) {
					multiplier = 200;
				}
				else {
					multiplier = 210;
				}
				break;
			}
		case RANGER: {
				if (mlevel < 58) {
					multiplier = 200;
				}
				else if (mlevel < 70) {
					multiplier = 210;
				}
				else {
					multiplier = 220;
				}
				break;
			}
		case MAGICIAN:
		case WIZARD:
		case NECROMANCER:
		case ENCHANTER: {
				if (mlevel < 70) {
					multiplier = 120;
				}
				else {
					multiplier = 127;
				}
				break;
			}
		default: {
				if (mlevel < 35) {
					multiplier = 210;
				}
				else if (mlevel < 45) {
					multiplier = 220;
				}
				else if (mlevel < 51) {
					multiplier = 230;
				}
				else if (mlevel < 56) {
					multiplier = 240;
				}
				else if (mlevel < 60) {
					multiplier = 250;
				}
				else {
					multiplier = 260;
				}
				break;
			}
	}
	return multiplier;
}

int32 Client::CalcBaseHP()
{
	if (ClientVersion() >= EQEmu::versions::ClientVersion::SoF && RuleB(Character, SoDClientUseSoDHPManaEnd)) {
		int stats = GetSTA();
		if (stats > 255) {
			stats = (stats - 255) / 2;
			stats += 255;
		}
		base_hp = 5;
		auto base_data = database.GetBaseData(GetLevel(), GetClass());
		if (base_data) {
			base_hp += base_data->base_hp + (base_data->hp_factor * stats);
			base_hp += (GetHeroicSTA() * 10);
		}
	}
	else {
		uint32 Post255;
		uint32 lm = GetClassLevelFactor();
		if ((GetSTA() - 255) / 2 > 0) {
			Post255 = (GetSTA() - 255) / 2;
		}
		else {
			Post255 = 0;
		}
		base_hp = (5) + (GetLevel() * lm / 10) + (((GetSTA() - Post255) * GetLevel() * lm / 3000)) + ((Post255 * GetLevel()) * lm / 6000);
	}
	return base_hp;
}

// This is for calculating Base HPs + STA bonus for SoD or later clients.
uint32 Client::GetClassHPFactor()
{
	int factor;
	// Note: Base HP factor under level 41 is equal to factor / 12, and from level 41 to 80 is factor / 6.
	// Base HP over level 80 is factor / 10
	// HP per STA point per level is factor / 30 for level 80+
	// HP per STA under level 40 is the level 80 HP Per STA / 120, and for over 40 it is / 60.
	switch (GetClass()) {
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
int32 Client::GetRawItemAC()
{
	int32 Total = 0;
	// this skips MainAmmo..add an '=' conditional if that slot is required (original behavior)
	for (int16 slot_id = EQEmu::legacy::EQUIPMENT_BEGIN; slot_id < EQEmu::legacy::EQUIPMENT_END; slot_id++) {
		const ItemInst* inst = m_inv[slot_id];
		if (inst && inst->IsType(ItemClassCommon)) {
			Total += inst->GetItem()->AC;
		}
	}
	return Total;
}

int32 Client::acmod()
{
	int agility = GetAGI();
	int level = GetLevel();
	if (agility < 1 || level < 1) {
		return (0);
	}
	if (agility <= 74) {
		if (agility == 1) {
			return -24;
		}
		else if (agility <= 3) {
			return -23;
		}
		else if (agility == 4) {
			return -22;
		}
		else if (agility <= 6) {
			return -21;
		}
		else if (agility <= 8) {
			return -20;
		}
		else if (agility == 9) {
			return -19;
		}
		else if (agility <= 11) {
			return -18;
		}
		else if (agility == 12) {
			return -17;
		}
		else if (agility <= 14) {
			return -16;
		}
		else if (agility <= 16) {
			return -15;
		}
		else if (agility == 17) {
			return -14;
		}
		else if (agility <= 19) {
			return -13;
		}
		else if (agility == 20) {
			return -12;
		}
		else if (agility <= 22) {
			return -11;
		}
		else if (agility <= 24) {
			return -10;
		}
		else if (agility == 25) {
			return -9;
		}
		else if (agility <= 27) {
			return -8;
		}
		else if (agility == 28) {
			return -7;
		}
		else if (agility <= 30) {
			return -6;
		}
		else if (agility <= 32) {
			return -5;
		}
		else if (agility == 33) {
			return -4;
		}
		else if (agility <= 35) {
			return -3;
		}
		else if (agility == 36) {
			return -2;
		}
		else if (agility <= 38) {
			return -1;
		}
		else if (agility <= 65) {
			return 0;
		}
		else if (agility <= 70) {
			return 1;
		}
		else if (agility <= 74) {
			return 5;
		}
	}
	else if (agility <= 137) {
		if (agility == 75) {
			if (level <= 6) {
				return 9;
			}
			else if (level <= 19) {
				return 23;
			}
			else if (level <= 39) {
				return 33;
			}
			else {
				return 39;
			}
		}
		else if (agility >= 76 && agility <= 79) {
			if (level <= 6) {
				return 10;
			}
			else if (level <= 19) {
				return 23;
			}
			else if (level <= 39) {
				return 33;
			}
			else {
				return 40;
			}
		}
		else if (agility == 80) {
			if (level <= 6) {
				return 11;
			}
			else if (level <= 19) {
				return 24;
			}
			else if (level <= 39) {
				return 34;
			}
			else {
				return 41;
			}
		}
		else if (agility >= 81 && agility <= 85) {
			if (level <= 6) {
				return 12;
			}
			else if (level <= 19) {
				return 25;
			}
			else if (level <= 39) {
				return 35;
			}
			else {
				return 42;
			}
		}
		else if (agility >= 86 && agility <= 90) {
			if (level <= 6) {
				return 12;
			}
			else if (level <= 19) {
				return 26;
			}
			else if (level <= 39) {
				return 36;
			}
			else {
				return 42;
			}
		}
		else if (agility >= 91 && agility <= 95) {
			if (level <= 6) {
				return 13;
			}
			else if (level <= 19) {
				return 26;
			}
			else if (level <= 39) {
				return 36;
			}
			else {
				return 43;
			}
		}
		else if (agility >= 96 && agility <= 99) {
			if (level <= 6) {
				return 14;
			}
			else if (level <= 19) {
				return 27;
			}
			else if (level <= 39) {
				return 37;
			}
			else {
				return 44;
			}
		}
		else if (agility == 100 && level >= 7) {
			if (level <= 19) {
				return 28;
			}
			else if (level <= 39) {
				return 38;
			}
			else {
				return 45;
			}
		}
		else if (level <= 6) {
			return 15;
		}
		//level is >6
		else if (agility >= 101 && agility <= 105) {
			if (level <= 19) {
				return 29;
			}
			else if (level <= 39) {
				return 39;    // not verified
			}
			else {
				return 45;
			}
		}
		else if (agility >= 106 && agility <= 110) {
			if (level <= 19) {
				return 29;
			}
			else if (level <= 39) {
				return 39;    // not verified
			}
			else {
				return 46;
			}
		}
		else if (agility >= 111 && agility <= 115) {
			if (level <= 19) {
				return 30;
			}
			else if (level <= 39) {
				return 40;    // not verified
			}
			else {
				return 47;
			}
		}
		else if (agility >= 116 && agility <= 119) {
			if (level <= 19) {
				return 31;
			}
			else if (level <= 39) {
				return 41;
			}
			else {
				return 47;
			}
		}
		else if (level <= 19) {
			return 32;
		}
		//level is > 19
		else if (agility == 120) {
			if (level <= 39) {
				return 42;
			}
			else {
				return 48;
			}
		}
		else if (agility <= 125) {
			if (level <= 39) {
				return 42;
			}
			else {
				return 49;
			}
		}
		else if (agility <= 135) {
			if (level <= 39) {
				return 42;
			}
			else {
				return 50;
			}
		}
		else {
			if (level <= 39) {
				return 42;
			}
			else {
				return 51;
			}
		}
	}
	else if (agility <= 300) {
		if (level <= 6) {
			if (agility <= 139) {
				return (21);
			}
			else if (agility == 140) {
				return (22);
			}
			else if (agility <= 145) {
				return (23);
			}
			else if (agility <= 150) {
				return (23);
			}
			else if (agility <= 155) {
				return (24);
			}
			else if (agility <= 159) {
				return (25);
			}
			else if (agility == 160) {
				return (26);
			}
			else if (agility <= 165) {
				return (26);
			}
			else if (agility <= 170) {
				return (27);
			}
			else if (agility <= 175) {
				return (28);
			}
			else if (agility <= 179) {
				return (28);
			}
			else if (agility == 180) {
				return (29);
			}
			else if (agility <= 185) {
				return (30);
			}
			else if (agility <= 190) {
				return (31);
			}
			else if (agility <= 195) {
				return (31);
			}
			else if (agility <= 199) {
				return (32);
			}
			else if (agility <= 219) {
				return (33);
			}
			else if (agility <= 239) {
				return (34);
			}
			else {
				return (35);
			}
		}
		else if (level <= 19) {
			if (agility <= 139) {
				return (34);
			}
			else if (agility == 140) {
				return (35);
			}
			else if (agility <= 145) {
				return (36);
			}
			else if (agility <= 150) {
				return (37);
			}
			else if (agility <= 155) {
				return (37);
			}
			else if (agility <= 159) {
				return (38);
			}
			else if (agility == 160) {
				return (39);
			}
			else if (agility <= 165) {
				return (40);
			}
			else if (agility <= 170) {
				return (40);
			}
			else if (agility <= 175) {
				return (41);
			}
			else if (agility <= 179) {
				return (42);
			}
			else if (agility == 180) {
				return (43);
			}
			else if (agility <= 185) {
				return (43);
			}
			else if (agility <= 190) {
				return (44);
			}
			else if (agility <= 195) {
				return (45);
			}
			else if (agility <= 199) {
				return (45);
			}
			else if (agility <= 219) {
				return (46);
			}
			else if (agility <= 239) {
				return (47);
			}
			else {
				return (48);
			}
		}
		else if (level <= 39) {
			if (agility <= 139) {
				return (44);
			}
			else if (agility == 140) {
				return (45);
			}
			else if (agility <= 145) {
				return (46);
			}
			else if (agility <= 150) {
				return (47);
			}
			else if (agility <= 155) {
				return (47);
			}
			else if (agility <= 159) {
				return (48);
			}
			else if (agility == 160) {
				return (49);
			}
			else if (agility <= 165) {
				return (50);
			}
			else if (agility <= 170) {
				return (50);
			}
			else if (agility <= 175) {
				return (51);
			}
			else if (agility <= 179) {
				return (52);
			}
			else if (agility == 180) {
				return (53);
			}
			else if (agility <= 185) {
				return (53);
			}
			else if (agility <= 190) {
				return (54);
			}
			else if (agility <= 195) {
				return (55);
			}
			else if (agility <= 199) {
				return (55);
			}
			else if (agility <= 219) {
				return (56);
			}
			else if (agility <= 239) {
				return (57);
			}
			else {
				return (58);
			}
		}
		else {	//lvl >= 40
			if (agility <= 139) {
				return (51);
			}
			else if (agility == 140) {
				return (52);
			}
			else if (agility <= 145) {
				return (53);
			}
			else if (agility <= 150) {
				return (53);
			}
			else if (agility <= 155) {
				return (54);
			}
			else if (agility <= 159) {
				return (55);
			}
			else if (agility == 160) {
				return (56);
			}
			else if (agility <= 165) {
				return (56);
			}
			else if (agility <= 170) {
				return (57);
			}
			else if (agility <= 175) {
				return (58);
			}
			else if (agility <= 179) {
				return (58);
			}
			else if (agility == 180) {
				return (59);
			}
			else if (agility <= 185) {
				return (60);
			}
			else if (agility <= 190) {
				return (61);
			}
			else if (agility <= 195) {
				return (61);
			}
			else if (agility <= 199) {
				return (62);
			}
			else if (agility <= 219) {
				return (63);
			}
			else if (agility <= 239) {
				return (64);
			}
			else {
				return (65);
			}
		}
	}
	else {
		//seems about 21 agil per extra AC pt over 300...
		return (65 + ((agility - 300) / 21));
	}
	Log.Out(Logs::Detail, Logs::Error, "Error in Client::acmod(): Agility: %i, Level: %i", agility, level);
	return 0;
};

// This is a testing formula for AC, the value this returns should be the same value as the one the client shows...
// ac1 and ac2 are probably the damage migitation and damage avoidance numbers, not sure which is which.
// I forgot to include the iksar defense bonus and i cant find my notes now...
// AC from spells are not included (cant even cast spells yet..)
int32 Client::CalcAC()
{
	// new formula
	int avoidance = (acmod() + ((GetSkill(SkillDefense) + itembonuses.HeroicAGI / 10) * 16) / 9);
	if (avoidance < 0) {
		avoidance = 0;
	}
	int mitigation = 0;
	if (m_pp.class_ == WIZARD || m_pp.class_ == MAGICIAN || m_pp.class_ == NECROMANCER || m_pp.class_ == ENCHANTER) {
		//something is wrong with this, naked casters have the wrong natural AC
//		mitigation = (spellbonuses.AC/3) + (GetSkill(DEFENSE)/2) + (itembonuses.AC+1);
		mitigation = (GetSkill(SkillDefense) + itembonuses.HeroicAGI / 10) / 4 + (itembonuses.AC + 1);
		//this might be off by 4..
		mitigation -= 4;
	}
	else {
//		mitigation = (spellbonuses.AC/4) + (GetSkill(DEFENSE)/3) + ((itembonuses.AC*4)/3);
		mitigation = (GetSkill(SkillDefense) + itembonuses.HeroicAGI / 10) / 3 + ((itembonuses.AC * 4) / 3);
		if (m_pp.class_ == MONK) {
			mitigation += GetLevel() * 13 / 10;    //the 13/10 might be wrong, but it is close...
		}
	}
	int displayed = 0;
	displayed += ((avoidance + mitigation) * 1000) / 847;	//natural AC
	//Iksar AC, untested
	if (GetRace() == IKSAR) {
		displayed += 12;
		int iksarlevel = GetLevel();
		iksarlevel -= 10;
		if (iksarlevel > 25) {
			iksarlevel = 25;
		}
		if (iksarlevel > 0) {
			displayed += iksarlevel * 12 / 10;
		}
	}
	// Shield AC bonus for HeroicSTR
	if (itembonuses.HeroicSTR) {
		bool equiped = CastToClient()->m_inv.GetItem(EQEmu::legacy::SlotSecondary);
		if (equiped) {
			uint8 shield = CastToClient()->m_inv.GetItem(EQEmu::legacy::SlotSecondary)->GetItem()->ItemType;
			if (shield == ItemTypeShield) {
				displayed += itembonuses.HeroicSTR / 2;
			}
		}
	}
	//spell AC bonuses are added directly to natural total
	displayed += spellbonuses.AC;
	AC = displayed;
	return (AC);
}

int32 Client::GetACMit()
{
	int mitigation = 0;
	if (m_pp.class_ == WIZARD || m_pp.class_ == MAGICIAN || m_pp.class_ == NECROMANCER || m_pp.class_ == ENCHANTER) {
		mitigation = (GetSkill(SkillDefense) + itembonuses.HeroicAGI / 10) / 4 + (itembonuses.AC + 1);
		mitigation -= 4;
	}
	else {
		mitigation = (GetSkill(SkillDefense) + itembonuses.HeroicAGI / 10) / 3 + ((itembonuses.AC * 4) / 3);
		if (m_pp.class_ == MONK) {
			mitigation += GetLevel() * 13 / 10;    //the 13/10 might be wrong, but it is close...
		}
	}
	// Shield AC bonus for HeroicSTR
	if (itembonuses.HeroicSTR) {
		bool equiped = CastToClient()->m_inv.GetItem(EQEmu::legacy::SlotSecondary);
		if (equiped) {
			uint8 shield = CastToClient()->m_inv.GetItem(EQEmu::legacy::SlotSecondary)->GetItem()->ItemType;
			if (shield == ItemTypeShield) {
				mitigation += itembonuses.HeroicSTR / 2;
			}
		}
	}
	return (mitigation * 1000 / 847);
}

int32 Client::GetACAvoid()
{
	int32 avoidance = (acmod() + ((GetSkill(SkillDefense) + itembonuses.HeroicAGI / 10) * 16) / 9);
	if (avoidance < 0) {
		avoidance = 0;
	}
	return (avoidance * 1000 / 847);
}

int32 Client::CalcMaxMana()
{
	switch (GetCasterClass()) {
		case 'I':
		case 'W': {
				max_mana = (CalcBaseMana() + itembonuses.Mana + spellbonuses.Mana + aabonuses.Mana + GroupLeadershipAAManaEnhancement());
				break;
			}
		case 'N': {
				max_mana = 0;
				break;
			}
		default: {
				Log.Out(Logs::Detail, Logs::Spells, "Invalid Class '%c' in CalcMaxMana", GetCasterClass());
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
	int mana_perc_cap = spellbonuses.ManaPercCap[0];
	if (mana_perc_cap) {
		int curMana_cap = (max_mana * mana_perc_cap) / 100;
		if (cur_mana > curMana_cap || (spellbonuses.ManaPercCap[1] && cur_mana > spellbonuses.ManaPercCap[1])) {
			cur_mana = curMana_cap;
		}
	}
	Log.Out(Logs::Detail, Logs::Spells, "Client::CalcMaxMana() called for %s - returning %d", GetName(), max_mana);
	return max_mana;
}

int32 Client::CalcBaseMana()
{
	int ConvertedWisInt = 0;
	int MindLesserFactor, MindFactor;
	int WisInt = 0;
	int base_mana = 0;
	int wisint_mana = 0;
	int32 max_m = 0;
	switch (GetCasterClass()) {
		case 'I':
			WisInt = GetINT();
			if (ClientVersion() >= EQEmu::versions::ClientVersion::SoF && RuleB(Character, SoDClientUseSoDHPManaEnd)) {
				if (WisInt > 100) {
					ConvertedWisInt = (((WisInt - 100) * 5 / 2) + 100);
					if (WisInt > 201) {
						ConvertedWisInt -= ((WisInt - 201) * 5 / 4);
					}
				}
				else {
					ConvertedWisInt = WisInt;
				}
				auto base_data = database.GetBaseData(GetLevel(), GetClass());
				if (base_data) {
					max_m = base_data->base_mana + (ConvertedWisInt * base_data->mana_factor) + (GetHeroicINT() * 10);
				}
			}
			else {
				if ((( WisInt - 199 ) / 2) > 0) {
					MindLesserFactor = ( WisInt - 199 ) / 2;
				}
				else {
					MindLesserFactor = 0;
				}
				MindFactor = WisInt - MindLesserFactor;
				if (WisInt > 100) {
					max_m = (((5 * (MindFactor + 20)) / 2) * 3 * GetLevel() / 40);
				}
				else {
					max_m = (((5 * (MindFactor + 200)) / 2) * 3 * GetLevel() / 100);
				}
			}
			break;
		case 'W':
			WisInt = GetWIS();
			if (ClientVersion() >= EQEmu::versions::ClientVersion::SoF && RuleB(Character, SoDClientUseSoDHPManaEnd)) {
				if (WisInt > 100) {
					ConvertedWisInt = (((WisInt - 100) * 5 / 2) + 100);
					if (WisInt > 201) {
						ConvertedWisInt -= ((WisInt - 201) * 5 / 4);
					}
				}
				else {
					ConvertedWisInt = WisInt;
				}
				auto base_data = database.GetBaseData(GetLevel(), GetClass());
				if (base_data) {
					max_m = base_data->base_mana + (ConvertedWisInt * base_data->mana_factor) + (GetHeroicWIS() * 10);
				}
			}
			else {
				if ((( WisInt - 199 ) / 2) > 0) {
					MindLesserFactor = ( WisInt - 199 ) / 2;
				}
				else {
					MindLesserFactor = 0;
				}
				MindFactor = WisInt - MindLesserFactor;
				if (WisInt > 100) {
					max_m = (((5 * (MindFactor + 20)) / 2) * 3 * GetLevel() / 40);
				}
				else {
					max_m = (((5 * (MindFactor + 200)) / 2) * 3 * GetLevel() / 100);
				}
			}
			break;
		case 'N': {
				max_m = 0;
				break;
			}
		default: {
				Log.Out(Logs::General, Logs::None, "Invalid Class '%c' in CalcMaxMana", GetCasterClass());
				max_m = 0;
				break;
			}
	}
	#if EQDEBUG >= 11
	Log.Out(Logs::General, Logs::None, "Client::CalcBaseMana() called for %s - returning %d", GetName(), max_m);
	#endif
	return max_m;
}

int32 Client::CalcBaseManaRegen()
{
	uint8 clevel = GetLevel();
	int32 regen = 0;
	if (IsSitting() || (GetHorseId() != 0)) {
		if (HasSkill(SkillMeditate)) {
			regen = (((GetSkill(SkillMeditate) / 10) + (clevel - (clevel / 4))) / 4) + 4;
		}
		else {
			regen = 2;
		}
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
	if (IsSitting() || (GetHorseId() != 0)) {
		BuffFadeBySitModifier();
		if (HasSkill(SkillMeditate)) {
			this->medding = true;
			regen = (((GetSkill(SkillMeditate) / 10) + (clevel - (clevel / 4))) / 4) + 4;
			regen += spellbonuses.ManaRegen + itembonuses.ManaRegen;
			CheckIncreaseSkill(SkillMeditate, nullptr, -5);
		}
		else {
			regen = 2 + spellbonuses.ManaRegen + itembonuses.ManaRegen;
		}
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
	switch (GetCasterClass()) {
		case 'I':
			cap += (itembonuses.HeroicINT / 25);
			break;
		case 'W':
			cap += (itembonuses.HeroicWIS / 25);
			break;
	}
	return (cap * RuleI(Character, ManaRegenMultiplier) / 100);
}

uint32 Client::CalcCurrentWeight()
{
	const Item_Struct* TempItem = 0;
	ItemInst* ins;
	uint32 Total = 0;
	int x;
	for (x = EQEmu::legacy::EQUIPMENT_BEGIN; x <= EQEmu::legacy::SlotCursor; x++) { // include cursor or not?
		TempItem = 0;
		ins = GetInv().GetItem(x);
		if (ins) {
			TempItem = ins->GetItem();
		}
		if (TempItem) {
			Total += TempItem->Weight;
		}
	}
	for (x = EQEmu::legacy::GENERAL_BAGS_BEGIN; x <= EQEmu::legacy::GENERAL_BAGS_END; x++) { // include cursor bags or not?
		int TmpWeight = 0;
		TempItem = 0;
		ins = GetInv().GetItem(x);
		if (ins) {
			TempItem = ins->GetItem();
		}
		if (TempItem) {
			TmpWeight = TempItem->Weight;
		}
		if (TmpWeight > 0) {
			// this code indicates that weight redux bags can only be in the first general inventory slot to be effective...
			// is this correct? or can we scan for the highest weight redux and use that? (need client verifications)
			int bagslot = EQEmu::legacy::SlotGeneral1;
			int reduction = 0;
			for (int m = EQEmu::legacy::GENERAL_BAGS_BEGIN + 10; m <= EQEmu::legacy::GENERAL_BAGS_END; m += 10) { // include cursor bags or not?
				if (x >= m) {
					bagslot += 1;
				}
			}
			ItemInst* baginst = GetInv().GetItem(bagslot);
			if (baginst && baginst->GetItem() && baginst->IsType(ItemClassContainer)) {
				reduction = baginst->GetItem()->BagWR;
			}
			if (reduction > 0) {
				TmpWeight -= TmpWeight * reduction / 100;
			}
			Total += TmpWeight;
		}
	}
	//TODO: coin weight reduction (from purses, etc), since client already calculates it
	/*  From the Wiki http://www.eqemulator.net/wiki/wikka.php?wakka=EQEmuDBSchemaitems under bagwr (thanks Trevius):
	    Interestingly, you can also have bags that reduce coin weight. However, in order to set bags to reduce coin weight, you MUST set the Item ID somewhere between 17201 and 17230. This is hard coded into the client.
	    The client is set to have certain coin weight reduction on a per Item ID basis within this range. The best way to create an new item to reduce coin weight is to examine existing bags in this range.
	    Search for the words "coin purse" with the #finditem command in game and the Bag WR setting on those bags is the amount they will reduce coin weight. It is easiest to overwrite one of those bags if you wish to create one with the
	    same weight reduction amount for coins. You can use other Item IDs in this range for setting coin weight reduction, but by using an existing item, at least you will know the amount the client will reduce it by before you create it.
	    This is the ONLY instance I have seen where the client is hard coded to particular Item IDs to set a certain property for an item. It is very odd.
	*/
	// SoD+ client has no weight for coin
	if (EQEmu::limits::CoinHasWeight(EQEmu::versions::ConvertClientVersionToInventoryVersion(ClientVersion()))) {
		Total += (m_pp.platinum + m_pp.gold + m_pp.silver + m_pp.copper) / 4;
	}
	float Packrat = (float)spellbonuses.Packrat + (float)aabonuses.Packrat + (float)itembonuses.Packrat;
	if (Packrat > 0) {
		Total = (uint32)((float)Total * (1.0f - ((Packrat * 1.0f) / 100.0f)));    //AndMetal: 1% per level, up to 5% (calculated from Titanium client). verified thru client that it reduces coin weight by the same %
	}
	//without casting to float & back to uint32, this didn't work right
	return Total;
}

int32 Client::CalcAlcoholPhysicalEffect()
{
	if (m_pp.intoxication <= 55) {
		return 0;
	}
	return (m_pp.intoxication - 40) / 16;
}

int32 Client::CalcSTR()
{
	int32 val = m_pp.STR + itembonuses.STR + spellbonuses.STR + CalcAlcoholPhysicalEffect();
	int32 mod = aabonuses.STR;
	STR = val + mod;
	if (STR < 1) {
		STR = 1;
	}
	int m = GetMaxSTR();
	if (STR > m) {
		STR = m;
	}
	return (STR);
}

int32 Client::CalcSTA()
{
	int32 val = m_pp.STA + itembonuses.STA + spellbonuses.STA + CalcAlcoholPhysicalEffect();;
	int32 mod = aabonuses.STA;
	STA = val + mod;
	if (STA < 1) {
		STA = 1;
	}
	int m = GetMaxSTA();
	if (STA > m) {
		STA = m;
	}
	return (STA);
}

int32 Client::CalcAGI()
{
	int32 val = m_pp.AGI + itembonuses.AGI + spellbonuses.AGI - CalcAlcoholPhysicalEffect();;
	int32 mod = aabonuses.AGI;
	int32 str = GetSTR();
	//Encumbered penalty
	if (weight > (str * 10)) {
		//AGI is halved when we double our weight, zeroed (defaults to 1) when we triple it. this includes AGI from AAs
		float total_agi = float(val + mod);
		float str_float = float(str);
		AGI = (int32)(((-total_agi) / (str_float * 2)) * (((float)weight / 10) - str_float) + total_agi);	//casting to an int assumes this will be floor'd. without using floats & casting to int16, the calculation doesn't work right
	}
	else {
		AGI = val + mod;
	}
	if (AGI < 1) {
		AGI = 1;
	}
	int m = GetMaxAGI();
	if (AGI > m) {
		AGI = m;
	}
	return (AGI);
}

int32 Client::CalcDEX()
{
	int32 val = m_pp.DEX + itembonuses.DEX + spellbonuses.DEX - CalcAlcoholPhysicalEffect();;
	int32 mod = aabonuses.DEX;
	DEX = val + mod;
	if (DEX < 1) {
		DEX = 1;
	}
	int m = GetMaxDEX();
	if (DEX > m) {
		DEX = m;
	}
	return (DEX);
}

int32 Client::CalcINT()
{
	int32 val = m_pp.INT + itembonuses.INT + spellbonuses.INT;
	int32 mod = aabonuses.INT;
	INT = val + mod;
	if (m_pp.intoxication) {
		int32 AlcINT = INT - (int32)((float)m_pp.intoxication / 200.0f * (float)INT) - 1;
		if ((AlcINT < (int)(0.2 * INT))) {
			INT = (int)(0.2f * (float)INT);
		}
		else {
			INT = AlcINT;
		}
	}
	if (INT < 1) {
		INT = 1;
	}
	int m = GetMaxINT();
	if (INT > m) {
		INT = m;
	}
	return (INT);
}

int32 Client::CalcWIS()
{
	int32 val = m_pp.WIS + itembonuses.WIS + spellbonuses.WIS;
	int32 mod = aabonuses.WIS;
	WIS = val + mod;
	if (m_pp.intoxication) {
		int32 AlcWIS = WIS - (int32)((float)m_pp.intoxication / 200.0f * (float)WIS) - 1;
		if ((AlcWIS < (int)(0.2 * WIS))) {
			WIS = (int)(0.2f * (float)WIS);
		}
		else {
			WIS = AlcWIS;
		}
	}
	if (WIS < 1) {
		WIS = 1;
	}
	int m = GetMaxWIS();
	if (WIS > m) {
		WIS = m;
	}
	return (WIS);
}

int32 Client::CalcCHA()
{
	int32 val = m_pp.CHA + itembonuses.CHA + spellbonuses.CHA;
	int32 mod = aabonuses.CHA;
	CHA = val + mod;
	if (CHA < 1) {
		CHA = 1;
	}
	int m = GetMaxCHA();
	if (CHA > m) {
		CHA = m;
	}
	return (CHA);
}

int Client::CalcHaste()
{
	/*  Tests: (based on results in newer char window)
	    68 v1 + 46 item + 25 over + 35 inhib = 204%
	    46 item + 5 v2 + 25 over + 35 inhib = 65%
	    68 v1 + 46 item + 5 v2 + 25 over + 35 inhib = 209%
	    75% slow + 35 inhib = 25%
	    35 inhib = 65%
	    75% slow = 25%
	    Conclusions:
	    the bigger effect in slow v. inhib wins
	    slow negates all other hastes
	    inhib will only negate all other hastes if you don't have v1 (ex. VQ)
	*/
	// slow beats all! Besides a better inhibit
	if (spellbonuses.haste < 0) {
		if (-spellbonuses.haste <= spellbonuses.inhibitmelee) {
			Haste = 100 - spellbonuses.inhibitmelee;
		}
		else {
			Haste = 100 + spellbonuses.haste;
		}
		return Haste;
	}
	// No haste and inhibit, kills all other hastes
	if (spellbonuses.haste == 0 && spellbonuses.inhibitmelee) {
		Haste = 100 - spellbonuses.inhibitmelee;
		return Haste;
	}
	int h = 0;
	int cap = 0;
	int level = GetLevel();
	// we know we have a haste spell and not slowed, no extra inhibit melee checks needed
	if (spellbonuses.haste) {
		h += spellbonuses.haste - spellbonuses.inhibitmelee;
	}
	if (spellbonuses.hastetype2 && level > 49) { // type 2 is capped at 10% and only available to 50+
		h += spellbonuses.hastetype2 > 10 ? 10 : spellbonuses.hastetype2;
	}
	// 26+ no cap, 1-25 10
	if (level > 25) { // 26+
		h += itembonuses.haste;
	}
	else {   // 1-25
		h += itembonuses.haste > 10 ? 10 : itembonuses.haste;
	}
	// 60+ 100, 51-59 85, 1-50 level+25
	if (level > 59) { // 60+
		cap = RuleI(Character, HasteCap);
	}
	else if (level > 50) {  // 51-59
		cap = 85;
	}
	else {   // 1-50
		cap = level + 25;
	}
	cap = mod_client_haste_cap(cap);
	if (h > cap) {
		h = cap;
	}
	// 51+ 25 (despite there being higher spells...), 1-50 10
	if (level > 50) { // 51+
		h += spellbonuses.hastetype3 > 25 ? 25 : spellbonuses.hastetype3;
	}
	else {   // 1-50
		h += spellbonuses.hastetype3 > 10 ? 10 : spellbonuses.hastetype3;
	}
	h += ExtraHaste;	//GM granted haste.
	h = mod_client_haste(h);
	Haste = 100 + h;
	return Haste;
}

//The AA multipliers are set to be 5, but were 2 on WR
//The resistant discipline which I think should be here is implemented
//in Mob::ResistSpell
int32	Client::CalcMR()
{
	//racial bases
	switch (GetBaseRace()) {
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
		{
			MR = 25;
			if (GetDrakkinHeritage() == 2)
				MR += 10;
			else if (GetDrakkinHeritage() == 5)
				MR += 2;
			break;
		}
		default:
			MR = 20;
	}
	MR += itembonuses.MR + spellbonuses.MR + aabonuses.MR;
	if (GetClass() == WARRIOR || GetClass() == BERSERKER) {
		MR += GetLevel() / 2;
	}
	if (MR < 1) {
		MR = 1;
	}
	if (MR > GetMaxMR()) {
		MR = GetMaxMR();
	}
	return (MR);
}

int32	Client::CalcFR()
{
	//racial bases
	switch (GetBaseRace()) {
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
		{
			FR = 25;
			if (GetDrakkinHeritage() == 0)
				FR += 10;
			else if (GetDrakkinHeritage() == 5)
				FR += 2;
			break;
		}
		default:
			FR = 20;
	}
	int c = GetClass();
	if (c == RANGER) {
		FR += 4;
		int l = GetLevel();
		if (l > 49) {
			FR += l - 49;
		}
	}
	if (c == MONK) {
		FR += 8;
		int l = GetLevel();
		if (l > 49) {
			FR += l - 49;
		}
	}
	FR += itembonuses.FR + spellbonuses.FR + aabonuses.FR;
	if (FR < 1) {
		FR = 1;
	}
	if (FR > GetMaxFR()) {
		FR = GetMaxFR();
	}
	return (FR);
}

int32	Client::CalcDR()
{
	//racial bases
	switch (GetBaseRace()) {
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
		{
			DR = 15;
			if (GetDrakkinHeritage() == 1)
				DR += 10;
			else if (GetDrakkinHeritage() == 5)
				DR += 2;
			break;
		}
		default:
			DR = 15;
	}
	int c = GetClass();
	// the monk one is part of base resist
	if (c == MONK) {
		int l = GetLevel();
		if (l > 50)
			DR += l - 50;
	}
	if (c == PALADIN) {
		DR += 8;
		int l = GetLevel();
		if (l > 49) {
			DR += l - 49;
		}
	}
	else if (c == SHADOWKNIGHT || c == BEASTLORD) {
		DR += 4;
		int l = GetLevel();
		if (l > 49) {
			DR += l - 49;
		}
	}
	DR += itembonuses.DR + spellbonuses.DR + aabonuses.DR;
	if (DR < 1) {
		DR = 1;
	}
	if (DR > GetMaxDR()) {
		DR = GetMaxDR();
	}
	return (DR);
}

int32	Client::CalcPR()
{
	//racial bases
	switch (GetBaseRace()) {
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
		{
			PR = 15;
			if (GetDrakkinHeritage() == 3)
				PR += 10;
			else if (GetDrakkinHeritage() == 5)
				PR += 2;
			break;
		}
		default:
			PR = 15;
	}
	int c = GetClass();
	// this monk bonus is part of the base
	if (c == MONK) {
		int l = GetLevel();
		if (l > 50)
			PR += l - 50;
	}
	if (c == ROGUE) {
		PR += 8;
		int l = GetLevel();
		if (l > 49) {
			PR += l - 49;
		}
	}
	else if (c == SHADOWKNIGHT) {
		PR += 4;
		int l = GetLevel();
		if (l > 49) {
			PR += l - 49;
		}
	}
	PR += itembonuses.PR + spellbonuses.PR + aabonuses.PR;
	if (PR < 1) {
		PR = 1;
	}
	if (PR > GetMaxPR()) {
		PR = GetMaxPR();
	}
	return (PR);
}

int32	Client::CalcCR()
{
	//racial bases
	switch (GetBaseRace()) {
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
		{
			CR = 25;
			if (GetDrakkinHeritage() == 4)
				CR += 10;
			else if (GetDrakkinHeritage() == 5)
				CR += 2;
			break;
		}
		default:
			CR = 25;
	}
	int c = GetClass();
	if (c == RANGER || c == BEASTLORD) {
		CR += 4;
		int l = GetLevel();
		if (l > 49) {
			CR += l - 49;
		}
	}
	CR += itembonuses.CR + spellbonuses.CR + aabonuses.CR;
	if (CR < 1) {
		CR = 1;
	}
	if (CR > GetMaxCR()) {
		CR = GetMaxCR();
	}
	return (CR);
}

int32	Client::CalcCorrup()
{
	Corrup = GetBaseCorrup() + itembonuses.Corrup + spellbonuses.Corrup + aabonuses.Corrup;
	if (Corrup > GetMaxCorrup()) {
		Corrup = GetMaxCorrup();
	}
	return (Corrup);
}

int32 Client::CalcATK()
{
	ATK = itembonuses.ATK + spellbonuses.ATK + aabonuses.ATK + GroupLeadershipAAOffenseEnhancement();
	return (ATK);
}

uint32 Mob::GetInstrumentMod(uint16 spell_id) const
{
	if (GetClass() != BARD || spells[spell_id].IsDisciplineBuff) // Puretone is Singing but doesn't get any mod
		return 10;

	uint32 effectmod = 10;
	int effectmodcap = 0;
	bool nocap = false;
	if (RuleB(Character, UseSpellFileSongCap)) {
		effectmodcap = spells[spell_id].songcap / 10;
		// this looks a bit weird, but easiest way I could think to keep both systems working
		if (effectmodcap == 0)
			nocap = true;
		else
			effectmodcap += 10;
	} else {
		effectmodcap = RuleI(Character, BaseInstrumentSoftCap);
	}
	// this should never use spell modifiers...
	// if a spell grants better modifers, they are copied into the item mods
	// because the spells are supposed to act just like having the intrument.
	// item mods are in 10ths of percent increases
	// clickies (Symphony of Battle) that have a song skill don't get AA bonus for some reason
	// but clickies that are songs (selo's on Composers Greaves) do get AA mod as well
	switch (spells[spell_id].skill) {
	case SkillPercussionInstruments:
		if (itembonuses.percussionMod == 0 && spellbonuses.percussionMod == 0)
			effectmod = 10;
		else if (GetSkill(SkillPercussionInstruments) == 0)
			effectmod = 10;
		else if (itembonuses.percussionMod > spellbonuses.percussionMod)
			effectmod = itembonuses.percussionMod;
		else
			effectmod = spellbonuses.percussionMod;
		if (IsBardSong(spell_id))
			effectmod += aabonuses.percussionMod;
		break;
	case SkillStringedInstruments:
		if (itembonuses.stringedMod == 0 && spellbonuses.stringedMod == 0)
			effectmod = 10;
		else if (GetSkill(SkillStringedInstruments) == 0)
			effectmod = 10;
		else if (itembonuses.stringedMod > spellbonuses.stringedMod)
			effectmod = itembonuses.stringedMod;
		else
			effectmod = spellbonuses.stringedMod;
		if (IsBardSong(spell_id))
			effectmod += aabonuses.stringedMod;
		break;
	case SkillWindInstruments:
		if (itembonuses.windMod == 0 && spellbonuses.windMod == 0)
			effectmod = 10;
		else if (GetSkill(SkillWindInstruments) == 0)
			effectmod = 10;
		else if (itembonuses.windMod > spellbonuses.windMod)
			effectmod = itembonuses.windMod;
		else
			effectmod = spellbonuses.windMod;
		if (IsBardSong(spell_id))
			effectmod += aabonuses.windMod;
		break;
	case SkillBrassInstruments:
		if (itembonuses.brassMod == 0 && spellbonuses.brassMod == 0)
			effectmod = 10;
		else if (GetSkill(SkillBrassInstruments) == 0)
			effectmod = 10;
		else if (itembonuses.brassMod > spellbonuses.brassMod)
			effectmod = itembonuses.brassMod;
		else
			effectmod = spellbonuses.brassMod;
		if (IsBardSong(spell_id))
			effectmod += aabonuses.brassMod;
		break;
	case SkillSinging:
		if (itembonuses.singingMod == 0 && spellbonuses.singingMod == 0)
			effectmod = 10;
		else if (itembonuses.singingMod > spellbonuses.singingMod)
			effectmod = itembonuses.singingMod;
		else
			effectmod = spellbonuses.singingMod;
		if (IsBardSong(spell_id))
			effectmod += aabonuses.singingMod + spellbonuses.Amplification;
		break;
	default:
		effectmod = 10;
		return effectmod;
	}
	if (!RuleB(Character, UseSpellFileSongCap))
		effectmodcap += aabonuses.songModCap + spellbonuses.songModCap + itembonuses.songModCap;
	if (effectmod < 10)
		effectmod = 10;
	if (!nocap && effectmod > effectmodcap) // if the cap is calculated to be 0 using new rules, no cap.
		effectmod = effectmodcap;
	Log.Out(Logs::Detail, Logs::Spells, "%s::GetInstrumentMod() spell=%d mod=%d modcap=%d\n", GetName(), spell_id,
		effectmod, effectmodcap);
	return effectmod;
}

void Client::CalcMaxEndurance()
{
	max_end = CalcBaseEndurance() + spellbonuses.Endurance + itembonuses.Endurance + aabonuses.Endurance;
	if (max_end < 0) {
		max_end = 0;
	}
	if (cur_end > max_end) {
		cur_end = max_end;
	}
	int end_perc_cap = spellbonuses.EndPercCap[0];
	if (end_perc_cap) {
		int curEnd_cap = (max_end * end_perc_cap) / 100;
		if (cur_end > curEnd_cap || (spellbonuses.EndPercCap[1] && cur_end > spellbonuses.EndPercCap[1])) {
			cur_end = curEnd_cap;
		}
	}
}

int32 Client::CalcBaseEndurance()
{
	int32 base_end = 0;
	if (ClientVersion() >= EQEmu::versions::ClientVersion::SoF && RuleB(Character, SoDClientUseSoDHPManaEnd)) {
		double heroic_stats = (GetHeroicSTR() + GetHeroicSTA() + GetHeroicDEX() + GetHeroicAGI()) / 4.0f;
		double stats = (GetSTR() + GetSTA() + GetDEX() + GetAGI()) / 4.0f;
		if (stats > 201.0f) {
			stats = 1.25f * (stats - 201.0f) + 352.5f;
		}
		else if (stats > 100.0f) {
			stats = 2.5f * (stats - 100.0f) + 100.0f;
		}
		auto base_data = database.GetBaseData(GetLevel(), GetClass());
		if (base_data) {
			base_end = base_data->base_end + (heroic_stats * 10.0f) + (base_data->endurance_factor * static_cast<int>(stats));
		}
	}
	else {
		int Stats = GetSTR() + GetSTA() + GetDEX() + GetAGI();
		int LevelBase = GetLevel() * 15;
		int at_most_800 = Stats;
		if (at_most_800 > 800) {
			at_most_800 = 800;
		}
		int Bonus400to800 = 0;
		int HalfBonus400to800 = 0;
		int Bonus800plus = 0;
		int HalfBonus800plus = 0;
		int BonusUpto800 = int( at_most_800 / 4 ) ;
		if (Stats > 400) {
			Bonus400to800 = int( (at_most_800 - 400) / 4 );
			HalfBonus400to800 = int( std::max( ( at_most_800 - 400 ), 0 ) / 8 );
			if (Stats > 800) {
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

int32 Client::CalcEnduranceRegen()
{
	int32 regen = int32(GetLevel() * 4 / 10) + 2;
	regen += aabonuses.EnduranceRegen + spellbonuses.EnduranceRegen + itembonuses.EnduranceRegen;
	return (regen * RuleI(Character, EnduranceRegenMultiplier) / 100);
}

int32 Client::CalcEnduranceRegenCap()
{
	int cap = (RuleI(Character, ItemEnduranceRegenCap) + itembonuses.HeroicSTR / 25 + itembonuses.HeroicDEX / 25 + itembonuses.HeroicAGI / 25 + itembonuses.HeroicSTA / 25);
	return (cap * RuleI(Character, EnduranceRegenMultiplier) / 100);
}

int32 Client::CalcItemATKCap()
{
	int cap = RuleI(Character, ItemATKCap) + itembonuses.ItemATKCap + spellbonuses.ItemATKCap + aabonuses.ItemATKCap;
	return cap;
}

int Client::GetRawACNoShield(int &shield_ac) const
{
	int ac = itembonuses.AC + spellbonuses.AC + aabonuses.AC;
	shield_ac = 0;
	const ItemInst *inst = m_inv.GetItem(EQEmu::legacy::SlotSecondary);
	if (inst) {
		if (inst->GetItem()->ItemType == ItemTypeShield) {
			ac -= inst->GetItem()->AC;
			shield_ac = inst->GetItem()->AC;
			for (uint8 i = AUG_INDEX_BEGIN; i < EQEmu::legacy::ITEM_COMMON_SIZE; i++) {
				if (inst->GetAugment(i)) {
					ac -= inst->GetAugment(i)->GetItem()->AC;
					shield_ac += inst->GetAugment(i)->GetItem()->AC;
				}
			}
		}
	}
	return ac;
}
