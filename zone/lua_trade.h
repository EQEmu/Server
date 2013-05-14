#ifndef EQEMU_LUA_TRADE_H
#define EQEMU_LUA_TRADE_H
#ifdef LUA_EQEMU

struct Lua_Trade
{
	uint32 item1_;
	uint32 item2_;
	uint32 item3_;
	uint32 item4_;
	uint32 item1_charges_;
	uint32 item2_charges_;
	uint32 item3_charges_;
	uint32 item4_charges_;
	uint32 item1_attuned_;
	uint32 item2_attuned_;
	uint32 item3_attuned_;
	uint32 item4_attuned_;
	uint32 platinum_;
	uint32 gold_;
	uint32 silver_;
	uint32 copper_;
};

#endif
#endif