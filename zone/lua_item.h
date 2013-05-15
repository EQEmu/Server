#ifndef EQEMU_LUA_ITEM_H
#define EQEMU_LUA_ITEM_H
#ifdef LUA_EQEMU

class ItemInst;

class Lua_Item
{
public:
	Lua_Item() { d_ = nullptr; }
	Lua_Item(ItemInst *d) { d_ = d; }
	virtual ~Lua_Item() { }

	operator ItemInst* () {
		if(d_) {
			return reinterpret_cast<ItemInst*>(d_);
		}

		return nullptr;
	}

	bool Null();
	bool Valid();

	void *d_;
};

#endif
#endif
