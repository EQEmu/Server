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




RULE_CATEGORY(Character)
RULE_INT(Character, MaxLevel, 65)
RULE_BOOL(Character, PerCharacterQglobalMaxLevel, false) // This will check for qglobal 'CharMaxLevel' character qglobal (Type 5), if player tries to level beyond that point, it will not go beyond that level
RULE_INT(Character, MaxExpLevel, 0) //Sets the Max Level attainable via Experience
RULE_INT(Character, DeathExpLossLevel, 10)	// Any level greater than this will lose exp on death
RULE_INT(Character, DeathExpLossMaxLevel, 255)	// Any level greater than this will no longer lose exp on death
RULE_INT(Character, DeathItemLossLevel, 10)
RULE_INT(Character, DeathExpLossMultiplier, 3) //Adjust how much exp is lost
RULE_BOOL(Character, UseDeathExpLossMult, false) //Adjust to use the above multiplier or to use code default.
RULE_INT(Character, CorpseDecayTimeMS, 10800000)
RULE_INT(Character, CorpseResTimeMS, 10800000) // time before cant res corpse(3 hours)
RULE_BOOL(Character, LeaveCorpses, true)
RULE_BOOL(Character, LeaveNakedCorpses, false)
RULE_INT(Character, MaxDraggedCorpses, 2)
RULE_REAL(Character, DragCorpseDistance, 400) // If the corpse is <= this distance from the player, it won't move
RULE_REAL(Character, ExpMultiplier, 0.5)
RULE_REAL(Character, AAExpMultiplier, 0.5)
RULE_REAL(Character, GroupExpMultiplier, 0.5)
RULE_REAL(Character, RaidExpMultiplier, 0.2)
RULE_BOOL(Character, UseXPConScaling, true)
RULE_INT(Character, ShowExpValues, 0) //0 - normal, 1 - Show raw experience values, 2 - Show raw experience values AND percent.
RULE_INT(Character, GreenModifier, 20)
RULE_INT(Character, LightBlueModifier, 40)
RULE_INT(Character, BlueModifier, 90)
RULE_INT(Character, WhiteModifier, 100)
RULE_INT(Character, YellowModifier, 125)
RULE_INT(Character, RedModifier, 150)
RULE_INT(Character, AutosaveIntervalS, 300)	//0=disabled
RULE_INT(Character, HPRegenMultiplier, 100)
RULE_INT(Character, ManaRegenMultiplier, 100)
RULE_INT(Character, EnduranceRegenMultiplier, 100)
RULE_BOOL(Character, OldMinMana, false) // this is used for servers that want to follow older skill cap formulas so they can still have some regen w/o mediate
RULE_INT(Character, ConsumptionMultiplier, 100) //item's hunger restored = this value * item's food level, 100 = normal, 50 = people eat 2x as fast, 200 = people eat 2x as slow
RULE_BOOL(Character, HealOnLevel, false)
RULE_BOOL(Character, FeignKillsPet, false)
RULE_INT(Character, ItemManaRegenCap, 15)
RULE_INT(Character, ItemHealthRegenCap, 30)
RULE_INT(Character, ItemDamageShieldCap, 30)
RULE_INT(Character, ItemAccuracyCap, 150)
RULE_INT(Character, ItemAvoidanceCap, 100)
RULE_INT(Character, ItemCombatEffectsCap, 100)
RULE_INT(Character, ItemShieldingCap, 35)
RULE_INT(Character, ItemSpellShieldingCap, 35)
RULE_INT(Character, ItemDoTShieldingCap, 35)
RULE_INT(Character, ItemStunResistCap, 35)
RULE_INT(Character, ItemStrikethroughCap, 35)
RULE_INT(Character, ItemATKCap, 250)
RULE_INT(Character, ItemHealAmtCap, 250)
RULE_INT(Character, ItemSpellDmgCap, 250)
RULE_INT(Character, ItemClairvoyanceCap, 250)
RULE_INT(Character, ItemDSMitigationCap, 50)
RULE_INT(Character, ItemEnduranceRegenCap, 15)
RULE_INT(Character, ItemExtraDmgCap, 150) // Cap for bonuses to melee skills like Bash, Frenzy, etc
RULE_INT(Character, HasteCap, 100) // Haste cap for non-v3(overhaste) haste.
RULE_INT(Character, SkillUpModifier, 100) //skill ups are at 100%
RULE_BOOL(Character, SharedBankPlat, false) //off by default to prevent duping for now
RULE_BOOL(Character, BindAnywhere, false)
RULE_BOOL(Character, RestRegenEnabled, true) // Enable OOC Regen
RULE_INT(Character, RestRegenHP, 180) // seconds until full from 0. this is actually zone setable, but most or all zones are 180
RULE_INT(Character, RestRegenMana, 180) // seconds until full from 0. this is actually zone setable, but most or all zones are 180
RULE_INT(Character, RestRegenEnd, 180) // seconds until full from 0. this is actually zone setable, but most or all zones are 180
RULE_INT(Character, RestRegenTimeToActivate, 30) // Time in seconds for rest state regen to kick in.
RULE_INT(Character, RestRegenRaidTimeToActivate, 300) // Time in seconds for rest state regen to kick in with a raid target.
RULE_INT(Character, KillsPerGroupLeadershipAA, 250) // Number of dark blues or above per Group Leadership AA
RULE_INT(Character, KillsPerRaidLeadershipAA, 250) // Number of dark blues or above per Raid Leadership AA
RULE_INT(Character, MaxFearDurationForPlayerCharacter, 4) //4 tics, each tic calculates every 6 seconds.
RULE_INT(Character, MaxCharmDurationForPlayerCharacter, 15)
RULE_INT(Character, BaseHPRegenBonusRaces, 4352)	//a bitmask of race(s) that receive the regen bonus. Iksar (4096) & Troll (256) = 4352. see common/races.h for the bitmask values
RULE_BOOL(Character, SoDClientUseSoDHPManaEnd, false)	// Setting this to true will allow SoD clients to use the SoD HP/Mana/End formulas and previous clients will use the old formulas
RULE_BOOL(Character, UseRaceClassExpBonuses, true)	// Setting this to true will enable Class and Racial experience rate bonuses
RULE_BOOL(Character, UseOldRaceExpPenalties, false)	// Setting this to true will enable racial exp penalties for Iksar, Troll, Ogre, and Barbarian, as well as the bonus for Halflings
RULE_BOOL(Character, UseOldClassExpPenalties, false)	// Setting this to true will enable old class exp penalties for Paladin, SK, Ranger, Bard, Monk, Wizard, Enchanter, Magician, and Necromancer, as well as the bonus for Rogues and Warriors
RULE_BOOL(Character, RespawnFromHover, false)		// Use Respawn window, or not.
RULE_INT(Character, RespawnFromHoverTimer, 300)	// Respawn Window countdown timer, in SECONDS
RULE_BOOL(Character, UseNewStatsWindow, true)		// New stats window shows everything
RULE_BOOL(Character, ItemCastsUseFocus, false) // If true, this allows item clickies to use focuses that have limited max levels on them
RULE_INT(Character, MinStatusForNoDropExemptions, 80) // This allows status x and higher to trade no drop items.
RULE_INT(Character, SkillCapMaxLevel, 75)	// Sets the Max Level used for Skill Caps (from skill_caps table). -1 makes it use MaxLevel rule value. It is set to 75 because PEQ only has skillcaps up to that level, and grabbing the players' skill past 75 will return 0, breaking all skills past that level. This helps servers with obsurd level caps (75+ level cap) function without any modifications.
RULE_INT(Character, StatCap, 0)
RULE_BOOL(Character, CheckCursorEmptyWhenLooting, true) // If true, a player cannot loot a corpse (player or NPC) with an item on their cursor
RULE_BOOL(Character, MaintainIntoxicationAcrossZones, true) // If true, alcohol effects are maintained across zoning and logging out/in.
RULE_BOOL(Character, EnableDiscoveredItems, true) // If enabled, it enables EVENT_DISCOVER_ITEM and also saves character names and timestamps for the first time an item is discovered.
RULE_BOOL(Character, EnableXTargetting, true) // Enable Extended Targetting Window, for users with UF and later clients.
RULE_BOOL(Character, EnableAggroMeter, true) // Enable Aggro Meter, for users with RoF and later clients.
RULE_BOOL(Character, KeepLevelOverMax, false) // Don't delevel a character that has somehow gone over the level cap
RULE_INT(Character, FoodLossPerUpdate, 32) // How much food/water you lose per stamina update
RULE_BOOL(Character, EnableHungerPenalties, false) // being hungry/thirsty has negative effects -- it does appear normal live servers do not have penalties
RULE_INT(Character, BaseInstrumentSoftCap, 36) // Softcap for instrument mods, 36 commonly referred to as "3.6" as well.
RULE_BOOL(Character, UseSpellFileSongCap, true) // When they removed the AA that increased the cap they removed the above and just use the spell field
RULE_INT(Character, BaseRunSpeedCap, 158) // Base Run Speed Cap, on live it's 158% which will give you a runspeed of 1.580 hard capped to 225.
RULE_INT(Character, OrnamentationAugmentType, 20) //Ornamentation Augment Type
RULE_REAL(Character, EnvironmentDamageMulipliter, 1)
RULE_BOOL(Character, UnmemSpellsOnDeath, true)
RULE_INT(Character, TradeskillUpAlchemy, 2) // Alchemy skillup rate adjust. Lower is faster.
RULE_INT(Character, TradeskillUpBaking, 2) // Baking skillup rate adjust. Lower is faster.
RULE_INT(Character, TradeskillUpBlacksmithing, 2) // Blacksmithing skillup rate adjust. Lower is faster.
RULE_INT(Character, TradeskillUpBrewing, 3) // Brewing skillup rate adjust. Lower is faster.
RULE_INT(Character, TradeskillUpFletching, 2) // Fletching skillup rate adjust. Lower is faster.
RULE_INT(Character, TradeskillUpJewelcrafting, 2) // Jewelcrafting skillup rate adjust. Lower is faster.
RULE_INT(Character, TradeskillUpMakePoison, 2) // Make Poison skillup rate adjust. Lower is faster.
RULE_INT(Character, TradeskillUpPottery, 4) // Pottery skillup rate adjust. Lower is faster.
RULE_INT(Character, TradeskillUpResearch, 1) // Research skillup rate adjust. Lower is faster.
RULE_INT(Character, TradeskillUpTinkering, 2) // Tinkering skillup rate adjust. Lower is faster.
RULE_BOOL(Character, MarqueeHPUpdates, false) // Will show Health % in center of screen < 100%
RULE_INT(Character, IksarCommonTongue, 95) // 95 By default (live-like?)
RULE_INT(Character, OgreCommonTongue, 95) // 95 By default (live-like?)
RULE_INT(Character, TrollCommonTongue, 95) // 95 By default (live-like?)
RULE_BOOL(Character, ActiveInvSnapshots, false) // Takes a periodic snapshot of inventory contents from online players
RULE_INT(Character, InvSnapshotMinIntervalM, 180) // Minimum time (in minutes) between inventory snapshots
RULE_INT(Character, InvSnapshotMinRetryM, 30) // Time (in minutes) to re-attempt an inventory snapshot after a failure
RULE_INT(Character, InvSnapshotHistoryD, 30) // Time (in days) to keep snapshot entries
RULE_BOOL(Character, RestrictSpellScribing, false) // Restricts spell scribing to allowable races/classes of spell scroll, if true
RULE_BOOL(Character, UseStackablePickPocketing, true) // Allows stackable pickpocketed items to stack instead of only being allowed in empty inventory slots
RULE_BOOL(Character, EnableAvoidanceCap, false)
RULE_INT(Character, AvoidanceCap, 750) // 750 Is a pretty good value, seen people dodge all attacks beyond 1,000 Avoidance
RULE_BOOL(Character, AllowMQTarget, false) // Disables putting players in the 'hackers' list for targeting beyond the clip plane or attempting to target something untargetable
RULE_BOOL(Character, UseOldBindWound, false) // Uses the original bind wound behavior
RULE_BOOL(Character, GrantHoTTOnCreate, false) // Grant Health of Target's Target leadership AA on character creation
RULE_BOOL(Character, UseOldConSystem, false) // Grant Health of Target's Target leadership AA on character creation
RULE_BOOL(Character, OPClientUpdateVisualDebug, false) // Shows a pulse and forward directional particle each time the client sends its position to server
RULE_CATEGORY_END()

RULE_CATEGORY(Mercs)
RULE_INT(Mercs, SuspendIntervalMS, 10000)
RULE_INT(Mercs, UpkeepIntervalMS, 180000)
RULE_INT(Mercs, SuspendIntervalS, 10)
RULE_INT(Mercs, UpkeepIntervalS, 180)
RULE_BOOL(Mercs, AllowMercs, false)
RULE_BOOL(Mercs, ChargeMercPurchaseCost, false)
RULE_BOOL(Mercs, ChargeMercUpkeepCost, false)
RULE_INT(Mercs, AggroRadius, 100)		// Determines the distance from which a merc will aggro group member's target(also used to determine the distance at which a healer merc will begin healing a group member)
RULE_INT(Mercs, AggroRadiusPuller, 25)	// Determines the distance from which a merc will aggro group member's target, if they have the group role of puller (also used to determine the distance at which a healer merc will begin healing a group member, if they have the group role of puller)
RULE_INT(Mercs, ResurrectRadius, 50)	// Determines the distance from which a healer merc will attempt to resurrect a group member's corpse
RULE_INT(Mercs, ScaleRate, 100)
RULE_BOOL(Mercs, MercsUsePathing, true) // Mercs will use node pathing when moving
RULE_BOOL(Mercs, AllowMercSuspendInCombat, true)
RULE_CATEGORY_END()

RULE_CATEGORY(Guild)
RULE_INT(Guild, MaxMembers, 2048)
RULE_BOOL(Guild, PlayerCreationAllowed, false)	// Allow players to create a guild using the window in Underfoot+
RULE_INT(Guild, PlayerCreationLimit, 1)		// Only allow use of the UF+ window if the account has < than this number of guild leaders on it
RULE_INT(Guild, PlayerCreationRequiredStatus, 0)	// Required admin status.
RULE_INT(Guild, PlayerCreationRequiredLevel, 0)	// Required Level of the player attempting to create the guild.
RULE_INT(Guild, PlayerCreationRequiredTime, 0)	// Required Time Entitled On Account (in Minutes) to create the guild.
RULE_CATEGORY_END()

RULE_CATEGORY(Skills)
RULE_INT(Skills, MaxTrainTradeskills, 21)
RULE_BOOL(Skills, UseLimitTradeskillSearchSkillDiff, true)
RULE_INT(Skills, MaxTradeskillSearchSkillDiff, 50)
RULE_INT(Skills, MaxTrainSpecializations, 50)	// Max level a GM trainer will train casting specializations
RULE_INT(Skills, SwimmingStartValue, 100)
RULE_BOOL(Skills, TrainSenseHeading, false)
RULE_INT(Skills, SenseHeadingStartValue, 200)
RULE_BOOL(Skills, SelfLanguageLearning, true)
RULE_CATEGORY_END()

RULE_CATEGORY(Pets)
RULE_REAL(Pets, AttackCommandRange, 150)
RULE_BOOL(Pets, UnTargetableSwarmPet, false)
RULE_REAL(Pets, PetPowerLevelCap, 10) // Max number of levels your pet can go up with pet power
RULE_CATEGORY_END()

RULE_CATEGORY(GM)
RULE_INT(GM, MinStatusToSummonItem, 250)
RULE_INT(GM, MinStatusToZoneAnywhere, 250)
RULE_CATEGORY_END()

RULE_CATEGORY(World)
RULE_INT(World, ZoneAutobootTimeoutMS, 60000)
RULE_INT(World, ClientKeepaliveTimeoutMS, 65000)
RULE_BOOL(World, UseBannedIPsTable, false) // Toggle whether or not to check incoming client connections against the Banned_IPs table. Set this value to false to disable this feature.
RULE_BOOL(World, EnableTutorialButton, true)
RULE_BOOL(World, EnableReturnHomeButton, true)
RULE_INT(World, MaxLevelForTutorial, 10)
RULE_INT(World, TutorialZoneID, 189)
RULE_INT(World, GuildBankZoneID, 345)
RULE_INT(World, MinOfflineTimeToReturnHome, 21600) // 21600 seconds is 6 Hours
RULE_INT(World, MaxClientsPerIP, -1) // Maximum number of clients allowed to connect per IP address if account status is < AddMaxClientsStatus. Default value: -1 (feature disabled)
RULE_INT(World, ExemptMaxClientsStatus, -1) // Exempt accounts from the MaxClientsPerIP and AddMaxClientsStatus rules, if their status is >= this value. Default value: -1 (feature disabled)
RULE_INT(World, AddMaxClientsPerIP, -1) // Maximum number of clients allowed to connect per IP address if account status is < ExemptMaxClientsStatus. Default value: -1 (feature disabled)
RULE_INT(World, AddMaxClientsStatus, -1) // Accounts with status >= this rule will be allowed to use the amount of accounts defined in the AddMaxClientsPerIP. Default value: -1 (feature disabled)
RULE_BOOL(World, MaxClientsSetByStatus, false) // If True, IP Limiting will be set to the status on the account as long as the status is > MaxClientsPerIP
RULE_BOOL(World, EnableIPExemptions, false) // If True, ip_exemptions table is used, if there is no entry for the IP it will default to RuleI(World, MaxClientsPerIP)
RULE_BOOL(World, ClearTempMerchantlist, true) // Clears temp merchant items when world boots.
RULE_BOOL(World, DeleteStaleCorpeBackups, true) // Deletes stale corpse backups older than 2 weeks.
RULE_INT(World, AccountSessionLimit, -1) //Max number of characters allowed on at once from a single account (-1 is disabled)
RULE_INT(World, ExemptAccountLimitStatus, -1) //Min status required to be exempt from multi-session per account limiting (-1 is disabled)
RULE_BOOL(World, GMAccountIPList, false) // Check ip list against GM Accounts, AntiHack GM Accounts.
RULE_INT(World, MinGMAntiHackStatus, 1) //Minimum GM status to check against AntiHack list
RULE_INT(World, SoFStartZoneID, -1) //Sets the Starting Zone for SoF Clients separate from Titanium Clients (-1 is disabled)
RULE_INT(World, TitaniumStartZoneID, -1) //Sets the Starting Zone for Titanium Clients (-1 is disabled). Replaces the old method.
RULE_INT(World, ExpansionSettings, 16383) // Sets the expansion settings for the server, This is sent on login to world and affects client expansion settings. Defaults to all expansions enabled up to TSS.
RULE_BOOL(World, UseClientBasedExpansionSettings, true) // if true it will overrule World, ExpansionSettings and set someone's expansion based on the client they're using
RULE_INT(World, PVPSettings, 0) // Sets the PVP settings for the server, 1 = Rallos Zek RuleSet, 2 = Tallon/Vallon Zek Ruleset, 4 = Sullon Zek Ruleset, 6 = Discord Ruleset, anything above 6 is the Discord Ruleset without the no-drop restrictions removed. TODO: Edit IsAttackAllowed in Zone to accomodate for these rules.
RULE_BOOL (World, IsGMPetitionWindowEnabled, false)
RULE_INT (World, FVNoDropFlag, 0) // Sets the Firiona Vie settings on the client. If set to 2, the flag will be set for GMs only, allowing trading of no-drop items.
RULE_BOOL (World, IPLimitDisconnectAll, false)
RULE_BOOL(World, MaxClientsSimplifiedLogic, false) // New logic that only uses ExemptMaxClientsStatus and MaxClientsPerIP. Done on the loginserver. This mimics the P99-style special IP rules.
RULE_INT (World, TellQueueSize, 20)
RULE_BOOL(World, StartZoneSameAsBindOnCreation, true) //Should the start zone ALWAYS be the same location as your bind?
RULE_CATEGORY_END()

RULE_CATEGORY(Zone)
RULE_INT(Zone, ClientLinkdeadMS, 180000) //the time a client remains link dead on the server after a sudden disconnection
RULE_INT(Zone, GraveyardTimeMS, 1200000) //ms time until a player corpse is moved to a zone's graveyard, if one is specified for the zone
RULE_BOOL(Zone, EnableShadowrest, 1) // enables or disables the shadowrest zone feature for player corpses. Default is turned on.
RULE_BOOL(Zone, UsePlayerCorpseBackups, true) // Keeps backups of player corpses.
RULE_INT(Zone, MQWarpExemptStatus, -1) // Required status level to exempt the MQWarpDetector. Set to -1 to disable this feature.
RULE_INT(Zone, MQZoneExemptStatus, -1) // Required status level to exempt the MQZoneDetector. Set to -1 to disable this feature.
RULE_INT(Zone, MQGateExemptStatus, -1) // Required status level to exempt the MQGateDetector. Set to -1 to disable this feature.
RULE_INT(Zone, MQGhostExemptStatus, -1) // Required status level to exempt the MGhostDetector. Set to -1 to disable this feature.
RULE_BOOL(Zone, EnableMQWarpDetector, true) // Enable the MQWarp Detector. Set to False to disable this feature.
RULE_BOOL(Zone, EnableMQZoneDetector, true) // Enable the MQZone Detector. Set to False to disable this feature.
RULE_BOOL(Zone, EnableMQGateDetector, true) // Enable the MQGate Detector. Set to False to disable this feature.
RULE_BOOL(Zone, EnableMQGhostDetector, true) // Enable the MQGhost Detector. Set to False to disable this feature.
RULE_REAL(Zone, MQWarpDetectionDistanceFactor, 9.0) //clients move at 4.4 about if in a straight line but with movement and to acct for lag we raise it a bit
RULE_BOOL(Zone, MarkMQWarpLT, false)
RULE_INT(Zone, AutoShutdownDelay, 5000) //How long a dynamic zone stays loaded while empty
RULE_INT(Zone, PEQZoneReuseTime, 900)	//How long, in seconds, until you can reuse the #peqzone command.
RULE_INT(Zone, PEQZoneDebuff1, 4454)		//First debuff casted by #peqzone Default is Cursed Keeper's Blight.
RULE_INT(Zone, PEQZoneDebuff2, 2209)		//Second debuff casted by #peqzone Default is Tendrils of Apathy.
RULE_BOOL(Zone, UsePEQZoneDebuffs, true)	//Will determine if #peqzone will debuff players or not when used.
RULE_REAL(Zone, HotZoneBonus, 0.75)
RULE_INT(Zone, ReservedInstances, 30) //Will reserve this many instance ids for globals... probably not a good idea to change this while a server is running.
RULE_INT(Zone, EbonCrystalItemID, 40902)
RULE_INT(Zone, RadiantCrystalItemID, 40903)
RULE_BOOL(Zone, LevelBasedEXPMods, false) // Allows you to use the level_exp_mods table in consideration to your players EXP hits
RULE_INT(Zone, WeatherTimer, 600) // Weather timer when no duration is available
RULE_BOOL(Zone, EnableLoggedOffReplenishments, true)
RULE_INT(Zone, MinOfflineTimeToReplenishments, 21600) // 21600 seconds is 6 Hours
RULE_BOOL(Zone, UseZoneController, true) // Enables the ability to use persistent quest based zone controllers (zone_controller.pl/lua)
RULE_BOOL(Zone, EnableZoneControllerGlobals, false) // Enables the ability to use quest globals with the zone controller NPC
RULE_INT(Zone, GlobalLootMultiplier, 1) // Sets Global Loot drop multiplier for database based drops, useful for double, triple loot etc.
RULE_CATEGORY_END()

RULE_CATEGORY(Map)
//enable these to help prevent mob hopping when they are pathing
RULE_BOOL(Map, FixPathingZWhenLoading, true)		//increases zone boot times a bit to reduce hopping.
RULE_BOOL(Map, FixPathingZAtWaypoints, false)	//alternative to `WhenLoading`, accomplishes the same thing but does it at each waypoint instead of once at boot time.
RULE_BOOL(Map, FixPathingZWhenMoving, false)		//very CPU intensive, but helps hopping with widely spaced waypoints.
RULE_BOOL(Map, FixPathingZOnSendTo, false)		//try to repair Z coords in the SendTo routine as well.
RULE_BOOL(Map, FixZWhenMoving, true)		// Automatically fix NPC Z coordinates when moving/pathing/engaged (Far less CPU intensive than its predecessor)
RULE_BOOL(Map, MobZVisualDebug, false)		// Displays spell effects determining whether or not NPC is hitting Best Z calcs (blue for hit, red for miss)
RULE_REAL(Map, FixPathingZMaxDeltaMoving, 20)	//at runtime while pathing: max change in Z to allow the BestZ code to apply.
RULE_REAL(Map, FixPathingZMaxDeltaWaypoint, 20)	//at runtime at each waypoint: max change in Z to allow the BestZ code to apply.
RULE_REAL(Map, FixPathingZMaxDeltaSendTo, 20)	//at runtime in SendTo: max change in Z to allow the BestZ code to apply.
RULE_REAL(Map, FixPathingZMaxDeltaLoading, 45)	//while loading each waypoint: max change in Z to allow the BestZ code to apply.
RULE_INT(Map, FindBestZHeightAdjust, 1)		// Adds this to the current Z before seeking the best Z position
RULE_CATEGORY_END()

RULE_CATEGORY(Pathing)
// Some of these rules may benefit by being made into columns in the zone table,
// for instance, in dungeons, the min LOS distances could be substantially lowered.
RULE_BOOL(Pathing, Aggro, true)		// Enable pathing for aggroed mobs.
RULE_BOOL(Pathing, AggroReturnToGrid, true)	// Enable pathing for aggroed roaming mobs returning to their previous waypoint.
RULE_BOOL(Pathing, Guard, true)		// Enable pathing for mobs moving to their guard point.
RULE_BOOL(Pathing, Find, true)		// Enable pathing for FindPerson requests from the client.
RULE_BOOL(Pathing, Fear, true)		// Enable pathing for fear
RULE_REAL(Pathing, ZDiffThresholdNew, 80)	// If a mob las LOS to it's target, it will run to it if the Z difference is < this.
RULE_INT(Pathing, LOSCheckFrequency, 1000)	// A mob will check for LOS to it's target this often (milliseconds).
RULE_INT(Pathing, RouteUpdateFrequencyShort, 1000)	// How often a new route will be calculated if the target has moved.
RULE_INT(Pathing, RouteUpdateFrequencyLong, 5000)	// How often a new route will be calculated if the target has moved.
// When a path has a path node route and it's target changes position, if it has RouteUpdateFrequencyNodeCount or less nodes to go on it's
// current path, it will recalculate it's path based on the RouteUpdateFrequencyShort timer, otherwise it will use the
// RouteUpdateFrequencyLong timer.
RULE_INT(Pathing, RouteUpdateFrequencyNodeCount, 5)
RULE_REAL(Pathing, MinDistanceForLOSCheckShort, 40000) // (NoRoot). While following a path, only check for LOS to target within this distance.
RULE_REAL(Pathing, MinDistanceForLOSCheckLong, 1000000) // (NoRoot). Min distance when initially attempting to acquire the target.
RULE_INT(Pathing, MinNodesLeftForLOSCheck, 4)	// Only check for LOS when we are down to this many path nodes left to run.
// This next rule was put in for situations where the mob and it's target may be on different sides of a 'hazard', e.g. a pit
// If the mob has LOS to it's target, even though there is a hazard in it's way, it may break off from the node path and run at
// the target, only to later detect the hazard and re-acquire a node path. Depending upon the placement of the path nodes, this
// can lead to the mob looping. The rule is intended to allow the mob to at least get closer to it's target each time before
// checking LOS and trying to head straight for it.
RULE_INT(Pathing, MinNodesTraversedForLOSCheck, 3)	// Only check for LOS after we have traversed this many path nodes.
RULE_INT(Pathing, CullNodesFromStart, 1)		// Checks LOS from Start point to second node for this many nodes and removes first node if there is LOS
RULE_INT(Pathing, CullNodesFromEnd, 1)		// Checks LOS from End point to second to last node for this many nodes and removes last node if there is LOS
RULE_REAL(Pathing, CandidateNodeRangeXY, 400)		// When searching for path start/end nodes, only nodes within this range will be considered.
RULE_REAL(Pathing, CandidateNodeRangeZ, 10)		// When searching for path start/end nodes, only nodes within this range will be considered.
RULE_CATEGORY_END()

RULE_CATEGORY(Watermap)
// enable these to use the water detection code. Requires Water Maps generated by awater utility
RULE_BOOL(Watermap, CheckWaypointsInWaterWhenLoading, false) // Does not apply BestZ as waypoints are loaded if they are in water
RULE_BOOL(Watermap, CheckForWaterAtWaypoints, false)		// Check if a mob has moved into/out of water when at waypoints and sets flymode
RULE_BOOL(Watermap, CheckForWaterWhenMoving, false)		// Checks if a mob has moved into/out of water each time it's loc is recalculated
RULE_BOOL(Watermap, CheckForWaterOnSendTo, false)		// Checks if a mob has moved into/out of water on SendTo
RULE_BOOL(Watermap, CheckForWaterWhenFishing, false)		// Only lets a player fish near water (if a water map exists for the zone)
RULE_REAL(Watermap, FishingRodLength, 30)			// How far in front of player water must be for fishing to work
RULE_REAL(Watermap, FishingLineLength, 100)		// If water is more than this far below the player, it is considered too far to fish
RULE_REAL(Watermap, FishingLineStepSize, 1)		// Basic step size for fishing calc, too small and it will eat cpu, too large and it will miss potential water
RULE_CATEGORY_END()

RULE_CATEGORY(Spells)
RULE_INT(Spells, AutoResistDiff, 15)
RULE_REAL(Spells, ResistChance, 2.0) //chance to resist given no resists and same level
RULE_REAL(Spells, ResistMod, 0.40) //multiplier, chance to resist = this * ResistAmount
RULE_REAL(Spells, PartialHitChance, 0.7) //The chance when a spell is resisted that it will partial hit.
RULE_REAL(Spells, PartialHitChanceFear, 0.25) //The chance when a fear spell is resisted that it will partial hit.
RULE_INT(Spells, BaseCritChance, 0) //base % chance that everyone has to crit a spell
RULE_INT(Spells, BaseCritRatio, 100) //base % bonus to damage on a successful spell crit. 100 = 2x damage
RULE_INT(Spells, WizCritLevel, 12) //level wizards first get spell crits
RULE_INT(Spells, WizCritChance, 7) //wiz's crit chance, on top of BaseCritChance
RULE_INT(Spells, WizCritRatio, 0) //wiz's crit bonus, on top of BaseCritRatio (should be 0 for Live-like)
RULE_INT(Spells, ResistPerLevelDiff, 85) //8.5 resist per level difference.
RULE_INT(Spells, TranslocateTimeLimit, 0) // If not zero, time in seconds to accept a Translocate.
RULE_INT(Spells, SacrificeMinLevel, 46)	//first level Sacrifice will work on
RULE_INT(Spells, SacrificeMaxLevel, 69)	//last level Sacrifice will work on
RULE_INT(Spells, SacrificeItemID, 9963)	//Item ID of the item Sacrifice will return (defaults to an EE)
RULE_BOOL(Spells, EnableSpellGlobals, false)	// If Enabled, spells check the spell_globals table and compare character data from the quest globals before allowing that spell to scribe with scribespells
RULE_INT(Spells, MaxBuffSlotsNPC, 60)	// default to Tit's limit
RULE_INT(Spells, MaxSongSlotsNPC, 0)	// NPCs don't have songs ...
RULE_INT(Spells, MaxDiscSlotsNPC, 0)	// NPCs don't have discs ...
RULE_INT(Spells, MaxTotalSlotsNPC, 60)	// default to Tit's limit
RULE_INT(Spells, MaxTotalSlotsPET, 30)	// default to Tit's limit
RULE_BOOL (Spells, EnableBlockedBuffs, true)
RULE_INT(Spells, ReflectType, 3) //0 = disabled, 1 = single target player spells only, 2 = all player spells, 3 = all single target spells, 4 = all spells
RULE_BOOL(Spells, ReflectMessagesClose, true) // Live functionality is for Reflect messages to show to players within close proximity, false shows just player reflecting
RULE_INT(Spells, VirusSpreadDistance, 30) // The distance a viral spell will jump to its next victim
RULE_BOOL(Spells, LiveLikeFocusEffects, true) // Determines whether specific healing, dmg and mana reduction focuses are randomized
RULE_INT(Spells, BaseImmunityLevel, 55) // The level that targets start to be immune to stun, fear and mez spells with a max level of 0.
RULE_BOOL(Spells, NPCIgnoreBaseImmunity, true) // Whether or not NPCs get to ignore the BaseImmunityLevel for their spells.
RULE_REAL(Spells, AvgSpellProcsPerMinute, 6.0) //Adjust rate for sympathetic spell procs
RULE_INT(Spells, ResistFalloff, 67) //Max that level that will adjust our resist chance based on level modifiers
RULE_INT(Spells, CharismaEffectiveness, 10) // Deterimes how much resist modification charisma applies to charm/pacify checks. Default 10 CHA = -1 resist mod.
RULE_INT(Spells, CharismaEffectivenessCap, 255) // Deterimes how much resist modification charisma applies to charm/pacify checks. Default 10 CHA = -1 resist mod.
RULE_BOOL(Spells, CharismaCharmDuration, false) // Allow CHA resist mod to extend charm duration.
RULE_INT(Spells, CharmBreakCheckChance, 25) //Determines chance for a charm break check to occur each buff tick.
RULE_INT(Spells, MaxCastTimeReduction, 50) //Max percent your spell cast time can be reduced by spell haste
RULE_INT(Spells, RootBreakFromSpells, 55) //Chance for root to break when cast on.
RULE_INT(Spells, DeathSaveCharismaMod, 3) //Determines how much charisma effects chance of death save firing.
RULE_INT(Spells, DivineInterventionHeal, 8000) //Divine intervention heal amount.
RULE_INT(Spells, AdditiveBonusWornType, 0) //Calc worn bonuses to add together (instead of taking highest) if set to THIS worn type. (2=Will covert live items automatically)
RULE_BOOL(Spells, UseCHAScribeHack, false) //ScribeSpells and TrainDiscs quest functions will ignore entries where field 12 is CHA.  What's the best way to do this?
RULE_BOOL(Spells, BuffLevelRestrictions, true) //Buffs will not land on low level toons like live
RULE_INT(Spells, RootBreakCheckChance, 70) //Determines chance for a root break check to occur each buff tick.
RULE_INT(Spells, FearBreakCheckChance, 70) //Determines chance for a fear break check to occur each buff tick.
RULE_INT(Spells, SuccorFailChance, 2) //Determines chance for a succor spell not to teleport an invidual player
RULE_INT(Spells, FRProjectileItem_Titanium, 1113) // Item id for Titanium clients for Fire 'spell projectile'.
RULE_INT(Spells, FRProjectileItem_SOF, 80684) // Item id for SOF clients for Fire 'spell projectile'.
RULE_INT(Spells, FRProjectileItem_NPC, 80684) // Item id for NPC Fire 'spell projectile'.
RULE_BOOL(Spells, UseLiveSpellProjectileGFX, false) // Use spell projectile graphics set in the spells_new table (player_1). Server must be using UF+ spell file.
RULE_BOOL(Spells, FocusCombatProcs, false) //Allow all combat procs to receive focus effects.
RULE_BOOL(Spells, PreNerfBardAEDoT, false) //Allow bard AOE dots to damage targets when moving.
RULE_INT(Spells, AI_SpellCastFinishedFailRecast, 800) // AI spell recast time(MS) when an spell is cast but fails (ie stunned).
RULE_INT(Spells, AI_EngagedNoSpellMinRecast, 500) // AI spell recast time(MS) check when no spell is cast while engaged. (min time in random)
RULE_INT(Spells, AI_EngagedNoSpellMaxRecast, 1000) // AI spell recast time(MS) check when no spell is cast engaged.(max time in random)
RULE_INT(Spells, AI_EngagedBeneficialSelfChance, 100) // Chance during first AI Cast check to do a beneficial spell on self.
RULE_INT(Spells, AI_EngagedBeneficialOtherChance, 25) // Chance during second AI Cast check to do a beneficial spell on others.
RULE_INT(Spells, AI_EngagedDetrimentalChance, 20) // Chance during third AI Cast check to do a determental spell on others.
RULE_INT(Spells, AI_PursueNoSpellMinRecast, 500) // AI spell recast time(MS) check when no spell is cast while chasing target. (min time in random)
RULE_INT(Spells, AI_PursueNoSpellMaxRecast, 2000) // AI spell recast time(MS) check when no spell is cast while chasing target. (max time in random)
RULE_INT(Spells, AI_PursueDetrimentalChance, 90) // Chance while chasing target to cast a detrimental spell.
RULE_INT(Spells, AI_IdleNoSpellMinRecast, 6000) // AI spell recast time(MS) check when no spell is cast while idle. (min time in random)
RULE_INT(Spells, AI_IdleNoSpellMaxRecast, 60000) // AI spell recast time(MS) check when no spell is cast while chasing target. (max time in random)
RULE_INT(Spells, AI_IdleBeneficialChance, 100) // Chance while idle to do a beneficial spell on self or others.
RULE_BOOL(Spells, SHDProcIDOffByOne, true) // pre June 2009 SHD spell procs were off by 1, they stopped doing this in June 2009 (so UF+ spell files need this false)
RULE_BOOL(Spells, Jun182014HundredHandsRevamp, false) // this should be true for if you import a spell file newer than June 18, 2014
RULE_BOOL(Spells, SwarmPetTargetLock, false) // Use old method of swarm pets target locking till target dies then despawning.
RULE_BOOL(Spells, NPC_UseFocusFromSpells, true) // Allow npcs to use most spell derived focus effects.
RULE_BOOL(Spells, NPC_UseFocusFromItems, false) // Allow npcs to use most item derived focus effects.
RULE_BOOL(Spells, UseAdditiveFocusFromWornSlot, false) // Allows an additive focus effect to be calculated from worn slot.
RULE_BOOL(Spells, AlwaysSendTargetsBuffs, false) // ignore LAA level if true
RULE_BOOL(Spells, FlatItemExtraSpellAmt, false) // allow SpellDmg stat to affect all spells, regardless of cast time/cooldown/etc
RULE_BOOL(Spells, IgnoreSpellDmgLvlRestriction, false) // ignore the 5 level spread on applying SpellDmg
RULE_BOOL(Spells, AllowItemTGB, false) // TGB doesn't work with items on live, custom servers want it though
RULE_BOOL(Spells, NPCInnateProcOverride, true) //  NPC innate procs override the target type to single target.
RULE_BOOL(Spells, OldRainTargets, false) // use old incorrectly implemented max targets for rains
RULE_CATEGORY_END()

RULE_CATEGORY(Combat)
RULE_INT(Combat, PetBaseCritChance, 0) // Pet Base crit chance
RULE_INT(Combat, NPCBashKickLevel, 6) //The level that npcs can KICK/BASH
RULE_INT(Combat, NPCBashKickStunChance, 15) //Percent chance that a bash/kick will stun
RULE_INT(Combat, MeleeCritDifficulty, 8900) // lower is easier
RULE_INT(Combat, ArcheryCritDifficulty, 3400) // lower is easier
RULE_INT(Combat, ThrowingCritDifficulty, 1100) // lower is easier
RULE_BOOL(Combat, NPCCanCrit, false) // true allows non PC pet NPCs to crit
RULE_BOOL(Combat, UseIntervalAC, true)
RULE_INT(Combat, PetAttackMagicLevel, 30)
RULE_BOOL(Combat, EnableFearPathing, true)
RULE_REAL(Combat, FleeMultiplier, 2.0) // Determines how quickly a NPC will slow down while fleeing. Decrease multiplier to slow NPC down quicker.
RULE_INT(Combat, FleeHPRatio, 25) //HP % when a NPC begins to flee.
RULE_BOOL(Combat, FleeIfNotAlone, false) // If false, mobs won't flee if other mobs are in combat with it.
RULE_BOOL(Combat, AdjustProcPerMinute, true)
RULE_REAL(Combat, AvgProcsPerMinute, 2.0)
RULE_REAL(Combat, ProcPerMinDexContrib, 0.075)
RULE_REAL(Combat, BaseProcChance, 0.035)
RULE_REAL(Combat, ProcDexDivideBy, 11000)
RULE_BOOL(Combat, AdjustSpecialProcPerMinute, true)  //Set PPM for special abilities like HeadShot (Live does this as of 4-14)
RULE_REAL(Combat, AvgSpecialProcsPerMinute, 2.0) //Unclear what best value is atm.
RULE_REAL(Combat, BaseHitChance, 69.0)
RULE_REAL(Combat, NPCBonusHitChance, 26.0)
RULE_REAL(Combat, HitFalloffMinor, 5.0) //hit will fall off up to 5% over the initial level range
RULE_REAL(Combat, HitFalloffModerate, 7.0) //hit will fall off up to 7% over the three levels after the initial level range
RULE_REAL(Combat, HitFalloffMajor, 50.0) //hit will fall off sharply if we're outside the minor and moderate range
RULE_REAL(Combat, HitBonusPerLevel, 1.2) //You gain this % of hit for every level you are above your target
RULE_REAL(Combat, WeaponSkillFalloff, 0.33) //For every weapon skill point that's not maxed you lose this % of hit
RULE_REAL(Combat, ArcheryHitPenalty, 0.25) //Archery has a hit penalty to try to help balance it with the plethora of long term +hit modifiers for it
RULE_REAL(Combat, AgiHitFactor, 0.01)
RULE_REAL(Combat, MinChancetoHit, 5.0) //Minimum % chance to hit with regular melee/ranged
RULE_REAL(Combat, MaxChancetoHit, 95.0) //Maximum % chance to hit with regular melee/ranged
RULE_INT(Combat, MinRangedAttackDist, 25) //Minimum Distance to use Ranged Attacks
RULE_BOOL(Combat, ArcheryBonusRequiresStationary, true) //does the 2x archery bonus chance require a stationary npc
RULE_REAL(Combat, ArcheryBaseDamageBonus, 1) // % Modifier to Base Archery Damage (.5 = 50% base damage, 1 = 100%, 2 = 200%)
RULE_REAL(Combat, ArcheryNPCMultiplier, 1.0) // this is multiplied by the regular dmg to get the archery dmg
RULE_BOOL(Combat, AssistNoTargetSelf, true) //when assisting a target that does not have a target: true = target self, false = leave target as was before assist (false = live like)
RULE_INT(Combat, MaxRampageTargets, 3) //max number of people hit with rampage
RULE_INT(Combat, DefaultRampageTargets, 1) // default number of people to hit with rampage
RULE_BOOL(Combat, RampageHitsTarget, false) // rampage will hit the target if it still has targets left
RULE_INT(Combat, MaxFlurryHits, 2) //max number of extra hits from flurry
RULE_INT(Combat, MonkDamageTableBonus, 5) //% bonus monks get to their damage table calcs
RULE_INT(Combat, FlyingKickBonus, 25) //% Modifier that this skill gets to str and skill bonuses
RULE_INT(Combat, DragonPunchBonus, 20) //% Modifier that this skill gets to str and skill bonuses
RULE_INT(Combat, EagleStrikeBonus, 15) //% Modifier that this skill gets to str and skill bonuses
RULE_INT(Combat, TigerClawBonus, 10) //% Modifier that this skill gets to str and skill bonuses
RULE_INT(Combat, RoundKickBonus, 5) //% Modifier that this skill gets to str and skill bonuses
RULE_INT(Combat, FrenzyBonus, 0) //% Modifier to damage
RULE_INT(Combat, BackstabBonus, 0) //% Modifier to damage
RULE_BOOL(Combat, ProcTargetOnly, true) //true = procs will only affect our target, false = procs will affect all of our targets
RULE_REAL(Combat, NPCACFactor, 2.25)
RULE_INT(Combat, ClothACSoftcap, 75)
RULE_INT(Combat, LeatherACSoftcap, 100)
RULE_INT(Combat, MonkACSoftcap, 120)
RULE_INT(Combat, ChainACSoftcap, 200)
RULE_INT(Combat, PlateACSoftcap, 300)
RULE_REAL(Combat, AAMitigationACFactor, 3.0)
RULE_REAL(Combat, WarriorACSoftcapReturn, 0.45)
RULE_REAL(Combat, KnightACSoftcapReturn, 0.33)
RULE_REAL(Combat, LowPlateChainACSoftcapReturn, 0.23)
RULE_REAL(Combat, LowChainLeatherACSoftcapReturn, 0.17)
RULE_REAL(Combat, CasterACSoftcapReturn, 0.06)
RULE_REAL(Combat, MiscACSoftcapReturn, 0.3)
RULE_BOOL(Combat, OldACSoftcapRules, false) // use old softcaps
RULE_BOOL(Combat, UseOldDamageIntervalRules, false) // use old damage formulas for everything
RULE_REAL(Combat, WarACSoftcapReturn, 0.3448) // new AC returns
RULE_REAL(Combat, ClrRngMnkBrdACSoftcapReturn, 0.3030)
RULE_REAL(Combat, PalShdACSoftcapReturn, 0.3226)
RULE_REAL(Combat, DruNecWizEncMagACSoftcapReturn, 0.2000)
RULE_REAL(Combat, RogShmBstBerACSoftcapReturn, 0.2500)
RULE_REAL(Combat, SoftcapFactor, 1.88)
RULE_REAL(Combat, ACthac0Factor, 0.55)
RULE_REAL(Combat, ACthac20Factor, 0.55)
RULE_INT(Combat, HitCapPre20, 40) // live has it capped at 40 for whatever dumb reason... this is mainly for custom servers
RULE_INT(Combat, HitCapPre10, 20) // live has it capped at 20, see above :p
RULE_INT(Combat, MinHastedDelay, 400) // how fast we can get with haste.
RULE_REAL(Combat, AvgDefProcsPerMinute, 2.0)
RULE_REAL(Combat, DefProcPerMinAgiContrib, 0.075) //How much agility contributes to defensive proc rate
RULE_INT(Combat, SpecialAttackACBonus, 15) //Percent amount of damage per AC gained for certain special attacks (damage = AC*SpecialAttackACBonus/100).
RULE_INT(Combat, NPCFlurryChance, 20) // Chance for NPC to flurry.
RULE_BOOL (Combat,TauntOverLevel, 1) //Allows you to taunt NPC's over warriors level.
RULE_REAL (Combat,TauntSkillFalloff, 0.33)//For every taunt skill point that's not maxed you lose this % chance to taunt.
RULE_BOOL (Combat,EXPFromDmgShield, false) //Determine if damage from a damage shield counts for EXP gain.
RULE_INT(Combat, MonkACBonusWeight, 15)
RULE_INT(Combat, ClientStunLevel, 55) //This is the level where client kicks and bashes can stun the target
RULE_INT(Combat, QuiverHasteCap, 1000) //Quiver haste cap 1000 on live for a while, currently 700 on live
RULE_BOOL(Combat, UseArcheryBonusRoll, false) //Make the 51+ archery bonus require an actual roll
RULE_INT(Combat, ArcheryBonusChance, 50)
RULE_INT(Combat, BerserkerFrenzyStart, 35)
RULE_INT(Combat, BerserkerFrenzyEnd, 45)
RULE_BOOL(Combat, OneProcPerWeapon, true) //If enabled, One proc per weapon per round
RULE_BOOL(Combat, ProjectileDmgOnImpact, true) //If enabled, projectiles (ie arrows) will hit on impact, instead of instantly.
RULE_BOOL(Combat, MeleePush, true) // enable melee push
RULE_INT(Combat, MeleePushChance, 50) // (NPCs) chance the target will be pushed. Made up, 100 actually isn't that bad
RULE_BOOL(Combat, UseLiveCombatRounds, true) // turn this false if you don't want to worry about fixing up combat rounds for NPCs
RULE_INT(Combat, NPCAssistCap, 5) // Maxiumium number of NPCs that will assist another NPC at once
RULE_INT(Combat, NPCAssistCapTimer, 6000) // Time in milliseconds a NPC will take to clear assist aggro cap space
RULE_BOOL(Combat, UseRevampHandToHand, false) // use h2h revamped dmg/delays I believe this was implemented during SoF
RULE_BOOL(Combat, ClassicMasterWu, false) // classic master wu uses a random special, modern doesn't
RULE_INT(Combat, LevelToStopDamageCaps, 0) // 1 will effectively disable them, 20 should give basically same results as old incorrect system
RULE_CATEGORY_END()

RULE_CATEGORY(NPC)
RULE_INT(NPC, MinorNPCCorpseDecayTimeMS, 450000) //level<55
RULE_INT(NPC, MajorNPCCorpseDecayTimeMS, 1500000) //level>=55
RULE_INT(NPC, CorpseUnlockTimer, 150000)
RULE_INT(NPC, EmptyNPCCorpseDecayTimeMS, 0)
RULE_BOOL (NPC, UseItemBonusesForNonPets, true)
RULE_INT(NPC, SayPauseTimeInSec, 5)
RULE_INT(NPC, OOCRegen, 0)
RULE_BOOL(NPC, BuffFriends, false)
RULE_BOOL(NPC, EnableNPCQuestJournal, false)
RULE_INT(NPC, LastFightingDelayMovingMin, 10000)
RULE_INT(NPC, LastFightingDelayMovingMax, 20000)
RULE_BOOL(NPC, SmartLastFightingDelayMoving, true)
RULE_BOOL(NPC, ReturnNonQuestNoDropItems, false)	// Returns NO DROP items on NPCs that don't have an EVENT_TRADE sub in their script
RULE_INT(NPC, StartEnrageValue, 9) // % HP that an NPC will begin to enrage
RULE_BOOL(NPC, LiveLikeEnrage, false) // If set to true then only player controlled pets will enrage
RULE_BOOL(NPC, EnableMeritBasedFaction, false) // If set to true, faction will given in the same way as experience (solo/group/raid)
RULE_INT(NPC, NPCToNPCAggroTimerMin, 500)
RULE_INT(NPC, NPCToNPCAggroTimerMax, 6000)
RULE_BOOL(NPC, UseClassAsLastName, true) // Uses class archetype as LastName for npcs with none
RULE_BOOL(NPC, NewLevelScaling, true) // Better level scaling, use old if new formulas would break your server
RULE_CATEGORY_END()

RULE_CATEGORY(Aggro)
RULE_BOOL(Aggro, SmartAggroList, true)
RULE_INT(Aggro, SittingAggroMod, 35) //35%
RULE_INT(Aggro, MeleeRangeAggroMod, 10) //10%
RULE_INT(Aggro, CurrentTargetAggroMod, 0) //0% -- will prefer our current target to any other; makes it harder for our npcs to switch targets.
RULE_INT(Aggro, CriticallyWoundedAggroMod, 100) //100%
RULE_INT(Aggro, SpellAggroMod, 100)
RULE_INT(Aggro, SongAggroMod, 33)
RULE_INT(Aggro, PetSpellAggroMod, 10)
RULE_REAL(Aggro, TunnelVisionAggroMod, 0.75) //people not currently the top hate generate this much hate on a Tunnel Vision mob
RULE_INT(Aggro, MaxScalingProcAggro, 400) // Set to -1 for no limit. Maxmimum amount of aggro that HP scaling SPA effect in a proc will add.
RULE_INT(Aggro, IntAggroThreshold, 75) // Int <= this will aggro regardless of level difference.
RULE_BOOL(Aggro, AllowTickPulling, false) // tick pulling is an exploit in an NPC's call for help fixed sometime in 2006 on live
RULE_BOOL(Aggro, UseLevelAggro, true) // Level 18+ and Undead will aggro regardless of level difference. (this will disabled Rule:IntAggroThreshold if set to true)
RULE_INT(Aggro, ClientAggroCheckInterval, 6) // Interval in which clients actually check for aggro - in seconds
RULE_CATEGORY_END()

RULE_CATEGORY(TaskSystem)
RULE_BOOL(TaskSystem, EnableTaskSystem, true) // Globally enable or disable the Task system
RULE_INT(TaskSystem, PeriodicCheckTimer, 5) // Seconds between checks for failed tasks. Also used by the 'Touch' activity
RULE_BOOL(TaskSystem, RecordCompletedTasks, true)
RULE_BOOL(TaskSystem, RecordCompletedOptionalActivities, false)
RULE_BOOL(TaskSystem, KeepOneRecordPerCompletedTask, true)
RULE_BOOL(TaskSystem, EnableTaskProximity, true)
RULE_CATEGORY_END()

RULE_CATEGORY(Range)
RULE_INT(Range, Say, 135)
RULE_INT(Range, Emote, 135)
RULE_INT(Range, BeginCast, 200)
RULE_INT(Range, Anims, 135)
RULE_INT(Range, SpellParticles, 135)
RULE_INT(Range, DamageMessages, 50)
RULE_INT(Range, SpellMessages, 75)
RULE_INT(Range, SongMessages, 75)
RULE_INT(Range, MobPositionUpdates, 600)
RULE_INT(Range, ClientPositionUpdates, 300)
RULE_INT(Range, ClientForceSpawnUpdateRange, 1000)
RULE_INT(Range, CriticalDamage, 80)
RULE_INT(Range, ClientNPCScan, 300)
RULE_CATEGORY_END()


#ifdef BOTS
RULE_CATEGORY(Bots)
RULE_INT(Bots, AAExpansion, 8) // Bots get AAs through this expansion
RULE_BOOL(Bots, AllowCamelCaseNames, false) // Allows the use of 'MyBot' type names
RULE_INT(Bots, CommandSpellRank, 1) // Filters bot command spells by rank (1, 2 and 3 are valid filters - any other number allows all ranks)
RULE_INT(Bots, CreationLimit, 150) // Number of bots that each account can create
RULE_BOOL(Bots, FinishBuffing, false) // Allow for buffs to complete even if the bot caster is out of mana. Only affects buffing out of combat.
RULE_BOOL(Bots, GroupBuffing, false) // Bots will cast single target buffs as group buffs, default is false for single. Does not make single target buffs work for MGB.
RULE_INT(Bots, HealRotationMaxMembers, 24) // Maximum number of heal rotation members
RULE_INT(Bots, HealRotationMaxTargets, 12) // Maximum number of heal rotation targets
RULE_REAL(Bots, ManaRegen, 2.0) // Adjust mana regen for bots, 1 is fast and higher numbers slow it down 3 is about the same as players.
RULE_BOOL(Bots, PreferNoManaCommandSpells, true) // Give sorting priority to newer no-mana spells (i.e., 'Bind Affinity')
RULE_BOOL(Bots, QuestableSpawnLimit, false) // Optional quest method to manage bot spawn limits using the quest_globals name bot_spawn_limit, see: /bazaar/Aediles_Thrall.pl
RULE_BOOL(Bots, QuestableSpells, false) // Anita Thrall's (Anita_Thrall.pl) Bot Spell Scriber quests.
RULE_INT(Bots, SpawnLimit, 71) // Number of bots a character can have spawned at one time, You + 71 bots is a 12 group pseudo-raid (bots are not raidable at this time)
RULE_BOOL(Bots, UpdatePositionWithTimer, false) // Sends a position update with every positive movement timer check
RULE_BOOL(Bots, UsePathing, true) // Bots will use node pathing when moving
RULE_BOOL(Bots, BotGroupXP, false) // Determines whether client gets xp for bots outside their group.
RULE_BOOL(Bots, BotBardUseOutOfCombatSongs, true) // Determines whether bard bots use additional out of combat songs (optional script)
RULE_BOOL(Bots, BotLevelsWithOwner, false) // Auto-updates spawned bots as owner levels/de-levels (false is original behavior)
RULE_BOOL(Bots, BotCharacterLevelEnabled, false) // Enables required level to spawn bots
RULE_INT(Bots, BotCharacterLevel, 0) // 0 as default (if level > this value you can spawn bots if BotCharacterLevelEnabled is true)
RULE_INT(Bots, CasterStopMeleeLevel, 13) // Level at which caster bots stop melee attacks
RULE_CATEGORY_END()
#endif

RULE_CATEGORY(Chat)
RULE_BOOL(Chat, ServerWideOOC, true)
RULE_BOOL(Chat, ServerWideAuction, true)
RULE_BOOL(Chat, EnableVoiceMacros, true)
RULE_BOOL(Chat, EnableMailKeyIPVerification, true)
RULE_BOOL(Chat, EnableAntiSpam, true)
RULE_BOOL(Chat, SuppressCommandErrors, false) // Do not suppress by default
RULE_INT(Chat, MinStatusToBypassAntiSpam, 100)
RULE_INT(Chat, MinimumMessagesPerInterval, 4)
RULE_INT(Chat, MaximumMessagesPerInterval, 12)
RULE_INT(Chat, MaxMessagesBeforeKick, 20)
RULE_INT(Chat, IntervalDurationMS, 60000)
RULE_INT(Chat, KarmaUpdateIntervalMS, 1200000)
RULE_INT(Chat, KarmaGlobalChatLimit, 72) //amount of karma you need to be able to talk in ooc/auction/chat below the level limit
RULE_INT(Chat, GlobalChatLevelLimit, 8) //level limit you need to of reached to talk in ooc/auction/chat if your karma is too low.
RULE_CATEGORY_END()

RULE_CATEGORY(Merchant)
RULE_BOOL(Merchant, UsePriceMod, true) // Use faction/charisma price modifiers.
RULE_REAL(Merchant, SellCostMod, 1.05) // Modifier for NPC sell price.
RULE_REAL(Merchant, BuyCostMod, 0.95) // Modifier for NPC buy price.
RULE_INT(Merchant, PriceBonusPct, 4) // Determines maximum price bonus from having good faction/CHA. Value is a percent.
RULE_INT(Merchant, PricePenaltyPct, 4) // Determines maximum price penalty from having bad faction/CHA. Value is a percent.
RULE_REAL(Merchant, ChaBonusMod, 3.45) // Determines CHA cap, from 104 CHA. 3.45 is 132 CHA at apprehensive. 0.34 is 400 CHA at apprehensive.
RULE_REAL(Merchant, ChaPenaltyMod, 1.52) // Determines CHA bottom, up to 102 CHA. 1.52 is 37 CHA at apprehensive. 0.98 is 0 CHA at apprehensive.
RULE_BOOL(Merchant, EnableAltCurrencySell, true) // Enables the ability to resell items to alternate currency merchants
RULE_CATEGORY_END()

RULE_CATEGORY(Bazaar)
RULE_BOOL(Bazaar, AuditTrail, false)
RULE_INT(Bazaar, MaxSearchResults, 50)
RULE_BOOL(Bazaar, EnableWarpToTrader, true)
RULE_INT(Bazaar, MaxBarterSearchResults, 200) // The max results returned in the /barter search
RULE_CATEGORY_END()

RULE_CATEGORY(Mail)
RULE_BOOL(Mail, EnableMailSystem, true) // If false, client won't bring up the Mail window.
RULE_INT(Mail, ExpireTrash, 0) // Time in seconds. 0 will delete all messages in the trash when the mailserver starts
RULE_INT(Mail, ExpireRead, 31536000) // 1 Year. Set to -1 for never
RULE_INT(Mail, ExpireUnread, 31536000) // 1 Year. Set to -1 for never
RULE_CATEGORY_END()

RULE_CATEGORY(Channels)
RULE_INT(Channels, RequiredStatusAdmin, 251) // Required status to administer chat channels
RULE_INT(Channels, RequiredStatusListAll, 251) // Required status to list all chat channels
RULE_INT(Channels, DeleteTimer, 1440) // Empty password protected channels will be deleted after this many minutes
RULE_CATEGORY_END()

RULE_CATEGORY(EventLog)
RULE_BOOL(EventLog, RecordSellToMerchant, false) // Record sales from a player to an NPC merchant in eventlog table
RULE_BOOL(EventLog, RecordBuyFromMerchant, false) // Record purchases by a player from an NPC merchant in eventlog table
RULE_CATEGORY_END()

RULE_CATEGORY(Adventure)
RULE_INT(Adventure, MinNumberForGroup, 2)
RULE_INT(Adventure, MaxNumberForGroup, 6)
RULE_INT(Adventure, MinNumberForRaid, 18)
RULE_INT(Adventure, MaxNumberForRaid, 36)
RULE_INT(Adventure, MaxLevelRange, 9)
RULE_INT(Adventure, NumberKillsForBossSpawn, 45)
RULE_REAL(Adventure, DistanceForRescueAccept, 10000.0)
RULE_REAL(Adventure, DistanceForRescueComplete, 2500.0)
RULE_INT(Adventure, ItemIDToEnablePorts, 41000) //0 to disable, otherwise using a LDoN portal will require the user to have this item.
RULE_INT(Adventure, LDoNTrapDistanceUse, 625)
RULE_REAL(Adventure, LDoNBaseTrapDifficulty, 15.0)
RULE_REAL(Adventure, LDoNCriticalFailTrapThreshold, 10.0)
RULE_INT(Adventure, LDoNAdventureExpireTime, 1800) //30 minutes to expire
RULE_CATEGORY_END()

RULE_CATEGORY(AA)
RULE_INT(AA, ExpPerPoint, 23976503)	//Amount of exp per AA. Is the same as the amount of exp to go from level 51 to level 52.
RULE_BOOL(AA, Stacking, true) //Allow AA that belong to the same group to stack on SOF+ clients.
RULE_CATEGORY_END()

RULE_CATEGORY(Console)
RULE_INT(Console, SessionTimeOut, 600000)	// Amount of time in ms for the console session to time out
RULE_CATEGORY_END()

RULE_CATEGORY(QueryServ)
RULE_BOOL(QueryServ, PlayerLogChat, false) // Logs Player Chat
RULE_BOOL(QueryServ, PlayerLogTrades, false) // Logs Player Trades
RULE_BOOL(QueryServ, PlayerLogHandins, false) // Logs Player Handins
RULE_BOOL(QueryServ, PlayerLogNPCKills, false) // Logs Player NPC Kills
RULE_BOOL(QueryServ, PlayerLogDeletes, false) // Logs Player Deletes
RULE_BOOL(QueryServ, PlayerLogMoves, false) // Logs Player Moves
RULE_BOOL(QueryServ, PlayerLogMerchantTransactions, false) // Logs Merchant Transactions
RULE_BOOL(QueryServ, PlayerLogPCCoordinates, false) // Logs Player Coordinates with certain events
RULE_BOOL(QueryServ, PlayerLogDropItem, false) // Logs Player Drop Item
RULE_BOOL(QueryServ, PlayerLogZone, false) // Logs Player Zone Events
RULE_BOOL(QueryServ, PlayerLogDeaths, false) // Logs Player Deaths
RULE_BOOL(QueryServ, PlayerLogConnectDisconnect, false) // Logs Player Connect Disconnect State
RULE_BOOL(QueryServ, PlayerLogLevels, false) // Logs Player Leveling/Deleveling
RULE_BOOL(QueryServ, PlayerLogAARate, false) // Logs Player AA Experience Rates
RULE_BOOL(QueryServ, PlayerLogQGlobalUpdate, false) // Logs Player QGlobal Updates
RULE_BOOL(QueryServ, PlayerLogTaskUpdates, false) // Logs Player Task Updates
RULE_BOOL(QueryServ, PlayerLogKeyringAddition, false) // Log PLayer Keyring additions
RULE_BOOL(QueryServ, PlayerLogAAPurchases, false) // Log Player AA Purchases
RULE_BOOL(QueryServ, PlayerLogTradeSkillEvents, false) // Log Player Tradeskill Transactions
RULE_BOOL(QueryServ, PlayerLogIssuedCommandes, false) // Log Player Issued Commands
RULE_BOOL(QueryServ, PlayerLogMoneyTransactions, false) // Log Player Money Transaction/Splits
RULE_BOOL(QueryServ, PlayerLogAlternateCurrencyTransactions, false) // Log Ploayer Alternate Currency Transactions
RULE_CATEGORY_END()

RULE_CATEGORY(Inventory)
RULE_BOOL(Inventory, EnforceAugmentRestriction, true) // Forces augment slot restrictions
RULE_BOOL(Inventory, EnforceAugmentUsability, true) // Forces augmented item usability
RULE_BOOL(Inventory, EnforceAugmentWear, true) // Forces augment wear slot validation
RULE_BOOL(Inventory, DeleteTransformationMold, true) //False if you want mold to last forever
RULE_BOOL(Inventory, AllowAnyWeaponTransformation, false) //Weapons can use any weapon transformation
RULE_BOOL(Inventory, TransformSummonedBags, false) //Transforms summoned bags into disenchanted ones instead of deleting
RULE_CATEGORY_END()

RULE_CATEGORY(Client)
RULE_BOOL(Client, UseLiveFactionMessage, false) // Allows players to see faction adjustments like Live
RULE_BOOL(Client, UseLiveBlockedMessage, false) // Allows players to see faction adjustments like Live
RULE_CATEGORY_END()

#undef RULE_CATEGORY
#undef RULE_INT
#undef RULE_REAL
#undef RULE_BOOL
#undef RULE_CATEGORY_END
