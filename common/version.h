/*  EQEMu:  Everquest Server Emulator
    Copyright (C) 2001-2002  EQEMu Development Team (http://eqemu.org)

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

#ifndef VERSION_H
#define VERSION_H

#define CURRENT_VERSION	        "0.8.0"
#define CURRENT_CHAT_VERSION    "EQEmulator " CURRENT_VERSION
#define CURRENT_ZONE_VERSION	"EQEmulator " CURRENT_VERSION
#define CURRENT_WORLD_VERSION   "EQEmulator " CURRENT_VERSION
#define COMPILE_DATE	__DATE__
#define COMPILE_TIME	__TIME__
#define	SVN_REVISION		"$Rev: 986 $"
#ifndef WIN32
	#define LAST_MODIFIED	__TIME__
#else
	#define LAST_MODIFIED	__TIMESTAMP__
#endif

#endif

