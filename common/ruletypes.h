


#ifndef RULE_CATEGORY
#define RULE_CATEGORY(name)
#endif
#ifndef RULE_INT
#define RULE_INT(cat, rule, default_value)
#endif
#ifndef RULE_REAL
#define RULE_REAL(cat, rule, default_value)
#endif
#ifndef RULE_BOOL
#define RULE_BOOL(cat, rule, default_value)
#endif
#ifndef RULE_CATEGORY_END
#define RULE_CATEGORY_END()
#endif




RULE_CATEGORY( Character )
RULE_INT ( Character, MaxLevel, 65 )
RULE_BOOL ( Character, PerCharacterQglobalMaxLevel, false) // This will check for qglobal 'CharMaxLevel' character qglobal (Type 5), if player tries to level beyond that point, it will not go beyond that level
RULE_INT ( Character, MaxExpLevel, 0 ) //Sets the Max Level attainable via Experience
RULE_INT ( Character, DeathExpLossLevel, 10 )	// Any level greater than this will lose exp on death
RULE_INT ( Character, DeathExpLossMaxLevel, 255 )	// Any level greater than this will no longer lose exp on death
RULE_INT ( Character, DeathItemLossLevel, 10 )
RULE_INT ( Character, DeathExpLossMultiplier, 3) //Adjust how much exp is lost
RULE_BOOL( Character, UseDeathExpLossMult, false ) //Adjust to use the above multiplier or to use code default.
RULE_INT ( Character, CorpseDecayTimeMS, 10800000 )
RULE_INT ( Character, CorpseResTimeMS, 10800000 ) // time before cant res corpse(3 hours)
RULE_BOOL( Character, LeaveCorpses, true )
RULE_BOOL( Character, LeaveNakedCorpses, false )
RULE_INT ( Character, MaxDraggedCorpses, 2 )
RULE_REAL( Character, DragCorpseDistance, 400) // If the corpse is <= this distance from the player, it won't move
RULE_REAL( Character, ExpMultiplier, 0.5 )
RULE_REAL( Character, AAExpMultiplier, 0.5 )
RULE_REAL( Character, GroupExpMultiplier, 0.5 )
RULE_REAL( Character, RaidExpMultiplier, 0.2 )
RULE_BOOL( Character, UseXPConScaling, true )
RULE_INT ( Character, LightBlueModifier, 40 ) 
RULE_INT ( Character, BlueModifier, 90 )
RULE_INT ( Character, WhiteModifier, 100 )
RULE_INT ( Character, YellowModifier, 125 )
RULE_INT ( Character, RedModifier, 150 ) 
RULE_INT ( Character, AutosaveIntervalS, 300 )	//0=disabled
RULE_INT ( Character, HPRegenMultiplier, 100)
RULE_INT ( Character, ManaRegenMultiplier, 100)
RULE_INT ( Character, EnduranceRegenMultiplier, 100)
RULE_INT ( Character, ConsumptionMultiplier, 100) //item's hunger restored = this value * item's food level, 100 = normal, 50 = people eat 2x as fast, 200 = people eat 2x as slow
RULE_BOOL( Character, HealOnLevel, false)
RULE_BOOL( Character, FeignKillsPet, false)
RULE_INT ( Character, ItemManaRegenCap, 15)
RULE_INT ( Character, ItemHealthRegenCap, 35)
RULE_INT ( Character, ItemDamageShieldCap, 30)
RULE_INT ( Character, ItemAccuracyCap, 150)
RULE_INT ( Character, ItemAvoidanceCap, 100)
RULE_INT ( Character, ItemCombatEffectsCap, 100)
RULE_INT ( Character, ItemShieldingCap, 35)
RULE_INT ( Character, ItemSpellShieldingCap, 35)
RULE_INT ( Character, ItemDoTShieldingCap, 35)
RULE_INT ( Character, ItemStunResistCap, 35)
RULE_INT ( Character, ItemStrikethroughCap, 35)
RULE_INT ( Character, ItemATKCap, 250)
RULE_INT ( Character, ItemHealAmtCap, 250)
RULE_INT ( Character, ItemSpellDmgCap, 250)
RULE_INT ( Character, ItemClairvoyanceCap, 250)
RULE_INT ( Character, ItemDSMitigationCap, 50)
RULE_INT ( Character, ItemEnduranceRegenCap, 15)
RULE_INT ( Character, ItemExtraDmgCap, 150) // Cap for bonuses to melee skills like Bash, Frenzy, etc
RULE_INT ( Character, HasteCap, 100) // Haste cap for non-v3(overhaste) haste.
RULE_INT ( Character, SkillUpModifier, 100) //skill ups are at 100%
RULE_BOOL ( Character, SharedBankPlat, false) //off by default to prevent duping for now
RULE_BOOL ( Character, BindAnywhere, false)
RULE_INT ( Character, RestRegenPercent, 0) // Set to >0 to enable rest state bonus HP and mana regen.
RULE_INT ( Character, RestRegenTimeToActivate, 30) // Time in seconds for rest state regen to kick in.
RULE_BOOL ( Character, RestRegenEndurance, false) // Whether rest regen will work for endurance or not.
RULE_INT ( Character, KillsPerGroupLeadershipAA, 250) // Number of dark blues or above per Group Leadership AA
RULE_INT ( Character, KillsPerRaidLeadershipAA, 250) // Number of dark blues or above per Raid Leadership AA
RULE_INT ( Character, MaxFearDurationForPlayerCharacter, 4) //4 tics, each tic calculates every 6 seconds.
RULE_INT ( Character, MaxCharmDurationForPlayerCharacter, 15)
RULE_INT ( Character, BaseHPRegenBonusRaces, 4352)	//a bitmask of race(s) that receive the regen bonus. Iksar (4096) & Troll (256) = 4352. see common/races.h for the bitmask values
RULE_BOOL ( Character, SoDClientUseSoDHPManaEnd, false)	// Setting this to true will allow SoD clients to use the SoD HP/Mana/End formulas and previous clients will use the old formulas
RULE_BOOL ( Character, UseRaceClassExpBonuses, true)	// Setting this to true will enable Class and Racial experience rate bonuses
RULE_BOOL ( Character, RespawnFromHover, false)		// Use Respawn window, or not.
RULE_INT ( Character, RespawnFromHoverTimer, 300)	// Respawn Window countdown timer, in SECONDS
RULE_BOOL ( Character, UseNewStatsWindow, true) 	// New stats window shows everything
RULE_BOOL ( Character, ItemCastsUseFocus, false) // If true, this allows item clickies to use focuses that have limited max levels on them
RULE_INT ( Character, MinStatusForNoDropExemptions, 80) // This allows status x and higher to trade no drop items.
RULE_INT ( Character, SkillCapMaxLevel, 75 )	// Sets the Max Level used for Skill Caps (from skill_caps table).  -1 makes it use MaxLevel rule value. It is set to 75 because PEQ only has skillcaps up to that level, and grabbing the players' skill past 75 will return 0, breaking all skills past that level. This helps servers with obsurd level caps (75+ level cap) function without any modifications.
RULE_INT ( Character, StatCap, 0 )
RULE_BOOL ( Character, CheckCursorEmptyWhenLooting, true ) // If true, a player cannot loot a corpse (player or NPC) with an item on their cursor
RULE_BOOL ( Character, MaintainIntoxicationAcrossZones, true ) // If true, alcohol effects are maintained across zoning and logging out/in.
RULE_BOOL ( Character, EnableDiscoveredItems, true ) // If enabled, it enables EVENT_DISCOVER_ITEM and also saves character names and timestamps for the first time an item is discovered.
RULE_BOOL ( Character, EnableXTargetting, true) // Enable Extended Targetting Window, for users with UF and later clients.
RULE_BOOL ( Character, KeepLevelOverMax, false) // Don't delevel a character that has somehow gone over the level cap
RULE_CATEGORY_END()

RULE_CATEGORY( Mercs )
RULE_INT (Mercs, SuspendIntervalMS, 10000)
RULE_INT (Mercs, UpkeepIntervalMS,  180000)
RULE_INT (Mercs, SuspendIntervalS, 10)
RULE_INT (Mercs, UpkeepIntervalS, 180)
RULE_BOOL (Mercs, AllowMercs, false)
RULE_BOOL (Mercs, ChargeMercPurchaseCost, false)
RULE_BOOL (Mercs, ChargeMercUpkeepCost, false)
RULE_INT (Mercs, AggroRadius, 100)		// Determines the distance from which a merc will aggro group member's target(also used to determine the distance at which a healer merc will begin healing a group member)
RULE_INT (Mercs, AggroRadiusPuller, 25)	// Determines the distance from which a merc will aggro group member's target, if they have the group role of puller (also used to determine the distance at which a healer merc will begin healing a group member, if they have the group role of puller)
RULE_INT (Mercs, ResurrectRadius, 50)	// Determines the distance from which a healer merc will attempt to resurrect a group member's corpse
RULE_INT (Mercs, ScaleRate, 100)
RULE_CATEGORY_END()

RULE_CATEGORY( Guild )
RULE_INT ( Guild, MaxMembers, 2048 )
RULE_BOOL ( Guild, PlayerCreationAllowed, false)	// Allow players to create a guild using the window in Underfoot+
RULE_INT ( Guild, PlayerCreationLimit, 1)		// Only allow use of the UF+ window if the account has < than this number of guild leaders on it
RULE_INT ( Guild, PlayerCreationRequiredStatus, 0)	// Required admin status.
RULE_INT ( Guild, PlayerCreationRequiredLevel, 0)	// Required Level of the player attempting to create the guild.
RULE_INT ( Guild, PlayerCreationRequiredTime, 0)	// Required Time Entitled On Account (in Minutes) to create the guild.

RULE_CATEGORY_END()

RULE_CATEGORY( Skills )
RULE_INT ( Skills, MaxTrainTradeskills, 21 )
RULE_BOOL ( Skills, UseLimitTradeskillSearchSkillDiff, true )
RULE_INT ( Skills, MaxTradeskillSearchSkillDiff, 50 )
RULE_INT ( Skills, MaxTrainSpecializations, 50 )	// Max level a GM trainer will train casting specializations
RULE_CATEGORY_END()

RULE_CATEGORY( Pets )
RULE_REAL( Pets, AttackCommandRange, 150 )
RULE_BOOL( Pets, UnTargetableSwarmPet, false )
RULE_CATEGORY_END()

RULE_CATEGORY( GM )
RULE_INT ( GM, MinStatusToZoneAnywhere, 250 )
RULE_CATEGORY_END()

RULE_CATEGORY( World )
RULE_INT ( World, ZoneAutobootTimeoutMS, 60000 )
RULE_INT ( World, ClientKeepaliveTimeoutMS, 65000 )
RULE_BOOL ( World, UseBannedIPsTable, false ) // Toggle whether or not to check incoming client connections against the Banned_IPs table. Set this value to false to disable this feature.
RULE_BOOL ( World, EnableTutorialButton, true)
RULE_BOOL ( World, EnableReturnHomeButton, true)
RULE_INT  ( World, MaxLevelForTutorial, 10)
RULE_INT  ( World, TutorialZoneID, 189)
RULE_INT  ( World, GuildBankZoneID, 345)
RULE_INT  ( World, MinOfflineTimeToReturnHome, 21600) // 21600 seconds is 6 Hours
RULE_INT ( World, MaxClientsPerIP, -1 ) // Maximum number of clients allowed to connect per IP address if account status is < AddMaxClientsStatus.  Default value: -1 (feature disabled)
RULE_INT ( World, ExemptMaxClientsStatus, -1 ) // Exempt accounts from the MaxClientsPerIP and AddMaxClientsStatus rules, if their status is >= this value.  Default value: -1 (feature disabled)
RULE_INT ( World, AddMaxClientsPerIP, -1 ) // Maximum number of clients allowed to connect per IP address if account status is < ExemptMaxClientsStatus.  Default value: -1 (feature disabled)
RULE_INT ( World, AddMaxClientsStatus, -1 ) // Accounts with status >= this rule will be allowed to use the amount of accounts defined in the AddMaxClientsPerIP.  Default value: -1 (feature disabled)
RULE_BOOL ( World, MaxClientsSetByStatus, false) // If True, IP Limiting will be set to the status on the account as long as the status is > MaxClientsPerIP
RULE_BOOL ( World, ClearTempMerchantlist, true) // Clears temp merchant items when world boots.
RULE_BOOL ( World, DeleteStaleCorpeBackups, true) // Deletes stale corpse backups older than 2 weeks.
RULE_INT ( World, AccountSessionLimit, -1 )  //Max number of characters allowed on at once from a single account (-1 is disabled)
RULE_INT ( World, ExemptAccountLimitStatus, -1 )  //Min status required to be exempt from multi-session per account limiting (-1 is disabled)
RULE_BOOL ( World, GMAccountIPList, false) // Check ip list against GM Accounts, AntiHack GM Accounts.
RULE_INT ( World, MinGMAntiHackStatus, 1 ) //Minimum GM status to check against AntiHack list
RULE_INT ( World, SoFStartZoneID, -1 ) //Sets the Starting Zone for SoF Clients separate from Titanium Clients (-1 is disabled)
RULE_INT ( World, ExpansionSettings, 16383) // Sets the expansion settings for the server, This is sent on login to world and affects client expansion settings. Defaults to all expansions enabled up to TSS.
RULE_INT ( World, PVPSettings, 0) // Sets the PVP settings for the server, 1 = Rallos Zek RuleSet, 2 = Tallon/Vallon Zek Ruleset, 4 = Sullon Zek Ruleset, 6 = Discord Ruleset, anything above 6 is the Discord Ruleset without the no-drop restrictions removed. TODO: Edit IsAttackAllowed in Zone to accomodate for these rules.
RULE_BOOL (World, IsGMPetitionWindowEnabled, false)
RULE_INT (World, FVNoDropFlag, 0) // Sets the Firiona Vie settings on the client. If set to 2, the flag will be set for GMs only, allowing trading of no-drop items.
RULE_BOOL (World, IPLimitDisconnectAll, false)
RULE_CATEGORY_END()

RULE_CATEGORY( Zone )
RULE_INT ( Zone,  NPCPositonUpdateTicCount, 32 ) //ms between intervals of sending a position update to the entire zone.
RULE_INT ( Zone,  ClientLinkdeadMS, 180000) //the time a client remains link dead on the server after a sudden disconnection
RULE_INT ( Zone,  GraveyardTimeMS, 1200000) //ms time until a player corpse is moved to a zone's graveyard, if one is specified for the zone
RULE_BOOL ( Zone, EnableShadowrest, 1 ) // enables or disables the shadowrest zone feature for player corpses. Default is turned on.
RULE_BOOL ( Zone, UsePlayerCorpseBackups, true) // Keeps backups of player corpses.
RULE_INT ( Zone, MQWarpExemptStatus, -1 ) // Required status level to exempt the MQWarpDetector.  Set to -1 to disable this feature.
RULE_INT ( Zone, MQZoneExemptStatus, -1 ) // Required status level to exempt the MQZoneDetector.  Set to -1 to disable this feature.
RULE_INT ( Zone, MQGateExemptStatus, -1 ) // Required status level to exempt the MQGateDetector.  Set to -1 to disable this feature.
RULE_INT ( Zone, MQGhostExemptStatus, -1 ) // Required status level to exempt the MGhostDetector.  Set to -1 to disable this feature.
RULE_BOOL ( Zone, EnableMQWarpDetector, true ) // Enable the MQWarp Detector.  Set to False to disable this feature.
RULE_BOOL ( Zone, EnableMQZoneDetector, true ) // Enable the MQZone Detector.  Set to False to disable this feature.
RULE_BOOL ( Zone, EnableMQGateDetector, true ) // Enable the MQGate Detector.  Set to False to disable this feature.
RULE_BOOL ( Zone, EnableMQGhostDetector, true ) // Enable the MQGhost Detector.  Set to False to disable this feature.
RULE_REAL ( Zone, MQWarpDetectionDistanceFactor, 9.0) //clients move at 4.4 about if in a straight line but with movement and to acct for lag we raise it a bit
RULE_BOOL ( Zone, MarkMQWarpLT, false )
RULE_INT ( Zone, AutoShutdownDelay, 5000 ) //How long a dynamic zone stays loaded while empty
RULE_INT ( Zone, PEQZoneReuseTime, 900 )	//How long, in seconds, until you can reuse the #peqzone command.
RULE_INT ( Zone, PEQZoneDebuff1, 4454 )	    //First debuff casted by #peqzone Default is Cursed Keeper's Blight.
RULE_INT ( Zone, PEQZoneDebuff2, 2209  )	//Second debuff casted by #peqzone Default is Tendrils of Apathy.
RULE_BOOL ( Zone, UsePEQZoneDebuffs, true )	//Will determine if #peqzone will debuff players or not when used.
RULE_REAL ( Zone, HotZoneBonus, 0.75 )
RULE_INT ( Zone, ReservedInstances, 30 ) //Will reserve this many instance ids for globals... probably not a good idea to change this while a server is running.
RULE_INT ( Zone, EbonCrystalItemID, 40902)
RULE_INT ( Zone, RadiantCrystalItemID, 40903)
RULE_BOOL ( Zone, LevelBasedEXPMods, false) // Allows you to use the level_exp_mods table in consideration to your players EXP hits
RULE_CATEGORY_END()

RULE_CATEGORY( Map )
//enable these to help prevent mob hopping when they are pathing
RULE_BOOL ( Map, FixPathingZWhenLoading, true )		//increases zone boot times a bit to reduce hopping.
RULE_BOOL ( Map, FixPathingZAtWaypoints, false )	//alternative to `WhenLoading`, accomplishes the same thing but does it at each waypoint instead of once at boot time.
RULE_BOOL ( Map, FixPathingZWhenMoving, false )		//very CPU intensive, but helps hopping with widely spaced waypoints.
RULE_BOOL ( Map, FixPathingZOnSendTo, false )		//try to repair Z coords in the SendTo routine as well.
RULE_REAL ( Map, FixPathingZMaxDeltaMoving, 20 )	//at runtime while pathing: max change in Z to allow the BestZ code to apply.
RULE_REAL ( Map, FixPathingZMaxDeltaWaypoint, 20 )	//at runtime at each waypoint: max change in Z to allow the BestZ code to apply.
RULE_REAL ( Map, FixPathingZMaxDeltaSendTo, 20 )	//at runtime in SendTo: max change in Z to allow the BestZ code to apply.
RULE_REAL ( Map, FixPathingZMaxDeltaLoading, 45 )	//while loading each waypoint: max change in Z to allow the BestZ code to apply.
RULE_BOOL ( Map, UseClosestZ, false)			// Move mobs to the nearest Z above or below, rather than just the nearest below.
							// Only set UseClosestZ true if all your .map files generated from EQGs were created
							// with azone2.
							//
RULE_INT  ( Map, FindBestZHeightAdjust, 1)		// Adds this to the current Z before seeking the best Z position
RULE_CATEGORY_END()

RULE_CATEGORY( Pathing )
// Some of these rules may benefit by being made into columns in the zone table,
// for instance, in dungeons, the min LOS distances could be substantially lowered.
RULE_BOOL ( Pathing, Aggro, true )		// Enable pathing for aggroed mobs.
RULE_BOOL ( Pathing, AggroReturnToGrid, true )	// Enable pathing for aggroed roaming mobs returning to their previous waypoint.
RULE_BOOL ( Pathing, Guard, true )		// Enable pathing for mobs moving to their guard point.
RULE_BOOL ( Pathing, Find, true )		// Enable pathing for FindPerson requests from the client.
RULE_BOOL ( Pathing, Fear, true )		// Enable pathing for fear
RULE_REAL ( Pathing, ZDiffThreshold, 10)	// If a mob las LOS to it's target, it will run to it if the Z difference is < this.
RULE_INT  ( Pathing, LOSCheckFrequency, 1000)	// A mob will check for LOS to it's target this often (milliseconds).
RULE_INT  ( Pathing, RouteUpdateFrequencyShort, 1000)	// How often a new route will be calculated if the target has moved.
RULE_INT  ( Pathing, RouteUpdateFrequencyLong, 5000)	// How often a new route will be calculated if the target has moved.
// When a path has a path node route and it's target changes position, if it has RouteUpdateFrequencyNodeCount or less nodes to go on it's
// current path, it will recalculate it's path based on the RouteUpdateFrequencyShort timer, otherwise it will use the
// RouteUpdateFrequencyLong timer.
RULE_INT  ( Pathing, RouteUpdateFrequencyNodeCount, 5) 
RULE_REAL ( Pathing, MinDistanceForLOSCheckShort, 40000) // (NoRoot). While following a path, only check for LOS to target within this distance.
RULE_REAL ( Pathing, MinDistanceForLOSCheckLong, 1000000) // (NoRoot). Min distance when initially attempting to acquire the target.
RULE_INT  ( Pathing, MinNodesLeftForLOSCheck, 4)	// Only check for LOS when we are down to this many path nodes left to run.
// This next rule was put in for situations where the mob and it's target may be on different sides of a 'hazard', e.g. a pit
// If the mob has LOS to it's target, even though there is a hazard in it's way, it may break off from the node path and run at
// the target, only to later detect the hazard and re-acquire a node path. Depending upon the placement of the path nodes, this
// can lead to the mob looping. The rule is intended to allow the mob to at least get closer to it's target each time before
// checking LOS and trying to head straight for it.
RULE_INT  ( Pathing, MinNodesTraversedForLOSCheck, 3)	// Only check for LOS after we have traversed this many path nodes.
RULE_INT  ( Pathing, CullNodesFromStart, 1)		// Checks LOS from Start point to second node for this many nodes and removes first node if there is LOS
RULE_INT  ( Pathing, CullNodesFromEnd, 1)		// Checks LOS from End point to second to last node for this many nodes and removes last node if there is LOS
RULE_REAL ( Pathing, CandidateNodeRangeXY, 400)		// When searching for path start/end nodes, only nodes within this range will be considered.
RULE_REAL ( Pathing, CandidateNodeRangeZ, 10)		// When searching for path start/end nodes, only nodes within this range will be considered.

RULE_CATEGORY_END()

RULE_CATEGORY( Watermap )
// enable these to use the water detection code. Requires Water Maps generated by awater utility
RULE_BOOL ( Watermap, CheckWaypointsInWaterWhenLoading, false ) // Does not apply BestZ as waypoints are loaded if they are in water
RULE_BOOL ( Watermap, CheckForWaterAtWaypoints, false) 		// Check if a mob has moved into/out of water when at waypoints and sets flymode
RULE_BOOL ( Watermap, CheckForWaterWhenMoving, false)		// Checks if a mob has moved into/out of water each time it's loc is recalculated
RULE_BOOL ( Watermap, CheckForWaterOnSendTo, false)		// Checks if a mob has moved into/out of water on SendTo
RULE_BOOL ( Watermap, CheckForWaterWhenFishing, false)		// Only lets a player fish near water (if a water map exists for the zone)
RULE_REAL ( Watermap, FishingRodLength, 30)			// How far in front of player water must be for fishing to work
RULE_REAL ( Watermap, FishingLineLength, 40)			// If water is more than this far below the player, it is considered too far to fish
RULE_CATEGORY_END()

RULE_CATEGORY( Spells )
RULE_INT ( Spells, AutoResistDiff, 15)
RULE_REAL ( Spells, ResistChance, 2.0) //chance to resist given no resists and same level
RULE_REAL ( Spells, ResistMod, 0.40) //multiplier, chance to resist = this * ResistAmount
RULE_REAL ( Spells, PartialHitChance, 0.7) //The chance when a spell is resisted that it will partial hit.
RULE_REAL ( Spells, PartialHitChanceFear, 0.25) //The chance when a fear spell is resisted that it will partial hit.
RULE_INT ( Spells, BaseCritChance, 0) //base % chance that everyone has to crit a spell
RULE_INT ( Spells, BaseCritRatio, 100) //base % bonus to damage on a successful spell crit. 100 = 2x damage
RULE_INT ( Spells, WizCritLevel, 12) //level wizards first get spell crits
RULE_INT ( Spells, WizCritChance, 7) //wiz's crit chance, on top of BaseCritChance
RULE_INT ( Spells, WizCritRatio, 0) //wiz's crit bonus, on top of BaseCritRatio (should be 0 for Live-like)
RULE_INT ( Spells, ResistPerLevelDiff, 85) //8.5 resist per level difference.
RULE_INT ( Spells, TranslocateTimeLimit, 0) // If not zero, time in seconds to accept a Translocate.
RULE_INT ( Spells, SacrificeMinLevel, 46)	//first level Sacrifice will work on
RULE_INT ( Spells, SacrificeMaxLevel, 69)	//last level Sacrifice will work on
RULE_INT ( Spells, SacrificeItemID, 9963)	//Item ID of the item Sacrifice will return (defaults to an EE)
RULE_BOOL ( Spells, EnableSpellGlobals, false)	// If Enabled, spells check the spell_globals table and compare character data from the quest globals before allowing that spell to scribe with scribespells
RULE_INT ( Spells, MaxBuffSlotsNPC, 25)
RULE_INT ( Spells, MaxSongSlotsNPC, 10)
RULE_INT ( Spells, MaxDiscSlotsNPC, 1)
RULE_INT ( Spells, MaxTotalSlotsNPC, 36)
RULE_INT ( Spells, MaxTotalSlotsPET, 25)	// do not set this higher than 25 until the player profile is removed from the blob
RULE_BOOL (Spells, EnableBlockedBuffs, true)
RULE_INT ( Spells, ReflectType, 1) //0 = disabled, 1 = single target player spells only, 2 = all player spells, 3 = all single target spells, 4 = all spells
RULE_INT ( Spells, VirusSpreadDistance, 30) // The distance a viral spell will jump to its next victim
RULE_BOOL( Spells, LiveLikeFocusEffects, true) // Determines whether specific healing, dmg and mana reduction focuses are randomized
RULE_INT ( Spells, BaseImmunityLevel, 55) // The level that targets start to be immune to stun, fear and mez spells with a max level of 0.
RULE_BOOL ( Spells, NPCIgnoreBaseImmunity, true) // Whether or not NPCs get to ignore the BaseImmunityLevel for their spells.
RULE_REAL ( Spells, AvgSpellProcsPerMinute, 6.0) //Adjust rate for sympathetic spell procs
RULE_INT ( Spells, ResistFalloff, 67) //Max that level that will adjust our resist chance based on level modifiers
RULE_INT ( Spells, CharismaEffectiveness, 10) // Deterimes how much resist modification charisma applies to charm/pacify checks. Default 10 CHA = -1 resist mod.
RULE_INT ( Spells, CharmBreakCheckChance, 25) //Determines chance for a charm break check to occur each buff tick.
RULE_INT ( Spells, MaxCastTimeReduction, 50) //Max percent your spell cast time can be reduced by spell haste
RULE_INT ( Spells, RootBreakFromSpells, 20) //Chance for root to break when cast on.
RULE_INT ( Spells, DeathSaveCharismaMod, 3) //Determines how much charisma effects chance of death save firing.
RULE_INT ( Spells, DivineInterventionHeal, 8000) //Divine intervention heal amount.
RULE_BOOL ( Spells, AdditiveBonusValues, false) //Allow certain bonuses to be calculated by adding together the value from each item, instead of taking the highest value. (ie Add together all Cleave Effects)
RULE_BOOL ( Spells, UseCHAScribeHack, false) //ScribeSpells and TrainDiscs quest functions will ignore entries where field 12 is CHA.  What's the best way to do this?
RULE_CATEGORY_END()

RULE_CATEGORY( Combat )
RULE_INT ( Combat, MeleeBaseCritChance, 0 ) //The base crit chance for non warriors, NOTE: This will apply to NPCs as well
RULE_INT ( Combat, WarBerBaseCritChance, 3 ) //The base crit chance for warriors and berserkers, only applies to clients
RULE_INT ( Combat, BerserkBaseCritChance, 6 ) //The bonus base crit chance you get when you're berserk
RULE_INT ( Combat, NPCBashKickLevel, 6 ) //The level that npcs can KICK/BASH
RULE_INT ( Combat, NPCBashKickStunChance, 15 ) //Percent chance that a bash/kick will stun
RULE_INT ( Combat, ClientBaseCritChance, 0 ) //The base crit chance for all clients, this will stack with warrior's/zerker's crit chance.
RULE_BOOL ( Combat, UseIntervalAC, true)
RULE_INT ( Combat, PetAttackMagicLevel, 30)
RULE_BOOL ( Combat, EnableFearPathing, true)
RULE_INT ( Combat, FleeHPRatio, 25)
RULE_INT ( Combat, FleeSnareHPRatio, 11) // HP at which snare will halt movement of a fleeing NPC.
RULE_BOOL ( Combat, FleeIfNotAlone, false) // If false, mobs won't flee if other mobs are in combat with it.
RULE_BOOL ( Combat, AdjustProcPerMinute, true)
RULE_REAL ( Combat, AvgProcsPerMinute, 2.0)
RULE_REAL ( Combat, ProcPerMinDexContrib, 0.075)
RULE_REAL ( Combat, BaseProcChance, 0.035)
RULE_REAL ( Combat, ProcDexDivideBy, 11000)
RULE_REAL ( Combat, BaseHitChance, 69.0)
RULE_REAL ( Combat, NPCBonusHitChance, 26.0)
RULE_REAL ( Combat, HitFalloffMinor, 5.0) //hit will fall off up to 5% over the initial level range
RULE_REAL ( Combat, HitFalloffModerate, 7.0) //hit will fall off up to 7% over the three levels after the initial level range
RULE_REAL ( Combat, HitFalloffMajor, 50.0) //hit will fall off sharply if we're outside the minor and moderate range
RULE_REAL ( Combat, HitBonusPerLevel, 1.2) //You gain this % of hit for every level you are above your target
RULE_REAL ( Combat, WeaponSkillFalloff, 0.33) //For every weapon skill point that's not maxed you lose this % of hit
RULE_REAL ( Combat, ArcheryHitPenalty, 0.25) //Archery has a hit penalty to try to help balance it with the plethora of long term +hit modifiers for it
RULE_REAL ( Combat, AgiHitFactor, 0.01) 
RULE_INT ( Combat, MinRangedAttackDist, 25) //Minimum Distance to use Ranged Attacks
RULE_BOOL ( Combat, ArcheryBonusRequiresStationary, true) //does the 2x archery bonus chance require a stationary npc
RULE_REAL ( Combat, ArcheryBaseDamageBonus, 1) // % Modifier to Base Archery Damage (.5 = 50% base damage, 1 = 100%, 2 = 200%)
RULE_REAL ( Combat, ArcheryNPCMultiplier, 1.0) // this is multiplied by the regular dmg to get the archery dmg
RULE_BOOL ( Combat, AssistNoTargetSelf, true)  //when assisting a target that does not have a target: true = target self, false = leave target as was before assist (false = live like)
RULE_INT ( Combat, MaxRampageTargets, 3) //max number of people hit with rampage
RULE_INT ( Combat, MaxFlurryHits, 2) //max number of extra hits from flurry
RULE_INT ( Combat, MonkDamageTableBonus, 5) //% bonus monks get to their damage table calcs
RULE_INT ( Combat, FlyingKickBonus, 25) //% Modifier that this skill gets to str and skill bonuses
RULE_INT ( Combat, DragonPunchBonus, 20) //% Modifier that this skill gets to str and skill bonuses
RULE_INT ( Combat, EagleStrikeBonus, 15) //% Modifier that this skill gets to str and skill bonuses
RULE_INT ( Combat, TigerClawBonus, 10) //% Modifier that this skill gets to str and skill bonuses
RULE_INT ( Combat, RoundKickBonus, 5) //% Modifier that this skill gets to str and skill bonuses
RULE_INT ( Combat, FrenzyBonus, 0) //% Modifier to damage
RULE_BOOL ( Combat, ProcTargetOnly, true) //true = procs will only affect our target, false = procs will affect all of our targets
RULE_REAL ( Combat, NPCACFactor, 2.25)
RULE_INT ( Combat, ClothACSoftcap, 75)
RULE_INT ( Combat, LeatherACSoftcap, 100)
RULE_INT ( Combat, MonkACSoftcap, 120)
RULE_INT ( Combat, ChainACSoftcap, 200)
RULE_INT ( Combat, PlateACSoftcap, 300)
RULE_REAL ( Combat, AAMitigationACFactor, 3.0)
RULE_REAL ( Combat, WarriorACSoftcapReturn, 0.45)
RULE_REAL ( Combat, KnightACSoftcapReturn, 0.33)
RULE_REAL ( Combat, LowPlateChainACSoftcapReturn, 0.23)
RULE_REAL ( Combat, LowChainLeatherACSoftcapReturn, 0.17)
RULE_REAL ( Combat, CasterACSoftcapReturn, 0.06)
RULE_REAL ( Combat, MiscACSoftcapReturn, 0.3)
RULE_REAL ( Combat, ACthac0Factor, 0.55)
RULE_REAL ( Combat, ACthac20Factor, 0.55)
RULE_INT ( Combat, HitCapPre20, 40) // live has it capped at 40 for whatever dumb reason... this is mainly for custom servers
RULE_INT ( Combat, HitCapPre10, 20) // live has it capped at 20, see above :p
RULE_INT ( Combat, MinHastedDelay, 400) // how fast we can get with haste.
RULE_REAL ( Combat, AvgDefProcsPerMinute, 2.0)
RULE_REAL ( Combat, DefProcPerMinAgiContrib, 0.075) //How much agility contributes to defensive proc rate
RULE_INT ( Combat, SpecialAttackACBonus, 15) //Percent amount of damage per AC gained for certain special attacks (damage = AC*SpecialAttackACBonus/100).
RULE_INT ( Combat, NPCFlurryChance, 20) // Chance for NPC to flurry.
RULE_BOOL (Combat,TauntOverLevel, 1) //Allows you to taunt NPC's over warriors level.
RULE_REAL (Combat,TauntSkillFalloff, 0.33)//For every taunt skill point that's not maxed you lose this % chance to taunt.
RULE_BOOL (Combat,EXPFromDmgShield, false) //Determine if damage from a damage shield counts for EXP gain.
RULE_INT ( Combat, MonkACBonusWeight, 15)
RULE_INT ( Combat, ClientStunLevel, 55) //This is the level where client kicks and bashes can stun the target
RULE_INT ( Combat, QuiverWRHasteDiv, 3) //Weight Reduction is divided by this to get haste contribution for quivers
RULE_BOOL ( Combat, UseArcheryBonusRoll, false) //Make the 51+ archery bonus require an actual roll
RULE_INT ( Combat, ArcheryBonusChance, 50)
RULE_CATEGORY_END()

RULE_CATEGORY( NPC )
RULE_INT ( NPC, MinorNPCCorpseDecayTimeMS, 450000 ) //level<55
RULE_INT ( NPC, MajorNPCCorpseDecayTimeMS, 1500000 ) //level>=55
RULE_INT ( NPC, CorpseUnlockTimer, 150000 )
RULE_INT ( NPC, EmptyNPCCorpseDecayTimeMS, 0 )
RULE_BOOL (NPC, UseItemBonusesForNonPets, true)
RULE_INT ( NPC, SayPauseTimeInSec, 5)
RULE_INT ( NPC, OOCRegen, 0)
RULE_BOOL ( NPC, BuffFriends, false )
RULE_BOOL ( NPC, EnableNPCQuestJournal, false)
RULE_INT ( NPC, LastFightingDelayMovingMin, 10000)
RULE_INT ( NPC, LastFightingDelayMovingMax, 20000)
RULE_BOOL ( NPC, SmartLastFightingDelayMoving, true)
RULE_BOOL ( NPC, ReturnNonQuestNoDropItems, false)	// Returns NO DROP items on NPCs that don't have an EVENT_ITEM sub in their script
RULE_INT ( NPC, StartEnrageValue, 9) // % HP that an NPC will begin to enrage
RULE_BOOL ( NPC, LiveLikeEnrage, false) // If set to true then only player controlled pets will enrage
RULE_BOOL (NPC, UseMultiQuest, false) // If true, NPC will remember items handed to them for classic multiquest support.
RULE_CATEGORY_END()

RULE_CATEGORY ( Aggro )
RULE_BOOL ( Aggro, SmartAggroList, true )
RULE_INT ( Aggro, SittingAggroMod, 35 ) //35%
RULE_INT ( Aggro, MeleeRangeAggroMod, 10 ) //10%
RULE_INT ( Aggro, CurrentTargetAggroMod, 0 ) //0% -- will prefer our current target to any other; makes it harder for our npcs to switch targets.
RULE_INT ( Aggro, CriticallyWoundedAggroMod, 100 ) //100%
RULE_INT ( Aggro, SpellAggroMod, 100 )
RULE_INT ( Aggro, SongAggroMod, 33 )
RULE_INT ( Aggro, PetSpellAggroMod, 10 )
RULE_REAL ( Aggro, TunnelVisionAggroMod, 0.75 ) //people not currently the top hate generate this much hate on a Tunnel Vision mob
RULE_INT ( Aggro, MaxStunProcAggro, 400 ) // Set to -1 for no limit. Maxmimum amount of aggro that a stun based proc will add.
RULE_CATEGORY_END()

RULE_CATEGORY ( TaskSystem)
RULE_BOOL ( TaskSystem, EnableTaskSystem, true) // Globally enable or disable the Task system
RULE_INT ( TaskSystem, PeriodicCheckTimer, 5) // Seconds between checks for failed tasks. Also used by the 'Touch' activity
RULE_BOOL ( TaskSystem, RecordCompletedTasks, true)
RULE_BOOL ( TaskSystem, RecordCompletedOptionalActivities, false)
RULE_BOOL ( TaskSystem, KeepOneRecordPerCompletedTask, true)
RULE_BOOL ( TaskSystem, EnableTaskProximity, true)
RULE_CATEGORY_END()

#ifdef BOTS
RULE_CATEGORY ( Bots )
RULE_REAL ( Bots, BotManaRegen, 2.0 ) // Adjust mana regen for bots, 1 is fast and higher numbers slow it down 3 is about the same as players.
RULE_BOOL ( Bots, BotFinishBuffing, false ) // Allow for buffs to complete even if the bot caster is out of mana.  Only affects buffing out of combat.
RULE_INT ( Bots, CreateBotCount, 150 ) // Number of bots that each account can create
RULE_INT ( Bots, SpawnBotCount, 71 ) // Number of bots a character can have spawned at one time, You + 71 bots is a 12 group raid
RULE_BOOL ( Bots, BotQuest, false ) // Optional quest method to manage bot spawn limits using the quest_globals name bot_spawn_limit, see: /bazaar/Aediles_Thrall.pl
RULE_BOOL ( Bots, BotGroupBuffing, false ) // Bots will cast single target buffs as group buffs, default is false for single. Does not make single target buffs work for MGB.
RULE_BOOL ( Bots, BotSpellQuest, false ) // Anita Thrall's (Anita_Thrall.pl) Bot Spell Scriber quests.
RULE_INT ( Bots, BotAAExpansion, 8 ) // Bots get AAs through this expansion
RULE_BOOL ( Bots, BotGroupXP, false ) // Determines whether client gets xp for bots outside their group.
RULE_CATEGORY_END()
#endif

RULE_CATEGORY ( Chat )
RULE_BOOL ( Chat, ServerWideOOC, true)
RULE_BOOL ( Chat, ServerWideAuction, true)
RULE_BOOL ( Chat, EnableVoiceMacros, true)
RULE_BOOL ( Chat, EnableMailKeyIPVerification, true)
RULE_BOOL ( Chat, EnableAntiSpam, true)
RULE_BOOL ( Chat, FlowCommandstoPerl_EVENT_SAY, false) // Allows you to parse #commands into EVENT_SAY (Useful in global_player.pl) that aren't found in the source - should probably be individual scripts per command sometime
RULE_INT ( Chat, MinStatusToBypassAntiSpam, 100)
RULE_INT ( Chat, MinimumMessagesPerInterval, 4)
RULE_INT ( Chat, MaximumMessagesPerInterval, 12)
RULE_INT ( Chat, MaxMessagesBeforeKick, 20)
RULE_INT ( Chat, IntervalDurationMS, 60000)
RULE_INT ( Chat, KarmaUpdateIntervalMS, 1200000)
RULE_INT ( Chat, KarmaGlobalChatLimit, 72) //amount of karma you need to be able to talk in ooc/auction/chat below the level limit
RULE_INT ( Chat, GlobalChatLevelLimit, 8) //level limit you need to of reached to talk in ooc/auction/chat if your karma is too low.
RULE_CATEGORY_END()

RULE_CATEGORY ( Merchant )
RULE_BOOL ( Merchant, UsePriceMod, true) // Use faction/charisma price modifiers.
RULE_REAL ( Merchant, SellCostMod, 1.05) // Modifier for NPC sell price.
RULE_REAL ( Merchant, BuyCostMod, 0.95) // Modifier for NPC buy price.
RULE_INT ( Merchant, PriceBonusPct, 4) // Determines maximum price bonus from having good faction/CHA. Value is a percent.
RULE_INT ( Merchant, PricePenaltyPct, 4) // Determines maximum price penalty from having bad faction/CHA. Value is a percent.
RULE_REAL( Merchant, ChaBonusMod, 3.45) // Determines CHA cap, from 104 CHA. 3.45 is 132 CHA at apprehensive. 0.34 is 400 CHA at apprehensive.
RULE_REAL ( Merchant, ChaPenaltyMod, 1.52) // Determines CHA bottom, up to 102 CHA. 1.52 is 37 CHA at apprehensive. 0.98 is 0 CHA at apprehensive.
RULE_CATEGORY_END()

RULE_CATEGORY ( Bazaar )
RULE_BOOL ( Bazaar, AuditTrail, false)
RULE_INT ( Bazaar, MaxSearchResults, 50)
RULE_BOOL ( Bazaar, EnableWarpToTrader, true)
RULE_INT ( Bazaar, MaxBarterSearchResults, 200) // The max results returned in the /barter search
RULE_CATEGORY_END()

RULE_CATEGORY ( Mail )
RULE_BOOL ( Mail, EnableMailSystem, true) // If false, client won't bring up the Mail window.
RULE_INT ( Mail, ExpireTrash, 0) // Time in seconds. 0 will delete all messages in the trash when the mailserver starts
RULE_INT ( Mail, ExpireRead, 31536000 ) // 1 Year. Set to -1 for never
RULE_INT ( Mail, ExpireUnread, 31536000 ) // 1 Year. Set to -1 for never
RULE_CATEGORY_END()

RULE_CATEGORY ( Channels )
RULE_INT ( Channels, RequiredStatusAdmin, 251) // Required status to administer chat channels
RULE_INT ( Channels, RequiredStatusListAll, 251) // Required status to list all chat channels
RULE_INT ( Channels, DeleteTimer, 1440) // Empty password protected channels will be deleted after this many minutes
RULE_CATEGORY_END()

RULE_CATEGORY ( EventLog )
RULE_BOOL ( EventLog, RecordSellToMerchant, false ) // Record sales from a player to an NPC merchant in eventlog table
RULE_BOOL ( EventLog, RecordBuyFromMerchant, false ) // Record purchases by a player from an NPC merchant in eventlog table
RULE_CATEGORY_END()

RULE_CATEGORY ( Adventure )
RULE_INT ( Adventure, MinNumberForGroup, 2 )
RULE_INT ( Adventure, MaxNumberForGroup, 6 )
RULE_INT ( Adventure, MinNumberForRaid, 18 )
RULE_INT ( Adventure, MaxNumberForRaid, 36 )
RULE_INT ( Adventure, MaxLevelRange, 9 )
RULE_INT ( Adventure, NumberKillsForBossSpawn, 45)
RULE_REAL ( Adventure, DistanceForRescueAccept, 10000.0)
RULE_REAL ( Adventure, DistanceForRescueComplete, 2500.0)
RULE_INT ( Adventure, ItemIDToEnablePorts, 41000 ) //0 to disable, otherwise using a LDoN portal will require the user to have this item.
RULE_INT ( Adventure, LDoNTrapDistanceUse, 625 )
RULE_REAL ( Adventure, LDoNBaseTrapDifficulty, 15.0 )
RULE_REAL ( Adventure, LDoNCriticalFailTrapThreshold, 10.0 )
RULE_INT ( Adventure, LDoNAdventureExpireTime, 1800) //30 minutes to expire
RULE_CATEGORY_END()

RULE_CATEGORY ( AA )
RULE_INT ( AA, ExpPerPoint, 23976503)	//Amount of exp per AA. Is the same as the amount of exp to go from level 51 to level 52.
RULE_BOOL ( AA, Stacking, true) //Allow AA that belong to the same group to stack on SOF+ clients.
RULE_CATEGORY_END()

RULE_CATEGORY( Console )
RULE_INT ( Console, SessionTimeOut, 600000 )	// Amount of time in ms for the console session to time out
RULE_CATEGORY_END()

RULE_CATEGORY( EQStream )
RULE_INT ( EQStream, RetransmitTimeoutMax, 5000 ) // maximum retransmit timeout before retransmitting unacked packets
RULE_INT ( EQStream, AverageDeltaMax, 2500 ) // maximum average rtt where we will still recalculate transmit rates
RULE_REAL ( EQStream, RetransmitTimeoutMult, 3.0 ) // multiplier applied to rtt stats to generate a retransmit timeout value
RULE_BOOL ( EQStream, RetransmitAckedPackets, true ) // should we restransmit packets that were already acked?
RULE_CATEGORY_END()

RULE_CATEGORY( QueryServ )
RULE_BOOL( QueryServ, PlayerChatLogging, false) // Logs Player Chat
RULE_BOOL( QueryServ, PlayerLogTrades, false) // Logs Player Trades
RULE_BOOL( QueryServ, PlayerLogHandins, false) // Logs Player Handins
RULE_BOOL( QueryServ, PlayerLogNPCKills, false) // Logs Player NPC Kills
RULE_BOOL( QueryServ, PlayerLogDeletes, false) // Logs Player Deletes
RULE_BOOL( QueryServ, PlayerLogMoves, false) // Logs Player Moves
RULE_BOOL( QueryServ, MerchantLogTransactions, false) // Logs Merchant Transactions
RULE_BOOL( QueryServ, PlayerLogPCCoordinates, false) // Logs Player Coordinates with certain events
RULE_CATEGORY_END()

#undef RULE_CATEGORY
#undef RULE_INT
#undef RULE_REAL
#undef RULE_BOOL
#undef RULE_CATEGORY_END

