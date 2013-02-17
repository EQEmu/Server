PerlPacket *
PerlPacket::new(const char *opcode = "OP_Unknown", uint32 len = 0);
	void DESTROY();
	
	bool SetOpcode(const char *opcode);
	void Resize(uint32 len);
	
	//sending functions
	void SendTo(Client *who);
	void SendToAll();
	
	//editing
	void Zero();
	void FromArray(int *numbers, uint32 length);
	void SetByte(uint32 pos, uint8 val);
	void SetShort(uint32 pos, uint16 val);
	void SetLong(uint32 pos, uint32 val);
	void SetFloat(uint32 pos, float val);
	void SetString(uint32 pos, char *str);
	
	void SetEQ1319(uint32 pos, float part13, float part19);
	void SetEQ1913(uint32 pos, float part19, float part13);
	
	//reading
	uint8 GetByte(uint32 pos);
	uint16 GetShort(uint32 pos);
	uint32 GetLong(uint32 pos);
	float GetFloat(uint32 pos);
