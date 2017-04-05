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

#ifdef BOTS

#include "bot.h"
#include "../common/string_util.h"

#if EQDEBUG >= 12
	#define BotAI_DEBUG_Spells	25
#elif EQDEBUG >= 9
	#define BotAI_DEBUG_Spells	10
#else
	#define BotAI_DEBUG_Spells	-1
#endif

bool Bot::AICastSpell(Mob* tar, uint8 iChance, uint32 iSpellTypes) {

	if (!tar) {
		return false;
	}

	if(!AI_HasSpells())
		return false;

	if (iChance < 100) {
		if (zone->random.Int(0, 100) > iChance){
			return false;
		}
	}

	if(tar->GetAppearance() == eaDead) {
		if((tar->IsClient() && tar->CastToClient()->GetFeigned()) || tar->IsBot()) {
			// do nothing
		}
		else {
			return false;
		}
	}

	uint8 botClass = GetClass();
	uint8 botLevel = GetLevel();

	bool checked_los = false;	//we do not check LOS until we are absolutely sure we need to, and we only do it once.

	bool castedSpell = false;

	BotSpell botSpell;
	botSpell.SpellId = 0;
	botSpell.SpellIndex = 0;
	botSpell.ManaCost = 0;

	switch (iSpellTypes) {
		case SpellType_Mez: {
			if (tar->GetBodyType() != BT_Giant) {
				if(!checked_los) {
					if(!CheckLosFN(tar))
						break;	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call

					checked_los = true;
				}

				//TODO
				//Check if single target or AoE mez is best
				//if (TARGETS ON MT IS => 3 THEN botSpell = AoEMez)
				//if (TARGETS ON MT IS <= 2 THEN botSpell = BestMez)

				botSpell = GetBestBotSpellForMez(this);

				if(botSpell.SpellId == 0)
					break;

				Mob* addMob = GetFirstIncomingMobToMez(this, botSpell);

				if(!addMob){
					//Say("!addMob.");
					break;}

				if(!(!addMob->IsImmuneToSpell(botSpell.SpellId, this) && addMob->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0))
					break;

				castedSpell = AIDoSpellCast(botSpell.SpellIndex, addMob, botSpell.ManaCost);

				if(castedSpell)
					BotGroupSay(this, "Attempting to mez %s.", addMob->GetCleanName());
			}
			break;
		}
		case SpellType_Heal: {
			if (tar->DontHealMeBefore() < Timer::GetCurrentTime()) {
				uint8 hpr = (uint8)tar->GetHPRatio();
				bool hasAggro = false;
				bool isPrimaryHealer = false;

				if(HasGroup()) {
					isPrimaryHealer = IsGroupHealer();
				}

				if(hpr < 95 || (tar->IsClient() && (hpr < 95)) || (botClass == BARD)) {
					if(tar->GetClass() == NECROMANCER) {
						// Give necromancers a chance to go lifetap something or cleric can spend too much mana on a necro
						if(hpr >= 40) {
							break;
						}
					}

					if(tar->GetClass() == SHAMAN) {
						// Give shaman the chance to canni without wasting the cleric's mana
						if(hpr >= 80) {
							break;
						}
					}

					// Evaluate the situation
					if((IsEngaged()) && ((botClass == CLERIC) || (botClass == DRUID) || (botClass == SHAMAN) || (botClass == PALADIN))) {
						if(tar->GetTarget() && tar->GetTarget()->GetHateTop() && tar->GetTarget()->GetHateTop() == tar) {
							hasAggro = true;
						}

						if(hpr < 35) {
							botSpell = GetBestBotSpellForFastHeal(this);
						}
						else if(hpr >= 35 && hpr < 70){
							if(GetNumberNeedingHealedInGroup(60, false) >= 3)
								botSpell = GetBestBotSpellForGroupHeal(this);

							if(botSpell.SpellId == 0)
								botSpell = GetBestBotSpellForPercentageHeal(this);
						}
						else if(hpr >= 70 && hpr < 95){
							if(GetNumberNeedingHealedInGroup(80, false) >= 3)
								botSpell = GetBestBotSpellForGroupHealOverTime(this);

							if(hasAggro)
								botSpell = GetBestBotSpellForPercentageHeal(this);
						}
						else {
							if(!tar->FindType(SE_HealOverTime))
								botSpell = GetBestBotSpellForHealOverTime(this);
						}
					}
					else if ((botClass == CLERIC) || (botClass == DRUID) || (botClass == SHAMAN) || (botClass == PALADIN)) {
						if(GetNumberNeedingHealedInGroup(40, true) >= 2){
							botSpell = GetBestBotSpellForGroupCompleteHeal(this);

							if(botSpell.SpellId == 0)
								botSpell = GetBestBotSpellForGroupHeal(this);

							if(botSpell.SpellId == 0)
								botSpell = GetBestBotSpellForGroupHealOverTime(this);

							if(hpr < 40) {
								if(botSpell.SpellId == 0)
									botSpell = GetBestBotSpellForPercentageHeal(this);
							}
						}
						else if(GetNumberNeedingHealedInGroup(60, true) >= 2){
							botSpell = GetBestBotSpellForGroupHeal(this);

							if(botSpell.SpellId == 0)
								botSpell = GetBestBotSpellForGroupHealOverTime(this);

							if(hpr < 40) {
								if(botSpell.SpellId == 0)
									botSpell = GetBestBotSpellForPercentageHeal(this);
							}
						}
						else if(hpr < 40)
							botSpell = GetBestBotSpellForPercentageHeal(this);
						else if(hpr >= 40 && hpr < 75)
							botSpell = GetBestBotSpellForRegularSingleTargetHeal(this);
						else {
							if(hpr < 90 && !tar->FindType(SE_HealOverTime))
								botSpell = GetBestBotSpellForHealOverTime(this);
						}
					}
					else {
						float hpRatioToCast = 0.0f;

						switch(this->GetBotStance())
						{
							case BotStanceEfficient:
							case BotStanceAggressive:
								hpRatioToCast = isPrimaryHealer?90.0f:50.0f;
								break;
							case BotStanceBalanced:
								hpRatioToCast = isPrimaryHealer?95.0f:75.0f;
								break;
							case BotStanceReactive:
								hpRatioToCast = isPrimaryHealer?100.0f:90.0f;
								break;
							case BotStanceBurn:
							case BotStanceBurnAE:
								hpRatioToCast = isPrimaryHealer?75.0f:25.0f;
								break;
							default:
								hpRatioToCast = isPrimaryHealer?100.0f:0.0f;
								break;
						}

						//If we're at specified mana % or below, don't heal as hybrid
						if(tar->GetHPRatio() <= hpRatioToCast)
							botSpell = GetBestBotSpellForRegularSingleTargetHeal(this);
					}

					if(botSpell.SpellId == 0)
						botSpell = GetBestBotSpellForRegularSingleTargetHeal(this);

					if(botSpell.SpellId == 0)
						botSpell = GetFirstBotSpellForSingleTargetHeal(this);

					if(botSpell.SpellId == 0 && botClass == BARD){
						botSpell = GetFirstBotSpellBySpellType(this, SpellType_Heal);
					}

					// If there is still no spell id, then there isn't going to be one so we are done
					if(botSpell.SpellId == 0)
						break;

					// Can we cast this spell on this target?
					if(!(spells[botSpell.SpellId].targettype==ST_GroupTeleport || spells[botSpell.SpellId].targettype == ST_Target || tar == this)
						&& !(tar->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0))
						break;

					uint32 TempDontHealMeBeforeTime = tar->DontHealMeBefore();

					castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost, &TempDontHealMeBeforeTime);

					if(castedSpell) {
						/*if(TempDontHealMeBeforeTime != tar->DontHealMeBefore())
							tar->SetDontHealMeBefore(TempDontHealMeBeforeTime);

						// For non-HoT heals, do a 4 second delay
						// TODO: Replace this code with logic that calculates the delay based on number of clerics in rotation
						//			and ignores heals for anyone except the main tank
						if(!IsHealOverTimeSpell(botSpell.SpellId)) {
							if(IsCompleteHealSpell(botSpell.SpellId)) {
								// Complete Heal 4 second rotation
								tar->SetDontHealMeBefore(Timer::GetCurrentTime() + 4000);
							}
							else {
								tar->SetDontHealMeBefore(Timer::GetCurrentTime() + 1000);
							}
						}*/
						if(botClass != BARD) {
							if(IsGroupSpell(botSpell.SpellId)){
								if(this->HasGroup()) {
									Group *g = this->GetGroup();

									if(g) {
										BotGroupSay(this, "Casting %s.", spells[botSpell.SpellId].name);

										for( int i = 0; i<MAX_GROUP_MEMBERS; i++) {
											if(g->members[i] && !g->members[i]->qglobal) {
												g->members[i]->SetDontHealMeBefore(Timer::GetCurrentTime() + 1000);
											}
										}
									}
								}
							}
							else {
								if(tar != this)		//we don't need spam of bots healing themselves
									BotGroupSay(this, "Casting %s on %s", spells[botSpell.SpellId].name, tar->GetCleanName());

								tar->SetDontHealMeBefore(Timer::GetCurrentTime() + 2000);
							}
						}
					}
				}
			}
			break;
		}
		case SpellType_Root: {
			if (!tar->IsRooted() && tar->DontRootMeBefore() < Timer::GetCurrentTime()) {
					if(!checked_los) {
						if(!CheckLosFN(tar))
							break;	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call

						checked_los = true;
					}

					// TODO: If there is a ranger in the group then don't allow root spells

					botSpell = GetFirstBotSpellBySpellType(this, iSpellTypes);

					if(botSpell.SpellId == 0)
						break;

					if(tar->CanBuffStack(botSpell.SpellId, botLevel, true) == 0)
						break;

					uint32 TempDontRootMeBefore = tar->DontRootMeBefore();

					castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost, &TempDontRootMeBefore);

					if(TempDontRootMeBefore != tar->DontRootMeBefore())
						tar->SetDontRootMeBefore(TempDontRootMeBefore);
			}
			break;
		}
		case SpellType_Buff: {
			if (tar->DontBuffMeBefore() < Timer::GetCurrentTime()) {
				std::list<BotSpell> buffSpellList = GetBotSpellsBySpellType(this, SpellType_Buff);

				for(std::list<BotSpell>::iterator itr = buffSpellList.begin(); itr != buffSpellList.end(); ++itr) {
					BotSpell selectedBotSpell = *itr;

					if(selectedBotSpell.SpellId == 0)
						continue;

					// no buffs with illusions.. use #bot command to cast illusions
					if(IsEffectInSpell(selectedBotSpell.SpellId, SE_Illusion) && tar != this)
						continue;

					//no teleport spells use #bot command to cast teleports
					if(IsEffectInSpell(selectedBotSpell.SpellId, SE_Teleport) || IsEffectInSpell(selectedBotSpell.SpellId, SE_Succor))
						continue;

					// can not cast buffs for your own pet only on another pet that isn't yours
					if((spells[selectedBotSpell.SpellId].targettype == ST_Pet) && (tar != this->GetPet()))
						continue;

					// Validate target

					if(!((spells[selectedBotSpell.SpellId].targettype == ST_Target || spells[selectedBotSpell.SpellId].targettype == ST_Pet || tar == this ||
						spells[selectedBotSpell.SpellId].targettype == ST_Group || spells[selectedBotSpell.SpellId].targettype == ST_GroupTeleport ||
						(botClass == BARD && spells[selectedBotSpell.SpellId].targettype == ST_AEBard))
						&& !tar->IsImmuneToSpell(selectedBotSpell.SpellId, this)
						&& (tar->CanBuffStack(selectedBotSpell.SpellId, botLevel, true) >= 0))) {
							continue;
					}

					// Put the zone levitate and movement check here since bots are able to bypass the client casting check
					if((IsEffectInSpell(selectedBotSpell.SpellId, SE_Levitate) && !zone->CanLevitate())
						|| (IsEffectInSpell(selectedBotSpell.SpellId, SE_MovementSpeed) && !zone->CanCastOutdoor())) {
							if(botClass != BARD || !IsSpellUsableThisZoneType(selectedBotSpell.SpellId, zone->GetZoneType())){
								continue;
							}
					}

					switch(tar->GetArchetype())
					{
						case ARCHETYPE_CASTER:
							//TODO: probably more caster specific spell effects in here
							if(IsEffectInSpell(selectedBotSpell.SpellId, SE_AttackSpeed) || IsEffectInSpell(selectedBotSpell.SpellId, SE_ATK) ||
								IsEffectInSpell(selectedBotSpell.SpellId, SE_STR) || IsEffectInSpell(selectedBotSpell.SpellId, SE_ReverseDS))
							{
								continue;
							}
							break;
						case ARCHETYPE_MELEE:
							if(IsEffectInSpell(selectedBotSpell.SpellId, SE_IncreaseSpellHaste) || IsEffectInSpell(selectedBotSpell.SpellId, SE_ManaPool) ||
								IsEffectInSpell(selectedBotSpell.SpellId, SE_CastingLevel) || IsEffectInSpell(selectedBotSpell.SpellId, SE_ManaRegen_v2) ||
								IsEffectInSpell(selectedBotSpell.SpellId, SE_CurrentMana))
							{
								continue;
							}
							break;
						case ARCHETYPE_HYBRID:
							//Hybrids get all buffs
						default:
							break;
					}

					if(botClass == ENCHANTER && IsEffectInSpell(selectedBotSpell.SpellId, SE_Rune))
					{
						float manaRatioToCast = 75.0f;

						switch(this->GetBotStance())
						{
							case BotStanceEfficient:
								manaRatioToCast = 90.0f;
								break;
							case BotStanceBalanced:
							case BotStanceAggressive:
								manaRatioToCast = 75.0f;
								break;
							case BotStanceReactive:
							case BotStanceBurn:
							case BotStanceBurnAE:
								manaRatioToCast = 50.0f;
								break;
							default:
								manaRatioToCast = 75.0f;
								break;
						}

						//If we're at specified mana % or below, don't rune as enchanter
						if(this->GetManaRatio() <= manaRatioToCast)
							break;
					}

					if(CheckSpellRecastTimers(this, itr->SpellIndex))
					{

						uint32 TempDontBuffMeBefore = tar->DontBuffMeBefore();

						castedSpell = AIDoSpellCast(selectedBotSpell.SpellIndex, tar, selectedBotSpell.ManaCost, &TempDontBuffMeBefore);

						if(TempDontBuffMeBefore != tar->DontBuffMeBefore())
							tar->SetDontBuffMeBefore(TempDontBuffMeBefore);
					}

					if(castedSpell)
						break;
				}
			}
			break;
		}
		case SpellType_Escape: {
			uint8 hpr = (uint8)GetHPRatio();
			bool mayGetAggro = false;

#ifdef IPC
			if (hpr <= 5 || (IsNPC() && CastToNPC()->IsInteractive() && tar != this) )
#else
			if(hpr > 15 && ((botClass == WIZARD) || (botClass == ENCHANTER) || (botClass == RANGER)))
				mayGetAggro = HasOrMayGetAggro(); //classes have hate reducing spells

			if (hpr <= 15 || mayGetAggro)
#endif
			{
				botSpell = GetFirstBotSpellBySpellType(this, iSpellTypes);

				if(botSpell.SpellId == 0)
					break;

				if(IsInvulnerabilitySpell(botSpell.SpellId))
					tar = this; //target self for invul type spells

				castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost);
			}
			break;
		}
		case SpellType_Nuke: {
			if((tar->GetHPRatio() <= 95.0f) || ((botClass == BARD) || (botClass == SHAMAN) || (botClass == ENCHANTER)))
			{
				if(!checked_los) {
					if(!CheckLosFN(tar))
						break;	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call

					checked_los = true;
				}

				if(botClass == CLERIC || botClass == ENCHANTER)
				{
					float manaRatioToCast = 75.0f;

					switch(this->GetBotStance())
					{
						case BotStanceEfficient:
							manaRatioToCast = 90.0f;
							break;
						case BotStanceBalanced:
							manaRatioToCast = 75.0f;
							break;
						case BotStanceReactive:
						case BotStanceAggressive:
							manaRatioToCast = 50.0f;
							break;
						case BotStanceBurn:
						case BotStanceBurnAE:
							manaRatioToCast = 25.0f;
							break;
						default:
							manaRatioToCast = 50.0f;
							break;
					}

					//If we're at specified mana % or below, don't nuke as cleric or enchanter
					if(this->GetManaRatio() <= manaRatioToCast)
						break;
				}

				if(botClass == MAGICIAN || botClass == SHADOWKNIGHT || botClass == NECROMANCER || botClass == PALADIN || botClass == RANGER || botClass == DRUID || botClass == CLERIC) {
					if(tar->GetBodyType() == BT_Undead || tar->GetBodyType() == BT_SummonedUndead || tar->GetBodyType() == BT_Vampire)
						botSpell = GetBestBotSpellForNukeByTargetType(this, ST_Undead);
					else if(tar->GetBodyType() == BT_Summoned || tar->GetBodyType() == BT_Summoned2 || tar->GetBodyType() == BT_Summoned3)
						botSpell = GetBestBotSpellForNukeByTargetType(this, ST_Summoned);
				}

				if(botClass == PALADIN || botClass == DRUID || botClass == CLERIC || botClass == ENCHANTER || botClass == WIZARD) {
					if(botSpell.SpellId == 0) {
						uint8 stunChance = (tar->IsCasting() ? 30: 15);

						if(botClass == PALADIN)
							stunChance = 50;

						if(!tar->GetSpecialAbility(UNSTUNABLE) && !tar->IsStunned() && (zone->random.Int(1, 100) <= stunChance)) {
							botSpell = GetBestBotSpellForStunByTargetType(this, ST_Target);
						}
					}
				}

				if(botClass == WIZARD && botSpell.SpellId == 0) {
					botSpell = GetBestBotWizardNukeSpellByTargetResists(this, tar);
				}

				if(botSpell.SpellId == 0)
					botSpell = GetBestBotSpellForNukeByTargetType(this, ST_Target);

				if(botSpell.SpellId == 0)
					break;

				if(!(!tar->IsImmuneToSpell(botSpell.SpellId, this) && (tar->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0)))
					break;

				if(IsFearSpell(botSpell.SpellId)) {
					// don't let fear cast if the npc isn't snared or rooted
					if(tar->GetSnaredAmount() == -1) {
						if(!tar->IsRooted())
							break;
					}
				}

				castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost);
			}
			break;
		}
		case SpellType_Dispel: {
			if(tar->GetHPRatio() > 95.0f) {
				if(!checked_los) {
					if(!CheckLosFN(tar))
						break;	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call

					checked_los = true;
				}

				botSpell = GetFirstBotSpellBySpellType(this, iSpellTypes);

				if(botSpell.SpellId == 0)
					break;

				// TODO: Check target to see if there is anything to dispel

				if(tar->CountDispellableBuffs() > 0) {
					castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost);
				}
			}
			break;
		}
		case SpellType_Pet: {
			//keep mobs from recasting pets when they have them.
			if (!IsPet() && !GetPetID() && !IsBotCharmer()) {
				if (botClass == WIZARD) {
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
					if (GetPetID())
						break;

					if (familiar_buff_slot >= 0) {
						BuffFadeBySlot(familiar_buff_slot);
						break;
					}

					botSpell = GetFirstBotSpellBySpellType(this, SpellType_Pet);
				}
				else if (botClass == MAGICIAN) {
					botSpell = GetBestBotMagicianPetSpell(this);
				}
				else {
					botSpell = GetFirstBotSpellBySpellType(this, SpellType_Pet);
				}

				if(botSpell.SpellId == 0)
					break;

				castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost);
			}
			break;
		}
		case SpellType_InCombatBuff: {

			if(botClass == SHAMAN) {
				checked_los = true;

				std::list<BotSpell> inCombatBuffList = GetBotSpellsBySpellType(this, SpellType_InCombatBuff);

				for(std::list<BotSpell>::iterator itr = inCombatBuffList.begin(); itr != inCombatBuffList.end(); ++itr) {
					BotSpell selectedBotSpell = *itr;

					if(selectedBotSpell.SpellId == 0)
						continue;

					if(CheckSpellRecastTimers(this, itr->SpellIndex))
					{
						if(!(!tar->IsImmuneToSpell(selectedBotSpell.SpellId, this) && (spells[selectedBotSpell.SpellId].buffduration < 1 || tar->CanBuffStack(selectedBotSpell.SpellId, botLevel, true) >= 0)))
							continue;

						//short duration buffs or other buffs only to be cast during combat.
						if (IsSelfConversionSpell(selectedBotSpell.SpellId)) {
							if(GetManaRatio() > 90.0f || GetHPRatio() < 50.0f || GetHPRatio() < (GetManaRatio() + 10.0f))
								break; //don't cast if low hp, lots of mana, or if mana is higher than hps
						}

						castedSpell = AIDoSpellCast(selectedBotSpell.SpellIndex, tar, selectedBotSpell.ManaCost);
					}

					if(castedSpell)
						break;
				}
			}
			else if(botClass == BARD) { 
				if (tar->DontBuffMeBefore() < Timer::GetCurrentTime()) {
					std::list<BotSpell> inCombatBuffList = GetBotSpellsBySpellType(this, SpellType_InCombatBuff);

					for(std::list<BotSpell>::iterator itr = inCombatBuffList.begin(); itr != inCombatBuffList.end(); ++itr) {
						BotSpell selectedBotSpell = *itr;

						if(selectedBotSpell.SpellId == 0)
							continue;

						if(CheckSpellRecastTimers(this, itr->SpellIndex)) {
							uint32 TempDontBuffMeBefore = tar->DontBuffMeBefore();

							// no buffs with illusions.. use #bot command to cast illusions
							if(IsEffectInSpell(selectedBotSpell.SpellId, SE_Illusion) && tar != this)
								continue;

							//no teleport spells use #bot command to cast teleports
							if(IsEffectInSpell(selectedBotSpell.SpellId, SE_Teleport) || IsEffectInSpell(selectedBotSpell.SpellId, SE_Succor))
								continue;

							// can not cast buffs for your own pet only on another pet that isn't yours
							if((spells[selectedBotSpell.SpellId].targettype == ST_Pet) && (tar != this->GetPet()))
								continue;

							// Validate target

							if(!((spells[selectedBotSpell.SpellId].targettype == ST_Target || spells[selectedBotSpell.SpellId].targettype == ST_Pet || tar == this ||
								spells[selectedBotSpell.SpellId].targettype == ST_Group || spells[selectedBotSpell.SpellId].targettype == ST_GroupTeleport ||
								(botClass == BARD && spells[selectedBotSpell.SpellId].targettype == ST_AEBard))
								&& !tar->IsImmuneToSpell(selectedBotSpell.SpellId, this)
								&& (tar->CanBuffStack(selectedBotSpell.SpellId, botLevel, true) >= 0))) {
									continue;
							}

							// Put the zone levitate and movement check here since bots are able to bypass the client casting check
							if((IsEffectInSpell(selectedBotSpell.SpellId, SE_Levitate) && !zone->CanLevitate())
								|| (IsEffectInSpell(selectedBotSpell.SpellId, SE_MovementSpeed) && !zone->CanCastOutdoor())) {
									if(!IsSpellUsableThisZoneType(selectedBotSpell.SpellId, zone->GetZoneType())) {
										continue;
									}
							}

							if(!IsGroupSpell(selectedBotSpell.SpellId)) {
								//Only check archetype if song is not a group spell
								switch(tar->GetArchetype()) {
									case ARCHETYPE_CASTER:
										//TODO: probably more caster specific spell effects in here
										if(IsEffectInSpell(selectedBotSpell.SpellId, SE_AttackSpeed) || IsEffectInSpell(selectedBotSpell.SpellId, SE_ATK) ||
											IsEffectInSpell(selectedBotSpell.SpellId, SE_STR) || IsEffectInSpell(selectedBotSpell.SpellId, SE_ReverseDS))
										{
											continue;
										}
										break;
									case ARCHETYPE_MELEE:
										if(IsEffectInSpell(selectedBotSpell.SpellId, SE_IncreaseSpellHaste) || IsEffectInSpell(selectedBotSpell.SpellId, SE_ManaPool) ||
											IsEffectInSpell(selectedBotSpell.SpellId, SE_CastingLevel) || IsEffectInSpell(selectedBotSpell.SpellId, SE_ManaRegen_v2) ||
											IsEffectInSpell(selectedBotSpell.SpellId, SE_CurrentMana))
										{
											continue;
										}
										break;
									case ARCHETYPE_HYBRID:
										//Hybrids get all buffs
									default:
										break;
								}
							}

							castedSpell = AIDoSpellCast(selectedBotSpell.SpellIndex, tar, selectedBotSpell.ManaCost, &TempDontBuffMeBefore);

							if(TempDontBuffMeBefore != tar->DontBuffMeBefore())
								tar->SetDontBuffMeBefore(TempDontBuffMeBefore);
						}

						if(castedSpell)
							break;
					}
				}
			}
			break;
		}
		case SpellType_Lifetap: {
			if (GetHPRatio() < 90.0f) {
				if(!checked_los) {
					if(!CheckLosFN(tar))
						break;	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call

					checked_los = true;
				}

				botSpell = GetFirstBotSpellBySpellType(this, iSpellTypes);

				if(botSpell.SpellId == 0)
					break;

				if(!(!tar->IsImmuneToSpell(botSpell.SpellId, this) && (tar->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0)))
					break;

				castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost);
			}
			break;
		}
		case SpellType_Snare: {
			if (tar->DontSnareMeBefore() < Timer::GetCurrentTime()) {
					if(!checked_los) {
						if(!CheckLosFN(tar))
							break;	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call

						checked_los = true;
					}

					botSpell = GetFirstBotSpellBySpellType(this, iSpellTypes);

					if(botSpell.SpellId == 0)
						break;

					if(!(!tar->IsImmuneToSpell(botSpell.SpellId, this) && tar->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0))
						break;

					uint32 TempDontSnareMeBefore = tar->DontSnareMeBefore();

					castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost, &TempDontSnareMeBefore);

					if(TempDontSnareMeBefore != tar->DontSnareMeBefore())
						tar->SetDontSnareMeBefore(TempDontSnareMeBefore);
			}
			break;
		}
		case SpellType_DOT: {
			if ((tar->GetHPRatio() <= 98.0f) && (tar->DontDotMeBefore() < Timer::GetCurrentTime()) && (tar->GetHPRatio() > 15.0f)) {
				if(!checked_los) {
					if(!CheckLosFN(tar))
						break;	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call

					checked_los = true;
				}

				if (GetClass() == BARD) {
					std::list<BotSpell_wPriority> dotList = GetPrioritizedBotSpellsBySpellType(this, SpellType_DOT);

					const int maxDotSelect = 5;
					int dotSelectCounter = 0;

					for (std::list<BotSpell_wPriority>::iterator itr = dotList.begin(); itr != dotList.end(); ++itr) {
						BotSpell selectedBotSpell = *itr;

						if (selectedBotSpell.SpellId == 0)
							continue;

						if (CheckSpellRecastTimers(this, itr->SpellIndex))
						{

							if (!(!tar->IsImmuneToSpell(selectedBotSpell.SpellId, this) && tar->CanBuffStack(selectedBotSpell.SpellId, botLevel, true) >= 0))
								continue;

							uint32 TempDontDotMeBefore = tar->DontDotMeBefore();

							castedSpell = AIDoSpellCast(selectedBotSpell.SpellIndex, tar, selectedBotSpell.ManaCost, &TempDontDotMeBefore);

							if (TempDontDotMeBefore != tar->DontDotMeBefore())
								tar->SetDontDotMeBefore(TempDontDotMeBefore);
						}

						dotSelectCounter++;

						if ((dotSelectCounter == maxDotSelect) || castedSpell)
							break;
					}
				}
				else {
					std::list<BotSpell> dotList = GetBotSpellsBySpellType(this, SpellType_DOT);

					const int maxDotSelect = 5;
					int dotSelectCounter = 0;

					for (std::list<BotSpell>::iterator itr = dotList.begin(); itr != dotList.end(); ++itr) {
						BotSpell selectedBotSpell = *itr;

						if (selectedBotSpell.SpellId == 0)
							continue;

						if (CheckSpellRecastTimers(this, itr->SpellIndex))
						{

							if (!(!tar->IsImmuneToSpell(selectedBotSpell.SpellId, this) && tar->CanBuffStack(selectedBotSpell.SpellId, botLevel, true) >= 0))
								continue;

							uint32 TempDontDotMeBefore = tar->DontDotMeBefore();

							castedSpell = AIDoSpellCast(selectedBotSpell.SpellIndex, tar, selectedBotSpell.ManaCost, &TempDontDotMeBefore);

							if (TempDontDotMeBefore != tar->DontDotMeBefore())
								tar->SetDontDotMeBefore(TempDontDotMeBefore);
						}

						dotSelectCounter++;

						if ((dotSelectCounter == maxDotSelect) || castedSpell)
							break;
					}
				}
			}
			break;
		}
		case SpellType_Slow: {
			if (tar->GetHPRatio() <= 99.0f) {

				if(!checked_los) {
					if(!CheckLosFN(tar))
						break;	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call

					checked_los = true;
				}

				switch (botClass) {
					case BARD: {
						// probably needs attackable check
						std::list<BotSpell_wPriority> botSongList = GetPrioritizedBotSpellsBySpellType(this, SpellType_Slow);
						for (auto iter : botSongList) {
							if (!iter.SpellId)
								continue;
							if (!CheckSpellRecastTimers(this, iter.SpellIndex))
								continue;
							if (spells[iter.SpellId].zonetype != -1 && zone->GetZoneType() != -1 && spells[iter.SpellId].zonetype != zone->GetZoneType()) // is this bit or index?
								continue;
							if (spells[iter.SpellId].targettype != ST_Target)
								continue;
							if (tar->CanBuffStack(iter.SpellId, botLevel, true) < 0)
								continue;

							castedSpell = AIDoSpellCast(iter.SpellIndex, tar, iter.ManaCost);
							if (castedSpell)
								break;
						}

						break;
					}
					case ENCHANTER: {
						botSpell = GetBestBotSpellForMagicBasedSlow(this);
						break;
					}
					case SHAMAN:
					case BEASTLORD: {
						botSpell = GetBestBotSpellForDiseaseBasedSlow(this);

						if(botSpell.SpellId == 0 || ((tar->GetMR() - 50) < (tar->GetDR() + spells[botSpell.SpellId].ResistDiff)))
							botSpell = GetBestBotSpellForMagicBasedSlow(this);
						break;
					}
				}

				if(botSpell.SpellId == 0)
					break;

				if(!(!tar->IsImmuneToSpell(botSpell.SpellId, this) && tar->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0))
					break;

				castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost);

				if(castedSpell && GetClass() != BARD)
					BotGroupSay(this, "Attempting to slow %s.", tar->GetCleanName());
			}
			break;
		}
		case SpellType_Debuff: {
			if((tar->GetHPRatio() <= 99.0f) || ((botClass == BARD) || (botClass == SHAMAN) || (botClass == ENCHANTER) || (botClass == DRUID)) && (tar->GetHPRatio() > 40.0f))
			{
				if(!checked_los) {
					if(!CheckLosFN(tar))
						break;	//cannot see target... we assume that no spell is going to work since we will only be casting detrimental spells in this call

					checked_los = true;
				}

				botSpell = GetBestBotSpellForResistDebuff(this, tar);

				if(botSpell.SpellId == 0)
					botSpell = GetDebuffBotSpell(this, tar);

				if(botSpell.SpellId == 0)
					break;

				if(!(!tar->IsImmuneToSpell(botSpell.SpellId, this) && (tar->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0)))
					break;

				castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost);
			}
			break;
		}
		case SpellType_Cure: {
			if(GetNeedsCured(tar) && (tar->DontCureMeBefore() < Timer::GetCurrentTime()) && !(GetNumberNeedingHealedInGroup(25, false) > 0) && !(GetNumberNeedingHealedInGroup(40, false) > 2))
			{
				botSpell = GetBestBotSpellForCure(this, tar);

				if(botSpell.SpellId == 0)
					break;

				uint32 TempDontCureMeBeforeTime = tar->DontCureMeBefore();

				castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost, &TempDontCureMeBeforeTime);

				if(castedSpell) {
					if(botClass != BARD) {
						if(IsGroupSpell(botSpell.SpellId)){
							Group *g;

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
			}
			break;
		}
		case SpellType_Resurrect:
			break;
		case SpellType_HateRedux: {
			// assumed group member at this point
			if (GetClass() == BARD) {
				std::list<BotSpell_wPriority> botSongList = GetPrioritizedBotSpellsBySpellType(this, SpellType_HateRedux);
				for (auto iter : botSongList) {
					if (!iter.SpellId)
						continue;
					if (!CheckSpellRecastTimers(this, iter.SpellIndex))
						continue;
					if (spells[iter.SpellId].zonetype != -1 && zone->GetZoneType() != -1 && spells[iter.SpellId].zonetype != zone->GetZoneType()) // is this bit or index?
						continue;
					if (spells[iter.SpellId].targettype != ST_Target)
						continue;
					if (tar->CanBuffStack(iter.SpellId, botLevel, true) < 0)
						continue;

					castedSpell = AIDoSpellCast(iter.SpellIndex, tar, iter.ManaCost);
					if (castedSpell) {
						BotGroupSay(this, "Attempting to reduce hate on %s.", tar->GetCleanName());
						break;
					}
				}
			}

			break;
		}
		case SpellType_InCombatBuffSong: {
			if (GetClass() != BARD || tar != this) // In-Combat songs can be cast Out-of-Combat in preparation for battle
				break;

			std::list<BotSpell_wPriority> botSongList = GetPrioritizedBotSpellsBySpellType(this, SpellType_InCombatBuffSong);
			for (auto iter : botSongList) {
				if (!iter.SpellId)
					continue;
				if (!CheckSpellRecastTimers(this, iter.SpellIndex))
					continue;
				if (spells[iter.SpellId].zonetype != -1 && zone->GetZoneType() != -1 && spells[iter.SpellId].zonetype != zone->GetZoneType()) // is this bit or index?
					continue;
				switch (spells[iter.SpellId].targettype) {
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

				castedSpell = AIDoSpellCast(iter.SpellIndex, tar, iter.ManaCost);
				if (castedSpell)
					break;
			}

			break;
		}
		case SpellType_OutOfCombatBuffSong: {
			if (GetClass() != BARD || tar != this || IsEngaged()) // Out-of-Combat songs can not be cast in combat
				break;

			std::list<BotSpell_wPriority> botSongList = GetPrioritizedBotSpellsBySpellType(this, SpellType_OutOfCombatBuffSong);
			for (auto iter : botSongList) {
				if (!iter.SpellId)
					continue;
				if (!CheckSpellRecastTimers(this, iter.SpellIndex))
					continue;
				if (spells[iter.SpellId].zonetype != -1 && zone->GetZoneType() != -1 && spells[iter.SpellId].zonetype != zone->GetZoneType()) // is this bit or index?
					continue;
				switch (spells[iter.SpellId].targettype) {
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

				castedSpell = AIDoSpellCast(iter.SpellIndex, tar, iter.ManaCost);
				if (castedSpell)
					break;
			}

			break;
		}
		case SpellType_PreCombatBuff: {
			break;
		}
		case SpellType_PreCombatBuffSong: {
			break;
		}
		default:
			break;
	}

	return castedSpell;
}

bool Bot::AIDoSpellCast(uint8 i, Mob* tar, int32 mana_cost, uint32* oDontDoAgainBefore) {
	bool result = false;

	// manacost has special values, -1 is no mana cost, -2 is instant cast (no mana)
	int32 manaCost = mana_cost;

	if (manaCost == -1)
		manaCost = spells[AIspells[i].spellid].mana;
	else if (manaCost == -2)
		manaCost = 0;

	int32 extraMana = 0;
	int32 hasMana = GetMana();

	// Allow bots to cast buff spells even if they are out of mana
	if(RuleB(Bots, FinishBuffing)) {
		if(manaCost > hasMana) {
			// Let's have the bots complete the buff time process
			if(AIspells[i].type & SpellType_Buff) {
				extraMana = manaCost - hasMana;
				SetMana(manaCost);
			}
		}
	}

	float dist2 = 0;

	if (AIspells[i].type & SpellType_Escape) {
		dist2 = 0;
	} else
		dist2 = DistanceSquared(m_Position, tar->GetPosition());

	if (((((spells[AIspells[i].spellid].targettype==ST_GroupTeleport && AIspells[i].type==2)
				|| spells[AIspells[i].spellid].targettype==ST_AECaster
				|| spells[AIspells[i].spellid].targettype==ST_Group
				|| spells[AIspells[i].spellid].targettype==ST_AEBard)
				&& dist2 <= spells[AIspells[i].spellid].aoerange*spells[AIspells[i].spellid].aoerange)
				|| dist2 <= GetActSpellRange(AIspells[i].spellid, spells[AIspells[i].spellid].range)*GetActSpellRange(AIspells[i].spellid, spells[AIspells[i].spellid].range)) && (mana_cost <= GetMana() || GetMana() == GetMaxMana()))
	{
		result = NPC::AIDoSpellCast(i, tar, mana_cost, oDontDoAgainBefore);

		if(IsCasting() && IsSitting())
			Stand();
	}

	// if the spell wasn't casted, then take back any extra mana that was given to the bot to cast that spell
	if(!result) {
		SetMana(hasMana);
		extraMana = false;
	}
	else { //handle spell recast and recast timers
		//if(GetClass() == BARD && IsGroupSpell(AIspells[i].spellid)) {
		//	// Bard buff songs have been moved to their own npc spell type..
		//	// Buff stacking is now checked as opposed to manipulating the timer to avoid rapid casting

		//	//AIspells[i].time_cancast = (spells[AIspells[i].spellid].recast_time > (spells[AIspells[i].spellid].buffduration * 6000)) ? Timer::GetCurrentTime() + spells[AIspells[i].spellid].recast_time : Timer::GetCurrentTime() + spells[AIspells[i].spellid].buffduration * 6000;
		//	//spellend_timer.Start(spells[AIspells[i].spellid].cast_time);
		//}
		//else
		//	AIspells[i].time_cancast = Timer::GetCurrentTime() + spells[AIspells[i].spellid].recast_time;

		AIspells[i].time_cancast = Timer::GetCurrentTime() + spells[AIspells[i].spellid].recast_time;

		if(spells[AIspells[i].spellid].EndurTimerIndex > 0) {
			SetSpellRecastTimer(spells[AIspells[i].spellid].EndurTimerIndex, spells[AIspells[i].spellid].recast_time);
		}
	}

	return result;
}

bool Bot::AI_PursueCastCheck() {
	bool result = false;

	if (AIautocastspell_timer->Check(false)) {

		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.

		Log(Logs::Detail, Logs::AI, "Bot Engaged (pursuing) autocast check triggered. Trying to cast offensive spells.");

		if(!AICastSpell(GetTarget(), 100, SpellType_Snare)) {
			if(!AICastSpell(GetTarget(), 100, SpellType_Lifetap)) {
				if(!AICastSpell(GetTarget(), 100, SpellType_Nuke)) {
					/*AIautocastspell_timer->Start(RandomTimer(500, 2000), false);
					result = true;*/
					result = true;
				}

				result = true;
			}

			result = true;
		}

		if(!AIautocastspell_timer->Enabled())
			AIautocastspell_timer->Start(RandomTimer(100, 250), false);
	}

	return result;
}

bool Bot::AI_IdleCastCheck() {
	bool result = false;

	if (AIautocastspell_timer->Check(false)) {
#if BotAI_DEBUG_Spells >= 25
		Log(Logs::Detail, Logs::AI, "Bot Non-Engaged autocast check triggered: %s", this->GetCleanName());
#endif
		AIautocastspell_timer->Disable();	//prevent the timer from going off AGAIN while we are casting.

		bool pre_combat = false;
		Mob* test_against = nullptr;

		if (HasGroup() && GetGroup()->GetLeader() && GetGroup()->GetLeader()->IsClient())
			test_against = GetGroup()->GetLeader();
		else if (GetOwner() && GetOwner()->IsClient())
			test_against = GetOwner();

		if (test_against && test_against->GetTarget() && test_against->GetTarget()->IsNPC() && !test_against->GetTarget()->IsPet())
			pre_combat = true;

		//Ok, IdleCastCheck depends of class.
		switch (GetClass()) {
		// Healers WITHOUT pets will check if a heal is needed before buffing.
		case CLERIC:
		case PALADIN:
		case RANGER: {
			if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_Cure)) {
				if (!AICastSpell(this, 100, SpellType_Heal)) {
					if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_Heal)) {
						if (!AICastSpell(this, 100, SpellType_Buff)) {
							if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_Buff)) {
								//
							}
						}
					}
				}
			}

			result = true;
			break;
		}
		// Pets class will first cast their pet, then buffs
		case DRUID:
		case MAGICIAN:
		case SHADOWKNIGHT:
		case SHAMAN:
		case NECROMANCER:
		case ENCHANTER:
		case BEASTLORD: {
			if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_Cure)) {
				if (!AICastSpell(this, 100, SpellType_Pet)) {
					if (!AICastSpell(this, 100, SpellType_Heal)) {
						if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_Heal)) {
							if (!AICastSpell(this, 100, SpellType_Buff)) {
								if (!AICastSpell(GetPet(), 100, SpellType_Heal)) {
									if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_Buff)) {
										//
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
		case WIZARD: { // This can eventually be move into the BEASTLORD case handler once pre-combat is fully implemented
			if (pre_combat) {
				if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_Cure)) {
					if (!AICastSpell(this, 100, SpellType_Pet)) {
						if (!AICastSpell(this, 100, SpellType_Heal)) {
							if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_Heal)) {
								if (!AICastSpell(this, 100, SpellType_Buff)) {
									if (!AICastSpell(GetPet(), 100, SpellType_Heal)) {
										if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_PreCombatBuff)) {
											if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_Buff)) {
												//
											}
										}
									}
								}
							}
						}
					}
				}
			}
			else {
				if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_Cure)) {
					if (!AICastSpell(this, 100, SpellType_Pet)) {
						if (!AICastSpell(this, 100, SpellType_Heal)) {
							if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_Heal)) {
								if (!AICastSpell(this, 100, SpellType_Buff)) {
									if (!AICastSpell(GetPet(), 100, SpellType_Heal)) {
										if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_Buff)) {
											//
										}
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
		case BARD: {
			if (pre_combat) {
				if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_Cure)) {
					if (!AICastSpell(this, 100, SpellType_PreCombatBuffSong)) {
						if (!AICastSpell(this, 100, SpellType_InCombatBuffSong)) {
							//
						}
					}
				}
			}
			else {
				if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, 100, BotAISpellRange, SpellType_Cure)) {
					if (!AICastSpell(this, 100, SpellType_OutOfCombatBuffSong)) {
						//
					}
				}
			}

			result = true;
			break;
		}
		default:
			break;
		}

		if(!AIautocastspell_timer->Enabled())
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
		BotStanceType botStance = GetBotStance();
		bool mayGetAggro = HasOrMayGetAggro();

		Log(Logs::Detail, Logs::AI, "Engaged autocast check triggered (BOTS). Trying to cast healing spells then maybe offensive spells.");

		if(botClass == CLERIC) {
			if(!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Escape), SpellType_Escape)) {
				if(!AICastSpell(this, GetChanceToCastBySpellType(SpellType_Heal), SpellType_Heal)) {
					if(!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_Heal), BotAISpellRange, SpellType_Heal)) {
						if(!AICastSpell(GetTarget(), mayGetAggro?0:GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
							if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_InCombatBuff), SpellType_InCombatBuff)) {
								//AIautocastspell_timer->Start(RandomTimer(100, 250), false);		// Do not give healer classes a lot of time off or your tank's die
								failedToCast = true;
							}
						}
					}
				}
			}
		}
		else if(botClass == DRUID) {
			if(!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Escape), SpellType_Escape)) {
				if(!AICastSpell(this, GetChanceToCastBySpellType(SpellType_Heal), SpellType_Heal)) {
					if(!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_Heal), BotAISpellRange, SpellType_Heal)) {
						if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Debuff), SpellType_Debuff)) {
							if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_DOT), SpellType_DOT)) {
								if(!AICastSpell(GetTarget(), mayGetAggro?0:GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
									//AIautocastspell_timer->Start(RandomTimer(100, 250), false);		// Do not give healer classes a lot of time off or your tank's die
									failedToCast = true;
								}
							}
						}
					}
				}
			}
		}
		else if(botClass == SHAMAN) {
			if(!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Escape), SpellType_Escape)) {
				if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Slow), SpellType_Slow)) {
					if(!AICastSpell(this, GetChanceToCastBySpellType(SpellType_Heal), SpellType_Heal)) {
						if(!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_Heal), BotAISpellRange, SpellType_Heal)) {
							if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Debuff), SpellType_Debuff)) {
								if (!AICastSpell(GetPet(), GetChanceToCastBySpellType(SpellType_Heal), SpellType_Heal)) {
									if(!AICastSpell(this, GetChanceToCastBySpellType(SpellType_InCombatBuff), SpellType_InCombatBuff)) {
										if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_DOT), SpellType_DOT)) {
											if(!AICastSpell(GetTarget(), mayGetAggro?0:GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
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
			}
		}
		else if(botClass == RANGER) {
			if(!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Escape), SpellType_Escape)) {
				if (!AICastSpell(this, GetChanceToCastBySpellType(SpellType_Heal), SpellType_Heal)) {
					if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_Heal), BotAISpellRange, SpellType_Heal)) {
						if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_DOT), SpellType_DOT)) {
							if (!AICastSpell(GetTarget(), mayGetAggro?0:GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
								//
								failedToCast = true;
							}
						}
					}
				}
			}
		}
		else if(botClass == BEASTLORD) {
			if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Slow), SpellType_Slow)) {
				if (!AICastSpell(this, GetChanceToCastBySpellType(SpellType_Heal), SpellType_Heal)) {
					if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_Heal), BotAISpellRange, SpellType_Heal)) {
						if (!AICastSpell(GetPet(), GetChanceToCastBySpellType(SpellType_Heal), SpellType_Heal)) {
							if (!AICastSpell(this, GetChanceToCastBySpellType(SpellType_Pet), SpellType_Pet)) {
								if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_DOT), SpellType_DOT)) {
									if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Debuff), SpellType_Debuff)) {
										if(!AICastSpell(GetTarget(), mayGetAggro?0:GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
											//
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
		else if(botClass == WIZARD) {
			if(!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Escape), SpellType_Escape)) {
				if (!AICastSpell(GetTarget(), mayGetAggro?0:GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
					//
					failedToCast = true;
				}
			}
		}
		else if(botClass == PALADIN) {
			if(!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Escape), SpellType_Escape)) {
				if (!AICastSpell(this, GetChanceToCastBySpellType(SpellType_Heal), SpellType_Heal)) {
					if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_Heal), BotAISpellRange, SpellType_Heal)) {
						if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
							if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_InCombatBuff), SpellType_InCombatBuff)) {
								//
								failedToCast = true;
							}
						}
					}
				}
			}
		}
		else if(botClass == SHADOWKNIGHT) {
			if(!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Escape), SpellType_Escape)) {
				if (!AICastSpell(this, GetChanceToCastBySpellType(SpellType_Pet), SpellType_Pet)) {
					if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Lifetap), SpellType_Lifetap)) {
						if (!AICastSpell(GetPet(), GetChanceToCastBySpellType(SpellType_Heal), SpellType_Heal)) {
							if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_DOT), SpellType_DOT)) {
								if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Debuff), SpellType_Debuff)) {
									if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
										//
										failedToCast = true;
									}
								}
							}
						}
					}
				}
			}
		}
		else if(botClass == MAGICIAN) {
			if (!AICastSpell(this, GetChanceToCastBySpellType(SpellType_Pet), SpellType_Pet)) {
				if (!AICastSpell(GetPet(), GetChanceToCastBySpellType(SpellType_Heal), SpellType_Heal)) {
					if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Debuff), SpellType_Debuff)) {
						if (!AICastSpell(GetTarget(), mayGetAggro?0:GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
							//
							failedToCast = true;
						}
					}
				}
			}
		}
		else if(botClass == NECROMANCER) {
			if(!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Escape), SpellType_Escape)) {
				if (!AICastSpell(this, GetChanceToCastBySpellType(SpellType_Pet), SpellType_Pet)) {
					if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Lifetap), SpellType_Lifetap)) {
						if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_DOT), SpellType_DOT)) {
							if (!AICastSpell(GetPet(), GetChanceToCastBySpellType(SpellType_Heal), SpellType_Heal)) {
								if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Debuff), SpellType_Debuff)) {
									if (!AICastSpell(GetTarget(), mayGetAggro?0:GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
										//
										failedToCast = true;
									}
								}
							}
						}
					}
				}
			}
		}
		else if(botClass == ENCHANTER) {
			if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Mez), SpellType_Mez)) {
				if(!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Escape), SpellType_Escape)) {
					if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Slow), SpellType_Slow)) {
						if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Debuff), SpellType_Debuff)) {
							if (!AICastSpell(GetTarget(), mayGetAggro?0:GetChanceToCastBySpellType(SpellType_DOT), SpellType_DOT)) {
								if (!AICastSpell(GetTarget(), mayGetAggro?0:GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {
									//
									failedToCast = true;
								}
							}
						}
					}
				}
			}
		}
		else if(botClass == BARD) {
			if (!entity_list.Bot_AICheckCloseBeneficialSpells(this, GetChanceToCastBySpellType(SpellType_HateRedux), BotAISpellRange, SpellType_HateRedux)) {
				if (!AICastSpell(this, GetChanceToCastBySpellType(SpellType_InCombatBuffSong), SpellType_InCombatBuffSong)) {
					if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Slow), SpellType_Slow)) {
						if (!AICastSpell(GetTarget(), mayGetAggro ? 0 : GetChanceToCastBySpellType(SpellType_DOT), SpellType_DOT)) {// Bards will use their dot songs
							if (!AICastSpell(GetTarget(), mayGetAggro ? 0 : GetChanceToCastBySpellType(SpellType_Nuke), SpellType_Nuke)) {// Bards will use their nuke songs
								if (!AICastSpell(GetTarget(), GetChanceToCastBySpellType(SpellType_Escape), SpellType_Escape)) {// Bards will use their escape songs
									//
									failedToCast = true;
								}
							}
						}
					}
				}
			}
		}

		if(!AIautocastspell_timer->Enabled()) {
			AIautocastspell_timer->Start(RandomTimer(150, 300), false);
		}

		if(!failedToCast)
			result = true;
	}

	return result;
}

bool Bot::AIHealRotation(Mob* tar, bool useFastHeals) {

	if (!tar) {
		return false;
	}

	if (!AI_HasSpells())
		return false;

	if(tar->GetAppearance() == eaDead) {
		if((tar->IsClient() && tar->CastToClient()->GetFeigned()) || tar->IsBot()) {
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

		if(botSpell.SpellId == 0)
			botSpell = GetBestBotSpellForFastHeal(this);
	}
	else {
		botSpell = GetBestBotSpellForPercentageHeal(this);

		if(botSpell.SpellId == 0)
			botSpell = GetBestBotSpellForRegularSingleTargetHeal(this);

		if(botSpell.SpellId == 0)
			botSpell = GetFirstBotSpellForSingleTargetHeal(this);

		if(botSpell.SpellId == 0){
			botSpell = GetFirstBotSpellBySpellType(this, SpellType_Heal);
		}
	}

#if BotAI_DEBUG_Spells >= 10
	Log(Logs::Detail, Logs::AI, "Bot::AIHealRotation: heal spellid = %u, fastheals = %c, casterlevel = %u",
		botSpell.SpellId, ((useFastHeals) ? ('T') : ('F')), GetLevel());
#endif
#if BotAI_DEBUG_Spells >= 25
	Log(Logs::Detail, Logs::AI, "Bot::AIHealRotation: target = %s, current_time = %u, donthealmebefore = %u", tar->GetCleanName(), Timer::GetCurrentTime(), tar->DontHealMeBefore());
#endif

	// If there is still no spell id, then there isn't going to be one so we are done
	if (botSpell.SpellId == 0)
		return false;

	// Can we cast this spell on this target?
	if (!(spells[botSpell.SpellId].targettype == ST_GroupTeleport || spells[botSpell.SpellId].targettype == ST_Target || tar == this)
		&& !(tar->CanBuffStack(botSpell.SpellId, botLevel, true) >= 0))
		return false;

	uint32 TempDontHealMeBeforeTime = tar->DontHealMeBefore();

	castedSpell = AIDoSpellCast(botSpell.SpellIndex, tar, botSpell.ManaCost, &TempDontHealMeBeforeTime);

	if(castedSpell)
		Say("Casting %s on %s, please stay in range!", spells[botSpell.SpellId].name, tar->GetCleanName());

	return castedSpell;
}

std::list<BotSpell> Bot::GetBotSpellsForSpellEffect(Bot* botCaster, int spellEffect) {
	std::list<BotSpell> result;

	if(botCaster && botCaster->AI_HasSpells()) {
		std::vector<AISpells_Struct> botSpellList = botCaster->GetBotSpells();

		for (int i = botSpellList.size() - 1; i >= 0; i--) {
			if (botSpellList[i].spellid <= 0 || botSpellList[i].spellid >= SPDAT_RECORDS) {
				// this is both to quit early to save cpu and to avoid casting bad spells
				// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
				continue;
			}

			if(IsEffectInSpell(botSpellList[i].spellid, spellEffect)) {
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

	if(botCaster && botCaster->AI_HasSpells()) {
		std::vector<AISpells_Struct> botSpellList = botCaster->GetBotSpells();

		for (int i = botSpellList.size() - 1; i >= 0; i--) {
			if (botSpellList[i].spellid <= 0 || botSpellList[i].spellid >= SPDAT_RECORDS) {
				// this is both to quit early to save cpu and to avoid casting bad spells
				// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
				continue;
			}

			if(IsEffectInSpell(botSpellList[i].spellid, spellEffect)) {
				if(spells[botSpellList[i].spellid].targettype == targetType) {
					BotSpell botSpell;
					botSpell.SpellId = botSpellList[i].spellid;
					botSpell.SpellIndex = i;
					botSpell.ManaCost = botSpellList[i].manacost;

					result.push_back(botSpell);
				}
			}
		}
	}

	return result;
}

std::list<BotSpell> Bot::GetBotSpellsBySpellType(Bot* botCaster, uint32 spellType) {
	std::list<BotSpell> result;

	if(botCaster && botCaster->AI_HasSpells()) {
		std::vector<AISpells_Struct> botSpellList = botCaster->GetBotSpells();

		for (int i = botSpellList.size() - 1; i >= 0; i--) {
			if (botSpellList[i].spellid <= 0 || botSpellList[i].spellid >= SPDAT_RECORDS) {
				// this is both to quit early to save cpu and to avoid casting bad spells
				// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
				continue;
			}

			if(botSpellList[i].type & spellType) {
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
		std::vector<AISpells_Struct> botSpellList = botCaster->GetBotSpells();

		for (int i = botSpellList.size() - 1; i >= 0; i--) {
			if (botSpellList[i].spellid <= 0 || botSpellList[i].spellid >= SPDAT_RECORDS) {
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

		if (result.size() > 1)
			result.sort([](BotSpell_wPriority& l, BotSpell_wPriority& r) { return l.Priority < r.Priority; });
	}

	return result;
}

BotSpell Bot::GetFirstBotSpellBySpellType(Bot* botCaster, uint32 spellType) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if(botCaster && botCaster->AI_HasSpells()) {
		std::vector<AISpells_Struct> botSpellList = botCaster->GetBotSpells();

		for (int i = botSpellList.size() - 1; i >= 0; i--) {
			if (botSpellList[i].spellid <= 0 || botSpellList[i].spellid >= SPDAT_RECORDS) {
				// this is both to quit early to save cpu and to avoid casting bad spells
				// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
				continue;
			}

			if((botSpellList[i].type & spellType) && CheckSpellRecastTimers(botCaster, i)) {
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

	if(botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, SE_CurrentHP);

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsFastHealSpell(botSpellListItr->SpellId) && CheckSpellRecastTimers(botCaster, botSpellListItr->SpellIndex)) {
				result.SpellId = botSpellListItr->SpellId;
				result.SpellIndex = botSpellListItr->SpellIndex;
				result.ManaCost = botSpellListItr->ManaCost;

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

	if(botCaster) {
		std::list<BotSpell> botHoTSpellList = GetBotSpellsForSpellEffect(botCaster, SE_HealOverTime);
		std::vector<AISpells_Struct> botSpellList = botCaster->GetBotSpells();

		for(std::list<BotSpell>::iterator botSpellListItr = botHoTSpellList.begin(); botSpellListItr != botHoTSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsHealOverTimeSpell(botSpellListItr->SpellId)) {

				for (int i = botSpellList.size() - 1; i >= 0; i--) {
					if (botSpellList[i].spellid <= 0 || botSpellList[i].spellid >= SPDAT_RECORDS) {
						// this is both to quit early to save cpu and to avoid casting bad spells
						// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
						continue;
					}

					if(botSpellList[i].spellid == botSpellListItr->SpellId && (botSpellList[i].type & SpellType_Heal) && CheckSpellRecastTimers(botCaster, botSpellListItr->SpellIndex)) {
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

BotSpell Bot::GetBestBotSpellForPercentageHeal(Bot *botCaster) {
	BotSpell result;

	result.SpellId = 0;
	result.SpellIndex = 0;
	result.ManaCost = 0;

	if(botCaster && botCaster->AI_HasSpells()) {
		std::vector<AISpells_Struct> botSpellList = botCaster->GetBotSpells();

		for (int i = botSpellList.size() - 1; i >= 0; i--) {
			if (botSpellList[i].spellid <= 0 || botSpellList[i].spellid >= SPDAT_RECORDS) {
				// this is both to quit early to save cpu and to avoid casting bad spells
				// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
				continue;
			}

			if(IsCompleteHealSpell(botSpellList[i].spellid) && CheckSpellRecastTimers(botCaster, i)) {
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

	if(botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, SE_CurrentHP);

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsRegularSingleTargetHealSpell(botSpellListItr->SpellId) && CheckSpellRecastTimers(botCaster, botSpellListItr->SpellIndex)) {
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

	if(botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, SE_CurrentHP);

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if((IsRegularSingleTargetHealSpell(botSpellListItr->SpellId) || IsFastHealSpell(botSpellListItr->SpellId)) && CheckSpellRecastTimers(botCaster, botSpellListItr->SpellIndex)) {
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

	if(botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, SE_CurrentHP);

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsRegularGroupHealSpell(botSpellListItr->SpellId) && CheckSpellRecastTimers(botCaster, botSpellListItr->SpellIndex)) {
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

	if(botCaster) {
		std::list<BotSpell> botHoTSpellList = GetBotSpellsForSpellEffect(botCaster, SE_HealOverTime);
		std::vector<AISpells_Struct> botSpellList = botCaster->GetBotSpells();

		for(std::list<BotSpell>::iterator botSpellListItr = botHoTSpellList.begin(); botSpellListItr != botHoTSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsGroupHealOverTimeSpell(botSpellListItr->SpellId)) {

				for (int i = botSpellList.size() - 1; i >= 0; i--) {
					if (botSpellList[i].spellid <= 0 || botSpellList[i].spellid >= SPDAT_RECORDS) {
						// this is both to quit early to save cpu and to avoid casting bad spells
						// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
						continue;
					}

					if(botSpellList[i].spellid == botSpellListItr->SpellId && (botSpellList[i].type & SpellType_Heal) && CheckSpellRecastTimers(botCaster, botSpellListItr->SpellIndex)) {
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

	if(botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, SE_CompleteHeal);

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsGroupCompleteHealSpell(botSpellListItr->SpellId) && CheckSpellRecastTimers(botCaster, botSpellListItr->SpellIndex)) {
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

	if(botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, SE_Mez);

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsMezSpell(botSpellListItr->SpellId) && CheckSpellRecastTimers(botCaster, botSpellListItr->SpellIndex)) {
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

	if(botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, SE_AttackSpeed);

		for (std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if (IsSlowSpell(botSpellListItr->SpellId) && spells[botSpellListItr->SpellId].resisttype == RESIST_MAGIC && CheckSpellRecastTimers(botCaster, botSpellListItr->SpellIndex)) {
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

	if(botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, SE_AttackSpeed);

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsSlowSpell(botSpellListItr->SpellId) && spells[botSpellListItr->SpellId].resisttype == RESIST_DISEASE && CheckSpellRecastTimers(botCaster, botSpellListItr->SpellIndex)) {
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

	if(botCaster && IsMezSpell(botSpell.SpellId)) {

		std::list<NPC*> npc_list;
		entity_list.GetNPCList(npc_list);

		for(std::list<NPC*>::iterator itr = npc_list.begin(); itr != npc_list.end(); ++itr) {
			NPC* npc = *itr;

			if(DistanceSquaredNoZ(npc->GetPosition(), botCaster->GetPosition()) <= botCaster->GetActSpellRange(botSpell.SpellId, spells[botSpell.SpellId].range)) {
				if(!npc->IsMezzed()) {
					if(botCaster->HasGroup()) {
						Group* g = botCaster->GetGroup();

						if(g) {
							for(int counter = 0; counter < g->GroupCount(); counter++) {
								if(npc->IsOnHatelist(g->members[counter]) && g->members[counter]->GetTarget() != npc && g->members[counter]->IsEngaged()) {
									result = npc;
									break;
								}
							}
						}
					}
				}
			}

			if(result)
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

	if(botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffect(botCaster, SE_SummonPet);

		std::string petType = GetBotMagicianPetType(botCaster);

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsSummonPetSpell(botSpellListItr->SpellId) && CheckSpellRecastTimers(botCaster, botSpellListItr->SpellIndex)) {
				if(!strncmp(spells[botSpellListItr->SpellId].teleport_zone, petType.c_str(), petType.length())) {
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

	if(botCaster) {
		if(botCaster->IsPetChooser()) {
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
			if(botCaster->GetLevel() == 2)
				result = std::string("SumWater");
			else if(botCaster->GetLevel() == 3)
				result = std::string("SumFire");
			else if(botCaster->GetLevel() == 4)
				result = std::string("SumAir");
			else if(botCaster->GetLevel() == 5)
				result = std::string("SumEarth");
			else if(botCaster->GetLevel() < 30) {
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

	if(botCaster) {
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffectAndTargetType(botCaster, SE_CurrentHP, targetType);

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsPureNukeSpell(botSpellListItr->SpellId) && IsDamageSpell(botSpellListItr->SpellId) && CheckSpellRecastTimers(botCaster, botSpellListItr->SpellIndex)) {
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

	if(botCaster)
	{
		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffectAndTargetType(botCaster, SE_Stun, targetType);

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr)
		{
			// Assuming all the spells have been loaded into this list by level and in descending order
			if(IsStunSpell(botSpellListItr->SpellId) && CheckSpellRecastTimers(botCaster, botSpellListItr->SpellIndex))
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

	if(botCaster && target) {
		const int lureResisValue = -100;
		const int maxTargetResistValue = 300;
		bool selectLureNuke = false;

		if((target->GetMR() > maxTargetResistValue) && (target->GetCR() > maxTargetResistValue) && (target->GetFR() > maxTargetResistValue))
			selectLureNuke = true;


		std::list<BotSpell> botSpellList = GetBotSpellsForSpellEffectAndTargetType(botCaster, SE_CurrentHP, ST_Target);

		BotSpell firstWizardMagicNukeSpellFound;
		firstWizardMagicNukeSpellFound.SpellId = 0;
		firstWizardMagicNukeSpellFound.SpellIndex = 0;
		firstWizardMagicNukeSpellFound.ManaCost = 0;

		for(std::list<BotSpell>::iterator botSpellListItr = botSpellList.begin(); botSpellListItr != botSpellList.end(); ++botSpellListItr) {
			// Assuming all the spells have been loaded into this list by level and in descending order
			bool spellSelected = false;

			if(CheckSpellRecastTimers(botCaster, botSpellListItr->SpellIndex)) {
				if(selectLureNuke && (spells[botSpellListItr->SpellId].ResistDiff < lureResisValue)) {
					spellSelected = true;
				}
				else if(IsPureNukeSpell(botSpellListItr->SpellId)) {
					if(((target->GetMR() < target->GetCR()) || (target->GetMR() < target->GetFR())) && (GetSpellResistType(botSpellListItr->SpellId) == RESIST_MAGIC)
						&& (spells[botSpellListItr->SpellId].ResistDiff > lureResisValue))
					{
						spellSelected = true;
					}
					else if(((target->GetCR() < target->GetMR()) || (target->GetCR() < target->GetFR())) && (GetSpellResistType(botSpellListItr->SpellId) == RESIST_COLD)
						&& (spells[botSpellListItr->SpellId].ResistDiff > lureResisValue))
					{
						spellSelected = true;
					}
					else if(((target->GetFR() < target->GetCR()) || (target->GetFR() < target->GetMR())) && (GetSpellResistType(botSpellListItr->SpellId) == RESIST_FIRE)
						&& (spells[botSpellListItr->SpellId].ResistDiff > lureResisValue))
					{
						spellSelected = true;
					}
					else if((GetSpellResistType(botSpellListItr->SpellId) == RESIST_MAGIC) && (spells[botSpellListItr->SpellId].ResistDiff > lureResisValue) && !IsStunSpell(botSpellListItr->SpellId)) {
						firstWizardMagicNukeSpellFound.SpellId = botSpellListItr->SpellId;
						firstWizardMagicNukeSpellFound.SpellIndex = botSpellListItr->SpellIndex;
						firstWizardMagicNukeSpellFound.ManaCost = botSpellListItr->ManaCost;
					}
				}
			}

			if(spellSelected) {
				result.SpellId = botSpellListItr->SpellId;
				result.SpellIndex = botSpellListItr->SpellIndex;
				result.ManaCost = botSpellListItr->ManaCost;

				break;
			}
		}

		if(result.SpellId == 0) {
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

	if(!tar || !botCaster)
		return result;

	if(botCaster && botCaster->AI_HasSpells()) {
		std::vector<AISpells_Struct> botSpellList = botCaster->GetBotSpells();

		for (int i = botSpellList.size() - 1; i >= 0; i--) {
			if (botSpellList[i].spellid <= 0 || botSpellList[i].spellid >= SPDAT_RECORDS) {
				// this is both to quit early to save cpu and to avoid casting bad spells
				// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
				continue;
			}

			if(((botSpellList[i].type & SpellType_Debuff) || IsDebuffSpell(botSpellList[i].spellid))
				&& (!tar->IsImmuneToSpell(botSpellList[i].spellid, botCaster)
				&& tar->CanBuffStack(botSpellList[i].spellid, botCaster->GetLevel(), true) >= 0)
				&& CheckSpellRecastTimers(botCaster, i)) {
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

	if(!tar)
		return result;

	int level_mod = (tar->GetLevel() - botCaster->GetLevel())* (tar->GetLevel() - botCaster->GetLevel()) / 2;
	if(tar->GetLevel() - botCaster->GetLevel() < 0)
	{
		level_mod = -level_mod;
	}
	bool needsMagicResistDebuff = (tar->GetMR() + level_mod) > 100 ? true: false;
	bool needsColdResistDebuff = (tar->GetCR() + level_mod) > 100 ? true: false;
	bool needsFireResistDebuff = (tar->GetFR() + level_mod) > 100 ? true: false;
	bool needsPoisonResistDebuff = (tar->GetPR() + level_mod) > 100 ? true: false;
	bool needsDiseaseResistDebuff = (tar->GetDR() + level_mod) > 100 ? true: false;

	if(botCaster && botCaster->AI_HasSpells()) {
		std::vector<AISpells_Struct> botSpellList = botCaster->GetBotSpells();

		for (int i = botSpellList.size() - 1; i >= 0; i--) {
			if (botSpellList[i].spellid <= 0 || botSpellList[i].spellid >= SPDAT_RECORDS) {
				// this is both to quit early to save cpu and to avoid casting bad spells
				// Bad info from database can trigger this incorrectly, but that should be fixed in DB, not here
				continue;
			}

			if(((botSpellList[i].type & SpellType_Debuff) || IsResistDebuffSpell(botSpellList[i].spellid))
				&& ((needsMagicResistDebuff && (IsEffectInSpell(botSpellList[i].spellid, SE_ResistMagic)) || IsEffectInSpell(botSpellList[i].spellid, SE_ResistAll))
				|| (needsColdResistDebuff && (IsEffectInSpell(botSpellList[i].spellid, SE_ResistCold)) || IsEffectInSpell(botSpellList[i].spellid, SE_ResistAll))
				|| (needsFireResistDebuff && (IsEffectInSpell(botSpellList[i].spellid, SE_ResistFire)) || IsEffectInSpell(botSpellList[i].spellid, SE_ResistAll))
				|| (needsPoisonResistDebuff && (IsEffectInSpell(botSpellList[i].spellid, SE_ResistPoison)) || IsEffectInSpell(botSpellList[i].spellid, SE_ResistAll))
				|| (needsDiseaseResistDebuff && (IsEffectInSpell(botSpellList[i].spellid, SE_ResistDisease)) || IsEffectInSpell(botSpellList[i].spellid, SE_ResistAll)))
				&& (!tar->IsImmuneToSpell(botSpellList[i].spellid, botCaster)
				&& tar->CanBuffStack(botSpellList[i].spellid, botCaster->GetLevel(), true) >= 0)
				&& CheckSpellRecastTimers(botCaster, i)) {
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

	if(!tar)
		return result;

	int countNeedsCured = 0;
	bool isPoisoned = tar->FindType(SE_PoisonCounter);
	bool isDiseased = tar->FindType(SE_DiseaseCounter);
	bool isCursed = tar->FindType(SE_CurseCounter);
	bool isCorrupted = tar->FindType(SE_CorruptionCounter);

	if(botCaster && botCaster->AI_HasSpells()) {
		std::list<BotSpell_wPriority> cureList = GetPrioritizedBotSpellsBySpellType(botCaster, SpellType_Cure);

		if(tar->HasGroup()) {
			Group *g = tar->GetGroup();

			if(g) {
				for( int i = 0; i<MAX_GROUP_MEMBERS; i++) {
					if(g->members[i] && !g->members[i]->qglobal) {
						if(botCaster->GetNeedsCured(g->members[i]))
							countNeedsCured++;
					}
				}
			}
		}

		//Check for group cure first
		if(countNeedsCured > 2) {
			for (std::list<BotSpell_wPriority>::iterator itr = cureList.begin(); itr != cureList.end(); ++itr) {
				BotSpell selectedBotSpell = *itr;

				if(IsGroupSpell(itr->SpellId) && CheckSpellRecastTimers(botCaster, itr->SpellIndex)) {
					if(selectedBotSpell.SpellId == 0)
						continue;

					if(isPoisoned && IsEffectInSpell(itr->SpellId, SE_PoisonCounter)) {
						spellSelected = true;
					}
					else if(isDiseased && IsEffectInSpell(itr->SpellId, SE_DiseaseCounter)) {
						spellSelected = true;
					}
					else if(isCursed && IsEffectInSpell(itr->SpellId, SE_CurseCounter)) {
						spellSelected = true;
					}
					else if(isCorrupted && IsEffectInSpell(itr->SpellId, SE_CorruptionCounter)) {
						spellSelected = true;
					}
					else if(IsEffectInSpell(itr->SpellId, SE_DispelDetrimental)) {
						spellSelected = true;
					}

					if(spellSelected)
					{
						result.SpellId = itr->SpellId;
						result.SpellIndex = itr->SpellIndex;
						result.ManaCost = itr->ManaCost;

						break;
					}
				}
			}
		}

		//no group cure for target- try to find single target spell
		if(!spellSelected) {
			for(std::list<BotSpell_wPriority>::iterator itr = cureList.begin(); itr != cureList.end(); ++itr) {
				BotSpell selectedBotSpell = *itr;

				if(CheckSpellRecastTimers(botCaster, itr->SpellIndex)) {
					if(selectedBotSpell.SpellId == 0)
						continue;

					if(isPoisoned && IsEffectInSpell(itr->SpellId, SE_PoisonCounter)) {
						spellSelected = true;
					}
					else if(isDiseased && IsEffectInSpell(itr->SpellId, SE_DiseaseCounter)) {
						spellSelected = true;
					}
					else if(isCursed && IsEffectInSpell(itr->SpellId, SE_CurseCounter)) {
						spellSelected = true;
					}
					else if(isCorrupted && IsEffectInSpell(itr->SpellId, SE_CorruptionCounter)) {
						spellSelected = true;
					}
					else if(IsEffectInSpell(itr->SpellId, SE_DispelDetrimental)) {
						spellSelected = true;
					}

					if(spellSelected)
					{
						result.SpellId = itr->SpellId;
						result.SpellIndex = itr->SpellIndex;
						result.ManaCost = itr->ManaCost;

						break;
					}
				}
			}
		}
	}

	return result;
}

void Bot::SetSpellRecastTimer(int timer_index, int32 recast_delay) {
	if(timer_index > 0 && timer_index <= MaxSpellTimer) {
		timers[timer_index - 1] = Timer::GetCurrentTime() + recast_delay;
	}
}

int32 Bot::GetSpellRecastTimer(Bot *caster, int timer_index) {
	int32 result = 0;
	if(caster) {
		if(timer_index > 0 && timer_index <= MaxSpellTimer) {
			result = caster->timers[timer_index - 1];
		}
	}
	return result;
}

bool Bot::CheckSpellRecastTimers(Bot *caster, int SpellIndex) {
	if(caster) {
		if(caster->AIspells[SpellIndex].time_cancast < Timer::GetCurrentTime()) { //checks spell recast
			if(GetSpellRecastTimer(caster, spells[caster->AIspells[SpellIndex].spellid].EndurTimerIndex) < Timer::GetCurrentTime()) { //checks for spells on the same timer
				return true; //can cast spell
			}
		}
	}
	return false;
}

void Bot::SetDisciplineRecastTimer(int timer_index, int32 recast_delay) {
	if(timer_index > 0 && timer_index <= MaxDisciplineTimer) {
		timers[DisciplineReuseStart + timer_index - 1] = Timer::GetCurrentTime() + recast_delay;
	}
}

int32 Bot::GetDisciplineRecastTimer(Bot *caster, int timer_index) {
	int32 result = 0;
	if(caster) {
		if(timer_index > 0 && timer_index <= MaxDisciplineTimer) {
			result = caster->timers[DisciplineReuseStart + timer_index - 1];
		}
	}
	return result;
}

uint32 Bot::GetDisciplineRemainingTime(Bot *caster, int timer_index) {
	int32 result = 0;
	if(caster) {
		if(timer_index > 0 && timer_index <= MaxDisciplineTimer) {
			if(GetDisciplineRecastTimer(caster, timer_index) > Timer::GetCurrentTime())
				result = GetDisciplineRecastTimer(caster, timer_index) - Timer::GetCurrentTime();
		}
	}
	return result;
}

bool Bot::CheckDisciplineRecastTimers(Bot *caster, int timer_index) {
	if(caster) {
		if(GetDisciplineRecastTimer(caster, timer_index) < Timer::GetCurrentTime()) { //checks for spells on the same timer
			return true; //can cast spell
		}
	}
	return false;
}

uint8 Bot::GetChanceToCastBySpellType(uint32 spellType)
{
	uint8 spell_type_index = MaxSpellTypes;
	switch (spellType) {
	case SpellType_Nuke:
		spell_type_index = SpellType_NukeIndex;
		break;
	case SpellType_Heal:
		spell_type_index = SpellType_HealIndex;
		break;
	case SpellType_Root:
		spell_type_index = SpellType_RootIndex;
		break;
	case SpellType_Buff:
		spell_type_index = SpellType_BuffIndex;
		break;
	case SpellType_Escape:
		spell_type_index = SpellType_EscapeIndex;
		break;
	case SpellType_Pet:
		spell_type_index = SpellType_PetIndex;
		break;
	case SpellType_Lifetap:
		spell_type_index = SpellType_LifetapIndex;
		break;
	case SpellType_Snare:
		spell_type_index = SpellType_SnareIndex;
		break;
	case SpellType_DOT:
		spell_type_index = SpellType_DOTIndex;
		break;
	case SpellType_Dispel:
		spell_type_index = SpellType_DispelIndex;
		break;
	case SpellType_InCombatBuff:
		spell_type_index = SpellType_InCombatBuffIndex;
		break;
	case SpellType_Mez:
		spell_type_index = SpellType_MezIndex;
		break;
	case SpellType_Charm:
		spell_type_index = SpellType_CharmIndex;
		break;
	case SpellType_Slow:
		spell_type_index = SpellType_SlowIndex;
		break;
	case SpellType_Debuff:
		spell_type_index = SpellType_DebuffIndex;
		break;
	case SpellType_Cure:
		spell_type_index = SpellType_CureIndex;
		break;
	case SpellType_Resurrect:
		spell_type_index = SpellType_ResurrectIndex;
		break;
	case SpellType_HateRedux:
		spell_type_index = SpellType_HateReduxIndex;
		break;
	case SpellType_InCombatBuffSong:
		spell_type_index = SpellType_InCombatBuffSongIndex;
		break;
	case SpellType_OutOfCombatBuffSong:
		spell_type_index = SpellType_OutOfCombatBuffSongIndex;
		break;
	case SpellType_PreCombatBuff:
		spell_type_index = SpellType_PreCombatBuffIndex;
		break;
	case SpellType_PreCombatBuffSong:
		spell_type_index = SpellType_PreCombatBuffSongIndex;
		break;
	default:
		spell_type_index = MaxSpellTypes;
		break;
	}
	if (spell_type_index >= MaxSpellTypes)
		return 0;

	uint8 class_index = GetClass();
	if (class_index > BERSERKER || class_index < WARRIOR)
		return 0;
	--class_index;

	uint8 stance_index = (uint8)GetBotStance();
	if (stance_index >= MaxStances)
		return 0;

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

	return botdb.GetSpellCastingChance(spell_type_index, class_index, stance_index, type_index);
}

#endif
