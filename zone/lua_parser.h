#ifndef _EQE_LUA_PARSER_H
#define _EQE_LUA_PARSER_H
#ifdef LUA_EQEMU

#include "quest_parser_collection.h"
#include "quest_interface.h"
#include <string>
#include <list>
#include <map>
#include <exception>

#include "zone_config.h"
#include "lua_mod.h"

extern const ZoneConfig *Config;

struct lua_State;
class Client;
class NPC;

namespace EQEmu
{
	class ItemInstance;
}

#include "lua_parser_events.h"

struct lua_registered_event;
namespace luabind {
	namespace adl {
		class object;
	}
}

class LuaParser : public QuestInterface {
public:
	~LuaParser();

	virtual int EventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data,
		std::vector<EQEmu::Any> *extra_pointers);
	virtual int EventGlobalNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data,
		std::vector<EQEmu::Any> *extra_pointers);
	virtual int EventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data,
		std::vector<EQEmu::Any> *extra_pointers);
	virtual int EventGlobalPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data,
		std::vector<EQEmu::Any> *extra_pointers);
	virtual int EventItem(QuestEventID evt, Client *client, EQEmu::ItemInstance *item, Mob *mob, std::string data, uint32 extra_data,
		std::vector<EQEmu::Any> *extra_pointers);
	virtual int EventSpell(QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, uint32 extra_data,
		std::vector<EQEmu::Any> *extra_pointers);
	virtual int EventEncounter(QuestEventID evt, std::string encounter_name, std::string data, uint32 extra_data,
		std::vector<EQEmu::Any> *extra_pointers);

	virtual bool HasQuestSub(uint32 npc_id, QuestEventID evt);
	virtual bool HasGlobalQuestSub(QuestEventID evt);
	virtual bool PlayerHasQuestSub(QuestEventID evt);
	virtual bool GlobalPlayerHasQuestSub(QuestEventID evt);
	virtual bool SpellHasQuestSub(uint32 spell_id, QuestEventID evt);
	virtual bool ItemHasQuestSub(EQEmu::ItemInstance *itm, QuestEventID evt);
	virtual bool EncounterHasQuestSub(std::string encounter_name, QuestEventID evt);

	virtual void LoadNPCScript(std::string filename, int npc_id);
	virtual void LoadGlobalNPCScript(std::string filename);
	virtual void LoadPlayerScript(std::string filename);
	virtual void LoadGlobalPlayerScript(std::string filename);
	virtual void LoadItemScript(std::string filename, EQEmu::ItemInstance *item);
	virtual void LoadSpellScript(std::string filename, uint32 spell_id);
	virtual void LoadEncounterScript(std::string filename, std::string encounter_name);

	virtual void AddVar(std::string name, std::string val);
	virtual std::string GetVar(std::string name);
	virtual void Init();
	virtual void ReloadQuests();
    virtual uint32 GetIdentifier() { return 0xb0712acc; }

	virtual int DispatchEventNPC(QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data,
		std::vector<EQEmu::Any> *extra_pointers);
	virtual int DispatchEventPlayer(QuestEventID evt, Client *client, std::string data, uint32 extra_data,
		std::vector<EQEmu::Any> *extra_pointers);
	virtual int DispatchEventItem(QuestEventID evt, Client *client, EQEmu::ItemInstance *item, Mob *mob, std::string data, uint32 extra_data,
		std::vector<EQEmu::Any> *extra_pointers);
	virtual int DispatchEventSpell(QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, uint32 extra_data,
		std::vector<EQEmu::Any> *extra_pointers);

	static LuaParser* Instance() {
		static LuaParser inst;
		return &inst;
	}

	bool HasFunction(std::string function, std::string package_name);

	//Mod Extensions
	bool ClientAttack(Mob *self, Mob* other, int Hand, bool bRiposte, bool IsStrikethrough, bool IsFromSpell, ExtraAttackOptions *opts, bool &ignoreDefault);
	bool NPCAttack(Mob *self, Mob* other, int Hand, bool bRiposte, bool IsStrikethrough, bool IsFromSpell, ExtraAttackOptions *opts, bool &ignoreDefault);
	bool BotAttack(Mob *self, Mob* other, int Hand, bool bRiposte, bool IsStrikethrough, bool IsFromSpell, ExtraAttackOptions *opts, bool &ignoreDefault);
	void MeleeMitigation(Mob *self, Mob *attacker, DamageHitInfo &hit, ExtraAttackOptions *opts, bool &ignoreDefault);
	void ApplyDamageTable(Mob *self, DamageHitInfo &hit, bool &ignoreDefault);
	bool AvoidDamage(Mob *self, Mob *other, DamageHitInfo &hit, bool &ignoreDefault);
	bool CheckHitChance(Mob *self, Mob* other, DamageHitInfo &hit, bool &ignoreDefault);
	void DoSpecialAttackDamage(Mob *self, Mob *who, EQEmu::skills::SkillType skill, int32 base_damage, int32 min_damage, int32 hate_override, int ReuseTime, bool &ignoreDefault);
	void DoRangedAttackDmg(Mob *self, Mob* other, bool Launch, int16 damage_mod, int16 chance_mod, EQEmu::skills::SkillType skill, float speed, const char *IDFile, bool &ignoreDefault);
	void DoArcheryAttackDmg(Mob *self, Mob *other, const EQEmu::ItemInstance *RangeWeapon, const EQEmu::ItemInstance *Ammo, uint16 weapon_damage, int16 chance_mod, int16 focus, 
		int ReuseTime, uint32 range_id, uint32 ammo_id, const EQEmu::ItemData *AmmoItem, int AmmoSlot, float speed, bool &ignoreDefault);
	void DoThrowingAttackDmg(Mob *self, Mob *other, const EQEmu::ItemInstance *RangeWeapon, const EQEmu::ItemData *AmmoItem, uint16 weapon_damage, int16 chance_mod, int16 focus, 
		int ReuseTime, uint32 range_id, int AmmoSlot, float speed, bool &ignoreDefault);
	void DoMeleeSkillAttackDmg(Mob *self, Mob *other, uint16 weapon_damage, EQEmu::skills::SkillType skillinuse, int16 chance_mod, int16 focus, bool CanRiposte, int ReuseTime, 
		bool &ignoreDefault);

private:
	LuaParser();
	LuaParser(const LuaParser&);
	LuaParser& operator=(const LuaParser&);

	int _EventNPC(std::string package_name, QuestEventID evt, NPC* npc, Mob *init, std::string data, uint32 extra_data,
		std::vector<EQEmu::Any> *extra_pointers, luabind::adl::object *l_func = nullptr);
	int _EventPlayer(std::string package_name, QuestEventID evt, Client *client, std::string data, uint32 extra_data,
		std::vector<EQEmu::Any> *extra_pointers, luabind::adl::object *l_func = nullptr);
	int _EventItem(std::string package_name, QuestEventID evt, Client *client, EQEmu::ItemInstance *item, Mob *mob, std::string data,
		uint32 extra_data, std::vector<EQEmu::Any> *extra_pointers, luabind::adl::object *l_func = nullptr);
	int _EventSpell(std::string package_name, QuestEventID evt, NPC* npc, Client *client, uint32 spell_id, uint32 extra_data,
		std::vector<EQEmu::Any> *extra_pointers, luabind::adl::object *l_func = nullptr);
	int _EventEncounter(std::string package_name, QuestEventID evt, std::string encounter_name, std::string data, uint32 extra_data,
		std::vector<EQEmu::Any> *extra_pointers);

	void LoadScript(std::string filename, std::string package_name);
	void MapFunctions(lua_State *L);
	QuestEventID ConvertLuaEvent(QuestEventID evt);

	std::map<std::string, std::string> vars_;
	std::map<std::string, bool> loaded_;
	std::vector<LuaMod> mods_;
	lua_State *L;

	NPCArgumentHandler NPCArgumentDispatch[_LargestEventID];
	PlayerArgumentHandler PlayerArgumentDispatch[_LargestEventID];
	ItemArgumentHandler ItemArgumentDispatch[_LargestEventID];
	SpellArgumentHandler SpellArgumentDispatch[_LargestEventID];
	EncounterArgumentHandler EncounterArgumentDispatch[_LargestEventID];

};

#endif
#endif
