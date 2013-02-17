	unsigned long	num_rows() { return (res) ? mysql_num_rows(res) : 0; }
	unsigned long	num_fields() { return (res) ? mysql_num_fields(res) : 0; }
	void		DESTROY() { }
	void		finish() { if (res) mysql_free_result(res); res=NULL; };
	vector<string>	fetch_row_array();
	map<string,string> fetch_row_hash();
	unsigned long *	fetch_lengths() { return (res) ? mysql_fetch_lengths(res) : 0; }
