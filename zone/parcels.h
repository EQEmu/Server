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
#ifndef PARCELS_H
#define PARCELS_H

#include "../common/repositories/parcels_repository.h"
#include "../common/repositories/parcel_merchants_repository.h"

#define PARCEL_SEND_ITEMS    0
#define PARCEL_SEND_MONEY    1
#define PARCEL_MONEY_ITEM_ID 22292 // item id of copper coin
#define PARCEL_MAX_ITEMS     RuleI(Parcel, ParcelMaxItems)
#define PARCEL_LIMIT         PARCEL_MAX_ITEMS + 5
#define PARCEL_BEGIN_SLOT    1
#define PARCEL_END_SLOT      PARCEL_MAX_ITEMS

#endif

