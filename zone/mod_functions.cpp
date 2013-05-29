#include "../common/debug.h"
#include "../common/timer.h"
#include <cmath>
#include <stdlib.h>
#include "spawn2.h"
#include "entity.h"
#include "masterentity.h"
#include "zone.h"
#include "spawngroup.h"
#include "zonedb.h"
#include "npc.h"
#include "mob.h"
#include "client.h"
#include "worldserver.h"
#include "QuestParserCollection.h"
#include "event_codes.h"
#include "embparser.h"
#include <string>
#include <iostream>

extern EntityList entity_list;
extern Zone* zone;

extern WorldServer worldserver;

using namespace std;

#define DW_STATBASE 70

void Zone::mod_init() {
    const NPCType* tmp = 0;
    if((tmp = database.GetNPCType(999999)))
    {
        NPC* npc = new NPC(tmp, 0, 0, 0, 0, 0, FlyMode3);
        if(npc)
        {
            entity_list.AddNPC(npc);
        }
    }
}

void Zone::mod_repop() {
    const NPCType* tmp = 0;
    if((tmp = database.GetNPCType(999999)))
    {
        NPC* npc = new NPC(tmp, 0, 0, 0, 0, 0, FlyMode3);
        if(npc)
        {
            entity_list.AddNPC(npc);
        }
    }
}

void NPC::mod_prespawn(Spawn2 *sp) {
    //The spawn has to have 1 kill to qualify
    if(sp->GetKillCount() < 1) { return; }
    //Ignore existing bosses
    if(lastname[0] == '[') { return; }
    //5% chance to elevate
    if(MakeRandomInt(0, 100) > 5) { return; }

    //Let everything else happen in perl.  Our job here is to set the last name
    int npcscore = GetScore();
    std::string bosstag = "<";
    int lvs = (int)(npcscore/10);
    for(int x = 0; x < 10; x++)
    {
        if(x < lvs) { bosstag += "+"; }
        else { bosstag += "="; }
    }
    bosstag += ">";
    strn0cpy(lastname, bosstag.c_str(), sizeof(lastname));

    TempName("DYNBOSS");
}

//Base damage from NPC::Attack
int NPC::mod_npc_damage(int damage, SkillType skillinuse, int hand, const Item_Struct* weapon, Mob* other) {
	float chbonus;
	int lbonus;

//	if(!IsPet() || !GetOwner()) { return(damage); }
//	if(!GetOwner()->IsClient()) { return(damage); }

	//Client pet power bonuses
	if(GetOwner() && GetOwner()->IsClient())
	{
		chbonus = (float)((float)GetOwner()->CastToClient()->GetActCHA() - DW_STATBASE) / 100;
		chbonus += 1;

		if(GetOwner()->GetLevel() > 50)
		{
			lbonus = GetOwner()->GetLevel() - 50;
			damage += lbonus * (20 * lbonus);
		}

		if(weapon)
		{
			damage += (int)ceil(((float)weapon->Damage / (float)weapon->Delay) * (float)GetOwner()->GetLevel());
		}

		damage = (int)ceil( (float)damage * chbonus );
		return(damage);
	}

	//Regular NPC damage - test for debuffs that currently do nothing.
	//str, sta, agi, dex, AC, ATK
	return(damage);
}

//Mob c has been given credit for a kill.  This is called after the regular EVENT_KILLED_MERIT event.
void NPC::mod_npc_killed_merit(Mob* c) {
	Mob *tmpmob = entity_list.GetMobByNpcTypeID(999999);
	if(tmpmob)
	{
		parse->EventNPC(EVENT_KILLED_MERIT, tmpmob->CastToNPC(), c, this->GetCleanName(), this->GetNPCTypeID());
	}
}

//Mob oos has been given credit for a kill.  This is called after the regular EVENT_DEATH event.
void NPC::mod_npc_killed(Mob* oos) {
	Mob *tmpmob = entity_list.GetMobByNpcTypeID(999999);
	if(tmpmob)
	{
		parse->EventNPC(EVENT_DEATH, tmpmob->CastToNPC(), oos, this->GetCleanName(), this->GetNPCTypeID());
	}
}

//Base damage from Client::Attack - can cover myriad skill types
int Client::mod_client_damage(int damage, SkillType skillinuse, int hand, const ItemInst* weapon, Mob* other) {
	float dmult = 1.1f;
	float cdex = (float)GetActDEX() - DW_STATBASE;
	float cstr = (float)GetActSTR() - DW_STATBASE;
	float cagi = (float)GetActAGI() - DW_STATBASE;
	float csta = (float)GetActSTA() - DW_STATBASE;

	//if(Admin() > 80) { Message(315, "Dex: %f, Str: %f, Agi: %f, Sta: %f", cdex, cstr, cagi, csta); }
	//if(Admin() > 80) { Message(315, "Str: %f, %d, %d", cstr, GetMaxSTR(), GetSTR()); }

	float cmax = ((float)GetLevel() * 2) + 150;
	if(GetLevel() > 49) { cmax = 10000; }

	if(cdex > cmax) { cdex = cmax; }
	if(cstr > cmax) { cstr = cmax; }
	if(cagi > cmax) { cagi = cmax; }
	if(csta > cmax) { csta = cmax; }

    switch(skillinuse)
    {
        case ARCHERY:
            dmult += cdex / 900;
            dmult += csta / 900;

            if(GetClass() == RANGER)
            {
                dmult += cdex / 1200;
            }
            break;

        case KICK:
        case HAND_TO_HAND:
        case BACKSTAB:
        case _1H_BLUNT:
        case _1H_SLASHING:
        case PIERCING:
        case THROWING:
            if(GetClass() == MONK || GetClass() == BEASTLORD)
            {
                if(GetRace() == BARBARIAN || GetRace() == TROLL || GetRace() == OGRE)
                {
                    dmult += cstr / 320;
                    dmult += csta / 320;
                    dmult += cdex / 150;
                    //2.325
                }
                else
                {
                    dmult += cdex / 400;
                    dmult += csta / 200;
                    dmult += cstr / 200;
                    //2.25
                }
            }
            else
            {
                dmult += cdex / 400;
                dmult += cstr / 200;
                dmult += csta / 200;
                //2.25
            }
            break;

        case BASH:
			if(GetClass() == WARRIOR)
			{
				dmult += csta / 100;
				dmult += cstr / 100;
			}
			else
			{
            	dmult += csta / 150;
            	dmult += cstr / 150;
			}
			break;

        case _2H_SLASHING:
        case _2H_BLUNT:
        case FRENZY:
			if(GetClass() == BERSERKER)
			{
				dmult += csta / 100;
				dmult += cstr / 100;
				if(berserk) { dmult += cstr / 100; }
			}
			else
			{
            	dmult += csta / 150;
            	dmult += cstr / 150;
			}
            break;

        default:
            dmult += cstr / 200;
            dmult += csta / 200;
            break;
    }

	if(GetLevel() > 50)
	{
		float lbonus = GetLevel() - 50;
		dmult += lbonus * 0.2;
	}

	int final = (int)((float)damage * dmult);

	if(skillinuse == ARCHERY)
	{
		final += (GetSkill(ARCHERY) + (GetLevel() * 2)) / 2;
		if(weapon) //We should always have a weapon here
		{
			final = (int)((float)final * (float)(0.6 + ((float)weapon->GetItem()->Delay / 100)));
		}
	}

	return(final);
}


//message is char[4096], don't screw it up. Return true for normal behavior, false to return immediately.
// Channels:
// 0  - Guild Chat
// 2  - Group Chat
// 3  - Shout
// 4  - Auction
// 5  - Out of Character
// 6  - Broadcast
// 7  - Tell
// 8  - Say
// 11 - GMSay
// 15 - Raid Chat
// 20 - UCS Relay for UF client and later
// 22 - Emotes for UF and later
bool Client::mod_client_message(char* message, uint8 chan_num) {
	//!commands for serverman
	if(message[0] == '!' && chan_num == 8)
	{
		Mob *tmpmob = entity_list.GetMobByNpcTypeID(999999);
		if(tmpmob)
		{
			parse->EventNPC(EVENT_SAY, tmpmob->CastToNPC(), this, message, 0);
			return(false);
		}
		Message(315, "Your pants are on sideways.");
		return(false);
	}

/*
	//Parse out link notation
	Seperator ms(message, ' ', 20);

	std::string finalmsg("");
	bool modded = false;
	for(int i = 0; i < ms.GetMaxArgNum(); i++)
	{
		if(i > 0) { finalmsg += " "; }
		if(strlen(ms.arg[i]) < 5) { finalmsg += ms.arg[i]; continue; }

		if( ms.arg[i][0] == '$' && ms.arg[i][1] == 'N' && ms.arg[i][2] == 'P' && ms.arg[i][3] == 'C')
		{
			std::string npclink("%!!dbnav NPC ");
			std::string npctext("NPC:");
			for(int x = 4; x < strlen(ms.arg[i]); x++)
			{
				npclink += ms.arg[i][x];
				npctext += ms.arg[i][x];
			}
			modded = true;
			char text[250];
			sprintf(text, "%S", npclink.c_str());
			finalmsg += quest_manager.saylink( text, true, const_cast<char*>(npctext.c_str()) );
		}
		else if( ms.arg[i][0] == '$' && ms.arg[i][1] == 'T' && ms.arg[i][2] == 'E' && ms.arg[i][3] == 'S' && ms.arg[i][4] == 'T')
		{
			modded = true;
			char text[250];
			sprintf(text, "%%!!help");
			finalmsg += quest_manager.saylink( text, true, text );
			finalmsg += "DBLINK TEST";
		}
		else
		{
			finalmsg += ms.arg[i];
		}
	}

	if(modded)
	{
	    strn0cpy(message, finalmsg.c_str(), 4096);
	}
*/
	return(true);
} //Potentially dangerous string handling here

//Skillup override.  When this is called the regular skillup check has failed.  Return false to proceed with default behavior.
//This will NOT allow a client to increase skill past a cap.
bool Client::mod_can_increase_skill(SkillType skillid, Mob* against_who) {
	//Let people skillup on golem training dummies.
	if(against_who->GetRace() == 405) { return(true); }
	return(false);
}

//chance of general skill increase, rolled against 0-99 where higher chance is better.
int16 Client::mod_increase_skill_chance(int16 chance, Mob* against_who) {
    float cint = (float)GetActINT() - DW_STATBASE;
    float cwis = (float)GetActWIS() - DW_STATBASE;

    float bonus = (cint + (cwis / 2)) / 10;

    if(bonus < 0.8) { bonus = 0.8; }

    return( (int)((float)chance * bonus) );
}

//Max percent of health you can bind wound starting with default value for class, item, and AA bonuses
int Client::mod_bindwound_percent(int max_percent, Mob* bindmob) {
	return(max_percent + 20);
}

//Final bind HP value after bonuses
int Client::mod_bindwound_hp(int bindhps, Mob* bindmob) {
	if(GetSkill(BIND_WOUND) > 200)
	{
		bindhps += GetSkill(BIND_WOUND) * 2;
	}
	else
	{
		bindhps += GetSkill(BIND_WOUND);
	}

	if(GetLevel() > 50)
	{
		float bonus = ((GetLevel() - 50) / 10) + 1;
		bindhps += (int)( (float)bindhps * bonus );
	}

	return(bindhps);
}

//Client haste as calculated by default formulas - In percent from 0-100
int Client::mod_client_haste(int h) {
	float agibase = (float)GetActAGI() - DW_STATBASE;
	float agibonus = 0;

	if(agibase > 0)
	{
		agibonus = agibase;

		float l1, l2, l3;

		switch(GetClass())
		{
			case ROGUE:
			case MONK:
				l1 = 1.5;
				l2 = 2.0;
				l3 = 2.5;
				break;

			default:
				l1 = 1.0;
				l2 = 1.5;
				l3 = 2.0;
		}

		if(GetLevel() > 19) { agibonus = agibase * l1; }
		if(GetLevel() > 39) { agibonus = agibase * l2; }
		if(GetLevel() > 59) { agibonus = agibase * l3; }
	}

	h += (int)(agibonus / 10);
	return(h);
}

//Haste cap override
int Client::mod_client_haste_cap(int cap)
{
	return( RuleI(Character, HasteCap) );
}

//This is called when a client cons a mob
void Client::mod_consider(Mob* tmob, Consider_Struct* con) {
	if(tmob->GetLastName()[0] == '[')
	{
		Message(14, "%s", tmob->GetLastName());
	}
}

//Return true to continue with normal behavior
bool Client::mod_saylink(const std::string& response, bool silentsaylink) {
	if(silentsaylink && strlen(response.c_str()) > 1 && response[0] == '%' && response[1] == '!')
	{
		Mob *tmpmob = entity_list.GetMobByNpcTypeID(999999);
		if(tmpmob)
		{
			parse->EventNPC(EVENT_SAY, tmpmob->CastToNPC(), this, response, 0);
			return(false);
		}
		Message(315, "Your pants are on sideways.");
		return(false);
	}
	return(true);
}

//Client pet power as calculated by default formulas and bonuses
int16 Client::mod_pet_power(int16 act_power, uint16 spell_id) {
	act_power += (int)(((float)GetActCHA() - DW_STATBASE) /2 ) + GetLevel();
	return(act_power);
}

//Chance to combine rolled against a random 0-99 where higher is better.
float Client::mod_tradeskill_chance(float chance, DBTradeskillRecipe_Struct *spec) {
	chance += 10;
	return(chance);
}

//Chance to skillup rolled against a random 0-99 where higher is better.
float Client::mod_tradeskill_skillup(float chance_stage2) {
	float cint = (float)CastToClient()->GetActINT() - DW_STATBASE;
	float cwis = (float)CastToClient()->GetActWIS() - DW_STATBASE;

	float bonus = (cint + (cwis / 2)) / 10;

	if(bonus < 0) { bonus = 0; }
	return(chance_stage2 + bonus);
}

//Tribute value override
int32 Client::mod_tribute_item_value(int32 pts, const ItemInst* item) {
	return(0);
}

//Death reporting
void Client::mod_client_death_npc(Mob* killerMob) {
	Mob *tmpmob = entity_list.GetMobByNpcTypeID(999999);
	if(tmpmob)
	{
		parse->EventNPC(EVENT_SLAY, tmpmob->CastToNPC(), this, killerMob->GetCleanName(), killerMob->GetLevel());
	}
}

void Client::mod_client_death_duel(Mob* killerMob) {
	Mob *tmpmob = entity_list.GetMobByNpcTypeID(999999);
	if(tmpmob)
	{
		std::string pname = std::string("PLAYER|") + killerMob->GetCleanName();
		parse->EventNPC(EVENT_SLAY, tmpmob->CastToNPC(), this, pname.c_str(), killerMob->GetLevel());
	}
}

void Client::mod_client_death_env() {
	Mob *tmpmob = entity_list.GetMobByNpcTypeID(999999);
	if(tmpmob)
	{
		parse->EventNPC(EVENT_SLAY, tmpmob->CastToNPC(), this, "ENVIRONMENT", 0);
	}
}

//Calculated xp before consider modifier
int32 Client::mod_client_xp(int32 in_xp, NPC *npc) {
    float xpmult = 2.0f;

    if(GetLevel() > 5)
    {
        xpmult =  1.5f;
    }
    if(GetLevel() > 10)
    {
        xpmult = 1.25f;
    }
    if(GetLevel() > 15)
    {
        xpmult = 1.0f;
    }

	return( (int32)((float)in_xp * xpmult) );
}

//Client XP formula.  Changes here will cause clients to change level after gaining or losing xp.
//Either modify this before your server goes live, or be prepared to write a quest script that fixes levels.
uint32 Client::mod_client_xp_for_level(uint32 xp, uint16 check_level) {
	uint16 check_levelm1 = check_level-1;
	float mod;

	mod = 4;
	float base = (check_levelm1)*(check_levelm1)*(check_levelm1);
	mod *= 1000;

	return(uint32(base * mod));
}

//effect_vallue - Spell effect value as calculated by default formulas.  You will want to ignore effects that don't lend themselves to scaling - pet ID's, gate coords, etc.
int Mob::mod_effect_value(int effect_value, uint16 spell_id, int effect_type, Mob* caster) {
//	if(IsClient()) { return(effect_value); }
	if(!caster) { return(effect_value); }
	if(!caster->IsClient()) { return(effect_value); }

	float mult = 1.0f;
	float spbonus = 0.0f;
	float spadd = 0.0f;

	int adval = caster->CastToClient()->Admin();
	adval = 0;

    if(caster->GetCasterClass() == 'W')
    {
        spbonus = (float)(caster->CastToClient()->GetActWIS() - DW_STATBASE) / 400;
        spbonus += (float)(caster->CastToClient()->GetActSTA() - DW_STATBASE) / 400;
		if(adval > 80) { caster->Message(315, "WIS FORMULA"); }
    }
    else if(caster->GetCasterClass() == 'I')
    {
	    if(caster->GetClass() == BARD)
    	{
        	spbonus = (float)(caster->CastToClient()->GetActCHA() - DW_STATBASE) / 400;
        	spbonus += (float)(caster->CastToClient()->GetActINT() - DW_STATBASE) / 400;
			if(adval > 80) { caster->Message(315, "CHA FORMULA"); }
    	}
		else
		{
        	spbonus = (float)(caster->CastToClient()->GetActINT() - DW_STATBASE) / 400;
        	spbonus += (float)(caster->CastToClient()->GetActDEX() - DW_STATBASE) / 400;
			if(adval > 80) { caster->Message(315, "INT FORMULA"); }
		}
    }
	else
	{
		//No proc/click bonuses for non casters
		if(adval > 80) { caster->Message(315, "CASTER CLASS NOT FOUND"); }
		return(effect_value);
	}

    //Add a fixed value to help things like bard songs, dots, and other low effect value spells
    spadd = (mult + spbonus) * ((float)caster->GetLevel() / 10);
    if(caster->GetLevel() > 50)
    {
        float lbonus = caster->GetLevel() - 50;
        spadd += spadd * (lbonus / 2);
    }

    switch(effect_type)
    {
        case SE_CriticalHitChance:
        case SE_SpellCritChance:
        case SE_CrippBlowChance:
        case SE_AvoidMeleeChance:
        case SE_RiposteChance:
        case SE_DodgeChance:
        case SE_ParryChance:
        case SE_DualWieldChance:
        case SE_DoubleAttackChance:
        case SE_MovementSpeed:
        case SE_AttackSpeed4:
        case SE_AttackSpeed3:
        case SE_AttackSpeed2:
        case SE_AttackSpeed:
            if(caster->GetLevel() < 30) { spbonus = spbonus / 2; }
            if(caster->GetLevel() >= 30 && caster->GetLevel() < 46) { spbonus = spbonus / 1.5; }
            mult += spbonus;
            //No fixed bonus on these types
            spadd = 0;
            break;

        case SE_CurrentHPOnce:
        case SE_CurrentHP:
            if(caster->GetLevel() < 30) { spbonus = spbonus / 2; }
            if(caster->GetLevel() >= 30 && caster->GetLevel() < 46) { spbonus = spbonus / 1.5; }
            mult += spbonus;
            break;

        case SE_DamageShield:
        case SE_Stun:
        case SE_ArmorClass:
        case SE_ATK:
        case SE_STR:
        case SE_DEX:
        case SE_AGI:
        case SE_STA:
        case SE_INT:
        case SE_WIS:
        case SE_CHA:
        case SE_CurrentMana:
        case SE_Lull:
        case SE_AddFaction:
        case SE_Stamina:
        case SE_ChangeFrenzyRad:
        case SE_DiseaseCounter:
        case SE_PoisonCounter:
        case SE_ResistFire:
        case SE_ResistCold:
        case SE_ResistPoison:
        case SE_ResistDisease:
        case SE_ResistMagic:
        case SE_Rune:
        case SE_TotalHP:
        case SE_TossUp:
        case SE_ManaPool:
        case SE_HealOverTime:
        case SE_CastingLevel:
        case SE_Hunger:
        case SE_CurseCounter:
        case SE_HealRate:
        case SE_ImprovedDamage:
        case SE_ImprovedHeal:
        case SE_SpellResistReduction:
        case SE_IncreaseSpellHaste:
        case SE_IncreaseSpellDuration:
        case SE_IncreaseRange:
        case SE_AllStats:
        case SE_MeleeLifetap:
        case SE_AllInstrumentMod:
        case SE_ResistSpellChance:
        case SE_ResistFearChance:
        case SE_HitChance:
        case SE_DamageModifier:
        case SE_MinDamageModifier:
        case SE_IncreaseBlockChance:
        case SE_CurrentEndurance:
        case SE_EndurancePool:
        case SE_CurrentEnduranceOnce:
        case SE_MaxHPChange:
        case SE_Accuracy:
        case SE_BardAEDot:
        case SE_CurrentManaOnce:
        case SE_FactionMod:
        case SE_CorruptionCounter:
        case SE_ResistCorruption:
            mult += spbonus;
            break;

        default:
			if(adval > 80) { caster->Message(315, "Generic effect - ignored"); }
			return(effect_value);
            break;
    }

	//Shroud of the bear and Blood of the Master
	if(caster->FindBuff(5045) || caster->FindBuff(1169))
	{
		if(adval > 80) { caster->Message(315, "Shroud of bear nerf"); }
		spadd = spadd * -1;
		mult = 1.0f;
	}

	int base_effect = effect_value;

    if(effect_value > 0) { effect_value += (int)ceil(spadd); }
    else { effect_value -= (int)ceil(spadd); }

    effect_value = (int)( ceil((float)effect_value * mult) );

	if(adval > 80) { caster->Message(315, "Spell Bonus: Base: %d, New: %d, Add: %d", base_effect, effect_value, spadd); }

    return( effect_value );
}

//chancetohit - 0 to 100 percent - set over 1000 for a guaranteed hit
float Mob::mod_hit_chance(float chancetohit, SkillType skillinuse, Mob* attacker) {
	if(!IsClient())
	{
		//Factor NPC debuffs
		return(chancetohit);
	}

	int divisor;

	switch(GetClass())
	{
		case MONK:
		case ROGUE:
			divisor = 8;
			divisor -= (GetLevel() / 100) * 2;
			break;

		default:
			divisor = 8;
	}

	float hitmod = ((float)CastToClient()->GetActDEX() - DW_STATBASE) / divisor;
	hitmod += ((float)CastToClient()->GetActAGI() - DW_STATBASE) / divisor;
	if(hitmod < 0) { hitmod = -5; }

	return(chancetohit + hitmod);
}

//Final riposte chance
float Mob::mod_riposte_chance(float ripostechance, Mob* attacker) {
	if(!IsClient()) { return(ripostechance); }
	return(ripostechance + CastToClient()->GetActSTA()/200);
}

//Final block chance
float Mob::mod_block_chance(float blockchance, Mob* attacker) {
	if(!IsClient()) { return(blockchance); }
	return(blockchance + CastToClient()->GetActSTA()/200);
}

//Final parry chance
float Mob::mod_parry_chance(float parrychance, Mob* attacker) {
	if(!IsClient()) { return(parrychance); }
	return(parrychance + CastToClient()->GetActDEX()/200);
}

//Final dodge chance
float Mob::mod_dodge_chance(float dodgechance, Mob* attacker) {
	if(!IsClient()) { return(dodgechance); }
	return(dodgechance + CastToClient()->GetActAGI()/200);
}

//Monk AC Bonus weight cap.  Defined in Combat:MonkACBonusWeight
//Usually 15, a monk under this weight threshold gets an AC bonus
float Mob::mod_monk_weight(float monkweight, Mob* attacker) {
	if(!IsClient()) { return(monkweight); }

	monkweight += (CastToClient()->GetActAGI()/100) + (CastToClient()->GetActSTR()/50);
	return(monkweight);
}

//Mitigation rating is compared to incoming attack rating.  Higher is better.
float Mob::mod_mitigation_rating(float mitigation_rating, Mob* attacker) {
	if(!IsClient()) { return(mitigation_rating); }

	int shield_ac = 0;
	float armor = (float)CastToClient()->GetRawACNoShield(shield_ac);

	switch(GetClass())
	{
		case WARRIOR:
			return(mitigation_rating + (armor/2));

		case SHADOWKNIGHT:
		case PALADIN:
			return(mitigation_rating + (armor/3));

		case RANGER:
		case ROGUE:
		case BARD:
			return(mitigation_rating + (armor/5));

		case BERSERKER:
			if(CastToClient()->berserk) { return(mitigation_rating / 1.5); }
			else { return(mitigation_rating); }

		default:
			return(mitigation_rating);
	}

	//Shouldn't ever get here
	return(mitigation_rating);
}

float Mob::mod_attack_rating(float attack_rating, Mob* defender) {
	if(!IsClient()) { return(attack_rating); }

	float hprmult = 1 + (1 - (GetHP()/GetMaxHP()));

	switch(GetClass())
	{
		case BERSERKER:
			return(attack_rating * hprmult);

		case ROGUE:
			if( BehindMob(defender, GetX(), GetY()) ) { return(attack_rating * 2); }
			else { return(attack_rating); }

		default:
			return(attack_rating);
	}

	return(attack_rating);
}

//Kick damage after all other bonuses are applied
int32 Mob::mod_kick_damage(int32 dmg) {
	if(!IsClient()) { return(dmg); }

	ItemInst *item = CastToClient()->GetInv().GetItem(SLOT_FEET);
	if(item)
	{
		dmg += item->GetItem()->AC;
		for(int i = 0; i < MAX_AUGMENT_SLOTS; ++i)
		{
			ItemInst *aug = item->GetAugment(i);
			if(aug)
			{
				dmg += aug->GetItem()->AC;
			}
		}
	}
	dmg += GetLevel() / 4;
	dmg = CastToClient()->mod_client_damage(dmg, KICK, 0, nullptr, nullptr);

	return(dmg);
}

//Slam and bash damage after all other bonuses are applied
int32 Mob::mod_bash_damage(int32 dmg) {
	if(!IsClient()) { return(dmg); }

    float lmult = (((float)GetLevel() * 2 ) / 100) + 1;
	bool foundshield = false;

    ItemInst *item = CastToClient()->GetInv().GetItem(SLOT_SECONDARY);
    if(item)
    {
        if(item->GetItem()->ItemType == ItemTypeShield)
        {
            foundshield = true;
            dmg += item->GetItem()->AC * lmult;
            for(int i = 0; i < MAX_AUGMENT_SLOTS; ++i)
            {
                ItemInst *aug = item->GetAugment(i);
                if(aug)
                {
                    dmg += aug->GetItem()->AC;
                }
            }
        }
    }

    if(!foundshield) //This is from a slam
    {
        item = CastToClient()->GetInv().GetItem(SLOT_SHOULDER);
        if(item)
        {
            if(item->GetItem()->ItemType == ItemTypeArmor)
            {
                dmg += item->GetItem()->AC * lmult;
                for(int i = 0; i < MAX_AUGMENT_SLOTS; ++i)
                {
                    ItemInst *aug = item->GetAugment(i);
                    if(aug)
                    {
                        dmg += aug->GetItem()->AC;
                    }
                }
            }
        }
    }

    dmg += GetLevel() / 4;
    dmg = CastToClient()->mod_client_damage(dmg, BASH, 0, nullptr, nullptr);

	return(dmg);
}

//Frenzy damage after all other bonuses are applied
int32 Mob::mod_frenzy_damage(int32 dmg) {
	if(!IsClient()) { return(dmg); }

	float hprmult = 1 + (1 - (GetHP()/GetMaxHP()));

	dmg = CastToClient()->mod_client_damage(dmg, FRENZY, 0, nullptr, nullptr);

	if(GetClass() == BERSERKER)
	{
		dmg = (int)( (float)dmg * hprmult );
	}

	return(dmg);
}

//Special attack damage after all other bonuses are applied.
int32 Mob::mod_monk_special_damage(int32 ndamage, SkillType skill_type) {
	if(!IsClient()) { return(ndamage); }

	ndamage = CastToClient()->mod_client_damage(ndamage, KICK, 0, nullptr, nullptr);
	return(ndamage);
}

//ndamage - Backstab damage as calculated by default formulas
int32 Mob::mod_backstab_damage(int32 ndamage) {
	if(!IsClient()) { return(ndamage); }

	float cdex = (float)CastToClient()->GetActDEX() - DW_STATBASE;
	float cstr = (float)CastToClient()->GetActSTR() - DW_STATBASE;
	float cagi = (float)CastToClient()->GetActAGI() - DW_STATBASE;
	float bsm = 1.0f;

	bsm += (cdex / 200) + ((float)GetLevel() / 100);
	bsm += cstr / 300;
	bsm += cagi / 300;

	if(GetLevel() > 50)
	{
		float lbonus = GetLevel() - 50;
		bsm += (lbonus * 0.1);
	}

	return( (int32)((float)ndamage * bsm) );
}

//Chance for 50+ archery bonus damage if Combat:UseArcheryBonusRoll is true.  Base is Combat:ArcheryBonusChance
int Mob::mod_archery_bonus_chance(int bonuschance, const ItemInst* RangeWeapon) { return(bonuschance); }

//Archery bonus damage
uint32 Mob::mod_archery_bonus_damage(uint32 MaxDmg, const ItemInst* RangeWeapon) { return(MaxDmg); }

//Final archery damage including bonus if it was applied.
int32 Mob::mod_archery_damage(int32 TotalDmg, bool hasbonus, const ItemInst* RangeWeapon) {
	if(!IsClient()) { return(TotalDmg); }

	TotalDmg += (GetLevel() * 2);
	TotalDmg = CastToClient()->mod_client_damage(TotalDmg, ARCHERY, 0, RangeWeapon, nullptr);

	return(TotalDmg);
}

//Thrown weapon damage after all other calcs
uint16 Mob::mod_throwing_damage(uint16 MaxDmg) {
	if(!IsClient()) { return(MaxDmg); }

	MaxDmg = CastToClient()->mod_client_damage(MaxDmg, THROWING, 0, nullptr, nullptr);
	return(MaxDmg);
}

int32 Mob::mod_cast_time(int32 cast_time) {
	if(!IsClient()) { return(cast_time); }

	float ctmod = 1;
	float cdex = (float)CastToClient()->GetActDEX() - DW_STATBASE;
	float ccast = 0;

	if(GetCasterClass() == 'W')
	{
		ccast = (float)CastToClient()->GetActWIS() - DW_STATBASE;
	}
	if(GetCasterClass() == 'I')
	{
		ccast = (float)CastToClient()->GetActINT() - DW_STATBASE;
	}
	if(ccast < 0.0001) { return(cast_time); }

	ctmod += cdex / 185;
	ctmod += ccast / 185;

	if(GetLevel() > 50)
	{
		float lbonus = GetLevel() - 50;
		ctmod += (lbonus * 0.1);
	}

	return( (int32)((float)cast_time / ctmod) );
}

//res - Default buff duration formula
int Mob::mod_buff_duration(int res, Mob* caster, Mob* target, uint16 spell_id) {
    //if(!IsClient()) { return(res); }
    if(res <= 1) { return(res); }
    if(!caster) { return(res); }
    if(!caster->IsClient()) { return(res); }

    float cmult = 1;

//    caster->Message(315, "Duration: %d", res);

    if(caster->GetClass() == BARD)
    {
        cmult = (float)(caster->CastToClient()->GetActCHA() - DW_STATBASE) / 200;
    }
    else if(caster->GetCasterClass() == 'W')
    {
        cmult = (float)(caster->CastToClient()->GetActWIS() - DW_STATBASE) / 200;
    }
    else if(caster->GetCasterClass() == 'I')
    {
        cmult = (float)(caster->CastToClient()->GetActINT() - DW_STATBASE) / 200;
    }

    cmult += (float)(caster->CastToClient()->GetActSTA() - DW_STATBASE) / 400;

    if(cmult < 1) { cmult = 1; }

    return( (int)((float)res * cmult) );
}

//Spell stack override - If this returns anything < 2, it will ignore all other stacking rules.
// See spells.cpp: Mob::CheckStackConflict
//  0 - No conflict
//  1 - Overwrite, spellid1 is replaced by spellid2
// -1 - Blocked, spellid2 will not land
//  2 - Default stacking behavior
int Mob::mod_spell_stack(uint16 spellid1, int caster_level1, Mob* caster1, uint16 spellid2, int caster_level2, Mob* caster2) {

	if(IsEffectInSpell(spellid1, SE_Illusion) && IsEffectInSpell(spellid2, SE_Illusion))
	{
		return(1);
	}
	return(2);
}

//Sum of various resists rolled against a value of 200.
int Mob::mod_spell_resist(int resist_chance, int level_mod, int resist_modifier, int target_resist, uint8 resist_type, uint16 spell_id, Mob* caster) {
	int final = resist_chance + level_mod + resist_modifier + target_resist;

	if(final > 185) { final = 185; } // Cap resist so it's always possible to land a spell (unless we hit the client level diff max).

	if(!caster) { return(final); }

	int temp_level_diff = GetLevel() - caster->GetLevel();
   	if(temp_level_diff > 15 && caster->GetLevel() < 46)
   	{
       	if(caster->IsClient())
       	{
           	if(caster->CastToClient()->GetAAXP() < 100)
           	{
               	return(0);
           	}
       	}
   	}

	if(!caster->IsClient()) { return(final); }

	float resistmod = 1.0f;

	//Make charisma a part of all resists
	resistmod += ((float)caster->CastToClient()->GetActCHA() - DW_STATBASE) / 20;

	//The other half is the casting stat
	if(caster->GetClass() == BARD)
	{
		resistmod += ((float)caster->CastToClient()->GetActCHA() - DW_STATBASE) / 20;
	}
	else if(caster->GetCasterClass() == 'W')
	{
	resistmod += ((float)caster->CastToClient()->GetActWIS() - DW_STATBASE) / 20;
	}
	else if(caster->GetCasterClass() == 'I')
	{
		resistmod += ((float)caster->CastToClient()->GetActINT() - DW_STATBASE) / 20;
	}

	final -= resistmod;

	if(caster->GetLevel() > 50)
	{
		final -= (int)( (caster->GetLevel() - 50) * 20 );
	}

	//Let the client be highly resistant to their own AoE
	if(
		(spells[spell_id].targettype == ST_AECaster || spells[spell_id].targettype == ST_AETarget) &&
		caster->CastToClient()->CharacterID() == CastToClient()->CharacterID()
	)
	{
		final = 185;
	}

	return(final);
}

//Spell is cast by this on spelltar, called from spellontarget after the event_cast_on NPC event
void Mob::mod_spell_cast(uint16 spell_id, Mob* spelltar, bool reflect, bool use_resist_adjust, int16 resist_adjust, bool isproc)
{
	float cval = 5;

	if(!IsClient()) { return; }
	if(GetClass() == WIZARD)
	{
		cval += (float)(CastToClient()->GetActINT() - DW_STATBASE) / 100;
		cval += (float)(CastToClient()->GetActDEX() - DW_STATBASE) / 100;
		cval += (float)(CastToClient()->GetActCHA() - DW_STATBASE) / 100;
		cval += (float)(CastToClient()->GetActSTA() - DW_STATBASE) / 100;

		if(cval > 50) { cval = 50; } //This absolutely needs a cap

		if(MakeRandomFloat(0, 99) < cval)
		{
			Message(14, "You channel addition power into the spell!");
			SpellOnTarget(spell_id, spelltar, reflect, use_resist_adjust, resist_adjust, isproc);

/*			This is already recursive
			if(MakeRandomFloat(0, 99) < cval)
			{
				Message(14, "You continue to channel additional power!");
				SpellOnTarget(spell_id, spelltar, reflect, use_resist_adjust, resist_adjust, isproc);

				if(MakeRandomFloat(0, 99) < cval)
				{
					Message(14, "You apply the full force of your concentration into the spell!");
					SpellOnTarget(spell_id, spelltar, reflect, use_resist_adjust, resist_adjust, isproc);
				}
			}
*/
		}
	}
}

//This is called right before regular event processing (the switch block)
void PerlembParser::mod_quest_event(QuestEventID event, uint32 objid, const char * data, NPC* npcmob, ItemInst* iteminst, Mob* mob, uint32 extradata, bool global, std::string packagename) {
	NPC *tnpc = 0;

	if(event == EVENT_KILLED_MERIT || event == EVENT_CAST_ON || event == EVENT_SLAY)
	{
		const NPCType* tmp = 0;
		if((tmp = database.GetNPCType(extradata)))
		{
			tnpc = new NPC(tmp, 0, 0, 0, 0, 0, FlyMode3);
		}
		else
		{
			tnpc = npcmob;
		}

		if(tnpc)
		{
			ExportVar(packagename.c_str(), "npcname", tnpc->GetCleanName());
			ExportVar(packagename.c_str(), "npclastname", tnpc->GetLastName());
			ExportVar(packagename.c_str(), "npclevel", tnpc->GetLevel());
			ExportVar(packagename.c_str(), "bodytype", tnpc->GetBodyType());
			ExportVar(packagename.c_str(), "npcid", tnpc->GetNPCTypeID());
			ExportVar(packagename.c_str(), "npcrace", tnpc->GetRace());
			ExportVar(packagename.c_str(), "npcclass", tnpc->GetClass());
		}
	}
}
