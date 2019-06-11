#pragma once

#include "../common/types.h"
#include "../common/packet_functions.h"
#include "../common/eq_packet_structs.h"
#include "../common/serialize_buffer.h"
#include "../net/packet.h"
#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>

constexpr auto INTERSERVER_TIMER = 10000;
constexpr auto LoginServer_StatusUpdateInterval = 15000;

//Certain ops needed for backwards compat with old LS can't enum without being really annoying.
constexpr auto ServerOP_UsertoWorldReq		= 0xAB00;
constexpr auto ServerOP_UsertoWorldResp		= 0xAB01;
constexpr auto ServerOP_LSClientAuth		= 0x1002;
constexpr auto ServerOP_LSFatalError		= 0x1003;
constexpr auto ServerOP_SystemwideMessage	= 0x1005;
constexpr auto ServerOP_LSRemoteAddr		= 0x1009;
constexpr auto ServerOP_LSAccountUpdate		= 0x100A;
constexpr auto ServerOP_NewLSInfo			= 0x1008;
constexpr auto ServerOP_LSInfo				= 0x1000;
constexpr auto ServerOP_LSStatus			= 0x1001;

enum ServerOpcode : int
{
	ServerOP_ChannelMessage,
	ServerOP_SetZone,
	ServerOP_ShutdownAll,
	ServerOP_ZoneShutdown,
	ServerOP_ZoneBootup,
	ServerOP_ZoneStatus,
	ServerOP_SetConnectInfo,
	ServerOP_EmoteMessage,
	ServerOP_ClientList,
	ServerOP_Who,
	ServerOP_ZonePlayer,
	ServerOP_KickPlayer,
	ServerOP_RefreshGuild,
	ServerOP_VoiceMacro,
	//ServerOP_GuildInvite,
	ServerOP_DeleteGuild,
	ServerOP_GuildRankUpdate,
	ServerOP_GuildCharRefresh,
	ServerOP_GuildMemberUpdate,
	ServerOP_RequestOnlineGuildMembers,
	ServerOP_OnlineGuildMembersResponse,
	ServerOP_LFGuildUpdate,
	ServerOP_FlagUpdate,
	ServerOP_GMGoto,
	ServerOP_MultiLineMsg,
	ServerOP_Lock,
	ServerOP_Motd,
	ServerOP_Uptime,
	ServerOP_Petition,
	ServerOP_KillPlayer,
	ServerOP_UpdateGM,
	ServerOP_RezzPlayer,
	ServerOP_ZoneReboot,
	ServerOP_ZoneToZoneRequest,
	ServerOP_AcceptWorldEntrance,
	ServerOP_ZAAuth,
	ServerOP_ZAAuthFailed,
	ServerOP_ZoneIncClient,
	ServerOP_ClientListKA,
	ServerOP_ChangeWID,
	ServerOP_IPLookup,
	ServerOP_LockZone,
	ServerOP_ItemStatus,
	ServerOP_OOCMute,
	ServerOP_Revoke,
	ServerOP_WebInterfaceCall,
	ServerOP_GroupIDReq,
	ServerOP_GroupIDReply,
	ServerOP_GroupLeave,
	ServerOP_RezzPlayerAccept,
	ServerOP_SpawnCondition,
	ServerOP_SpawnEvent,
	ServerOP_SetLaunchName,
	ServerOP_RezzPlayerReject,
	ServerOP_SpawnPlayerCorpse,
	ServerOP_Consent,
	ServerOP_Consent_Response,
	ServerOP_ForceGroupUpdate,
	ServerOP_OOZGroupMessage,
	ServerOP_DisbandGroup,
	ServerOP_GroupJoin,
	ServerOP_UpdateSpawn,
	ServerOP_SpawnStatusChange,
	ServerOP_ReloadTasks,
	ServerOP_DepopAllPlayersCorpses,
	ServerOP_ReloadTitles,
	ServerOP_QGlobalUpdate,
	ServerOP_QGlobalDelete,
	ServerOP_DepopPlayerCorpse,
	ServerOP_RequestTellQueue,
	ServerOP_ChangeSharedMem,
	ServerOP_WebInterfaceEvent,
	ServerOP_WebInterfaceSubscribe,
	ServerOP_WebInterfaceUnsubscribe,
	ServerOP_RaidAdd,
	ServerOP_RaidRemove,
	ServerOP_RaidDisband,
	ServerOP_RaidLockFlag,
	ServerOP_RaidGroupLeader,
	ServerOP_RaidLeader,
	ServerOP_RaidGroupSay,
	ServerOP_RaidSay,
	ServerOP_DetailsChange,
	ServerOP_UpdateGroup,
	ServerOP_RaidGroupDisband,
	ServerOP_RaidChangeGroup,
	ServerOP_RaidGroupAdd,
	ServerOP_RaidGroupRemove,
	ServerOP_GroupInvite,
	ServerOP_GroupFollow,
	ServerOP_GroupFollowAck,
	ServerOP_GroupCancelInvite,
	ServerOP_RaidMOTD,
	ServerOP_InstanceUpdateTime,
	ServerOP_AdventureRequest,
	ServerOP_AdventureRequestAccept,
	ServerOP_AdventureRequestDeny,
	ServerOP_AdventureRequestCreate,
	ServerOP_AdventureData,
	ServerOP_AdventureDataClear,
	ServerOP_AdventureCreateDeny,
	ServerOP_AdventureDataRequest,
	ServerOP_AdventureClickDoor,
	ServerOP_AdventureClickDoorReply,
	ServerOP_AdventureClickDoorError,
	ServerOP_AdventureLeave,
	ServerOP_AdventureLeaveReply,
	ServerOP_AdventureLeaveDeny,
	ServerOP_AdventureCountUpdate,
	ServerOP_AdventureZoneData,
	ServerOP_AdventureAssaCountUpdate,
	ServerOP_AdventureFinish,
	ServerOP_AdventureLeaderboard,
	ServerOP_WhoAll,
	ServerOP_FriendsWho,
	ServerOP_LFGMatches,
	ServerOP_LFPUpdate,
	ServerOP_LFPMatches,
	ServerOP_ClientVersionSummary,
	ServerOP_ListWorlds,
	ServerOP_PeerConnect,
	ServerOP_TaskRequest,
	ServerOP_TaskGrant,
	ServerOP_TaskReject,
	ServerOP_TaskAddPlayer,
	ServerOP_TaskRemovePlayer,
	ServerOP_TaskZoneCreated,
	ServerOP_TaskZoneFailed,
	ServerOP_EncapPacket,
	ServerOP_WorldListUpdate,
	ServerOP_WorldListRemove,
	ServerOP_TriggerWorldListRefresh,
	ServerOP_WhoAllReply,
	ServerOP_SetWorldTime,
	ServerOP_GetWorldTime,
	ServerOP_SyncWorldTime,
	ServerOP_RefreshCensorship,
	ServerOP_LSZoneInfo,
	ServerOP_LSZoneStart,
	ServerOP_LSZoneBoot,
	ServerOP_LSZoneShutdown,
	ServerOP_LSZoneSleep,
	ServerOP_LSPlayerLeftWorld,
	ServerOP_LSPlayerJoinWorld,
	ServerOP_LSPlayerZoneChange,
	ServerOP_LauncherConnectInfo,
	ServerOP_LauncherZoneRequest,
	ServerOP_LauncherZoneStatus,
	ServerOP_DoZoneCommand,
	ServerOP_UCSMessage,
	ServerOP_UCSMailMessage,
	ServerOP_ReloadRules,
	ServerOP_ReloadRulesWorld,
	ServerOP_CameraShake,
	ServerOP_QueryServGeneric,
	ServerOP_CZSignalClient,
	ServerOP_CZSignalClientByName,
	ServerOP_CZMessagePlayer,
	ServerOP_ReloadWorld,
	ServerOP_ReloadLogs,
	ServerOP_ReloadPerlExportSettings,
	ServerOP_CZSetEntityVariableByClientName,
	ServerOP_UCSServerStatusRequest,
	ServerOP_UCSServerStatusReply,
	ServerOP_Speech,
	ServerOP_QSPlayerLogTrades,
	ServerOP_QSPlayerLogHandins,
	ServerOP_QSPlayerLogNPCKills,
	ServerOP_QSPlayerLogDeletes,
	ServerOP_QSPlayerLogMoves,
	ServerOP_QSPlayerLogMerchantTransactions,
	ServerOP_QSSendQuery,
	ServerOP_CZSignalNPC,
	ServerOP_CZSetEntityVariableByNPCTypeID,
	ServerOP_WWMarquee,
	ServerOP_QSPlayerDropItem,
	ServerOP_RouteTo,

	/*Tasks*/
	ServerOP_GetClientTaskState
};




/* Query Serv Generic Packet Flag/Type Enumeration */
enum { QSG_LFGuild = 0 }; 
enum {	QSG_LFGuild_PlayerMatches = 0, QSG_LFGuild_UpdatePlayerInfo, QSG_LFGuild_RequestPlayerInfo, QSG_LFGuild_UpdateGuildInfo, QSG_LFGuild_GuildMatches,
	QSG_LFGuild_RequestGuildInfo };

/************ PACKET RELATED STRUCT ************/
class ServerPacket
{
public:
	~ServerPacket() { safe_delete_array(pBuffer); }
	ServerPacket(uint16 in_opcode = 0, uint32 in_size = 0) {
		this->compressed = false;
		size = in_size;
		opcode = in_opcode;
		if (size == 0) {
			pBuffer = 0;
		}
		else {
			pBuffer = new uchar[size];
			memset(pBuffer, 0, size);
		}
		_wpos = 0;
		_rpos = 0;
	}

	ServerPacket(uint16 in_opcode, const EQ::Net::Packet &p) {
		this->compressed = false;
		size = (uint32)p.Length();
		opcode = in_opcode;
		if (size == 0) {
			pBuffer = 0;
		}
		else {
			pBuffer = new uchar[size];
			memcpy(pBuffer, p.Data(), size);
		}
		_wpos = 0;
		_rpos = 0;
	}

	ServerPacket(uint16 in_opcode, SerializeBuffer &buf)
	{
		compressed = false;
		size = buf.m_pos;
		buf.m_pos = 0;
		opcode = in_opcode;
		pBuffer = buf.m_buffer;
		buf.m_buffer = 0;
		buf.m_capacity = 0;
		_wpos = 0;
		_rpos = 0;
	}

	ServerPacket* Copy() {
		if (this == 0) {
			return 0;
		}
		ServerPacket* ret = new ServerPacket(this->opcode, this->size);
		if (this->size)
			memcpy(ret->pBuffer, this->pBuffer, this->size);
		ret->compressed = this->compressed;
		ret->InflatedSize = this->InflatedSize;
		return ret;
	}

	void WriteUInt8(uint8 value) { *(uint8 *)(pBuffer + _wpos) = value; _wpos += sizeof(uint8); }
	void WriteUInt32(uint32 value) { *(uint32 *)(pBuffer + _wpos) = value; _wpos += sizeof(uint32); }
	void WriteString(const char * str) { uint32 len = static_cast<uint32>(strlen(str)) + 1; memcpy(pBuffer + _wpos, str, len); _wpos += len; }

	uint8 ReadUInt8() { uint8 value = *(uint8 *)(pBuffer + _rpos); _rpos += sizeof(uint8); return value; }
	uint32 ReadUInt32() { uint32 value = *(uint32 *)(pBuffer + _rpos); _rpos += sizeof(uint32); return value; }
	void ReadString(char *str) { uint32 len = static_cast<uint32>(strlen((char *)(pBuffer + _rpos))) + 1; memcpy(str, pBuffer + _rpos, len); _rpos += len; }

	uint32 GetWritePosition() { return _wpos; }
	uint32 GetReadPosition() { return _rpos; }
	void SetWritePosition(uint32 Newwpos) { _wpos = Newwpos; }
	void WriteSkipBytes(uint32 count) { _wpos += count; }
	void ReadSkipBytes(uint32 count) { _rpos += count; }
	void SetReadPosition(uint32 Newrpos) { _rpos = Newrpos; }

	uint32	size;
	uint16	opcode;
	uchar*	pBuffer;
	uint32	_wpos;
	uint32	_rpos;
	bool	compressed;
	uint32	InflatedSize;
	uint32	destination;
};

#pragma pack(1)

struct SPackSendQueue {
	uint16 size;
	uchar buffer[0];
};

struct ServerZoneStateChange_struct {
	uint32 ZoneServerID;
	char adminname[64];
	uint32 zoneid;
	uint16 instanceid;
	bool makestatic;
};

struct ServerZoneIncomingClient_Struct {
	uint32	zoneid;		// in case the zone shut down, boot it back up
	uint16	instanceid; // instance id if it exists for booting up
	uint32	ip;			// client's IP address
	uint32	wid;		// client's WorldID#
	uint32	accid;
	int16	admin;
	uint32	charid;
	bool	tellsoff;
	char	charname[64];
	char	lskey[30];
};

struct ServerChangeWID_Struct {
	uint32	charid;
	uint32	newwid;
};
struct SendGroup_Struct{
	uint8	grouptotal;
	uint32	zoneid;
	char	leader[64];
	char	thismember[64];
	char	members[5][64];
};

struct ServerGroupFollow_Struct {
	uint32 CharacterID;
	GroupGeneric_Struct gf;
};

struct ServerGroupFollowAck_Struct {
	char Name[64];
};


struct ServerChannelMessage_Struct {
	char deliverto[64];
	char to[64];
	char from[64];
	uint8 fromadmin;
	bool noreply;
	uint16 chan_num;
	uint32 guilddbid;
	uint16 language;
	uint8 queued; // 0 = not queued, 1 = queued, 2 = queue full, 3 = offline
	char message[0];
};

struct ServerEmoteMessage_Struct {
	char	to[64];
	uint32	guilddbid;
	int16	minstatus;
	uint32	type;
	char	message[0];
};

struct ServerVoiceMacro_Struct {
	char	From[64];
	union {
		char	To[64];
		uint32	GroupID;
		uint32	RaidID;
	};
	uint32	Type;
	uint32	Voice;
	uint32	MacroNumber;
};

struct ServerClientList_Struct {
	uint8	remove;
	uint32	wid;
	uint32	IP;
	uint32	zone;
	uint16	instance_id;
	int16	Admin;
	uint32	charid;
	char	name[64];
	uint32	AccountID;
	char	AccountName[30];
	uint32	LSAccountID;
	char	lskey[30];
	uint16	race;
	uint8	class_;
	uint8	level;
	uint8	anon;
	bool	tellsoff;
	uint32	guild_id;
	bool	LFG;
	uint8	gm;
	uint8	ClientVersion;
	uint8	LFGFromLevel;
	uint8	LFGToLevel;
	bool	LFGMatchFilter;
	char	LFGComments[64];
};

struct ServerClientListKeepAlive_Struct {
	uint32	numupdates;
	uint32	wid[0];
};

struct ServerZonePlayer_Struct {
	char	adminname[64];
	int16	adminrank;
	uint8	ignorerestrictions;
	char	name[64];
	char	zone[25];
	uint32	instance_id;
	float	x_pos;
	float	y_pos;
	float	z_pos;
};

struct RezzPlayer_Struct {
	uint32	dbid;
	uint32	exp;
	uint16	rezzopcode;
	//char	packet[160];
	Resurrect_Struct rez;
};

struct ServerZoneReboot_Struct {
//	char	ip1[250];
	char	ip2[250];
	uint16	port;
	uint32	zoneid;
};

struct SetZone_Struct {
	uint16	instanceid;
	uint32	zoneid;
	bool	staticzone;
};

struct ServerKickPlayer_Struct {
	char adminname[64];
	int16 adminrank;
	char name[64];
	uint32 AccountID;
};

struct ServerLSInfo_Struct {
	char	name[201];				// name the worldserver wants
	char	address[250];			// DNS address of the server
	char	account[31];			// account name for the worldserver
	char	password[31];			// password for the name
	char	protocolversion[25];	// Major protocol version number
	char	serverversion[64];		// minor server software version number
	uint8	servertype;				// 0=world, 1=chat, 2=login, 3=MeshLogin
};

struct ServerNewLSInfo_Struct {
	char	name[201];				// name the worldserver wants
	char	shortname[50];				// shortname the worldserver wants
	char	remote_address[125];			// DNS address of the server
	char	local_address[125];			// DNS address of the server
	char	account[31];			// account name for the worldserver
	char	password[31];			// password for the name
	char	protocolversion[25];	// Major protocol version number
	char	serverversion[64];		// minor server software version number
	uint8	servertype;				// 0=world, 1=chat, 2=login, 3=MeshLogin
};

struct ServerLSAccountUpdate_Struct {			// for updating info on login server
	char	worldaccount[31];			// account name for the worldserver
	char	worldpassword[31];			// password for the name
	uint32	useraccountid;				// player account ID
	char	useraccount[31];			// player account name
	char	userpassword[51];			// player account password
	char	useremail[101];				// player account email address
};

struct ServerLSStatus_Struct {
	int32 status;
	int32 num_players;
	int32 num_zones;
};
struct ZoneInfo_Struct {
	uint32 zone;
	uint16 count;
	uint32 zone_wid;
};
struct ZoneBoot_Struct {
	uint32 zone;
	uint32 instance;
	char compile_time[25];
	uint32 zone_wid;
};
struct ZoneShutdown_Struct {
	uint32 zone;
	uint32 instance;
	uint32 zone_wid;
};
struct ServerLSZoneSleep_Struct {
	uint32 zone;
	uint32 instance;
	uint32 zone_wid;
};

struct ServerLSPlayerJoinWorld_Struct {
	uint32 lsaccount_id;
	char key[30];
};

struct ServerLSPlayerLeftWorld_Struct {
	uint32 lsaccount_id;
	char key[30];
};

struct ServerLSPlayerZoneChange_Struct {
	uint32 lsaccount_id;
	uint32 from; // 0 = world
	uint32 to; // 0 = world
};

struct ClientAuth_Struct {
	uint32 lsaccount_id; // ID# in login server's db
	char name[30]; // username in login server's db
	char key[30]; // the Key the client will present
	uint8 lsadmin; // login server admin level
	int16 worldadmin; // login's suggested worldadmin level setting for this user, up to the world if they want to obey it
	uint32 ip;
	uint8 local; // 1 if the client is from the local network

	template <class Archive>
	void serialize(Archive &ar)
	{
		ar(lsaccount_id, name, key, lsadmin, worldadmin, ip, local);
	}
};

struct ServerSystemwideMessage {
	uint32	lsaccount_id;
	char	key[30];		// sessionID key for verification
	uint32	type;
	char	message[0];
};

struct ServerLSPeerConnect {
	uint32	ip;
	uint16	port;
};

struct ServerConnectInfo {
	char	address[250];
	char	local_address[250];
	uint16	port;
	uint32  process_id;
};

struct ServerGMGoto_Struct {
	char	myname[64];
	char	gotoname[64];
	int16	admin;
};

struct ServerMultiLineMsg_Struct {
	char	to[64];
	char	message[0];
};

struct ServerLock_Struct {
	char	myname[64]; // User that did it
	uint8	mode; // 0 = Unlocked ; 1 = Locked
};

struct ServerMotd_Struct {
	char	myname[64]; // User that set the motd
	char	motd[512]; // the new MoTD
};

struct ServerUptime_Struct {
	uint32	zoneserverid;	// 0 for world
	char	adminname[64];
};

struct ServerPetitionUpdate_Struct {
	uint32 petid; // Petition Number
	uint8 status; // 0x00 = ReRead DB -- 0x01 = Checkout -- More? Dunno... lol
};

struct ServerWhoAll_Struct {
	int16 admin;
	uint32 fromid;
	char from[64];
	char whom[64];
	uint16 wrace; // FF FF = no race
	uint16 wclass; // FF FF = no class
	uint16 lvllow; // FF FF = no numbers
	uint16 lvlhigh; // FF FF = no numbers
	uint16 gmlookup; // FF FF = not doing /who all gm
};

struct ServerFriendsWho_Struct {
	uint32 FromID;
	char FromName[64];
	char FriendsString[1];
};

struct ServerKillPlayer_Struct {
	char gmname[64];
	char target[64];
	int16 admin;
};

struct ServerUpdateGM_Struct {
	char gmname[64];
	bool gmstatus;
};

struct ServerEncapPacket_Struct {
	uint32	ToID;	// ID number of the LWorld on the other server
	uint16	opcode;
	uint16	size;
	uchar	data[0];
};

struct ZoneToZone_Struct {
	char	name[64];
	uint32	guild_id;
	uint32	requested_zone_id;
	uint32	requested_instance_id;
	uint32	current_zone_id;
	uint32	current_instance_id;
	int8	response;
	int16	admin;
	uint8	ignorerestrictions;
};

struct WorldToZone_Struct {
	uint32	account_id;
	int8	response;
};
struct WorldShutDown_Struct {
	uint32	time;
	uint32	interval;
};
struct ServerSyncWorldList_Struct {
	uint32	RemoteID;
	uint32	ip;
	int32	status;
	char	name[201];
	char	address[250];
	char	account[31];
	uint32	accountid;
	uint8	authlevel;
	uint8	servertype;		// 0=world, 1=chat, 2=login
	uint32	adminid;
	uint8	greenname;
	uint8	showdown;
	int32	num_players;
	int32	num_zones;
	bool	placeholder;
};

struct UsertoWorldRequest_Struct {
	uint32	lsaccountid;
	uint32	worldid;
	uint32	FromID;
	uint32	ToID;
	char	IPAddr[64];
};

struct UsertoWorldResponse_Struct {
	uint32	lsaccountid;
	uint32	worldid;
	int8	response; // -3) World Full, -2) Banned, -1) Suspended, 0) Denied, 1) Allowed
	uint32	FromID;
	uint32	ToID;
};

// generic struct to be used for alot of simple zone->world questions
struct ServerGenericWorldQuery_Struct {
	char	from[64];	// charname the query is from
	int16	admin;		// char's admin level
	char	query[0];	// text of the query
};

struct ServerLockZone_Struct {
	uint8	op;
	char	adminname[64];
	uint16	zoneID;
};

struct RevokeStruct {
	char adminname[64];
	char name[64];
	bool toggle; //0 off 1 on
};

struct ServerGroupIDReply_Struct {
	uint32 start;	//a range of group IDs to use.
	uint32 end;
};

struct ServerGroupLeave_Struct {
	uint32 zoneid;
	uint16 instance_id;
	uint32 gid;
	char member_name[64];	//kick this member from the group
};

struct ServerGroupJoin_Struct {
	uint32 zoneid;
	uint16 instance_id;
	uint32 gid;
	char member_name[64];	//this person is joining the group
};

struct ServerForceGroupUpdate_Struct {
	uint32 origZoneID;
	uint16 instance_id;
	uint32 gid;
};

struct ServerGroupChannelMessage_Struct {
	uint32 zoneid;
	uint16 instanceid;
	uint32 groupid;
	char from[64];
	char message[0];
};

struct ServerDisbandGroup_Struct {
	uint32 zoneid;
	uint16 instance_id;
	uint32 groupid;
};

struct SimpleName_Struct{
	char name[64];
};

struct ServerSpawnCondition_Struct {
	uint32 zoneID;
	uint32 instanceID;
	uint16 condition_id;
	int16 value;
};

struct ServerSpawnEvent_Struct {
	uint32	zoneID;
	uint32	event_id;
};

//zone->world
struct LaunchName_Struct {
	char launcher_name[32];
	char zone_name[16];
};

struct LauncherConnectInfo {
	char name[64];
};

typedef enum {
	ZR_Start,
	ZR_Restart,
	ZR_Stop
} ZoneRequestCommands;
struct LauncherZoneRequest {
	uint8 command;
	char short_name[33];
	uint16 port;
};

struct LauncherZoneStatus {
	char short_name[33];
	uint32 start_count;
	uint8 running;
};


struct ServerGuildID_Struct {
	uint32 guild_id;
};

struct ServerGuildRefresh_Struct {
	uint32 guild_id;
	uint8 name_change;
	uint8 motd_change;
	uint8 rank_change;
	uint8 relation_change;
};

struct ServerGuildCharRefresh_Struct {
	uint32 guild_id;
	uint32 old_guild_id;
	uint32 char_id;
};

struct ServerGuildRankUpdate_Struct
{
	uint32 GuildID;
	char MemberName[64];
	uint32 Rank;
	uint32 Banker;
};

struct ServerGuildMemberUpdate_Struct {
	uint32 GuildID;
	char MemberName[64];
	uint32 ZoneID;
	uint32 LastSeen;
};

struct SpawnPlayerCorpse_Struct {
	uint32 player_corpse_id;
	uint32 zone_id;
};

struct ServerOP_Consent_Struct {
	char grantname[64];
	char ownername[64];
	uint8 permission;
	uint32 zone_id;
	uint16 instance_id;
	uint32 message_string_id;
};

struct ReloadTasks_Struct {
	uint32 Command;
	uint32 Parameter;
};

struct ServerDepopAllPlayersCorpses_Struct
{
	uint32 CharacterID;
	uint32 ZoneID;
	uint16 InstanceID;
};

struct ServerDepopPlayerCorpse_Struct
{
	uint32 DBID;
	uint32 ZoneID;
	uint16 InstanceID;
};

struct ServerRaidGeneralAction_Struct {
	uint32 zoneid;
	uint16 instance_id;
	uint32 rid;
	uint32 gid;
	char playername[64];
};

struct ServerRaidGroupAction_Struct { //add / remove depends on opcode.
	char membername[64]; //member who's adding / leaving
	uint32 gid; //group id to send to.
	uint32 rid; //raid id to send to.
};

struct ServerRaidMessage_Struct {
	uint32 rid;
	uint32 gid;
	char from[64];
	char message[0];
};

struct ServerRaidMOTD_Struct {
	uint32 rid;
	char motd[0];
};

struct ServerLFGMatchesRequest_Struct {
	uint32	FromID;
	uint8	QuerierLevel;
	char	FromName[64];
	uint8	FromLevel;
	uint8	ToLevel;
	uint32	Classes;
};

struct ServerLFGMatchesResponse_Struct {
	char Name[64];
	uint8 Level;
	uint8 Class_;
	uint16 Zone;
	uint16 GuildID;
	uint16 Anon;
	char Comments[64];
};

struct ServerLFPUpdate_Struct {
	uint32 LeaderID;
	uint8 Action;
	uint8 MatchFilter;
	uint32	FromLevel;
	uint32	ToLevel;
	uint32	Classes;
	char	Comments[64];
	GroupLFPMemberEntry Members[MAX_GROUP_MEMBERS];
};

struct ServerLFPMatchesResponse_Struct {
	uint32	FromLevel;
	uint32	ToLevel;
	uint32	Classes;
	GroupLFPMemberEntry Members[MAX_GROUP_MEMBERS];
	char Comments[64];
};

struct ServerLFPMatchesRequest_Struct {
	uint32	FromID;
	uint8	FromLevel;
	uint8	ToLevel;
	uint8	QuerierClass;
	uint8	QuerierLevel;
	char	FromName[64];
};

struct UpdateSpawnTimer_Struct {
	uint32 id;
	uint32 duration;
};

struct ServerInstanceUpdateTime_Struct
{
	uint16 instance_id;
	uint32 new_duration;
};

struct ServerSpawnStatusChange_Struct
{
	uint32 id;
	bool new_status;
};

struct ServerQGlobalUpdate_Struct
{
	uint32 id;
	char name[64];
	char value[128];
	uint32 npc_id;
	uint32 char_id;
	uint32 zone_id;
	uint32 expdate;
	uint32 from_zone_id;
	uint32 from_instance_id;
};

struct ServerQGlobalDelete_Struct
{
	char name[64];
	uint32 npc_id;
	uint32 char_id;
	uint32 zone_id;
	uint32 from_zone_id;
	uint32 from_instance_id;
};

struct ServerRequestOnlineGuildMembers_Struct
{
	uint32	FromID;
	uint32	GuildID;
};

struct ServerRequestClientVersionSummary_Struct
{
	char Name[64];
};

struct ServerAdventureRequest_Struct
{
	char leader[64];
	uint32 template_id;
	uint8 type;
	uint8 risk;
	uint8 member_count;
};

struct ServerAdventureRequestDeny_Struct
{
	char leader[64];
	char reason[512];
};

struct ServerAdventureRequestAccept_Struct
{
	char leader[64];
	char text[512];
	uint32 theme;
	uint32 id;
	uint32 member_count;
};

struct ServerAdventureRequestCreate_Struct
{
	char leader[64];
	uint32 theme;
	uint32 id;
	uint32 member_count;
};

struct ServerSendAdventureData_Struct
{
	char player[64];
	char text[512];
	uint32 time_left;
	uint32 time_to_enter;
	uint32 risk;
	float x;
	float y;
	int count;
	int total;
	uint32 zone_in_id;
	uint32 zone_in_object;
	uint16 instance_id;
	uint32 finished_adventures;
};

struct ServerFinishedAdventures_Struct
{
	uint32 zone_in_id;
	uint32 zone_in_object;
};

struct ServerPlayerClickedAdventureDoor_Struct
{
	char player[64];
	int id;
	int zone_id;
};

struct ServerPlayerClickedAdventureDoorReply_Struct
{
	char player[64];
	int zone_id;
	int instance_id;
	float x;
	float y;
	float z;
	float h;
};

struct ServerAdventureCount_Struct
{
	uint16 instance_id;
};

struct ServerAdventureCountUpdate_Struct
{
	char player[64];
	int count;
	int total;
};

struct ServerZoneAdventureDataReply_Struct
{
	uint16 instance_id;
	int count;
	int total;
	int type;
	int data_id;
	int assa_count;
	int assa_x;
	int assa_y;
	int assa_z;
	int assa_h;
	int dest_x;
	int dest_y;
	int dest_z;
	int dest_h;
};

struct ServerAdventureFinish_Struct
{
	char player[64];
	bool win;
	int points;
	int theme;
};

struct ServerLeaderboardRequest_Struct
{
	char player[64];
	uint8 type;
};

struct ServerCameraShake_Struct
{
	uint32 duration; // milliseconds
	uint32 intensity; // number from 1-10
};

struct ServerMailMessageHeader_Struct {
	char from[64];
	char to[64];
	char subject[128];
	char message[0];
};

struct Server_Speech_Struct {
	char	to[64];
	char	from[64];
	uint32	guilddbid;
	int16	minstatus;
	uint32	type;
	char	message[0];
};

struct CZClientSignal_Struct {
	int charid;
	uint32 data;
};

struct CZNPCSignal_Struct {
	uint32 npctype_id;
	uint32 data;
};

struct CZClientSignalByName_Struct {
	char Name[64];
	uint32 data;
};

struct QSTradeItems_Struct {
	uint32 from_id;
	uint16 from_slot;
	uint32 to_id;
	uint16 to_slot;
	uint32 item_id;
	uint16 charges;
	uint32 aug_1;
	uint32 aug_2;
	uint32 aug_3;
	uint32 aug_4;
	uint32 aug_5;
};

struct QSPlayerLogTrade_Struct {
	uint32				char1_id;
	MoneyUpdate_Struct	char1_money;
	uint16				char1_count;
	uint32				char2_id;
	MoneyUpdate_Struct	char2_money;
	uint16				char2_count;
	uint16				_detail_count;
	QSTradeItems_Struct items[0];
};

struct QSDropItems_Struct {
	uint32 item_id;
	uint16 charges;
	uint32 aug_1;
	uint32 aug_2;
	uint32 aug_3;
	uint32 aug_4;
	uint32 aug_5;
};

struct QSPlayerDropItem_Struct {
	uint32 char_id;
	bool pickup; // 0 drop, 1 pickup
	uint32 zone_id;
	int x;
	int y;
	int z;
	uint16	_detail_count;
	QSDropItems_Struct items[0];
};

struct QSHandinItems_Struct {
	char action_type[7]; // handin, return or reward
	uint16 char_slot;
	uint32 item_id;
	uint16 charges;
	uint32 aug_1;
	uint32 aug_2;
	uint32 aug_3;
	uint32 aug_4;
	uint32 aug_5;
};

struct QSPlayerLogHandin_Struct {
	uint32				quest_id;
	uint32				char_id;
	MoneyUpdate_Struct	char_money;
	uint16				char_count;
	uint32				npc_id;
	MoneyUpdate_Struct	npc_money;
	uint16				npc_count;
	uint16				_detail_count;
	QSHandinItems_Struct items[0];
};

struct QSPlayerLogNPCKillSub_Struct{
	uint32 NPCID;
	uint32 ZoneID;
	uint32 Type;
};

struct QSPlayerLogNPCKillsPlayers_Struct{
	uint32 char_id;
};

struct QSPlayerLogNPCKill_Struct{
	QSPlayerLogNPCKillSub_Struct s1;
	QSPlayerLogNPCKillsPlayers_Struct Chars[0];
};

struct QSDeleteItems_Struct {
	uint16 char_slot;
	uint32 item_id;
	uint16 charges;
	uint32 aug_1;
	uint32 aug_2;
	uint32 aug_3;
	uint32 aug_4;
	uint32 aug_5;
};

struct QSPlayerLogDelete_Struct {
	uint32					char_id;
	uint16					stack_size; // '0' indicates full stack or non-stackable item move
	uint16					char_count;
	QSDeleteItems_Struct	items[0];
};

struct QSMoveItems_Struct {
	uint16 from_slot;
	uint16 to_slot;
	uint32 item_id;
	uint16 charges;
	uint32 aug_1;
	uint32 aug_2;
	uint32 aug_3;
	uint32 aug_4;
	uint32 aug_5;
};

struct QSPlayerLogMove_Struct {
	uint32			char_id;
	uint16			from_slot;
	uint16			to_slot;
	uint16			stack_size; // '0' indicates full stack or non-stackable item move
	uint16			char_count;
	bool			postaction;
	QSMoveItems_Struct items[0];
};

struct QSTransactionItems_Struct {
	uint16 char_slot;
	uint32 item_id;
	uint16 charges;
	uint32 aug_1;
	uint32 aug_2;
	uint32 aug_3;
	uint32 aug_4;
	uint32 aug_5;
};

struct QSMerchantLogTransaction_Struct {
	uint32					zone_id;
	uint32					merchant_id;
	MoneyUpdate_Struct		merchant_money;
	uint16					merchant_count;
	uint32					char_id;
	MoneyUpdate_Struct		char_money;
	uint16					char_count;
	QSTransactionItems_Struct items[0];
};

struct QSGeneralQuery_Struct {
	char QueryString[0];
};

struct CZMessagePlayer_Struct {
	uint32	Type;
	char	CharName[64];
	char	Message[512];
};

struct WWMarquee_Struct {
	uint32 Type;
	uint32 Priority;
	uint32 FadeIn;
	uint32 FadeOut;
	uint32 Duration;
	char Message[512];
};

struct CZSetEntVarByNPCTypeID_Struct {
	uint32 npctype_id;
	char id[256];
	char m_var[256];
};

struct CZSetEntVarByClientName_Struct {
	char CharName[64];
	char id[256];
	char m_var[256];
};

struct ReloadWorld_Struct{
	uint32 Option;
};

struct ServerRequestTellQueue_Struct {
	char	name[64];
};

struct UCSServerStatus_Struct {
	uint8 available; // non-zero=true, 0=false
	union {
		struct {
			uint16 port;
			uint16 unused;
		};
		uint32 timestamp;
	};
};

// shared task related communications
struct ServerSharedTaskMember_Struct { // used for various things we just need the ID and a name (add, remove, etc)
	uint32 id;
	char name[64];
};
// error constants
#define TASKJOINOOZ_CAN				0
#define TASKJOINOOZ_NOTASK			1
#define TASKJOINOOZ_HAVEONE			2
#define TASKJOINOOZ_LEVEL			3
#define TASKJOINOOZ_TIMER			4

#pragma pack()

/*
 * Routing
 */

struct RouteToMessage
{
	std::string filter;
	std::string identifier;
	std::string id;
	size_t payload_size;

	template <class Archive>
	void serialize(Archive &ar)
	{
		ar(filter, identifier, id, payload_size);
	}
};

/*
 * Tasks
 */

enum TaskMessageTypes
{
	TaskGetClientTaskState = 1
};

struct GetClientTaskStateRequest
{
	uint32 client_id;

	template <class Archive>
	void serialize(Archive &ar)
	{
		ar(client_id);
	}
};
