/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

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
#ifndef EQEMuError_H
#define EQEMuError_H

#include "../common/types.h"

enum eEQEMuError { EQEMuError_NoError,
	EQEMuError_Mysql_1405,
	EQEMuError_Mysql_2003,
	EQEMuError_Mysql_2005,
	EQEMuError_Mysql_2007,
	EQEMuError_MaxErrorID };

void AddEQEMuError(eEQEMuError iError, bool iExitNow = false);
void AddEQEMuError(char* iError, bool iExitNow = false);
uint32 CheckEQEMuError();
void CheckEQEMuErrorAndPause();

#endif

