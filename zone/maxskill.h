// This file needs more than just 'std' updates

uint16 Mob::MaxSkill_weapon(uint16 skillid, uint16 class_, uint16 level) const
{
	if (skillid > HIGHEST_SKILL) {
		return 0;
	}
	uint16 r_value = 0;
	switch (skillid) {
		case _1H_BLUNT:
		case _2H_BLUNT:
		case PIERCING:
		case HAND_TO_HAND:
		case _1H_SLASHING:
		case _2H_SLASHING: {
				switch (class_) {
					// Pure melee classes
					case WARRIOR:
					case WARRIORGM: {
							r_value = 5 + (level * 5);
							if ( level < 51 && r_value > 200) {
								r_value = 200;
							}
							if ( level > 50 && r_value > 250 ) {
								r_value = 250;
							}
							switch (skillid) {
								case PIERCING: {
										if ( r_value > 240 ) {
											r_value = 240;
										}
										break;
									}
								case HAND_TO_HAND: {
										if ( r_value > 100 ) {
											r_value = 100;
										}
										break;
									}
								default:
									break;
							}
							break;
						}
					case MONK:
					case MONKGM: {
							r_value = 5 + (level * 5);
							if ( level < 51 && r_value > 240)
								if ( r_value > 240 ) {
									r_value = 240;
								}
							switch (skillid) {
								case HAND_TO_HAND: {
										if ( r_value > 225 && level < 51 ) {
											r_value = 225;
										}
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
					case ROGUE:
					case ROGUEGM: {
							r_value = 5 + (level * 5);
							if ( level > 50 && r_value > 250 ) {
								r_value = 250;
							}
							if ( level < 51 ) {
								if ( r_value > 200 && skillid != PIERCING ) {
									r_value = 200;
								}
								if ( r_value > 210 && skillid == PIERCING ) {
									r_value = 210;
								}
							}
							if (skillid == HAND_TO_HAND && r_value > 100) {
								r_value = 100;
							}
							break;
						}
					case BERSERKER:
					case BERSERKERGM: {
							r_value = 5 + (level * 5);
							if ( level < 51 && r_value > 240) {
								r_value = 240;
							}
							switch (skillid) {
								case HAND_TO_HAND: {
										if ( r_value > 198) {
											r_value = 198;
										}
										break;
									}
								case PIERCING: {
										if ( r_value > 240) {
											r_value = 240;
										}
										break;
									}
								case _2H_BLUNT:
								case _2H_SLASHING: {
										if ( r_value > 252 ) {
											r_value = 252;
										}
										break;
									}
								default:
									r_value = 0;
									break;
							}
							break;
						}
					// Priest classes
					case CLERIC:
					case CLERICGM: {
							r_value = 4 + (level * 4);
							if ( r_value > 175 ) {
								r_value = 175;
							}
							switch (skillid) {
								case HAND_TO_HAND: {
										if ( r_value > 75 ) {
											r_value = 75;
										}
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
					case DRUID:
					case DRUIDGM: {
							r_value = 4 + (level * 4);
							if ( r_value > 175 ) {
								r_value = 175;
							}
							switch (skillid) {
								case HAND_TO_HAND: {
										if ( r_value > 75 ) {
											r_value = 75;
										}
									}
								case PIERCING:
								case _2H_SLASHING: {
										r_value = 0;
										break;
									}
								default:
									break;
							}
							break;
						}
					case SHAMAN:
					case SHAMANGM: {
							r_value = 4 + (level * 4);
							if ( r_value > 200 ) {
								r_value = 200;
							}
							switch (skillid) {
								case HAND_TO_HAND: {
										if ( r_value > 75 ) {
											r_value = 75;
										}
									}
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
					// Hybrids
					case RANGER:
					case RANGERGM: {
							r_value = 5 + (level * 5);
							if ( level > 50 ) {
								if ( r_value > 250 ) {
									r_value = 250;
								}
								switch (skillid) {
									case PIERCING: {
											if ( r_value > 240 ) {
												r_value = 240;
											}
											break;
										}
									default:
										break;
								}
							} else if ( level < 51 ) {
								if ( r_value > 200 ) {
									r_value = 200;
								}
							}
							switch (skillid) {
								case HAND_TO_HAND: {
										if ( r_value > 100 ) {
											r_value = 100;
										}
										break;
									}
								default:
									break;
							}
							break;
						}
					case PALADIN:
					case PALADINGM:
					case SHADOWKNIGHT:
					case SHADOWKNIGHTGM: {
							r_value = 5 + (level * 5);
							if ( level > 50 && r_value > 225 ) {
								r_value = 225;
							}
							if ( level < 51 && r_value > 200 ) {
								r_value = 200;
							}
							switch (skillid) {
								case HAND_TO_HAND: {
										if ( r_value > 100 ) {
											r_value = 100;
										}
										break;
									}
								default:
									break;
							}
							break;
						}
					case BARD:
					case BARDGM: {
							r_value = 5 + (level * 5);
							if ( level > 51 && r_value > 225 ) {
								r_value = 225;
							}
							if ( level < 51 && r_value > 200 ) {
								r_value = 200;
							}
							switch (skillid) {
								case HAND_TO_HAND: {
										if ( r_value > 100 ) {
											r_value = 100;
										}
										break;
									}
								case _2H_BLUNT:
								case _2H_SLASHING: {
										r_value = 0;
									}
								default:
									break;
							}
							break;
						}
					case BEASTLORD:
					case BEASTLORDGM: {
							r_value = 4 + (level * 4);
							if ( level > 51 ) {
								if ( r_value > 225 ) {
									r_value = 225;
								}
							}
							if ( level < 51 && r_value > 200 ) {
								r_value = 200;
							}
							switch (skillid) {
								case HAND_TO_HAND: {
										r_value = 5 + (level * 5); // Beastlords use different max skill formula only for h2h 200/250
										if ( level < 51 ) {
											r_value = 200;
										}
										break;
									}
								case _1H_SLASHING:
								case _2H_SLASHING: {
										r_value = 0;
										break;
									}
								default:
									break;
							}
							if ( r_value > 250 ) {
								r_value = 250;
							}
							break;
						}
					// Pure casters
					case NECROMANCER:
					case NECROMANCERGM:
					case WIZARD:
					case WIZARDGM:
					case MAGICIAN:
					case MAGICIANGM:
					case ENCHANTER:
					case ENCHANTERGM: {
							r_value = 3 + (level * 3);
							if ( r_value > 110 ) {
								r_value = 110;
							}
							switch (skillid) {
								case HAND_TO_HAND: {
										if ( r_value > 75 ) {
											r_value = 75;
										}
									}
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
					default:
						#if EQDEBUG
						std::cout << "MaxSkill_Weapon() Unknown class: " << class_ << std::endl;
						#endif
						break;
				}
				break;// Switch Class
			}
		default:
			#if EQDEBUG
			std::cout << "Unknown weapon skill: " << skillid << std::endl;
			#endif
			break;
	}// Switch skill
	if (r_value > 252) {
		r_value = 252;
	}
	return r_value;
}

uint16 Mob::MaxSkill_offensive(uint16 skillid, uint16 class_, uint16 level) const
{
	uint16 r_value = 0;
	switch (skillid) {
		case OFFENSE: {
				switch (class_) {
					// Melee
					case WARRIOR:
					case WARRIORGM:
					case BERSERKER:
					case BERSERKERGM:
					case ROGUE:
					case ROGUEGM: {
							// 210 252 5*level+5
							r_value = ((level * 5) + 5);
							if ( level < 51 ) {
								if (r_value > 210) {
									r_value = 210;
								}
							}
							if (r_value > 252) {
								r_value = 252;
							}
							break;
						}
					case MONK:
					case MONKGM: {
							// 230 252 5*level+5
							r_value = ((level * 5) + 5);
							if ( level < 51 ) {
								if (r_value > 230) {
									r_value = 230;
								}
							}
							if (r_value > 252) {
								r_value = 252;
							}
							break;
						}
					// Priest
					case DRUID:
					case DRUIDGM:
					case SHAMAN:
					case SHAMANGM:
					case CLERIC:
					case CLERICGM: {
							// 200 200 4*level+4
							r_value = ((level * 4) + 4);
							if (r_value > 200) {
								r_value = 200;
							}
							break;
						}
					// Hybrid
					case BEASTLORD:
					case BEASTLORDGM: {
							// 200 252 5*level+5
							r_value = ((level * 5) + 5);
							if ( level < 51 ) {
								if (r_value > 200) {
									r_value = 200;
								}
							}
							if (r_value > 252) {
								r_value = 252;
							}
							break;
						}
					case PALADIN:
					case PALADINGM:
					case SHADOWKNIGHT:
					case SHADOWKNIGHTGM:
					case BARD:
					case BARDGM: {
							// 200 225 5*level+5
							r_value = ((level * 5) + 5);
							if ( level < 51 ) {
								if (r_value > 200) {
									r_value = 200;
								}
							}
							if (r_value > 225) {
								r_value = 225;
							}
							break;
						}
					case RANGER:
					case RANGERGM: {
							// 210 252 5*level+5
							r_value = ((level * 5) + 5);
							if ( level < 51 ) {
								if (r_value > 210) {
									r_value = 210;
								}
							}
							if (r_value > 252) {
								r_value = 252;
							}
							break;
						}
					// Pure
					case NECROMANCER:
					case NECROMANCERGM:
					case WIZARD:
					case WIZARDGM:
					case MAGICIAN:
					case MAGICIANGM:
					case ENCHANTER:
					case ENCHANTERGM: {
							// 140 140 level*4
							r_value = (level * 4);
							if (r_value > 140) {
								r_value = 140;
							}
							break;
						}
					default:
						break;
				}
				break;
			}
		case THROWING: {
				switch (class_) {
					// Melee
					case BERSERKER:
					case BERSERKERGM:
					case ROGUE:
					case ROGUEGM: {
							// 220 250
							r_value = ((level * 5) + 5);
							if ( level < 51 ) {
								if (r_value > 220) {
									r_value = 220;
								}
							}
							if (r_value > 250) {
								r_value = 250;
							}
							break;
						}
					case WARRIOR:
					case WARRIORGM:
					case MONK:
					case MONKGM: {
							// 113 200
							r_value = ((level * 5) + 5);
							if ( level < 51 ) {
								if (r_value > 113) {
									r_value = 113;
								}
							}
							if (r_value > 200) {
								r_value = 200;
							}
							break;
						}
					// Hybrid
					case BEASTLORD:
					case BEASTLORDGM:
					case BARD:
					case BARDGM:
					case RANGER:
					case RANGERGM: {
							// 113
							r_value = ((level * 5) + 5);
							if ( r_value > 113 ) {
								r_value = 113;
							}
							break;
						}
					// Pure
					case NECROMANCER:
					case NECROMANCERGM:
					case WIZARD:
					case WIZARDGM:
					case MAGICIAN:
					case MAGICIANGM:
					case ENCHANTER:
					case ENCHANTERGM: {
							// 75
							r_value = ((level * 3) + 3);
							if ( r_value > 75 ) {
								r_value = 75;
							}
							break;
						}
					// No skill classes
					case DRUID:
					case DRUIDGM:
					case SHAMAN:
					case SHAMANGM:
					case CLERIC:
					case CLERICGM:
					case PALADIN:
					case PALADINGM:
					case SHADOWKNIGHT:
					case SHADOWKNIGHTGM:
					default:
						r_value = 0;
						break;
				}
				break;
			}
/////////////////////////////////////////////////
		case ARCHERY: {
				switch (class_) {
					// Melee
					case ROGUE:
					case ROGUEGM:
					case WARRIOR:
					case WARRIORGM: {
							// 200 240
							r_value = ((level * 5) + 5);
							if ( level < 51 && r_value > 200) {
								r_value = 200;
							}
							if (r_value > 240) {
								r_value = 240;
							}
							break;
						}
					// Hybrid
					case PALADIN:
					case PALADINGM:
					case SHADOWKNIGHT:
					case SHADOWKNIGHTGM: {
							// 75 75
							r_value = ((level * 5) + 5);
							if ( r_value > 75 ) {
								r_value = 75;
							}
							break;
						}
					case RANGER:
					case RANGERGM: {
							// 240 240
							r_value = ((level * 5) + 5);
							if ( r_value > 240 ) {
								r_value = 240;
							}
							break;
						}
					// Pure
					// No skill classes
					// Melee
					case MONK:
					case MONKGM:
					// Priest
					case DRUID:
					case DRUIDGM:
					case SHAMAN:
					case SHAMANGM:
					case CLERIC:
					case CLERICGM:
					// Pure
					case NECROMANCER:
					case NECROMANCERGM:
					case WIZARD:
					case WIZARDGM:
					case MAGICIAN:
					case MAGICIANGM:
					case ENCHANTER:
					case ENCHANTERGM:
					// Hybrid
					case BEASTLORD:
					case BEASTLORDGM:
					case BARD:
					case BARDGM:
					default:
						r_value = 0;
						break;
				}
				break;
			}
/////////////////////////////////////////////////
		case DOUBLE_ATTACK: {
				switch (class_) {
					// Melee
					case ROGUE:
					case ROGUEGM: {
							// 16 200 240
							r_value = ((level * 5) + 5);
							if ( level < 16 ) {
								r_value = 0;
							}
							if ( level < 51 ) {
								if (r_value > 200) {
									r_value = 200;
								}
							}
							if (r_value > 240) {
								r_value = 240;
							}
							break;
						}
					case BERSERKER:
					case BERSERKERGM:
					case WARRIOR:
					case WARRIORGM: {
							// 15 205 245
							r_value = ((level * 5) + 5);
							if ( level < 15 ) {
								r_value = 0;
							}
							if ( level < 51 ) {
								if (r_value > 200) {
									r_value = 200;
								}
							}
							if (r_value > 245) {
								r_value = 245;
							}
							break;
						}
					case MONK:
					case MONKGM: {
							// 15 210 250
							r_value = ((level * 5) + 5);
							if ( level < 15 ) {
								r_value = 0;
							}
							if ( level < 51 ) {
								if (r_value > 210) {
									r_value = 210;
								}
							}
							if (r_value > 250) {
								r_value = 250;
							}
							break;
						}
					// Hybrid
					case PALADIN:
					case PALADINGM:
					case SHADOWKNIGHT:
					case SHADOWKNIGHTGM: {
							// 20 200 235
							r_value = ((level * 5) + 5);
							if ( level < 20 ) {
								r_value = 0;
							}
							if ( level < 51 ) {
								if (r_value > 200) {
									r_value = 200;
								}
							}
							if (r_value > 235) {
								r_value = 235;
							}
							break;
						}
					case RANGER:
					case RANGERGM: {
							// 20 200 245
							r_value = ((level * 5) + 5);
							if ( level < 20 ) {
								r_value = 0;
							}
							if ( level < 51 ) {
								if (r_value > 200) {
									r_value = 200;
								}
							}
							if (r_value > 245) {
								r_value = 245;
							}
							break;
						}
					// Pure
					// No skill classes
					// Melee
					// Priest
					case DRUID:
					case DRUIDGM:
					case SHAMAN:
					case SHAMANGM:
					case CLERIC:
					case CLERICGM:
					// Pure
					case NECROMANCER:
					case NECROMANCERGM:
					case WIZARD:
					case WIZARDGM:
					case MAGICIAN:
					case MAGICIANGM:
					case ENCHANTER:
					case ENCHANTERGM:
					// Hybrid
					case BEASTLORD:
					case BEASTLORDGM:
					case BARD:
					case BARDGM:
					default:
						r_value = 0;
						break;
				}
				break;
			}
/////////////////////////////////////////////////
		case DUAL_WIELD: {
				switch (class_) {
					// Melee
					case MONK:
					case MONKGM: {
							// 1 252 252
							r_value = level * 7; // This can't be right can it?
							break
							;
						}
					case WARRIOR:
					case WARRIORGM:
					case ROGUE:
					case ROGUEGM: {
							// 15 210 245
							r_value = ((level * 5) + 5);
							if ( level < 13 ) {
								r_value = 0;
							}
							if ( level < 51 ) {
								if (r_value > 210) {
									r_value = 210;
								}
							}
							if (r_value > 245) {
								r_value = 245;
							}
							break;
						}
					// Hybrid
					case BEASTLORD:
					case BEASTLORDGM:
					// 17 210 245
					case RANGER:
					case RANGERGM: {
							// 17 210 245
							r_value = ((level * 5) + 5);
							if ( level < 17 ) {
								r_value = 0;
							}
							if ( level < 51 ) {
								if (r_value > 210) {
									r_value = 210;
								}
							}
							if (r_value > 245) {
								r_value = 245;
							}
							break;
						}
					case BARD:
					case BARDGM: {
							// 17 210 210
							r_value = ((level * 5) + 5);
							if ( level < 17 ) {
								r_value = 0;
							}
							if (r_value > 210) {
								r_value = 210;
							}
							break;
						}
					// No skill classes
					// Melee
					// Priest
					case DRUID:
					case DRUIDGM:
					case SHAMAN:
					case SHAMANGM:
					case CLERIC:
					case CLERICGM:
					// Pure
					case NECROMANCER:
					case NECROMANCERGM:
					case WIZARD:
					case WIZARDGM:
					case MAGICIAN:
					case MAGICIANGM:
					case ENCHANTER:
					case ENCHANTERGM:
					// Hybrid
					case PALADIN:
					case PALADINGM:
					case SHADOWKNIGHT:
					case SHADOWKNIGHTGM:
					default: {
							r_value = 0;
							break;
						}
				}// end Class switch
				break;
			} // end case DUAL_WIELD:
////////////////////////////////////////////////////////
		case KICK: {
				switch (class_) {
					// Melee
					case BERSERKER:
					case BERSERKERGM:
					case WARRIOR:
					case WARRIORGM: {
							// 1 149 210
							r_value = ((level * 5) + 5);
							if ( level < 51 ) {
								if (r_value > 149) {
									r_value = 149;
								}
							}
							if (r_value > 210) {
								r_value = 210;
							}
							break;
						}
					case MONK:
					case MONKGM: {
							// 1 200 250
							r_value = ((level * 5) + 5);
							if ( level < 51 ) {
								if (r_value > 200) {
									r_value = 200;
								}
							}
							if (r_value > 250) {
								r_value = 250;
							}
							break;
						}
					// Hybrid
					case RANGER:
					case RANGERGM: {
							// 5 149 205
							r_value = ((level * 5) + 5);
							if ( level < 5 ) {
								r_value = 0;
							}
							if ( level < 51 ) {
								if (r_value > 149) {
									r_value = 149;
								}
							}
							if (r_value > 205) {
								r_value = 205;
							}
							break;
						}
					case BEASTLORD:
					case BEASTLORDGM: {
							// 5 180 230
							r_value = ((level * 5) + 5);
							if ( level < 5 ) {
								r_value = 0;
							}
							if ( level < 51 ) {
								if (r_value > 180) {
									r_value = 180;
								}
							}
							if (r_value > 230) {
								r_value = 230;
							}
							break;
						}
					// Pure
					// No skill classes
					case ROGUE:
					case ROGUEGM:
					// Melee
					// Priest
					case DRUID:
					case DRUIDGM:
					case SHAMAN:
					case SHAMANGM:
					case CLERIC:
					case CLERICGM:
					// Pure
					case NECROMANCER:
					case NECROMANCERGM:
					case WIZARD:
					case WIZARDGM:
					case MAGICIAN:
					case MAGICIANGM:
					case ENCHANTER:
					case ENCHANTERGM:
					// Hybrid
					case PALADIN:
					case PALADINGM:
					case SHADOWKNIGHT:
					case SHADOWKNIGHTGM:
					case BARD:
					case BARDGM:
					default:
						r_value = 0;
						break;
				}
				break;
			}
////////////////////////////////////////////////////////
		case BASH: {
				r_value = ((level * 5) + 5);
				switch (class_) {
					// Melee
					case WARRIOR:
					case WARRIORGM: {
							// 6 220 240
							if (level < 6) {
								r_value = 0;
							}
							if (level < 51 && r_value > 220) {
								r_value = 220;
							}
							if (r_value > 240) {
								r_value = 240;
							}
							break;
						}
					// Priest
					case CLERIC:
					case CLERICGM: {
							// 25 180 200
							if (level < 25) {
								r_value = 0;
							}
							if (level < 51 && r_value > 180) {
								r_value = 180;
							}
							if (r_value > 200) {
								r_value = 200;
							}
							break;
						}
					// Hybrid
					case PALADIN:
					case PALADINGM:
					case SHADOWKNIGHT:
					case SHADOWKNIGHTGM: {
							// 6 175 200
							if (level < 6) {
								r_value = 0;
							}
							if (level < 51 && r_value > 175) {
								r_value = 175;
							}
							if (r_value > 200) {
								r_value = 200;
							}
							break;
						}
					// Pure
					// No skill classes
					// Melee
					case MONK:
					case MONKGM:
					case ROGUE:
					case ROGUEGM:
					// Priest
					case DRUID:
					case DRUIDGM:
					case SHAMAN:
					case SHAMANGM:
					// Pure
					case NECROMANCER:
					case NECROMANCERGM:
					case WIZARD:
					case WIZARDGM:
					case MAGICIAN:
					case MAGICIANGM:
					case ENCHANTER:
					case ENCHANTERGM:
					// Hybrid
					case BEASTLORD:
					case BEASTLORDGM:
					case RANGER:
					case RANGERGM:
					case BARD:
					case BARDGM: {
							// switch (race) {
							//  case BARBARIAN:
							// case TROLL:
							//case OGRE:{
							// r_value = 50;
							//break;
							//}
							//default: break;
							//}
							r_value = 0;
							break;
						}
				}
				break;
			}
////////////////////////////////////////////////////////
		default:
			#if EQDEBUG >= 1
			std::cout << "Unknown Offensive skill: " << skillid << std::endl;
			#endif
			break;
	}// Switch skill
	if (r_value > 252) {
		r_value = 252;
	}
	return r_value;
}

uint16 Mob::MaxSkill_defensive(uint16 skillid, uint16 class_, uint16 level) const
{
	uint16 r_value = 0;
	switch (skillid) {
		case DEFENSE: {
				switch (class_) {
					// Melee
					case WARRIOR:
					case WARRIORGM: {
							// 210 252 5*level+5
							r_value = ((level * 5) + 5);
							if ( level < 51 ) {
								if (r_value > 210) {
									r_value = 210;
								}
							}
							if (r_value > 252) {
								r_value = 252;
							}
							break;
						}
					case ROGUE:
					case ROGUEGM: {
							// 200 252 5*level+5
							r_value = ((level * 5) + 5);
							if ( level < 51 ) {
								if (r_value > 200) {
									r_value = 200;
								}
							}
							if (r_value > 252) {
								r_value = 252;
							}
							break;
						}
					case MONK:
					case MONKGM: {
							// 230 252 5*level+5
							r_value = ((level * 5) + 5);
							if ( level < 51 ) {
								if (r_value > 230) {
									r_value = 230;
								}
							}
							if (r_value > 252) {
								r_value = 252;
							}
							break;
						}
					case BERSERKER:
					case BERSERKERGM: {
							// 230 252 5*level+5
							r_value = ((level * 5) + 5);
							if ( level < 51 ) {
								if (r_value > 230) {
									r_value = 230;
								}
							}
							if (r_value > 252) {
								r_value = 252;
							}
							break;
						}
					// Priest
					case DRUID:
					case DRUIDGM:
					case SHAMAN:
					case SHAMANGM:
					case CLERIC:
					case CLERICGM: {
							// 200 200 4*level+4
							r_value = ((level * 4) + 4);
							if (r_value > 200) {
								r_value = 200;
							}
							break;
						}
					// Hybrid
					case BEASTLORD:
					case BEASTLORDGM: {
							// 210 252 5*level+5
							r_value = ((level * 5) + 5);
							if ( level < 51 ) {
								if (r_value > 210) {
									r_value = 210;
								}
							}
							if (r_value > 252) {
								r_value = 252;
							}
							break;
						}
					case PALADIN:
					case PALADINGM:
					case SHADOWKNIGHT:
					case SHADOWKNIGHTGM: {
							// 210 252 5*level+5
							r_value = ((level * 5) + 5);
							if ( level < 51 ) {
								if (r_value > 210) {
									r_value = 210;
								}
							}
							if (r_value > 252) {
								r_value = 252;
							}
							break;
						}
					case BARD:
					case BARDGM: {
							// 200 252 5*level+5
							r_value = ((level * 5) + 5);
							if ( level < 51 ) {
								if (r_value > 200) {
									r_value = 200;
								}
							}
							if (r_value > 252) {
								r_value = 252;
							}
							break;
						}
					case RANGER:
					case RANGERGM: {
							// 200 200 5*level+5
							r_value = ((level * 5) + 5);
							if (r_value > 200) {
								r_value = 200;
							}
							break;
						}
					// Pure
					case NECROMANCER:
					case NECROMANCERGM:
					case WIZARD:
					case WIZARDGM:
					case MAGICIAN:
					case MAGICIANGM:
					case ENCHANTER:
					case ENCHANTERGM: {
							// 145 145 level*4
							r_value = (level * 4);
							if (r_value > 140) {
								r_value = 140;
							}
							break;
						}
					default:
						break;
				}
				break;
			}
		case PARRY: {
				switch (class_) {
					// Melee
					case ROGUE:
					case ROGUEGM: {
							// 12 200 230
							r_value = ((level * 5) + 5);
							if ( level < 12 ) {
								r_value = 0;
							}
							if (r_value > 200 && level < 51 ) {
								r_value = 200;
							}
							if (r_value > 230) {
								r_value = 230;
							}
							break;
						}
					case WARRIOR:
					case WARRIORGM: {
							// 10 200 230
							r_value = ((level * 5) + 5);
							if ( level < 10 ) {
								r_value = 0;
							}
							if (r_value > 200 && level < 51 ) {
								r_value = 200;
							}
							if (r_value > 230) {
								r_value = 230;
							}
							break;
						}
					case BERSERKER:
					case BERSERKERGM: {
							r_value = ((level * 5) + 5);
							if ( level < 10 ) {
								r_value = 0;
							}
							if (r_value > 175) {
								r_value = 175;
							}
							break;
						}
					// Hybrid
					case BARD:
					case BARDGM: {
							// 53 0 75
							r_value = ((level * 5) + 5);
							if ( level < 53 ) {
								r_value = 0;
							}
							if (r_value > 75) {
								r_value = 75;
							}
							break;
						}
					case PALADIN:
					case PALADINGM:
					case SHADOWKNIGHT:
					case SHADOWKNIGHTGM: {
							// 17 175 205
							r_value = ((level * 5) + 5);
							if ( level < 17 ) {
								r_value = 0;
							}
							if (r_value > 175 && level < 51 ) {
								r_value = 175;
							}
							if (r_value > 205) {
								r_value = 205;
							}
							break;
						}
					case RANGER:
					case RANGERGM: {
							// 18 185 220
							r_value = ((level * 5) + 5);
							if ( level < 18 ) {
								r_value = 0;
							}
							if (r_value > 185 && level < 51 ) {
								r_value = 185;
							}
							if (r_value > 220) {
								r_value = 220;
							}
							break;
						}
					// Pure
					// No skill classes
					// Melee
					case MONK:
					case MONKGM:
					// Priest
					case DRUID:
					case DRUIDGM:
					case SHAMAN:
					case SHAMANGM:
					case CLERIC:
					case CLERICGM:
					// Pure
					case NECROMANCER:
					case NECROMANCERGM:
					case WIZARD:
					case WIZARDGM:
					case MAGICIAN:
					case MAGICIANGM:
					case ENCHANTER:
					case ENCHANTERGM:
					// Hybrid
					case BEASTLORD:
					case BEASTLORDGM:
					default:
						r_value = 0;
						break;
				}
				break;
			}
		case RIPOSTE: {
				switch (class_) {
					// Melee
					case BERSERKER:
					case BERSERKERGM:
					case WARRIOR:
					case WARRIORGM: {
							// 25 200 225
							r_value = ((level * 5) + 5);
							if ( level < 25 ) {
								r_value = 0;
							}
							if (r_value > 200 && level < 51 ) {
								r_value = 200;
							}
							if (r_value > 225) {
								r_value = 225;
							}
							break;
						}
					case ROGUE:
					case ROGUEGM: {
							// 30 200 225
							r_value = ((level * 5) + 5);
							if ( level < 30 ) {
								r_value = 0;
							}
							if (r_value > 200 && level < 51 ) {
								r_value = 200;
							}
							if (r_value > 225) {
								r_value = 225;
							}
							break;
						}
					case MONK:
					case MONKGM: {
							// 35 200 225
							r_value = ((level * 5) + 5);
							if ( level < 35 ) {
								r_value = 0;
							}
							if (r_value > 200 && level < 51 ) {
								r_value = 200;
							}
							if (r_value > 225) {
								r_value = 225;
							}
							break;
						}
					// Hybrid
					case BEASTLORD:
					case BEASTLORDGM: {
							// 40 150 185
							r_value = ((level * 5) + 5);
							if ( level < 40 ) {
								r_value = 0;
							}
							if (r_value > 150 && level < 51 ) {
								r_value = 150;
							}
							if (r_value > 185) {
								r_value = 185;
							}
							break;
						}
					case BARD:
					case BARDGM: {
							// 58 75 75
							r_value = ((level * 5) + 5);
							if ( level < 58 ) {
								r_value = 0;
							}
							if (r_value > 75) {
								r_value = 75;
							}
							break;
						}
					case PALADIN:
					case PALADINGM:
					case SHADOWKNIGHT:
					case SHADOWKNIGHTGM: {
							// 30 175 200
							r_value = ((level * 5) + 5);
							if ( level < 30 ) {
								r_value = 0;
							}
							if (r_value > 175 && level < 51 ) {
								r_value = 175;
							}
							if (r_value > 200) {
								r_value = 200;
							}
							break;
						}
					case RANGER:
					case RANGERGM: {
							// 35 150 150
							r_value = ((level * 5) + 5);
							if ( level < 35 ) {
								r_value = 0;
							}
							if (r_value > 150) {
								r_value = 150;
							}
							break;
						}
					// Pure
					// No skill classes
					// Melee
					// Priest
					case DRUID:
					case DRUIDGM:
					case SHAMAN:
					case SHAMANGM:
					case CLERIC:
					case CLERICGM:
					// Pure
					case NECROMANCER:
					case NECROMANCERGM:
					case WIZARD:
					case WIZARDGM:
					case MAGICIAN:
					case MAGICIANGM:
					case ENCHANTER:
					case ENCHANTERGM:
					// Hybrid
					default:
						r_value = 0;
						break;
				}
				break;
			}
		case DODGE: {
				switch (class_) {
					// Melee
					case BERSERKER:
					case BERSERKERGM:
					case WARRIOR:
					case WARRIORGM: {
							// 6 140 175
							r_value = ((level * 5) + 5);
							if ( level < 6 ) {
								r_value = 0;
							}
							if (r_value > 140 && level < 51 ) {
								r_value = 140;
							}
							if (r_value > 175) {
								r_value = 175;
							}
							break;
						}
					case ROGUE:
					case ROGUEGM: {
							// 4 150 210
							r_value = ((level * 5) + 5);
							if ( level < 4 ) {
								r_value = 0;
							}
							if (r_value > 150 && level < 51 ) {
								r_value = 150;
							}
							if (r_value > 210) {
								r_value = 210;
							}
							break;
						}
					case MONK:
					case MONKGM: {
							// 1 200 230
							r_value = ((level * 5) + 5);
							if (r_value > 200) {
								r_value = 200;
							}
							if (r_value > 230) {
								r_value = 230;
							}
							break;
						}
					// Priest
					case DRUID:
					case DRUIDGM:
					case SHAMAN:
					case SHAMANGM:
					case CLERIC:
					case CLERICGM: {
							// 15 75 75 4*level+4
							r_value = ((level * 4) + 4);
							if ( level < 15 ) {
								r_value = 0;
							}
							if (r_value > 75) {
								r_value = 75;
							}
							break;
						}
					// Hybrid
					case BEASTLORD:
					case BEASTLORDGM:
					case PALADIN:
					case PALADINGM:
					case SHADOWKNIGHT:
					case SHADOWKNIGHTGM:
					case BARD:
					case BARDGM: {
							// 10 125 155 5*level+5
							r_value = ((level * 5) + 5);
							if ( level < 10 ) {
								r_value = 0;
							}
							if (r_value > 125 && level < 51 ) {
								r_value = 125;
							}
							if (r_value > 155) {
								r_value = 155;
							}
							break;
						}
					case RANGER:
					case RANGERGM: {
							// 8 137 170 5*level+5
							r_value = ((level * 5) + 5);
							if ( level < 8 ) {
								r_value = 0;
							}
							if (r_value > 137 && level < 51 ) {
								r_value = 137;
							}
							if (r_value > 170) {
								r_value = 170;
							}
							break;
						}
					// Pure
					case NECROMANCER:
					case NECROMANCERGM:
					case WIZARD:
					case WIZARDGM:
					case MAGICIAN:
					case MAGICIANGM:
					case ENCHANTER:
					case ENCHANTERGM: {
							// 22 75 75 3*level+3
							r_value = ((level * 3) + 3);
							if ( level < 22 ) {
								r_value = 0;
							}
							if (r_value > 75) {
								r_value = 75;
							}
							break;
						}
					// No skill classes
					// Melee
					// Priest
					// Pure
					// Hybrid
					default:
						break;
				}
				break;
			}
		// Other
		case TAUNT: {
				switch (class_) {
					// Melee
					case WARRIOR:
					case WARRIORGM: {
							// 1 200 200
							r_value = ((level * 5) + 5);
							if (r_value > 200) {
								r_value = 200;
							}
							break;
						}
					// Priest
					// Hybrid
					case PALADIN:
					case PALADINGM:
					case SHADOWKNIGHT:
					case SHADOWKNIGHTGM: {
							// 1 180 180
							r_value = ((level * 5) + 5);
							if (r_value > 180) {
								r_value = 180;
							}
							break;
						}
					case RANGER:
					case RANGERGM: {
							// 1 150 150
							r_value = ((level * 5) + 5);
							if (r_value > 150) {
								r_value = 150;
							}
							break;
						}
					// Pure
					// No skill classes
					// Melee
					case ROGUE:
					case ROGUEGM:
					case MONK:
					case MONKGM:
					// Priest
					case DRUID:
					case DRUIDGM:
					case SHAMAN:
					case SHAMANGM:
					case CLERIC:
					case CLERICGM:
					// Pure
					case NECROMANCER:
					case NECROMANCERGM:
					case WIZARD:
					case WIZARDGM:
					case MAGICIAN:
					case MAGICIANGM:
					case ENCHANTER:
					case ENCHANTERGM:
					// Hybrid
					case BEASTLORD:
					case BEASTLORDGM:
					case BARD:
					case BARDGM:
					default:
						break;
				}
				break;
			}
		case DISARM: {
				switch (class_) {
					// Melee
					case WARRIOR:
					case WARRIORGM: {
							// 35 200 200
							r_value = ((level * 5) + 5);
							if (level < 35) {
								r_value = 0;
							}
							if (r_value > 200) {
								r_value = 200;
							}
							break;
						}
					case ROGUE:
					case ROGUEGM:
					case MONK:
					case MONKGM: {
							// 27 200 200
							r_value = ((level * 5) + 5);
							if (level < 27) {
								r_value = 0;
							}
							if (r_value > 200) {
								r_value = 200;
							}
							break;
						}
					case BERSERKER:
					case BERSERKERGM: {
							// 35 65 65
							r_value = ((level * 5) + 5);
							if (level < 35) {
								r_value = 0;
							}
							if (r_value > 65) {
								r_value = 65;
							}
							break;
						}
					// Priest
					// Hybrid
					case PALADIN:
					case PALADINGM:
					case SHADOWKNIGHT:
					case SHADOWKNIGHTGM: {
							// 40 70 70
							r_value = ((level * 5) + 5);
							if (level < 40) {
								r_value = 0;
							}
							if (r_value > 70) {
								r_value = 70;
							}
							break;
						}
					case RANGER:
					case RANGERGM: {
							// 35 55 55
							r_value = ((level * 5) + 5);
							if (level < 35) {
								r_value = 0;
							}
							if (r_value > 55) {
								r_value = 55;
							}
							break;
						}
					// Pure
					// No skill classes
					// Melee
					// Priest
					case DRUID:
					case DRUIDGM:
					case SHAMAN:
					case SHAMANGM:
					case CLERIC:
					case CLERICGM:
					// Pure
					case NECROMANCER:
					case NECROMANCERGM:
					case WIZARD:
					case WIZARDGM:
					case MAGICIAN:
					case MAGICIANGM:
					case ENCHANTER:
					case ENCHANTERGM:
					// Hybrid
					case BARD:
					case BARDGM:
					case BEASTLORD:
					case BEASTLORDGM:
					default:
						break;
				}
				break;
			}
////////////////////////////////////////////////////////
		default:
			#if EQDEBUG
			std::cout << "Unknown Defensive skill: " << skillid << std::endl;
			#endif
			break;
	}// Switch skill
	if (r_value > 252) {
		r_value = 252;
	}
	return r_value;
}

uint16 Mob::MaxSkill_arcane(uint16 skillid, uint16 class_, uint16 level) const
{
	uint16 r_value = 0;
	switch (skillid) {
		case MEDITATE:
		case ABJURE:
		case ALTERATION:
		case CHANNELING:
		case CONJURATION:
		case DIVINATION:
		case EVOCATION: {
				r_value = ((level * 5) + 5);
				switch (class_) {
					// Hybrid
					case RANGER:
					case RANGERGM: {
							// 9 235 235
							// Channel 9 200 215
							// Med 12 185 235
							if (level < 9) {
								r_value = 0;
							}
							if (level < 12 && skillid == MEDITATE) {
								r_value = 0;
							}
							if (r_value > 0 && skillid == CHANNELING) {
								if ( level < 51 && r_value > 200) {
									r_value = 200;
								}
								if (r_value > 215) {
									r_value = 215;
								}
							}
							if (r_value > 0 && skillid == MEDITATE) {
								if ( level < 51 && r_value > 185) {
									r_value = 185;
								}
								if (r_value > 235) {
									r_value = 235;
								}
							}
							break;
						}
					case BEASTLORD:
					case BEASTLORDGM:
					case PALADIN:
					case PALADINGM:
					case SHADOWKNIGHT:
					case SHADOWKNIGHTGM: {
							// 9 235 235
							// Channel 9 200 220
							// Med 12 185 235
							if (level < 9) {
								r_value = 0;
							}
							if (level < 12 && skillid == MEDITATE) {
								r_value = 0;
							}
							if (r_value > 0 && skillid == CHANNELING) {
								if ( level < 51 && r_value > 185) {
									r_value = 185;
								}
								if (r_value > 220) {
									r_value = 220;
								}
							}
							if (r_value > 0 && skillid == MEDITATE) {
								if ( level < 51 && r_value > 185) {
									r_value = 185;
								}
								if (r_value > 235) {
									r_value = 235;
								}
							}
							break;
						}
					// Priest
					case CLERIC:
					case CLERICGM:
					case DRUID:
					case DRUIDGM:
					case SHAMAN:
					case SHAMANGM: {
							// 1 235 235
							// Channel 4 200 220
							// Med 8 235 252
							if (level < 4 && skillid == CHANNELING) {
								r_value = 0;
							}
							if (level < 8 && skillid == MEDITATE) {
								r_value = 0;
							}
							if (r_value > 0 && skillid == CHANNELING) {
								if ( level < 51 && r_value > 200) {
									r_value = 200;
								}
								if (r_value > 220) {
									r_value = 220;
								}
							}
							if (r_value > 0 && skillid == MEDITATE) {
								if ( level < 51 && r_value > 235) {
									r_value = 235;
								}
								if (r_value > 252) {
									r_value = 252;
								}
							}
							break;
						}
					// Int caster
					case ENCHANTER:
					case ENCHANTERGM:
					case MAGICIAN:
					case MAGICIANGM:
					case NECROMANCER:
					case NECROMANCERGM:
					case WIZARD:
					case WIZARDGM: {
							// 1 235 235
							// Channel 1 200 220
							// Med 4 235 252
							if (level < 4 && skillid == MEDITATE) {
								r_value = 0;
							}
							if (r_value > 0 && skillid == CHANNELING) {
								if ( level < 51 && r_value > 200) {
									r_value = 200;
								}
								if (r_value > 220) {
									r_value = 220;
								}
							}
							if (r_value > 0 && skillid == MEDITATE) {
								if ( level < 51 && r_value > 235) {
									r_value = 235;
								}
								if (r_value > 252) {
									r_value = 252;
								}
							}
							break;
						}
					case BARD:
					case BARDGM: {
							r_value = 0;
							if (level > 9 && skillid == MEDITATE) {
								r_value = 1;
							}
							break;
						}
					default:
						// Unknown class
						r_value = 0;
						break;
				}// Class Switch
				break;
			}
		case SPECIALIZE_ABJURE:
		case SPECIALIZE_ALTERATION:
		case SPECIALIZE_CONJURATION:
		case SPECIALIZE_DIVINATION:
		case SPECIALIZE_EVOCATION: {
				r_value = ((level * 5) + 5);
				switch (class_) {
					// Non-int casters
					case CLERIC:
					case CLERICGM:
					case DRUID:
					case DRUIDGM:
					case SHAMAN:
					case SHAMANGM:
						if (level < 30) {
							r_value = 0;
							break;
						}
					// Int caster
					case ENCHANTER:
					case ENCHANTERGM:
					case MAGICIAN:
					case MAGICIANGM:
					case NECROMANCER:
					case NECROMANCERGM:
					case WIZARD:
					case WIZARDGM: {
							if (level < 20) {
								r_value = 0;
								break;
							}
							//make sure only 1 skill can be over 50
							uint16 hskill = 0;
							uint16 high = 0;
							uint16 cur;
							cur = GetSkill(SPECIALIZE_ABJURE);
							if (cur > high) {
								hskill = SPECIALIZE_ABJURE;
								high = cur;
							}
							cur = GetSkill(SPECIALIZE_ALTERATION);
							if (cur > high) {
								hskill = SPECIALIZE_ALTERATION;
								high = cur;
							}
							cur = GetSkill(SPECIALIZE_CONJURATION);
							if (cur > high) {
								hskill = SPECIALIZE_CONJURATION;
								high = cur;
							}
							cur = GetSkill(SPECIALIZE_DIVINATION);
							if (cur > high) {
								hskill = SPECIALIZE_DIVINATION;
								high = cur;
							}
							cur = GetSkill(SPECIALIZE_EVOCATION);
							if (cur > high) {
								hskill = SPECIALIZE_EVOCATION;
								high = cur;
							}
							if (high > 50 && hskill != skillid) {
								r_value = 50;
								break;
							}
							if (r_value > 200) {
								r_value = 200;
							}
							break;
						}
					default: {
							r_value = 0;
							break;
						}
				}// Class Switch
				break;
			}
		case RESEARCH: {
				r_value = ((level * 5) + 5);
				switch (class_) {
					// Int caster
					case ENCHANTER:
					case ENCHANTERGM:
					case MAGICIAN:
					case MAGICIANGM:
					case NECROMANCER:
					case NECROMANCERGM:
					case WIZARD:
					case WIZARDGM: {
							// Res 16 200 200
							if (level < 16) {
								r_value = 0;
							}
							if (r_value > 200) {
								r_value = 200;
							}
							// FIXME Only let one SPEC go above what ever limit theres supposed to be
							break;
						}
					default: {
							r_value = 0;
							break;
						}
				}// Class Switch
				break;
			}
		case BRASS_INSTRUMENTS:
		case SINGING:
		case STRINGED_INSTRUMENTS:
		case WIND_INSTRUMENTS:
		case PERCUSSION_INSTRUMENTS: {
				switch (class_) {
					case BARD:
					case BARDGM: {
							r_value = ((level * 5) + 5);
							if (level < 5 && skillid == PERCUSSION_INSTRUMENTS) {
								r_value = 0;
							}
							if (level < 8 && skillid == STRINGED_INSTRUMENTS) {
								r_value = 0;
							}
							if (level < 11 && skillid == BRASS_INSTRUMENTS) {
								r_value = 0;
							}
							if (level < 14 && skillid == WIND_INSTRUMENTS) {
								r_value = 0;
							}
							if (r_value > 235) {
								r_value = 235;
							}
							break;
						}
					default:
						r_value = 0;
						break;
				}// Class Switch
				break;
			}
////////////////////////////////////////////////////////
		default:
			#if EQDEBUG
			std::cout << "Unknown arcane skill: " << skillid << std::endl;
			#endif
			break;
	}// Switch skill
	if (r_value > 252) {
		r_value = 252;
	}
	return r_value;
}

uint16 Mob::MaxSkill_class(uint16 skillid, uint16 class_, uint16 level) const
{
	uint16 r_value = 0;
	switch (skillid) {
		// Rogue
		case APPLY_POISON:
		case MAKE_POISON:
		case PICK_POCKETS:
		case BACKSTAB: {
				switch (class_) {
					// Melee
					case ROGUE:
					case ROGUEGM: {
							r_value = ((level * 5) + 5);
							switch (skillid) {
								case APPLY_POISON: {
										// 18 200 200
										if (level < 18) {
											r_value = 0;
										}
										if (r_value > 200) {
											r_value = 200;
										}
										break;
									}
								case MAKE_POISON: {
										// 20 200 250
										if (level < 20) {
											r_value = 0;
										}
										if (level < 51 && r_value > 200) {
											r_value = 200;
										}
										if (r_value > 250) {
											r_value = 250;
										}
										break;
									}
								case PICK_POCKETS: {
										// 7 200 210
										if (level < 7) {
											r_value = 0;
										}
										if (level < 51 && r_value > 200) {
											r_value = 200;
										}
										if (r_value > 210) {
											r_value = 210;
										}
										break;
									}
								case BACKSTAB: {
										// 10 200 225
										if (level < 10) {
											r_value = 0;
										}
										if (level < 51 && r_value > 200) {
											r_value = 200;
										}
										if (r_value > 225) {
											r_value = 225;
										}
										break;
									}
								default:
									r_value = 0;
									break;
							}
							break;
						}
					default:
						r_value = 0;
						break;
				}// Class Switch
				break;
			}
		// Monk
		case BLOCKSKILL: {
				switch (class_) {
					case BEASTLORD:
					case BEASTLORDGM: {
							r_value = (((level - 25) * 5) + 5);
							// 12 200 230
							if (level < 25) {
								r_value = 0;
							}
							if (level < 51 && r_value > 200) {
								r_value = 200;
							}
							if (r_value > 230) {
								r_value = 230;
							}
							break;
						}
					case MONK:
					case MONKGM: {
							r_value = ((level * 5) + 5);
							// 12 200 230
							if (level < 12) {
								r_value = 0;
							}
							if (level < 51 && r_value > 200) {
								r_value = 200;
							}
							if (r_value > 230) {
								r_value = 230;
							}
							break;
						}
				}
				break;
			}
		case FEIGN_DEATH:
		case MEND:
		case DRAGON_PUNCH:
		case EAGLE_STRIKE:
		case FLYING_KICK:
		case ROUND_KICK:
		case TIGER_CLAW: {
				switch (class_) {
					case MONK:
					case MONKGM: {
							r_value = ((level * 5) + 5);
							switch (skillid) {
								case MEND: {
										// 1 200 200
										if (r_value > 200) {
											r_value = 200;
										}
										break;
									}
								case ROUND_KICK: {
										// 5 200 225
										if (level < 5) {
											r_value = 0;
										}
										if (level < 51 && r_value > 200) {
											r_value = 200;
										}
										if (r_value > 225) {
											r_value = 225;
										}
										break;
									}
								case TIGER_CLAW: {
										// 10 200 225
										if (level < 10) {
											r_value = 0;
										}
										if (level < 51 && r_value > 200) {
											r_value = 200;
										}
										if (r_value > 225) {
											r_value = 225;
										}
										break;
									}
								case FEIGN_DEATH: {
										// 17 200 200
										if (level < 17) {
											r_value = 0;
										}
										if (r_value > 200) {
											r_value = 200;
										}
										break;
									}
								case EAGLE_STRIKE: {
										// 20 200 225
										if (level < 20) {
											r_value = 0;
										}
										if (level < 51 && r_value > 200) {
											r_value = 200;
										}
										if (r_value > 225) {
											r_value = 225;
										}
										break;
									}
								case DRAGON_PUNCH: {
										// 25 200 225
										if (level < 25) {
											r_value = 0;
										}
										if (level < 51 && r_value > 200) {
											r_value = 200;
										}
										if (r_value > 225) {
											r_value = 225;
										}
										break;
									}
								case FLYING_KICK: {
										// 30 200 225
										if (level < 30) {
											r_value = 0;
										}
										if (level < 51 && r_value > 200) {
											r_value = 200;
										}
										if (r_value > 225) {
											r_value = 225;
										}
										break;
									}
								default:
									r_value = 0;
									break;
							}
							break;
						}
					default:
						r_value = 0;
						break;
				}// Class Switch
				break;
			}
		//Berzerkers
		case BERSERKING: {
				switch (class_) {
					case BERSERKER:
					case BERSERKERGM: {
							r_value = ((level * 5) + 5);
							if (r_value > 200) {
								r_value = 200;
							}
						}
					default:
						r_value = 0;
						break;
				}
				break;
			}
		// Shaman
		case ALCHEMY: {
				switch (class_) {
					case SHAMAN:
					case SHAMANGM: {
							// 25 130 180
							r_value = ((level * 5) + 5);
							if (level < 25) {
								r_value = 0;
							}
							if (level < 51 && r_value > 130) {
								r_value = 130;
							}
							if (r_value > 180) {
								r_value = 180;
							}
							break;
						}
					default:
						r_value = 0;
						break;
				}// Class Switch
				break;
			}
///////////////////////////////////////////
//////////////////////////////////////////
// Shared skill
		// Shared Rogue
		case HIDE: {
				switch (class_) {
					// True class
					case ROGUE:
					case ROGUEGM: {
							r_value = ((level * 5) + 5);
							if (r_value > 200) {
								r_value = 200;
							}
							break;
						}
					// Hybrids
					case RANGER:
					case RANGERGM:
					case SHADOWKNIGHT:
					case SHADOWKNIGHTGM: { //75 cap
							if (level >= 35) {
								r_value = (((level - 35) * 5) + 5);
								if (r_value > 75) {
									r_value = 75;
								}
							}
							break;
						}
					case BARD:
					case BARDGM: { //40 cap
							if (level > 25) {
								r_value = (((level - 25) * 5) + 5);
								if (r_value > 40) {
									r_value = 40;
								}
							}
							break;
						}
					default:
						r_value = 0;
						break;
				}// Class Switch
				break;
			}
		case SNEAK: {
				switch (class_) {
					// True class
					case ROGUE:
					case ROGUEGM: {
							r_value = ((level * 5) + 5);
							if (r_value > 200) {
								r_value = 200;
							}
							break;
						}
					// Hybrids
					case MONK:
					case MONKGM: { //113 cap
							if (level >= 8) {
								r_value = (((level - 8) * 5) + 5);
								if (r_value > 113) {
									r_value = 113;
								}
							}
							break;
						}
					case RANGER:
					case RANGERGM: { //75 cap
							if (level >= 10) {
								r_value = (((level - 10) * 5) + 5);
								if (r_value > 75) {
									r_value = 75;
								}
							}
							break;
						}
					case BARD:
					case BARDGM: { //75 cap
							if (level >= 17) {
								r_value = (((level - 17) * 5) + 5);
								if (r_value > 75) {
									r_value = 75;
								}
							}
							break;
						}
					case BEASTLORD:
					case BEASTLORDGM: { //50 cap
							if (level >= 50) {
								r_value = (((level - 50) * 5) + 5);
								if (r_value > 50) {
									r_value = 50;
								}
							}
							break;
						}
					default:
						r_value = 0;
						break;
				}// Class Switch
				break;
			}
		case SENSE_TRAPS:
		case PICK_LOCK:
		case DISARM_TRAPS: {
				switch (class_) {
					// True class
					case ROGUE:
					case ROGUEGM: {
							r_value = ((level * 5) + 5);
							if (r_value > 200) {
								r_value = 200;
							}
							break;
						}
					// Hybrids
					case BARD:
					case BARDGM: { //100 cap
							if (level >= 30) { //this is wrong I think...
								r_value = (((level - 30) * 5) + 5);
								if (r_value > 100) {
									r_value = 100;
								}
							}
							break;
						}
					default:
						r_value = 0;
						break;
				}// Class Switch
				break;
			}
		case SAFE_FALL: {
				switch (class_) {
					// Hybrids
					case BARD:
					case BARDGM: { //40 cap
							if (level >= 24) {
								r_value = (((level - 24) * 5) + 5);
								if (r_value > 40) {
									r_value = 40;
								}
							}
							break;
						}
					// Melee
					case MONK:
					case MONKGM: {
							if (level >= 3) {
								r_value = (((level - 3) * 5) + 5);
								if (r_value > 200) {
									r_value = 200;
								}
							}
							break;
						}
					case ROGUE:
					case ROGUEGM: { //100 cap
							if (level >= 12) {
								r_value = (((level - 12) * 5) + 5);
								if (r_value > 100) {
									r_value = 100;
								}
							}
							break;
						}
					default:
						r_value = 0;
						break;
				}// Class Switch
				break;
			}
		case INTIMIDATION: {
				switch (class_) {
					case BARD:
					case BARDGM: { //100 cap
							if (level >= 26) {
								r_value = (((level - 26) * 5) + 5);
								if (r_value > 100) {
									r_value = 100;
								}
							}
							break;
						}
					// Melee
					case MONK:
					case MONKGM: {
							if (level >= 18) {
								r_value = (((level - 18) * 5) + 5);
								if (r_value > 200) {
									r_value = 200;
								}
							}
							break;
						}
					case BERSERKER:
					case BERSERKERGM: {
							if (level >= 20) {
								r_value = (((level - 20) * 5) + 5);
								if (r_value > 200) {
									r_value = 200;
								}
							}
							break;
						}
					case ROGUE:
					case ROGUEGM: {
							if (level >= 22) {
								r_value = (((level - 22) * 5) + 5);
								if (r_value > 200) {
									r_value = 200;
								}
							}
							break;
						}
					default:
						r_value = 0;
						break;
				}// Class Switch
				break;
			}
		// Druid/Ranger/Bard
		case FORAGE: {
				switch (class_) {
					case RANGER:
					case RANGERGM: {
							if (level > 3) {
								r_value = (((level - 3) * 5) + 5);
								if (r_value > 200) {
									r_value = 200;
								}
							}
							break;
						}
					case DRUID:
					case DRUIDGM: {
							r_value = ((level * 5) + 5);
							if (r_value > 200) {
								r_value = 200;
							}
							break;
						}
					case MONK:
					case MONKGM:
					case BARD:
					case BARDGM:
						r_value = 55;
						break;
					default:
						r_value = 50;
						break;
				}// Class Switch
				break;
			}
		case TRACKING: {
				switch (class_) {
					case RANGER:
					case RANGERGM:
					case BARD:
					case BARDGM:
					case DRUID:
					case DRUIDGM:
						r_value = 200;
						break;
					default:
						r_value = 0;
						break;
				}// Class Switch
				break;
			}
////////////////////////////////////////////////////////
		default:
			#if EQDEBUG
			std::cout << "Unknown class skill: " << skillid << std::endl;
			#endif
			break;
	}// Switch skill
	if (r_value > 252) {
		r_value = 252;
	}
	return r_value;
}

uint16 Mob::MaxSkill(uint16 skillid, uint16 class_, uint16 level) const
{
	uint16 r_value = 0;
	switch (skillid) {
		case _1H_BLUNT:
		case _2H_BLUNT:
		case PIERCING:
		case HAND_TO_HAND:
		case _1H_SLASHING:
		case _2H_SLASHING: {
				r_value = MaxSkill_weapon(skillid, class_, level);
				break;
			}
		case OFFENSE:
		case THROWING:
		case ARCHERY:
		case DOUBLE_ATTACK:
		case DUAL_WIELD:
		case KICK:
		case BASH: {
				r_value = MaxSkill_offensive(skillid, class_, level);
				break;
			}
		case DEFENSE:
		case PARRY:
		case RIPOSTE:
		case DODGE:
		case TAUNT:
		case DISARM: {
				r_value = MaxSkill_defensive(skillid, class_, level);
				break;
			}
		case MEDITATE:
		case ABJURE:
		case ALTERATION:
		case CHANNELING:
		case CONJURATION:
		case DIVINATION:
		case EVOCATION:
		case SPECIALIZE_ABJURE:
		case SPECIALIZE_ALTERATION:
		case SPECIALIZE_CONJURATION:
		case SPECIALIZE_DIVINATION:
		case SPECIALIZE_EVOCATION:
		case RESEARCH:
		case BRASS_INSTRUMENTS:
		case SINGING:
		case STRINGED_INSTRUMENTS:
		case WIND_INSTRUMENTS:
		case PERCUSSION_INSTRUMENTS: {
				r_value = MaxSkill_arcane(skillid, class_, level);
				break;
			}
///////////////////////////////////////////
///////////////////////////////////////////
// Class skills
		// Rogue
		case APPLY_POISON:
		case MAKE_POISON:
		case PICK_POCKETS:
		case BACKSTAB:
		// Monk
		case FEIGN_DEATH:
		case MEND:
		case DRAGON_PUNCH:
		case EAGLE_STRIKE:
		case FLYING_KICK:
		case ROUND_KICK:
		case TIGER_CLAW:
		case BLOCKSKILL:
		case ALCHEMY:
		case HIDE:
		case SNEAK:
		case SENSE_TRAPS:
		case PICK_LOCK:
		case DISARM_TRAPS:
		case SAFE_FALL:
		case INTIMIDATION:
		// Druid/Ranger/Bard
		case FORAGE:
		case TRACKING: {
				r_value = MaxSkill_class(skillid, class_, level);
				break;
			}
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
		case FISHING: {
				// Check for Any Trade above 200, check for X (aa skill) Trades above 200
				r_value = 250;
				break;
			}
/////////////////////////////////////
/////////////////////////////////////
		// Gnome
		case TINKERING: {
				if ( race == GNOME && level > 24 ) {
					r_value = ((level * 5) + 5);
					break;
				}
				r_value = 0;
				break;
			}
/////////////////////////////////////////
// Common
/////////////////////////////////////////
		case BIND_WOUND: {
				switch (class_) {
					case BARD:
					case BARDGM: {
							r_value = ((level * 5) + 5);
							if (level >= 50) {
								if (r_value > 210) {
									r_value = 210;
								}
							} else {
								if (r_value > 200) {
									r_value = 200;
								}
							}
							break;
						}
					case CLERIC:
					case CLERICGM: {
							r_value = ((level * 5) + 5);
							if (level >= 50) {
								if (r_value > 201) {
									r_value = 201;
								}
							} else {
								if (r_value > 200) {
									r_value = 200;
								}
							}
							break;
						}
					case DRUID:
					case DRUIDGM:
					case SHAMAN:
					case SHAMANGM: {
							r_value = ((level * 5) + 5);
							if (r_value > 200) {
								r_value = 200;
							}
							break;
						}
					case MAGICIAN:
					case MAGICIANGM:
					case ENCHANTER:
					case ENCHANTERGM:
					case NECROMANCER:
					case NECROMANCERGM:
					case WIZARD:
					case WIZARDGM: {
							r_value = ((level * 5) + 5);
							if (r_value > 100) {
								r_value = 100;
							}
							break;
						}
					case BEASTLORD:
					case BEASTLORDGM:
					case BERSERKER:
					case BERSERKERGM:
					case MONK:
					case MONKGM: {
							r_value = ((level * 5) + 5);
							if (level >= 50) {
								if (r_value > 210) {
									r_value = 210;
								}
							} else {
								if (r_value > 200) {
									r_value = 200;
								}
							}
							break;
						}
					case PALADIN:
					case PALADINGM: {
							if (level > 10) {
								r_value = (((level - 10) * 5) + 5);
								if (level >= 50) {
									if (r_value > 210) {
										r_value = 210;
									}
								} else {
									if (r_value > 200) {
										r_value = 200;
									}
								}
							}
							break;
						}
					case RANGER:
					case RANGERGM: {
							if (level > 15) {
								r_value = (((level - 15) * 5) + 5);
								if (level >= 50) {
									if (r_value > 200) {
										r_value = 200;
									}
								} else {
									if (r_value > 150) {
										r_value = 150;
									}
								}
							}
							break;
						}
					case ROGUE:
					case ROGUEGM: {
							r_value = ((level * 5) + 5);
							if (level >= 50) {
								if (r_value > 210) {
									r_value = 210;
								}
							} else {
								if (r_value > 176) {
									r_value = 176;
								}
							}
							break;
						}
					case SHADOWKNIGHT:
					case SHADOWKNIGHTGM: {
							r_value = ((level * 5) + 5);
							if (level >= 50) {
								if (r_value > 200) {
									r_value = 200;
								}
							} else {
								if (r_value > 150) {
									r_value = 150;
								}
							}
							break;
						}
					case WARRIOR:
					case WARRIORGM: {
							if (level > 5) {
								r_value = (((level - 5) * 5) + 5);
								if (level >= 50) {
									if (r_value > 210) {
										r_value = 210;
									}
								} else {
									if (r_value > 175) {
										r_value = 175;
									}
								}
							}
							break;
						}
					default:
						r_value = 0;
						break;
				}
				break;
			}
		case SENSE_HEADING:
		case SWIMMING:
		case ALCOHOL_TOLERANCE:
		case BEGGING: {
				r_value = 5 + (level * 5);
				if (r_value > 200) {
					r_value = 200;
				}
				break;
			}
		//case BERSERKING:
		default: {
				// Unknown skill we should like print something to a log/debug here
				r_value = 0;
				break;
			}
	}
	if (r_value >= 253) {
		r_value = 252;
	}
	return  r_value;
}

