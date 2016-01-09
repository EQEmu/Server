#ifndef EQEMU_LUA_BEHAVIOR_INTERFACE_H
#define EQEMU_LUA_BEHAVIOR_INTERFACE_H
#ifdef LUA_EQEMU

namespace luabind {
	namespace adl {
		class object;
	}
}

struct lua_State;

class LuaBehaviorInterface
{
public:
	LuaBehaviorInterface();
	~LuaBehaviorInterface();
	bool Init();
private:
	bool MapFunctions();
	lua_State *L;
};

#endif
#endif
