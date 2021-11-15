#ifndef CLIENTENTRY_H_
#define CLIENTENTRY_H_

#include "../common/types.h"
#include "../common/md5.h"
//#include "../common/eq_packet_structs.h"
#include "../common/servertalk.h"
#include "../common/rulesys.h"
#include <vector>

typedef enum
{
	Never,
	Offline,
	Online,
	CharSelect,
	Zoning,
	InZone
} CLE_Status;

static const char * CLEStatusString[] = {
	"Never",
	"Offline",
	"Online",
	"CharSelect",
	"Zoning",
	"InZone"
};

class ZoneServer;
struct ServerClientList_Struct;

class ClientListEntry {
public:

	/**
	 * @param id
	 * @param in_loginserver_id
	 * @param in_loginserver_name
	 * @param in_login_name
	 * @param in_login_key
	 * @param in_is_world_admin
	 * @param ip
	 * @param local
	 */
	ClientListEntry(
		uint32 id,
		uint32 in_loginserver_id,
		const char *in_loginserver_name,
		const char *in_login_name,
		const char *in_login_key,
		int16 in_is_world_admin = 0,
		uint32 ip = 0,
		uint8 local = 0
	);

	/**
	 * @param id
	 * @param iZS
	 * @param scl
	 * @param iOnline
	 */
	ClientListEntry(uint32 id, uint32 iAccID, const char* iAccName, MD5& iMD5Pass, int16 iAdmin = AccountStatus::Player);
	ClientListEntry(uint32 id, ZoneServer* iZS, ServerClientList_Struct* scl, CLE_Status iOnline);
	~ClientListEntry();
	bool	CheckStale();
	void	Update(ZoneServer* zoneserver, ServerClientList_Struct* scl, CLE_Status iOnline = CLE_Status::InZone);
	void	LSUpdate(ZoneServer* zoneserver);
	void	LSZoneChange(ZoneToZone_Struct* ztz);
	bool	CheckAuth(uint32 loginserver_account_id, const char* key_password);
	bool	CheckAuth(const char* iName, MD5& iMD5Password);
	bool	CheckAuth(uint32 id, const char* key, uint32 ip);
	void	SetOnline(ZoneServer* iZS, CLE_Status iOnline);
	void	SetOnline(CLE_Status iOnline = CLE_Status::Online);
	void	SetChar(uint32 iCharID, const char* iCharName);
	inline CLE_Status Online()		{ return pOnline; }
	inline const uint32	GetID() const	{ return id; }
	inline const uint32	GetIP() const	{ return pIP; }
	inline void			SetIP(const uint32& iIP) { pIP = iIP; }
	inline void			KeepAlive()		{ stale = 0; }
	inline uint8			GetStaleCounter() const { return stale; }
	void	LeavingZone(ZoneServer* iZS = 0, CLE_Status iOnline = CLE_Status::Offline);
	void	Camp(ZoneServer* iZS = 0);

	// Login Server stuff
	inline const char*  LoginServer() const   { return source_loginserver; }
	inline uint32		LSID()	const		{ return pLSID; }
	inline uint32		LSAccountID() const	{ return pLSID; }
	inline const char*	LSName() const		{ return loginserver_account_name; }
	inline int16		WorldAdmin() const	{ return pworldadmin; }
	inline const char*	GetLSKey() const	{ return plskey; }
	inline const CLE_Status	GetOnline() const	{ return pOnline; }

	// Account stuff
	inline uint32		AccountID() const		{ return paccountid; }
	inline const char*	AccountName() const		{ return paccountname; }
	inline int16		Admin() const			{ return padmin; }
	inline void			SetAdmin(uint16 iAdmin)	{ padmin = iAdmin; }

	// Character info
	inline ZoneServer*	Server() const		{ return pzoneserver; }
	inline void			ClearServer()		{ pzoneserver = 0; }
	inline uint32		CharID() const		{ return pcharid; }
	inline const char*	name() const		{ return pname; }
	inline uint32		zone() const		{ return pzone; }
	inline uint16		instance() const	{ return pinstance; }
	inline uint8			level() const		{ return plevel; }
	inline uint8			class_() const		{ return pclass_; }
	inline uint16		race() const		{ return prace; }
	inline uint8			Anon()				{ return panon; }
	inline uint8			TellsOff() const	{ return ptellsoff; }
	inline uint32		GuildID() const	{ return pguild_id; }
	inline void			SetGuild(uint32 guild_id) { pguild_id = guild_id; }
	inline bool			LFG() const			{ return pLFG; }
	inline uint8			GetGM() const		{ return gm; }
	inline void			SetGM(uint8 igm)	{ gm = igm; }
	inline void			SetZone(uint32 zone) { pzone = zone; }
	inline bool	IsLocalClient() const { return plocal; }
	inline uint8			GetLFGFromLevel() const { return pLFGFromLevel; }
	inline uint8			GetLFGToLevel() const { return pLFGToLevel; }
	inline bool			GetLFGMatchFilter() const { return pLFGMatchFilter; }
	inline const char*		GetLFGComments() const { return pLFGComments; }
	inline uint8	GetClientVersion() { return pClientVersion; }

	inline bool TellQueueFull() const { return tell_queue.size() >= RuleI(World, TellQueueSize); }
	inline bool TellQueueEmpty() const { return tell_queue.empty(); }
	inline void PushToTellQueue(ServerChannelMessage_Struct *scm) { tell_queue.push_back(scm); }
	void ProcessTellQueue();

	void SetPendingExpeditionInvite(ServerPacket* pack) { p_pending_expedition_invite.reset(pack->Copy()); };
	std::unique_ptr<ServerPacket> GetPendingExpeditionInvite() { return std::move(p_pending_expedition_invite); }

private:
	void	ClearVars(bool iAll = false);

	const uint32	id;
	uint32	pIP;
	CLE_Status pOnline;
	uint8	stale;

	// Login Server stuff
	char	source_loginserver[64]{}; //Loginserver we came from.
	uint32	pLSID;
	char	loginserver_account_name[32]{};
	char	plskey[16]{};
	int16	pworldadmin;		// Login server's suggested admin status setting
	bool	plocal;

	// Account stuff
	uint32	paccountid;
	char	paccountname[32]{};
	MD5		pMD5Pass;
	int16	padmin{};

	// Character info
	ZoneServer* pzoneserver{};
	uint32	pzone{};
	uint16	pinstance{};
	uint32	pcharid{};
	char	pname[64]{};
	uint8	plevel{};
	uint8	pclass_{};
	uint16	prace{};
	uint8	panon{};
	uint8	ptellsoff{};
	uint32	pguild_id{};
	bool	pLFG{};
	uint8	gm{};
	uint8	pClientVersion{};
	uint8	pLFGFromLevel{};
	uint8	pLFGToLevel{};
	bool	pLFGMatchFilter{};
	char	pLFGComments[64]{};

	// Tell Queue -- really a vector :D
	std::vector<ServerChannelMessage_Struct *> tell_queue;

	std::unique_ptr<ServerPacket> p_pending_expedition_invite = nullptr;
};

#endif /*CLIENTENTRY_H_*/

