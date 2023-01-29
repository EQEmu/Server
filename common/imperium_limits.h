/*	EQEMu: Everquest Server Emulator

	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef COMMON_IMPERIUM_LIMITS_H
#define COMMON_IMPERIUM_LIMITS_H

namespace Imperium
{
	const int16 IINVALID = -1;
	const int16 INULL = 0;

	namespace invtype {
		const int16 BANK_SIZE = 24;
		const int16 MERCHANT_SIZE = 200;
		const int16 BAZAAR_SIZE = 200;
		const int16 SHARED_BANK_SIZE = 2;
		const int16 TRADE_SIZE = 8;
		const int16 WORLD_SIZE = 10;
		const int16 TRIBUTE_SIZE = 5;
		const int16 GUILD_TRIBUTE_SIZE = 2;//unverified copy from rof2
	} /*invtype*/

	namespace invslot {
		const int16 BANK_BEGIN = 2001;
		const int16 BANK_END = (BANK_BEGIN + invtype::BANK_SIZE) - 1;

		const int16 SHARED_BANK_BEGIN = 2500;
		const int16 SHARED_BANK_END = (SHARED_BANK_BEGIN + invtype::SHARED_BANK_SIZE) - 1;

		const int16 TRADE_BEGIN = 6400;
		const int16 TRADE_END = (TRADE_BEGIN + invtype::TRADE_SIZE) - 1;

		const int16 WORLD_BEGIN = 7000;
		const int16 WORLD_END = (WORLD_BEGIN + invtype::WORLD_SIZE) - 1;

		const int16 TRIBUTE_BEGIN = 7500;
		const int16 TRIBUTE_END = (TRIBUTE_BEGIN + invtype::TRIBUTE_SIZE) - 1;

		const int16 GUILD_TRIBUTE_BEGIN = 7550;
		const int16 GUILD_TRIBUTE_END = (GUILD_TRIBUTE_BEGIN + invtype::GUILD_TRIBUTE_SIZE) - 1;
	} /*invslot*/

	namespace invbag {
		const int16 SLOT_INVALID = IINVALID;
		const int16 SLOT_BEGIN = INULL;
		const int16 SLOT_END = 49; //254;
		const int16 SLOT_COUNT = 50; //255;

		const int16 GENERAL_BAGS_BEGIN = 251; //kept this at titanium but moved it to here so it's easily changed

		const int16 CURSOR_BAG_BEGIN = 1100;

		const int16 SHARED_BANK_BAGS_BEGIN = 2600;

		const int16 BANK_BAGS_BEGIN = 2901;

		const int16 TRADE_BAGS_BEGIN = 6501;
	} /*invbag*/

	namespace constants {
		const int16 CHARACTER_CREATION_LIMIT = 12;

	}

}; /*Imperium*/
#endif /*COMMON_IMPERIUM_LIMITS_H*/
