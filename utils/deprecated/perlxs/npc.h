	void SignalNPC(int _signal_id);
	FACTION_VALUE CheckNPCFactionAlly(sint32 other_faction);
	void	AddItem(int32 itemid, int8 charges, int8 slot = 0);
	void	AddLootTable();
	void    RemoveItem(uint16 item_id, int16 quantity = 0, int16 slot = 0);
	void	ClearItemList();
	void	AddCash(int16 in_copper, int16 in_silver, int16 in_gold, int16 in_platinum);
	void	RemoveCash();
	int32	CountLoot();
	inline int32	GetLoottableID()	{ return loottable_id; }
	inline uint32	GetCopper()		{ return copper; }
	inline uint32	GetSilver()		{ return silver; }
	inline uint32	GetGold()		{ return gold; }
	inline uint32	GetPlatinum()	{ return platinum; }

	inline void	SetCopper(uint32 amt)		{ copper = amt; }
	inline void	SetSilver(uint32 amt)		{ silver = amt; }
	inline void	SetGold(uint32 amt)			{ gold = amt; }
	inline void	SetPlatinum(uint32 amt)		{ platinum = amt; }
	void SetGrid(int16 grid_){ grid=grid_; }
	void SetSp2(int32 sg2){ spawn_group=sg2; }
	int16 GetWaypointMax(){ return wp_m; }
	sint16 GetGrid(){ return grid; }
	int32 GetSp2(){ return spawn_group; }

	inline const sint32&	GetNPCFactionID()	{ return npc_faction_id; }
	inline sint32			GetPrimaryFaction()	{ return primary_faction; }
	sint32	GetNPCHate(Mob* in_ent)  {return hate_list.GetEntHate(in_ent);}
    bool    IsOnHatelist(Mob*p) { return hate_list.IsOnHateList(p);}

	void	SetNPCFactionID(sint32 in) { npc_faction_id = in; database.GetFactionIdsForNPC(npc_faction_id, &faction_list, &primary_faction); }
	int16	GetMaxDMG() {return max_dmg;}
	bool	IsAnimal() { return(bodytype == 21); }
	int16   GetPetSpellID() {return pet_spell_id;}
	void    SetPetSpellID(int16 amt) {pet_spell_id = amt;}
	int32	GetMaxDamage(int8 tlevel);
	void    SetTaunting(bool tog) {taunting = tog;}
	void	PickPocket(Client* thief);
	void	StartSwarmTimer(int32 duration) { swarm_timer.Start(duration); }
	void	DoClassAttacks(Mob *target);



	int		GetMaxWp() const { return max_wp; }
	void				DisplayWaypointInfo(Client *to);
	void				CalculateNewWaypoint();
//	int8				CalculateHeadingToNextWaypoint();
//	float				CalculateDistanceToNextWaypoint();
	void				AssignWaypoints(int32 grid);
	void				SetWaypointPause();
	void				UpdateWaypoint(int wp_index);
	// quest wandering commands
	void				StopWandering();
	void				ResumeWandering();
	void				PauseWandering(int pausetime);
	void				MoveTo(float mtx, float mty, float mtz, float mth, bool saveguardspot);
	
	void				NextGuardPosition();
	void				SaveGuardSpot(bool iClearGuardSpot = false);
	inline bool			IsGuarding() const { return(guard_heading != 0); }
	void				AI_SetRoambox(float iDist, float iMaxX, float iMinX, float iMaxY, float iMinY, int32 iDelay = 2500);
	const int32& GetNPCSpellsID();
	
