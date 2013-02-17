
//BEGIN CLIENT METHODS

	void	SendSound();
	bool	Save(int8 iCommitNow);
	void	SaveBackup();

	inline bool	Connected();
	inline bool	InZone();
	inline void	Kick();
	inline void	Disconnect();
	inline bool IsLD();
	void	WorldKick();
	inline int8	GetAnon();
	void	Duck();
	void	Stand();

	void	SetGM(bool toggle);
	void	SetPVP(bool toggle);
	inline bool	GetPVP();
	inline bool	GetGM();

	inline void	SetBaseClass(uint32 i);
	inline void	SetBaseRace(uint32 i);
	inline void	SetBaseGender(uint32 i);

	inline int8	GetBaseFace();

	inline int8	GetLanguageSkill(int16 n);
	inline Const_char *	GetLastName();
	inline int32	GetLDoNPoints();


	inline int8	GetBaseSTR();
	inline int8	GetBaseSTA();
	inline int8	GetBaseCHA();
	inline int8	GetBaseDEX();
	inline int8	GetBaseINT();
	inline int8	GetBaseAGI();
	inline int8	GetBaseWIS();

	int16 GetWeight();
	inline uint32	GetEXP();
	bool	UpdateLDoNPoints(sint32 points, int32 theme);
	inline  void SetDeity(uint32 i);

	void	AddEXP(uint32 add_exp, int8 conlevel = 0xFF, bool resexp = false);
	void	SetEXP(uint32 set_exp, uint32 set_aaxp, bool resexp=false);

	void	SetBindPoint(int to_zone = -1, float new_x = 0.0f, float new_y = 0.0f, float new_z = 0.0f);
	void	MovePC(int32 zoneID, float x, float y, float z, float heading);
	void	ChangeLastName(const char* in_lastname);

    FACTION_VALUE   GetFactionLevel(int32 char_id, int32 npc_id, int32 p_race, int32 p_class, int32 p_deity, sint32 pFaction, Mob* tnpc);

	void	SetFactionLevel(int32 char_id, int32 npc_id, int8 char_class, int8 char_race, int8 char_deity);
	void    SetFactionLevel2(int32 char_id, sint32 faction_id, int8 char_class, int8 char_race, int8 char_deity, sint32 value);
	sint16	GetRawItemAC();

	inline int32	AccountID();
	inline Const_char *	AccountName();
	inline sint16	Admin();
	inline int32	CharacterID();
	void	UpdateAdmin(bool iFromDB = true);
	void	UpdateWho(int8 remove = 0);

	inline int8	GuildRank();
	inline int32	GuildID();

	int8	GetFace();

	bool	TakeMoneyFromPP(uint32 copper);
	void	AddMoneyToPP(uint32 copper, uint32 silver, uint32 gold,uint32 platinum,bool updateclient);

	bool	TGB() {return tgb;}

	int16	GetSkillPoints() {return m_pp.points;}
	void	SetSkillPoints(int inp) {m_pp.points = inp;}

	void	IncreaseSkill(int skill_id, int value = 1) { if (skill_id <= HIGHEST_SKILL) { m_pp.skills[skill_id] += value; } }
	void	IncreaseLanguageSkill(int skill_id, int value = 1) { if (skill_id < 26) { m_pp.languages[skill_id] += value; } }
	virtual uint16 GetSkill(SkillType skill_id) const { if (skill_id <= HIGHEST_SKILL) { return((itembonuses.skillmod[skill_id] > 0)? m_pp.skills[skill_id]*(100 + itembonuses.skillmod[skill_id])/100 : m_pp.skills[skill_id]); } return 0; }
	uint32		GetRawSkill(SkillType skill_id) const { if (skill_id <= HIGHEST_SKILL) { return(m_pp.skills[skill_id]); } return 0; }
	bool HasSkill(SkillType skill_id) const;
	bool CanHaveSkill(SkillType skill_id) const;
	void SetSkill(SkillType skill_num, int8 value); // socket 12-29-01
	void	AddSkill(SkillType skillid, int8 value);
	void CheckSpecializeIncrease(int16 spell_id);
	bool	CheckIncreaseSkill(SkillType skillid, int chancemodi = 0);
	void    SetLanguageSkill(int langid, int value);
	int8    MaxSkill(SkillType skillid);

	void	GMKill();
	inline bool	IsMedding();
	inline int16	GetDuelTarget();
	inline bool	IsDueling();
	inline void	SetDuelTarget(int16 set_id);
	inline void	SetDueling(bool duel);

	void  ResetAA();

	void MemSpell(int16 spell_id, int slot, bool update_client = true);
	void UnmemSpell(int slot, bool update_client = true);
	void UnmemSpellAll(bool update_client = true);
	void ScribeSpell(int16 spell_id, int slot, bool update_client = true);
	void UnscribeSpell(int slot, bool update_client = true);
	void UnscribeSpellAll(bool update_client = true);

	inline bool	IsSitting();
	inline bool	IsBecomeNPC();
	inline int8	GetBecomeNPCLevel();
	inline void	SetBecomeNPC(bool flag);
	inline void	SetBecomeNPCLevel(int8 level);
	bool	LootToStack(uint32 itemid);
	void	SetFeigned(bool in_feigned);
	inline bool    GetFeigned();

	inline bool AutoSplitEnabled();

	void SetHorseId(int16 horseid_in);
	int16 GetHorseId();

	uint32	NukeItem(uint32 itemnum);
	void	SetTint(sint16 slot_id, uint32 color);
	void	SetMaterial(sint16 slot_id, uint32 item_id);
	void	Undye();
	uint32	GetItemIDAt(sint16 slot_id);
	void	DeleteItemInInventory(sint16 slot_id, sint8 quantity = 0, bool client_update = false);
	void	SummonItem(uint32 item_id, sint8 charges = 0);
	void	SetStats(int8 type,sint16 increase_val);
	void    IncStats(int8 type,sint16 increase_val);
	void	DropItem(sint16 slot_id);

	void	BreakInvis();
	Group*	GetGroup();
	void	LeaveGroup();

	bool	Hungry();
	bool	Thirsty();
	int16	GetInstrumentMod(int16 spell_id);
	bool	DecreaseByID(int32 type, int8 amt);
	int8	SlotConvert2(int8 slot);
	void	Escape();
	void    RemoveNoRent();
	void	RangedAttack(Mob* other);
	void	ThrowingAttack(Mob* other);
	void	GoFish();
	void	ForageItem();
	float	CalcPriceMod(Mob* other = 0, bool reverse = false);
	void	ResetTrade();
	bool	UseDiscipline(int32 spell_id, int32 target);
	sint32  GetCharacterFactionLevel(sint32 faction_id);


	void	SetZoneFlag(uint32 zone_id);
	void	ClearZoneFlag(uint32 zone_id);
	bool	HasZoneFlag(uint32 zone_id) const;
	void	SendZoneFlagInfo(Client *to) const;
	void	LoadZoneFlags();
	void	SetAATitle(const char *txt);

//TODO: inventory and ptimers interfaces
//I think there are two GetAA methods in client














