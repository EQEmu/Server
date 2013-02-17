--
--
-- type should be one of:
-- 
-- 244 - YOU are rotting! / attacker beings to decay
-- 245 - YOU are chilled to the bone / attacker is chilled to the bone
-- 246 - YOU are freezing! / attacker is freezing
-- 247 - YOU are tormented! / attacker is tormented
-- 248 - YOU are burned! / attacker is burned
-- 249 - YOU are pierced by thorns / attacker is pierced by thorns
-- 
-- Table structure for table `damageshieldtypes`
-- 

DROP TABLE IF EXISTS `damageshieldtypes`;
CREATE TABLE IF NOT EXISTS `damageshieldtypes` (
  `spellid` int(10) unsigned NOT NULL,
  `type` tinyint(3) unsigned NOT NULL,
  PRIMARY KEY  (`spellid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
--
-- The types below are a guess and not guaranteed to be correct.
--
-- Shield of Brambles
REPLACE into damageshieldtypes values(129,249); -- THORNS
-- Feedback
REPLACE into damageshieldtypes values(191,248); -- BURNT
-- Shield of Thistles
REPLACE into damageshieldtypes values(256,249); -- THORNS
-- Shield of Barbs
REPLACE into damageshieldtypes values(273,249); -- THORNS
-- Shield of Fire
REPLACE into damageshieldtypes values(332,248); -- BURNT
-- Shield of Thorns
REPLACE into damageshieldtypes values(356,249); -- THORNS
-- Banshee Aura
REPLACE into damageshieldtypes values(364,247); -- TORMENTED
-- O'Keils Radiation
REPLACE into damageshieldtypes values(378,248); -- BURNT
-- Shield of Flame
REPLACE into damageshieldtypes values(411,248); -- BURNT
-- Shield of Lava
REPLACE into damageshieldtypes values(412,248); -- BURNT
-- Shield of Spikes
REPLACE into damageshieldtypes values(432,249); -- THORNS
-- Inferno Shield
REPLACE into damageshieldtypes values(479,248); -- BURNT
-- Thistlecoat
REPLACE into damageshieldtypes values(515,249); -- THORNS
-- Barbcoat
REPLACE into damageshieldtypes values(516,249); -- THORNS
-- Bramblecoat
REPLACE into damageshieldtypes values(517,249); -- THORNS
-- Spikecoat
REPLACE into damageshieldtypes values(518,249); -- THORNS
-- Thorncoat
REPLACE into damageshieldtypes values(519,249); -- THORNS
-- Illusion: Fire Elemental
REPLACE into damageshieldtypes values(598,248); -- BURNT
-- Barrier of Combustion
REPLACE into damageshieldtypes values(680,248); -- BURNT
-- Psalm of Warmth
REPLACE into damageshieldtypes values(712,248); -- BURNT
-- Psalm of Cooling
REPLACE into damageshieldtypes values(713,246); -- FREEZING
-- Psalm of Vitality
REPLACE into damageshieldtypes values(715,245); -- CHILLED
-- Psalm of Purity
REPLACE into damageshieldtypes values(716,245); -- CHILLED
-- Chromatic Chaos
REPLACE into damageshieldtypes values(771,247); -- TORMENTED
-- AuraofElementalMastery
REPLACE into damageshieldtypes values(847,248); -- BURNT
-- FireElementalAura
REPLACE into damageshieldtypes values(927,248); -- BURNT
-- Vampire Aura
REPLACE into damageshieldtypes values(930,245); -- CHILLED
-- BarbedBones
REPLACE into damageshieldtypes values(932,249); -- THORNS
-- FrostElementalAura
REPLACE into damageshieldtypes values(953,246); -- FREEZING
-- Fwexar's Rage
REPLACE into damageshieldtypes values(1069,247); -- TORMENTED
-- Spiked Adornment
REPLACE into damageshieldtypes values(1143,249); -- THORNS
-- Bristling Armament
REPLACE into damageshieldtypes values(1249,249); -- THORNS
-- Scorching Skin
REPLACE into damageshieldtypes values(1251,248); -- BURNT
-- Kilva's Skin of Flame
REPLACE into damageshieldtypes values(1331,248); -- BURNT
-- Frostreaver's Blessing
REPLACE into damageshieldtypes values(1350,247); -- TORMENTED
-- Discordant Energy
REPLACE into damageshieldtypes values(1357,248); -- BURNT
-- Fiery Might
REPLACE into damageshieldtypes values(1367,248); -- BURNT
-- O'Keils Flickering Flame
REPLACE into damageshieldtypes values(1419,248); -- BURNT
-- Call of Earth
REPLACE into damageshieldtypes values(1462,249); -- THORNS
-- Bladecoat
REPLACE into damageshieldtypes values(1558,249); -- THORNS
-- Shield of Blades
REPLACE into damageshieldtypes values(1560,249); -- THORNS
-- Legacy of Thorn
REPLACE into damageshieldtypes values(1561,249); -- THORNS
-- Cadeau of Flame
REPLACE into damageshieldtypes values(1667,248); -- BURNT
-- Boon of Immolation
REPLACE into damageshieldtypes values(1668,248); -- BURNT
-- Aegis of Ro
REPLACE into damageshieldtypes values(1669,248); -- BURNT
-- Legacy of Spike
REPLACE into damageshieldtypes values(1727,249); -- THORNS
-- McVaxius` Rousing Rondo
REPLACE into damageshieldtypes values(1760,248); -- BURNT
-- Shield of the Winter
REPLACE into damageshieldtypes values(1795,246); -- FREEZING
-- Acting Fire Shield
REPLACE into damageshieldtypes values(1910,248); -- BURNT
-- Acting Poison Shield
REPLACE into damageshieldtypes values(1911,245); -- CHILLED
-- Acting Cold Shield
REPLACE into damageshieldtypes values(1912,246); -- FREEZING
-- Acting Disease Shield
REPLACE into damageshieldtypes values(1913,244); -- CHILLED
-- Shield of the Eighth
REPLACE into damageshieldtypes values(1963,247); -- TORMENTED
-- Thorny Shield
REPLACE into damageshieldtypes values(1975,249); -- THORNS
-- Aegis of Bathezid
REPLACE into damageshieldtypes values(2018,248); -- BURNT
-- Blessing of the Vah Shir
REPLACE into damageshieldtypes values(2067,247); -- TORMENTED
-- Ancient: Legacy of Blades
REPLACE into damageshieldtypes values(2125,249); -- THORNS
-- Shield of Darkness
REPLACE into damageshieldtypes values(2166,247); -- TORMENTED
-- Icicle Aura
REPLACE into damageshieldtypes values(2197,246); -- FREEZING
-- Tempest Guard
REPLACE into damageshieldtypes values(2316,248); -- BURNT
-- O'Keils Embers
REPLACE into damageshieldtypes values(2551,248); -- BURNT
-- Cloak of the Akheva
REPLACE into damageshieldtypes values(2580,245); -- CHILLED
-- Riftwind's Protection
REPLACE into damageshieldtypes values(2593,249); -- THORNS
-- Force of Nature
REPLACE into damageshieldtypes values(2595,249); -- THORNS
-- Warder's Protection
REPLACE into damageshieldtypes values(2600,249); -- THORNS
-- Unity of Fire
REPLACE into damageshieldtypes values(2673,248); -- BURNT
-- Sonic Feedback
REPLACE into damageshieldtypes values(2773,247); -- TORMENTED
-- Fire Elemental Form I
REPLACE into damageshieldtypes values(2795,248); -- BURNT
-- Fire Elemental Form II
REPLACE into damageshieldtypes values(2796,248); -- BURNT
-- Fire Elemental Form III
REPLACE into damageshieldtypes values(2797,248); -- BURNT
-- Golem Coat
REPLACE into damageshieldtypes values(2831,249); -- THORNS
-- Aura of Vinitras
REPLACE into damageshieldtypes values(2926,245); -- CHILLED
-- Smoldering Bulwark
REPLACE into damageshieldtypes values(2976,248); -- BURNT
-- Corona of Sol
REPLACE into damageshieldtypes values(3006,248); -- BURNT
-- Protection of the Wild
REPLACE into damageshieldtypes values(3039,249); -- THORNS
-- Shield of Eternal Flame
REPLACE into damageshieldtypes values(3056,248); -- BURNT
-- Portal Flames
REPLACE into damageshieldtypes values(3065,248); -- BURNT
-- Flameshield of Ro
REPLACE into damageshieldtypes values(3198,248); -- BURNT
-- Wrath of the Wild
REPLACE into damageshieldtypes values(3255,249); -- THORNS
-- Wrath of the Wild
REPLACE into damageshieldtypes values(3256,249); -- THORNS
-- Wrath of the Wild
REPLACE into damageshieldtypes values(3257,249); -- THORNS
-- Spirit of the Wood
REPLACE into damageshieldtypes values(3277,249); -- THORNS
-- Spirit of the Wood
REPLACE into damageshieldtypes values(3278,249); -- THORNS
-- Spirit of the Wood
REPLACE into damageshieldtypes values(3279,249); -- THORNS
-- Legacy of Bracken
REPLACE into damageshieldtypes values(3295,249); -- THORNS
-- Psalm of Veeshan
REPLACE into damageshieldtypes values(3368,249); -- THORNS
-- Warsong of Zek
REPLACE into damageshieldtypes values(3374,247); -- TORMENTED
-- Call of the Rathe
REPLACE into damageshieldtypes values(3419,247); -- TORMENTED
-- Shield of Bracken
REPLACE into damageshieldtypes values(3448,249); -- THORNS
-- Brackencoat
REPLACE into damageshieldtypes values(3450,249); -- THORNS
-- Maelstrom of Ro
REPLACE into damageshieldtypes values(3486,248); -- BURNT
-- Cloak of Luclin
REPLACE into damageshieldtypes values(3490,245); -- CHILLED
-- O`Keils Levity
REPLACE into damageshieldtypes values(3581,248); -- BURNT
-- Shield of Pain
REPLACE into damageshieldtypes values(3608,247); -- TORMENTED
-- Shield of Torment
REPLACE into damageshieldtypes values(3609,247); -- TORMENTED
-- Pyrokinetic Aura
REPLACE into damageshieldtypes values(3741,248); -- BURNT
-- Aquatic Aura
REPLACE into damageshieldtypes values(3743,247); -- TORMENTED
-- Caustic Aura
REPLACE into damageshieldtypes values(3745,248); -- BURNT
-- Barrier of Hatred
REPLACE into damageshieldtypes values(3766,245); -- CHILLED
-- Thistle Ward
REPLACE into damageshieldtypes values(3858,249); -- THORNS
-- Bramble Ward
REPLACE into damageshieldtypes values(3859,249); -- THORNS
-- Spike Ward
REPLACE into damageshieldtypes values(3860,249); -- THORNS
-- Vengeful Guard
REPLACE into damageshieldtypes values(3871,245); -- CHILLED
-- Aura of the Defender
REPLACE into damageshieldtypes values(3879,248); -- BURNT
-- Plagued Torment
REPLACE into damageshieldtypes values(3916,247); -- TORMENTED
-- Acid Aura
REPLACE into damageshieldtypes values(4115,248); -- BURNT
-- Shield of Pain I
REPLACE into damageshieldtypes values(4219,247); -- TORMENTED
-- Shield of Pain II
REPLACE into damageshieldtypes values(4220,247); -- TORMENTED
-- Shield of Pain III
REPLACE into damageshieldtypes values(4221,247); -- TORMENTED
-- Shield of Pain IV
REPLACE into damageshieldtypes values(4222,247); -- TORMENTED
-- Shield of Pain V
REPLACE into damageshieldtypes values(4223,247); -- TORMENTED
-- Shield of Pain VI
REPLACE into damageshieldtypes values(4224,247); -- TORMENTED
-- Shield of Pain VII
REPLACE into damageshieldtypes values(4225,247); -- TORMENTED
-- Shield of Pain VIII
REPLACE into damageshieldtypes values(4226,247); -- TORMENTED
-- Shield of Pain IX
REPLACE into damageshieldtypes values(4227,247); -- TORMENTED
-- Shield of Pain X
REPLACE into damageshieldtypes values(4228,247); -- TORMENTED
-- Icicle Shield
REPLACE into damageshieldtypes values(4289,246); -- FREEZING
-- Allure of Hatred
REPLACE into damageshieldtypes values(4441,247); -- TORMENTED
-- Petrad's Protection
REPLACE into damageshieldtypes values(4473,248); -- BURNT
-- Fire Elemental Form IV
REPLACE into damageshieldtypes values(4560,248); -- BURNT
-- Fire Elemental Form V
REPLACE into damageshieldtypes values(4561,248); -- BURNT
-- Aura of the Hunter
REPLACE into damageshieldtypes values(4663,249); -- THORNS
-- Blood Shield
REPLACE into damageshieldtypes values(4703,248); -- BURNT
-- Gleaming Armor
REPLACE into damageshieldtypes values(4715,248); -- BURNT
-- Bloodhound's Revenge
REPLACE into damageshieldtypes values(4731,248); -- BURNT
-- Haste of the Tunat'Muram
REPLACE into damageshieldtypes values(4740,249); -- THORNS
-- Protection of Discord I
REPLACE into damageshieldtypes values(4744,247); -- TORMENTED
-- Protection of Discord II
REPLACE into damageshieldtypes values(4745,247); -- TORMENTED
-- Protection of Discord III
REPLACE into damageshieldtypes values(4746,247); -- TORMENTED
-- Malevolent Retribution
REPLACE into damageshieldtypes values(4848,245); -- CHILLED
-- War March of the Mastruq
REPLACE into damageshieldtypes values(4871,247); -- TORMENTED
-- Arrow of Intensity
REPLACE into damageshieldtypes values(4999,248); -- BURNT
-- Frost Shield
REPLACE into damageshieldtypes values(5098,246); -- FREEZING
-- Frost Guard
REPLACE into damageshieldtypes values(5099,246); -- FREEZING
-- Basilisk Aura
REPLACE into damageshieldtypes values(5171,248); -- BURNT
-- Shield of Briar
REPLACE into damageshieldtypes values(5302,249); -- THORNS
-- Guard of the Earth
REPLACE into damageshieldtypes values(5305,249); -- THORNS
-- Briarcoat
REPLACE into damageshieldtypes values(5307,249); -- THORNS
-- Ward of the Hunter
REPLACE into damageshieldtypes values(5317,249); -- THORNS
-- Cloak of Discord
REPLACE into damageshieldtypes values(5339,248); -- BURNT
-- Nettle Shield
REPLACE into damageshieldtypes values(5358,249); -- THORNS
-- Nettlecoat
REPLACE into damageshieldtypes values(5362,249); -- THORNS
-- Circle of Nettles
REPLACE into damageshieldtypes values(5365,249); -- THORNS
-- War March of Muram
REPLACE into damageshieldtypes values(5376,247); -- TORMENTED
-- Fireskin
REPLACE into damageshieldtypes values(5466,248); -- BURNT
-- Circle of Fireskin
REPLACE into damageshieldtypes values(5488,248); -- BURNT
-- Pyrilen Skin
REPLACE into damageshieldtypes values(5492,248); -- BURNT
-- Jagged Spikes
REPLACE into damageshieldtypes values(5698,249); -- THORNS
-- Flickering Fire
REPLACE into damageshieldtypes values(5705,248); -- BURNT
-- Splinters
REPLACE into damageshieldtypes values(5802,249); -- THORNS
-- Electric Shock
REPLACE into damageshieldtypes values(5810,248); -- BURNT
-- Static Shield
REPLACE into damageshieldtypes values(5811,248); -- BURNT
-- Flamegore's Fire
REPLACE into damageshieldtypes values(5817,248); -- BURNT
-- Spirit of the Grove
REPLACE into damageshieldtypes values(5881,249); -- THORNS
-- Spirit of the Grove
REPLACE into damageshieldtypes values(5882,249); -- THORNS
-- Spirit of the Grove
REPLACE into damageshieldtypes values(5883,249); -- THORNS
-- Pyrilen Ember
REPLACE into damageshieldtypes values(5997,248); -- BURNT
-- Gelidran Sleet
REPLACE into damageshieldtypes values(6006,246); -- FREEZING
-- Ancient: Veil of Pyrilonus
REPLACE into damageshieldtypes values(6145,248); -- BURNT
-- Hungry Vines Recourse
REPLACE into damageshieldtypes values(6154,249); -- THORNS
-- Discordant Spikes
REPLACE into damageshieldtypes values(6160,249); -- THORNS
-- Hateful Guard
REPLACE into damageshieldtypes values(6373,247); -- TORMENTED
-- Malleable Ice
REPLACE into damageshieldtypes values(6558,246); -- FREEZING
-- Mass Illusion: Fire Elemental
REPLACE into damageshieldtypes values(6581,248); -- BURNT
-- Vishimtar's Aura
REPLACE into damageshieldtypes values(6643,245); -- CHILLED
-- Embrace of Life
REPLACE into damageshieldtypes values(6650,247); -- TORMENTED
-- Storm Guard
REPLACE into damageshieldtypes values(6769,247); -- TORMENTED
-- Razor Bones
REPLACE into damageshieldtypes values(6833,249); -- THORNS
-- Ithiasor's Aura
REPLACE into damageshieldtypes values(6887,245); -- CHILLED
-- Clan Aura
REPLACE into damageshieldtypes values(7082,247); -- TORMENTED
-- Orcish Bulwark
REPLACE into damageshieldtypes values(7129,247); -- TORMENTED
-- Fire Skin I
REPLACE into damageshieldtypes values(7589,248); -- BURNT
-- Fire Skin II
REPLACE into damageshieldtypes values(7590,248); -- BURNT
-- Fire Skin III
REPLACE into damageshieldtypes values(7591,248); -- BURNT
-- Fire Skin IV
REPLACE into damageshieldtypes values(7592,248); -- BURNT
-- Fire Skin V
REPLACE into damageshieldtypes values(7593,248); -- BURNT
-- Fire Skin VI
REPLACE into damageshieldtypes values(7594,248); -- BURNT
-- Fire Skin VII
REPLACE into damageshieldtypes values(7595,248); -- BURNT
-- Fire Skin VIII
REPLACE into damageshieldtypes values(7596,248); -- BURNT
-- Fire Skin IX
REPLACE into damageshieldtypes values(7597,248); -- BURNT
-- Fire Skin X
REPLACE into damageshieldtypes values(7598,248); -- BURNT
-- Root Rage
REPLACE into damageshieldtypes values(7702,247); -- TORMENTED
-- Last Stand
REPLACE into damageshieldtypes values(7715,247); -- TORMENTED
-- BloodBarbs
REPLACE into damageshieldtypes values(7747,249); -- THORNS
-- Reaper's Stance
REPLACE into damageshieldtypes values(8189,247); -- TORMENTED
-- Convergence of Spirits
REPLACE into damageshieldtypes values(8190,249); -- THORNS
-- Convergence of Spirits
REPLACE into damageshieldtypes values(8191,249); -- THORNS
-- Convergence of Spirits
REPLACE into damageshieldtypes values(8192,249); -- THORNS
-- Spiritfury
REPLACE into damageshieldtypes values(8246,247); -- TORMENTED
