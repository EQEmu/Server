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
	m_connection->OnMessage(std::bind(&QueryServ::HandleMessage, this, std::placeholders::_1, std::placeholders::_2));
	m_connection->OnConnect([this](EQ::Net::ServertalkClient *client) {
		m_is_qs_connected = true;
		LogInfo("Query Server connection established to [{}] [{}]", client->Handle()->RemoteIP(), client->Handle()->RemotePort());
	});

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

	if (m_is_qs_connected) {
		m_connection->SendPacket(pack);
		return true;
	}

	return false;
}

void QueryServ::HandleMessage(uint16 opcode, const EQ::Net::Packet &p)
{
	ServerPacket tpack(opcode, p);
	auto pack = &tpack;

	switch (opcode) {
		default: {
			LogInfo("Unknown ServerOP Received <red>[{}]", opcode);
			break;
		}
	}
}
