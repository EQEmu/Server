#include "../common/races.h"
#include "../common/classes.h"
#include "../zone/skills.h"

uint8 MaxSkillTable(uint16 skillid, uint16 race, uint16 eqclass, uint16 level);
/* TODO:
	Load MaxSkillTable function into ram as a really big matrix:
	MaxSkillTable[skillid][race][eqclass][level]

	in preperation for that, dont put anything that wont work in
	that table into MaxSkillTable (ie, AA checks, skill values that
	depend on other skill values, etc), put it into MaxSkill instead
*/
uint8 MaxSkill(uint16 skillid, uint16 race, uint16 eqclass, uint16 level) {
	uint8 ret = MaxSkillTable(skillid, race, eqclass, level);
	return ret;
}

/* SPECIAL VALUES:
	level:
		0 = "skill level at character create"
		TODO: catch levels > 65 (ie, npcs)
	race:
		NPCs will always have their skills maxed, and often ignore class
		restrictions, accomplished by the special values below.
		EMU_RACE_NPC
		EMU_RACE_PET
		EMU_RACE_UNKNOWN
	return value:
		TODO: Find out the specal values for the client for "your class/race doesnt have this skill" and
		"must put one point in at GM", etc

*/
uint8 MaxSkillTable(uint16 skillid, uint16 race, uint16 eqclass, uint16 level) {
	uint16 r_value = 0;

	switch (skillid) {
		///////////////
		// Melee Weapon/ Hand to Hand
		///////////////
		case _1H_BLUNT:
		case _2H_BLUNT:
		case PIERCING:
		case HAND_TO_HAND:
		case _1H_SLASHING:
		case _2H_SLASHING:{
			switch (eqclass) {
				// Pure melee classes
				case WARRIOR: case WARRIORGM: {
					r_value = 5 + (level*5);
					if ( level < 51 && r_value > 200)
						r_value = 200;
					if ( level > 51 && r_value > 250 )
						r_value = 250;
					switch (skillid) {
						case PIERCING: {
							if ( r_value > 240 )
								r_value = 240;
							break;
						}
						case HAND_TO_HAND: {
							if ( r_value > 100 )
								r_value = 100;
							break;
						}
						default:
							break;
					}
					break;
				}
				case MONK: case MONKGM: {
					r_value = 5 + (level*5);
					if ( level < 51 && r_value > 240)
						if ( r_value > 240 )
							r_value = 240;
					if ( r_value > 252 )
						r_value = 252;
					switch (skillid) {
						case HAND_TO_HAND:{
							if ( r_value > 225 && level < 51 )
								r_value = 225;
							break;
						}
						case PIERCING:
						case _1H_SLASHING:
						case _2H_SLASHING:{
							r_value = 0;
							break;
						}
						default:
							break;
					}
					break;
				}
				case ROGUE: case ROGUEGM: {
					r_value = 5 + (level*5);
					if ( level > 50 ) {
						if ( r_value > 250 )
							r_value = 250;
					}
					else if ( level < 51 ) {
						if ( r_value > 200 && skillid != PIERCING )
							r_value = 200;
						switch (skillid) {
							case PIERCING: {
								if (r_value > 210)
									r_value = 210;
								break;
							}
							default:
								break;
						}
					}
					switch (skillid) {
						case HAND_TO_HAND:{
							if ( r_value > 100 )
								r_value = 100;
							break;
						}
						default:
							break;

					}
					break;
				}

		//////////////////////////////////////////////////////////////
		// Melee Weapon/ Hand to Hand
		// Priest classes
		//////////////////////////////////////////////////////////////
				case CLERIC: case CLERICGM:{
					r_value = 4 + (level*4);
					if ( r_value > 175 ){
						r_value = 175;
					}
					switch (skillid) {
						case HAND_TO_HAND:{
							if ( r_value > 75 )
								r_value = 75;
							break;
						}
						case PIERCING:
						case _1H_SLASHING:
						case _2H_SLASHING: {
							r_value = 0;
							break;
						}
						default:
							break;
					}
					break;
				}
				case DRUID: case DRUIDGM:{
					r_value = 4 + (level*4);
					if ( r_value > 175 ){
						r_value = 175;
					}
					switch (skillid) {
						case HAND_TO_HAND:{
							if ( r_value > 75 )
								r_value = 75;
						}
						case PIERCING:
						case _2H_SLASHING:{
							r_value = 0;
							break;
						}
						default:
							break;
					}
					break;
				}
				case SHAMAN: case SHAMANGM:{
					r_value = 4 + (level*4);
					if ( r_value > 200 ){
						r_value = 200;
					}
					switch (skillid) {
						case HAND_TO_HAND: {
							if ( r_value > 75 )
							r_value = 75;
						}
						case _1H_SLASHING:
						case _2H_SLASHING:{
							r_value = 0;
							break;
						}
						default:
							break;
					}
					break;
				}

		///////////////////////////////////////////////////////////
		// Melee Weapon/ Hand to Hand
		// Hybrids
		//////////////////////////////////////////////////////////
				case RANGER: case RANGERGM:{
					r_value = 5 + (level*5);
					if ( level > 50 ) {
						if ( r_value > 250 )
							r_value = 250;

						switch (skillid) {
							case PIERCING: {
								if ( r_value > 240 )
									r_value = 240;
								break;
							}
							default:
								break;
						}
					}
					else if ( level < 51 ) {
						if ( r_value > 200 )
							r_value = 200;
					}
					switch (skillid) {
						case HAND_TO_HAND:{
							if ( r_value > 100 )
								r_value = 100;
							break;
						}
						default:
							break;
					}
					break;
				}
				case PALADIN: case PALADINGM:
				case SHADOWKNIGHT: case SHADOWKNIGHTGM:{
					r_value = 5 + (level*5);
					if ( level > 50 ){
						if ( r_value > 225 )
							r_value = 225;
					}
					if ( level < 51 ){
						if ( r_value > 200 )
							r_value = 200;
					}

					switch (skillid) {
						case HAND_TO_HAND:{
							if ( r_value > 100 )
								r_value = 100;
							break;
						}
						default:
							break;
					}
					break;
				}
				case BARD: case BARDGM: {
					r_value = 5 + (level*5);
					if ( level > 51 && r_value > 225 )
						r_value = 225;
					if ( level < 51 && r_value > 200 )
						r_value = 200;
					switch (skillid) {
						case HAND_TO_HAND:{
							if ( r_value > 100 )
								r_value = 100;
							break;
						}
						case _2H_BLUNT:
						case _2H_SLASHING:{
							r_value = 0;

						}
						default:
							break;
					}
					break;
				}

				case BEASTLORD: case BEASTLORDGM:{
					r_value = 4 + (level*4);
					if ( level > 51 ){
						if ( r_value > 225 && skillid != HAND_TO_HAND )
							r_value = 225;
					}
					if ( r_value > 250 )
						r_value = 250;
					if ( level < 51 && r_value > 200 )
						r_value = 200;

					switch (skillid) {
						case HAND_TO_HAND:{
							r_value = 5 + (level*5);
							if ( level < 51 )
								r_value = 200;

							if ( r_value > 250 )
								r_value = 250;
							break;
						}
						case _1H_SLASHING:
						case _2H_SLASHING:{
							r_value = 0;
							break;
						}
						default:
							break;
					}
					break;
				}

		// Melee Weapon/ Hand to Hand
		// Pure casters
				case NECROMANCER: case NECROMANCERGM:
				case WIZARD: case WIZARDGM:
				case MAGICIAN: case MAGICIANGM:
				case ENCHANTER: case ENCHANTERGM:{
					r_value = 3 + (level*3);
					if ( r_value > 110 )
						r_value = 110;
					switch (skillid) {
						case HAND_TO_HAND:{
							if ( r_value > 75 )
								r_value = 75;
							break;
						}
						case _1H_SLASHING:
						case _2H_SLASHING:{
							r_value = 0;
							break;
						}
						default:
							break;
					}
				}
				default: {
					r_value = 0;
					break;
				}
			}// end switch(eqclass)
			break;
		} // end case weapon skills


/////////////////////////////////////////////////////////////
// Combat non weapon
/////////////////////////////////////////////////////////////

// Attack
		case OFFENSE: {
			switch (eqclass) {
				// Melee
				case WARRIOR: case WARRIORGM:
				case ROGUE: case ROGUEGM:{
					// 210 252 5*level+5
					r_value = ((level*5) + 5);
					if ( level < 51 ) {
						if (r_value > 210)
							r_value = 210;
					}
					if (r_value > 252)
						r_value = 252;
					break;
				}
				case MONK: case MONKGM:{
					// 230 252 5*level+5
					r_value = ((level*5) + 5);
					if ( level < 51 ) {
						if (r_value > 230)
							r_value = 230;
					}
					if (r_value > 252)
						r_value = 252;
					break;
				}
				// Priest
				case DRUID: case DRUIDGM:
				case SHAMAN: case SHAMANGM:
				case CLERIC: case CLERICGM:{
					// 200 200 4*level+4
					r_value = ((level*4) + 4);
					if (r_value > 200)
						r_value = 200;
					break;
				}
					// Hybrid
				case BEASTLORD: case BEASTLORDGM:{
					// 200 252 5*level+5
					r_value = ((level*5) + 5);
					if ( level < 51 ) {
						if (r_value > 200)
							r_value = 200;
					}
					if (r_value > 252)
						r_value = 252;
					break;
				}
				case PALADIN: case PALADINGM:
				case SHADOWKNIGHT: case SHADOWKNIGHTGM:
				case BARD: case BARDGM:{
					// 200 225 5*level+5

					r_value = ((level*5) + 5);
					if ( level < 51 ) {
						if (r_value > 200)
							r_value = 200;
					}
					if (r_value > 225)
						r_value = 225;
					break;
				}
				case RANGER: case RANGERGM:{
					// 210 252 5*level+5
					r_value = ((level*5) + 5);
					if ( level < 51 ) {
						if (r_value > 210)
							r_value = 210;
					}
					if (r_value > 252)
						r_value = 252;
					break;
				}
				// Pure
				case NECROMANCER: case NECROMANCERGM:
				case WIZARD: case WIZARDGM:
				case MAGICIAN: case MAGICIANGM:
				case ENCHANTER: case ENCHANTERGM:{
					// 140 140 level*4
					r_value = (level*4);
					if (r_value > 140)
						r_value = 140;
					break;
				}
				default: {
					r_value = 0;
					break;
				}
			} // end switch (eqclass)
			break;
		} // end case OFFENSE
		case THROWING: {
			switch (eqclass) {
				// Melee
				case ROGUE: case ROGUEGM:{
				// 220 250
					r_value = ((level*5) + 5);
					if ( level < 51 ) {
						if (r_value > 220)
							r_value = 220;
					}
					if (r_value > 250)
						r_value = 250;
					break;
				}
				case WARRIOR: case WARRIORGM:
				case MONK: case MONKGM:{
					// 113 200
					r_value = ((level*5) + 5);
					if ( level < 51 ) {
						if (r_value > 113)
							r_value = 113;
					}
					if (r_value > 200)
						r_value = 200;
					break;
				}
				// Hybrid
				case BEASTLORD: case BEASTLORDGM:
				case BARD: case BARDGM:
				case RANGER: case RANGERGM:{
					// 113
					r_value = ((level*5) + 5);
					if ( r_value > 113 )
						r_value = 113;
					break;
				}
				// Pure
				case NECROMANCER: case NECROMANCERGM:
				case WIZARD: case WIZARDGM:
				case MAGICIAN: case MAGICIANGM:
				case ENCHANTER: case ENCHANTERGM:{
					// 75
					r_value = ((level*3) + 3);
					if ( r_value > 75 )
						r_value = 75;
					break;
				}
				// No skill classes
				case DRUID: case DRUIDGM:
				case SHAMAN: case SHAMANGM:
				case CLERIC: case CLERICGM:
				case PALADIN: case PALADINGM:
				case SHADOWKNIGHT: case SHADOWKNIGHTGM:
				default: {
					r_value = 0;
					break;
				}
			} // end switch (eqclass)
			break;
		} // end case THROWING:
		case ARCHERY: {
			switch (eqclass) {
				// Melee
				case ROGUE: case ROGUEGM:
				case WARRIOR: case WARRIORGM:{
					// 200 240
						r_value = ((level*5) + 5);
					if ( level < 51 && r_value > 200)
						r_value = 200;
					if (r_value > 240)
						r_value = 240;
					break;
				}
				// Hybrid
				case PALADIN: case PALADINGM:
				case SHADOWKNIGHT: case SHADOWKNIGHTGM:{
					// 75 75
					r_value = ((level*5) + 5);
					if ( r_value > 75 )
						r_value = 75;
					break;
				}
				case RANGER: case RANGERGM:{
					// 240 240
					r_value = ((level*5) + 5);
					if ( r_value > 240 )
						r_value = 240;
					break;
				}
				// Pure
				// No skill classes
				// Melee
				case MONK: case MONKGM:
				// Priest
				case DRUID: case DRUIDGM:
				case SHAMAN: case SHAMANGM:
				case CLERIC: case CLERICGM:
				// Pure
				case NECROMANCER: case NECROMANCERGM:
				case WIZARD: case WIZARDGM:
				case MAGICIAN: case MAGICIANGM:
				case ENCHANTER: case ENCHANTERGM:
				// Hybrid
				case BEASTLORD: case BEASTLORDGM:
				case BARD: case BARDGM:
				default: {
					r_value = 0;
					break;
				}
			} // end switch (eqclass)
			break;
		} // end case ARCHERY:
		case DOUBLE_ATTACK: {
			switch (eqclass) {
				// Melee
				case ROGUE: case ROGUEGM:{
					// 16 200 240
					r_value = ((level*5) + 5);
					if ( level < 16 )
						r_value = 0;
					if ( level < 51 ) {
						if (r_value > 200)
							r_value = 200;
					}
					if (r_value > 240)
						r_value = 240;
					break;
				}
				case WARRIOR: case WARRIORGM:{
					// 15 205 245
					r_value = ((level*5) + 5);
					if ( level < 15 )
						r_value = 0;
					if ( level < 51 ) {
						if (r_value > 200)
							r_value = 200;
					}
					if (r_value > 245)
						r_value = 245;
					break;
				}
				case MONK: case MONKGM:{
					// 15 210 250
					r_value = ((level*5) + 5);
					if ( level < 15 )
						r_value = 0;
					if ( level < 51 ) {
						if (r_value > 210)
							r_value = 210;
					}
					if (r_value > 250)
						r_value = 250;
					break;
				}
				// Hybrid
				case PALADIN: case PALADINGM:
				case SHADOWKNIGHT: case SHADOWKNIGHTGM:{
					// 20 200 235
					r_value = ((level*5) + 5);
					if ( level < 20 )
						r_value = 0;
					if ( level < 51 ) {
						if (r_value > 200)
						r_value = 200;
					}
					if (r_value > 235)
						r_value = 235;
					break;
				}
				case RANGER: case RANGERGM:{
					// 20 200 245
					r_value = ((level*5) + 5);
					if ( level < 20 )
						r_value = 0;
					if ( level < 51 ) {
						if (r_value > 200)
						r_value = 200;
					}
					if (r_value > 245)
						r_value = 245;
					break;
				}
				// Pure
				// No skill classes
				// Melee
				// Priest
				case DRUID: case DRUIDGM:
				case SHAMAN: case SHAMANGM:
				case CLERIC: case CLERICGM:
				// Pure
				case NECROMANCER: case NECROMANCERGM:
				case WIZARD: case WIZARDGM:
				case MAGICIAN: case MAGICIANGM:
				case ENCHANTER: case ENCHANTERGM:
				// Hybrid
				case BEASTLORD: case BEASTLORDGM:
				case BARD: case BARDGM:
				default: {
					r_value = 0;

					break;
				}
			} // end switch (eqclass)
			break;
		} // end case DOUBLE_ATTACK:
		case DUEL_WIELD: {
			switch (eqclass) {
				// Melee
				case MONK: case MONKGM:{
				// 1 252 252
					r_value = level*7; // This can't be right can it?
					break;
				}
				case WARRIOR: case WARRIORGM:
				case ROGUE: case ROGUEGM: {
				// 15 210 245
					r_value = ((level*5) + 5);
					if ( level < 15 )
						r_value = 0;
					if ( level < 51 ) {
						if (r_value > 210)
						r_value = 210;
					}
					if (r_value > 245)
						r_value = 245;
					break;
				}
				// Hybrid
				case BEASTLORD: case BEASTLORDGM:
				// 17 210 245
				case RANGER: case RANGERGM:{
				// 17 210 245
					r_value = ((level*5) + 5);
					if ( level < 17 )
						r_value = 0;
					if ( level < 51 ) {
						if (r_value > 210)
						r_value = 210;
					}
					if (r_value > 245)
						r_value = 245;
					break;
				}
				case BARD: case BARDGM:{
				// 17 210 210
					r_value = ((level*5) + 5);
					if ( level < 17 )
						r_value = 0;
					if (r_value > 210)
						r_value = 210;
					break;
				}
				// No skill classes
				// Melee
				// Priest
				case DRUID: case DRUIDGM:
				case SHAMAN: case SHAMANGM:
				case CLERIC: case CLERICGM:
				// Pure
				case NECROMANCER: case NECROMANCERGM:
				case WIZARD: case WIZARDGM:
				case MAGICIAN: case MAGICIANGM:
				case ENCHANTER: case ENCHANTERGM:
				// Hybrid
				case PALADIN: case PALADINGM:
				case SHADOWKNIGHT: case SHADOWKNIGHTGM:
				default: {
					r_value = 0;
					break;
				}
			}// end Class switch
			break;
		} // end case DUEL_WIELD:
		case KICK: {
			switch (eqclass) {
				// Melee
				case WARRIOR: case WARRIORGM:{
					// 1 149 210
					r_value = ((level*5) + 5);
					if ( level < 51 ) {
						if (r_value > 149)
							r_value = 149;
					}
					if (r_value > 210)
						r_value = 210;
					break;
				}
				case MONK: case MONKGM:{
					// 1 200 250
					r_value = ((level*5) + 5);
					if ( level < 51 ) {
						if (r_value > 200)
							r_value = 200;
					}
					if (r_value > 250)
						r_value = 250;
					break;
				}
				// Hybrid
				case RANGER: case RANGERGM:{
					// 5 149 205
					r_value = ((level*5) + 5);
					if ( level < 5 )
						r_value = 0;
					if ( level < 51 ) {
						if (r_value > 149)
							r_value = 149;
					}
					if (r_value > 205)
						r_value = 205;
					break;
				}
				case BEASTLORD: case BEASTLORDGM:{
					// 5 180 230
					r_value = ((level*5) + 5);
					if ( level < 5 )
						r_value = 0;
					if ( level < 51 ) {
						if (r_value > 180)
							r_value = 180;
					}
					if (r_value > 230)
						r_value = 230;
					break;
				}
				// Pure
				// No skill classes
				case ROGUE: case ROGUEGM:
				// Melee
				// Priest
				case DRUID: case DRUIDGM:
				case SHAMAN: case SHAMANGM:
				case CLERIC: case CLERICGM:
				// Pure
				case NECROMANCER: case NECROMANCERGM:
				case WIZARD: case WIZARDGM:
				case MAGICIAN: case MAGICIANGM:
				case ENCHANTER: case ENCHANTERGM:
				// Hybrid
				case PALADIN: case PALADINGM:
				case SHADOWKNIGHT: case SHADOWKNIGHTGM:
				case BARD: case BARDGM:
				default: {
					r_value = 0;
					break;
				}
			} // end switch(eqclass)
			break;
		} // end case KICK:
		///////////
		// FIXME Where is slam?
		// Quagmire: Slam = bash w/ race check
		case BASH:{
			r_value = ((level*5)+5);
			switch (eqclass) {
				// Melee
				case WARRIOR: case WARRIORGM:{
					// 6 220 240
					if (level < 6)
						r_value = 0;
					if (level < 51 && r_value > 220)
						r_value = 220;
					if (r_value > 240)
						r_value = 240;
					break;
				}
				// Priest
				case CLERIC: case CLERICGM:{
					// 25 180 200
					if (level < 25)
						r_value = 0;
					if (level < 51 && r_value > 180)
						r_value = 180;
					if (r_value > 200)
						r_value = 200;
					break;
				}
				// Hybrid
				case PALADIN: case PALADINGM:
				case SHADOWKNIGHT: case SHADOWKNIGHTGM:{
					// 6 175 200
					if (level < 6)
						r_value = 0;
					if (level < 51 && r_value > 175)
						r_value = 175;
					if (r_value > 200)
						r_value = 200;
					break;
				}
				// Pure
				// No skill classes
				// Melee
				case MONK: case MONKGM:
				case ROGUE: case ROGUEGM:
				// Priest
				case DRUID: case DRUIDGM:
				case SHAMAN: case SHAMANGM:
				// Pure
				case NECROMANCER: case NECROMANCERGM:
				case WIZARD: case WIZARDGM:
				case MAGICIAN: case MAGICIANGM:
				case ENCHANTER: case ENCHANTERGM:
				// Hybrid
				case BEASTLORD: case BEASTLORDGM:
				case RANGER: case RANGERGM:
				case BARD: case BARDGM:{
					switch (race) {
						case BARBARIAN:
						case TROLL:
						case OGRE:{
							r_value = 50;
							break;
						}
						default: {
							break;
						}
					} // end switch (race)
					r_value = 0;
					break;
				}
			}
			break;
		} // end case BASH:
		/////////////////////////////////////
		/////////////////////////////////////
		// Defensive skills
		case DEFENSE:{
			switch (eqclass) {
				// Melee
				case WARRIOR: case WARRIORGM:{
					// 210 252 5*level+5
					r_value = ((level*5) + 5);
					if ( level < 51 ) {
						if (r_value > 210)
							r_value = 210;
					}
					if (r_value > 252)
						r_value = 252;
					break;
				}
				case ROGUE: case ROGUEGM:{
					// 200 252 5*level+5
					r_value = ((level*5) + 5);
					if ( level < 51 ) {
						if (r_value > 200)
							r_value = 200;
					}
					if (r_value > 252)
						r_value = 252;
					break;
				}
				case MONK: case MONKGM:{
					// 230 252 5*level+5
					r_value = ((level*5) + 5);
					if ( level < 51 ) {
						if (r_value > 230)
							r_value = 230;
					}
					if (r_value > 252)
						r_value = 252;
					break;
				}
				// Priest
				case DRUID: case DRUIDGM:
				case SHAMAN: case SHAMANGM:
				case CLERIC: case CLERICGM:{
					// 200 200 4*level+4
					r_value = ((level*4) + 4);
					if (r_value > 200)
						r_value = 200;
					break;
				}
				// Hybrid
				case BEASTLORD: case BEASTLORDGM:{
					// 210 252 5*level+5
					r_value = ((level*5) + 5);
					if ( level < 51 ) {
						if (r_value > 210)
							r_value = 210;
					}
					if (r_value > 252)
						r_value = 252;
					break;
				}
				case PALADIN: case PALADINGM:
				case SHADOWKNIGHT: case SHADOWKNIGHTGM:{
					// 210 252 5*level+5
					r_value = ((level*5) + 5);
					if ( level < 51 ) {
						if (r_value > 210)
							r_value = 210;
					}
					if (r_value > 252)
						r_value = 252;
					break;
				}
				case BARD: case BARDGM:{
					// 200 252 5*level+5
					r_value = ((level*5) + 5);
					if ( level < 51 ) {
						if (r_value > 200)
							r_value = 200;
					}
					if (r_value > 252)
						r_value = 252;
					break;
				}
				case RANGER: case RANGERGM:{
					// 200 200 5*level+5
					r_value = ((level*5) + 5);
					if (r_value > 200)
						r_value = 200;
					break;
				}
				// Pure
				case NECROMANCER: case NECROMANCERGM:
				case WIZARD: case WIZARDGM:
				case MAGICIAN: case MAGICIANGM:
				case ENCHANTER: case ENCHANTERGM:{
					// 145 145 level*4
					r_value = (level*4);
					if (r_value > 140)
						r_value = 140;
					break;
				}
				default: {
					break;
				}
			} // end switch(eqclass)
			break;
		} // end case DEFENSE:
		case PARRY:{
			switch (eqclass) {
				// Melee
				case ROGUE: case ROGUEGM:{
					// 12 200 230
					r_value = ((level*5) + 5);
					if ( level < 12 )
						r_value = 0;
					if (r_value > 200 && level < 51 )
						r_value = 200;
					if (r_value > 230)
						r_value = 230;
					break;
				}
				case WARRIOR: case WARRIORGM:{
					// 10 200 230
					r_value = ((level*5) + 5);
					if ( level < 10 )
						r_value = 0;
					if (r_value > 200 && level < 51 )
						r_value = 200;
					if (r_value > 230)
						r_value = 230;
					break;
				}
				// Hybrid
				case BARD: case BARDGM:{
					// 53 0 75
					r_value = ((level*5) + 5);
					if ( level < 53 )
						r_value = 0;
					if (r_value > 75)
						r_value = 75;
					break;
				}
				case PALADIN: case PALADINGM:
				case SHADOWKNIGHT: case SHADOWKNIGHTGM:{
					// 17 175 205
					r_value = ((level*5) + 5);
					if ( level < 17 )
						r_value = 0;
					if (r_value > 175 && level < 51 )
						r_value = 175;
					if (r_value > 205)
						r_value = 205;
					break;
				}
				case RANGER: case RANGERGM:{
					// 18 185 220
					r_value = ((level*5) + 5);
					if ( level < 18 )
						r_value = 0;
					if (r_value > 185 && level < 51 )
						r_value = 185;
					if (r_value > 220)
						r_value = 220;
					break;
				}
				// Pure
				// No skill classes
				// Melee
				case MONK: case MONKGM:
				// Priest
				case DRUID: case DRUIDGM:
				case SHAMAN: case SHAMANGM:
				case CLERIC: case CLERICGM:
				// Pure
				case NECROMANCER: case NECROMANCERGM:
				case WIZARD: case WIZARDGM:
				case MAGICIAN: case MAGICIANGM:
				case ENCHANTER: case ENCHANTERGM:
				// Hybrid
				case BEASTLORD: case BEASTLORDGM:
				default: {
					r_value = 0;
					break;
				}
			} // end switch (eqclass)
			break;
		} // end case PARRY:
		case RIPOSTE:{
			switch (eqclass) {
				// Melee
				case WARRIOR: case WARRIORGM:{
					// 25 200 225
					r_value = ((level*5) + 5);
					if ( level < 25 )
						r_value = 0;
					if (r_value > 200 && level < 51 )
						r_value = 200;
					if (r_value > 225)
						r_value = 225;
					break;
				}
				case ROGUE: case ROGUEGM:{
					// 30 200 225
					r_value = ((level*5) + 5);
					if ( level < 30 )
						r_value = 0;
					if (r_value > 200 && level < 51 )
						r_value = 200;
					if (r_value > 225)
						r_value = 225;
					break;
				}
				case MONK: case MONKGM:{
					// 35 200 225
					r_value = ((level*5) + 5);
					if ( level < 35 )
						r_value = 0;
					if (r_value > 200 && level < 51 )
						r_value = 200;
					if (r_value > 225)
						r_value = 225;
					break;
				}
				// Hybrid
				case BEASTLORD: case BEASTLORDGM:{
					// 40 150 185
					r_value = ((level*5) + 5);
					if ( level < 40 )
						r_value = 0;
					if (r_value > 150 && level < 51 )
						r_value = 150;
					if (r_value > 185)
						r_value = 185;
					break;
				}
				case BARD: case BARDGM:{
					// 58 75 75
					r_value = ((level*5) + 5);
					if ( level < 58 )
						r_value = 0;
					if (r_value > 75)
						r_value = 75;
					break;
				}
				case PALADIN: case PALADINGM:
				case SHADOWKNIGHT: case SHADOWKNIGHTGM:{
					// 30 175 200
					r_value = ((level*5) + 5);
					if ( level < 30 )
						r_value = 0;
					if (r_value > 175 && level < 51 )
						r_value = 175;
					if (r_value > 200)
						r_value = 200;
					break;
				}
				case RANGER: case RANGERGM:{
					// 35 150 150
					r_value = ((level*5) + 5);
					if ( level < 35 )
						r_value = 0;
					if (r_value > 150)
						r_value = 150;
					break;
				}
				// Pure
				// No skill classes
				// Melee
				// Priest
				case DRUID: case DRUIDGM:
				case SHAMAN: case SHAMANGM:
				case CLERIC: case CLERICGM:
				// Pure
				case NECROMANCER: case NECROMANCERGM:
				case WIZARD: case WIZARDGM:
				case MAGICIAN: case MAGICIANGM:
				case ENCHANTER: case ENCHANTERGM:
				// Hybrid
				default: {
					r_value = 0;
					break;
				}
			} // end switch (eqclass)
			break;
		} // end case RIPOSTE:
		case DODGE:{
			switch (eqclass) {
				// Melee
				case WARRIOR: case WARRIORGM:{
					// 6 140 175
					r_value = ((level*5) + 5);
					if ( level < 6 )
						r_value = 0;
					if (r_value > 140 && level < 51 )
						r_value = 140;
					if (r_value > 175)
						r_value = 175;
					break;
				}
				case ROGUE: case ROGUEGM:{
					// 4 150 210
					r_value = ((level*5) + 5);
					if ( level < 4 )
						r_value = 0;
					if (r_value > 150 && level < 51 )
						r_value = 150;
					if (r_value > 210)
						r_value = 210;
					break;
				}
				case MONK: case MONKGM:{
					// 1 200 230
					r_value = ((level*5) + 5);
					if (r_value > 200)
						r_value = 200;
					if (r_value > 230)
						r_value = 230;
					break;
				}
				// Priest
				case DRUID: case DRUIDGM:
				case SHAMAN: case SHAMANGM:
				case CLERIC: case CLERICGM:{
					// 15 75 75 4*level+4
					r_value = ((level*4) + 4);
					if ( level < 15 )
						r_value = 0;
					if (r_value > 75)
						r_value = 75;
					break;
				}
				// Hybrid
				case BEASTLORD: case BEASTLORDGM:
				case PALADIN: case PALADINGM:
				case SHADOWKNIGHT: case SHADOWKNIGHTGM:
				case BARD: case BARDGM:{
					// 10 125 155 5*level+5
					r_value = ((level*5) + 5);
					if ( level < 10 )
						r_value = 0;
					if (r_value > 125 && level < 51 )
						r_value = 125;
					if (r_value > 155)
						r_value = 155;
					break;
				}

				case RANGER: case RANGERGM:{
					// 8 137 170 5*level+5
					r_value = ((level*5) + 5);
					if ( level < 8 )
						r_value = 0;
					if (r_value > 137 && level < 51 )
						r_value = 137;
					if (r_value > 170)
						r_value = 170;
					break;
				}
				// Pure
				case NECROMANCER: case NECROMANCERGM:
				case WIZARD: case WIZARDGM:
				case MAGICIAN: case MAGICIANGM:
				case ENCHANTER: case ENCHANTERGM:{
					// 22 75 75 3*level+3
					r_value = ((level*3) + 3);
					if ( level < 22 )
						r_value = 0;
					if (r_value > 75)
						r_value = 75;
					break;
				}
				// No skill classes
				// Melee
				// Priest
				// Pure
				// Hybrid
				default: {
					r_value = 0;
					break;
				}
			} // end switch (eqclass)
			break;
		} // end case DODGE:
		// Other
		case TAUNT:{
			switch (eqclass) {
				// Melee
				case WARRIOR: case WARRIORGM:{
					// 1 200 200
					r_value = ((level*5) + 5);
					if (r_value > 200)
						r_value = 200;
					break;
				}
				// Priest
				// Hybrid
				case PALADIN: case PALADINGM:
				case SHADOWKNIGHT: case SHADOWKNIGHTGM:{
					// 1 180 180
					r_value = ((level*5) + 5);
					if (r_value > 180)
						r_value = 180;
					break;
				}
				case RANGER: case RANGERGM:{
					// 1 150 150
					r_value = ((level*5) + 5);
					if (r_value > 150)
						r_value = 150;
					break;
				}
				// Pure
				// No skill classes
				// Melee
				case ROGUE: case ROGUEGM:
				case MONK: case MONKGM:
				// Priest
				case DRUID: case DRUIDGM:
				case SHAMAN: case SHAMANGM:
				case CLERIC: case CLERICGM:
				// Pure
				case NECROMANCER: case NECROMANCERGM:
				case WIZARD: case WIZARDGM:
				case MAGICIAN: case MAGICIANGM:
				case ENCHANTER: case ENCHANTERGM:
				// Hybrid
				case BEASTLORD: case BEASTLORDGM:
				case BARD: case BARDGM:
				default: {
					r_value = 0;
					break;
				}
			} // end swtich (eqclass)
			break;
		} // end case TAUNT:

		case DISARM:{
			switch (eqclass) {
				// Melee
				case WARRIOR: case WARRIORGM:{
					// 35 200 200
					r_value = ((level*5) + 5);
					if (level < 35)
						r_value = 0;
					if (r_value > 200)
						r_value = 200;
					break;
				}
				case ROGUE: case ROGUEGM:
				case MONK: case MONKGM:{
					// 27 200 200
					r_value = ((level*5) + 5);
					if (level < 27)
						r_value = 0;
					if (r_value > 200)
						r_value = 200;
					break;
				}
				// Priest
				// Hybrid
				case PALADIN: case PALADINGM:
				case SHADOWKNIGHT: case SHADOWKNIGHTGM:{
					// 40 70 70
					r_value = ((level*5) + 5);
					if (level < 40)
						r_value = 0;
					if (r_value > 70)
						r_value = 70;
					break;
				}
				case RANGER: case RANGERGM:{
					// 35 55 55
					r_value = ((level*5) + 5);
					if (level < 35)
						r_value = 0;
					if (r_value > 55)
						r_value = 55;
					break;
				}
				// Pure
				// No skill classes
				// Melee
				// Priest
				case DRUID: case DRUIDGM:
				case SHAMAN: case SHAMANGM:
				case CLERIC: case CLERICGM:
				// Pure
				case NECROMANCER: case NECROMANCERGM:
				case WIZARD: case WIZARDGM:
				case MAGICIAN: case MAGICIANGM:
				case ENCHANTER: case ENCHANTERGM:
				// Hybrid
				case BARD: case BARDGM:
				case BEASTLORD: case BEASTLORDGM:
				default: {
					r_value = 0;
					break;
				}
			} // end switch (eqclass)
			break;
		} // end case DISARM:
		///////////////////////////////////////////
		///////////////////////////////////////////
		// Spell Skills
		case MEDITATE:
		case ABJURE:

		case ALTERATION:
		case CHANNELING:
		case CONJURATION:
		case DIVINATION:

		case EVOCATION:{
			r_value = ((level*5) + 5);
			switch(eqclass){
				// Hybrid
				case RANGER: case RANGERGM:{
					// 9 235 235
					// Channel 9 200 215
					// Med 12 185 235
					if (level < 9)
						r_value = 0;
					if (level < 12 && skillid == MEDITATE)
						r_value = 0;
					if (r_value > 0 && skillid == CHANNELING) {
						if ( level < 51 && r_value > 200)
						r_value = 200;
						if (r_value > 215)
							r_value = 215;
					}
					if (r_value > 0 && skillid == MEDITATE) {
						if ( level < 51 && r_value > 185)
							r_value = 185;
						if (r_value > 235)
							r_value = 235;
					}
					break;
				}
				case BEASTLORD: case BEASTLORDGM:
				case PALADIN: case PALADINGM:
				case SHADOWKNIGHT: case SHADOWKNIGHTGM:{
					// 9 235 235
					// Channel 9 200 220
					// Med 12 185 235
					if (level < 9)
						r_value = 0;
					if (level < 12 && skillid == MEDITATE)
						r_value = 0;
					if (r_value > 0 && skillid == CHANNELING) {
						if ( level < 51 && r_value > 185)
							r_value = 185;
						if (r_value > 220)
							r_value = 220;
					}
					if (r_value > 0 && skillid == MEDITATE) {
						if ( level < 51 && r_value > 185)
							r_value = 185;
						if (r_value > 235)
							r_value = 235;
					}
					break;
				}
				// Priest
				case CLERIC: case CLERICGM:
				case DRUID: case DRUIDGM:
				case SHAMAN: case SHAMANGM:{
					// 1 235 235
					// Channel 4 200 220
					// Med 8 235 252
					if (level < 4 && skillid == CHANNELING)
						r_value = 0;
					if (level < 8 && skillid == MEDITATE)
						r_value = 0;
					if (r_value > 0 && skillid == CHANNELING) {
						if ( level < 51 && r_value > 200)
							r_value = 200;
						if (r_value > 220)
						r_value = 220;
					}
					if (r_value > 0 && skillid == MEDITATE) {
						if ( level < 51 && r_value > 235)
							r_value = 235;
						if (r_value > 252)
							r_value = 252;
					}
					break;
				}
				// Int caster
				case ENCHANTER: case ENCHANTERGM:
				case MAGICIAN: case MAGICIANGM:
				case NECROMANCER: case NECROMANCERGM:
				case WIZARD: case WIZARDGM:{
					// 1 235 235
					// Channel 1 200 220
					// Med 4 235 252
					if (level < 4 && skillid == MEDITATE)
						r_value = 0;
					if (r_value > 0 && skillid == CHANNELING) {
						if ( level < 51 && r_value > 200)
							r_value = 200;
						if (r_value > 220)
							r_value = 220;
					}
					if (r_value > 0 && skillid == MEDITATE) {
						if ( level < 51 && r_value > 235)
							r_value = 235;
						if (r_value > 252)
					r_value = 252;
					}
					break;
				}
				case BARD: case BARDGM:{
					r_value = 0;
					if (level > 9 && skillid == MEDITATE)
						r_value = 1;
					break;
				}
				default: {
					// Unknown class
					r_value = 0;
					break;
				}
			}// Class Switch
			break;
		} // end spell skills

		case SPECIALIZE_ABJURE:
		case SPECIALIZE_ALTERATION:
		case SPECIALIZE_CONJURATION:
		case SPECIALIZE_DIVINATION:
		case SPECIALIZE_EVOCATION:
		case RESEARCH:{
			r_value = ((level*5) + 5);
			switch(eqclass){
				// Int caster
				case ENCHANTER: case ENCHANTERGM:
				case MAGICIAN: case MAGICIANGM:
				case NECROMANCER: case NECROMANCERGM:
				case WIZARD: case WIZARDGM:{
					// Res 16 200 200
					if (level < 16 && skillid == RESEARCH)
					r_value = 0;
					if (r_value > 0 && skillid == RESEARCH) {
					if (r_value > 200)
					r_value = 200;
					}
					if (r_value > 235)
					r_value = 235;
					// FIXME Only let one SPEC go above what ever limit theres supposed to be
					break;
				}
				default:{
					r_value = 0;
					break;
				}
			}// Class Switch
			break;
		} // end specilize & research skills

		case BRASS_INSTRUMENTS:
		case SINGING:
		case STRINGED_INSTRUMENTS:
		case WIND_INSTRUMENTS:
		case PERCUSSION_INSTRUMENTS:{
			switch(eqclass){
				case BARD: case BARDGM:{
					r_value = ((level*5) + 5);
					if (level < 5 && skillid == PERCUSSION_INSTRUMENTS){
						r_value = 0;
					}
					if (level < 8 && skillid == STRINGED_INSTRUMENTS){
						r_value = 0;
					}
					if (level < 11 && skillid == BRASS_INSTRUMENTS){
						r_value = 0;
					}
					if (level < 14 && skillid == WIND_INSTRUMENTS){
						r_value = 0;
					}
					if (r_value > 235)
						r_value = 235;
					break;
				}
				default: {
					r_value = 0;
				}
				break;
			}// Class Switch
			break;
		} // bard song skills
		///////////////////////////////////////////
		///////////////////////////////////////////
		// Class skills
		// Rogue
		case APPLY_POISON:
		case MAKE_POISON:
		case PICK_POCKETS:
		case BACKSTAB:{
			switch (eqclass) {
				// Melee
				case ROGUE: case ROGUEGM: {
					r_value = ((level*5) + 5);
					switch (skillid){
						case APPLY_POISON:{
							// 18 200 200
							if (level < 18)
								r_value = 0;
							if (r_value > 200)
								r_value = 200;
							break;
						}
						case MAKE_POISON:{
							// 20 200 250
							if (level < 20)
								r_value = 0;
							if (level < 51 && r_value > 200)
								r_value = 200;
							if (r_value > 250)
								r_value = 250;
							break;
						}
						case PICK_POCKETS:{
							// 7 200 210
							if (level < 7)
								r_value = 0;
							if (level < 51 && r_value > 200)
								r_value = 200;
							if (r_value > 210)
								r_value = 210;
							break;
						}
						case BACKSTAB:{
							// 10 200 225
							if (level < 10)
								r_value = 0;
							if (level < 51 && r_value > 200)
								r_value = 200;
							if (r_value > 225)
								r_value = 225;
							break;
						}
						default: {
							r_value = 0;
							break;
						}
					} // end switch (skillid)
					break;
				} // end case ROGUE: case ROGUEGM:
				default: {
					r_value = 0;
					break;
				}
			}// Class Switch
			break;
		} // end rogue skills
		// Monk
		case FEIGN_DEATH:
		case MEND:
		case DRAGON_PUNCH:
		case EAGLE_STRIKE:
		case FLYING_KICK:
		case ROUND_KICK:
		case TIGER_CLAW:
		case BLOCKSKILL:{
			switch(eqclass){
				case MONK: case MONKGM:{
					r_value = ((level*5) + 5);
					switch (skillid){
						case MEND:{
							// 1 200 200
							if (r_value > 200)
								r_value = 200;
							break;
						}
						case ROUND_KICK:{
							// 5 200 225
							if (level < 5)
								r_value = 0;
							if (level < 51 && r_value > 200)
								r_value = 200;
							if (r_value > 225)
								r_value = 225;
							break;
						}
						case TIGER_CLAW:{
							// 10 200 225
							if (level < 10)
								r_value = 0;
							if (level < 51 && r_value > 200)
								r_value = 200;
							if (r_value > 225)
								r_value = 225;
							break;
						}
						case BLOCKSKILL:{
							// 12 200 230
							if (level < 12)
								r_value = 0;
							if (level < 51 && r_value > 200)
								r_value = 200;
							if (r_value > 230)
								r_value = 230;
							break;
						}
						case FEIGN_DEATH:{
							// 17 200 200
							if (level < 17)
								r_value = 0;
							if (r_value > 200)
								r_value = 200;
							break;
						}
						case EAGLE_STRIKE:{
							// 20 200 225
							if (level < 20)
								r_value = 0;
							if (level < 51 && r_value > 200)
								r_value = 200;
							if (r_value > 225)
								r_value = 225;
							break;
						}
						case DRAGON_PUNCH:{
							// 25 200 225
							if (level < 25)
								r_value = 0;
							if (level < 51 && r_value > 200)
								r_value = 200;
							if (r_value > 225)
								r_value = 225;
							break;
						}
						case FLYING_KICK:{
							// 30 200 225
							if (level < 30)
								r_value = 0;
							if (level < 51 && r_value > 200)
								r_value = 200;
							if (r_value > 225)
								r_value = 225;
							break;
						}
						default: {
							r_value = 0;
							break;
						}
					} // end switch (skillid)
					break;
				} // end case MONK: case MONKGM:
				default: {
					r_value = 0;
					break;
				}
			}// Class Switch
			break;
		} // end monk skills
		// Shaman
		case ALCHEMY:{
			switch(eqclass){
				case SHAMAN: case SHAMANGM:{
					// 25 130 180
					r_value = ((level*5) + 5);
					if (level < 25)
						r_value = 0;
					if (level < 51 && r_value > 130)
						r_value = 130;
					if (r_value > 180)
						r_value = 180;
					break;
				}
				default: {
					r_value = 0;
					break;
				}
			}// Class Switch
			break;
		} // end case ALCHEMY:
		///////////////////////////////////////////
		//////////////////////////////////////////
		// Shared skill
		// Shared Rogue
		case HIDE:
		case SNEAK:{
			switch(eqclass){
				// True class
				case ROGUE: case ROGUEGM:{
					break;
				}
				// Hybrids
				case MONK: case MONKGM:
				case RANGER: case RANGERGM:
				case SHADOWKNIGHT: case SHADOWKNIGHTGM:
				case BARD: case BARDGM:{
					break;
				}
				default: {
					r_value = 0;
					break;
				}
			}// Class Switch
		} // end sneak/hide
		case SENSE_TRAPS:
		case PICK_LOCK:
		case DISARM_TRAPS:{
			switch(eqclass){
				// True class
				case ROGUE: case ROGUEGM:{
					break;
				}
				// Hybrids
				case BARD: case BARDGM:{
					break;
				}
				default: {
					r_value = 0;
					break;
				}
			}// Class Switch
			break;
		} // end case SENSE_TRAPS/PICK_LOCK/DISARM_TRAPS
		case SAFE_FALL:
		case INTIMIDATION:{
			switch(eqclass){
				// Melee
				case MONK: case MONKGM:
				case ROGUE: case ROGUEGM:{
				break;
				}
				default: {
					r_value = 0;
					break;
				}
			}// Class Switch
			break;
		} // end SAFE_FALL/INTIMIDATION
		// Druid/Ranger/Bard
		case FORAGE:{
			switch(eqclass) {
				case DRUID: case DRUIDGM:
				case RANGER: case RANGERGM:{
					if (r_value > 200)
						r_value = 200;
					break;
				}
				case BARD: case BARDGM: {
					r_value = 55;
					break;
				}
				default: {
					r_value = 00;
					break;
				}
			} // end switch (eqclass)
			break;
		} // end case FORAGE:
		case TRACKING:{
			switch(eqclass){
				case RANGER: case RANGERGM:
				case BARD: case BARDGM:
				case DRUID: case DRUIDGM: {
				}
				default: {
					r_value = 0;
					break;
				}
			}// Class Switch
		} // end case TRACKING
		///////////////////////////////////////////
		///////////////////////////////////////////
		// Tradeskills
		case BAKING:
		case TAILORING:
		case BLACKSMITHING:
		case FLETCHING:
		case BREWING:
		case JEWELRY_MAKING:
		case POTTERY:
		case FISHING:{
			// Check for Any Trade above 200, check for X (aa skill) Trades above 200
			r_value = 200;
			break;
		}

		///////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////
		// Gnome
		///////////////////////////////////////////////////////////////////
		case TINKERING:{
			if ( race == GNOME && level > 24 ) {
				r_value = ((level*5)+5);
				break;
			}
			r_value = 0;
			break;
		} // end case TINKERING:

		/////////////////////////////////////////
		// Common
		/////////////////////////////////////////
		case BIND_WOUND:{
			r_value = 5 + (level*5);
			if (level > 50){
				// Check for aa and class
			}
			if (r_value > 200)
				r_value = 200;
			switch (eqclass) {
				case ENCHANTER: case ENCHANTERGM:
				case MAGICIAN: case MAGICIANGM:
				case NECROMANCER: case NECROMANCERGM:
				case WIZARD: case WIZARDGM:{
					if ( r_value > 100 )
						r_value = 100;
				}
				default: {
					break;
				}
			} // end switch (eqclass)
			break;
		} // end case BIND_WOUND:
		case SENSE_HEADING:
		case SWIMMING:
		case ALCOHOL_TOLERANCE:
		case BEGGING:{
			r_value = 5 + (level*5);
			if (r_value > 200)
				r_value = 200;
			break;
		}
		//case BERSERKING:
		default: {
			// Unknown skill we should like print something to a log/debug here
			r_value = 0;
			break;
		}
	} // end switch (skillid)
// NO skill may go over 252
	if (r_value > 252)
		r_value = 252;
	return r_value;
}
