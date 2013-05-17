#ifndef EQEMU_LUA_PTR_H
#define EQEMU_LUA_PTR_H
#ifdef LUA_EQEMU

class Lua_Ptr
{
public:
	Lua_Ptr() {
	}
	
	Lua_Ptr(void *d) : d_(d) {
	}
	
	~Lua_Ptr() {
	}
	
	void *GetLuaPtrData() {
		return d_;
	}
	
	void SetLuaPtrData(void *d) {
		d_ = d;
	}
	
	bool Null() {
		return d_ == nullptr;
	}
	
	bool Valid() {
		return d_ != nullptr;
	}
	
protected:
	void *d_;
};

#endif
#endif