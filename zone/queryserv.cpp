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
#include "../common/strings.h"
#include "queryserv.h"
#include "worldserver.h"


extern WorldServer worldserver;
extern QueryServ  *QServ;

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

void QueryServ::Connect()
{
	m_connection = std::make_unique<EQ::Net::ServertalkClient>(Config->QSHost, Config->QSPort, false, "Zone", Config->SharedKey);
	m_keepalive = std::make_unique<EQ::Timer>(30000, true, std::bind(&QueryServ::OnKeepAlive, this, std::placeholders::_1));
	m_connection->OnMessage(std::bind(&QueryServ::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));

	is_qs_connected = true;
	LogInfo(
		"New Query Server connection to [{}:{}]",
		Config->QSHost,
		Config->QSPort
	);

}

bool QueryServ::SendPacket(ServerPacket *pack)
{
	if (m_connection.get() == nullptr) {
		Connect();
	}

	if (!m_connection.get()) {
		return false;
	}

	if (is_qs_connected) {
		m_connection->SendPacket(pack);
		return true;
	}

	LogInfo("SendPacket request with QS Server Offline.");
	return false;
}

void QueryServ::OnKeepAlive(EQ::Timer *t)
{
	ServerPacket pack(ServerOP_KeepAlive, 0);
	m_connection->SendPacket(&pack);
	is_qs_connected = false;
}

void QueryServ::HandleMessage(uint16 opcode, const EQ::Net::Packet &p)
{
	ServerPacket tpack(opcode, p);
	auto pack = &tpack;

	switch (opcode) {
		case ServerOP_KeepAlive: {
			is_qs_connected = true;
			LogInfo("ServerOP_KeepAlive Received from QueryServ::HandleMessage");
			break;
		}
		default: {
			LogInfo("Unknown ServerOP Received <red>[{}]", opcode);
			break;
		}
	}
}
