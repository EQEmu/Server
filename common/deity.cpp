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


EQEmu::deity::DeityTypeBit EQEmu::deity::ConvertDeityTypeToDeityTypeBit(DeityType deity_type)
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
	};
}

EQEmu::deity::DeityType EQEmu::deity::ConvertDeityTypeBitToDeityType(DeityTypeBit deity_type_bit)
{
	switch (deity_type_bit) {
	case bit_DeityAgnostic:
		return DeityAgnostic;
	case bit_DeityBertoxxulous:
		return DeityBertoxxulous;
	case bit_DeityBrellSirilis:
		return DeityBrellSirilis;
	case bit_DeityCazicThule:
		return DeityCazicThule;
	case bit_DeityErollisiMarr:
		return DeityErollisiMarr;
	case bit_DeityBristlebane:
		return DeityBristlebane;
	case bit_DeityInnoruuk:
		return DeityInnoruuk;
	case bit_DeityKarana:
		return DeityKarana;
	case bit_DeityMithanielMarr:
		return DeityMithanielMarr;
	case bit_DeityPrexus:
		return DeityPrexus;
	case bit_DeityQuellious:
		return DeityQuellious;
	case bit_DeityRallosZek:
		return DeityRallosZek;
	case bit_DeityRodcetNife:
		return DeityRodcetNife;
	case bit_DeitySolusekRo:
		return DeitySolusekRo;
	case bit_DeityTheTribunal:
		return DeityTheTribunal;
	case bit_DeityTunare:
		return DeityTunare;
	case bit_DeityVeeshan:
		return DeityVeeshan;
	default:
		return DeityUnknown;
	};
}

const char* EQEmu::deity::DeityName(DeityType deity_type)
{
	switch (deity_type) {
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
