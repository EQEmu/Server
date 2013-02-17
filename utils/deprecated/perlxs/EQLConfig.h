	Const_char * GetName() const { return(m_name.c_str()); }
	int GetStaticCount() const { return(m_zones.size()); }
	bool IsConnected() const;	//is this launcher connected to world
	
	void DeleteLauncher();	//kill this launcher and all its zones.
	
	void RestartZone(Const_char *zone_ref);
	void StopZone(Const_char *zone_ref);
	void StartZone(Const_char *zone_ref);
	
	bool BootStaticZone(Const_char *short_name, uint16 port);
	bool ChangeStaticZone(Const_char *short_name, uint16 port);
	bool DeleteStaticZone(Const_char *short_name);
	
	bool SetDynamicCount(int count);
	int  GetDynamicCount() const;
	
	vector<string> ListZones();	//returns an array of zone refs
	map<string,string> GetZoneDetails(Const_char *zone_ref);
