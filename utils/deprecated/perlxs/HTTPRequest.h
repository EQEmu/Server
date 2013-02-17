
	
	Const_char * get(Const_char *name, Const_char *default_value = "") const;
	int getInt(Const_char *name, int default_value = 0) const;
	float getFloat(Const_char *name, float default_value = 0.0) const;
	
	//returns a database-safe string
	Const_char * getEscaped(Const_char *name, Const_char *default_value = "") const;
	
	map<string,string> get_all() const;
	
	void redirect(Const_char *URL);
	void SetResponseCode(Const_char *code);
	void header(Const_char *name, Const_char *value);
