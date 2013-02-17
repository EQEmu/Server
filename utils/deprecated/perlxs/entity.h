
	Mob*	GetMobID(int16 id);
	Mob*	GetMob(const char* name);
	Mob*	GetMobByNpcTypeID(int32 get_id);
	Client* GetClientByName(const char *name); 
	Client* GetClientByAccID(int32 accid);
	Client* GetClientByID(int16 id);
	Client* GetClientByCharID(int32 iCharID);
	Client* GetClientByWID(int32 iWID);
	Group*	GetGroupByMob(Mob* mob);
	Group*	GetGroupByClient(Client* client);
	Group*  GetGroupByID(int32 id);
	Group*  GetGroupByLeaderName(char* leader);
	Corpse*	GetCorpseByOwner(Client* client);
	Corpse* GetCorpseByID(int16 id);
	Corpse* GetCorpseByName(char* name);
	void ClearClientPetitionQueue();
    bool CanAddHateForMob(Mob *p);

	void	Clear();
	bool	RemoveMob(int16 delete_id);
	bool	RemoveClient(int16 delete_id);
	bool	RemoveNPC(int16 delete_id);
	bool	RemoveGroup(int32 delete_id);
	bool	RemoveCorpse(int16 delete_id);
	bool	RemoveDoor(int16 delete_id);
	bool	RemoveTrap(int16 delete_id);
	bool	RemoveObject(int16 delete_id);
	void	RemoveAllMobs();
	void	RemoveAllClients();
	void	RemoveAllNPCs();
	void	RemoveAllGroups();
	void	RemoveAllCorpses();
	void	RemoveAllDoors();
	void	RemoveAllTraps();
	void	RemoveAllObjects();
	
	void	Message(int32 to_guilddbid, int32 type, const char* message, ...);
	void	MessageStatus(int32 to_guilddbid, int to_minstatus, int32 type, const char* message, ...);
	void	MessageClose(Mob* sender, bool skipsender, float dist, int32 type, const char* message, ...);

	void    RemoveFromTargets(Mob* mob);
    void    ReplaceWithTarget(Mob* pOldMob, Mob*pNewTarget);
	
	void	OpenDoorsNear(NPC* opener);

	char*	MakeNameUnique(char* name);
	static char*	RemoveNumbers(char* name);
// signal quest command support
	void	SignalMobsByNPCID(int32 npc_type, int signal_id);
	void    RemoveEntity(int16 id);

	sint32	DeleteNPCCorpses();
	sint32	DeletePlayerCorpses();
	void	HalveAggro(Mob* who);
	void	DoubleAggro(Mob* who);
	void	ClearFeignAggro(Mob* targ);
	
	bool	Fighting(Mob* targ);
	void    RemoveFromHateLists(Mob* mob, bool settoone = false);
	void	MessageGroup(Mob* sender, bool skipclose, int32 type, const char* message, ...);
	
