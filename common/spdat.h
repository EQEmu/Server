/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

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
#ifndef SPDAT_H
#define SPDAT_H

#include "classes.h"
#include "skills.h"

#define SPELL_UNKNOWN 0xFFFF
#define SPELLBOOK_UNKNOWN 0xFFFFFFFF		//player profile spells are 32 bit

//some spell IDs which will prolly change, but are needed
#define SPELL_LEECH_TOUCH 2766
#define SPELL_LAY_ON_HANDS 87
#define SPELL_HARM_TOUCH 88
#define SPELL_HARM_TOUCH2 2821
#define SPELL_IMP_HARM_TOUCH 2774
#define SPELL_NPC_HARM_TOUCH 929


#define EFFECT_COUNT 12
#define MAX_SPELL_TRIGGER 12	// One for each slot(only 6 for AA since AA use 2)
#define MAX_RESISTABLE_EFFECTS 12	// Number of effects that are typcially checked agianst resists.
#define MaxLimitInclude 16 //Number(x 0.5) of focus Limiters that have inclusive checks used when calcing focus effects
#define MAX_SKILL_PROCS 4 //Number of spells to check skill procs from. (This is arbitrary) [Single spell can have multiple proc checks]
#define MAX_SYMPATHETIC_PROCS 10 // Number of sympathetic procs a client can have (This is arbitrary)


const int Z_AGGRO=10;

const int MobAISpellRange=100; // max range of buffs
const int SpellType_Nuke=1;
const int SpellType_Heal=2;
const int SpellType_Root=4;
const int SpellType_Buff=8;
const int SpellType_Escape=16;
const int SpellType_Pet=32;
const int SpellType_Lifetap=64;
const int SpellType_Snare=128;
const int SpellType_DOT=256;
const int SpellType_Dispel=512;
const int SpellType_InCombatBuff=1024;
const int SpellType_Mez=2048;
const int SpellType_Charm=4096;
const int SpellType_Slow = 8192;
const int SpellType_Debuff = 16384;
const int SpellType_Cure = 32768;
const int SpellType_Resurrect = 65536;

const int SpellTypes_Detrimental = SpellType_Nuke|SpellType_Root|SpellType_Lifetap|SpellType_Snare|SpellType_DOT|SpellType_Dispel|SpellType_Mez|SpellType_Charm|SpellType_Debuff|SpellType_Slow;
const int SpellTypes_Beneficial = SpellType_Heal|SpellType_Buff|SpellType_Escape|SpellType_Pet|SpellType_InCombatBuff|SpellType_Cure;

#define SpellType_Any		0xFFFF

enum SpellAffectIndex {
	SAI_Calm			= 12, // Lull and Alliance Spells
	SAI_Dispell_Sight	= 14, // Dispells and Spells like Bind Sight
	SAI_Memory_Blur		= 27,
	SAI_Calm_Song		= 43 // Lull and Alliance Songs
};
enum RESISTTYPE
{
	RESIST_NONE = 0,
	RESIST_MAGIC = 1,
	RESIST_FIRE = 2,
	RESIST_COLD = 3,
	RESIST_POISON = 4,
	RESIST_DISEASE = 5,
	RESIST_CHROMATIC = 6,
	RESIST_PRISMATIC = 7,
	RESIST_PHYSICAL = 8,	// see Muscle Shock, Back Swing
	RESIST_CORRUPTION = 9
};

//Target Type IDs
typedef enum {
/* 01 */	ST_TargetOptional = 0x01,
/* 02 */	ST_AEClientV1 = 0x02,
/* 03 */	ST_GroupTeleport = 0x03,
/* 04 */	ST_AECaster = 0x04,
/* 05 */	ST_Target = 0x05,
/* 06 */	ST_Self = 0x06,
/* 07 */	// NOT USED
/* 08 */	ST_AETarget = 0x08,
/* 09 */	ST_Animal = 0x09,
/* 10 */	ST_Undead = 0x0a,
/* 11 */	ST_Summoned = 0x0b,
/* 12 */	// NOT USED error is 218 (This spell only works on things that are flying.)
/* 13 */	ST_Tap = 0x0d,
/* 14 */	ST_Pet = 0x0e,
/* 15 */	ST_Corpse = 0x0f,
/* 16 */	ST_Plant = 0x10,
/* 17 */	ST_Giant = 0x11, //special giant
/* 18 */	ST_Dragon = 0x12, //special dragon
/* 19 */	// NOT USED error is 227 (This spell only works on specific coldain.)
/* 20 */	ST_TargetAETap = 0x14,
/* 21 */	// NOT USED same switch case as ST_Undead
/* 22 */	// NOT USED same switch case as ST_Summoned
/* 23 */	// NOT USED same switch case as ST_Animal
/* 24 */	ST_UndeadAE = 0x18,
/* 25 */	ST_SummonedAE = 0x19,
/* 26 */	// NOT USED
/* 27 */	// NOT USED error is 223 (This spell only works on insects.)
/* 28 */	// NOT USED error is 223 (This spell only works on insects.)
/* 29 */	// NOT USED
/* 30 */	// NOT USED
/* 31 */	// NOT USED
/* 32 */	ST_AETargetHateList = 0x20,
/* 33 */	ST_HateList = 0x21,
/* 34 */	ST_LDoNChest_Cursed = 0x22,
/* 35 */	ST_Muramite = 0x23, //only works on special muramites
/* 36 */	ST_AreaClientOnly = 0x24,
/* 37 */	ST_AreaNPCOnly = 0x25,
/* 38 */	ST_SummonedPet = 0x26,
/* 39 */	ST_GroupNoPets = 0x27,
/* 40 */	ST_AEBard = 0x28,
/* 41 */	ST_Group = 0x29,
/* 42 */	ST_Directional = 0x2a, //ae around this target between two angles
/* 43 */	ST_GroupClientAndPet = 0x2b,
/* 44 */	ST_Beam = 0x2c,
/* 45 */	ST_Ring = 0x2d,
/* 46 */	ST_TargetsTarget = 0x2e, // uses the target of your target
/* 47 */	ST_PetMaster = 0x2f, // uses the master as target
/* 48 */	// UNKNOWN
/* 49 */	// NOT USED
/* 50 */	ST_TargetAENoPlayersPets = 0x32,
} SpellTargetType;

typedef enum {
	DS_DECAY = 244,
	DS_CHILLED = 245,
	DS_FREEZING = 246,
	DS_TORMENT = 247,
	DS_BURN = 248,
	DS_THORNS = 249
} DmgShieldType;

//Spell Effect IDs
// https://forums.daybreakgames.com/eq/index.php?threads/enumerated-spa-list.206288/
// mirror: http://pastebin.com/MYeQqGwe
#define SE_CurrentHP					0	// implemented - Heals and nukes, repeates every tic if in a buff
#define SE_ArmorClass					1	// implemented
#define SE_ATK							2	// implemented
#define SE_MovementSpeed				3	// implemented - SoW, SoC, etc
#define SE_STR							4	// implemented
#define SE_DEX							5	// implemented
#define SE_AGI							6	// implemented
#define SE_STA							7	// implemented
#define SE_INT							8	// implemented
#define SE_WIS							9	// implemented
#define SE_CHA							10	// implemented - used as a spacer
#define SE_AttackSpeed					11	// implemented
#define SE_Invisibility					12	// implemented - TO DO: Implemented Invisiblity Levels
#define SE_SeeInvis						13	// implemented - TO DO: Implemented See Invisiblity Levels
#define SE_WaterBreathing				14	// implemented
#define SE_CurrentMana					15	// implemented
//#define SE_NPCFrenzy					16	// not used
//#define SE_NPCAwareness				17	// not used
#define SE_Lull							18	// implemented - Reaction Radius
#define SE_AddFaction					19	// implemented - Alliance line
#define SE_Blind						20	// implemented
#define SE_Stun							21	// implemented
#define SE_Charm						22	// implemented
#define SE_Fear							23	// implemented
#define SE_Stamina						24	// implemented - Invigor and such
#define SE_BindAffinity					25	// implemented - TO DO: Implement 2nd and 3rd Recall (value 2,3 ect). Sets additional bind points.
#define SE_Gate							26	// implemented - Gate to bind point
#define SE_CancelMagic					27	// implemented
#define SE_InvisVsUndead				28	// implemented
#define SE_InvisVsAnimals				29	// implemented
#define SE_ChangeFrenzyRad				30	// implemented - Pacify
#define SE_Mez							31	// implemented
#define SE_SummonItem					32	// implemented
#define SE_SummonPet					33	// implemented
//#define SE_Confuse					34	// not used (Nimbus of Temporal Rifting) ?
#define SE_DiseaseCounter				35	// implemented
#define SE_PoisonCounter				36	// implemented
//#define SE_DetectHostile				37	// not used
//#define SE_DetectMagic				38	// not used
//#define SE_DetectPoison				39	// not used
#define SE_DivineAura					40	// implemented
#define SE_Destroy						41	// implemented - Disintegrate, Banishment of Shadows
#define SE_ShadowStep					42	// implemented
#define SE_Berserk						43	// implemented (*not used in any known live spell) Makes client 'Berserk' giving crip blow chance.
#define SE_Lycanthropy					44	// implemented
#define SE_Vampirism					45	// implemented (*not used in any known live spell) Stackable lifetap from melee.
#define SE_ResistFire					46	// implemented
#define SE_ResistCold					47	// implemented
#define SE_ResistPoison					48	// implemented
#define SE_ResistDisease				49	// implemented
#define SE_ResistMagic					50	// implemented
//#define SE_DetectTraps				51	// not used
#define SE_SenseDead					52	// implemented
#define SE_SenseSummoned				53	// implemented
#define SE_SenseAnimals					54	// implemented
#define SE_Rune							55	// implemented
#define SE_TrueNorth					56	// implemented
#define SE_Levitate						57	// implemented
#define SE_Illusion						58	// implemented
#define SE_DamageShield					59	// implemented
//#define SE_TransferItem				60	// not used
#define SE_Identify						61	// implemented
//#define SE_ItemID						62	// not used
#define SE_WipeHateList					63	// implemented
#define SE_SpinTarget					64	// implemented - TO DO: Not sure stun portion is working correctly
#define SE_InfraVision					65	// implemented
#define SE_UltraVision					66	// implemented
#define SE_EyeOfZomm					67	// implemented
#define SE_ReclaimPet					68	// implemented
#define SE_TotalHP						69	// implemented
//#define SE_CorpseBomb					70	// not used
#define SE_NecPet						71	// implemented
//#define SE_PreserveCorpse				72	// not used
#define SE_BindSight					73	// implemented
#define SE_FeignDeath					74	// implemented
#define SE_VoiceGraft					75	// implemented
#define SE_Sentinel						76	// *not implemented?(just seems to send a message)
#define SE_LocateCorpse					77	// implemented
#define SE_AbsorbMagicAtt				78	// implemented - Rune for spells
#define SE_CurrentHPOnce				79	// implemented - Heals and nukes, non-repeating if in a buff
//#define SE_EnchantLight				80	// not used
#define SE_Revive						81	// implemented - Resurrect
#define SE_SummonPC						82	// implemented
#define SE_Teleport						83	// implemented
#define SE_TossUp						84	// implemented - Gravity Flux
#define SE_WeaponProc					85	// implemented - i.e. Call of Fire
#define SE_Harmony						86	// implemented
#define SE_MagnifyVision				87	// implemented - Telescope
#define SE_Succor						88	// implemented - Evacuate/Succor lines
#define SE_ModelSize					89	// implemented - Shrink, Growth
//#define SE_Cloak						90	// *not implemented - Used in only 2 spells
#define SE_SummonCorpse					91	// implemented
#define SE_InstantHate					92	// implemented - add hate
#define SE_StopRain						93	// implemented - Wake of Karana
#define SE_NegateIfCombat				94	// implemented
#define SE_Sacrifice					95	// implemented
#define SE_Silence						96	// implemented
#define SE_ManaPool						97	// implemented
#define SE_AttackSpeed2					98	// implemented - Melody of Ervaj
#define SE_Root							99	// implemented
#define SE_HealOverTime					100	// implemented
#define SE_CompleteHeal					101	// implemented
#define SE_Fearless						102	// implemented - Valiant Companion
#define SE_CallPet						103	// implemented - Summon Companion
#define SE_Translocate					104	// implemented
#define SE_AntiGate						105	// implemented - Translocational Anchor
#define SE_SummonBSTPet					106	// implemented
#define SE_AlterNPCLevel				107	// implemented - not used on live
#define SE_Familiar						108	// implemented
#define SE_SummonItemIntoBag			109	// implemented - summons stuff into container
//#define SE_IncreaseArchery			110	// not used
#define SE_ResistAll					111	// implemented - Note: Physical Resists are not modified by this effect.
#define SE_CastingLevel					112	// implemented
#define	SE_SummonHorse					113	// implemented
#define SE_ChangeAggro					114	// implemented - Hate modifing buffs(ie horrifying visage)
#define SE_Hunger						115	// implemented - Song of Sustenance
#define SE_CurseCounter					116	// implemented
#define SE_MagicWeapon					117	// implemented - makes weapon magical
#define SE_Amplification				118	// implemented - Harmonize/Amplification (stacks with other singing mods)
#define SE_AttackSpeed3					119	// implemented
#define SE_HealRate						120	// implemented - reduces healing by a %
#define SE_ReverseDS					121 // implemented
//#define SE_ReduceSkill				122	// not used
#define SE_Screech						123	// implemented Spell Blocker(If have buff with value +1 will block any effect with -1)
#define SE_ImprovedDamage				124 // implemented
#define SE_ImprovedHeal					125 // implemented
#define SE_SpellResistReduction			126 // implemented
#define SE_IncreaseSpellHaste			127 // implemented
#define SE_IncreaseSpellDuration		128 // implemented
#define SE_IncreaseRange				129 // implemented
#define SE_SpellHateMod					130 // implemented
#define SE_ReduceReagentCost			131 // implemented
#define SE_ReduceManaCost				132 // implemented
#define SE_FcStunTimeMod				133	// implemented - Modify duration of stuns.
#define SE_LimitMaxLevel				134 // implemented
#define SE_LimitResist					135 // implemented
#define SE_LimitTarget					136 // implemented
#define SE_LimitEffect					137 // implemented
#define SE_LimitSpellType				138 // implemented
#define SE_LimitSpell					139 // implemented
#define SE_LimitMinDur					140 // implemented
#define SE_LimitInstant					141 // implemented
#define SE_LimitMinLevel				142 // implemented
#define SE_LimitCastTimeMin				143 // implemented
#define SE_LimitCastTimeMax				144	// implemented (*not used in any known live spell)
#define SE_Teleport2					145	// implemented - Banishment of the Pantheon
//#define SE_ElectricityResist			146	// *not implemented (Lightning Rod: 23233)
#define SE_PercentalHeal				147 // implemented
#define SE_StackingCommand_Block		148 // implemented?
#define SE_StackingCommand_Overwrite	149 // implemented?
#define SE_DeathSave					150 // implemented
#define SE_SuspendPet					151	// *not implemented as bonus
#define SE_TemporaryPets				152	// implemented
#define SE_BalanceHP					153 // implemented
#define SE_DispelDetrimental			154 // implemented
#define SE_SpellCritDmgIncrease			155 // implemented - no known live spells use this currently
#define SE_IllusionCopy					156	// implemented - Deception
#define SE_SpellDamageShield			157	// implemented - Petrad's Protection
#define SE_Reflect						158 // implemented
#define SE_AllStats						159	// implemented
//#define SE_MakeDrunk					160 // *not implemented - Effect works entirely client side (Should check against tolerance)
#define SE_MitigateSpellDamage			161	// implemented - rune with max value
#define SE_MitigateMeleeDamage			162	// implemented - rune with max value
#define SE_NegateAttacks				163	// implemented
#define SE_AppraiseLDonChest			164	// implemented
#define SE_DisarmLDoNTrap				165	// implemented
#define SE_UnlockLDoNChest				166	// implemented
#define SE_PetPowerIncrease				167 // implemented
#define SE_MeleeMitigation				168	// implemented
#define SE_CriticalHitChance			169	// implemented
#define SE_SpellCritChance				170	// implemented
#define SE_CrippBlowChance				171	// implemented
#define SE_AvoidMeleeChance				172	// implemented
#define SE_RiposteChance				173	// implemented
#define SE_DodgeChance					174	// implemented
#define SE_ParryChance					175	// implemented
#define SE_DualWieldChance				176	// implemented
#define SE_DoubleAttackChance			177	// implemented
#define SE_MeleeLifetap					178	// implemented
#define SE_AllInstrumentMod				179	// implemented
#define SE_ResistSpellChance			180	// implemented
#define SE_ResistFearChance				181	// implemented
#define SE_HundredHands					182	// implemented
#define SE_MeleeSkillCheck				183	// implemented
#define SE_HitChance					184	// implemented
#define SE_DamageModifier				185	// implemented
#define SE_MinDamageModifier			186	// implemented
#define SE_BalanceMana					187	// implemented - Balances party mana
#define SE_IncreaseBlockChance			188	// implemented
#define SE_CurrentEndurance				189	// implemented
#define SE_EndurancePool				190	// implemented
#define SE_Amnesia						191	// implemented - Silence vs Melee Effect
#define SE_Hate							192	// implemented - Instant and hate over time.
#define SE_SkillAttack					193	// implemented
#define SE_FadingMemories				194	// implemented
#define SE_StunResist					195	// implemented
#define SE_StrikeThrough				196	// implemented
#define SE_SkillDamageTaken				197	// implemented
#define SE_CurrentEnduranceOnce			198	// implemented
#define SE_Taunt						199	// implemented - % chance to taunt the target
#define SE_ProcChance					200	// implemented
#define SE_RangedProc					201	// implemented
#define SE_IllusionOther				202	// implemented - Project Illusion
#define SE_MassGroupBuff				203	// implemented
#define SE_GroupFearImmunity			204	// implemented - (Does not use bonus)
#define SE_Rampage						205	// implemented
#define SE_AETaunt						206	// implemented
#define SE_FleshToBone					207	// implemented
//#define SE_PurgePoison				208	// not used
#define SE_DispelBeneficial				209 // implemented
//#define SE_PetShield					210	// *not implemented
#define SE_AEMelee						211	// implemented TO DO: Implement to allow NPC use (client only atm).
#define SE_FrenziedDevastation			212	// implemented - increase spell criticals + all DD spells cast 2x mana.
#define SE_PetMaxHP						213	// implemented[AA] - increases the maximum hit points of your pet
#define SE_MaxHPChange					214	// implemented
#define SE_PetAvoidance					215	// implemented[AA] - increases pet ability to avoid melee damage
#define SE_Accuracy						216	// implemented
#define SE_HeadShot						217	// implemented - ability to head shot (base2 = damage)
#define SE_PetCriticalHit				218 // implemented[AA] - gives pets a baseline critical hit chance
#define SE_SlayUndead					219	// implemented - Allow extra damage against undead (base1 = rate, base2 = damage mod).
#define SE_SkillDamageAmount			220	// implemented
#define SE_Packrat						221 // implemented as bonus
#define SE_BlockBehind					222	// implemented - Chance to block from behind (with our without Shield)
#define SE_DoubleRiposte				223	// implemented - Chance to double riposte [not used on live]
#define	SE_GiveDoubleRiposte			224 // implemented[AA]
#define SE_GiveDoubleAttack				225	// implemented[AA] - Allow any class to double attack with set chance.
#define SE_TwoHandBash					226 // *not implemented as bonus
#define SE_ReduceSkillTimer				227	// implemented
#define SE_ReduceFallDamage				228	// implented - reduce the damage that you take from falling
#define SE_PersistantCasting			229 // implemented
#define SE_ExtendedShielding			230	// not used as bonus - increase range of /shield ability
#define SE_StunBashChance				231	// implemented - increase chance to stun from bash.
#define SE_DivineSave					232	// implemented (base1 == % chance on death to insta-res) (base2 == spell cast on save)
#define SE_Metabolism					233	// implemented - Modifies food/drink consumption rates.
#define SE_ReduceApplyPoisonTime		234	// not implemented as bonus - reduces the time to apply poison
#define	SE_ChannelChanceSpells			235 // implemented[AA] - chance to channel from SPELLS *No longer used on live.
//#define SE_FreePet					236	// not used
#define SE_GivePetGroupTarget			237 // implemented[AA] - (Pet Affinity)
#define SE_IllusionPersistence			238	// implemented - lends persistence to your illusionary disguises, causing them to last until you die or the illusion is forcibly removed.
#define SE_FeignedCastOnChance			239	// implemented - ability gives you an increasing chance for your feigned deaths to not be revealed by spells cast upon you.
//#define SE_StringUnbreakable			240	// not used [Likely related to above - you become immune to feign breaking on a resisted spell and have a good chance of feigning through a spell that successfully lands upon you.]
#define SE_ImprovedReclaimEnergy		241	// implemented - increase the amount of mana returned to you when reclaiming your pet.
#define SE_IncreaseChanceMemwipe		242	// implemented - increases the chance to wipe hate with memory blurr
#define SE_CharmBreakChance				243	// implemented - Total Domination
#define	SE_RootBreakChance				244	// implemented[AA] reduce the chance that your root will break.
#define SE_TrapCircumvention			245	// *not implemented[AA] - decreases the chance that you will set off a trap when opening a chest
#define SE_SetBreathLevel				246 // *not implemented as bonus
#define SE_RaiseSkillCap				247	// implemented[AA] - adds skill over the skill cap.
#define SE_SecondaryForte				248 // not implemented as bonus(gives you a 2nd specialize skill that can go past 50 to 100)
#define SE_SecondaryDmgInc				249 // implemented[AA] Allows off hand weapon to recieve a damage bonus (Sinister Strikes)
#define SE_SpellProcChance				250	// implemented - Increase chance to proc from melee proc spells (ie Spirit of Panther)
#define SE_ConsumeProjectile			251	// implemented[AA] - chance to not consume an arrow (ConsumeProjectile = 100)
#define SE_FrontalBackstabChance		252	// implemented[AA] - chance to perform a full damage backstab from front.
#define SE_FrontalBackstabMinDmg		253	// implemented[AA] - allow a frontal backstab for mininum damage.
#define SE_Blank						254 // implemented
#define SE_ShieldDuration				255	// not implemented as bonus - increases duration of /shield
#define SE_ShroudofStealth				256	// implemented
#define SE_PetDiscipline				257 // not implemented as bonus - /pet hold
#define SE_TripleBackstab				258 // implemented[AA] - chance to perform a triple backstab
#define SE_CombatStability				259 // implemented[AA] - damage mitigation
#define SE_AddSingingMod				260 // implemented[AA] - Instrument/Singing Mastery, base1 is the mod, base2 is the ItemType
#define SE_SongModCap					261	// implemented[AA] - Song Mod cap increase (no longer used on live)
#define SE_RaiseStatCap					262 // implemented
#define SE_TradeSkillMastery			263	// implemented - lets you raise more than one tradeskill above master.
#define SE_HastenedAASkill			    264 // implemented
#define SE_MasteryofPast				265 // implemented[AA] - Spells less than effect values level can not be fizzled
#define SE_ExtraAttackChance			266 // implemented - increase chance to score an extra attack with a 2-Handed Weapon.
#define SE_PetDiscipline2				267 // *not implemented - /pet focus, /pet no cast
#define SE_ReduceTradeskillFail			268 // implemented - reduces chance to fail with given tradeskill by a percent chance
#define SE_MaxBindWound					269	// implemented[AA] - Increase max HP you can bind wound.
#define SE_BardSongRange				270	// implemented[AA] - increase range of beneficial bard songs (Sionachie's Crescendo)
#define SE_BaseMovementSpeed			271 // implemented[AA] - mods basemove speed, doesn't stack with other move mods
#define SE_CastingLevel2				272 // implemented
#define SE_CriticalDoTChance			273	// implemented
#define SE_CriticalHealChance			274	// implemented
#define SE_CriticalMend					275	// implemented[AA] - chance to critical monk mend
#define SE_Ambidexterity				276 // implemented[AA] - increase chance to duel weild by adding bonus 'skill'
#define SE_UnfailingDivinity			277	// implemented[AA] - ability grants your Death Pact-type spells a second chance to successfully heal their target, also can cause said spells to do a portion of their healing value even on a complete failure.
#define	SE_FinishingBlow				278 // implemented[AA] - chance to do massive damage under 10% HP (base1 = chance, base2 = damage)
#define SE_Flurry						279	// implemented
#define SE_PetFlurry					280 // implemented[AA]
#define SE_FeignedMinion				281	// *not implemented[AA] ability allows you to instruct your pet to feign death via the '/pet feign' command. value = succeed chance
#define SE_ImprovedBindWound			282	// implemented[AA] - increase bind wound amount by percent.
#define SE_DoubleSpecialAttack			283	// implemented[AA] - Chance to perform second special attack as monk
//#define SE_LoHSetHeal					284	// not used
#define SE_NimbleEvasion				285	// *not implemented - base1 = 100 for max
#define SE_FcDamageAmt					286	// implemented - adds direct spell damage
#define SE_SpellDurationIncByTic		287 // implemented
#define SE_SkillAttackProc				288	// implemented[AA] - Chance to proc spell on skill attack usage (ex. Dragon Punch)
#define SE_CastOnFadeEffect				289 // implemented - Triggers only if fades after natural duration.
#define SE_IncreaseRunSpeedCap			290	// implemented[AA] - increases run speed over the hard cap
#define SE_Purify						291 // implemented - Removes determental effects
#define SE_StrikeThrough2				292	// implemented[AA] - increasing chance of bypassing an opponent's special defenses, such as dodge, block, parry, and riposte.
#define SE_FrontalStunResist			293	// implemented[AA] - Reduce chance to be stunned from front. -- live descriptions sounds like this isn't limited to frontal anymore
#define SE_CriticalSpellChance			294 // implemented - increase chance to critical hit and critical damage modifier.
//#define SE_ReduceTimerSpecial			295	// not used
#define SE_FcSpellVulnerability			296	// implemented - increase in incoming spell damage
#define SE_FcDamageAmtIncoming			297 // implemented - debuff that adds points damage to spells cast on target (focus effect).
#define SE_ChangeHeight					298	// implemented
#define SE_WakeTheDead					299	// implemented
#define SE_Doppelganger					300	// implemented
#define SE_ArcheryDamageModifier		301	// implemented[AA] - increase archery damage by percent
#define SE_FcDamagePctCrit				302	// implemented - spell focus that is applied after critical hits has been calculated.
#define SE_FcDamageAmtCrit				303	// implemented - adds direct spell damage
#define SE_OffhandRiposteFail			304 // implemented as bonus - enemy cannot riposte offhand attacks
#define SE_MitigateDamageShield			305 // implemented - off hand attacks only (Shielding Resistance)
//#define SE_ArmyOfTheDead				306 // *not implemented NecroAA - This ability calls up to five shades of nearby corpses back to life to serve the necromancer. The soulless abominations will mindlessly fight the target until called back to the afterlife some time later. The first rank summons up to three shades that serve for 60 seconds, and each additional rank adds one more possible shade and increases their duration by 15 seconds
//#define SE_Appraisal					307 // *not implemented Rogue AA - This ability allows you to estimate the selling price of an item you are holding on your cursor.
#define SE_SuspendMinion				308 // implemented
#define SE_GateCastersBindpoint			309 // implemented - Gate to casters bind point
#define SE_ReduceReuseTimer				310 // implemented
#define SE_LimitCombatSkills			311 // implemented - Excludes focus from procs (except if proc is a memorizable spell)
#define SE_Sanctuary					312 // implemented - Places caster at bottom hate list, effect fades if cast cast spell on targets other than self.
#define SE_ForageAdditionalItems		313	// implemented[AA] - chance to forage additional items
#define SE_Invisibility2				314 // implemented - fixed duration invisible
#define SE_InvisVsUndead2				315 // implemented - fixed duration ITU
//#define SE_ImprovedInvisAnimals		316	// not used
#define SE_ItemHPRegenCapIncrease		317	// implemented[AA] - increases amount of health regen gained via items
#define SE_ItemManaRegenCapIncrease		318 // implemented - increases amount of mana regen you can gain via items
#define SE_CriticalHealOverTime			319 // implemented
#define SE_ShieldBlock					320	// implemented - Block attacks with shield
#define SE_ReduceHate					321 // implemented
#define SE_GateToHomeCity				322 // implemented
#define SE_DefensiveProc				323 // implemented
#define SE_HPToMana						324 // implemented
#define SE_NoBreakAESneak				325	// implemented[AA] - [AA Nerves of Steel] increasing chance to remain hidden when they are an indirect target of an AoE spell.
#define SE_SpellSlotIncrease			326 // *not implemented as bonus - increases your spell slot availability
#define SE_MysticalAttune				327 // implemented - increases amount of buffs that a player can have
#define SE_DelayDeath					328 // implemented - increases how far you can fall below 0 hp before you die
#define SE_ManaAbsorbPercentDamage		329 // implemented
#define SE_CriticalDamageMob			330	// implemented
#define SE_Salvage						331 // implemented - chance to recover items that would be destroyed in failed tradeskill combine
#define SE_SummonToCorpse				332 // *not implemented AA - Call of the Wild (Druid/Shaman Res spell with no exp)
#define SE_CastOnRuneFadeEffect			333 // implemented
#define SE_BardAEDot					334	// implemented
#define SE_BlockNextSpellFocus			335	// implemented - base1 chance to block next spell ie Puratus (8494)
//#define SE_IllusionaryTarget			336	// not used
#define SE_PercentXPIncrease			337	// implemented
#define SE_SummonAndResAllCorpses		338	// implemented
#define SE_TriggerOnCast				339	// implemented
#define SE_SpellTrigger					340	// implemented - chance to trigger spell
#define SE_ItemAttackCapIncrease		341	// implemented[AA] - increases the maximum amount of attack you can gain from items.
#define SE_ImmuneFleeing				342	// implemented - stop mob from fleeing
#define SE_InterruptCasting				343	// implemented - % chance to interrupt spells being cast every tic. Cacophony (8272)
#define SE_ChannelChanceItems			344	// implemented[AA] - chance to not have ITEM effects interrupted when you take damage.
#define SE_AssassinateLevel				345	// implemented as bonus - AA Assisination max level to kill
#define SE_HeadShotLevel				346	// implemented[AA] - HeadShot max level to kill
#define SE_DoubleRangedAttack			347	// implemented - chance at an additional archery attack (consumes arrow)
#define SE_LimitManaMin					348	// implemented
#define SE_ShieldEquipDmgMod			349	// implemented[AA] Increase melee base damage (indirectly increasing hate) when wearing a shield.
#define SE_ManaBurn						350	// implemented - Drains mana for damage/heal at a defined ratio up to a defined maximum amount of mana.
//#define SE_PersistentEffect			351	// *not implemented. creates a trap/totem that casts a spell (spell id + base1?) when anything comes near it. can probably make a beacon for this
//#define SE_IncreaseTrapCount			352	// *not implemented - looks to be some type of invulnerability? Test ITC (8755)
//#define SE_AdditionalAura				353	// *not implemented - allows use of more than 1 aura, aa effect
//#define SE_DeactivateAllTraps			354	// *not implemented - looks to be some type of invulnerability? Test DAT (8757)
//#define SE_LearnTrap					355	// *not implemented - looks to be some type of invulnerability? Test LT (8758)
//#define SE_ChangeTriggerType			356	// not used
#define SE_FcMute						357	// implemented - silences casting of spells that contain specific spell effects (focus limited)
#define SE_CurrentManaOnce				358	// implemented
//#define SE_PassiveSenseTrap			359	// *not implemented - Invulnerability (Brell's Blessing)
#define SE_ProcOnKillShot				360	// implemented - a buff that has a base1 % to cast spell base2 when you kill a "challenging foe" base3 min level
#define SE_SpellOnDeath					361	// implemented - casts spell on death of buffed
#define SE_PotionBeltSlots				362	// *not implemented[AA] 'Quick Draw' expands the potion belt by one additional available item slot per rank.
#define SE_BandolierSlots				363	// *not implemented[AA] 'Battle Ready' expands the bandolier by one additional save slot per rank.
#define SE_TripleAttackChance			364	// implemented
#define SE_ProcOnSpellKillShot			365	// implemented - chance to trigger a spell on kill when the kill is caused by a specific spell with this effect in it (10470 Venin)
#define SE_GroupShielding				366	// *not implemented[AA] This gives you /shieldgroup
#define SE_SetBodyType					367	// implemented - set body type of base1 so it can be affected by spells that are limited to that type (Plant, Animal, Undead, etc)
//#define SE_FactionMod					368	// *not implemented - increases faction with base1 (faction id, live won't match up w/ ours) by base2
#define SE_CorruptionCounter			369	// implemented
#define SE_ResistCorruption				370	// implemented
#define SE_AttackSpeed4					371 // implemented - stackable slow effect 'Inhibit Melee'
#define SE_ForageSkill					372	// *not implemented[AA] Will increase the skill cap for those that have the Forage skill and grant the skill and raise the cap to those that do not.
#define SE_CastOnFadeEffectAlways		373 // implemented - Triggers if fades after natural duration OR from rune/numhits fades.
#define SE_ApplyEffect					374 // implemented
#define SE_DotCritDmgIncrease			375	// implemented - Increase damage of DoT critical amount
//#define SE_Fling						376	// *not implemented - used in 2 test spells  (12945 | Movement Test Spell 1)
#define SE_CastOnFadeEffectNPC			377	// implemented - Triggers only if fades after natural duration (On live these are usually players spells that effect an NPC).
#define SE_SpellEffectResistChance		378	// implemented - Increase chance to resist specific spell effect (base1=value, base2=spell effect id)
#define SE_ShadowStepDirectional		379 // implemented - handled by client
#define SE_Knockdown					380 // implemented - small knock back(handled by client)
//#define SE_KnockTowardCaster			381	// *not implemented (Call of Hither) knocks you back to caster (value) distance units infront
#define SE_NegateSpellEffect			382 // implemented - negates specific spell bonuses for duration of the debuff.
#define SE_SympatheticProc				383 // implemented - focus on items that has chance to proc a spell when you cast
#define SE_Leap							384	// implemented - Leap effect, ie stomping leap
#define SE_LimitSpellGroup				385	// implemented - Limits to spell group(ie type 3 reuse reduction augs that are class specific and thus all share s SG)
#define SE_CastOnCurer					386 // implemented - Casts a spell on the person curing
#define SE_CastOnCure					387 // implemented - Casts a spell on the cured person
//#define SE_SummonCorpseZone			388 // *not implemented - summons a corpse from any zone(nec AA)
#define SE_FcTimerRefresh				389 // implemented - Refresh spell icons
//#define SE_FcTimerLockout				390 // *not implemented - Sets recast timers to specific value, focus limited.
#define SE_LimitManaMax					391	// implemented
#define SE_FcHealAmt					392 // implemented - Adds or removes healing from spells
#define SE_FcHealPctIncoming			393 // implemented - HealRate with focus restrictions.
#define SE_FcHealAmtIncoming			394 // implemented - Adds/Removes amount of healing on target by X value with foucs restrictions.
#define SE_FcHealPctCritIncoming		395 // implemented[AA] - Increases chance of having a heal crit when cast on you. [focus limited]
#define SE_FcHealAmtCrit				396 // implemented - Adds a direct healing amount to spells
#define SE_PetMeleeMitigation			397 // implemented[AA] - additional mitigation to your pets. Adds AC.
#define SE_SwarmPetDuration				398 // implemented - Affects the duration of swarm pets
#define SE_FcTwincast					399 // implemented - cast 2 spells for every 1
#define SE_HealGroupFromMana			400 // implemented - Drains mana and heals for each point of mana drained
#define SE_ManaDrainWithDmg				401 // implemented - Deals damage based on the amount of mana drained
#define SE_EndDrainWithDmg				402 // implemented - Deals damage for the amount of endurance drained
#define SE_LimitSpellClass				403 // implemented - Limits to specific types of spells (see CheckSpellCategory)
#define SE_LimitSpellSubclass			404 // *not implemented - Limits to specific types of spells (see CheckSpellCategory) [Categories NOT defined yet]
#define SE_TwoHandBluntBlock			405 // implemented - chance to block attacks when using two hand blunt weapons (similiar to shield block)
#define SE_CastonNumHitFade				406 // implemented - casts a spell when a buff fades due to its numhits being depleted
#define SE_CastonFocusEffect			407 // implemented - casts a spell if focus limits are met (ie triggers when a focus effects is applied)
#define SE_LimitHPPercent				408 // implemented - limited to a certain percent of your hp(ie heals up to 50%)
#define SE_LimitManaPercent				409 // implemented - limited to a certain percent of your mana
#define SE_LimitEndPercent				410 // implemented - limited to a certain percent of your end
#define SE_LimitClass					411 // implemented - Limits to spells of a certain class (Note: The class value in dbase is +1 in relation to item class value)
#define SE_LimitRace					412 // implemented - Limits to spells cast by a certain race (Note: not used in any known live spells)
#define SE_FcBaseEffects				413 // implemented - Increases the power of bard songs, skill attacks, runes, bard allowed foci, damage/heal
#define SE_LimitCastingSkill			414 // implemented - Limit a focus to include spells cast using a specific skill.
//#define SE_FFItemClass				415 // not used
#define SE_ACv2							416 // implemented - New AC spell effect
#define SE_ManaRegen_v2					417 // implemented - New mana regen effect
#define SE_SkillDamageAmount2			418 // implemented - adds skill damage directly to certain attacks
#define SE_AddMeleeProc					419 // implemented - Adds a proc
#define SE_FcLimitUse					420 // implemented - increases numhits count by percent (Note: not used in any known live spells)
#define SE_FcIncreaseNumHits			421 // implemented[AA] - increases number of hits a buff has till fade. (focus)
#define SE_LimitUseMin					422 // implemented - limit a focus to require a min amount of numhits value (used with above)
#define SE_LimitUseType					423 // implemented	- limit a focus to require a certain numhits type
#define SE_GravityEffect				424 // implemented - Pulls/pushes you toward/away the mob at a set pace
//#define SE_Display					425 // *not implemented - Illusion: Flying Dragon(21626)
#define SE_IncreaseExtTargetWindow		426 // *not implmented[AA] - increases the capacity of your extended target window
#define SE_SkillProc					427 // implemented - chance to proc when using a skill(ie taunt)
#define SE_LimitToSkill					428 // implemented - limits what skills will effect a skill proc
#define SE_SkillProcSuccess				429 // implemented - chance to proc when tje skill in use successfully fires.
//#define SE_PostEffect					430 // *not implemented - Fear of the Dark(27641) - Alters vision
//#define SE_PostEffectData				431 // *not implemented - Fear of the Dark(27641) - Alters vision
//#define SE_ExpandMaxActiveTrophyBen	432 // not used
#define SE_CriticalDotDecay				433 // implemented - increase critical dot chance, effect decays based on level of spell it effects. (12266)
#define SE_CriticalHealDecay			434 // implemented - increase critical heal chance, effect decays based on level of spell it effects.
#define SE_CriticalRegenDecay			435 // implemented - increase critical heal over time chance, effect decays based on level of spell it effects.
//#define SE_BeneficialCountDownHold	436 // not used ( 23491 | ABTest Buff Hold)
//#define SE_TeleporttoAnchor			437 // *not implemented - Teleport Guild Hall Anchor(33099)
//#define SE_TranslocatetoAnchor		438 // *not implemented - Translocate Primary Anchor (27750)
#define SE_Assassinate					439 // implemented[AA] - Assassinate damage
#define SE_FinishingBlowLvl				440 // implemented[AA] - Sets the level Finishing blow can be triggered on an NPC
#define SE_DistanceRemoval				441 // implemented - Buff is removed from target when target moves X amount of distance away from where initially hit.
#define SE_TriggerOnReqTarget			442 // implemented - triggers a spell which a certain criteria are met (below X amount of hp,mana,end, number of pets on hatelist)
#define SE_TriggerOnReqCaster			443 // implemented - triggers a spell which a certain criteria are met (below X amount of hp,mana,end, number of pets on hatelist)
#define SE_ImprovedTaunt				444 // implemented - Locks Aggro On Caster and Decrease other Players Aggro by X% on NPC targets below level Y
//#define SE_AddMercSlot				445 // *not implemented[AA] - [Hero's Barracks] Allows you to conscript additional mercs.
#define SE_AStacker						446 // implementet - bufff stacking blocker (26219 | Qirik's Watch)
#define SE_BStacker						447 // implemented
#define SE_CStacker						448 // implemented
#define SE_DStacker						449 // implemented
#define SE_MitigateDotDamage			450 // implemented  DOT spell mitigation rune with max value
#define SE_MeleeThresholdGuard			451 // implemented  Partial Melee Rune that only is lowered if melee hits are over X amount of damage
#define SE_SpellThresholdGuard			452 // implemented  Partial Spell Rune that only is lowered if spell hits are over X amount of damage
#define SE_TriggerMeleeThreshold		453 // implemented  Trigger effect on X amount of melee damage taken in a single hit
#define SE_TriggerSpellThreshold		454 // implemented  Trigger effect on X amount of spell damage taken in a single hit
#define SE_AddHatePct					455 // implemented  Modify total hate by %
#define SE_AddHateOverTimePct			456 // implemented  Modify total hate by % over time.
#define SE_ResourceTap					457 // implemented  Coverts a percent of dmg from dmg spells(DD/DoT) to hp/mana/end.
#define SE_FactionModPct				458 // implemented  Modifies faction gains and losses by percent.
#define SE_DamageModifier2				459 // implemented - Modifies melee damage by skill type
//#define SE_Ff_Override_NotFocusable	460 // 
#define SE_ImprovedDamage2				461 // implemented - Increase spell damage by percent (SE_Fc_Damage_%2)
#define SE_FcDamageAmt2					462 // implemented - Increase spell damage by flat amount (SE_Fc_Damage_Amt2)	
//#define SE_Shield_Target				463 // 
#define SE_PC_Pet_Rampage				464 // implemented - Base1 % chance to do rampage for base2 % of damage each melee round
//#define SE_PC_Pet_AE_Rampage			465 // Would assume as above but need to confirm.
#define SE_PC_Pet_Flurry_Chance			466 // implemented - Base1 % chance to do flurry from double attack hit.
//#define SE_DS_Mitigation_Amount		467 // 
//#define SE_DS_Mitigation_Percentage	468 // 
//#define SE_Chance_Best_in_Spell_Grp   469 //  
//#define SE_Trigger_Best_in_Spell Grp  470 // 
//#define SE_Double_Melee_Round			471 // 


// LAST


#define DF_Permanent			50
#define DF_Aura					51

// note this struct is historical, we don't actually need it to be
// aligned to anything, but for maintaining it it is kept in the order that
// the fields in the text file are. the numbering is not offset, but field
// number. note that the id field is counted as 0, this way the numbers
// here match the numbers given to sep in the loading function net.cpp
//
#define SPELL_LOAD_FIELD_COUNT 236

struct SPDat_Spell_Struct
{
/* 000 */	int id;	// not used -- SPELLINDEX
/* 001 */	char name[64]; // Name of the spell -- SPELLNAME
/* 002 */	char player_1[32]; // "PLAYER_1" -- ACTORTAG
/* 003 */	char teleport_zone[64];	// Teleport zone, pet name summoned, or item summoned -- NPC_FILENAME
/* 004 */	char you_cast[64]; // Message when you cast -- CASTERMETXT
/* 005 */	char other_casts[64]; // Message when other casts -- CASTEROTHERTXT
/* 006 */	char cast_on_you[64]; // Message when spell is cast on you -- CASTEDMETXT
/* 007 */	char cast_on_other[64]; // Message when spell is cast on someone else -- CASTEDOTHERTXT
/* 008 */	char spell_fades[64]; // Spell fades -- SPELLGONE
/* 009 */	float range; // -- RANGE
/* 010 */	float aoerange; // -- IMPACTRANGE
/* 011 */	float pushback; // -- OUTFORCE
/* 012 */	float pushup; // -- UPFORCE
/* 013 */	uint32 cast_time; // Cast time -- CASTINGTIME
/* 014 */	uint32 recovery_time; // Recovery time -- RECOVERYDELAY
/* 015 */	uint32 recast_time; // Recast same spell time -- SPELLDELAY
/* 016 */	uint32 buffdurationformula; // -- DURATIONBASE
/* 017 */	uint32 buffduration; // -- DURATIONCAP
/* 018 */	uint32 AEDuration;	// sentinel, rain of something -- IMPACTDURATION
/* 019 */	uint16 mana; // Mana Used -- MANACOST
/* 020 */	int base[EFFECT_COUNT];	//various purposes -- BASEAFFECT1 .. BASEAFFECT12
/* 032 */	int base2[EFFECT_COUNT]; //various purposes -- BASE_EFFECT2_1 ... BASE_EFFECT2_12
/* 044 */	int32 max[EFFECT_COUNT]; // -- AFFECT1CAP ... AFFECT12CAP
/* 056 */	//uint16 icon; // Spell icon -- IMAGENUMBER
/* 057 */	//uint16 memicon; // Icon on membarthing -- MEMIMAGENUMBER
/* 058 */	int32 components[4]; // reagents -- EXPENDREAGENT1 ... EXPENDREAGENT4
/* 062 */	int component_counts[4]; // amount of regents used -- EXPENDQTY1 ... EXPENDQTY4
/* 066 */	int NoexpendReagent[4];	// focus items (Need but not used; Flame Lick has a Fire Beetle Eye focus.)
											// If it is a number between 1-4 it means components[number] is a focus and not to expend it
											// If it is a valid itemid it means this item is a focus as well
											// -- NOEXPENDREAGENT1 ... NOEXPENDREAGENT4
/* 070 */	uint16 formula[EFFECT_COUNT]; // Spell's value formula -- LEVELAFFECT1MOD ... LEVELAFFECT12MOD
/* 082 */	//int LightType; // probaly another effecttype flag -- LIGHTTYPE
/* 083 */	int8 goodEffect; //0=detrimental, 1=Beneficial, 2=Beneficial, Group Only -- BENEFICIAL
/* 084 */	int Activated; // probably another effecttype flag -- ACTIVATED
/* 085 */	int resisttype; // -- RESISTTYPE
/* 086 */	int effectid[EFFECT_COUNT];	// Spell's effects -- SPELLAFFECT1 ... SPELLAFFECT12
/* 098 */	SpellTargetType targettype;	// Spell's Target -- TYPENUMBER
/* 099 */	int basediff; // base difficulty fizzle adjustment -- BASEDIFFICULTY
/* 100 */	EQEmu::skills::SkillType skill; // -- CASTINGSKILL
/* 101 */	int8 zonetype; // 01=Outdoors, 02=dungeons, ff=Any -- ZONETYPE
/* 102 */	int8 EnvironmentType; // -- ENVIRONMENTTYPE
/* 103 */	int8 TimeOfDay; // -- TIMEOFDAY
/* 104 */	uint8 classes[PLAYER_CLASS_COUNT]; // Classes, and their min levels -- WARRIORMIN ... BERSERKERMIN
/* 120 */	uint8 CastingAnim; // -- CASTINGANIM
/* 121 */	//uint8 TargetAnim; // -- TARGETANIM
/* 122 */	//uint32 TravelType; // -- TRAVELTYPE
/* 123 */	uint16 SpellAffectIndex; // -- SPELLAFFECTINDEX
/* 124 */	int8 disallow_sit; // 124: high-end Yaulp spells (V, VI, VII, VIII [Rk 1, 2, & 3], & Gallenite's Bark of Fury -- CANCELONSIT
/* 125 */	int8 diety_agnostic;// 125: Words of the Skeptic -- DIETY_AGNOSTIC
/* 126 */	int8 deities[16];	// Deity check. 201 - 216 per http://www.eqemulator.net/wiki/wikka.php?wakka=DeityList
										// -1: Restrict to Deity; 1: Restrict to Deity, but only used on non-Live (Test Server "Blessing of ...") spells; 0: Don't restrict
										// the client actually stores deities in a single int32_t
										// -- DIETY_BERTOXXULOUS ... DIETY_VEESHAN
/* 142 */	//int8 npc_no_cast;			// 142: between 0 & 100 -- NPC_NO_CAST
/* 143 */	//int ai_pt_bonus;			// 143: always set to 0, client doesn't save this -- AI_PT_BONUS
/* 144 */	//int16 new_icon	// Spell icon used by the client in uifiles/default/spells??.tga, both for spell gems & buff window. Looks to depreciate icon & memicon -- NEW_ICON
/* 145 */	//int16 spellanim; // Doesn't look like it's the same as #doanim, so not sure what this is, particles I think -- SPELL_EFFECT_INDEX
/* 146 */	bool uninterruptable;	// Looks like anything != 0 is uninterruptable. Values are mostly -1, 0, & 1 (Fetid Breath = 90?) -- NO_INTERRUPT
/* 147 */	int16 ResistDiff; // -- RESIST_MOD
/* 148 */	bool dot_stacking_exempt; // -- NOT_STACKABLE_DOT
/* 149 */	//int deletable; // -- DELETE_OK
/* 150 */	uint16 RecourseLink; // -- REFLECT_SPELLINDEX
/* 151 */	bool no_partial_resist;	// 151: -1, 0, or 1 -- NO_PARTIAL_SAVE
/* 152 */	//bool small_targets_only; // -- SMALL_TARGETS_ONLY
/* 153 */	//bool uses_persistent_particles; // -- USES_PERSISTENT_PARTICLES
/* 154 */	int8 short_buff_box;	// != 0, goes to short buff box. -- BARD_BUFF_BOX
/* 155 */	int descnum; // eqstr of description of spell -- DESCRIPTION_INDEX
/* 156 */	int typedescnum; // eqstr of type description -- PRIMARY_CATEGORY
/* 157 */	int effectdescnum; // eqstr of effect description -- SECONDARY_CATEGORY_1
/* 158 */	//int secondary_category_2;   //Category Desc ID 3 -- SECONDARY_CATEGORY_2
/* 159 */	bool npc_no_los; // -- NO_NPC_LOS
/* 160 */	//bool feedbackable; // -- FEEDBACKABLE
/* 161 */	bool reflectable; // -- REFLECTABLE
/* 162 */	int bonushate; // -- HATE_MOD
/* 163 */	//int resist_per_level; // -- RESIST_PER_LEVEL
/* 164 */	//int resist_cap; // for most spells this appears to mimic ResistDiff -- RESIST_CAP
/* 165 */	bool ldon_trap; //Flag found on all LDON trap / chest related spells. -- AFFECT_INANIMATE
/* 166 */	int EndurCost; // -- STAMINA_COST
/* 167 */	int8 EndurTimerIndex; // bad name, used for all spells -- TIMER_INDEX
/* 168 */	bool IsDisciplineBuff; //Will goto the combat window when cast -- IS_SKILL
/* 169 - 172*/ //These are zero for ALL spells, also removed from live -- ATTACK_OPENING, DEFENSE_OPENING, SKILL_OPENING, NPC_ERROR_OPENING
/* 173 */	int HateAdded; // -- SPELL_HATE_GIVEN
/* 174 */	int EndurUpkeep; // -- ENDUR_UPKEEP
/* 175 */	int numhitstype; // defines which type of behavior will tick down the numhit counter. -- LIMITED_USE_TYPE
/* 176 */	int numhits; // -- LIMITED_USE_COUNT
/* 177 */	int pvpresistbase; // -- PVP_RESIST_MOD
/* 178 */	int pvpresistcalc; // -- PVP_RESIST_PER_LEVEL
/* 179 */	int pvpresistcap; // -- PVP_RESIST_CAP
/* 180 */	int spell_category; // -- GLOBAL_GROUP
/* 181 */	//int pvp_duration; // buffdurationformula for PvP -- PVP_DURATION
/* 182 */	//int pvp_duration_cap; // buffduration for PvP -- PVP_DURATION_CAP
/* 183 */	int pcnpc_only_flag; // valid values are 0, 1 = PCs (and mercs), and 2 = NPCs (and not mercs) -- PCNPC_ONLY_FLAG
/* 184 */	bool cast_not_standing; // this is checked in the client's EQ_Spell::IsCastWhileInvisSpell, this also blocks SE_InterruptCasting from affecting this spell -- CAST_NOT_STANDING
/* 185 */	bool can_mgb; // 0=no, -1 or 1 = yes -- CAN_MGB
/* 186 */	int dispel_flag; // -- NO_DISPELL
/* 187 */	//int npc_category; // -- NPC_MEM_CATEGORY
/* 188 */	//int npc_usefulness; // -- NPC_USEFULNESS
/* 189 */	int MinResist; // -- MIN_RESIST
/* 190 */	int MaxResist; // -- MAX_RESIST
/* 191 */	uint8 viral_targets; // -- MIN_SPREAD_TIME
/* 192 */	uint8 viral_timer; // -- MAX_SPREAD_TIME
/* 193 */	int NimbusEffect; // -- DURATION_PARTICLE_EFFECT
/* 194 */	float directional_start; //Cone Start Angle: -- CONE_START_ANGLE
/* 195 */	float directional_end; // Cone End Angle: -- CONE_END_ANGLE
/* 196 */   bool sneak; // effect can only be used if sneaking (rogue 'Daggerfall' ect) -- SNEAK_ATTACK
/* 197 */	bool not_focusable; //prevents focus effects from being applied to spell -- NOT_FOCUSABLE
/* 198 */   bool no_detrimental_spell_aggro; // -- NO_DETRIMENTAL_SPELL_AGGRO
/* 199 */	//bool show_wear_off_message; // -- SHOW_WEAR_OFF_MESSAGE
/* 200 */	bool suspendable; // buff is suspended in suspended buff zones -- IS_COUNTDOWN_HELD
/* 201 */	int viral_range; // -- SPREAD_RADIUS
/* 202 */	int songcap; // individual song cap -- BASE_EFFECTS_FOCUS_CAP
/* 203 */	//bool stacks_with_self; // -- STACKS_WITH_SELF
/* 204 */	//int not_shown_to_player; // client skips this -- NOT_SHOWN_TO_PLAYER
/* 205 */	bool no_block; // -- NO_BUFF_BLOCK
/* 206 */	//int8 anim_variation; // -- ANIM_VARIATION
/* 207 */	int spellgroup; // -- SPELL_GROUP
/* 208 */	int rank; //increments AA effects with same name -- SPELL_GROUP_RANK
/* 209 */	int no_resist; //makes spells unresistable, which makes charms unbreakable as well. -- NO_RESIST
/* 210 */	// bool allow_spellscribe; // -- ALLOW_SPELLSCRIBE
/* 211 */	int CastRestriction; //Various restriction categories for spells most seem targetable race related but have also seen others for instance only castable if target hp 20% or lower or only if target out of combat -- SPELL_REQ_ASSOCIATION_ID
/* 212 */	bool AllowRest; // -- BYPASS_REGEN_CHECK
/* 213 */	bool InCombat; //Allow spell if target is in combat -- CAN_CAST_IN_COMBAT
/* 214 */   bool OutofCombat; //Allow spell if target is out of combat -- CAN_CAST_OUT_OF_COMBAT
/* 215 */	//bool show_dot_message; // -- SHOW_DOT_MESSAGE
/* 216 */	//bool invalid; // -- INVALID
/* 217 */   int override_crit_chance; //Places a cap on the max chance to critical -- OVERRIDE_CRIT_CHANCE
/* 218 */	int aemaxtargets;  //Is used for various AE effects -- MAX_TARGETS
/* 219 */	int no_heal_damage_item_mod; // -- NO_HEAL_DAMAGE_ITEM_MOD
/* 220 */	//int caster_requirement_id; // -- CASTER_REQUIREMENT_ID
/* 221 */	//int spell_class; // -- SPELL_CLASS
/* 222 */	//int spell_subclass; // -- SPELL_SUBCLASS
/* 223 */	//int ai_valid_targets; // -- AI_VALID_TARGETS
/* 224 */	bool persistdeath; // buff doesn't get stripped on death -- NO_STRIP_ON_DEATH
/* 225 */	//float base_effects_focus_slope; // -- BASE_EFFECTS_FOCUS_SLOPE
/* 226 */	//float base_effects_focus_offset; // -- BASE_EFFECTS_FOCUS_OFFSET
/* 227 */	float min_dist; //spell power modified by distance from caster (Min Distance) -- DISTANCE_MOD_CLOSE_DIST
/* 228 */	float min_dist_mod;  //spell power modified by distance from caster (Modifier at Min Distance) -- DISTANCE_MOD_CLOSE_MULT
/* 229 */	float max_dist; //spell power modified by distance from caster (Max Distance) -- DISTANCE_MOD_FAR_DIST
/* 230 */   float max_dist_mod; //spell power modified by distance from caster (Modifier at Max Distance) -- DISTANCE_MOD_FAR_MULT
/* The client also does this
 *  v26 = *(float *)&v4->DistanceModFarDist - *(float *)&v4->DistanceModCloseDist;
 *  if ( v26 > -0.00000011920929 && v26 < 0.00000011920929 )
 *    v26 = 1.0;
 *  v27 = (st7_0 - *(float *)&v4->DistanceModCloseMult) / v26;
 *  *(float *)&v4->DistanceMod = v27;
 */
/* 231 */   float min_range; //Min casting range -- MIN_RANGE
/* 232 */   bool no_remove; //prevents buff from being removed by click -- NO_REMOVE
/* 233 */	//int spell_recourse_type; // -- SPELL_RECOURSE_TYPE
/* 234 */	//bool only_during_fast_regen; // -- ONLY_DURING_FAST_REGEN
/* 235 */	//bool is_beta_only; // -- IS_BETA_ONLY
/* 236 */	//int spell_subgroup; // -- SPELL_SUBGROUP
			uint8 DamageShieldType; // This field does not exist in spells_us.txt
};

extern const SPDat_Spell_Struct* spells;
extern int32 SPDAT_RECORDS;

bool IsTargetableAESpell(uint16 spell_id);
bool IsSacrificeSpell(uint16 spell_id);
bool IsLifetapSpell(uint16 spell_id);
bool IsMezSpell(uint16 spell_id);
bool IsStunSpell(uint16 spell_id);
bool IsSlowSpell(uint16 spell_id);
bool IsHasteSpell(uint16 spell_id);
bool IsHarmonySpell(uint16 spell_id);
bool IsPercentalHealSpell(uint16 spell_id);
bool IsGroupOnlySpell(uint16 spell_id);
bool IsBeneficialSpell(uint16 spell_id);
bool IsDetrimentalSpell(uint16 spell_id);
bool IsInvulnerabilitySpell(uint16 spell_id);
bool IsCHDurationSpell(uint16 spell_id);
bool IsPoisonCounterSpell(uint16 spell_id);
bool IsDiseaseCounterSpell(uint16 spell_id);
bool IsSummonItemSpell(uint16 spell_id);
bool IsSummonSkeletonSpell(uint16 spell_id);
bool IsSummonPetSpell(uint16 spell_id);
bool IsSummonPCSpell(uint16 spell_id);
bool IsCharmSpell(uint16 spell_id);
bool IsBlindSpell(uint16 spell_id);
bool IsEffectHitpointsSpell(uint16 spell_id);
bool IsReduceCastTimeSpell(uint16 spell_id);
bool IsIncreaseDurationSpell(uint16 spell_id);
bool IsReduceManaSpell(uint16 spell_id);
bool IsExtRangeSpell(uint16 spell_id);
bool IsImprovedHealingSpell(uint16 spell_id);
bool IsImprovedDamageSpell(uint16 spell_id);
bool IsAEDurationSpell(uint16 spell_id);
bool IsPureNukeSpell(uint16 spell_id);
bool IsAENukeSpell(uint16 spell_id);
bool IsPBAENukeSpell(uint16 spell_id);
bool IsAERainNukeSpell(uint16 spell_id);
bool IsPartialCapableSpell(uint16 spell_id);
bool IsResistableSpell(uint16 spell_id);
bool IsGroupSpell(uint16 spell_id);
bool IsTGBCompatibleSpell(uint16 spell_id);
bool IsBardSong(uint16 spell_id);
bool IsEffectInSpell(uint16 spellid, int effect);
bool IsBlankSpellEffect(uint16 spellid, int effect_index);
bool IsValidSpell(uint32 spellid);
bool IsSummonSpell(uint16 spellid);
bool IsEvacSpell(uint16 spellid);
bool IsDamageSpell(uint16 spellid);
bool IsFearSpell(uint16 spellid);
bool IsCureSpell(uint16 spellid);
bool BeneficialSpell(uint16 spell_id);
bool GroupOnlySpell(uint16 spell_id);
int GetSpellEffectIndex(uint16 spell_id, int effect);
int CanUseSpell(uint16 spellid, int classa, int level);
int GetMinLevel(uint16 spell_id);
int GetSpellLevel(uint16 spell_id, int classa);
int CalcBuffDuration_formula(int level, int formula, int duration);
int32 CalculatePoisonCounters(uint16 spell_id);
int32 CalculateDiseaseCounters(uint16 spell_id);
int32 CalculateCurseCounters(uint16 spell_id);
int32 CalculateCorruptionCounters(uint16 spell_id);
int32 CalculateCounters(uint16 spell_id);
bool IsDisciplineBuff(uint16 spell_id);
bool IsDiscipline(uint16 spell_id);
bool IsCombatSkill(uint16 spell_id);
bool IsResurrectionEffects(uint16 spell_id);
bool IsRuneSpell(uint16 spell_id);
bool IsMagicRuneSpell(uint16 spell_id);
bool IsManaTapSpell(uint16 spell_id);
bool IsAllianceSpellLine(uint16 spell_id);
bool IsDeathSaveSpell(uint16 spell_id);
bool IsFullDeathSaveSpell(uint16 spell_id);
bool IsPartialDeathSaveSpell(uint16 spell_id);
bool IsShadowStepSpell(uint16 spell_id);
bool IsSuccorSpell(uint16 spell_id);
bool IsTeleportSpell(uint16 spell_id);
bool IsGateSpell(uint16 spell_id);
bool IsPlayerIllusionSpell(uint16 spell_id); // seveian 2008-09-23
bool IsLDoNObjectSpell(uint16 spell_id);
int32 GetSpellResistType(uint16 spell_id);
int32 GetSpellTargetType(uint16 spell_id);
bool IsHealOverTimeSpell(uint16 spell_id);
bool IsCompleteHealSpell(uint16 spell_id);
bool IsFastHealSpell(uint16 spell_id);
bool IsVeryFastHealSpell(uint16 spell_id);
bool IsRegularSingleTargetHealSpell(uint16 spell_id);
bool IsRegularGroupHealSpell(uint16 spell_id);
bool IsGroupCompleteHealSpell(uint16 spell_id);
bool IsGroupHealOverTimeSpell(uint16 spell_id);
bool IsDebuffSpell(uint16 spell_id);
bool IsResistDebuffSpell(uint16 spell_id);
bool IsSelfConversionSpell(uint16 spell_id);
bool IsBuffSpell(uint16 spell_id);
bool IsPersistDeathSpell(uint16 spell_id);
bool IsSuspendableSpell(uint16 spell_id);
uint32 GetMorphTrigger(uint32 spell_id);
bool IsCastonFadeDurationSpell(uint16 spell_id);
bool IsPowerDistModSpell(uint16 spell_id);
uint32 GetPartialMeleeRuneReduction(uint32 spell_id);
uint32 GetPartialMagicRuneReduction(uint32 spell_id);
uint32 GetPartialMeleeRuneAmount(uint32 spell_id);
uint32 GetPartialMagicRuneAmount(uint32 spell_id);
bool NoDetrimentalSpellAggro(uint16 spell_id);
bool IsStackableDot(uint16 spell_id);
bool IsCastWhileInvis(uint16 spell_id);
bool IsEffectIgnoredInStacking(int spa);

int CalcPetHp(int levelb, int classb, int STA = 75);
const char *GetRandPetName();
int GetSpellEffectDescNum(uint16 spell_id);
DmgShieldType GetDamageShieldType(uint16 spell_id, int32 DSType = 0);
bool DetrimentalSpellAllowsRest(uint16 spell_id);
uint32 GetNimbusEffect(uint16 spell_id);
int32 GetFuriousBash(uint16 spell_id);
bool IsShortDurationBuff(uint16 spell_id);
bool IsSpellUsableThisZoneType(uint16 spell_id, uint8 zone_type);
const char *GetSpellName(int16 spell_id);

#endif
