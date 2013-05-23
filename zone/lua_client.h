#ifndef EQEMU_LUA_CLIENT_H
#define EQEMU_LUA_CLIENT_H
#ifdef LUA_EQEMU

#include "lua_mob.h"

class Client;

namespace luabind {
	struct scope;
}

luabind::scope lua_register_client();

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

	/*
	void SendSound();
	void Save();
	void Save(bool commit_now);
	void SaveBackup();
	bool Connected();
	bool InZone();
	void Kick();
	void Disconnect();
	bool IsLD();
	WorldKick
	GetAnon
	Duck
	Stand
	SetGM
	SetPVP
	GetPVP
	GetGM
	SetBaseClass
	SetBaseRace
	SetBaseGender
	GetBaseFace
	GetLanguageSkill
	GetLastName
	GetLDoNPointsTheme
	GetBaseSTR
	GetBaseSTA
	GetBaseCHA
	GetBaseDEX
	GetBaseINT
	GetBaseAGI
	GetBaseWIS
	GetWeight
	GetEXP
	GetAAExp
	GetTotalSecondsPlayed
	UpdateLDoNPoints
	SetDeity
	AddEXP
	SetEXP
	SetBindPoint
	GetBindX
	GetBindY
	GetBindZ
	GetBindHeading
	GetBindZoneID
	MovePC
	MovePCInstance
	ChangeLastName
	GetFactionLevel
	SetFactionLevel
	SetFactionLevel2
	GetRawItemAC
	AccountID
	AccountName
	Admin
	CharacterID
	UpdateAdmin
	UpdateWho
	GuildRank
	GuildID
	GetFace
	TakeMoneyFromPP
	AddMoneyToPP
	TGB
	GetSkillPoints
	SetSkillPoints
	IncreaseSkill
	IncreaseLanguageSkill
	GetSkill
	GetRawSkill
	HasSkill
	CanHaveSkill
	SetSkill
	AddSkill
	CheckSpecializeIncrease
	CheckIncreaseSkill
	SetLanguageSkill
	MaxSkill
	GMKill
	IsMedding
	GetDuelTarget
	IsDueling
	SetDuelTarget
	SetDueling
	ResetAA
	MemSpell
	UnmemSpell
	UnmemSpellAll
	ScribeSpell
	UnscribeSpell
	UnscribeSpellAll
	UntrainDisc
	UntrainDiscAll
	IsSitting
	IsBecomeNPC
	GetBecomeNPCLevel
	SetBecomeNPC
	SetBecomeNPCLevel
	LootToStack
	SetFeigned
	GetFeigned
	AutoSplitEnabled
	SetHorseId
	GetHorseId
	NukeItem
	SetTint
	SetMaterial
	Undye
	GetItemIDAt
	GetAugmentIDAt
	DeleteItemInInventory
	SummonItem
	SetStats
	IncStats
	DropItem
	BreakInvis
	GetGroup
	LeaveGroup
	GetRaid
	IsGrouped
	IsRaidGrouped
	Hungry
	Thirsty
	GetInstrumentMod
	DecreaseByID
	SlotConvert2
	Escape
	RemoveNoRent
	GoFish
	ForageItem
	CalcPriceMod
	ResetTrade
	UseDiscipline
	GetCharacterFactionLevel
	SetZoneFlag
	ClearZoneFlag
	HasZoneFlag
	SendZoneFlagInfo
	LoadZoneFlags
	SetAATitle
	GetClientVersion
	GetClientVersionBit
	SetTitleSuffix
	SetAAPoints
	GetAAPoints
	GetSpentAA
	AddAAPoints
	RefundAA
	GetModCharacterFactionLevel
	GetLDoNWins
	GetLDoNLosses
	GetLDoNWinsTheme
	GetLDoNLossesTheme
	GetItemAt
	GetAugmentAt
	GetStartZone
	SetStartZone
	KeyRingAdd
	KeyRingCheck
	AddPVPPoints
	AddCrystals
	GetPVPPoints
	GetRadiantCrystals
	GetEbonCrystals
	ReadBook
	UpdateGroupAAs
	GetGroupPoints
	GetRaidPoints
	LearnRecipe
	GetEndurance
	GetMaxEndurance
	GetEnduranceRatio
	SetEndurance
	SendOPTranslocateConfirm
	NPCSpawn
	GetIP
	AddLevelBasedExp
	IncrementAA
	GetAALevel
	MarkCompassLoc
	ClearCompassMark
	GetFreeSpellBookSlot
	GetSpellBookSlotBySpellID
	UpdateTaskActivity
	AssignTask
	FailTask
	IsTaskCompleted
	IsTaskActive
	IsTaskActivityActive
	GetCorpseCount
	GetCorpseID
	GetCorpseItemAt
	AssignToInstance
	Freeze
	UnFreeze
	GetAggroCount
	GetCarriedMoney
	GetAllMoney
	GetItemInInventory
	SetCustomItemData
	GetCustomItemData
	OpenLFGuildWindow
	SignalClient
	AddAlternateCurrencyValue
	SendWebLink
	GetInstanceID
	HasSpellScribed
	SetAccountFlag
	GetAccountFlag
	*/
};

#endif
#endif