/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2004 EQEMu Development Team (http://eqemulator.org)

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

//extends the perl parser to use C methods
//instead of the command queue.

#ifndef PERLPARSER_H
#define PERLPARSER_H

#ifdef EMBPERL
#ifdef EMBPERL_XS

#include "embparser.h"

class PerlXSParser : public PerlembParser {
public:
	PerlXSParser();
//	~PerlXSParser();

	virtual void SendCommands(const char * pkgprefix, const char *event, uint32 object_id, Mob* other, Mob* mob, ItemInst* iteminst, const SPDat_Spell_Struct *spell);
protected:
	void map_funs();

	SV *_empty_sv;
};


#endif //EMBPERL_XS
#endif //EMBPERL

#endif //PERLPARSER_H
