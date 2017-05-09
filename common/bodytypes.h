/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

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
#ifndef BODYTYPES_H
#define BODYTYPES_H

typedef enum {
	BT_Humanoid			= 1,
	BT_Lycanthrope		= 2,
	BT_Undead			= 3,
	BT_Giant			= 4,
	BT_Construct		= 5,
	BT_Extraplanar		= 6,
	BT_Magical			= 7,	//this name might be a bit off,
	BT_SummonedUndead	= 8,
	BT_RaidGiant		= 9,
	//					...
	BT_NoTarget			= 11,	//no name, can't target this bodytype
	BT_Vampire			= 12,
	BT_Atenha_Ra		= 13,
	BT_Greater_Akheva	= 14,
	BT_Khati_Sha		= 15,
	BT_Seru				= 16,	//not confirmed....
	BT_Draz_Nurakk      = 18,
	BT_Zek				= 19,
	BT_Luggald			= 20,
	BT_Animal			= 21,
	BT_Insect			= 22,
	BT_Monster			= 23,
	BT_Summoned			= 24,	//Elemental?
	BT_Plant			= 25,
	BT_Dragon			= 26,
	BT_Summoned2		= 27,
	BT_Summoned3		= 28,
						//29
	BT_VeliousDragon	= 30,	//might not be a tight set
	//					...
	BT_Dragon3			= 32,
	BT_Boxes			= 33,
	BT_Muramite			= 34,	//tribal dudes
	//					...
	BT_NoTarget2		= 60,
	//					...
	BT_SwarmPet			= 63,	//is this valid, or made up?
	//					...
	BT_InvisMan			= 66,	//no name, seen on 'InvisMan', can be /targeted
	BT_Special			= 67
} bodyType;
/* bodytypes above 64 make the mob not show up */

#endif
