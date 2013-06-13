/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2005 EQEMu Development Team (http://eqemulator.net)

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
/* 12 */	// NOT USED
/* 13 */	ST_Tap = 0x0d,
/* 14 */	ST_Pet = 0x0e,
/* 15 */	ST_Corpse = 0x0f,
/* 16 */	ST_Plant = 0x10,
/* 17 */	ST_Giant = 0x11, //special giant
/* 18 */	ST_Dragon = 0x12, //special dragon
/* 19 */	// NOT USED
/* 20 */	ST_TargetAETap = 0x14,
/* 21 */	// NOT USED
/* 22 */	// NOT USED
/* 23 */	// NOT USED
/* 24 */	ST_UndeadAE = 0x18,
/* 25 */	ST_SummonedAE = 0x19,
/* 26 */	// NOT USED
/* 27 */	// NOT USED
/* 28 */	// NOT USED
/* 29 */	// NOT USED
/* 30 */	// NOT USED
/* 31 */	// NOT USED
/* 32 */	ST_AECaster2 = 0x20, //ae caster hatelist maybe?
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
/* 44 */	ST_Beam = 0x2c, //like directional but facing in front of you always
/* 45 */	//ST_Ring = 0x2d, // Like a mix of PB ae + rain spell(has ae duration)
/* 46 */	ST_TargetsTarget = 0x2e, // uses the target of your target
/* 47 */	//ST_PetMaster = 0x2e, // uses the master as target
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
#define SE_Invisibility					12	// implemented
#define SE_SeeInvis						13	// implemented
#define SE_WaterBreathing				14	// implemented
#define SE_CurrentMana					15	// implemented
//#define SE_Unknown16					16	// not used
//#define SE_Unknown17					17	// not used
#define SE_Lull							18	// implemented - Reaction Radius
#define SE_AddFaction					19	// implemented - Alliance line
#define SE_Blind						20	// implemented
#define SE_Stun							21	// implemented
#define SE_Charm						22	// implemented
#define SE_Fear							23	// implemented
#define SE_Stamina						24	// implemented - Invigor and such
#define SE_BindAffinity					25	// implemented
#define SE_Gate							26	// implemented - Gate to bind point
#define SE_CancelMagic					27	// implemented
#define SE_InvisVsUndead				28	// implemented
#define SE_InvisVsAnimals				29	// implemented
#define SE_ChangeFrenzyRad				30	// implemented - Pacify
#define SE_Mez							31	// implemented
#define SE_SummonItem					32	// implemented
#define SE_SummonPet					33	// implemented
//#define SE_Unknown34					34	// not used (Nimbus of Temporal Rifting) ?
#define SE_DiseaseCounter				35	// implemented
#define SE_PoisonCounter				36	// implemented
//#define SE_Unknown37					37	// not used
//#define SE_Unknown38					38	// not used
//#define SE_Unknown39					39	// not used
#define SE_DivineAura					40	// implemented
#define SE_Destroy						41	// implemented - Disintegrate, Banishment of Shadows
#define SE_ShadowStep					42	// implemented
//#define SE_Unknown43					43	// not used
#define SE_Lycanthropy					44	// implemented
//#define SE_Unknown45					45	// not used
#define SE_ResistFire					46	// implemented
#define SE_ResistCold					47	// implemented
#define SE_ResistPoison					48	// implemented
#define SE_ResistDisease				49	// implemented
#define SE_ResistMagic					50	// implemented
//#define SE_Unknown51					51	// not used
#define SE_SenseDead					52	// implemented
#define SE_SenseSummoned				53	// implemented
#define SE_SenseAnimals					54	// implemented
#define SE_Rune							55	// implemented
#define SE_TrueNorth					56	// implemented
#define SE_Levitate						57	// implemented
#define SE_Illusion						58	// implemented
#define SE_DamageShield					59	// implemented
//#define SE_Unknown60					60	// not used
#define SE_Identify						61	// implemented
//#define SE_Unknown62					62	// not used
#define SE_WipeHateList					63	// implemented
#define SE_SpinTarget					64	// implemented
#define SE_InfraVision					65	// implemented
#define SE_UltraVision					66	// implemented
#define SE_EyeOfZomm					67	// implemented
#define SE_ReclaimPet					68	// implemented
#define SE_TotalHP						69	// implemented
//#define SE_Unknown70					70	// not used
#define SE_NecPet						71	// implemented
//#define SE_Unknown72					72	// not used
#define SE_BindSight					73	// implemented
#define SE_FeignDeath					74	// implemented
#define SE_VoiceGraft					75	// implemented
#define SE_Sentinel						76	// *not implemented?(just seems to send a message)
#define SE_LocateCorpse					77	// implemented
#define SE_AbsorbMagicAtt				78	// implemented - Rune for spells
#define SE_CurrentHPOnce				79	// implemented - Heals and nukes, non-repeating if in a buff
//#define SE_Unknown80					80	// not used
#define SE_Revive						81	// implemented - Resurrect
#define SE_SummonPC						82	// implemented
#define SE_Teleport						83	// implemented
#define SE_TossUp						84	// implemented - Gravity Flux
#define SE_WeaponProc					85	// implemented - i.e. Call of Fire
#define SE_Harmony						86	// implemented
#define SE_MagnifyVision				87	// implemented - Telescope
#define SE_Succor						88	// implemented - Evacuate/Succor lines
#define SE_ModelSize					89	// implemented - Shrink, Growth
#define SE_Cloak						90	// *not implemented - Used in only 2 spells
#define SE_SummonCorpse					91	// implemented
#define SE_Calm							92	// implemented - Hate modifier stuff(poorly named)
#define SE_StopRain						93	// implemented - Wake of Karana
#define SE_NegateIfCombat				94	// *not implemented? - Works client side but there is comment todo in spell effects...Component of Spirit of Scale
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
//#define SE_Unknown107					107	// not used
#define SE_Familiar						108	// implemented
#define SE_SummonItemIntoBag			109	// implemented - summons stuff into container
//#define SE_Unknown110					110	// not used
#define SE_ResistAll					111	// implemented
#define SE_CastingLevel					112	// implemented
#define	SE_SummonHorse					113	// implemented
#define SE_ChangeAggro					114	// implemented - Hate modifing buffs(ie horrifying visage)
#define SE_Hunger						115	// implemented - Song of Sustenance
#define SE_CurseCounter					116	// implemented
#define SE_MagicWeapon					117	// implemented - makes weapon magical
#define SE_SingingSkill					118	// *implemented - needs AA conversion
#define SE_AttackSpeed3					119	// implemented
#define SE_HealRate						120	// implemented - reduces healing by a %
#define SE_ReverseDS					121 // implemented
//#define SE_Unknown122					122	// not used
#define SE_Screech						123	// implemented? Spell Blocker(can only have one buff with this effect at one time)
#define SE_ImprovedDamage				124 // implemented
#define SE_ImprovedHeal					125 // implemented
#define SE_SpellResistReduction			126 // implemented
#define SE_IncreaseSpellHaste			127 // implemented
#define SE_IncreaseSpellDuration		128 // implemented
#define SE_IncreaseRange				129 // implemented
#define SE_SpellHateMod					130 // implemented
#define SE_ReduceReagentCost			131 // implemented
#define SE_ReduceManaCost				132 // implemented
//#define SE_Unknown133					133	// not used
#define SE_LimitMaxLevel				134 // implemented
#define SE_LimitResist					135 // implemented
#define SE_LimitTarget					136 // implemented
#define SE_LimitEffect					137 // implemented
#define SE_LimitSpellType				138 // implemented
#define SE_LimitSpell					139 // implemented
#define SE_LimitMinDur					140 // implemented
#define SE_LimitInstant					141 // implemented
#define SE_LimitMinLevel				142 // implemented
#define SE_LimitCastTime				143 // implemented
//#define SE_Unknown144					144	// not used
#define SE_Teleport2					145	// implemented - Banishment of the Pantheon
//#define SE_Unknown146					146	// not used (Lightning Rod) Electrical Resist? (exp. VoA)
#define SE_PercentalHeal				147 // implemented
#define SE_StackingCommand_Block		148 // implemented?
#define SE_StackingCommand_Overwrite	149 // implemented?
#define SE_DeathSave					150 // implemented
#define SE_SuspendPet					151	// *not implemented as bonus
#define SE_TemporaryPets				152	// implemented
#define SE_BalanceHP					153 // implemented
#define SE_DispelDetrimental			154 // implemented
#define SE_SpellCritDmgIncrease			155 // implemented
#define SE_IllusionCopy					156	// implemented - Deception
#define SE_SpellDamageShield			157	// implemented - Petrad's Protection
#define SE_Reflect						158 // implemented
#define SE_AllStats						159	// implemented
#define SE_MakeDrunk					160 // implemented - poorly though, should check against tolerance
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
#define SE_Hate2						192	// implemented
#define SE_SkillAttack					193	// implemented
#define SE_FadingMemories				194	// implemented
#define SE_StunResist					195	// implemented
#define SE_Strikethrough				196	// implemented
#define SE_SkillDamageTaken				197	// implemented
#define SE_CurrentEnduranceOnce			198	// implemented
#define SE_Taunt						199	// implemented - % chance to taunt the target
#define SE_ProcChance					200	// implemented
#define SE_RangedProc					201	// implemented
#define SE_IllusionOther				202	// *not implemented as bonus(Project Illusion)
#define SE_MassGroupBuff				203	// *not implemented as bonus
#define SE_GroupFearImmunity			204	// *not implemented as bonus
#define SE_Rampage						205	// implemented
#define SE_AETaunt						206	// implemented
#define SE_FleshToBone					207	// implemented
//#define SE_Unknown208					208	// not used
#define SE_DispelBeneficial				209 // implemented
#define SE_PetShield					210	// *not implemented
#define SE_AEMelee						211	// implemented
#define SE_CastingSkills				212	// *not implemented -Include/Exclude Casting Skill type. (*no longer used on live)
#define SE_PetMaxHP						213	// implemented[AA] - increases the maximum hit points of your pet
#define SE_MaxHPChange					214	// implemented
#define SE_PetAvoidance					215	// implemented[AA] - increases pet ability to avoid melee damage
#define SE_Accuracy						216	// implemented
#define SE_HeadShot						217	// not implemented as bonus - ability to head shot (base2 = damage)
#define SE_PetCriticalHit				218 // implemented[AA] - gives pets a baseline critical hit chance
#define SE_SlayUndead					219	// implemented - Allow extra damage against undead (base1 = rate, base2 = damage mod).
#define SE_SkillDamageAmount			220	// implemented
#define SE_Packrat						221 // not implemented as bonus
#define SE_BlockBehind					222	// implemented - Chance to block from behind (with our without Shield)
//#define SE_Unknown223					223	// not used
#define	SE_GiveDoubleRiposte			224 // implemented[AA]
#define SE_GiveDoubleAttack				225	// implemented[AA] - Allow any class to double attack with set chance.
#define SE_TwoHandBash					226 // not implemented as bonus
#define SE_ReduceSkillTimer				227	// implemented
#define SE_ReduceFallDamage				228	// not implented as bonus - reduce the damage that you take from falling
#define SE_PersistantCasting			229 // implemented
#define SE_ExtendedShielding			230	// not used as bonus - increase range of /shield ability
//#define SE_Unknown231					231	// not used *Unknown limit used in AA Overpowering Strikes (Decrease chance stun resist)
#define SE_DivineSave					232	// implemented (base1 == % chance on death to insta-res) (base2 == spell cast on save)
#define SE_Metabolism					233	// *not implemented - (Crown of Feathers) Increase metabolism?
#define SE_ReduceApplyPoisonTime		234	// not implemented as bonus - reduces the time to apply poison
#define	SE_ChannelChanceSpells			235 // implemented[AA] - chance to channel from SPELLS *No longer used on live.
//#define SE_Unknown236					236	// not used
#define SE_GivePetGroupTarget			237 // implemented[AA] - (Pet Affinity)
#define SE_IllusionPersistence			238	// *not implemented - lends persistence to your illusionary disguises, causing them to last until you die or the illusion is forcibly removed.
#define SE_FeignedCastOnChance			239	// *not implemented as bonus - ability gives you an increasing chance for your feigned deaths to not be revealed by spells cast upon you.
//#define SE_Unknown240					240	// not used [Likely related to above - you become immune to feign breaking on a resisted spell and have a good chance of feigning through a spell that successfully lands upon you.]
#define SE_ImprovedReclaimEnergy		241	// not implemented as bonus - increase the amount of mana returned to you when reclaiming your pet.
#define SE_ChanceWipeHateList			242	// *not implemented - increases the chance to wipe hate with memory blurr
#define SE_CharmBreakChance				243	// implemented - Total Domination
#define	SE_RootBreakChance				244	// implemented[AA] reduce the chance that your root will break.
#define SE_TrapCircumvention			245	// *not implemented[AA] - decreases the chance that you will set off a trap when opening a chest
#define SE_SetBreathLevel				246 // not implemented as bonus
#define SE_RaiseSkillCap				247	// *not implemented[AA] - adds skill over the skill cap.
#define SE_SecondaryForte				248 // not implemented as bonus(gives you a 2nd specialize skill that can go past 50 to 100)
#define SE_SecondaryDmgInc				249 // implemented[AA] Allows off hand weapon to recieve a damage bonus (Sinister Strikes)
#define SE_SpellProcChance				250	// implemented - Increase chance to sympathetic proc by %
#define SE_ConsumeProjectile			251	// implemented[AA] - chance to not consume an arrow (ConsumeProjectile = 100)
#define SE_FrontalBackstabChance		252	// implemented[AA] - chance to perform a full damage backstab from front.
#define SE_FrontalBackstabMinDmg		253	// implemented[AA] - allow a frontal backstab for mininum damage.
#define SE_Blank						254 // implemented
#define SE_ShieldDuration				255	// not implemented as bonus - increases duration of /shield
#define SE_ShroudofStealth				256	// not implemented as bonus - rogue improved invs
#define SE_PetDiscipline				257 // not implemented as bonus - /pet hold
#define SE_TripleBackstab				258 // implemented[AA] - chance to perform a triple backstab
#define SE_CombatStability				259 // implemented[AA] - damage mitigation
#define SE_AddSingingMod				260 // *not implemented
//#define SE_Unknown261					261	// not used
#define SE_RaiseStatCap					262 // implemented
#define	SE_TradeSkillMastery			263	// implemented - lets you raise more than one tradeskill above master.
#define SE_HastenedAASkill				264 // not implemented as bonus - Use redux field in aa_actions table for this effect
#define SE_MasteryofPast				265 // implemented[AA] - Spells less than effect values level can not be fizzled
#define SE_ExtraAttackChance			266 // implemented - increase chance to score an extra attack with a 2-Handed Weapon.
#define SE_PetDiscipline2				267 // *not implemented - /pet focus, /pet no cast
#define SE_ReduceTradeskillFail			268 // *not implemented? - reduces chance to fail with given tradeskill by a percent chance
#define SE_MaxBindWound					269	// implemented[AA] - Increase max HP you can bind wound.
#define SE_BardSongRange				270	// implemented[AA] - increase range of beneficial bard songs (Sionachie's Crescendo)
#define SE_BaseMovementSpeed			271 // implemented[AA] - mods basemove speed, doesn't stack with other move mods
#define SE_CastingLevel2				272 // implemented
#define SE_CriticalDoTChance			273	// implemented
#define SE_CriticalHealChance			274	// implemented
//#define SE_Unknown275					275	// not used
#define SE_Ambidexterity				276 // implemented[AA] - increase chance to duel weild by adding bonus 'skill'
#define SE_UnfailingDivinity			277	// implemented[AA] - ability grants your Death Pact-type spells a second chance to successfully heal their target, also can cause said spells to do a portion of their healing value even on a complete failure.
#define	SE_FinishingBlow				278 // implemented[AA] - chance to do massive damage under 10% HP (base1 = chance, base2 = damage)
#define SE_Flurry						279	// implemented
#define SE_PetFlurry					280 // implemented[AA]
#define SE_FeignedMinion				281	// *not implemented[AA] ability allows you to instruct your pet to feign death via the '/pet feign' command. value = succeed chance
#define SE_ImprovedBindWound			282	// implemented[AA] - increase bind wound amount by percent.
#define SE_DoubleSpecialAttack			283	// implemented[AA] - Chance to perform second special attack as monk
//#define SE_Unknown284					284	// not used
#define SE_NimbleEvasion				285	// *not implemented - base1 = 100 for max
#define SE_SpellDamage					286	// implemented - adds direct spell damage
#define SE_SpellDurationIncByTic		287 // implemented
#define SE_SpecialAttackKBProc			288	// implemented[AA] - Chance to to do a knockback from special attacks [AA Dragon Punch].
#define SE_ImprovedSpellEffect			289 // implemented
#define SE_IncreaseRunSpeedCap			290	// implemented[AA] - increases run speed over the hard cap
#define SE_Purify						291 // implemented - Removes determental effects
#define SE_StrikeThrough2				292	// implemented[AA] - increasing chance of bypassing an opponent's special defenses, such as dodge, block, parry, and riposte.
#define SE_FrontalStunResist			293	// implemented[AA] - Reduce chance to be stunned from front.
#define SE_CriticalSpellChance			294 // implemented
//#define SE_Unknown295					295	// not used
#define SE_SpellVulnerability			296	// implemented - increase in incoming spell damage
#define SE_Empathy						297 // implemented - debuff that adds points damage to spells cast on target (focus effect).
#define SE_ChangeHeight					298	// implemented
#define SE_WakeTheDead					299	// implemented
#define SE_Doppelganger					300	// implemented
#define SE_ArcheryDamageModifier		301	// implemented[AA] - increase archery damage by percent
#define SE_ImprovedDamage2				302	// implemented - spell damage focus
#define SE_FF_Damage_Amount				303	// implemented - adds direct spell damage
#define SE_OffhandRiposteFail			304 // not implemented as bonus - enemy cannot riposte offhand attacks
#define SE_MitigateDamageShield			305 // implemented - off hand attacks only (Shielding Resistance)
#define SE_ArmyOfTheDead				306 // *not implemented NecroAA - This ability calls up to five shades of nearby corpses back to life to serve the necromancer. The soulless abominations will mindlessly fight the target until called back to the afterlife some time later. The first rank summons up to three shades that serve for 60 seconds, and each additional rank adds one more possible shade and increases their duration by 15 seconds
#define SE_Appraisal					307 // *not implemented Rogue AA - This ability allows you to estimate the selling price of an item you are holding on your cursor.
#define SE_SuspendMinion				308 // not implemented as bonus
#define SE_YetAnotherGate				309 // implemented
#define SE_ReduceReuseTimer				310 // implemented
#define SE_CombatSkills					311 // implemented
#define SE_Sanctuary					312 // *not implemented
#define SE_ForageAdditionalItems		313	// implemented[AA] - chance to forage additional items
#define SE_Invisibility2				314 // implemented - fixed duration invisible
#define SE_InvisVsUndead2				315 // implemented - fixed duration ITU
//#define SE_Unknown316					316	// not used
#define SE_ItemHPRegenCapIncrease		317	// implemented[AA] - increases amount of health regen gained via items
#define SE_ItemManaRegenCapIncrease		318 // implemented - increases amount of mana regen you can gain via items
#define SE_CriticalHealOverTime			319 // implemented
#define SE_ShieldBlock					320	// implemented - Block attacks with shield
#define SE_ReduceHate					321 // implemented
#define SE_GateToHomeCity				322 // implemented
#define SE_DefensiveProc				323 // implemented
#define SE_HPToMana						324 // implemented
#define SE_ChanceInvsBreakToAoE			325	// *not implemented[AA] - [AA Nerves of Steel] increasing chance to remain hidden when they are an indirect target of an AoE spell.
#define SE_SpellSlotIncrease			326 // not implemented as bonus - increases your spell slot availability
#define SE_MysticalAttune				327 // implemented - increases amount of buffs that a player can have
#define SE_DelayDeath					328 // implemented - increases how far you can fall below 0 hp before you die
#define SE_ManaAbsorbPercentDamage		329 // implemented
#define SE_CriticalDamageMob			330	// implemented
#define SE_Salvage						331 // *not implemented - chance to recover items that would be destroyed in failed tradeskill combine
#define SE_SummonToCorpse				332 // *not implemented AA - Call of the Wild (Druid/Shaman Res spell with no exp)
#define SE_EffectOnFade					333 // implemented
#define SE_BardAEDot					334	// implemented
#define SE_BlockNextSpellFocus			335	// implemented - base1 chance to block next spell ie Puratus (8494)
//#define SE_Unknown336					336	// not used
#define SE_PercentXPIncrease			337	// implemented
#define SE_SummonAndResAllCorpses		338	// implemented
#define SE_TriggerOnCast				339	// implemented
#define SE_SpellTrigger					340	// implemented - chance to trigger spell
#define SE_ItemAttackCapIncrease		341	// implemented[AA] - increases the maximum amount of attack you can gain from items.
#define SE_ImmuneFleeing				342	// implemented - stop mob from fleeing
#define SE_InterruptCasting				343	// implemented - % chance to interrupt spells being cast every tic. Cacophony (8272)
#define SE_ChannelChanceItems			344	// implemented[AA] - chance to not have ITEM effects interrupted when you take damage.
#define SE_AssassinationLevel			345	// not implemented as bonus - AA Assisination max level to kill
#define SE_HeadShotLevel				346	// not implemented as bonus - AA HeadShot max level to kill
#define SE_ExtraArcheryAttack			347	// not implemented - chance at an additional archery attack (consumes arrow)
#define SE_LimitManaCost				348	// implemented
#define SE_ShieldEquipHateMod			349	// *not implemented[AA] ie. used to increase melee hate when wearing a shield w/ Shield Specialist AA.
#define SE_ManaBurn						350	// implemented - Drains mana for damage/heal at a defined ratio up to a defined maximum amount of mana.
#define SE_PersistentEffect				351	// *not implemented. creates a trap/totem that casts a spell (spell id + base1?) when anything comes near it. can probably make a beacon for this
#define SE_Unknown352					352	// *not implemented - looks to be some type of invulnerability? Test ITC (8755)
#define SE_AdditionalAura				353	// *not implemented - allows use of more than 1 aura, aa effect
#define SE_Unknown354					354	// *not implemented - looks to be some type of invulnerability? Test DAT (8757)
#define SE_Unknown355					355	// *not implemented - looks to be some type of invulnerability? Test LT (8758)
//#define SE_Unknown356					356	// not used
//#define SE_Unknown357					357	// *not implemented - (Stunted Growth) Something to do with negate effects? Focus? Chance?
#define SE_CurrentManaOnce				358	// implemented
#define SE_Invulnerabilty				359	// *not implemented - Invulnerability (Brell's Blessing)
#define SE_SpellOnKill					360	// implemented - a buff that has a base1 % to cast spell base2 when you kill a "challenging foe" base3 min level
#define SE_SpellOnDeath					361	// implemented - casts spell on death of buffed
#define SE_PotionBeltSlots				362	// *not implemented[AA] 'Quick Draw' expands the potion belt by one additional available item slot per rank.
#define SE_BandolierSlots				363	// *not implemented[AA] 'Battle Ready' expands the bandolier by one additional save slot per rank.
#define SE_TripleAttackChance			364	// implemented
#define SE_SpellOnKill2					365	// implemented - chance to trigger a spell on kill when the kill is caused by a specific spell with this effect in it (10470 Venin)
#define SE_ShieldEquipDmgMod			366	// *not implemented - [AA Shield Specialist] - damage bonus to weapon if shield equiped.
#define SE_SetBodyType					367	// implemented - set body type of base1 so it can be affected by spells that are limited to that type (Plant, Animal, Undead, etc)
#define SE_FactionMod					368	// *not implemented - increases faction with base1 (faction id, live won't match up w/ ours) by base2
#define SE_CorruptionCounter			369	// implemented
#define SE_ResistCorruption				370	// implemented
#define SE_AttackSpeed4					371 // implemented - stackable slow effect 'Inhibit Melee'
#define SE_ForageSkill					372	// *not implemented[AA] Will increase the skill cap for those that have the Forage skill and grant the skill and raise the cap to those that do not.
#define SE_CastOnWearoff				373 // implemented
#define SE_ApplyEffect					374 // implemented
#define SE_DotCritDmgIncrease			375	// implemented - Increase damage of DoT critical amount
//#define SE_Unknown376					376	// *not implemented - used in 2 spells
#define SE_BossSpellTrigger				377	// implemented - spell is cast on fade
#define SE_SpellEffectResistChance		378	// implemented - Increase chance to resist specific spell effect (base1=value, base2=spell effect id)
#define SE_ShadowStepDirectional		379 // implemented - handled by client
#define SE_Knockdown					380 // implemented - small knock back(handled by client)
#define	SE_KnockTowardCaster			381	// *not implemented (Call of Hither) knocks you back to caster (value) distance units infront
#define SE_NegateSpellEffect			382 // implemented - negates specific spell bonuses for duration of the debuff.
#define SE_SympatheticProc				383 // implemented - focus on items that has chance to proc a spell when you cast
#define SE_Leap							384	// implemented - Leap effect, ie stomping leap
#define SE_LimitSpellGroup				385	// implemented - Limits to spell group(ie type 3 reuse reduction augs that are class specific and thus all share s SG)
#define SE_CastOnCurer					386 // implemented - Casts a spell on the person curing
#define SE_CastOnCure					387 // implemented - Casts a spell on the cured person
#define SE_SummonCorpseZone				388 // *not implemented - summons a corpse from any zone(nec AA)
#define SE_Forceful_Rejuv				389 // Refresh spell icons
#define SE_CastResistRestrict			390 // *not implemented - some sort of restriction of what resist spells you can cast
//#define SE_Unknown391					391 // not used (Warlord's Fury/Twinproc) likely a focus limit
#define SE_AdditionalHeal2				392 // implemented - Adds or removes healing from spells
#define SE_HealRate2					393 // implemented - HealRate with focus restrictions.
//#define SE_Unknown394					394 // *not implemented - (Diminishing Presence) Adds or removes healing from spells
#define SE_CriticalHealRate				395 // implemented[AA] - Increases chance of having a heal crit when cast on you. [focus limited]
#define SE_AdditionalHeal				396 // implemented - Adds a direct healing amount to spells
#define SE_PetMeleeMitigation			397 // *not implemented[AA] - additional mitigation to your pets.
#define SE_SwarmPetDuration				398 // implemented - Affects the duration of swarm pets
#define SE_Twincast						399 // implemented - cast 2 spells for every 1
#define SE_HealGroupFromMana			400 // implemented - Drains mana and heals for each point of mana drained
#define SE_ManaDrainWithDmg				401 // implemented - Deals damage based on the amount of mana drained
#define SE_EndDrainWithDmg				402 // implemented - Deals damage for the amount of endurance drained
#define SE_ReluctantBene				403 // *not implemented - Reluctant Benevolence(21662)
#define SE_LimitExcludeSkill			404 // implemented - Limit a focus to exclude spells cast using a specific skill.
#define SE_TwoHandBluntBlock			405 // implemented - chance to block attacks when using two hand blunt weapons (similiar to shield block)
#define SE_CastonNumHitFade				406 // implemented - casts a spell when a buff fades due to its numhits being depleted
//#define SE_Unknown397					407 // *not implemented (Diminished Presence) Triggerable spell effect
#define SE_LimitHPPercent				408 // implemented - limited to a certain percent of your hp(ie heals up to 50%)
#define SE_LimitManaPercent				409 // implemented - limited to a certain percent of your mana
#define SE_LimitEndPercent				410 // implemented - limited to a certain percent of your end
#define SE_LimitClass					411 // implemented - Limits to spells of a certain class (Note: The class value in dbase is +1 in relation to item class value)
//#define SE_Unknown412					412 // not used
#define SE_IncreaseSpellPower			413 // implemented - Increases the power of bard songs, skill attacks, runes, bard allowed foci, damage/heal
#define SE_LimitSpellSkill				414 // implemented - Limit a focus to include spells cast using a specific skill.
//#define SE_Unknown415					415 // not used
#define SE_ACv2							416 // implemented - New AC spell effect
#define SE_ManaRegen_v2					417 // implemented - New mana regen effect
#define SE_SkillDamageAmount2			418 // implemented - adds skill damage directly to certain attacks
#define SE_AddMeleeProc					419 // implemented - Adds a proc
//#define SE_Unknown420					420 // *not used
#define SE_IncreaseNumHits				421 // implemented[AA] - increases number of hits a buff has till fade. (focus)
//#define SE_Unknown422					422 // not used - Seen in Lasting Bravery likely a focus limit
//#define SE_Unknown423					423 // not used	- Seen in Lasting Bravery likely a focus limit
#define SE_GravityEffect				424 // implemented - Pulls/pushes you toward/away the mob at a set pace
#define SE_Display						425 // *not implemented - Illusion: Flying Dragon(21626)
#define SE_IncreaseExtTargetWindow		426 // *not implmented[AA] - increases the capacity of your extended target window
#define SE_SkillProc					427 // implemented - chance to proc when using a skill(ie taunt)
#define SE_LimitToSkill					428 // implemented - limits what skills will effect a skill proc
#define SE_SkillProc2					429 // implemented - chance to proc when using a skill (most have hit limits)
//#define SE_Unknown430					430 // *not implemented - Fear of the Dark(27641)
//#define SE_Unknown431					431 // *not implemented - Fear of the Dark(27641)
//#define SE_Unknown432					432 // not used
//#define SE_Uknonwn433					433 // not used
#define SE_CriticalHealChance2			434 // implemented - increase critical heal chance
#define SE_CriticalHealOverTime2		435 // implemented - increase critical heal over time chance
//#define SE_Unknown432					436 // not used
#define SE_Anchor						437 // *not implemented - Teleport Guild Hall Anchor(33099)
//#define SE_Unknown438					438 // not used
#define SE_IncreaseAssassinationLvl		439 // *not implemented[AA] - increases the maximum level of humanoid that can be affected by assassination
#define SE_FinishingBlowLvl				440 // implemented[AA] - Sets the level Finishing blow can be triggered on an NPC
#define SE_MovementSpeed2				441 // *not implemented - new snare effect
#define SE_TriggerOnHPAmount			442 // *not implemented - triggers a spell which a certain hp level is reached
//#define SE_Unknown443					443 // *not implemented - related to Finishing Blow AA
#define SE_AggroLock					444 // *not implemented - target will ignore all but caster for duration
#define SE_AdditionalMercenary			445 // *not implemented[AA] - [Hero's Barracks] Allows you to conscript additional mercs.
// LAST


#define DF_Permanent			50

// solar: note this struct is historical, we don't actually need it to be
// aligned to anything, but for maintaining it it is kept in the order that
// the fields in the text file are. the numbering is not offset, but field
// number. note that the id field is counted as 0, this way the numbers
// here match the numbers given to sep in the loading function net.cpp
//
struct SPDat_Spell_Struct
{
/* 000 */	//int id;	// not used
/* 001 */	char		name[64]; // Name of the spell
/* 002 */	char		player_1[32]; // "PLAYER_1"
/* 003 */	char		teleport_zone[64];	// Teleport zone, pet name summoned, or item summoned
/* 004 */	char		you_cast[64]; // Message when you cast
/* 005 */	char		other_casts[64]; // Message when other casts
/* 006 */	char		cast_on_you[64]; // Message when spell is cast on you
/* 007 */	char		cast_on_other[64]; // Message when spell is cast on someone else
/* 008 */	char		spell_fades[64]; // Spell fades
/* 009 */	float		range;
/* 010 */	float		aoerange;
/* 011 */	float		pushback;
/* 012 */	float		pushup;
/* 013 */	uint32		cast_time; // Cast time
/* 014 */	uint32		recovery_time; // Recovery time
/* 015 */	uint32		recast_time; // Recast same spell time
/* 016 */	uint32		buffdurationformula;
/* 017 */	uint32		buffduration;
/* 018 */	uint32		AEDuration;	// sentinel, rain of something
/* 019 */	uint16		mana; // Mana Used
/* 020 */	int			base[EFFECT_COUNT];	//various purposes
/* 032 */	int			base2[EFFECT_COUNT]; //various purposes
/* 044 */	int32		max[EFFECT_COUNT];
/* 056 */	//uint16 icon; // Spell icon
/* 057 */	//uint16 memicon; // Icon on membarthing
/* 058 */	int32		components[4]; // reagents
/* 062 */	int			component_counts[4]; // amount of regents used
/* 066 */	int			NoexpendReagent[4];	// focus items (Need but not used; Flame Lick has a Fire Beetle Eye focus.)
											// If it is a number between 1-4 it means components[number] is a focus and not to expend it
											// If it is a valid itemid it means this item is a focus as well
/* 070 */	uint16		formula[EFFECT_COUNT]; // Spell's value formula
/* 082 */	//int LightType; // probaly another effecttype flag
/* 083 */	int8 goodEffect; //0=detrimental, 1=Beneficial, 2=Beneficial, Group Only
/* 084 */	int			Activated; // probaly another effecttype flag
/* 085 */	int			resisttype;
/* 086 */	int			effectid[EFFECT_COUNT];	// Spell's effects
/* 098 */	SpellTargetType	targettype;	// Spell's Target
/* 099 */	int			basediff; // base difficulty fizzle adjustment
/* 100 */	SkillType	skill;
/* 101 */	int8 zonetype; // 01=Outdoors, 02=dungeons, ff=Any
/* 102 */	int8 EnvironmentType;
/* 103 */	int8 TimeOfDay;
/* 104 */	uint8		classes[PLAYER_CLASS_COUNT]; // Classes, and their min levels
/* 120 */	uint8		CastingAnim;
/* 121 */	//uint8 TargetAnim;
/* 122 */	//uint32 TravelType;
/* 123 */	uint16		SpellAffectIndex;
/* 124 */	int8 disallow_sit; // 124: high-end Yaulp spells (V, VI, VII, VIII [Rk 1, 2, & 3], & Gallenite's Bark of Fury
/* 125 */								// 125: Words of the Skeptic
/* 126 */	int8		deities[16];	// Deity check. 201 - 216 per http://www.eqemulator.net/wiki/wikka.php?wakka=DeityList
										// -1: Restrict to Deity; 1: Restrict to Deity, but only used on non-Live (Test Server "Blessing of ...") spells; 0: Don't restrict
/* 142 */						// 142: between 0 & 100
								// 143: always set to 0
/* 144 */	//int16 new_icon	// Spell icon used by the client in uifiles/default/spells??.tga, both for spell gems & buff window. Looks to depreciate icon & memicon
/* 145 */	//int16 spellanim; // Doesn't look like it's the same as #doanim, so not sure what this is
/* 146 */	int8		uninterruptable;	// Looks like anything != 0 is uninterruptable. Values are mostly -1, 0, & 1 (Fetid Breath = 90?)
/* 147 */	int16		ResistDiff;
/* 148 */	//int dot_stacking_exempt;
/* 149 */	//int deletable;
/* 150 */	uint16 RecourseLink;
/* 151 */						// 151: -1, 0, or 1
								// 152 & 153: all set to 0
/* 154 */	int8		short_buff_box;	// != 0, goes to short buff box.
/* 155 */	int			descnum; // eqstr of description of spell
/* 156 */	//int typedescnum; // eqstr of type description
/* 157 */	int			effectdescnum; // eqstr of effect description
/* 158 */
/* 162 */	int			bonushate;
/* 163 */
/* 166 */	int			EndurCost;
/* 167 */	int8 EndurTimerIndex;
/* 168 */	//int IsDisciplineBuff; //Will goto the combat window when cast
/* 169 */
/* 173 */	int			HateAdded;
/* 174 */	int			EndurUpkeep;
/* 175 */
/* 176 */	int			numhits;
/* 177 */	int			pvpresistbase;
/* 178 */	int			pvpresistcalc;
/* 179 */	int			pvpresistcap;
/* 180 */	int			spell_category;
/* 181 */
/* 185 */	int8 can_mgb; // 0=no, -1 or 1 = yes
/* 186 */	int			dispel_flag;
/* 189 */	int			MinResist;
/* 190 */	int			MaxResist;
/* 191 */	uint8 viral_targets;
/* 192 */	uint8 viral_timer;
/* 193 */	int			NimbusEffect;
/* 194 */	float		directional_start;
/* 195 */	float		directional_end;
/* 207 */	int			spellgroup;
/* 209 */	int			powerful_flag; // Need more investigation to figure out what to call this, for now we know -1 makes charm spells not break before their duration is complete, it does alot more though
/* 211 */	int			CastRestriction; //Various restriction categories for spells most seem targetable race related but have also seen others for instance only castable if target hp 20% or lower or only if target out of combat
/* 212 */	bool		AllowRest;
/* 219 */	//int maxtargets; // not in DB yet, is used for beam and ring spells for target # limits
			uint8		DamageShieldType; // This field does not exist in spells_us.txt
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
uint32 GetMorphTrigger(uint32 spell_id);
uint32 GetPartialMeleeRuneReduction(uint32 spell_id);
uint32 GetPartialMagicRuneReduction(uint32 spell_id);
uint32 GetPartialMeleeRuneAmount(uint32 spell_id);
uint32 GetPartialMagicRuneAmount(uint32 spell_id);

int CalcPetHp(int levelb, int classb, int STA = 75);
const char *GetRandPetName();
int GetSpellEffectDescNum(uint16 spell_id);
DmgShieldType GetDamageShieldType(uint16 spell_id);
bool DetrimentalSpellAllowsRest(uint16 spell_id);
uint32 GetNimbusEffect(uint16 spell_id);
int32 GetFuriousBash(uint16 spell_id);
bool IsShortDurationBuff(uint16 spell_id);
const char *GetSpellName(int16 spell_id);

#endif
