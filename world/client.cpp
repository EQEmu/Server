#include "../common/debug.h"
#include "../common/EQPacket.h"
#include "../common/EQStreamIntf.h"
#include "../common/misc.h"
#include "../common/rulesys.h"
#include "../common/emu_opcodes.h"
#include "../common/eq_packet_structs.h"
#include "../common/packet_dump.h"
#include "../common/EQStreamIntf.h"
#include "../common/Item.h"
#include "../common/races.h"
#include "../common/classes.h"
#include "../common/languages.h"
#include "../common/skills.h"
#include "../common/extprofile.h"
#include "../common/StringUtil.h"
#include "../common/clientversions.h"

#include "client.h"
#include "worlddb.h"
#include "WorldConfig.h"
#include "LoginServer.h"
#include "LoginServerList.h"
#include "zoneserver.h"
#include "zonelist.h"
#include "clientlist.h"
#include "wguild_mgr.h"
#include "SoFCharCreateData.h"

#include <iostream>
#include <iomanip>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <limits.h>

//FatherNitwit: uncomment to enable my IP based authentication hack
//#define IPBASED_AUTH_HACK

// Disgrace: for windows compile
#ifdef _WINDOWS
	#include <windows.h>
	#include <winsock.h>
#else
	
	#ifdef FREEBSD //Timothy Whitman - January 7, 2003
		#include <sys/types.h>
	#endif

	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
#endif

std::vector<RaceClassAllocation> character_create_allocations;
std::vector<RaceClassCombos> character_create_race_class_combos;

extern ZSList zoneserver_list;
extern LoginServerList loginserverlist;
extern ClientList client_list;
extern uint32 numclients;
extern volatile bool RunLoops;



Client::Client(EQStreamInterface* ieqs)
:	autobootup_timeout(RuleI(World, ZoneAutobootTimeoutMS)),
	CLE_keepalive_timer(RuleI(World, ClientKeepaliveTimeoutMS)),
	connect(1000),
	eqs(ieqs)
{
	// Live does not send datarate as of 3/11/2005
	//eqs->SetDataRate(7);
	ip = eqs->GetRemoteIP();
	port = ntohs(eqs->GetRemotePort());

	autobootup_timeout.Disable();
	connect.Disable();
	seencharsel = false;
	cle = 0;
	zoneID = 0;
	char_name[0] = 0;
	charid = 0;
	pwaitingforbootup = 0;
	StartInTutorial = false;
	ClientVersionBit = 0;
	numclients++;

	ClientVersionBit = 1 << (eqs->ClientVersion() - 1);
}

Client::~Client() {
	if (RunLoops && cle && zoneID == 0)
		cle->SetOnline(CLE_Status_Offline);

	numclients--;

	//let the stream factory know were done with this stream
	eqs->Close();
	eqs->ReleaseFromUse();
}

void Client::SendLogServer()
{
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_LogServer, sizeof(LogServer_Struct));
	LogServer_Struct *l=(LogServer_Struct *)outapp->pBuffer;
	const char *wsn=WorldConfig::get()->ShortName.c_str();
	memcpy(l->worldshortname,wsn,strlen(wsn));

	if(RuleB(Mail, EnableMailSystem))
		l->enablemail = 1;

	if(RuleB(Chat, EnableVoiceMacros))
		l->enablevoicemacros = 1;

	l->enable_pvp = (RuleI(World, PVPSettings));

	if(RuleB(World, IsGMPetitionWindowEnabled))
		l->enable_petition_wnd = 1;

	if(RuleI(World, FVNoDropFlag) == 1 || RuleI(World, FVNoDropFlag) == 2 && GetAdmin() > RuleI(Character, MinStatusForNoDropExemptions))
		l->enable_FV = 1;

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendEnterWorld(std::string name)
{
char char_name[32]= { 0 };
	if (pZoning && database.GetLiveChar(GetAccountID(), char_name)) {
		if(database.GetAccountIDByChar(char_name) != GetAccountID()) {
			eqs->Close();
			return;
		} else {
			clog(WORLD__CLIENT,"Telling client to continue session.");
		}
	}

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_EnterWorld, strlen(char_name)+1);
	memcpy(outapp->pBuffer,char_name,strlen(char_name)+1);
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendExpansionInfo() {
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_ExpansionInfo, sizeof(ExpansionInfo_Struct));
	ExpansionInfo_Struct *eis = (ExpansionInfo_Struct*)outapp->pBuffer;
	eis->Expansions = (RuleI(World, ExpansionSettings));
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendCharInfo() {
	if (cle) {
		cle->SetOnline(CLE_Status_CharSelect);
	}

	if (ClientVersionBit & BIT_RoFAndLater)
	{
		// Can make max char per account into a rule - New to VoA
		SendMaxCharCreate(10);
		SendMembership();
		SendMembershipSettings();
	}

	seencharsel = true;


	// Send OP_SendCharInfo
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_SendCharInfo, sizeof(CharacterSelect_Struct));
	CharacterSelect_Struct* cs = (CharacterSelect_Struct*)outapp->pBuffer;

	database.GetCharSelectInfo(GetAccountID(), cs);

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendMaxCharCreate(int max_chars) {
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_SendMaxCharacters, sizeof(MaxCharacters_Struct));
	MaxCharacters_Struct* mc = (MaxCharacters_Struct*)outapp->pBuffer;

	mc->max_chars = max_chars;

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendMembership() {
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_SendMembership, sizeof(Membership_Struct));
	Membership_Struct* mc = (Membership_Struct*)outapp->pBuffer;

	/*
		The remaining entry fields probably hold more membership restriction data that needs to be identified.
		Here is a possible list based on the EQ Website membership comparison list:
		1. Spell Ranks Allowed
		2. Prestige Items Usable
		3. In-Game Mail Service (send/recieve)
		4. Parcel Delivery (send/recieve)
		5. Loyalty Rewards Per-Week (30, 60, Max)
		6. Mercenary Tiers (Apprentice 1-2, Apprentice All Tiers, All Tiers)
		7. Neighborhood House
		8. Active Journal Quests (Tasks?) (10, 15, Max)
		9. Guild Function (join, join and create)
		10. Broker System (restricted, unlimited)
		11. Voice Chat
		12. Chat Ability
		13. Progression Server
		14. Customer Support
		15. In-Game Popup Advertising
		That is 15 possible fields, and there are 15 unknowns in the struct...Coincidence?
	*/

	mc->membership = 2;				//Hardcode to gold for now. We don't use anything else.
	mc->races = 0x1ffff;			// Available Races (4110 for silver)
	mc->classes = 0x1ffff;			// Available Classes (4614 for silver) - Was 0x101ffff
	mc->entrysize = 21;				// Number of membership setting entries below
	mc->entries[0] = 0xffffffff;	// Max AA Restriction
	mc->entries[1] = 0xffffffff;	// Max Level Restriction
	mc->entries[2] = 0xffffffff;	// Max Char Slots per Account (not used by client?)
	mc->entries[3] = 0xffffffff;	// 1 for Silver
	mc->entries[4] = 8;				// Main Inventory Size (0xffffffff on Live for Gold, but limitting to 8 until 10 is supported)
	mc->entries[5] = 0xffffffff;	// Max Platinum per level
	mc->entries[6] = 1;				// 0 for Silver
	mc->entries[7] = 1;				// 0 for Silver
	mc->entries[8] = 1;				// 1 for Silver
	mc->entries[9] = 0xffffffff;	// Unknown - Maybe Loyalty Points every 12 hours? 60 per week for Silver
	mc->entries[10] = 1;			// 1 for Silver
	mc->entries[11] = 0xffffffff;	// Shared Bank Slots
	mc->entries[12] = 0xffffffff;	// Unknown - Maybe Max Active Tasks?
	mc->entries[13] = 1;			// 1 for Silver
	mc->entries[14] = 1;			// 0 for Silver
	mc->entries[15] = 1;			// 0 for Silver
	mc->entries[16] = 1;			// 1 for Silver
	mc->entries[17] = 1;			// 0 for Silver
	mc->entries[18] = 1;			// 0 for Silver
	mc->entries[19] = 0xffffffff;	// 0 for Silver
	mc->entries[20] = 0xffffffff;	// 0 for Silver
	mc->exit_url_length = 0;
	//mc->exit_url = 0; // Used on Live: "http://www.everquest.com/free-to-play/exit-silver"

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendMembershipSettings() {
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_SendMembershipDetails, sizeof(Membership_Details_Struct));
	Membership_Details_Struct* mds = (Membership_Details_Struct*)outapp->pBuffer;

	mds->membership_setting_count = 66;
	int32 gold_settings[22] = {-1,-1,-1,-1,-1,-1,1,1,1,-1,1,-1,-1,1,1,1,1,1,1,-1,-1,0};
	uint32 entry_count = 0;
	for (int setting_id=0; setting_id < 22; setting_id++)
	{
		for (int setting_index=0; setting_index < 3; setting_index++)
		{

			mds->settings[entry_count].setting_index = setting_index;
			mds->settings[entry_count].setting_id = setting_id;
			mds->settings[entry_count].setting_value = gold_settings[setting_id];
			entry_count++;
		}
	}

	mds->race_entry_count = 15;
	mds->class_entry_count = 15;

	uint32 cur_purchase_id = 90287;
	uint32 cur_purchase_id2 = 90301;
	uint32 cur_bitwise_value = 1;
	for (int entry_id=0; entry_id < 15; entry_id++)
	{
		if (entry_id == 0)
		{
			mds->membership_races[entry_id].purchase_id = 1;
			mds->membership_races[entry_id].bitwise_entry = 0x1ffff;
			mds->membership_classes[entry_id].purchase_id = 1;
			mds->membership_classes[entry_id].bitwise_entry = 0x1ffff;
		}
		else
		{
			mds->membership_races[entry_id].purchase_id = cur_purchase_id;

			if (entry_id < 3)
			{
				mds->membership_classes[entry_id].purchase_id = cur_purchase_id;
			}
			else
			{
				mds->membership_classes[entry_id].purchase_id = cur_purchase_id2;
				cur_purchase_id2++;
			}

			if (entry_id == 1)
			{
				mds->membership_races[entry_id].bitwise_entry = 4110;
				mds->membership_classes[entry_id].bitwise_entry = 4614;
			}
			else if (entry_id == 2)
			{
				mds->membership_races[entry_id].bitwise_entry = 4110;
				mds->membership_classes[entry_id].bitwise_entry = 4614;
			}
			else
			{
				if (entry_id == 12)
				{
					// Live Skips 4096
					cur_bitwise_value *= 2;
				}
				mds->membership_races[entry_id].bitwise_entry = cur_bitwise_value;
				mds->membership_classes[entry_id].bitwise_entry = cur_bitwise_value;
			}
			cur_purchase_id++;
		}
		cur_bitwise_value *= 2;
	}
	mds->exit_url_length = 0;	// Live uses 42
	//strcpy(eq->exit_url, "http://www.everquest.com/free-to-play/exit");
	mds->exit_url_length2 = 0;	// Live uses 49
	//strcpy(eq->exit_url2, "http://www.everquest.com/free-to-play/exit-silver");

	/*
	Account Access Level Settings

	ID	-	Free	Silver	Gold	-	Possible Setting
	00	-	250		1000	-1		-	Max AA Restriction
	01	-	-1		-1		-1		-	Max Level Restriction
	02	-	2		4		-1		-	Max Char Slots per Account
	03	-	1		1		-1		-	Max Spell Rank
	04	-	4		6		-1		-	Main Inventory Size
	05	-	100		500		-1		-	Max Platinum per level
	06	-	0		0		1		-	Send Mail?
	07	-	0		0		1		-	Send Parcels?
	08	-	1		1		1		-	Voice Chat Unlimited?
	09	-	2		5		-1		-	Mercenary Tiers
	10	-	0		1		1		-	Create Guilds?
	11	-	0		0		-1		-	Shared Bank Slots
	12	-	9		14		-1		-	Max Journal Quests - 1
	13	-	0		1		1		-	Neighborhood-House Allowed?
	14	-	0		0		1		-	Prestige Enabled?
	15	-	0		0		1		-	Broker System Unlimited?
	16	-	0		1		1		-	Chat UnRestricted?
	17	-	0		0		1		-	Progression Server Access?
	18	-	0		0		1		-	Full Customer Support?
	19	-	0		0		-1		-	0 for Silver
	20	-	0		0		-1		-	0 for Silver
	21	-	0		0		0		-	Unknown 0
	*/

	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendPostEnterWorld() {
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_PostEnterWorld, 1);
	outapp->size=0;
	QueuePacket(outapp);
	safe_delete(outapp);
}

bool Client::HandleSendLoginInfoPacket(const EQApplicationPacket *app) {
	if (app->size != sizeof(LoginInfo_Struct)) {
		return false;
	}

	LoginInfo_Struct *li=(LoginInfo_Struct *)app->pBuffer;

	// Quagmire - max len for name is 18, pass 15
	char name[19] = {0};
	char password[16] = {0};
	strn0cpy(name, (char*)li->login_info,18);
	strn0cpy(password, (char*)&(li->login_info[strlen(name)+1]), 15);

	if (strlen(password) <= 1) {
		// TODO: Find out how to tell the client wrong username/password
		clog(WORLD__CLIENT_ERR,"Login without a password");
		return false;
	}

	pZoning=(li->zoning==1);

#ifdef IPBASED_AUTH_HACK
	struct in_addr tmpip;
	tmpip.s_addr = ip;
#endif
	uint32 id=0;
	bool minilogin = loginserverlist.MiniLogin();
	if(minilogin){
		struct in_addr miniip;
		miniip.s_addr = ip;
		id = database.GetMiniLoginAccount(inet_ntoa(miniip));
	}
	else if(strncasecmp(name, "LS#", 3) == 0)
		id=atoi(&name[3]);
	else
		id=atoi(name);
#ifdef IPBASED_AUTH_HACK
	if ((cle = zoneserver_list.CheckAuth(inet_ntoa(tmpip), password)))
#else
	if (loginserverlist.Connected() == false && !pZoning) {
		clog(WORLD__CLIENT_ERR,"Error: Login server login while not connected to login server.");
		return false;
	}
	if ((minilogin && (cle = client_list.CheckAuth(id,password,ip))) || (cle = client_list.CheckAuth(id, password)))
#endif
	{
		if (cle->AccountID() == 0 || (!minilogin && cle->LSID()==0)) {
			clog(WORLD__CLIENT_ERR,"ID is 0. Is this server connected to minilogin?");
			if(!minilogin)
				clog(WORLD__CLIENT_ERR,"If so you forget the minilogin variable...");
			else
				clog(WORLD__CLIENT_ERR,"Could not find a minilogin account, verify ip address logging into minilogin is the same that is in your account table.");
			return false;
		}

		cle->SetOnline();

		clog(WORLD__CLIENT,"Logged in. Mode=%s",pZoning ? "(Zoning)" : "(CharSel)");

		if(minilogin){
			WorldConfig::DisableStats();
			clog(WORLD__CLIENT,"MiniLogin Account #%d",cle->AccountID());
		}
		else {
			clog(WORLD__CLIENT,"LS Account #%d",cle->LSID());
		}

		const WorldConfig *Config=WorldConfig::get();

		if(Config->UpdateStats){
			ServerPacket* pack = new ServerPacket;
			pack->opcode = ServerOP_LSPlayerJoinWorld;
			pack->size = sizeof(ServerLSPlayerJoinWorld_Struct);
			pack->pBuffer = new uchar[pack->size];
			memset(pack->pBuffer,0,pack->size);
			ServerLSPlayerJoinWorld_Struct* join =(ServerLSPlayerJoinWorld_Struct*)pack->pBuffer;
			strcpy(join->key,GetLSKey());
			join->lsaccount_id = GetLSID();
			loginserverlist.SendPacket(pack);
			safe_delete(pack);
		}

		if (!pZoning)
			SendGuildList();
		SendLogServer();
		SendApproveWorld();
		SendEnterWorld(cle->name());
		SendPostEnterWorld();
		if (!pZoning) {
			SendExpansionInfo();
			SendCharInfo();
			database.LoginIP(cle->AccountID(), long2ip(GetIP()).c_str());
		}

	}
	else {
		// TODO: Find out how to tell the client wrong username/password
		clog(WORLD__CLIENT_ERR,"Bad/Expired session key '%s'",name);
		return false;
	}

	if (!cle)
		return true;

	cle->SetIP(GetIP());
	return true;
}

bool Client::HandleNameApprovalPacket(const EQApplicationPacket *app) {

	if (GetAccountID() == 0) {
		clog(WORLD__CLIENT_ERR,"Name approval request with no logged in account");
		return false;
	}

	snprintf(char_name, 64, "%s", (char*)app->pBuffer);
	uchar race = app->pBuffer[64];
	uchar clas = app->pBuffer[68];

	clog(WORLD__CLIENT,"Name approval request. Name=%s, race=%s, class=%s",char_name,GetRaceName(race),GetEQClassName(clas));

	EQApplicationPacket *outapp;
	outapp = new EQApplicationPacket;
	outapp->SetOpcode(OP_ApproveName);
	outapp->pBuffer = new uchar[1];
	outapp->size = 1;

	bool valid;
	if(!database.CheckNameFilter(char_name)) {
		valid = false;
	}
	else if(char_name[0] < 'A' && char_name[0] > 'Z') {
		//name must begin with an upper-case letter.
		valid = false;
	}
	else if (database.ReserveName(GetAccountID(), char_name)) {
		valid = true;
	}
	else {
		valid = false;
	}
	outapp->pBuffer[0] = valid? 1 : 0;
	QueuePacket(outapp);
	safe_delete(outapp);

	if(!valid) {
		memset(char_name, 0, sizeof(char_name));
	}

	return true;
}

bool Client::HandleGenerateRandomNamePacket(const EQApplicationPacket *app) {
	// creates up to a 10 char name
	char vowels[18]="aeiouyaeiouaeioe";
	char cons[48]="bcdfghjklmnpqrstvwxzybcdgklmnprstvwbcdgkpstrkd";
	char rndname[17]="\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
	char paircons[33]="ngrkndstshthphsktrdrbrgrfrclcr";
	int rndnum=MakeRandomInt(0, 75),n=1;
	bool dlc=false;
	bool vwl=false;
	bool dbl=false;
	if (rndnum>63)
	{	// rndnum is 0 - 75 where 64-75 is cons pair, 17-63 is cons, 0-16 is vowel
		rndnum=(rndnum-61)*2;	// name can't start with "ng" "nd" or "rk"
		rndname[0]=paircons[rndnum];
		rndname[1]=paircons[rndnum+1];
		n=2;
	}
	else if (rndnum>16)
	{
		rndnum-=17;
		rndname[0]=cons[rndnum];
	}
	else
	{
		rndname[0]=vowels[rndnum];
		vwl=true;
	}
	int namlen=MakeRandomInt(5, 10);
	for (int i=n;i<namlen;i++)
	{
		dlc=false;
		if (vwl)	//last char was a vowel
		{			// so pick a cons or cons pair
			rndnum=MakeRandomInt(0, 62);
			if (rndnum>46)
			{	// pick a cons pair
				if (i>namlen-3)	// last 2 chars in name?
				{	// name can only end in cons pair "rk" "st" "sh" "th" "ph" "sk" "nd" or "ng"
					rndnum=MakeRandomInt(0, 7)*2;
				}
				else
				{	// pick any from the set
					rndnum=(rndnum-47)*2;
				}
				rndname[i]=paircons[rndnum];
				rndname[i+1]=paircons[rndnum+1];
				dlc=true;	// flag keeps second letter from being doubled below
				i+=1;
			}
			else
			{	// select a single cons
				rndname[i]=cons[rndnum];
			}
		}
		else
		{		// select a vowel
			rndname[i]=vowels[MakeRandomInt(0, 16)];
		}
		vwl=!vwl;
		if (!dbl && !dlc)
		{	// one chance at double letters in name
			if (!MakeRandomInt(0, i+9))	// chances decrease towards end of name
			{
				rndname[i+1]=rndname[i];
				dbl=true;
				i+=1;
			}
		}
	}

	rndname[0]=toupper(rndname[0]);
	NameGeneration_Struct* ngs = (NameGeneration_Struct*)app->pBuffer;
	memset(ngs->name,0,64);
	strcpy(ngs->name,rndname);

	QueuePacket(app);
	return true;
}

bool Client::HandleCharacterCreateRequestPacket(const EQApplicationPacket *app) {
	// New OpCode in SoF
	uint32 allocs = character_create_allocations.size();
	uint32 combos = character_create_race_class_combos.size();
	uint32 len = sizeof(RaceClassAllocation) * allocs;
	len += sizeof(RaceClassCombos) * combos;
	len += sizeof(uint8);
	len += sizeof(uint32);
	len += sizeof(uint32);

	EQApplicationPacket *outapp = new EQApplicationPacket(OP_CharacterCreateRequest, len);
	unsigned char *ptr = outapp->pBuffer;
	*((uint8*)ptr) = 0;
	ptr += sizeof(uint8);

	*((uint32*)ptr) = allocs;
	ptr += sizeof(uint32);

	for(int i = 0; i < allocs; ++i) {
		RaceClassAllocation *alc = (RaceClassAllocation*)ptr;

		alc->Index = character_create_allocations[i].Index;
		for(int j = 0; j < 7; ++j) {
			alc->BaseStats[j] = character_create_allocations[i].BaseStats[j];
			alc->DefaultPointAllocation[j] = character_create_allocations[i].DefaultPointAllocation[j];
		}
		ptr += sizeof(RaceClassAllocation);
	}

	*((uint32*)ptr) = combos;
	ptr += sizeof(uint32);
	for(int i = 0; i < combos; ++i) {
		RaceClassCombos *cmb = (RaceClassCombos*)ptr;
		cmb->ExpansionRequired = character_create_race_class_combos[i].ExpansionRequired;
		cmb->Race = character_create_race_class_combos[i].Race;
		cmb->Class = character_create_race_class_combos[i].Class;
		cmb->Deity = character_create_race_class_combos[i].Deity;
		cmb->AllocationIndex = character_create_race_class_combos[i].AllocationIndex;
		cmb->Zone = character_create_race_class_combos[i].Zone;
		ptr += sizeof(RaceClassCombos);
	}

	QueuePacket(outapp);
	safe_delete(outapp);
	return true;
}

bool Client::HandleCharacterCreatePacket(const EQApplicationPacket *app) {
	if (GetAccountID() == 0)
	{
		clog(WORLD__CLIENT_ERR,"Account ID not set; unable to create character.");
		return false;
	}
	else if (app->size != sizeof(CharCreate_Struct))
	{
		clog(WORLD__CLIENT_ERR,"Wrong size on OP_CharacterCreate. Got: %d, Expected: %d",app->size,sizeof(CharCreate_Struct));
		DumpPacket(app);
		// the previous behavior was essentially returning true here
		// but that seems a bit odd to me.
		return true;
	}

	CharCreate_Struct *cc = (CharCreate_Struct*)app->pBuffer;
	if(OPCharCreate(char_name, cc) == false)
	{
		database.DeleteCharacter(char_name);
		EQApplicationPacket *outapp = new EQApplicationPacket(OP_ApproveName, 1);
		outapp->pBuffer[0] = 0;
		QueuePacket(outapp);
		safe_delete(outapp);
	}
	else
		SendCharInfo();

	return true;
}

bool Client::HandleEnterWorldPacket(const EQApplicationPacket *app) {

	if (GetAccountID() == 0) {
		clog(WORLD__CLIENT_ERR,"Enter world with no logged in account");
		eqs->Close();
		return true;
	}

	if(GetAdmin() < 0)
	{
		clog(WORLD__CLIENT,"Account banned or suspended.");
		eqs->Close();
		return true;
	}

	if (RuleI(World, MaxClientsPerIP) >= 0) {
		client_list.GetCLEIP(this->GetIP()); //Check current CLE Entry IPs against incoming connection
	}

	EnterWorld_Struct *ew=(EnterWorld_Struct *)app->pBuffer;
	strn0cpy(char_name, ew->name, 64);

	EQApplicationPacket *outapp;
	uint32 tmpaccid = 0;
	charid = database.GetCharacterInfo(char_name, &tmpaccid, &zoneID, &instanceID);
	if (charid == 0 || tmpaccid != GetAccountID()) {
		clog(WORLD__CLIENT_ERR,"Could not get CharInfo for '%s'",char_name);
		eqs->Close();
		return true;
	}

	// Make sure this account owns this character
	if (tmpaccid != GetAccountID()) {
		clog(WORLD__CLIENT_ERR,"This account does not own the character named '%s'",char_name);
		eqs->Close();
		return true;
	}

	if(!pZoning && ew->return_home)
	{
		CharacterSelect_Struct* cs = new CharacterSelect_Struct;
		memset(cs, 0, sizeof(CharacterSelect_Struct));
		database.GetCharSelectInfo(GetAccountID(), cs);
		bool home_enabled = false;

		for(int x = 0; x < 10; ++x)
		{
			if(strcasecmp(cs->name[x], char_name) == 0)
			{
				if(cs->gohome[x] == 1)
				{
					home_enabled = true;
					return true;
				}
			}
		}
		safe_delete(cs);

		if(home_enabled)
		{
			zoneID = database.MoveCharacterToBind(charid,4);
		}
		else
		{
			clog(WORLD__CLIENT_ERR,"'%s' is trying to go home before they're able...",char_name);
			database.SetHackerFlag(GetAccountName(), char_name, "MQGoHome: player tried to go home before they were able.");
			eqs->Close();
			return true;
		}
	}

	if(!pZoning && (RuleB(World, EnableTutorialButton) && (ew->tutorial || StartInTutorial))) {
		CharacterSelect_Struct* cs = new CharacterSelect_Struct;
		memset(cs, 0, sizeof(CharacterSelect_Struct));
		database.GetCharSelectInfo(GetAccountID(), cs);
		bool tutorial_enabled = false;

		for(int x = 0; x < 10; ++x)
		{
			if(strcasecmp(cs->name[x], char_name) == 0)
			{
				if(cs->tutorial[x] == 1)
				{
					tutorial_enabled = true;
					return true;
				}
			}
		}
		safe_delete(cs);

		if(tutorial_enabled)
		{
			zoneID = RuleI(World, TutorialZoneID);
			database.MoveCharacterToZone(charid, database.GetZoneName(zoneID));
		}
		else
		{
			clog(WORLD__CLIENT_ERR,"'%s' is trying to go to tutorial but are not allowed...",char_name);
			database.SetHackerFlag(GetAccountName(), char_name, "MQTutorial: player tried to enter the tutorial without having tutorial enabled for this character.");
			eqs->Close();
			return true;
		}
	}

	if (zoneID == 0 || !database.GetZoneName(zoneID)) {
		// This is to save people in an invalid zone, once it's removed from the DB
		database.MoveCharacterToZone(charid, "arena");
		clog(WORLD__CLIENT_ERR, "Zone not found in database zone_id=%i, moveing char to arena character:%s", zoneID, char_name);
	}

	if(instanceID > 0)
	{
		if(!database.VerifyInstanceAlive(instanceID, GetCharID()))
		{
			zoneID = database.MoveCharacterToBind(charid);
			instanceID = 0;
		}
		else
		{
			if(!database.VerifyZoneInstance(zoneID, instanceID))
			{
				zoneID = database.MoveCharacterToBind(charid);
				instanceID = 0;
			}
		}
	}

	if(!pZoning) {
		database.SetGroupID(char_name, 0, charid);
		database.SetLoginFlags(charid, false, false, 1);
	}
	else{
		uint32 groupid=database.GetGroupID(char_name);
		if(groupid>0){
			char* leader=0;
			char leaderbuf[64]={0};
			if((leader=database.GetGroupLeaderForLogin(char_name,leaderbuf)) && strlen(leader)>1){
				EQApplicationPacket* outapp3 = new EQApplicationPacket(OP_GroupUpdate,sizeof(GroupJoin_Struct));
				GroupJoin_Struct* gj=(GroupJoin_Struct*)outapp3->pBuffer;
				gj->action=8;
				strcpy(gj->yourname,char_name);
				strcpy(gj->membername,leader);
				QueuePacket(outapp3);
				safe_delete(outapp3);
			}
		}
	}

	outapp = new EQApplicationPacket(OP_MOTD);
	char tmp[500] = {0};
	if (database.GetVariable("MOTD", tmp, 500)) {
		outapp->size = strlen(tmp)+1;
		outapp->pBuffer = new uchar[outapp->size];
		memset(outapp->pBuffer,0,outapp->size);
		strcpy((char*)outapp->pBuffer, tmp);

	} else {
		// Null Message of the Day. :)
		outapp->size = 1;
		outapp->pBuffer = new uchar[outapp->size];
		outapp->pBuffer[0] = 0;
	}
	QueuePacket(outapp);
	safe_delete(outapp);

	int MailKey = MakeRandomInt(1, INT_MAX);

	database.SetMailKey(charid, GetIP(), MailKey);

	char ConnectionType;

	if(ClientVersionBit & BIT_UnderfootAndLater)
		ConnectionType = 'U';
	else if(ClientVersionBit & BIT_SoFAndLater)
		ConnectionType = 'S';
	else
		ConnectionType = 'C';

	EQApplicationPacket *outapp2 = new EQApplicationPacket(OP_SetChatServer);
	char buffer[112];

	const WorldConfig *Config = WorldConfig::get();

	sprintf(buffer,"%s,%i,%s.%s,%c%08X",
		Config->ChatHost.c_str(),
		Config->ChatPort,
		Config->ShortName.c_str(),
		this->GetCharName(), ConnectionType, MailKey
	);
	outapp2->size=strlen(buffer)+1;
	outapp2->pBuffer = new uchar[outapp2->size];
	memcpy(outapp2->pBuffer,buffer,outapp2->size);
	QueuePacket(outapp2);
	safe_delete(outapp2);

	outapp2 = new EQApplicationPacket(OP_SetChatServer2);

	if(ClientVersionBit & BIT_TitaniumAndEarlier)
		ConnectionType = 'M';

	sprintf(buffer,"%s,%i,%s.%s,%c%08X",
		Config->MailHost.c_str(),
		Config->MailPort,
		Config->ShortName.c_str(),
		this->GetCharName(), ConnectionType, MailKey
	);
	outapp2->size=strlen(buffer)+1;
	outapp2->pBuffer = new uchar[outapp2->size];
	memcpy(outapp2->pBuffer,buffer,outapp2->size);
	QueuePacket(outapp2);
	safe_delete(outapp2);

	EnterWorld();

	return true;
}

bool Client::HandleDeleteCharacterPacket(const EQApplicationPacket *app) {

	uint32 char_acct_id = database.GetAccountIDByChar((char*)app->pBuffer);
	if(char_acct_id == GetAccountID())
	{
		clog(WORLD__CLIENT,"Delete character: %s",app->pBuffer);
		database.DeleteCharacter((char *)app->pBuffer);
		SendCharInfo();
	}

	return true;
}

bool Client::HandleZoneChangePacket(const EQApplicationPacket *app) {
	// HoT sends this to world while zoning and wants it echoed back.
	if(ClientVersionBit & BIT_RoFAndLater)
	{
		QueuePacket(app);
	}
	return true;
}

bool Client::HandlePacket(const EQApplicationPacket *app) {

	EmuOpcode opcode = app->GetOpcode();

	clog(WORLD__CLIENT_TRACE,"Recevied EQApplicationPacket");
	_pkt(WORLD__CLIENT_TRACE,app);

	if (!eqs->CheckState(ESTABLISHED)) {
		clog(WORLD__CLIENT,"Client disconnected (net inactive on send)");
		return false;
	}

	// Voidd: Anti-GM Account hack, Checks source ip against valid GM Account IP Addresses
	if (RuleB(World, GMAccountIPList) && this->GetAdmin() >= (RuleI(World, MinGMAntiHackStatus))) {
		if(!database.CheckGMIPs(long2ip(this->GetIP()).c_str(), this->GetAccountID())) {
			clog(WORLD__CLIENT,"GM Account not permited from source address %s and accountid %i", long2ip(this->GetIP()).c_str(), this->GetAccountID());
			eqs->Close();
		}
	}

	if (GetAccountID() == 0 && opcode != OP_SendLoginInfo) {
		// Got a packet other than OP_SendLoginInfo when not logged in
		clog(WORLD__CLIENT_ERR,"Expecting OP_SendLoginInfo, got %s", OpcodeNames[opcode]);
		return false;
	}
	else if (opcode == OP_AckPacket) {
		return true;
	}

	switch(opcode)
	{
		case OP_World_Client_CRC1:
		case OP_World_Client_CRC2:
		{
			// There is no obvious entry in the CC struct to indicate that the 'Start Tutorial button
			// is selected when a character is created. I have observed that in this case, OP_EnterWorld is sent
			// before OP_World_Client_CRC1. Therefore, if we receive OP_World_Client_CRC1 before OP_EnterWorld,
			// then 'Start Tutorial' was not chosen.
			StartInTutorial = false;
			return true;
		}
		case OP_SendLoginInfo:
		{
			return HandleSendLoginInfoPacket(app);
		}
		case OP_ApproveName: //Name approval
		{
			return HandleNameApprovalPacket(app);
		}
		case OP_RandomNameGenerator:
		{
			return HandleGenerateRandomNamePacket(app);
		}
		case OP_CharacterCreateRequest:
		{
			// New OpCode in SoF
			return HandleCharacterCreateRequestPacket(app);
		}
		case OP_CharacterCreate: //Char create
		{
			return HandleCharacterCreatePacket(app);
		}
		case OP_EnterWorld: // Enter world
		{
			return HandleEnterWorldPacket(app);
		}
		case OP_DeleteCharacter:
		{
			return HandleDeleteCharacterPacket(app);
		}
		case OP_WorldComplete:
		{
			eqs->Close();
			return true;
		}
		case OP_ZoneChange:
		{
			// HoT sends this to world while zoning and wants it echoed back.
			return HandleZoneChangePacket(app);
		}
		case OP_LoginUnknown1:
		case OP_LoginUnknown2:
		case OP_CrashDump:
		case OP_WearChange:
		case OP_LoginComplete:
		case OP_ApproveWorld:
		case OP_WorldClientReady:
		{
			// Essentially we are just 'eating' these packets, indicating
			// they are handled.
			return true;
		}
		default:
		{
			clog(WORLD__CLIENT_ERR,"Received unknown EQApplicationPacket");
			_pkt(WORLD__CLIENT_ERR,app);
			return true;
		}
	}
	return true;
}

bool Client::Process() {
	bool ret = true;
	//bool sendguilds = true;
	sockaddr_in to;

	memset((char *) &to, 0, sizeof(to));
	to.sin_family = AF_INET;
	to.sin_port = port;
	to.sin_addr.s_addr = ip;

	if (autobootup_timeout.Check()) {
		clog(WORLD__CLIENT_ERR, "Zone bootup timer expired, bootup failed or too slow.");
		ZoneUnavail();
	}
	if(connect.Check()){
		SendGuildList();// Send OPCode: OP_GuildsList
		SendApproveWorld();
		connect.Disable();
	}
	if (CLE_keepalive_timer.Check()) {
		if (cle)
			cle->KeepAlive();
	}

	/************ Get all packets from packet manager out queue and process them ************/
	EQApplicationPacket *app = 0;
	while(ret && (app = (EQApplicationPacket *)eqs->PopPacket())) {
		ret = HandlePacket(app);

		delete app;
	}

	if (!eqs->CheckState(ESTABLISHED)) {
		if(WorldConfig::get()->UpdateStats){
			ServerPacket* pack = new ServerPacket;
			pack->opcode = ServerOP_LSPlayerLeftWorld;
			pack->size = sizeof(ServerLSPlayerLeftWorld_Struct);
			pack->pBuffer = new uchar[pack->size];
			memset(pack->pBuffer,0,pack->size);
			ServerLSPlayerLeftWorld_Struct* logout =(ServerLSPlayerLeftWorld_Struct*)pack->pBuffer;
			strcpy(logout->key,GetLSKey());
			logout->lsaccount_id = GetLSID();
			loginserverlist.SendPacket(pack);
			safe_delete(pack);
		}
		clog(WORLD__CLIENT,"Client disconnected (not active in process)");
		return false;
	}

	return ret;
}

void Client::EnterWorld(bool TryBootup) {
	if (zoneID == 0)
		return;

	ZoneServer* zs = nullptr;
	if(instanceID > 0)
	{
		if(database.VerifyInstanceAlive(instanceID, GetCharID()))
		{
			if(database.VerifyZoneInstance(zoneID, instanceID))
			{
				zs = zoneserver_list.FindByInstanceID(instanceID);
			}
			else
			{
				instanceID = 0;
				zs = nullptr;
				database.MoveCharacterToBind(GetCharID());
				ZoneUnavail();
				return;
			}
		}
		else
		{
			instanceID = 0;
			zs = nullptr;
			database.MoveCharacterToBind(GetCharID());
			ZoneUnavail();
			return;
		}
	}
	else
		zs = zoneserver_list.FindByZoneID(zoneID);


	const char *zone_name=database.GetZoneName(zoneID, true);
	if (zs) {
		// warn the world we're comming, so it knows not to shutdown
		zs->IncommingClient(this);
	}
	else {
		if (TryBootup) {
			clog(WORLD__CLIENT,"Attempting autobootup of %s (%d:%d)",zone_name,zoneID,instanceID);
			autobootup_timeout.Start();
			pwaitingforbootup = zoneserver_list.TriggerBootup(zoneID, instanceID);
			if (pwaitingforbootup == 0) {
				clog(WORLD__CLIENT_ERR,"No zoneserver available to boot up.");
				ZoneUnavail();
			}
			return;
		}
		else {
			clog(WORLD__CLIENT_ERR,"Requested zone %s is no running.",zone_name);
			ZoneUnavail();
			return;
		}
	}
	pwaitingforbootup = 0;

	cle->SetChar(charid, char_name);
	database.UpdateLiveChar(char_name, GetAccountID());
	clog(WORLD__CLIENT,"%s %s (%d:%d)",seencharsel ? "Entering zone" : "Zoning to",zone_name,zoneID,instanceID);
//	database.SetAuthentication(account_id, char_name, zone_name, ip);

	if (seencharsel) {
		if (GetAdmin() < 80 && zoneserver_list.IsZoneLocked(zoneID)) {
			clog(WORLD__CLIENT_ERR,"Enter world failed. Zone is locked.");
			ZoneUnavail();
			return;
		}

		ServerPacket* pack = new ServerPacket;
		pack->opcode = ServerOP_AcceptWorldEntrance;
		pack->size = sizeof(WorldToZone_Struct);
		pack->pBuffer = new uchar[pack->size];
		memset(pack->pBuffer, 0, pack->size);
		WorldToZone_Struct* wtz = (WorldToZone_Struct*) pack->pBuffer;
		wtz->account_id = GetAccountID();
		wtz->response = 0;
		zs->SendPacket(pack);
		delete pack;
	}
	else {	// if they havent seen character select screen, we can assume this is a zone
			// to zone movement, which should be preauthorized before they leave the previous zone
		Clearance(1);
	}
}

void Client::Clearance(int8 response)
{
	ZoneServer* zs = nullptr;
	if(instanceID > 0)
	{
		zs = zoneserver_list.FindByInstanceID(instanceID);
	}
	else
	{
		zs = zoneserver_list.FindByZoneID(zoneID);
	}

	if(zs == 0 || response == -1 || response == 0)
	{
		if (zs == 0)
		{
			clog(WORLD__CLIENT_ERR,"Unable to find zoneserver in Client::Clearance!!");
		} else {
			clog(WORLD__CLIENT_ERR, "Invalid response %d in Client::Clearance", response);
		}

		ZoneUnavail();
		return;
	}

	EQApplicationPacket* outapp;

	if (zs->GetCAddress() == nullptr) {
		clog(WORLD__CLIENT_ERR, "Unable to do zs->GetCAddress() in Client::Clearance!!");
		ZoneUnavail();
		return;
	}

	if (zoneID == 0) {
		clog(WORLD__CLIENT_ERR, "zoneID is nullptr in Client::Clearance!!");
		ZoneUnavail();
		return;
	}

	const char* zonename = database.GetZoneName(zoneID);
	if (zonename == 0) {
		clog(WORLD__CLIENT_ERR, "zonename is nullptr in Client::Clearance!!");
		ZoneUnavail();
		return;
	}

	// @bp This is the chat server
	/*
	char packetData[] = "64.37.148.34.9876,MyServer,Testchar,23cd2c95";
	outapp = new EQApplicationPacket(OP_0x0282, sizeof(packetData));
	strcpy((char*)outapp->pBuffer, packetData);
	QueuePacket(outapp);
	delete outapp;
	*/

	// Send zone server IP data
	outapp = new EQApplicationPacket(OP_ZoneServerInfo, sizeof(ZoneServerInfo_Struct));
	ZoneServerInfo_Struct* zsi = (ZoneServerInfo_Struct*)outapp->pBuffer;
	const char *zs_addr=zs->GetCAddress();
	if (!zs_addr[0]) {
		if (cle->IsLocalClient()) {
			struct in_addr in;
			in.s_addr = zs->GetIP();
			zs_addr=inet_ntoa(in);
			if (!strcmp(zs_addr,"127.0.0.1"))
				zs_addr=WorldConfig::get()->LocalAddress.c_str();
		} else {
			zs_addr=WorldConfig::get()->WorldAddress.c_str();
		}
	}
	strcpy(zsi->ip, zs_addr);
	zsi->port =zs->GetCPort();
	clog(WORLD__CLIENT,"Sending client to zone %s (%d:%d) at %s:%d",zonename,zoneID,instanceID,zsi->ip,zsi->port);
	QueuePacket(outapp);
	safe_delete(outapp);

	if (cle)
		cle->SetOnline(CLE_Status_Zoning);
}

void Client::ZoneUnavail() {
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_ZoneUnavail, sizeof(ZoneUnavail_Struct));
	ZoneUnavail_Struct* ua = (ZoneUnavail_Struct*)outapp->pBuffer;
	const char* zonename = database.GetZoneName(zoneID);
	if (zonename)
		strcpy(ua->zonename, zonename);
	QueuePacket(outapp);
	delete outapp;

	zoneID = 0;
	pwaitingforbootup = 0;
	autobootup_timeout.Disable();
}

bool Client::GenPassKey(char* key) {
	char* passKey=nullptr;
	*passKey += ((char)('A'+((int)MakeRandomInt(0, 25))));
	*passKey += ((char)('A'+((int)MakeRandomInt(0, 25))));
	memcpy(key, passKey, strlen(passKey));
	return true;
}

void Client::QueuePacket(const EQApplicationPacket* app, bool ack_req) {
	clog(WORLD__CLIENT_TRACE, "Sending EQApplicationPacket OpCode 0x%04x",app->GetOpcode());
	_pkt(WORLD__CLIENT_TRACE, app);

	ack_req = true;	// It's broke right now, dont delete this line till fix it. =P
	eqs->QueuePacket(app, ack_req);
}

void Client::SendGuildList() {
	EQApplicationPacket *outapp;
	outapp = new EQApplicationPacket(OP_GuildsList);

	//ask the guild manager to build us a nice guild list packet
	outapp->pBuffer = guild_mgr.MakeGuildList("", outapp->size);
	if(outapp->pBuffer == nullptr) {
		clog(GUILDS__ERROR, "Unable to make guild list!");
		return;
	}

	clog(GUILDS__OUT_PACKETS, "Sending OP_GuildsList of length %d", outapp->size);
//	_pkt(GUILDS__OUT_PACKET_TRACE, outapp);

	eqs->FastQueuePacket((EQApplicationPacket **)&outapp);
}

// @merth: I have no idea what this struct is for, so it's hardcoded for now
void Client::SendApproveWorld()
{
	EQApplicationPacket* outapp;

	// Send OPCode: OP_ApproveWorld, size: 544
	outapp = new EQApplicationPacket(OP_ApproveWorld, sizeof(ApproveWorld_Struct));
	ApproveWorld_Struct* aw = (ApproveWorld_Struct*)outapp->pBuffer;
	uchar foo[] = {
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x95,0x5E,0x30,0xA5,0xCA,0xD4,0xEA,0xF5,
//0xCB,0x14,0xFC,0xF7,0x78,0xE2,0x73,0x15,0x90,0x17,0xCE,0x7A,0xEB,0xEC,0x3C,0x34,
//0x5C,0x6D,0x10,0x05,0xFC,0xEA,0xED,0x19,0xC5,0x0D,0x7A,0x82,0x17,0xCC,0xCC,0x71,
//0x56,0x38,0xDF,0x78,0x8D,0xE6,0x44,0xD3,0x6F,0xDB,0xE3,0xCF,0x21,0x30,0x75,0x2F,
//0xCD,0xDC,0xE9,0xB4,0xA4,0x4E,0x58,0xDE,0xEE,0x54,0xDD,0x87,0xDA,0xE9,0xC6,0xC8,
//0x02,0xDD,0xC4,0xFD,0x94,0x36,0x32,0xAD,0x1B,0x39,0x0F,0x00,0x00,0x00,0x00,0x00,

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x37,0x87,0x13,0xbe,0xc8,0xa7,0x77,0xcb,
0x27,0xed,0xe1,0xe6,0x5d,0x1c,0xaa,0xd3,0x3c,0x26,0x3b,0x6d,0x8c,0xdb,0x36,0x8d,
0x91,0x72,0xf5,0xbb,0xe0,0x5c,0x50,0x6f,0x09,0x6d,0xc9,0x1e,0xe7,0x2e,0xf4,0x38,
0x1b,0x5e,0xa8,0xc2,0xfe,0xb4,0x18,0x4a,0xf7,0x72,0x85,0x13,0xf5,0x63,0x6c,0x16,
0x69,0xf4,0xe0,0x17,0xff,0x87,0x11,0xf3,0x2b,0xb7,0x73,0x04,0x37,0xca,0xd5,0x77,
0xf8,0x03,0x20,0x0a,0x56,0x8b,0xfb,0x35,0xff,0x59,0x00,0x00,0x00,0x00,0x00,0x00,

//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1f,0x42,0x69,0x2a,0x87,0xdd,0x04,0x3d,
//0x7f,0xb1,0xb3,0xbb,0xde,0xd5,0x5f,0xfc,0x1f,0xb3,0x25,0x94,0x16,0xd5,0xf3,0x97,
//0x43,0xdf,0xb9,0x69,0x68,0xdf,0x2b,0x64,0x98,0xf5,0x44,0xbe,0x38,0x65,0xef,0xff,
//0x36,0x89,0x90,0xcf,0x26,0xbb,0x9f,0x76,0xd5,0xaf,0x6d,0xf2,0x08,0xbe,0xce,0xd8,
//0x3e,0x4b,0x53,0x8a,0xf3,0x44,0x7c,0x19,0x49,0x5d,0x97,0x99,0xd8,0x8b,0xee,0x10,
//0x1a,0x7d,0xb7,0x8b,0x49,0x9b,0x40,0x8c,0xea,0x49,0x09,0x00,0x00,0x00,0x00,0x00,
//
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x15,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x53,0xC3,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00
};
	memcpy(aw->unknown544, foo, sizeof(foo));
	QueuePacket(outapp);
	safe_delete(outapp);
}

bool Client::OPCharCreate(char *name, CharCreate_Struct *cc)
{
	PlayerProfile_Struct pp;
	ExtendedProfile_Struct ext;
	Inventory inv;
	time_t bday = time(nullptr);
	char startzone[50]={0};
	uint32 i;
	struct in_addr	in;


	int stats_sum = cc->STR + cc->STA + cc->AGI + cc->DEX +
		cc->WIS + cc->INT + cc->CHA;

	in.s_addr = GetIP();
	clog(WORLD__CLIENT,"Character creation request from %s LS#%d (%s:%d) : ", GetCLE()->LSName(), GetCLE()->LSID(), inet_ntoa(in), GetPort());
	clog(WORLD__CLIENT,"Name: %s", name);
	clog(WORLD__CLIENT,"Race: %d  Class: %d  Gender: %d  Deity: %d  Start zone: %d",
		cc->race, cc->class_, cc->gender, cc->deity, cc->start_zone);
	clog(WORLD__CLIENT,"STR  STA  AGI  DEX  WIS  INT  CHA    Total");
	clog(WORLD__CLIENT,"%3d  %3d  %3d  %3d  %3d  %3d  %3d     %3d",
		cc->STR, cc->STA, cc->AGI, cc->DEX, cc->WIS, cc->INT, cc->CHA,
		stats_sum);
	clog(WORLD__CLIENT,"Face: %d  Eye colors: %d %d", cc->face, cc->eyecolor1, cc->eyecolor2);
	clog(WORLD__CLIENT,"Hairstyle: %d  Haircolor: %d", cc->hairstyle, cc->haircolor);
	clog(WORLD__CLIENT,"Beard: %d  Beardcolor: %d", cc->beard, cc->beardcolor);

	// validate the char creation struct
	if(ClientVersionBit & BIT_SoFAndLater) {
		if(!CheckCharCreateInfoSoF(cc))
		{
			clog(WORLD__CLIENT_ERR,"CheckCharCreateInfo did not validate the request (bad race/class/stats)");
			return false;
		}
	} else {
		if(!CheckCharCreateInfoTitanium(cc))
		{
			clog(WORLD__CLIENT_ERR,"CheckCharCreateInfo did not validate the request (bad race/class/stats)");
			return false;
		}
	}

	// Convert incoming cc_s to the new PlayerProfile_Struct
	memset(&pp, 0, sizeof(PlayerProfile_Struct));	// start building the profile

	InitExtendedProfile(&ext);

	strn0cpy(pp.name, name, 63);
	// clean the capitalization of the name
#if 0	// on second thought, don't - this will just make the creation fail
// because the name won't match what was already reserved earlier
	for (i = 0; pp.name[i] && i < 63; i++)
	{
		if(!isalpha(pp.name[i]))
			return false;
		pp.name[i] = tolower(pp.name[i]);
	}
	pp.name[0] = toupper(pp.name[0]);
#endif

	pp.race				= cc->race;
	pp.class_			= cc->class_;
	pp.gender			= cc->gender;
	pp.deity			= cc->deity;
	pp.STR				= cc->STR;
	pp.STA				= cc->STA;
	pp.AGI				= cc->AGI;
	pp.DEX				= cc->DEX;
	pp.WIS				= cc->WIS;
	pp.INT				= cc->INT;
	pp.CHA				= cc->CHA;
	pp.face				= cc->face;
	pp.eyecolor1		= cc->eyecolor1;
	pp.eyecolor2		= cc->eyecolor2;
	pp.hairstyle		= cc->hairstyle;
	pp.haircolor		= cc->haircolor;
	pp.beard			= cc->beard;
	pp.beardcolor		= cc->beardcolor;
	pp.drakkin_heritage		= cc->drakkin_heritage;
	pp.drakkin_tattoo		= cc->drakkin_tattoo;
	pp.drakkin_details		= cc->drakkin_details;
	pp.birthday		= bday;
	pp.lastlogin	= bday;
	pp.level			= 1;
	pp.points			= 5;
	pp.cur_hp			= 1000; // 1k hp during dev only
	//what was the point of this? zone dosent handle this:
	//pp.expAA			= 0xFFFFFFFF;

	pp.hunger_level = 6000;
	pp.thirst_level = 6000;


	// FIXME: FV roleplay, database goodness...

	// Racial Languages
	SetRacialLanguages( &pp ); // bUsh
	SetRaceStartingSkills( &pp ); // bUsh
	SetClassStartingSkills( &pp ); // bUsh
	pp.skills[SENSE_HEADING] = 200;
	// Some one fucking fix this to use a field name. -Doodman
	//pp.unknown3596[28] = 15; // @bp: This is to enable disc usage
//	strcpy(pp.servername, WorldConfig::get()->ShortName.c_str());


	for(i = 0; i < MAX_PP_SPELLBOOK; i++)
		pp.spell_book[i] = 0xFFFFFFFF;

	for(i = 0; i < MAX_PP_MEMSPELL; i++)
		pp.mem_spells[i] = 0xFFFFFFFF;

	for(i = 0; i < BUFF_COUNT; i++)
		pp.buffs[i].spellid = 0xFFFF;


	//was memset(pp.unknown3704, 0xffffffff, 8);
	//but I dont think thats what you really wanted to do...
	//memset is byte based

	//If server is PVP by default, make all character set to it.
	pp.pvp = database.GetServerType() == 1 ? 1 : 0;

	//If it is an SoF Client and the SoF Start Zone rule is set, send new chars there
	if((ClientVersionBit & BIT_SoFAndLater) && (RuleI(World, SoFStartZoneID) > 0)) {
		clog(WORLD__CLIENT,"Found 'SoFStartZoneID' rule setting: %i", (RuleI(World, SoFStartZoneID)));
		pp.zone_id = (RuleI(World, SoFStartZoneID));
		if(pp.zone_id)
			database.GetSafePoints(pp.zone_id, 0, &pp.x, &pp.y, &pp.z);
		else
			clog(WORLD__CLIENT_ERR,"Error getting zone id for Zone ID %i", (RuleI(World, SoFStartZoneID)));
	}
	else
	{
		// if there's a startzone variable put them in there
		if(database.GetVariable("startzone", startzone, 50))
		{
			clog(WORLD__CLIENT,"Found 'startzone' variable setting: %s", startzone);
			pp.zone_id = database.GetZoneID(startzone);
			if(pp.zone_id)
				database.GetSafePoints(pp.zone_id, 0, &pp.x, &pp.y, &pp.z);
			else
				clog(WORLD__CLIENT_ERR,"Error getting zone id for '%s'", startzone);
		}
		else	// otherwise use normal starting zone logic
		{
			bool ValidStartZone = false;

			if(ClientVersionBit & BIT_TitaniumAndEarlier)
				ValidStartZone = database.GetStartZone(&pp, cc);
			else
				ValidStartZone = database.GetStartZoneSoF(&pp, cc);

			if(!ValidStartZone)
				return false;
		}
	}

	if(!pp.zone_id)
	{
		pp.zone_id = 1;		// qeynos
		pp.x = pp.y = pp.z = -1;
	}

	if(!pp.binds[0].zoneId)
	{
		pp.binds[0].zoneId = pp.zone_id;
		pp.binds[0].x = pp.x;
		pp.binds[0].y = pp.y;
		pp.binds[0].z = pp.z;
		pp.binds[0].heading = pp.heading;
	}

	// set starting city location to the initial bind point
	pp.binds[4] = pp.binds[0];


	clog(WORLD__CLIENT,"Current location: %s  %0.2f, %0.2f, %0.2f, %0.2f",
		database.GetZoneName(pp.zone_id), pp.x, pp.y, pp.z, pp.heading);
	clog(WORLD__CLIENT,"Bind location: %s  %0.2f, %0.2f, %0.2f",
		database.GetZoneName(pp.binds[0].zoneId), pp.binds[0].x, pp.binds[0].y, pp.binds[0].z);


	// Starting Items inventory
	database.SetStartingItems(&pp, &inv, pp.race, pp.class_, pp.deity, pp.zone_id, pp.name, GetAdmin());


	// now we give the pp and the inv we made to StoreCharacter
	// to see if we can store it
	if (!database.StoreCharacter(GetAccountID(), &pp, &inv, &ext))
	{
		clog(WORLD__CLIENT_ERR,"Character creation failed: %s", pp.name);
		return false;
	}
	else
	{
		clog(WORLD__CLIENT,"Character creation successful: %s", pp.name);
		return true;
	}
}

// returns true if the request is ok, false if there's an error
bool CheckCharCreateInfoSoF(CharCreate_Struct *cc)
{
	if(!cc) return false;

	_log(WORLD__CLIENT, "Validating char creation info...");

	RaceClassCombos class_combo;
	bool found = false;
	int combos = character_create_race_class_combos.size();
	for(int i = 0; i < combos; ++i) {
		if(character_create_race_class_combos[i].Class == cc->class_ &&
			character_create_race_class_combos[i].Race == cc->race &&
			character_create_race_class_combos[i].Deity == cc->deity) {
				if(RuleB(World, EnableTutorialButton) &&
					(RuleI(World, TutorialZoneID) == cc->start_zone ||
					(character_create_race_class_combos[i].Zone == cc->start_zone))) {
					class_combo = character_create_race_class_combos[i];
					found = true;
					break;
				} else if(character_create_race_class_combos[i].Zone == cc->start_zone) {
					class_combo = character_create_race_class_combos[i];
					found = true;
					break;
				}
		}
	}

	if(!found) {
		_log(WORLD__CLIENT_ERR, "Could not find class/race/deity/start_zone combination");
		return false;
	}

	uint32 max_stats = 0;
	uint32 allocs = character_create_allocations.size();
	RaceClassAllocation allocation = {0};
	found = false;
	for(int i = 0; i < combos; ++i) {
		if(character_create_allocations[i].Index == class_combo.AllocationIndex) {
			allocation = character_create_allocations[i];
			found = true;
			break;
		}
	}

	if(!found) {
		_log(WORLD__CLIENT_ERR, "Could not find starting stats for selected character combo, cannot verify stats");
		return false;
	}

	max_stats = allocation.DefaultPointAllocation[0] +
		allocation.DefaultPointAllocation[1] +
		allocation.DefaultPointAllocation[2] +
		allocation.DefaultPointAllocation[3] +
		allocation.DefaultPointAllocation[4] +
		allocation.DefaultPointAllocation[5] +
		allocation.DefaultPointAllocation[6];

	if(cc->STR > allocation.BaseStats[0] + max_stats || cc->STR < allocation.BaseStats[0]) {
		_log(WORLD__CLIENT_ERR, "Strength out of range");
		return false;
	}

	if(cc->DEX > allocation.BaseStats[1] + max_stats || cc->DEX < allocation.BaseStats[1]) {
		_log(WORLD__CLIENT_ERR, "Dexterity out of range");
		return false;
	}

	if(cc->AGI > allocation.BaseStats[2] + max_stats || cc->AGI < allocation.BaseStats[2]) {
		_log(WORLD__CLIENT_ERR, "Agility out of range");
		return false;
	}

	if(cc->STA > allocation.BaseStats[3] + max_stats || cc->STA < allocation.BaseStats[3]) {
		_log(WORLD__CLIENT_ERR, "Stamina out of range");
		return false;
	}

	if(cc->INT > allocation.BaseStats[4] + max_stats || cc->INT < allocation.BaseStats[4]) {
		_log(WORLD__CLIENT_ERR, "Intelligence out of range");
		return false;
	}

	if(cc->WIS > allocation.BaseStats[5] + max_stats || cc->WIS < allocation.BaseStats[5]) {
		_log(WORLD__CLIENT_ERR, "Wisdom out of range");
		return false;
	}

	if(cc->CHA > allocation.BaseStats[6] + max_stats || cc->CHA < allocation.BaseStats[6]) {
		_log(WORLD__CLIENT_ERR, "Charisma out of range");
		return false;
	}

	uint32 current_stats = 0;
	current_stats += cc->STR - allocation.BaseStats[0];
	current_stats += cc->DEX - allocation.BaseStats[1];
	current_stats += cc->AGI - allocation.BaseStats[2];
	current_stats += cc->STA - allocation.BaseStats[3];
	current_stats += cc->INT - allocation.BaseStats[4];
	current_stats += cc->WIS - allocation.BaseStats[5];
	current_stats += cc->CHA - allocation.BaseStats[6];
	if(current_stats > max_stats) {
		_log(WORLD__CLIENT_ERR, "Current Stats > Maximum Stats");
		return false;
	}

	return true;
}

bool CheckCharCreateInfoTitanium(CharCreate_Struct *cc)
{
	uint32 bSTR, bSTA, bAGI, bDEX, bWIS, bINT, bCHA, bTOTAL, cTOTAL, stat_points; //these are all uint32 in CharCreate_Struct, so we'll make them uint32 here to make the compiler shut up
	int classtemp, racetemp;
	int Charerrors = 0;


// solar: if this is increased you'll have to add a column to the classrace
// table below
#define _TABLE_RACES 16

	static const int BaseRace[_TABLE_RACES][7] =
	{            /* STR  STA  AGI  DEX  WIS  INT  CHR */
	{ /*Human*/      75,  75,  75,  75,  75,  75,  75},
	{ /*Barbarian*/ 103,  95,  82,  70,  70,  60,  55},
	{ /*Erudite*/    60,  70,  70,  70,  83, 107,  70},
	{ /*Wood Elf*/   65,  65,  95,  80,  80,  75,  75},
	{ /*High Elf*/   55,  65,  85,  70,  95,  92,  80},
	{ /*Dark Elf*/   60,  65,  90,  75,  83,  99,  60},
	{ /*Half Elf*/   70,  70,  90,  85,  60,  75,  75},
	{ /*Dwarf*/      90,  90,  70,  90,  83,  60,  45},
	{ /*Troll*/     108, 109,  83,  75,  60,  52,  40},
	{ /*Ogre*/      130, 122,  70,  70,  67,  60,  37},
	{ /*Halfling*/   70,  75,  95,  90,  80,  67,  50},
	{ /*Gnome*/      60,  70,  85,  85,  67,  98,  60},
	{ /*Iksar*/      70,  70,  90,  85,  80,  75,  55},
	{ /*Vah Shir*/   90,  75,  90,  70,  70,  65,  65},
	{ /*Froglok*/    70,  80, 100, 100,  75,  75,  50},
	{ /*Drakkin*/    70,  80,  85,  75,  80,  85,  75}
	};

	static const int BaseClass[PLAYER_CLASS_COUNT][8] =
	{              /* STR  STA  AGI  DEX  WIS  INT  CHR  ADD*/
	{ /*Warrior*/      10,  10,   5,   0,   0,   0,   0,  25},
	{ /*Cleric*/        5,   5,   0,   0,  10,   0,   0,  30},
	{ /*Paladin*/      10,   5,   0,   0,   5,   0,  10,  20},
	{ /*Ranger*/        5,  10,  10,   0,   5,   0,   0,  20},
	{ /*ShadowKnight*/ 10,   5,   0,   0,   0,   10,  5,  20},
	{ /*Druid*/         0,  10,   0,   0,  10,   0,   0,  30},
	{ /*Monk*/          5,   5,  10,  10,   0,   0,   0,  20},
	{ /*Bard*/          5,   0,   0,  10,   0,   0,  10,  25},
	{ /*Rouge*/         0,   0,  10,  10,   0,   0,   0,  30},
	{ /*Shaman*/        0,   5,   0,   0,  10,   0,   5,  30},
	{ /*Necromancer*/   0,   0,   0,  10,   0,  10,   0,  30},
	{ /*Wizard*/        0,  10,   0,   0,   0,  10,   0,  30},
	{ /*Magician*/      0,  10,   0,   0,   0,  10,   0,  30},
	{ /*Enchanter*/     0,   0,   0,   0,   0,  10,  10,  30},
	{ /*Beastlord*/     0,  10,   5,   0,  10,   0,   5,  20},
	{ /*Berserker*/    10,   5,   0,  10,   0,   0,   0,  25}
	};

	static const bool ClassRaceLookupTable[PLAYER_CLASS_COUNT][_TABLE_RACES]=
	{                   /*Human  Barbarian Erudite Woodelf Highelf Darkelf Halfelf Dwarf  Troll  Ogre   Halfling Gnome  Iksar  Vahshir Froglok Drakkin*/
	{ /*Warrior*/         true,  true,     false,  true,   false,  true,   true,   true,  true,  true,  true,    true,  true,  true,   true,   true},
	{ /*Cleric*/          true,  false,    true,   false,  true,   true,   true,   true,  false, false, true,    true,  false, false,  true,   true},
	{ /*Paladin*/         true,  false,    true,   false,  true,   false,  true,   true,  false, false, true,    true,  false, false,  true,   true},
	{ /*Ranger*/          true,  false,    false,  true,   false,  false,  true,   false, false, false, true,    false, false, false,  false,  true},
	{ /*ShadowKnight*/    true,  false,    true,   false,  false,  true,   false,  false, true,  true,  false,   true,  true,  false,  true,   true},
	{ /*Druid*/           true,  false,    false,  true,   false,  false,  true,   false, false, false, true,    false, false, false,  false,  true},
	{ /*Monk*/            true,  false,    false,  false,  false,  false,  false,  false, false, false, false,   false, true,  false,  false,  true},
	{ /*Bard*/            true,  false,    false,  true,   false,  false,  true,   false, false, false, false,   false, false, true,   false,  true},
	{ /*Rogue*/           true,  true,     false,  true,   false,  true,   true,   true,  false, false, true,    true,  false, true,   true,   true},
	{ /*Shaman*/          false, true,     false,  false,  false,  false,  false,  false, true,  true,  false,   false, true,  true,   true,   false},
	{ /*Necromancer*/     true,  false,    true,   false,  false,  true,   false,  false, false, false, false,   true,  true,  false,  true,   true},
	{ /*Wizard*/          true,  false,    true,   false,  true,   true,   false,  false, false, false, false,   true,  false, false,  true,   true},
	{ /*Magician*/        true,  false,    true,   false,  true,   true,   false,  false, false, false, false,   true,  false, false,  false,  true},
	{ /*Enchanter*/       true,  false,    true,   false,  true,   true,   false,  false, false, false, false,   true,  false, false,  false,  true},
	{ /*Beastlord*/       false, true,     false,  false,  false,  false,  false,  false, true,  true,  false,   false, true,  true,   false,  false},
	{ /*Berserker*/       false, true,     false,  false,  false,  false,  false,  true,  true,  true,  false,   false, false, true,   false,  false}
	};//Initial table by kathgar, editted by Wiz for accuracy, solar too

	if(!cc) return false;

	_log(WORLD__CLIENT,"Validating char creation info...");

	classtemp = cc->class_ - 1;
	racetemp = cc->race - 1;
	// these have non sequential race numbers so they need to be mapped
	if (cc->race == FROGLOK) racetemp = 14;
	if (cc->race == VAHSHIR) racetemp = 13;
	if (cc->race == IKSAR) racetemp = 12;
	if (cc->race == DRAKKIN) racetemp = 15;

	// if out of range looking it up in the table would crash stuff
	// so we return from these
	if(classtemp >= PLAYER_CLASS_COUNT)
	{
		_log(WORLD__CLIENT_ERR,"  class is out of range");
		return false;
	}
	if(racetemp >= _TABLE_RACES)
	{
		_log(WORLD__CLIENT_ERR,"  race is out of range");
		return false;
	}

	if(!ClassRaceLookupTable[classtemp][racetemp]) //Lookup table better than a bunch of ifs?
	{
		_log(WORLD__CLIENT_ERR,"  invalid race/class combination");
		// we return from this one, since if it's an invalid combination our table
		// doesn't have meaningful values for the stats
		return false;
	}

	// solar: add up the base values for this class/race
	// this is what they start with, and they have stat_points more
	// that can distributed
	bSTR = BaseClass[classtemp][0] + BaseRace[racetemp][0];
	bSTA = BaseClass[classtemp][1] + BaseRace[racetemp][1];
	bAGI = BaseClass[classtemp][2] + BaseRace[racetemp][2];
	bDEX = BaseClass[classtemp][3] + BaseRace[racetemp][3];
	bWIS = BaseClass[classtemp][4] + BaseRace[racetemp][4];
	bINT = BaseClass[classtemp][5] + BaseRace[racetemp][5];
	bCHA = BaseClass[classtemp][6] + BaseRace[racetemp][6];
	stat_points = BaseClass[classtemp][7];
	bTOTAL = bSTR + bSTA + bAGI + bDEX + bWIS + bINT + bCHA;
	cTOTAL = cc->STR + cc->STA + cc->AGI + cc->DEX + cc->WIS + cc->INT + cc->CHA;

	// solar: the first check makes sure the total is exactly what was expected.
	// this will catch all the stat cheating, but there's still the issue
	// of reducing CHA or INT or something, to use for STR, so we check
	// that none are lower than the base or higher than base + stat_points
	// NOTE: these could just be else if, but i want to see all the stats
	// that are messed up not just the first hit

	if(bTOTAL + stat_points != cTOTAL)
	{
		_log(WORLD__CLIENT_ERR,"  stat points total doesn't match expected value: expecting %d got %d", bTOTAL + stat_points, cTOTAL);
		Charerrors++;
	}

	if(cc->STR > bSTR + stat_points || cc->STR < bSTR)
	{
		_log(WORLD__CLIENT_ERR,"  stat STR is out of range");
		Charerrors++;
	}
	if(cc->STA > bSTA + stat_points || cc->STA < bSTA)
	{
		_log(WORLD__CLIENT_ERR,"  stat STA is out of range");
		Charerrors++;
	}
	if(cc->AGI > bAGI + stat_points || cc->AGI < bAGI)
	{
		_log(WORLD__CLIENT_ERR,"  stat AGI is out of range");
		Charerrors++;
	}
	if(cc->DEX > bDEX + stat_points || cc->DEX < bDEX)
	{
		_log(WORLD__CLIENT_ERR,"  stat DEX is out of range");
		Charerrors++;
	}
	if(cc->WIS > bWIS + stat_points || cc->WIS < bWIS)
	{
		_log(WORLD__CLIENT_ERR,"  stat WIS is out of range");
		Charerrors++;
	}
	if(cc->INT > bINT + stat_points || cc->INT < bINT)
	{
		_log(WORLD__CLIENT_ERR,"  stat INT is out of range");
		Charerrors++;
	}
	if(cc->CHA > bCHA + stat_points || cc->CHA < bCHA)
	{
		_log(WORLD__CLIENT_ERR,"  stat CHA is out of range");
		Charerrors++;
	}

	/*TODO: Check for deity/class/race.. it'd be nice, but probably of any real use to hack(faction, deity based items are all I can think of)
	I am NOT writing those tables - kathgar*/

	_log(WORLD__CLIENT,"Found %d errors in character creation request", Charerrors);

	return Charerrors == 0;
}

void Client::SetClassStartingSkills( PlayerProfile_Struct *pp )
{
	for(uint32 i = 0; i <= HIGHEST_SKILL; ++i) {
		if(pp->skills[i] == 0) {
			if(i >= SPECIALIZE_ABJURE && i <= SPECIALIZE_EVOCATION) {
				continue;
			}

			if(i == MAKE_POISON ||
				i == TINKERING ||
				i == RESEARCH ||
				i == ALCHEMY ||
				i == BAKING ||
				i == TAILORING ||
				i == BLACKSMITHING ||
				i == FLETCHING ||
				i == BREWING ||
				i == POTTERY ||
				i == JEWELRY_MAKING ||
				i == BEGGING) {
				continue;
			}

			pp->skills[i] = database.GetSkillCap(pp->class_, (SkillType)i, 1);
		}
	}
}

void Client::SetRaceStartingSkills( PlayerProfile_Struct *pp )
{
	switch( pp->race )
	{
	case BARBARIAN:
	case DWARF:
	case ERUDITE:
	case HALF_ELF:
	case HIGH_ELF:
	case HUMAN:
	case OGRE:
	case TROLL:
	case DRAKKIN:	//Drakkin are supposed to get a starting AA Skill
		{
			// No Race Specific Skills
			break;
		}
	case DARK_ELF:
		{
			pp->skills[HIDE] = 50;
			break;
		}
	case FROGLOK:
		{
			pp->skills[SWIMMING] = 125;
			break;
		}
	case GNOME:
		{
			pp->skills[TINKERING] = 50;
			break;
		}
	case HALFLING:
		{
			pp->skills[HIDE] = 50;
			pp->skills[SNEAK] = 50;
			break;
		}
	case IKSAR:
		{
			pp->skills[FORAGE] = 50;
			pp->skills[SWIMMING] = 100;
			break;
		}
	case WOOD_ELF:
		{
			pp->skills[FORAGE] = 50;
			pp->skills[HIDE] = 50;
			break;
		}
	case VAHSHIR:
		{
			pp->skills[SAFE_FALL] = 50;
			pp->skills[SNEAK] = 50;
			break;
		}
	}
}

void Client::SetRacialLanguages( PlayerProfile_Struct *pp )
{
	switch( pp->race )
	{
	case BARBARIAN:
		{
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_BARBARIAN] = 100;
			break;
		}
	case DARK_ELF:
		{
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_DARK_ELVISH] = 100;
			pp->languages[LANG_DARK_SPEECH] = 100;
			pp->languages[LANG_ELDER_ELVISH] = 100;
			pp->languages[LANG_ELVISH] = 25;
			break;
		}
	case DWARF:
		{
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_DWARVISH] = 100;
			pp->languages[LANG_GNOMISH] = 25;
			break;
		}
	case ERUDITE:
		{
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_ERUDIAN] = 100;
			break;
		}
	case FROGLOK:
		{
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_FROGLOK] = 100;
			pp->languages[LANG_TROLL] = 25;
			break;
		}
	case GNOME:
		{
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_DWARVISH] = 25;
			pp->languages[LANG_GNOMISH] = 100;
			break;
		}
	case HALF_ELF:
		{
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_ELVISH] = 100;
			break;
		}
	case HALFLING:
		{
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_HALFLING] = 100;
			break;
		}
	case HIGH_ELF:
		{
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_DARK_ELVISH] = 25;
			pp->languages[LANG_ELDER_ELVISH] = 25;
			pp->languages[LANG_ELVISH] = 100;
			break;
		}
	case HUMAN:
		{
			pp->languages[LANG_COMMON_TONGUE] = 100;
			break;
		}
	case IKSAR:
		{
			pp->languages[LANG_COMMON_TONGUE] = 95;
			pp->languages[LANG_DARK_SPEECH] = 100;
			pp->languages[LANG_LIZARDMAN] = 100;
			break;
		}
	case OGRE:
		{
			pp->languages[LANG_COMMON_TONGUE] = 95;
			pp->languages[LANG_DARK_SPEECH] = 100;
			pp->languages[LANG_OGRE] = 100;
			break;
		}
	case TROLL:
		{
			pp->languages[LANG_COMMON_TONGUE] = 95;
			pp->languages[LANG_DARK_SPEECH] = 100;
			pp->languages[LANG_TROLL] = 100;
			break;
		}
	case WOOD_ELF:
		{
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_ELVISH] = 100;
			break;
		}
	case VAHSHIR:
		{
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_COMBINE_TONGUE] = 100;
			pp->languages[LANG_ERUDIAN] = 25;
			pp->languages[LANG_VAH_SHIR] = 100;
			break;
		}
	case DRAKKIN:
		{
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_ELDER_DRAGON] = 100;
			pp->languages[LANG_DRAGON] = 100;
			break;
		}
	}
}

