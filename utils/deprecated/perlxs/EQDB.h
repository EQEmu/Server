	//NOTE: you must have a space after the * of a return value
	
	unsigned int field_count();
	unsigned long affected_rows();
	unsigned long insert_id();
	unsigned int get_errno();
	Const_char * error();
	EQDBRes * query(Const_char *q);
	Const_char * escape_string(Const_char *from);		//NOT THREAD SAFE! (m_escapeBuffer)
