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
		case RACE_HUMAN_1:
		case RACE_HIGHPASS_CITIZEN_67:
		case RACE_QEYNOS_CITIZEN_71:
			return "Human";
		case RACE_BARBARIAN_2:
		case RACE_HALAS_CITIZEN_90:
			return "Barbarian";
		case RACE_ERUDITE_3:
		case RACE_ERUDITE_CITIZEN_78:
			return "Erudite";
		case RACE_WOOD_ELF_4:
			return "Wood Elf";
		case RACE_HIGH_ELF_5:
			return "High Elf";
		case RACE_DARK_ELF_6:
		case RACE_NERIAK_CITIZEN_77:
			return "Dark Elf";
		case RACE_HALF_ELF_7:
			return "Half Elf";
		case RACE_DWARF_8:
		case RACE_KALADIM_CITIZEN_94:
			return "Dwarf";
		case RACE_TROLL_9:
		case RACE_GROBB_CITIZEN_92:
			return "Troll";
		case RACE_OGRE_10:
		case RACE_OGGOK_CITIZEN_93:
			return "Ogre";
		case RACE_HALFLING_11:
		case RACE_RIVERVALE_CITIZEN_81:
			return "Halfling";
		case RACE_GNOME_12:
			return "Gnome";
		case RACE_AVIAK_13:
			return "Aviak";
		case RACE_WEREWOLF_14:
		case RACE_WEREWOLF_454:
			return "Werewolf";
		case RACE_BROWNIE_15:
			return "Brownie";
		case RACE_CENTAUR_16:
			return "Centaur";
		case RACE_GOLEM_17:
			return "Golem";
		case RACE_GIANT_18:
			return "Giant";
		case RACE_TRAKANON_19:
			return "Trakanon";
		case RACE_VENRIL_SATHIR_20:
			return "Venril Sathir";
		case RACE_EVIL_EYE_21:
			return "Evil Eye";
		case RACE_BEETLE_22:
			return "Beetle";
		case RACE_KERRAN_23:
			return "Kerran";
		case RACE_FISH_24:
			return "Fish";
		case RACE_FAIRY_25:
			return "Fairy";
		case RACE_FROGLOK_26:
		case RACE_FROGLOK_GHOUL_27:
		case RACE_FROGLOK_330:
			return "Froglok";
		case RACE_FUNGUSMAN_28:
			return "Fungusman";
		case RACE_GARGOYLE_29:
		case RACE_GARGOYLE_464:
			return "Gargoyle";
		case RACE_GASBAG_30:
			return "Gasbag";
		case RACE_GELATINOUS_CUBE_31:
			return "Gelatinous Cube";
		case RACE_GHOST_32:
		case RACE_DWARF_GHOST_117:
		case RACE_ERUDITE_GHOST_118:
			return "Ghost";
		case RACE_GHOUL_33:
		case RACE_GHOUL_571:
			return "Ghoul";
		case RACE_GIANT_BAT_34:
		case RACE_BAT_260:
		case RACE_BAT_416:
			return "Bat";
		case RACE_GIANT_EEL_35:
			return "Eel";
		case RACE_GIANT_RAT_36:
		case RACE_RAT_415:
			return "Rat";
		case RACE_GIANT_SNAKE_37:
		case RACE_SNAKE_468:
			return "Snake";
		case RACE_GIANT_SPIDER_38:
		case RACE_SPIDER_440:
			return "Spider";
		case RACE_GNOLL_39:
		case RACE_GNOLL_524:
		case RACE_GNOLL_617:
			return "Gnoll";
		case RACE_GOBLIN_40:
		case RACE_BLOODGILL_59:
		case RACE_KUNARK_GOBLIN_137:
		case RACE_NEW_GOBLIN_369:
			return "Goblin";
		case RACE_GORILLA_41:
			return "Gorilla";
		case RACE_WOLF_42:
		case RACE_WOLF_ELEMENTAL_120:
		case RACE_WOLF_482:
			return "Wolf";
		case RACE_BEAR_43:
			return "Bear";
		case RACE_FREEPORT_GUARD_44:
		case RACE_FELGUARD_106:
		case RACE_FAYGUARD_112:
			return "Guard";
		case RACE_DEMI_LICH_45:
			return "Demi Lich";
		case RACE_IMP_46:
			return "Imp";
		case RACE_GRIFFIN_47:
		case RACE_GRIFFIN_525:
			return "Griffin";
		case RACE_KOBOLD_48:
		case RACE_KOBOLD_455:
			return "Kobold";
		case RACE_LAVA_DRAGON_49:
		case RACE_DRAGON_SKELETON_122:
			return "Dragon";
		case RACE_LION_50:
			return "Lion";
		case RACE_LIZARD_MAN_51:
			return "Lizard Man";
		case RACE_MIMIC_52:
			return "Mimic";
		case RACE_MINOTAUR_53:
		case RACE_MINOTAUR_420:
		case RACE_MINOTAUR_470:
		case RACE_MINOTAUR_574:
			return "Minotaur";
		case RACE_ORC_54:
		case RACE_ORC_458:
			return "Orc";
		case RACE_HUMAN_BEGGAR_55:
			return "Beggar";
		case RACE_PIXIE_56:
			return "Pixie";
		case RACE_DRACHNID_57:
		case RACE_DRACHNID_461:
			return "Drachnid";
		case RACE_SOLUSEK_RO_58:
		case RACE_SOLUSEK_RO_247:
			return "Solusek Ro";
		case RACE_SKELETON_60:
		case RACE_NEW_SKELETON_367:
		case RACE_SKELETON_484:
			return "Skeleton";
		case RACE_SHARK_61:
			return "Shark";
		case RACE_TUNARE_62:
			return "Tunare";
		case RACE_TIGER_63:
			return "Tiger";
		case RACE_TREANT_64:
		case RACE_TREANT_496:
			return "Treant";
		case RACE_VAMPIRE_65:
		case RACE_ELF_VAMPIRE_98:
		case RACE_VAMPYRE_208:
		case RACE_UNDEAD_VAMPIRE_359:
		case RACE_VAMPIRE_360:
		case RACE_MASTER_VAMPIRE_365:
		case RACE_VAMPIRE_497:
			return "Vampire";
		case RACE_STATUE_OF_RALLOS_ZEK_66:
		case RACE_NEW_RALLOS_ZEK_288:
			return "Rallos Zek";
		case RACE_TENTACLE_68:
			return "Tentacle Terror";
		case RACE_WISP_69:
			return "Will-O-Wisp";
		case RACE_ZOMBIE_70:
		case RACE_ZOMBIE_471:
			return "Zombie";
		case RACE_SHIP_72:
			return "Ship";
		case RACE_LAUNCH_73:
			return "Launch";
		case RACE_PIRANHA_74:
			return "Piranha";
		case RACE_ELEMENTAL_75:
			return "Elemental";
		case RACE_PUMA_76:
		case RACE_PUMA_439:
		case RACE_PUMA_584:
			return "Puma";
		case RACE_BIXIE_79:
		case RACE_BIXIE_520:
			return "Bixie";
		case RACE_REANIMATED_HAND_80:
			return "Reanimated Hand";
		case RACE_SCARECROW_82:
		case RACE_SCARECROW_575:
			return "Scarecrow";
		case RACE_SKUNK_83:
			return "Skunk";
		case RACE_SNAKE_ELEMENTAL_84:
			return "Snake Elemental";
		case RACE_SPECTRE_85:
		case RACE_SPECTRE_485:
			return "Spectre";
		case RACE_SPHINX_86:
		case RACE_SPHINX_565:
			return "Sphinx";
		case RACE_ARMADILLO_87:
			return "Armadillo";
		case RACE_CLOCKWORK_GNOME_88:
			return "Clockwork Gnome";
		case RACE_DRAKE_89:
		case RACE_DRAKE_430:
		case RACE_DRAKE_432:
			return "Drake";
		case RACE_ALLIGATOR_91:
		case RACE_ALLIGATOR_479:
			return "Alligator";
		case RACE_CAZIC_THULE_95:
		case RACE_CAZIC_THULE_670:
			return "Cazic Thule";
		case RACE_COCKATRICE_96:
			return "Cockatrice";
		case RACE_DAISY_MAN_97:
			return "Daisy Man";
		case RACE_DENIZEN_99:
		case RACE_AMYGDALAN_663:
			return "Amygdalan";
		case RACE_DERVISH_100:
			return "Dervish";
		case RACE_EFREETI_101:
			return "Efreeti";
		case RACE_FROGLOK_TADPOLE_102:
			return "Tadpole";
		case RACE_PHINIGEL_AUTROPOS_103:
		case RACE_KEDGE_561:
			return "Kedge";
		case RACE_LEECH_104:
			return "Leech";
		case RACE_SWORDFISH_105:
			return "Swordfish";
		case RACE_MAMMOTH_107:
		case RACE_MAMMOTH_528:
			return "Mammoth";
		case RACE_EYE_OF_ZOMM_108:
			return "Eye";
		case RACE_WASP_109:
			return "Wasp";
		case RACE_MERMAID_110:
			return "Mermaid";
		case RACE_HARPIE_111:
		case RACE_HARPY_527:
			return "Harpy";
		case RACE_DRIXIE_113:
			return "Drixie";
		case RACE_GHOST_SHIP_114:
		case RACE_GHOST_SHIP_552:
			return "Ghost Ship";
		case RACE_CLAM_115:
			return "Clam";
		case RACE_SEA_HORSE_116:
			return "Seahorse";
		case RACE_SABERTOOTH_119:
			return "Saber-toothed Cat";
		case RACE_GORGON_121:
			return "Gorgon";
		case RACE_INNORUUK_123:
			return "Innoruuk";
		case RACE_UNICORN_124:
		case RACE_UNICORN_519:
			return "Unicorn";
		case RACE_PEGASUS_125:
		case RACE_PEGASUS_493:
		case RACE_PEGASUS_732:
			return "Pegasus";
		case RACE_DJINN_126:
			return "Djinn";
		case RACE_INVISIBLE_MAN_127:
			return "Invisible Man";
		case RACE_IKSAR_128:
			return "Iksar";
		case RACE_SCORPION_129:
			return "Scorpion";
		case RACE_VAH_SHIR_130:
			return "Vah Shir";
		case RACE_SARNAK_131:
		case RACE_SARNAK_610:
			return "Sarnak";
		case RACE_DRAGLOCK_132:
		case RACE_DRAKKIN_522:
			return "Drakkin";
		default:
			return "UNKNOWN RACE";
	}
}

uint32 GetPlayerRaceValue(uint16 race_id) {
	switch (race_id) {
		case HUMAN:
		case BARBARIAN:
		case ERUDITE:
		case WOOD_ELF:
		case HIGH_ELF:
		case DARK_ELF:
		case HALF_ELF:
		case DWARF:
		case TROLL:
		case OGRE:
		case HALFLING:
		case GNOME:
			return race_id;
		case IKSAR:
			return PLAYER_RACE_IKSAR;
		case VAHSHIR:
			return PLAYER_RACE_VAHSHIR;
		case FROGLOK:
		case FROGLOK2:
			return PLAYER_RACE_FROGLOK;
		case DRAKKIN:
			return PLAYER_RACE_DRAKKIN;
		default:
			return PLAYER_RACE_UNKNOWN;
	}
}

uint32 GetPlayerRaceBit(uint16 race_id) {
	switch (race_id) {
		case HUMAN:
			return PLAYER_RACE_HUMAN_BIT;
		case BARBARIAN:
			return PLAYER_RACE_BARBARIAN_BIT;
		case ERUDITE:
			return PLAYER_RACE_ERUDITE_BIT;
		case WOOD_ELF:
			return PLAYER_RACE_WOOD_ELF_BIT;
		case HIGH_ELF:
			return PLAYER_RACE_HIGH_ELF_BIT;
		case DARK_ELF:
			return PLAYER_RACE_DARK_ELF_BIT;
		case HALF_ELF:
			return PLAYER_RACE_HALF_ELF_BIT;
		case DWARF:
			return PLAYER_RACE_DWARF_BIT;
		case TROLL:
			return PLAYER_RACE_TROLL_BIT;
		case OGRE:
			return PLAYER_RACE_OGRE_BIT;
		case HALFLING:
			return PLAYER_RACE_HALFLING_BIT;
		case GNOME:
			return PLAYER_RACE_GNOME_BIT;
		case IKSAR:
			return PLAYER_RACE_IKSAR_BIT;
		case VAHSHIR:
			return PLAYER_RACE_VAHSHIR_BIT;
		case FROGLOK:
			return PLAYER_RACE_FROGLOK_BIT;
		case DRAKKIN:
			return PLAYER_RACE_DRAKKIN_BIT;
		default:
			return PLAYER_RACE_UNKNOWN_BIT;
	}
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

	if (gender == FEMALE) {
		return female_height[race];
	}

	return male_height[race];
}

// PlayerAppearance prep
#define HUMAN_MALE ((HUMAN << 8) | MALE)
#define HUMAN_FEMALE ((HUMAN << 8) | FEMALE)
#define BARBARIAN_MALE ((BARBARIAN << 8) | MALE)
#define BARBARIAN_FEMALE ((BARBARIAN << 8) | FEMALE)
#define ERUDITE_MALE ((ERUDITE << 8) | MALE)
#define ERUDITE_FEMALE ((ERUDITE << 8) | FEMALE)
#define WOOD_ELF_MALE ((WOOD_ELF << 8) | MALE)
#define WOOD_ELF_FEMALE ((WOOD_ELF << 8) | FEMALE)
#define HIGH_ELF_MALE ((HIGH_ELF << 8) | MALE)
#define HIGH_ELF_FEMALE ((HIGH_ELF << 8) | FEMALE)
#define DARK_ELF_MALE ((DARK_ELF << 8) | MALE)
#define DARK_ELF_FEMALE ((DARK_ELF << 8) | FEMALE)
#define HALF_ELF_MALE ((HALF_ELF << 8) | MALE)
#define HALF_ELF_FEMALE ((HALF_ELF << 8) | FEMALE)
#define DWARF_MALE ((DWARF << 8) | MALE)
#define DWARF_FEMALE ((DWARF << 8) | FEMALE)
#define TROLL_MALE ((TROLL << 8) | MALE)
#define TROLL_FEMALE ((TROLL << 8) | FEMALE)
#define OGRE_MALE ((OGRE << 8) | MALE)
#define OGRE_FEMALE ((OGRE << 8) | FEMALE)
#define HALFLING_MALE ((HALFLING << 8) | MALE)
#define HALFLING_FEMALE ((HALFLING << 8) | FEMALE)
#define GNOME_MALE ((GNOME << 8) | MALE)
#define GNOME_FEMALE ((GNOME << 8) | FEMALE)
#define IKSAR_MALE ((IKSAR << 8) | MALE)
#define IKSAR_FEMALE ((IKSAR << 8) | FEMALE)
#define VAHSHIR_MALE ((VAHSHIR << 8) | MALE)
#define VAHSHIR_FEMALE ((VAHSHIR << 8) | FEMALE)
#define FROGLOK_MALE ((FROGLOK << 8) | MALE)
#define FROGLOK_FEMALE ((FROGLOK << 8) | FEMALE)
#define DRAKKIN_MALE ((DRAKKIN << 8) | MALE)
#define DRAKKIN_FEMALE ((DRAKKIN << 8) | FEMALE)

#define BINDRG(r, g) (((int)r << 8) | g)


bool PlayerAppearance::IsValidBeard(uint16 race_id, uint8 gender_id, uint8 beard_value, bool use_luclin)
{
	if (beard_value == 0xFF)
		return true;

	if (use_luclin) {
		switch (BINDRG(race_id, gender_id)) {
		case DWARF_FEMALE:
			if (beard_value <= 1)
				return true;
			break;
		case HIGH_ELF_MALE:
		case DARK_ELF_MALE:
		case HALF_ELF_MALE:
		case DRAKKIN_FEMALE:
			if (beard_value <= 3)
				return true;
			break;
		case HUMAN_MALE:
		case BARBARIAN_MALE:
		case ERUDITE_MALE:
		case DWARF_MALE:
		case HALFLING_MALE:
		case GNOME_MALE:
			if (beard_value <= 5)
				return true;
			break;
		case DRAKKIN_MALE:
			if (beard_value <= 11)
				return true;
			break;
		default:
			break;
		}
		return false;
	}
	else {
		switch (BINDRG(race_id, gender_id)) {
		case DRAKKIN_FEMALE:
			if (beard_value <= 3)
				return true;
			break;
		case DRAKKIN_MALE:
			if (beard_value <= 11)
				return true;
			break;
		default:
			break;
		}
		return false;
	}
}

bool PlayerAppearance::IsValidBeardColor(uint16 race_id, uint8 gender_id, uint8 beard_color_value, bool use_luclin)
{
	if (beard_color_value == 0xFF)
	return true;

	switch (BINDRG(race_id, gender_id)) {
	case GNOME_MALE:
		if (beard_color_value <= 24)
			return true;
		break;
	case HUMAN_MALE:
	case BARBARIAN_MALE:
	case ERUDITE_MALE:
	case HALF_ELF_MALE:
	case DWARF_MALE:
	case DWARF_FEMALE:
	case HALFLING_MALE:
		if (beard_color_value <= 19)
			return true;
		break;
	case DARK_ELF_MALE:
		if (beard_color_value >= 13 &&  beard_color_value <= 18)
			return true;
		break;
	case HIGH_ELF_MALE:
		if (beard_color_value <= 14)
			return true;
		break;
	case FROGLOK_MALE:
	case FROGLOK_FEMALE:
	case DRAKKIN_MALE:
	case DRAKKIN_FEMALE:
		if (beard_color_value <= 3)
			return true;
		break;
	default:
		break;
	}
	return false;
}

bool PlayerAppearance::IsValidDetail(uint16 race_id, uint8 gender_id, uint32 detail_value, bool use_luclin)
{
	if (detail_value == 0xFFFFFFFF)
		return true;

	switch (BINDRG(race_id, gender_id)) {
	case DRAKKIN_MALE:
	case DRAKKIN_FEMALE:
		if (detail_value <= 7)
			return true;
		break;
	default:
		break;
	}
	return false;
}

bool PlayerAppearance::IsValidEyeColor(uint16 race_id, uint8 gender_id, uint8 eye_color_value, bool use_luclin)
{
	return true; // need valid criteria

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
	case VAHSHIR_MALE:
	case VAHSHIR_FEMALE:
		if (eye_color_value <= 9)
			return true;
		break;
	case TROLL_MALE:
	case TROLL_FEMALE:
		if (eye_color_value <= 10)
			return true;
		break;
	case FROGLOK_MALE:
	case FROGLOK_FEMALE:
	case DRAKKIN_MALE:
	case DRAKKIN_FEMALE:
		if (eye_color_value <= 11)
			return true;
		break;
	default:
		break;
	}
	return false;
}

bool PlayerAppearance::IsValidFace(uint16 race_id, uint8 gender_id, uint8 face_value, bool use_luclin)
{
	if (face_value == 0xFF)
		return true;

	switch (BINDRG(race_id, gender_id)) {
	case DRAKKIN_MALE:
	case DRAKKIN_FEMALE:
		if (face_value <= 6)
			return true;
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
	case VAHSHIR_MALE:
	case VAHSHIR_FEMALE:
		if (face_value <= 7)
			return true;
		break;
	case FROGLOK_MALE:
	case FROGLOK_FEMALE:
		if (face_value <= 9)
			return true;
		break;
	default:
		break;
	}
	return false;
}

bool PlayerAppearance::IsValidHair(uint16 race_id, uint8 gender_id, uint8 hair_value, bool use_luclin)
{
	if (hair_value == 0xFF)
		return true;

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
			if (hair_value <= 3)
				return true;
			break;
		case ERUDITE_MALE:
			if (hair_value <= 5)
				return true;
			break;
		case DRAKKIN_FEMALE:
			if (hair_value <= 7)
				return true;
			break;
		case ERUDITE_FEMALE:
		case DRAKKIN_MALE:
			if (hair_value <= 8)
				return true;
			break;
		default:
			break;
		}
		return false;
	}
	else {
		switch (BINDRG(race_id, gender_id)) {
		case DRAKKIN_FEMALE:
			if (hair_value <= 7)
				return true;
			break;
		case DRAKKIN_MALE:
			if (hair_value <= 8)
				return true;
			break;
		default:
			break;
		}
		return false;
	}
}

bool PlayerAppearance::IsValidHairColor(uint16 race_id, uint8 gender_id, uint8 hair_color_value, bool use_luclin)
{
	if (hair_color_value == 0xFF)
		return true;

	switch (BINDRG(race_id, gender_id)) {
	case GNOME_MALE:
	case GNOME_FEMALE:
		if (hair_color_value <= 24)
			return true;
		break;
	case TROLL_FEMALE:
	case OGRE_FEMALE:
		if (hair_color_value <= 23)
			return true;
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
		if (hair_color_value <= 19)
			return true;
		break;
	case DARK_ELF_MALE:
	case DARK_ELF_FEMALE:
		if (hair_color_value >= 13 && hair_color_value <= 18)
			return true;
		break;
	case HIGH_ELF_MALE:
	case HIGH_ELF_FEMALE:
		if (hair_color_value <= 14)
			return true;
		break;
	case FROGLOK_MALE:
	case FROGLOK_FEMALE:
	case DRAKKIN_MALE:
	case DRAKKIN_FEMALE:
		if (hair_color_value <= 3)
			return true;
		break;
	default:
		break;
	}
	return false;
}

bool PlayerAppearance::IsValidHead(uint16 race_id, uint8 gender_id, uint8 head_value, bool use_luclin)
{
	if (head_value == 0xFF)
		return true;

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
		case VAHSHIR_MALE:
		case VAHSHIR_FEMALE:
		case FROGLOK_MALE:
		case FROGLOK_FEMALE:
		case DRAKKIN_MALE:
		case DRAKKIN_FEMALE:
			if (head_value <= 3)
				return true;
			break;
		case ERUDITE_MALE:
		case ERUDITE_FEMALE:
			if (head_value <= 4)
				return true;
			break;
		default:
			break;
		}
		return false;
	}
	else {
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
		case VAHSHIR_MALE:
		case VAHSHIR_FEMALE:
		case FROGLOK_MALE:
		case FROGLOK_FEMALE:
		case DRAKKIN_MALE:
		case DRAKKIN_FEMALE:
			if (head_value <= 3)
				return true;
			break;
		case GNOME_MALE:
		case GNOME_FEMALE:
			if (head_value <= 4)
				return true;
			break;
		default:
			break;
		}
		return false;
	}
}

bool PlayerAppearance::IsValidHeritage(uint16 race_id, uint8 gender_id, uint32 heritage_value, bool use_luclin)
{
	if (heritage_value == 0xFFFFFFFF)
		return true;

	switch (BINDRG(race_id, gender_id)) {
	case DRAKKIN_MALE:
	case DRAKKIN_FEMALE:
		if (heritage_value <= 7) // > 5 seems to jumble other features..else, some heritages have 'specialized' features
			return true;
		break;
	default:
		break;
	}
	return false;
}

bool PlayerAppearance::IsValidTattoo(uint16 race_id, uint8 gender_id, uint32 tattoo_value, bool use_luclin)
{
	if (tattoo_value == 0xFFFFFFFF)
		return true;

	switch (BINDRG(race_id, gender_id)) {
	case DRAKKIN_MALE:
	case DRAKKIN_FEMALE:
		if (tattoo_value <= 7)
			return true;
		break;
	default:
		break;
	}
	return false;
}

bool PlayerAppearance::IsValidTexture(uint16 race_id, uint8 gender_id, uint8 texture_value, bool use_luclin)
{
	if (texture_value == 0xFF)
		return true;

	if (use_luclin) {
		switch (BINDRG(race_id, gender_id)) {
		case HUMAN_MALE:
		case HUMAN_FEMALE:
		case IKSAR_MALE:
		case IKSAR_FEMALE:
		case DRAKKIN_MALE:
		case DRAKKIN_FEMALE:
			if ((texture_value >= 10 && texture_value <= 16) || texture_value <= 4)
				return true;
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
			if ((texture_value >= 10 && texture_value <= 16) || texture_value <= 3)
				return true;
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
		case VAHSHIR_MALE:
		case VAHSHIR_FEMALE:
			if (texture_value <= 3)
				return true;
			break;
		default:
			break;
		}
		return false;
	}
	else {
		switch (BINDRG(race_id, gender_id)) {
		case HUMAN_MALE:
		case HUMAN_FEMALE:
		case ERUDITE_MALE:
		case ERUDITE_FEMALE:
		case DRAKKIN_MALE:
		case DRAKKIN_FEMALE:
			if ((texture_value >= 10 && texture_value <= 16) || texture_value <= 4)
				return true;
			break;
		case HIGH_ELF_MALE:
		case HIGH_ELF_FEMALE:
		case DARK_ELF_MALE:
		case DARK_ELF_FEMALE:
		case GNOME_MALE:
		case GNOME_FEMALE:
		case FROGLOK_MALE:
		case FROGLOK_FEMALE:
			if ((texture_value >= 10 && texture_value <= 16) || texture_value <= 3)
				return true;
			break;
		case VAHSHIR_MALE:
		case VAHSHIR_FEMALE:
			if (texture_value == 50 || texture_value <= 3)
				return true;
			break;
		case IKSAR_MALE:
		case IKSAR_FEMALE:
			if (texture_value == 10 || texture_value <= 4)
				return true;
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
			if (texture_value <= 3)
				return true;
			break;
		default:
			break;
		}
		return false;
	}
}

bool PlayerAppearance::IsValidWoad(uint16 race_id, uint8 gender_id, uint8 woad_value, bool use_luclin)
{
	if (woad_value == 0xFF)
		return true;

	if (use_luclin) {
		switch (BINDRG(race_id, gender_id)) {
		case BARBARIAN_MALE:
		case BARBARIAN_FEMALE:
			if (woad_value <= 8)
				return true;
			break;
		default:
			break;
		}
	}
	return false;
}

const char* GetGenderName(uint32 gender_id) {
	const char* gender_name = "Unknown";
	if (gender_id == MALE) {
		gender_name = "Male";
	} else if (gender_id == FEMALE) {
		gender_name = "Female";
	} else if (gender_id == NEUTER) {
		gender_name = "Neuter";
	}
	return gender_name;
}

const std::string GetPlayerRaceAbbreviation(uint16 race_id)
{
	if (!IsPlayerRace(race_id)) {
		return std::string("UNK");
	}

	switch (race_id) {
		case RACE_HUMAN_1:
			return "HUM";
		case RACE_BARBARIAN_2:
			return "BAR";
		case RACE_ERUDITE_3:
			return "ERU";
		case RACE_WOOD_ELF_4:
			return "ELF";
		case RACE_HIGH_ELF_5:
			return "HIE";
		case RACE_DARK_ELF_6:
			return "DEF";
		case RACE_HALF_ELF_7:
			return "HEF";
		case RACE_DWARF_8:
			return "DWF";
		case RACE_TROLL_9:
			return "TRL";
		case RACE_OGRE_10:
			return "OGR";
		case RACE_HALFLING_11:
			return "HFL";
		case RACE_GNOME_12:
			return "GNM";
		case RACE_IKSAR_128:
			return "IKS";
		case RACE_VAH_SHIR_130:
			return "VAH";
		case RACE_FROGLOK_330:
			return "FRG";
		case RACE_DRAKKIN_522:
			return "DRK";
	}

	return std::string("UNK");
}

bool IsPlayerRace(uint16 race_id) {
	return (
		EQ::ValueWithin(race_id, RACE_HUMAN_1, RACE_GNOME_12) ||
		race_id == RACE_IKSAR_128 ||
		race_id == RACE_VAH_SHIR_130 ||
		race_id == RACE_FROGLOK_330 ||
		race_id == RACE_DRAKKIN_522
	);
}
