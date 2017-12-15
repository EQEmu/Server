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

#include "../common/global_define.h"
#include "../common/features.h"
#include "../common/rulesys.h"
#include "../common/string_util.h"

#include "client.h"
#include "groups.h"
#include "mob.h"
#include "raids.h"

#include "queryserv.h"
#include "quest_parser_collection.h"
#include "lua_parser.h"
#include "string_ids.h"

#ifdef BOTS
#include "bot.h"
#endif

extern QueryServ* QServ;


static uint32 MaxBankedGroupLeadershipPoints(int Level)
{
	if(Level < 35)
		return 4;

	if(Level < 51)
		return 6;

	return 8;
}

static uint32 MaxBankedRaidLeadershipPoints(int Level)
{
	if(Level < 45)
		return 6;

	if(Level < 55)
		return 8;

	return 10;
}

uint32 Client::CalcEXP(uint8 conlevel) {

	uint32 in_add_exp = EXP_FORMULA;


	if((XPRate != 0))
		in_add_exp = static_cast<uint32>(in_add_exp * (static_cast<float>(XPRate) / 100.0f));

	float totalmod = 1.0;
	float zemmod = 1.0;
	//get modifiers
	if(RuleR(Character, ExpMultiplier) >= 0){
		totalmod *= RuleR(Character, ExpMultiplier);
	}

	if(zone->newzone_data.zone_exp_multiplier >= 0){
		zemmod *= zone->newzone_data.zone_exp_multiplier;
	}

	if(RuleB(Character,UseRaceClassExpBonuses))
	{
		if(GetBaseRace() == HALFLING){
			totalmod *= 1.05;
		}

		if(GetClass() == ROGUE || GetClass() == WARRIOR){
			totalmod *= 1.05;
		}
	}

	if(zone->IsHotzone())
	{
		totalmod += RuleR(Zone, HotZoneBonus);
	}

	in_add_exp = uint32(float(in_add_exp) * totalmod * zemmod);

	if(RuleB(Character,UseXPConScaling))
	{
		if (conlevel != 0xFF) {
			switch (conlevel)
			{
			case CON_GRAY:
				in_add_exp = 0;
				return 0;
			case CON_GREEN:
				in_add_exp = in_add_exp * RuleI(Character, GreenModifier) / 100;
				break;
			case CON_LIGHTBLUE:
				in_add_exp = in_add_exp * RuleI(Character, LightBlueModifier)/100;
				break;
			case CON_BLUE:
				in_add_exp = in_add_exp * RuleI(Character, BlueModifier)/100;
				break;
			case CON_WHITE:
				in_add_exp = in_add_exp * RuleI(Character, WhiteModifier)/100;
				break;
			case CON_YELLOW:
				in_add_exp = in_add_exp * RuleI(Character, YellowModifier)/100;
				break;
			case CON_RED:
				in_add_exp = in_add_exp * RuleI(Character, RedModifier)/100;
				break;
			}
		}
	}

	float aatotalmod = 1.0;
	if(zone->newzone_data.zone_exp_multiplier >= 0){
		aatotalmod *= zone->newzone_data.zone_exp_multiplier;
	}



	if(RuleB(Character,UseRaceClassExpBonuses))
	{
		if(GetBaseRace() == HALFLING){
			aatotalmod *= 1.05;
		}

		if(GetClass() == ROGUE || GetClass() == WARRIOR){
			aatotalmod *= 1.05;
		}
	}

	if(RuleB(Zone, LevelBasedEXPMods)){
		if(zone->level_exp_mod[GetLevel()].ExpMod){
			in_add_exp *= zone->level_exp_mod[GetLevel()].ExpMod;
		}
	}

	return in_add_exp;
}

uint32 Client::GetExperienceForKill(Mob *against)
{
#ifdef LUA_EQEMU
	uint32 lua_ret = 0;
	bool ignoreDefault = false;
	lua_ret = LuaParser::Instance()->GetExperienceForKill(this, against, ignoreDefault);

	if (ignoreDefault) {
		return lua_ret;
	}
#endif

	if (against && against->IsNPC()) {
		uint32 level = (uint32)against->GetLevel();
		return EXP_FORMULA;
	}

	return 0;
}

void Client::AddEXP(uint32 in_add_exp, uint8 conlevel, bool resexp) {

	this->EVENT_ITEM_ScriptStopReturn();

	uint32 add_exp = in_add_exp;

	if(!resexp && (XPRate != 0))
		add_exp = static_cast<uint32>(in_add_exp * (static_cast<float>(XPRate) / 100.0f));

	if (m_epp.perAA<0 || m_epp.perAA>100)
		m_epp.perAA=0;	// stop exploit with sanity check

	uint32 add_aaxp;
	if(resexp) {
		add_aaxp = 0;
	} else {

		//figure out how much of this goes to AAs
		add_aaxp = add_exp * m_epp.perAA / 100;
		//take that amount away from regular exp
		add_exp -= add_aaxp;

		float totalmod = 1.0;
		float zemmod = 1.0;
		//get modifiers
		if(RuleR(Character, ExpMultiplier) >= 0){
			totalmod *= RuleR(Character, ExpMultiplier);
		}

		if(zone->newzone_data.zone_exp_multiplier >= 0){
			zemmod *= zone->newzone_data.zone_exp_multiplier;
		}

		if(RuleB(Character,UseRaceClassExpBonuses))
		{
			if(GetBaseRace() == HALFLING){
				totalmod *= 1.05;
			}

			if(GetClass() == ROGUE || GetClass() == WARRIOR){
				totalmod *= 1.05;
			}
		}

		if(zone->IsHotzone())
		{
			totalmod += RuleR(Zone, HotZoneBonus);
		}

		add_exp = uint32(float(add_exp) * totalmod * zemmod);

		if(RuleB(Character,UseXPConScaling))
		{
			if (conlevel != 0xFF && !resexp) {
				switch (conlevel)
				{
					case CON_GRAY:
						add_exp = 0;
						add_aaxp = 0;
						return;
					case CON_GREEN:
						add_exp = add_exp * RuleI(Character, GreenModifier) / 100;
						add_aaxp = add_aaxp * RuleI(Character, GreenModifier) / 100;
						break;
					case CON_LIGHTBLUE:
							add_exp = add_exp * RuleI(Character, LightBlueModifier)/100;
							add_aaxp = add_aaxp * RuleI(Character, LightBlueModifier)/100;
						break;
					case CON_BLUE:
							add_exp = add_exp * RuleI(Character, BlueModifier)/100;
							add_aaxp = add_aaxp * RuleI(Character, BlueModifier)/100;
						break;
					case CON_WHITE:
							add_exp = add_exp * RuleI(Character, WhiteModifier)/100;
							add_aaxp = add_aaxp * RuleI(Character, WhiteModifier)/100;
						break;
					case CON_YELLOW:
							add_exp = add_exp * RuleI(Character, YellowModifier)/100;
							add_aaxp = add_aaxp * RuleI(Character, YellowModifier)/100;
						break;
					case CON_RED:
							add_exp = add_exp * RuleI(Character, RedModifier)/100;
							add_aaxp = add_aaxp * RuleI(Character, RedModifier)/100;
						break;
				}
			}
		}

		if (IsLeadershipEXPOn() && (conlevel == CON_BLUE || conlevel == CON_WHITE || conlevel == CON_YELLOW || conlevel == CON_RED)) {
			add_exp = static_cast<uint32>(static_cast<float>(add_exp) * 0.8f);

			if (GetGroup()) {
				if (m_pp.group_leadership_points < MaxBankedGroupLeadershipPoints(GetLevel())
						&& RuleI(Character, KillsPerGroupLeadershipAA) > 0) {
					uint32 exp = GROUP_EXP_PER_POINT / RuleI(Character, KillsPerGroupLeadershipAA);
					Client *mentoree = GetGroup()->GetMentoree();
					if (GetGroup()->GetMentorPercent() && mentoree &&
							mentoree->GetGroupPoints() < MaxBankedGroupLeadershipPoints(mentoree->GetLevel())) {
						uint32 mentor_exp = exp * (GetGroup()->GetMentorPercent() / 100.0f);
						exp -= mentor_exp;
						mentoree->AddLeadershipEXP(mentor_exp, 0); // ends up rounded down
						mentoree->Message_StringID(MT_Leadership, GAIN_GROUP_LEADERSHIP_EXP);
					}
					if (exp > 0) { // possible if you mentor 100% to the other client
						AddLeadershipEXP(exp, 0); // ends up rounded up if mentored, no idea how live actually does it
						Message_StringID(MT_Leadership, GAIN_GROUP_LEADERSHIP_EXP);
					}
				} else {
					Message_StringID(MT_Leadership, MAX_GROUP_LEADERSHIP_POINTS);
				}
			} else {
				Raid *raid = GetRaid();
				// Raid leaders CAN NOT gain group AA XP, other group leaders can though!
				if (raid->IsLeader(this)) {
					if (m_pp.raid_leadership_points < MaxBankedRaidLeadershipPoints(GetLevel())
							&& RuleI(Character, KillsPerRaidLeadershipAA) > 0) {
						AddLeadershipEXP(0, RAID_EXP_PER_POINT / RuleI(Character, KillsPerRaidLeadershipAA));
						Message_StringID(MT_Leadership, GAIN_RAID_LEADERSHIP_EXP);
					} else {
						Message_StringID(MT_Leadership, MAX_RAID_LEADERSHIP_POINTS);
					}
				} else {
					if (m_pp.group_leadership_points < MaxBankedGroupLeadershipPoints(GetLevel())
							&& RuleI(Character, KillsPerGroupLeadershipAA) > 0) {
						uint32 group_id = raid->GetGroup(this);
						uint32 exp = GROUP_EXP_PER_POINT / RuleI(Character, KillsPerGroupLeadershipAA);
						Client *mentoree = raid->GetMentoree(group_id);
						if (raid->GetMentorPercent(group_id) && mentoree &&
								mentoree->GetGroupPoints() < MaxBankedGroupLeadershipPoints(mentoree->GetLevel())) {
							uint32 mentor_exp = exp * (raid->GetMentorPercent(group_id) / 100.0f);
							exp -= mentor_exp;
							mentoree->AddLeadershipEXP(mentor_exp, 0);
							mentoree->Message_StringID(MT_Leadership, GAIN_GROUP_LEADERSHIP_EXP);
						}
						if (exp > 0) {
							AddLeadershipEXP(exp, 0);
							Message_StringID(MT_Leadership, GAIN_GROUP_LEADERSHIP_EXP);
						}
					} else {
						Message_StringID(MT_Leadership, MAX_GROUP_LEADERSHIP_POINTS);
					}
				}
			}

		}

	}	//end !resexp

	float aatotalmod = 1.0;
	if(zone->newzone_data.zone_exp_multiplier >= 0){
		aatotalmod *= zone->newzone_data.zone_exp_multiplier;
	}

	// Shouldn't race not affect AA XP?
	if(RuleB(Character,UseRaceClassExpBonuses))
	{
		if(GetBaseRace() == HALFLING){
			aatotalmod *= 1.05;
		}

		if(GetClass() == ROGUE || GetClass() == WARRIOR){
			aatotalmod *= 1.05;
		}
	}

	// why wasn't this here? Where should it be?
	if(zone->IsHotzone())
	{
		aatotalmod += RuleR(Zone, HotZoneBonus);
	}

	if(RuleB(Zone, LevelBasedEXPMods)){
		if(zone->level_exp_mod[GetLevel()].ExpMod){
			add_exp *= zone->level_exp_mod[GetLevel()].ExpMod;
			add_aaxp *= zone->level_exp_mod[GetLevel()].AAExpMod;
		}
	}

	uint32 exp = GetEXP() + add_exp;

	uint32 aaexp = (uint32)(RuleR(Character, AAExpMultiplier) * add_aaxp * aatotalmod);
	uint32 had_aaexp = GetAAXP();
	aaexp += had_aaexp;
	if(aaexp < had_aaexp)
		aaexp = had_aaexp;	//watch for wrap

	SetEXP(exp, aaexp, resexp);
}

void Client::SetEXP(uint32 set_exp, uint32 set_aaxp, bool isrezzexp) {
	Log(Logs::Detail, Logs::None, "Attempting to Set Exp for %s (XP: %u, AAXP: %u, Rez: %s)", this->GetCleanName(), set_exp, set_aaxp, isrezzexp ? "true" : "false");

	auto max_AAXP = GetRequiredAAExperience();
	if (max_AAXP == 0 || GetEXPForLevel(GetLevel()) == 0xFFFFFFFF) {
		Message(13, "Error in Client::SetEXP. EXP not set.");
		return; // Must be invalid class/race
	}
	uint32 i = 0;
	uint32 membercount = 0;
	if(GetGroup())
	{
		for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
			if (GetGroup()->members[i] != nullptr) {
				membercount++;
			}
		}
	}

	if ((set_exp + set_aaxp) > (m_pp.exp+m_pp.expAA)) {

		uint32 exp_gained = set_exp - m_pp.exp;
		uint32 aaxp_gained = set_aaxp - m_pp.expAA;
		float exp_percent = (float)((float)exp_gained / (float)(GetEXPForLevel(GetLevel() + 1) - GetEXPForLevel(GetLevel())))*(float)100; //EXP needed for level
		float aaxp_percent = (float)((float)aaxp_gained / (float)(RuleI(AA, ExpPerPoint)))*(float)100; //AAEXP needed for level
		std::string exp_amount_message = "";
		if (RuleI(Character, ShowExpValues) >= 1) {
			if (exp_gained > 0 && aaxp_gained > 0) exp_amount_message = StringFormat("%u, %u AA", exp_gained, aaxp_gained);
			else if (exp_gained > 0) exp_amount_message = StringFormat("%u", exp_gained);
			else exp_amount_message = StringFormat("%u AA", aaxp_gained);
		}

		std::string exp_percent_message = "";
		if (RuleI(Character, ShowExpValues) >= 2) {
			if (exp_gained > 0 && aaxp_gained > 0) exp_percent_message = StringFormat("(%.3f%%, %.3f%%AA)", exp_percent, aaxp_percent);
			else if (exp_gained > 0) exp_percent_message = StringFormat("(%.3f%%)", exp_percent);
			else exp_percent_message = StringFormat("(%.3f%%AA)", aaxp_percent);
		}

		if (isrezzexp) {
			if (RuleI(Character, ShowExpValues) > 0) 
				Message(MT_Experience, "You regain %s experience from resurrection. %s", exp_amount_message.c_str(), exp_percent_message.c_str());
			else Message_StringID(MT_Experience, REZ_REGAIN);
		} else {
			if (membercount > 1) {
				if (RuleI(Character, ShowExpValues) > 0) 
					Message(MT_Experience, "You have gained %s party experience! %s", exp_amount_message.c_str(), exp_percent_message.c_str());
				else Message_StringID(MT_Experience, GAIN_GROUPXP);
			}
			else if (IsRaidGrouped()) {
				if (RuleI(Character, ShowExpValues) > 0) 
					Message(MT_Experience, "You have gained %s raid experience! %s", exp_amount_message.c_str(), exp_percent_message.c_str());
				else Message_StringID(MT_Experience, GAIN_RAIDEXP);
			} 
			else {
				if (RuleI(Character, ShowExpValues) > 0) 
					Message(MT_Experience, "You have gained %s experience! %s", exp_amount_message.c_str(), exp_percent_message.c_str());
				else Message_StringID(MT_Experience, GAIN_XP);				
			}
		}
	}
	else if((set_exp + set_aaxp) < (m_pp.exp+m_pp.expAA)){ //only loss message if you lose exp, no message if you gained/lost nothing.
		uint32 exp_lost = m_pp.exp - set_exp;
		float exp_percent = (float)((float)exp_lost / (float)(GetEXPForLevel(GetLevel() + 1) - GetEXPForLevel(GetLevel())))*(float)100;

		if (RuleI(Character, ShowExpValues) == 1 && exp_lost > 0) Message(15, "You have lost %i experience.", exp_lost);
		else if (RuleI(Character, ShowExpValues) == 2 && exp_lost > 0) Message(15, "You have lost %i experience. (%.3f%%)", exp_lost, exp_percent);
		else Message(15, "You have lost experience.");		
	}

	//check_level represents the level we should be when we have
	//this ammount of exp (once these loops complete)
	uint16 check_level = GetLevel()+1;
	//see if we gained any levels
	bool level_increase = true;
	int8 level_count = 0;

	while (set_exp >= GetEXPForLevel(check_level)) {
		check_level++;
		if (check_level > 127) {	//hard level cap
			check_level = 127;
			break;
		}
		level_count++;

		if(GetMercID())
			UpdateMercLevel();
	}
	//see if we lost any levels
	while (set_exp < GetEXPForLevel(check_level-1)) {
		check_level--;
		if (check_level < 2) {	//hard level minimum
			check_level = 2;
			break;
		}
		level_increase = false;
		if(GetMercID())
			UpdateMercLevel();
	}
	check_level--;


	//see if we gained any AAs
	if (set_aaxp >= max_AAXP) {
		/*
			Note: AA exp is stored differently than normal exp.
			Exp points are only stored in m_pp.expAA until you
			gain a full AA point, once you gain it, a point is
			added to m_pp.aapoints and the ammount needed to gain
			that point is subtracted from m_pp.expAA

			then, once they spend an AA point, it is subtracted from
			m_pp.aapoints. In theory it then goes into m_pp.aapoints_spent,
			but im not sure if we have that in the right spot.
		*/
		//record how many points we have
		uint32 last_unspentAA = m_pp.aapoints;

		//figure out how many AA points we get from the exp were setting
		m_pp.aapoints = set_aaxp / max_AAXP;
		Log(Logs::Detail, Logs::None, "Calculating additional AA Points from AAXP for %s: %u / %u = %.1f points", this->GetCleanName(), set_aaxp, max_AAXP, (float)set_aaxp / (float)max_AAXP);

		//get remainder exp points, set in PP below
		set_aaxp = set_aaxp - (max_AAXP * m_pp.aapoints);

		//add in how many points we had
		m_pp.aapoints += last_unspentAA;

		//figure out how many points were actually gained
		/*uint32 gained = m_pp.aapoints - last_unspentAA;*/	//unused

		//Message(15, "You have gained %d skill points!!", m_pp.aapoints - last_unspentAA);
		char val1[20]={0};
		Message_StringID(MT_Experience, GAIN_ABILITY_POINT, ConvertArray(m_pp.aapoints, val1),m_pp.aapoints == 1 ? "" : "(s)");	//You have gained an ability point! You now have %1 ability point%2.
		
		/* QS: PlayerLogAARate */
		if (RuleB(QueryServ, PlayerLogAARate)){
			int add_points = (m_pp.aapoints - last_unspentAA);
			std::string query = StringFormat("INSERT INTO `qs_player_aa_rate_hourly` (char_id, aa_count, hour_time) VALUES (%i, %i, UNIX_TIMESTAMP() - MOD(UNIX_TIMESTAMP(), 3600)) ON DUPLICATE KEY UPDATE `aa_count` = `aa_count` + %i", this->CharacterID(), add_points, add_points);
			QServ->SendQuery(query.c_str());
		}

		//Message(15, "You now have %d skill points available to spend.", m_pp.aapoints);
	}

	uint8 maxlevel = RuleI(Character, MaxExpLevel) + 1;

	if(maxlevel <= 1)
		maxlevel = RuleI(Character, MaxLevel) + 1;

	if(check_level > maxlevel) {
		check_level = maxlevel;

		if(RuleB(Character, KeepLevelOverMax)) {
			set_exp = GetEXPForLevel(GetLevel()+1);
		}
		else {
			set_exp = GetEXPForLevel(maxlevel);
		}
	}

	if(RuleB(Character, PerCharacterQglobalMaxLevel)){
		uint32 MaxLevel = GetCharMaxLevelFromQGlobal();
		if(MaxLevel){
			if(GetLevel() >= MaxLevel){
				uint32 expneeded = GetEXPForLevel(MaxLevel);
				if(set_exp > expneeded) {
					set_exp = expneeded;
				}
			}
		}
	}

	if ((GetLevel() != check_level) && !(check_level >= maxlevel)) {
		char val1[20]={0};
		if (level_increase)
		{
			if (level_count == 1)
				Message_StringID(MT_Experience, GAIN_LEVEL, ConvertArray(check_level, val1));
			else
				Message(15, "Welcome to level %i!", check_level);

			if (check_level == RuleI(Character, DeathItemLossLevel))
				Message_StringID(15, CORPSE_ITEM_LOST);

			if (check_level == RuleI(Character, DeathExpLossLevel))
				Message_StringID(15, CORPSE_EXP_LOST);
		}
		else
			Message_StringID(MT_Experience, LOSE_LEVEL, ConvertArray(check_level, val1));

#ifdef BOTS
		uint8 myoldlevel = GetLevel();
#endif

		SetLevel(check_level);

#ifdef BOTS
		if(RuleB(Bots, BotLevelsWithOwner))
			// hack way of doing this..but, least invasive... (same criteria as gain level for sendlvlapp)
			Bot::LevelBotWithClient(this, GetLevel(), (myoldlevel==check_level-1));
#endif
	}

	//If were at max level then stop gaining experience if we make it to the cap
	if(GetLevel() == maxlevel - 1){
		uint32 expneeded = GetEXPForLevel(maxlevel);
		if(set_exp > expneeded) {
			set_exp = expneeded;
		}
	}

	//set the client's EXP and AAEXP
	m_pp.exp = set_exp;
	m_pp.expAA = set_aaxp;

	if (GetLevel() < 51) {
		m_epp.perAA = 0;	// turn off aa exp if they drop below 51
	} else
		SendAlternateAdvancementStats();	//otherwise, send them an AA update

	//send the expdata in any case so the xp bar isnt stuck after leveling
	uint32 tmpxp1 = GetEXPForLevel(GetLevel()+1);
	uint32 tmpxp2 = GetEXPForLevel(GetLevel());
	// Quag: crash bug fix... Divide by zero when tmpxp1 and 2 equalled each other, most likely the error case from GetEXPForLevel() (invalid class, etc)
	if (tmpxp1 != tmpxp2 && tmpxp1 != 0xFFFFFFFF && tmpxp2 != 0xFFFFFFFF) {
		auto outapp = new EQApplicationPacket(OP_ExpUpdate, sizeof(ExpUpdate_Struct));
		ExpUpdate_Struct* eu = (ExpUpdate_Struct*)outapp->pBuffer;
		float tmpxp = (float) ( (float) set_exp-tmpxp2 ) / ( (float) tmpxp1-tmpxp2 );
		eu->exp = (uint32)(330.0f * tmpxp);
		FastQueuePacket(&outapp);
	}

	if (admin>=100 && GetGM()) {
		char val1[20]={0};
		char val2[20]={0};
		char val3[20]={0};
		Message_StringID(MT_Experience, GM_GAINXP, ConvertArray(set_aaxp,val1),ConvertArray(set_exp,val2),ConvertArray(GetEXPForLevel(GetLevel()+1),val3));	//[GM] You have gained %1 AXP and %2 EXP (%3).
	}
}

void Client::SetLevel(uint8 set_level, bool command)
{
	if (GetEXPForLevel(set_level) == 0xFFFFFFFF) {
		Log(Logs::General, Logs::Error, "Client::SetLevel() GetEXPForLevel(%i) = 0xFFFFFFFF", set_level);
		return;
	}

	auto outapp = new EQApplicationPacket(OP_LevelUpdate, sizeof(LevelUpdate_Struct));
	LevelUpdate_Struct* lu = (LevelUpdate_Struct*)outapp->pBuffer;
	lu->level = set_level;
	if(m_pp.level2 != 0)
		lu->level_old = m_pp.level2;
	else
		lu->level_old = level;

	level = set_level;

	if(IsRaidGrouped()) {
		Raid *r = this->GetRaid();
		if(r){
			r->UpdateLevel(GetName(), set_level);
		}
	}
	if(set_level > m_pp.level2) {
		if(m_pp.level2 == 0)
			m_pp.points += 5;
		else
			m_pp.points += (5 * (set_level - m_pp.level2));

		m_pp.level2 = set_level;
	}
	if(set_level > m_pp.level) {
		parse->EventPlayer(EVENT_LEVEL_UP, this, "", 0);
		/* QS: PlayerLogLevels */
		if (RuleB(QueryServ, PlayerLogLevels)){
			std::string event_desc = StringFormat("Leveled UP :: to Level:%i from Level:%i in zoneid:%i instid:%i", set_level, m_pp.level, this->GetZoneID(), this->GetInstanceID());
			QServ->PlayerLogEvent(Player_Log_Levels, this->CharacterID(), event_desc); 
		}
	}
	else if (set_level < m_pp.level){
		/* QS: PlayerLogLevels */
		if (RuleB(QueryServ, PlayerLogLevels)){
			std::string event_desc = StringFormat("Leveled DOWN :: to Level:%i from Level:%i in zoneid:%i instid:%i", set_level, m_pp.level, this->GetZoneID(), this->GetInstanceID());
			QServ->PlayerLogEvent(Player_Log_Levels, this->CharacterID(), event_desc);
		}
	}

	m_pp.level = set_level;
	if (command){
		m_pp.exp = GetEXPForLevel(set_level);
		Message(15, "Welcome to level %i!", set_level);
		lu->exp = 0;
	}
	else {
		float tmpxp = (float) ( (float) m_pp.exp - GetEXPForLevel( GetLevel() )) / ( (float) GetEXPForLevel(GetLevel()+1) - GetEXPForLevel(GetLevel()));
		lu->exp = (uint32)(330.0f * tmpxp);
	}
	QueuePacket(outapp);
	safe_delete(outapp);
	this->SendAppearancePacket(AT_WhoLevel, set_level); // who level change

	Log(Logs::General, Logs::Normal, "Setting Level for %s to %i", GetName(), set_level);

	CalcBonuses();

	if(!RuleB(Character, HealOnLevel)) {
		int mhp = CalcMaxHP();
		if(GetHP() > mhp)
			SetHP(mhp);
	}
	else {
		SetHP(CalcMaxHP()); // Why not, lets give them a free heal
	}

	DoTributeUpdate();
	SendHPUpdate();
	SetMana(CalcMaxMana());
	UpdateWho();

	UpdateMercLevel();

	Save();
}

// Note: The client calculates exp separately, we cant change this function
// Add: You can set the values you want now, client will be always sync :) - Merkur
uint32 Client::GetEXPForLevel(uint16 check_level)
{
#ifdef LUA_EQEMU
	uint32 lua_ret = 0;
	bool ignoreDefault = false;
	lua_ret = LuaParser::Instance()->GetEXPForLevel(this, check_level, ignoreDefault);

	if (ignoreDefault) {
		return lua_ret;
	}
#endif

	uint16 check_levelm1 = check_level-1;
	float mod;
	if (check_level < 31)
		mod = 1.0;
	else if (check_level < 36)
		mod = 1.1;
	else if (check_level < 41)
		mod = 1.2;
	else if (check_level < 46)
		mod = 1.3;
	else if (check_level < 52)
		mod = 1.4;
	else if (check_level < 53)
		mod = 1.5;
	else if (check_level < 54)
		mod = 1.6;
	else if (check_level < 55)
		mod = 1.7;
	else if (check_level < 56)
		mod = 1.9;
	else if (check_level < 57)
		mod = 2.1;
	else if (check_level < 58)
		mod = 2.3;
	else if (check_level < 59)
		mod = 2.5;
	else if (check_level < 60)
		mod = 2.7;
	else if (check_level < 61)
		mod = 3.0;
	else
		mod = 3.1;

	float base = (check_levelm1)*(check_levelm1)*(check_levelm1);

	mod *= 1000;

	uint32 finalxp = uint32(base * mod);

	if(RuleB(Character,UseOldRaceExpPenalties))
	{
		float racemod = 1.0;
		if(GetBaseRace() == TROLL || GetBaseRace() == IKSAR) {
			racemod = 1.2;
		} else if(GetBaseRace() == OGRE) {
			racemod = 1.15;
		} else if(GetBaseRace() == BARBARIAN) {
			racemod = 1.05;
		} else if(GetBaseRace() == HALFLING) {
			racemod = 0.95;
		}

		finalxp = uint32(finalxp * racemod);
	}

	if(RuleB(Character,UseOldClassExpPenalties))
	{
		float classmod = 1.0;
		if(GetClass() == PALADIN || GetClass() == SHADOWKNIGHT || GetClass() == RANGER || GetClass() == BARD) {
			classmod = 1.4;
		} else if(GetClass() == MONK) {
			classmod = 1.2;
		} else if(GetClass() == WIZARD || GetClass() == ENCHANTER || GetClass() == MAGICIAN || GetClass() == NECROMANCER) {
			classmod = 1.1;
		} else if(GetClass() == ROGUE) {
			classmod = 0.91;
		} else if(GetClass() == WARRIOR) {
			classmod = 0.9;
		}

		finalxp = uint32(finalxp * classmod);
	}

	finalxp = mod_client_xp_for_level(finalxp, check_level);

	return finalxp;
}

void Client::AddLevelBasedExp(uint8 exp_percentage, uint8 max_level) 
{ 
	uint32	award;
	uint32	xp_for_level;

	if (exp_percentage > 100) 
	{ 
		exp_percentage = 100; 
	} 

	if (!max_level || GetLevel() < max_level)
	{ 
		max_level = GetLevel(); 
	} 

	xp_for_level = GetEXPForLevel(max_level + 1) - GetEXPForLevel(max_level);
	award = xp_for_level * exp_percentage / 100; 

	if(RuleB(Zone, LevelBasedEXPMods))
	{
		if(zone->level_exp_mod[GetLevel()].ExpMod)
		{
			award *= zone->level_exp_mod[GetLevel()].ExpMod;
		}
	}

	uint32 newexp = GetEXP() + award;
	SetEXP(newexp, GetAAXP());
}

void Group::SplitExp(uint32 exp, Mob* other) {
	if( other->CastToNPC()->MerchantType != 0 ) // Ensure NPC isn't a merchant
		return;

	if(other->GetOwner() && other->GetOwner()->IsClient()) // Ensure owner isn't pc
		return;

	unsigned int i;
	uint32 groupexp = exp;
	uint8 membercount = 0;
	uint8 maxlevel = 1;

	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (members[i] != nullptr) {
			if(members[i]->GetLevel() > maxlevel)
				maxlevel = members[i]->GetLevel();

			membercount++;
		}
	}

	float groupmod;
	if (membercount > 1 && membercount < 6)
		groupmod = 1 + .2*(membercount - 1); //2members=1.2exp, 3=1.4, 4=1.6, 5=1.8
	else if (membercount == 6)
		groupmod = 2.16;
	else
		groupmod = 1.0;
	if(membercount > 1 &&  membercount <= 6)
		groupexp += (uint32)((float)exp * groupmod * (RuleR(Character, GroupExpMultiplier)));

	int conlevel = Mob::GetLevelCon(maxlevel, other->GetLevel());
	if(conlevel == CON_GRAY)
		return;	//no exp for greenies...

	if (membercount == 0)
		return;

	for (i = 0; i < MAX_GROUP_MEMBERS; i++) {
		if (members[i] != nullptr && members[i]->IsClient()) // If Group Member is Client
		{
			Client *cmember = members[i]->CastToClient();
			// add exp + exp cap
			int16 diff = cmember->GetLevel() - maxlevel;
			int16 maxdiff = -(cmember->GetLevel()*15/10 - cmember->GetLevel());
				if(maxdiff > -5)
					maxdiff = -5;
			if (diff >= (maxdiff)) { /*Instead of person who killed the mob, the person who has the highest level in the group*/
				uint32 tmp = (cmember->GetLevel()+3) * (cmember->GetLevel()+3) * 75 * 35 / 10;
				uint32 tmp2 = groupexp / membercount;
				cmember->AddEXP( tmp < tmp2 ? tmp : tmp2, conlevel );
			}
		}
	}
}

void Raid::SplitExp(uint32 exp, Mob* other) {
	if( other->CastToNPC()->MerchantType != 0 ) // Ensure NPC isn't a merchant
		return;

	if(other->GetOwner() && other->GetOwner()->IsClient()) // Ensure owner isn't pc
		return;

	uint32 groupexp = exp;
	uint8 membercount = 0;
	uint8 maxlevel = 1;

	for (int i = 0; i < MAX_RAID_MEMBERS; i++) {
		if (members[i].member != nullptr) {
			if(members[i].member->GetLevel() > maxlevel)
				maxlevel = members[i].member->GetLevel();

			membercount++;
		}
	}

	groupexp = (uint32)((float)groupexp * (1.0f-(RuleR(Character, RaidExpMultiplier))));

	int conlevel = Mob::GetLevelCon(maxlevel, other->GetLevel());
	if(conlevel == CON_GRAY)
		return;	//no exp for greenies...

	if (membercount == 0)
		return;

	for (unsigned int x = 0; x < MAX_RAID_MEMBERS; x++) {
		if (members[x].member != nullptr) // If Group Member is Client
		{
			Client *cmember = members[x].member;
			// add exp + exp cap
			int16 diff = cmember->GetLevel() - maxlevel;
			int16 maxdiff = -(cmember->GetLevel()*15/10 - cmember->GetLevel());
			if(maxdiff > -5)
				maxdiff = -5;
			if (diff >= (maxdiff)) { /*Instead of person who killed the mob, the person who has the highest level in the group*/
				uint32 tmp = (cmember->GetLevel()+3) * (cmember->GetLevel()+3) * 75 * 35 / 10;
				uint32 tmp2 = (groupexp / membercount) + 1;
				cmember->AddEXP( tmp < tmp2 ? tmp : tmp2, conlevel );
			}
		}
	}
}

void Client::SetLeadershipEXP(uint32 group_exp, uint32 raid_exp) {
	while(group_exp >= GROUP_EXP_PER_POINT) {
		group_exp -= GROUP_EXP_PER_POINT;
		m_pp.group_leadership_points++;
		Message_StringID(MT_Leadership, GAIN_GROUP_LEADERSHIP_POINT);
	}
	while(raid_exp >= RAID_EXP_PER_POINT) {
		raid_exp -= RAID_EXP_PER_POINT;
		m_pp.raid_leadership_points++;
		Message_StringID(MT_Leadership, GAIN_RAID_LEADERSHIP_POINT);
	}

	m_pp.group_leadership_exp = group_exp;
	m_pp.raid_leadership_exp = raid_exp;

	SendLeadershipEXPUpdate();
}

void Client::AddLeadershipEXP(uint32 group_exp, uint32 raid_exp) {
	SetLeadershipEXP(GetGroupEXP() + group_exp, GetRaidEXP() + raid_exp);
}

void Client::SendLeadershipEXPUpdate() {
	auto outapp = new EQApplicationPacket(OP_LeadershipExpUpdate, sizeof(LeadershipExpUpdate_Struct));
	LeadershipExpUpdate_Struct* eu = (LeadershipExpUpdate_Struct *) outapp->pBuffer;

	eu->group_leadership_exp = m_pp.group_leadership_exp;
	eu->group_leadership_points = m_pp.group_leadership_points;
	eu->raid_leadership_exp = m_pp.raid_leadership_exp;
	eu->raid_leadership_points = m_pp.raid_leadership_points;

	FastQueuePacket(&outapp);
}

uint32 Client::GetCharMaxLevelFromQGlobal() {
	QGlobalCache *char_c = nullptr;
	char_c = this->GetQGlobals();

	std::list<QGlobal> globalMap;
	uint32 ntype = 0;

	if(char_c) {
		QGlobalCache::Combine(globalMap, char_c->GetBucket(), ntype, this->CharacterID(), zone->GetZoneID());
	}

	auto iter = globalMap.begin();
	uint32 gcount = 0;
	while(iter != globalMap.end()) {
		if((*iter).name.compare("CharMaxLevel") == 0){
			return atoi((*iter).value.c_str());
		} 
		++iter;
		++gcount;
	}

	return false;
}

uint32 Client::GetRequiredAAExperience() {
#ifdef LUA_EQEMU
	uint32 lua_ret = 0;
	bool ignoreDefault = false;
	lua_ret = LuaParser::Instance()->GetRequiredAAExperience(this, ignoreDefault);

	if (ignoreDefault) {
		return lua_ret;
	}
#endif

	return RuleI(AA, ExpPerPoint);
}
