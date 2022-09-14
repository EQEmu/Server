#ifdef LUA_EQEMU

#include <sol/sol.hpp>

#include "client.h"
#include "npc.h"
#ifdef BOTS
#include "lua_bot.h"
#endif
#include "lua_item.h"
#include "lua_iteminst.h"
#include "lua_mob.h"
#include "lua_npc.h"
#include "lua_hate_list.h"
#include "lua_client.h"
#include "lua_stat_bonuses.h"
#include "dialogue_window.h"

void lua_register_mob(sol::state_view &sv)
{
	auto mob = sv.new_usertype<Lua_Mob>("Mob", sol::constructors<Lua_Mob>(), sol::base_classes, sol::bases<Lua_Entity>());
	mob["AddNimbusEffect"] = (void(Lua_Mob::*)(int))&Lua_Mob::AddNimbusEffect;
	mob["AddToHateList"] =
	    sol::overload((void(Lua_Mob::*)(Lua_Mob)) & Lua_Mob::AddToHateList,
			  (void(Lua_Mob::*)(Lua_Mob, int64)) & Lua_Mob::AddToHateList,
			  (void(Lua_Mob::*)(Lua_Mob, int64, int64)) & Lua_Mob::AddToHateList,
			  (void(Lua_Mob::*)(Lua_Mob, int64, int64, bool)) & Lua_Mob::AddToHateList,
			  (void(Lua_Mob::*)(Lua_Mob, int64, int64, bool, bool)) & Lua_Mob::AddToHateList,
			  (void(Lua_Mob::*)(Lua_Mob, int64, int64, bool, bool, bool)) & Lua_Mob::AddToHateList);
	mob["ApplySpellBuff"] = sol::overload((void(Lua_Mob::*)(int)) & Lua_Mob::ApplySpellBuff,
					      (void(Lua_Mob::*)(int, int)) & Lua_Mob::ApplySpellBuff);
	mob["Attack"] = sol::overload((bool(Lua_Mob::*)(Lua_Mob)) & Lua_Mob::Attack,
				      (bool(Lua_Mob::*)(Lua_Mob, int)) & Lua_Mob::Attack,
				      (bool(Lua_Mob::*)(Lua_Mob, int, bool)) & Lua_Mob::Attack,
				      (bool(Lua_Mob::*)(Lua_Mob, int, bool, bool)) & Lua_Mob::Attack,
				      (bool(Lua_Mob::*)(Lua_Mob, int, bool, bool, bool)) & Lua_Mob::Attack,
				      (bool(Lua_Mob::*)(Lua_Mob, int, bool, bool, bool, sol::table)) & Lua_Mob::Attack);
	mob["AttackAnimation"] = &Lua_Mob::AttackAnimation;
	mob["BehindMob"] = sol::overload((bool(Lua_Mob::*)(Lua_Mob)) & Lua_Mob::BehindMob,
					 (bool(Lua_Mob::*)(Lua_Mob, float)) & Lua_Mob::BehindMob,
					 (bool(Lua_Mob::*)(Lua_Mob, float, float)) & Lua_Mob::BehindMob,
					 (bool(Lua_Mob::*)(void)) & Lua_Mob::BehindMob);
	mob["BuffCount"] = &Lua_Mob::BuffCount;
	mob["BuffFadeAll"] = (void(Lua_Mob::*)(void))&Lua_Mob::BuffFadeAll;
	mob["BuffFadeByEffect"] = sol::overload((void(Lua_Mob::*)(int)) & Lua_Mob::BuffFadeByEffect,
						(void(Lua_Mob::*)(int, int)) & Lua_Mob::BuffFadeByEffect);
	mob["BuffFadeBySlot"] = sol::overload((void(Lua_Mob::*)(int)) & Lua_Mob::BuffFadeBySlot,
					      (void(Lua_Mob::*)(int, bool)) & Lua_Mob::BuffFadeBySlot);
	mob["BuffFadeBySpellID"] = (void(Lua_Mob::*)(int))&Lua_Mob::BuffFadeBySpellID;
	mob["CalculateDistance"] = (float(Lua_Mob::*)(double,double,double))&Lua_Mob::CalculateDistance;
	mob["CalculateHeadingToTarget"] = (double(Lua_Mob::*)(double,double))&Lua_Mob::CalculateHeadingToTarget;
	mob["CameraEffect"] = sol::overload((void(Lua_Mob::*)(uint32, float)) & Lua_Mob::CameraEffect,
					    (void(Lua_Mob::*)(uint32, float, Lua_Client)) & Lua_Mob::CameraEffect,
					    (void(Lua_Mob::*)(uint32, float, Lua_Client, bool)) & Lua_Mob::CameraEffect,
					    (int(Lua_Mob::*)(int, int)) & Lua_Mob::CanBuffStack,
					    (int(Lua_Mob::*)(int, int, bool)) & Lua_Mob::CanBuffStack);
	mob["CanClassEquipItem"] = &Lua_Mob::CanClassEquipItem;
	mob["CanRaceEquipItem"] = &Lua_Mob::CanRaceEquipItem;
	mob["CanThisClassBlock"] = (bool(Lua_Mob::*)(void))&Lua_Mob::CanThisClassBlock;
	mob["CanThisClassDodge"] = (bool(Lua_Mob::*)(void))&Lua_Mob::CanThisClassDodge;
	mob["CanThisClassDoubleAttack"] = (bool(Lua_Mob::*)(void))&Lua_Mob::CanThisClassDoubleAttack;
	mob["CanThisClassDualWield"] = (bool(Lua_Mob::*)(void))&Lua_Mob::CanThisClassDualWield;
	mob["CanThisClassParry"] = (bool(Lua_Mob::*)(void))&Lua_Mob::CanThisClassParry;
	mob["CanThisClassRiposte"] = (bool(Lua_Mob::*)(void))&Lua_Mob::CanThisClassRiposte;
	mob["CastSpell"] = sol::overload(
	    (bool(Lua_Mob::*)(int, int)) & Lua_Mob::CastSpell, (bool(Lua_Mob::*)(int, int, int)) & Lua_Mob::CastSpell,
	    (bool(Lua_Mob::*)(int, int, int, int)) & Lua_Mob::CastSpell,
	    (bool(Lua_Mob::*)(int, int, int, int, int)) & Lua_Mob::CastSpell,
	    (bool(Lua_Mob::*)(int, int, int, int, int, int)) & Lua_Mob::CastSpell,
	    (bool(Lua_Mob::*)(int, int, int, int, int, int, int, int)) & Lua_Mob::CastSpell,
	    (bool(Lua_Mob::*)(int, int, int, int, int, int, int, int, int)) & Lua_Mob::CastSpell);
	mob["ChangeBeard"] = (void(Lua_Mob::*)(int))&Lua_Mob::ChangeBeard;
	mob["ChangeBeardColor"] = (void(Lua_Mob::*)(int))&Lua_Mob::ChangeBeardColor;
	mob["ChangeDrakkinDetails"] = (void(Lua_Mob::*)(int))&Lua_Mob::ChangeDrakkinDetails;
	mob["ChangeDrakkinHeritage"] = (void(Lua_Mob::*)(int))&Lua_Mob::ChangeDrakkinHeritage;
	mob["ChangeDrakkinTattoo"] = (void(Lua_Mob::*)(int))&Lua_Mob::ChangeDrakkinTattoo;
	mob["ChangeEyeColor1"] = (void(Lua_Mob::*)(int))&Lua_Mob::ChangeEyeColor1;
	mob["ChangeEyeColor2"] = (void(Lua_Mob::*)(int))&Lua_Mob::ChangeEyeColor2;
	mob["ChangeGender"] = (void(Lua_Mob::*)(int))&Lua_Mob::ChangeGender;
	mob["ChangeHairColor"] = (void(Lua_Mob::*)(int))&Lua_Mob::ChangeHairColor;
	mob["ChangeHairStyle"] = (void(Lua_Mob::*)(int))&Lua_Mob::ChangeHairStyle;
	mob["ChangeHelmTexture"] = (void(Lua_Mob::*)(int))&Lua_Mob::ChangeHelmTexture;
	mob["ChangeLuclinFace"] = (void(Lua_Mob::*)(int))&Lua_Mob::ChangeLuclinFace;
	mob["ChangeRace"] = (void(Lua_Mob::*)(int))&Lua_Mob::ChangeRace;
	mob["ChangeSize"] = sol::overload((void(Lua_Mob::*)(double)) & Lua_Mob::ChangeSize,
					  (void(Lua_Mob::*)(double, bool)) & Lua_Mob::ChangeSize);
	mob["ChangeTexture"] = (void(Lua_Mob::*)(int))&Lua_Mob::ChangeTexture;
	mob["Charmed"] = (bool(Lua_Mob::*)(void))&Lua_Mob::Charmed;
	mob["CheckAggro"] = (bool(Lua_Mob::*)(Lua_Mob))&Lua_Mob::CheckAggro;
	mob["CheckAggroAmount"] = sol::overload((int(Lua_Mob::*)(int)) & Lua_Mob::CheckAggroAmount,
						(int(Lua_Mob::*)(int, bool)) & Lua_Mob::CheckAggroAmount);
	mob["CheckHealAggroAmount"] = sol::overload((int(Lua_Mob::*)(int)) & Lua_Mob::CheckHealAggroAmount,
						    (int(Lua_Mob::*)(int, uint32)) & Lua_Mob::CheckHealAggroAmount);
	mob["CheckLoS"] = (bool(Lua_Mob::*)(Lua_Mob))&Lua_Mob::CheckLoS;
	mob["CheckLoSToLoc"] =
	    sol::overload((bool(Lua_Mob::*)(double, double, double)) & Lua_Mob::CheckLoSToLoc,
			  (bool(Lua_Mob::*)(double, double, double, double)) & Lua_Mob::CheckLoSToLoc);
	mob["CheckNumHitsRemaining"] = &Lua_Mob::CheckNumHitsRemaining;
	mob["ClearSpecialAbilities"] = (void(Lua_Mob::*)(void))&Lua_Mob::ClearSpecialAbilities;
	mob["CombatRange"] = (bool(Lua_Mob::*)(Lua_Mob))&Lua_Mob::CombatRange;
	mob["Damage"] = sol::overload((void(Lua_Mob::*)(Lua_Mob, int64, int, int)) & Lua_Mob::Damage,
				      (void(Lua_Mob::*)(Lua_Mob, int64, int, int, bool)) & Lua_Mob::Damage,
				      (void(Lua_Mob::*)(Lua_Mob, int64, int, int, bool, int)) & Lua_Mob::Damage,
				      (void(Lua_Mob::*)(Lua_Mob, int64, int, int, bool, int, bool)) & Lua_Mob::Damage);
	mob["DelGlobal"] = (void(Lua_Mob::*)(const char*))&Lua_Mob::DelGlobal;
	mob["DeleteBucket"] = (void(Lua_Mob::*)(std::string))&Lua_Mob::DeleteBucket;
	mob["Depop"] =
	    sol::overload((void(Lua_Mob::*)(bool)) & Lua_Mob::Depop, (void(Lua_Mob::*)(void)) & Lua_Mob::Depop);
	mob["DivineAura"] = (bool(Lua_Mob::*)(void))&Lua_Mob::DivineAura;
	mob["DoAnim"] =
	    sol::overload((void(Lua_Mob::*)(int)) & Lua_Mob::DoAnim, (void(Lua_Mob::*)(int, int)) & Lua_Mob::DoAnim,
			  (void(Lua_Mob::*)(int, int, bool)) & Lua_Mob::DoAnim,
			  (void(Lua_Mob::*)(int, int, bool, int)) & Lua_Mob::DoAnim);
	mob["DoArcheryAttackDmg"] = sol::overload(
	    (void(Lua_Mob::*)(Lua_Mob)) & Lua_Mob::DoArcheryAttackDmg,
	    (void(Lua_Mob::*)(Lua_Mob, Lua_ItemInst)) & Lua_Mob::DoArcheryAttackDmg,
	    (void(Lua_Mob::*)(Lua_Mob, Lua_ItemInst, Lua_ItemInst)) & Lua_Mob::DoArcheryAttackDmg,
	    (void(Lua_Mob::*)(Lua_Mob, Lua_ItemInst, Lua_ItemInst, int)) & Lua_Mob::DoArcheryAttackDmg,
	    (void(Lua_Mob::*)(Lua_Mob, Lua_ItemInst, Lua_ItemInst, int, int)) & Lua_Mob::DoArcheryAttackDmg,
	    (void(Lua_Mob::*)(Lua_Mob, Lua_ItemInst, Lua_ItemInst, int, int, int)) & Lua_Mob::DoArcheryAttackDmg);
	mob["DoKnockback"] = (void(Lua_Mob::*)(Lua_Mob,uint32,uint32))&Lua_Mob::DoKnockback;
	mob["DoMeleeSkillAttackDmg"] =
	    sol::overload((void(Lua_Mob::*)(Lua_Mob, int, int)) & Lua_Mob::DoMeleeSkillAttackDmg,
			  (void(Lua_Mob::*)(Lua_Mob, int, int, int)) & Lua_Mob::DoMeleeSkillAttackDmg,
			  (void(Lua_Mob::*)(Lua_Mob, int, int, int, int)) & Lua_Mob::DoMeleeSkillAttackDmg,
			  (void(Lua_Mob::*)(Lua_Mob, int, int, int, int, bool)) & Lua_Mob::DoMeleeSkillAttackDmg);
	mob["DoSpecialAttackDamage"] =
	    sol::overload((void(Lua_Mob::*)(Lua_Mob, int, int)) & Lua_Mob::DoSpecialAttackDamage,
			  (void(Lua_Mob::*)(Lua_Mob, int, int, int)) & Lua_Mob::DoSpecialAttackDamage,
			  (void(Lua_Mob::*)(Lua_Mob, int, int, int, int)) & Lua_Mob::DoSpecialAttackDamage,
			  (void(Lua_Mob::*)(Lua_Mob, int, int, int, int, int)) & Lua_Mob::DoSpecialAttackDamage);
	mob["DoThrowingAttackDmg"] = sol::overload(
	    (void(Lua_Mob::*)(Lua_Mob)) & Lua_Mob::DoThrowingAttackDmg,
	    (void(Lua_Mob::*)(Lua_Mob, Lua_ItemInst)) & Lua_Mob::DoThrowingAttackDmg,
	    (void(Lua_Mob::*)(Lua_Mob, Lua_ItemInst, Lua_Item)) & Lua_Mob::DoThrowingAttackDmg,
	    (void(Lua_Mob::*)(Lua_Mob, Lua_ItemInst, Lua_Item, int)) & Lua_Mob::DoThrowingAttackDmg,
	    (void(Lua_Mob::*)(Lua_Mob, Lua_ItemInst, Lua_Item, int, int)) & Lua_Mob::DoThrowingAttackDmg,
	    (void(Lua_Mob::*)(Lua_Mob, Lua_ItemInst, Lua_Item, int, int, int)) & Lua_Mob::DoThrowingAttackDmg);
	mob["DoubleAggro"] = &Lua_Mob::DoubleAggro;
	mob["Emote"] = &Lua_Mob::Emote;
	mob["EntityVariableExists"] = (bool(Lua_Mob::*)(const char*))&Lua_Mob::EntityVariableExists;
	mob["FaceTarget"] = (void(Lua_Mob::*)(Lua_Mob))&Lua_Mob::FaceTarget;
	mob["FindBuff"] = &Lua_Mob::FindBuff;
	mob["FindBuffBySlot"] = (uint16(Lua_Mob::*)(int))&Lua_Mob::FindBuffBySlot;
	mob["FindGroundZ"] = sol::overload((double(Lua_Mob::*)(double, double)) & Lua_Mob::FindGroundZ,
					   (double(Lua_Mob::*)(double, double, double)) & Lua_Mob::FindGroundZ);
	mob["FindType"] = sol::overload((bool(Lua_Mob::*)(int)) & Lua_Mob::FindType,
					(bool(Lua_Mob::*)(int, bool)) & Lua_Mob::FindType,
					(bool(Lua_Mob::*)(int, bool, int)) & Lua_Mob::FindType);
	mob["GMMove"] = sol::overload((void(Lua_Mob::*)(double, double, double)) & Lua_Mob::GMMove,
				      (void(Lua_Mob::*)(double, double, double, double)) & Lua_Mob::GMMove);
	mob["GetAA"] = (int(Lua_Mob::*)(int))&Lua_Mob::GetAA;
	mob["GetAABonuses"] = &Lua_Mob::GetAABonuses;
	mob["GetAAByAAID"] = (int(Lua_Mob::*)(int))&Lua_Mob::GetAAByAAID;
	mob["GetAC"] = &Lua_Mob::GetAC;
	mob["GetAGI"] = &Lua_Mob::GetAGI;
	mob["GetATK"] = &Lua_Mob::GetATK;
	mob["GetAggroRange"] = (float(Lua_Mob::*)(void))&Lua_Mob::GetAggroRange;
	mob["GetAllowBeneficial"] = (bool(Lua_Mob::*)(void))&Lua_Mob::GetAllowBeneficial;
	mob["GetAppearance"] = (uint32(Lua_Mob::*)(void))&Lua_Mob::GetAppearance;
	mob["GetAssistRange"] = (float(Lua_Mob::*)(void))&Lua_Mob::GetAssistRange;
	mob["GetBaseGender"] = &Lua_Mob::GetBaseGender;
	mob["GetBaseRace"] = &Lua_Mob::GetBaseRace;
	mob["GetBaseSize"] = (double(Lua_Mob::*)(void))&Lua_Mob::GetBaseSize;
	mob["GetBeard"] = &Lua_Mob::GetBeard;
	mob["GetBeardColor"] = &Lua_Mob::GetBeardColor;
	mob["GetBodyType"] = &Lua_Mob::GetBodyType;
	mob["GetBucket"] = (std::string(Lua_Mob::*)(std::string))&Lua_Mob::GetBucket;
	mob["GetBucketExpires"] = (std::string(Lua_Mob::*)(std::string))&Lua_Mob::GetBucketExpires;
	mob["GetBucketKey"] = (std::string(Lua_Mob::*)(void))&Lua_Mob::GetBucketKey;
	mob["GetBucketRemaining"] = (std::string(Lua_Mob::*)(std::string))&Lua_Mob::GetBucketRemaining;
	mob["GetBuffSlotFromType"] = &Lua_Mob::GetBuffSlotFromType;
	mob["GetBuffStatValueBySlot"] = (void(Lua_Mob::*)(uint8, const char*))& Lua_Mob::GetBuffStatValueBySlot;
	mob["GetBuffStatValueBySpell"] = (void(Lua_Mob::*)(int, const char*))&Lua_Mob::GetBuffStatValueBySpell;
	mob["GetCHA"] = &Lua_Mob::GetCHA;
	mob["GetCR"] = &Lua_Mob::GetCR;
	mob["GetCasterLevel"] = &Lua_Mob::GetCasterLevel;
	mob["GetClass"] = &Lua_Mob::GetClass;
	mob["GetClassName"] = &Lua_Mob::GetClassName;
	mob["GetCleanName"] = &Lua_Mob::GetCleanName;
	mob["GetCorruption"] = &Lua_Mob::GetCorruption;
	mob["GetDEX"] = &Lua_Mob::GetDEX;
	mob["GetDR"] = &Lua_Mob::GetDR;
	mob["GetDamageAmount"] = (uint32(Lua_Mob::*)(Lua_Mob))&Lua_Mob::GetDamageAmount;
	mob["GetDeity"] = &Lua_Mob::GetDeity;
	mob["GetDisplayAC"] = &Lua_Mob::GetDisplayAC;
	mob["GetDrakkinDetails"] = &Lua_Mob::GetDrakkinDetails;
	mob["GetDrakkinHeritage"] = &Lua_Mob::GetDrakkinHeritage;
	mob["GetDrakkinTattoo"] = &Lua_Mob::GetDrakkinTattoo;
	mob["GetEntityVariable"] = (const char*(Lua_Mob::*)(const char*))&Lua_Mob::GetEntityVariable;
	mob["GetEyeColor1"] = &Lua_Mob::GetEyeColor1;
	mob["GetEyeColor2"] = &Lua_Mob::GetEyeColor2;
	mob["GetFR"] = &Lua_Mob::GetFR;
	mob["GetFcDamageAmtIncoming"] = &Lua_Mob::GetFcDamageAmtIncoming;
	mob["GetFlurryChance"] = (int(Lua_Mob::*)(void))&Lua_Mob::GetFlurryChance;
	mob["GetGender"] = &Lua_Mob::GetGender;
	mob["GetGlobal"] = (std::string(Lua_Mob::*)(const char*))&Lua_Mob::GetGlobal;
	mob["GetHP"] = &Lua_Mob::GetHP;
	mob["GetHPRatio"] = &Lua_Mob::GetHPRatio;
	mob["GetHairColor"] = &Lua_Mob::GetHairColor;
	mob["GetHairStyle"] = &Lua_Mob::GetHairStyle;
	mob["GetHandToHandDamage"] = (int(Lua_Mob::*)(void))&Lua_Mob::GetHandToHandDamage;
	mob["GetHandToHandDelay"] = (int(Lua_Mob::*)(void))&Lua_Mob::GetHandToHandDelay;
	mob["GetHaste"] = (int(Lua_Mob::*)(void))&Lua_Mob::GetHaste;
	mob["GetHateAmount"] = sol::overload((int64(Lua_Mob::*)(Lua_Mob)) & Lua_Mob::GetHateAmount,
					     (int64(Lua_Mob::*)(Lua_Mob, bool)) & Lua_Mob::GetHateAmount);
	mob["GetHateClosest"] = &Lua_Mob::GetHateClosest;
	mob["GetHateDamageTop"] = (Lua_Mob(Lua_Mob::*)(Lua_Mob))&Lua_Mob::GetHateDamageTop;
	mob["GetHateList"] = &Lua_Mob::GetHateList;
	mob["GetHateListByDistance"] = sol::overload((Lua_HateList(Lua_Mob::*)(int)) & Lua_Mob::GetHateListByDistance,
						     (Lua_HateList(Lua_Mob::*)(void)) & Lua_Mob::GetHateListByDistance);
	mob["GetHateRandom"] = (Lua_Mob(Lua_Mob::*)(void))&Lua_Mob::GetHateRandom;
#ifdef BOTS
	mob["GetHateRandomBot"] = (Lua_Bot(Lua_Mob::*)(void))&Lua_Mob::GetHateRandomBot;
#endif
	mob["GetHateRandomClient"] = (Lua_Client(Lua_Mob::*)(void))&Lua_Mob::GetHateRandomClient;
	mob["GetHateRandomNPC"] = (Lua_NPC(Lua_Mob::*)(void))&Lua_Mob::GetHateRandomNPC;
	mob["GetHateTop"] = (Lua_Mob(Lua_Mob::*)(void))&Lua_Mob::GetHateTop;
	mob["GetHeading"] = &Lua_Mob::GetHeading;
	mob["GetHelmTexture"] = &Lua_Mob::GetHelmTexture;
	mob["GetHerosForgeModel"] = (int32(Lua_Mob::*)(uint8))&Lua_Mob::GetHerosForgeModel;
	mob["GetINT"] = &Lua_Mob::GetINT;
	mob["GetInvisibleLevel"] = (uint8(Lua_Mob::*)(void))&Lua_Mob::GetInvisibleLevel;
	mob["GetInvisibleUndeadLevel"] = (uint8(Lua_Mob::*)(void))&Lua_Mob::GetInvisibleUndeadLevel;
	mob["GetInvul"] = (bool(Lua_Mob::*)(void))&Lua_Mob::GetInvul;
	mob["GetItemBonuses"] = &Lua_Mob::GetItemBonuses;
	mob["GetItemHPBonuses"] = &Lua_Mob::GetItemHPBonuses;
	mob["GetItemStat"] = (int(Lua_Mob::*)(uint32,const char*))&Lua_Mob::GetItemStat;
	mob["GetLastName"] = &Lua_Mob::GetLastName;
	mob["GetLevel"] = &Lua_Mob::GetLevel;
	mob["GetLevelCon"] = sol::overload((uint32(Lua_Mob::*)(int)) & Lua_Mob::GetLevelCon,
					   (uint32(Lua_Mob::*)(int, int)) & Lua_Mob::GetLevelCon);
	mob["GetLuclinFace"] = &Lua_Mob::GetLuclinFace;
	mob["GetMR"] = &Lua_Mob::GetMR;
	mob["GetMana"] = &Lua_Mob::GetMana;
	mob["GetManaRatio"] = &Lua_Mob::GetManaRatio;
	mob["GetMaxAGI"] = &Lua_Mob::GetMaxAGI;
	mob["GetMaxCHA"] = &Lua_Mob::GetMaxCHA;
	mob["GetMaxDEX"] = &Lua_Mob::GetMaxDEX;
	mob["GetMaxHP"] = &Lua_Mob::GetMaxHP;
	mob["GetMaxINT"] = &Lua_Mob::GetMaxINT;
	mob["GetMaxMana"] = &Lua_Mob::GetMaxMana;
	mob["GetMaxSTA"] = &Lua_Mob::GetMaxSTA;
	mob["GetMaxSTR"] = &Lua_Mob::GetMaxSTR;
	mob["GetMaxWIS"] = &Lua_Mob::GetMaxWIS;
	mob["GetMeleeDamageMod_SE"] = &Lua_Mob::GetMeleeDamageMod_SE;
	mob["GetMeleeMinDamageMod_SE"] = &Lua_Mob::GetMeleeMinDamageMod_SE;
	mob["GetMeleeMitigation"] = (int32(Lua_Mob::*)(void))&Lua_Mob::GetMeleeMitigation;
	mob["GetModSkillDmgTaken"] = (int(Lua_Mob::*)(int))&Lua_Mob::GetModSkillDmgTaken;
	mob["GetModVulnerability"] = (int(Lua_Mob::*)(int))&Lua_Mob::GetModVulnerability;
	mob["GetNPCTypeID"] = &Lua_Mob::GetNPCTypeID;
	mob["GetName"] = &Lua_Mob::GetName;
	mob["GetNimbusEffect1"] = (uint8(Lua_Mob::*)(void))&Lua_Mob::GetNimbusEffect1;
	mob["GetNimbusEffect2"] = (uint8(Lua_Mob::*)(void))&Lua_Mob::GetNimbusEffect2;
	mob["GetNimbusEffect3"] = (uint8(Lua_Mob::*)(void))&Lua_Mob::GetNimbusEffect3;
	mob["GetOrigBodyType"] = &Lua_Mob::GetOrigBodyType;
	mob["GetOwner"] = &Lua_Mob::GetOwner;
	mob["GetPR"] = &Lua_Mob::GetPR;
	mob["GetPet"] = &Lua_Mob::GetPet;
	mob["GetPetOrder"] = (int(Lua_Mob::*)(void))&Lua_Mob::GetPetOrder;
	mob["GetPhR"] = &Lua_Mob::GetPhR;
	mob["GetRace"] = &Lua_Mob::GetRace;
	mob["GetRaceName"] = &Lua_Mob::GetRaceName;
	mob["GetResist"] = (int(Lua_Mob::*)(int))&Lua_Mob::GetResist;
	mob["GetReverseFactionCon"] = (int(Lua_Mob::*)(Lua_Mob))&Lua_Mob::GetReverseFactionCon;
	mob["GetRunspeed"] = &Lua_Mob::GetRunspeed;
	mob["GetSTA"] = &Lua_Mob::GetSTA;
	mob["GetSTR"] = &Lua_Mob::GetSTR;
	mob["GetShuffledHateList"] = &Lua_Mob::GetShuffledHateList;
	mob["GetSize"] = &Lua_Mob::GetSize;
	mob["GetSkill"] = (int(Lua_Mob::*)(int))&Lua_Mob::GetSkill;
	mob["GetSkillDmgAmt"] = (int(Lua_Mob::*)(int))&Lua_Mob::GetSkillDmgAmt;
	mob["GetSkillDmgTaken"] = (int(Lua_Mob::*)(int))&Lua_Mob::GetSkillDmgTaken;
	mob["GetSpecialAbility"] = (int(Lua_Mob::*)(int))&Lua_Mob::GetSpecialAbility;
	mob["GetSpecialAbilityParam"] = (int(Lua_Mob::*)(int,int))&Lua_Mob::GetSpecialAbilityParam;
	mob["GetSpecializeSkillValue"] = &Lua_Mob::GetSpecializeSkillValue;
	mob["GetSpellBonuses"] = &Lua_Mob::GetSpellBonuses;
	mob["GetSpellHPBonuses"] = &Lua_Mob::GetSpellHPBonuses;
	mob["GetTarget"] = &Lua_Mob::GetTarget;
	mob["GetTexture"] = &Lua_Mob::GetTexture;
	mob["GetWIS"] = &Lua_Mob::GetWIS;
	mob["GetWalkspeed"] = &Lua_Mob::GetWalkspeed;
	mob["GetWaypointH"] = &Lua_Mob::GetWaypointH;
	mob["GetWaypointID"] = &Lua_Mob::GetWaypointID;
	mob["GetWaypointPause"] = &Lua_Mob::GetWaypointPause;
	mob["GetWaypointX"] = &Lua_Mob::GetWaypointX;
	mob["GetWaypointY"] = &Lua_Mob::GetWaypointY;
	mob["GetWaypointZ"] = &Lua_Mob::GetWaypointZ;
	mob["GetWeaponDamage"] = &Lua_Mob::GetWeaponDamage;
	mob["GetWeaponDamageBonus"] = &Lua_Mob::GetWeaponDamageBonus;
	mob["GetX"] = &Lua_Mob::GetX;
	mob["GetY"] = &Lua_Mob::GetY;
	mob["GetZ"] = &Lua_Mob::GetZ;
	mob["GotoBind"] = &Lua_Mob::GotoBind;
	mob["HalveAggro"] = &Lua_Mob::HalveAggro;
	mob["HasNPCSpecialAtk"] = (bool(Lua_Mob::*)(const char*))&Lua_Mob::HasNPCSpecialAtk;
	mob["HasOwner"] = (bool(Lua_Mob::*)(void))&Lua_Mob::HasOwner;
	mob["HasPet"] = (bool(Lua_Mob::*)(void))&Lua_Mob::HasPet;
	mob["HasProcs"] = &Lua_Mob::HasProcs;
	mob["HasShieldEquiped"] = (bool(Lua_Mob::*)(void))&Lua_Mob::HasShieldEquiped;
	mob["HasTwoHandBluntEquiped"] = (bool(Lua_Mob::*)(void))&Lua_Mob::HasTwoHandBluntEquiped;
	mob["HasTwoHanderEquipped"] = (bool(Lua_Mob::*)(void))&Lua_Mob::HasTwoHanderEquipped;
	mob["Heal"] = &Lua_Mob::Heal;
	mob["HealDamage"] = sol::overload((void(Lua_Mob::*)(uint64)) & Lua_Mob::HealDamage,
					  (void(Lua_Mob::*)(uint64, Lua_Mob)) & Lua_Mob::HealDamage);
	mob["InterruptSpell"] = sol::overload((void(Lua_Mob::*)(int)) & Lua_Mob::InterruptSpell,
					      (void(Lua_Mob::*)(void)) & Lua_Mob::InterruptSpell);
	mob["IsAIControlled"] = (bool(Lua_Mob::*)(void))&Lua_Mob::IsAIControlled;
	mob["IsAmnesiad"] = (bool(Lua_Mob::*)(void))&Lua_Mob::IsAmnesiad;
	mob["IsAttackAllowed"] = &Lua_Mob::IsAttackAllowed;
	mob["IsBeneficialAllowed"] = (bool(Lua_Mob::*)(Lua_Mob))&Lua_Mob::IsBeneficialAllowed;
	mob["IsBerserk"] = &Lua_Mob::IsBerserk;
	mob["IsBlind"] = (bool(Lua_Mob::*)(void))&Lua_Mob::IsBlind;
	mob["IsCasting"] = &Lua_Mob::IsCasting;
	mob["IsEliteMaterialItem"] = (uint32(Lua_Mob::*)(uint8))&Lua_Mob::IsEliteMaterialItem;
	mob["IsEngaged"] =
	    sol::overload((bool(Lua_Mob::*)(void)) & Lua_Mob::IsEngaged, (bool(Lua_Mob::*)(void)) & Lua_Mob::IsEnraged);
	mob["IsFeared"] = (bool(Lua_Mob::*)(void))&Lua_Mob::IsFeared;
	mob["IsHorse"] = &Lua_Mob::IsHorse;
	mob["IsImmuneToSpell"] = (bool(Lua_Mob::*)(int,Lua_Mob))&Lua_Mob::IsImmuneToSpell;
	mob["IsInvisible"] = sol::overload((bool(Lua_Mob::*)(Lua_Mob)) & Lua_Mob::IsInvisible,
					   (bool(Lua_Mob::*)(void)) & Lua_Mob::IsInvisible);
	mob["IsMeleeDisabled"] = (bool(Lua_Mob::*)(void))&Lua_Mob::IsMeleeDisabled;
	mob["IsMezzed"] = (bool(Lua_Mob::*)(void))&Lua_Mob::IsMezzed;
	mob["IsMoving"] = &Lua_Mob::IsMoving;
	mob["IsPet"] = (bool(Lua_Mob::*)(void))&Lua_Mob::IsPet;
	mob["IsRoamer"] = (bool(Lua_Mob::*)(void))&Lua_Mob::IsRoamer;
	mob["IsRooted"] = (bool(Lua_Mob::*)(void))&Lua_Mob::IsRooted;
	mob["IsRunning"] = (bool(Lua_Mob::*)(void))&Lua_Mob::IsRunning;
	mob["IsSilenced"] = (bool(Lua_Mob::*)(void))&Lua_Mob::IsSilenced;
	mob["IsStunned"] = (bool(Lua_Mob::*)(void))&Lua_Mob::IsStunned;
	mob["IsTargetable"] = (bool(Lua_Mob::*)(void))&Lua_Mob::IsTargetable;
	mob["IsTargeted"] = &Lua_Mob::IsTargeted;
	mob["IsWarriorClass"] = &Lua_Mob::IsWarriorClass;
	mob["Kill"] = (void(Lua_Mob::*)(void))&Lua_Mob::Kill;
	mob["Mesmerize"] = (void(Lua_Mob::*)(void))&Lua_Mob::Mesmerize;
	mob["Message"] = &Lua_Mob::Message;
	mob["MessageString"] = &Lua_Mob::MessageString;
	mob["Message_StringID"] = &Lua_Mob::MessageString;
	mob["ModSkillDmgTaken"] = (void(Lua_Mob::*)(int,int))&Lua_Mob::ModSkillDmgTaken;
	mob["ModVulnerability"] = (void(Lua_Mob::*)(int,int))&Lua_Mob::ModVulnerability;
	mob["NPCSpecialAttacks"] =
	    sol::overload((void(Lua_Mob::*)(const char *, int)) & Lua_Mob::NPCSpecialAttacks,
			  (void(Lua_Mob::*)(const char *, int, bool)) & Lua_Mob::NPCSpecialAttacks,
			  (void(Lua_Mob::*)(const char *, int, bool, bool)) & Lua_Mob::NPCSpecialAttacks);
	mob["NavigateTo"] = (void(Lua_Mob::*)(double,double,double))&Lua_Mob::NavigateTo;
	mob["ProcessSpecialAbilities"] = (void(Lua_Mob::*)(std::string))&Lua_Mob::ProcessSpecialAbilities;
	mob["ProjectileAnimation"] = sol::overload(
	    (void(Lua_Mob::*)(Lua_Mob, int)) & Lua_Mob::ProjectileAnimation,
	    (void(Lua_Mob::*)(Lua_Mob, int, bool)) & Lua_Mob::ProjectileAnimation,
	    (void(Lua_Mob::*)(Lua_Mob, int, bool, double)) & Lua_Mob::ProjectileAnimation,
	    (void(Lua_Mob::*)(Lua_Mob, int, bool, double, double)) & Lua_Mob::ProjectileAnimation,
	    (void(Lua_Mob::*)(Lua_Mob, int, bool, double, double, double)) & Lua_Mob::ProjectileAnimation,
	    (void(Lua_Mob::*)(Lua_Mob, int, bool, double, double, double, double)) & Lua_Mob::ProjectileAnimation);
	mob["QuestSay"] = sol::overload((void(Lua_Mob::*)(Lua_Client, const char *)) & Lua_Mob::QuestSay,
					(void(Lua_Mob::*)(Lua_Client, const char *, sol::table)) & Lua_Mob::QuestSay);
	mob["RandomizeFeatures"] = (void(Lua_Mob::*)(bool,bool))&Lua_Mob::RandomizeFeatures;
	mob["RangedAttack"] = &Lua_Mob::RangedAttack;
	mob["RemoveAllNimbusEffects"] = &Lua_Mob::RemoveAllNimbusEffects;
	mob["RemoveNimbusEffect"] = (void(Lua_Mob::*)(int))&Lua_Mob::RemoveNimbusEffect;
	mob["RemovePet"] = &Lua_Mob::RemovePet;
	mob["ResistSpell"] =
	    sol::overload((double(Lua_Mob::*)(int, int, Lua_Mob)) & Lua_Mob::ResistSpell,
			  (double(Lua_Mob::*)(int, int, Lua_Mob, bool)) & Lua_Mob::ResistSpell,
			  (double(Lua_Mob::*)(int, int, Lua_Mob, bool, int)) & Lua_Mob::ResistSpell,
			  (double(Lua_Mob::*)(int, int, Lua_Mob, bool, int, bool)) & Lua_Mob::ResistSpell);
	mob["RunTo"] = (void(Lua_Mob::*)(double, double, double))&Lua_Mob::RunTo;
	mob["Say"] = sol::overload((void(Lua_Mob::*)(const char *)) & Lua_Mob::Say,
				   (void(Lua_Mob::*)(const char *, int)) & Lua_Mob::Say);
	mob["SeeHide"] = (bool(Lua_Mob::*)(void))&Lua_Mob::SeeHide;
	mob["SeeImprovedHide"] = (bool(Lua_Mob::*)(bool))&Lua_Mob::SeeImprovedHide;
	mob["SeeInvisible"] = (uint8(Lua_Mob::*)(void))&Lua_Mob::SeeInvisible;
	mob["SeeInvisibleUndead"] = (uint8(Lua_Mob::*)(void))&Lua_Mob::SeeInvisibleUndead;
	mob["SetSeeInvisibleLevel"] = (void(Lua_Mob::*)(uint8))&Lua_Mob::SetSeeInvisibleLevel;
	mob["SetSeeInvisibleUndeadLevel"] = (void(Lua_Mob::*)(uint8))&Lua_Mob::SetSeeInvisibleUndeadLevel;
	mob["SendAppearanceEffect"] = sol::overload(
	    (void(Lua_Mob::*)(uint32, uint32, uint32, uint32, uint32)) & Lua_Mob::SendAppearanceEffect,
	    (void(Lua_Mob::*)(uint32, uint32, uint32, uint32, uint32, Lua_Client)) & Lua_Mob::SendAppearanceEffect);
	mob["SendBeginCast"] = &Lua_Mob::SendBeginCast;
	mob["SendIllusionPacket"] = (void(Lua_Mob::*)(sol::table))&Lua_Mob::SendIllusionPacket;
	mob["SendSpellEffect"] = sol::overload(
	    (void(Lua_Mob::*)(uint32, uint32, uint32, bool, uint32)) & Lua_Mob::SendSpellEffect,
	    (void(Lua_Mob::*)(uint32, uint32, uint32, bool, uint32, bool)) & Lua_Mob::SendSpellEffect,
	    (void(Lua_Mob::*)(uint32, uint32, uint32, bool, uint32, bool, Lua_Client)) & Lua_Mob::SendSpellEffect);
	mob["SendTo"] = (void(Lua_Mob::*)(double,double,double))&Lua_Mob::SendTo;
	mob["SendToFixZ"] = (void(Lua_Mob::*)(double,double,double))&Lua_Mob::SendToFixZ;
	mob["SetAA"] = sol::overload((bool(Lua_Mob::*)(int, int)) & Lua_Mob::SetAA,
				     (bool(Lua_Mob::*)(int, int, int)) & Lua_Mob::SetAA);
	mob["SetAllowBeneficial"] = (void(Lua_Mob::*)(bool))&Lua_Mob::SetAllowBeneficial;
	mob["SetAppearance"] = sol::overload((void(Lua_Mob::*)(int)) & Lua_Mob::SetAppearance,
					     (void(Lua_Mob::*)(int, bool)) & Lua_Mob::SetAppearance);
	mob["SetBodyType"] = (void(Lua_Mob::*)(int,bool))&Lua_Mob::SetBodyType;
	mob["SetBucket"] =
	    sol::overload((void(Lua_Mob::*)(std::string, std::string)) & Lua_Mob::SetBucket,
			  (void(Lua_Mob::*)(std::string, std::string, std::string)) & Lua_Mob::SetBucket);
	mob["SetBuffDuration"] = sol::overload((void(Lua_Mob::*)(int)) & Lua_Mob::SetBuffDuration,
					       (void(Lua_Mob::*)(int, int)) & Lua_Mob::SetBuffDuration);
	mob["SetCurrentWP"] = &Lua_Mob::SetCurrentWP;
	mob["SetDestructibleObject"] = (void(Lua_Mob::*)(bool))&Lua_Mob::SetDestructibleObject;
	mob["SetDisableMelee"] = (void(Lua_Mob::*)(bool))&Lua_Mob::SetDisableMelee;
	mob["SetEntityVariable"] = (void(Lua_Mob::*)(const char*,const char*))&Lua_Mob::SetEntityVariable;
	mob["SetExtraHaste"] = (void(Lua_Mob::*)(int))&Lua_Mob::SetExtraHaste;
	mob["SetFlurryChance"] = (void(Lua_Mob::*)(int))&Lua_Mob::SetFlurryChance;
	mob["SetFlyMode"] = (void(Lua_Mob::*)(int))&Lua_Mob::SetFlyMode;
	mob["SetGender"] = (void(Lua_Mob::*)(int))&Lua_Mob::SetGender;
	mob["SetGlobal"] = sol::overload(
	    (void(Lua_Mob::*)(const char *, const char *, int, const char *)) & Lua_Mob::SetGlobal,
	    (void(Lua_Mob::*)(const char *, const char *, int, const char *, Lua_Mob)) & Lua_Mob::SetGlobal);
	mob["SetHP"] = &Lua_Mob::SetHP;
	mob["SetHate"] = sol::overload((void(Lua_Mob::*)(Lua_Mob)) & Lua_Mob::SetHate,
				       (void(Lua_Mob::*)(Lua_Mob, int64)) & Lua_Mob::SetHate,
				       (void(Lua_Mob::*)(Lua_Mob, int64, int64)) & Lua_Mob::SetHate);
	mob["SetHeading"] = (void(Lua_Mob::*)(double))&Lua_Mob::SetHeading;
	mob["SetInvisible"] = &Lua_Mob::SetInvisible;
	mob["SetInvul"] = (void(Lua_Mob::*)(bool))&Lua_Mob::SetInvul;
	mob["SetLevel"] = sol::overload((void(Lua_Mob::*)(int)) & Lua_Mob::SetLevel,
					(void(Lua_Mob::*)(int, bool)) & Lua_Mob::SetLevel);
	mob["SetMana"] = &Lua_Mob::SetMana;
	mob["SetOOCRegen"] = (void(Lua_Mob::*)(int64))&Lua_Mob::SetOOCRegen;
	mob["SetPet"] = &Lua_Mob::SetPet;
	mob["SetPetOrder"] = (void(Lua_Mob::*)(int))&Lua_Mob::SetPetOrder;
	mob["SetPseudoRoot"] = (void(Lua_Mob::*)(bool))&Lua_Mob::SetPseudoRoot;
	mob["SetRace"] = (void(Lua_Mob::*)(int))&Lua_Mob::SetRace;
	mob["SetRunning"] = (void(Lua_Mob::*)(bool))&Lua_Mob::SetRunning;
	mob["SetSlotTint"] = (void(Lua_Mob::*)(int,int,int,int))&Lua_Mob::SetSlotTint;
	mob["SetSpecialAbility"] = (void(Lua_Mob::*)(int,int))&Lua_Mob::SetSpecialAbility;
	mob["SetSpecialAbilityParam"] = (void(Lua_Mob::*)(int,int,int))&Lua_Mob::SetSpecialAbilityParam;
	mob["SetTarget"] = &Lua_Mob::SetTarget;
	mob["SetTargetable"] = (void(Lua_Mob::*)(bool))&Lua_Mob::SetTargetable;
	mob["SetTexture"] = (void(Lua_Mob::*)(int))&Lua_Mob::SetTexture;
	mob["Shout"] = sol::overload((void(Lua_Mob::*)(const char *)) & Lua_Mob::Shout,
				     (void(Lua_Mob::*)(const char *, int)) & Lua_Mob::Shout);
	mob["Signal"] = (void(Lua_Mob::*)(uint32))&Lua_Mob::Signal;
	mob["SpellEffect"] = &Lua_Mob::SpellEffect;
	mob["SpellFinished"] =
	    sol::overload((bool(Lua_Mob::*)(int, Lua_Mob)) & Lua_Mob::SpellFinished,
			  (bool(Lua_Mob::*)(int, Lua_Mob, int)) & Lua_Mob::SpellFinished,
			  (bool(Lua_Mob::*)(int, Lua_Mob, int, int)) & Lua_Mob::SpellFinished,
			  (bool(Lua_Mob::*)(int, Lua_Mob, int, int, uint32)) & Lua_Mob::SpellFinished,
			  (bool(Lua_Mob::*)(int, Lua_Mob, int, int, uint32, int)) & Lua_Mob::SpellFinished,
			  (bool(Lua_Mob::*)(int, Lua_Mob, int, int, uint32, int, bool)) & Lua_Mob::SpellFinished);
	mob["Spin"] = (void(Lua_Mob::*)(void))&Lua_Mob::Spin;
	mob["StopNavigation"] = (void(Lua_Mob::*)(void))&Lua_Mob::StopNavigation;
	mob["Stun"] = (void(Lua_Mob::*)(int))&Lua_Mob::Stun;
	mob["TarGlobal"] = (void(Lua_Mob::*)(const char*,const char*,const char*,int,int,int))&Lua_Mob::TarGlobal;
	mob["TempName"] = sol::overload((void(Lua_Mob::*)(const char *)) & Lua_Mob::TempName,
					(void(Lua_Mob::*)(void)) & Lua_Mob::TempName);
	mob["ThrowingAttack"] = &Lua_Mob::ThrowingAttack;
	mob["TryMoveAlong"] = sol::overload((void(Lua_Mob::*)(float, float)) & Lua_Mob::TryMoveAlong,
					    (void(Lua_Mob::*)(float, float, bool)) & Lua_Mob::TryMoveAlong);
	mob["UnStun"] = (void(Lua_Mob::*)(void))&Lua_Mob::UnStun;
	mob["WalkTo"] = (void(Lua_Mob::*)(double, double, double))&Lua_Mob::WalkTo;
	mob["WearChange"] = (void(Lua_Mob::*)(int,int,uint32))&Lua_Mob::WearChange;
	mob["WipeHateList"] = (void(Lua_Mob::*)(void))&Lua_Mob::WipeHateList;
}

void lua_register_special_abilities(sol::state_view &sv)
{
	sv.new_enum(
		"SpecialAbility",

		"summon", static_cast<int>(SPECATK_SUMMON),
		"enrage", static_cast<int>(SPECATK_ENRAGE),
		"rampage", static_cast<int>(SPECATK_RAMPAGE),
		"area_rampage", static_cast<int>(SPECATK_AREA_RAMPAGE),
		"flurry", static_cast<int>(SPECATK_FLURRY),
		"triple_attack", static_cast<int>(SPECATK_TRIPLE),
		"quad_attack", static_cast<int>(SPECATK_QUAD),
		"innate_dual_wield", static_cast<int>(SPECATK_INNATE_DW),
		"bane_attack", static_cast<int>(SPECATK_BANE),
		"magical_attack", static_cast<int>(SPECATK_MAGICAL),
		"ranged_attack", static_cast<int>(SPECATK_RANGED_ATK),
		"unslowable", static_cast<int>(UNSLOWABLE),
		"unmezable", static_cast<int>(UNMEZABLE),
		"uncharmable", static_cast<int>(UNCHARMABLE),
		"unstunable", static_cast<int>(UNSTUNABLE),
		"unsnareable", static_cast<int>(UNSNAREABLE),
		"unfearable", static_cast<int>(UNFEARABLE),
		"undispellable", static_cast<int>(UNDISPELLABLE),
		"immune_melee", static_cast<int>(IMMUNE_MELEE),
		"immune_magic", static_cast<int>(IMMUNE_MAGIC),
		"immune_fleeing", static_cast<int>(IMMUNE_FLEEING),
		"immune_melee_except_bane", static_cast<int>(IMMUNE_MELEE_EXCEPT_BANE),
		"immune_melee_except_magical", static_cast<int>(IMMUNE_MELEE_NONMAGICAL),
		"immune_aggro", static_cast<int>(IMMUNE_AGGRO),
		"immune_aggro_on", static_cast<int>(IMMUNE_AGGRO_ON),
		"immune_casting_from_range", static_cast<int>(IMMUNE_CASTING_FROM_RANGE),
		"immune_feign_death", static_cast<int>(IMMUNE_FEIGN_DEATH),
		"immune_taunt", static_cast<int>(IMMUNE_TAUNT),
		"tunnelvision", static_cast<int>(NPC_TUNNELVISION),
		"dont_buff_friends", static_cast<int>(NPC_NO_BUFFHEAL_FRIENDS),
		"immune_pacify", static_cast<int>(IMMUNE_PACIFY),
		"leash", static_cast<int>(LEASH),
		"tether", static_cast<int>(TETHER),
		"destructible_object", static_cast<int>(DESTRUCTIBLE_OBJECT),
		"no_harm_from_client", static_cast<int>(NO_HARM_FROM_CLIENT),
		"always_flee", static_cast<int>(ALWAYS_FLEE),
		"flee_percent", static_cast<int>(FLEE_PERCENT),
		"allow_beneficial", static_cast<int>(ALLOW_BENEFICIAL),
		"disable_melee", static_cast<int>(DISABLE_MELEE),
		"npc_chase_distance", static_cast<int>(NPC_CHASE_DISTANCE),
		"allow_to_tank", static_cast<int>(ALLOW_TO_TANK),
		"ignore_root_aggro_rules", static_cast<int>(IGNORE_ROOT_AGGRO_RULES),
		"casting_resist_diff", static_cast<int>(CASTING_RESIST_DIFF),
		"counter_avoid_damage", static_cast<int>(COUNTER_AVOID_DAMAGE),
		"immune_ranged_attacks", static_cast<int>(IMMUNE_RANGED_ATTACKS),
		"immune_damage_client", static_cast<int>(IMMUNE_DAMAGE_CLIENT),
		"immune_damage_npc", static_cast<int>(IMMUNE_DAMAGE_NPC),
		"immune_aggro_client", static_cast<int>(IMMUNE_AGGRO_CLIENT),
		"immune_aggro_npc", static_cast<int>(IMMUNE_AGGRO_NPC),
		"modify_avoid_damage", static_cast<int>(MODIFY_AVOID_DAMAGE));
}

#endif
