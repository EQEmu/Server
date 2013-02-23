/*  EQEMu:  Everquest Server Emulator
    Copyright (C) 2001-2002  EQEMu Development Team (http://eqemu.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "../common/debug.h"
#include <iostream>
using namespace std;
#include <stdio.h>
#include <stdlib.h>

#ifndef WIN32
	#include <pthread.h>
#endif
#include "../common/faction.h"
#include "zonedb.h"
#include "masterentity.h"
#include "zone.h"
#include "../common/MiscFunctions.h"

extern Zone* zone;

//TODO: This file is terrible: half of it needs to be in common and half in zone

#ifdef _WINDOWS
	#define snprintf	_snprintf
#endif

//#define FACTIONS_DEBUG 5

const char *FactionValueToString(FACTION_VALUE fv) {
	switch(fv) {
	case FACTION_ALLY:
		return("Ally");
	case FACTION_WARMLY:
		return("Warmly");
	case FACTION_KINDLY:
		return("Kindly");
	case FACTION_AMIABLE:
		return("Amiable");
    case FACTION_INDIFFERENT:
    	return("Indifferent");
    case FACTION_APPREHENSIVE:
    	return("Apprehensive");
    case FACTION_DUBIOUS:
    	return("Dubious");
    case FACTION_THREATENLY:
    	return("Threatenly");
    case FACTION_SCOWLS:
    	return("Scowls, ready to attack.");
    default:
    	break;
    }
    return("Unknown Faction Con");
}


//o--------------------------------------------------------------
//| Name: CalculateFaction; rembrant, Dec. 16, 2001
//o--------------------------------------------------------------
//| Notes: Returns the faction message value.
//|        Modify these values to taste.
//o--------------------------------------------------------------
FACTION_VALUE CalculateFaction(FactionMods* fm, int32 tmpCharacter_value)
{
#if FACTIONS_DEBUG >= 5
	LogFile->write(EQEMuLog::Debug, "called CalculateFaction(0x%x, %ld)", fm, (unsigned long)tmpCharacter_value);
#endif
	int32 character_value = tmpCharacter_value;
	if (fm)
		character_value += fm->base + fm->class_mod + fm->race_mod + fm->deity_mod;
	if(character_value >=  1101) return FACTION_ALLY;
	if(character_value >=   701 && character_value <= 1100) return FACTION_WARMLY;
	if(character_value >=   401 && character_value <=  700) return FACTION_KINDLY;
	if(character_value >=   101 && character_value <=  400) return FACTION_AMIABLE;
	if(character_value >=     0 && character_value <=  100) return FACTION_INDIFFERENT;
	if(character_value >=  -100 && character_value <=   -1) return FACTION_APPREHENSIVE;
	if(character_value >=  -700 && character_value <= -101) return FACTION_DUBIOUS;
	if(character_value >=  -999 && character_value <= -701) return FACTION_THREATENLY;
	if(character_value <= -1000) return FACTION_SCOWLS;
	return FACTION_INDIFFERENT;
}

// neotokyo: this function should check if some races have more than one race define
bool IsOfEqualRace(int r1, int r2)
{
    if (r1 == r2)
        return true;
    // TODO: add more values
    switch(r1)
    {
    case DARK_ELF:
        if (r2 == 77)
            return true;
        break;
    case BARBARIAN:
        if (r2 == 90)
            return true;
    }
    return false;
}

// neotokyo: trolls endure ogres, dark elves, ...
bool IsOfIndiffRace(int r1, int r2)
{
    if (r1 == r2)
        return true;
    // TODO: add more values
    switch(r1)
    {
    case DARK_ELF:
    case OGRE:
    case TROLL:
        if (r2 == OGRE || r2 == TROLL || r2 == DARK_ELF)
            return true;
        break;
    case HUMAN:
    case BARBARIAN:
    case HALF_ELF:
    case GNOME:
    case HALFLING:
    case WOOD_ELF:
        if (r2 == HUMAN ||
            r2 == BARBARIAN ||
            r2 == ERUDITE ||
            r2 == HALF_ELF ||
            r2 == GNOME ||
            r2 == HALFLING ||
            r2 == DWARF ||
            r2 == HIGH_ELF ||
            r2 == WOOD_ELF)
            return true;
        break;
    case ERUDITE:
        if (r2 == HUMAN || r2 == HALF_ELF)
            return true;
        break;
    case DWARF:
        if (r2 == HALFLING || r2 == GNOME)
            return true;
        break;
    case HIGH_ELF:
        if (r2 == WOOD_ELF)
            return true;
        break;
    case VAHSHIR:
        return true;
    case IKSAR:
        return false;
    }
    return false;
}

// returns what Other thinks of this
FACTION_VALUE Client::GetReverseFactionCon(Mob* iOther) {
#if FACTIONS_DEBUG >= 5
	LogFile->write(EQEMuLog::Debug, "called $s::GetReverseFactionCon(%s)", GetName(), iOther->GetName());
#endif
	
	if (GetOwnerID()) {
		return GetOwnerOrSelf()->GetReverseFactionCon(iOther);
	}
	
	iOther = iOther->GetOwnerOrSelf();
	
#if FACTIONS_DEBUG >= 5
	LogFile->write(EQEMuLog::Debug, "	%s'd primary faction = %d", iOther->GetName(), iOther->GetPrimaryFaction());
#endif
	if (iOther->GetPrimaryFaction() < 0)
		return GetSpecialFactionCon(iOther);
	
	if (iOther->GetPrimaryFaction() == 0)
		return FACTION_INDIFFERENT;

	return GetFactionLevel(CharacterID(), 0, GetRace(), GetClass(), GetDeity(), iOther->GetPrimaryFaction(), iOther);
}

//this is called with 'this' as the mob being looked at, and
//iOther the mob who is doing the looking. It should figure out
//what iOther thinks about 'this'
FACTION_VALUE NPC::GetReverseFactionCon(Mob* iOther) {
#if FACTIONS_DEBUG >= 20
	LogFile->write(EQEMuLog::Debug, "called N $s::GetReverseFactionCon(%s)", GetName(), iOther->GetName());
#endif
	
	_ZP(NPC_GetReverseFactionCon);

	iOther = iOther->GetOwnerOrSelf();
	int primaryFaction= iOther->GetPrimaryFaction();

#if FACTIONS_DEBUG >= 20
	LogFile->write(EQEMuLog::Debug, "	%s'd primary faction = %d", iOther->GetName(), primaryFaction);
#endif
	
	//I am pretty sure that this special faction call is backwards
	//and should be iOther->GetSpecialFactionCon(this)
	if (primaryFaction < 0)
		return GetSpecialFactionCon(iOther);
	
	if (primaryFaction == 0)
		return FACTION_INDIFFERENT;
	
	//if we are a pet, use our owner's faction stuff
	Mob *own = GetOwner();
	if (own != NULL)
		return own->GetReverseFactionCon(iOther);
	
	//make sure iOther is an npc
	//also, if we dont have a faction, then they arnt gunna think anything of us either
	if(!iOther->IsNPC() || GetPrimaryFaction() == 0)
		return(FACTION_INDIFFERENT);
	
	//if we get here, iOther is an NPC too
	
	//otherwise, employ the npc faction stuff
	//so we need to look at iOther's faction table to see
	//what iOther thinks about our primary faction
	return(iOther->CastToNPC()->CheckNPCFactionAlly(GetPrimaryFaction()));
}

//Look through our faction list and return a faction con based 
//on the npc_value for the other person's primary faction in our list.
FACTION_VALUE NPC::CheckNPCFactionAlly(int32 other_faction) {
	list<struct NPCFaction*>::iterator cur,end;
	cur = faction_list.begin();
	end = faction_list.end();
	for(; cur != end; cur++) {
		struct NPCFaction* fac = *cur;
		if ((int32)fac->factionID == other_faction) {
			if (fac->npc_value > 0)
				return FACTION_ALLY;
			else if (fac->npc_value < 0)
				return FACTION_SCOWLS;
			else
				return FACTION_INDIFFERENT;
		}
	}
	return FACTION_INDIFFERENT;
}


bool NPC::IsFactionListAlly(uint32 other_faction) {
/*	list<struct NPCFaction*>::iterator cur,end;
	cur = faction_list.begin();
	end = faction_list.end();
	for(; cur != end; cur++) {
		struct NPCFaction* fac = *cur;
		if (fac->factionID == other_faction && fac->npc_value > 0)
			return(true);
	}
	return(false);*/
	return(CheckNPCFactionAlly(other_faction) == FACTION_ALLY);
}

// Faction Mods for Alliance type spells
void Mob::AddFactionBonus(uint32 pFactionID,int32 bonus) {
    map <uint32, int32> :: const_iterator faction_bonus;
	typedef std::pair <uint32, int32> NewFactionBonus;

	faction_bonus = faction_bonuses.find(pFactionID);
	if(faction_bonus == faction_bonuses.end())
	{
		faction_bonuses.insert(NewFactionBonus(pFactionID,bonus));
	}
	else
	{
		if(faction_bonus->second<bonus)
		{
			faction_bonuses.erase(pFactionID);
			faction_bonuses.insert(NewFactionBonus(pFactionID,bonus));
		}
	}
}

// Faction Mods from items
void Mob::AddItemFactionBonus(uint32 pFactionID,int32 bonus) {
    map <uint32, int32> :: const_iterator faction_bonus;
	typedef std::pair <uint32, int32> NewFactionBonus;

	faction_bonus = item_faction_bonuses.find(pFactionID);
	if(faction_bonus == item_faction_bonuses.end())
	{
		item_faction_bonuses.insert(NewFactionBonus(pFactionID,bonus));
	}
	else
	{
		if((bonus > 0 && faction_bonus->second < bonus) || (bonus < 0 && faction_bonus->second > bonus))
		{
			item_faction_bonuses.erase(pFactionID);
			item_faction_bonuses.insert(NewFactionBonus(pFactionID,bonus));
		}
	}
}

int32 Mob::GetFactionBonus(uint32 pFactionID) {
    map <uint32, int32> :: const_iterator faction_bonus;
	faction_bonus = faction_bonuses.find(pFactionID);
	if(faction_bonus != faction_bonuses.end())
	{
		return (*faction_bonus).second;
	}
	return 0;
}

int32 Mob::GetItemFactionBonus(uint32 pFactionID) {
    map <uint32, int32> :: const_iterator faction_bonus;
	faction_bonus = item_faction_bonuses.find(pFactionID);
	if(faction_bonus != item_faction_bonuses.end())
	{
		return (*faction_bonus).second;
	}
	return 0;
}

void Mob::ClearItemFactionBonuses() {
	map <uint32, int32> :: iterator itr;
	for(itr = item_faction_bonuses.begin(); itr != item_faction_bonuses.end(); itr++)
	{
		item_faction_bonuses.erase(itr->first);
	}
}

FACTION_VALUE Mob::GetSpecialFactionCon(Mob* iOther) {
#if FACTIONS_DEBUG >= 5
	LogFile->write(EQEMuLog::Debug, "called $s::GetSpecialFactionCon(%s)", GetName(), iOther->GetName());
#endif
	
	if (!iOther)
		return FACTION_INDIFFERENT;

	iOther = iOther->GetOwnerOrSelf();
	Mob* self = this->GetOwnerOrSelf();

	bool selfAIcontrolled = self->IsAIControlled();
	bool iOtherAIControlled = iOther->IsAIControlled();
	int selfPrimaryFaction = self->GetPrimaryFaction();
	int iOtherPrimaryFaction = iOther->GetPrimaryFaction();

#if FACTIONS_DEBUG >= 5
	LogFile->write(EQEMuLog::Debug, "	GSFC %d %d %d %d", selfAIcontrolled, iOtherAIControlled, selfPrimaryFaction, iOtherPrimaryFaction);
#endif
	
	if (selfPrimaryFaction >= 0 && selfAIcontrolled)
		return FACTION_INDIFFERENT;
	if (iOther->GetPrimaryFaction() >= 0)
		return FACTION_INDIFFERENT;
/* special values:
	-2 = indiff to player, ally to AI on special values, indiff to AI
	-3 = dub to player, ally to AI on special values, indiff to AI
	-4 = atk to player, ally to AI on special values, indiff to AI
	-5 = indiff to player, indiff to AI
	-6 = dub to player, indiff to AI
	-7 = atk to player, indiff to AI
	-8 = indiff to players, ally to AI on same value, indiff to AI
	-9 = dub to players, ally to AI on same value, indiff to AI
	-10 = atk to players, ally to AI on same value, indiff to AI
	-11 = indiff to players, ally to AI on same value, atk to AI
	-12 = dub to players, ally to AI on same value, atk to AI
	-13 = atk to players, ally to AI on same value, atk to AI
*/
	switch (iOtherPrimaryFaction) {
		case -2: // -2 = indiff to player, ally to AI on special values, indiff to AI
			if (selfAIcontrolled && iOtherAIControlled)
				return FACTION_ALLY;
			else
				return FACTION_INDIFFERENT;
		case -3: // -3 = dub to player, ally to AI on special values, indiff to AI
			if (selfAIcontrolled && iOtherAIControlled)
				return FACTION_ALLY;
			else
				return FACTION_DUBIOUS;
		case -4: // -4 = atk to player, ally to AI on special values, indiff to AI
			if (selfAIcontrolled && iOtherAIControlled)
				return FACTION_ALLY;
			else
				return FACTION_SCOWLS;
		case -5: // -5 = indiff to player, indiff to AI
			return FACTION_INDIFFERENT;
		case -6: // -6 = dub to player, indiff to AI
			if (selfAIcontrolled && iOtherAIControlled)
				return FACTION_INDIFFERENT;
			else
				return FACTION_DUBIOUS;
		case -7: // -7 = atk to player, indiff to AI
			if (selfAIcontrolled && iOtherAIControlled)
				return FACTION_INDIFFERENT;
			else
				return FACTION_SCOWLS;
		case -8: // -8 = indiff to players, ally to AI on same value, indiff to AI
			if (selfAIcontrolled && iOtherAIControlled) {
				if (selfPrimaryFaction == iOtherPrimaryFaction)
					return FACTION_ALLY;
				else
					return FACTION_INDIFFERENT;
			}
			else
				return FACTION_INDIFFERENT;
		case -9: // -9 = dub to players, ally to AI on same value, indiff to AI
			if (selfAIcontrolled && iOtherAIControlled) {
				if (selfPrimaryFaction == iOtherPrimaryFaction)
					return FACTION_ALLY;
				else
					return FACTION_INDIFFERENT;
			}
			else
				return FACTION_DUBIOUS;
		case -10: // -10 = atk to players, ally to AI on same value, indiff to AI
			if (selfAIcontrolled && iOtherAIControlled) {
				if (selfPrimaryFaction == iOtherPrimaryFaction)
					return FACTION_ALLY;
				else
					return FACTION_INDIFFERENT;
			}
			else
				return FACTION_SCOWLS;
		case -11: // -11 = indiff to players, ally to AI on same value, atk to AI
			if (selfAIcontrolled && iOtherAIControlled) {
				if (selfPrimaryFaction == iOtherPrimaryFaction)
					return FACTION_ALLY;
				else
					return FACTION_SCOWLS;
			}
			else
				return FACTION_INDIFFERENT;
		case -12: // -12 = dub to players, ally to AI on same value, atk to AI
			if (selfAIcontrolled && iOtherAIControlled) {
				if (selfPrimaryFaction == iOtherPrimaryFaction)
					return FACTION_ALLY;
				else
					return FACTION_SCOWLS;


			}
			else
				return FACTION_DUBIOUS;
		case -13: // -13 = atk to players, ally to AI on same value, atk to AI
			if (selfAIcontrolled && iOtherAIControlled) {
				if (selfPrimaryFaction == iOtherPrimaryFaction)
					return FACTION_ALLY;
				else
					return FACTION_SCOWLS;
			}
			else
				return FACTION_SCOWLS;
		default:
			return FACTION_INDIFFERENT;
	}
}

//o--------------------------------------------------------------
//| Name: GetFactionLevel; rembrant, Dec. 16, 2001
//o--------------------------------------------------------------
//| Notes: Gets the characters faction standing with the
//|        specified NPC.
//|        Will return Indifferent on failure.
//o--------------------------------------------------------------
FACTION_VALUE Client::GetFactionLevel(uint32 char_id, uint32 npc_id, uint32 p_race, uint32 p_class, uint32 p_deity, int32 pFaction, Mob* tnpc)
{
#if FACTIONS_DEBUG >= 5
	LogFile->write(EQEMuLog::Debug, "called %s::GetFactionLevel(%lu, %lu, %lu, %lu, %lu, %lu, %s)", GetName(), (unsigned long)char_id, (unsigned long)npc_id, (unsigned long)p_race, (unsigned long)p_class, (unsigned long)p_deity, (unsigned long)pFaction, tnpc?tnpc->GetName():"(NULL)");
#endif
	
	_ZP(Client_GetFactionLevel);

	if (pFaction < 0)
		return GetSpecialFactionCon(tnpc);
	FACTION_VALUE fac = FACTION_INDIFFERENT;
	//int32 pFacValue;  -Trumpcard: commenting. Not currently used.
	int32 tmpFactionValue;
	FactionMods fmods;

    // neotokyo: few optimizations
    if (GetFeigned())
		return FACTION_INDIFFERENT;
    if (invisible_undead && tnpc && !tnpc->SeeInvisibleUndead())
        return FACTION_INDIFFERENT;
    if (IsInvisible(tnpc))
		return FACTION_INDIFFERENT;
    if (tnpc && tnpc->GetOwnerID() != 0) // pets con amiably to owner and indiff to rest
		if (char_id == tnpc->GetOwner()->CastToClient()->CharacterID())
			return FACTION_AMIABLE;
		else
			return FACTION_INDIFFERENT;

    //First get the NPC's Primary faction
	if(pFaction > 0)
	{
		//Get the faction data from the database
		if(database.GetFactionData(&fmods, p_class, p_race, p_deity, pFaction))
		{
			//Get the players current faction with pFaction
			tmpFactionValue = GetCharacterFactionLevel(pFaction);
			// Everhood - tack on any bonuses from Alliance type spell effects
			tmpFactionValue += GetFactionBonus(pFaction);
			tmpFactionValue += GetItemFactionBonus(pFaction);
			//Return the faction to the client
			fac = CalculateFaction(&fmods, tmpFactionValue);
			//Message(0,"Faction: %i %i %i %i",fmods.base,fmods.class_mod,fmods.race_mod,fmods.deity_mod);
			//Message(0,"tmpFactionValue: %i, fac: %i",tmpFactionValue,fac);
		}
	}
	else
    {    	//pFaction == 0
    	return(FACTION_INDIFFERENT);
    	/*
    	I think this is a good idea, but the consensus seems to be
    	that if the faction is not in the DB, it should not be 
    	made up based on race and class like this is doing.
    	
        fmods.base = 0;
        fmods.deity_mod = 0;

        if (tnpc && p_class == (uint32) tnpc->GetClass()%16)
            fmods.class_mod = 301;
        else if (tnpc && tnpc->IsNPC() && tnpc->CastToNPC()->MerchantType == 0)
            fmods.class_mod = -101;
        else
            fmods.class_mod = 0;

        if (tnpc && IsOfEqualRace(p_race, tnpc->GetRace()) )
            fmods.race_mod = 101;
        else if (tnpc && IsOfIndiffRace(p_race, tnpc->GetRace()) )
            fmods.race_mod = 0;
        else if (tnpc)
            fmods.race_mod = -51;
        else
            fmods.race_mod = 0;
        fac = CalculateFaction(&fmods, 0);
        */
    }

    // merchant fix
    if (tnpc && tnpc->IsNPC() && tnpc->CastToNPC()->MerchantType && (fac == FACTION_THREATENLY || fac == FACTION_SCOWLS))
        fac = FACTION_DUBIOUS;

	if (tnpc != 0 && fac != FACTION_SCOWLS && tnpc->CastToNPC()->CheckAggro(this))
		fac = FACTION_THREATENLY;

#if FACTIONS_DEBUG >= 5
	LogFile->write(EQEMuLog::Debug, "%s::GetFactionLevel() result: %d", GetName(), fac);
#endif
	return fac;
}

//o--------------------------------------------------------------
//| Name: SetFactionLevel; rembrant, Dec. 20, 2001
//o--------------------------------------------------------------
//| Notes: Sets the characters faction standing with the
//|        specified NPC.
//o--------------------------------------------------------------
void  Client::SetFactionLevel(uint32 char_id, uint32 npc_id, uint8 char_class, uint8 char_race, uint8 char_deity)
{
	_ZP(Client_SetFactionLevel);
	int32 faction_id[MAX_NPC_FACTIONS]={ 0,0,0,0,0,0,0,0,0,0 };
	int32 npc_value[MAX_NPC_FACTIONS]={ 0,0,0,0,0,0,0,0,0,0 };
	uint8 temp[MAX_NPC_FACTIONS]={ 0,0,0,0,0,0,0,0,0,0 };
	int32 mod;
	int32 t;
	int32 tmpValue;
	int32 current_value;
	FactionMods fm;
	// Get the npc faction list
	if(!database.GetNPCFactionList(npc_id, faction_id, npc_value, temp))
		return;
	for(int i = 0;i<MAX_NPC_FACTIONS;i++)
	{
		if(faction_id[i] <= 0)
			continue;
		
		// Get the faction modifiers
		if(database.GetFactionData(&fm,char_class,char_race,char_deity,faction_id[i]))
		{
			// Get the characters current value with that faction
			current_value = GetCharacterFactionLevel(faction_id[i]);
			
			if(this->itembonuses.HeroicCHA) {
				int faction_mod = itembonuses.HeroicCHA / 5;
				// If our result isn't truncated, then just do that
				if(npc_value[i] * faction_mod / 100 != 0) 
					npc_value[i] += npc_value[i] * faction_mod / 100;
				// If our result is truncated, then double a mob's value every once and a while to equal what they would have got
				else {
					if(MakeRandomInt(0, 100) < faction_mod)
						npc_value[i] *= 2;
				}
			}
			//figure out their modifier
			mod = fm.base + fm.class_mod + fm.race_mod + fm.deity_mod;
			if(mod > MAX_FACTION)
				mod = MAX_FACTION;
			else if(mod < MIN_FACTION)
				mod = MIN_FACTION;
			
			// Calculate the faction
            if(npc_value[i] != 0) {
			    tmpValue = current_value + mod + npc_value[i];
			    
			    // Make sure faction hits don't go to GMs...
			    if (m_pp.gm==1 && (tmpValue < current_value)) {
			    	tmpValue = current_value;
			    }
			    
			    // Make sure we dont go over the min/max faction limits
			    if(tmpValue >= MAX_FACTION)
			    {
			    	t = MAX_FACTION - mod;
			    	if(current_value == t) {
			    		//do nothing, it is already maxed out
			    	} else if(!(database.SetCharacterFactionLevel(char_id, faction_id[i], t, temp[i], factionvalues)))
			    	{
			    		return;
			    	}
			    }
			    else if(tmpValue <= MIN_FACTION)
			    {
			    	t = MIN_FACTION - mod;
			    	if(current_value == t) {
			    		//do nothing, it is already maxed out
			    	} else if(!(database.SetCharacterFactionLevel(char_id, faction_id[i], t, temp[i], factionvalues)))
			    	{
			    		return;
			    	}
			    }
			    else
			    {
			    	if(!(database.SetCharacterFactionLevel(char_id, faction_id[i], current_value + npc_value[i], temp[i], factionvalues)))
			    	{
			    		return;
			    	}
			    }
			    if(tmpValue <= MIN_FACTION)
			    	tmpValue = MIN_FACTION;

			    char* msg = BuildFactionMessage(npc_value[i],faction_id[i],tmpValue,temp[i]);
			    if (msg != 0)
			    	Message(0, msg);
			    safe_delete_array(msg);
            }
        }
	}
	return;
}

void  Client::SetFactionLevel2(uint32 char_id, int32 faction_id, uint8 char_class, uint8 char_race, uint8 char_deity, int32 value, uint8 temp)
{
	_ZP(Client_SetFactionLevel2);
//	int32 tmpValue;
	int32 current_value;
//	FactionMods fm;
	//Get the npc faction list
	if(faction_id > 0 && value != 0) {
		//Get the faction modifiers
		current_value = GetCharacterFactionLevel(faction_id) + value;
		if(!(database.SetCharacterFactionLevel(char_id, faction_id, current_value, temp, factionvalues)))
			return;

		char* msg = BuildFactionMessage(value, faction_id, current_value, temp);
		if (msg != 0)
			Message(0, msg);
		safe_delete(msg);

	}
	return;
}

int32 Client::GetCharacterFactionLevel(int32 faction_id)
{
	if (faction_id <= 0)
		return 0;
	faction_map::iterator res;
	res = factionvalues.find(faction_id);
	if(res == factionvalues.end())
		return(0);
	return(res->second);
}

// returns the character's faction level, adjusted for racial, class, and deity modifiers
int32 Client::GetModCharacterFactionLevel(int32 faction_id) {
	int32 Modded = GetCharacterFactionLevel(faction_id);
	FactionMods fm;
	if(database.GetFactionData(&fm,GetClass(),GetRace(),GetDeity(),faction_id)) 
		Modded += fm.base + fm.class_mod + fm.race_mod + fm.deity_mod;
	if (Modded > MAX_FACTION)
		Modded = MAX_FACTION;

	return Modded;
}

bool ZoneDatabase::GetFactionData(FactionMods* fm, uint32 class_mod, uint32 race_mod, uint32 deity_mod, int32 faction_id) {
	if (faction_id <= 0 || faction_id > (int32) max_faction)
		return false;	

    if (faction_array[faction_id] == 0){
		return false;
	}

    fm->base = faction_array[faction_id]->base;

    if(class_mod > 0) {
        char str[32];
        sprintf(str, "c%u", class_mod);

        std::map<std::string, int16>::const_iterator iter = faction_array[faction_id]->mods.find(str);
        if(iter != faction_array[faction_id]->mods.end()) {
            fm->class_mod = iter->second;
        } else {
            fm->class_mod = 0;
        }
    } else {
        fm->class_mod = 0;
    }

    if(race_mod > 0) {
        char str[32];
        sprintf(str, "r%u", race_mod);

        std::map<std::string, int16>::iterator iter = faction_array[faction_id]->mods.find(str);
        if(iter != faction_array[faction_id]->mods.end()) {
            fm->race_mod = iter->second;
        } else {
            fm->race_mod = 0;
        }
    } else {
        fm->race_mod = 0;
    }

    if(deity_mod > 0) {
        char str[32];
        sprintf(str, "d%u", deity_mod);

        std::map<std::string, int16>::iterator iter = faction_array[faction_id]->mods.find(str);
        if(iter != faction_array[faction_id]->mods.end()) {
            fm->deity_mod = iter->second;
        } else {
            fm->deity_mod = 0;
        }
    } else {
        fm->deity_mod = 0;
    }

	return true;
}


bool ZoneDatabase::LoadFactionValues(uint32 char_id, faction_map & val_list) {
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
	if (RunQuery(query, MakeAnyLenString(&query, "SELECT faction_id,current_value FROM faction_values WHERE char_id = %i",char_id), errbuf, &result)) {
		safe_delete_array(query);
		bool ret = LoadFactionValues_result(result, val_list);
		mysql_free_result(result);
		return ret;
	}
	else {
		cerr << "Error in LoadFactionValues query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
	}
	return false;
}

bool ZoneDatabase::LoadFactionValues_result(MYSQL_RES* result, faction_map & val_list) {
    MYSQL_ROW row;
	while((row = mysql_fetch_row(result))) {
		val_list[atoi(row[0])] = atoi(row[1]);
	}
	return true;
}

//o--------------------------------------------------------------
//| Name: BuildFactionMessage; rembrant, Dec. 16, 2001
//o--------------------------------------------------------------
//| Purpose: duh?
//o--------------------------------------------------------------
char* BuildFactionMessage(int32 tmpvalue, int32 faction_id, int32 totalvalue, uint8 temp)
{
/*

This should be replaced to send string-ID based messages using:
#define FACTION_WORST 469 //Your faction standing with %1 could not possibly get any worse.
#define FACTION_WORSE 470 //Your faction standing with %1 got worse.
#define FACTION_BEST 471 //Your faction standing with %1 could not possibly get any better.
#define FACTION_BETTER 472 //Your faction standing with %1 got better.

some day.

*/
	//tmpvalue is the change as best I can tell.
	char *faction_message = 0;

	char name[50];

	if(database.GetFactionName(faction_id, name, sizeof(name)) == false) {
		snprintf(name, sizeof(name),"Faction%i",faction_id);
	}

	if(tmpvalue == 0 || temp == 1 || temp == 2) {
		return 0;
	}
	else if (totalvalue >= MAX_FACTION) {
		MakeAnyLenString(&faction_message, "Your faction standing with %s could not possibly get any better!", name);
		return faction_message;
	}
	else if(tmpvalue > 0 && totalvalue < MAX_FACTION) {
		MakeAnyLenString(&faction_message, "Your faction standing with %s has gotten better!", name);
		return faction_message;
	}
	else if(tmpvalue < 0 && totalvalue > MIN_FACTION) {
		MakeAnyLenString(&faction_message, "Your faction standing with %s has gotten worse!", name);
		return faction_message;
	}
	else if(totalvalue <= MIN_FACTION) {
		MakeAnyLenString(&faction_message, "Your faction standing with %s could not possibly get any worse!", name);
		return faction_message;
	}
	return 0;
}

//o--------------------------------------------------------------
//| Name: GetFactionName; rembrant, Dec. 16
//o--------------------------------------------------------------
//| Notes: Retrieves the name of the specified faction
//|        Returns false on failure.
//o--------------------------------------------------------------
bool ZoneDatabase::GetFactionName(int32 faction_id, char* name, uint32 buflen) {
	if ((faction_id <= 0) || faction_id > int32(max_faction) ||(faction_array[faction_id] == 0))
		return false;
	if (faction_array[faction_id]->name[0] != 0) {
		strn0cpy(name, faction_array[faction_id]->name, buflen);
		return true;
	}
	return false;

}

//o--------------------------------------------------------------
//| Name: GetNPCFactionList; rembrant, Dec. 16, 2001
//o--------------------------------------------------------------
//| Purpose: Gets a list of faction_id's and values bound to
//|          the npc_id.
//|          Returns false on failure.
//o--------------------------------------------------------------
bool ZoneDatabase::GetNPCFactionList(uint32 npcfaction_id, int32* faction_id, int32* value, uint8* temp, int32* primary_faction) {
	if (npcfaction_id <= 0) {
		if (primary_faction)
			*primary_faction = npcfaction_id;
		return true;
	}
	const NPCFactionList* nfl = GetNPCFactionEntry(npcfaction_id);
	if (!nfl)
		return false;
	if (primary_faction)
		*primary_faction = nfl->primaryfaction;
	for (int i=0; i<MAX_NPC_FACTIONS; i++) {
		faction_id[i] = nfl->factionid[i];
		value[i] = nfl->factionvalue[i];
		temp[i] = nfl->factiontemp[i];
	}
	return true;
}

//o--------------------------------------------------------------
//| Name: SetCharacterFactionLevel; rembrant, Dec. 20, 2001
//o--------------------------------------------------------------
//| Purpose: Update characters faction level with specified
//|          faction_id to specified value.
//|          Returns false on failure.
//o--------------------------------------------------------------
bool ZoneDatabase::SetCharacterFactionLevel(uint32 char_id, int32 faction_id, int32 value, uint8 temp, faction_map &val_list)
{
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
	uint32 affected_rows = 0;
	
	if (!RunQuery(query, MakeAnyLenString(&query, 
		"DELETE FROM faction_values WHERE char_id=%i AND faction_id = %i", 
		char_id, faction_id), errbuf)) {
		cerr << "Error in SetCharacterFactionLevel query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
		return false;
	}

	if(value == 0)
	{
		safe_delete_array(query);
		return true;
	}

	if(temp == 2)
		temp = 0;

	if(temp == 3)
		temp = 1;

	if (!RunQuery(query, MakeAnyLenString(&query, 
		"INSERT INTO faction_values (char_id,faction_id,current_value,temp) VALUES (%i,%i,%i,%i)", 
		char_id, faction_id,value,temp), errbuf, 0, &affected_rows)) {
		cerr << "Error in SetCharacterFactionLevel query '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
		return false;
	}
	
	safe_delete_array(query);
	
	if (affected_rows == 0)
	{
		return false;
	}
	
	val_list[faction_id] = value;
	return(true);
}

bool ZoneDatabase::LoadFactionData()
{
	char errbuf[MYSQL_ERRMSG_SIZE];
    char *query = 0;
    MYSQL_RES *result;
    MYSQL_ROW row;
	query = new char[256];
	strcpy(query, "SELECT MAX(id) FROM faction_list");
	
	
	if (RunQuery(query, strlen(query), errbuf, &result)) {
		safe_delete_array(query);
		row = mysql_fetch_row(result);
		if (row && row[0])
		{ 
			max_faction = atoi(row[0]);
			faction_array = new Faction*[max_faction+1];
			for(unsigned int i=0; i<max_faction; i++)
			{
				faction_array[i] = NULL;
			}
			mysql_free_result(result);
			
			MakeAnyLenString(&query, "SELECT id,name,base FROM faction_list");
			if (RunQuery(query, strlen(query), errbuf, &result))
			{
				safe_delete_array(query);
				while((row = mysql_fetch_row(result)))
				{
					uint32 index = atoi(row[0]);
					faction_array[index] = new Faction;
					strn0cpy(faction_array[index]->name, row[1], 50);					
					faction_array[index]->base = atoi(row[2]);

                    char sec_errbuf[MYSQL_ERRMSG_SIZE];
                    MYSQL_RES *sec_result;
                    MYSQL_ROW sec_row;
                    MakeAnyLenString(&query, "SELECT `mod`, `mod_name` FROM `faction_list_mod` WHERE faction_id=%u", index);
                    if (RunQuery(query, strlen(query), sec_errbuf, &sec_result)) {
                        while((sec_row = mysql_fetch_row(sec_result)))
                        {
                            faction_array[index]->mods[sec_row[1]] = atoi(sec_row[0]);
                        }
                        mysql_free_result(sec_result);
                    }
                    safe_delete_array(query);
				}
				mysql_free_result(result);
			}
			else {
				cerr << "Error in LoadFactionData '" << query << "' " << errbuf << endl;
				safe_delete_array(query);
				return false;
			}
		}
		else {
			mysql_free_result(result);
		}
	}
	else {
		cerr << "Error in LoadFactionData '" << query << "' " << errbuf << endl;
		safe_delete_array(query);
		return false;
	}
	return true;
}

bool ZoneDatabase::GetFactionIdsForNPC(uint32 nfl_id, list<struct NPCFaction*> *faction_list, int32* primary_faction) {
	if (nfl_id <= 0) {
		list<struct NPCFaction*>::iterator cur,end;
		cur = faction_list->begin();
		end = faction_list->end();
		for(; cur != end; cur++) {
			struct NPCFaction* tmp = *cur;
			safe_delete(tmp);
		}
		
		faction_list->clear();
		if (primary_faction)
			*primary_faction = nfl_id;
		return true;
	}
	const NPCFactionList* nfl = GetNPCFactionEntry(nfl_id);
	if (!nfl)
		return false;
	if (primary_faction)
		*primary_faction = nfl->primaryfaction;
	
	list<struct NPCFaction*>::iterator cur,end;
	cur = faction_list->begin();
	end = faction_list->end();
	for(; cur != end; cur++) {
		struct NPCFaction* tmp = *cur;
		safe_delete(tmp);
	}
	faction_list->clear();
	for (int i=0; i<MAX_NPC_FACTIONS; i++) {
		struct NPCFaction *pFac;
		if (nfl->factionid[i]) {
			pFac = new struct NPCFaction;
			pFac->factionID = nfl->factionid[i];
			pFac->value_mod = nfl->factionvalue[i];
			pFac->npc_value = nfl->factionnpcvalue[i];
			pFac->temp = nfl->factiontemp[i];
/*			if (nfl->primaryfaction == pFac->factionID)
				pFac->primary = true;
			else
				pFac->primary = false;
*/
			faction_list->push_back(pFac);
		}
	}
	return true;
}

bool Client::HatedByClass(uint32 p_race, uint32 p_class, uint32 p_deity, int32 pFaction)
{
	
	bool Result = false;
	_ZP(Client_GetFactionLevel);

	int32 tmpFactionValue;
	FactionMods fmods;

    //First get the NPC's Primary faction
	if(pFaction > 0)
	{
		//Get the faction data from the database
		if(database.GetFactionData(&fmods, p_class, p_race, p_deity, pFaction))
		{
			tmpFactionValue = GetCharacterFactionLevel(pFaction);
			tmpFactionValue += GetFactionBonus(pFaction);
			tmpFactionValue += GetItemFactionBonus(pFaction);
			CalculateFaction(&fmods, tmpFactionValue);
			if(fmods.class_mod < fmods.race_mod)
				Result = true;
		}
	}
	return Result;
}
