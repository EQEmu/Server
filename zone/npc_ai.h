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

#ifndef NPCAI_H
#define NPCAI_H
#include "../common/types.h"



/*
Even con color is wrong and so is the message but i can't seem to find it with a for loop.
Seems more like a bitwise comparison client side..
							con->level = 2; // GREEN
							con->level = 18; // LIGHT BLUE
							con->level = 4; // BLUE
							con->level = 20; // EVEN
							con->level = 15; // YELLOW
							con->level = 13; // RED
*/
#define DEFAULT_AGGRORADIUS 70
#define DEFAULT_FRENYRADIUS 70
#define MAX_SHIELDRADIUS 20

uint32 GetLevelCon(uint8 PlayerLevel, uint8 NPCLevel);

#endif
