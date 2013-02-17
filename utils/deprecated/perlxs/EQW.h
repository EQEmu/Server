	//NOTE: you must have a space after the * of a return value
	Const_char * GetConfig(Const_char *var_name);
	void LockWorld();
	void UnlockWorld();
	
	bool LSConnected();
	void LSReconnect();
	
	int  CountZones();
	vector<string> ListBootedZones();	//returns an array of zone_refs (opaque)
	map<string,string> GetZoneDetails(Const_char *zone_ref);	//returns a hash ref of details
	
	int  CountPlayers();
	vector<string> ListPlayers(Const_char *zone_name = "");	//returns an array of player refs (opaque)
	map<string,string> GetPlayerDetails(Const_char *player_ref);	//returns a hash ref of details
	
	int  CountLaunchers(bool active_only);
//	vector<string> ListActiveLaunchers();	//returns an array of launcher names
	vector<string> ListLaunchers();	//returns an array of launcher names
	EQLConfig * GetLauncher(Const_char *launcher_name);	//returns the EQLConfig object for the specified launcher.
	void CreateLauncher(Const_char *launcher_name, int dynamic_count);
//	EQLConfig * FindLauncher(Const_char *zone_ref);
	
	//Guild routines, mostly wrappers around guild_mgr
	int32	CreateGuild(const char* name, int32 leader_char_id);
	bool	DeleteGuild(int32 guild_id);
	bool	RenameGuild(int32 guild_id, const char* name);
	bool	SetGuildMOTD(int32 guild_id, const char* motd, const char *setter);
	bool	SetGuildLeader(int32 guild_id, int32 leader_char_id);
	bool	SetGuild(int32 charid, int32 guild_id, int8 rank);
	bool	SetGuildRank(int32 charid, int8 rank);
	bool	SetBankerFlag(int32 charid, bool is_banker);
	bool	SetTributeFlag(int32 charid, bool enabled);
	bool	SetPublicNote(int32 charid, const char *note);
	
	
