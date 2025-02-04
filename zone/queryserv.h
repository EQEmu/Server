#ifndef QUERYSERV_ZONE_H
#define QUERYSERV_ZONE_H

//#include "../common/types.h"
#include "../common/net/servertalk_server.h"
#include "../common/net/servertalk_client_connection.h"
//#include "../common/servertalk.h"
#include "../common/event/timer.h"


/*
	enum PlayerGenericLogEventTypes
	These Enums are for the generic logging table that are not complex and require more advanced logic
*/

enum PlayerGenericLogEventTypes {
	Player_Log_Quest = 1,
	Player_Log_Zoning,
	Player_Log_Deaths,
	Player_Log_Connect_State,
	Player_Log_Levels,
	Player_Log_Keyring_Addition,
	Player_Log_QGlobal_Update,
	Player_Log_Task_Updates,
	Player_Log_AA_Purchases,
	Player_Log_Trade_Skill_Events,
	Player_Log_Issued_Commands,
	Player_Log_Money_Transactions,
	Player_Log_Alternate_Currency_Transactions,
	Player_Log_Guild_Tribute_Item_Donation,
	Player_Log_Guild_Tribute_Plat_Donation
};


class QueryServ {
public:
	QueryServ();
	~QueryServ();
	void SendQuery(std::string Query);
	void Connect();
	bool SendPacket(ServerPacket *pack);
	void HandleMessage(uint16 opcode, const EQ::Net::Packet &p);

private:
	std::unique_ptr<EQ::Net::ServertalkClient> m_connection;
	bool                                       m_is_qs_connected{false };
};

class QueryServConnection
{
public:
	QueryServConnection();
	void AddConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection);
	void RemoveConnection(std::shared_ptr<EQ::Net::ServertalkServerConnection> connection);
	void HandleGenericMessage(uint16_t opcode, EQ::Net::Packet &p);
	void HandleLFGuildUpdateMessage(uint16_t opcode, EQ::Net::Packet &p);
	bool SendPacket(ServerPacket* pack);
	void OnKeepAlive(EQ::Timer *t);
private:
	std::map<std::string, std::shared_ptr<EQ::Net::ServertalkServerConnection>> m_streams;
	std::unique_ptr<EQ::Timer> m_keepalive;
};

#endif /* QUERYSERV_ZONE_H */
