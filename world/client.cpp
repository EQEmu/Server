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
	
	#ifdef FREEBSD
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

Client::Client(EQStreamInterface* ieqs) : autobootup_timeout(RuleI(World, ZoneAutobootTimeoutMS)), CLE_keepalive_timer(RuleI(World, ClientKeepaliveTimeoutMS)), connect(1000), eqs(ieqs) {
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
	eqs->Close();
	eqs->ReleaseFromUse();
}

void Client::SendLogServer() {
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

void Client::SendEnterWorld(std::string name) {
	char char_name[32]= { 0 };
	if (pZoning && database.GetLiveChar(GetAccountID(), char_name)) {
		if(database.GetAccountIDByChar(char_name) != GetAccountID()) {
			eqs->Close();
			return;
		}
		else
			clog(WORLD__CLIENT,"Telling client to continue session.");
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
	if (cle)
		cle->SetOnline(CLE_Status_CharSelect);
	if (ClientVersionBit & BIT_RoFAndLater) {
		SendMaxCharCreate(10);
		SendMembership();
		SendMembershipSettings();
	}
	seencharsel = true;
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
	mc->membership = 2;
	mc->races = 0x1ffff;
	mc->classes = 0x1ffff;
	mc->entrysize = 21;
	mc->entries[0] = 0xffffffff;
	mc->entries[1] = 0xffffffff;
	mc->entries[2] = 0xffffffff;
	mc->entries[3] = 0xffffffff;
	mc->entries[4] = 8;
	mc->entries[5] = 0xffffffff;
	mc->entries[6] = 1;
	mc->entries[7] = 1;
	mc->entries[8] = 1;
	mc->entries[9] = 0xffffffff;
	mc->entries[10] = 1;
	mc->entries[11] = 0xffffffff;
	mc->entries[12] = 0xffffffff;
	mc->entries[13] = 1;
	mc->entries[14] = 1;
	mc->entries[15] = 1;
	mc->entries[16] = 1;
	mc->entries[17] = 1;
	mc->entries[18] = 1;
	mc->entries[19] = 0xffffffff;
	mc->entries[20] = 0xffffffff;
	mc->exit_url_length = 0;
	QueuePacket(outapp);
	safe_delete(outapp);
}

void Client::SendMembershipSettings() {
	EQApplicationPacket *outapp = new EQApplicationPacket(OP_SendMembershipDetails, sizeof(Membership_Details_Struct));
	Membership_Details_Struct* mds = (Membership_Details_Struct*)outapp->pBuffer;
	mds->membership_setting_count = 66;
	int32 gold_settings[22] = {-1,-1,-1,-1,-1,-1,1,1,1,-1,1,-1,-1,1,1,1,1,1,1,-1,-1,0};
	uint32 entry_count = 0;
	for (int setting_id=0; setting_id < 22; setting_id++) {
		for (int setting_index=0; setting_index < 3; setting_index++) {
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
	for (int entry_id=0; entry_id < 15; entry_id++) {
		if (entry_id == 0) {
			mds->membership_races[entry_id].purchase_id = 1;
			mds->membership_races[entry_id].bitwise_entry = 0x1ffff;
			mds->membership_classes[entry_id].purchase_id = 1;
			mds->membership_classes[entry_id].bitwise_entry = 0x1ffff;
		}
		else {
			mds->membership_races[entry_id].purchase_id = cur_purchase_id;
			if (entry_id < 3)
				mds->membership_classes[entry_id].purchase_id = cur_purchase_id;
			else {
				mds->membership_classes[entry_id].purchase_id = cur_purchase_id2;
				cur_purchase_id2++;
			}
			if (entry_id == 1) {
				mds->membership_races[entry_id].bitwise_entry = 4110;
				mds->membership_classes[entry_id].bitwise_entry = 4614;
			}
			else if (entry_id == 2)	{
				mds->membership_races[entry_id].bitwise_entry = 4110;
				mds->membership_classes[entry_id].bitwise_entry = 4614;
			}
			else {
				if (entry_id == 12)
					cur_bitwise_value *= 2;
				mds->membership_races[entry_id].bitwise_entry = cur_bitwise_value;
				mds->membership_classes[entry_id].bitwise_entry = cur_bitwise_value;
			}
			cur_purchase_id++;
		}
		cur_bitwise_value *= 2;
	}
	mds->exit_url_length = 0;
	mds->exit_url_length2 = 0;
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
	if (app->size != sizeof(LoginInfo_Struct))
		return false;
	LoginInfo_Struct *li = (LoginInfo_Struct *)app->pBuffer;
	char name[19] = {0};
	char password[16] = {0};
	strn0cpy(name, (char*)li->login_info,18);
	strn0cpy(password, (char*)&(li->login_info[strlen(name)+1]), 15);
	if (strlen(password) <= 1) {
		// TODO: Find out how to tell the client wrong username/password
		clog(WORLD__CLIENT_ERR,"Login without a password");
		return false;
	}
	pZoning = (li->zoning == 1);
#ifdef IPBASED_AUTH_HACK
	struct in_addr tmpip;
	tmpip.s_addr = ip;
#endif
	uint32 id=0;
	bool minilogin = loginserverlist.MiniLogin();
	if(minilogin) {
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
		else
			clog(WORLD__CLIENT,"LS Account #%d",cle->LSID());
		const WorldConfig *Config=WorldConfig::get();
		if(Config->UpdateStats) {
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
	if(!database.CheckNameFilter(char_name))
		valid = false;
	else if(char_name[0] < 'A' && char_name[0] > 'Z')
		valid = false;
	else if (database.ReserveName(GetAccountID(), char_name))
		valid = true;
	else
		valid = false;
	outapp->pBuffer[0] = valid? 1 : 0;
	QueuePacket(outapp);
	safe_delete(outapp);
	if(!valid)
		memset(char_name, 0, sizeof(char_name));
	return true;
}

bool Client::HandleGenerateRandomNamePacket(const EQApplicationPacket *app) {
	char vowels[18] = "aeiouyaeiouaeioe";
	char cons[48] = "bcdfghjklmnpqrstvwxzybcdgklmnprstvwbcdgkpstrkd";
	char rndname[17] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
	char paircons[33] = "ngrkndstshthphsktrdrbrgrfrclcr";
	int rndnum = MakeRandomInt(0, 75), n = 1;
	bool dlc = false;
	bool vwl = false;
	bool dbl = false;
	if (rndnum > 63) {
		rndnum = ((rndnum - 61) * 2);
		rndname[0]=paircons[rndnum];
		rndname[1]=paircons[rndnum+1];
		n=2;
	}
	else if (rndnum > 16) {
		rndnum -= 17;
		rndname[0] = cons[rndnum];
	}
	else {
		rndname[0] = vowels[rndnum];
		vwl = true;
	}
	int namlen=MakeRandomInt(5, 10);
	for (int i = n; i < namlen; i++) {
		dlc = false;
		if (vwl) {
			rndnum = MakeRandomInt(0, 62);
			if (rndnum > 46) {
				if (i > (namlen - 3))
					rndnum = (MakeRandomInt(0, 7) * 2);
				else
					rndnum = ((rndnum - 47) * 2);
				rndname[i] = paircons[rndnum];
				rndname[i+1] = paircons[rndnum+1];
				dlc = true;	// flag keeps second letter from being doubled below
				i += 1;
			}
			else
				rndname[i]=cons[rndnum];
		}
		else
			rndname[i] = vowels[MakeRandomInt(0, 16)];
		vwl=!vwl;
		if (!dbl && !dlc) {
			if (!MakeRandomInt(0, i+9)) {
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
	if (GetAccountID() == 0) {
		clog(WORLD__CLIENT_ERR,"Account ID not set; unable to create character.");
		return false;
	}
	else if (app->size != sizeof(CharCreate_Struct)) {
		clog(WORLD__CLIENT_ERR,"Wrong size on OP_CharacterCreate. Got: %d, Expected: %d",app->size,sizeof(CharCreate_Struct));
		DumpPacket(app);
		return true;
	}
	CharCreate_Struct *cc = (CharCreate_Struct*)app->pBuffer;
	if(OPCharCreate(char_name, cc) == false) {
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
	if(GetAdmin() < 0) {
		clog(WORLD__CLIENT,"Account banned or suspended.");
		eqs->Close();
		return true;
	}
	if (RuleI(World, MaxClientsPerIP) >= 0)
		client_list.GetCLEIP(this->GetIP());
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
	if (tmpaccid != GetAccountID()) {
		clog(WORLD__CLIENT_ERR,"This account does not own the character named '%s'",char_name);
		eqs->Close();
		return true;
	}
	if(!pZoning && ew->return_home && !ew->tutorial) {
		CharacterSelect_Struct* cs = new CharacterSelect_Struct;
		memset(cs, 0, sizeof(CharacterSelect_Struct));
		database.GetCharSelectInfo(GetAccountID(), cs);
		bool home_enabled = false;
		for(int x = 0; x < 10; ++x) {
			if(strcasecmp(cs->name[x], char_name) == 0) {
				if(cs->gohome[x] == 1) {
					home_enabled = true;
					break;
				}
			}
		}
		safe_delete(cs);
		if(home_enabled)
			zoneID = database.MoveCharacterToBind(charid,4);
		else {
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
		for(int x = 0; x < 10; ++x) {
			if(strcasecmp(cs->name[x], char_name) == 0) {
				if(cs->tutorial[x] == 1) {
					tutorial_enabled = true;
					break;
				}
			}
		}
		safe_delete(cs);
		if(tutorial_enabled) {
			zoneID = RuleI(World, TutorialZoneID);
			database.MoveCharacterToZone(charid, database.GetZoneName(zoneID));
		}
		else {
			clog(WORLD__CLIENT_ERR,"'%s' is trying to go to tutorial but are not allowed...",char_name);
			database.SetHackerFlag(GetAccountName(), char_name, "MQTutorial: player tried to enter the tutorial without having tutorial enabled for this character.");
			eqs->Close();
			return true;
		}
	}
	if (zoneID == 0 || !database.GetZoneName(zoneID)) {
		database.MoveCharacterToZone(charid, "arena");
		clog(WORLD__CLIENT_ERR, "Zone not found in database zone_id=%i, moveing char to arena character:%s", zoneID, char_name);
	}
	if(instanceID > 0) {
		if(!database.VerifyInstanceAlive(instanceID, GetCharID())) {
			zoneID = database.MoveCharacterToBind(charid);
			instanceID = 0;
		}
		else {
			if(!database.VerifyZoneInstance(zoneID, instanceID)) {
				zoneID = database.MoveCharacterToBind(charid);
				instanceID = 0;
			}
		}
	}

	if(!pZoning) {
		database.SetGroupID(char_name, 0, charid);
		database.SetLoginFlags(charid, false, false, 1);
	}
	else {
		uint32 groupid=database.GetGroupID(char_name);
		if(groupid>0) {
			char* leader=0;
			char leaderbuf[64]={0};
			if((leader=database.GetGroupLeaderForLogin(char_name,leaderbuf)) && strlen(leader) > 1) {
				EQApplicationPacket* outapp3 = new EQApplicationPacket(OP_GroupUpdate,sizeof(GroupJoin_Struct));
				GroupJoin_Struct* gj = (GroupJoin_Struct*)outapp3->pBuffer;
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

	}
	else {
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
	sprintf(buffer,"%s,%i,%s.%s,%c%08X", Config->ChatHost.c_str(), Config->ChatPort, Config->ShortName.c_str(), this->GetCharName(), ConnectionType, MailKey);
	outapp2->size=strlen(buffer)+1;
	outapp2->pBuffer = new uchar[outapp2->size];
	memcpy(outapp2->pBuffer,buffer,outapp2->size);
	QueuePacket(outapp2);
	safe_delete(outapp2);
	outapp2 = new EQApplicationPacket(OP_SetChatServer2);
	sprintf(buffer,"%s,%i,%s.%s,%c%08X", Config->MailHost.c_str(), Config->MailPort, Config->ShortName.c_str(), this->GetCharName(), ConnectionType, MailKey);
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
	if(char_acct_id == GetAccountID()) {
		clog(WORLD__CLIENT,"Delete character: %s",app->pBuffer);
		database.DeleteCharacter((char *)app->pBuffer);
		SendCharInfo();
	}
	return true;
}

bool Client::HandleZoneChangePacket(const EQApplicationPacket *app) {
	if(ClientVersionBit & BIT_RoFAndLater)
		QueuePacket(app);
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
	if (RuleB(World, GMAccountIPList) && this->GetAdmin() >= (RuleI(World, MinGMAntiHackStatus))) {
		if(!database.CheckGMIPs(long2ip(this->GetIP()).c_str(), this->GetAccountID())) {
			clog(WORLD__CLIENT,"GM Account not permited from source address %s and accountid %i", long2ip(this->GetIP()).c_str(), this->GetAccountID());
			eqs->Close();
		}
	}
	if (GetAccountID() == 0 && opcode != OP_SendLoginInfo) {
		clog(WORLD__CLIENT_ERR,"Expecting OP_SendLoginInfo, got %s", OpcodeNames[opcode]);
		return false;
	}
	else if (opcode == OP_AckPacket)
		return true;

	switch(opcode) {
		case OP_World_Client_CRC1:
		case OP_World_Client_CRC2: {
			StartInTutorial = false;
			return true;
		}
		case OP_SendLoginInfo:
			return HandleSendLoginInfoPacket(app);
		case OP_ApproveName:
			return HandleNameApprovalPacket(app);
		case OP_RandomNameGenerator:
			return HandleGenerateRandomNamePacket(app);
		case OP_CharacterCreateRequest:
			return HandleCharacterCreateRequestPacket(app);
		case OP_CharacterCreate:
			return HandleCharacterCreatePacket(app);
		case OP_EnterWorld:
			return HandleEnterWorldPacket(app);
		case OP_DeleteCharacter:
			return HandleDeleteCharacterPacket(app);
		case OP_WorldComplete: {
			eqs->Close();
			return true;
		}
		case OP_ZoneChange:
			return HandleZoneChangePacket(app);
		case OP_LoginUnknown1:
		case OP_LoginUnknown2:
		case OP_CrashDump:
		case OP_WearChange:
		case OP_LoginComplete:
		case OP_ApproveWorld:
		case OP_WorldClientReady:
			return true;
		default: {
			clog(WORLD__CLIENT_ERR,"Received unknown EQApplicationPacket");
			_pkt(WORLD__CLIENT_ERR,app);
			return true;
		}
	}
	return true;
}

bool Client::Process() {
	bool ret = true;
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
		SendGuildList();
		SendApproveWorld();
		connect.Disable();
	}
	if (CLE_keepalive_timer.Check()) {
		if (cle)
			cle->KeepAlive();
	}
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
	if(instanceID > 0) {
		if(database.VerifyInstanceAlive(instanceID, GetCharID())) {
			if(database.VerifyZoneInstance(zoneID, instanceID))
				zs = zoneserver_list.FindByInstanceID(instanceID);
			else {
				instanceID = 0;
				zs = nullptr;
				database.MoveCharacterToBind(GetCharID());
				ZoneUnavail();
				return;
			}
		}
		else {
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
	if (zs)
		zs->IncomingClient(this);
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
	else
		Clearance(1);
}

void Client::Clearance(int8 response) {
	ZoneServer* zs = nullptr;
	if(instanceID > 0)
		zs = zoneserver_list.FindByInstanceID(instanceID);
	else
		zs = zoneserver_list.FindByZoneID(zoneID);

	if(zs == 0 || response == -1 || response == 0) {
		if (zs == 0)
			clog(WORLD__CLIENT_ERR,"Unable to find zoneserver in Client::Clearance!!");
		else
			clog(WORLD__CLIENT_ERR, "Invalid response %d in Client::Clearance", response);
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
		}
		else
			zs_addr=WorldConfig::get()->WorldAddress.c_str();
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
	ack_req = true;
	eqs->QueuePacket(app, ack_req);
}

void Client::SendGuildList() {
	EQApplicationPacket *outapp;
	outapp = new EQApplicationPacket(OP_GuildsList);
	outapp->pBuffer = guild_mgr.MakeGuildList("", outapp->size);
	if(outapp->pBuffer == nullptr) {
		clog(GUILDS__ERROR, "Unable to make guild list!");
		return;
	}
	clog(GUILDS__OUT_PACKETS, "Sending OP_GuildsList of length %d", outapp->size);
	eqs->FastQueuePacket((EQApplicationPacket **)&outapp);
}

void Client::SendApproveWorld() {
	EQApplicationPacket* outapp;
	outapp = new EQApplicationPacket(OP_ApproveWorld, sizeof(ApproveWorld_Struct));
	ApproveWorld_Struct* aw = (ApproveWorld_Struct*)outapp->pBuffer;
	uchar foo[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x37,0x87,0x13,0xbe,0xc8,0xa7,0x77,0xcb,0x27,0xed,0xe1,0xe6,0x5d,0x1c,0xaa,0xd3,0x3c,0x26,0x3b,0x6d,0x8c,0xdb,0x36,0x8d,0x91,0x72,0xf5,0xbb,0xe0,0x5c,0x50,0x6f,0x09,0x6d,0xc9,0x1e,0xe7,0x2e,0xf4,0x38,0x1b,0x5e,0xa8,0xc2,0xfe,0xb4,0x18,0x4a,0xf7,0x72,0x85,0x13,0xf5,0x63,0x6c,0x16,0x69,0xf4,0xe0,0x17,0xff,0x87,0x11,0xf3,0x2b,0xb7,0x73,0x04,0x37,0xca,0xd5,0x77,0xf8,0x03,0x20,0x0a,0x56,0x8b,0xfb,0x35,0xff,0x59,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x15,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x53,0xC3,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00};
	memcpy(aw->unknown544, foo, sizeof(foo));
	QueuePacket(outapp);
	safe_delete(outapp);
}

bool Client::OPCharCreate(char *name, CharCreate_Struct *cc) {
	PlayerProfile_Struct pp;
	ExtendedProfile_Struct ext;
	Inventory inv;
	time_t bday = time(nullptr);
	char startzone[50]={0};
	uint32 i;
	struct in_addr	in;
	int stats_sum = cc->STR + cc->STA + cc->AGI + cc->DEX + cc->WIS + cc->INT + cc->CHA;
	in.s_addr = GetIP();
	clog(WORLD__CLIENT,"Character creation request from %s LS#%d (%s:%d) : ", GetCLE()->LSName(), GetCLE()->LSID(), inet_ntoa(in), GetPort());
	clog(WORLD__CLIENT,"Name: %s", name);
	clog(WORLD__CLIENT,"Race: %d  Class: %d  Gender: %d  Deity: %d  Start zone: %d", cc->race, cc->class_, cc->gender, cc->deity, cc->start_zone);
	clog(WORLD__CLIENT,"STR  STA  AGI  DEX  WIS  INT  CHA    Total");
	clog(WORLD__CLIENT,"%3d  %3d  %3d  %3d  %3d  %3d  %3d     %3d", cc->STR, cc->STA, cc->AGI, cc->DEX, cc->WIS, cc->INT, cc->CHA, stats_sum);
	clog(WORLD__CLIENT,"Face: %d  Eye colors: %d %d", cc->face, cc->eyecolor1, cc->eyecolor2);
	clog(WORLD__CLIENT,"Hairstyle: %d  Haircolor: %d", cc->hairstyle, cc->haircolor);
	clog(WORLD__CLIENT,"Beard: %d  Beardcolor: %d", cc->beard, cc->beardcolor);
	if(!CheckCharCreateInfoSoF(cc))
	{
		clog(WORLD__CLIENT_ERR,"CheckCharCreateInfo did not validate the request (bad race/class/stats)");
		return false;
	}
	memset(&pp, 0, sizeof(PlayerProfile_Struct));
	InitExtendedProfile(&ext);
	strn0cpy(pp.name, name, 63);
#if 0
	for (i = 0; pp.name[i] && i < 63; i++) {
		if(!isalpha(pp.name[i]))
			return false;
		pp.name[i] = tolower(pp.name[i]);
	}
	pp.name[0] = toupper(pp.name[0]);
#endif
	pp.race = cc->race;
	pp.class_ = cc->class_;
	pp.gender = cc->gender;
	pp.deity = cc->deity;
	pp.STR = cc->STR;
	pp.STA = cc->STA;
	pp.AGI = cc->AGI;
	pp.DEX = cc->DEX;
	pp.WIS = cc->WIS;
	pp.INT = cc->INT;
	pp.CHA = cc->CHA;
	pp.face = cc->face;
	pp.eyecolor1 = cc->eyecolor1;
	pp.eyecolor2 = cc->eyecolor2;
	pp.hairstyle = cc->hairstyle;
	pp.haircolor = cc->haircolor;
	pp.beard = cc->beard;
	pp.beardcolor = cc->beardcolor;
	pp.drakkin_heritage = cc->drakkin_heritage;
	pp.drakkin_tattoo = cc->drakkin_tattoo;
	pp.drakkin_details = cc->drakkin_details;
	pp.birthday = bday;
	pp.lastlogin = bday;
	pp.level = 1;
	pp.points = 5;
	pp.cur_hp = 1000;
	pp.hunger_level = 6000;
	pp.thirst_level = 6000;
	SetRacialLanguages(&pp);
	SetRaceStartingSkills(&pp);
	SetClassStartingSkills(&pp);
	pp.skills[SkillSenseHeading] = 200;
	for(i = 0; i < MAX_PP_SPELLBOOK; i++)
		pp.spell_book[i] = 0xFFFFFFFF;
	for(i = 0; i < MAX_PP_MEMSPELL; i++)
		pp.mem_spells[i] = 0xFFFFFFFF;
	for(i = 0; i < BUFF_COUNT; i++)
		pp.buffs[i].spellid = 0xFFFF;
	pp.pvp = database.GetServerType() == 1 ? 1 : 0;
	if((ClientVersionBit & BIT_SoFAndLater) && (RuleI(World, SoFStartZoneID) > 0)) {
		clog(WORLD__CLIENT,"Found 'SoFStartZoneID' rule setting: %i", (RuleI(World, SoFStartZoneID)));
		pp.zone_id = (RuleI(World, SoFStartZoneID));
		if(pp.zone_id)
			database.GetSafePoints(pp.zone_id, 0, &pp.x, &pp.y, &pp.z);
		else
			clog(WORLD__CLIENT_ERR,"Error getting zone id for Zone ID %i", (RuleI(World, SoFStartZoneID)));
	}
	else {
		if(database.GetVariable("startzone", startzone, 50)) {
			clog(WORLD__CLIENT,"Found 'startzone' variable setting: %s", startzone);
			pp.zone_id = database.GetZoneID(startzone);
			if(pp.zone_id)
				database.GetSafePoints(pp.zone_id, 0, &pp.x, &pp.y, &pp.z);
			else
				clog(WORLD__CLIENT_ERR,"Error getting zone id for '%s'", startzone);
		}
		else {
			bool ValidStartZone = false;
			ValidStartZone = database.GetStartZoneSoF(&pp, cc);
			if(!ValidStartZone)
				return false;
		}
	}
	if(!pp.zone_id) {
		pp.zone_id = 1;
		pp.x = pp.y = pp.z = -1;
	}
	if(!pp.binds[0].zoneId) {
		pp.binds[0].zoneId = pp.zone_id;
		pp.binds[0].x = pp.x;
		pp.binds[0].y = pp.y;
		pp.binds[0].z = pp.z;
		pp.binds[0].heading = pp.heading;
	}
	pp.binds[4] = pp.binds[0];
	clog(WORLD__CLIENT,"Current location: %s  %0.2f, %0.2f, %0.2f, %0.2f", database.GetZoneName(pp.zone_id), pp.x, pp.y, pp.z, pp.heading);
	clog(WORLD__CLIENT,"Bind location: %s  %0.2f, %0.2f, %0.2f", database.GetZoneName(pp.binds[0].zoneId), pp.binds[0].x, pp.binds[0].y, pp.binds[0].z);
	database.SetStartingItems(&pp, &inv, pp.race, pp.class_, pp.deity, pp.zone_id, pp.name, GetAdmin());
	if (!database.StoreCharacter(GetAccountID(), &pp, &inv, &ext)) {
		clog(WORLD__CLIENT_ERR,"Character creation failed: %s", pp.name);
		return false;
	}
	else {
		clog(WORLD__CLIENT,"Character creation successful: %s", pp.name);
		return true;
	}
}

bool CheckCharCreateInfoSoF(CharCreate_Struct *cc) {
	if(!cc)
		return false;
	_log(WORLD__CLIENT, "Validating char creation info...");
	RaceClassCombos class_combo;
	bool found = false;
	int combos = character_create_race_class_combos.size();
	for(int i = 0; i < combos; ++i) {
		if(character_create_race_class_combos[i].Class == cc->class_ && character_create_race_class_combos[i].Race == cc->race && character_create_race_class_combos[i].Deity == cc->deity) {
			if(RuleB(World, EnableTutorialButton) && (RuleI(World, TutorialZoneID) == cc->start_zone ||	(character_create_race_class_combos[i].Zone == cc->start_zone))) {
				class_combo = character_create_race_class_combos[i];
				found = true;
				break;
			}
			else if(character_create_race_class_combos[i].Zone == cc->start_zone) {
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
	max_stats = allocation.DefaultPointAllocation[0] + allocation.DefaultPointAllocation[1] + allocation.DefaultPointAllocation[2] + allocation.DefaultPointAllocation[3] + allocation.DefaultPointAllocation[4] + allocation.DefaultPointAllocation[5] + allocation.DefaultPointAllocation[6];
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

void Client::SetClassStartingSkills(PlayerProfile_Struct *pp) {
	for(uint32 i = 0; i <= HIGHEST_SKILL; ++i) {
		if(pp->skills[i] == 0) {
			if(i >= SkillSpecializeAbjure && i <= SkillSpecializeEvocation)
				continue;
			if(i == SkillMakePoison || i == SkillTinkering || i == SkillResearch || i == SkillAlchemy || i == SkillBaking || i == SkillTailoring || i == SkillBlacksmithing || i == SkillFletching || i == SkillBrewing || i == SkillPottery || i == SkillJewelryMaking || i == SkillBegging)
				continue;
			pp->skills[i] = database.GetSkillCap(pp->class_, (SkillUseTypes)i, 1);
		}
	}
}

void Client::SetRaceStartingSkills(PlayerProfile_Struct *pp) {
	switch(pp->race)
	{
		case DARK_ELF: {
			pp->skills[SkillHide] = 50;
			break;
		}
		case FROGLOK: {
			pp->skills[SkillSwimming] = 125;
			break;
		}
		case GNOME: {
			pp->skills[SkillTinkering] = 50;
			break;
		}
		case HALFLING: {
			pp->skills[SkillHide] = 50;
			pp->skills[SkillSneak] = 50;
			break;
		}
		case IKSAR: {
			pp->skills[SkillForage] = 50;
			pp->skills[SkillSwimming] = 100;
			break;
		}
		case WOOD_ELF: {
			pp->skills[SkillForage] = 50;
			pp->skills[SkillHide] = 50;
			break;
		}
		case VAHSHIR: {
			pp->skills[SkillSafeFall] = 50;
			pp->skills[SkillSneak] = 50;
			break;
		}		
		default:
			break;
	}
}

void Client::SetRacialLanguages(PlayerProfile_Struct *pp) {
	switch(pp->race) {
		case BARBARIAN: {
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_BARBARIAN] = 100;
			break;
		}
		case DARK_ELF: {
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_DARK_ELVISH] = 100;
			pp->languages[LANG_DARK_SPEECH] = 100;
			pp->languages[LANG_ELDER_ELVISH] = 100;
			pp->languages[LANG_ELVISH] = 25;
			break;
		}
		case DWARF: {
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_DWARVISH] = 100;
			pp->languages[LANG_GNOMISH] = 25;
			break;
		}
		case ERUDITE: {
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_ERUDIAN] = 100;
			break;
		}
		case FROGLOK: {
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_FROGLOK] = 100;
			pp->languages[LANG_TROLL] = 25;
			break;
		}
		case GNOME: {
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_DWARVISH] = 25;
			pp->languages[LANG_GNOMISH] = 100;
			break;
		}
		case HALF_ELF: {
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_ELVISH] = 100;
			break;
		}
		case HALFLING: {
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_HALFLING] = 100;
			break;
		}
		case HIGH_ELF: {
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_DARK_ELVISH] = 25;
			pp->languages[LANG_ELDER_ELVISH] = 25;
			pp->languages[LANG_ELVISH] = 100;
			break;
		}
		case HUMAN: {
			pp->languages[LANG_COMMON_TONGUE] = 100;
			break;
		}
		case IKSAR: {
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_DARK_SPEECH] = 100;
			pp->languages[LANG_LIZARDMAN] = 100;
			break;
		}
		case OGRE: {
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_DARK_SPEECH] = 100;
			pp->languages[LANG_OGRE] = 100;
			break;
		}
		case TROLL: {
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_DARK_SPEECH] = 100;
			pp->languages[LANG_TROLL] = 100;
			break;
		}
		case WOOD_ELF: {
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_ELVISH] = 100;
			break;
		}
		case VAHSHIR: {
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_COMBINE_TONGUE] = 100;
			pp->languages[LANG_ERUDIAN] = 25;
			pp->languages[LANG_VAH_SHIR] = 100;
			break;
		}
		case DRAKKIN: {
			pp->languages[LANG_COMMON_TONGUE] = 100;
			pp->languages[LANG_ELDER_DRAGON] = 100;
			pp->languages[LANG_DRAGON] = 100;
			break;
		}
	}
}