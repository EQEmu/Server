/*	EQEMu:  Everquest Server Emulator
	
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


EQEmu::Deity::TypeBits EQEmu::Deity::ConvertDeityToDeityBit(Types deity)
{
	switch (deity) {
	case DeityBertoxxulous:
		return BIT_DeityBertoxxulous;
	case DeityBrellSirilis:
		return BIT_DeityBrellSirilis;
	case DeityCazicThule:
		return BIT_DeityCazicThule;
	case DeityErollisiMarr:
		return BIT_DeityErollisiMarr;
	case DeityBristlebane:
		return BIT_DeityBristlebane;
	case DeityInnoruuk:
		return BIT_DeityInnoruuk;
	case DeityKarana:
		return BIT_DeityKarana;
	case DeityMithanielMarr:
		return BIT_DeityMithanielMarr;
	case DeityPrexus:
		return BIT_DeityPrexus;
	case DeityQuellious:
		return BIT_DeityQuellious;
	case DeityRallosZek:
		return BIT_DeityRallosZek;
	case DeityRodcetNife:
		return BIT_DeityRodcetNife;
	case DeitySolusekRo:
		return BIT_DeitySolusekRo;
	case DeityTheTribunal:
		return BIT_DeityTheTribunal;
	case DeityTunare:
		return BIT_DeityTunare;
	case DeityVeeshan:
		return BIT_DeityVeeshan;
	case DeityAgnostic_LB:
	case DeityAgnostic:
		return BIT_DeityAgnostic;
	default:
		return BIT_DeityAll;
	};
}

EQEmu::Deity::Types EQEmu::Deity::ConvertDeityBitToDeity(TypeBits deity_bit)
{
	switch (deity_bit) {
	case BIT_DeityAgnostic:
		return DeityAgnostic;
	case BIT_DeityBertoxxulous:
		return DeityBertoxxulous;
	case BIT_DeityBrellSirilis:
		return DeityBrellSirilis;
	case BIT_DeityCazicThule:
		return DeityCazicThule;
	case BIT_DeityErollisiMarr:
		return DeityErollisiMarr;
	case BIT_DeityBristlebane:
		return DeityBristlebane;
	case BIT_DeityInnoruuk:
		return DeityInnoruuk;
	case BIT_DeityKarana:
		return DeityKarana;
	case BIT_DeityMithanielMarr:
		return DeityMithanielMarr;
	case BIT_DeityPrexus:
		return DeityPrexus;
	case BIT_DeityQuellious:
		return DeityQuellious;
	case BIT_DeityRallosZek:
		return DeityRallosZek;
	case BIT_DeityRodcetNife:
		return DeityRodcetNife;
	case BIT_DeitySolusekRo:
		return DeitySolusekRo;
	case BIT_DeityTheTribunal:
		return DeityTheTribunal;
	case BIT_DeityTunare:
		return DeityTunare;
	case BIT_DeityVeeshan:
		return DeityVeeshan;
	default:
		return DeityUnknown;
	};
}

const char* EQEmu::Deity::GetDeityName(Types deity)
{
	switch (deity) {
	case DeityBertoxxulous:
		return "Bertoxxulous";
	case DeityBrellSirilis:
		return "Brell Serilis";
	case DeityCazicThule:
		return "Cazic-Thule";
	case DeityErollisiMarr:
		return "Erollisi Marr";
	case DeityBristlebane:
		return "Bristlebane";
	case DeityInnoruuk:
		return "Innoruuk";
	case DeityKarana:
		return "Karana";
	case DeityMithanielMarr:
		return "Mithaniel Marr";
	case DeityPrexus:
		return "Prexus";
	case DeityQuellious:
		return "Quellious";
	case DeityRallosZek:
		return "Rallos Zek";
	case DeityRodcetNife:
		return "Rodcet Nife";
	case DeitySolusekRo:
		return "Solusek Ro";
	case DeityTheTribunal:
		return "The Tribunal";
	case DeityTunare:
		return "Tunare";
	case DeityVeeshan:
		return "Veeshan";
	case DeityAgnostic_LB:
	case DeityAgnostic:
		return "Agnostic";
	default:
		return "Unknown";
	};
}
