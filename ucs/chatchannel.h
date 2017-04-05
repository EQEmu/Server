#ifndef CHATCHANNEL_H
#define CHATCHANNEL_H

//#include "clientlist.h"
#include "../common/linked_list.h"
#include "../common/timer.h"
#include <string>
#include <vector>

class Client;

class ChatChannel {

public:

	ChatChannel(std::string inName, std::string inOwner, std::string inPassword, bool inPermanent, int inMinimumStatus = 0);
	~ChatChannel();

	void AddClient(Client *c);
	bool RemoveClient(Client *c);
	bool IsClientInChannel(Client *c);

	int MemberCount(int Status);
	const std::string &GetName() { return Name; }
	void SendMessageToChannel(std::string Message, Client* Sender);
	bool CheckPassword(std::string inPassword) { return Password.empty() || Password == inPassword; }
	void SetPassword(std::string inPassword);
	bool IsOwner(std::string Name) { return (Owner == Name); }
	void SetOwner(std::string inOwner);
	void SendChannelMembers(Client *c);
	int GetMinStatus() { return MinimumStatus; }
	bool ReadyToDelete() { return DeleteTimer.Check(); }
	void SendOPList(Client *c);
	void AddInvitee(const std::string &Invitee);
	void RemoveInvitee(std::string Invitee);
	bool IsInvitee(std::string Invitee);
	void AddModerator(const std::string &Moderator);
	void RemoveModerator(const std::string &Moderator);
	bool IsModerator(std::string Moderator);
	void AddVoice(const std::string &Voiced);
	void RemoveVoice(const std::string &Voiced);
	bool HasVoice(std::string Voiced);
	inline bool IsModerated() { return Moderated; }
	void SetModerated(bool inModerated);

	friend class ChatChannelList;

private:

	std::string Name;
	std::string Owner;
	std::string Password;

	bool Permanent;
	bool Moderated;

	int MinimumStatus;

	Timer DeleteTimer;

	LinkedList<Client*> ClientsInChannel;

	std::vector<std::string> Moderators;
	std::vector<std::string> Invitees;
	std::vector<std::string> Voiced;

};

class ChatChannelList {

public:
	ChatChannel* CreateChannel(std::string Name, std::string Owner, std::string Passwordi, bool Permanent, int MinimumStatus = 0);
	ChatChannel* FindChannel(std::string Name);
	ChatChannel* AddClientToChannel(std::string Channel, Client *c);
	ChatChannel* RemoveClientFromChannel(std::string Channel, Client *c);
	void RemoveClientFromAllChannels(Client *c);
	void RemoveChannel(ChatChannel *Channel);
	void RemoveAllChannels();
	void SendAllChannels(Client *c);
	void Process();

private:

	LinkedList<ChatChannel*> ChatChannels;

};

std::string CapitaliseName(std::string inString);

#endif
