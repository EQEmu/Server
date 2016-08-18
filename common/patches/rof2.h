/*	EQEMu: Everquest Server Emulator
	
	Copyright (C) 2001-2016 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef COMMON_ROF2_H
#define COMMON_ROF2_H

#include "../struct_strategy.h"

class EQStreamIdentifier;

namespace RoF2
{

	//these are the only public member of this namespace.
	extern void Register(EQStreamIdentifier &into);
	extern void Reload();



	//you should not directly access anything below..
	//I just dont feel like making a seperate header for it.

	class Strategy : public StructStrategy {
	public:
		Strategy();

	protected:

		virtual std::string Describe() const;
		virtual const EQEmu::versions::ClientVersion ClientVersion() const;

		//magic macro to declare our opcode processors
		#include "ss_declare.h"
		#include "rof2_ops.h"
	};

	enum class CastingSlot : uint32 {
		Gem1       = 0,
		Gem2       = 1,
		Gem3       = 2,
		Gem4       = 3,
		Gem5       = 4,
		Gem6       = 5,
		Gem7       = 6,
		Gem8       = 7,
		Gem9       = 8,
		Gem10      = 9,
		Gem11      = 10,
		Gem12      = 11,
		Item       = 12,
		Discipline = 13,
		AltAbility = 0xFF
	};

}; /*RoF2*/

#endif /*COMMON_ROF2_H*/
