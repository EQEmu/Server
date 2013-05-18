#ifndef EQEMU_LUA_PTR_H
#define EQEMU_LUA_PTR_H
#ifdef LUA_EQEMU

template<typename T>
class Lua_Ptr
{
public:
	Lua_Ptr() {
	}
	
	Lua_Ptr(T *d) : d_(d) {
	}
	
	~Lua_Ptr() {
	}
	
	T *GetLuaPtrData() {
		return d_;
	}
	
	void SetLuaPtrData(T *d) {
		d_ = d;
	}
	
	bool Null() {
		return d_ == nullptr;
	}
	
	bool Valid() {
		return d_ != nullptr;
	}
	
protected:
	T *d_;
};

#endif
#endif