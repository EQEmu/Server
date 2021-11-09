#ifndef EQEMU_LOGINSTRUCTURES_H
#define EQEMU_LOGINSTRUCTURES_H

#pragma pack(1)

// unencrypted base message header in all packets
struct LoginBaseMessage_Struct
{
	int32_t sequence;     // request type/login sequence (2: handshake, 3: login, 4: serverlist, ...)
	bool    compressed;   // true: deflated
	int8_t  encrypt_type; // 1: invert (unused) 2: des (2 for encrypted player logins and order expansions) (client uses what it sent, ignores in reply)
	int32_t unk3;         // unused?
};

struct LoginBaseReplyMessage_Struct
{
	bool    success;      // 0: failure (shows error string) 1: success
	int32_t error_str_id; // last error eqlsstr id, default: 101 (no error)
	char    str[1];       // variable length, unknown (may be unused, this struct is a common pattern elsewhere)
};

struct LoginHandShakeReply_Struct
{
	LoginBaseMessage_Struct base_header;
	LoginBaseReplyMessage_Struct base_reply;
	char unknown[1]; // variable length string
};

// for reference, login buffer is variable (minimum size 8 due to encryption)
struct PlayerLogin_Struct
{
	LoginBaseMessage_Struct base_header;
	char username[1];
	char password[1];
};

// variable length, can use directly if not serializing strings
struct PlayerLoginReply_Struct
{
	// base header excluded to make struct data easier to encrypt
	//LoginBaseMessage_Struct base_header;
	LoginBaseReplyMessage_Struct base_reply;

	int8_t  unk1;                       // (default: 0)
	int8_t  unk2;                       // (default: 0)
	int32_t lsid;                       // (default: -1)
	char    key[11];                    // client reads until null (variable length)
	int32_t failed_attempts;
	bool    show_player_count;          // admin flag, enables admin button and shows server player counts (default: false)
	int32_t offer_min_days;             // guess, needs more investigation, maybe expansion offers (default: 99)
	int32_t offer_min_views;            // guess (default: -1)
	int32_t offer_cooldown_minutes;     // guess (default: 0)
	int32_t web_offer_number;           // web order view number, 0 nothing (default: 0)
	int32_t web_offer_min_days;         // number of days to show offer (based on first offer time in client eqls ini) (default: 99)
	int32_t web_offer_min_views;        // mininum views, -1 for no minimum, 0 for never shows (based on client eqls ini) (default: -1)
	int32_t web_offer_cooldown_minutes; // minimum minutes between offers (based on last offer time in client eqls ini) (default: 0)
	char    username[1];                // variable length, if not empty client attempts to re-login to server select when quitting from char select and sends this in a struct
	char    unknown[1];                 // variable length, password unlikely? client doesn't send this on re-login from char select
};

// variable length, for reference
struct LoginClientServerData_Struct
{
	char    ip[1];
	int32_t server_type;      // legends, preferred, standard
	int32_t server_id;
	char    server_name[1];
	char    country_code[1];  // if doesn't match client locale then server is colored dark grey in list and joining is prevented (to block for "us" use one of "kr", "tw", "jp", "de", "fr", or "cn") (ISO 3166-1 alpha-2)
	char    language_code[1];
	int32_t server_status;    // see ServerStatusFlags
	int32_t player_count;
};

// variable length, for reference
struct ServerListReply_Struct
{
	LoginBaseMessage_Struct base_header;
	LoginBaseReplyMessage_Struct base_reply;

	int32_t server_count;
	LoginClientServerData_Struct servers[0];
};

struct PlayEverquestRequest_Struct {
	LoginBaseMessage_Struct base_header;
	uint32 server_number;
};

// SCJoinServerReply
struct PlayEverquestResponse_Struct {
	LoginBaseMessage_Struct base_header;
	LoginBaseReplyMessage_Struct base_reply;
	uint32 server_number;
};


#pragma pack()

#endif

