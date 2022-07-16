/*	EQEMu: Everquest Server Emulator
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

#include "../common/global_define.h"
#include "../common/servertalk.h"
#include "../common/string_util.h"
#include "queryserv.h"
#include "worldserver.h"


extern WorldServer worldserver;
extern QueryServ *QServ;

QueryServ::QueryServ()
{
}

QueryServ::~QueryServ()
{
}

void QueryServ::SendQuery(std::string Query)
{
	auto pack = new ServerPacket(ServerOP_QSSendQuery, Query.length() + 5);
	pack->WriteUInt32(Query.length()); /* Pack Query String Size so it can be dynamically broken out at queryserv */
	pack->WriteString(Query.c_str()); /* Query */
	worldserver.SendPacket(pack);
	safe_delete(pack);
}

void QueryServ::PlayerLogEvent(int Event_Type, int Character_ID, std::string Event_Desc)
{
	std::string query = StringFormat(
		"INSERT INTO `qs_player_events` (event, char_id, event_desc, time) VALUES (%i, %i, '%s', UNIX_TIMESTAMP(now()))",
		Event_Type,
		Character_ID,
		EscapeString(Event_Desc).c_str());
	SendQuery(query);
}
