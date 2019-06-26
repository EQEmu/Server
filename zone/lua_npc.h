#ifndef EQEMU_LUA_NPC_H
#define EQEMU_LUA_NPC_H
#ifdef LUA_EQEMU

#include "lua_mob.h"

class NPC;
class Lua_Mob;
class Lua_NPC;
class Lua_Client;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_npc();

class Lua_NPC : public Lua_Mob
{
	typedef NPC NativeType;
public:
	Lua_NPC() { SetLuaPtrData(nullptr); }
	Lua_NPC(NPC *d) { SetLuaPtrData(reinterpret_cast<Entity*>(d)); }
	virtual ~Lua_NPC() { }

	operator NPC*() {
		return reinterpret_cast<NPC*>(GetLuaPtrData());
	}

	void Signal(int id);
	int CheckNPCFactionAlly(int faction);
	void AddItem(int item_id, int charges);
	void AddItem(int item_id, int charges, bool equip);
	void AddItem(int item_id, int charges, bool equip, int aug1);
	void AddItem(int item_id, int charges, bool equip, int aug1, int aug2);
	void AddItem(int item_id, int charges, bool equip, int aug1, int aug2, int aug3);
	void AddItem(int item_id, int charges, bool equip, int aug1, int aug2, int aug3, int aug4);
	void AddItem(int item_id, int charges, bool equip, int aug1, int aug2, int aug3, int aug4, int aug5);
	void AddItem(int item_id, int charges, bool equip, int aug1, int aug2, int aug3, int aug4, int aug5, int aug6);
	void AddLootTable();
	void AddLootTable(int id);
	void RemoveItem(int item_id);
	void RemoveItem(int item_id, int quantity);
	void RemoveItem(int item_id, int quantity, int slot);
	void ClearItemList();
	void AddCash(int copper, int silver, int gold, int platinum);
	void RemoveCash();
	int CountLoot();
	int GetLoottableID();
	uint32 GetCopper();
	uint32 GetSilver();
	uint32 GetGold();
	uint32 GetPlatinum();
	void SetCopper(uint32 amt);
	void SetSilver(uint32 amt);
	void SetGold(uint32 amt);
	void SetPlatinum(uint32 amt);
	void SetGrid(int grid);
	void SetSaveWaypoint(int wp);
	void SetSp2(int sg2);
	int GetWaypointMax();
	int GetGrid();
	uint32 GetSp2();
	int GetNPCFactionID();
	int GetPrimaryFaction();
	int GetNPCHate(Lua_Mob ent);
	bool IsOnHatelist(Lua_Mob ent);
	void SetNPCFactionID(int id);
	uint32 GetMaxDMG();
	uint32 GetMinDMG();
	bool IsAnimal();
	int GetPetSpellID();
	void SetPetSpellID(int id);
	uint32 GetMaxDamage(int level);
	void SetTaunting(bool t);
	void PickPocket(Lua_Client thief);
	void StartSwarmTimer(uint32 duration);
	void DoClassAttacks(Lua_Mob target);
	int GetMaxWp();
	void DisplayWaypointInfo(Lua_Client to);
	void CalculateNewWaypoint();
	void AssignWaypoints(int grid);
	void SetWaypointPause();
	void UpdateWaypoint(int wp);
	void StopWandering();
	void ResumeWandering();
	void PauseWandering(int pause_time);
	void MoveTo(float x, float y, float z, float h, bool save);
	void NextGuardPosition();
	void SaveGuardSpot(float x, float y, float z, float heading);
	bool IsGuarding();
	void AI_SetRoambox(float dist, float max_x, float min_x, float max_y, float min_y);
	void AI_SetRoambox(float dist, float max_x, float min_x, float max_y, float min_y, uint32 delay, uint32 mindelay);
	void SetFollowID(int id);
	void SetFollowDistance(int dist);
	void SetFollowCanRun(bool v);
	int GetFollowID();
	int GetFollowDistance();
	bool GetFollowCanRun();
	int GetNPCSpellsID();
	int GetSpawnPointID();
	float GetSpawnPointX();
	float GetSpawnPointY();
	float GetSpawnPointZ();
	float GetSpawnPointH();
	float GetGuardPointX();
	float GetGuardPointY();
	float GetGuardPointZ();
	void SetPrimSkill(int skill_id);
	void SetSecSkill(int skill_id);
	int GetPrimSkill();
	int GetSecSkill();
	int GetSwarmOwner();
	int GetSwarmTarget();
	void SetSwarmTarget(int target);
	void ModifyNPCStat(const char *stat, const char *value);
	void AddAISpell(int priority, int spell_id, int type, int mana_cost, int recast_delay, int resist_adjust);
	void AddAISpell(int priority, int spell_id, int type, int mana_cost, int recast_delay, int resist_adjust, int min_hp, int max_hp);
	void RemoveAISpell(int spell_id);
	void SetSpellFocusDMG(int focus);
	void SetSpellFocusHeal(int focus);
	int GetSpellFocusDMG();
	int GetSpellFocusHeal();
	float GetSlowMitigation();
	float GetAttackSpeed();
	int GetAttackDelay();
	int GetAccuracyRating();
	int GetSpawnKillCount();
	int GetScore();
	void MerchantOpenShop();
	void MerchantCloseShop();
	int GetRawAC();
	int GetAvoidanceRating();
};

#endif
#endif
