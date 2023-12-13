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
		case Races::Human:
		case Races::HighpassCitizen:
		case Races::QeynosCitizen:
		case Races::Human2:
			return "Human";
		case Races::Barbarian:
		case Races::HalasCitizen:
			return "Barbarian";
		case Races::Erudite:
		case Races::EruditeCitizen:
		case Races::Erudite2:
			return "Erudite";
		case Races::WoodElf:
			return "Wood Elf";
		case Races::HighElf:
			return "High Elf";
		case Races::DarkElf:
		case Races::NeriakCitizen:
			return "Dark Elf";
		case Races::HalfElf:
			return "Half Elf";
		case Races::Dwarf:
		case Races::KaladimCitizen:
			return "Dwarf";
		case Races::Troll:
		case Races::GrobbCitizen:
		case Races::TrollCrewMember:
		case Races::PirateDeckhand:
		case Races::BrokenSkullPirate:
			return "Troll";
		case Races::Ogre:
		case Races::OggokCitizen:
		case Races::Ogre2:
			return "Ogre";
		case Races::Halfling:
		case Races::RivervaleCitizen:
			return "Halfling";
		case Races::Gnome:
			return "Gnome";
		case Races::Aviak:
		case Races::Aviak2:
			return "Aviak";
		case Races::Werewolf:
		case Races::Werewolf2:
		case Races::Werewolf3:
			return "Werewolf";
		case Races::Brownie:
		case Races::Brownie2:
			return "Brownie";
		case Races::Centaur:
		case Races::Centaur2:
			return "Centaur";
		case Races::Golem:
		case Races::Golem2:
			return "Golem";
		case Races::Giant:
		case Races::ForestGiant:
		case Races::FrostGiant:
		case Races::StormGiant:
		case Races::EarthGolem:
		case Races::IronGolem:
		case Races::StormGolem:
		case Races::AirGolem:
		case Races::WoodGolem:
		case Races::FireGolem:
		case Races::WaterGolem:
		case Races::Giant2:
		case Races::Giant3:
		case Races::Giant4:
			return "Giant";
		case Races::Trakanon:
			return "Trakanon";
		case Races::VenrilSathir:
			return "Venril Sathir";
		case Races::EvilEye:
		case Races::EvilEye2:
		case Races::EvilEye3:
			return "Evil Eye";
		case Races::Beetle:
		case Races::Beetle2:
		case Races::Beetle3:
			return "Beetle";
		case Races::Kerran:
		case Races::Kerran2:
			return "Kerran";
		case Races::Fish:
		case Races::KunarkFish:
			return "Fish";
		case Races::Fairy:
		case Races::Fairy2:
			return "Fairy";
		case Races::Froglok:
		case Races::FroglokGhoul:
		case Races::Froglok2:
			return "Froglok";
		case Races::Fungusman:
			return "Fungusman";
		case Races::Gargoyle:
		case Races::Gargoyle2:
			return "Gargoyle";
		case Races::Gasbag:
			return "Gasbag";
		case Races::GelatinousCube:
		case Races::GelatinousCube2:
			return "Gelatinous Cube";
		case Races::Ghost:
		case Races::DwarfGhost:
		case Races::EruditeGhost:
		case Races::PirateGhost:
			return "Ghost";
		case Races::Ghoul:
		case Races::Ghoul2:
			return "Ghoul";
		case Races::GiantBat:
		case Races::Bat:
		case Races::Bat2:
			return "Bat";
		case Races::GiantEel:
			return "Eel";
		case Races::GiantRat:
		case Races::Rat:
			return "Rat";
		case Races::GiantSnake:
		case Races::Snake:
			return "Snake";
		case Races::GiantSpider:
		case Races::Spider:
			return "Spider";
		case Races::Gnoll:
		case Races::Gnoll2:
		case Races::Gnoll3:
			return "Gnoll";
		case Races::Goblin:
		case Races::Bloodgill:
		case Races::KunarkGoblin:
		case Races::NewGoblin:
		case Races::Goblin2:
			return "Goblin";
		case Races::Gorilla:
		case Races::Gorilla2:
			return "Gorilla";
		case Races::Wolf:
		case Races::WolfElemental:
		case Races::Wolf2:
			return "Wolf";
		case Races::Bear:
		case Races::Bear2:
		case Races::Bear3:
			return "Bear";
		case Races::FreeportGuard:
		case Races::Felguard:
		case Races::Fayguard:
			return "Guard";
		case Races::DemiLich:
			return "Demi Lich";
		case Races::Imp:
			return "Imp";
		case Races::Griffin:
		case Races::Griffin2:
			return "Griffin";
		case Races::Kobold:
		case Races::Kobold2:
			return "Kobold";
		case Races::LavaDragon:
		case Races::DragonSkeleton:
		case Races::WaterDragon:
		case Races::VeliousDragon:
		case Races::ClockworkDragon:
		case Races::BlackAndWhiteDragon:
		case Races::GhostDragon:
		case Races::PrismaticDragon:
		case Races::Dragon:
		case Races::Dragon2:
		case Races::Dragon3:
		case Races::Dragon4:
		case Races::Dragon5:
		case Races::Dragon6:
		case Races::Dragon7:
			return "Dragon";
		case Races::Lion:
			return "Lion";
		case Races::LizardMan:
			return "Lizard Man";
		case Races::Mimic:
			return "Mimic";
		case Races::Minotaur:
		case Races::Minotaur2:
		case Races::Minotaur3:
		case Races::Minotaur4:
			return "Minotaur";
		case Races::Orc:
		case Races::Orc2:
			return "Orc";
		case Races::HumanBeggar:
			return "Beggar";
		case Races::Pixie:
			return "Pixie";
		case Races::Drachnid:
		case Races::Drachnid2:
			return "Drachnid";
		case Races::SolusekRo:
		case Races::SolusekRo2:
			return "Solusek Ro";
		case Races::Skeleton:
		case Races::Skeleton2:
		case Races::Skeleton3:
			return "Skeleton";
		case Races::Shark:
			return "Shark";
		case Races::Tunare:
			return "Tunare";
		case Races::Tiger:
			return "Tiger";
		case Races::Treant:
		case Races::Treant2:
		case Races::Treant3:
			return "Treant";
		case Races::Vampire:
		case Races::ElfVampire:
		case Races::Vampire2:
		case Races::VampireVolatalis:
		case Races::UndeadVampire:
		case Races::Vampire3:
		case Races::MasterVampire:
		case Races::Vampire4:
			return "Vampire";
		case Races::StatueOfRallosZek:
		case Races::NewRallosZek:
			return "Rallos Zek";
		case Races::TentacleTerror:
		case Races::TentacleTerror2:
			return "Tentacle Terror";
		case Races::Wisp:
			return "Will-O-Wisp";
		case Races::Zombie:
		case Races::Zombie2:
			return "Zombie";
		case Races::Ship:
			return "Ship";
		case Races::Launch:
			return "Launch";
		case Races::Piranha:
			return "Piranha";
		case Races::Elemental:
			return "Elemental";
		case Races::Puma:
		case Races::Puma2:
		case Races::Puma3:
			return "Puma";
		case Races::Bixie:
		case Races::Bixie2:
			return "Bixie";
		case Races::ReanimatedHand:
			return "Reanimated Hand";
		case Races::Scarecrow:
		case Races::Scarecrow2:
			return "Scarecrow";
		case Races::Skunk:
			return "Skunk";
		case Races::SnakeElemental:
			return "Snake Elemental";
		case Races::Spectre:
		case Races::Spectre2:
			return "Spectre";
		case Races::Sphinx:
		case Races::Sphinx2:
			return "Sphinx";
		case Races::Armadillo:
			return "Armadillo";
		case Races::ClockworkGnome:
			return "Clockwork Gnome";
		case Races::Drake:
		case Races::Drake2:
		case Races::Drake3:
			return "Drake";
		case Races::Alligator:
		case Races::Alligator2:
			return "Alligator";
		case Races::CazicThule:
		case Races::CazicThule2:
			return "Cazic Thule";
		case Races::Cockatrice:
			return "Cockatrice";
		case Races::DaisyMan:
			return "Daisy Man";
		case Races::Denizen:
		case Races::Amygdalan:
			return "Amygdalan";
		case Races::Dervish:
		case Races::Dervish2:
		case Races::Dervish3:
		case Races::Dervish4:
		case Races::Dervish5:
		case Races::Dervish6:
			return "Dervish";
		case Races::Efreeti:
		case Races::Efreeti2:
			return "Efreeti";
		case Races::FroglokTadpole:
			return "Tadpole";
		case Races::PhinigelAutropos:
		case Races::Kedge:
			return "Kedge";
		case Races::Leech:
			return "Leech";
		case Races::Swordfish:
			return "Swordfish";
		case Races::Mammoth:
		case Races::Mammoth2:
			return "Mammoth";
		case Races::EyeOfZomm:
			return "Eye";
		case Races::Wasp:
			return "Wasp";
		case Races::Mermaid:
			return "Mermaid";
		case Races::Harpy:
		case Races::Harpy2:
			return "Harpy";
		case Races::Drixie:
			return "Drixie";
		case Races::GhostShip:
		case Races::GhostShip2:
			return "Ghost Ship";
		case Races::Clam:
			return "Clam";
		case Races::SeaHorse:
			return "Seahorse";
		case Races::Sabertooth:
			return "Saber-toothed Cat";
		case Races::Gorgon:
			return "Gorgon";
		case Races::Innoruuk:
			return "Innoruuk";
		case Races::Unicorn:
		case Races::Unicorn2:
		case Races::Unicorn3:
			return "Unicorn";
		case Races::Pegasus:
		case Races::Pegasus2:
		case Races::Pegasus3:
			return "Pegasus";
		case Races::Djinn:
			return "Djinn";
		case Races::InvisibleMan:
		case Races::InvisibleMan2:
		case Races::InvisibleMan3:
			return "Invisible Man";
		case Races::Iksar:
		case Races::IksarCitizen:
			return "Iksar";
		case Races::Scorpion:
		case Races::IksarScorpion:
		case Races::Scorpion2:
			return "Scorpion";
		case Races::VahShir:
		case Races::VahShirKing:
		case Races::VahShirGuard:
			return "Vah Shir";
		case Races::Sarnak:
		case Races::Sarnak2:
			return "Sarnak";
		case Races::Draglock:
			return "Draglok";
		case Races::Drolvarg:
			return "Drolvarg";
		case Races::Mosquito:
			return "Mosquito";
		case Races::Rhinoceros:
			return "Rhinoceros";
		case Races::Xalgoz:
			return "Xalgoz";
		case Races::Yeti:
			return "Yeti";
		case Races::Boat:
		case Races::Boat2:
			return "Boat";
		case Races::MinorIllusion:
			return "Minor Illusion";
		case Races::Tree:
			return "Tree";
		case Races::Burynai:
		case Races::Burynai2:
			return "Burynai";
		case Races::Goo:
		case Races::Goo2:
		case Races::Goo3:
		case Races::Goo4:
			return "Goo";
		case Races::SarnakSpirit:
			return "Sarnak Spirit";
		case Races::IksarSpirit:
			return "Iksar Spirit";
		case Races::Erollisi:
			return "Erollisi";
		case Races::Tribunal:
		case Races::TribunalNew:
			return "Tribunal";
		case Races::Bertoxxulous:
		case Races::BertoxxulousNew:
			return "Bertoxxulous";
		case Races::Bristlebane:
			return "Bristlebane";
		case Races::FayDrake:
			return "Fay Drake";
		case Races::UndeadSarnak:
			return "Undead Sarnak";
		case Races::Ratman:
		case Races::Ratman2:
			return "Ratman";
		case Races::Wyvern:
		case Races::Wyvern2:
			return "Wyvern";
		case Races::Wurm:
		case Races::Wurm2:
			return "Wurm";
		case Races::Devourer:
			return "Devourer";
		case Races::IksarGolem:
			return "Iksar Golem";
		case Races::UndeadIksar:
			return "Undead Iksar";
		case Races::ManEatingPlant:
			return "Man-Eating Plant";
		case Races::Raptor:
		case Races::Raptor2:
			return "Raptor";
		case Races::SarnakGolem:
			return "Sarnak Golem";
		case Races::AnimatedHand:
			return "Animated Hand";
		case Races::Succulent:
			return "Succulent";
		case Races::Holgresh:
			return "Holgresh";
		case Races::Brontotherium:
			return "Brontotherium";
		case Races::SnowDervish:
			return "Snow Dervish";
		case Races::DireWolf:
			return "Dire Wolf";
		case Races::Manticore:
			return "Manticore";
		case Races::Totem:
		case Races::Totem2:
			return "Totem";
		case Races::IceSpectre:
			return "Ice Spectre";
		case Races::EnchantedArmor:
			return "Enchanted Armor";
		case Races::SnowRabbit:
			return "Snow Rabbit";
		case Races::Walrus:
			return "Walrus";
		case Races::Geonid:
			return "Geonid";
		case Races::Yakkar:
			return "Yakkar";
		case Races::Faun:
			return "Faun";
		case Races::Coldain:
		case Races::Coldain2:
		case Races::Coldain3:
			return "Coldain";
		case Races::Hag:
			return "Hag";
		case Races::Hippogriff:
			return "Hippogriff";
		case Races::Siren:
		case Races::Siren2:
			return "Siren";
		case Races::Othmir:
			return "Othmir";
		case Races::Ulthork:
			return "Ulthork";
		case Races::Abhorrent:
			return "Abhorrent";
		case Races::SeaTurtle:
			return "Sea Turtle";
		case Races::RonnieTest:
			return "Ronnie Test";
		case Races::Shiknar:
			return "Shik'Nar";
		case Races::Rockhopper:
			return "Rockhopper";
		case Races::Underbulk:
			return "Underbulk";
		case Races::Grimling:
			return "Grimling";
		case Races::Worm:
			return "Worm";
		case Races::EvanTest:
			return "Evan Test";
		case Races::KhatiSha:
			return "Khati Sha";
		case Races::Owlbear:
			return "Owlbear";
		case Races::RhinoBeetle:
			return "Rhino Beetle";
		case Races::EarthElemental:
		case Races::EarthElemental2:
			return "Earth Elemental";
		case Races::AirElemental:
		case Races::AirElemental2:
			return "Air Elemental";
		case Races::WaterElemental:
		case Races::WaterElemental2:
			return "Water Elemental";
		case Races::FireElemental:
		case Races::FireElemental2:
			return "Fire Elemental";
		case Races::WetfangMinnow:
			return "Wetfang Minnow";
		case Races::ThoughtHorror:
			return "Thought Horror";
		case Races::Tegi:
			return "Tegi";
		case Races::HorseMount:
		case Races::HorseMount2:
		case Races::HorseMount3:
			return "Horse";
		case Races::Shissar:
		case Races::Shissar2:
			return "Shissar";
		case Races::FungalFiend:
			return "Fungal Fiend";
		case Races::Stonegrabber:
			return "Stonegrabber";
		case Races::ScarletCheetah:
			return "Cheetah";
		case Races::Zelniak:
			return "Zelniak";
		case Races::Lightcrawler:
			return "Lightcrawler";
		case Races::Shade:
		case Races::Shade2:
		case Races::Shade3:
			return "Shade";
		case Races::Sunflower:
			return "Sunflower";
		case Races::Shadel:
			return "Shadel";
		case Races::Shrieker:
			return "Shrieker";
		case Races::Galorian:
			return "Galorian";
		case Races::Netherbian:
			return "Netherbian";
		case Races::Akhevan:
			return "Akhevan";
		case Races::GriegVeneficus:
			return "Grieg Veneficus";
		case Races::SonicWolf:
			return "Sonic Wolf";
		case Races::GroundShaker:
			return "Ground Shaker";
		case Races::VahShirSkeleton:
			return "Vah Shir Skeleton";
		case Races::Wretch:
			return "Wretch";
		case Races::LordInquisitorSeru:
			return "Lord Inquisitor Seru";
		case Races::Recuso:
			return "Recuso";
		case Races::TeleportMan:
			return "Teleport Man";
		case Races::Nymph:
			return "Nymph";
		case Races::Dryad:
			return "Dryad";
		case Races::Fly:
			return "Fly";
		case Races::TarewMarr:
			return "Tarew Marr";
		case Races::ClockworkGolem:
			return "Clockwork Golem";
		case Races::ClockworkBrain:
			return "Clockwork Brain";
		case Races::Banshee:
		case Races::Banshee2:
		case Races::Banshee3:
			return "Banshee";
		case Races::GuardOfJustice:
			return "Guard of Justice";
		case Races::MiniPom:
			return "Mini POM";
		case Races::DiseasedFiend:
			return "Diseased Fiend";
		case Races::SolusekRoGuard:
			return "Solusek Ro Guard";
		case Races::TerrisThule:
			return "Terris-Thule";
		case Races::Vegerog:
			return "Vegerog";
		case Races::Crocodile:
			return "Crocodile";
		case Races::Hraquis:
			return "Hraquis";
		case Races::Tranquilion:
			return "Tranquilion";
		case Races::TinSoldier:
			return "Tin Soldier";
		case Races::NightmareWraith:
			return "Nightmare Wraith";
		case Races::Malarian:
			return "Malarian";
		case Races::KnightOfPestilence:
			return "Knight of Pestilence";
		case Races::Lepertoloth:
			return "Lepertoloth";
		case Races::Bubonian:
			return "Bubonian";
		case Races::BubonianUnderling:
			return "Bubonian Underling";
		case Races::Pusling:
			return "Pusling";
		case Races::WaterMephit:
			return "Water Mephit";
		case Races::Stormrider:
			return "Stormrider";
		case Races::JunkBeast:
			return "Junk Beast";
		case Races::BrokenClockwork:
			return "Broken Clockwork";
		case Races::GiantClockwork:
			return "Giant Clockwork";
		case Races::ClockworkBeetle:
			return "Clockwork Beetle";
		case Races::NightmareGoblin:
			return "Nightmare Goblin";
		case Races::Karana:
			return "Karana";
		case Races::BloodRaven:
			return "Blood Raven";
		case Races::NightmareGargoyle:
			return "Nightmare Gargoyle";
		case Races::MouthOfInsanity:
			return "Mouth of Insanity";
		case Races::SkeletalHorse:
			return "Skeletal Horse";
		case Races::Saryrn:
			return "Saryrn";
		case Races::FenninRo:
			return "Fennin Ro";
		case Races::Tormentor:
			return "Tormentor";
		case Races::SoulDevourer:
			return "Soul Devourer";
		case Races::Nightmare:
			return "Nightmare";
		case Races::VallonZek:
			return "Vallon Zek";
		case Races::TallonZek:
			return "Tallon Zek";
		case Races::AirMephit:
			return "Air Mephit";
		case Races::EarthMephit:
			return "Earth Mephit";
		case Races::FireMephit:
			return "Fire Mephit";
		case Races::NightmareMephit:
			return "Nightmare Mephit";
		case Races::Zebuxoruk:
			return "Zebuxoruk";
		case Races::MithanielMarr:
			return "Mithaniel Marr";
		case Races::UndeadKnight:
			return "Undead Knight";
		case Races::Rathe:
			return "The Rathe";
		case Races::Xegony:
			return "Xegony";
		case Races::Fiend:
			return "Fiend";
		case Races::TestObject:
			return "Test Object";
		case Races::Crab:
			return "Crab";
		case Races::Phoenix:
			return "Phoenix";
		case Races::Quarm:
			return "Quarm";
		case Races::WarWraith:
			return "War Wraith";
		case Races::Wrulon:
		case Races::Wrulon2:
			return "Wrulon";
		case Races::Kraken:
			return "Kraken";
		case Races::PoisonFrog:
			return "Poison Frog";
		case Races::Nilborien:
			return "Nilborien";
		case Races::Valorian:
		case Races::Valorian2:
			return "Valorian";
		case Races::WarBoar:
		case Races::WarBoar2:
			return "War Boar";
		case Races::AnimatedArmor:
			return "Animated Armor";
		case Races::UndeadFootman:
			return "Undead Footman";
		case Races::RallosOgre:
			return "Rallos Zek Minion";
		case Races::Arachnid:
			return "Arachnid";
		case Races::CrystalSpider:
			return "Crystal Spider";
		case Races::ZebuxoruksCage:
			return "Zebuxoruk's Cage";
		case Races::Portal:
			return "BoT Portal";
		case Races::OneArmedPirate:
		case Races::SpiritmasterNadox:
		case Races::BrokenSkullTaskmaster:
		case Races::GnomePirate:
		case Races::DarkElfPirate:
		case Races::OgrePirate:
		case Races::HumanPirate:
		case Races::EruditePirate:
			return "Pirate";
		case Races::Frog:
		case Races::Frog2:
			return "Frog";
		case Races::TrollZombie:
			return "Troll Zombie";
		case Races::Luggald:
		case Races::Luggald2:
		case Races::Luggald3:
			return "Luggald";
		case Races::Drogmor:
			return "Drogmor";
		case Races::FroglokSkeleton:
			return "Froglok Skeleton";
		case Races::UndeadFroglok:
			return "Undead Froglok";
		case Races::KnightOfHate:
			return "Knight of Hate";
		case Races::ArcanistOfHate:
			return "Arcanist of Hate";
		case Races::Veksar:
		case Races::Veksar2:
		case Races::Veksar3:
			return "Veksar";
		case Races::Chokidai:
			return "Chokidai";
		case Races::UndeadChokidai:
			return "Undead Chokidai";
		case Races::UndeadVeksar:
			return "Undead Veksar";
		case Races::RujarkianOrc:
		case Races::MasterOrc:
			return "Rujarkian Orc";
		case Races::BoneGolem:
		case Races::BoneGolem2:
			return "Bone Golem";
		case Races::Synarcana:
			return "Synarcana";
		case Races::SandElf:
			return "Sand Elf";
		case Races::Mummy:
			return "Mummy";
		case Races::Insect:
			return "Insect";
		case Races::FroglokGhost:
			return "Froglok Ghost";
		case Races::Box:
			return "Box";
		case Races::Barrel:
			return "Barrel";
		case Races::Chest:
		case Races::Chest2:
		case Races::Chest3:
			return "Chest";
		case Races::Vase:
			return "Vase";
		case Races::Table:
			return "Table";
		case Races::WeaponRack:
		case Races::WeaponRack2:
			return "Weapon Rack";
		case Races::Coffin:
		case Races::Coffin2:
			return "Coffin";
		case Races::Bones:
			return "Bones";
		case Races::Jokester:
			return "Jokester";
		case Races::Nihil:
			return "Nihil";
		case Races::Trusik:
			return "Trusik";
		case Races::StoneWorker:
		case Races::StoneWorker2:
			return "Stone Worker";
		case Races::Hynid:
			return "Hynid";
		case Races::Turepta:
			return "Turepta";
		case Races::Cragbeast:
			return "Cragbeast";
		case Races::Stonemite:
			return "Stonemite";
		case Races::Ukun:
			return "Ukun";
		case Races::Ixt:
			return "Ixt";
		case Races::Ikaav:
			return "Ikaav";
		case Races::Aneuk:
			return "Aneuk";
		case Races::Kyv:
			return "Kyv";
		case Races::Noc:
			return "Noc";
		case Races::Ratuk:
			return "Ra`tuk";
		case Races::Taneth:
			return "Taneth";
		case Races::Huvul:
			return "Huvul";
		case Races::Mutna:
			return "Mutna";
		case Races::Mastruq:
			return "Mastruq";
		case Races::Taelosian:
			return "Taelosian";
		case Races::DiscordShip:
			return "Discord Ship";
		case Races::MataMuram:
			return "Mata Muram";
		case Races::LightingWarrior:
			return "Lightning Warrior";
		case Races::Succubus:
			return "Succubus";
		case Races::Bazu:
			return "Bazu";
		case Races::Feran:
			return "Feran";
		case Races::Pyrilen:
			return "Pyrilen";
		case Races::Chimera:
		case Races::Chimera2:
			return "Chimera";
		case Races::Dragorn:
			return "Dragorn";
		case Races::Murkglider:
			return "Murkglider";
		case Races::Gelidran:
			return "Gelidran";
		case Races::Discordling:
			return "Discordling";
		case Races::Girplan:
			return "Girplan";
		case Races::DragornBox:
			return "Dragorn Box";
		case Races::RunedOrb:
			return "Runed Orb";
		case Races::DragonBones:
			return "Dragon Bones";
		case Races::MuramiteArmorPile:
			return "Muramite Armor Pile";
		case Races::CrystalShard:
			return "Crystal Shard";
		case Races::Portal2:
			return "Portal";
		case Races::CoinPurse:
			return "Coin Purse";
		case Races::RockPile:
			return "Rock Pile";
		case Races::MurkgliderEggSack:
			return "Murglider Egg Sack";
		case Races::Kirin:
		case Races::Kirin2:
			return "Kirin";
		case Races::Basilisk:
			return "Basilisk";
		case Races::SpiderQueen:
			return "Spider Queen";
		case Races::AnimatedStatue:
		case Races::AnimatedStatue2:
			return "Animated Statue";
		case Races::DragonEgg:
			return "Dragon Egg";
		case Races::DragonStatue:
			return "Dragon Statue";
		case Races::LavaRock:
			return "Lava Rock";
		case Races::SpiderEggSack:
			return "Spider Egg Sack";
		case Races::LavaSpider:
			return "Lava Spider";
		case Races::LavaSpiderQueen:
			return "Lava Spider Queen";
		case Races::Sporali:
			return "Sporali";
		case Races::Gnomework:
			return "Gnomework";
		case Races::Corathus:
			return "Corathus";
		case Races::Coral:
			return "Coral";
		case Races::DrachnidCocoon:
			return "Drachnid Cocoon";
		case Races::FungusPatch:
			return "Fungus Patch";
		case Races::Witheran:
		case Races::Witheran2:
			return "Witheran";
		case Races::DarkLord:
			return "Dark Lord";
		case Races::Shiliskin:
			return "Shiliskin";
		case Races::ClockworkBoar:
			return "Clockwork Boar";
		case Races::ScaledWolf:
			return "Scaled Wolf";
		case Races::SpiritWolf:
			return "Spirit Wolf";
		case Races::Bolvirk:
			return "Bolvirk";
		case Races::Elddar:
			return "Elddar";
		case Races::ForestGiant2:
			return "Forest Giant";
		case Races::ShamblingMound:
			return "Shambling Mound";
		case Races::Scrykin:
			return "Scrykin";
		case Races::AyonaeRo:
			return "Ayonae Ro";
		case Races::SullonZek:
			return "Sullon Zek";
		case Races::Banner:
		case Races::Banner2:
		case Races::Banner3:
		case Races::Banner4:
		case Races::Banner5:
		case Races::Banner6:
		case Races::Banner7:
			return "Banner";
		case Races::Flag:
			return "Flag";
		case Races::Rowboat:
			return "Rowboat";
		case Races::BearTrap:
			return "Bear Trap";
		case Races::ClockworkBomb:
			return "Clockwork Bomb";
		case Races::DynamiteKeg:
			return "Dynamite Keg";
		case Races::PressurePlate:
			return "Pressure Plate";
		case Races::PufferSpore:
			return "Puffer Spore";
		case Races::StoneRing:
			return "Stone Ring";
		case Races::RootTentacle:
			return "Root Tentacle";
		case Races::RunicSymbol:
			return "Runic Symbol";
		case Races::SaltpetterBomb:
			return "Saltpetter Bomb";
		case Races::FloatingSkull:
			return "Floating Skull";
		case Races::SpikeTrap:
			return "Spike Trap";
		case Races::Web:
			return "Web";
		case Races::WickerBasket:
			return "Wicker Basket";
		case Races::Drakkin:
			return "Drakkin";
		case Races::GiantShade:
			return "Giant Shade";
		case Races::Satyr:
			return "Satyr";
		case Races::Dynleth:
			return "Dyn'Leth";
		case Races::ArmorRack:
			return "Armor Rack";
		case Races::HoneyPot:
			return "Honey Pot";
		case Races::JumJumBucket:
			return "Jum Jum Bucket";
		case Races::Plant:
		case Races::Plant2:
			return "Plant";
		case Races::StoneJug:
		case Races::StoneJug2:
			return "Stone Jug";
		case Races::Toolbox:
			return "Toolbox";
		case Races::WineCask:
		case Races::WineCask2:
			return "Wine Cask";
		case Races::ElvenBoat:
			return "Elven Boat";
		case Races::GnomishBoat:
			return "Gnomish Boat";
		case Races::UndeadBoat:
			return "Undead Boat";
		case Races::MerchantShip:
			return "Merchant Ship";
		case Races::PirateShip:
			return "Pirate Ship";
		case Races::Campfire:
			return "Campfire";
		case Races::Exoskeleton:
			return "Exoskeleton";
		case Races::ClockworkGuardian:
			return "Clockwork Guardian";
		case Races::Rotocopter:
			return "Rotocopter";
		case Races::Wereorc:
			return "Wereorc";
		case Races::Worg:
		case Races::Worg2:
			return "Worg";
		case Races::Boulder:
			return "Boulder";
		case Races::ElvenGhost:
			return "Elven Ghost";
		case Races::HumanGhost:
			return "Human Ghost";
		case Races::Crystal:
			return "Crystal";
		case Races::GuardianCpu:
			return "Guardian CPU";
		case Races::Mansion:
			return "Mansion";
		case Races::FloatingIsland:
			return "Floating Island";
		case Races::Cragslither:
			return "Cragslither";
		case Races::SpellParticle:
			return "Spell Particle";
		case Races::InvisibleManOfZomm:
			return "Invisible Man of Zomm";
		case Races::RobocopterOfZomm:
			return "Robocopter of Zomm";
		case Races::Dracolich:
			return "Dracolich";
		case Races::IksarGhost:
			return "Iksar Ghost";
		case Races::IksarSkeleton:
			return "Iksar Skeleton";
		case Races::Mephit:
			return "Mephit";
		case Races::Muddite:
			return "Muddite";
		case Races::Tsetsian:
			return "Tsetsian";
		case Races::Nekhon:
			return "Nekhon";
		case Races::HydraCrystal:
			return "Hydra Crystal";
		case Races::CrystalSphere:
			return "Crystal Sphere";
		case Races::Sokokar:
			return "Sokokar";
		case Races::StonePylon:
			return "Stone Pylon";
		case Races::DemonVulture:
			return "Demon Vulture";
		case Races::Wagon:
			return "Wagon";
		case Races::GodOfDiscord:
			return "God of Discord";
		case Races::FeranMount:
			return "Feran Mount";
		case Races::SokokarMount:
		case Races::SokokarMount2:
			return "Sokokar Mount";
		case Races::TenthAnniversaryBanner:
			return "10th Anniversary Banner";
		case Races::TenthAnniversaryCake:
			return "10th Anniversary Cake";
		case Races::HydraMount:
			return "Hydra Mount";
		case Races::Hydra:
			return "Hydra";
		case Races::WeddingFlowers:
			return "Wedding Flowers";
		case Races::WeddingArbor:
			return "Wedding Arbor";
		case Races::WeddingAltar:
			return "Wedding Altar";
		case Races::PowderKeg:
			return "Powder Keg";
		case Races::Apexus:
			return "Apexus";
		case Races::Bellikos:
			return "Bellikos";
		case Races::BrellsFirstCreation:
			return "Brell's First Creation";
		case Races::Brell:
			return "Brell";
		case Races::CrystalskinAmbuloid:
			return "Crystalskin Ambuloud";
		case Races::CliknarQueen:
			return "Cliknar Queen";
		case Races::CliknarSoldier:
			return "Cliknar Soldier";
		case Races::CliknarWorker:
			return "Cliknar Worker";
		case Races::CrystalskinSessiloid:
			return "Crystalskin Sessiloid";
		case Races::Genari:
			return "Genari";
		case Races::Gigyn:
			return "Gigyn";
		case Races::GrekenYoungAdult:
			return "Greken - Young Adult";
		case Races::GrekenYoung:
			return "Greken - Young";
		case Races::CliknarMount:
			return "Cliknar Mount";
		case Races::Telmira:
			return "Telmira";
		case Races::SpiderMount:
			return "Spider Mount";
		case Races::BearMount:
			return "Bear Mount";
		case Races::RatMount:
			return "Rat Mount";
		case Races::SessiloidMount:
			return "Sessiloid Mount";
		case Races::MorellThule:
			return "Morell-Thule";
		case Races::Marionette:
			return "Marionette";
		case Races::BookDervish:
			return "Book Dervish";
		case Races::TopiaryLion:
			return "Topiary Lion";
		case Races::RotDog:
			return "Rot Dog";
		case Races::Sandman:
			return "Sandman";
		case Races::GrandfatherClock:
			return "Grandfather Clock";
		case Races::GingerbreadMan:
			return "Gingerbread Man";
		case Races::RoyalGuard:
			return "Royal Guard";
		case Races::Rabbit:
			return "Rabbit";
		case Races::BlindDreamer:
			return "Blind Dreamer";
		case Races::TopiaryLionMount:
			return "Topiary Lion Mount";
		case Races::RotDogMount:
			return "Rot Dog Mount";
		case Races::GoralMount:
			return "Goral Mount";
		case Races::SelyrahMount:
			return "Selyrah Mount";
		case Races::ScleraMount:
			return "Sclera Mount";
		case Races::BraxiMount:
			return "Braxi Mount";
		case Races::KangonMount:
			return "Kangon Mount";
		case Races::WurmMount:
			return "Wurm Mount";
		case Races::RaptorMount:
			return "Raptor Mount";
		case Races::Whirligig:
			return "Whirligig";
		case Races::GnomishBalloon:
			return "Gnomish Balloon";
		case Races::GnomishRocketPack:
			return "Gnomish Rocket Pack";
		case Races::GnomishHoveringTransport:
			return "Gnomish Hovering Transport";
		case Races::Selyrah:
			return "Selyrah";
		case Races::Goral:
			return "Goral";
		case Races::Braxi:
			return "Braxi";
		case Races::Kangon:
			return "Kangon";
		case Races::FloatingTower:
			return "Floating Tower";
		case Races::ExplosiveCart:
			return "Explosive Cart";
		case Races::BlimpShip:
			return "Blimp Ship";
		case Races::Tumbleweed:
			return "Tumbleweed";
		case Races::Alaran:
			return "Alaran";
		case Races::Swinetor:
			return "Swinetor";
		case Races::Triumvirate:
			return "Triumvirate";
		case Races::Hadal:
			return "Hadal";
		case Races::HoveringPlatform:
			return "Hovering Platform";
		case Races::ParasiticScavenger:
			return "Parasitic Scavenger";
		case Races::Grendlaen:
			return "Grendlaen";
		case Races::ShipInABottle:
			return "Ship in a Bottle";
		case Races::AlaranSentryStone:
			return "Alaran Sentry Stone";
		case Races::RegenerationPool:
			return "Regeneration Pool";
		case Races::TeleportationStand:
			return "Teleportation Stand";
		case Races::RelicCase:
			return "Relic Case";
		case Races::AlaranGhost:
			return "Alaran Ghost";
		case Races::Skystrider:
			return "Skystrider";
		case Races::WaterSpout:
			return "Water Spout";
		case Races::AviakPullAlong:
			return "Aviak Pull Along";
		case Races::Cat:
			return "Cat";
		case Races::ElkHead:
			return "Elk Head";
		case Races::Holgresh2:
			return "Holgresh";
		case Races::VineMaw:
			return "Vine Maw";
		case Races::FallenKnight:
			return "Fallen Knight";
		case Races::FlyingCarpet:
			return "Flying Carpet";
		case Races::CarrierHand:
			return "Carrier Hand";
		case Races::Akheva:
			return "Akheva";
		case Races::ServantOfShadow:
			return "Servant of Shadow";
		case Races::Luclin:
		case Races::Luclin2:
		case Races::Luclin3:
		case Races::Luclin4:
			return "Luclin";
		case Races::Xaric:
			return "Xaric";
		case Races::Orb:
			return "Orb";
		case Races::InteractiveObject:
			return "Interactive Object";
		case Races::Node:
			return "Node";
		case Races::Unknown:
		case Races::Unknown2:
		case Races::Unknown3:
		case Races::Unknown4:
		case Races::Unknown5:
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

	return gender == Genders::Male ? male_height[race] : female_height[race];
}

// PlayerAppearance prep
#define HUMAN_MALE ((Races::Human << 8) | Genders::Male)
#define HUMAN_FEMALE ((Races::Human << 8) | Genders::Female)
#define BARBARIAN_MALE ((Races::Barbarian << 8) | Genders::Male)
#define BARBARIAN_FEMALE ((Races::Barbarian << 8) | Genders::Female)
#define ERUDITE_MALE ((Races::Erudite << 8) | Genders::Male)
#define ERUDITE_FEMALE ((Races::Erudite << 8) | Genders::Female)
#define WOOD_ELF_MALE ((Races::WoodElf << 8) | Genders::Male)
#define WOOD_ELF_FEMALE ((Races::WoodElf << 8) | Genders::Female)
#define HIGH_ELF_MALE ((Races::HighElf << 8) | Genders::Male)
#define HIGH_ELF_FEMALE ((Races::HighElf << 8) | Genders::Female)
#define DARK_ELF_MALE ((Races::DarkElf << 8) | Genders::Male)
#define DARK_ELF_FEMALE ((Races::DarkElf << 8) | Genders::Female)
#define HALF_ELF_MALE ((Races::HalfElf << 8) | Genders::Male)
#define HALF_ELF_FEMALE ((Races::HalfElf << 8) | Genders::Female)
#define DWARF_MALE ((Races::Dwarf << 8) | Genders::Male)
#define DWARF_FEMALE ((Races::Dwarf << 8) | Genders::Female)
#define TROLL_MALE ((Races::Troll << 8) | Genders::Male)
#define TROLL_FEMALE ((Races::Troll << 8) | Genders::Female)
#define OGRE_MALE ((Races::Ogre << 8) | Genders::Male)
#define OGRE_FEMALE ((Races::Ogre << 8) | Genders::Female)
#define HALFLING_MALE ((Races::Halfling << 8) | Genders::Male)
#define HALFLING_FEMALE ((Races::Halfling << 8) | Genders::Female)
#define GNOME_MALE ((Races::Gnome << 8) | Genders::Male)
#define GNOME_FEMALE ((Races::Gnome << 8) | Genders::Female)
#define IKSAR_MALE ((Races::Iksar << 8) | Genders::Male)
#define IKSAR_FEMALE ((Races::Iksar << 8) | Genders::Female)
#define VAH_SHIR_MALE ((Races::VahShir << 8) | Genders::Male)
#define VAH_SHIR_FEMALE ((Races::VahShir << 8) | Genders::Female)
#define FROGLOK_MALE ((Races::Froglok << 8) | Genders::Male)
#define FROGLOK_FEMALE ((Races::Froglok << 8) | Genders::Female)
#define DRAKKIN_MALE ((Races::Drakkin << 8) | Genders::Male)
#define DRAKKIN_FEMALE ((Races::Drakkin << 8) | Genders::Female)

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
	if (!EQ::ValueWithin(gender_id, Genders::Male, Genders::Neuter)) {
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
		case Races::Human:
			return "HUM";
		case Races::Barbarian:
			return "BAR";
		case Races::Erudite:
			return "ERU";
		case Races::WoodElf:
			return "ELF";
		case Races::HighElf:
			return "HIE";
		case Races::DarkElf:
			return "DEF";
		case Races::HalfElf:
			return "HEF";
		case Races::Dwarf:
			return "DWF";
		case Races::Troll:
			return "TRL";
		case Races::Ogre:
			return "OGR";
		case Races::Halfling:
			return "HFL";
		case Races::Gnome:
			return "GNM";
		case Races::Iksar:
			return "IKS";
		case Races::VahShir:
			return "VAH";
		case Races::Froglok:
			return "FRG";
		case Races::Drakkin:
			return "DRK";
	}

	return std::string("UNK");
}

bool IsPlayerRace(uint16 race_id) {
	return (
		EQ::ValueWithin(race_id, Races::Human, Races::Gnome) ||
		race_id == Races::Iksar ||
		race_id == Races::VahShir ||
		race_id == Races::Froglok ||
		race_id == Races::Drakkin
	);
}
