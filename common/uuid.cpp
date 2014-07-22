/*
	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2014 EQEMu Development Team (http://eqemulator.net)
	
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

#include "uuid.h"

#ifdef WIN32
#include <rpc.h>
#else
#include <uuid/uuid.h>
#endif

std::string CreateUUID() {
#ifdef WIN32
	UUID uuid;
	UuidCreate(&uuid);
	unsigned char *str = nullptr;
	UuidToStringA(&uuid, &str);
	std::string s((char*)str);
	RpcStringFreeA(&str);
	return s;
#else
	char str[64] = { 0 };
	uuid_t uuid;
	uuid_generate_random(uuid);
	uuid_unparse(uuid, str);
	return str;
#endif
}
