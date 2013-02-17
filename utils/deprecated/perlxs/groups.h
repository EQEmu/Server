	void	DisbandGroup();
	bool	IsGroupMember(Mob* client);
	void	CastGroupSpell(Mob* caster,uint16 spellid);
	void	SplitExp(uint32 exp, Mob* other);
	void	GroupMessage(Mob* sender,const char* message);
	int32	GetTotalGroupDamage(Mob* other);
	void	SplitMoney(uint32 copper, uint32 silver, uint32 gold, uint32 platinum);
	void	SetLeader(Mob* newleader){ leader=newleader; };
	Mob*	GetLeader(){ return leader; };
	char*	GetLeaderName(){ return membername[0]; };
	void	SendHPPacketsTo(Mob* newmember);
	void	SendHPPacketsFrom(Mob* newmember);
	bool	IsLeader(Mob* leadertest) { return leadertest==leader; };
	int8	GroupCount();
	int32	GetHighestLevel();
	void	TeleportGroup(Mob* sender, int32 zoneID, float x, float y, float z, float heading);
	inline const int32 GetID()	const { return id; }
