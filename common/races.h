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
#define OGGOK_CITIZEN 93
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

#define RT_ABHORRENT 193
#define RT_AIR_ELEMENTAL 210
#define RT_AIR_ELEMENTAL_2 475
#define RT_AIR_MEPHIT 291
#define RT_AKHEVA 230
#define RT_AKHEVA_2 722
#define RT_ALARAN 695
#define RT_ALARAN_GHOST 708
#define RT_ALARAN_SENTRY_STONE 703
#define RT_ALLIGATOR 91
#define RT_ALLIGATOR_2 479
#define RT_AMYGDALAN 99
#define RT_AMYGDALAN_2 663
#define RT_ANEUK 395
#define RT_ANIMATED_ARMOR 323
#define RT_ANIMATED_HAND 166
#define RT_ANIMATED_STATUE 442
#define RT_ANIMATED_STATUE_2 448
#define RT_APEXUS 637
#define RT_ARACHNID 326
#define RT_ARCANIST_OF_HATE 352
#define RT_ARMADILLO 87
#define RT_ARMOR_RACK 535
#define RT_AVIAK 13
#define RT_AVIAK_2 558
#define RT_AVIAK_PULL_ALONG 711
#define RT_AYONAE_RO 498
#define RT_BANNER 500
#define RT_BANNER_2 553
#define RT_BANNER_3 554
#define RT_BANNER_4 555
#define RT_BANNER_5 556
#define RT_BANNER_6 557
#define RT_BANNER_7 586
#define RT_BANNER_10TH_ANNIVERSARY 628
#define RT_BANSHEE 250
#define RT_BANSHEE_2 487
#define RT_BANSHEE_3 488
#define RT_BARBARIAN 2
#define RT_BARBARIAN_2 90
#define RT_BARREL 377
#define RT_BARREL_BARGE_SHIP 546
#define RT_BASILISK 436
#define RT_BAT 34
#define RT_BAT_2 260
#define RT_BAT_3 416
#define RT_BAZU 409
#define RT_BEAR 43
#define RT_BEAR_2 305
#define RT_BEAR_3 480
#define RT_BEAR_MOUNT 655
#define RT_BEAR_TRAP 503
#define RT_BEETLE 22
#define RT_BEETLE_2 559
#define RT_BEETLE_3 716
#define RT_BEGGAR 55
#define RT_BELLIKOS 638
#define RT_BERTOXXULOUS 152
#define RT_BERTOXXULOUS_2 255
#define RT_BIXIE 79
#define RT_BIXIE_2 520
#define RT_BLIMP_SHIP 693
#define RT_BLIND_DREAMER 669
#define RT_BLOOD_RAVEN 279
#define RT_BOAT 141
#define RT_BOAT_2 533
#define RT_BOLVIRK 486
#define RT_BONE_GOLEM 362
#define RT_BONE_GOLEM_2 491
#define RT_BONES 383
#define RT_BOOK_DERVISH 660
#define RT_BOT_PORTAL 329
#define RT_BOULDER 585
#define RT_BOX 376
#define RT_BRAXI 688
#define RT_BRAXI_MOUNT 676
#define RT_BRELL 640
#define RT_BRELLS_FIRST_CREATION 639
#define RT_BRISTLEBANE 153
#define RT_BROKEN_CLOCKWORK 274
#define RT_BRONTOTHERIUM 169
#define RT_BROWNIE 15
#define RT_BROWNIE_2 568
#define RT_BUBONIAN 268
#define RT_BUBONIAN_UNDERLING 269
#define RT_BURYNAI 144
#define RT_BURYNAI_2 602
#define RT_CAKE_10TH_ANNIVERSARY 629
#define RT_CAMPFIRE 567
#define RT_CARRIER_HAND 721
#define RT_CAT 713
#define RT_CAZIC_THULE 95
#define RT_CAZIC_THULE_2 670
#define RT_CENTAUR 16
#define RT_CENTAUR_2 521
#define RT_CHEST 378
#define RT_CHEST_2 589
#define RT_CHEST_3 590
#define RT_CHIMERA 412
#define RT_CHIMERA_2 582
#define RT_CHOKIDAI 356
#define RT_CLAM 115
#define RT_CLIKNAR_MOUNT 652
#define RT_CLIKNAR_QUEEN 642
#define RT_CLIKNAR_SOLDIER 643
#define RT_CLIKNAR_WORKER 644
#define RT_CLOCKWORK_BEETLE 276
#define RT_CLOCKWORK_BOAR 472
#define RT_CLOCKWORK_BOMB 504
#define RT_CLOCKWORK_BRAIN 249
#define RT_CLOCKWORK_GNOME 88
#define RT_CLOCKWORK_GOLEM 248
#define RT_CLOCKWORK_GUARDIAN 572
#define RT_COCKATRICE 96
#define RT_COFFIN 382
#define RT_COFFIN_2 592
#define RT_COIN_PURSE 427
#define RT_COLDAIN 183
#define RT_COLDAIN_2 645
#define RT_COLDAIN_3 646
#define RT_CORAL 460
#define RT_CORATHUS 459
#define RT_CRAB 302
#define RT_CRAGBEAST 390
#define RT_CRAGSLITHER 597
#define RT_CROCODILE 259
#define RT_CRYSTAL 591
#define RT_CRYSTAL_SHARD 425
#define RT_CRYSTAL_SPHERE 616
#define RT_CRYSTAL_SPIDER 327
#define RT_CRYSTALSKIN_AMBULOID 641
#define RT_CRYSTALSKIN_SESSILOID 647
#define RT_DAISY_MAN 97
#define RT_DARK_ELF 6
#define RT_DARK_ELF_2 77
#define RT_DARK_LORD 466
#define RT_DEMI_LICH 45
#define RT_DEMON_VULTURE 620
#define RT_DERVISH 100
#define RT_DERVISH_2 372
#define RT_DERVISH_3 431
#define RT_DERVISH_4 704
#define RT_DERVISH_VER_5 726
#define RT_DERVISH_VER_6 727
#define RT_DEVOURER 159
#define RT_DIRE_WOLF 171
#define RT_DISCORD_SHIP 404
#define RT_DISCORDLING 418
#define RT_DISEASED_FIEND 253
#define RT_DJINN 126
#define RT_DRACHNID 57
#define RT_DRACHNID_2 461
#define RT_DRACHNID_COCOON 462
#define RT_DRACOLICH 604
#define RT_DRAGLOCK 132
#define RT_DRAGON 49
#define RT_DRAGON_2 122
#define RT_DRAGON_3 165
#define RT_DRAGON_4 184
#define RT_DRAGON_5 192
#define RT_DRAGON_6 195
#define RT_DRAGON_7 196
#define RT_DRAGON_8 198
#define RT_DRAGON_9 304
#define RT_DRAGON_10 435
#define RT_DRAGON_11 437
#define RT_DRAGON_12 438
#define RT_DRAGON_13 452
#define RT_DRAGON_14 530
#define RT_DRAGON_15 531
#define RT_DRAGON_16 569
#define RT_DRAGON_BONES 423
#define RT_DRAGON_EGG 445
#define RT_DRAGON_STATUE 446
#define RT_DRAGORN 413
#define RT_DRAGORN_BOX 421
#define RT_DRAKE 89
#define RT_DRAKE_2 430
#define RT_DRAKE_3 432
#define RT_DRAKKIN 522
#define RT_DRIXIE 113
#define RT_DROGMORE 348
#define RT_DROLVARG 133
#define RT_DRYAD 243
#define RT_DWARF 8
#define RT_DWARF_2 94
#define RT_DYNAMITE_KEG 505
#define RT_DYNLETH 532
#define RT_EARTH_ELEMENTAL 209
#define RT_EARTH_ELEMENTAL_2 476
#define RT_EARTH_MEPHIT 292
#define RT_EEL 35
#define RT_EFREETI 101
#define RT_EFREETI_2 320
#define RT_ELDDAR 489
#define RT_ELEMENTAL 75
#define RT_ELK_HEAD 714
#define RT_ELVEN_BOAT 544
#define RT_ELVEN_GHOST 587
#define RT_ENCHANTED_ARMOR 175
#define RT_EROLLISI 150
#define RT_ERUDITE 3
#define RT_ERUDITE_2 78
#define RT_ERUDITE_3 678
#define RT_EVAN_TEST 204
#define RT_EVIL_EYE 21
#define RT_EVIL_EYE_2 375
#define RT_EVIL_EYE_3 469
#define RT_EXOSKELETON 570
#define RT_EXPLOSIVE_CART 692
#define RT_EYE 108
#define RT_FAIRY 25
#define RT_FAIRY_2 473
#define RT_FALLEN_KNIGHT 719
#define RT_FAUN 182
#define RT_FAY_DRAKE 154
#define RT_FENNIN_RO 284
#define RT_FERAN 410
#define RT_FERAN_MOUNT 623
#define RT_FIEND 300
#define RT_FIRE_ELEMENTAL 212
#define RT_FIRE_ELEMENTAL_2 477
#define RT_FIRE_MEPHIT 293
#define RT_FISH 24
#define RT_FISH_2 148
#define RT_FLAG 501
#define RT_FLOATING_ISLAND 596
#define RT_FLOATING_SKULL 512
#define RT_FLOATING_TOWER 691
#define RT_FLY 245
#define RT_FLYING_CARPET 720
#define RT_FOREST_GIANT 490
#define RT_FROG 343
#define RT_FROG_2 603
#define RT_FROGLOK 26
#define RT_FROGLOK_2 27
#define RT_FROGLOK_3 330
#define RT_FROGLOK_GHOST 371
#define RT_FROGLOK_SKELETON 349
#define RT_FUNGAL_FIEND 218
#define RT_FUNGUS_PATCH 463
#define RT_FUNGUSMAN 28
#define RT_GALORIAN 228
#define RT_GARGOYLE 29
#define RT_GARGOYLE_2 464
#define RT_GASBAG 30
#define RT_GELATINOUS_CUBE 31
#define RT_GELATINOUS_CUBE_2 712
#define RT_GELIDRAN 417
#define RT_GENARI 648
#define RT_GEONID 178
#define RT_GHOST 32
#define RT_GHOST_2 117
#define RT_GHOST_3 118
#define RT_GHOST_4 334
#define RT_GHOST_SHIP 114
#define RT_GHOST_SHIP_2 552
#define RT_GHOUL 33
#define RT_GHOUL_2 571
#define RT_GIANT 18
#define RT_GIANT_2 140
#define RT_GIANT_3 188
#define RT_GIANT_4 189
#define RT_GIANT_5 306
#define RT_GIANT_6 307
#define RT_GIANT_7 308
#define RT_GIANT_8 309
#define RT_GIANT_9 310
#define RT_GIANT_10 311
#define RT_GIANT_11 312
#define RT_GIANT_12 453
#define RT_GIANT_13 523
#define RT_GIANT_CLOCKWORK 275
#define RT_GIANT_RALLOSIAN_MATS 626
#define RT_GIANT_SHADE 526
#define RT_GIGYN 649
#define RT_GINGERBREAD_MAN 666
#define RT_GIRPLAN 419
#define RT_GNOLL 39
#define RT_GNOLL_2 524
#define RT_GNOLL_3 617
#define RT_GNOME 12
#define RT_GNOMEWORK 457
#define RT_GNOMISH_BALLOON 683
#define RT_GNOMISH_BOAT 545
#define RT_GNOMISH_HOVERING_TRANSPORT 685
#define RT_GNOMISH_ROCKET_PACK 684
#define RT_GOBLIN 40
#define RT_GOBLIN_2 59
#define RT_GOBLIN_3 137
#define RT_GOBLIN_4 369
#define RT_GOBLIN_5 433
#define RT_GOD_LUCLIN_VER_2 728
#define RT_GOD_LUCLIN_VER_3 729
#define RT_GOD_LUCLIN_VER_4 731
#define RT_GOD_OF_DISCORD 622
#define RT_GOLEM 17
#define RT_GOLEM_2 374
#define RT_GOO 145
#define RT_GOO_2 547
#define RT_GOO_3 548
#define RT_GOO_4 549
#define RT_GORAL 687
#define RT_GORAL_MOUNT 673
#define RT_GORGON 121
#define RT_GORILLA 41
#define RT_GORILLA_2 560
#define RT_GRANDFATHER_CLOCK 665
#define RT_GREKEN_YOUNG 651
#define RT_GREKEN_YOUNG_ADULT 650
#define RT_GRENDLAEN 701
#define RT_GRIEG_VENEFICUS 231
#define RT_GRIFFIN 47
#define RT_GRIFFIN_2 525
#define RT_GRIMLING 202
#define RT_GROUND_SHAKER 233
#define RT_GUARD 44
#define RT_GUARD_2 106
#define RT_GUARD_3 112
#define RT_GUARD_4 239
#define RT_GUARD_OF_JUSTICE 251
#define RT_GUARDIAN_CPU 593
#define RT_HADAL 698
#define RT_HAG 185
#define RT_HALF_ELF 7
#define RT_HALFLING 11
#define RT_HALFLING_2 81
#define RT_HARPY 111
#define RT_HARPY_2 527
#define RT_HIGH_ELF 5
#define RT_HIPPOGRIFF 186
#define RT_HOLGRESH 168
#define RT_HOLGRESH_2 715
#define RT_HONEY_POT 536
#define RT_HORSE 216
#define RT_HORSE_2 492
#define RT_HORSE_3 518
#define RT_HOVERING_PLATFORM 699
#define RT_HRAQUIS 261
#define RT_HUMAN 1
#define RT_HUMAN_2 67
#define RT_HUMAN_3 71
#define RT_HUMAN_4 566
#define RT_HUMAN_GHOST 588
#define RT_HUVUL 400
#define RT_HYDRA_CRYSTAL 615
#define RT_HYDRA_MOUNT 631
#define RT_HYDRA_NPC 632
#define RT_HYNID 388
#define RT_ICE_SPECTRE 174
#define RT_IKAAV 394
#define RT_IKSAR 128
#define RT_IKSAR_2 139
#define RT_IKSAR_GHOST 605
#define RT_IKSAR_GOLEM 160
#define RT_IKSAR_SKELETON 606
#define RT_IKSAR_SPIRIT 147
#define RT_IMP 46
#define RT_INNORUUK 123
#define RT_INSECT 370
#define RT_INTERACTIVE_OBJECT 2250
#define RT_INVISIBLE_MAN 127
#define RT_INVISIBLE_MAN_2 681
#define RT_INVISIBLE_MAN_3 690
#define RT_INVISIBLE_MAN_OF_ZOMM 600
#define RT_IXT 393
#define RT_JOKESTER 384
#define RT_JUM_JUM_BUCKET 537
#define RT_JUNK_BEAST 273
#define RT_KANGON 689
#define RT_KANGON_MOUNT 677
#define RT_KARANA 278
#define RT_KEDGE 103
#define RT_KEDGE_2 561
#define RT_KERRAN 23
#define RT_KERRAN_2 562
#define RT_KIRIN 434
#define RT_KIRIN_2 583
#define RT_KNIGHT_OF_HATE 351
#define RT_KNIGHT_OF_PESTILENCE 266
#define RT_KOBOLD 48
#define RT_KOBOLD_2 455
#define RT_KRAKEN 315
#define RT_KYV 396
#define RT_LAUNCH 73
#define RT_LAVA_ROCK 447
#define RT_LAVA_SPIDER 450
#define RT_LAVA_SPIDER_QUEEN 451
#define RT_LEECH 104
#define RT_LEPERTOLOTH 267
#define RT_LIGHTCRAWLER 223
#define RT_LIGHTNING_WARRIOR 407
#define RT_LION 50
#define RT_LIZARD_MAN 51
#define RT_LUCLIN 724
#define RT_LUGGALD 345
#define RT_LUGGALD_2 346
#define RT_LUGGALDS 347
#define RT_MALARIAN 265
#define RT_MAMMOTH 107
#define RT_MAMMOTH_2 528
#define RT_MAN_EATING_PLANT 162
#define RT_MANSION 595
#define RT_MANTICORE 172
#define RT_MANTRAP 573
#define RT_MARIONETTE 659
#define RT_MASTRUQ 402
#define RT_MATA_MURAM 406
#define RT_MEDIUM_PLANT 541
#define RT_MEPHIT 607
#define RT_MERCHANT_SHIP 550
#define RT_MERMAID 110
#define RT_MIMIC 52
#define RT_MINI_POM 252
#define RT_MINOTAUR 53
#define RT_MINOTAUR_2 420
#define RT_MINOTAUR_3 470
#define RT_MINOTAUR_4 574
#define RT_MITHANIEL_MARR 296
#define RT_MORELL_THULE 658
#define RT_MOSQUITO 134
#define RT_MOUTH_OF_INSANITY 281
#define RT_MUDDITE 608
#define RT_MUMMY 368
#define RT_MURAMITE_ARMOR_PILE 424
#define RT_MURKGLIDER 414
#define RT_MURKGLIDER_EGG_SAC 429
#define RT_MUTNA 401
#define RT_NEKHON 614
#define RT_NETHERBIAN 229
#define RT_NIGHTMARE 287
#define RT_NIGHTMARE_GARGOYLE 280
#define RT_NIGHTMARE_GOBLIN 277
#define RT_NIGHTMARE_MEPHIT 294
#define RT_NIGHTMARE_UNICORN 517
#define RT_NIGHTMARE_UNICORN_2 519
#define RT_NIGHTMARE_WRAITH 264
#define RT_NIHIL 385
#define RT_NILBORIEN 317
#define RT_NOC 397
#define RT_NYMPH 242
#define RT_OGRE 10
#define RT_OGRE_2 93
#define RT_OGRE_NPC_MALE 624
#define RT_ORB 730
#define RT_ORC 54
#define RT_ORC_2 458
#define RT_OTHMIR 190
#define RT_OWLBEAR 206
#define RT_PARASITIC_SCAVENGER 700
#define RT_PEGASUS 125
#define RT_PEGASUS_2 493
#define RT_PEGASUS_3 732
#define RT_PHOENIX 303
#define RT_PIRANHA 74
#define RT_PIRATE 335
#define RT_PIRATE_2 336
#define RT_PIRATE_3 337
#define RT_PIRATE_4 338
#define RT_PIRATE_5 339
#define RT_PIRATE_6 340
#define RT_PIRATE_7 341
#define RT_PIRATE_8 342
#define RT_PIRATE_SHIP 551
#define RT_PIXIE 56
#define RT_POISON_FROG 316
#define RT_PORTAL 426
#define RT_POWDER_KEG 636
#define RT_PRESSURE_PLATE 506
#define RT_PUFFER_SPORE 507
#define RT_PUMA 76
#define RT_PUMA_2 439
#define RT_PUMA_3 584
#define RT_PUSLING 270
#define RT_PYRILEN 411
#define RT_RA_TUK 398
#define RT_RABBIT 668
#define RT_RALLOS_ZEK 66
#define RT_RALLOS_ZEK_2 288
#define RT_RALLOS_ZEK_MINION 325
#define RT_RAPTOR 163
#define RT_RAPTOR_2 609
#define RT_RAPTOR_MOUNT 680
#define RT_RAT 36
#define RT_RAT_2 415
#define RT_RAT_MOUNT 656
#define RT_RATMAN 156
#define RT_RATMAN_2 718
#define RT_REANIMATED_HAND 80
#define RT_RECUSO 237
#define RT_REGENERATION_POOL 705
#define RT_RELIC_CASE 707
#define RT_RHINO_BEETLE 207
#define RT_RHINOCEROS 135
#define RT_ROBOCOPTER_OF_ZOMM 601
#define RT_ROCK_PILE 428
#define RT_ROCKHOPPER 200
#define RT_RONNIE_TEST 197
#define RT_ROOT_TENTACLE 509
#define RT_ROT_DOG_MOUNT 672
#define RT_ROTDOG 662
#define RT_ROTOCOPTER 577
#define RT_ROWBOAT 502
#define RT_ROYAL_GUARD 667
#define RT_RUJARKIAN_ORC 361
#define RT_RUJARKIAN_ORC_2 366
#define RT_RUNED_ORB 422
#define RT_RUNIC_SYMBOL 510
#define RT_SABER_TOOTHED_CAT 119
#define RT_SALTPETTER_BOMB 511
#define RT_SAND_ELF 364
#define RT_SANDMAN 664
#define RT_SARNAK 131
#define RT_SARNAK_2 610
#define RT_SARNAK_GOLEM 164
#define RT_SARNAK_SPIRIT 146
#define RT_SARYRN 283
#define RT_SATYR 529
#define RT_SCALED_WOLF 481
#define RT_SCARECROW 82
#define RT_SCARECROW_2 575
#define RT_SCARLET_CHEETAH 221
#define RT_SCLERA_MOUNT 675
#define RT_SCORPION 129
#define RT_SCORPION_2 149
#define RT_SCORPION_3 611
#define RT_SCRYKIN 495
#define RT_SEA_TURTLE 194
#define RT_SEAHORSE 116
#define RT_SELYRAH 686
#define RT_SELYRAH_MOUNT 674
#define RT_SERU 236
#define RT_SERVANT_OF_SHADOW 723
#define RT_SESSILOID_MOUNT 657
#define RT_SHADE 224
#define RT_SHADE_2 373
#define RT_SHADE_3 576
#define RT_SHADEL 205
#define RT_SHAMBLING_MOUND 494
#define RT_SHARK 61
#define RT_SHIKNAR 199
#define RT_SHILISKIN 467
#define RT_SHIP 72
#define RT_SHIP_IN_A_BOTTLE 702
#define RT_SHISSAR 217
#define RT_SHISSAR_2 563
#define RT_SHRIEKER 227
#define RT_SIREN 187
#define RT_SIREN_2 564
#define RT_SKELETAL_HORSE 282
#define RT_SKELETON 60
#define RT_SKELETON_2 367
#define RT_SKELETON_3 484
#define RT_SKUNK 83
#define RT_SKYSTRIDER 709
#define RT_SMALL_PLANT 540
#define RT_SNAKE 37
#define RT_SNAKE_2 468
#define RT_SNAKE_ELEMENTAL 84
#define RT_SNOW_DERVISH 170
#define RT_SNOW_RABBIT 176
#define RT_SOKOKAR 618
#define RT_SOKOKAR_MOUNT 625
#define RT_SOKOKAR_W_SADDLE 627
#define RT_SOLUSEK_RO 58
#define RT_SOLUSEK_RO_2 247
#define RT_SOLUSEK_RO_GUARD 254
#define RT_SONIC_WOLF 232
#define RT_SOUL_DEVOURER 286
#define RT_SPECTRE 85
#define RT_SPECTRE_2 485
#define RT_SPELL_PARTICLE_1 599
#define RT_SPHINX 86
#define RT_SPHINX_2 565
#define RT_SPIDER 38
#define RT_SPIDER_2 440
#define RT_SPIDER_EGG_SACK 449
#define RT_SPIDER_MOUNT 654
#define RT_SPIDER_QUEEN 441
#define RT_SPIKE_TRAP 513
#define RT_SPIRIT_WOLF 483
#define RT_SPORALI 456
#define RT_STONE_JUG 539
#define RT_STONE_PYLON 619
#define RT_STONE_RING 508
#define RT_STONE_WORKER 387
#define RT_STONE_WORKER_2 405
#define RT_STONEGRABBER 220
#define RT_STONEMITE 391
#define RT_STORMRIDER 272
#define RT_SUCCUBUS 408
#define RT_SUCCULENT 167
#define RT_SULLON_ZEK 499
#define RT_SUN_REVENANT 226
#define RT_SUNFLOWER 225
#define RT_SWINETOR 696
#define RT_SWORDFISH 105
#define RT_SYNARCANA 363
#define RT_TABLE 380
#define RT_TADPOLE 102
#define RT_TAELOSIAN 403
#define RT_TALL_PLANT 542
#define RT_TALLON_ZEK 290
#define RT_TANETH 399
#define RT_TAREW_MARR 246
#define RT_TEGI 215
#define RT_TELEPORT_MAN 240
#define RT_TELEPORTATION_STAND 706
#define RT_TELMIRA 653
#define RT_TENTACLE_TERROR 68
#define RT_TENTACLE_TERROR_2 578
#define RT_TERRIS_THULE 257
#define RT_TEST_OBJECT 301
#define RT_THE_RATHE 298
#define RT_THE_TRIBUNAL 256
#define RT_THOUGHT_HORROR 214
#define RT_TIGER 63
#define RT_TIN_SOLDIER 263
#define RT_TOOLBOX 538
#define RT_TOPIARY_LION 661
#define RT_TOPIARY_LION_MOUNT 671
#define RT_TORMENTOR 285
#define RT_TOTEM 173
#define RT_TOTEM_2 514
#define RT_TRAKANON 19
#define RT_TRANQUILION 262
#define RT_TREANT 64
#define RT_TREANT_2 244
#define RT_TREANT_3 496
#define RT_TRIBUNAL 151
#define RT_TRIUMVIRATE 697
#define RT_TROLL 9
#define RT_TROLL_2 92
#define RT_TROLL_3 331
#define RT_TROLL_4 332
#define RT_TROLL_5 333
#define RT_TROLL_ZOMBIE 344
#define RT_TRUSIK 386
#define RT_TSETSIAN 612
#define RT_TUMBLEWEED 694
#define RT_TUNARE 62
#define RT_TUREPTA 389
#define RT_UKUN 392
#define RT_ULTHORK 191
#define RT_UNDEAD_CHOKIDAI 357
#define RT_UNDEAD_FOOTMAN 324
#define RT_UNDEAD_FROGLOK 350
#define RT_UNDEAD_IKSAR 161
#define RT_UNDEAD_KNIGHT 297
#define RT_UNDEAD_SARNAK 155
#define RT_UNDEAD_VEKSAR 358
#define RT_UNDERBULK 201
#define RT_UNICORN 124
#define RT_UNKNOWN_RACE 0
#define RT_UNKNOWN_RACE_2 142
#define RT_UNKNOWN_RACE_3 143
#define RT_UNKNOWN_RACE_4 179
#define RT_UNKNOWN_RACE_5 180
#define RT_UNKNOWN_RACE_6 443
#define RT_UNKNOWN_RACE_7 444
#define RT_VAH_SHIR 130
#define RT_VAH_SHIR_2 238
#define RT_VAH_SHIR_SKELETON 234
#define RT_VALLON_ZEK 289
#define RT_VALORIAN 318
#define RT_VALORIAN_2 322
#define RT_VAMPIRE 65
#define RT_VAMPIRE_2 98
#define RT_VAMPIRE_3 208
#define RT_VAMPIRE_4 219
#define RT_VAMPIRE_5 359
#define RT_VAMPIRE_6 360
#define RT_VAMPIRE_7 365
#define RT_VAMPIRE_8 497
#define RT_VASE 379
#define RT_VEGEROG 258
#define RT_VEKSAR 353
#define RT_VEKSAR_2 354
#define RT_VEKSAR_3 355
#define RT_VENRIL_SATHIR 20
#define RT_VINE_MAW 717
#define RT_WAGON 621
#define RT_WALRUS 177
#define RT_WAR_BOAR 319
#define RT_WAR_BOAR_2 321
#define RT_WAR_WRAITH 313
#define RT_WASP 109
#define RT_WATER_ELEMENTAL 211
#define RT_WATER_ELEMENTAL_2 478
#define RT_WATER_MEPHIT 271
#define RT_WATER_SPOUT 710
#define RT_WEAPON_RACK 381
#define RT_WEAPON_RACK_2 534
#define RT_WEB 515
#define RT_WEDDING_ALTAR 635
#define RT_WEDDING_ARBOR 634
#define RT_WEDDING_FLOWERS 633
#define RT_WEREORC 579
#define RT_WEREWOLF 14
#define RT_WEREWOLF_2 241
#define RT_WEREWOLF_3 454
#define RT_WETFANG_MINNOW 213
#define RT_WHIRLIGIG 682
#define RT_WICKER_BASKET 516
#define RT_WILL_O_WISP 69
#define RT_WINE_CASK 543
#define RT_WINE_CASK_2 630
#define RT_WITHERAN 465
#define RT_WITHERAN_2 474
#define RT_WOLF 42
#define RT_WOLF_2 120
#define RT_WOLF_3 482
#define RT_WOOD_ELF 4
#define RT_WORG 580
#define RT_WORG_2 594
#define RT_WORM 203
#define RT_WRETCH 235
#define RT_WRULON 314
#define RT_WRULON_2 598
#define RT_WURM 158
#define RT_WURM_2 613
#define RT_WURM_MOUNT 679
#define RT_WYVERN 157
#define RT_WYVERN_2 581
#define RT_XALGOZ 136
#define RT_XARIC_THE_UNSPOKEN 725
#define RT_XEGONY 299
#define RT_YAKKAR 181
#define RT_YETI 138
#define RT_ZEBUXORUK 295
#define RT_ZEBUXORUKS_CAGE 328
#define RT_ZELNIAK 222
#define RT_ZOMBIE 70
#define RT_ZOMBIE_2 471


const char* GetRaceIDName(uint16 race_id);
const char* GetPlayerRaceName(uint32 player_race_value);

uint32 GetPlayerRaceValue(uint16 race_id);
uint32 GetPlayerRaceBit(uint16 race_id);

uint16 GetRaceIDFromPlayerRaceValue(uint32 player_race_value);
uint16 GetRaceIDFromPlayerRaceBit(uint32 player_race_bit);

float GetRaceGenderDefaultHeight(int race, int gender);


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

