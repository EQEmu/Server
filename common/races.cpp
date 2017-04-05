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

const char* GetRaceIDName(uint16 race_id)
{
	switch (race_id) {
	// Old Race Labels
	//case HUMAN:
	//	return "Human";
	//case BARBARIAN:
	//	return "Barbarian";
	//case ERUDITE:
	//	return "Erudite";
	//case WOOD_ELF:
	//	return "Wood Elf";
	//case HIGH_ELF:
	//	return "High Elf";
	//case DARK_ELF:
	//	return "Dark Elf";
	//case HALF_ELF:
	//	return "Half Elf";
	//case DWARF:
	//	return "Dwarf";
	//case TROLL:
	//	return "Troll";
	//case OGRE:
	//	return "Ogre";
	//case HALFLING:
	//	return "Halfling";
	//case GNOME:
	//	return "Gnome";
	//case IKSAR:
	//	return "Iksar";
	//case WEREWOLF:
	//	return "Werewolf";
	//case SKELETON:
	//	return "Skeleton";
	//case ELEMENTAL:
	//	return "Elemental";
	//case EYE_OF_ZOMM:
	//	return "Eye of Zomm";
	//case WOLF_ELEMENTAL:
	//	return "Wolf Elemental";
	//case IKSAR_SKELETON:
	//	return "Iksar Skeleton";
	//case VAHSHIR:
	//	return "Vah Shir";
	//case FROGLOK:
	//case FROGLOK2: // "Piranha"? (low-byte issue..)
	//	return "Froglok";
	//case DRAKKIN:
	//	return "Drakkin";
	
	// RoF2 Race Labels
	case RT_ABHORRENT:
		return "Abhorrent";
	case RT_AIR_ELEMENTAL:
	case RT_AIR_ELEMENTAL_2:
		return "Air Elemental";
	case RT_AIR_MEPHIT:
		return "Air Mephit";
	case RT_AKHEVA:
	case RT_AKHEVA_2:
		return "Akheva";
	case RT_ALARAN:
		return "Alaran";
	case RT_ALARAN_GHOST:
		return "Alaran Ghost";
	case RT_ALARAN_SENTRY_STONE:
		return "Alaran Sentry Stone";
	case RT_ALLIGATOR:
	case RT_ALLIGATOR_2:
		return "Alligator";
	case RT_AMYGDALAN:
	case RT_AMYGDALAN_2:
		return "Amygdalan";
	case RT_ANEUK:
		return "Aneuk";
	case RT_ANIMATED_ARMOR:
		return "Animated Armor";
	case RT_ANIMATED_HAND:
		return "Animated Hand";
	case RT_ANIMATED_STATUE:
	case RT_ANIMATED_STATUE_2:
		return "Animated Statue";
	case RT_APEXUS:
		return "Apexus";
	case RT_ARACHNID:
		return "Arachnid";
	case RT_ARCANIST_OF_HATE:
		return "Arcanist of Hate";
	case RT_ARMADILLO:
		return "Armadillo";
	case RT_ARMOR_RACK:
		return "Armor Rack";
	case RT_AVIAK:
	case RT_AVIAK_2:
		return "Aviak";
	case RT_AVIAK_PULL_ALONG:
		return "Aviak Pull Along";
	case RT_AYONAE_RO:
		return "Ayonae Ro";
	case RT_BANNER:
	case RT_BANNER_2:
	case RT_BANNER_3:
	case RT_BANNER_4:
	case RT_BANNER_5:
	case RT_BANNER_6:
	case RT_BANNER_7:
		return "Banner";
	case RT_BANNER_10TH_ANNIVERSARY:
		return "10th Anniversary Banner";
	case RT_BANSHEE:
	case RT_BANSHEE_2:
	case RT_BANSHEE_3:
		return "Banshee";
	case RT_BARBARIAN:
	case RT_BARBARIAN_2:
		return "Barbarian";
	case RT_BARREL:
		return "Barrel";
	case RT_BARREL_BARGE_SHIP:
		return "Barrel Barge Ship";
	case RT_BASILISK:
		return "Basilisk";
	case RT_BAT:
	case RT_BAT_2:
	case RT_BAT_3:
		return "Bat";
	case RT_BAZU:
		return "Bazu";
	case RT_BEAR:
	case RT_BEAR_2:
	case RT_BEAR_3:
		return "Bear";
	case RT_BEAR_MOUNT:
		return "Bear Mount";
	case RT_BEAR_TRAP:
		return "Bear Trap";
	case RT_BEETLE:
	case RT_BEETLE_2:
	case RT_BEETLE_3:
		return "Beetle";
	case RT_BEGGAR:
		return "Beggar";
	case RT_BELLIKOS:
		return "Bellikos";
	case RT_BERTOXXULOUS:
	case RT_BERTOXXULOUS_2:
		return "Bertoxxulous";
	case RT_BIXIE:
	case RT_BIXIE_2:
		return "Bixie";
	case RT_BLIMP_SHIP:
		return "Blimp Ship";
	case RT_BLIND_DREAMER:
		return "Blind Dreamer";
	case RT_BLOOD_RAVEN:
		return "Blood Raven";
	case RT_BOAT:
	case RT_BOAT_2:
		return "Boat";
	case RT_BOLVIRK:
		return "Bolvirk";
	case RT_BONE_GOLEM:
	case RT_BONE_GOLEM_2:
		return "Bone Golem";
	case RT_BONES:
		return "Bones";
	case RT_BOOK_DERVISH:
		return "Book Dervish";
	case RT_BOT_PORTAL:
		return "BoT Portal";
	case RT_BOULDER:
		return "Boulder";
	case RT_BOX:
		return "Box";
	case RT_BRAXI:
		return "Braxi";
	case RT_BRAXI_MOUNT:
		return "Braxi Mount";
	case RT_BRELL:
		return "Brell";
	case RT_BRELLS_FIRST_CREATION:
		return "Brell's First Creation";
	case RT_BRISTLEBANE:
		return "Bristlebane";
	case RT_BROKEN_CLOCKWORK:
		return "Broken Clockwork";
	case RT_BRONTOTHERIUM:
		return "Brontotherium";
	case RT_BROWNIE:
	case RT_BROWNIE_2:
		return "Brownie";
	case RT_BUBONIAN:
		return "Bubonian";
	case RT_BUBONIAN_UNDERLING:
		return "Bubonian Underling";
	case RT_BURYNAI:
	case RT_BURYNAI_2:
		return "Burynai";
	case RT_CAKE_10TH_ANNIVERSARY:
		return "10th Anniversary Cake";
	case RT_CAMPFIRE:
		return "Campfire";
	case RT_CARRIER_HAND:
		return "Carrier Hand";
	case RT_CAT:
		return "Cat";
	case RT_CAZIC_THULE:
	case RT_CAZIC_THULE_2:
		return "Cazic Thule";
	case RT_CENTAUR:
	case RT_CENTAUR_2:
		return "Centaur";
	case RT_CHEST:
	case RT_CHEST_2:
	case RT_CHEST_3:
		return "Chest";
	case RT_CHIMERA:
	case RT_CHIMERA_2:
		return "Chimera";
	case RT_CHOKIDAI:
		return "Chokidai";
	case RT_CLAM:
		return "Clam";
	case RT_CLIKNAR_MOUNT:
		return "Cliknar Mount";
	case RT_CLIKNAR_QUEEN:
		return "Cliknar Queen";
	case RT_CLIKNAR_SOLDIER:
		return "Cliknar Soldier";
	case RT_CLIKNAR_WORKER:
		return "Cliknar Worker";
	case RT_CLOCKWORK_BEETLE:
		return "Clockwork Beetle";
	case RT_CLOCKWORK_BOAR:
		return "Clockwork Boar";
	case RT_CLOCKWORK_BOMB:
		return "Clockwork Bomb";
	case RT_CLOCKWORK_BRAIN:
		return "Clockwork Brain";
	case RT_CLOCKWORK_GNOME:
		return "Clockwork Gnome";
	case RT_CLOCKWORK_GOLEM:
		return "Clockwork Golem";
	case RT_CLOCKWORK_GUARDIAN:
		return "Clockwork Guardian";
	case RT_COCKATRICE:
		return "Cockatrice";
	case RT_COFFIN:
	case RT_COFFIN_2:
		return "Coffin";
	case RT_COIN_PURSE:
		return "Coin Purse";
	case RT_COLDAIN:
	case RT_COLDAIN_2:
	case RT_COLDAIN_3:
		return "Coldain";
	case RT_CORAL:
		return "Coral";
	case RT_CORATHUS:
		return "Corathus";
	case RT_CRAB:
		return "Crab";
	case RT_CRAGBEAST:
		return "Cragbeast";
	case RT_CRAGSLITHER:
		return "Cragslither";
	case RT_CROCODILE:
		return "Crocodile";
	case RT_CRYSTAL:
		return "Crystal";
	case RT_CRYSTAL_SHARD:
		return "Crystal Shard";
	case RT_CRYSTAL_SPHERE:
		return "Crystal Sphere";
	case RT_CRYSTAL_SPIDER:
		return "Crystal Spider";
	case RT_CRYSTALSKIN_AMBULOID:
		return "Crystalskin Ambuloid";
	case RT_CRYSTALSKIN_SESSILOID:
		return "Crystalskin Sessiloid";
	case RT_DAISY_MAN:
		return "Daisy Man";
	case RT_DARK_ELF:
	case RT_DARK_ELF_2:
		return "Dark Elf";
	case RT_DARK_LORD:
		return "Dark Lord";
	case RT_DEMI_LICH:
		return "Demi Lich";
	case RT_DEMON_VULTURE:
		return "Demon Vulture";
	case RT_DERVISH:
	case RT_DERVISH_2:
	case RT_DERVISH_3:
	case RT_DERVISH_4:
		return "Dervish";
	case RT_DERVISH_VER_5:
		return "Dervish(Ver. 5)";
	case RT_DERVISH_VER_6:
		return "Dervish(Ver. 6)";
	case RT_DEVOURER:
		return "Devourer";
	case RT_DIRE_WOLF:
		return "Dire Wolf";
	case RT_DISCORD_SHIP:
		return "Discord Ship";
	case RT_DISCORDLING:
		return "Discordling";
	case RT_DISEASED_FIEND:
		return "Diseased Fiend";
	case RT_DJINN:
		return "Djinn";
	case RT_DRACHNID:
	case RT_DRACHNID_2:
		return "Drachnid";
	case RT_DRACHNID_COCOON:
		return "Drachnid Cocoon";
	case RT_DRACOLICH:
		return "Dracolich";
	case RT_DRAGLOCK:
		return "Draglock";
	case RT_DRAGON:
	case RT_DRAGON_2:
	case RT_DRAGON_3:
	case RT_DRAGON_4:
	case RT_DRAGON_5:
	case RT_DRAGON_6:
	case RT_DRAGON_7:
	case RT_DRAGON_8:
	case RT_DRAGON_9:
	case RT_DRAGON_10:
	case RT_DRAGON_11:
	case RT_DRAGON_12:
	case RT_DRAGON_13:
	case RT_DRAGON_14:
	case RT_DRAGON_15:
	case RT_DRAGON_16:
		return "Dragon";
	case RT_DRAGON_BONES:
		return "Dragon Bones";
	case RT_DRAGON_EGG:
		return "Dragon Egg";
	case RT_DRAGON_STATUE:
		return "Dragon Statue";
	case RT_DRAGORN:
		return "Dragorn";
	case RT_DRAGORN_BOX:
		return "Dragorn Box";
	case RT_DRAKE:
	case RT_DRAKE_2:
	case RT_DRAKE_3:
		return "Drake";
	case RT_DRAKKIN:
		return "Drakkin";
	case RT_DRIXIE:
		return "Drixie";
	case RT_DROGMORE:
		return "Drogmore";
	case RT_DROLVARG:
		return "Drolvarg";
	case RT_DRYAD:
		return "Dryad";
	case RT_DWARF:
	case RT_DWARF_2:
		return "Dwarf";
	case RT_DYNAMITE_KEG:
		return "Dynamite Keg";
	case RT_DYNLETH:
		return "Dyn'Leth";
	case RT_EARTH_ELEMENTAL:
	case RT_EARTH_ELEMENTAL_2:
		return "Earth Elemental";
	case RT_EARTH_MEPHIT:
		return "Earth Mephit";
	case RT_EEL:
		return "Eel";
	case RT_EFREETI:
	case RT_EFREETI_2:
		return "Efreeti";
	case RT_ELDDAR:
		return "Elddar";
	case RT_ELEMENTAL:
		return "Elemental";
	case RT_ELK_HEAD:
		return "Elk Head";
	case RT_ELVEN_BOAT:
		return "Elven Boat";
	case RT_ELVEN_GHOST:
		return "Elven Ghost";
	case RT_ENCHANTED_ARMOR:
		return "Enchanted Armor";
	case RT_EROLLISI:
		return "Erollisi";
	case RT_ERUDITE:
	case RT_ERUDITE_2:
	case RT_ERUDITE_3:
		return "Erudite";
	case RT_EVAN_TEST:
		return "Evan Test";
	case RT_EVIL_EYE:
	case RT_EVIL_EYE_2:
	case RT_EVIL_EYE_3:
		return "Evil Eye";
	case RT_EXOSKELETON:
		return "Exoskeleton";
	case RT_EXPLOSIVE_CART:
		return "Explosive Cart";
	case RT_EYE:
		return "Eye";
	case RT_FAIRY:
	case RT_FAIRY_2:
		return "Fairy";
	case RT_FALLEN_KNIGHT:
		return "Fallen Knight";
	case RT_FAUN:
		return "Faun";
	case RT_FAY_DRAKE:
		return "Fay Drake";
	case RT_FENNIN_RO:
		return "Fennin Ro";
	case RT_FERAN:
		return "Feran";
	case RT_FERAN_MOUNT:
		return "Feran Mount";
	case RT_FIEND:
		return "Fiend";
	case RT_FIRE_ELEMENTAL:
	case RT_FIRE_ELEMENTAL_2:
		return "Fire Elemental";
	case RT_FIRE_MEPHIT:
		return "Fire Mephit";
	case RT_FISH:
	case RT_FISH_2:
		return "Fish";
	case RT_FLAG:
		return "Flag";
	case RT_FLOATING_ISLAND:
		return "Floating Island";
	case RT_FLOATING_SKULL:
		return "Floating Skull";
	case RT_FLOATING_TOWER:
		return "Floating Tower";
	case RT_FLY:
		return "Fly";
	case RT_FLYING_CARPET:
		return "Flying Carpet";
	case RT_FOREST_GIANT:
		return "Forest Giant";
	case RT_FROG:
	case RT_FROG_2:
		return "Frog";
	case RT_FROGLOK:
	case RT_FROGLOK_2:
	case RT_FROGLOK_3:
		return "Froglok";
	case RT_FROGLOK_GHOST:
		return "Froglok Ghost";
	case RT_FROGLOK_SKELETON:
		return "Froglok Skeleton";
	case RT_FUNGAL_FIEND:
		return "Fungal Fiend";
	case RT_FUNGUS_PATCH:
		return "Fungus Patch";
	case RT_FUNGUSMAN:
		return "Fungusman";
	case RT_GALORIAN:
		return "Galorian";
	case RT_GARGOYLE:
	case RT_GARGOYLE_2:
		return "Gargoyle";
	case RT_GASBAG:
		return "Gasbag";
	case RT_GELATINOUS_CUBE:
	case RT_GELATINOUS_CUBE_2:
		return "Gelatinous Cube";
	case RT_GELIDRAN:
		return "Gelidran";
	case RT_GENARI:
		return "Genari";
	case RT_GEONID:
		return "Geonid";
	case RT_GHOST:
	case RT_GHOST_2:
	case RT_GHOST_3:
	case RT_GHOST_4:
		return "Ghost";
	case RT_GHOST_SHIP:
	case RT_GHOST_SHIP_2:
		return "Ghost Ship";
	case RT_GHOUL:
	case RT_GHOUL_2:
		return "Ghoul";
	case RT_GIANT:
	case RT_GIANT_2:
	case RT_GIANT_3:
	case RT_GIANT_4:
	case RT_GIANT_5:
	case RT_GIANT_6:
	case RT_GIANT_7:
	case RT_GIANT_8:
	case RT_GIANT_9:
	case RT_GIANT_10:
	case RT_GIANT_11:
	case RT_GIANT_12:
	case RT_GIANT_13:
		return "Giant";
	case RT_GIANT_CLOCKWORK:
		return "Giant Clockwork";
	case RT_GIANT_RALLOSIAN_MATS:
		return "Giant(Rallosian mats)";
	case RT_GIANT_SHADE:
		return "Giant Shade";
	case RT_GIGYN:
		return "Gigyn";
	case RT_GINGERBREAD_MAN:
		return "Gingerbread Man";
	case RT_GIRPLAN:
		return "Girplan";
	case RT_GNOLL:
	case RT_GNOLL_2:
	case RT_GNOLL_3:
		return "Gnoll";
	case RT_GNOME:
		return "Gnome";
	case RT_GNOMEWORK:
		return "Gnomework";
	case RT_GNOMISH_BALLOON:
		return "Gnomish Balloon";
	case RT_GNOMISH_BOAT:
		return "Gnomish Boat";
	case RT_GNOMISH_HOVERING_TRANSPORT:
		return "Gnomish Hovering Transport";
	case RT_GNOMISH_ROCKET_PACK:
		return "Gnomish Rocket Pack";
	case RT_GOBLIN:
	case RT_GOBLIN_2:
	case RT_GOBLIN_3:
	case RT_GOBLIN_4:
	case RT_GOBLIN_5:
		return "Goblin";
	case RT_GOD_LUCLIN_VER_2:
		return "God - Luclin(Ver. 2)";
	case RT_GOD_LUCLIN_VER_3:
		return "God - Luclin(Ver. 3)";
	case RT_GOD_LUCLIN_VER_4:
		return "God - Luclin(Ver. 4)";
	case RT_GOD_OF_DISCORD:
		return "God of Discord";
	case RT_GOLEM:
	case RT_GOLEM_2:
		return "Golem";
	case RT_GOO:
	case RT_GOO_2:
	case RT_GOO_3:
	case RT_GOO_4:
		return "Goo";
	case RT_GORAL:
		return "Goral";
	case RT_GORAL_MOUNT:
		return "Goral Mount";
	case RT_GORGON:
		return "Gorgon";
	case RT_GORILLA:
	case RT_GORILLA_2:
		return "Gorilla";
	case RT_GRANDFATHER_CLOCK:
		return "Grandfather Clock";
	case RT_GREKEN_YOUNG:
		return "Greken - Young";
	case RT_GREKEN_YOUNG_ADULT:
		return "Greken - Young Adult";
	case RT_GRENDLAEN:
		return "Grendlaen";
	case RT_GRIEG_VENEFICUS:
		return "Grieg Veneficus";
	case RT_GRIFFIN:
	case RT_GRIFFIN_2:
		return "Griffin";
	case RT_GRIMLING:
		return "Grimling";
	case RT_GROUND_SHAKER:
		return "Ground Shaker";
	case RT_GUARD:
	case RT_GUARD_2:
	case RT_GUARD_3:
	case RT_GUARD_4:
		return "Guard";
	case RT_GUARD_OF_JUSTICE:
		return "Guard of Justice";
	case RT_GUARDIAN_CPU:
		return "Guardian CPU";
	case RT_HADAL:
		return "Hadal";
	case RT_HAG:
		return "Hag";
	case RT_HALF_ELF:
		return "Half Elf";
	case RT_HALFLING:
	case RT_HALFLING_2:
		return "Halfling";
	case RT_HARPY:
	case RT_HARPY_2:
		return "Harpy";
	case RT_HIGH_ELF:
		return "High Elf";
	case RT_HIPPOGRIFF:
		return "Hippogriff";
	case RT_HOLGRESH:
	case RT_HOLGRESH_2:
		return "Holgresh";
	case RT_HONEY_POT:
		return "Honey Pot";
	case RT_HORSE:
	case RT_HORSE_2:
	case RT_HORSE_3:
		return "Horse";
	case RT_HOVERING_PLATFORM:
		return "Hovering Platform";
	case RT_HRAQUIS:
		return "Hraquis";
	case RT_HUMAN:
	case RT_HUMAN_2:
	case RT_HUMAN_3:
	case RT_HUMAN_4:
		return "Human";
	case RT_HUMAN_GHOST:
		return "Human Ghost";
	case RT_HUVUL:
		return "Huvul";
	case RT_HYDRA_CRYSTAL:
		return "Hydra Crystal";
	case RT_HYDRA_MOUNT:
		return "Hydra Mount";
	case RT_HYDRA_NPC:
		return "Hydra NPC";
	case RT_HYNID:
		return "Hynid";
	case RT_ICE_SPECTRE:
		return "Ice Spectre";
	case RT_IKAAV:
		return "Ikaav";
	case RT_IKSAR:
	case RT_IKSAR_2:
		return "Iksar";
	case RT_IKSAR_GHOST:
		return "Iksar Ghost";
	case RT_IKSAR_GOLEM:
		return "Iksar Golem";
	case RT_IKSAR_SKELETON:
		return "Iksar Skeleton";
	case RT_IKSAR_SPIRIT:
		return "Iksar Spirit";
	case RT_IMP:
		return "Imp";
	case RT_INNORUUK:
		return "Innoruuk";
	case RT_INSECT:
		return "Insect";
	case RT_INTERACTIVE_OBJECT:
		return "Interactive Object";
	case RT_INVISIBLE_MAN:
	case RT_INVISIBLE_MAN_2:
	case RT_INVISIBLE_MAN_3:
		return "Invisible Man";
	case RT_INVISIBLE_MAN_OF_ZOMM:
		return "Invisible Man of Zomm";
	case RT_IXT:
		return "Ixt";
	case RT_JOKESTER:
		return "Jokester";
	case RT_JUM_JUM_BUCKET:
		return "Jum Jum Bucket";
	case RT_JUNK_BEAST:
		return "Junk Beast";
	case RT_KANGON:
		return "Kangon";
	case RT_KANGON_MOUNT:
		return "Kangon Mount";
	case RT_KARANA:
		return "Karana";
	case RT_KEDGE:
	case RT_KEDGE_2:
		return "Kedge";
	case RT_KERRAN:
	case RT_KERRAN_2:
		return "Kerran";
	case RT_KIRIN:
	case RT_KIRIN_2:
		return "Kirin";
	case RT_KNIGHT_OF_HATE:
		return "Knight of Hate";
	case RT_KNIGHT_OF_PESTILENCE:
		return "Knight of Pestilence";
	case RT_KOBOLD:
	case RT_KOBOLD_2:
		return "Kobold";
	case RT_KRAKEN:
		return "Kraken";
	case RT_KYV:
		return "Kyv";
	case RT_LAUNCH:
		return "Launch";
	case RT_LAVA_ROCK:
		return "Lava Rock";
	case RT_LAVA_SPIDER:
		return "Lava Spider";
	case RT_LAVA_SPIDER_QUEEN:
		return "Lava Spider Queen";
	case RT_LEECH:
		return "Leech";
	case RT_LEPERTOLOTH:
		return "Lepertoloth";
	case RT_LIGHTCRAWLER:
		return "Lightcrawler";
	case RT_LIGHTNING_WARRIOR:
		return "Lightning Warrior";
	case RT_LION:
		return "Lion";
	case RT_LIZARD_MAN:
		return "Lizard Man";
	case RT_LUCLIN:
		return "Luclin";
	case RT_LUGGALD:
	case RT_LUGGALD_2:
		return "Luggald";
	case RT_LUGGALDS:
		return "Luggalds";
	case RT_MALARIAN:
		return "Malarian";
	case RT_MAMMOTH:
	case RT_MAMMOTH_2:
		return "Mammoth";
	case RT_MAN_EATING_PLANT:
		return "Man - Eating Plant";
	case RT_MANSION:
		return "Mansion";
	case RT_MANTICORE:
		return "Manticore";
	case RT_MANTRAP:
		return "Mantrap";
	case RT_MARIONETTE:
		return "Marionette";
	case RT_MASTRUQ:
		return "Mastruq";
	case RT_MATA_MURAM:
		return "Mata Muram";
	case RT_MEDIUM_PLANT:
		return "Medium Plant";
	case RT_MEPHIT:
		return "Mephit";
	case RT_MERCHANT_SHIP:
		return "Merchant Ship";
	case RT_MERMAID:
		return "Mermaid";
	case RT_MIMIC:
		return "Mimic";
	case RT_MINI_POM:
		return "Mini POM";
	case RT_MINOTAUR:
	case RT_MINOTAUR_2:
	case RT_MINOTAUR_3:
	case RT_MINOTAUR_4:
		return "Minotaur";
	case RT_MITHANIEL_MARR:
		return "Mithaniel Marr";
	case RT_MORELL_THULE:
		return "Morell Thule";
	case RT_MOSQUITO:
		return "Mosquito";
	case RT_MOUTH_OF_INSANITY:
		return "Mouth of Insanity";
	case RT_MUDDITE:
		return "Muddite";
	case RT_MUMMY:
		return "Mummy";
	case RT_MURAMITE_ARMOR_PILE:
		return "Muramite Armor Pile";
	case RT_MURKGLIDER:
		return "Murkglider";
	case RT_MURKGLIDER_EGG_SAC:
		return "Murkglider Egg Sac";
	case RT_MUTNA:
		return "Mutna";
	case RT_NEKHON:
		return "Nekhon";
	case RT_NETHERBIAN:
		return "Netherbian";
	case RT_NIGHTMARE:
		return "Nightmare";
	case RT_NIGHTMARE_GARGOYLE:
		return "Nightmare Gargoyle";
	case RT_NIGHTMARE_GOBLIN:
		return "Nightmare Goblin";
	case RT_NIGHTMARE_MEPHIT:
		return "Nightmare Mephit";
	case RT_NIGHTMARE_UNICORN:
	case RT_NIGHTMARE_UNICORN_2:
		return "Nightmare / Unicorn";
	case RT_NIGHTMARE_WRAITH:
		return "Nightmare Wraith";
	case RT_NIHIL:
		return "Nihil";
	case RT_NILBORIEN:
		return "Nilborien";
	case RT_NOC:
		return "Noc";
	case RT_NYMPH:
		return "Nymph";
	case RT_OGRE:
	case RT_OGRE_2:
		return "Ogre";
	case RT_OGRE_NPC_MALE:
		return "Ogre NPC - Male";
	case RT_ORB:
		return "Orb";
	case RT_ORC:
	case RT_ORC_2:
		return "Orc";
	case RT_OTHMIR:
		return "Othmir";
	case RT_OWLBEAR:
		return "Owlbear";
	case RT_PARASITIC_SCAVENGER:
		return "Parasitic Scavenger";
	case RT_PEGASUS:
	case RT_PEGASUS_2:
	case RT_PEGASUS_3:
		return "Pegasus";
	case RT_PHOENIX:
		return "Phoenix";
	case RT_PIRANHA:
		return "Piranha";
	case RT_PIRATE:
	case RT_PIRATE_2:
	case RT_PIRATE_3:
	case RT_PIRATE_4:
	case RT_PIRATE_5:
	case RT_PIRATE_6:
	case RT_PIRATE_7:
	case RT_PIRATE_8:
		return "Pirate";
	case RT_PIRATE_SHIP:
		return "Pirate Ship";
	case RT_PIXIE:
		return "Pixie";
	case RT_POISON_FROG:
		return "Poison Frog";
	case RT_PORTAL:
		return "Portal";
	case RT_POWDER_KEG:
		return "Powder Keg";
	case RT_PRESSURE_PLATE:
		return "Pressure Plate";
	case RT_PUFFER_SPORE:
		return "Puffer Spore";
	case RT_PUMA:
	case RT_PUMA_2:
	case RT_PUMA_3:
		return "Puma";
	case RT_PUSLING:
		return "Pusling";
	case RT_PYRILEN:
		return "Pyrilen";
	case RT_RA_TUK:
		return "Ra`tuk";
	case RT_RABBIT:
		return "Rabbit";
	case RT_RALLOS_ZEK:
	case RT_RALLOS_ZEK_2:
		return "Rallos Zek";
	case RT_RALLOS_ZEK_MINION:
		return "Rallos Zek Minion";
	case RT_RAPTOR:
	case RT_RAPTOR_2:
		return "Raptor";
	case RT_RAPTOR_MOUNT:
		return "Raptor Mount";
	case RT_RAT:
	case RT_RAT_2:
		return "Rat";
	case RT_RAT_MOUNT:
		return "Rat Mount";
	case RT_RATMAN:
	case RT_RATMAN_2:
		return "Ratman";
	case RT_REANIMATED_HAND:
		return "Reanimated Hand";
	case RT_RECUSO:
		return "Recuso";
	case RT_REGENERATION_POOL:
		return "Regeneration Pool";
	case RT_RELIC_CASE:
		return "Relic case ";
	case RT_RHINO_BEETLE:
		return "Rhino Beetle";
	case RT_RHINOCEROS:
		return "Rhinoceros";
	case RT_ROBOCOPTER_OF_ZOMM:
		return "Robocopter of Zomm";
	case RT_ROCK_PILE:
		return "Rock Pile";
	case RT_ROCKHOPPER:
		return "Rockhopper";
	case RT_RONNIE_TEST:
		return "Ronnie Test";
	case RT_ROOT_TENTACLE:
		return "Root Tentacle";
	case RT_ROT_DOG_MOUNT:
		return "Rot Dog Mount";
	case RT_ROTDOG:
		return "Rotdog";
	case RT_ROTOCOPTER:
		return "Rotocopter";
	case RT_ROWBOAT:
		return "Rowboat";
	case RT_ROYAL_GUARD:
		return "Royal Guard";
	case RT_RUJARKIAN_ORC:
	case RT_RUJARKIAN_ORC_2:
		return "Rujarkian Orc";
	case RT_RUNED_ORB:
		return "Runed Orb";
	case RT_RUNIC_SYMBOL:
		return "Runic Symbol";
	case RT_SABER_TOOTHED_CAT:
		return "Saber - toothed Cat";
	case RT_SALTPETTER_BOMB:
		return "Saltpetter Bomb";
	case RT_SAND_ELF:
		return "Sand Elf";
	case RT_SANDMAN:
		return "Sandman";
	case RT_SARNAK:
	case RT_SARNAK_2:
		return "Sarnak";
	case RT_SARNAK_GOLEM:
		return "Sarnak Golem";
	case RT_SARNAK_SPIRIT:
		return "Sarnak Spirit";
	case RT_SARYRN:
		return "Saryrn";
	case RT_SATYR:
		return "Satyr";
	case RT_SCALED_WOLF:
		return "Scaled Wolf";
	case RT_SCARECROW:
	case RT_SCARECROW_2:
		return "Scarecrow";
	case RT_SCARLET_CHEETAH:
		return "Scarlet Cheetah";
	case RT_SCLERA_MOUNT:
		return "Sclera Mount";
	case RT_SCORPION:
	case RT_SCORPION_2:
	case RT_SCORPION_3:
		return "Scorpion";
	case RT_SCRYKIN:
		return "Scrykin";
	case RT_SEA_TURTLE:
		return "Sea Turtle";
	case RT_SEAHORSE:
		return "Seahorse";
	case RT_SELYRAH:
		return "Selyrah";
	case RT_SELYRAH_MOUNT:
		return "Selyrah Mount";
	case RT_SERU:
		return "Seru";
	case RT_SERVANT_OF_SHADOW:
		return "Servant of Shadow";
	case RT_SESSILOID_MOUNT:
		return "Sessiloid Mount";
	case RT_SHADE:
	case RT_SHADE_2:
	case RT_SHADE_3:
		return "Shade";
	case RT_SHADEL:
		return "Shadel";
	case RT_SHAMBLING_MOUND:
		return "Shambling Mound";
	case RT_SHARK:
		return "Shark";
	case RT_SHIKNAR:
		return "Shik'Nar";
	case RT_SHILISKIN:
		return "Shiliskin";
	case RT_SHIP:
		return "Ship";
	case RT_SHIP_IN_A_BOTTLE:
		return "Ship in a Bottle";
	case RT_SHISSAR:
	case RT_SHISSAR_2:
		return "Shissar";
	case RT_SHRIEKER:
		return "Shrieker";
	case RT_SIREN:
	case RT_SIREN_2:
		return "Siren";
	case RT_SKELETAL_HORSE:
		return "Skeletal Horse";
	case RT_SKELETON:
	case RT_SKELETON_2:
	case RT_SKELETON_3:
		return "Skeleton";
	case RT_SKUNK:
		return "Skunk";
	case RT_SKYSTRIDER:
		return "Skystrider";
	case RT_SMALL_PLANT:
		return "Small Plant";
	case RT_SNAKE:
	case RT_SNAKE_2:
		return "Snake";
	case RT_SNAKE_ELEMENTAL:
		return "Snake Elemental";
	case RT_SNOW_DERVISH:
		return "Snow Dervish";
	case RT_SNOW_RABBIT:
		return "Snow Rabbit";
	case RT_SOKOKAR:
		return "Sokokar";
	case RT_SOKOKAR_MOUNT:
		return "Sokokar Mount";
	case RT_SOKOKAR_W_SADDLE:
		return "Sokokar(w saddle)";
	case RT_SOLUSEK_RO:
	case RT_SOLUSEK_RO_2:
		return "Solusek Ro";
	case RT_SOLUSEK_RO_GUARD:
		return "Solusek Ro Guard";
	case RT_SONIC_WOLF:
		return "Sonic Wolf";
	case RT_SOUL_DEVOURER:
		return "Soul Devourer";
	case RT_SPECTRE:
	case RT_SPECTRE_2:
		return "Spectre";
	case RT_SPELL_PARTICLE_1:
		return "Spell Particle 1";
	case RT_SPHINX:
	case RT_SPHINX_2:
		return "Sphinx";
	case RT_SPIDER:
	case RT_SPIDER_2:
		return "Spider";
	case RT_SPIDER_EGG_SACK:
		return "Spider Egg Sack";
	case RT_SPIDER_MOUNT:
		return "Spider Mount";
	case RT_SPIDER_QUEEN:
		return "Spider Queen";
	case RT_SPIKE_TRAP:
		return "Spike Trap";
	case RT_SPIRIT_WOLF:
		return "Spirit Wolf";
	case RT_SPORALI:
		return "Sporali";
	case RT_STONE_JUG:
		return "Stone Jug";
	case RT_STONE_PYLON:
		return "Stone Pylon";
	case RT_STONE_RING:
		return "Stone Ring";
	case RT_STONE_WORKER:
	case RT_STONE_WORKER_2:
		return "Stone Worker";
	case RT_STONEGRABBER:
		return "Stonegrabber";
	case RT_STONEMITE:
		return "Stonemite";
	case RT_STORMRIDER:
		return "Stormrider";
	case RT_SUCCUBUS:
		return "Succubus";
	case RT_SUCCULENT:
		return "Succulent";
	case RT_SULLON_ZEK:
		return "Sullon Zek";
	case RT_SUN_REVENANT:
		return "Sun Revenant";
	case RT_SUNFLOWER:
		return "Sunflower";
	case RT_SWINETOR:
		return "Swinetor";
	case RT_SWORDFISH:
		return "Swordfish";
	case RT_SYNARCANA:
		return "Synarcana";
	case RT_TABLE:
		return "Table";
	case RT_TADPOLE:
		return "Tadpole";
	case RT_TAELOSIAN:
		return "Taelosian";
	case RT_TALL_PLANT:
		return "Tall Plant";
	case RT_TALLON_ZEK:
		return "Tallon Zek";
	case RT_TANETH:
		return "Taneth";
	case RT_TAREW_MARR:
		return "Tarew Marr";
	case RT_TEGI:
		return "Tegi";
	case RT_TELEPORT_MAN:
		return "Teleport Man";
	case RT_TELEPORTATION_STAND:
		return "Teleportation Stand";
	case RT_TELMIRA:
		return "Telmira";
	case RT_TENTACLE_TERROR:
	case RT_TENTACLE_TERROR_2:
		return "Tentacle Terror";
	case RT_TERRIS_THULE:
		return "Terris Thule";
	case RT_TEST_OBJECT:
		return "Test Object";
	case RT_THE_RATHE:
		return "The Rathe";
	case RT_THE_TRIBUNAL:
		return "The Tribunal";
	case RT_THOUGHT_HORROR:
		return "Thought Horror";
	case RT_TIGER:
		return "Tiger";
	case RT_TIN_SOLDIER:
		return "Tin Soldier";
	case RT_TOOLBOX:
		return "Toolbox";
	case RT_TOPIARY_LION:
		return "Topiary Lion";
	case RT_TOPIARY_LION_MOUNT:
		return "Topiary Lion Mount";
	case RT_TORMENTOR:
		return "Tormentor";
	case RT_TOTEM:
	case RT_TOTEM_2:
		return "Totem";
	case RT_TRAKANON:
		return "Trakanon";
	case RT_TRANQUILION:
		return "Tranquilion";
	case RT_TREANT:
	case RT_TREANT_2:
	case RT_TREANT_3:
		return "Treant";
	case RT_TRIBUNAL:
		return "Tribunal";
	case RT_TRIUMVIRATE:
		return "Triumvirate";
	case RT_TROLL:
	case RT_TROLL_2:
	case RT_TROLL_3:
	case RT_TROLL_4:
	case RT_TROLL_5:
		return "Troll";
	case RT_TROLL_ZOMBIE:
		return "Troll Zombie";
	case RT_TRUSIK:
		return "Trusik";
	case RT_TSETSIAN:
		return "Tsetsian";
	case RT_TUMBLEWEED:
		return "Tumbleweed";
	case RT_TUNARE:
		return "Tunare";
	case RT_TUREPTA:
		return "Turepta";
	case RT_UKUN:
		return "Ukun";
	case RT_ULTHORK:
		return "Ulthork";
	case RT_UNDEAD_CHOKIDAI:
		return "Undead Chokidai";
	case RT_UNDEAD_FOOTMAN:
		return "Undead Footman";
	case RT_UNDEAD_FROGLOK:
		return "Undead Froglok";
	case RT_UNDEAD_IKSAR:
		return "Undead Iksar";
	case RT_UNDEAD_KNIGHT:
		return "Undead Knight";
	case RT_UNDEAD_SARNAK:
		return "Undead Sarnak";
	case RT_UNDEAD_VEKSAR:
		return "Undead Veksar";
	case RT_UNDERBULK:
		return "Underbulk";
	case RT_UNICORN:
		return "Unicorn";
	case RT_UNKNOWN_RACE:
	case RT_UNKNOWN_RACE_2:
	case RT_UNKNOWN_RACE_3:
	case RT_UNKNOWN_RACE_4:
	case RT_UNKNOWN_RACE_5:
	case RT_UNKNOWN_RACE_6:
	case RT_UNKNOWN_RACE_7:
		return "UNKNOWN RACE";
	case RT_VAH_SHIR:
	case RT_VAH_SHIR_2:
		return "Vah Shir";
	case RT_VAH_SHIR_SKELETON:
		return "Vah Shir Skeleton";
	case RT_VALLON_ZEK:
		return "Vallon Zek";
	case RT_VALORIAN:
	case RT_VALORIAN_2:
		return "Valorian";
	case RT_VAMPIRE:
	case RT_VAMPIRE_2:
	case RT_VAMPIRE_3:
	case RT_VAMPIRE_4:
	case RT_VAMPIRE_5:
	case RT_VAMPIRE_6:
	case RT_VAMPIRE_7:
	case RT_VAMPIRE_8:
		return "Vampire";
	case RT_VASE:
		return "Vase";
	case RT_VEGEROG:
		return "Vegerog";
	case RT_VEKSAR:
	case RT_VEKSAR_2:
	case RT_VEKSAR_3:
		return "Veksar";
	case RT_VENRIL_SATHIR:
		return "Venril Sathir";
	case RT_VINE_MAW:
		return "Vine Maw";
	case RT_WAGON:
		return "Wagon";
	case RT_WALRUS:
		return "Walrus";
	case RT_WAR_BOAR:
	case RT_WAR_BOAR_2:
		return "War Boar";
	case RT_WAR_WRAITH:
		return "War Wraith";
	case RT_WASP:
		return "Wasp";
	case RT_WATER_ELEMENTAL:
	case RT_WATER_ELEMENTAL_2:
		return "Water Elemental";
	case RT_WATER_MEPHIT:
		return "Water Mephit";
	case RT_WATER_SPOUT:
		return "Water Spout";
	case RT_WEAPON_RACK:
	case RT_WEAPON_RACK_2:
		return "Weapon Rack";
	case RT_WEB:
		return "Web";
	case RT_WEDDING_ALTAR:
		return "Wedding Altar";
	case RT_WEDDING_ARBOR:
		return "Wedding Arbor";
	case RT_WEDDING_FLOWERS:
		return "Wedding Flowers";
	case RT_WEREORC:
		return "Wereorc";
	case RT_WEREWOLF:
	case RT_WEREWOLF_2:
	case RT_WEREWOLF_3:
		return "Werewolf";
	case RT_WETFANG_MINNOW:
		return "Wetfang Minnow";
	case RT_WHIRLIGIG:
		return "Whirligig";
	case RT_WICKER_BASKET:
		return "Wicker Basket";
	case RT_WILL_O_WISP:
		return "Will - O - Wisp";
	case RT_WINE_CASK:
	case RT_WINE_CASK_2:
		return "Wine Cask";
	case RT_WITHERAN:
	case RT_WITHERAN_2:
		return "Witheran";
	case RT_WOLF:
	case RT_WOLF_2:
	case RT_WOLF_3:
		return "Wolf";
	case RT_WOOD_ELF:
		return "Wood Elf";
	case RT_WORG:
	case RT_WORG_2:
		return "Worg";
	case RT_WORM:
		return "Worm";
	case RT_WRETCH:
		return "Wretch";
	case RT_WRULON:
	case RT_WRULON_2:
		return "Wrulon";
	case RT_WURM:
	case RT_WURM_2:
		return "Wurm";
	case RT_WURM_MOUNT:
		return "Wurm Mount";
	case RT_WYVERN:
	case RT_WYVERN_2:
		return "Wyvern";
	case RT_XALGOZ:
		return "Xalgoz";
	case RT_XARIC_THE_UNSPOKEN:
		return "Xaric the Unspoken";
	case RT_XEGONY:
		return "Xegony";
	case RT_YAKKAR:
		return "Yakkar";
	case RT_YETI:
		return "Yeti";
	case RT_ZEBUXORUK:
		return "Zebuxoruk";
	case RT_ZEBUXORUKS_CAGE:
		return "Zebuxoruk's Cage";
	case RT_ZELNIAK:
		return "Zelniak";
	case RT_ZOMBIE:
	case RT_ZOMBIE_2:
		return "Zombie";
	default:
		return "UNKNOWN RACE";
	}
}

const char* GetPlayerRaceName(uint32 player_race_value)
{
	return GetRaceIDName(GetRaceIDFromPlayerRaceValue(player_race_value));
}

uint32 GetPlayerRaceValue(uint16 race_id)
{
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
		return PLAYER_RACE_UNKNOWN; // watch
	}
}

uint32 GetPlayerRaceBit(uint16 race_id)
{
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

uint16 GetRaceIDFromPlayerRaceValue(uint32 player_race_value)
{
	switch (player_race_value) {
	case PLAYER_RACE_HUMAN:
	case PLAYER_RACE_BARBARIAN:
	case PLAYER_RACE_ERUDITE:
	case PLAYER_RACE_WOOD_ELF:
	case PLAYER_RACE_HIGH_ELF:
	case PLAYER_RACE_DARK_ELF:
	case PLAYER_RACE_HALF_ELF:
	case PLAYER_RACE_DWARF:
	case PLAYER_RACE_TROLL:
	case PLAYER_RACE_OGRE:
	case PLAYER_RACE_HALFLING:
	case PLAYER_RACE_GNOME:
		return player_race_value;
	case PLAYER_RACE_IKSAR:
		return IKSAR;
	case PLAYER_RACE_VAHSHIR:
		return VAHSHIR;
	case PLAYER_RACE_FROGLOK:
		return FROGLOK;
	case PLAYER_RACE_DRAKKIN:
		return DRAKKIN;
	default:
		return PLAYER_RACE_UNKNOWN; // watch
	}
}

uint16 GetRaceIDFromPlayerRaceBit(uint32 player_race_bit)
{
	switch (player_race_bit) {
	case PLAYER_RACE_HUMAN_BIT:
		return HUMAN;
	case PLAYER_RACE_BARBARIAN_BIT:
		return BARBARIAN;
	case PLAYER_RACE_ERUDITE_BIT:
		return ERUDITE;
	case PLAYER_RACE_WOOD_ELF_BIT:
		return WOOD_ELF;
	case PLAYER_RACE_HIGH_ELF_BIT:
		return HIGH_ELF;
	case PLAYER_RACE_DARK_ELF_BIT:
		return DARK_ELF;
	case PLAYER_RACE_HALF_ELF_BIT:
		return HALF_ELF;
	case PLAYER_RACE_DWARF_BIT:
		return DWARF;
	case PLAYER_RACE_TROLL_BIT:
		return TROLL;
	case PLAYER_RACE_OGRE_BIT:
		return OGRE;
	case PLAYER_RACE_HALFLING_BIT:
		return HALFLING;
	case PLAYER_RACE_GNOME_BIT:
		return GNOME;
	case PLAYER_RACE_IKSAR_BIT:
		return IKSAR;
	case PLAYER_RACE_VAHSHIR_BIT:
		return VAHSHIR;
	case PLAYER_RACE_FROGLOK_BIT:
		return FROGLOK;
	case PLAYER_RACE_DRAKKIN_BIT:
		return DRAKKIN;
	default:
		return PLAYER_RACE_UNKNOWN; // watch
	}
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
