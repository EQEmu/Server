#ifndef EQEMU_LUA_CLIENT_H
#define EQEMU_LUA_CLIENT_H
#ifdef LUA_EQEMU

#include "lua_mob.h"

class Client;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_client();
luabind::scope lua_register_inventory_where();

class Lua_Client : public Lua_Mob
{
	typedef Client NativeType;
public:
	Lua_Client() { }
	Lua_Client(Client *d) { SetLuaPtrData(d); }
	virtual ~Lua_Client() { }

	operator Client*() {
		void *d = GetLuaPtrData();
		if(d) {
			return reinterpret_cast<Client*>(d);
		}

		return nullptr;
	}

	void SendSound();
	void Save(int commit_now = 0);
	void SaveBackup();
	bool Connected();
	bool InZone();
	void Kick();
	void Disconnect();
	bool IsLD();
	void WorldKick();
	bool GetAnon();
	void Duck();
	void Stand();
	void SetGM(bool v);
	void SetPVP(bool v);
	bool GetPVP();
	bool GetGM();
	void SetBaseClass(int v);
	void SetBaseRace(int v);
	void SetBaseGender(int v);
	int GetBaseFace();
	int GetLanguageSkill(int skill_id);
	const char *GetLastName();
	int GetLDoNPointsTheme(int theme);
	int GetBaseSTR();
	int GetBaseSTA();
	int GetBaseCHA();
	int GetBaseDEX();
	int GetBaseINT();
	int GetBaseAGI();
	int GetBaseWIS();
	int GetWeight();
	uint32 GetEXP();
	uint32 GetAAExp();
	uint32 GetTotalSecondsPlayed();
	void UpdateLDoNPoints(int points, uint32 theme);
	void SetDeity(int v);
	void AddEXP(uint32 add_exp, int conlevel = 255, bool resexp = false);
	void SetEXP(uint32 set_exp, uint32 set_aaxp, bool resexp = false);
	void SetBindPoint(int to_zone = -1, float new_x = 0.0f, float new_y = 0.0f, float new_z = 0.0f);
	float GetBindX(int index = 0);
	float GetBindY(int index = 0);
	float GetBindZ(int index = 0);
	float GetBindHeading(int index = 0);
	uint32 GetBindZoneID(int index = 0);
	void MovePC(int zone, float x, float y, float z, float heading);
	void MovePCInstance(int zone, int instance, float x, float y, float z, float heading);
	void ChangeLastName(const char *in);
	int GetFactionLevel(uint32 char_id, uint32 npc_id, uint32 race, uint32 class_, uint32 deity, uint32 faction, Lua_NPC npc);
	void SetFactionLevel(uint32 char_id, uint32 npc_id, int char_class, int char_race, int char_deity);
	void SetFactionLevel2(uint32 char_id, int faction_id, int char_class, int char_race, int char_deity, int value, int temp);
	int GetRawItemAC();
	uint32 AccountID();
	const char *AccountName();
	int Admin();
	uint32 CharacterID();
	int GuildRank();
	uint32 GuildID();
	int GetFace();

	bool TakeMoneyFromPP(uint64 copper, bool update_client = false);
	void AddMoneyToPP(uint32 copper, uint32 silver, uint32 gold, uint32 platinum, bool update_client = false);
	bool TGB();
	int GetSkillPoints();
	void SetSkillPoints(int skill);
	void IncreaseSkill(int skill_id, int value = 1);
	void IncreaseLanguageSkill(int skill_id, int value = 1);
	int GetRawSkill(int skill_id);
	bool HasSkill(int skill_id);
	bool CanHaveSkill(int skill_id);
	void SetSkill(int skill_id, int value);
	void AddSkill(int skill_id, int value);
	void CheckSpecializeIncrease(int spell_id);
	void CheckIncreaseSkill(int skill_id, Lua_Mob target, int chance_mod = 0);
	void SetLanguageSkill(int language, int value);
	int MaxSkill(int skill_id);
	bool IsMedding();
	int GetDuelTarget();
	bool IsDueling();
	void SetDuelTarget(int c);
	void SetDueling(bool v);
	void ResetAA();
	void MemSpell(int spell_id, int slot, bool update_client = true);
	void UnmemSpell(int slot, bool update_client = true);
	void UnmemSpellAll(bool update_client = true);
	void ScribeSpell(int spell_id, int slot, bool update_client = true);
	void UnscribeSpell(int slot, bool update_client = true);
	void UnscribeSpellAll(bool update_client = true);
	void UntrainDisc(int slot, bool update_client = true);
	void UntrainDiscAll(bool update_client = true);
	bool IsSitting();
	void SetFeigned(bool v);
	bool GetFeigned();
	bool AutoSplitEnabled();
	void SetHorseId(int id);
	int GetHorseId();
	void NukeItem(uint32 item_num, int where_to_check);
	void SetTint(int slot_id, uint32 color);
	void SetMaterial(int slot_id, uint32 item_id);
	void Undye();
	int GetItemIDAt(int slot_id);
	int GetAugmentIDAt(int slot_id, int aug_slot);
	void DeleteItemInInventory(int slot_id, int quantity, bool update_client = true);
	void SummonItem(uint32 item_id);
	void SummonItem(uint32 item_id, int charges);
	void SummonItem(uint32 item_id, int charges, uint32 aug1);
	void SummonItem(uint32 item_id, int charges, uint32 aug1, uint32 aug2);
	void SummonItem(uint32 item_id, int charges, uint32 aug1, uint32 aug2, uint32 aug3);
	void SummonItem(uint32 item_id, int charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4);
	void SummonItem(uint32 item_id, int charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5);
	void SummonItem(uint32 item_id, int charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5, 
		bool attuned);
	void SummonItem(uint32 item_id, int charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5, 
		bool attuned, int to_slot);
	void SetStats(int type, int value);
	void IncStats(int type, int value);
	void DropItem(int slot_id);
	void BreakInvis();
	void LeaveGroup();
	bool IsGrouped();
	bool IsRaidGrouped();
	bool Hungry();
	bool Thirsty();
	int GetInstrumentMod(int spell_id);
	bool DecreaseByID(uint32 type, int amt);
	void Escape();
	void GoFish();
	void ForageItem(bool guarantee = false);
	float CalcPriceMod(Lua_Mob other, bool reverse);
	void ResetTrade();
	bool UseDiscipline(int spell_id, int target_id);
	int GetCharacterFactionLevel(int faction_id);
	void SetZoneFlag(int zone_id);
	void ClearZoneFlag(int zone_id);
	bool HasZoneFlag(int zone_id);
	void SendZoneFlagInfo(Lua_Client to);
	void SetAATitle(const char *title);
	int GetClientVersion();
	uint32 GetClientVersionBit();
	void SetTitleSuffix(const char *text);
	void SetAAPoints(int points);
	int GetAAPoints();
	int GetSpentAA();
	void AddAAPoints(int points);
	void RefundAA();
	int GetModCharacterFactionLevel(int faction);
	int GetLDoNWins();
	int GetLDoNLosses();
	int GetLDoNWinsTheme(int theme);
	int GetLDoNLossesTheme(int theme);
	Lua_ItemInst GetItemAt(int slot);
	int GetStartZone();
	void SetStartZone(int zone_id, float x = 0.0f, float y = 0.0f, float z = 0.0f);
	void KeyRingAdd(uint32 item);
	bool KeyRingCheck(uint32 item);
	void AddPVPPoints(uint32 points);
	void AddCrystals(uint32 radiant, uint32 ebon);
	uint32 GetPVPPoints();
	uint32 GetRadiantCrystals();
	uint32 GetEbonCrystals();
	void QuestReadBook(const char *text, int type);
	void UpdateGroupAAs(int points, uint32 type);
	uint32 GetGroupPoints();
	uint32 GetRaidPoints();
	void LearnRecipe(uint32 recipe);
	int GetEndurance();
	int GetMaxEndurance();
	int GetEndurancePercent();
	void SetEndurance(int endur);
	void SendOPTranslocateConfirm(Lua_Mob caster, int spell_id);
	uint32 GetIP();
	void AddLevelBasedExp(int exp_pct, int max_level = 0);
	void IncrementAA(int aa);
	void MarkSingleCompassLoc(float in_x, float in_y, float in_z, int count = 1);
	int GetNextAvailableSpellBookSlot(int start = 0);
	int FindSpellBookSlotBySpellID(int spell_id);
	void UpdateTaskActivity(int task, int activity, int count);
	void AssignTask(int task, int npc_id);
	void FailTask(int task);
	bool IsTaskCompleted(int task);
	bool IsTaskActive(int task);
	bool IsTaskActivityActive(int task, int activity);
	int GetCorpseCount();
	int GetCorpseID(int corpse);
	int GetCorpseItemAt(int corpse, int slot);
	void AssignToInstance(int instance_id);
	void Freeze();
	void UnFreeze();
	int GetAggroCount();
	uint64 GetCarriedMoney();
	uint64 GetAllMoney();
	void OpenLFGuildWindow();
	void Signal(uint32 id);
	void AddAlternateCurrencyValue(uint32 currency, int amount);
	void SendWebLink(const char *site);
	bool HasSpellScribed(int spell_id);
	void SetAccountFlag(std::string flag, std::string val);
	std::string GetAccountFlag(std::string flag);

	/*
	//unsup features
	Lua_Group GetGroup();
	Lua_Raid GetRaid();
	*/
};

#endif
#endif