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
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifndef _EQEMU_VERSION_H
#define _EQEMU_VERSION_H

#define LOGIN_VERSION "0.8.0"
#define EQEMU_PROTOCOL_VERSION "0.3.10"

#define CURRENT_VERSION "1.1.3"

/*
	Everytime a Database SQL is added to Github,
		increment CURRENT_BINARY_DATABASE_VERSION number and make sure you update the manifest
	Manifest: https://github.com/EQEmu/Server/blob/master/utils/sql/db_update_manifest.txt
*/

#define CURRENT_BINARY_DATABASE_VERSION 9115
#ifdef BOTS
	#define CURRENT_BINARY_BOTS_DATABASE_VERSION 9017
#else
	#define CURRENT_BINARY_BOTS_DATABASE_VERSION 0 // must be 0
#endif
#define COMPILE_DATE	__DATE__
#define COMPILE_TIME	__TIME__
#ifndef WIN32
	#define LAST_MODIFIED	__TIME__
#else
	#define LAST_MODIFIED	__TIMESTAMP__
#endif

#endif

