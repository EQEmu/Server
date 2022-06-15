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
#include "zone_store.h"
#include "npc.h"
#include "mob.h"
#include "client.h"
#include "worldserver.h"
#include "quest_parser_collection.h"
#include "event_codes.h"
#include "embparser.h"
#include <string>
#include <iostream>

extern EntityList entity_list;
extern Zone* zone;

extern WorldServer worldserver;

//All functions that modify a value are passed the value as it was computed by default formulas and bonuses.  In most cases this should be the final value that will be used.

//These are called when a zone boots or is repopped
void Zone::mod_init() { return; }
void Zone::mod_repop() { return; }

//Pre-spawn hook called from the NPC object to be spawned
void NPC::mod_prespawn(Spawn2 *sp) { return; }

//Base damage from NPC::Attack
int NPC::mod_npc_damage(int64 damage, SkillType skillinuse, int hand, const Item_Struct* weapon, Mob* other) { return(damage); }

//Mob c has been given credit for a kill.  This is called after the regular EVENT_KILLED_MERIT event.
void NPC::mod_npc_killed_merit(Mob* c) { return; }

//Mob oos has been given credit for a kill.  This is called after the regular EVENT_DEATH event.
void NPC::mod_npc_killed(Mob* oos) { return; }

//Base damage from Client::Attack - can cover myriad skill types
int Client::mod_client_damage(int64 damage, SkillType skillinuse, int hand, const ItemInst* weapon, Mob* other) { return(damage); }

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
bool Client::mod_client_message(char* message, uint8 chan_num) { return(true); }

//Skillup override.  When this is called the regular skillup check has failed.  Return false to proceed with default behavior.
//This will NOT allow a client to increase skill past a cap.
bool Client::mod_can_increase_skill(SkillType skillid, Mob* against_who) { return(false); }

//chance of general skill increase, rolled against 0-99 where higher chance is better.
int16 Client::mod_increase_skill_chance(int16 chance, Mob* against_who) { return(chance); }

//Max percent of health you can bind wound starting with default value for class, item, and AA bonuses
int Client::mod_bindwound_percent(int max_percent, Mob* bindmob) { return(max_percent); }

//Final bind HP value after bonuses
int Client::mod_bindwound_hp(int bindhps, Mob* bindmob) { return(bindhps); }

//Client haste as calculated by default formulas - In percent from 0-100
int Client::mod_client_haste(int h) { return(h); }

//Haste cap override
int Client::mod_client_haste_cap(int cap) { return(cap); }

//This is called when a client cons a mob
void Client::mod_consider(Mob* tmob, Consider_Struct* con) { return; }

//Return true to continue with normal behavior, false returns in the parent function
bool Client::mod_saylink(const std::string& response, bool silentsaylink) { return(true); }

//Client pet power as calculated by default formulas and bonuses
int16 Client::mod_pet_power(int16 act_power, uint16 spell_id) { return(act_power); }

//Chance to combine rolled against a random 0-99 where higher is better.
float Client::mod_tradeskill_chance(float chance, DBTradeskillRecipe_Struct *spec) { return(chance); }

//Chance to skillup rolled against a random 0-99 where higher is better.
float Client::mod_tradeskill_skillup(float chance_stage2) { return(chance_stage2); }

//Tribute value override
int32 Client::mod_tribute_item_value(int32 pts, const ItemInst* item) { return(pts); }

//Death reporting
void Client::mod_client_death_npc(Mob* killerMob) { return; }
void Client::mod_client_death_duel(Mob* killerMob) { return; }
void Client::mod_client_death_env() { return; }

//Calculated xp before consider modifier, called whenever a client gets XP for killing a mob.
int32 Client::mod_client_xp(int32 in_xp, NPC *npc) { return(in_xp); }

//Client XP formula.  Changes here will cause clients to change level after gaining or losing xp.
//Either modify this before your server goes live, or be prepared to write a quest script that fixes levels.
//To adjust how much XP is given per kill, use mod_client_xp
uint32 Client::mod_client_xp_for_level(uint32 xp, uint16 check_level) { return(xp); }

//effect_vallue - Spell effect value as calculated by default formulas.  You will want to ignore effects that don't lend themselves to scaling - pet ID's, gate coords, etc.
int Mob::mod_effect_value(int effect_value, uint16 spell_id, int effect_type, Mob* caster) { return(effect_value); }

//chancetohit - 0 to 100 percent - set over 1000 for a guaranteed hit
float Mob::mod_hit_chance(float chancetohit, SkillType skillinuse, Mob* attacker) { return(chancetohit); }

//Final riposte chance
float Mob::mod_riposte_chance(float ripostechance, Mob* attacker) { return(ripostechance); }

//Final block chance
float Mob::mod_block_chance(float blockchance, Mob* attacker) { return(blockchance); }

//Final parry chance
float Mob::mod_parry_chance(float parrychance, Mob* attacker) { return(parrychance); }

//Final dodge chance
float Mob::mod_dodge_chance(float dodgechance, Mob* attacker) { return(dodgechance); }

//Usually 15, a monk under this weight threshold gets an AC bonus
float Mob::mod_monk_weight(float monkweight, Mob* attacker) { return(monkweight); }

//Mitigation rating is compared to incoming attack rating.  Higher is better.
float Mob::mod_mitigation_rating(float mitigation_rating, Mob* attacker) { return(mitigation_rating); }
float Mob::mod_attack_rating(float attack_rating, Mob* defender) { return(attack_rating); }

//Kick damage after all other bonuses are applied
int64 Mob::mod_kick_damage(int64 dmg) { return(dmg); }

//Slam and bash damage after all other bonuses are applied
int64 Mob::mod_bash_damage(int64 dmg) { return(dmg); }

//Frenzy damage after all other bonuses are applied
int64 Mob::mod_frenzy_damage(int64 dmg) { return(dmg); }

//Special attack damage after all other bonuses are applied.
int64 Mob::mod_monk_special_damage(int64 ndamage, EQ::skills::SkillType skill_type) { return(ndamage); }

//ndamage - Backstab damage as calculated by default formulas
int64 Mob::mod_backstab_damage(int64 ndamage) { return(ndamage); }

//Chance for 50+ archery bonus damage if Combat:UseArcheryBonusRoll is true.
int64 Mob::mod_archery_bonus_chance(int bonuschance, const ItemInst* RangeWeapon) { return(bonuschance); }

//Archery bonus damage
uint64 Mob::mod_archery_bonus_damage(uint64 MaxDmg, const ItemInst* RangeWeapon) { return(MaxDmg); }

//Final archery damage including bonus if it was applied.
int64 Mob::mod_archery_damage(int64 TotalDmg, bool hasbonus, const ItemInst* RangeWeapon) { return(TotalDmg); }

//Thrown weapon damage after all other calcs
uint64 Mob::mod_throwing_damage(uint64 MaxDmg) { return(MaxDmg); }

//Spell cast time in milliseconds - will not sync with client cast time bar, but does work.
int32 Mob::mod_cast_time(int32 cast_time) { return(cast_time); }

//res - Default buff duration formula
int Mob::mod_buff_duration(int res, Mob* caster, Mob* target, uint16 spell_id) { return(res); }

//Spell stack override - If this returns anything < 2, it will ignore all other stacking rules.
// See spells.cpp: Mob::CheckStackConflict
//  0 - No conflict
//  1 - Overwrite, spellid1 is replaced by spellid2
// -1 - Blocked, spellid2 will not land
//  2 - Default stacking behavior
int Mob::mod_spell_stack(uint16 spellid1, int caster_level1, Mob* caster1, uint16 spellid2, int caster_level2, Mob* caster2) { return(2); }

//Sum of various resists rolled against a value of 200.
int Mob::mod_spell_resist(int resist_chance, int level_mod, int resist_modifier, int target_resist, uint8 resist_type, uint16 spell_id, Mob* caster) {
	int final = resist_chance + level_mod + resist_modifier + target_resist;
	return(final);
}

//Spell is cast by this on spelltar, called from spellontarget after the event_cast_on NPC event
void Mob::mod_spell_cast(uint16 spell_id, Mob* spelltar, bool reflect, bool use_resist_adjust, int16 resist_adjust, bool isproc) { return; }
