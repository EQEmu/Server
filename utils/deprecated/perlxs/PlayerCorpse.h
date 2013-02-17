	int32	GetCharID()			{ return charid; }
	int32	GetDecayTime()		{ if (!corpse_decay_timer.Enabled()) return 0xFFFFFFFF; else return corpse_decay_timer.GetRemainingTime(); }
	inline void		Lock()			{ pLocked = true; }
	inline void		UnLock()		{ pLocked = false; }
	inline bool		IsLocked()		{ return pLocked; }
	inline void		ResetLooter()	{ BeingLootedBy = 0xFFFFFFFF; }
	inline int32	GetDBID()		{ return dbid; }
	inline char*	GetOwnerName()	{ return orgname;}
	void	SetDecayTimer(int32 decaytime);
	bool	IsEmpty();
	void	AddItem(uint32 itemnum, int8 charges, sint16 slot = 0);
	uint32	GetWornItem(sint16 equipSlot) const;
	void	RemoveItem(int16 lootslot);
	void	SetCash(int16 in_copper, int16 in_silver, int16 in_gold, int16 in_platinum);
	void	RemoveCash();
	int32	CountItems();
	void	Delete();
	uint32	GetCopper()		{ return copper; }
	uint32	GetSilver()		{ return silver; }
	uint32	GetGold()		{ return gold; }
	uint32	GetPlatinum()	{ return platinum; }
	void	Summon(Client* client, bool spell);
	void	CastRezz(int16 spellid, Mob* Caster);
	void	CompleteRezz();
	bool CanMobLoot(int charid);
	void AllowMobLoot(Mob *them, int8 slot);
	void AddLooter(Mob *who);
	bool IsRezzed() { return isrezzed; }
