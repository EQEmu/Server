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
#include "../common/debug.h"

// Quagmire - i was really surprised, but i couldnt find the equivilent standard library function
signed char sign(signed int tmp) {
	if (tmp > 0)
		return 1;
	else if (tmp < 0)
		return -1;
	else
		return 0;
}

signed char sign(double tmp) {
	if (tmp > 0)
		return 1;
	else if (tmp < 0)
		return -1;
	else
		return 0;
}

uint32 pow32(uint32 base, uint32 exponet) {
	uint32 ret = 1;
	for (uint32 i=1; i<=exponet; i++)
		ret *= base;
	return ret;
}

