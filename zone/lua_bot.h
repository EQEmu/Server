#ifndef EQEMU_LUA_BOT_H
#define EQEMU_LUA_BOT_H
#ifdef LUA_EQEMU

#include "lua_mob.h"

class Bot;
class Lua_Bot;
class Lua_Mob;
class Lua_Group;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_bot();

class Lua_Bot : public Lua_Mob
{
	typedef Bot NativeType;
public:
	Lua_Bot() { SetLuaPtrData(nullptr); }
	Lua_Bot(Bot *d) { SetLuaPtrData(reinterpret_cast<Entity*>(d)); }
	virtual ~Lua_Bot() { }

	operator Bot*() {
		return reinterpret_cast<Bot*>(GetLuaPtrData());
	}

	void AddBotItem(uint16 slot_id, uint32 item_id);
	void AddBotItem(uint16 slot_id, uint32 item_id, int16 charges);
	void AddBotItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned);
	void AddBotItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned, uint32 augment_one);
	void AddBotItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned, uint32 augment_one, uint32 augment_two);
	void AddBotItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned, uint32 augment_one, uint32 augment_two, uint32 augment_three);
	void AddBotItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned, uint32 augment_one, uint32 augment_two, uint32 augment_three, uint32 augment_four);
	void AddBotItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned, uint32 augment_one, uint32 augment_two, uint32 augment_three, uint32 augment_four, uint32 augment_five);
	void AddBotItem(uint16 slot_id, uint32 item_id, int16 charges, bool attuned, uint32 augment_one, uint32 augment_two, uint32 augment_three, uint32 augment_four, uint32 augment_five, uint32 augment_six);
	uint32 CountBotItem(uint32 item_id);
	Lua_ItemInst GetBotItem(uint16 slot_id);
	uint32 GetBotItemIDBySlot(uint16 slot_id);
	int GetExpansionBitmask();
	Lua_Mob GetOwner();
	bool HasBotItem(uint32 item_id);
	void OwnerMessage(std::string message);
	bool ReloadBotDataBuckets();
	bool ReloadBotOwnerDataBuckets();
	bool ReloadBotSpells();
	void ReloadBotSpellSettings();
	void RemoveBotItem(uint32 item_id);
	void SetExpansionBitmask(int expansion_bitmask);
	void SetExpansionBitmask(int expansion_bitmask, bool save);
	void Signal(int signal_id);
	bool HasBotSpellEntry(uint16 spellid);
	void SendPayload(int payload_id);
	void SendPayload(int payload_id, std::string payload_value);
	uint32 GetBotID();
	void Camp();
	void Camp(bool save_to_database);
	Lua_ItemInst GetAugmentAt(int16 slot_id, uint8 augment_index);
	int GetAugmentIDAt(int16 slot_id, uint8 augment_index);
	Lua_ItemInst GetItemAt(int16 slot_id);
	int GetItemIDAt(int16 slot_id);
	void SendSpellAnim(uint16 target_id, uint16 spell_id);

	void ApplySpell(int spell_id);
	void ApplySpell(int spell_id, int duration);
	void ApplySpell(int spell_id, int duration, bool allow_pets);

	void ApplySpellGroup(int spell_id);
	void ApplySpellGroup(int spell_id, int duration);
	void ApplySpellGroup(int spell_id, int duration, bool allow_pets);

	void SetSpellDuration(int spell_id);
	void SetSpellDuration(int spell_id, int duration);
	void SetSpellDuration(int spell_id, int duration, bool allow_pets);

	void SetSpellDurationGroup(int spell_id);
	void SetSpellDurationGroup(int spell_id, int duration);
	void SetSpellDurationGroup(int spell_id, int duration, bool allow_pets);

	void SetSpellDurationRaid(int spell_id);
	void SetSpellDurationRaid(int spell_id, int duration);
	void SetSpellDurationRaid(int spell_id, int duration, bool allow_pets);
	void SetSpellDurationRaid(int spell_id, int duration, bool allow_pets, bool is_raid_group_only);

	int CountAugmentEquippedByID(uint32 item_id);
	int CountItemEquippedByID(uint32 item_id);
	bool HasAugmentEquippedByID(uint32 item_id);
	bool HasItemEquippedByID(uint32 item_id);
	int GetHealAmount();
	int GetSpellDamage();

	void Escape();
	int GetInstrumentMod(int spell_id);

	int GetBaseSTR();
	int GetBaseSTA();
	int GetBaseCHA();
	int GetBaseDEX();
	int GetBaseINT();
	int GetBaseAGI();
	int GetBaseWIS();

	Lua_Group GetGroup();
	int GetRawItemAC();

	bool IsGrouped();
	bool IsStanding();
	bool IsSitting();
	void Sit();
	void Stand();

	void Fling(float target_x, float target_y, float target_z);
	void Fling(float target_x, float target_y, float target_z, bool ignore_los);
	void Fling(float target_x, float target_y, float target_z, bool ignore_los, bool clip_through_walls);
	void Fling(float value, float target_x, float target_y, float target_z);
	void Fling(float value, float target_x, float target_y, float target_z, bool ignore_los);
	void Fling(float value, float target_x, float target_y, float target_z, bool ignore_los, bool clip_through_walls);
};

#endif
#endif
