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
#ifndef DEITY_H
#define DEITY_H

#include "types.h"
#include <string>

// NOTE: This code is not fully implemented since there are no references in the existing code

/*
**	Diety types
**
**	(ref: eqstr_us.txt)
**
**	(Another orphaned enumeration...)
*/
enum DeityTypes
{
/*----*/	DeityUnknown = 0,
/*----*/	DeityAgnostic_LB = 140,
/*3251*/	DeityBertoxxulous = 201,
/*3262*/	DeityBrellSirilis,
/*3253*/	DeityCazicThule,
/*3256*/	DeityErollisiMarr,
/*3252*/	DeityBristlebane,
/*3254*/	DeityInnoruuk,
/*3255*/	DeityKarana,
/*3257*/	DeityMithanielMarr,
/*3259*/	DeityPrexus,
/*3260*/	DeityQuellious,
/*3266*/	DeityRallosZek,
/*3258*/	DeityRodcetNife,
/*3261*/	DeitySolusekRo,
/*3263*/	DeityTheTribunal,
/*3264*/	DeityTunare,
/*3265*/	DeityVeeshan,
/*3250*/	DeityAgnostic = 396
};

/*
**	Deity type bits
**
**	(New orphan, but make use of it!)
*/
enum DeityTypeBits : uint32
{
	BIT_DeityAll			= 0x00000000,
	BIT_DeityAgnostic		= 0x00000001,
	BIT_DeityBertoxxulous	= 0x00000002,
	BIT_DeityBrellSirilis	= 0x00000004,
	BIT_DeityCazicThule		= 0x00000008,
	BIT_DeityErollisiMarr	= 0x00000010,
	BIT_DeityBristlebane	= 0x00000020,
	BIT_DeityInnoruuk		= 0x00000040,
	BIT_DeityKarana			= 0x00000080,
	BIT_DeityMithanielMarr	= 0x00000100,
	BIT_DeityPrexus			= 0x00000200,
	BIT_DeityQuellious		= 0x00000400,
	BIT_DeityRallosZek		= 0x00000800,
	BIT_DeityRodcetNife		= 0x00001000,
	BIT_DeitySolusekRo		= 0x00002000,
	BIT_DeityTheTribunal	= 0x00004000,
	BIT_DeityTunare			= 0x00008000,
	BIT_DeityVeeshan		= 0x00010000
};

static DeityTypeBits ConvertDeityToBitDeity(DeityTypes deity)
{
	switch(deity)
	{
		case DeityBertoxxulous:		{ return BIT_DeityBertoxxulous; }
		case DeityBrellSirilis:		{ return BIT_DeityBrellSirilis; }
		case DeityCazicThule:		{ return BIT_DeityCazicThule; }
		case DeityErollisiMarr:		{ return BIT_DeityErollisiMarr; }
		case DeityBristlebane:		{ return BIT_DeityBristlebane; }
		case DeityInnoruuk:			{ return BIT_DeityInnoruuk; }
		case DeityKarana:			{ return BIT_DeityKarana; }
		case DeityMithanielMarr:	{ return BIT_DeityMithanielMarr; }
		case DeityPrexus:			{ return BIT_DeityPrexus; }
		case DeityQuellious:		{ return BIT_DeityQuellious; }
		case DeityRallosZek:		{ return BIT_DeityRallosZek; }
		case DeityRodcetNife:		{ return BIT_DeityRodcetNife; }
		case DeitySolusekRo:		{ return BIT_DeitySolusekRo; }
		case DeityTheTribunal:		{ return BIT_DeityTheTribunal; }
		case DeityTunare:			{ return BIT_DeityTunare; }
		case DeityVeeshan:			{ return BIT_DeityVeeshan; }
		case DeityAgnostic_LB:
		case DeityAgnostic:			{ return BIT_DeityAgnostic; }
		default: { break; }
	};

	return BIT_DeityAll;
};

static DeityTypes ConvertBitDeityToDeity(DeityTypeBits deity_bit)
{
	switch(deity_bit)
	{
		case BIT_DeityAgnostic:			{ return DeityAgnostic; }
		case BIT_DeityBertoxxulous:		{ return DeityBertoxxulous; }
		case BIT_DeityBrellSirilis:		{ return DeityBrellSirilis; }
		case BIT_DeityCazicThule:		{ return DeityCazicThule; }
		case BIT_DeityErollisiMarr:		{ return DeityErollisiMarr; }
		case BIT_DeityBristlebane:		{ return DeityBristlebane; }
		case BIT_DeityInnoruuk:			{ return DeityInnoruuk; }
		case BIT_DeityKarana:			{ return DeityKarana; }
		case BIT_DeityMithanielMarr:	{ return DeityMithanielMarr; }
		case BIT_DeityPrexus:			{ return DeityPrexus; }
		case BIT_DeityQuellious:		{ return DeityQuellious; }
		case BIT_DeityRallosZek:		{ return DeityRallosZek; }
		case BIT_DeityRodcetNife:		{ return DeityRodcetNife; }
		case BIT_DeitySolusekRo:		{ return DeitySolusekRo; }
		case BIT_DeityTheTribunal:		{ return DeityTheTribunal; }
		case BIT_DeityTunare:			{ return DeityTunare; }
		case BIT_DeityVeeshan:			{ return DeityVeeshan; }
		default: { break; }
	};

	return DeityUnknown;
};

static std::string GetDeityName(DeityTypes deity)
{
	switch(deity)
	{
		case DeityBertoxxulous:		{ return "Bertoxxulous"; }
		case DeityBrellSirilis:		{ return "Brell Serilis"; }
		case DeityCazicThule:		{ return "Cazic-Thule"; }
		case DeityErollisiMarr:		{ return "Erollisi Marr"; }
		case DeityBristlebane:		{ return "Bristlebane"; }
		case DeityInnoruuk:			{ return "Innoruuk"; }
		case DeityKarana:			{ return "Karana"; }
		case DeityMithanielMarr:	{ return "Mithaniel Marr"; }
		case DeityPrexus:			{ return "Prexus"; }
		case DeityQuellious:		{ return "Quellious"; }
		case DeityRallosZek:		{ return "Rallos Zek"; }
		case DeityRodcetNife:		{ return "Rodcet Nife"; }
		case DeitySolusekRo:		{ return "Solusek Ro"; }
		case DeityTheTribunal:		{ return "The Tribunal"; }
		case DeityTunare:			{ return "Tunare"; }
		case DeityVeeshan:			{ return "Veeshan"; }
		case DeityAgnostic_LB:
		case DeityAgnostic:			{ return "Agnostic"; }
		default: { break; }
	};

	return "Unknown";
};

#endif
