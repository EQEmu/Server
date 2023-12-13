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

#include "../common/races.h"
#include "data_verification.h"

const char *GetRaceIDName(uint16 race_id)
{
	switch (race_id) {
		case Race::Human:
		case Race::HighpassCitizen:
		case Race::QeynosCitizen:
		case Race::Human2:
			return "Human";
		case Race::Barbarian:
		case Race::HalasCitizen:
			return "Barbarian";
		case Race::Erudite:
		case Race::EruditeCitizen:
		case Race::Erudite2:
			return "Erudite";
		case Race::WoodElf:
			return "Wood Elf";
		case Race::HighElf:
			return "High Elf";
		case Race::DarkElf:
		case Race::NeriakCitizen:
			return "Dark Elf";
		case Race::HalfElf:
			return "Half Elf";
		case Race::Dwarf:
		case Race::KaladimCitizen:
			return "Dwarf";
		case Race::Troll:
		case Race::GrobbCitizen:
		case Race::TrollCrewMember:
		case Race::PirateDeckhand:
		case Race::BrokenSkullPirate:
			return "Troll";
		case Race::Ogre:
		case Race::OggokCitizen:
		case Race::Ogre2:
			return "Ogre";
		case Race::Halfling:
		case Race::RivervaleCitizen:
			return "Halfling";
		case Race::Gnome:
			return "Gnome";
		case Race::Aviak:
		case Race::Aviak2:
			return "Aviak";
		case Race::Werewolf:
		case Race::Werewolf2:
		case Race::Werewolf3:
			return "Werewolf";
		case Race::Brownie:
		case Race::Brownie2:
			return "Brownie";
		case Race::Centaur:
		case Race::Centaur2:
			return "Centaur";
		case Race::Golem:
		case Race::Golem2:
			return "Golem";
		case Race::Giant:
		case Race::ForestGiant:
		case Race::FrostGiant:
		case Race::StormGiant:
		case Race::EarthGolem:
		case Race::IronGolem:
		case Race::StormGolem:
		case Race::AirGolem:
		case Race::WoodGolem:
		case Race::FireGolem:
		case Race::WaterGolem:
		case Race::Giant2:
		case Race::Giant3:
		case Race::Giant4:
			return "Giant";
		case Race::Trakanon:
			return "Trakanon";
		case Race::VenrilSathir:
			return "Venril Sathir";
		case Race::EvilEye:
		case Race::EvilEye2:
		case Race::EvilEye3:
			return "Evil Eye";
		case Race::Beetle:
		case Race::Beetle2:
		case Race::Beetle3:
			return "Beetle";
		case Race::Kerran:
		case Race::Kerran2:
			return "Kerran";
		case Race::Fish:
		case Race::KunarkFish:
			return "Fish";
		case Race::Fairy:
		case Race::Fairy2:
			return "Fairy";
		case Race::Froglok:
		case Race::FroglokGhoul:
		case Race::Froglok2:
			return "Froglok";
		case Race::Fungusman:
			return "Fungusman";
		case Race::Gargoyle:
		case Race::Gargoyle2:
			return "Gargoyle";
		case Race::Gasbag:
			return "Gasbag";
		case Race::GelatinousCube:
		case Race::GelatinousCube2:
			return "Gelatinous Cube";
		case Race::Ghost:
		case Race::DwarfGhost:
		case Race::EruditeGhost:
		case Race::PirateGhost:
			return "Ghost";
		case Race::Ghoul:
		case Race::Ghoul2:
			return "Ghoul";
		case Race::GiantBat:
		case Race::Bat:
		case Race::Bat2:
			return "Bat";
		case Race::GiantEel:
			return "Eel";
		case Race::GiantRat:
		case Race::Rat:
			return "Rat";
		case Race::GiantSnake:
		case Race::Snake:
			return "Snake";
		case Race::GiantSpider:
		case Race::Spider:
			return "Spider";
		case Race::Gnoll:
		case Race::Gnoll2:
		case Race::Gnoll3:
			return "Gnoll";
		case Race::Goblin:
		case Race::Bloodgill:
		case Race::KunarkGoblin:
		case Race::NewGoblin:
		case Race::Goblin2:
			return "Goblin";
		case Race::Gorilla:
		case Race::Gorilla2:
			return "Gorilla";
		case Race::Wolf:
		case Race::WolfElemental:
		case Race::Wolf2:
			return "Wolf";
		case Race::Bear:
		case Race::Bear2:
		case Race::Bear3:
			return "Bear";
		case Race::FreeportGuard:
		case Race::Felguard:
		case Race::Fayguard:
			return "Guard";
		case Race::DemiLich:
			return "Demi Lich";
		case Race::Imp:
			return "Imp";
		case Race::Griffin:
		case Race::Griffin2:
			return "Griffin";
		case Race::Kobold:
		case Race::Kobold2:
			return "Kobold";
		case Race::LavaDragon:
		case Race::DragonSkeleton:
		case Race::WaterDragon:
		case Race::VeliousDragon:
		case Race::ClockworkDragon:
		case Race::BlackAndWhiteDragon:
		case Race::GhostDragon:
		case Race::PrismaticDragon:
		case Race::Dragon:
		case Race::Dragon2:
		case Race::Dragon3:
		case Race::Dragon4:
		case Race::Dragon5:
		case Race::Dragon6:
		case Race::Dragon7:
			return "Dragon";
		case Race::Lion:
			return "Lion";
		case Race::LizardMan:
			return "Lizard Man";
		case Race::Mimic:
			return "Mimic";
		case Race::Minotaur:
		case Race::Minotaur2:
		case Race::Minotaur3:
		case Race::Minotaur4:
			return "Minotaur";
		case Race::Orc:
		case Race::Orc2:
			return "Orc";
		case Race::HumanBeggar:
			return "Beggar";
		case Race::Pixie:
			return "Pixie";
		case Race::Drachnid:
		case Race::Drachnid2:
			return "Drachnid";
		case Race::SolusekRo:
		case Race::SolusekRo2:
			return "Solusek Ro";
		case Race::Skeleton:
		case Race::Skeleton2:
		case Race::Skeleton3:
			return "Skeleton";
		case Race::Shark:
			return "Shark";
		case Race::Tunare:
			return "Tunare";
		case Race::Tiger:
			return "Tiger";
		case Race::Treant:
		case Race::Treant2:
		case Race::Treant3:
			return "Treant";
		case Race::Vampire:
		case Race::ElfVampire:
		case Race::Vampire2:
		case Race::VampireVolatalis:
		case Race::UndeadVampire:
		case Race::Vampire3:
		case Race::MasterVampire:
		case Race::Vampire4:
			return "Vampire";
		case Race::StatueOfRallosZek:
		case Race::NewRallosZek:
			return "Rallos Zek";
		case Race::TentacleTerror:
		case Race::TentacleTerror2:
			return "Tentacle Terror";
		case Race::Wisp:
			return "Will-O-Wisp";
		case Race::Zombie:
		case Race::Zombie2:
			return "Zombie";
		case Race::Ship:
			return "Ship";
		case Race::Launch:
			return "Launch";
		case Race::Piranha:
			return "Piranha";
		case Race::Elemental:
			return "Elemental";
		case Race::Puma:
		case Race::Puma2:
		case Race::Puma3:
			return "Puma";
		case Race::Bixie:
		case Race::Bixie2:
			return "Bixie";
		case Race::ReanimatedHand:
			return "Reanimated Hand";
		case Race::Scarecrow:
		case Race::Scarecrow2:
			return "Scarecrow";
		case Race::Skunk:
			return "Skunk";
		case Race::SnakeElemental:
			return "Snake Elemental";
		case Race::Spectre:
		case Race::Spectre2:
			return "Spectre";
		case Race::Sphinx:
		case Race::Sphinx2:
			return "Sphinx";
		case Race::Armadillo:
			return "Armadillo";
		case Race::ClockworkGnome:
			return "Clockwork Gnome";
		case Race::Drake:
		case Race::Drake2:
		case Race::Drake3:
			return "Drake";
		case Race::Alligator:
		case Race::Alligator2:
			return "Alligator";
		case Race::CazicThule:
		case Race::CazicThule2:
			return "Cazic Thule";
		case Race::Cockatrice:
			return "Cockatrice";
		case Race::DaisyMan:
			return "Daisy Man";
		case Race::Denizen:
		case Race::Amygdalan:
			return "Amygdalan";
		case Race::Dervish:
		case Race::Dervish2:
		case Race::Dervish3:
		case Race::Dervish4:
		case Race::Dervish5:
		case Race::Dervish6:
			return "Dervish";
		case Race::Efreeti:
		case Race::Efreeti2:
			return "Efreeti";
		case Race::FroglokTadpole:
			return "Tadpole";
		case Race::PhinigelAutropos:
		case Race::Kedge:
			return "Kedge";
		case Race::Leech:
			return "Leech";
		case Race::Swordfish:
			return "Swordfish";
		case Race::Mammoth:
		case Race::Mammoth2:
			return "Mammoth";
		case Race::EyeOfZomm:
			return "Eye";
		case Race::Wasp:
			return "Wasp";
		case Race::Mermaid:
			return "Mermaid";
		case Race::Harpy:
		case Race::Harpy2:
			return "Harpy";
		case Race::Drixie:
			return "Drixie";
		case Race::GhostShip:
		case Race::GhostShip2:
			return "Ghost Ship";
		case Race::Clam:
			return "Clam";
		case Race::SeaHorse:
			return "Seahorse";
		case Race::Sabertooth:
			return "Saber-toothed Cat";
		case Race::Gorgon:
			return "Gorgon";
		case Race::Innoruuk:
			return "Innoruuk";
		case Race::Unicorn:
		case Race::Unicorn2:
		case Race::Unicorn3:
			return "Unicorn";
		case Race::Pegasus:
		case Race::Pegasus2:
		case Race::Pegasus3:
			return "Pegasus";
		case Race::Djinn:
			return "Djinn";
		case Race::InvisibleMan:
		case Race::InvisibleMan2:
		case Race::InvisibleMan3:
			return "Invisible Man";
		case Race::Iksar:
		case Race::IksarCitizen:
			return "Iksar";
		case Race::Scorpion:
		case Race::IksarScorpion:
		case Race::Scorpion2:
			return "Scorpion";
		case Race::VahShir:
		case Race::VahShirKing:
		case Race::VahShirGuard:
			return "Vah Shir";
		case Race::Sarnak:
		case Race::Sarnak2:
			return "Sarnak";
		case Race::Draglock:
			return "Draglok";
		case Race::Drolvarg:
			return "Drolvarg";
		case Race::Mosquito:
			return "Mosquito";
		case Race::Rhinoceros:
			return "Rhinoceros";
		case Race::Xalgoz:
			return "Xalgoz";
		case Race::Yeti:
			return "Yeti";
		case Race::Boat:
		case Race::Boat2:
			return "Boat";
		case Race::MinorIllusion:
			return "Minor Illusion";
		case Race::Tree:
			return "Tree";
		case Race::Burynai:
		case Race::Burynai2:
			return "Burynai";
		case Race::Goo:
		case Race::Goo2:
		case Race::Goo3:
		case Race::Goo4:
			return "Goo";
		case Race::SarnakSpirit:
			return "Sarnak Spirit";
		case Race::IksarSpirit:
			return "Iksar Spirit";
		case Race::Erollisi:
			return "Erollisi";
		case Race::Tribunal:
		case Race::TribunalNew:
			return "Tribunal";
		case Race::Bertoxxulous:
		case Race::BertoxxulousNew:
			return "Bertoxxulous";
		case Race::Bristlebane:
			return "Bristlebane";
		case Race::FayDrake:
			return "Fay Drake";
		case Race::UndeadSarnak:
			return "Undead Sarnak";
		case Race::Ratman:
		case Race::Ratman2:
			return "Ratman";
		case Race::Wyvern:
		case Race::Wyvern2:
			return "Wyvern";
		case Race::Wurm:
		case Race::Wurm2:
			return "Wurm";
		case Race::Devourer:
			return "Devourer";
		case Race::IksarGolem:
			return "Iksar Golem";
		case Race::UndeadIksar:
			return "Undead Iksar";
		case Race::ManEatingPlant:
			return "Man-Eating Plant";
		case Race::Raptor:
		case Race::Raptor2:
			return "Raptor";
		case Race::SarnakGolem:
			return "Sarnak Golem";
		case Race::AnimatedHand:
			return "Animated Hand";
		case Race::Succulent:
			return "Succulent";
		case Race::Holgresh:
			return "Holgresh";
		case Race::Brontotherium:
			return "Brontotherium";
		case Race::SnowDervish:
			return "Snow Dervish";
		case Race::DireWolf:
			return "Dire Wolf";
		case Race::Manticore:
			return "Manticore";
		case Race::Totem:
		case Race::Totem2:
			return "Totem";
		case Race::IceSpectre:
			return "Ice Spectre";
		case Race::EnchantedArmor:
			return "Enchanted Armor";
		case Race::SnowRabbit:
			return "Snow Rabbit";
		case Race::Walrus:
			return "Walrus";
		case Race::Geonid:
			return "Geonid";
		case Race::Yakkar:
			return "Yakkar";
		case Race::Faun:
			return "Faun";
		case Race::Coldain:
		case Race::Coldain2:
		case Race::Coldain3:
			return "Coldain";
		case Race::Hag:
			return "Hag";
		case Race::Hippogriff:
			return "Hippogriff";
		case Race::Siren:
		case Race::Siren2:
			return "Siren";
		case Race::Othmir:
			return "Othmir";
		case Race::Ulthork:
			return "Ulthork";
		case Race::Abhorrent:
			return "Abhorrent";
		case Race::SeaTurtle:
			return "Sea Turtle";
		case Race::RonnieTest:
			return "Ronnie Test";
		case Race::Shiknar:
			return "Shik'Nar";
		case Race::Rockhopper:
			return "Rockhopper";
		case Race::Underbulk:
			return "Underbulk";
		case Race::Grimling:
			return "Grimling";
		case Race::Worm:
			return "Worm";
		case Race::EvanTest:
			return "Evan Test";
		case Race::KhatiSha:
			return "Khati Sha";
		case Race::Owlbear:
			return "Owlbear";
		case Race::RhinoBeetle:
			return "Rhino Beetle";
		case Race::EarthElemental:
		case Race::EarthElemental2:
			return "Earth Elemental";
		case Race::AirElemental:
		case Race::AirElemental2:
			return "Air Elemental";
		case Race::WaterElemental:
		case Race::WaterElemental2:
			return "Water Elemental";
		case Race::FireElemental:
		case Race::FireElemental2:
			return "Fire Elemental";
		case Race::WetfangMinnow:
			return "Wetfang Minnow";
		case Race::ThoughtHorror:
			return "Thought Horror";
		case Race::Tegi:
			return "Tegi";
		case Race::Horse:
		case Race::Horse2:
		case Race::Horse3:
			return "Horse";
		case Race::Shissar:
		case Race::Shissar2:
			return "Shissar";
		case Race::FungalFiend:
			return "Fungal Fiend";
		case Race::Stonegrabber:
			return "Stonegrabber";
		case Race::ScarletCheetah:
			return "Cheetah";
		case Race::Zelniak:
			return "Zelniak";
		case Race::Lightcrawler:
			return "Lightcrawler";
		case Race::Shade:
		case Race::Shade2:
		case Race::Shade3:
			return "Shade";
		case Race::Sunflower:
			return "Sunflower";
		case Race::Shadel:
			return "Shadel";
		case Race::Shrieker:
			return "Shrieker";
		case Race::Galorian:
			return "Galorian";
		case Race::Netherbian:
			return "Netherbian";
		case Race::Akhevan:
			return "Akhevan";
		case Race::GriegVeneficus:
			return "Grieg Veneficus";
		case Race::SonicWolf:
			return "Sonic Wolf";
		case Race::GroundShaker:
			return "Ground Shaker";
		case Race::VahShirSkeleton:
			return "Vah Shir Skeleton";
		case Race::Wretch:
			return "Wretch";
		case Race::LordInquisitorSeru:
			return "Lord Inquisitor Seru";
		case Race::Recuso:
			return "Recuso";
		case Race::TeleportMan:
			return "Teleport Man";
		case Race::Nymph:
			return "Nymph";
		case Race::Dryad:
			return "Dryad";
		case Race::Fly:
			return "Fly";
		case Race::TarewMarr:
			return "Tarew Marr";
		case Race::ClockworkGolem:
			return "Clockwork Golem";
		case Race::ClockworkBrain:
			return "Clockwork Brain";
		case Race::Banshee:
		case Race::Banshee2:
		case Race::Banshee3:
			return "Banshee";
		case Race::GuardOfJustice:
			return "Guard of Justice";
		case Race::MiniPom:
			return "Mini POM";
		case Race::DiseasedFiend:
			return "Diseased Fiend";
		case Race::SolusekRoGuard:
			return "Solusek Ro Guard";
		case Race::TerrisThule:
			return "Terris-Thule";
		case Race::Vegerog:
			return "Vegerog";
		case Race::Crocodile:
			return "Crocodile";
		case Race::Hraquis:
			return "Hraquis";
		case Race::Tranquilion:
			return "Tranquilion";
		case Race::TinSoldier:
			return "Tin Soldier";
		case Race::NightmareWraith:
			return "Nightmare Wraith";
		case Race::Malarian:
			return "Malarian";
		case Race::KnightOfPestilence:
			return "Knight of Pestilence";
		case Race::Lepertoloth:
			return "Lepertoloth";
		case Race::Bubonian:
			return "Bubonian";
		case Race::BubonianUnderling:
			return "Bubonian Underling";
		case Race::Pusling:
			return "Pusling";
		case Race::WaterMephit:
			return "Water Mephit";
		case Race::Stormrider:
			return "Stormrider";
		case Race::JunkBeast:
			return "Junk Beast";
		case Race::BrokenClockwork:
			return "Broken Clockwork";
		case Race::GiantClockwork:
			return "Giant Clockwork";
		case Race::ClockworkBeetle:
			return "Clockwork Beetle";
		case Race::NightmareGoblin:
			return "Nightmare Goblin";
		case Race::Karana:
			return "Karana";
		case Race::BloodRaven:
			return "Blood Raven";
		case Race::NightmareGargoyle:
			return "Nightmare Gargoyle";
		case Race::MouthOfInsanity:
			return "Mouth of Insanity";
		case Race::SkeletalHorse:
			return "Skeletal Horse";
		case Race::Saryrn:
			return "Saryrn";
		case Race::FenninRo:
			return "Fennin Ro";
		case Race::Tormentor:
			return "Tormentor";
		case Race::SoulDevourer:
			return "Soul Devourer";
		case Race::Nightmare:
			return "Nightmare";
		case Race::VallonZek:
			return "Vallon Zek";
		case Race::TallonZek:
			return "Tallon Zek";
		case Race::AirMephit:
			return "Air Mephit";
		case Race::EarthMephit:
			return "Earth Mephit";
		case Race::FireMephit:
			return "Fire Mephit";
		case Race::NightmareMephit:
			return "Nightmare Mephit";
		case Race::Zebuxoruk:
			return "Zebuxoruk";
		case Race::MithanielMarr:
			return "Mithaniel Marr";
		case Race::UndeadKnight:
			return "Undead Knight";
		case Race::Rathe:
			return "The Rathe";
		case Race::Xegony:
			return "Xegony";
		case Race::Fiend:
			return "Fiend";
		case Race::TestObject:
			return "Test Object";
		case Race::Crab:
			return "Crab";
		case Race::Phoenix:
			return "Phoenix";
		case Race::Quarm:
			return "Quarm";
		case Race::WarWraith:
			return "War Wraith";
		case Race::Wrulon:
		case Race::Wrulon2:
			return "Wrulon";
		case Race::Kraken:
			return "Kraken";
		case Race::PoisonFrog:
			return "Poison Frog";
		case Race::Nilborien:
			return "Nilborien";
		case Race::Valorian:
		case Race::Valorian2:
			return "Valorian";
		case Race::WarBoar:
		case Race::WarBoar2:
			return "War Boar";
		case Race::AnimatedArmor:
			return "Animated Armor";
		case Race::UndeadFootman:
			return "Undead Footman";
		case Race::RallosOgre:
			return "Rallos Zek Minion";
		case Race::Arachnid:
			return "Arachnid";
		case Race::CrystalSpider:
			return "Crystal Spider";
		case Race::ZebuxoruksCage:
			return "Zebuxoruk's Cage";
		case Race::Portal:
			return "BoT Portal";
		case Race::OneArmedPirate:
		case Race::SpiritmasterNadox:
		case Race::BrokenSkullTaskmaster:
		case Race::GnomePirate:
		case Race::DarkElfPirate:
		case Race::OgrePirate:
		case Race::HumanPirate:
		case Race::EruditePirate:
			return "Pirate";
		case Race::Frog:
		case Race::Frog2:
			return "Frog";
		case Race::TrollZombie:
			return "Troll Zombie";
		case Race::Luggald:
		case Race::Luggald2:
		case Race::Luggald3:
			return "Luggald";
		case Race::Drogmor:
			return "Drogmor";
		case Race::FroglokSkeleton:
			return "Froglok Skeleton";
		case Race::UndeadFroglok:
			return "Undead Froglok";
		case Race::KnightOfHate:
			return "Knight of Hate";
		case Race::ArcanistOfHate:
			return "Arcanist of Hate";
		case Race::Veksar:
		case Race::Veksar2:
		case Race::Veksar3:
			return "Veksar";
		case Race::Chokidai:
			return "Chokidai";
		case Race::UndeadChokidai:
			return "Undead Chokidai";
		case Race::UndeadVeksar:
			return "Undead Veksar";
		case Race::RujarkianOrc:
		case Race::MasterOrc:
			return "Rujarkian Orc";
		case Race::BoneGolem:
		case Race::BoneGolem2:
			return "Bone Golem";
		case Race::Synarcana:
			return "Synarcana";
		case Race::SandElf:
			return "Sand Elf";
		case Race::Mummy:
			return "Mummy";
		case Race::Insect:
			return "Insect";
		case Race::FroglokGhost:
			return "Froglok Ghost";
		case Race::Box:
			return "Box";
		case Race::Barrel:
			return "Barrel";
		case Race::Chest:
		case Race::Chest2:
		case Race::Chest3:
			return "Chest";
		case Race::Vase:
			return "Vase";
		case Race::Table:
			return "Table";
		case Race::WeaponRack:
		case Race::WeaponRack2:
			return "Weapon Rack";
		case Race::Coffin:
		case Race::Coffin2:
			return "Coffin";
		case Race::Bones:
			return "Bones";
		case Race::Jokester:
			return "Jokester";
		case Race::Nihil:
			return "Nihil";
		case Race::Trusik:
			return "Trusik";
		case Race::StoneWorker:
		case Race::StoneWorker2:
			return "Stone Worker";
		case Race::Hynid:
			return "Hynid";
		case Race::Turepta:
			return "Turepta";
		case Race::Cragbeast:
			return "Cragbeast";
		case Race::Stonemite:
			return "Stonemite";
		case Race::Ukun:
			return "Ukun";
		case Race::Ixt:
			return "Ixt";
		case Race::Ikaav:
			return "Ikaav";
		case Race::Aneuk:
			return "Aneuk";
		case Race::Kyv:
			return "Kyv";
		case Race::Noc:
			return "Noc";
		case Race::Ratuk:
			return "Ra`tuk";
		case Race::Taneth:
			return "Taneth";
		case Race::Huvul:
			return "Huvul";
		case Race::Mutna:
			return "Mutna";
		case Race::Mastruq:
			return "Mastruq";
		case Race::Taelosian:
			return "Taelosian";
		case Race::DiscordShip:
			return "Discord Ship";
		case Race::MataMuram:
			return "Mata Muram";
		case Race::LightingWarrior:
			return "Lightning Warrior";
		case Race::Succubus:
			return "Succubus";
		case Race::Bazu:
			return "Bazu";
		case Race::Feran:
			return "Feran";
		case Race::Pyrilen:
			return "Pyrilen";
		case Race::Chimera:
		case Race::Chimera2:
			return "Chimera";
		case Race::Dragorn:
			return "Dragorn";
		case Race::Murkglider:
			return "Murkglider";
		case Race::Gelidran:
			return "Gelidran";
		case Race::Discordling:
			return "Discordling";
		case Race::Girplan:
			return "Girplan";
		case Race::DragornBox:
			return "Dragorn Box";
		case Race::RunedOrb:
			return "Runed Orb";
		case Race::DragonBones:
			return "Dragon Bones";
		case Race::MuramiteArmorPile:
			return "Muramite Armor Pile";
		case Race::CrystalShard:
			return "Crystal Shard";
		case Race::Portal2:
			return "Portal";
		case Race::CoinPurse:
			return "Coin Purse";
		case Race::RockPile:
			return "Rock Pile";
		case Race::MurkgliderEggSack:
			return "Murglider Egg Sack";
		case Race::Kirin:
		case Race::Kirin2:
			return "Kirin";
		case Race::Basilisk:
			return "Basilisk";
		case Race::SpiderQueen:
			return "Spider Queen";
		case Race::AnimatedStatue:
		case Race::AnimatedStatue2:
			return "Animated Statue";
		case Race::DragonEgg:
			return "Dragon Egg";
		case Race::DragonStatue:
			return "Dragon Statue";
		case Race::LavaRock:
			return "Lava Rock";
		case Race::SpiderEggSack:
			return "Spider Egg Sack";
		case Race::LavaSpider:
			return "Lava Spider";
		case Race::LavaSpiderQueen:
			return "Lava Spider Queen";
		case Race::Sporali:
			return "Sporali";
		case Race::Gnomework:
			return "Gnomework";
		case Race::Corathus:
			return "Corathus";
		case Race::Coral:
			return "Coral";
		case Race::DrachnidCocoon:
			return "Drachnid Cocoon";
		case Race::FungusPatch:
			return "Fungus Patch";
		case Race::Witheran:
		case Race::Witheran2:
			return "Witheran";
		case Race::DarkLord:
			return "Dark Lord";
		case Race::Shiliskin:
			return "Shiliskin";
		case Race::ClockworkBoar:
			return "Clockwork Boar";
		case Race::ScaledWolf:
			return "Scaled Wolf";
		case Race::SpiritWolf:
			return "Spirit Wolf";
		case Race::Bolvirk:
			return "Bolvirk";
		case Race::Elddar:
			return "Elddar";
		case Race::ForestGiant2:
			return "Forest Giant";
		case Race::ShamblingMound:
			return "Shambling Mound";
		case Race::Scrykin:
			return "Scrykin";
		case Race::AyonaeRo:
			return "Ayonae Ro";
		case Race::SullonZek:
			return "Sullon Zek";
		case Race::Banner:
		case Race::Banner2:
		case Race::Banner3:
		case Race::Banner4:
		case Race::Banner5:
		case Race::Banner6:
		case Race::Banner7:
			return "Banner";
		case Race::Flag:
			return "Flag";
		case Race::Rowboat:
			return "Rowboat";
		case Race::BearTrap:
			return "Bear Trap";
		case Race::ClockworkBomb:
			return "Clockwork Bomb";
		case Race::DynamiteKeg:
			return "Dynamite Keg";
		case Race::PressurePlate:
			return "Pressure Plate";
		case Race::PufferSpore:
			return "Puffer Spore";
		case Race::StoneRing:
			return "Stone Ring";
		case Race::RootTentacle:
			return "Root Tentacle";
		case Race::RunicSymbol:
			return "Runic Symbol";
		case Race::SaltpetterBomb:
			return "Saltpetter Bomb";
		case Race::FloatingSkull:
			return "Floating Skull";
		case Race::SpikeTrap:
			return "Spike Trap";
		case Race::Web:
			return "Web";
		case Race::WickerBasket:
			return "Wicker Basket";
		case Race::Drakkin:
			return "Drakkin";
		case Race::GiantShade:
			return "Giant Shade";
		case Race::Satyr:
			return "Satyr";
		case Race::Dynleth:
			return "Dyn'Leth";
		case Race::ArmorRack:
			return "Armor Rack";
		case Race::HoneyPot:
			return "Honey Pot";
		case Race::JumJumBucket:
			return "Jum Jum Bucket";
		case Race::Plant:
		case Race::Plant2:
			return "Plant";
		case Race::StoneJug:
		case Race::StoneJug2:
			return "Stone Jug";
		case Race::Toolbox:
			return "Toolbox";
		case Race::WineCask:
		case Race::WineCask2:
			return "Wine Cask";
		case Race::ElvenBoat:
			return "Elven Boat";
		case Race::GnomishBoat:
			return "Gnomish Boat";
		case Race::UndeadBoat:
			return "Undead Boat";
		case Race::MerchantShip:
			return "Merchant Ship";
		case Race::PirateShip:
			return "Pirate Ship";
		case Race::Campfire:
			return "Campfire";
		case Race::Exoskeleton:
			return "Exoskeleton";
		case Race::ClockworkGuardian:
			return "Clockwork Guardian";
		case Race::Rotocopter:
			return "Rotocopter";
		case Race::Wereorc:
			return "Wereorc";
		case Race::Worg:
		case Race::Worg2:
			return "Worg";
		case Race::Boulder:
			return "Boulder";
		case Race::ElvenGhost:
			return "Elven Ghost";
		case Race::HumanGhost:
			return "Human Ghost";
		case Race::Crystal:
			return "Crystal";
		case Race::GuardianCpu:
			return "Guardian CPU";
		case Race::Mansion:
			return "Mansion";
		case Race::FloatingIsland:
			return "Floating Island";
		case Race::Cragslither:
			return "Cragslither";
		case Race::SpellParticle:
			return "Spell Particle";
		case Race::InvisibleManOfZomm:
			return "Invisible Man of Zomm";
		case Race::RobocopterOfZomm:
			return "Robocopter of Zomm";
		case Race::Dracolich:
			return "Dracolich";
		case Race::IksarGhost:
			return "Iksar Ghost";
		case Race::IksarSkeleton:
			return "Iksar Skeleton";
		case Race::Mephit:
			return "Mephit";
		case Race::Muddite:
			return "Muddite";
		case Race::Tsetsian:
			return "Tsetsian";
		case Race::Nekhon:
			return "Nekhon";
		case Race::HydraCrystal:
			return "Hydra Crystal";
		case Race::CrystalSphere:
			return "Crystal Sphere";
		case Race::Sokokar:
			return "Sokokar";
		case Race::StonePylon:
			return "Stone Pylon";
		case Race::DemonVulture:
			return "Demon Vulture";
		case Race::Wagon:
			return "Wagon";
		case Race::GodOfDiscord:
			return "God of Discord";
		case Race::FeranMount:
			return "Feran Mount";
		case Race::SokokarMount:
		case Race::SokokarMount2:
			return "Sokokar Mount";
		case Race::TenthAnniversaryBanner:
			return "10th Anniversary Banner";
		case Race::TenthAnniversaryCake:
			return "10th Anniversary Cake";
		case Race::HydraMount:
			return "Hydra Mount";
		case Race::Hydra:
			return "Hydra";
		case Race::WeddingFlowers:
			return "Wedding Flowers";
		case Race::WeddingArbor:
			return "Wedding Arbor";
		case Race::WeddingAltar:
			return "Wedding Altar";
		case Race::PowderKeg:
			return "Powder Keg";
		case Race::Apexus:
			return "Apexus";
		case Race::Bellikos:
			return "Bellikos";
		case Race::BrellsFirstCreation:
			return "Brell's First Creation";
		case Race::Brell:
			return "Brell";
		case Race::CrystalskinAmbuloid:
			return "Crystalskin Ambuloud";
		case Race::CliknarQueen:
			return "Cliknar Queen";
		case Race::CliknarSoldier:
			return "Cliknar Soldier";
		case Race::CliknarWorker:
			return "Cliknar Worker";
		case Race::CrystalskinSessiloid:
			return "Crystalskin Sessiloid";
		case Race::Genari:
			return "Genari";
		case Race::Gigyn:
			return "Gigyn";
		case Race::GrekenYoungAdult:
			return "Greken - Young Adult";
		case Race::GrekenYoung:
			return "Greken - Young";
		case Race::CliknarMount:
			return "Cliknar Mount";
		case Race::Telmira:
			return "Telmira";
		case Race::SpiderMount:
			return "Spider Mount";
		case Race::BearMount:
			return "Bear Mount";
		case Race::RatMount:
			return "Rat Mount";
		case Race::SessiloidMount:
			return "Sessiloid Mount";
		case Race::MorellThule:
			return "Morell-Thule";
		case Race::Marionette:
			return "Marionette";
		case Race::BookDervish:
			return "Book Dervish";
		case Race::TopiaryLion:
			return "Topiary Lion";
		case Race::RotDog:
			return "Rot Dog";
		case Race::Sandman:
			return "Sandman";
		case Race::GrandfatherClock:
			return "Grandfather Clock";
		case Race::GingerbreadMan:
			return "Gingerbread Man";
		case Race::RoyalGuard:
			return "Royal Guard";
		case Race::Rabbit:
			return "Rabbit";
		case Race::BlindDreamer:
			return "Blind Dreamer";
		case Race::TopiaryLionMount:
			return "Topiary Lion Mount";
		case Race::RotDogMount:
			return "Rot Dog Mount";
		case Race::GoralMount:
			return "Goral Mount";
		case Race::SelyrahMount:
			return "Selyrah Mount";
		case Race::ScleraMount:
			return "Sclera Mount";
		case Race::BraxiMount:
			return "Braxi Mount";
		case Race::KangonMount:
			return "Kangon Mount";
		case Race::WurmMount:
			return "Wurm Mount";
		case Race::RaptorMount:
			return "Raptor Mount";
		case Race::Whirligig:
			return "Whirligig";
		case Race::GnomishBalloon:
			return "Gnomish Balloon";
		case Race::GnomishRocketPack:
			return "Gnomish Rocket Pack";
		case Race::GnomishHoveringTransport:
			return "Gnomish Hovering Transport";
		case Race::Selyrah:
			return "Selyrah";
		case Race::Goral:
			return "Goral";
		case Race::Braxi:
			return "Braxi";
		case Race::Kangon:
			return "Kangon";
		case Race::FloatingTower:
			return "Floating Tower";
		case Race::ExplosiveCart:
			return "Explosive Cart";
		case Race::BlimpShip:
			return "Blimp Ship";
		case Race::Tumbleweed:
			return "Tumbleweed";
		case Race::Alaran:
			return "Alaran";
		case Race::Swinetor:
			return "Swinetor";
		case Race::Triumvirate:
			return "Triumvirate";
		case Race::Hadal:
			return "Hadal";
		case Race::HoveringPlatform:
			return "Hovering Platform";
		case Race::ParasiticScavenger:
			return "Parasitic Scavenger";
		case Race::Grendlaen:
			return "Grendlaen";
		case Race::ShipInABottle:
			return "Ship in a Bottle";
		case Race::AlaranSentryStone:
			return "Alaran Sentry Stone";
		case Race::RegenerationPool:
			return "Regeneration Pool";
		case Race::TeleportationStand:
			return "Teleportation Stand";
		case Race::RelicCase:
			return "Relic Case";
		case Race::AlaranGhost:
			return "Alaran Ghost";
		case Race::Skystrider:
			return "Skystrider";
		case Race::WaterSpout:
			return "Water Spout";
		case Race::AviakPullAlong:
			return "Aviak Pull Along";
		case Race::Cat:
			return "Cat";
		case Race::ElkHead:
			return "Elk Head";
		case Race::Holgresh2:
			return "Holgresh";
		case Race::VineMaw:
			return "Vine Maw";
		case Race::FallenKnight:
			return "Fallen Knight";
		case Race::FlyingCarpet:
			return "Flying Carpet";
		case Race::CarrierHand:
			return "Carrier Hand";
		case Race::Akheva:
			return "Akheva";
		case Race::ServantOfShadow:
			return "Servant of Shadow";
		case Race::Luclin:
		case Race::Luclin2:
		case Race::Luclin3:
		case Race::Luclin4:
			return "Luclin";
		case Race::Xaric:
			return "Xaric";
		case Race::Orb:
			return "Orb";
		case Race::InteractiveObject:
			return "Interactive Object";
		case Race::Node:
			return "Node";
		case Race::Unknown:
		case Race::Unknown2:
		case Race::Unknown3:
		case Race::Unknown4:
		case Race::Unknown5:
		default:
			return "Unknown Race";
	}
}

uint32 GetPlayerRaceValue(uint16 race_id) {
	if (!IsPlayerRace(race_id)) {
		return 0;
	}

	return player_race_indexes[race_id];
}

uint32 GetPlayerRaceBit(uint16 race_id) {
	if (!IsPlayerRace(race_id)) {
		return 0;
	}

	return player_race_bitmasks[race_id];
}

float GetRaceGenderDefaultHeight(int race, int gender)
{
	static float male_height[] = {
	    6.0f,  6.0f,  7.0f,   6.0f,   5.0f,  6.0f,  5.0f,   5.5f,  4.0f,  8.0f,  9.0f,  3.5f,  3.0f,  6.0f,   6.0f,
	    2.0f,  8.5f,  8.0f,   21.0f,  20.0f, 6.0f,  6.0f,   3.5f,  3.0f,  6.0f,  2.0f,  5.0f,  5.0f,  6.0f,   6.0f,
	    6.0f,  7.5f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  5.0f,  6.0f,  6.0f,  7.0f,  4.0f,  4.7f,   6.0f,
	    8.0f,  3.0f,  12.0f,  5.0f,   21.0f, 6.0f,  6.0f,   3.0f,  9.0f,  6.0f,  6.0f,  2.0f,  6.0f,  3.0f,   6.0f,
	    4.0f,  20.0f, 5.0f,   5.0f,   6.0f,  9.0f,  25.0f,  6.0f,  6.0f,  10.0f, 6.0f,  6.0f,  6.0f,  6.0f,   2.5f,
	    7.0f,  6.0f,  5.0f,   6.0f,   1.5f,  1.0f,  3.5f,   7.0f,  6.0f,  6.0f,  6.0f,  6.0f,  7.0f,  3.0f,   3.0f,
	    7.0f,  12.0f, 8.0f,   9.0f,   4.0f,  11.5f, 8.0f,   6.0f,  6.0f,  12.0f, 6.0f,  6.0f,  6.0f,  20.0f,  10.0f,
	    6.5f,  6.0f,  17.0f,  1.0f,   4.0f,  6.0f,  8.0f,   5.0f,  1.0f,  6.0f,  6.0f,  5.0f,  5.0f,  5.0f,   9.0f,
	    3.0f,  8.0f,  2.0f,   24.0f,  6.0f,  10.0f, 6.0f,   6.0f,  6.0f,  3.0f,  7.0f,  9.0f,  6.0f,  11.0f,  2.5f,
	    14.0f, 8.0f,  7.0f,   12.0f,  6.0f,  27.0f, 6.0f,   6.0f,  6.0f,  6.0f,  2.0f,  9.0f,  9.0f,  6.0f,   9.0f,
	    3.0f,  3.0f,  6.0f,   6.0f,   10.0f, 6.0f,  6.0f,   15.0f, 15.0f, 9.0f,  7.0f,  6.0f,  6.0f,  7.0f,   8.0f,
	    3.0f,  3.0f,  6.0f,   7.0f,   13.0f, 6.0f,  6.0f,   9.0f,  5.0f,  7.0f,  9.0f,  6.0f,  6.0f,  8.0f,   6.0f,
	    6.0f,  5.5f,  6.0f,   4.0f,   25.0f, 6.0f,  6.0f,   6.0f,  22.0f, 20.0f, 6.0f,  10.0f, 13.5f, 12.0f,  3.0f,
	    30.0f, 6.0f,  6.0f,   35.0f,  1.5f,  8.0f,  3.0f,   6.0f,  2.0f,  6.0f,  6.0f,  5.0f,  2.0f,  7.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   4.0f,   6.0f,  6.0f,  6.0f,   8.0f,  8.0f,  7.0f,  8.0f,  6.0f,  7.0f,  6.0f,   7.0f,
	    6.0f,  10.0f, 3.0f,   6.0f,   8.0f,  9.0f,  15.0f,  5.0f,  10.0f, 7.0f,  6.0f,  7.0f,  6.0f,  7.0f,   7.0f,
	    12.0f, 6.0f,  4.0f,   6.0f,   5.0f,  3.0f,  30.0f,  30.0f, 15.0f, 20.0f, 6.0f,  10.0f, 6.0f,  14.0f,  14.0f,
	    16.0f, 15.0f, 30.0f,  15.0f,  7.5f,  5.0f,  4.0f,   6.0f,  15.0f, 6.5f,  3.0f,  12.0f, 10.0f, 10.5f,  10.0f,
	    7.5f,  6.0f,  6.0f,   12.5f,  9.0f,  20.0f, 2.0f,   10.0f, 25.0f, 8.0f,  6.0f,  6.0f,  10.0f, 18.0f,  45.0f,
	    13.0f, 15.0f, 8.0f,   30.0f,  25.0f, 25.0f, 10.0f,  13.0f, 5.0f,  3.5f,  15.0f, 35.0f, 11.0f, 15.0f,  50.0f,
	    13.0f, 6.0f,  7.0f,   6.0f,   60.0f, 6.0f,  22.0f,  22.0f, 21.0f, 22.0f, 15.0f, 25.0f, 23.0f, 8.0f,   15.0f,
	    10.0f, 6.0f,  7.0f,   6.0f,   12.0f, 9.5f,  6.0f,   12.0f, 12.0f, 12.0f, 15.0f, 4.0f,  5.0f,  105.0f, 20.0f,
	    5.0f,  10.0f, 10.0f,  10.0f,  20.0f, 13.5f, 8.0f,   10.0f, 3.0f,  5.0f,  9.0f,  6.0f,  6.0f,  6.0f,   10.0f,
	    8.0f,  8.0f,  8.0f,   6.0f,   6.0f,  5.0f,  5.0f,   5.0f,  9.0f,  9.0f,  9.0f,  6.0f,  8.5f,  6.0f,   7.0f,
	    8.0f,  7.0f,  11.0f,  6.0f,   7.0f,  9.0f,  8.0f,   6.0f,  8.0f,  6.0f,  6.0f,  6.0f,  6.0f,  9.0f,   10.0f,
	    6.0f,  3.0f,  4.0f,   3.0f,   3.0f,  4.0f,  10.0f,  10.0f, 2.0f,  8.0f,  6.0f,  6.0f,  14.0f, 7.0f,   5.0f,
	    9.0f,  7.0f,  7.0f,   10.0f,  10.0f, 12.0f, 9.0f,   7.0f,  12.0f, 13.0f, 16.0f, 6.0f,  9.0f,  6.0f,   6.0f,
	    10.0f, 25.0f, 15.0f,  6.0f,   25.0f, 6.0f,  6.0f,   8.0f,  11.0f, 6.0f,  9.0f,  2.0f,  6.0f,  5.0f,   4.0f,
	    8.5f,  6.0f,  6.0f,   6.0f,   4.0f,  6.0f,  15.0f,  1.0f,  2.0f,  6.0f,  40.0f, 8.0f,  12.0f, 3.0f,   8.0f,
	    99.0f, 9.0f,  100.0f, 100.0f, 10.0f, 6.0f,  27.5f,  20.0f, 6.0f,  6.0f,  5.0f,  6.0f,  8.0f,  5.0f,   3.0f,
	    11.5f, 25.0f, 80.0f,  20.0f,  9.0f,  8.0f,  5.0f,   4.0f,  7.0f,  10.0f, 6.0f,  11.0f, 8.0f,  5.0f,   6.0f,
	    6.0f,  30.0f, 7.0f,   15.0f,  9.0f,  6.0f,  9.0f,   6.0f,  3.0f,  32.5f, 15.0f, 7.5f,  10.0f, 10.0f,  6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  9.0f,  20.0f,  6.0f,  6.0f,  6.0f,  25.0f, 12.0f, 6.0f,  8.0f,   6.0f,
	    6.0f,  20.0f, 10.0f,  8.0f,   12.0f, 8.0f,  2.0f,   6.0f,  3.0f,  6.0f,  7.0f,  1.5f,  6.0f,  3.0f,   3.0f,
	    3.0f,  3.0f,  2.0f,   3.0f,   3.0f,  6.0f,  6.0f,   6.0f,  4.5f,  7.0f,  6.0f,  7.0f,  6.0f,  22.0f,  8.0f,
	    15.0f, 22.0f, 8.0f,   15.0f,  6.0f,  80.0f, 150.0f, 7.0f,  6.0f,  6.0f,  6.0f,  12.0f, 6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   35.0f, 20.0f, 9.0f,  6.0f,  6.0f,  6.0f,  20.0f,  20.0f,
	    20.0f, 20.0f, 20.0f,  9.0f,   4.0f,  4.0f,  10.0f,  5.0f,  8.0f,  6.0f,  10.0f, 6.0f,  6.0f,  2.0f,   36.0f,
	    14.0f, 7.0f,  250.0f, 6.0f,   9.0f,  6.0f,  7.0f,   4.0f,  6.0f,  8.0f,  6.0f,  23.0f, 6.0f,  6.0f,   6.0f,
	    70.0f, 6.0f,  7.0f,   6.0f,   6.0f,  6.0f,  20.0f,  6.0f,  6.0f,  6.0f,  5.0f,  1.0f,  6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,   6.0f,
	    4.0f,  4.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  10.0f, 6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  7.0f,  7.0f,   7.0f,  7.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,   8.0f,
	    6.0f,  6.0f,  6.0f,   7.0f,   6.0f,  6.0f,  6.0f,   7.5f,  6.0f,  6.0f,  4.0f,  6.0f,  3.0f,  6.0f,   6.0f,
	    1.0f,  9.0f,  7.0f,   8.0f,   7.0f,  8.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  8.0f,
	};

	static float female_height[] = {
	    6.0f,  6.0f,  7.0f,   6.0f,   5.0f,  6.0f,  5.0f,   5.5f,  4.0f,  8.0f,  9.0f,  3.5f,  3.0f,  6.0f,   6.0f,
	    2.0f,  8.5f,  8.0f,   21.0f,  20.0f, 6.0f,  6.0f,   3.5f,  3.0f,  6.0f,  2.0f,  5.0f,  5.0f,  6.0f,   6.0f,
	    6.0f,  7.5f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  5.0f,  6.0f,  6.0f,  7.0f,  4.0f,  4.7f,   6.0f,
	    8.0f,  3.0f,  12.0f,  5.0f,   21.0f, 6.0f,  6.0f,   3.0f,  9.0f,  6.0f,  6.0f,  2.0f,  6.0f,  3.0f,   6.0f,
	    4.0f,  20.0f, 5.0f,   5.0f,   6.0f,  9.0f,  25.0f,  6.0f,  6.0f,  10.0f, 6.0f,  6.0f,  6.0f,  6.0f,   2.5f,
	    7.0f,  6.0f,  5.0f,   6.0f,   1.5f,  1.0f,  3.5f,   7.0f,  6.0f,  6.0f,  6.0f,  6.0f,  7.0f,  3.0f,   3.0f,
	    7.0f,  12.0f, 8.0f,   9.0f,   4.0f,  11.5f, 8.0f,   6.0f,  6.0f,  12.0f, 6.0f,  6.0f,  6.0f,  20.0f,  10.0f,
	    6.5f,  6.0f,  17.0f,  1.0f,   4.0f,  6.0f,  8.0f,   5.0f,  1.0f,  6.0f,  6.0f,  5.0f,  5.0f,  5.0f,   9.0f,
	    3.0f,  8.0f,  2.0f,   24.0f,  6.0f,  10.0f, 6.0f,   6.0f,  6.0f,  3.0f,  7.0f,  9.0f,  6.0f,  11.0f,  2.5f,
	    14.0f, 8.0f,  7.0f,   12.0f,  6.0f,  27.0f, 6.0f,   6.0f,  6.0f,  6.0f,  2.0f,  9.0f,  9.0f,  6.0f,   9.0f,
	    3.0f,  3.0f,  6.0f,   6.0f,   10.0f, 6.0f,  6.0f,   15.0f, 15.0f, 9.0f,  7.0f,  6.0f,  6.0f,  7.0f,   8.0f,
	    3.0f,  3.0f,  6.0f,   7.0f,   13.0f, 6.0f,  6.0f,   9.0f,  5.0f,  7.0f,  9.0f,  6.0f,  6.0f,  8.0f,   6.0f,
	    6.0f,  5.5f,  6.0f,   4.0f,   25.0f, 6.0f,  6.0f,   6.0f,  22.0f, 20.0f, 6.0f,  10.0f, 13.5f, 12.0f,  3.0f,
	    30.0f, 6.0f,  6.0f,   35.0f,  1.5f,  8.0f,  3.0f,   6.0f,  2.0f,  6.0f,  6.0f,  5.0f,  2.0f,  7.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   4.0f,   6.0f,  6.0f,  6.0f,   8.0f,  8.0f,  7.0f,  8.0f,  6.0f,  7.0f,  6.0f,   7.0f,
	    6.0f,  10.0f, 3.0f,   6.0f,   8.0f,  9.0f,  15.0f,  5.0f,  10.0f, 7.0f,  6.0f,  7.0f,  6.0f,  7.0f,   7.0f,
	    12.0f, 6.0f,  4.0f,   6.0f,   5.0f,  3.0f,  30.0f,  30.0f, 15.0f, 20.0f, 6.0f,  10.0f, 6.0f,  14.0f,  14.0f,
	    16.0f, 15.0f, 30.0f,  15.0f,  7.5f,  5.0f,  4.0f,   6.0f,  15.0f, 6.5f,  3.0f,  12.0f, 10.0f, 10.5f,  10.0f,
	    7.5f,  6.0f,  6.0f,   12.5f,  9.0f,  20.0f, 2.0f,   10.0f, 25.0f, 8.0f,  6.0f,  6.0f,  10.0f, 18.0f,  45.0f,
	    13.0f, 15.0f, 8.0f,   30.0f,  25.0f, 25.0f, 10.0f,  13.0f, 5.0f,  3.5f,  15.0f, 35.0f, 11.0f, 15.0f,  50.0f,
	    13.0f, 6.0f,  7.0f,   6.0f,   60.0f, 6.0f,  22.0f,  22.0f, 21.0f, 22.0f, 15.0f, 25.0f, 23.0f, 8.0f,   15.0f,
	    10.0f, 6.0f,  7.0f,   6.0f,   12.0f, 9.5f,  6.0f,   12.0f, 12.0f, 12.0f, 15.0f, 4.0f,  5.0f,  105.0f, 20.0f,
	    5.0f,  10.0f, 10.0f,  10.0f,  20.0f, 13.5f, 8.0f,   10.0f, 3.0f,  5.0f,  9.0f,  6.0f,  6.0f,  6.0f,   10.0f,
	    8.0f,  8.0f,  8.0f,   6.0f,   6.0f,  5.0f,  5.0f,   5.0f,  9.0f,  9.0f,  9.0f,  6.0f,  8.5f,  6.0f,   7.0f,
	    8.0f,  7.0f,  11.0f,  6.0f,   7.0f,  9.0f,  8.0f,   6.0f,  8.0f,  6.0f,  6.0f,  6.0f,  6.0f,  9.0f,   10.0f,
	    6.0f,  3.0f,  4.0f,   3.0f,   3.0f,  4.0f,  10.0f,  10.0f, 2.0f,  8.0f,  6.0f,  6.0f,  14.0f, 7.0f,   5.0f,
	    9.0f,  7.0f,  7.0f,   10.0f,  10.0f, 12.0f, 9.0f,   7.0f,  12.0f, 13.0f, 16.0f, 6.0f,  9.0f,  6.0f,   6.0f,
	    10.0f, 25.0f, 15.0f,  6.0f,   25.0f, 6.0f,  6.0f,   8.0f,  11.0f, 6.0f,  9.0f,  2.0f,  6.0f,  5.0f,   4.0f,
	    8.5f,  6.0f,  6.0f,   6.0f,   4.0f,  6.0f,  15.0f,  1.0f,  2.0f,  6.0f,  40.0f, 8.0f,  12.0f, 3.0f,   8.0f,
	    99.0f, 9.0f,  100.0f, 100.0f, 10.0f, 6.0f,  27.5f,  20.0f, 6.0f,  6.0f,  5.0f,  6.0f,  8.0f,  5.0f,   3.0f,
	    11.5f, 25.0f, 80.0f,  20.0f,  9.0f,  8.0f,  5.0f,   4.0f,  7.0f,  10.0f, 6.0f,  11.0f, 8.0f,  5.0f,   6.0f,
	    6.0f,  30.0f, 7.0f,   15.0f,  9.0f,  6.0f,  9.0f,   6.0f,  3.0f,  32.5f, 15.0f, 7.5f,  10.0f, 10.0f,  6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  9.0f,  20.0f,  6.0f,  6.0f,  6.0f,  25.0f, 12.0f, 6.0f,  8.0f,   6.0f,
	    6.0f,  20.0f, 9.0f,   8.0f,   12.0f, 8.0f,  2.0f,   6.0f,  3.0f,  6.0f,  7.0f,  1.5f,  6.0f,  3.0f,   3.0f,
	    3.0f,  3.0f,  2.0f,   3.0f,   3.0f,  6.0f,  6.0f,   6.0f,  4.5f,  7.0f,  6.0f,  7.0f,  5.7f,  22.0f,  8.0f,
	    15.0f, 22.0f, 8.0f,   15.0f,  6.0f,  80.0f, 150.0f, 7.0f,  6.0f,  6.0f,  6.0f,  12.0f, 6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   35.0f, 20.0f, 9.0f,  6.0f,  6.0f,  6.0f,  20.0f,  20.0f,
	    20.0f, 20.0f, 20.0f,  9.0f,   4.0f,  4.0f,  10.0f,  5.0f,  8.0f,  6.0f,  10.0f, 5.7f,  6.0f,  2.0f,   36.0f,
	    14.0f, 7.0f,  250.0f, 6.0f,   9.0f,  6.0f,  7.0f,   4.0f,  6.0f,  8.0f,  6.0f,  23.0f, 6.0f,  6.0f,   6.0f,
	    70.0f, 6.0f,  7.0f,   6.0f,   6.0f,  6.0f,  20.0f,  6.0f,  6.0f,  6.0f,  5.0f,  1.0f,  6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,   6.0f,
	    4.0f,  4.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  10.0f, 6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  6.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,   6.0f,
	    6.0f,  6.0f,  6.0f,   6.0f,   6.0f,  7.0f,  7.0f,   7.0f,  7.0f,  6.0f,  6.0f,  6.0f,  6.0f,  6.0f,   8.0f,
	    6.0f,  6.0f,  6.0f,   7.0f,   6.0f,  6.0f,  6.0f,   7.5f,  6.0f,  6.0f,  4.0f,  6.0f,  3.0f,  6.0f,   6.0f,
	    1.0f,  9.0f,  7.0f,   8.0f,   7.0f,  8.0f,  6.0f,   6.0f,  6.0f,  6.0f,  6.0f,  8.0f,
	};

	const auto size = sizeof(male_height) / sizeof(male_height[0]);

	if (race >= size) {
		return 6.0f;
	}

	return gender == Gender::Male ? male_height[race] : female_height[race];
}

// PlayerAppearance prep
#define HUMAN_MALE ((Race::Human << 8) | Gender::Male)
#define HUMAN_FEMALE ((Race::Human << 8) | Gender::Female)
#define BARBARIAN_MALE ((Race::Barbarian << 8) | Gender::Male)
#define BARBARIAN_FEMALE ((Race::Barbarian << 8) | Gender::Female)
#define ERUDITE_MALE ((Race::Erudite << 8) | Gender::Male)
#define ERUDITE_FEMALE ((Race::Erudite << 8) | Gender::Female)
#define WOOD_ELF_MALE ((Race::WoodElf << 8) | Gender::Male)
#define WOOD_ELF_FEMALE ((Race::WoodElf << 8) | Gender::Female)
#define HIGH_ELF_MALE ((Race::HighElf << 8) | Gender::Male)
#define HIGH_ELF_FEMALE ((Race::HighElf << 8) | Gender::Female)
#define DARK_ELF_MALE ((Race::DarkElf << 8) | Gender::Male)
#define DARK_ELF_FEMALE ((Race::DarkElf << 8) | Gender::Female)
#define HALF_ELF_MALE ((Race::HalfElf << 8) | Gender::Male)
#define HALF_ELF_FEMALE ((Race::HalfElf << 8) | Gender::Female)
#define DWARF_MALE ((Race::Dwarf << 8) | Gender::Male)
#define DWARF_FEMALE ((Race::Dwarf << 8) | Gender::Female)
#define TROLL_MALE ((Race::Troll << 8) | Gender::Male)
#define TROLL_FEMALE ((Race::Troll << 8) | Gender::Female)
#define OGRE_MALE ((Race::Ogre << 8) | Gender::Male)
#define OGRE_FEMALE ((Race::Ogre << 8) | Gender::Female)
#define HALFLING_MALE ((Race::Halfling << 8) | Gender::Male)
#define HALFLING_FEMALE ((Race::Halfling << 8) | Gender::Female)
#define GNOME_MALE ((Race::Gnome << 8) | Gender::Male)
#define GNOME_FEMALE ((Race::Gnome << 8) | Gender::Female)
#define IKSAR_MALE ((Race::Iksar << 8) | Gender::Male)
#define IKSAR_FEMALE ((Race::Iksar << 8) | Gender::Female)
#define VAH_SHIR_MALE ((Race::VahShir << 8) | Gender::Male)
#define VAH_SHIR_FEMALE ((Race::VahShir << 8) | Gender::Female)
#define FROGLOK_MALE ((Race::Froglok << 8) | Gender::Male)
#define FROGLOK_FEMALE ((Race::Froglok << 8) | Gender::Female)
#define DRAKKIN_MALE ((Race::Drakkin << 8) | Gender::Male)
#define DRAKKIN_FEMALE ((Race::Drakkin << 8) | Gender::Female)

#define BINDRG(r, g) (((int)r << 8) | g)


bool PlayerAppearance::IsValidBeard(uint16 race_id, uint8 gender_id, uint8 beard_value, bool use_luclin)
{
	if (beard_value == UINT8_MAX) {
		return true;
	}

	if (use_luclin) {
		switch (BINDRG(race_id, gender_id)) {
			case DWARF_FEMALE:
				if (beard_value <= 1) {
					return true;
				}

				break;
			case HIGH_ELF_MALE:
			case DARK_ELF_MALE:
			case HALF_ELF_MALE:
			case DRAKKIN_FEMALE:
				if (beard_value <= 3) {
					return true;
				}

				break;
			case HUMAN_MALE:
			case BARBARIAN_MALE:
			case ERUDITE_MALE:
			case DWARF_MALE:
			case HALFLING_MALE:
			case GNOME_MALE:
				if (beard_value <= 5) {
					return true;
				}

				break;
			case DRAKKIN_MALE:
				if (beard_value <= 11) {
					return true;
				}

				break;
			default:
				break;
		}

		return false;
	}

	switch (BINDRG(race_id, gender_id)) {
		case DRAKKIN_FEMALE:
			if (beard_value <= 3) {
				return true;
			}

			break;
		case DRAKKIN_MALE:
			if (beard_value <= 11) {
				return true;
			}

			break;
		default:
			break;
	}

	return false;
}

bool PlayerAppearance::IsValidBeardColor(uint16 race_id, uint8 gender_id, uint8 beard_color_value, bool use_luclin)
{
	if (beard_color_value == UINT8_MAX) {
		return true;
	}

	switch (BINDRG(race_id, gender_id)) {
		case GNOME_MALE:
			if (beard_color_value <= 24) {
				return true;
			}

			break;
		case HUMAN_MALE:
		case BARBARIAN_MALE:
		case ERUDITE_MALE:
		case HALF_ELF_MALE:
		case DWARF_MALE:
		case DWARF_FEMALE:
		case HALFLING_MALE:
			if (beard_color_value <= 19) {
				return true;
			}

			break;
		case DARK_ELF_MALE:
			if (EQ::ValueWithin(beard_color_value, 13, 18)) {
				return true;
			}

			break;
		case HIGH_ELF_MALE:
			if (beard_color_value <= 14) {
				return true;
			}

			break;
		case FROGLOK_MALE:
		case FROGLOK_FEMALE:
		case DRAKKIN_MALE:
		case DRAKKIN_FEMALE:
			if (beard_color_value <= 3) {
				return true;
			}

			break;
		default:
			break;
	}

	return false;
}

bool PlayerAppearance::IsValidDetail(uint16 race_id, uint8 gender_id, uint32 detail_value, bool use_luclin)
{
	if (detail_value == UINT32_MAX) {
		return true;
	}

	switch (BINDRG(race_id, gender_id)) {
		case DRAKKIN_MALE:
		case DRAKKIN_FEMALE:
			if (detail_value <= 7) {
				return true;
			}

			break;
		default:
			break;
	}

	return false;
}

bool PlayerAppearance::IsValidEyeColor(uint16 race_id, uint8 gender_id, uint8 eye_color_value, bool use_luclin)
{
	switch (BINDRG(race_id, gender_id)) {
		case HUMAN_MALE:
		case HUMAN_FEMALE:
		case BARBARIAN_MALE:
		case BARBARIAN_FEMALE:
		case ERUDITE_MALE:
		case ERUDITE_FEMALE:
		case WOOD_ELF_MALE:
		case WOOD_ELF_FEMALE:
		case HIGH_ELF_MALE:
		case HIGH_ELF_FEMALE:
		case DARK_ELF_MALE:
		case DARK_ELF_FEMALE:
		case HALF_ELF_MALE:
		case HALF_ELF_FEMALE:
		case DWARF_MALE:
		case DWARF_FEMALE:
		case OGRE_MALE:
		case OGRE_FEMALE:
		case HALFLING_MALE:
		case HALFLING_FEMALE:
		case GNOME_MALE:
		case GNOME_FEMALE:
		case IKSAR_MALE:
		case IKSAR_FEMALE:
		case VAH_SHIR_MALE:
		case VAH_SHIR_FEMALE:
			if (eye_color_value <= 9) {
				return true;
			}

			break;
		case TROLL_MALE:
		case TROLL_FEMALE:
			if (eye_color_value <= 10) {
				return true;
			}

			break;
		case FROGLOK_MALE:
		case FROGLOK_FEMALE:
		case DRAKKIN_MALE:
		case DRAKKIN_FEMALE:
			if (eye_color_value <= 11) {
				return true;
			}

			break;
		default:
			break;
	}

	return false;
}

bool PlayerAppearance::IsValidFace(uint16 race_id, uint8 gender_id, uint8 face_value, bool use_luclin)
{
	if (face_value == UINT8_MAX) {
		return true;
	}

	switch (BINDRG(race_id, gender_id)) {
		case DRAKKIN_MALE:
		case DRAKKIN_FEMALE:
			if (face_value <= 6) {
				return true;
			}

			break;
		case HUMAN_MALE:
		case HUMAN_FEMALE:
		case BARBARIAN_MALE:
		case BARBARIAN_FEMALE:
		case ERUDITE_MALE:
		case ERUDITE_FEMALE:
		case WOOD_ELF_MALE:
		case WOOD_ELF_FEMALE:
		case HIGH_ELF_MALE:
		case HIGH_ELF_FEMALE:
		case DARK_ELF_MALE:
		case DARK_ELF_FEMALE:
		case HALF_ELF_MALE:
		case HALF_ELF_FEMALE:
		case DWARF_MALE:
		case DWARF_FEMALE:
		case TROLL_MALE:
		case TROLL_FEMALE:
		case OGRE_MALE:
		case OGRE_FEMALE:
		case HALFLING_MALE:
		case HALFLING_FEMALE:
		case GNOME_MALE:
		case GNOME_FEMALE:
		case IKSAR_MALE:
		case IKSAR_FEMALE:
		case VAH_SHIR_MALE:
		case VAH_SHIR_FEMALE:
			if (face_value <= 7) {
				return true;
			}

			break;
		case FROGLOK_MALE:
		case FROGLOK_FEMALE:
			if (face_value <= 9) {
				return true;
			}

			break;
		default:
			break;
	}

	return false;
}

bool PlayerAppearance::IsValidHair(uint16 race_id, uint8 gender_id, uint8 hair_value, bool use_luclin)
{
	if (hair_value == UINT8_MAX) {
		return true;
	}

	if (use_luclin) {
		switch (BINDRG(race_id, gender_id)) {
			case HUMAN_MALE:
			case HUMAN_FEMALE:
			case BARBARIAN_MALE:
			case BARBARIAN_FEMALE:
			case WOOD_ELF_MALE:
			case WOOD_ELF_FEMALE:
			case HIGH_ELF_MALE:
			case HIGH_ELF_FEMALE:
			case DARK_ELF_MALE:
			case DARK_ELF_FEMALE:
			case HALF_ELF_MALE:
			case HALF_ELF_FEMALE:
			case DWARF_MALE:
			case DWARF_FEMALE:
			case TROLL_FEMALE:
			case OGRE_FEMALE:
			case HALFLING_MALE:
			case HALFLING_FEMALE:
			case GNOME_MALE:
			case GNOME_FEMALE:
				if (hair_value <= 3) {
					return true;
				}

				break;
			case ERUDITE_MALE:
				if (hair_value <= 5) {
					return true;
				}

				break;
			case DRAKKIN_FEMALE:
				if (hair_value <= 7) {
					return true;
				}

				break;
			case ERUDITE_FEMALE:
			case DRAKKIN_MALE:
				if (hair_value <= 8) {
					return true;
				}

				break;
			default:
				break;
		}

		return false;
	}

	switch (BINDRG(race_id, gender_id)) {
		case DRAKKIN_FEMALE:
			if (hair_value <= 7) {
				return true;
			}

			break;
		case DRAKKIN_MALE:
			if (hair_value <= 8) {
				return true;
			}

			break;
		default:
			break;
	}
	return false;
}

bool PlayerAppearance::IsValidHairColor(uint16 race_id, uint8 gender_id, uint8 hair_color_value, bool use_luclin)
{
	if (hair_color_value == UINT8_MAX) {
		return true;
	}

	switch (BINDRG(race_id, gender_id)) {
		case GNOME_MALE:
		case GNOME_FEMALE:
			if (hair_color_value <= 24) {
				return true;
			}

			break;
		case TROLL_FEMALE:
		case OGRE_FEMALE:
			if (hair_color_value <= 23) {
				return true;
			}

			break;
		case HUMAN_MALE:
		case HUMAN_FEMALE:
		case BARBARIAN_MALE:
		case BARBARIAN_FEMALE:
		case WOOD_ELF_MALE:
		case WOOD_ELF_FEMALE:
		case HALF_ELF_MALE:
		case HALF_ELF_FEMALE:
		case DWARF_MALE:
		case DWARF_FEMALE:
		case HALFLING_MALE:
		case HALFLING_FEMALE:
			if (hair_color_value <= 19) {
				return true;
			}

			break;
		case DARK_ELF_MALE:
		case DARK_ELF_FEMALE:
			if (EQ::ValueWithin(hair_color_value, 13, 18)) {
				return true;
			}

			break;
		case HIGH_ELF_MALE:
		case HIGH_ELF_FEMALE:
			if (hair_color_value <= 14) {
				return true;
			}

			break;
		case FROGLOK_MALE:
		case FROGLOK_FEMALE:
		case DRAKKIN_MALE:
		case DRAKKIN_FEMALE:
			if (hair_color_value <= 3) {
				return true;
			}

			break;
		default:
			break;
	}

	return false;
}

bool PlayerAppearance::IsValidHead(uint16 race_id, uint8 gender_id, uint8 head_value, bool use_luclin)
{
	if (head_value == UINT8_MAX) {
		return true;
	}

	if (use_luclin) {
		switch (BINDRG(race_id, gender_id)) {
			case HUMAN_MALE:
			case HUMAN_FEMALE:
			case BARBARIAN_MALE:
			case BARBARIAN_FEMALE:
			case WOOD_ELF_MALE:
			case WOOD_ELF_FEMALE:
			case HIGH_ELF_MALE:
			case HIGH_ELF_FEMALE:
			case DARK_ELF_MALE:
			case DARK_ELF_FEMALE:
			case HALF_ELF_MALE:
			case HALF_ELF_FEMALE:
			case DWARF_MALE:
			case DWARF_FEMALE:
			case TROLL_MALE:
			case TROLL_FEMALE:
			case OGRE_MALE:
			case OGRE_FEMALE:
			case HALFLING_MALE:
			case HALFLING_FEMALE:
			case GNOME_MALE:
			case GNOME_FEMALE:
			case IKSAR_MALE:
			case IKSAR_FEMALE:
			case VAH_SHIR_MALE:
			case VAH_SHIR_FEMALE:
			case FROGLOK_MALE:
			case FROGLOK_FEMALE:
			case DRAKKIN_MALE:
			case DRAKKIN_FEMALE:
				if (head_value <= 3) {
					return true;
				}

				break;
			case ERUDITE_MALE:
			case ERUDITE_FEMALE:
				if (head_value <= 4) {
					return true;
				}

				break;
			default:
				break;
		}
		return false;
	}

	switch (BINDRG(race_id, gender_id)) {
		case HUMAN_MALE:
		case HUMAN_FEMALE:
		case BARBARIAN_MALE:
		case BARBARIAN_FEMALE:
		case ERUDITE_MALE:
		case ERUDITE_FEMALE:
		case WOOD_ELF_MALE:
		case WOOD_ELF_FEMALE:
		case HIGH_ELF_MALE:
		case HIGH_ELF_FEMALE:
		case DARK_ELF_MALE:
		case DARK_ELF_FEMALE:
		case HALF_ELF_MALE:
		case HALF_ELF_FEMALE:
		case DWARF_MALE:
		case DWARF_FEMALE:
		case TROLL_MALE:
		case TROLL_FEMALE:
		case OGRE_MALE:
		case OGRE_FEMALE:
		case HALFLING_MALE:
		case HALFLING_FEMALE:
		case IKSAR_MALE:
		case IKSAR_FEMALE:
		case VAH_SHIR_MALE:
		case VAH_SHIR_FEMALE:
		case FROGLOK_MALE:
		case FROGLOK_FEMALE:
		case DRAKKIN_MALE:
		case DRAKKIN_FEMALE:
			if (head_value <= 3) {
				return true;
			}

			break;
		case GNOME_MALE:
		case GNOME_FEMALE:
			if (head_value <= 4) {
				return true;
			}

			break;
		default:
			break;
	}

	return false;
}

bool PlayerAppearance::IsValidHeritage(uint16 race_id, uint8 gender_id, uint32 heritage_value, bool use_luclin)
{
	if (heritage_value == UINT32_MAX) {
		return true;
	}

	switch (BINDRG(race_id, gender_id)) {
		case DRAKKIN_MALE:
		case DRAKKIN_FEMALE:
			if (heritage_value <= 7) {
				// > 5 seems to jumble other features..else, some heritages have 'specialized' features
				return true;
			}
			break;
		default:
			break;
	}

	return false;
}

bool PlayerAppearance::IsValidTattoo(uint16 race_id, uint8 gender_id, uint32 tattoo_value, bool use_luclin)
{
	if (tattoo_value == UINT32_MAX) {
		return true;
	}

	switch (BINDRG(race_id, gender_id)) {
		case DRAKKIN_MALE:
		case DRAKKIN_FEMALE:
			if (tattoo_value <= 7) {
				return true;
			}

			break;
		default:
			break;
	}

	return false;
}

bool PlayerAppearance::IsValidTexture(uint16 race_id, uint8 gender_id, uint8 texture_value, bool use_luclin)
{
	if (texture_value == UINT8_MAX) {
		return true;
	}

	if (use_luclin) {
		switch (BINDRG(race_id, gender_id)) {
			case HUMAN_MALE:
			case HUMAN_FEMALE:
			case IKSAR_MALE:
			case IKSAR_FEMALE:
			case DRAKKIN_MALE:
			case DRAKKIN_FEMALE:
				if (EQ::ValueWithin(texture_value, 10, 16) || texture_value <= 4) {
					return true;
				}

				break;
			case ERUDITE_MALE:
			case ERUDITE_FEMALE:
			case HIGH_ELF_MALE:
			case HIGH_ELF_FEMALE:
			case DARK_ELF_MALE:
			case DARK_ELF_FEMALE:
			case GNOME_MALE:
			case GNOME_FEMALE:
			case FROGLOK_MALE:
			case FROGLOK_FEMALE:
				if (EQ::ValueWithin(texture_value, 10, 16) || texture_value <= 3) {
					return true;
				}

				break;
			case BARBARIAN_MALE:
			case BARBARIAN_FEMALE:
			case WOOD_ELF_MALE:
			case WOOD_ELF_FEMALE:
			case HALF_ELF_MALE:
			case HALF_ELF_FEMALE:
			case DWARF_MALE:
			case DWARF_FEMALE:
			case TROLL_MALE:
			case TROLL_FEMALE:
			case OGRE_MALE:
			case OGRE_FEMALE:
			case HALFLING_MALE:
			case HALFLING_FEMALE:
			case VAH_SHIR_MALE:
			case VAH_SHIR_FEMALE:
				if (texture_value <= 3) {
					return true;
				}

				break;
			default:
				break;
		}

		return false;
	}

	switch (BINDRG(race_id, gender_id)) {
		case HUMAN_MALE:
		case HUMAN_FEMALE:
		case ERUDITE_MALE:
		case ERUDITE_FEMALE:
		case DRAKKIN_MALE:
		case DRAKKIN_FEMALE:
			if (EQ::ValueWithin(texture_value, 10, 16) || texture_value <= 4) {
				return true;
			}

			break;
		case HIGH_ELF_MALE:
		case HIGH_ELF_FEMALE:
		case DARK_ELF_MALE:
		case DARK_ELF_FEMALE:
		case GNOME_MALE:
		case GNOME_FEMALE:
		case FROGLOK_MALE:
		case FROGLOK_FEMALE:
			if (EQ::ValueWithin(texture_value, 10, 16) || texture_value <= 3) {
				return true;
			}

			break;
		case VAH_SHIR_MALE:
		case VAH_SHIR_FEMALE:
			if (texture_value == 50 || texture_value <= 3) {
				return true;
			}

			break;
		case IKSAR_MALE:
		case IKSAR_FEMALE:
			if (texture_value == 10 || texture_value <= 4) {
				return true;
			}

			break;
		case BARBARIAN_MALE:
		case BARBARIAN_FEMALE:
		case WOOD_ELF_MALE:
		case WOOD_ELF_FEMALE:
		case HALF_ELF_MALE:
		case HALF_ELF_FEMALE:
		case DWARF_MALE:
		case DWARF_FEMALE:
		case TROLL_MALE:
		case TROLL_FEMALE:
		case OGRE_MALE:
		case OGRE_FEMALE:
		case HALFLING_MALE:
		case HALFLING_FEMALE:
			if (texture_value <= 3) {
				return true;
			}

			break;
		default:
			break;
	}

	return false;
}

bool PlayerAppearance::IsValidWoad(uint16 race_id, uint8 gender_id, uint8 woad_value, bool use_luclin)
{
	if (woad_value == UINT8_MAX) {
		return true;
	}

	if (use_luclin) {
		switch (BINDRG(race_id, gender_id)) {
			case BARBARIAN_MALE:
			case BARBARIAN_FEMALE:
				if (woad_value <= 8) {
					return true;
				}

				break;
			default:
				break;
		}
	}

	return false;
}

const char* GetGenderName(uint8 gender_id) {
	if (!EQ::ValueWithin(gender_id, Gender::Male, Gender::Neuter)) {
		return "Unknown";
	}

	return gender_names[gender_id].c_str();
}

const std::string GetPlayerRaceAbbreviation(uint16 race_id)
{
	if (!IsPlayerRace(race_id)) {
		return std::string("UNK");
	}

	switch (race_id) {
		case Race::Human:
			return "HUM";
		case Race::Barbarian:
			return "BAR";
		case Race::Erudite:
			return "ERU";
		case Race::WoodElf:
			return "ELF";
		case Race::HighElf:
			return "HIE";
		case Race::DarkElf:
			return "DEF";
		case Race::HalfElf:
			return "HEF";
		case Race::Dwarf:
			return "DWF";
		case Race::Troll:
			return "TRL";
		case Race::Ogre:
			return "OGR";
		case Race::Halfling:
			return "HFL";
		case Race::Gnome:
			return "GNM";
		case Race::Iksar:
			return "IKS";
		case Race::VahShir:
			return "VAH";
		case Race::Froglok:
			return "FRG";
		case Race::Drakkin:
			return "DRK";
	}

	return std::string("UNK");
}

bool IsPlayerRace(uint16 race_id) {
	return (
		EQ::ValueWithin(race_id, Race::Human, Race::Gnome) ||
		race_id == Race::Iksar ||
		race_id == Race::VahShir ||
		race_id == Race::Froglok ||
		race_id == Race::Drakkin
	);
}
