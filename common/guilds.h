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

#ifndef GUILD_H
#define GUILD_H

#include "types.h"

#define GUILD_NONE		0xFFFFFFFF // user has no guild

#define GUILD_MAX_RANK 8 // 0-2 - some places in the code assume a single digit, dont go above 9

//defines for standard ranks
#define GUILD_MEMBER	0
#define GUILD_OFFICER	1
#define GUILD_LEADER	2
#define GUILD_RANK_NONE (GUILD_MAX_RANK+1)

typedef enum {
	GUILD_HEAR		= 0,
	GUILD_SPEAK		= 1,
	GUILD_INVITE	= 2,
	GUILD_REMOVE	= 3,
	GUILD_PROMOTE	= 4,
	GUILD_DEMOTE	= 5,
	GUILD_MOTD		= 6,
	GUILD_WARPEACE	= 7,
	_MaxGuildAction
} GuildAction;

#endif
