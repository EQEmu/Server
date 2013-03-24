#ifndef STRING_IDS
#define STRING_IDS

//These strings are loaded from eqstr_us.txt, but may vary between client versions. Maybe we could make this an enum that's dependent on the client version?
#define GENERIC_9_STRINGS			1		//%1 %2 %3 %4 %5 %6 %7 %8 %9
#define TARGET_OUT_OF_RANGE			100		//Your target is out of range, get closer!
#define TARGET_NOT_FOUND			101		//Target player not found.
#define CANNOT_BIND					105		//You cannot form an affinity with this area.  Try a city.
#define SPELL_DOES_NOT_WORK_HERE	106		//This spell does not work here.
#define SPELL_DOES_NOT_WORK_PLANE	107		//This spell does not work on this plane.
#define CANT_SEE_TARGET				108		//You cannot see your target.
#define MGB_STRING					113		//The next group buff you cast will hit all targets in range.
#define ESCAPE                      114     //You escape from combat, hiding yourself from view.
#define TARGET_TOO_FAR				124		//Your target is too far away, get closer!
#define PROC_TOOLOW					126		//Your will is not sufficient to command this weapon.
#define PROC_PETTOOLOW				127		//Your pet's will is not sufficient to command its weapon.
#define DOORS_LOCKED				130		//It's locked and you're not holding the key.
#define DOORS_CANT_PICK				131		//This lock cannot be picked.
#define DOORS_INSUFFICIENT_SKILL	132		//You are not sufficiently skilled to pick this lock.
#define DOORS_GM					133		//You opened the locked door with your magic GM key.
#define ITEMS_INSUFFICIENT_LEVEL	136		//You are not sufficient level to use this item.
#define GAIN_XP						138		//You gain experience!!
#define GAIN_GROUPXP				139		//You gain party experience!!
#define BOW_DOUBLE_DAMAGE			143		//Your bow shot did double dmg.
#define FORAGE_GRUBS				150		//You have scrounged up some fishing grubs.
#define FORAGE_WATER				151		//You have scrounged up some water.
#define FORAGE_FOOD					152		//You have scrounged up some food.
#define FORAGE_DRINK				153		//You have scrounged up some drink.
#define FORAGE_NOEAT				154		//You have scrounged up something that doesn't look edible.
#define FORAGE_FAILED				155		//You fail to locate any food nearby.
#define ALREADY_FISHING				156		//You are already fishing!
#define FISHING_NO_POLE				160		//You can't fish without a fishing pole, go buy one.
#define FISHING_EQUIP_POLE			161		//You need to put your fishing pole in your primary hand.
#define FISHING_NO_BAIT				162		//You can't fish without fishing bait, go buy some.
#define FISHING_CAST				163		//You cast your line.
#define NOT_SCARING					164		//You're not scaring anyone.
#define FISHING_STOP				165		//You stop fishing and go on your way.
#define FISHING_LAND				166		//Trying to catch land sharks perhaps?
#define FISHING_LAVA				167		//Trying to catch a fire elemental or something?
#define FISHING_FAILED				168		//You didn't catch anything.
#define FISHING_POLE_BROKE			169		//Your fishing pole broke!
#define FISHING_SUCCESS				170		//You caught, something...
#define FISHING_SPILL_BEER			171		//You spill your beer while bringing in your line.
#define FISHING_LOST_BAIT			172		//You lost your bait!
#define SPELL_FIZZLE				173		//Your spell fizzles!
#define MUST_EQUIP_ITEM				179		//You cannot use this item unless it is equipped.
#define MISS_NOTE					180		//You miss a note, bringing your song to a close!
#define CANNOT_USE_ITEM				181		//Your race, class, or deity cannot use this item.
#define ITEM_OUT_OF_CHARGES			182		//Item is out of charges.
#define TARGET_NO_MANA				191		//Your target has no mana to affect
#define TARGET_GROUP_MEMBER			196		//You must first target a group member.
#define INSUFFICIENT_MANA			199		//Insufficient Mana to cast this spell!
#define SAC_TOO_LOW					203		//This being is not a worthy sacrifice.
#define SAC_TOO_HIGH				204		//This being is too powerful to be a sacrifice.
#define CANNOT_SAC_SELF				205		//You cannot sacrifice yourself.
#define SILENCED_STRING				207		//You *CANNOT* cast spells, you have been silenced!
#define CANNOT_AFFECT_PC			210		//That spell can not affect this target PC.
#define SPELL_NEED_TAR				214		//You must first select a target for this spell!
#define ONLY_ON_CORPSES				221		//This spell only works on corpses.
#define CANT_DRAIN_SELF				224		//You can't drain yourself!
#define CORPSE_NOT_VALID			230		//This corpse is not valid.
#define CORPSE_TOO_OLD				231		//This player cannot be resurrected. The corpse is too old.
#define CAST_OUTDOORS				234		//You can only cast this spell in the outdoors.
#define SPELL_RECAST				236		//Spell recast time not yet met.
#define SPELL_RECOVERY				237		//Spell recovery time not yet met.
#define CANNOT_MEZ					239		//Your target cannot be mesmerized.
#define CANNOT_MEZ_WITH_SPELL		240		//Your target cannot be mesmerized (with this spell).
#define IMMUNE_STUN					241		//Your target is immune to the stun portion of this effect.
#define IMMUNE_ATKSPEED				242		//Your target is immune to changes in its attack speed.
#define IMMUNE_FEAR					243		//Your target is immune to fear spells.
#define IMMUNE_MOVEMENT				244		//Your target is immune to changes in its run speed.
#define ONLY_ONE_PET				246		//You cannot have more than one pet at a time.
#define CANNOT_CHARM_YET			248		//Your target is too high of a level for your charm spell.
#define CANNOT_AFFECT_NPC			251		//That spell can not affect this target NPC.
#define SUSPEND_MINION_HAS_AGGRO	256		//Your pet is the focus of something's attention.
#define NO_PET						255		//You do not have a pet.
#define CORPSE_CANT_SENSE			262		//You cannot sense any corpses for this PC in this zone.
#define SPELL_NO_HOLD				263		//Your spell did not take hold.
#define CANNOT_CHARM				267		//This NPC cannot be charmed.
#define SPELL_NO_EFFECT				268		//Your target looks unaffected.
#define NO_INSTRUMENT_SKILL			269		//Stick to singing until you learn to play this instrument.
#define REGAIN_AND_CONTINUE			270		//You regain your concentration and continue your casting.
#define SPELL_WOULDNT_HOLD			271		//Your spell would not have taken hold on your target.
#define MISSING_SPELL_COMP			272		//You are missing some required spell components.
#define INVIS_BEGIN_BREAK			275		//You feel yourself starting to appear.
#define DISCIPLINE_CONLOST			278		//You lose the concentration to remain in your fighting discipline.
#define REZ_REGAIN					289		//You regain some experience from resurrection.
#define DUP_LORE					290		//Duplicate lore items are not allowed. 
#define TGB_ON						293		//Target other group buff is *ON*.
#define TGB_OFF						294		//Target other group buff is *OFF*.
#define LDON_SENSE_TRAP1			306		//You do not Sense any traps.
#define TRADESKILL_NOCOMBINE		334		//You cannot combine these items in this container type!
#define TRADESKILL_FAILED			336		//You lacked the skills to fashion the items together.
#define TRADESKILL_TRIVIAL			338		//You can no longer advance your skill from making this item.
#define TRADESKILL_SUCCEED			339		//You have fashioned the items together to create something new!
#define MEND_CRITICAL				349		//You magically mend your wounds and heal considerable damage.
#define MEND_SUCCESS				350		//You mend your wounds and heal some damage.
#define MEND_WORSEN					351		//You have worsened your wounds!
#define MEND_FAIL					352		//You have failed to mend your wounds.
#define LDON_SENSE_TRAP2			367		//You have not detected any traps.
#define LOOT_LORE_ERROR				371		//You cannot loot this Lore Item.  You already have one.
#define PICK_LORE					379		//You cannot pick up a lore item you already possess.
#define CONSENT_DENIED				390		//You do not have consent to summon that corpse.
#define DISCIPLINE_RDY				393		//You are ready to use a new discipline now.
#define CONSENT_INVALID_NAME		397		//Not a valid consent name.
#define CONSENT_NPC					398		//You cannot consent NPC\'s.
#define CONSENT_YOURSELF			399		//You cannot consent yourself.
#define SONG_NEEDS_DRUM				405		//You need to play a percussion instrument for this song
#define SONG_NEEDS_WIND				406		//You need to play a wind instrument for this song
#define SONG_NEEDS_STRINGS			407		//You need to play a stringed instrument for this song
#define SONG_NEEDS_BRASS			408		//You need to play a brass instrument for this song
#define AA_GAIN_ABILITY				410		//You have gained the ability "%T1" at a cost of %2 ability %T3.
#define AA_IMPROVE					411		//You have improved %T1 %2 at a cost of %3 ability %T4.
#define AA_REUSE_MSG				413		//You can use the ability %B1(1) again in %2 hour(s) %3 minute(s) %4 seconds.
#define AA_REUSE_MSG2				414		//You can use the ability %B1(1) again in %2 minute(s) %3 seconds.
#define YOU_HEALED					419		//%1 has healed you for %2 points of damage.
#define BEGINS_TO_GLOW				422		//Your %1 begins to glow.
#define ALREADY_INVIS				423		//%1 tries to cast an invisibility spell on you, but you are already invisible.
#define YOU_ARE_PROTECTED			424		//%1 tries to cast a spell on you, but you are protected.
#define TARGET_RESISTED				425		//Your target resisted the %1 spell.
#define YOU_RESIST					426		//You resist the %1 spell!
#define SUMMONING_CORPSE			429		//Summoning your corpse.
#define SUMMONING_CORPSE_OTHER		430		//Summoning %1's corpse.
#define MISSING_SPELL_COMP_ITEM		433		//You are missing %1.
#define OTHER_HIT_NONMELEE			434		//%1 was hit by non-melee for %2 points of damage.
#define SPELL_WORN_OFF_OF			436		//Your %1 spell has worn off of %2.
#define SPELL_WORN_OFF				437		//Your %1 spell has worn off.
#define INTERRUPT_SPELL				439		//Your spell is interrupted.
#define LOSE_LEVEL					442		//You LOST a level! You are now level %1!
#define GAIN_ABILITY_POINT			446		//You have gained an ability point!  You now have %1 ability point%2.
#define GAIN_LEVEL					447		//You have gained a level! Welcome to level %1!
#define LANG_SKILL_IMPROVED			449		//Your language skills have improved.
#define OTHER_LOOTED_MESSAGE		466		//--%1 has looted a %2--
#define LOOTED_MESSAGE				467		//--You have looted a %1--
#define FACTION_WORST				469		//Your faction standing with %1 could not possibly get any worse.
#define FACTION_WORSE				470		//Your faction standing with %1 got worse.
#define FACTION_BEST				471		//Your faction standing with %1 could not possibly get any better.
#define FACTION_BETTER				472		//Your faction standing with %1 got better.
#define PET_REPORT_HP				488		//I have %1 percent of my hit points left.
#define CORPSE_DECAY1				495		//This corpse will decay in %1 minute(s) %2 seconds.
#define DISC_LEVEL_ERROR			503		//You must be a level %1 ... to use this discipline.
#define DISCIPLINE_CANUSEIN			504		//You can use a new discipline in %1 minutes %2 seconds.
#define PVP_ON						552		//You are now player kill and follow the ways of Discord.
#define GENERIC_STRINGID_SAY		554		//%1 says '%T2'
#define CANNOT_WAKE					555		//%1 tells you, 'I am unable to wake %2, master.'
#define GUILD_NAME_IN_USE			711	//You cannot create a guild with that name, that guild already exists on this server.
#define GM_GAINXP					1002	//[GM] You have gained %1 AXP and %2 EXP (%3).
#define FINISHING_BLOW				1009	//%1 scores a Finishing Blow!!
#define ASSASSINATES				1016	//%1 ASSASSINATES their victim!!
#define CRIPPLING_BLOW				1021	//%1 lands a Crippling Blow!(%2)
#define CRITICAL_HIT				1023	//%1 scores a critical hit! (%2)
#define RESISTS_URGE				1025	//%1 resists their urge to flee.
#define BERSERK_START				1027	//%1 goes into a berserker frenzy!
#define DEATH_PACT					1028	//%1's death pact has been benevolently fulfilled!
#define DIVINE_INTERVENTION			1029	//%1 has been rescued by divine intervention!
#define BERSERK_END					1030	//%1 is no longer berserk.
#define GATES						1031	//%1 Gates.
#define GENERIC_SAY					1032	//%1 says '%2'
#define OTHER_REGAIN_CAST			1033	//%1 regains concentration and continues casting.
#define GENERIC_SHOUT				1034	//%1 shouts '%2'
#define GENERIC_EMOTE				1036	//%1 %2
#define NPC_ENRAGE_START			1042	//%1 has become ENRAGED.
#define NPC_ENRAGE_END				1043	//%1 is no longer enraged.
#define NPC_RAMPAGE					1044	//%1 goes on a RAMPAGE!
#define NPC_FLURRY					1045	//%1 executes a FLURRY of attacks on %2!
#define DISCIPLINE_FEARLESS			1076	//%1 becomes fearless.
#define DUEL_FINISHED				1088	//dont know text
#define EATING_MESSAGE				1091	//Chomp, chomp, chomp...  %1 takes a bite from a %2.
#define DRINKING_MESSAGE			1093	//Glug, glug, glug...  %1 takes a drink from a %2.
#define SUCCESSFUL_TAUNT            1095    //I'll teach you to interfere with me %3. 
#define PET_SIT_STRING				1130	//Changing position, Master.
#define PET_CALMING					1131	//Sorry, Master..calming down.
#define PET_FOLLOWING				1132	//Following you, Master.
#define PET_GUARDME_STRING			1133	//Guarding you, Master.
#define PET_GUARDINGLIFE			1134	//Guarding with my life..oh splendid one.
#define PET_GETLOST_STRING			1135	//As you wish, oh great one.
#define PET_LEADERIS				1136	//My leader is %3.
#define I_FOLLOW_NOONE				1137	//I follow no one.
#define MERCHANT_BUSY				1143	//I'm sorry, I am busy right now.
#define MERCHANT_GREETING			1144	//Welcome to my shop, %3.
#define MERCHANT_HANDY_ITEM1		1145	//Hello there, %3. How about a nice %4?
#define MERCHANT_HANDY_ITEM2		1146	//Greetings, %3. You look like you could use a %4.
#define MERCHANT_HANDY_ITEM3		1147	//Hi there %3, just browsing?  Have you seen the %4 I just got in?
#define MERCHANT_HANDY_ITEM4		1148	//Welcome to my shop, %3. You would probably find a %4 handy.
#define AA_POINT					1197	//point
#define AA_POINTS					1215	//points
#define SPELL_FIZZLE_OTHER			1218	//%1's spell fizzles!
#define MISSED_NOTE_OTHER			1219	//A missed note brings %1's song to a close!
#define CORPSE_DECAY_NOW			1227	//This corpse is waiting to expire.
#define SURNAME_REJECTED			1374	//Your new surname was rejected.  Please try a different name.
#define DUEL_DECLINE				1383	//%1 has declined your challenge to duel to the death.
#define DUEL_ACCEPTED				1384	//%1 has already accepted a duel with someone else.
#define DUEL_CONSIDERING			1385	//%1 is considering a duel with someone else.
#define PLAYER_REGAIN				1394	//You have control of yourself again.
#define REZZ_ALREADY_PENDING		1379	//You were unable to restore the corpse to life, but you may have success with a later attempt.
#define IN_USE						1406	//Someone else is using that.  Try again later.
#define DUEL_FLED					1408	//%1 has defeated %2 in a duel to the death! %3 has fled like a cowardly dog!
#define MEMBER_OF_YOUR_GUILD		1429
#define OFFICER_OF_YOUR_GUILD		1430
#define LEADER_OF_YOUR_GUILD		1431
#define RECEIVED_PLATINUM			1452	//You receive %1 Platinum from %2.
#define RECEIVED_GOLD				1453	//You receive %1 Gold from %2.
#define RECEIVED_SILVER				1454	//You receive %1 Silver from %2.
#define RECEIVED_COPPER				1455	//You receive %1 Copper from %2.
#define STRING_FEIGNFAILED			1456	//%1 has fallen to the ground.
#define DOORS_SUCCESSFUL_PICK		1457	//You successfully picked the lock.
#define PLAYER_CHARMED				1461	//You lose control of yourself!
#define TRADER_BUSY					1468	//That Trader is currently with a customer. Please wait until their transaction is finished.
#define SENSE_CORPSE_DIRECTION		1563	//You sense a corpse in this direction.
#define SUSPEND_MINION_UNSUSPEND	3267	//%1 tells you, 'I live again...'
#define SUSPEND_MINION_SUSPEND		3268	//%1 tells you, 'By your command, master.'
#define ONLY_SUMMONED_PETS			3269	//3269 This effect only works with summoned pets.
#define SUSPEND_MINION_FIGHTING		3270    //Your pet must be at peace, first.
#define TRADESKILL_MISSING_ITEM		3455	//You are missing a %1.
#define TRADESKILL_MISSING_COMPONENTS	3456	//Sorry, but you don't have everything you need for this recipe in your general inventory.
#define TRADESKILL_LEARN_RECIPE		3457	//You have learned the recipe %1!
#define CORPSEDRAG_LIMIT		4061	//You are already dragging as much as you can!
#define CORPSEDRAG_ALREADY		4062	//You are already dragging %1.
#define CORPSEDRAG_SOMEONE_ELSE		4063	//Someone else is dragging %1.
#define CORPSEDRAG_BEGIN		4064	//You begin to drag %1.
#define CORPSEDRAG_STOPALL		4065	//You stop dragging the corpses.
#define CORPSEDRAG_STOP			4066	//You stop dragging the corpse.
#define WHOALL_NO_RESULTS			5029	//There are no players in EverQuest that match those who filters.
#define PETITION_NO_DELETE			5053	//You do not have a petition in the queue.
#define PETITION_DELETED			5054	//Your petition was successfully deleted.
#define GAIN_RAIDEXP				5085	//You gained raid experience!
#define ADVENTURE_COMPLETE			5147	//You received %1 points for successfully completing the adventure.
#define PET_ATTACKING				5501	//%1 tells you, 'Attacking %2 Master.'
#define FATAL_BOW_SHOT				5745	//%1 performs a FATAL BOW SHOT!!
#define MELEE_SILENCE				5806	//You *CANNOT* use this melee ability, you are suffering from amnesia!
#define DISCIPLINE_REUSE_MSG		5807	//You can use the ability %1 again in %2 hour(s) %3 minute(s) %4 seconds.
#define DISCIPLINE_REUSE_MSG2		5808	//You can use the ability %1 again in %2 minute(s) %3 seconds.
#define FAILED_TAUNT                5811    //You have failed to taunt your target.
#define AA_NO_TARGET				5825	//You must first select a target for this ability!
#define GUILD_BANK_CANNOT_DEPOSIT	6097	// Cannot deposit this item.  Containers must be empty, and only one of each LORE and no NO TRADE or TEMPORARY items may be deposited.
#define GUILD_BANK_FULL				6098	// There is no more room in the Guild Bank.
#define GUILD_BANK_TRANSFERRED		6100	// '%1' transferred to Guild Bank from Deposits.
#define GUILD_BANK_EMPTY_HANDS		6108	// You must empty your hands to withdraw from the Guild Bank.
#define GENERIC_STRING				6688	//%1  (used to any basic message)
#define SENTINEL_TRIG_YOU			6724	//You have triggered your sentinel.
#define SENTINEL_TRIG_OTHER			6725	//%1 has triggered your sentinel.
#define IDENTIFY_SPELL				6765	//Item Lore: %1.
#define LDON_DONT_KNOW_TRAPPED		7552	//You do not know if this object is trapped.
#define LDON_HAVE_DISARMED			7553	//You have disarmed %1!
#define LDON_ACCIDENT_SETOFF		7554	//You accidentally set off the trap!
#define LDON_HAVE_NOT_DISARMED		7555	//You have not disarmed %1.
#define LDON_ACCIDENT_SETOFF2		7556	//You accidentally set off the trap!
#define LDON_CERTAIN_TRAP			7557	//You are certain that %1 is trapped.
#define LDON_CERTAIN_NOT_TRAP		7558	//You are certain that %1 is not trapped.
#define LDON_CANT_DETERMINE_TRAP	7559	//You are unable to determine if %1 is trapped.
#define LDON_PICKLOCK_SUCCESS		7560	//You have successfully picked %1!
#define LDON_PICKLOCK_FAILURE		7561	//You have failed to pick %1.
#define LDON_STILL_LOCKED			7562	//You cannot open %1, it is locked.
#define LDON_BASH_CHEST				7563	//%1 try to %2 %3, but do no damage.
#define DOORS_NO_PICK				7564	//You must have a lock pick in your inventory to do this.
#define LDON_NO_LOCKPICK			7564	//You must have a lock pick in your inventory to do this.
#define LDON_WAS_NOT_LOCKED			7565	//%1 was not locked.
#define LDON_WAS_NOT_TRAPPED		7566	//%1 was not trapped
#define GAIN_GROUP_LEADERSHIP_POINT	8585	//
#define GAIN_RAID_LEADERSHIP_POINT	8589	//
#define MAX_GROUP_LEADERSHIP_POINTS	8584	//
#define MAX_RAID_LEADERSHIP_POINTS	8591	//
#define LEADERSHIP_EXP_ON			8653	//
#define LEADERSHIP_EXP_OFF			8654	//
#define CURRENT_SPELL_EFFECTS		8757	//%1's current spell effects: 
#define GAIN_GROUP_LEADERSHIP_EXP	8788	//
#define GAIN_RAID_LEADERSHIP_EXP	8789	//
#define BUFF_MINUTES_REMAINING		8799	//%1 (%2 minutes remaining)
#define YOU_HEAL					9068	//You have healed %1 for %2 points of damage.
#define OTHER_HIT_DOT				9072	//%1 has taken %2 damage from your %3.
#define HIT_NON_MELEE				9073	//%1 hit %2 for %3 points of non-melee damage.
#define SHAKE_OFF_STUN				9077
#define STRIKETHROUGH_STRING		9078	//You strike through your opponent's defenses!
#define SPELL_REFLECT				9082	//%1's spell has been reflected by %2.
#define NEW_SPELLS_AVAIL			9149	//You have new spells available to you.  Check the merchants near your guild master.
#define AE_RAMPAGE					11015	//%1 goes on a WILD RAMPAGE!
#define FACE_ACCEPTED				12028	//Facial features accepted.
#define SPELL_LEVEL_TO_LOW			12048	//You will have to achieve level %1 before you can scribe the %2.
#define ATTACKFAILED				12158	//%1 try to %2 %3, but %4!
#define HIT_STRING					12183	//hit
#define CRUSH_STRING				12191	//crush
#define PIERCE_STRING				12193	//pierce
#define KICK_STRING					12195	//kick
#define STRIKE_STRING				12197	//strike
#define BACKSTAB_STRING				12199	//backstab
#define BASH_STRING					12201	//bash
#define GUILD_NOT_MEMBER			12242	//You are not a member of any guild.
#define MEMBER_OF_X_GUILD			12256
#define OFFICER_OF_X_GUILD			12257
#define LEADER_OF_X_GUILD			12258
#define NOT_IN_A_GUILD				12259
#define TARGET_PLAYER_FOR_GUILD_STATUS		12260
#define GROUP_INVITEE_NOT_FOUND		12268	//You must target a player or use /invite <name> to invite someone to your group.
#define GROUP_INVITEE_SELF			12270	//12270 You cannot invite yourself.
#define NOT_IN_CONTROL				12368	//You do not have control of yourself right now.
#define ALREADY_CASTING				12442	//You are already casting a spell!
#define SENSE_CORPSE_NOT_NAME		12446	//You don't sense any corpses of that name.
#define SENSE_CORPSE_NONE			12447	//You don't sense any corpses.
#define NOT_HOLDING_ITEM			12452	//You are not holding an item!
#define SENSE_UNDEAD				12471	//You sense undead in this direction.
#define SENSE_ANIMAL				12472	//You sense an animal in this direction.
#define SENSE_SUMMONED				12473	//You sense a summoned being in this direction.
#define SENSE_NOTHING				12474	//You don't sense anything.
#define LDON_SENSE_TRAP3			12476	//You don't sense any traps.
#define INTERRUPT_SPELL_OTHER		12478	//%1's casting is interrupted!
#define YOU_HIT_NONMELEE			12481	//You were hit by non-melee for %1 damage.
#define TRACK_LOST_TARGET			12681	//You have lost your tracking target.
#define TRACK_STRAIGHT_AHEAD		12676
#define TRACK_AHEAD_AND_TO			12677
#define TRACK_TO_THE				12678
#define TRACK_BEHIND_AND_TO			12679
#define TRACK_BEHIND_YOU			12680
#define BEAM_SMILE					12501	//%1 beams a smile at %2
#define SONG_ENDS_ABRUPTLY			12686	//Your song ends abruptly.
#define SONG_ENDS					12687	//Your song ends.
#define SONG_ENDS_OTHER				12688	//%1's song ends.
#define SONG_ENDS_ABRUPTLY_OTHER	12689	//%1's song ends abruptly.
#define DIVINE_AURA_NO_ATK			12695	//You can't attack while invulnerable!
#define TRY_ATTACKING_SOMEONE		12696	//Try attacking someone other than yourself, it's more productive.
#define BACKSTAB_WEAPON				12874	//You need a piercing weapon as your primary weapon in order to backstab
#define MORE_SKILLED_THAN_I			12931	//%1 tells you, 'You are more skilled than I!  What could I possibly teach you?'
#define SURNAME_EXISTS				12939	//You already have a surname.  Operation failed.
#define SURNAME_LEVEL				12940	//You can only submit a surname upon reaching the 20th level.  Operation failed.
#define SURNAME_TOO_LONG			12942	//Surname must be less than 20 characters in length.
#define REPORT_ONCE					12945	//You may only submit a report once per time that you zone. Thank you.
#define NOW_INVISIBLE				12950	//%1 is now Invisible.
#define NOW_VISIBLE					12951	//%1 is now Visible.
#define GUILD_NOT_MEMBER2			12966	//You are not in a guild.
#define DISC_LEVEL_USE_ERROR		13004	//You are not sufficient level to use this discipline.
#define TOGGLE_ON					13172	//Asking server to turn ON your incoming tells.
#define TOGGLE_OFF					13173	//Asking server to turn OFF all incoming tells for you.
#define DUEL_INPROGRESS				13251	//You have already accepted a duel with someone else cowardly dog.
#define GENERIC_MISS				15041	//%1 missed %2

#endif
