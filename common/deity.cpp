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

#include "deity.h"

EQ::deity::DeityTypeBit EQ::deity::GetDeityBitmask(DeityType deity_type)
{
	switch (deity_type) {
		case DeityBertoxxulous:
			return bit_DeityBertoxxulous;
		case DeityBrellSirilis:
			return bit_DeityBrellSirilis;
		case DeityCazicThule:
			return bit_DeityCazicThule;
		case DeityErollisiMarr:
			return bit_DeityErollisiMarr;
		case DeityBristlebane:
			return bit_DeityBristlebane;
		case DeityInnoruuk:
			return bit_DeityInnoruuk;
		case DeityKarana:
			return bit_DeityKarana;
		case DeityMithanielMarr:
			return bit_DeityMithanielMarr;
		case DeityPrexus:
			return bit_DeityPrexus;
		case DeityQuellious:
			return bit_DeityQuellious;
		case DeityRallosZek:
			return bit_DeityRallosZek;
		case DeityRodcetNife:
			return bit_DeityRodcetNife;
		case DeitySolusekRo:
			return bit_DeitySolusekRo;
		case DeityTheTribunal:
			return bit_DeityTheTribunal;
		case DeityTunare:
			return bit_DeityTunare;
		case DeityVeeshan:
			return bit_DeityVeeshan;
		case DeityAgnostic_LB:
		case DeityAgnostic:
			return bit_DeityAgnostic;
		default:
			return bit_DeityAll;
	}
}

const std::map<EQ::deity::DeityType, std::string>& EQ::deity::GetDeityMap()
{
	static const std::map<EQ::deity::DeityType, std::string> deity_map = {
		{ DeityAgnostic, "Agnostic" },
		{ DeityAgnostic_LB, "Agnostic" },
		{ DeityBertoxxulous, "Bertoxxulous" },
		{ DeityBrellSirilis, "Brell Serilis" },
		{ DeityBristlebane, "Bristlebane" },
		{ DeityCazicThule, "Cazic-Thule" },
		{ DeityErollisiMarr, "Erollisi Marr" },
		{ DeityInnoruuk, "Innoruuk" },
		{ DeityKarana, "Karana" },
		{ DeityMithanielMarr, "Mithaniel Marr" },
		{ DeityPrexus, "Prexus" },
		{ DeityQuellious, "Quellious" },
		{ DeityRallosZek, "Rallos Zek" },
		{ DeityRodcetNife, "Rodcet Nife" },
		{ DeitySolusekRo, "Solusek Ro" },
		{ DeityTheTribunal, "The Tribunal" },
		{ DeityTunare, "Tunare" },
		{ DeityVeeshan, "Veeshan" }
	};

	return deity_map;
}

std::string EQ::deity::GetDeityName(DeityType deity_type)
{

	if (EQ::deity::GetDeityMap().find(deity_type) != EQ::deity::GetDeityMap().end()) {
		return EQ::deity::GetDeityMap().find(deity_type)->second;
	}

	return std::string();
}
