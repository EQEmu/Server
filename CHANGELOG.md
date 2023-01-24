## [22.1.1] - 01/23/2022

### Fixes

* Fix botgrouplist to display unique entries. ([#2785](https://github.com/EQEmu/Server/pull/2785)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-23
* Fix scenario where dereferenced object could be null. ([#2784](https://github.com/EQEmu/Server/pull/2784)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-23

## [22.1.0] - 01/22/2022

This is a first release using the new build system. Changelog entry representative of last year. Subsequent releases will consist of incremental changes since the last release.

### AA

* Fix AA tables dump ([#2769](https://github.com/EQEmu/Server/pull/2769)) ([Akkadius](https://github.com/Akkadius)) 2023-01-22

### AI

* Add Support to Heals to allow Trigger based spells ([#2709](https://github.com/EQEmu/Server/pull/2709)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-08
* Spell Type (1024) InCombatBuff were spam casting ([#2030](https://github.com/EQEmu/Server/pull/2030)) ([noudess](https://github.com/noudess)) 2022-03-07

### API

* Apply spells with custom buff durations and adjust existing spell buff durations. ([#1997](https://github.com/EQEmu/Server/pull/1997)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-15
* Fix for SetBuffDuration function to check bard slots. ([#2009](https://github.com/EQEmu/Server/pull/2009)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-17
* GetNPCStat can now return default stat values. ([#2048](https://github.com/EQEmu/Server/pull/2048)) ([KayenEQ](https://github.com/KayenEQ)) 2022-03-11
* GetNPCStat default better naming ([#2053](https://github.com/EQEmu/Server/pull/2053)) ([KayenEQ](https://github.com/KayenEQ)) 2022-03-13
* Methods for getting more information on quest timers. ([#2060](https://github.com/EQEmu/Server/pull/2060)) ([KayenEQ](https://github.com/KayenEQ)) 2022-04-13
* Perl functions added to apply spell effects directly to NPCs without requiring buffs. ([#1975](https://github.com/EQEmu/Server/pull/1975)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-08
* Perl functions to set invulnerable to and modify environmental damage. ([#2044](https://github.com/EQEmu/Server/pull/2044)) ([KayenEQ](https://github.com/KayenEQ)) 2022-03-09
* Reload API ([#2716](https://github.com/EQEmu/Server/pull/2716)) ([Akkadius](https://github.com/Akkadius)) 2023-01-11
* perl added GetNPCStat(identifier) ([#2012](https://github.com/EQEmu/Server/pull/2012)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-20

### Aggro

* Cleanup Mob::CombatRange ([#2652](https://github.com/EQEmu/Server/pull/2652)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-20
* Rooted mobs will add other hated targets to Hate list ([#2180](https://github.com/EQEmu/Server/pull/2180)) ([noudess](https://github.com/noudess)) 2022-05-27

### Appveyor

* Remove bots preprocessor  ([Akkadius](https://github.com/Akkadius)) 2023-01-20

### Backups

* Use World CLI for Database Backups ([#2286](https://github.com/EQEmu/Server/pull/2286)) ([Akkadius](https://github.com/Akkadius)) 2022-07-07

### Bot/Merc

* Cleanup methods, and virtual overrides. ([#2734](https://github.com/EQEmu/Server/pull/2734)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-15

### Bots

* Add Bot Command Reloading ([#2773](https://github.com/EQEmu/Server/pull/2773)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-22
* Add Bot-specific Spell Settings. ([#2553](https://github.com/EQEmu/Server/pull/2553)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-27
* Add Buff support for Bards under AI_IdleCastChecks ([#2590](https://github.com/EQEmu/Server/pull/2590)) ([Aeadoin](https://github.com/Aeadoin)) 2022-11-28
* Add Data Bucket support to Bot Spell Entries. ([#2505](https://github.com/EQEmu/Server/pull/2505)) ([Aeadoin](https://github.com/Aeadoin)) 2022-11-06
* Add EVENT_TRADE Support to Bots. ([#2560](https://github.com/EQEmu/Server/pull/2560)) ([Aeadoin](https://github.com/Aeadoin)) 2022-11-25
* Add Event_Trade Support for ^inventorygive Command ([#2628](https://github.com/EQEmu/Server/pull/2628)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-11
* Add Expansion Bitmask Quest APIs. ([#2523](https://github.com/EQEmu/Server/pull/2523)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-16
* Add GetBotOwnerByBotID Method ([#2715](https://github.com/EQEmu/Server/pull/2715)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-11
* Add Melee Support for Casting, Cleanup Bot Casting Logic ([#2571](https://github.com/EQEmu/Server/pull/2571)) ([Aeadoin](https://github.com/Aeadoin)) 2022-11-25
* Add Quest API Methods ([#2631](https://github.com/EQEmu/Server/pull/2631)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-12
* Add Quest API Support for Limits. ([#2522](https://github.com/EQEmu/Server/pull/2522)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-17
* Add Rule Allowing Bots to Equip Any Race Items ([#2578](https://github.com/EQEmu/Server/pull/2578)) ([Aeadoin](https://github.com/Aeadoin)) 2022-11-26
* Add Support for AA bonuses that were missing. ([#2764](https://github.com/EQEmu/Server/pull/2764)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-20
* Add Support for Bots to receive Auras, and other AoE Buffs. ([#2586](https://github.com/EQEmu/Server/pull/2586)) ([Aeadoin](https://github.com/Aeadoin)) 2022-11-27
* Add Virtual Override for Bot::Attack ([#2771](https://github.com/EQEmu/Server/pull/2771)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-20
* Add give/remove saylinks to ^itemuse. ([#2503](https://github.com/EQEmu/Server/pull/2503)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-10-30
* Add support for Bot scripting. ([#2515](https://github.com/EQEmu/Server/pull/2515)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-17
* Apply Spells:IgnoreSpellDmgLvlRestriction to bots ([#2024](https://github.com/EQEmu/Server/pull/2024)) ([catapultam-habeo](https://github.com/catapultam-habeo)) 2022-03-07
* Bot::PerformTradeWithClient Cleanup. ([#2084](https://github.com/EQEmu/Server/pull/2084)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-01
* Cleanup Bot Spell Functions, reduce reliance on NPC Functions/Attributes ([#2495](https://github.com/EQEmu/Server/pull/2495)) ([Aeadoin](https://github.com/Aeadoin)) 2022-10-29
* Cleanup Fast Rest Regen ([#2626](https://github.com/EQEmu/Server/pull/2626)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-07
* Cleanup Say Event Parse. ([#2557](https://github.com/EQEmu/Server/pull/2557)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-20
* Cleanup Spell Settings Commands ([#2607](https://github.com/EQEmu/Server/pull/2607)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-04
* Cleanup ^inventoryremove, ^inventorylist, and ^list Commands and bot groups. ([#2273](https://github.com/EQEmu/Server/pull/2273)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-07-03
* Cleanup and remove preprocessors. ([#2757](https://github.com/EQEmu/Server/pull/2757)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-20
* Cleanup various Bot Spell Focus methods ([#2649](https://github.com/EQEmu/Server/pull/2649)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-16
* Convert Load, Save, SaveNew, and Delete to Repositories. ([#2614](https://github.com/EQEmu/Server/pull/2614)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-04
* Expanded Bot Spell Settings List. ([#2606](https://github.com/EQEmu/Server/pull/2606)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-03
* Fix Bot Spell Type "In Combat Buffs" ([#2711](https://github.com/EQEmu/Server/pull/2711)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-08
* Fix Gender not saving as GetBaseGender on BotSave ([#2639](https://github.com/EQEmu/Server/pull/2639)) ([nytmyr](https://github.com/nytmyr)) 2022-12-13
* Fix Slow Query in QueryNameAvailablity ([#2781](https://github.com/EQEmu/Server/pull/2781)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-22
* Fix ^dyearmor command math. ([#2081](https://github.com/EQEmu/Server/pull/2081)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-04-30
* Fix bot spawn when bot id = char_id ([#1984](https://github.com/EQEmu/Server/pull/1984)) ([neckkola](https://github.com/neckkola)) 2022-03-07
* Hotfix for possible crash. ([#2539](https://github.com/EQEmu/Server/pull/2539)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-14
* Melee Bot Support for Spell Settings Commands ([#2599](https://github.com/EQEmu/Server/pull/2599)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-01
* Move Bot Spell Loading process to constructor from calcbotstats() ([#2583](https://github.com/EQEmu/Server/pull/2583)) ([Aeadoin](https://github.com/Aeadoin)) 2022-11-27
* Optimize inventory loading. ([#2588](https://github.com/EQEmu/Server/pull/2588)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-28
* Post pre-processor fixes ([#2770](https://github.com/EQEmu/Server/pull/2770)) ([Akkadius](https://github.com/Akkadius)) 2023-01-20
* Resolve incorrect values on Bot Creation ([#2644](https://github.com/EQEmu/Server/pull/2644)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-14
* Restrict Bot Groups from spawning while Feigned. ([#2761](https://github.com/EQEmu/Server/pull/2761)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-19
* Save Bot Toggle Archer Setting between Loads. ([#2612](https://github.com/EQEmu/Server/pull/2612)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-04
* Update Bot Heal & Damage methods to more closely match Clients + Bugfixes ([#2045](https://github.com/EQEmu/Server/pull/2045)) ([catapultam-habeo](https://github.com/catapultam-habeo)) 2022-03-11
* Update Bot Logic to ignore ST_TargetsTarget when buffing ([#2584](https://github.com/EQEmu/Server/pull/2584)) ([Aeadoin](https://github.com/Aeadoin)) 2022-11-27

### Bug

* Fixed trade items record log ([#2003](https://github.com/EQEmu/Server/pull/2003)) ([cybernine186](https://github.com/cybernine186)) 2022-02-17
* Loot Drop Randomization adjustment ([#2368](https://github.com/EQEmu/Server/pull/2368)) ([fryguy503](https://github.com/fryguy503)) 2022-08-31
* UINT32 EmoteID ([#2369](https://github.com/EQEmu/Server/pull/2369)) ([fryguy503](https://github.com/fryguy503)) 2022-08-13

### Bug Fix

* Boats should never get FixZ'd ([#2246](https://github.com/EQEmu/Server/pull/2246)) ([noudess](https://github.com/noudess)) 2022-07-02
* Clamp Item Ldon Sell Back Rates. ([#2592](https://github.com/EQEmu/Server/pull/2592)) ([Aeadoin](https://github.com/Aeadoin)) 2022-11-30
* Zone Flags Regression ([#2760](https://github.com/EQEmu/Server/pull/2760)) ([Akkadius](https://github.com/Akkadius)) 2023-01-19

### C++20

* Arithmetic on different enums is deprecated ([#2752](https://github.com/EQEmu/Server/pull/2752)) ([mackal](https://github.com/mackal)) 2023-01-17
* Enable C++20 + Fixes + FMT 9.1 ([#2664](https://github.com/EQEmu/Server/pull/2664)) ([Akkadius](https://github.com/Akkadius)) 2022-12-21

### CI

* Hook tests back up ([#2316](https://github.com/EQEmu/Server/pull/2316)) ([Akkadius](https://github.com/Akkadius)) 2022-07-27

### CPP

* Update C++ standard to C++17 ([#2308](https://github.com/EQEmu/Server/pull/2308)) ([mackal](https://github.com/mackal)) 2022-07-27

### Cereal

* Bump to v1.3.2 from v1.2.2 ([#2654](https://github.com/EQEmu/Server/pull/2654)) ([Akkadius](https://github.com/Akkadius)) 2022-12-20

### Client

* Fix IsMoving for Client ([#2318](https://github.com/EQEmu/Server/pull/2318)) ([Akkadius](https://github.com/Akkadius)) 2022-07-27
* Remove unimplemented Client Insight Method. ([#2663](https://github.com/EQEmu/Server/pull/2663)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-21

### Code

* Removed vscode setting ([#2753](https://github.com/EQEmu/Server/pull/2753)) ([xackery](https://github.com/xackery)) 2023-01-17

### Code Cleanup

* Add Validation to varchar number item fields. ([#2241](https://github.com/EQEmu/Server/pull/2241)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-06-04
* Cleanup #kick message. ([#2164](https://github.com/EQEmu/Server/pull/2164)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-10
* Cleanup Haste references and Lua API calls for unsigned to signed. ([#2240](https://github.com/EQEmu/Server/pull/2240)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-06-04
* Cleanup code smells and compiler warnings in common/shareddb ([#2270](https://github.com/EQEmu/Server/pull/2270)) ([Quintinon](https://github.com/Quintinon)) 2022-07-03
* Cleanup magic numbers ([#2662](https://github.com/EQEmu/Server/pull/2662)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-20
* Cleanup spell and max level bucket logic. ([#2181](https://github.com/EQEmu/Server/pull/2181)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-28
* Extra Space in NPC::AISpellsList(). ([#2555](https://github.com/EQEmu/Server/pull/2555)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-20
* Fix unintended copies in zone/zonedb.cpp by changing auto to auto& ([#2271](https://github.com/EQEmu/Server/pull/2271)) ([Quintinon](https://github.com/Quintinon)) 2022-07-03
* Make use of std::abs where possible. ([#2739](https://github.com/EQEmu/Server/pull/2739)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-15
* Merge Client::Attack and Bot::Attack into Mob::Attack ([#2756](https://github.com/EQEmu/Server/pull/2756)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-20
* Move Client::Undye() to client.cpp from #path Command. ([#2188](https://github.com/EQEmu/Server/pull/2188)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-21
* Possible issues with variable/parameter name equality. ([#2161](https://github.com/EQEmu/Server/pull/2161)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-10
* Quest API push methods using invalid types. ([#2172](https://github.com/EQEmu/Server/pull/2172)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-16
* Remove unused basic_functions.h ([#2729](https://github.com/EQEmu/Server/pull/2729)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-13
* Remove unused maxskill.h. ([#2728](https://github.com/EQEmu/Server/pull/2728)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-13
* Remove unused methods. ([#2171](https://github.com/EQEmu/Server/pull/2171)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-15
* Remove unusued Max Item ID Constant ([#2528](https://github.com/EQEmu/Server/pull/2528)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-08
* Rework Lua QuestReward to not use try/catch blocks ([#2417](https://github.com/EQEmu/Server/pull/2417)) ([mackal](https://github.com/mackal)) 2022-09-03
* Send eqstr message in AddAAPoints ([#2507](https://github.com/EQEmu/Server/pull/2507)) ([hgtw](https://github.com/hgtw)) 2022-10-29
* Update to EQEmu #2253 to clean up message strings ([#2279](https://github.com/EQEmu/Server/pull/2279)) ([fryguy503](https://github.com/fryguy503)) 2022-07-03

### Combat

* /shield command "too far away message" ([#1999](https://github.com/EQEmu/Server/pull/1999)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-14
* Basic Combat Recording ([#2090](https://github.com/EQEmu/Server/pull/2090)) ([Akkadius](https://github.com/Akkadius)) 2022-05-01
* Fix Frenzy vs opponents immune to non-magic ([#2095](https://github.com/EQEmu/Server/pull/2095)) ([noudess](https://github.com/noudess)) 2022-05-03
* Fix shield calculation ([#2234](https://github.com/EQEmu/Server/pull/2234)) ([Quintinon](https://github.com/Quintinon)) 2022-06-01
* Legacy Combat Middleware Affected by PR #1858 ([#1939](https://github.com/EQEmu/Server/pull/1939)) ([Akkadius](https://github.com/Akkadius)) 2022-01-30

### Commands

* #bind Typo. ([#2196](https://github.com/EQEmu/Server/pull/2196)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-23
* #ginfo Cleanup. ([#1955](https://github.com/EQEmu/Server/pull/1955)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-03
* #reload Command Overhaul. ([#2162](https://github.com/EQEmu/Server/pull/2162)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-10
* #reload level_mods could cause Non-Booted zones to crash. ([#2670](https://github.com/EQEmu/Server/pull/2670)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-24
* Add #bugs Command. ([#2559](https://github.com/EQEmu/Server/pull/2559)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-22
* Add #feature Command. ([#2142](https://github.com/EQEmu/Server/pull/2142)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-06
* Add #findcharacter Command. ([#2692](https://github.com/EQEmu/Server/pull/2692)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-03
* Add #findrecipe and #viewrecipe Commands. ([#2401](https://github.com/EQEmu/Server/pull/2401)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-08-31
* Add #setanon Command ([#2690](https://github.com/EQEmu/Server/pull/2690)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-03
* Add #suspendmulti Command. ([#2619](https://github.com/EQEmu/Server/pull/2619)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-11
* Add BestZ and Region Data to #loc ([#2245](https://github.com/EQEmu/Server/pull/2245)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-06-05
* Add additional #peqzone functionality. ([#2085](https://github.com/EQEmu/Server/pull/2085)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-01
* Add max_hp back to #modifynpcstat command. ([#2638](https://github.com/EQEmu/Server/pull/2638)) ([nytmyr](https://github.com/nytmyr)) 2022-12-13
* Adding movespeed to #showstats output ([#2596](https://github.com/EQEmu/Server/pull/2596)) ([fryguy503](https://github.com/fryguy503)) 2022-11-30
* Bug fix for #logs command. ([#2008](https://github.com/EQEmu/Server/pull/2008)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-17
* Cleanup #ai Command. ([#1980](https://github.com/EQEmu/Server/pull/1980)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-11
* Cleanup #appearanceeffects Command. ([#2777](https://github.com/EQEmu/Server/pull/2777)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-22
* Cleanup #attack Command. ([#2103](https://github.com/EQEmu/Server/pull/2103)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-04
* Cleanup #ban, #ipban, #flag, #kick, #setlsinfo, and #setpass Commands. ([#2104](https://github.com/EQEmu/Server/pull/2104)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-08
* Cleanup #chat Command. ([#2581](https://github.com/EQEmu/Server/pull/2581)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-27
* Cleanup #corpsefix Command. ([#2197](https://github.com/EQEmu/Server/pull/2197)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-27
* Cleanup #cvs Command. ([#2153](https://github.com/EQEmu/Server/pull/2153)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-08
* Cleanup #date Command. ([#2228](https://github.com/EQEmu/Server/pull/2228)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-06-01
* Cleanup #dbspawn2 Command. ([#2493](https://github.com/EQEmu/Server/pull/2493)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-10-30
* Cleanup #delacct Command. ([#2567](https://github.com/EQEmu/Server/pull/2567)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-22
* Cleanup #depop Command. ([#2536](https://github.com/EQEmu/Server/pull/2536)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-14
* Cleanup #depopzone Command. ([#2537](https://github.com/EQEmu/Server/pull/2537)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-14
* Cleanup #devtools Command. ([#2538](https://github.com/EQEmu/Server/pull/2538)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-14
* Cleanup #doanim Command. ([#2540](https://github.com/EQEmu/Server/pull/2540)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-14
* Cleanup #emote Command. ([#2535](https://github.com/EQEmu/Server/pull/2535)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-14
* Cleanup #emotesearch and #emoteview Command. ([#2494](https://github.com/EQEmu/Server/pull/2494)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-10-30
* Cleanup #emptyinventory Command. ([#2219](https://github.com/EQEmu/Server/pull/2219)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-29
* Cleanup #findaliases and #help Commands. ([#2204](https://github.com/EQEmu/Server/pull/2204)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-27
* Cleanup #findclass and #findrace Commands. ([#2211](https://github.com/EQEmu/Server/pull/2211)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-27
* Cleanup #flagedit Command. ([#1968](https://github.com/EQEmu/Server/pull/1968)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-10
* Cleanup #freeze and #unfreeze Commands. ([#2102](https://github.com/EQEmu/Server/pull/2102)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-04
* Cleanup #gassign Command. ([#2101](https://github.com/EQEmu/Server/pull/2101)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-06
* Cleanup #gearup Command. ([#2589](https://github.com/EQEmu/Server/pull/2589)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-27
* Cleanup #getvariable Command. ([#2100](https://github.com/EQEmu/Server/pull/2100)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-04
* Cleanup #guild Command ([#2693](https://github.com/EQEmu/Server/pull/2693)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-04
* Cleanup #hatelist Command. ([#1976](https://github.com/EQEmu/Server/pull/1976)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-10
* Cleanup #heromodel Command. ([#2566](https://github.com/EQEmu/Server/pull/2566)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-22
* Cleanup #kill Command. ([#2195](https://github.com/EQEmu/Server/pull/2195)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-23
* Cleanup #level Command. ([#2203](https://github.com/EQEmu/Server/pull/2203)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-27
* Cleanup #logs Command. ([#1969](https://github.com/EQEmu/Server/pull/1969)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-10
* Cleanup #makepet Command. ([#2105](https://github.com/EQEmu/Server/pull/2105)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #modifynpcstat Command. ([#2499](https://github.com/EQEmu/Server/pull/2499)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-10-30
* Cleanup #motd Command. ([#2190](https://github.com/EQEmu/Server/pull/2190)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-23
* Cleanup #name Command. ([#1977](https://github.com/EQEmu/Server/pull/1977)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-10
* Cleanup #netstats Command. ([#1970](https://github.com/EQEmu/Server/pull/1970)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-10
* Cleanup #npcedit Command. ([#2582](https://github.com/EQEmu/Server/pull/2582)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-27
* Cleanup #npcedit, #lastname, #title, and #titlesuffix Commands. ([#2215](https://github.com/EQEmu/Server/pull/2215)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-28
* Cleanup #npceditmass command. ([#1957](https://github.com/EQEmu/Server/pull/1957)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-03
* Cleanup #npcemote Command. ([#2106](https://github.com/EQEmu/Server/pull/2106)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #npcloot Command. ([#1974](https://github.com/EQEmu/Server/pull/1974)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-11
* Cleanup #npcsay and #npcshout Commands. ([#2107](https://github.com/EQEmu/Server/pull/2107)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #npcspecialattk Command. ([#2108](https://github.com/EQEmu/Server/pull/2108)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #npctype_cache Command. ([#2109](https://github.com/EQEmu/Server/pull/2109)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #npctypespawn Command. ([#2110](https://github.com/EQEmu/Server/pull/2110)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #nudge Command. ([#2220](https://github.com/EQEmu/Server/pull/2220)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-29
* Cleanup #oocmute Command. ([#2191](https://github.com/EQEmu/Server/pull/2191)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-27
* Cleanup #opcode Command. ([#2547](https://github.com/EQEmu/Server/pull/2547)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-16
* Cleanup #profanity Command. ([#2113](https://github.com/EQEmu/Server/pull/2113)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #push Command. ([#2114](https://github.com/EQEmu/Server/pull/2114)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #qglobal Command. ([#2115](https://github.com/EQEmu/Server/pull/2115)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #randomizefeatures Command. ([#2118](https://github.com/EQEmu/Server/pull/2118)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #refreshgroup Command. ([#2119](https://github.com/EQEmu/Server/pull/2119)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #reloadaa Command. ([#2120](https://github.com/EQEmu/Server/pull/2120)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #reloadallrules Command. ([#2121](https://github.com/EQEmu/Server/pull/2121)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #reloadlevelmods Command. ([#2122](https://github.com/EQEmu/Server/pull/2122)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #reloadmerchants Command. ([#2123](https://github.com/EQEmu/Server/pull/2123)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #reloadperlexportsettings Command. ([#2124](https://github.com/EQEmu/Server/pull/2124)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #reloadrulesworld Command. ([#2128](https://github.com/EQEmu/Server/pull/2128)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #reloadstatic Command. ([#2130](https://github.com/EQEmu/Server/pull/2130)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #reloadtitles Command. ([#2125](https://github.com/EQEmu/Server/pull/2125)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #reloadtraps Command. ([#2126](https://github.com/EQEmu/Server/pull/2126)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #reloadworld and #repop Command. ([#2127](https://github.com/EQEmu/Server/pull/2127)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #reloadzps Command. ([#2129](https://github.com/EQEmu/Server/pull/2129)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #resetaa Command. ([#2132](https://github.com/EQEmu/Server/pull/2132)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #resetaa_timer Command. ([#2131](https://github.com/EQEmu/Server/pull/2131)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #resetdisc_timer Command. ([#2133](https://github.com/EQEmu/Server/pull/2133)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-06
* Cleanup #revoke Command. ([#2134](https://github.com/EQEmu/Server/pull/2134)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #roambox Command. ([#2135](https://github.com/EQEmu/Server/pull/2135)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-06
* Cleanup #rules Command. ([#2593](https://github.com/EQEmu/Server/pull/2593)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-10
* Cleanup #save Command. ([#2136](https://github.com/EQEmu/Server/pull/2136)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-06
* Cleanup #scale Command. ([#2591](https://github.com/EQEmu/Server/pull/2591)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-05
* Cleanup #scribespell and #scribespells Commands. ([#2534](https://github.com/EQEmu/Server/pull/2534)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-14
* Cleanup #sensetrap Command. ([#2137](https://github.com/EQEmu/Server/pull/2137)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #serverinfo Command. ([#2568](https://github.com/EQEmu/Server/pull/2568)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-22
* Cleanup #serverrules Command. ([#2139](https://github.com/EQEmu/Server/pull/2139)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-06
* Cleanup #setlanguage Command. ([#2464](https://github.com/EQEmu/Server/pull/2464)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-10-13
* Cleanup #setskillall Command. ([#1992](https://github.com/EQEmu/Server/pull/1992)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-15
* Cleanup #shownpcgloballoot and #showzonegloballoot Command. ([#2141](https://github.com/EQEmu/Server/pull/2141)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-06
* Cleanup #showskills Command. ([#1994](https://github.com/EQEmu/Server/pull/1994)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-15
* Cleanup #spawneditmass Command. ([#2229](https://github.com/EQEmu/Server/pull/2229)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-06-04
* Cleanup #spawnfix Command. ([#2143](https://github.com/EQEmu/Server/pull/2143)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-06
* Cleanup #spawnstatus Command. ([#2144](https://github.com/EQEmu/Server/pull/2144)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-06
* Cleanup #summon Command. ([#2145](https://github.com/EQEmu/Server/pull/2145)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-06
* Cleanup #summonburiedplayercorpse Command. ([#2146](https://github.com/EQEmu/Server/pull/2146)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #suspend Command. ([#2564](https://github.com/EQEmu/Server/pull/2564)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-22
* Cleanup #task Command. ([#2071](https://github.com/EQEmu/Server/pull/2071)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-04-14
* Cleanup #time and #timezone Command. ([#2147](https://github.com/EQEmu/Server/pull/2147)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #timers Command. ([#2562](https://github.com/EQEmu/Server/pull/2562)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-22
* Cleanup #trapinfo Command. ([#2148](https://github.com/EQEmu/Server/pull/2148)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #ucs Command. ([#2149](https://github.com/EQEmu/Server/pull/2149)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Cleanup #undye and #undyeme Commands. ([#1966](https://github.com/EQEmu/Server/pull/1966)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-10
* Cleanup #unscribespell Command. ([#1998](https://github.com/EQEmu/Server/pull/1998)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-16
* Cleanup #untraindisc Command. ([#1996](https://github.com/EQEmu/Server/pull/1996)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-16
* Cleanup #version Command. ([#1967](https://github.com/EQEmu/Server/pull/1967)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-02-10
* Cleanup #worldwide command. ([#2021](https://github.com/EQEmu/Server/pull/2021)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-03-01
* Cleanup #xtargets Command. ([#2545](https://github.com/EQEmu/Server/pull/2545)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-17
* Cleanup #zone and #zoneinstance Commands. ([#2202](https://github.com/EQEmu/Server/pull/2202)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-27
* Command Status Reload and Helper Method ([#2377](https://github.com/EQEmu/Server/pull/2377)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-08-20
* Consolidate #lock and #unlock Commands into #serverlock. ([#2193](https://github.com/EQEmu/Server/pull/2193)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-23
* Fix #copycharacter command crash ([#2446](https://github.com/EQEmu/Server/pull/2446)) ([Akkadius](https://github.com/Akkadius)) 2022-09-25
* Fix #killallnpcs from crashing ([#2037](https://github.com/EQEmu/Server/pull/2037)) ([Akkadius](https://github.com/Akkadius)) 2022-03-07
* Fix Flymode Command Help Prompt ([#2669](https://github.com/EQEmu/Server/pull/2669)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-23
* Fix typos in #ban and #ipban Commands. ([#2209](https://github.com/EQEmu/Server/pull/2209)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-26
* Make #damage require a target ([#2426](https://github.com/EQEmu/Server/pull/2426)) ([hgtw](https://github.com/hgtw)) 2022-09-05
* Nested Command Aliases ([#2636](https://github.com/EQEmu/Server/pull/2636)) ([Akkadius](https://github.com/Akkadius)) 2022-12-15
* Remove #guildapprove, #guildcreate, and #guildlist Commands ([#2775](https://github.com/EQEmu/Server/pull/2775)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-22
* Remove #iteminfo Command. ([#2565](https://github.com/EQEmu/Server/pull/2565)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-22
* Remove #profiledump and #profilereset Commands. ([#2546](https://github.com/EQEmu/Server/pull/2546)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-16
* Remove #undyeme Command. ([#2776](https://github.com/EQEmu/Server/pull/2776)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-22
* Remove unused #bestz and #pf Commands. ([#2112](https://github.com/EQEmu/Server/pull/2112)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Remove unused/broken #deletegraveyard and #setgraveyard Commands. ([#2198](https://github.com/EQEmu/Server/pull/2198)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-23

### Compile

* Decrease build times using unity build strategy ([#2089](https://github.com/EQEmu/Server/pull/2089)) ([Akkadius](https://github.com/Akkadius)) 2022-05-01

### Crash

* Fix reload crashes ([#2462](https://github.com/EQEmu/Server/pull/2462)) ([Akkadius](https://github.com/Akkadius)) 2022-09-30
* Fix spawn race condition shown by #repop ([#2455](https://github.com/EQEmu/Server/pull/2455)) ([Akkadius](https://github.com/Akkadius)) 2022-09-29
* Linux Crash Dump Improvements ([#2296](https://github.com/EQEmu/Server/pull/2296)) ([Akkadius](https://github.com/Akkadius)) 2022-07-14
* Pointer validation in mob iteration loops ([#2490](https://github.com/EQEmu/Server/pull/2490)) ([Akkadius](https://github.com/Akkadius)) 2022-10-15
* Stability Fixes ([#2489](https://github.com/EQEmu/Server/pull/2489)) ([Akkadius](https://github.com/Akkadius)) 2022-10-15
* Websocket Crash fix race when fetching log categories ([#2456](https://github.com/EQEmu/Server/pull/2456)) ([Akkadius](https://github.com/Akkadius)) 2022-09-29

### Database

* Add Primary ID Keys to Tables ([#2036](https://github.com/EQEmu/Server/pull/2036)) ([Akkadius](https://github.com/Akkadius)) 2022-03-07
* Add fallback migration for logsys columns ([#2457](https://github.com/EQEmu/Server/pull/2457)) ([Akkadius](https://github.com/Akkadius)) 2022-09-29
* Update 2022_01_10_checksum_verification.sql ([#2041](https://github.com/EQEmu/Server/pull/2041)) ([joligario](https://github.com/joligario)) 2022-03-07

### Diawind

* Plus sign markdown fix ([#2727](https://github.com/EQEmu/Server/pull/2727)) ([Akkadius](https://github.com/Akkadius)) 2023-01-12

### Doors

* Fix Misty PoK Stone ([#2482](https://github.com/EQEmu/Server/pull/2482)) ([Akkadius](https://github.com/Akkadius)) 2022-10-14
* Fix Neriak PoK Stone ([#2486](https://github.com/EQEmu/Server/pull/2486)) ([Coreidan](https://github.com/Coreidan)) 2022-10-15
* Fix door target zone heading data ([#2414](https://github.com/EQEmu/Server/pull/2414)) ([Akkadius](https://github.com/Akkadius)) 2022-09-05
* Improvements to door manipulation ([#2370](https://github.com/EQEmu/Server/pull/2370)) ([Akkadius](https://github.com/Akkadius)) 2022-08-13

### Drone

* Speed up drone builds ([#2092](https://github.com/EQEmu/Server/pull/2092)) ([Akkadius](https://github.com/Akkadius)) 2022-05-02

### Expansions

* Expansion Deprecation Revert ([#2312](https://github.com/EQEmu/Server/pull/2312)) ([Akkadius](https://github.com/Akkadius)) 2022-07-15
* Zone expansion consistency changes ([#2380](https://github.com/EQEmu/Server/pull/2380)) ([Akkadius](https://github.com/Akkadius)) 2022-08-22

### Experience

* Change Exp Calculations to be 64 bit where needed. ([#2677](https://github.com/EQEmu/Server/pull/2677)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-31

### Feature

* AA Cap Limit ([#2423](https://github.com/EQEmu/Server/pull/2423)) ([fryguy503](https://github.com/fryguy503)) 2022-10-13
* Add "Keeps Sold Items" Flag to NPCs ([#2671](https://github.com/EQEmu/Server/pull/2671)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-25
* Add Experience Gain Toggle. ([#2676](https://github.com/EQEmu/Server/pull/2676)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-12-30
* Add Guild Chat to Console. ([#2387](https://github.com/EQEmu/Server/pull/2387)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-08-22
* Add Hate Override for Heals ([#2485](https://github.com/EQEmu/Server/pull/2485)) ([Aeadoin](https://github.com/Aeadoin)) 2022-10-14
* Add Rule to Disable Group EXP Modifier. ([#2741](https://github.com/EQEmu/Server/pull/2741)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-15
* Add Support for "Show Mine Only" Filters ([#2484](https://github.com/EQEmu/Server/pull/2484)) ([Aeadoin](https://github.com/Aeadoin)) 2022-10-13
* Add Type 49545 to Spell Resistrictions ([#2436](https://github.com/EQEmu/Server/pull/2436)) ([Aeadoin](https://github.com/Aeadoin)) 2022-09-20
* Add humanoid and non-wielded restrictions to pick pocket ([#2276](https://github.com/EQEmu/Server/pull/2276)) ([noudess](https://github.com/noudess)) 2022-07-03
* Add player /inspect quest event ([#2508](https://github.com/EQEmu/Server/pull/2508)) ([hgtw](https://github.com/hgtw)) 2022-10-29
* Add special ability to block /open ([#2506](https://github.com/EQEmu/Server/pull/2506)) ([hgtw](https://github.com/hgtw)) 2022-10-29
* Allow Focus Effects to be Filtered out. ([#2447](https://github.com/EQEmu/Server/pull/2447)) ([Aeadoin](https://github.com/Aeadoin)) 2022-09-25
* Allow pets to zone with permanent (buffdurationformula 50) buffs to maintain them through zone transitions ([#2035](https://github.com/EQEmu/Server/pull/2035)) ([catapultam-habeo](https://github.com/catapultam-habeo)) 2022-03-07
* Bind Wound and Forage while mounted. ([#2257](https://github.com/EQEmu/Server/pull/2257)) ([fryguy503](https://github.com/fryguy503)) 2022-07-03
* Change #scribespells to be aware of spellgroups & ranks ([#2501](https://github.com/EQEmu/Server/pull/2501)) ([Aeadoin](https://github.com/Aeadoin)) 2022-11-06
* Change GetSkillDmgAmt to int32 ([#2364](https://github.com/EQEmu/Server/pull/2364)) ([Aeadoin](https://github.com/Aeadoin)) 2022-08-10
* Change Lifetap Emotes to be filterable. ([#2454](https://github.com/EQEmu/Server/pull/2454)) ([Aeadoin](https://github.com/Aeadoin)) 2022-09-29
* Change Mana Costs to use Signed Int ([#2384](https://github.com/EQEmu/Server/pull/2384)) ([Aeadoin](https://github.com/Aeadoin)) 2022-08-21
* Change mana_used to int32 ([#2321](https://github.com/EQEmu/Server/pull/2321)) ([Aeadoin](https://github.com/Aeadoin)) 2022-07-30
* Client Checksum Verification (Resubmit old 1678) ([#1922](https://github.com/EQEmu/Server/pull/1922)) ([noudess](https://github.com/noudess)) 2022-03-07
* EQ2-style implied targeting for spells. ([#2032](https://github.com/EQEmu/Server/pull/2032)) ([catapultam-habeo](https://github.com/catapultam-habeo)) 2022-03-07
* Faction Association ([#2408](https://github.com/EQEmu/Server/pull/2408)) ([mackal](https://github.com/mackal)) 2022-09-03
* GM State Change Persistance ([#2328](https://github.com/EQEmu/Server/pull/2328)) ([fryguy503](https://github.com/fryguy503)) 2022-07-31
* Implement Heroic Strikethrough to NPCs ([#2395](https://github.com/EQEmu/Server/pull/2395)) ([Aeadoin](https://github.com/Aeadoin)) 2022-08-31
* Implement OP_CashReward ([#2307](https://github.com/EQEmu/Server/pull/2307)) ([mackal](https://github.com/mackal)) 2022-07-15
* Instance Version Specific Experience Modifiers ([#2376](https://github.com/EQEmu/Server/pull/2376)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-08-20
* NPCs with bows and arrows do ranged attacks ([#2322](https://github.com/EQEmu/Server/pull/2322)) ([mackal](https://github.com/mackal)) 2022-07-30
* Soft Delete Bots on Character Soft Delete ([#2467](https://github.com/EQEmu/Server/pull/2467)) ([Aeadoin](https://github.com/Aeadoin)) 2022-10-13
* Spell Ranks will now work with AllowSpellMemorizeFromItem Rule ([#2475](https://github.com/EQEmu/Server/pull/2475)) ([Aeadoin](https://github.com/Aeadoin)) 2022-10-13
* Update HateMod used by SPA 114 to Int32. ([#2428](https://github.com/EQEmu/Server/pull/2428)) ([Aeadoin](https://github.com/Aeadoin)) 2022-09-08

### Git

* Add Clangd Generated Files to .gitignore ([#2684](https://github.com/EQEmu/Server/pull/2684)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-31

### Hotfix

* Add Bazaar portal discs to SQL  ([Akkadius](https://github.com/Akkadius)) 2022-09-05
* Add discord_webhooks to server tables  ([Akkadius](https://github.com/Akkadius)) 2022-07-03
* Blocks are nested too deeply. ([#2689](https://github.com/EQEmu/Server/pull/2689)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-01
* Cleanup #questerrors Command. ([#2116](https://github.com/EQEmu/Server/pull/2116)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-07
* Compiling fails on FMT 9.1 with Bots ([#2665](https://github.com/EQEmu/Server/pull/2665)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-21
* Correct database call to point to the content_db connection  ([Akkadius](https://github.com/Akkadius)) 2022-06-12
* Corrected misnamed Database Query file for Experience Toggle ([#2683](https://github.com/EQEmu/Server/pull/2683)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-31
* Faction associations file naming / lock consistency  ([Akkadius](https://github.com/Akkadius)) 2022-09-05
* Fix DB version merge  ([Akkadius](https://github.com/Akkadius)) 2022-05-08
* Fix door click crash issue if destination zone doesn't exist  ([Akkadius](https://github.com/Akkadius)) 2023-01-20
* Fix issue with Bot Loading with 0 Health causing buffs to be lost. ([#2552](https://github.com/EQEmu/Server/pull/2552)) ([Aeadoin](https://github.com/Aeadoin)) 2022-11-18
* Fix lua mod load path  ([Akkadius](https://github.com/Akkadius)) 2022-09-29
* Fix merge issue  ([Akkadius](https://github.com/Akkadius)) 2022-07-14
* Fix path load ordering for CLI commands  ([Akkadius](https://github.com/Akkadius)) 2022-10-16
* Fix potential race for crash dumps (Linux)  ([Akkadius](https://github.com/Akkadius)) 2022-07-31
* Fix regression caused by #2129  ([Akkadius](https://github.com/Akkadius)) 2022-05-09
* Flipped positive / negative values for legacy_combat.lua  ([Akkadius](https://github.com/Akkadius)) 2022-06-09
* Force collation on conversion script  ([Akkadius](https://github.com/Akkadius)) 2022-09-28
* Instances Repository Fix ([#2576](https://github.com/EQEmu/Server/pull/2576)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-26
* Login Server failing to compile on Windows. ([#2758](https://github.com/EQEmu/Server/pull/2758)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-19
* Lua Parser Needs Lua_ItemInst ([#2696](https://github.com/EQEmu/Server/pull/2696)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-01
* Make sure we have a proper split size before assuming we can split it  ([Akkadius](https://github.com/Akkadius)) 2023-01-20
* Move discord_webhooks to state tables because we don't want webhooks being exported  ([Akkadius](https://github.com/Akkadius)) 2022-07-03
* Possible windows compile fix  ([Akkadius](https://github.com/Akkadius)) 2022-07-07
* Possible windows compile fix take 2  ([Akkadius](https://github.com/Akkadius)) 2022-07-07
* Remove appveyor fetch bots  ([Akkadius](https://github.com/Akkadius)) 2023-01-21
* Remove expansion field from account for those who have it ([#2357](https://github.com/EQEmu/Server/pull/2357)) ([Akkadius](https://github.com/Akkadius)) 2022-08-01
* Resolve Zone Crashing when grouped with Bots. ([#2747](https://github.com/EQEmu/Server/pull/2747)) ([Aeadoin](https://github.com/Aeadoin)) 2023-01-16
* Resolve issue with Bot Casting after zoning. ([#2617](https://github.com/EQEmu/Server/pull/2617)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-04
* Return weather_type_map  ([Akkadius](https://github.com/Akkadius)) 2022-10-14
* SQL Update  ([Akkadius](https://github.com/Akkadius)) 2022-07-31
* Shared Memory Protection Fixes  ([Akkadius](https://github.com/Akkadius)) 2022-07-27
* Windows compile fix take 3 (final)  ([Akkadius](https://github.com/Akkadius)) 2022-07-07
* fix manifest  ([Akkadius](https://github.com/Akkadius)) 2022-07-16

### Library

* Bump httplib to 0.11.2 ([#2442](https://github.com/EQEmu/Server/pull/2442)) ([Akkadius](https://github.com/Akkadius)) 2022-09-28

### Loading

* Zone Version Loading Fixes ([#2233](https://github.com/EQEmu/Server/pull/2233)) ([Akkadius](https://github.com/Akkadius)) 2022-06-01

### Logging

* Add stack trace in code paths that shouldn't occur ([#2453](https://github.com/EQEmu/Server/pull/2453)) ([Akkadius](https://github.com/Akkadius)) 2022-09-28
* Cleanup AI Logging Events ([#2615](https://github.com/EQEmu/Server/pull/2615)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-04
* Fix log messages to final damage values ([#2056](https://github.com/EQEmu/Server/pull/2056)) ([noudess](https://github.com/noudess)) 2022-03-14
* Fix zoning log typo ([#2478](https://github.com/EQEmu/Server/pull/2478)) ([Akkadius](https://github.com/Akkadius)) 2022-10-11
* Force crash logs to always be on regardless of setting ([#2762](https://github.com/EQEmu/Server/pull/2762)) ([Akkadius](https://github.com/Akkadius)) 2023-01-20
* Improvements to GM Say Logging ([#2765](https://github.com/EQEmu/Server/pull/2765)) ([Akkadius](https://github.com/Akkadius)) 2023-01-20
* Logging Improvements ([#2755](https://github.com/EQEmu/Server/pull/2755)) ([Akkadius](https://github.com/Akkadius)) 2023-01-18
* More AI Logging Cleanup ([#2616](https://github.com/EQEmu/Server/pull/2616)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-10
* Netcode Logging Unify ([#2443](https://github.com/EQEmu/Server/pull/2443)) ([Akkadius](https://github.com/Akkadius)) 2022-09-28
* Remove function prefixes ([#2766](https://github.com/EQEmu/Server/pull/2766)) ([Akkadius](https://github.com/Akkadius)) 2023-01-20
* Remove loginserver unhandled error ([#2458](https://github.com/EQEmu/Server/pull/2458)) ([Akkadius](https://github.com/Akkadius)) 2022-09-29
* Reset stream so we don't bold the whole line  ([Akkadius](https://github.com/Akkadius)) 2023-01-18
* Table Injection - Member Variable Cleanup ([#2281](https://github.com/EQEmu/Server/pull/2281)) ([Akkadius](https://github.com/Akkadius)) 2022-07-07
* Update BUILD_LOGGING=false Blank Aliases ([#2083](https://github.com/EQEmu/Server/pull/2083)) ([Akkadius](https://github.com/Akkadius)) 2022-05-01

### Login

* Added OP_ExpansionPacketData for RoF2 and update payload for Titanium ([#2186](https://github.com/EQEmu/Server/pull/2186)) ([neckkola](https://github.com/neckkola)) 2022-07-14

### Logs

* #logs list Improvements ([#2302](https://github.com/EQEmu/Server/pull/2302)) ([Akkadius](https://github.com/Akkadius)) 2022-07-14
* Fix GMSay Log Regression ([#2298](https://github.com/EQEmu/Server/pull/2298)) ([Akkadius](https://github.com/Akkadius)) 2022-07-14
* Have #reload logs also reload UCS logging ([#2491](https://github.com/EQEmu/Server/pull/2491)) ([Akkadius](https://github.com/Akkadius)) 2022-10-15

### Loot

* Add #lootsim (Loot Simulator) command ([#2375](https://github.com/EQEmu/Server/pull/2375)) ([Akkadius](https://github.com/Akkadius)) 2022-08-20
* Remove unnecessary loot error messages. ([#2261](https://github.com/EQEmu/Server/pull/2261)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-06-12

### Luabind

* Silence deprecation warning ([#2657](https://github.com/EQEmu/Server/pull/2657)) ([Akkadius](https://github.com/Akkadius)) 2022-12-20

### Luamod

* Add CalcSpellEffectValue_formula to luamods ([#2721](https://github.com/EQEmu/Server/pull/2721)) ([Natedog2012](https://github.com/Natedog2012)) 2023-01-11

### Manifest

* Its not_empty not notempty ([#2394](https://github.com/EQEmu/Server/pull/2394)) ([mackal](https://github.com/mackal)) 2022-08-23

### Merchant

* LDoNSellBackRate support for Rule Merchant:EnableAltCurrencySell ([#2570](https://github.com/EQEmu/Server/pull/2570)) ([Aeadoin](https://github.com/Aeadoin)) 2022-11-25

### Mercs

* Add Mercenary Support ([#2745](https://github.com/EQEmu/Server/pull/2745)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-17

### Messages

* Convert messages from Spells to FocusEffect where necessary. ([#2243](https://github.com/EQEmu/Server/pull/2243)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-06-08

### Netcode

* Adjust first packet for compress flag ([#2326](https://github.com/EQEmu/Server/pull/2326)) ([hgtw](https://github.com/hgtw)) 2022-07-27

### Opcode

* Implement SetFace opcode ([#2167](https://github.com/EQEmu/Server/pull/2167)) ([hgtw](https://github.com/hgtw)) 2022-05-11

### Optimization

* Handle channel name filter checks in memory ([#2767](https://github.com/EQEmu/Server/pull/2767)) ([Valorith](https://github.com/Valorith)) 2023-01-20

### Pathing

* Fix pathing z-correctness for certain models ([#2430](https://github.com/EQEmu/Server/pull/2430)) ([Akkadius](https://github.com/Akkadius)) 2022-09-11

### Process

* Process Execution Refactor ([#2632](https://github.com/EQEmu/Server/pull/2632)) ([Akkadius](https://github.com/Akkadius)) 2022-12-11

### QS

* Database class name change ([#2743](https://github.com/EQEmu/Server/pull/2743)) ([Akkadius](https://github.com/Akkadius)) 2023-01-15

### Quests

* Improve Quest Error Handling ([#2635](https://github.com/EQEmu/Server/pull/2635)) ([Akkadius](https://github.com/Akkadius)) 2022-12-13
* Improve Quest Error Handling - Add back in process based syntax validation ([#2646](https://github.com/EQEmu/Server/pull/2646)) ([Akkadius](https://github.com/Akkadius)) 2022-12-15

### Refactor

* Simplify NPC Loading ([#2087](https://github.com/EQEmu/Server/pull/2087)) ([Akkadius](https://github.com/Akkadius)) 2022-05-01

### Regen

* Fix possible overflow in CalcHPRegenCap(). ([#2185](https://github.com/EQEmu/Server/pull/2185)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-27
* Implement Per Second HP Regen for NPCs ([#2086](https://github.com/EQEmu/Server/pull/2086)) ([Akkadius](https://github.com/Akkadius)) 2022-05-01

### Repositories

* Add Bot Repositories. ([#2529](https://github.com/EQEmu/Server/pull/2529)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-16
* Add Cereal support to repository generator ([#2660](https://github.com/EQEmu/Server/pull/2660)) ([Akkadius](https://github.com/Akkadius)) 2022-12-20
* Add GetMaxId, Count ([#2371](https://github.com/EQEmu/Server/pull/2371)) ([Akkadius](https://github.com/Akkadius)) 2022-08-13
* Add more precise types to repository generator ([#2391](https://github.com/EQEmu/Server/pull/2391)) ([mackal](https://github.com/mackal)) 2022-08-31
* Cast floats to avoid grid repository warnings ([#2094](https://github.com/EQEmu/Server/pull/2094)) ([hgtw](https://github.com/hgtw)) 2022-05-02
* Migrate LoadPerlEventExportSettings to use repositories ([#2637](https://github.com/EQEmu/Server/pull/2637)) ([Akkadius](https://github.com/Akkadius)) 2022-12-15
* Modernize character recipe list ([#2385](https://github.com/EQEmu/Server/pull/2385)) ([Akkadius](https://github.com/Akkadius)) 2022-08-22
* Update Character EXP Modifiers Repository ([#2530](https://github.com/EQEmu/Server/pull/2530)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-11-14
* Update repositories ([#2040](https://github.com/EQEmu/Server/pull/2040)) ([Akkadius](https://github.com/Akkadius)) 2022-03-11

### Roambox

* Improve Path Finding ([#2324](https://github.com/EQEmu/Server/pull/2324)) ([noudess](https://github.com/noudess)) 2022-07-30

### Rules

* Add Backstab Rules ([#2666](https://github.com/EQEmu/Server/pull/2666)) ([Valorith](https://github.com/Valorith)) 2022-12-21
* Add Frontal Stun Immunity Rules. ([#2217](https://github.com/EQEmu/Server/pull/2217)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-06-07
* Add Keep Level on Death ([#2319](https://github.com/EQEmu/Server/pull/2319)) ([trentdm](https://github.com/trentdm)) 2022-07-30
* Add LDoN Loot Count Modifier Rule ([#2694](https://github.com/EQEmu/Server/pull/2694)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-03
* Add ManaOnDeath and EndurOnDeath ([#2661](https://github.com/EQEmu/Server/pull/2661)) ([fryguy503](https://github.com/fryguy503)) 2022-12-20
* Add Rule to Disable NPC Last Names. ([#2227](https://github.com/EQEmu/Server/pull/2227)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-06-04
* Add Rule to Enable Tells with #hideme ([#2358](https://github.com/EQEmu/Server/pull/2358)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-08-04
* Add Rule to allow Assassinate on non-Humanoid body types. ([#2331](https://github.com/EQEmu/Server/pull/2331)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-07-29
* Add Rule to allow Headshots on non-Humanoid body types. ([#2329](https://github.com/EQEmu/Server/pull/2329)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-07-29
* Add Rules to disable various item functionalities and cleanup data types. ([#2225](https://github.com/EQEmu/Server/pull/2225)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-06-01
* Add Spells:BuffsFadeOnDeath. ([#2200](https://github.com/EQEmu/Server/pull/2200)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-27
* Add Spells:IllusionsAlwaysPersist. ([#2199](https://github.com/EQEmu/Server/pull/2199)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-27
* Add Toggle for Warrior Shielding ([#2496](https://github.com/EQEmu/Server/pull/2496)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-10-22
* Add adjustment for zone forage. ([#2330](https://github.com/EQEmu/Server/pull/2330)) ([fryguy503](https://github.com/fryguy503)) 2022-07-30
* Add rule for NPC Level Based Buff Restrictions. ([#2708](https://github.com/EQEmu/Server/pull/2708)) ([noudess](https://github.com/noudess)) 2023-01-15
* Add rule to allow players to permanently save chat channels to database, up to a limit. ([#2706](https://github.com/EQEmu/Server/pull/2706)) ([Valorith](https://github.com/Valorith)) 2023-01-19
* Change TradeskillUp Rules to be Floats ([#2674](https://github.com/EQEmu/Server/pull/2674)) ([Aeadoin](https://github.com/Aeadoin)) 2022-12-25
* Cleanup all unused rules. ([#2184](https://github.com/EQEmu/Server/pull/2184)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-23
* Rule Gate Pet Zoning ([#2625](https://github.com/EQEmu/Server/pull/2625)) ([fryguy503](https://github.com/fryguy503)) 2022-12-07
* Rule to allow cap on % XP gain per kill ([#2667](https://github.com/EQEmu/Server/pull/2667)) ([Valorith](https://github.com/Valorith)) 2022-12-25
* Update logic checks everywhere for FVNoDropFlag. ([#2179](https://github.com/EQEmu/Server/pull/2179)) ([Quintinon](https://github.com/Quintinon)) 2022-07-30

### SQL

* Bugs Table Migration  (#2602) ([#2559](https://github.com/EQEmu/Server/pull/2559)) ([joligario](https://github.com/joligario)) 2022-12-01
* Update 2023_01_15_merc_data.sql ([#2763](https://github.com/EQEmu/Server/pull/2763)) ([joligario](https://github.com/joligario)) 2023-01-20

### Saylinks

* Add Silent helper ([#2372](https://github.com/EQEmu/Server/pull/2372)) ([Akkadius](https://github.com/Akkadius)) 2022-08-13
* Convert all GM Command Saylinks to Silent Saylinks. ([#2373](https://github.com/EQEmu/Server/pull/2373)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-08-14
* Inject Saylinks in MessageClose API ([#2335](https://github.com/EQEmu/Server/pull/2335)) ([hgtw](https://github.com/hgtw)) 2022-07-31
* Refactor saylink injection ([#2315](https://github.com/EQEmu/Server/pull/2315)) ([hgtw](https://github.com/hgtw)) 2022-07-27

### Server

* Configuration Issues Checker (LAN Detection) ([#2283](https://github.com/EQEmu/Server/pull/2283)) ([Akkadius](https://github.com/Akkadius)) 2022-07-07

### Skills

* Configurable Exponential Decay Formula for Skill Up ([#1887](https://github.com/EQEmu/Server/pull/1887)) ([mmcgarvey](https://github.com/mmcgarvey)) 2022-01-30

### Spells

* AE Duration effect (Rains) will now work with Target Ring and PBAE spells. ([#2000](https://github.com/EQEmu/Server/pull/2000)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-16
* Allow damage spells to heal if quest based spell mitigation is over 100 pct. ([#1978](https://github.com/EQEmu/Server/pull/1978)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-08
* Bard songs from item clickies should not require components ([#2011](https://github.com/EQEmu/Server/pull/2011)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-18
* Fix for AA and Discipline recast timers being set on spell casting failure. ([#1971](https://github.com/EQEmu/Server/pull/1971)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-08
* Fix for AA recast timers not resetting properly ([#1989](https://github.com/EQEmu/Server/pull/1989)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-11
* Fixes for numhits type 7 counter incrementing incorrectly. ([#2022](https://github.com/EQEmu/Server/pull/2022)) ([KayenEQ](https://github.com/KayenEQ)) 2022-03-02
* Illusions will now persist onto the corpse when mob is killed. ([#1960](https://github.com/EQEmu/Server/pull/1960)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-05
* Invisibility updates and rework ([#1991](https://github.com/EQEmu/Server/pull/1991)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-15
* Major update to Bard song pulsing, Bard item clicks while singing, and spell casting restriction logic. ([#1954](https://github.com/EQEmu/Server/pull/1954)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-07
* SPA 311 SE_LimitCombatSkills should prevent focusing of procs even if proc is a 'casted' spell. ([#1961](https://github.com/EQEmu/Server/pull/1961)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-05
* SPA 79 SE_CurrentHPOnce now will check for focus, critical and partial resist checks, except for buffs. ([#2018](https://github.com/EQEmu/Server/pull/2018)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-24
* Support for 'HateAdded' spell field to apply negative values to reduce hate. ([#1953](https://github.com/EQEmu/Server/pull/1953)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-03
* Support for SPA 194 SE_FadingMemories to use max level checks on aggroed mobs ([#1979](https://github.com/EQEmu/Server/pull/1979)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-09
* Swarm pet aggro logic fix ([#1956](https://github.com/EQEmu/Server/pull/1956)) ([KayenEQ](https://github.com/KayenEQ)) 2022-02-04
* Target's Target Combat Range Rule ([#2274](https://github.com/EQEmu/Server/pull/2274)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-06-24
* Update to target types Beam and Cone to ignore invalid targets. ([#2080](https://github.com/EQEmu/Server/pull/2080)) ([KayenEQ](https://github.com/KayenEQ)) 2022-05-01
* Updates to spell field 'cast not stands' to ignore casting restrictions ([#1938](https://github.com/EQEmu/Server/pull/1938)) ([KayenEQ](https://github.com/KayenEQ)) 2022-01-29

### Strings

* Refactor Strings Usage ([#2305](https://github.com/EQEmu/Server/pull/2305)) ([Akkadius](https://github.com/Akkadius)) 2022-07-14

### Tasks

* Add Task Reward Points Field ([#2317](https://github.com/EQEmu/Server/pull/2317)) ([hgtw](https://github.com/hgtw)) 2022-07-30
* Add method to filter shared task offers ([#2497](https://github.com/EQEmu/Server/pull/2497)) ([hgtw](https://github.com/hgtw)) 2022-10-29
* Add pre-task update event ([#2512](https://github.com/EQEmu/Server/pull/2512)) ([hgtw](https://github.com/hgtw)) 2022-10-30
* Add rule to update multiple task elements ([#2427](https://github.com/EQEmu/Server/pull/2427)) ([hgtw](https://github.com/hgtw)) 2022-09-06
* Add task accept packet validation ([#2354](https://github.com/EQEmu/Server/pull/2354)) ([hgtw](https://github.com/hgtw)) 2022-07-31
* Apply full duration mission replay timers ([#2299](https://github.com/EQEmu/Server/pull/2299)) ([hgtw](https://github.com/hgtw)) 2022-07-14
* Change zone task data container ([#2410](https://github.com/EQEmu/Server/pull/2410)) ([hgtw](https://github.com/hgtw)) 2022-09-03
* Crash fix with data input sanitization ([#2629](https://github.com/EQEmu/Server/pull/2629)) ([Akkadius](https://github.com/Akkadius)) 2022-12-10
* Data validation for zone_version ([#2381](https://github.com/EQEmu/Server/pull/2381)) ([Akkadius](https://github.com/Akkadius)) 2022-08-21
* Fix #task command crash on bad input ([#2301](https://github.com/EQEmu/Server/pull/2301)) ([hgtw](https://github.com/hgtw)) 2022-07-14
* Fix validation loading ([#2230](https://github.com/EQEmu/Server/pull/2230)) ([Akkadius](https://github.com/Akkadius)) 2022-06-01
* Implement Task Goal Match List ([#2097](https://github.com/EQEmu/Server/pull/2097)) ([Akkadius](https://github.com/Akkadius)) 2022-05-07
* Implement task activity prerequisites ([#2374](https://github.com/EQEmu/Server/pull/2374)) ([hgtw](https://github.com/hgtw)) 2022-08-22
* Let task completion event block task rewards ([#2511](https://github.com/EQEmu/Server/pull/2511)) ([hgtw](https://github.com/hgtw)) 2022-10-30
* Let task reward find free bag slots ([#2431](https://github.com/EQEmu/Server/pull/2431)) ([hgtw](https://github.com/hgtw)) 2022-09-18
* Make #task reloadall not quit shared tasks ([#2351](https://github.com/EQEmu/Server/pull/2351)) ([hgtw](https://github.com/hgtw)) 2022-07-31
* Make Task Selector Cooldown Optional ([#2420](https://github.com/EQEmu/Server/pull/2420)) ([hgtw](https://github.com/hgtw)) 2022-09-03
* Only allow shared task completion once ([#2422](https://github.com/EQEmu/Server/pull/2422)) ([hgtw](https://github.com/hgtw)) 2022-09-03
* Only update loot tasks for NPC corpses ([#2513](https://github.com/EQEmu/Server/pull/2513)) ([hgtw](https://github.com/hgtw)) 2022-11-05
* Place task item rewards in free slots ([#2300](https://github.com/EQEmu/Server/pull/2300)) ([hgtw](https://github.com/hgtw)) 2022-07-14
* Remove delivered task items from trades ([#2405](https://github.com/EQEmu/Server/pull/2405)) ([hgtw](https://github.com/hgtw)) 2022-09-02
* Replace task goals with explicit fields ([#2402](https://github.com/EQEmu/Server/pull/2402)) ([hgtw](https://github.com/hgtw)) 2022-09-02
* Reward clients on shared task completion sync ([#2306](https://github.com/EQEmu/Server/pull/2306)) ([hgtw](https://github.com/hgtw)) 2022-07-16
* Schema simplification ([#2449](https://github.com/EQEmu/Server/pull/2449)) ([hgtw](https://github.com/hgtw)) 2022-09-28
* Send Client Message for All Solo Task Updates ([#2336](https://github.com/EQEmu/Server/pull/2336)) ([hgtw](https://github.com/hgtw)) 2022-07-31
* Support Raw NPC Names in Task Goal List ([#2333](https://github.com/EQEmu/Server/pull/2333)) ([hgtw](https://github.com/hgtw)) 2022-07-30
* Tweak task update messages ([#2406](https://github.com/EQEmu/Server/pull/2406)) ([hgtw](https://github.com/hgtw)) 2022-09-02
* Use CashReward for Tasks ([#2332](https://github.com/EQEmu/Server/pull/2332)) ([hgtw](https://github.com/hgtw)) 2022-07-30
* Use dz switch id for task touch events ([#2344](https://github.com/EQEmu/Server/pull/2344)) ([hgtw](https://github.com/hgtw)) 2022-07-31
* Use zone currencies instead of hard-coded enum. ([#2459](https://github.com/EQEmu/Server/pull/2459)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-09-29
* Zone Version Matching ([#2303](https://github.com/EQEmu/Server/pull/2303)) ([Akkadius](https://github.com/Akkadius)) 2022-07-14

### Telnet

* Add guildsay to console commands and Guild Channel to QueueMessage. ([#2263](https://github.com/EQEmu/Server/pull/2263)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-07-03

### Titles

* Cleanup titles, title suffix, and last name methods. ([#2174](https://github.com/EQEmu/Server/pull/2174)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-20

### Typo

* Remove CanTradeFVNoDropItem() Duplicate ([#2352](https://github.com/EQEmu/Server/pull/2352)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-07-31

### UCS

* Auto Client Reconnection ([#2154](https://github.com/EQEmu/Server/pull/2154)) ([Akkadius](https://github.com/Akkadius)) 2022-05-08

### Utility

* Add std::string_view overloads for std::from_chars ([#2392](https://github.com/EQEmu/Server/pull/2392)) ([mackal](https://github.com/mackal)) 2022-08-31

### Validation

* Add Size Validation to #hotfix. ([#2304](https://github.com/EQEmu/Server/pull/2304)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-07-14

### Websocket

* Fix cpp20/gcc11 compile failure ([#2737](https://github.com/EQEmu/Server/pull/2737)) ([Akkadius](https://github.com/Akkadius)) 2023-01-15

### World

* Add more descriptive LS auth erroring ([#2293](https://github.com/EQEmu/Server/pull/2293)) ([Akkadius](https://github.com/Akkadius)) 2022-07-14
* World Bootup Consolidation ([#2294](https://github.com/EQEmu/Server/pull/2294)) ([Akkadius](https://github.com/Akkadius)) 2022-07-14

### XTarget

* Revert All XTarget Corpse Changes ([#1944](https://github.com/EQEmu/Server/pull/1944)) ([mmcgarvey](https://github.com/mmcgarvey)) 2022-01-24

### Zone

* Add missing safe_heading assignment ([#2407](https://github.com/EQEmu/Server/pull/2407)) ([hgtw](https://github.com/hgtw)) 2022-09-02
* Deprecate Zone `expansion` Field ([#2297](https://github.com/EQEmu/Server/pull/2297)) ([Akkadius](https://github.com/Akkadius)) 2022-07-14
* Fix and simplify zone shutdown logic ([#2390](https://github.com/EQEmu/Server/pull/2390)) ([Akkadius](https://github.com/Akkadius)) 2022-08-31

### Zones

* Add Max Level Check to Zones. ([#2714](https://github.com/EQEmu/Server/pull/2714)) ([Kinglykrab](https://github.com/Kinglykrab)) 2023-01-11

### Zoning

* Fix zone race condition ([#2479](https://github.com/EQEmu/Server/pull/2479)) ([Akkadius](https://github.com/Akkadius)) 2022-10-12
* Fix zoning logic issues ([#2412](https://github.com/EQEmu/Server/pull/2412)) ([Akkadius](https://github.com/Akkadius)) 2022-09-03
* Possible zoning under world fix ([#2424](https://github.com/EQEmu/Server/pull/2424)) ([Akkadius](https://github.com/Akkadius)) 2022-10-11
* Revert #2424 ([#2492](https://github.com/EQEmu/Server/pull/2492)) ([Akkadius](https://github.com/Akkadius)) 2022-10-16

### eqemu_server.pl

* Remove non-working fetch_latest_windows_binaries() ([#2445](https://github.com/EQEmu/Server/pull/2445)) ([Akkadius](https://github.com/Akkadius)) 2022-09-25

### int64

* Fix int64 for OOC Regen and GetHP(), GetMaxHP(), GetItemHPBonuses() in Perl/Lua. ([#2218](https://github.com/EQEmu/Server/pull/2218)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-29
* Further int64 cleanup in Perl SetHP() and GetSpellHPBonuses() in Perl/Lua. ([#2222](https://github.com/EQEmu/Server/pull/2222)) ([Kinglykrab](https://github.com/Kinglykrab)) 2022-05-29
* Hate Fixes ([#2163](https://github.com/EQEmu/Server/pull/2163)) ([Akkadius](https://github.com/Akkadius)) 2022-05-10
* Support for HP / Mana / End / Damage / Hate ([#2091](https://github.com/EQEmu/Server/pull/2091)) ([Akkadius](https://github.com/Akkadius)) 2022-05-08
* Windows Compile Fixes ([#2155](https://github.com/EQEmu/Server/pull/2155)) ([Akkadius](https://github.com/Akkadius)) 2022-05-08

### libuv

* Bump to v1.44.2 from v1.26.0 ([#2658](https://github.com/EQEmu/Server/pull/2658)) ([Akkadius](https://github.com/Akkadius)) 2022-12-20
