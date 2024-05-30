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
#include "data_verification.h"
#include <map>
#include <string>
#include <vector>

namespace Race {
	constexpr uint16 Doug                     = 0;
	constexpr uint16 Human                    = 1;
	constexpr uint16 Barbarian                = 2;
	constexpr uint16 Erudite                  = 3;
	constexpr uint16 WoodElf                  = 4;
	constexpr uint16 HighElf                  = 5;
	constexpr uint16 DarkElf                  = 6;
	constexpr uint16 HalfElf                  = 7;
	constexpr uint16 Dwarf                    = 8;
	constexpr uint16 Troll                    = 9;
	constexpr uint16 Ogre                     = 10;
	constexpr uint16 Halfling                 = 11;
	constexpr uint16 Gnome                    = 12;
	constexpr uint16 Aviak                    = 13;
	constexpr uint16 Werewolf                 = 14;
	constexpr uint16 Brownie                  = 15;
	constexpr uint16 Centaur                  = 16;
	constexpr uint16 Golem                    = 17;
	constexpr uint16 Giant                    = 18;
	constexpr uint16 Trakanon                 = 19;
	constexpr uint16 VenrilSathir             = 20;
	constexpr uint16 EvilEye                  = 21;
	constexpr uint16 Beetle                   = 22;
	constexpr uint16 Kerran                   = 23;
	constexpr uint16 Fish                     = 24;
	constexpr uint16 Fairy                    = 25;
	constexpr uint16 Froglok                  = 26;
	constexpr uint16 FroglokGhoul             = 27;
	constexpr uint16 Fungusman                = 28;
	constexpr uint16 Gargoyle                 = 29;
	constexpr uint16 Gasbag                   = 30;
	constexpr uint16 GelatinousCube           = 31;
	constexpr uint16 Ghost                    = 32;
	constexpr uint16 Ghoul                    = 33;
	constexpr uint16 GiantBat                 = 34;
	constexpr uint16 GiantEel                 = 35;
	constexpr uint16 GiantRat                 = 36;
	constexpr uint16 GiantSnake               = 37;
	constexpr uint16 GiantSpider              = 38;
	constexpr uint16 Gnoll                    = 39;
	constexpr uint16 Goblin                   = 40;
	constexpr uint16 Gorilla                  = 41;
	constexpr uint16 Wolf                     = 42;
	constexpr uint16 Bear                     = 43;
	constexpr uint16 FreeportGuard            = 44;
	constexpr uint16 DemiLich                 = 45;
	constexpr uint16 Imp                      = 46;
	constexpr uint16 Griffin                  = 47;
	constexpr uint16 Kobold                   = 48;
	constexpr uint16 LavaDragon               = 49;
	constexpr uint16 Lion                     = 50;
	constexpr uint16 LizardMan                = 51;
	constexpr uint16 Mimic                    = 52;
	constexpr uint16 Minotaur                 = 53;
	constexpr uint16 Orc                      = 54;
	constexpr uint16 HumanBeggar              = 55;
	constexpr uint16 Pixie                    = 56;
	constexpr uint16 Drachnid                 = 57;
	constexpr uint16 SolusekRo                = 58;
	constexpr uint16 Bloodgill                = 59;
	constexpr uint16 Skeleton                 = 60;
	constexpr uint16 Shark                    = 61;
	constexpr uint16 Tunare                   = 62;
	constexpr uint16 Tiger                    = 63;
	constexpr uint16 Treant                   = 64;
	constexpr uint16 Vampire                  = 65;
	constexpr uint16 StatueOfRallosZek        = 66;
	constexpr uint16 HighpassCitizen          = 67;
	constexpr uint16 TentacleTerror           = 68;
	constexpr uint16 Wisp                     = 69;
	constexpr uint16 Zombie                   = 70;
	constexpr uint16 QeynosCitizen            = 71;
	constexpr uint16 Ship                     = 72;
	constexpr uint16 Launch                   = 73;
	constexpr uint16 Piranha                  = 74;
	constexpr uint16 Elemental                = 75;
	constexpr uint16 Puma                     = 76;
	constexpr uint16 NeriakCitizen            = 77;
	constexpr uint16 EruditeCitizen           = 78;
	constexpr uint16 Bixie                    = 79;
	constexpr uint16 ReanimatedHand           = 80;
	constexpr uint16 RivervaleCitizen         = 81;
	constexpr uint16 Scarecrow                = 82;
	constexpr uint16 Skunk                    = 83;
	constexpr uint16 SnakeElemental           = 84;
	constexpr uint16 Spectre                  = 85;
	constexpr uint16 Sphinx                   = 86;
	constexpr uint16 Armadillo                = 87;
	constexpr uint16 ClockworkGnome           = 88;
	constexpr uint16 Drake                    = 89;
	constexpr uint16 HalasCitizen             = 90;
	constexpr uint16 Alligator                = 91;
	constexpr uint16 GrobbCitizen             = 92;
	constexpr uint16 OggokCitizen             = 93;
	constexpr uint16 KaladimCitizen           = 94;
	constexpr uint16 CazicThule               = 95;
	constexpr uint16 Cockatrice               = 96;
	constexpr uint16 DaisyMan                 = 97;
	constexpr uint16 ElfVampire               = 98;
	constexpr uint16 Denizen                  = 99;
	constexpr uint16 Dervish                  = 100;
	constexpr uint16 Efreeti                  = 101;
	constexpr uint16 FroglokTadpole           = 102;
	constexpr uint16 PhinigelAutropos         = 103;
	constexpr uint16 Leech                    = 104;
	constexpr uint16 Swordfish                = 105;
	constexpr uint16 Felguard                 = 106;
	constexpr uint16 Mammoth                  = 107;
	constexpr uint16 EyeOfZomm                = 108;
	constexpr uint16 Wasp                     = 109;
	constexpr uint16 Mermaid                  = 110;
	constexpr uint16 Harpy                    = 111;
	constexpr uint16 Fayguard                 = 112;
	constexpr uint16 Drixie                   = 113;
	constexpr uint16 GhostShip                = 114;
	constexpr uint16 Clam                     = 115;
	constexpr uint16 SeaHorse                 = 116;
	constexpr uint16 DwarfGhost               = 117;
	constexpr uint16 EruditeGhost             = 118;
	constexpr uint16 Sabertooth               = 119;
	constexpr uint16 WolfElemental            = 120;
	constexpr uint16 Gorgon                   = 121;
	constexpr uint16 DragonSkeleton           = 122;
	constexpr uint16 Innoruuk                 = 123;
	constexpr uint16 Unicorn                  = 124;
	constexpr uint16 Pegasus                  = 125;
	constexpr uint16 Djinn                    = 126;
	constexpr uint16 InvisibleMan             = 127;
	constexpr uint16 Iksar                    = 128;
	constexpr uint16 Scorpion                 = 129;
	constexpr uint16 VahShir                  = 130;
	constexpr uint16 Sarnak                   = 131;
	constexpr uint16 Draglock                 = 132;
	constexpr uint16 Drolvarg                 = 133;
	constexpr uint16 Mosquito                 = 134;
	constexpr uint16 Rhinoceros               = 135;
	constexpr uint16 Xalgoz                   = 136;
	constexpr uint16 KunarkGoblin             = 137;
	constexpr uint16 Yeti                     = 138;
	constexpr uint16 IksarCitizen             = 139;
	constexpr uint16 ForestGiant              = 140;
	constexpr uint16 Boat                     = 141;
	constexpr uint16 MinorIllusion            = 142;
	constexpr uint16 Tree                     = 143;
	constexpr uint16 Burynai                  = 144;
	constexpr uint16 Goo                      = 145;
	constexpr uint16 SarnakSpirit             = 146;
	constexpr uint16 IksarSpirit              = 147;
	constexpr uint16 KunarkFish               = 148;
	constexpr uint16 IksarScorpion            = 149;
	constexpr uint16 Erollisi                 = 150;
	constexpr uint16 Tribunal                 = 151;
	constexpr uint16 Bertoxxulous             = 152;
	constexpr uint16 Bristlebane              = 153;
	constexpr uint16 FayDrake                 = 154;
	constexpr uint16 UndeadSarnak             = 155;
	constexpr uint16 Ratman                   = 156;
	constexpr uint16 Wyvern                   = 157;
	constexpr uint16 Wurm                     = 158;
	constexpr uint16 Devourer                 = 159;
	constexpr uint16 IksarGolem               = 160;
	constexpr uint16 UndeadIksar              = 161;
	constexpr uint16 ManEatingPlant           = 162;
	constexpr uint16 Raptor                   = 163;
	constexpr uint16 SarnakGolem              = 164;
	constexpr uint16 WaterDragon              = 165;
	constexpr uint16 AnimatedHand             = 166;
	constexpr uint16 Succulent                = 167;
	constexpr uint16 Holgresh                 = 168;
	constexpr uint16 Brontotherium            = 169;
	constexpr uint16 SnowDervish              = 170;
	constexpr uint16 DireWolf                 = 171;
	constexpr uint16 Manticore                = 172;
	constexpr uint16 Totem                    = 173;
	constexpr uint16 IceSpectre               = 174;
	constexpr uint16 EnchantedArmor           = 175;
	constexpr uint16 SnowRabbit               = 176;
	constexpr uint16 Walrus                   = 177;
	constexpr uint16 Geonid                   = 178;
	constexpr uint16 Unknown                  = 179;
	constexpr uint16 Unknown2                 = 180;
	constexpr uint16 Yakkar                   = 181;
	constexpr uint16 Faun                     = 182;
	constexpr uint16 Coldain                  = 183;
	constexpr uint16 VeliousDragon            = 184;
	constexpr uint16 Hag                      = 185;
	constexpr uint16 Hippogriff               = 186;
	constexpr uint16 Siren                    = 187;
	constexpr uint16 FrostGiant               = 188;
	constexpr uint16 StormGiant               = 189;
	constexpr uint16 Othmir                   = 190;
	constexpr uint16 Ulthork                  = 191;
	constexpr uint16 ClockworkDragon          = 192;
	constexpr uint16 Abhorrent                = 193;
	constexpr uint16 SeaTurtle                = 194;
	constexpr uint16 BlackAndWhiteDragon      = 195;
	constexpr uint16 GhostDragon              = 196;
	constexpr uint16 RonnieTest               = 197;
	constexpr uint16 PrismaticDragon          = 198;
	constexpr uint16 Shiknar                  = 199;
	constexpr uint16 Rockhopper               = 200;
	constexpr uint16 Underbulk                = 201;
	constexpr uint16 Grimling                 = 202;
	constexpr uint16 Worm                     = 203;
	constexpr uint16 EvanTest                 = 204;
	constexpr uint16 KhatiSha                 = 205;
	constexpr uint16 Owlbear                  = 206;
	constexpr uint16 RhinoBeetle              = 207;
	constexpr uint16 Vampire2                 = 208;
	constexpr uint16 EarthElemental           = 209;
	constexpr uint16 AirElemental             = 210;
	constexpr uint16 WaterElemental           = 211;
	constexpr uint16 FireElemental            = 212;
	constexpr uint16 WetfangMinnow            = 213;
	constexpr uint16 ThoughtHorror            = 214;
	constexpr uint16 Tegi                     = 215;
	constexpr uint16 Horse                    = 216;
	constexpr uint16 Shissar                  = 217;
	constexpr uint16 FungalFiend              = 218;
	constexpr uint16 VampireVolatalis         = 219;
	constexpr uint16 Stonegrabber             = 220;
	constexpr uint16 ScarletCheetah           = 221;
	constexpr uint16 Zelniak                  = 222;
	constexpr uint16 Lightcrawler             = 223;
	constexpr uint16 Shade                    = 224;
	constexpr uint16 Sunflower                = 225;
	constexpr uint16 Shadel                   = 226;
	constexpr uint16 Shrieker                 = 227;
	constexpr uint16 Galorian                 = 228;
	constexpr uint16 Netherbian               = 229;
	constexpr uint16 Akhevan                  = 230;
	constexpr uint16 GriegVeneficus           = 231;
	constexpr uint16 SonicWolf                = 232;
	constexpr uint16 GroundShaker             = 233;
	constexpr uint16 VahShirSkeleton          = 234;
	constexpr uint16 Wretch                   = 235;
	constexpr uint16 LordInquisitorSeru       = 236;
	constexpr uint16 Recuso                   = 237;
	constexpr uint16 VahShirKing              = 238;
	constexpr uint16 VahShirGuard             = 239;
	constexpr uint16 TeleportMan              = 240;
	constexpr uint16 Werewolf2                = 241;
	constexpr uint16 Nymph                    = 242;
	constexpr uint16 Dryad                    = 243;
	constexpr uint16 Treant2                  = 244;
	constexpr uint16 Fly                      = 245;
	constexpr uint16 TarewMarr                = 246;
	constexpr uint16 SolusekRo2               = 247;
	constexpr uint16 ClockworkGolem           = 248;
	constexpr uint16 ClockworkBrain           = 249;
	constexpr uint16 Banshee                  = 250;
	constexpr uint16 GuardOfJustice           = 251;
	constexpr uint16 MiniPom                  = 252;
	constexpr uint16 DiseasedFiend            = 253;
	constexpr uint16 SolusekRoGuard           = 254;
	constexpr uint16 BertoxxulousNew          = 255;
	constexpr uint16 TribunalNew              = 256;
	constexpr uint16 TerrisThule              = 257;
	constexpr uint16 Vegerog                  = 258;
	constexpr uint16 Crocodile                = 259;
	constexpr uint16 Bat                      = 260;
	constexpr uint16 Hraquis                  = 261;
	constexpr uint16 Tranquilion              = 262;
	constexpr uint16 TinSoldier               = 263;
	constexpr uint16 NightmareWraith          = 264;
	constexpr uint16 Malarian                 = 265;
	constexpr uint16 KnightOfPestilence       = 266;
	constexpr uint16 Lepertoloth              = 267;
	constexpr uint16 Bubonian                 = 268;
	constexpr uint16 BubonianUnderling        = 269;
	constexpr uint16 Pusling                  = 270;
	constexpr uint16 WaterMephit              = 271;
	constexpr uint16 Stormrider               = 272;
	constexpr uint16 JunkBeast                = 273;
	constexpr uint16 BrokenClockwork          = 274;
	constexpr uint16 GiantClockwork           = 275;
	constexpr uint16 ClockworkBeetle          = 276;
	constexpr uint16 NightmareGoblin          = 277;
	constexpr uint16 Karana                   = 278;
	constexpr uint16 BloodRaven               = 279;
	constexpr uint16 NightmareGargoyle        = 280;
	constexpr uint16 MouthOfInsanity          = 281;
	constexpr uint16 SkeletalHorse            = 282;
	constexpr uint16 Saryrn                   = 283;
	constexpr uint16 FenninRo                 = 284;
	constexpr uint16 Tormentor                = 285;
	constexpr uint16 SoulDevourer             = 286;
	constexpr uint16 Nightmare                = 287;
	constexpr uint16 NewRallosZek             = 288;
	constexpr uint16 VallonZek                = 289;
	constexpr uint16 TallonZek                = 290;
	constexpr uint16 AirMephit                = 291;
	constexpr uint16 EarthMephit              = 292;
	constexpr uint16 FireMephit               = 293;
	constexpr uint16 NightmareMephit          = 294;
	constexpr uint16 Zebuxoruk                = 295;
	constexpr uint16 MithanielMarr            = 296;
	constexpr uint16 UndeadKnight             = 297;
	constexpr uint16 Rathe                    = 298;
	constexpr uint16 Xegony                   = 299;
	constexpr uint16 Fiend                    = 300;
	constexpr uint16 TestObject               = 301;
	constexpr uint16 Crab                     = 302;
	constexpr uint16 Phoenix                  = 303;
	constexpr uint16 Quarm                    = 304;
	constexpr uint16 Bear2                    = 305;
	constexpr uint16 EarthGolem               = 306;
	constexpr uint16 IronGolem                = 307;
	constexpr uint16 StormGolem               = 308;
	constexpr uint16 AirGolem                 = 309;
	constexpr uint16 WoodGolem                = 310;
	constexpr uint16 FireGolem                = 311;
	constexpr uint16 WaterGolem               = 312;
	constexpr uint16 WarWraith                = 313;
	constexpr uint16 Wrulon                   = 314;
	constexpr uint16 Kraken                   = 315;
	constexpr uint16 PoisonFrog               = 316;
	constexpr uint16 Nilborien                = 317;
	constexpr uint16 Valorian                 = 318;
	constexpr uint16 WarBoar                  = 319;
	constexpr uint16 Efreeti2                 = 320;
	constexpr uint16 WarBoar2                 = 321;
	constexpr uint16 Valorian2                = 322;
	constexpr uint16 AnimatedArmor            = 323;
	constexpr uint16 UndeadFootman            = 324;
	constexpr uint16 RallosOgre               = 325;
	constexpr uint16 Arachnid                 = 326;
	constexpr uint16 CrystalSpider            = 327;
	constexpr uint16 ZebuxoruksCage           = 328;
	constexpr uint16 Portal                   = 329;
	constexpr uint16 Froglok2                 = 330;
	constexpr uint16 TrollCrewMember          = 331;
	constexpr uint16 PirateDeckhand           = 332;
	constexpr uint16 BrokenSkullPirate        = 333;
	constexpr uint16 PirateGhost              = 334;
	constexpr uint16 OneArmedPirate           = 335;
	constexpr uint16 SpiritmasterNadox        = 336;
	constexpr uint16 BrokenSkullTaskmaster    = 337;
	constexpr uint16 GnomePirate              = 338;
	constexpr uint16 DarkElfPirate            = 339;
	constexpr uint16 OgrePirate               = 340;
	constexpr uint16 HumanPirate              = 341;
	constexpr uint16 EruditePirate            = 342;
	constexpr uint16 Frog                     = 343;
	constexpr uint16 TrollZombie              = 344;
	constexpr uint16 Luggald                  = 345;
	constexpr uint16 Luggald2                 = 346;
	constexpr uint16 Luggald3                 = 347;
	constexpr uint16 Drogmor                  = 348;
	constexpr uint16 FroglokSkeleton          = 349;
	constexpr uint16 UndeadFroglok            = 350;
	constexpr uint16 KnightOfHate             = 351;
	constexpr uint16 ArcanistOfHate           = 352;
	constexpr uint16 Veksar                   = 353;
	constexpr uint16 Veksar2                  = 354;
	constexpr uint16 Veksar3                  = 355;
	constexpr uint16 Chokidai                 = 356;
	constexpr uint16 UndeadChokidai           = 357;
	constexpr uint16 UndeadVeksar             = 358;
	constexpr uint16 UndeadVampire            = 359;
	constexpr uint16 Vampire3                 = 360;
	constexpr uint16 RujarkianOrc             = 361;
	constexpr uint16 BoneGolem                = 362;
	constexpr uint16 Synarcana                = 363;
	constexpr uint16 SandElf                  = 364;
	constexpr uint16 MasterVampire            = 365;
	constexpr uint16 MasterOrc                = 366;
	constexpr uint16 Skeleton2                = 367;
	constexpr uint16 Mummy                    = 368;
	constexpr uint16 NewGoblin                = 369;
	constexpr uint16 Insect                   = 370;
	constexpr uint16 FroglokGhost             = 371;
	constexpr uint16 Dervish2                 = 372;
	constexpr uint16 Shade2                   = 373;
	constexpr uint16 Golem2                   = 374;
	constexpr uint16 EvilEye2                 = 375;
	constexpr uint16 Box                      = 376;
	constexpr uint16 Barrel                   = 377;
	constexpr uint16 Chest                    = 378;
	constexpr uint16 Vase                     = 379;
	constexpr uint16 Table                    = 380;
	constexpr uint16 WeaponRack               = 381;
	constexpr uint16 Coffin                   = 382;
	constexpr uint16 Bones                    = 383;
	constexpr uint16 Jokester                 = 384;
	constexpr uint16 Nihil                    = 385;
	constexpr uint16 Trusik                   = 386;
	constexpr uint16 StoneWorker              = 387;
	constexpr uint16 Hynid                    = 388;
	constexpr uint16 Turepta                  = 389;
	constexpr uint16 Cragbeast                = 390;
	constexpr uint16 Stonemite                = 391;
	constexpr uint16 Ukun                     = 392;
	constexpr uint16 Ixt                      = 393;
	constexpr uint16 Ikaav                    = 394;
	constexpr uint16 Aneuk                    = 395;
	constexpr uint16 Kyv                      = 396;
	constexpr uint16 Noc                      = 397;
	constexpr uint16 Ratuk                    = 398;
	constexpr uint16 Taneth                   = 399;
	constexpr uint16 Huvul                    = 400;
	constexpr uint16 Mutna                    = 401;
	constexpr uint16 Mastruq                  = 402;
	constexpr uint16 Taelosian                = 403;
	constexpr uint16 DiscordShip              = 404;
	constexpr uint16 StoneWorker2             = 405;
	constexpr uint16 MataMuram                = 406;
	constexpr uint16 LightingWarrior          = 407;
	constexpr uint16 Succubus                 = 408;
	constexpr uint16 Bazu                     = 409;
	constexpr uint16 Feran                    = 410;
	constexpr uint16 Pyrilen                  = 411;
	constexpr uint16 Chimera                  = 412;
	constexpr uint16 Dragorn                  = 413;
	constexpr uint16 Murkglider               = 414;
	constexpr uint16 Rat                      = 415;
	constexpr uint16 Bat2                     = 416;
	constexpr uint16 Gelidran                 = 417;
	constexpr uint16 Discordling              = 418;
	constexpr uint16 Girplan                  = 419;
	constexpr uint16 Minotaur2                = 420;
	constexpr uint16 DragornBox               = 421;
	constexpr uint16 RunedOrb                 = 422;
	constexpr uint16 DragonBones              = 423;
	constexpr uint16 MuramiteArmorPile        = 424;
	constexpr uint16 CrystalShard             = 425;
	constexpr uint16 Portal2                  = 426;
	constexpr uint16 CoinPurse                = 427;
	constexpr uint16 RockPile                 = 428;
	constexpr uint16 MurkgliderEggSack        = 429;
	constexpr uint16 Drake2                   = 430;
	constexpr uint16 Dervish3                 = 431;
	constexpr uint16 Drake3                   = 432;
	constexpr uint16 Goblin2                  = 433;
	constexpr uint16 Kirin                    = 434;
	constexpr uint16 Dragon                   = 435;
	constexpr uint16 Basilisk                 = 436;
	constexpr uint16 Dragon2                  = 437;
	constexpr uint16 Dragon3                  = 438;
	constexpr uint16 Puma2                    = 439;
	constexpr uint16 Spider                   = 440;
	constexpr uint16 SpiderQueen              = 441;
	constexpr uint16 AnimatedStatue           = 442;
	constexpr uint16 Unknown3                 = 443;
	constexpr uint16 Unknown4                 = 444;
	constexpr uint16 DragonEgg                = 445;
	constexpr uint16 DragonStatue             = 446;
	constexpr uint16 LavaRock                 = 447;
	constexpr uint16 AnimatedStatue2          = 448;
	constexpr uint16 SpiderEggSack            = 449;
	constexpr uint16 LavaSpider               = 450;
	constexpr uint16 LavaSpiderQueen          = 451;
	constexpr uint16 Dragon4                  = 452;
	constexpr uint16 Giant2                   = 453;
	constexpr uint16 Werewolf3                = 454;
	constexpr uint16 Kobold2                  = 455;
	constexpr uint16 Sporali                  = 456;
	constexpr uint16 Gnomework                = 457;
	constexpr uint16 Orc2                     = 458;
	constexpr uint16 Corathus                 = 459;
	constexpr uint16 Coral                    = 460;
	constexpr uint16 Drachnid2                = 461;
	constexpr uint16 DrachnidCocoon           = 462;
	constexpr uint16 FungusPatch              = 463;
	constexpr uint16 Gargoyle2                = 464;
	constexpr uint16 Witheran                 = 465;
	constexpr uint16 DarkLord                 = 466;
	constexpr uint16 Shiliskin                = 467;
	constexpr uint16 Snake                    = 468;
	constexpr uint16 EvilEye3                 = 469;
	constexpr uint16 Minotaur3                = 470;
	constexpr uint16 Zombie2                  = 471;
	constexpr uint16 ClockworkBoar            = 472;
	constexpr uint16 Fairy2                   = 473;
	constexpr uint16 Witheran2                = 474;
	constexpr uint16 AirElemental2            = 475;
	constexpr uint16 EarthElemental2          = 476;
	constexpr uint16 FireElemental2           = 477;
	constexpr uint16 WaterElemental2          = 478;
	constexpr uint16 Alligator2               = 479;
	constexpr uint16 Bear3                    = 480;
	constexpr uint16 ScaledWolf               = 481;
	constexpr uint16 Wolf2                    = 482;
	constexpr uint16 SpiritWolf               = 483;
	constexpr uint16 Skeleton3                = 484;
	constexpr uint16 Spectre2                 = 485;
	constexpr uint16 Bolvirk                  = 486;
	constexpr uint16 Banshee2                 = 487;
	constexpr uint16 Banshee3                 = 488;
	constexpr uint16 Elddar                   = 489;
	constexpr uint16 ForestGiant2             = 490;
	constexpr uint16 BoneGolem2               = 491;
	constexpr uint16 Horse2                   = 492;
	constexpr uint16 Pegasus2                 = 493;
	constexpr uint16 ShamblingMound           = 494;
	constexpr uint16 Scrykin                  = 495;
	constexpr uint16 Treant3                  = 496;
	constexpr uint16 Vampire4                 = 497;
	constexpr uint16 AyonaeRo                 = 498;
	constexpr uint16 SullonZek                = 499;
	constexpr uint16 Banner                   = 500;
	constexpr uint16 Flag                     = 501;
	constexpr uint16 Rowboat                  = 502;
	constexpr uint16 BearTrap                 = 503;
	constexpr uint16 ClockworkBomb            = 504;
	constexpr uint16 DynamiteKeg              = 505;
	constexpr uint16 PressurePlate            = 506;
	constexpr uint16 PufferSpore              = 507;
	constexpr uint16 StoneRing                = 508;
	constexpr uint16 RootTentacle             = 509;
	constexpr uint16 RunicSymbol              = 510;
	constexpr uint16 SaltpetterBomb           = 511;
	constexpr uint16 FloatingSkull            = 512;
	constexpr uint16 SpikeTrap                = 513;
	constexpr uint16 Totem2                   = 514;
	constexpr uint16 Web                      = 515;
	constexpr uint16 WickerBasket             = 516;
	constexpr uint16 Unicorn2                 = 517;
	constexpr uint16 Horse3                   = 518;
	constexpr uint16 Unicorn3                 = 519;
	constexpr uint16 Bixie2                   = 520;
	constexpr uint16 Centaur2                 = 521;
	constexpr uint16 Drakkin                  = 522;
	constexpr uint16 Giant3                   = 523;
	constexpr uint16 Gnoll2                   = 524;
	constexpr uint16 Griffin2                 = 525;
	constexpr uint16 GiantShade               = 526;
	constexpr uint16 Harpy2                   = 527;
	constexpr uint16 Mammoth2                 = 528;
	constexpr uint16 Satyr                    = 529;
	constexpr uint16 Dragon5                  = 530;
	constexpr uint16 Dragon6                  = 531;
	constexpr uint16 Dynleth                  = 532;
	constexpr uint16 Boat2                    = 533;
	constexpr uint16 WeaponRack2              = 534;
	constexpr uint16 ArmorRack                = 535;
	constexpr uint16 HoneyPot                 = 536;
	constexpr uint16 JumJumBucket             = 537;
	constexpr uint16 Plant                    = 538;
	constexpr uint16 StoneJug                 = 539;
	constexpr uint16 Plant2                   = 540;
	constexpr uint16 Toolbox                  = 541;
	constexpr uint16 WineCask                 = 542;
	constexpr uint16 StoneJug2                = 543;
	constexpr uint16 ElvenBoat                = 544;
	constexpr uint16 GnomishBoat              = 545;
	constexpr uint16 UndeadBoat               = 546;
	constexpr uint16 Goo2                     = 547;
	constexpr uint16 Goo3                     = 548;
	constexpr uint16 Goo4                     = 549;
	constexpr uint16 MerchantShip             = 550;
	constexpr uint16 PirateShip               = 551;
	constexpr uint16 GhostShip2               = 552;
	constexpr uint16 Banner2                  = 553;
	constexpr uint16 Banner3                  = 554;
	constexpr uint16 Banner4                  = 555;
	constexpr uint16 Banner5                  = 556;
	constexpr uint16 Banner6                  = 557;
	constexpr uint16 Aviak2                   = 558;
	constexpr uint16 Beetle2                  = 559;
	constexpr uint16 Gorilla2                 = 560;
	constexpr uint16 Kedge                    = 561;
	constexpr uint16 Kerran2                  = 562;
	constexpr uint16 Shissar2                 = 563;
	constexpr uint16 Siren2                   = 564;
	constexpr uint16 Sphinx2                  = 565;
	constexpr uint16 Human2                   = 566;
	constexpr uint16 Campfire                 = 567;
	constexpr uint16 Brownie2                 = 568;
	constexpr uint16 Dragon7                  = 569;
	constexpr uint16 Exoskeleton              = 570;
	constexpr uint16 Ghoul2                   = 571;
	constexpr uint16 ClockworkGuardian        = 572;
	constexpr uint16 Unknown5                 = 573;
	constexpr uint16 Minotaur4                = 574;
	constexpr uint16 Scarecrow2               = 575;
	constexpr uint16 Shade3                   = 576;
	constexpr uint16 Rotocopter               = 577;
	constexpr uint16 TentacleTerror2          = 578;
	constexpr uint16 Wereorc                  = 579;
	constexpr uint16 Worg                     = 580;
	constexpr uint16 Wyvern2                  = 581;
	constexpr uint16 Chimera2                 = 582;
	constexpr uint16 Kirin2                   = 583;
	constexpr uint16 Puma3                    = 584;
	constexpr uint16 Boulder                  = 585;
	constexpr uint16 Banner7                  = 586;
	constexpr uint16 ElvenGhost               = 587;
	constexpr uint16 HumanGhost               = 588;
	constexpr uint16 Chest2                   = 589;
	constexpr uint16 Chest3                   = 590;
	constexpr uint16 Crystal                  = 591;
	constexpr uint16 Coffin2                  = 592;
	constexpr uint16 GuardianCpu              = 593;
	constexpr uint16 Worg2                    = 594;
	constexpr uint16 Mansion                  = 595;
	constexpr uint16 FloatingIsland           = 596;
	constexpr uint16 Cragslither              = 597;
	constexpr uint16 Wrulon2                  = 598;
	constexpr uint16 SpellParticle            = 599;
	constexpr uint16 InvisibleManOfZomm       = 600;
	constexpr uint16 RobocopterOfZomm         = 601;
	constexpr uint16 Burynai2                 = 602;
	constexpr uint16 Frog2                    = 603;
	constexpr uint16 Dracolich                = 604;
	constexpr uint16 IksarGhost               = 605;
	constexpr uint16 IksarSkeleton            = 606;
	constexpr uint16 Mephit                   = 607;
	constexpr uint16 Muddite                  = 608;
	constexpr uint16 Raptor2                  = 609;
	constexpr uint16 Sarnak2                  = 610;
	constexpr uint16 Scorpion2                = 611;
	constexpr uint16 Tsetsian                 = 612;
	constexpr uint16 Wurm2                    = 613;
	constexpr uint16 Nekhon                   = 614;
	constexpr uint16 HydraCrystal             = 615;
	constexpr uint16 CrystalSphere            = 616;
	constexpr uint16 Gnoll3                   = 617;
	constexpr uint16 Sokokar                  = 618;
	constexpr uint16 StonePylon               = 619;
	constexpr uint16 DemonVulture             = 620;
	constexpr uint16 Wagon                    = 621;
	constexpr uint16 GodOfDiscord             = 622;
	constexpr uint16 FeranMount               = 623;
	constexpr uint16 Ogre2                    = 624;
	constexpr uint16 SokokarMount             = 625;
	constexpr uint16 Giant4                   = 626;
	constexpr uint16 SokokarMount2            = 627;
	constexpr uint16 TenthAnniversaryBanner   = 628;
	constexpr uint16 TenthAnniversaryCake     = 629;
	constexpr uint16 WineCask2                = 630;
	constexpr uint16 HydraMount               = 631;
	constexpr uint16 Hydra                    = 632;
	constexpr uint16 WeddingFlowers           = 633;
	constexpr uint16 WeddingArbor             = 634;
	constexpr uint16 WeddingAltar             = 635;
	constexpr uint16 PowderKeg                = 636;
	constexpr uint16 Apexus                   = 637;
	constexpr uint16 Bellikos                 = 638;
	constexpr uint16 BrellsFirstCreation      = 639;
	constexpr uint16 Brell                    = 640;
	constexpr uint16 CrystalskinAmbuloid      = 641;
	constexpr uint16 CliknarQueen             = 642;
	constexpr uint16 CliknarSoldier           = 643;
	constexpr uint16 CliknarWorker            = 644;
	constexpr uint16 Coldain2                 = 645;
	constexpr uint16 Coldain3                 = 646;
	constexpr uint16 CrystalskinSessiloid     = 647;
	constexpr uint16 Genari                   = 648;
	constexpr uint16 Gigyn                    = 649;
	constexpr uint16 GrekenYoungAdult         = 650;
	constexpr uint16 GrekenYoung              = 651;
	constexpr uint16 CliknarMount             = 652;
	constexpr uint16 Telmira                  = 653;
	constexpr uint16 SpiderMount              = 654;
	constexpr uint16 BearMount                = 655;
	constexpr uint16 RatMount                 = 656;
	constexpr uint16 SessiloidMount           = 657;
	constexpr uint16 MorellThule              = 658;
	constexpr uint16 Marionette               = 659;
	constexpr uint16 BookDervish              = 660;
	constexpr uint16 TopiaryLion              = 661;
	constexpr uint16 RotDog                   = 662;
	constexpr uint16 Amygdalan                = 663;
	constexpr uint16 Sandman                  = 664;
	constexpr uint16 GrandfatherClock         = 665;
	constexpr uint16 GingerbreadMan           = 666;
	constexpr uint16 RoyalGuard               = 667;
	constexpr uint16 Rabbit                   = 668;
	constexpr uint16 BlindDreamer             = 669;
	constexpr uint16 CazicThule2              = 670;
	constexpr uint16 TopiaryLionMount         = 671;
	constexpr uint16 RotDogMount              = 672;
	constexpr uint16 GoralMount               = 673;
	constexpr uint16 SelyrahMount             = 674;
	constexpr uint16 ScleraMount              = 675;
	constexpr uint16 BraxiMount               = 676;
	constexpr uint16 KangonMount              = 677;
	constexpr uint16 Erudite2                 = 678;
	constexpr uint16 WurmMount                = 679;
	constexpr uint16 RaptorMount              = 680;
	constexpr uint16 InvisibleMan2            = 681;
	constexpr uint16 Whirligig                = 682;
	constexpr uint16 GnomishBalloon           = 683;
	constexpr uint16 GnomishRocketPack        = 684;
	constexpr uint16 GnomishHoveringTransport = 685;
	constexpr uint16 Selyrah                  = 686;
	constexpr uint16 Goral                    = 687;
	constexpr uint16 Braxi                    = 688;
	constexpr uint16 Kangon                   = 689;
	constexpr uint16 InvisibleMan3            = 690;
	constexpr uint16 FloatingTower            = 691;
	constexpr uint16 ExplosiveCart            = 692;
	constexpr uint16 BlimpShip                = 693;
	constexpr uint16 Tumbleweed               = 694;
	constexpr uint16 Alaran                   = 695;
	constexpr uint16 Swinetor                 = 696;
	constexpr uint16 Triumvirate              = 697;
	constexpr uint16 Hadal                    = 698;
	constexpr uint16 HoveringPlatform         = 699;
	constexpr uint16 ParasiticScavenger       = 700;
	constexpr uint16 Grendlaen                = 701;
	constexpr uint16 ShipInABottle            = 702;
	constexpr uint16 AlaranSentryStone        = 703;
	constexpr uint16 Dervish4                 = 704;
	constexpr uint16 RegenerationPool         = 705;
	constexpr uint16 TeleportationStand       = 706;
	constexpr uint16 RelicCase                = 707;
	constexpr uint16 AlaranGhost              = 708;
	constexpr uint16 Skystrider               = 709;
	constexpr uint16 WaterSpout               = 710;
	constexpr uint16 AviakPullAlong           = 711;
	constexpr uint16 GelatinousCube2          = 712;
	constexpr uint16 Cat                      = 713;
	constexpr uint16 ElkHead                  = 714;
	constexpr uint16 Holgresh2                = 715;
	constexpr uint16 Beetle3                  = 716;
	constexpr uint16 VineMaw                  = 717;
	constexpr uint16 Ratman2                  = 718;
	constexpr uint16 FallenKnight             = 719;
	constexpr uint16 FlyingCarpet             = 720;
	constexpr uint16 CarrierHand              = 721;
	constexpr uint16 Akheva                   = 722;
	constexpr uint16 ServantOfShadow          = 723;
	constexpr uint16 Luclin                   = 724;
	constexpr uint16 Xaric                    = 725;
	constexpr uint16 Dervish5                 = 726;
	constexpr uint16 Dervish6                 = 727;
	constexpr uint16 Luclin2                  = 728;
	constexpr uint16 Luclin3                  = 729;
	constexpr uint16 Orb                      = 730;
	constexpr uint16 Luclin4                  = 731;
	constexpr uint16 Pegasus3                 = 732;
	constexpr uint16 InteractiveObject        = 2250;
	constexpr uint16 Node                     = 2254;

	const std::string& GetName(uint16 race_id);
	const std::string& GetAbbreviation(uint16 race_id);

	bool IsPlayerRace(uint16 race_id);
	bool IsValid(uint16 race_id);

	uint16 GetPlayerValue(uint16 race_id);
	uint16 GetPlayerBit(uint16 race_id);
}

namespace Gender {
	constexpr uint8 Male   = 0;
	constexpr uint8 Female = 1;
	constexpr uint8 Neuter = 2;

	const std::string& GetGenderName(uint8 gender_id);
	float GetRaceGenderDefaultHeight(uint16 race_id, uint8 gender_id);
}

static std::map<uint8, std::string> gender_map = {
	{ Gender::Male,   "Male" },
	{ Gender::Female, "Female" },
	{ Gender::Neuter, "Neuter" }
};

namespace PlayerRace {
	constexpr uint16 Unknown   = 0;
	constexpr uint16 Human     = 1;
	constexpr uint16 Barbarian = 2;
	constexpr uint16 Erudite   = 3;
	constexpr uint16 WoodElf   = 4;
	constexpr uint16 HighElf   = 5;
	constexpr uint16 DarkElf   = 6;
	constexpr uint16 HalfElf   = 7;
	constexpr uint16 Dwarf     = 8;
	constexpr uint16 Troll     = 9;
	constexpr uint16 Ogre      = 10;
	constexpr uint16 Halfling  = 11;
	constexpr uint16 Gnome     = 12;
	constexpr uint16 Iksar     = 13;
	constexpr uint16 VahShir   = 14;
	constexpr uint16 Froglok   = 15;
	constexpr uint16 Drakkin   = 16;
	constexpr uint16 Count     = 16;
}

namespace PlayerRaceBitmask {
	constexpr uint16 Unknown   = 0;
	constexpr uint16 Human     = 1;
	constexpr uint16 Barbarian = 2;
	constexpr uint16 Erudite   = 4;
	constexpr uint16 WoodElf   = 8;
	constexpr uint16 HighElf   = 16;
	constexpr uint16 DarkElf   = 32;
	constexpr uint16 HalfElf   = 64;
	constexpr uint16 Dwarf     = 128;
	constexpr uint16 Troll     = 256;
	constexpr uint16 Ogre      = 512;
	constexpr uint16 Halfling  = 1024;
	constexpr uint16 Gnome     = 2048;
	constexpr uint16 Iksar     = 4096;
	constexpr uint16 VahShir   = 8192;
	constexpr uint16 Froglok   = 16384;
	constexpr uint16 Drakkin   = 32768;
	constexpr uint16 All       = 65535;
}

static std::map<uint16, uint16> player_race_bitmasks = {
	{ Race::Human,     PlayerRaceBitmask::Human },
	{ Race::Barbarian, PlayerRaceBitmask::Barbarian },
	{ Race::Erudite,   PlayerRaceBitmask::Erudite },
	{ Race::WoodElf,   PlayerRaceBitmask::WoodElf },
	{ Race::HighElf,   PlayerRaceBitmask::HighElf },
	{ Race::DarkElf,   PlayerRaceBitmask::DarkElf },
	{ Race::HalfElf,   PlayerRaceBitmask::HalfElf },
	{ Race::Dwarf,     PlayerRaceBitmask::Dwarf },
	{ Race::Troll,     PlayerRaceBitmask::Troll },
	{ Race::Ogre,      PlayerRaceBitmask::Ogre },
	{ Race::Halfling,  PlayerRaceBitmask::Halfling },
	{ Race::Gnome,     PlayerRaceBitmask::Gnome },
	{ Race::Iksar,     PlayerRaceBitmask::Iksar },
	{ Race::VahShir,   PlayerRaceBitmask::VahShir },
	{ Race::Froglok,   PlayerRaceBitmask::Froglok },
	{ Race::Drakkin,   PlayerRaceBitmask::Drakkin },
};

static std::map<uint16, std::string> player_race_abbreviations = {
	{ Race::Human,     "HUM" },
	{ Race::Barbarian, "BAR" },
	{ Race::Erudite,   "ERU" },
	{ Race::WoodElf,   "ELF" },
	{ Race::HighElf,   "HIE" },
	{ Race::DarkElf,   "DEF" },
	{ Race::HalfElf,   "HEF" },
	{ Race::Dwarf,     "DWF" },
	{ Race::Troll,     "TRL" },
	{ Race::Ogre,      "OGR" },
	{ Race::Halfling,  "HFL" },
	{ Race::Gnome,     "GNM" },
	{ Race::Iksar,     "IKS" },
	{ Race::VahShir,   "VAH" },
	{ Race::Froglok2,  "FRG" },
	{ Race::Drakkin,   "DRK" }
};

static const std::map<uint16, std::vector<float>> race_sizes = {
	{ Race::Human,                 { 7.0f,   7.0f }},
	{ Race::Barbarian,             { 5.0f,   5.0f }},
	{ Race::Erudite,               { 5.0f,   5.0f }},
	{ Race::WoodElf,               { 5.5f,   5.5f }},
	{ Race::HighElf,               { 4.0f,   4.0f }},
	{ Race::DarkElf,               { 8.0f,   8.0f }},
	{ Race::HalfElf,               { 9.0f,   9.0f }},
	{ Race::Dwarf,                 { 3.5f,   3.5f }},
	{ Race::Troll,                 { 3.0f,   3.0f }},
	{ Race::Ogre,                  { 2.0f,   2.0f }},
	{ Race::Halfling,              { 8.5f,   8.5f }},
	{ Race::Gnome,                 { 8.0f,   8.0f }},
	{ Race::Aviak,                 { 21.0f,  21.0f }},
	{ Race::Werewolf,              { 20.0f,  20.0f }},
	{ Race::Brownie,               { 3.5f,   3.5f }},
	{ Race::Centaur,               { 3.0f,   3.0f }},
	{ Race::Golem,                 { 2.0f,   2.0f }},
	{ Race::Giant,                 { 5.0f,   5.0f }},
	{ Race::Trakanon,              { 5.0f,   5.0f }},
	{ Race::VenrilSathir,          { 7.5f,   7.5f }},
	{ Race::EvilEye,               { 5.0f,   5.0f }},
	{ Race::Beetle,                { 7.0f,   7.0f }},
	{ Race::Kerran,                { 4.0f,   4.0f }},
	{ Race::Fish,                  { 4.7f,   4.7f }},
	{ Race::Fairy,                 { 8.0f,   8.0f }},
	{ Race::Froglok,               { 3.0f,   3.0f }},
	{ Race::FroglokGhoul,          { 12.0f,  12.0f }},
	{ Race::Fungusman,             { 5.0f,   5.0f }},
	{ Race::Gargoyle,              { 21.0f,  21.0f }},
	{ Race::Gasbag,                { 3.0f,   3.0f }},
	{ Race::GelatinousCube,        { 9.0f,   9.0f }},
	{ Race::Ghost,                 { 2.0f,   2.0f }},
	{ Race::Ghoul,                 { 3.0f,   3.0f }},
	{ Race::GiantBat,              { 4.0f,   4.0f }},
	{ Race::GiantEel,              { 20.0f,  20.0f }},
	{ Race::GiantRat,              { 5.0f,   5.0f }},
	{ Race::GiantSnake,            { 5.0f,   5.0f }},
	{ Race::GiantSpider,           { 9.0f,   9.0f }},
	{ Race::Gnoll,                 { 25.0f,  25.0f }},
	{ Race::Goblin,                { 10.0f,  10.0f }},
	{ Race::Gorilla,               { 2.5f,   2.5f }},
	{ Race::Wolf,                  { 7.0f,   7.0f }},
	{ Race::Bear,                  { 5.0f,   5.0f }},
	{ Race::FreeportGuard,         { 1.5f,   1.5f }},
	{ Race::DemiLich,              { 1.0f,   1.0f }},
	{ Race::Imp,                   { 3.5f,   3.5f }},
	{ Race::Griffin,               { 7.0f,   7.0f }},
	{ Race::Kobold,                { 7.0f,   7.0f }},
	{ Race::LavaDragon,            { 3.0f,   3.0f }},
	{ Race::Lion,                  { 3.0f,   3.0f }},
	{ Race::LizardMan,             { 7.0f,   7.0f }},
	{ Race::Mimic,                 { 12.0f,  12.0f }},
	{ Race::Minotaur,              { 8.0f,   8.0f }},
	{ Race::Orc,                   { 9.0f,   9.0f }},
	{ Race::HumanBeggar,           { 4.0f,   4.0f }},
	{ Race::Pixie,                 { 11.5f,  11.5f }},
	{ Race::Drachnid,              { 8.0f,   8.0f }},
	{ Race::SolusekRo,             { 12.0f,  12.0f }},
	{ Race::Bloodgill,             { 20.0f,  20.0f }},
	{ Race::Skeleton,              { 10.0f,  10.0f }},
	{ Race::Shark,                 { 6.5f,   6.5f }},
	{ Race::Tunare,                { 17.0f,  17.0f }},
	{ Race::Tiger,                 { 1.0f,   1.0f }},
	{ Race::Treant,                { 4.0f,   4.0f }},
	{ Race::Vampire,               { 8.0f,   8.0f }},
	{ Race::StatueOfRallosZek,     { 5.0f,   5.0f }},
	{ Race::HighpassCitizen,       { 1.0f,   1.0f }},
	{ Race::TentacleTerror,        { 5.0f,   5.0f }},
	{ Race::Wisp,                  { 5.0f,   5.0f }},
	{ Race::Zombie,                { 5.0f,   5.0f }},
	{ Race::QeynosCitizen,         { 9.0f,   9.0f }},
	{ Race::Ship,                  { 3.0f,   3.0f }},
	{ Race::Launch,                { 8.0f,   8.0f }},
	{ Race::Piranha,               { 2.0f,   2.0f }},
	{ Race::Elemental,             { 24.0f,  24.0f }},
	{ Race::Puma,                  { 10.0f,  10.0f }},
	{ Race::NeriakCitizen,         { 3.0f,   3.0f }},
	{ Race::EruditeCitizen,        { 7.0f,   7.0f }},
	{ Race::Bixie,                 { 9.0f,   9.0f }},
	{ Race::ReanimatedHand,        { 11.0f,  11.0f }},
	{ Race::RivervaleCitizen,      { 2.5f,   2.5f }},
	{ Race::Scarecrow,             { 14.0f,  14.0f }},
	{ Race::Skunk,                 { 8.0f,   8.0f }},
	{ Race::SnakeElemental,        { 7.0f,   7.0f }},
	{ Race::Spectre,               { 12.0f,  12.0f }},
	{ Race::Sphinx,                { 27.0f,  27.0f }},
	{ Race::Armadillo,             { 2.0f,   2.0f }},
	{ Race::ClockworkGnome,        { 9.0f,   9.0f }},
	{ Race::Drake,                 { 9.0f,   9.0f }},
	{ Race::HalasCitizen,          { 9.0f,   9.0f }},
	{ Race::Alligator,             { 3.0f,   3.0f }},
	{ Race::GrobbCitizen,          { 3.0f,   3.0f }},
	{ Race::OggokCitizen,          { 10.0f,  10.0f }},
	{ Race::KaladimCitizen,        { 15.0f,  15.0f }},
	{ Race::CazicThule,            { 15.0f,  15.0f }},
	{ Race::Cockatrice,            { 9.0f,   9.0f }},
	{ Race::DaisyMan,              { 7.0f,   7.0f }},
	{ Race::ElfVampire,            { 7.0f,   7.0f }},
	{ Race::Denizen,               { 8.0f,   8.0f }},
	{ Race::Dervish,               { 3.0f,   3.0f }},
	{ Race::Efreeti,               { 3.0f,   3.0f }},
	{ Race::FroglokTadpole,        { 7.0f,   7.0f }},
	{ Race::PhinigelAutropos,      { 13.0f,  13.0f }},
	{ Race::Leech,                 { 9.0f,   9.0f }},
	{ Race::Swordfish,             { 5.0f,   5.0f }},
	{ Race::Felguard,              { 7.0f,   7.0f }},
	{ Race::Mammoth,               { 9.0f,   9.0f }},
	{ Race::EyeOfZomm,             { 8.0f,   8.0f }},
	{ Race::Wasp,                  { 5.5f,   5.5f }},
	{ Race::Mermaid,               { 4.0f,   4.0f }},
	{ Race::Harpy,                 { 25.0f,  25.0f }},
	{ Race::Fayguard,              { 22.0f,  22.0f }},
	{ Race::Drixie,                { 20.0f,  20.0f }},
	{ Race::GhostShip,             { 10.0f,  10.0f }},
	{ Race::Clam,                  { 13.5f,  13.5f }},
	{ Race::SeaHorse,              { 12.0f,  12.0f }},
	{ Race::DwarfGhost,            { 3.0f,   3.0f }},
	{ Race::EruditeGhost,          { 30.0f,  30.0f }},
	{ Race::Sabertooth,            { 35.0f,  35.0f }},
	{ Race::WolfElemental,         { 1.5f,   1.5f }},
	{ Race::Gorgon,                { 8.0f,   8.0f }},
	{ Race::DragonSkeleton,        { 3.0f,   3.0f }},
	{ Race::Innoruuk,              { 2.0f,   2.0f }},
	{ Race::Unicorn,               { 5.0f,   5.0f }},
	{ Race::Pegasus,               { 2.0f,   2.0f }},
	{ Race::Djinn,                 { 7.0f,   7.0f }},
	{ Race::InvisibleMan,          { 4.0f,   4.0f }},
	{ Race::Iksar,                 { 8.0f,   8.0f }},
	{ Race::Scorpion,              { 8.0f,   8.0f }},
	{ Race::VahShir,               { 7.0f,   7.0f }},
	{ Race::Sarnak,                { 8.0f,   8.0f }},
	{ Race::Draglock,              { 7.0f,   7.0f }},
	{ Race::Drolvarg,              { 7.0f,   7.0f }},
	{ Race::Mosquito,              { 10.0f,  10.0f }},
	{ Race::Rhinoceros,            { 3.0f,   3.0f }},
	{ Race::Xalgoz,                { 8.0f,   8.0f }},
	{ Race::KunarkGoblin,          { 9.0f,   9.0f }},
	{ Race::Yeti,                  { 15.0f,  15.0f }},
	{ Race::IksarCitizen,          { 5.0f,   5.0f }},
	{ Race::ForestGiant,           { 10.0f,  10.0f }},
	{ Race::Boat,                  { 7.0f,   7.0f }},
	{ Race::MinorIllusion,         { 7.0f,   7.0f }},
	{ Race::Tree,                  { 7.0f,   7.0f }},
	{ Race::Burynai,               { 7.0f,   7.0f }},
	{ Race::Goo,                   { 12.0f,  12.0f }},
	{ Race::SarnakSpirit,          { 4.0f,   4.0f }},
	{ Race::IksarSpirit,           { 5.0f,   5.0f }},
	{ Race::KunarkFish,            { 3.0f,   3.0f }},
	{ Race::IksarScorpion,         { 30.0f,  30.0f }},
	{ Race::Erollisi,              { 30.0f,  30.0f }},
	{ Race::Tribunal,              { 15.0f,  15.0f }},
	{ Race::Bertoxxulous,          { 20.0f,  20.0f }},
	{ Race::Bristlebane,           { 10.0f,  10.0f }},
	{ Race::FayDrake,              { 14.0f,  14.0f }},
	{ Race::UndeadSarnak,          { 14.0f,  14.0f }},
	{ Race::Ratman,                { 16.0f,  16.0f }},
	{ Race::Wyvern,                { 15.0f,  15.0f }},
	{ Race::Wurm,                  { 30.0f,  30.0f }},
	{ Race::Devourer,              { 15.0f,  15.0f }},
	{ Race::IksarGolem,            { 7.5f,   7.5f }},
	{ Race::UndeadIksar,           { 5.0f,   5.0f }},
	{ Race::ManEatingPlant,        { 4.0f,   4.0f }},
	{ Race::Raptor,                { 15.0f,  15.0f }},
	{ Race::SarnakGolem,           { 6.5f,   6.5f }},
	{ Race::WaterDragon,           { 3.0f,   3.0f }},
	{ Race::AnimatedHand,          { 12.0f,  12.0f }},
	{ Race::Succulent,             { 10.0f,  10.0f }},
	{ Race::Holgresh,              { 10.5f,  10.5f }},
	{ Race::Brontotherium,         { 10.0f,  10.0f }},
	{ Race::SnowDervish,           { 7.5f,   7.5f }},
	{ Race::DireWolf,              { 12.5f,  12.5f }},
	{ Race::Manticore,             { 9.0f,   9.0f }},
	{ Race::Totem,                 { 20.0f,  20.0f }},
	{ Race::IceSpectre,            { 2.0f,   2.0f }},
	{ Race::EnchantedArmor,        { 10.0f,  10.0f }},
	{ Race::SnowRabbit,            { 25.0f,  25.0f }},
	{ Race::Walrus,                { 8.0f,   8.0f }},
	{ Race::Geonid,                { 10.0f,  10.0f }},
	{ Race::Unknown,               { 18.0f,  18.0f }},
	{ Race::Unknown2,              { 45.0f,  45.0f }},
	{ Race::Yakkar,                { 13.0f,  13.0f }},
	{ Race::Faun,                  { 15.0f,  15.0f }},
	{ Race::Coldain,               { 8.0f,   8.0f }},
	{ Race::VeliousDragon,         { 30.0f,  30.0f }},
	{ Race::Hag,                   { 25.0f,  25.0f }},
	{ Race::Hippogriff,            { 25.0f,  25.0f }},
	{ Race::Siren,                 { 10.0f,  10.0f }},
	{ Race::FrostGiant,            { 13.0f,  13.0f }},
	{ Race::StormGiant,            { 5.0f,   5.0f }},
	{ Race::Othmir,                { 3.5f,   3.5f }},
	{ Race::Ulthork,               { 15.0f,  15.0f }},
	{ Race::ClockworkDragon,       { 35.0f,  35.0f }},
	{ Race::Abhorrent,             { 11.0f,  11.0f }},
	{ Race::SeaTurtle,             { 15.0f,  15.0f }},
	{ Race::BlackAndWhiteDragon,   { 50.0f,  50.0f }},
	{ Race::GhostDragon,           { 13.0f,  13.0f }},
	{ Race::RonnieTest,            { 7.0f,   7.0f }},
	{ Race::PrismaticDragon,       { 60.0f,  60.0f }},
	{ Race::Shiknar,               { 22.0f,  22.0f }},
	{ Race::Rockhopper,            { 22.0f,  22.0f }},
	{ Race::Underbulk,             { 21.0f,  21.0f }},
	{ Race::Grimling,              { 22.0f,  22.0f }},
	{ Race::Worm,                  { 15.0f,  15.0f }},
	{ Race::EvanTest,              { 25.0f,  25.0f }},
	{ Race::KhatiSha,              { 23.0f,  23.0f }},
	{ Race::Owlbear,               { 8.0f,   8.0f }},
	{ Race::RhinoBeetle,           { 15.0f,  15.0f }},
	{ Race::Vampire2,              { 10.0f,  10.0f }},
	{ Race::EarthElemental,        { 7.0f,   7.0f }},
	{ Race::AirElemental,          { 12.0f,  12.0f }},
	{ Race::WaterElemental,        { 9.5f,   9.5f }},
	{ Race::FireElemental,         { 12.0f,  12.0f }},
	{ Race::WetfangMinnow,         { 12.0f,  12.0f }},
	{ Race::ThoughtHorror,         { 12.0f,  12.0f }},
	{ Race::Tegi,                  { 15.0f,  15.0f }},
	{ Race::Horse,                 { 4.0f,   4.0f }},
	{ Race::Shissar,               { 5.0f,   5.0f }},
	{ Race::FungalFiend,           { 105.0f, 105.0f }},
	{ Race::VampireVolatalis,      { 20.0f,  20.0f }},
	{ Race::Stonegrabber,          { 5.0f,   5.0f }},
	{ Race::ScarletCheetah,        { 10.0f,  10.0f }},
	{ Race::Zelniak,               { 10.0f,  10.0f }},
	{ Race::Lightcrawler,          { 10.0f,  10.0f }},
	{ Race::Shade,                 { 20.0f,  20.0f }},
	{ Race::Sunflower,             { 13.5f,  13.5f }},
	{ Race::Shadel,                { 8.0f,   8.0f }},
	{ Race::Shrieker,              { 10.0f,  10.0f }},
	{ Race::Galorian,              { 3.0f,   3.0f }},
	{ Race::Netherbian,            { 5.0f,   5.0f }},
	{ Race::Akhevan,               { 9.0f,   9.0f }},
	{ Race::GriegVeneficus,        { 10.0f,  10.0f }},
	{ Race::SonicWolf,             { 8.0f,   8.0f }},
	{ Race::GroundShaker,          { 8.0f,   8.0f }},
	{ Race::VahShirSkeleton,       { 8.0f,   8.0f }},
	{ Race::Wretch,                { 5.0f,   5.0f }},
	{ Race::LordInquisitorSeru,    { 5.0f,   5.0f }},
	{ Race::Recuso,                { 5.0f,   5.0f }},
	{ Race::VahShirKing,           { 9.0f,   9.0f }},
	{ Race::VahShirGuard,          { 9.0f,   9.0f }},
	{ Race::TeleportMan,           { 9.0f,   9.0f }},
	{ Race::Werewolf2,             { 8.5f,   8.5f }},
	{ Race::Nymph,                 { 7.0f,   7.0f }},
	{ Race::Dryad,                 { 8.0f,   8.0f }},
	{ Race::Treant2,               { 7.0f,   7.0f }},
	{ Race::Fly,                   { 11.0f,  11.0f }},
	{ Race::TarewMarr,             { 7.0f,   7.0f }},
	{ Race::SolusekRo2,            { 9.0f,   9.0f }},
	{ Race::ClockworkGolem,        { 8.0f,   8.0f }},
	{ Race::ClockworkBrain,        { 8.0f,   8.0f }},
	{ Race::Banshee,               { 9.0f,   9.0f }},
	{ Race::GuardOfJustice,        { 10.0f,  10.0f }},
	{ Race::MiniPom,               { 3.0f,   3.0f }},
	{ Race::DiseasedFiend,         { 4.0f,   4.0f }},
	{ Race::SolusekRoGuard,        { 3.0f,   3.0f }},
	{ Race::BertoxxulousNew,       { 3.0f,   3.0f }},
	{ Race::TribunalNew,           { 4.0f,   4.0f }},
	{ Race::TerrisThule,           { 10.0f,  10.0f }},
	{ Race::Vegerog,               { 10.0f,  10.0f }},
	{ Race::Crocodile,             { 2.0f,   2.0f }},
	{ Race::Bat,                   { 8.0f,   8.0f }},
	{ Race::Hraquis,               { 14.0f,  14.0f }},
	{ Race::Tranquilion,           { 7.0f,   7.0f }},
	{ Race::TinSoldier,            { 5.0f,   5.0f }},
	{ Race::NightmareWraith,       { 9.0f,   9.0f }},
	{ Race::Malarian,              { 7.0f,   7.0f }},
	{ Race::KnightOfPestilence,    { 7.0f,   7.0f }},
	{ Race::Lepertoloth,           { 10.0f,  10.0f }},
	{ Race::Bubonian,              { 10.0f,  10.0f }},
	{ Race::BubonianUnderling,     { 12.0f,  12.0f }},
	{ Race::Pusling,               { 9.0f,   9.0f }},
	{ Race::WaterMephit,           { 7.0f,   7.0f }},
	{ Race::Stormrider,            { 12.0f,  12.0f }},
	{ Race::JunkBeast,             { 13.0f,  13.0f }},
	{ Race::BrokenClockwork,       { 16.0f,  16.0f }},
	{ Race::GiantClockwork,        { 9.0f,   9.0f }},
	{ Race::ClockworkBeetle,       { 10.0f,  10.0f }},
	{ Race::NightmareGoblin,       { 25.0f,  25.0f }},
	{ Race::Karana,                { 15.0f,  15.0f }},
	{ Race::BloodRaven,            { 25.0f,  25.0f }},
	{ Race::NightmareGargoyle,     { 8.0f,   8.0f }},
	{ Race::MouthOfInsanity,       { 11.0f,  11.0f }},
	{ Race::SkeletalHorse,         { 9.0f,   9.0f }},
	{ Race::Saryrn,                { 2.0f,   2.0f }},
	{ Race::FenninRo,              { 5.0f,   5.0f }},
	{ Race::Tormentor,             { 4.0f,   4.0f }},
	{ Race::SoulDevourer,          { 8.5f,   8.5f }},
	{ Race::Nightmare,             { 4.0f,   4.0f }},
	{ Race::NewRallosZek,          { 15.0f,  15.0f }},
	{ Race::VallonZek,             { 1.0f,   1.0f }},
	{ Race::TallonZek,             { 2.0f,   2.0f }},
	{ Race::AirMephit,             { 40.0f,  40.0f }},
	{ Race::EarthMephit,           { 8.0f,   8.0f }},
	{ Race::FireMephit,            { 12.0f,  12.0f }},
	{ Race::NightmareMephit,       { 3.0f,   3.0f }},
	{ Race::Zebuxoruk,             { 8.0f,   8.0f }},
	{ Race::MithanielMarr,         { 99.0f,  99.0f }},
	{ Race::UndeadKnight,          { 9.0f,   9.0f }},
	{ Race::Rathe,                 { 100.0f, 100.0f }},
	{ Race::Xegony,                { 100.0f, 100.0f }},
	{ Race::Fiend,                 { 10.0f,  10.0f }},
	{ Race::TestObject,            { 27.5f,  27.5f }},
	{ Race::Crab,                  { 20.0f,  20.0f }},
	{ Race::Phoenix,               { 5.0f,   5.0f }},
	{ Race::Quarm,                 { 8.0f,   8.0f }},
	{ Race::Bear2,                 { 5.0f,   5.0f }},
	{ Race::EarthGolem,            { 3.0f,   3.0f }},
	{ Race::IronGolem,             { 11.5f,  11.5f }},
	{ Race::StormGolem,            { 25.0f,  25.0f }},
	{ Race::AirGolem,              { 80.0f,  80.0f }},
	{ Race::WoodGolem,             { 20.0f,  20.0f }},
	{ Race::FireGolem,             { 9.0f,   9.0f }},
	{ Race::WaterGolem,            { 8.0f,   8.0f }},
	{ Race::WarWraith,             { 5.0f,   5.0f }},
	{ Race::Wrulon,                { 4.0f,   4.0f }},
	{ Race::Kraken,                { 7.0f,   7.0f }},
	{ Race::PoisonFrog,            { 10.0f,  10.0f }},
	{ Race::Nilborien,             { 11.0f,  11.0f }},
	{ Race::Valorian,              { 8.0f,   8.0f }},
	{ Race::WarBoar,               { 5.0f,   5.0f }},
	{ Race::Efreeti2,              { 30.0f,  30.0f }},
	{ Race::WarBoar2,              { 7.0f,   7.0f }},
	{ Race::Valorian2,             { 15.0f,  15.0f }},
	{ Race::AnimatedArmor,         { 9.0f,   9.0f }},
	{ Race::UndeadFootman,         { 9.0f,   9.0f }},
	{ Race::RallosOgre,            { 3.0f,   3.0f }},
	{ Race::Arachnid,              { 32.5f,  32.5f }},
	{ Race::CrystalSpider,         { 15.0f,  15.0f }},
	{ Race::ZebuxoruksCage,        { 7.5f,   7.5f }},
	{ Race::Portal,                { 10.0f,  10.0f }},
	{ Race::Froglok2,              { 10.0f,  10.0f }},
	{ Race::TrollCrewMember,       { 9.0f,   9.0f }},
	{ Race::PirateDeckhand,        { 20.0f,  20.0f }},
	{ Race::BrokenSkullPirate,     { 25.0f,  25.0f }},
	{ Race::PirateGhost,           { 12.0f,  12.0f }},
	{ Race::OneArmedPirate,        { 8.0f,   8.0f }},
	{ Race::SpiritmasterNadox,     { 20.0f,  20.0f }},
	{ Race::BrokenSkullTaskmaster, { 9.0f,   10.0f }},
	{ Race::GnomePirate,           { 8.0f,   8.0f }},
	{ Race::DarkElfPirate,         { 12.0f,  12.0f }},
	{ Race::OgrePirate,            { 8.0f,   8.0f }},
	{ Race::HumanPirate,           { 2.0f,   2.0f }},
	{ Race::EruditePirate,         { 3.0f,   3.0f }},
	{ Race::Frog,                  { 7.0f,   7.0f }},
	{ Race::TrollZombie,           { 1.5f,   1.5f }},
	{ Race::Luggald,               { 3.0f,   3.0f }},
	{ Race::Luggald2,              { 3.0f,   3.0f }},
	{ Race::Luggald3,              { 3.0f,   3.0f }},
	{ Race::Drogmor,               { 3.0f,   3.0f }},
	{ Race::FroglokSkeleton,       { 2.0f,   2.0f }},
	{ Race::UndeadFroglok,         { 3.0f,   3.0f }},
	{ Race::KnightOfHate,          { 3.0f,   3.0f }},
	{ Race::ArcanistOfHate,        { 4.5f,   4.5f }},
	{ Race::Veksar,                { 7.0f,   7.0f }},
	{ Race::Veksar2,               { 7.0f,   7.0f }},
	{ Race::Veksar3,               { 5.7f,   22.0f }},
	{ Race::Chokidai,              { 22.0f,  8.0f }},
	{ Race::UndeadChokidai,        { 8.0f,   15.0f }},
	{ Race::UndeadVeksar,          { 15.0f,  22.0f }},
	{ Race::UndeadVampire,         { 22.0f,  8.0f }},
	{ Race::Vampire3,              { 8.0f,   15.0f }},
	{ Race::RujarkianOrc,          { 15.0f,  80.0f }},
	{ Race::BoneGolem,             { 80.0f,  150.0f }},
	{ Race::Synarcana,             { 150.0f, 7.0f }},
	{ Race::SandElf,               { 7.0f,   12.0f }},
	{ Race::MasterVampire,         { 12.0f,  35.0f }},
	{ Race::MasterOrc,             { 35.0f,  20.0f }},
	{ Race::Skeleton2,             { 20.0f,  9.0f }},
	{ Race::Mummy,                 { 9.0f,   20.0f }},
	{ Race::NewGoblin,             { 20.0f,  20.0f }},
	{ Race::Insect,                { 20.0f,  20.0f }},
	{ Race::FroglokGhost,          { 20.0f,  20.0f }},
	{ Race::Dervish2,              { 20.0f,  20.0f }},
	{ Race::Shade2,                { 20.0f,  9.0f }},
	{ Race::Golem2,                { 9.0f,   4.0f }},
	{ Race::EvilEye2,              { 4.0f,   4.0f }},
	{ Race::Box,                   { 4.0f,   10.0f }},
	{ Race::Barrel,                { 10.0f,  5.0f }},
	{ Race::Chest,                 { 5.0f,   8.0f }},
	{ Race::Vase,                  { 8.0f,   10.0f }},
	{ Race::Table,                 { 10.0f,  2.0f }},
	{ Race::WeaponRack,            { 5.7f,   36.0f }},
	{ Race::Coffin,                { 2.0f,   14.0f }},
	{ Race::Bones,                 { 36.0f,  7.0f }},
	{ Race::Jokester,              { 14.0f,  250.0f }},
	{ Race::Nihil,                 { 7.0f,   9.0f }},
	{ Race::Trusik,                { 250.0f, 7.0f }},
	{ Race::StoneWorker,           { 9.0f,   4.0f }},
	{ Race::Hynid,                 { 7.0f,   8.0f }},
	{ Race::Turepta,               { 4.0f,   23.0f }},
	{ Race::Cragbeast,             { 8.0f,   70.0f }},
	{ Race::Stonemite,             { 23.0f,  7.0f }},
	{ Race::Ukun,                  { 70.0f,  20.0f }},
	{ Race::Ixt,                   { 7.0f,   5.0f }},
	{ Race::Ikaav,                 { 20.0f,  1.0f }},
	{ Race::Aneuk,                 { 5.0f,   4.0f }},
	{ Race::Kyv,                   { 1.0f,   4.0f }},
	{ Race::Noc,                   { 4.0f,   10.0f }},
	{ Race::Ratuk,                 { 4.0f,   7.0f }},
	{ Race::Taneth,                { 10.0f,  7.0f }},
	{ Race::Huvul,                 { 7.0f,   7.0f }},
	{ Race::Mutna,                 { 7.0f,   7.0f }},
	{ Race::Mastruq,               { 7.0f,   8.0f }},
	{ Race::Taelosian,             { 7.0f,   7.0f }},
	{ Race::DiscordShip,           { 8.0f,   7.5f }},
	{ Race::StoneWorker2,          { 7.0f,   4.0f }},
	{ Race::MataMuram,             { 7.5f,   3.0f }},
	{ Race::LightingWarrior,       { 4.0f,   1.0f }},
	{ Race::Succubus,              { 3.0f,   9.0f }},
	{ Race::Bazu,                  { 1.0f,   7.0f }},
	{ Race::Feran,                 { 9.0f,   8.0f }},
	{ Race::Pyrilen,               { 7.0f,   7.0f }},
	{ Race::Chimera,               { 8.0f,   8.0f }},
	{ Race::Dragorn,               { 7.0f,   8.0f }},
};

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

#endif

