/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemu.org)

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

#ifndef RACES_H
#define RACES_H
#include "../common/types.h"

#define MALE 0
#define FEMALE 1
#define NEUTER 2

//theres a big list straight from the client below.

#define HUMAN 1
#define BARBARIAN 2
#define ERUDITE 3
#define WOOD_ELF 4
#define HIGH_ELF 5
#define DARK_ELF 6
#define HALF_ELF 7
#define DWARF 8
#define TROLL 9
#define OGRE 10
#define HALFLING 11
#define GNOME 12
#define WEREWOLF 14
#define WOLF 42
#define BEAR 43
#define SKELETON 60
#define TIGER 63
#define ELEMENTAL 75
#define ALLIGATOR 91
#define EYE_OF_ZOMM 108
#define WOLF_ELEMENTAL 120
#define INVISIBLE_MAN 127
#define IKSAR 128
#define VAHSHIR 130
#define CONTROLLED_BOAT 141
#define MINOR_ILL_OBJ 142
#define TREE 143
#define IKSAR_SKELETON 161
#define FROGLOK 330
// TODO: check all clients for (BYTE) usage of '/who all' class and remove FROGLOK2, if possible (330 - 74 = 256 .. WORD->BYTE conversion loss...)
#define FROGLOK2 74	// Not sure why /who all reports race as 74 for frogloks
#define FAIRY 473
#define DRAKKIN 522 // 32768
#define EMU_RACE_NPC 131069 // was 65533
#define EMU_RACE_PET 131070 // was 65534
#define EMU_RACE_UNKNOWN 131071 // was 65535


// player race values
#define PLAYER_RACE_UNKNOWN 0
#define PLAYER_RACE_HUMAN 1
#define PLAYER_RACE_BARBARIAN 2
#define PLAYER_RACE_ERUDITE 3
#define PLAYER_RACE_WOOD_ELF 4
#define PLAYER_RACE_HIGH_ELF 5
#define PLAYER_RACE_DARK_ELF 6
#define PLAYER_RACE_HALF_ELF 7
#define PLAYER_RACE_DWARF 8
#define PLAYER_RACE_TROLL 9
#define PLAYER_RACE_OGRE 10
#define PLAYER_RACE_HALFLING 11
#define PLAYER_RACE_GNOME 12
#define PLAYER_RACE_IKSAR 13
#define PLAYER_RACE_VAHSHIR 14
#define PLAYER_RACE_FROGLOK 15
#define PLAYER_RACE_DRAKKIN 16

#define PLAYER_RACE_COUNT 16


#define PLAYER_RACE_EMU_NPC 17
#define PLAYER_RACE_EMU_PET 18
#define PLAYER_RACE_EMU_COUNT 19


// player race bits
#define PLAYER_RACE_UNKNOWN_BIT 0
#define PLAYER_RACE_HUMAN_BIT 1
#define PLAYER_RACE_BARBARIAN_BIT 2
#define PLAYER_RACE_ERUDITE_BIT 4
#define PLAYER_RACE_WOOD_ELF_BIT 8
#define PLAYER_RACE_HIGH_ELF_BIT 16
#define PLAYER_RACE_DARK_ELF_BIT 32
#define PLAYER_RACE_HALF_ELF_BIT 64
#define PLAYER_RACE_DWARF_BIT 128
#define PLAYER_RACE_TROLL_BIT 256
#define PLAYER_RACE_OGRE_BIT 512
#define PLAYER_RACE_HALFLING_BIT 1024
#define PLAYER_RACE_GNOME_BIT 2048
#define PLAYER_RACE_IKSAR_BIT 4096
#define PLAYER_RACE_VAHSHIR_BIT 8192
#define PLAYER_RACE_FROGLOK_BIT 16384
#define PLAYER_RACE_DRAKKIN_BIT 32768

#define PLAYER_RACE_ALL_MASK 65535


const char* GetRaceIDName(uint16 race_id);
const char* GetPlayerRaceName(uint32 player_race_value);

uint32 GetPlayerRaceValue(uint16 race_id);
uint32 GetPlayerRaceBit(uint16 race_id);

uint16 GetRaceIDFromPlayerRaceValue(uint32 player_race_value);
uint16 GetRaceIDFromPlayerRaceBit(uint32 player_race_bit);


// player race-/gender-based model feature validators
namespace PlayerAppearance
{
	bool IsValidBeard(uint16 race_id, uint8 gender_id, uint8 beard_value, bool use_luclin = true);
	bool IsValidBeardColor(uint16 race_id, uint8 gender_id, uint8 beard_color_value, bool use_luclin = true);
	bool IsValidDetail(uint16 race_id, uint8 gender_id, uint32 detail_value, bool use_luclin = true);
	bool IsValidEyeColor(uint16 race_id, uint8 gender_id, uint8 eye_color_value, bool use_luclin = true);
	bool IsValidFace(uint16 race_id, uint8 gender_id, uint8 face_value, bool use_luclin = true);
	bool IsValidHair(uint16 race_id, uint8 gender_id, uint8 hair_value, bool use_luclin = true);
	bool IsValidHairColor(uint16 race_id, uint8 gender_id, uint8 hair_color_value, bool use_luclin = true);
	bool IsValidHead(uint16 race_id, uint8 gender_id, uint8 head_value, bool use_luclin = true);
	bool IsValidHeritage(uint16 race_id, uint8 gender_id, uint32 heritage_value, bool use_luclin = true);
	bool IsValidTattoo(uint16 race_id, uint8 gender_id, uint32 tattoo_value, bool use_luclin = true);
	bool IsValidTexture(uint16 race_id, uint8 gender_id, uint8 texture_value, bool use_luclin = true);
	bool IsValidWoad(uint16 race_id, uint8 gender_id, uint8 woad_value, bool use_luclin = true);
}

/*

//pulled from the client by ksmith:
$races_table = array(
		1 => "Human",
		2 => "Barbarian",
		3 => "Erudite",
		4 => "Wood Elf",
		5 => "High Elf",
		6 => "Dark Elf",
		7 => "Half Elf",
		8 => "Dwarf",
		9 => "Troll",
		10 => "Ogre",
		11 => "Halfling",
		12 => "Gnome",
		13 => "Aviak",
		14 => "Were Wolf",
		15 => "Brownie",
		16 => "Centaur",
		17 => "Golem",
		18 => "Giant / Cyclops",
		19 => "Trakenon",
		20 => "Doppleganger",
		21 => "Evil Eye",
		22 => "Beetle",
		23 => "Kerra",
		24 => "Fish",
		25 => "Fairy",
		26 => "Old Froglok",
		27 => "Old Froglok Ghoul",
		28 => "Fungusman",
		29 => "Gargoyle",
		30 => "Gasbag",
		31 => "Gelatinous Cube",
		32 => "Ghost",
		33 => "Ghoul",
		34 => "Giant Bat",
		35 => "Giant Eel",
		36 => "Giant Rat",
		37 => "Giant Snake",
		38 => "Giant Spider",
		39 => "Gnoll",
		40 => "Goblin",
		41 => "Gorilla",
		42 => "Wolf",
		43 => "Bear",
		44 => "Freeport Guards",
		45 => "Demi Lich",
		46 => "Imp",
		47 => "Griffin",
		48 => "Kobold",
		49 => "Lava Dragon",
		50 => "Lion",
		51 => "Lizard Man",
		52 => "Mimic",
		53 => "Minotaur",
		54 => "Orc",
		55 => "Human Beggar",
		56 => "Pixie",
		57 => "Dracnid",
		58 => "Solusek Ro",
		59 => "Bloodgills",
		60 => "Skeleton",
		61 => "Shark",
		62 => "Tunare",
		63 => "Tiger",
		64 => "Treant",
		65 => "Vampire",
		66 => "Rallos Zek",
		67 => "Highpass Citizen",
		68 => "Tentacle",
		69 => "Will 'O Wisp",
		70 => "Zombie",
		71 => "Qeynos Citizen",
		72 => "Ship",
		73 => "Launch",
		74 => "Piranha",
		75 => "Elemental",
		76 => "Puma",
		77 => "Neriak Citizen",
		78 => "Erudite Citizen",
		79 => "Bixie",
		80 => "Reanimated Hand",
		81 => "Rivervale Citizen",
		82 => "Scarecrow",
		83 => "Skunk",
		84 => "Snake Elemental",
		85 => "Spectre",
		86 => "Sphinx",
		87 => "Armadillo",
		88 => "Clockwork Gnome",
		89 => "Drake",
		90 => "Halas Citizen",
		91 => "Alligator",
		92 => "Grobb Citizen",
		93 => "Oggok Citizen",
		94 => "Kaladim Citizen",
		95 => "Cazic Thule",
		96 => "Cockatrice",
		97 => "Daisy Man",
		98 => "Elf Vampire",
		99 => "Denizen",
		100 => "Dervish",
		101 => "Efreeti",
		102 => "Old Froglok Tadpole",
		103 => "Kedge",
		104 => "Leech",
		105 => "Swordfish",
		106 => "Felguard",
		107 => "Mammoth",
		108 => "Eye of Zomm",
		109 => "Wasp",
		110 => "Mermaid",
		111 => "Harpie",
		112 => "Fayguard",
		113 => "Drixie",
		114 => "Ghost Ship",
		115 => "Clam",
		116 => "Sea Horse",
		117 => "Ghost Dwarf",
		118 => "Erudite Ghost",
		119 => "Sabertooth Cat",
		120 => "Wolf Elemental",
		121 => "Gorgon",
		122 => "Dragon Skeleton",
		123 => "Innoruuk",
		124 => "Unicorn",
		125 => "Pegasus",
		126 => "Djinn",
		127 => "Invisible Man",
		128 => "Iksar",
		129 => "Scorpion",
		130 => "Vah Shir",
		131 => "Sarnak",
		132 => "Draglock",
		133 => "Lycanthrope",
		134 => "Mosquito",
		135 => "Rhino",
		136 => "Xalgoz",
		137 => "Kunark Goblin",
		138 => "Yeti",
		139 => "Iksar Citizen",
		140 => "Forest Giant",
		141 => "Boat",
		142 => "UNKNOWN RACE",
		143 => "UNKNOWN RACE",
		144 => "Burynai",
		145 => "Goo",
		146 => "Spectral Sarnak",
		147 => "Spectral Iksar",
		148 => "Kunark Fish",
		149 => "Iksar Scorpion",
		150 => "Erollisi",
		151 => "Tribunal",
		152 => "Bertoxxulous",
		153 => "Bristlebane",
		154 => "Fay Drake",
		155 => "Sarnak Skeleton",
		156 => "Ratman",
		157 => "Wyvern",
		158 => "Wurm",
		159 => "Devourer",
		160 => "Iksar Golem",
		161 => "Iksar Skeleton",
		162 => "Man Eating Plant",
		163 => "Raptor",
		164 => "Sarnak Golem",
		165 => "Water Dragon",
		166 => "Iksar Hand",
		167 => "Succulent",
		168 => "Flying Monkey",
		169 => "Brontotherium",
		170 => "Snow Dervish",
		171 => "Dire Wolf",
		172 => "Manticore",
		173 => "Totem",
		174 => "Cold Spectre",
		175 => "Enchanted Armor",
		176 => "Snow Bunny",
		177 => "Walrus",
		178 => "Rock-gem Men",
		179 => "UNKNOWN RACE",
		180 => "UNKNOWN RACE",
		181 => "Yak Man",
		182 => "Faun",
		183 => "Coldain",
		184 => "Velious Dragons",
		185 => "Hag",
		186 => "Hippogriff",
		187 => "Siren",
		188 => "Frost Giant",
		189 => "Storm Giant",
		190 => "Ottermen",
		191 => "Walrus Man",
		192 => "Clockwork Dragon",
		193 => "Abhorent",
		194 => "Sea Turtle",
		195 => "Black and White Dragons",
		196 => "Ghost Dragon",
		197 => "Ronnie Test",
		198 => "Prismatic Dragon",
		199 => "ShikNar",
		200 => "Rockhopper",
		201 => "Underbulk",
		202 => "Grimling",
		203 => "Vacuum Worm",
		204 => "Evan Test",
		205 => "Kahli Shah",
		206 => "Owlbear",
		207 => "Rhino Beetle",
		208 => "Vampyre",
		209 => "Earth Elemental",
		210 => "Air Elemental",
		211 => "Water Elemental",
		212 => "Fire Elemental",
		213 => "Wetfang Minnow",
		214 => "Thought Horror",
		215 => "Tegi",
		216 => "Horse",
		217 => "Shissar",
		218 => "Fungal Fiend",
		219 => "Vampire Volatalis",
		220 => "StoneGrabber",
		221 => "Scarlet Cheetah",
		222 => "Zelniak",
		223 => "Lightcrawler",
		224 => "Shade",
		225 => "Sunflower",
		226 => "Sun Revenant",
		227 => "Shrieker",
		228 => "Galorian",
		229 => "Netherbian",
		230 => "Akheva",
		231 => "Spire Spirit",
		232 => "Sonic Wolf",
		233 => "Ground Shaker",
		234 => "Vah Shir Skeleton",
		235 => "Mutant Humanoid",
		236 => "Seru",
		237 => "Recuso",
		238 => "Vah Shir King",
		239 => "Vah Shir Guard",
		240 => "Teleport Man",
		241 => "Lujein",
		242 => "Naiad",
		243 => "Nymph",
		244 => "Ent",
		245 => "Fly Man",
		246 => "Tarew Marr",
		247 => "Sol Ro",
		248 => "Clockwork Golem",
		249 => "Clockwork Brain",
		250 => "Spectral Banshee",
		251 => "Guard of Justice",
		252 => "UNKNOWN RACE",
		253 => "Disease Boss",
		254 => "Sol Ro Guard",
		255 => "New Bertox",
		256 => "New Tribunal",
		257 => "Terris Thule",
		258 => "Vegerog",
		259 => "Crocodile",
		260 => "Bat",
		261 => "Slarghilug",
		262 => "Tranquilion",
		263 => "Tin Soldier",
		264 => "Nightmare Wraith",
		265 => "Malarian",
		266 => "Knight of Pestilence",
		267 => "Lepertoloth",
		268 => "Bubonian Boss",
		269 => "Bubonian Underling",
		270 => "Pusling",
		271 => "Water Mephit",
		272 => "Stormrider",
		273 => "Junk Beast",
		274 => "Broken Clockwork",
		275 => "Giant Clockwork",
		276 => "Clockwork Beetle",
		277 => "Nightmare Goblin",
		278 => "Karana",
		279 => "Blood Raven",
		280 => "Nightmare Gargoyle",
		281 => "Mouths of Insanity",
		282 => "Skeletal Horse",
		283 => "Saryn",
		284 => "Fennin Ro",
		285 => "Tormentor",
		286 => "Necro Priest",
		287 => "Nightmare",
		288 => "New Rallos Zek",
		289 => "Vallon Zek",
		290 => "Tallon Zek",
		291 => "Air Mephit",
		292 => "Earth Mephit",
		293 => "Fire Mephit",
		294 => "Nightmare Mephit",
		295 => "Zebuxoruk",
		296 => "Mithaniel Marr",
		297 => "Undead Knight",
		298 => "The Rathe",
		299 => "Xegony",
		300 => "Fiend",
		301 => "Test Object",
		302 => "Crab",
		303 => "Phoenix",
		304 => "PoP Dragon",
		305 => "PoP Bear",
		306 => "Storm Taarid",
		307 => "Storm Satuur",
		308 => "Storm Kuraaln",
		309 => "Storm Volaas",
		310 => "Storm Mana",
		311 => "Storm Fire",
		312 => "Storm Celestial",
		313 => "War Wraith",
		314 => "Wrulon",
		315 => "Kraken",
		316 => "Poison Frog",
		317 => "Queztocoatal",
		318 => "Valorian",
		319 => "War Boar",
		320 => "PoP Efreeti",
		321 => "War Boar Unarmored",
		322 => "Black Knight",
		323 => "Animated Armor",
		324 => "Undead Footman",
		325 => "Rallos Zek Minion",
		326 => "Arachnid",
		327 => "Crystal Spider",
		328 => "Zeb Cage",
		329 => "BoT Portal",
		330 => "Froglok",
		331 => "Troll Buccaneer",
		332 => "Troll Freebooter",
		333 => "Troll Sea Rover",
		334 => "Spectre Pirate Boss",
		335 => "Pirate Boss",
		336 => "Pirate Dark Shaman",
		337 => "Pirate Officer",
		338 => "Gnome Pirate",
		339 => "Dark Elf Pirate",
		340 => "Ogre Pirate",
		341 => "Human Pirate",
		342 => "Erudite Pirate",
		343 => "Poison Dart Frog",
		344 => "Troll Zombie",
		345 => "Luggald Land",
		346 => "Luggald Armored",
		347 => "Luggald Robed",
		348 => "Froglok Mount",
		349 => "Froglok Skeleton",
		350 => "Undead Froglok",
		351 => "Chosen Warrior",
		352 => "Chosen Wizard",
		353 => "Veksar",
		354 => "Greater Veksar",
		355 => "Veksar Boss",
		356 => "Chokadai",
		357 => "Undead Chokadai",
		358 => "Undead Veksar",
		359 => "Vampire Lesser",
		360 => "Vampire Elite",
		361 => "Rujakian Orc",
		362 => "Bone Golem",
		363 => "Synarcana",
		364 => "Sand Elf",
		365 => "Vampire Master",
		366 => "Rujakian Orc Elite",
		367 => "Skeleton New",
		368 => "Mummy New",
		369 => "Goblin New",
		370 => "Insect",
		371 => "Froglok Ghost",
		372 => "Dervish New",
		373 => "Shadow Creatue",
		374 => "Golem New",
		375 => "Evil Eye New",
		376 => "Box",
		377 => "Barrel",
		378 => "Chest",
		379 => "Vase",
		380 => "Table",
		381 => "Weapons Rack",
		382 => "Coffin",
		383 => "Bones",
		384 => "Jokester",
		385 => "Talosian Nihil",
		386 => "Talosian Exile",
		387 => "Talosian Golem",
		388 => "Talosian Wolf",
		389 => "Talosian Amphibian",
		390 => "Talosian Mountain Beast",
		391 => "Talosian Trilobyte",
		392 => "Invader War Hound",
		393 => "Invader Elite Centaur",
		394 => "Invader Lamia",
		395 => "Invader Cyclops",
		396 => "Kyv",
		397 => "Invader Soldier",
		398 => "Invader Brute",
		399 => "Invader Force Commander",
		400 => "Invader Lieutenant Boss",
		401 => "Invader War Beast",
		402 => "Invader Soldier Elite",
		403 => "UNKNOWN RACE",
		404 => "Discord Ship",
*/

#endif

