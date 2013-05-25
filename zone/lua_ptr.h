#ifndef EQEMU_LUA_PTR_H
#define EQEMU_LUA_PTR_H
#ifdef LUA_EQEMU

//TODO: Remove the error checking by a flag since this adds significant overhead to each c call
#define Lua_Safe_Call_Void() if(!d_) { return; } NativeType *self = reinterpret_cast<NativeType*>(d_)
#define Lua_Safe_Call_Bool() if(!d_) { return false; } NativeType *self = reinterpret_cast<NativeType*>(d_)
#define Lua_Safe_Call_Int() if(!d_) { return 0; } NativeType *self = reinterpret_cast<NativeType*>(d_)
#define Lua_Safe_Call_Real() if(!d_) { return 0.0; } NativeType *self = reinterpret_cast<NativeType*>(d_)
#define Lua_Safe_Call_String() if(!d_) { return ""; } NativeType *self = reinterpret_cast<NativeType*>(d_)
#define Lua_Safe_Call_Entity() if(!d_) { return Lua_Entity(); } NativeType *self = reinterpret_cast<NativeType*>(d_)
#define Lua_Safe_Call_Mob() if(!d_) { return Lua_Mob(); } NativeType *self = reinterpret_cast<NativeType*>(d_)
#define Lua_Safe_Call_NPC() if(!d_) { return Lua_NPC(); } NativeType *self = reinterpret_cast<NativeType*>(d_)
#define Lua_Safe_Call_Client() if(!d_) { return Lua_Client(); } NativeType *self = reinterpret_cast<NativeType*>(d_)
#define Lua_Safe_Call_HateList() if(!d_) { return Lua_HateList(); } NativeType *self = reinterpret_cast<NativeType*>(d_)
#define Lua_Safe_Call_Item() if(!d_) { return Lua_Item(); } NativeType *self = reinterpret_cast<NativeType*>(d_)
#define Lua_Safe_Call_ItemInst() if(!d_) { return Lua_ItemInst(); } NativeType *self = reinterpret_cast<NativeType*>(d_)

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