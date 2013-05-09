/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2008 EQEMu Development Team (http://eqemulator.net)

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

#ifndef CHATSERVER_CLIENTLIST_H
#define CHATSERVER_CLIENTLIST_H

#include "../common/opcodemgr.h"
#include "../common/EQStreamType.h"
#include "../common/EQStreamFactory.h"
#include "../common/rulesys.h"
#include "chatchannel.h"
#include <list>
#include <vector>

#define MAX_JOINED_CHANNELS 10

enum {
	CommandJoin = 0, CommandLeaveAll, CommandLeave, CommandListAll, CommandList, CommandSet, CommandAnnounce, CommandSetOwner,
	CommandOPList, CommandInvite, CommandGrant, CommandModerate, CommandVoice, CommandKick,
	CommandPassword, CommandToggleInvites, CommandAFK, CommandUptime,
	CommandGetHeaders, CommandGetBody, CommandMailTo, CommandSetMessageStatus, CommandSelectMailBox,
	CommandSetMailForwarding, CommandBuddy, CommandIgnorePlayer,
	CommandEndOfList
};

struct CommandEntry {
	const char *CommandString;
	int CommandCode;
};

typedef enum { ConnectionTypeUnknown, ConnectionTypeCombined, ConnectionTypeMail, ConnectionTypeChat } ConnectionType;

static const CommandEntry Commands[] = {
					{ "join", CommandJoin },
					{ "leaveall", CommandLeaveAll },
					{ "leave", CommandLeave },
					{ "listall", CommandListAll },
					{ "list", CommandList },
					{ "set", CommandSet },
					{ "announce", CommandAnnounce },
					{ "setowner", CommandSetOwner },
					{ "oplist", CommandOPList },
					{ "invite", CommandInvite },
					{ "grant", CommandGrant },
					{ "moderate", CommandModerate },
					{ "voice", CommandVoice },
					{ "kick", CommandKick },
					{ "password", CommandPassword },
					{ "toggleinvites", CommandToggleInvites },
					{ "afk", CommandAFK },
					{ "uptime", CommandUptime },
					{ "getheaders", CommandGetHeaders },
					{ "getbody", CommandGetBody },
					{ "mailto", CommandMailTo },
					{ "setmessagestatus", CommandSetMessageStatus },
					{ "selectmailbox", CommandSelectMailBox },
					{ "setmailforwarding", CommandSetMailForwarding },
					{ "buddy", CommandBuddy },
					{ "ignoreplayer", CommandIgnorePlayer },
					{ "", CommandEndOfList } };

struct CharacterEntry {
	int CharID;
	int Level;
	string Name;
};

class Client {

public:
	Client(EQStream* eqs);
	~Client();

	EQStream *ClientStream;
	void AddCharacter(int CharID, const char *CharacterName, int Level);
	void ClearCharacters() { Characters.clear(); }
	void SendMailBoxes();
	inline void QueuePacket(const EQApplicationPacket *p, bool ack_req=true) { ClientStream->QueuePacket(p, ack_req); }
	string GetName() { if(Characters.size()) return Characters[0].Name; else return ""; }
	void JoinChannels(string ChannelList);
	void LeaveChannels(string ChannelList);
	void LeaveAllChannels(bool SendUpdatedChannelList = true);
	void AddToChannelList(ChatChannel *JoinedChannel);
	void RemoveFromChannelList(ChatChannel *JoinedChannel);
	void SendChannelMessage(string Message);
	void SendChannelMessage(string ChannelName, string Message, Client *Sender);
	void SendChannelMessageByNumber(string Message);
	void SendChannelList();
	void CloseConnection();
	void ToggleAnnounce(string State);
	bool IsAnnounceOn() { return (Announce == true); }
	void AnnounceJoin(ChatChannel *Channel, Client *c);
	void AnnounceLeave(ChatChannel *Channel, Client *c);
	void GeneralChannelMessage(string Message);
	void GeneralChannelMessage(const char *Characters);
	void SetChannelPassword(string ChannelPassword);
	void ProcessChannelList(string CommandString);
	void AccountUpdate();
	int ChannelCount();
	inline void SetAccountID(int inAccountID) { AccountID = inAccountID; }
	inline int GetAccountID() { return AccountID; }
	inline void SetAccountStatus(int inStatus) { Status = inStatus; }
	inline void SetHideMe(bool inHideMe) { HideMe = inHideMe; }
	inline void SetKarma(uint32 inKarma) { TotalKarma = inKarma; }
	inline int GetAccountStatus() { return Status; }
	inline bool GetHideMe() { return HideMe; }
	inline uint32 GetKarma() { return TotalKarma; }
	void SetChannelOwner(string CommandString);
	void OPList(string CommandString);
	void ChannelInvite(string CommandString);
	void ChannelGrantModerator(string CommandString);
	void ChannelGrantVoice(string CommandString);
	void ChannelKick(string CommandString);
	void ChannelModerate(string CommandString);
	string ChannelSlotName(int ChannelNumber);
	void ToggleInvites();
	bool InvitesAllowed() { return AllowInvites; }
	bool IsRevoked() { return Revoked; }
	void SetRevoked(bool r) { Revoked = r; }
	inline bool IsChannelAdmin() { return (Status >= RuleI(Channels, RequiredStatusAdmin)); }
	inline bool CanListAllChannels() { return (Status >= RuleI(Channels, RequiredStatusListAll)); }
	void SendHelp();
	inline bool GetForceDisconnect() { return ForceDisconnect; }
	string MailBoxName();
	int GetMailBoxNumber() { return CurrentMailBox; }
	int GetMailBoxNumber(string CharacterName);
	void SetConnectionType(char c);
	ConnectionType GetConnectionType() { return TypeOfConnection; }
	inline bool IsMailConnection() { return (TypeOfConnection == ConnectionTypeMail) || (TypeOfConnection == ConnectionTypeCombined); }
	void SendNotification(int MailBoxNumber, string From, string Subject, int MessageID);
	void ChangeMailBox(int NewMailBox);
	inline void SetMailBox(int NewMailBox) { CurrentMailBox = NewMailBox; }
	void SendFriends();
	int GetCharID();
	void SendUptime();

private:
	unsigned int CurrentMailBox;
	vector<CharacterEntry> Characters;
	ChatChannel *JoinedChannels[MAX_JOINED_CHANNELS];
	bool Announce;
	int AccountID;
	int Status;
	bool HideMe;
	bool AllowInvites;
	bool Revoked;

	//Anti Spam Stuff
	Timer *AccountGrabUpdateTimer;
	uint32 TotalKarma;

	Timer *GlobalChatLimiterTimer; //60 seconds
	int AttemptedMessages;
	bool ForceDisconnect;
	ConnectionType TypeOfConnection;
	bool UnderfootOrLater;
};

class Clientlist {

public:
	Clientlist(int MailPort);
	void	Process();
	void	CloseAllConnections();
	Client *FindCharacter(string CharacterName);
	void	CheckForStaleConnections(Client *c);
	Client *IsCharacterOnline(string CharacterName);
	void ProcessOPMailCommand(Client *c, string CommandString);

private:

	EQStreamFactory *chatsf;

	list<Client*> ClientChatConnections;

	OpcodeManager *ChatOpMgr;
};

#endif
