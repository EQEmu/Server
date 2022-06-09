#ifndef QUERYSERV_ZONE_H
#define QUERYSERV_ZONE_H


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
};


class QueryServ{
	public:
		QueryServ();
		~QueryServ();
		void SendQuery(std::string Query);
		void PlayerLogEvent(int Event_Type, int Character_ID, std::string Event_Desc);
};

#endif /* QUERYSERV_ZONE_H */
