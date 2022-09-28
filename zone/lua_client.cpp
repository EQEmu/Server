#ifdef LUA_EQEMU

#include <sol/sol.hpp>

#include "client.h"
#include "dynamic_zone.h"
#include "expedition_request.h"
#include "lua_client.h"
#include "lua_expedition.h"
#include "lua_npc.h"
#include "lua_item.h"
#include "lua_iteminst.h"
#include "lua_inventory.h"
#include "lua_group.h"
#include "lua_raid.h"
#include "lua_packet.h"
#include "dialogue_window.h"
#include "titles.h"
#include "../common/expedition_lockout_timer.h"

void lua_register_client(sol::state_view &sv)
{
	auto client = sv.new_usertype<Lua_Client>("Client", sol::constructors<Lua_Client()>(), sol::base_classes,
						  sol::bases<Lua_Mob, Lua_Entity>());
	client["AccountID"] = (uint32(Lua_Client::*)(void))&Lua_Client::AccountID;
	client["AccountName"] = (const char *(Lua_Client::*)(void))&Lua_Client::AccountName;
	client["AddAAPoints"] = (void(Lua_Client::*)(int))&Lua_Client::AddAAPoints;
	client["AddAlternateCurrencyValue"] = (void(Lua_Client::*)(uint32,int))&Lua_Client::AddAlternateCurrencyValue;
	client["AddCrystals"] = (void(Lua_Client::*)(uint32,uint32))&Lua_Client::AddCrystals;
	client["AddEXP"] = sol::overload((void(Lua_Client::*)(uint32))&Lua_Client::AddEXP,
					 (void(Lua_Client::*)(uint32, int))&Lua_Client::AddEXP,
					 (void(Lua_Client::*)(uint32, int, bool))&Lua_Client::AddEXP);
	client["AddExpeditionLockout"] = sol::overload(
	    (void(Lua_Client::*)(std::string, std::string, uint32))&Lua_Client::AddExpeditionLockout,
	    (void(Lua_Client::*)(std::string, std::string, uint32, std::string))&Lua_Client::AddExpeditionLockout);
	client["AddExpeditionLockoutDuration"] = sol::overload(
	    (void(Lua_Client::*)(std::string, std::string, int))&Lua_Client::AddExpeditionLockoutDuration,
	    (void(Lua_Client::*)(std::string, std::string, int, std::string))&Lua_Client::AddExpeditionLockoutDuration);
	client["AddItem"] = (void(Lua_Client::*)(sol::table))&Lua_Client::AddItem;
	client["AddLDoNLoss"] = (void(Lua_Client::*)(uint32))&Lua_Client::AddLDoNLoss;
	client["AddLDoNWin"] = (void(Lua_Client::*)(uint32))&Lua_Client::AddLDoNWin;
	client["AddLevelBasedExp"] =
	    sol::overload((void(Lua_Client::*)(int))&Lua_Client::AddLevelBasedExp,
			  (void(Lua_Client::*)(int, int))&Lua_Client::AddLevelBasedExp,
			  (void(Lua_Client::*)(int, int, bool))&Lua_Client::AddLevelBasedExp);
	client["AddMoneyToPP"] =
	    sol::overload((void(Lua_Client::*)(uint32, uint32, uint32, uint32))&Lua_Client::AddMoneyToPP,
			  (void(Lua_Client::*)(uint32, uint32, uint32, uint32, bool))&Lua_Client::AddMoneyToPP);
	client["AddPlatinum"] = sol::overload((void(Lua_Client::*)(uint32))&Lua_Client::AddPlatinum,
					      (void(Lua_Client::*)(uint32, bool))&Lua_Client::AddPlatinum);
	client["AddPVPPoints"] = (void(Lua_Client::*)(uint32))&Lua_Client::AddPVPPoints;
	client["AddSkill"] = (void(Lua_Client::*)(int,int))&Lua_Client::AddSkill;
	client["Admin"] = (int16(Lua_Client::*)(void))&Lua_Client::Admin;
	client["AssignTask"] = sol::overload((void(Lua_Client::*)(int))&Lua_Client::AssignTask,
					     (void(Lua_Client::*)(int, int))&Lua_Client::AssignTask,
					     (void(Lua_Client::*)(int, int, bool))&Lua_Client::AssignTask);
	client["AssignToInstance"] = (void(Lua_Client::*)(int))&Lua_Client::AssignToInstance;
	client["AutoSplitEnabled"] = (bool(Lua_Client::*)(void))&Lua_Client::AutoSplitEnabled;
	client["BreakInvis"] = (void(Lua_Client::*)(void))&Lua_Client::BreakInvis;
	client["CalcATK"] = &Lua_Client::CalcATK;
	client["CalcCurrentWeight"] = &Lua_Client::CalcCurrentWeight;
	client["CalcPriceMod"] = (float(Lua_Client::*)(Lua_Mob,bool))&Lua_Client::CalcPriceMod;
	client["CanHaveSkill"] = (bool(Lua_Client::*)(int))&Lua_Client::CanHaveSkill;
	client["CashReward"] = &Lua_Client::CashReward;
	client["ChangeLastName"] = (void(Lua_Client::*)(std::string))&Lua_Client::ChangeLastName;
	client["CharacterID"] = (uint32(Lua_Client::*)(void))&Lua_Client::CharacterID;
	client["CheckIncreaseSkill"] =
	    sol::overload((void(Lua_Client::*)(int, Lua_Mob))&Lua_Client::CheckIncreaseSkill,
			  (void(Lua_Client::*)(int, Lua_Mob, int))&Lua_Client::CheckIncreaseSkill);
	client["CheckSpecializeIncrease"] = (void(Lua_Client::*)(int))&Lua_Client::CheckSpecializeIncrease;
	client["ClearCompassMark"] =(void(Lua_Client::*)(void))&Lua_Client::ClearCompassMark;
	client["ClearPEQZoneFlag"] = (void(Lua_Client::*)(uint32))&Lua_Client::ClearPEQZoneFlag;
	client["ClearZoneFlag"] = (void(Lua_Client::*)(uint32))&Lua_Client::ClearZoneFlag;
	client["Connected"] = (bool(Lua_Client::*)(void))&Lua_Client::Connected;
	client["CountAugmentEquippedByID"] = (int(Lua_Client::*)(uint32))&Lua_Client::CountAugmentEquippedByID;
	client["CountItem"] = (int(Lua_Client::*)(uint32))&Lua_Client::CountItem;
	client["CountItemEquippedByID"] = (int(Lua_Client::*)(uint32))&Lua_Client::CountItemEquippedByID;
	client["CreateExpedition"] = sol::overload(
	    (Lua_Expedition(Lua_Client::*)(sol::table))&Lua_Client::CreateExpedition,
	    (Lua_Expedition(Lua_Client::*)(std::string, uint32, uint32, std::string, uint32, uint32))
		&Lua_Client::CreateExpedition,
	    (Lua_Expedition(Lua_Client::*)(std::string, uint32, uint32, std::string, uint32, uint32, bool))
		&Lua_Client::CreateExpedition);
	client["CreateExpeditionFromTemplate"] = &Lua_Client::CreateExpeditionFromTemplate;
	client["CreateTaskDynamicZone"] = &Lua_Client::CreateTaskDynamicZone;
	client["DecreaseByID"] = (bool(Lua_Client::*)(uint32,int))&Lua_Client::DecreaseByID;
	client["DeleteItemInInventory"] =
	    sol::overload((void(Lua_Client::*)(int, int))&Lua_Client::DeleteItemInInventory,
			  (void(Lua_Client::*)(int, int, bool))&Lua_Client::DeleteItemInInventory);
	client["DiaWind"] = (void(Lua_Client::*)(std::string))&Lua_Client::DialogueWindow;
	client["DialogueWindow"] = (void(Lua_Client::*)(std::string))&Lua_Client::DialogueWindow;
	client["DisableAreaEndRegen"] = &Lua_Client::DisableAreaEndRegen;
	client["DisableAreaHPRegen"] = &Lua_Client::DisableAreaHPRegen;
	client["DisableAreaManaRegen"] = &Lua_Client::DisableAreaManaRegen;
	client["DisableAreaRegens"] = &Lua_Client::DisableAreaRegens;
	client["Disconnect"] = (void(Lua_Client::*)(void))&Lua_Client::Disconnect;
	client["DropItem"] = (void(Lua_Client::*)(int))&Lua_Client::DropItem;
	client["Duck"] = (void(Lua_Client::*)(void))&Lua_Client::Duck;
	client["DyeArmorBySlot"] =
	    sol::overload((void(Lua_Client::*)(uint8, uint8, uint8, uint8))&Lua_Client::DyeArmorBySlot,
			  (void(Lua_Client::*)(uint8, uint8, uint8, uint8, uint8))&Lua_Client::DyeArmorBySlot);
	client["EnableAreaEndRegen"] = &Lua_Client::EnableAreaEndRegen;
	client["EnableAreaHPRegen"] = &Lua_Client::EnableAreaHPRegen;
	client["EnableAreaManaRegen"] = &Lua_Client::EnableAreaManaRegen;
	client["EnableAreaRegens"] = &Lua_Client::EnableAreaRegens;
	client["Escape"] = (void(Lua_Client::*)(void))&Lua_Client::Escape;
	client["FailTask"] = (void(Lua_Client::*)(int))&Lua_Client::FailTask;
	client["FilteredMessage"] = &Lua_Client::FilteredMessage;
	client["FindEmptyMemSlot"] = (int(Lua_Client::*)(void))&Lua_Client::FindEmptyMemSlot;
	client["FindMemmedSpellBySlot"] = (uint16(Lua_Client::*)(int))&Lua_Client::FindMemmedSpellBySlot;
	client["FindMemmedSpellBySpellID"] = (int(Lua_Client::*)(uint16))&Lua_Client::FindMemmedSpellBySpellID;
	client["FindSpellBookSlotBySpellID"] = (int(Lua_Client::*)(int))&Lua_Client::FindSpellBookSlotBySpellID;
	client["Fling"] =
	    sol::overload((void(Lua_Client::*)(float, float, float, float))&Lua_Client::Fling,
			  (void(Lua_Client::*)(float, float, float, float, bool))&Lua_Client::Fling,
			  (void(Lua_Client::*)(float, float, float, float, bool, bool))&Lua_Client::Fling);
	client["ForageItem"] = sol::overload((void(Lua_Client::*)(bool))&Lua_Client::ForageItem,
					     (void(Lua_Client::*)(void))&Lua_Client::ForageItem);
	client["Freeze"] = (void(Lua_Client::*)(void))&Lua_Client::Freeze;
	client["GetAAEXPModifier"] =
	    sol::overload((double(Lua_Client::*)(uint32))&Lua_Client::GetAAEXPModifier,
			  (double(Lua_Client::*)(uint32, int16))&Lua_Client::GetAAEXPModifier);
	client["GetAAExp"] = (uint32(Lua_Client::*)(void))&Lua_Client::GetAAExp;
	client["GetAAPercent"] = (uint32(Lua_Client::*)(void))&Lua_Client::GetAAPercent;
	client["GetAAPoints"] = (int(Lua_Client::*)(void))&Lua_Client::GetAAPoints;
	client["GetAFK"] = (int(Lua_Client::*)(void))&Lua_Client::GetAFK;
	client["GetAccountAge"] = (int(Lua_Client::*)(void))&Lua_Client::GetAccountAge;
	client["GetAccountFlag"] = (std::string(Lua_Client::*)(std::string))&Lua_Client::GetAccountFlag;
	client["GetAggroCount"] = (int(Lua_Client::*)(void))&Lua_Client::GetAggroCount;
	client["GetAllMoney"] = (uint64(Lua_Client::*)(void))&Lua_Client::GetAllMoney;
	client["GetAlternateCurrencyValue"] = (int(Lua_Client::*)(uint32))&Lua_Client::GetAlternateCurrencyValue;
	client["GetAnon"] = (int(Lua_Client::*)(void))&Lua_Client::GetAnon;
	client["GetAugmentIDAt"] = (int(Lua_Client::*)(int,int))&Lua_Client::GetAugmentIDAt;
	client["GetBaseAGI"] = (int(Lua_Client::*)(void))&Lua_Client::GetBaseAGI;
	client["GetBaseCHA"] = (int(Lua_Client::*)(void))&Lua_Client::GetBaseCHA;
	client["GetBaseDEX"] = (int(Lua_Client::*)(void))&Lua_Client::GetBaseDEX;
	client["GetBaseFace"] = (int(Lua_Client::*)(void))&Lua_Client::GetBaseFace;
	client["GetBaseINT"] = (int(Lua_Client::*)(void))&Lua_Client::GetBaseINT;
	client["GetBaseSTA"] = (int(Lua_Client::*)(void))&Lua_Client::GetBaseSTA;
	client["GetBaseSTR"] = (int(Lua_Client::*)(void))&Lua_Client::GetBaseSTR;
	client["GetBaseWIS"] = (int(Lua_Client::*)(void))&Lua_Client::GetBaseWIS;
	client["GetBindHeading"] = sol::overload((float(Lua_Client::*)(int))&Lua_Client::GetBindHeading,
						 (float(Lua_Client::*)(void))&Lua_Client::GetBindHeading);
	client["GetBindX"] = sol::overload((float(Lua_Client::*)(int))&Lua_Client::GetBindX,
					   (float(Lua_Client::*)(void))&Lua_Client::GetBindX);
	client["GetBindY"] = sol::overload((float(Lua_Client::*)(int))&Lua_Client::GetBindY,
					   (float(Lua_Client::*)(void))&Lua_Client::GetBindY);
	client["GetBindZ"] = sol::overload((float(Lua_Client::*)(int))&Lua_Client::GetBindZ,
					   (float(Lua_Client::*)(void))&Lua_Client::GetBindZ);
	client["GetBindZoneID"] = sol::overload((uint32(Lua_Client::*)(int))&Lua_Client::GetBindZoneID,
						(uint32(Lua_Client::*)(void))&Lua_Client::GetBindZoneID);
	client["GetCarriedMoney"] = (uint64(Lua_Client::*)(void))&Lua_Client::GetCarriedMoney;
	client["GetCarriedPlatinum"] = (uint32(Lua_Client::*)(void))&Lua_Client::GetCarriedPlatinum;
	client["GetCharacterFactionLevel"] = (int(Lua_Client::*)(int))&Lua_Client::GetCharacterFactionLevel;
	client["GetClassBitmask"] = (int(Lua_Client::*)(void))&Lua_Client::GetClassBitmask;
	client["GetClientMaxLevel"] = (int(Lua_Client::*)(void))&Lua_Client::GetClientMaxLevel;
	client["GetClientVersion"] = (int(Lua_Client::*)(void))&Lua_Client::GetClientVersion;
	client["GetClientVersionBit"] = (uint32(Lua_Client::*)(void))&Lua_Client::GetClientVersionBit;
	client["GetCorpseCount"] = (int(Lua_Client::*)(void))&Lua_Client::GetCorpseCount;
	client["GetCorpseID"] = (int(Lua_Client::*)(int))&Lua_Client::GetCorpseID;
	client["GetCorpseItemAt"] = (int(Lua_Client::*)(int,int))&Lua_Client::GetCorpseItemAt;
	client["GetDiscSlotBySpellID"] = (int(Lua_Client::*)(int32))&Lua_Client::GetDiscSlotBySpellID;
	client["GetDisciplineTimer"] = (uint32(Lua_Client::*)(uint32))&Lua_Client::GetDisciplineTimer;
	client["GetDuelTarget"] = (int(Lua_Client::*)(void))&Lua_Client::GetDuelTarget;
	client["GetEXP"] = (uint32(Lua_Client::*)(void))&Lua_Client::GetEXP;
	client["GetEXPModifier"] = sol::overload((double(Lua_Client::*)(uint32))&Lua_Client::GetEXPModifier,
						 (double(Lua_Client::*)(uint32, int16))&Lua_Client::GetEXPModifier);
	client["GetEbonCrystals"] = (uint32(Lua_Client::*)(void))&Lua_Client::GetEbonCrystals;
	client["GetEndurance"] = (int(Lua_Client::*)(void))&Lua_Client::GetEndurance;
	client["GetEndurancePercent"] = (int(Lua_Client::*)(void))&Lua_Client::GetEndurancePercent;
	client["GetEnvironmentDamageModifier"] = (int(Lua_Client::*)(void))&Lua_Client::GetEnvironmentDamageModifier;
	client["GetExpedition"] = (Lua_Expedition(Lua_Client::*)(void))&Lua_Client::GetExpedition;
	client["GetExpeditionLockouts"] = sol::overload(
	    (sol::table(Lua_Client::*)(sol::this_state))&Lua_Client::GetExpeditionLockouts,
	    (sol::table(Lua_Client::*)(sol::this_state, std::string))&Lua_Client::GetExpeditionLockouts);
	client["GetFace"] = (int(Lua_Client::*)(void))&Lua_Client::GetFace;
	client["GetFactionLevel"] = (int(Lua_Client::*)(uint32,uint32,uint32,uint32,uint32,uint32,Lua_NPC))&Lua_Client::GetFactionLevel;
	client["GetFeigned"] = (bool(Lua_Client::*)(void))&Lua_Client::GetFeigned;
	client["GetGM"] = (bool(Lua_Client::*)(void))&Lua_Client::GetGM;
	client["GetGMStatus"] = (int16(Lua_Client::*)(void))&Lua_Client::GetGMStatus;
	client["GetGroup"] = (Lua_Group(Lua_Client::*)(void))&Lua_Client::GetGroup;
	client["GetGroupPoints"] = (uint32(Lua_Client::*)(void))&Lua_Client::GetGroupPoints;
	client["GetHorseId"] = (int(Lua_Client::*)(void))&Lua_Client::GetHorseId;
	client["GetHealAmount"] = (int(Lua_Client::*)(void))&Lua_Client::GetHealAmount;
	client["GetHunger"] = (int(Lua_Client::*)(void))&Lua_Client::GetHunger;
	client["GetIP"] = (uint32(Lua_Client::*)(void))&Lua_Client::GetIP;
	client["GetIPExemption"] = (int(Lua_Client::*)(void))&Lua_Client::GetIPExemption;
	client["GetIPString"] = (std::string(Lua_Client::*)(void))&Lua_Client::GetIPString;
	client["GetInstrumentMod"] = (int(Lua_Client::*)(int))&Lua_Client::GetInstrumentMod;
	client["GetInventory"] = (Lua_Inventory(Lua_Client::*)(void))&Lua_Client::GetInventory;
	client["GetInvulnerableEnvironmentDamage"] = (bool(Lua_Client::*)(void))&Lua_Client::GetInvulnerableEnvironmentDamage;
	client["GetItemIDAt"] = (int(Lua_Client::*)(int))&Lua_Client::GetItemIDAt;
	client["GetLDoNLosses"] = (int(Lua_Client::*)(void))&Lua_Client::GetLDoNLosses;
	client["GetLDoNLossesTheme"] = (int(Lua_Client::*)(int))&Lua_Client::GetLDoNLossesTheme;
	client["GetLDoNPointsTheme"] = (int(Lua_Client::*)(int))&Lua_Client::GetLDoNPointsTheme;
	client["GetLDoNWins"] = (int(Lua_Client::*)(void))&Lua_Client::GetLDoNWins;
	client["GetLDoNWinsTheme"] = (int(Lua_Client::*)(int))&Lua_Client::GetLDoNWinsTheme;
	client["GetLanguageSkill"] = (int(Lua_Client::*)(int))&Lua_Client::GetLanguageSkill;
	client["GetLearnableDisciplines"] = sol::overload(
	    (sol::table(Lua_Client::*)(sol::this_state))&Lua_Client::GetLearnableDisciplines,
	    (sol::table(Lua_Client::*)(sol::this_state, uint8))&Lua_Client::GetLearnableDisciplines,
	    (sol::table(Lua_Client::*)(sol::this_state, uint8, uint8))&Lua_Client::GetLearnableDisciplines);
	client["GetLearnedDisciplines"] = (sol::table(Lua_Client::*)(sol::this_state))&Lua_Client::GetLearnedDisciplines;
	client["GetLockoutExpeditionUUID"] = (std::string(Lua_Client::*)(std::string, std::string))&Lua_Client::GetLockoutExpeditionUUID;
	client["GetMaxEndurance"] = (int(Lua_Client::*)(void))&Lua_Client::GetMaxEndurance;
	client["GetMemmedSpells"] = (sol::table(Lua_Client::*)(sol::this_state))&Lua_Client::GetMemmedSpells;
	client["GetModCharacterFactionLevel"] = (int(Lua_Client::*)(int))&Lua_Client::GetModCharacterFactionLevel;
	client["GetMoney"] = (uint32(Lua_Client::*)(uint8, uint8))&Lua_Client::GetMoney;
	client["GetNextAvailableDisciplineSlot"] =
	    sol::overload((int(Lua_Client::*)(int))&Lua_Client::GetNextAvailableDisciplineSlot,
			  (int(Lua_Client::*)(void))&Lua_Client::GetNextAvailableDisciplineSlot);
	client["GetNextAvailableSpellBookSlot"] =
	    sol::overload((int(Lua_Client::*)(int))&Lua_Client::GetNextAvailableSpellBookSlot,
			  (int(Lua_Client::*)(void))&Lua_Client::GetNextAvailableSpellBookSlot);
	client["GetPVP"] = (bool(Lua_Client::*)(void))&Lua_Client::GetPVP;
	client["GetPVPPoints"] = (uint32(Lua_Client::*)(void))&Lua_Client::GetPVPPoints;
	client["GetRaceBitmask"] = (int(Lua_Client::*)(void))&Lua_Client::GetRaceBitmask;
	client["GetRadiantCrystals"] = (uint32(Lua_Client::*)(void))&Lua_Client::GetRadiantCrystals;
	client["GetRaid"] = (Lua_Raid(Lua_Client::*)(void))&Lua_Client::GetRaid;
	client["GetRaidPoints"] = (uint32(Lua_Client::*)(void))&Lua_Client::GetRaidPoints;
	client["GetRawItemAC"] = (int(Lua_Client::*)(void))&Lua_Client::GetRawItemAC;
	client["GetRawSkill"] = (int(Lua_Client::*)(int))&Lua_Client::GetRawSkill;
	client["GetRecipeMadeCount"] = (int(Lua_Client::*)(uint32))&Lua_Client::GetRecipeMadeCount;
	client["GetScribeableSpells"] =
	    sol::overload((sol::table(Lua_Client::*)(sol::this_state))&Lua_Client::GetScribeableSpells,
			  (sol::table(Lua_Client::*)(sol::this_state, uint8))&Lua_Client::GetScribeableSpells,
			  (sol::table(Lua_Client::*)(sol::this_state, uint8, uint8))&Lua_Client::GetScribeableSpells);
	client["GetScribedSpells"] = (sol::table(Lua_Client::*)(sol::this_state))&Lua_Client::GetScribedSpells;
	client["GetSkillPoints"] = (int(Lua_Client::*)(void))&Lua_Client::GetSkillPoints;
	client["GetSpellDamage"] = (int(Lua_Client::*)(void))&Lua_Client::GetSpellDamage;
	client["GetSpellIDByBookSlot"] = (uint32(Lua_Client::*)(int))&Lua_Client::GetSpellIDByBookSlot;
	client["GetSpentAA"] = (int(Lua_Client::*)(void))&Lua_Client::GetSpentAA;
	client["GetStartZone"] = (int(Lua_Client::*)(void))&Lua_Client::GetStartZone;
	client["GetTargetRingX"] = (float(Lua_Client::*)(void))&Lua_Client::GetTargetRingX;
	client["GetTargetRingY"] = (float(Lua_Client::*)(void))&Lua_Client::GetTargetRingY;
	client["GetTargetRingZ"] = (float(Lua_Client::*)(void))&Lua_Client::GetTargetRingZ;
	client["GetThirst"] = (int(Lua_Client::*)(void))&Lua_Client::GetThirst;
	client["GetTotalSecondsPlayed"] = (uint32(Lua_Client::*)(void))&Lua_Client::GetTotalSecondsPlayed;
	client["GetWeight"] = (int(Lua_Client::*)(void))&Lua_Client::GetWeight;
	client["GoFish"] = (void(Lua_Client::*)(void))&Lua_Client::GoFish;
	client["GrantAlternateAdvancementAbility"] =
	    sol::overload((bool(Lua_Client::*)(int, int))&Lua_Client::GrantAlternateAdvancementAbility,
			  (bool(Lua_Client::*)(int, int, bool))&Lua_Client::GrantAlternateAdvancementAbility);
	client["GuildID"] = (uint32(Lua_Client::*)(void))&Lua_Client::GuildID;
	client["GuildRank"] = (int(Lua_Client::*)(void))&Lua_Client::GuildRank;
	client["HasAugmentEquippedByID"] = (bool(Lua_Client::*)(uint32))&Lua_Client::HasAugmentEquippedByID;
	client["HasDisciplineLearned"] = (bool(Lua_Client::*)(uint16))&Lua_Client::HasDisciplineLearned;
	client["HasExpeditionLockout"] = (bool(Lua_Client::*)(std::string, std::string))&Lua_Client::HasExpeditionLockout;
	client["HasItemEquippedByID"] = (bool(Lua_Client::*)(uint32))&Lua_Client::HasItemEquippedByID;
	client["HasPEQZoneFlag"] = (bool(Lua_Client::*)(uint32))&Lua_Client::HasPEQZoneFlag;
	client["HasRecipeLearned"] = (bool(Lua_Client::*)(uint32))&Lua_Client::HasRecipeLearned;
	client["HasSkill"] = (bool(Lua_Client::*)(int))&Lua_Client::HasSkill;
	client["HasSpellScribed"] = (bool(Lua_Client::*)(int))&Lua_Client::HasSpellScribed;
	client["HasZoneFlag"] = (bool(Lua_Client::*)(uint32))&Lua_Client::HasZoneFlag;
	client["Hungry"] = (bool(Lua_Client::*)(void))&Lua_Client::Hungry;
	client["InZone"] = (bool(Lua_Client::*)(void))&Lua_Client::InZone;
	client["IncStats"] = (void(Lua_Client::*)(int,int))&Lua_Client::IncStats;
	client["IncreaseLanguageSkill"] =
	    sol::overload((void(Lua_Client::*)(int))&Lua_Client::IncreaseLanguageSkill,
			  (void(Lua_Client::*)(int, int))&Lua_Client::IncreaseLanguageSkill);
	client["IncreaseSkill"] = sol::overload((void(Lua_Client::*)(int))&Lua_Client::IncreaseSkill,
						(void(Lua_Client::*)(int, int))&Lua_Client::IncreaseSkill);
	client["IncrementAA"] = (void(Lua_Client::*)(int))&Lua_Client::IncrementAA;
	client["IsCrouching"] = (bool(Lua_Client::*)(void))&Lua_Client::IsCrouching;
	client["IsDead"] = &Lua_Client::IsDead;
	client["IsDueling"] = (bool(Lua_Client::*)(void))&Lua_Client::IsDueling;
	client["IsGrouped"] = (bool(Lua_Client::*)(void))&Lua_Client::IsGrouped;
	client["IsLD"] = (bool(Lua_Client::*)(void))&Lua_Client::IsLD;
	client["IsMedding"] = (bool(Lua_Client::*)(void))&Lua_Client::IsMedding;
	client["IsRaidGrouped"] = (bool(Lua_Client::*)(void))&Lua_Client::IsRaidGrouped;
	client["IsSitting"] = (bool(Lua_Client::*)(void))&Lua_Client::IsSitting;
	client["IsStanding"] = (bool(Lua_Client::*)(void))&Lua_Client::IsStanding;
	client["IsTaskActive"] = (bool(Lua_Client::*)(int))&Lua_Client::IsTaskActive;
	client["IsTaskActivityActive"] = (bool(Lua_Client::*)(int,int))&Lua_Client::IsTaskActivityActive;
	client["IsTaskCompleted"] = (bool(Lua_Client::*)(int))&Lua_Client::IsTaskCompleted;
	client["KeyRingAdd"] = (void(Lua_Client::*)(uint32))&Lua_Client::KeyRingAdd;
	client["KeyRingCheck"] = (bool(Lua_Client::*)(uint32))&Lua_Client::KeyRingCheck;
	client["Kick"] = (void(Lua_Client::*)(void))&Lua_Client::Kick;
	client["LearnDisciplines"] = (uint16(Lua_Client::*)(uint8,uint8))&Lua_Client::LearnDisciplines;
	client["LearnRecipe"] = (void(Lua_Client::*)(uint32))&Lua_Client::LearnRecipe;
	client["LeaveGroup"] = (void(Lua_Client::*)(void))&Lua_Client::LeaveGroup;
	client["LoadPEQZoneFlags"] = (void(Lua_Client::*)(void))&Lua_Client::LoadPEQZoneFlags;
	client["LoadZoneFlags"] = (void(Lua_Client::*)(void))&Lua_Client::LoadZoneFlags;
	client["LockSharedTask"] = &Lua_Client::LockSharedTask;
	client["MarkSingleCompassLoc"] =
	    sol::overload((void(Lua_Client::*)(float, float, float))&Lua_Client::MarkSingleCompassLoc,
			  (void(Lua_Client::*)(float, float, float, int))&Lua_Client::MarkSingleCompassLoc);
	client["MaxSkill"] = (int(Lua_Client::*)(int))&Lua_Client::MaxSkill;
	client["MemSpell"] = sol::overload((void(Lua_Client::*)(int, int))&Lua_Client::MemSpell,
					   (void(Lua_Client::*)(int, int, bool))&Lua_Client::MemSpell);
	client["MemmedCount"] = (int(Lua_Client::*)(void))&Lua_Client::MemmedCount;
	client["MovePC"] = (void(Lua_Client::*)(int,float,float,float,float))&Lua_Client::MovePC;
	client["MovePCDynamicZone"] =
	    sol::overload((void(Lua_Client::*)(std::string))&Lua_Client::MovePCDynamicZone,
			  (void(Lua_Client::*)(std::string, int))&Lua_Client::MovePCDynamicZone,
			  (void(Lua_Client::*)(std::string, int, bool))&Lua_Client::MovePCDynamicZone,
			  (void(Lua_Client::*)(uint32))&Lua_Client::MovePCDynamicZone,
			  (void(Lua_Client::*)(uint32, int))&Lua_Client::MovePCDynamicZone,
			  (void(Lua_Client::*)(uint32, int, bool))&Lua_Client::MovePCDynamicZone);
	client["MovePCInstance"] = (void(Lua_Client::*)(int,int,float,float,float,float))&Lua_Client::MovePCInstance;
	client["MoveZone"] = (void(Lua_Client::*)(const char*))&Lua_Client::MoveZone;
	client["MoveZoneGroup"] = (void(Lua_Client::*)(const char*))&Lua_Client::MoveZoneGroup;
	client["MoveZoneInstance"] = (void(Lua_Client::*)(uint16))&Lua_Client::MoveZoneInstance;
	client["MoveZoneInstanceGroup"] = (void(Lua_Client::*)(uint16))&Lua_Client::MoveZoneInstanceGroup;
	client["MoveZoneInstanceRaid"] = (void(Lua_Client::*)(uint16))&Lua_Client::MoveZoneInstanceRaid;
	client["MoveZoneRaid"] = (void(Lua_Client::*)(const char*))&Lua_Client::MoveZoneRaid;
	client["NotifyNewTitlesAvailable"] = (void(Lua_Client::*)(void))&Lua_Client::NotifyNewTitlesAvailable;
	client["NukeItem"] = sol::overload((void(Lua_Client::*)(uint32))&Lua_Client::NukeItem,
					   (void(Lua_Client::*)(uint32, int))&Lua_Client::NukeItem);
	client["OpenLFGuildWindow"] = (void(Lua_Client::*)(void))&Lua_Client::OpenLFGuildWindow;
	client["PlayMP3"] = (void(Lua_Client::*)(std::string))&Lua_Client::PlayMP3;
	client["Popup"] = sol::overload(
	    (void(Lua_Client::*)(const char *, const char *))&Lua_Client::Popup,
	    (void(Lua_Client::*)(const char *, const char *, uint32))&Lua_Client::Popup,
	    (void(Lua_Client::*)(const char *, const char *, uint32, uint32))&Lua_Client::Popup,
	    (void(Lua_Client::*)(const char *, const char *, uint32, uint32, uint32))&Lua_Client::Popup,
	    (void(Lua_Client::*)(const char *, const char *, uint32, uint32, uint32, uint32))&Lua_Client::Popup,
	    (void(Lua_Client::*)(const char *, const char *, uint32, uint32, uint32, uint32, const char *,
				 const char *))&Lua_Client::Popup,
	    (void(Lua_Client::*)(const char *, const char *, uint32, uint32, uint32, uint32, const char *, const char *,
				 uint32))&Lua_Client::Popup);
	client["PushItemOnCursor"] = (bool(Lua_Client::*)(Lua_ItemInst))&Lua_Client::PushItemOnCursor;
	client["PutItemInInventory"] = (bool(Lua_Client::*)(int,Lua_ItemInst))&Lua_Client::PutItemInInventory;
	client["QuestReadBook"] = (void(Lua_Client::*)(const char *,int))&Lua_Client::QuestReadBook;
	client["QuestReward"] = sol::overload(
	    (void(Lua_Client::*)(Lua_Mob))&Lua_Client::QuestReward,
	    (void(Lua_Client::*)(Lua_Mob, sol::table))&Lua_Client::QuestReward,
	    (void(Lua_Client::*)(Lua_Mob, uint32))&Lua_Client::QuestReward,
	    (void(Lua_Client::*)(Lua_Mob, uint32, uint32))&Lua_Client::QuestReward,
	    (void(Lua_Client::*)(Lua_Mob, uint32, uint32, uint32))&Lua_Client::QuestReward,
	    (void(Lua_Client::*)(Lua_Mob, uint32, uint32, uint32, uint32))&Lua_Client::QuestReward,
	    (void(Lua_Client::*)(Lua_Mob, uint32, uint32, uint32, uint32, uint32))&Lua_Client::QuestReward,
	    (void(Lua_Client::*)(Lua_Mob, uint32, uint32, uint32, uint32, uint32, uint32))&Lua_Client::QuestReward,
	    (void(Lua_Client::*)(Lua_Mob, uint32, uint32, uint32, uint32, uint32, uint32, bool))&Lua_Client::QuestReward);
	client["QueuePacket"] =
	    sol::overload((void(Lua_Client::*)(Lua_Packet))&Lua_Client::QueuePacket,
			  (void(Lua_Client::*)(Lua_Packet, bool))&Lua_Client::QueuePacket,
			  (void(Lua_Client::*)(Lua_Packet, bool, int))&Lua_Client::QueuePacket,
			  (void(Lua_Client::*)(Lua_Packet, bool, int, int))&Lua_Client::QueuePacket);
	client["ReadBookByName"] = (void(Lua_Client::*)(std::string,uint8))&Lua_Client::ReadBookByName;
	client["RefundAA"] = (void(Lua_Client::*)(void))&Lua_Client::RefundAA;
	client["RemoveAllExpeditionLockouts"] =
	    sol::overload((void(Lua_Client::*)(std::string))&Lua_Client::RemoveAllExpeditionLockouts,
			  (void(Lua_Client::*)(void))&Lua_Client::RemoveAllExpeditionLockouts);
	client["RemoveExpeditionLockout"] = (void(Lua_Client::*)(std::string, std::string))&Lua_Client::RemoveExpeditionLockout;
	client["RemoveItem"] = sol::overload((void(Lua_Client::*)(uint32))&Lua_Client::RemoveItem,
					     (void(Lua_Client::*)(uint32, uint32))&Lua_Client::RemoveItem);
	client["RemoveLDoNLoss"] = (void(Lua_Client::*)(uint32))&Lua_Client::RemoveLDoNLoss;
	client["RemoveLDoNWin"] = (void(Lua_Client::*)(uint32))&Lua_Client::RemoveLDoNWin;
	client["ResetAA"] = (void(Lua_Client::*)(void))&Lua_Client::ResetAA;
	client["ResetAllDisciplineTimers"] = (void(Lua_Client::*)(void))&Lua_Client::ResetAllDisciplineTimers;
	client["ResetAllCastbarCooldowns"] = (void(Lua_Client::*)(void))&Lua_Client::ResetAllCastbarCooldowns;
	client["ResetCastbarCooldownBySlot"] = (void(Lua_Client::*)(int))&Lua_Client::ResetCastbarCooldownBySlot;
	client["ResetCastbarCooldownBySpellID"] = (void(Lua_Client::*)(uint32))&Lua_Client::ResetCastbarCooldownBySpellID;
	client["ResetDisciplineTimer"] = (void(Lua_Client::*)(uint32))&Lua_Client::ResetDisciplineTimer;
	client["ResetTrade"] = (void(Lua_Client::*)(void))&Lua_Client::ResetTrade;
	client["RewardFaction"] = (void(Lua_Client::*)(int,int))&Lua_Client::RewardFaction;
	client["Save"] = sol::overload((void(Lua_Client::*)(int))&Lua_Client::Save,
				       (void(Lua_Client::*)(void))&Lua_Client::Save);
	client["SaveBackup"] = (void(Lua_Client::*)(void))&Lua_Client::SaveBackup;
	client["ScribeSpell"] = sol::overload((void(Lua_Client::*)(int, int))&Lua_Client::ScribeSpell,
					      (void(Lua_Client::*)(int, int, bool))&Lua_Client::ScribeSpell);
	client["ScribeSpells"] = (uint16(Lua_Client::*)(uint8,uint8))&Lua_Client::ScribeSpells;
	client["SendColoredText"] = (void(Lua_Client::*)(uint32, std::string))&Lua_Client::SendColoredText;
	client["SendItemScale"] = (void(Lua_Client::*)(Lua_ItemInst))&Lua_Client::SendItemScale;
	client["SendMarqueeMessage"] = (void(Lua_Client::*)(uint32, uint32, uint32, uint32, uint32, std::string))&Lua_Client::SendMarqueeMessage;
	client["SendOPTranslocateConfirm"] = (void(Lua_Client::*)(Lua_Mob,int))&Lua_Client::SendOPTranslocateConfirm;
	client["SendPEQZoneFlagInfo"] = (void(Lua_Client::*)(Lua_Client))&Lua_Client::SendPEQZoneFlagInfo;
	client["SendSound"] = (void(Lua_Client::*)(void))&Lua_Client::SendSound;
	client["SendToGuildHall"] = (void(Lua_Client::*)(void))&Lua_Client::SendToGuildHall;
	client["SendToInstance"] = (void(Lua_Client::*)(std::string,std::string,uint32,float,float,float,float,std::string,uint32))&Lua_Client::SendToInstance;
	client["SendWebLink"] = (void(Lua_Client::*)(const char *))&Lua_Client::SendWebLink;
	client["SendZoneFlagInfo"] = (void(Lua_Client::*)(Lua_Client))&Lua_Client::SendZoneFlagInfo;
	client["SetAAEXPModifier"] =
	    sol::overload((void(Lua_Client::*)(uint32, double))&Lua_Client::SetAAEXPModifier,
			  (void(Lua_Client::*)(uint32, double, int16))&Lua_Client::SetAAEXPModifier);
	client["SetAAPoints"] = (void(Lua_Client::*)(int))&Lua_Client::SetAAPoints;
	client["SetAATitle"] = sol::overload((void(Lua_Client::*)(std::string))&Lua_Client::SetAATitle,
					     (void(Lua_Client::*)(std::string, bool))&Lua_Client::SetAATitle);
	client["SetAFK"] = (void(Lua_Client::*)(uint8))&Lua_Client::SetAFK;
	client["SetAccountFlag"] =
	    sol::overload((void(Lua_Client::*)(std::string, std::string))&Lua_Client::SetAccountFlag,
			  (void(Lua_Client::*)(std::string, std::string))&Lua_Client::SetAccountFlag);
	client["SetAlternateCurrencyValue"] = (void(Lua_Client::*)(uint32,int))&Lua_Client::SetAlternateCurrencyValue;
	client["SetAnon"] = (void(Lua_Client::*)(uint8))&Lua_Client::SetAnon;
	client["SetBaseClass"] = (void(Lua_Client::*)(int))&Lua_Client::SetBaseClass;
	client["SetBaseGender"] = (void(Lua_Client::*)(int))&Lua_Client::SetBaseGender;
	client["SetBaseRace"] = (void(Lua_Client::*)(int))&Lua_Client::SetBaseRace;
	client["SetBindPoint"] =
	    sol::overload((void(Lua_Client::*)(int))&Lua_Client::SetBindPoint,
			  (void(Lua_Client::*)(int, int))&Lua_Client::SetBindPoint,
			  (void(Lua_Client::*)(int, int, float))&Lua_Client::SetBindPoint,
			  (void(Lua_Client::*)(int, int, float, float))&Lua_Client::SetBindPoint,
			  (void(Lua_Client::*)(int, int, float, float, float))&Lua_Client::SetBindPoint,
			  (void(Lua_Client::*)(int, int, float, float, float, float))&Lua_Client::SetBindPoint,
			  (void(Lua_Client::*)(void)) & Lua_Client::SetBindPoint);
	client["SetClientMaxLevel"] = (void(Lua_Client::*)(int))&Lua_Client::SetClientMaxLevel;
	client["SetConsumption"] = (void(Lua_Client::*)(int, int))&Lua_Client::SetConsumption;
	client["SetDeity"] = (void(Lua_Client::*)(int))&Lua_Client::SetDeity;
	client["SetDuelTarget"] = (void(Lua_Client::*)(int))&Lua_Client::SetDuelTarget;
	client["SetDueling"] = (void(Lua_Client::*)(bool))&Lua_Client::SetDueling;
	client["SetEXP"] = sol::overload((void(Lua_Client::*)(uint32, uint32))&Lua_Client::SetEXP,
					 (void(Lua_Client::*)(uint32, uint32, bool))&Lua_Client::SetEXP);
	client["SetEXPModifier"] =
	    sol::overload((void(Lua_Client::*)(uint32, double))&Lua_Client::SetEXPModifier,
			  (void(Lua_Client::*)(uint32, double, int16))&Lua_Client::SetEXPModifier);
	client["SetEbonCrystals"] = (void(Lua_Client::*)(uint32))&Lua_Client::SetEbonCrystals;
	client["SetEndurance"] = (void(Lua_Client::*)(int))&Lua_Client::SetEndurance;
	client["SetEnvironmentDamageModifier"] = (void(Lua_Client::*)(int))&Lua_Client::SetEnvironmentDamageModifier;
	client["SetFactionLevel"] = (void(Lua_Client::*)(uint32,uint32,int,int,int))&Lua_Client::SetFactionLevel;
	client["SetFactionLevel2"] = (void(Lua_Client::*)(uint32,int,int,int,int,int,int))&Lua_Client::SetFactionLevel2;
	client["SetFeigned"] = (void(Lua_Client::*)(bool))&Lua_Client::SetFeigned;
	client["SetGM"] = (void(Lua_Client::*)(bool))&Lua_Client::SetGM;
	client["SetGMStatus"] = (void(Lua_Client::*)(int16))&Lua_Client::SetGMStatus;
	client["SetHideMe"] = (void(Lua_Client::*)(bool))&Lua_Client::SetHideMe;
	client["SetHorseId"] = (void(Lua_Client::*)(int))&Lua_Client::SetHorseId;
	client["SetHunger"] = (void(Lua_Client::*)(int))&Lua_Client::SetHunger;
	client["SetInvulnerableEnvironmentDamage"] = (void(Lua_Client::*)(int))&Lua_Client::SetInvulnerableEnvironmentDamage;
	client["SetIPExemption"] = (void(Lua_Client::*)(int))&Lua_Client::SetIPExemption;
	client["SetLanguageSkill"] = (void(Lua_Client::*)(int,int))&Lua_Client::SetLanguageSkill;
	client["SetMaterial"] = (void(Lua_Client::*)(int,uint32))&Lua_Client::SetMaterial;
	client["SetPEQZoneFlag"] = (void(Lua_Client::*)(uint32))&Lua_Client::SetPEQZoneFlag;
	client["SetPVP"] = (void(Lua_Client::*)(bool))&Lua_Client::SetPVP;
	client["SetPrimaryWeaponOrnamentation"] = (void(Lua_Client::*)(uint32))&Lua_Client::SetPrimaryWeaponOrnamentation;
	client["SetRadiantCrystals"] = (void(Lua_Client::*)(uint32))&Lua_Client::SetRadiantCrystals;
	client["SetSecondaryWeaponOrnamentation"] = (void(Lua_Client::*)(uint32))&Lua_Client::SetSecondaryWeaponOrnamentation;
	client["SetSkill"] = (void(Lua_Client::*)(int,int))&Lua_Client::SetSkill;
	client["SetSkillPoints"] = (void(Lua_Client::*)(int))&Lua_Client::SetSkillPoints;
	client["SetStartZone"] =
	    sol::overload((void(Lua_Client::*)(int))&Lua_Client::SetStartZone,
			  (void(Lua_Client::*)(int, float))&Lua_Client::SetStartZone,
			  (void(Lua_Client::*)(int, float, float))&Lua_Client::SetStartZone,
			  (void(Lua_Client::*)(int, float, float, float))&Lua_Client::SetStartZone);
	client["SetStats"] = (void(Lua_Client::*)(int,int))&Lua_Client::SetStats;
	client["SetThirst"] = (void(Lua_Client::*)(int))&Lua_Client::SetThirst;
	client["SetTint"] = (void(Lua_Client::*)(int,uint32))&Lua_Client::SetTint;
	client["SetTitleSuffix"] = (void(Lua_Client::*)(const char *))&Lua_Client::SetTitleSuffix;
	client["SetZoneFlag"] = (void(Lua_Client::*)(uint32))&Lua_Client::SetZoneFlag;
	client["Signal"] = (void(Lua_Client::*)(uint32))&Lua_Client::Signal;
	client["Sit"] = (void(Lua_Client::*)(void))&Lua_Client::Sit;
	client["Stand"] = (void(Lua_Client::*)(void))&Lua_Client::Stand;
	client["SummonBaggedItems"] = (void(Lua_Client::*)(uint32,sol::table))&Lua_Client::SummonBaggedItems;
	client["SummonItem"] = sol::overload(
	    (void(Lua_Client::*)(uint32))&Lua_Client::SummonItem,
	    (void(Lua_Client::*)(uint32, int))&Lua_Client::SummonItem,
	    (void(Lua_Client::*)(uint32, int, uint32))&Lua_Client::SummonItem,
	    (void(Lua_Client::*)(uint32, int, uint32, uint32))&Lua_Client::SummonItem,
	    (void(Lua_Client::*)(uint32, int, uint32, uint32, uint32))&Lua_Client::SummonItem,
	    (void(Lua_Client::*)(uint32, int, uint32, uint32, uint32, uint32))&Lua_Client::SummonItem,
	    (void(Lua_Client::*)(uint32, int, uint32, uint32, uint32, uint32, uint32))&Lua_Client::SummonItem,
	    (void(Lua_Client::*)(uint32, int, uint32, uint32, uint32, uint32, uint32, bool))&Lua_Client::SummonItem,
	    (void(Lua_Client::*)(uint32, int, uint32, uint32, uint32, uint32, uint32, bool, int))&Lua_Client::SummonItem);
	client["TGB"] = (bool(Lua_Client::*)(void))&Lua_Client::TGB;
	client["TakeMoneyFromPP"] = sol::overload((bool(Lua_Client::*)(uint64))&Lua_Client::TakeMoneyFromPP,
						  (bool(Lua_Client::*)(uint64, bool))&Lua_Client::TakeMoneyFromPP);
	client["TakePlatinum"] = sol::overload((bool(Lua_Client::*)(uint32))&Lua_Client::TakePlatinum,
					       (bool(Lua_Client::*)(uint32, bool))&Lua_Client::TakePlatinum);
	client["TaskSelector"] = sol::overload((void(Lua_Client::*)(sol::table))&Lua_Client::TaskSelector,
					       (void(Lua_Client::*)(sol::table, bool))&Lua_Client::TaskSelector);
	client["TeleportToPlayerByCharID"] = (bool(Lua_Client::*)(uint32))&Lua_Client::TeleportToPlayerByCharID;
	client["TeleportToPlayerByName"] = (bool(Lua_Client::*)(std::string))&Lua_Client::TeleportToPlayerByName;
	client["TeleportGroupToPlayerByCharID"] = (bool(Lua_Client::*)(uint32))&Lua_Client::TeleportGroupToPlayerByCharID;
	client["TeleportGroupToPlayerByName"] = (bool(Lua_Client::*)(std::string))&Lua_Client::TeleportGroupToPlayerByName;
	client["TeleportRaidToPlayerByCharID"] = (bool(Lua_Client::*)(uint32))&Lua_Client::TeleportRaidToPlayerByCharID;
	client["TeleportRaidToPlayerByName"] = (bool(Lua_Client::*)(std::string))&Lua_Client::TeleportRaidToPlayerByName;
	client["Thirsty"] = (bool(Lua_Client::*)(void))&Lua_Client::Thirsty;
	client["TrainDisc"] = (void(Lua_Client::*)(int))&Lua_Client::TrainDisc;
	client["TrainDiscBySpellID"] = (void(Lua_Client::*)(int32))&Lua_Client::TrainDiscBySpellID;
	client["UnFreeze"] = (void(Lua_Client::*)(void))&Lua_Client::UnFreeze;
	client["Undye"] = (void(Lua_Client::*)(void))&Lua_Client::Undye;
	client["UnmemSpell"] = sol::overload((void(Lua_Client::*)(int))&Lua_Client::UnmemSpell,
					     (void(Lua_Client::*)(int, bool))&Lua_Client::UnmemSpell);
	client["UnmemSpellAll"] = sol::overload((void(Lua_Client::*)(bool))&Lua_Client::UnmemSpellAll,
						(void(Lua_Client::*)(void))&Lua_Client::UnmemSpellAll);
	client["UnmemSpellBySpellID"] = (void(Lua_Client::*)(int32))&Lua_Client::UnmemSpellBySpellID;
	client["UnscribeSpell"] = sol::overload((void(Lua_Client::*)(int))&Lua_Client::UnscribeSpell,
						(void(Lua_Client::*)(int, bool))&Lua_Client::UnscribeSpell);
	client["UnscribeSpellAll"] = sol::overload((void(Lua_Client::*)(bool))&Lua_Client::UnscribeSpellAll,
						   (void(Lua_Client::*)(void))&Lua_Client::UnscribeSpellAll);
	client["UnscribeSpellBySpellID"] =
	    sol::overload((void(Lua_Client::*)(uint16))&Lua_Client::UnscribeSpellBySpellID,
			  (void(Lua_Client::*)(uint16, bool))&Lua_Client::UnscribeSpellBySpellID);
	client["UntrainDisc"] = sol::overload((void(Lua_Client::*)(int))&Lua_Client::UntrainDisc,
					      (void(Lua_Client::*)(int, bool))&Lua_Client::UntrainDisc);
	client["UntrainDiscAll"] = sol::overload((void(Lua_Client::*)(bool))&Lua_Client::UntrainDiscAll,
						 (void(Lua_Client::*)(void))&Lua_Client::UntrainDiscAll);
	client["UntrainDiscBySpellID"] =
	    sol::overload((void(Lua_Client::*)(uint16))&Lua_Client::UntrainDiscBySpellID,
			  (void(Lua_Client::*)(uint16, bool))&Lua_Client::UntrainDiscBySpellID);
	client["UpdateGroupAAs"] = (void(Lua_Client::*)(int,uint32))&Lua_Client::UpdateGroupAAs;
	client["UpdateLDoNPoints"] = (void(Lua_Client::*)(uint32,int))&Lua_Client::UpdateLDoNPoints;
	client["UpdateTaskActivity"] = (void(Lua_Client::*)(int,int,int))&Lua_Client::UpdateTaskActivity;
	client["UseDiscipline"] = (bool(Lua_Client::*)(int,int))&Lua_Client::UseDiscipline;
	client["WorldKick"] = (void(Lua_Client::*)(void))&Lua_Client::WorldKick;
}

void lua_register_inventory_where(sol::state_view &sv)
{
	sv.new_enum("InventoryWhere", "Personal", invWherePersonal, "Bank", invWhereBank, "SharedBank",
		    invWhereSharedBank, "Trading", invWhereTrading, "Cursor", invWhereCursor);
}
#endif
