#ifndef CHATCHANNEL_H
#define CHATCHANNEL_H

//#include "clientlist.h"
#include "../common/linked_list.h"
#include "../common/timer.h"
#include <string>
#include <list>

using namespace std;

class Client;

class ChatChannel {

public:

	ChatChannel(string inName, string inOwner, string inPassword, bool inPermanent, int inMinimumStatus = 0);
	~ChatChannel();

	void AddClient(Client *c);
	bool RemoveClient(Client *c);
	bool IsClientInChannel(Client *c);

	int MemberCount(int Status);
	string GetName() { return Name; }
	void SendMessageToChannel(string Message, Client* Sender);
	bool CheckPassword(string inPassword) { return ((Password.length() == 0) || (Password == inPassword)); }
	void SetPassword(string inPassword);
	bool IsOwner(string Name) { return (Owner == Name); }
	void SetOwner(string inOwner);
	void SendChannelMembers(Client *c);
	int GetMinStatus() { return MinimumStatus; }
	bool ReadyToDelete() { return DeleteTimer.Check(); }
	void SendOPList(Client *c);
	void AddInvitee(string Invitee);
	void RemoveInvitee(string Invitee);
	bool IsInvitee(string Invitee);
	void AddModerator(string Moderator);
	void RemoveModerator(string Modeerator);
	bool IsModerator(string Moderator);
	void AddVoice(string Voiced);
	void RemoveVoice(string Voiced);
	bool HasVoice(string Voiced);
	inline bool IsModerated() { return Moderated; }
	void SetModerated(bool inModerated);

	friend class ChatChannelList;

private:

	string Name;
	string Owner;
	string Password;

	bool Permanent;
	bool Moderated;

	int MinimumStatus;

	Timer DeleteTimer;

	LinkedList<Client*> ClientsInChannel;

	list<string> Moderators;
	list<string> Invitees;
	list<string> Voiced;

};

class ChatChannelList {

public:
	ChatChannel* CreateChannel(string Name, string Owner, string Passwordi, bool Permanent, int MinimumStatus = 0);
	ChatChannel* FindChannel(string Name);
	ChatChannel* AddClientToChannel(string Channel, Client *c);
	ChatChannel* RemoveClientFromChannel(string Channel, Client *c);
	void RemoveClientFromAllChannels(Client *c);
	void RemoveChannel(ChatChannel *Channel);
	void RemoveAllChannels();
	void SendAllChannels(Client *c);
	void Process();

private:

	LinkedList<ChatChannel*> ChatChannels;

};

string CapitaliseName(string inString);

#endif
