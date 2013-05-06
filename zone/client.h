/*  EQEMu:  Everquest Server Emulator
	Copyright (C) 2001-2003  EQEMu Development Team (http://eqemulator.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef CLIENT_H
#define CLIENT_H
class Client;

#include "../common/timer.h"
#include "../common/ptimer.h"
#include "../common/emu_opcodes.h"
#include "../common/eq_packet_structs.h"
#include "../common/eq_constants.h"
#include "../common/EQStreamIntf.h"
#include "../common/EQPacket.h"
#include "../common/linked_list.h"
#include "../common/extprofile.h"
#include "zonedb.h"
#include "errno.h"
#include "../common/classes.h"
#include "../common/races.h"
#include "../common/deity.h"
#include "mob.h"
#include "npc.h"
#include "merc.h"
#include "zone.h"
#include "AA.h"
#include "../common/seperator.h"
#include "../common/Item.h"
#include "updatemgr.h"
#include "../common/guilds.h"
#include "questmgr.h"
#include <float.h>
#include <set>
#include <string>
#include "../common/item_struct.h"
#include "../common/clientversions.h"
#include "QGlobals.h"

#define CLIENT_TIMEOUT		90000
#define CLIENT_LD_TIMEOUT	30000 // length of time client stays in zone after LDing
#define TARGETING_RANGE		200	// range for /assist and /target
#define XTARGET_HARDCAP		20

extern Zone* zone;
extern TaskManager *taskmanager;

class CLIENTPACKET
{
public:
    CLIENTPACKET();
    ~CLIENTPACKET();
    EQApplicationPacket *app;
    bool ack_req;
};

enum {	//Type arguments to the Message* routines.
	//all not explicitly listed are the same grey color
	clientMessageWhite0 = 0,
	clientMessageLoot = 2,	//dark green
	clientMessageTradeskill = 4,	//light blue
	clientMessageTell = 5,		//magenta
	clientMessageWhite = 7,
	clientMessageWhite2 = 10,
	clientMessageLightGrey = 12,
	clientMessageError = 13,	//red
	clientMessageGreen = 14,
	clientMessageYellow = 15,
	clientMessageBlue = 16,
	clientMessageGroup = 18,	//cyan
	clientMessageWhite3 = 20,
};

#define SPELLBAR_UNLOCK 0x2bc
enum {	//scribing argument to MemorizeSpell
	memSpellScribing = 0,
	memSpellMemorize = 1,
	memSpellForget = 2,
	memSpellSpellbar = 3
};

#define USE_ITEM_SPELL_SLOT 10
#define POTION_BELT_SPELL_SLOT 11
#define DISCIPLINE_SPELL_SLOT 10
#define ABILITY_SPELL_SLOT 9

//Modes for the zoning state of the client.
typedef enum {
	ZoneToSafeCoords,		// Always send ZonePlayerToBind_Struct to client: Succor/Evac
	GMSummon,				// Always send ZonePlayerToBind_Struct to client: Only a GM Summon
	ZoneToBindPoint,		// Always send ZonePlayerToBind_Struct to client: Death Only
	ZoneSolicited,			// Always send ZonePlayerToBind_Struct to client: Portal, Translocate, Evac spells that have a x y z coord in the spell data
	ZoneUnsolicited,
	GateToBindPoint,		// Always send RequestClientZoneChange_Struct to client: Gate spell or Translocate To Bind Point spell
	SummonPC,				// In-zone GMMove() always: Call of the Hero spell or some other type of in zone only summons
	Rewind,					// Summon to /rewind location.
	EvacToSafeCoords
} ZoneMode;

typedef enum {
	MQWarp,
	MQWarpShadowStep,
	MQWarpKnockBack,
	MQWarpLight,
	MQZone,
	MQZoneUnknownDest,
	MQGate,
	MQGhost
} CheatTypes;

enum {
	HideCorpseNone = 0,
	HideCorpseAll = 1,
	HideCorpseAllButGroup = 2,
	HideCorpseLooted = 3,
	HideCorpseNPC = 5
};

typedef enum 
{
	Empty = 0,
	Auto = 1,
	CurrentTargetPC = 2,
	CurrentTargetNPC = 3,
	TargetsTarget = 4,
	GroupTank = 5,
	GroupTankTarget = 6,
	GroupAssist = 7,
	GroupAssistTarget = 8,
	Puller = 9,
	PullerTarget = 10,
	GroupMarkTarget1 = 11,
	GroupMarkTarget2 = 12,
	GroupMarkTarget3 = 13,
	RaidAssist1 = 14,
	RaidAssist2 = 15,
	RaidAssist3 = 16,
	RaidAssist1Target = 17,
	RaidAssist2Target = 18,
	RaidAssist3Target = 19,
	RaidMarkTarget1 = 20,
	RaidMarkTarget2 = 21,
	RaidMarkTarget3 = 22,
	MyPet = 23,
	MyPetTarget = 24,
	MyMercenary = 25,
	MyMercenaryTarget = 26

} XTargetType;

struct XTarget_Struct
{
	XTargetType Type;
	uint16 ID;
	char Name[65];
};
	
	
const uint32 POPUPID_UPDATE_SHOWSTATSWINDOW = 1000000;

struct ClientReward
{
	uint32 id;
	uint32 amount;
};

class ClientFactory {
public:
	Client *MakeClient(EQStream* ieqs);
};

class Client : public Mob
{
public:
	//pull in opcode mappings:
	#include "client_packet.h"

	Client(EQStreamInterface * ieqs);
    ~Client();

	//abstract virtual function implementations requird by base abstract class
	virtual void Death(Mob* killerMob, int32 damage, uint16 spell_id, SkillType attack_skill);
	virtual void Damage(Mob* from, int32 damage, uint16 spell_id, SkillType attack_skill, bool avoidable = true, int8 buffslot = -1, bool iBuffTic = false);
	virtual bool Attack(Mob* other, int Hand = 13, bool FromRiposte = false, bool IsStrikethrough = false, bool IsFromSpell = false);
	virtual bool HasRaid() { return (GetRaid() ? true : false);  }
	virtual bool HasGroup() { return (GetGroup() ? true : false); }
	virtual Raid* GetRaid() { return entity_list.GetRaidByClient(this); }
	virtual Group* GetGroup() { return entity_list.GetGroupByClient(this); }

//	void	Discipline(ClientDiscipline_Struct* disc_in, Mob* tar);
	void	AI_Init();
	void	AI_Start(uint32 iMoveDelay = 0);
	void	AI_Stop();
	void	AI_Process();
	void	AI_SpellCast();
	void	Trader_ShowItems();
	void    Trader_CustomerBrowsing(Client *Customer);
	void	Trader_EndTrader();
	void	Trader_StartTrader();
	uint8	WithCustomer(uint16 NewCustomer);
	void	KeyRingLoad();
	void	KeyRingAdd(uint32 item_id);
	bool	KeyRingCheck(uint32 item_id);
    void	KeyRingList();
	virtual bool IsClient() const { return true; }
	virtual void DBAWComplete(uint8 workpt_b1, DBAsyncWork* dbaw);
	bool	FinishConnState2(DBAsyncWork* dbaw);
	void	CompleteConnect();
	bool	TryStacking(ItemInst* item, uint8 type = ItemPacketTrade, bool try_worn = true, bool try_cursor = true);
	void	SendTraderPacket(Client* trader, uint32 Unknown72 = 51);
	void	SendBuyerPacket(Client* Buyer);
	GetItems_Struct* GetTraderItems();
	void	SendBazaarWelcome();
	void	DyeArmor(DyeStruct* dye);
	uint8	SlotConvert(uint8 slot,bool bracer=false);
	void	Message_StringID(uint32 type, uint32 string_id, uint32 distance = 0);
	void	Message_StringID(uint32 type, uint32 string_id, const char* message,const char* message2=0,const char* message3=0,const char* message4=0,const char* message5=0,const char* message6=0,const char* message7=0,const char* message8=0,const char* message9=0, uint32 distance = 0);
	void	SendBazaarResults(uint32 trader_id,uint32 class_,uint32 race,uint32 stat,uint32 slot,uint32 type,char name[64],uint32 minprice,uint32 maxprice);
	void	SendTraderItem(uint32 item_id,uint16 quantity);
	uint16	FindTraderItem(int32 SerialNumber,uint16 Quantity);
	ItemInst* FindTraderItemBySerialNumber(int32 SerialNumber);
	void	FindAndNukeTraderItem(int32 item_id,uint16 quantity,Client* customer,uint16 traderslot);
	void    NukeTraderItem(uint16 slot,int16 charges,uint16 quantity,Client* customer,uint16 traderslot, int uniqueid);
	void	ReturnTraderReq(const EQApplicationPacket* app,int16 traderitemcharges);
	void	TradeRequestFailed(const EQApplicationPacket* app);
	void	BuyTraderItem(TraderBuy_Struct* tbs,Client* trader,const EQApplicationPacket* app);
	void	TraderUpdate(uint16 slot_id,uint32 trader_id);
	void	FinishTrade(Mob* with, ServerPacket* qspack = nullptr, bool finalizer = false);
	void    SendZonePoints();

	void	SendBuyerResults(char *SearchQuery, uint32 SearchID);
	void	ShowBuyLines(const EQApplicationPacket *app);
	void	SellToBuyer(const EQApplicationPacket *app);
	void	ToggleBuyerMode(bool TurnOn);
	void	UpdateBuyLine(const EQApplicationPacket *app);
	void	BuyerItemSearch(const EQApplicationPacket *app);
	void	SetBuyerWelcomeMessage(const char* WelcomeMessage) { BuyerWelcomeMessage = WelcomeMessage; }
	const 	char* GetBuyerWelcomeMessage() { return BuyerWelcomeMessage.c_str(); }

	void	FillSpawnStruct(NewSpawn_Struct* ns, Mob* ForWho);
	virtual bool Process();
	void	LogMerchant(Client* player, Mob* merchant, uint32 quantity, uint32 price, const Item_Struct* item, bool buying);
	void	SendPacketQueue(bool Block = true);
	void	QueuePacket(const EQApplicationPacket* app, bool ack_req = true, CLIENT_CONN_STATUS = CLIENT_CONNECTINGALL, eqFilterType filter=FilterNone);
	void	FastQueuePacket(EQApplicationPacket** app, bool ack_req = true, CLIENT_CONN_STATUS = CLIENT_CONNECTINGALL);
	void	ChannelMessageReceived(uint8 chan_num, uint8 language, uint8 lang_skill, const char* orig_message, const char* targetname=nullptr);
	void	ChannelMessageSend(const char* from, const char* to, uint8 chan_num, uint8 language, const char* message, ...);
	void	ChannelMessageSend(const char* from, const char* to, uint8 chan_num, uint8 language, uint8 lang_skill, const char* message, ...);
	void	Message(uint32 type, const char* message, ...);
	void    QuestJournalledMessage(const char *npcname, const char* message);
	void	VoiceMacroReceived(uint32 Type, char *Target, uint32 MacroNumber);
	void	SendSound();
	void	LearnRecipe(uint32 recipeID);
	bool	CanIncreaseTradeskill(SkillType tradeskill);

	EQApplicationPacket*	ReturnItemPacket(int16 slot_id, const ItemInst* inst, ItemPacketType packet_type);

	bool			GetRevoked() const { return revoked; }
	void			SetRevoked(bool rev) { revoked = rev; }
	inline uint32	GetIP()			const { return ip; }
	inline bool		GetHideMe()			const { return gmhideme; }
	void			SetHideMe(bool hm);
	inline uint16	GetPort()		const { return port; }
	bool			IsDead() const { return(dead); }
	inline bool		IsLFP() { return LFP; }
	void			UpdateLFP();

	virtual bool	Save() { return Save(0); }
			bool	Save(uint8 iCommitNow); // 0 = delayed, 1=async now, 2=sync now
			void	SaveBackup();

	inline bool ClientDataLoaded() const { return client_data_loaded; }
	inline bool	Connected()		const { return (client_state == CLIENT_CONNECTED); }
	inline bool	InZone()		const { return (client_state == CLIENT_CONNECTED || client_state == CLIENT_LINKDEAD); }
	inline void	Kick()			{ client_state = CLIENT_KICKED; }
	inline void	Disconnect()	{ eqs->Close(); client_state = DISCONNECTED; }
	inline bool IsLD()			const { return (bool) (client_state == CLIENT_LINKDEAD); }
	void	WorldKick();
	inline uint8	GetAnon()		const {  return m_pp.anon; }
	inline PlayerProfile_Struct& GetPP()	{ return m_pp; }
	inline ExtendedProfile_Struct& GetEPP()	{ return m_epp; }
	inline Inventory& GetInv()				{ return m_inv; }
	inline const Inventory& GetInv() const	{ return m_inv; }
	inline PetInfo* GetPetInfo(uint16 pet)	{ return (pet==1)?&m_suspendedminion:&m_petinfo; }
	inline InspectMessage_Struct& GetInspectMessage()			  { return m_inspect_message; }
	inline const InspectMessage_Struct& GetInspectMessage() const { return m_inspect_message; }

	bool	CheckAccess(int16 iDBLevel, int16 iDefaultLevel);

	void	CheckQuests(const char* zonename, const char* message, uint32 npc_id, uint32 item_id, Mob* other);
	void	LogLoot(Client* player,Corpse* corpse,const Item_Struct* item);
	bool	AutoAttackEnabled() const { return auto_attack; }
	bool	AutoFireEnabled() const { return auto_fire; }
	void	MakeCorpse(uint32 exploss);

	bool	ChangeFirstName(const char* in_firstname,const char* gmname);

	void	Duck();
	void	Stand();

	virtual void	SetMaxHP();
	int32	LevelRegen();
	void	HPTick();
	void	SetGM(bool toggle);
	void	SetPVP(bool toggle);

	inline bool	GetPVP()	const { return zone->GetZoneID() == 77 ? true : (m_pp.pvp != 0); }
	inline bool	GetGM()		const { return m_pp.gm != 0; }

	inline void	SetBaseClass(uint32 i) { m_pp.class_=i; }
	inline void	SetBaseRace(uint32 i) { m_pp.race=i; }
	inline void	SetBaseGender(uint32 i) { m_pp.gender=i; }
	inline  void SetDeity(uint32 i) {m_pp.deity=i;deity=i;}

	inline uint8		GetLevel2()			const { return m_pp.level2; }
	inline uint16	GetBaseRace()		const { return m_pp.race; }
	inline uint16	GetBaseClass()		const { return m_pp.class_; }
	inline uint8		GetBaseGender()		const { return m_pp.gender; }
	inline uint8		GetBaseFace()		const { return m_pp.face; }
	inline uint8		GetBaseHairColor()	const { return m_pp.haircolor; }
	inline uint8		GetBaseBeardColor()	const { return m_pp.beardcolor; }
	inline uint8		GetBaseEyeColor()	const { return m_pp.eyecolor1; }
	inline uint8		GetBaseHairStyle()	const { return m_pp.hairstyle; }
	inline uint8		GetBaseBeard()		const { return m_pp.beard; }
	inline uint8		GetBaseHeritage()	const { return m_pp.drakkin_heritage; }
	inline uint8		GetBaseTattoo()		const { return m_pp.drakkin_tattoo; }
	inline uint8		GetBaseDetails()	const { return m_pp.drakkin_details; }
	inline const float	GetBindX(uint32 index = 0)	const { return m_pp.binds[index].x; }
	inline const float	GetBindY(uint32 index = 0)			const { return m_pp.binds[index].y; }
	inline const float	GetBindZ(uint32 index = 0)			const { return m_pp.binds[index].z; }
	inline const float	GetBindHeading(uint32 index = 0)			const { return m_pp.binds[index].heading; }
	inline uint32	GetBindZoneID(uint32 index = 0)			const { return m_pp.binds[index].zoneId; }
	int32			CalcMaxMana();
	int32			CalcBaseMana();
	const int32&	SetMana(int32 amount);
	int32 			CalcManaRegenCap();

	void	ServerFilter(SetServerFilter_Struct* filter);
	void	BulkSendTraderInventory(uint32 char_id);
	void    SendSingleTraderItem(uint32 char_id, int uniqueid);
	void	BulkSendMerchantInventory(int merchant_id, int npcid);

	inline uint8	GetLanguageSkill(uint16 n)	const { return m_pp.languages[n]; }

	void	SendPickPocketResponse(Mob *from, uint32 amt, int type, const Item_Struct* item = nullptr);

	inline const char*	GetLastName() const	{ return lastname; }

	inline float ProximityX() const { return(proximity_x); }
	inline float ProximityY() const { return(proximity_y); }
	inline float ProximityZ() const { return(proximity_z); }
	inline void ClearAllProximities() { entity_list.ProcessMove(this, FLT_MAX, FLT_MAX, FLT_MAX); proximity_x = FLT_MAX; proximity_y = FLT_MAX; proximity_z = FLT_MAX; }

	/*
		Begin client modifiers
	*/

	virtual void CalcBonuses();
	//these are all precalculated now
	inline virtual int16	GetAC()		const { return AC; }
	inline virtual int16 GetATK() const { return ATK + itembonuses.ATK + spellbonuses.ATK + ((GetSTR() + GetSkill(OFFENSE)) * 9 / 10); }
	inline virtual int16 GetATKBonus() const { return itembonuses.ATK + spellbonuses.ATK; }
	inline virtual int	GetHaste() const { return Haste; }
	int GetRawACNoShield(int &shield_ac) const;

	inline virtual int16	GetSTR()	const { return STR; }
	inline virtual int16	GetSTA()	const { return STA; }
	inline virtual int16	GetDEX()	const { return DEX; }
	inline virtual int16	GetAGI()	const { return AGI; }
	inline virtual int16	GetINT()	const { return INT; }
	inline virtual int16	GetWIS()	const { return WIS; }
	inline virtual int16	GetCHA()	const { return CHA; }
	inline virtual int16	GetMR()	const { return MR; }
	inline virtual int16	GetFR()	const { return FR; }
	inline virtual int16	GetDR()	const { return DR; }
	inline virtual int16	GetPR()	const { return PR; }
	inline virtual int16	GetCR()	const { return CR; }
	inline virtual int16	GetCorrup()	const { return Corrup; }
	
	int16	GetMaxStat() const;
	int16	GetMaxResist() const;
	int16  GetMaxSTR() const;
    int16  GetMaxSTA() const;
    int16  GetMaxDEX() const;
    int16  GetMaxAGI() const;
    int16  GetMaxINT() const;
    int16  GetMaxWIS() const;
    int16  GetMaxCHA() const;
	int16  GetMaxMR() const;
	int16  GetMaxPR() const;
	int16  GetMaxDR() const;
	int16  GetMaxCR() const;
	int16  GetMaxFR() const;
	int16  GetMaxCorrup() const;
	inline uint8	GetBaseSTR()	const { return m_pp.STR; }
	inline uint8	GetBaseSTA()	const { return m_pp.STA; }
	inline uint8	GetBaseCHA()	const { return m_pp.CHA; }
	inline uint8	GetBaseDEX()	const { return m_pp.DEX; }
	inline uint8	GetBaseINT()	const { return m_pp.INT; }
	inline uint8	GetBaseAGI()	const { return m_pp.AGI; }
	inline uint8	GetBaseWIS()	const { return m_pp.WIS; }
	inline uint8	GetBaseCorrup()	const { return 15; } // Same for all

	inline virtual int16	GetHeroicSTR()	const { return itembonuses.HeroicSTR; }
	inline virtual int16	GetHeroicSTA()	const { return itembonuses.HeroicSTA; }
	inline virtual int16	GetHeroicDEX()	const { return itembonuses.HeroicDEX; }
	inline virtual int16	GetHeroicAGI()	const { return itembonuses.HeroicAGI; }
	inline virtual int16	GetHeroicINT()	const { return itembonuses.HeroicINT; }
	inline virtual int16	GetHeroicWIS()	const { return itembonuses.HeroicWIS; }
	inline virtual int16	GetHeroicCHA()	const { return itembonuses.HeroicCHA; }
	inline virtual int16	GetHeroicMR()	const { return itembonuses.HeroicMR; }
	inline virtual int16	GetHeroicFR()	const { return itembonuses.HeroicFR; }
	inline virtual int16	GetHeroicDR()	const { return itembonuses.HeroicDR; }
	inline virtual int16	GetHeroicPR()	const { return itembonuses.HeroicPR; }
	inline virtual int16	GetHeroicCR()	const { return itembonuses.HeroicCR; }
	inline virtual int16	GetHeroicCorrup()	const { return itembonuses.HeroicCorrup; }
	// Mod2
	inline virtual int16	GetShielding()		const { return itembonuses.MeleeMitigation; }
	inline virtual int16	GetSpellShield()	const { return itembonuses.SpellShield; }
	inline virtual int16	GetDoTShield()		const { return itembonuses.DoTShielding; }
	inline virtual int16	GetStunResist()		const { return itembonuses.StunResist; }
	inline virtual int16	GetStrikeThrough()	const { return itembonuses.StrikeThrough; }
	inline virtual int16	GetAvoidance()		const { return itembonuses.AvoidMeleeChance; }
	inline virtual int16	GetAccuracy()		const { return itembonuses.HitChance; }
	inline virtual int16	GetCombatEffects()	const { return itembonuses.ProcChance; }
	inline virtual int16	GetDS()				const { return itembonuses.DamageShield; }
	// Mod3
	inline virtual int16	GetHealAmt()		const { return itembonuses.HealAmt; }
	inline virtual int16	GetSpellDmg()		const { return itembonuses.SpellDmg; }
	inline virtual int16	GetClair()			const { return itembonuses.Clairvoyance; }
	inline virtual int16	GetDSMit()			const { return itembonuses.DSMitigation; }
	
	inline virtual int16	GetSingMod()		const { return itembonuses.singingMod; }
	inline virtual int16	GetBrassMod()		const { return itembonuses.brassMod; }
	inline virtual int16	GetPercMod()		const { return itembonuses.percussionMod; }
	inline virtual int16	GetStringMod()		const { return itembonuses.stringedMod; }
	inline virtual int16	GetWindMod()		const { return itembonuses.windMod; }
	
	inline virtual int16	GetDelayDeath()		const { return aabonuses.DelayDeath + spellbonuses.DelayDeath + itembonuses.DelayDeath + 11; }
	
	int32 Additional_SpellDmg(uint16 spell_id, bool bufftick = false);
	int32 Additional_Heal(uint16 spell_id);
	float GetActSpellRange(uint16 spell_id, float range, bool IsBard = false);
	int32 GetActSpellDamage(uint16 spell_id, int32 value);
	int32 GetActSpellHealing(uint16 spell_id, int32 value);
	int32 GetActSpellCost(uint16 spell_id, int32);
	int32 GetActSpellDuration(uint16 spell_id, int32);
	int32 GetActSpellCasttime(uint16 spell_id, int32);
	int32 GetDotFocus(uint16 spell_id, int32 value);
	int32 GetActDoTDamage(uint16 spell_id, int32 value);
	virtual bool CheckFizzle(uint16 spell_id);
	virtual int GetCurrentBuffSlots() const;
	virtual int GetCurrentSongSlots() const;
	virtual int GetCurrentDiscSlots() const { return 1; }
	virtual int GetMaxBuffSlots() const { return 25; }
	virtual int GetMaxSongSlots() const { return 12; }
	virtual int GetMaxDiscSlots() const { return 1; }
	virtual int GetMaxTotalSlots() const { return 38; }
	virtual void InitializeBuffSlots();
	virtual void UninitializeBuffSlots();

	inline const int32	GetBaseHP() const { return base_hp; }

	uint32 GetWeight() const { return(weight); }
	inline void RecalcWeight() { weight = CalcCurrentWeight(); }
	uint32 CalcCurrentWeight();
	inline uint32	GetCopper()		const { return m_pp.copper; }
	inline uint32	GetSilver()		const { return m_pp.silver; }
	inline uint32	GetGold()		const { return m_pp.gold; }
	inline uint32	GetPlatinum()	const { return m_pp.platinum; }


	/*Endurance and such*/
	void	CalcMaxEndurance();	//This calculates the maximum endurance we can have
	int32	CalcBaseEndurance();	//Calculates Base End
	int32	CalcEnduranceRegen();	//Calculates endurance regen used in DoEnduranceRegen()
	int32	GetEndurance()	const {return cur_end;}	//This gets our current endurance
	int32	GetMaxEndurance() const {return max_end;}	//This gets our endurance from the last CalcMaxEndurance() call
	int32	CalcEnduranceRegenCap();	
	int32	CalcHPRegenCap();
	inline uint8 GetEndurancePercent() { return (uint8)((float)cur_end / (float)max_end * 100.0f); }
	void SetEndurance(int32 newEnd);	//This sets the current endurance to the new value
	void DoEnduranceRegen();	//This Regenerates endurance
	void DoEnduranceUpkeep();	//does the endurance upkeep

	//This calculates total Attack Rating to match very close to what the client should show
	uint16 GetTotalATK();
	uint16 GetATKRating();
	//This gets the skill value of the item type equiped in the Primary Slot
	uint16 GetPrimarySkillValue();

    bool Flurry();
    bool Rampage();
	void DurationRampage(uint32 duration);

	inline uint32	GetEXP()		const { return m_pp.exp; }

	bool	UpdateLDoNPoints(int32 points, uint32 theme);
	void	SetPVPPoints(uint32 Points) { m_pp.PVPCurrentPoints = Points; }
	uint32	GetPVPPoints() { return m_pp.PVPCurrentPoints; }
	void	AddPVPPoints(uint32 Points);
	uint32	GetRadiantCrystals() { return m_pp.currentRadCrystals; }
	void	SetRadiantCrystals(uint32 Crystals) { m_pp.currentRadCrystals = Crystals; }
	uint32	GetEbonCrystals() { return m_pp.currentEbonCrystals; }
	void	SetEbonCrystals(uint32 Crystals) { m_pp.currentEbonCrystals = Crystals; }
	void	AddCrystals(uint32 Radiant, uint32 Ebon);
	void	SendCrystalCounts();

	void	AddEXP(uint32 in_add_exp, uint8 conlevel = 0xFF, bool resexp = false);
	void	SetEXP(uint32 set_exp, uint32 set_aaxp, bool resexp=false);
	void	AddLevelBasedExp(uint8 exp_percentage, uint8 max_level=0);
	void	SetLeadershipEXP(uint32 group_exp, uint32 raid_exp);
	void	AddLeadershipEXP(uint32 group_exp, uint32 raid_exp);
	void	SendLeadershipEXPUpdate();
	bool	IsLeadershipEXPOn();
	inline	int GetLeadershipAA(int AAID) { return  m_pp.leader_abilities.ranks[AAID]; }
	int	GroupLeadershipAAHealthEnhancement();
	int	GroupLeadershipAAManaEnhancement();
	int	GroupLeadershipAAHealthRegeneration();
	int	GroupLeadershipAAOffenseEnhancement();
	void	InspectBuffs(Client* Inspector, int Rank);
	uint32  GetRaidPoints() { return(m_pp.raid_leadership_points); }
	uint32  GetGroupPoints() { return(m_pp.group_leadership_points); }
	uint32  GetRaidEXP() { return(m_pp.raid_leadership_exp); }
	uint32  GetGroupEXP() { return(m_pp.group_leadership_exp); }
	uint32  GetTotalSecondsPlayed() { return(TotalSecondsPlayed); }
	virtual void SetLevel(uint8 set_level, bool command = false);
	void	GoToBind(uint8 bindnum = 0);
	void	GoToSafeCoords(uint16 zone_id, uint16 instance_id);
	void	Gate();
	void	SetBindPoint(int to_zone = -1, float new_x = 0.0f, float new_y = 0.0f, float new_z = 0.0f);
	void	SetStartZone(uint32 zoneid, float x = 0.0f, float y =0.0f, float z = 0.0f);
	uint32	GetStartZone(void);
	void	MovePC(const char* zonename, float x, float y, float z, float heading, uint8 ignorerestrictions = 0, ZoneMode zm = ZoneSolicited);
	void	MovePC(uint32 zoneID, float x, float y, float z, float heading, uint8 ignorerestrictions = 0, ZoneMode zm = ZoneSolicited);
	void	MovePC(float x, float y, float z, float heading, uint8 ignorerestrictions = 0, ZoneMode zm = ZoneSolicited);
	void	MovePC(uint32 zoneID, uint32 instanceID, float x, float y, float z, float heading, uint8 ignorerestrictions = 0, ZoneMode zm = ZoneSolicited);
	void	AssignToInstance(uint16 instance_id);
	void	WhoAll();
	bool	CheckLoreConflict(const Item_Struct* item);
	void	ChangeLastName(const char* in_lastname);
	void	GetGroupAAs(GroupLeadershipAA_Struct *into) const;
	void	ClearGroupAAs();
	void	UpdateGroupAAs(int32 points, uint32 type);
	void	SacrificeConfirm(Client* caster);
	void	Sacrifice(Client* caster);
	void	GoToDeath();
	inline const int32	GetInstanceID() const { return zone->GetInstanceID(); }

	FACTION_VALUE	GetReverseFactionCon(Mob* iOther);
    FACTION_VALUE   GetFactionLevel(uint32 char_id, uint32 npc_id, uint32 p_race, uint32 p_class, uint32 p_deity, int32 pFaction, Mob* tnpc);
	int32	GetCharacterFactionLevel(int32 faction_id);
	int32  GetModCharacterFactionLevel(int32 faction_id);
	bool	HatedByClass(uint32 p_race, uint32 p_class, uint32 p_deity, int32 pFaction);
    char* BuildFactionMessage(int32 tmpvalue, int32 faction_id, int32 totalvalue, uint8 temp);

	void	SetFactionLevel(uint32 char_id, uint32 npc_id, uint8 char_class, uint8 char_race, uint8 char_deity);
	void    SetFactionLevel2(uint32 char_id, int32 faction_id, uint8 char_class, uint8 char_race, uint8 char_deity, int32 value, uint8 temp);
	int16	GetRawItemAC();
	uint16	GetCombinedAC_TEST();

	inline uint32	LSAccountID()	const { return lsaccountid; }
	inline uint32	GetWID()		const { return WID; }
	inline void		SetWID(uint32 iWID) { WID = iWID; }
	inline uint32	AccountID()		const { return account_id; }
	inline const char* AccountName()const { return account_name; }
	inline int16	Admin()			const { return admin; }
	inline uint32	CharacterID()	const { return character_id; }
	void	UpdateAdmin(bool iFromDB = true);
	void	UpdateWho(uint8 remove = 0);
	bool	GMHideMe(Client* client = 0);

	inline bool IsInAGuild() const { return(guild_id != GUILD_NONE && guild_id != 0); }
	inline bool IsInGuild(uint32 in_gid) const { return(in_gid == guild_id && IsInAGuild()); }
	inline uint32	GuildID() const { return guild_id; }
	inline uint8	GuildRank()		const { return guildrank; }
	void	SendGuildMOTD(bool GetGuildMOTDReply = false);
	void	SendGuildURL();
	void	SendGuildChannel();
	void	SendGuildSpawnAppearance();
	void	SendGuildMembers();
    void	SendGuildList();
	void	SendGuildJoin(GuildJoin_Struct* gj);
	void	RefreshGuildInfo();


	void	SendManaUpdatePacket();
	void	SendManaUpdate();
	void	SendEnduranceUpdate();
	uint8	GetFace()		const { return m_pp.face; }
	void	WhoAll(Who_All_Struct* whom);
    void	FriendsWho(char *FriendsString);

	void	Stun(int duration);
	void	UnStun();
	void	ReadBook(BookRequest_Struct *book);
	void	QuestReadBook(const char* text, uint8 type);
	void	SendClientMoneyUpdate(uint8 type,uint32 amount);
	void	SendMoneyUpdate();
	bool	TakeMoneyFromPP(uint64 copper, bool updateclient=false);
	void	AddMoneyToPP(uint64 copper,bool updateclient);
	void	AddMoneyToPP(uint32 copper, uint32 silver, uint32 gold,uint32 platinum,bool updateclient);
	bool	HasMoney(uint64 copper);
	uint64	GetCarriedMoney();
	uint64	GetAllMoney();

	bool	IsDiscovered(uint32 itemid);
	void	DiscoverItem(uint32 itemid);

	bool	TGB() const { return tgb; }

	void	OnDisconnect(bool hard_disconnect);

	uint16	GetSkillPoints() {return m_pp.points;}
	void	SetSkillPoints(int inp) {m_pp.points = inp;}

	void	IncreaseSkill(int skill_id, int value = 1) { if (skill_id <= HIGHEST_SKILL) { m_pp.skills[skill_id] += value; } }
	void	IncreaseLanguageSkill(int skill_id, int value = 1);
	virtual uint16 GetSkill(SkillType skill_id) const { if (skill_id <= HIGHEST_SKILL) { return((itembonuses.skillmod[skill_id] > 0)? m_pp.skills[skill_id]*(100 + itembonuses.skillmod[skill_id])/100 : m_pp.skills[skill_id]); } return 0; }
	uint32	GetRawSkill(SkillType skill_id) const { if (skill_id <= HIGHEST_SKILL) { return(m_pp.skills[skill_id]); } return 0; }
	bool	HasSkill(SkillType skill_id) const;
	bool	CanHaveSkill(SkillType skill_id) const;
	void	SetSkill(SkillType skill_num, uint16 value);
	void	AddSkill(SkillType skillid, uint16 value);
	void	CheckSpecializeIncrease(uint16 spell_id);
	void	CheckSongSkillIncrease(uint16 spell_id);
	bool	CheckIncreaseSkill(SkillType skillid, Mob *against_who, int chancemodi = 0);
	void	CheckLanguageSkillIncrease(uint8 langid, uint8 TeacherSkill);
	void    SetLanguageSkill(int langid, int value);
	void	SetHoTT(uint32 mobid);
	void	ShowSkillsWindow();
	void	SendStatsWindow(Client* client, bool use_window);

	uint16	MaxSkill(SkillType skillid, uint16 class_, uint16 level) const;
	inline	uint16	MaxSkill(SkillType skillid) const { return MaxSkill(skillid, GetClass(), GetLevel()); }
	uint8	SkillTrainLevel(SkillType skillid, uint16 class_);

	void TradeskillSearchResults(const char *query, unsigned long qlen, unsigned long objtype, unsigned long someid);
	void SendTradeskillDetails(uint32 recipe_id);
	bool TradeskillExecute(DBTradeskillRecipe_Struct *spec);
	void CheckIncreaseTradeskill(int16 bonusstat, int16 stat_modifier, float skillup_modifier, uint16 success_modifier, SkillType tradeskill);

	void GMKill();
	inline bool	IsMedding()	const {return medding;}
	inline uint16 GetDuelTarget() const { return duel_target; }
	inline bool	IsDueling() const { return duelaccepted; }
	inline void	SetDuelTarget(uint16 set_id) { duel_target=set_id; }
	inline void	SetDueling(bool duel) { duelaccepted = duel; }
	// use this one instead
	void MemSpell(uint16 spell_id, int slot, bool update_client = true);
	void UnmemSpell(int slot, bool update_client = true);
	void UnmemSpellAll(bool update_client = true);
	void ScribeSpell(uint16 spell_id, int slot, bool update_client = true);
	void UnscribeSpell(int slot, bool update_client = true);
	void UnscribeSpellAll(bool update_client = true);
	void UntrainDisc(int slot, bool update_client = true);
	void UntrainDiscAll(bool update_client = true);
	bool SpellGlobalCheck(uint16 Spell_ID, uint16 Char_ID);
	uint32 GetCharMaxLevelFromQGlobal();

	inline bool	IsSitting() const {return (playeraction == 1);}
	inline bool	IsBecomeNPC() const { return npcflag; }
	inline uint8	GetBecomeNPCLevel() const { return npclevel; }
	inline void	SetBecomeNPC(bool flag) { npcflag = flag; }
	inline void	SetBecomeNPCLevel(uint8 level) { npclevel = level; }
	bool LootToStack(uint32 itemid);
	void SetFeigned(bool in_feigned);
	/// this cures timing issues cuz dead animation isn't done but server side feigning is?
	inline bool    GetFeigned()	const { return(feigned); }
	EQStreamInterface* Connection() { return eqs; }
#ifdef PACKET_PROFILER
	void DumpPacketProfile() { if(eqs) eqs->DumpPacketProfile(); }
#endif
	uint32 GetEquipment(uint8 material_slot) const;	// returns item id
	uint32 GetEquipmentColor(uint8 material_slot) const;

	inline bool AutoSplitEnabled() { return m_pp.autosplit != 0; }

	void SummonHorse(uint16 spell_id);
	void SetHorseId(uint16 horseid_in);
	uint16 GetHorseId() const { return horseId; }

	void NPCSpawn(NPC *target_npc, const char *identifier, uint32 extra = 0);

	bool BindWound(Mob* bindmob, bool start, bool fail = false);
	void SetTradeskillObject(Object* object) { m_tradeskill_object = object; }
	Object* GetTradeskillObject() { return m_tradeskill_object; }
	void	SendTributes();
	void	SendGuildTributes();
	void	DoTributeUpdate();
	void	SendTributeDetails(uint32 client_id, uint32 tribute_id);
	int32	TributeItem(uint32 slot, uint32 quantity);
	int32	TributeMoney(uint32 platinum);
	void	AddTributePoints(int32 ammount);
	void	ChangeTributeSettings(TributeInfo_Struct *t);
	void	SendTributeTimer();
	void	ToggleTribute(bool enabled);
	void	SendPathPacket(vector<FindPerson_Point> &path);

	inline PTimerList &GetPTimers() { return(p_timers); }

	//AA Methods
	void  SendAAList();
	void  ResetAA();
	void  SendAA(uint32 id, int seq=1);
	void  SendPreviousAA(uint32 id, int seq=1);
	void  BuyAA(AA_Action* action);
	//this function is used by some AA stuff
	void MemorizeSpell(uint32 slot,uint32 spellid,uint32 scribing);
	void	SetAATitle(const char *Title);
	void	SetTitleSuffix(const char *txt);
	inline uint32	GetMaxAAXP(void) const { return max_AAXP; }
	inline uint32  GetAAXP()   const { return m_pp.expAA; }
	void SendAAStats();
	void SendAATable();
	void SendAATimers();
	int  GetAATimerID(aaID activate);
	int  CalcAAReuseTimer(const AA_DBAction *caa);
	void ActivateAA(aaID activate);
	void SendAATimer(uint32 ability, uint32 begin, uint32 end);
	void EnableAAEffect(aaEffectType type, uint32 duration = 0);
	void DisableAAEffect(aaEffectType type);
	bool CheckAAEffect(aaEffectType type);
	void HandleAAAction(aaID activate);
	uint32 GetAA(uint32 aa_id) const;
	bool SetAA(uint32 aa_id, uint32 new_value);
	inline uint32 GetAAPointsSpent() { return m_pp.aapoints_spent; }
	int16 CalcAAFocusEffect(focusType type, uint16 focus_spell, uint16 spell_id);
	int16 CalcAAFocus(focusType type, uint32 aa_ID, uint16 spell_id);

	int16 acmod();

	// Item methods
	uint32	NukeItem(uint32 itemnum, uint8 where_to_check = 
        (invWhereWorn | invWherePersonal | invWhereBank | invWhereSharedBank | invWhereTrading | invWhereCursor));
	void	SetTint(int16 slot_id, uint32 color);
	void	SetTint(int16 slot_id, Color_Struct& color);
	void	SetMaterial(int16 slot_id, uint32 item_id);
	void	Undye();
	uint32	GetItemIDAt(int16 slot_id);
	uint32	GetAugmentIDAt(int16 slot_id, uint8 augslot);
	bool	PutItemInInventory(int16 slot_id, const ItemInst& inst, bool client_update = false);
	bool	PushItemOnCursor(const ItemInst& inst, bool client_update = false);
	void	DeleteItemInInventory(int16 slot_id, int8 quantity = 0, bool client_update = false, bool update_db = true);
	bool	SwapItem(MoveItem_Struct* move_in);
	void	SwapItemResync(MoveItem_Struct* move_slots);
	void	QSSwapItemAuditor(MoveItem_Struct* move_in, bool postaction_call = false);
	void	PutLootInInventory(int16 slot_id, const ItemInst &inst, ServerLootItem_Struct** bag_item_data = 0);
	bool	AutoPutLootInInventory(ItemInst& inst, bool try_worn = false, bool try_cursor = true, ServerLootItem_Struct** bag_item_data = 0);
	void	SummonItem(uint32 item_id, int16 charges = 0, uint32 aug1=0, uint32 aug2=0, uint32 aug3=0, uint32 aug4=0, uint32 aug5=0, bool attuned=false, uint16 to_slot=SLOT_CURSOR);
	void	SetStats(uint8 type,int16 set_val);
	void	IncStats(uint8 type,int16 increase_val);
	void	DropItem(int16 slot_id);
	bool	MakeItemLink(char* &ret_link, const ItemInst* inst);
	int		GetItemLinkHash(const ItemInst* inst);
	void	SendItemLink(const ItemInst* inst, bool sendtoall=false);
	void	SendLootItemInPacket(const ItemInst* inst, int16 slot_id);
	void	SendItemPacket(int16 slot_id, const ItemInst* inst, ItemPacketType packet_type);
	bool	IsValidSlot(uint32 slot);
	bool    IsBankSlot(uint32 slot);

	inline	bool IsTrader() const { return(Trader); }
	inline	bool IsBuyer() const { return(Buyer); }
	eqFilterMode	GetFilter(eqFilterType filter_id) const { return ClientFilters[filter_id]; }
	void	SetFilter(eqFilterType filter_id, eqFilterMode value) { ClientFilters[filter_id]=value; }

	void	BreakInvis();
	void	LeaveGroup();

	bool	Hungry() const {if (GetGM()) return false; return m_pp.hunger_level <= 3000;}
	bool	Thirsty() const {if (GetGM()) return false; return m_pp.thirst_level <= 3000;}

	bool	CheckTradeLoreConflict(Client* other);
	void	LinkDead();
	void	Insight(uint32 t_id);
    bool	CheckDoubleAttack(bool tripleAttack = false);

	//remove charges/multiple objects from inventory:
	//bool	DecreaseByType(uint32 type, uint8 amt);
	bool	DecreaseByID(uint32 type, uint8 amt);
	uint8	SlotConvert2(uint8 slot);	//Maybe not needed.
	void	Escape(); //AA Escape
	void    RemoveNoRent(bool client_update = true);
	void	RemoveDuplicateLore(bool client_update = true);
	void	MoveSlotNotAllowed(bool client_update = true);
	virtual void	RangedAttack(Mob* other);
	virtual void	ThrowingAttack(Mob* other);
	void	DoClassAttacks(Mob *ca_target, uint16 skill = -1, bool IsRiposte=false);

	void	SetZoneFlag(uint32 zone_id);
	void	ClearZoneFlag(uint32 zone_id);
	bool	HasZoneFlag(uint32 zone_id) const;
	void	SendZoneFlagInfo(Client *to) const;
	void	LoadZoneFlags();

	void	ChangeSQLLog(const char *file);
	void	LogSQL(const char *fmt, ...);
	bool	CanFish();
	void	GoFish();
	void	ForageItem(bool guarantee = false);
	//Calculate vendor price modifier based on CHA: (reverse==selling)
	float	CalcPriceMod(Mob* other = 0, bool reverse = false);
	void	ResetTrade();
	void	DropInst(const ItemInst* inst);
	bool	TrainDiscipline(uint32 itemid);
	void	SendDisciplineUpdate();
	bool	UseDiscipline(uint32 spell_id, uint32 target);

   	bool   CheckTitle(int titleset);
   	void   EnableTitle(int titleset);
   	void   RemoveTitle(int titleset);

#ifdef PACKET_UPDATE_MANAGER
	inline UpdateManager *GetUpdateManager() { return(&update_manager); }
#endif
	void	EnteringMessages(Client* client);
	void	SendRules(Client* client);
	std::list<std::string> consent_list;

	//Anti-Cheat Stuff
	uint32 m_TimeSinceLastPositionCheck;
	float m_DistanceSinceLastPositionCheck;
	bool m_CheatDetectMoved;
	void SetShadowStepExemption(bool v);
	void SetKnockBackExemption(bool v);
	void SetPortExemption(bool v);
	void SetSenseExemption(bool v) { m_SenseExemption = v; }
	const bool IsShadowStepExempted() const { return m_ShadowStepExemption; }
	const bool IsKnockBackExempted() const { return m_KnockBackExemption; }
	const bool IsPortExempted() const { return m_PortExemption; }
	const bool IsSenseExempted() const { return m_SenseExemption; }
	const bool GetGMSpeed() const { return (gmspeed > 0); }
	void CheatDetected(CheatTypes CheatType, float x, float y, float z);
	const bool IsMQExemptedArea(uint32 zoneID, float x, float y, float z) const;
	bool CanUseReport;

	//This is used to later set the buff duration of the spell, in slot to duration.
	//Doesn't appear to work directly after the client recieves an action packet.
	void SendBuffDurationPacket(uint16 spell_id, int duration, int inlevel);

	void	ProcessInspectRequest(Client* requestee, Client* requester);
	bool	ClientFinishedLoading() { return (conn_state == ClientConnectFinished); }
	int		FindSpellBookSlotBySpellID(uint16 spellid);
	int		GetNextAvailableSpellBookSlot(int starting_slot = 0);
	inline uint32 GetSpellByBookSlot(int book_slot) { return m_pp.spell_book[book_slot]; }
	inline bool HasSpellScribed(int spellid) { return (FindSpellBookSlotBySpellID(spellid) != -1 ? true : false); }
	uint16	GetMaxSkillAfterSpecializationRules(SkillType skillid, uint16 maxSkill);
	void	SendPopupToClient(const char *Title, const char *Text, uint32 PopupID = 0, uint32 Buttons = 0, uint32 Duration = 0);
	void	SendWindow(uint32 PopupID, uint32 NegativeID, uint32 Buttons, const char *ButtonName0, const char *ButtonName1, uint32 Duration, int title_type, Client* target, const char *Title, const char *Text, ...);
	bool	PendingTranslocate;
	time_t	TranslocateTime;
 	bool	PendingSacrifice;
 	string	SacrificeCaster;
 	struct	Translocate_Struct PendingTranslocateData;
 	void	SendOPTranslocateConfirm(Mob *Caster, uint16 SpellID);

	//      Task System Methods
	void	LoadClientTaskState();
	void	RemoveClientTaskState();
	void 	SendTaskActivityComplete(int TaskID, int ActivityID, int TaskIndex, int TaskIncomplete=1);
	void 	SendTaskFailed(int TaskID, int TaskIndex);
	void 	SendTaskComplete(int TaskIndex);

	inline void CancelTask(int TaskIndex) { if(taskstate) taskstate->CancelTask(this, TaskIndex); }

	inline bool SaveTaskState()
		    { return (taskmanager ? taskmanager->SaveClientState(this, taskstate) : false); }

	inline bool IsTaskStateLoaded() { return taskstate != nullptr; }

	inline bool IsTaskActive(int TaskID)
		    { return (taskstate ? taskstate->IsTaskActive(TaskID) : false); }

	inline bool IsTaskActivityActive(int TaskID, int ActivityID)
	            { return (taskstate ? taskstate->IsTaskActivityActive(TaskID, ActivityID) : false); }

	inline ActivityState GetTaskActivityState(int index, int ActivityID)
	      		     { return (taskstate ? taskstate->GetTaskActivityState(index, ActivityID) : ActivityHidden); }

	inline void UpdateTaskActivity(int TaskID, int ActivityID, int Count)
		    { if(taskstate) taskstate->UpdateTaskActivity(this, TaskID, ActivityID, Count); }

	inline void ResetTaskActivity(int TaskID, int ActivityID)
		    { if(taskstate) taskstate->ResetTaskActivity(this, TaskID, ActivityID); }

	inline void UpdateTasksOnKill(int NPCTypeID)
	            { if(taskstate) taskstate->UpdateTasksOnKill(this, NPCTypeID); }

	inline void UpdateTasksForItem(ActivityType Type, int ItemID, int Count=1)
	            { if(taskstate) taskstate->UpdateTasksForItem(this, Type, ItemID, Count); }

	inline void UpdateTasksOnExplore(int ExploreID)
	            { if(taskstate) taskstate->UpdateTasksOnExplore(this, ExploreID); }

	inline bool UpdateTasksOnSpeakWith(int NPCTypeID)
	            { if(taskstate) return taskstate->UpdateTasksOnSpeakWith(this, NPCTypeID); else return false; }

	inline bool UpdateTasksOnDeliver(uint32 *Items, int Cash, int NPCTypeID)
	            { if(taskstate) return taskstate->UpdateTasksOnDeliver(this, Items, Cash, NPCTypeID); else return false; }

	inline void TaskSetSelector(Mob *mob, int TaskSetID)
	            { if(taskmanager) taskmanager->TaskSetSelector(this, taskstate, mob, TaskSetID); }

	inline void EnableTask(int TaskCount, int *TaskList)
	            { if(taskstate) taskstate->EnableTask(CharacterID(), TaskCount, TaskList); }

	inline void DisableTask(int TaskCount, int *TaskList)
	            { if(taskstate) taskstate->DisableTask(CharacterID(), TaskCount, TaskList); }

	inline bool IsTaskEnabled(int TaskID)
	            { return (taskstate ? taskstate->IsTaskEnabled(TaskID) : false); }

	inline void ProcessTaskProximities(float X, float Y, float Z)
	            { if(taskstate) taskstate->ProcessTaskProximities(this, X, Y, Z); }

	inline void AssignTask(int TaskID, int NPCID)
	            { if(taskstate) taskstate->AcceptNewTask(this, TaskID, NPCID); }

	inline int ActiveSpeakTask(int NPCID)
	            { if(taskstate) return taskstate->ActiveSpeakTask(NPCID); else return 0; }

	inline int ActiveSpeakActivity(int NPCID, int TaskID)
	            { if(taskstate) return taskstate->ActiveSpeakActivity(NPCID, TaskID); else return 0; }

	inline void FailTask(int TaskID)
	            { if(taskstate) taskstate->FailTask(this, TaskID); }

	inline int TaskTimeLeft(int TaskID)
	            { return (taskstate ? taskstate->TaskTimeLeft(TaskID) : 0); }

	inline int EnabledTaskCount(int TaskSetID)
	            { return (taskstate ? taskstate->EnabledTaskCount(TaskSetID) : -1); }

	inline int IsTaskCompleted(int TaskID)
	            { return (taskstate ? taskstate->IsTaskCompleted(TaskID) : -1); }

	inline void ShowClientTasks()
	            { if(taskstate) taskstate->ShowClientTasks(this); }

	inline void CancelAllTasks()
	            { if(taskstate) taskstate->CancelAllTasks(this); }

	inline int GetActiveTaskCount()
		   { return (taskstate ? taskstate->GetActiveTaskCount() : 0); }

	inline int GetActiveTaskID(int index)
		   { return (taskstate ? taskstate->GetActiveTaskID(index) : -1); }

	inline int GetTaskStartTime(int index)
		   { return (taskstate ? taskstate->GetTaskStartTime(index) : -1); }

	inline bool IsTaskActivityCompleted(int index, int ActivityID)
		    { return (taskstate ? taskstate->IsTaskActivityCompleted(index, ActivityID) : false); }

	inline int GetTaskActivityDoneCount(int ClientTaskIndex, int ActivityID)
	 	   { return (taskstate ? taskstate->GetTaskActivityDoneCount(ClientTaskIndex, ActivityID) :0); }
	
	inline int GetTaskActivityDoneCountFromTaskID(int TaskID, int ActivityID)
	 	   { return (taskstate ? taskstate->GetTaskActivityDoneCountFromTaskID(TaskID, ActivityID) :0); }

	inline int ActiveTasksInSet(int TaskSet)
	 	   { return (taskstate ? taskstate->ActiveTasksInSet(TaskSet) :0); }

	inline int CompletedTasksInSet(int TaskSet)
	 	   { return (taskstate ? taskstate->CompletedTasksInSet(TaskSet) :0); }

	inline const EQClientVersion GetClientVersion() const { return ClientVersion; }
	inline const uint32 GetClientVersionBit() const { return ClientVersionBit; }

	/** Adventure Stuff **/
	void SendAdventureError(const char *error);
	void SendAdventureDetails();
	void SendAdventureCount(uint32 count, uint32 total);
	void NewAdventure(int id, int theme, const char *text, int member_count, const char *members);
	bool IsOnAdventure();
	void LeaveAdventure();
	void AdventureFinish(bool win, int theme, int points);
	void SetAdventureData(char *data) { adv_data = data; }
	void ClearAdventureData() { safe_delete(adv_data); }
	bool HasAdventureData() { return adv_data != nullptr; }
	void ClearCurrentAdventure();
	void PendingAdventureRequest() { adventure_request_timer = new Timer(8000); }
	bool GetPendingAdventureRequest() const { return (adventure_request_timer != nullptr); }
	void ClearPendingAdventureRequest() { safe_delete(adventure_request_timer); }
	void PendingAdventureCreate() { adventure_create_timer = new Timer(8000); }
	bool GetPendingAdventureCreate() const { return (adventure_create_timer != nullptr); }
	void ClearPendingAdventureCreate() { safe_delete(adventure_create_timer); }
	void PendingAdventureLeave() { adventure_leave_timer = new Timer(8000); }
	bool GetPendingAdventureLeave() const { return (adventure_leave_timer != nullptr); }
	void ClearPendingAdventureLeave() { safe_delete(adventure_leave_timer); }
	void PendingAdventureDoorClick() { adventure_door_timer = new Timer(8000); }
	bool GetPendingAdventureDoorClick() const { return (adventure_door_timer != nullptr); }
	void ClearPendingAdventureDoorClick() { safe_delete(adventure_door_timer); }
	void ClearPendingAdventureData();

	int GetAggroCount();
	void IncrementAggroCount();
	void DecrementAggroCount();
	void SendPVPStats();
	void SendDisciplineTimers();
	void SendRespawnBinds();

	uint32 GetLDoNWins() { return (m_pp.ldon_wins_guk + m_pp.ldon_wins_mir + m_pp.ldon_wins_mmc + m_pp.ldon_wins_ruj + m_pp.ldon_wins_tak); }
	uint32 GetLDoNLosses() { return (m_pp.ldon_losses_guk + m_pp.ldon_losses_mir + m_pp.ldon_losses_mmc + m_pp.ldon_losses_ruj + m_pp.ldon_losses_tak); }
	uint32 GetLDoNWinsTheme(uint32 t);
	uint32 GetLDoNLossesTheme(uint32 t);
	uint32 GetLDoNPointsTheme(uint32 t);
	void UpdateLDoNWins(uint32 t, int32 n);
	void UpdateLDoNLosses(uint32 t, int32 n);
	void CheckLDoNHail(Mob *target);
	void CheckEmoteHail(Mob *target, const char* message);

	void HandleLDoNOpen(NPC *target);
	void HandleLDoNSenseTraps(NPC *target, uint16 skill, uint8 type);
	void HandleLDoNDisarm(NPC *target, uint16 skill, uint8 type);
	void HandleLDoNPickLock(NPC *target, uint16 skill, uint8 type);
	int	LDoNChest_SkillCheck(NPC *target, int skill);

	void MarkSingleCompassLoc(float in_x, float in_y, float in_z, uint8 count=1);

	void CalcItemScale(bool login = false);
	bool CalcItemScale(uint32 slot_x, uint32 slot_y, bool login = false);
	void SummonAndRezzAllCorpses();
	void SummonAllCorpses(float dest_x, float dest_y, float dest_z, float dest_heading);
	void DepopAllCorpses();
	void DepopPlayerCorpse(uint32 dbid);
	void BuryPlayerCorpses();
	void SuspendMinion();
	void Doppelganger(uint16 spell_id, Mob *target, const char *name_override, int pet_count, int pet_duration);
	void NotifyNewTitlesAvailable();
	void Signal(uint32 data);
	Mob *GetBindSightTarget() { return bind_sight_target; }
	void SetBindSightTarget(Mob *n) { bind_sight_target = n; }
	const uint16 GetBoatID() const { return BoatID; }
	void SendRewards();
	bool TryReward(uint32 claim_id);
	QGlobalCache *GetQGlobals() { return qGlobals; }
	QGlobalCache *CreateQGlobals() { qGlobals = new QGlobalCache(); return qGlobals; }
	void GuildBankAck();
	void GuildBankDepositAck(bool Fail);
	inline bool IsGuildBanker() { return GuildBanker; }
	void ClearGuildBank();
	void SendGroupCreatePacket();
	void SendGroupLeaderChangePacket(const char *LeaderName);
	void SendGroupJoinAcknowledge();
	void DoTracking();
	inline bool IsTracking() { return (TrackingID > 0); }
	inline void SetPendingGuildInvitation(bool inPendingGuildInvitation) { PendingGuildInvitation = inPendingGuildInvitation; }
	inline bool GetPendingGuildInvitation() { return PendingGuildInvitation; }
	void LocateCorpse();
	void SendTargetCommand(uint32 EntityID);
	bool	MoveItemToInventory(ItemInst *BInst, bool UpdateClient = false);
	void HandleRespawnFromHover(uint32 Option);
	bool IsHoveringForRespawn() { return RespawnFromHoverTimer.Enabled(); }
	void SetPendingRezzData(int XP, uint32 DBID, uint16 SpellID, const char *CorpseName) { PendingRezzXP = XP; PendingRezzDBID = DBID; PendingRezzSpellID = SpellID; PendingRezzCorpseName = CorpseName; }
	bool IsRezzPending() { return PendingRezzSpellID > 0; }
	void ClearHover();
	inline bool IsBlockedBuff(int16 SpellID) { return PlayerBlockedBuffs.find(SpellID) != PlayerBlockedBuffs.end(); }
	inline bool IsBlockedPetBuff(int16 SpellID) { return PetBlockedBuffs.find(SpellID) != PetBlockedBuffs.end(); }
	bool IsDraggingCorpse(const char* CorpseName);
	inline bool IsDraggingCorpse() { return (DraggedCorpses.size() > 0); }
	void DragCorpses();
	inline void ClearDraggedCorpses() { DraggedCorpses.clear(); }
	void SendAltCurrencies();
	void SetAlternateCurrencyValue(uint32 currency_id, uint32 new_amount);
	void AddAlternateCurrencyValue(uint32 currency_id, int32 amount);
	void SendAlternateCurrencyValues();
	void SendAlternateCurrencyValue(uint32 currency_id, bool send_if_null = true);
	uint32 GetAlternateCurrencyValue(uint32 currency_id) const;
	void OpenLFGuildWindow();
	void HandleLFGuildResponse(ServerPacket *pack);
	void SendLFGuildStatus();
	void SendGuildLFGuildStatus();
	inline bool XTargettingAvailable() const { return ((ClientVersionBit & BIT_UnderfootAndLater) && RuleB(Character, EnableXTargetting)); }
	inline uint8 GetMaxXTargets() const { return MaxXTargets; }
	void SetMaxXTargets(uint8 NewMax);
	bool IsXTarget(const Mob *m) const;
	bool IsClientXTarget(const Client *c) const;
	void UpdateClientXTarget(Client *c);
	void UpdateXTargetType(XTargetType Type, Mob *m, const char *Name = nullptr);
	void AddAutoXTarget(Mob *m);
	void RemoveXTarget(Mob *m, bool OnlyAutoSlots);
	void SendXTargetPacket(uint32 Slot, Mob *m);
	void RemoveGroupXTargets();
	void RemoveAutoXTargets();
	void ShowXTargets(Client *c);
	void InitializeMercInfo();
	bool CheckCanHireMerc(Mob* merchant, uint32 template_id);
	bool CheckCanRetainMerc(uint32 upkeep);
	bool CheckCanUnsuspendMerc();
	bool CheckCanDismissMerc();
	inline uint32 GetMercID()	const { return mercid; }
	inline uint8 GetMercSlot()	const { return mercSlot; }
	void SetMercID( uint32 newmercid) { mercid = newmercid; }
	void SetMercSlot( uint8 newmercslot) { mercSlot = newmercslot; }
	Merc* GetMerc();
	MercInfo& GetMercInfo(uint8 slot) { return m_mercinfo[slot]; }
	MercInfo& GetMercInfo() { return m_mercinfo[mercSlot]; }
	uint8 GetNumMercs();
	void SetMerc(Merc* newmerc);
	void SendMercMerchantResponsePacket(int32 response_type);
	void SendMercenaryUnknownPacket(uint8 type);
	void SendMercenaryUnsuspendPacket(uint8 type);
	void SendMercTimerPacket(int32 entity_id, int32 merc_state, int32 suspended_time, int32 update_interval = 900000, int32 unk01 = 180000);
	void SendMercSuspendResponsePacket(uint32 suspended_time);
	void SendMercAssignPacket(uint32 entityID, uint32 unk01, uint32 unk02);
	void SendMercPersonalInfo();
	void SendClearMercInfo();
	void SuspendMercCommand();
	void SpawnMercOnZone();
	void SpawnMerc(Merc* merc, bool setMaxStats);
	void UpdateMercTimer();
	void UpdateMercLevel();
	void CheckMercSuspendTimer();
	Timer* GetMercTimer() { return &merc_timer; };
	const char* GetRacePlural(Client* client);
	const char* GetClassPlural(Client* client);
	void  SendWebLink(const char* website);

	bool StoreTurnInItems(Mob* with);
	void DuplicateLoreMessage(uint32 ItemID);
	void GarbleMessage(char *, uint8);

    void TickItemCheck();
	void TryItemTick(int slot);
	int16 GetActSTR() { return( min(GetMaxSTR(), GetSTR()) ); }
	int16 GetActSTA() { return( min(GetMaxSTA(), GetSTA()) ); }
	int16 GetActDEX() { return( min(GetMaxDEX(), GetDEX()) ); }
	int16 GetActAGI() { return( min(GetMaxAGI(), GetAGI()) ); }
	int16 GetActINT() { return( min(GetMaxINT(), GetINT()) ); }
	int16 GetActWIS() { return( min(GetMaxWIS(), GetWIS()) ); }
	int16 GetActCHA() { return( min(GetMaxCHA(), GetCHA()) ); }
    void LoadAccountFlags();
    void SetAccountFlag(std::string flag, std::string val);
    std::string GetAccountFlag(std::string flag);    float GetDamageMultiplier(SkillType);
	int mod_client_damage(int damage, SkillType skillinuse, int hand, ItemInst* weapon, Mob* other);
	bool mod_client_message(char* message, uint8 chan_num);
	bool mod_can_increase_skill(SkillType skillid, Mob* against_who);
	int16 mod_increase_skill_chance(int16 chance, Mob* against_who);
	int mod_bindwound_percent(int max_percent, Mob* bindmob);
	int mod_bindwound_hp(int bindhps, Mob* bindmob);
	int mod_client_haste(int h);
	void mod_consider(Mob* tmob, Consider_Struct* con);
	bool mod_saylink(const std::string&, bool silentsaylink);
	int16 mod_pet_power(int16 act_power, uint16 spell_id);
	float mod_tradeskill_chance(float chance, DBTradeskillRecipe_Struct *spec);
	float mod_tradeskill_skillup(float chance_stage2);
	int32 mod_tribute_item_value(int32 pts);
	void mod_client_death_npc(Mob* killerMob);
	void mod_client_death_duel(Mob* killerMob);
	void mod_client_death_env();

protected:
	friend class Mob;
	void CalcItemBonuses(StatBonuses* newbon);
	void AddItemBonuses(const ItemInst *inst, StatBonuses* newbon, bool isAug = false, bool isTribute = false);
	int  CalcRecommendedLevelBonus(uint8 level, uint8 reclevel, int basestat);
	void CalcEdibleBonuses(StatBonuses* newbon);
	void CalcAABonuses(StatBonuses* newbon);
	void ApplyAABonuses(uint32 aaid, uint32 slots, StatBonuses* newbon); 
	void MakeBuffFadePacket(uint16 spell_id, int slot_id, bool send_message = true);
	bool client_data_loaded;

	int16	GetFocusEffect(focusType type, uint16 spell_id);
	int16	GetSympatheticFocusEffect(focusType type, uint16 spell_id); 

	Mob*	bind_sight_target;

	VERTEX aa_los_me;
	VERTEX aa_los_them;
	Mob *aa_los_them_mob;
	bool los_status;
	QGlobalCache *qGlobals;

	/** Adventure Variables **/
	Timer *adventure_request_timer;
	Timer *adventure_create_timer;
	Timer *adventure_leave_timer;
	Timer *adventure_door_timer;
	Timer *adventure_stats_timer;
	Timer *adventure_leaderboard_timer;
	int adv_requested_theme;
	int adv_requested_id;
	char *adv_requested_data;
	int adv_requested_member_count;
	char *adv_data;

private:
	eqFilterMode ClientFilters[_FilterCount];
	int32	HandlePacket(const EQApplicationPacket *app);
	void	OPTGB(const EQApplicationPacket *app);
	void	OPRezzAnswer(uint32 Action, uint32 SpellID, uint16 ZoneID, uint16 InstanceID, float x, float y, float z);
	void	OPMemorizeSpell(const EQApplicationPacket *app);
	void	OPMoveCoin(const EQApplicationPacket* app);
	void	MoveItemCharges(ItemInst &from, int16 to_slot, uint8 type);
	void	OPGMTraining(const EQApplicationPacket *app);
	void	OPGMEndTraining(const EQApplicationPacket *app);
	void	OPGMTrainSkill(const EQApplicationPacket *app);
	void	OPGMSummon(const EQApplicationPacket *app);
	void	OPCombatAbility(const EQApplicationPacket *app);

	// Bandolier Methods
	void	CreateBandolier(const EQApplicationPacket *app);
	void	RemoveBandolier(const EQApplicationPacket *app);
	void	SetBandolier(const EQApplicationPacket *app);

	void	HandleTraderPriceUpdate(const EQApplicationPacket *app);

	int16    CalcAC();
	int16    GetACMit();
	int16    GetACAvoid();
	int16    CalcATK();
	int      CalcHaste();

	int16   CalcAlcoholPhysicalEffect();
	int16   CalcSTR();
	int16   CalcSTA();
	int16   CalcDEX();
	int16   CalcAGI();
	int16   CalcINT();
	int16   CalcWIS();
	int16   CalcCHA();

    int16	CalcMR();
	int16	CalcFR();
	int16	CalcDR();
	int16	CalcPR();
	int16	CalcCR();
	int16	CalcCorrup();
	int32	CalcMaxHP();
	int32	CalcBaseHP();
	int32	CalcHPRegen();
	int32	CalcManaRegen();
	int32	CalcBaseManaRegen();
	uint32	GetClassHPFactor();
	void DoHPRegen();
	void DoManaRegen();
	void DoStaminaUpdate();
	void CalcRestState();

	uint32 pLastUpdate;
	uint32 pLastUpdateWZ;
	uint8  playeraction;

	EQStreamInterface* eqs;

	uint32				ip;
	uint16				port;
    CLIENT_CONN_STATUS  client_state;
	uint32				character_id;
	uint32				WID;
	uint32				account_id;
	char				account_name[30];
	uint32				lsaccountid;
	char				lskey[30];
	int16				admin;
	uint32				guild_id;
	uint8				guildrank; // player's rank in the guild, 0-GUILD_MAX_RANK
	bool				GuildBanker;
	uint16				duel_target;
	bool				duelaccepted;
	std::list<uint32> keyring;
	bool				tellsoff;	// GM /toggle
	bool				gmhideme;
	bool				LFG;
	bool				LFP;
	uint8				LFGFromLevel;
	uint8				LFGToLevel;
	bool				LFGMatchFilter;
	char				LFGComments[64];
	bool				AFK;
	bool				auto_attack;
	bool				auto_fire;
	uint8				gmspeed;
	bool				medding;
	uint16				horseId;
	bool				revoked;
	uint32				pQueuedSaveWorkID;
	uint16				pClientSideTarget;
	uint32				weight;
	bool				berserk;
	bool				dead;
	uint16				BoatID;
	uint16				TrackingID;
	uint16				CustomerID;
	uint32              account_creation;
	uint8				firstlogon;
	uint32              mercid;				// current merc 
	uint8				mercSlot;			// selected merc slot
	bool	Trader;
	bool	Buyer;
	string	BuyerWelcomeMessage;
	bool	AbilityTimer;
	int Haste;  //precalced value

	int32				max_end;
	int32				cur_end;

	PlayerProfile_Struct		m_pp;
	ExtendedProfile_Struct		m_epp;
	Inventory					m_inv;
	Object*						m_tradeskill_object;
	PetInfo						m_petinfo; // current pet data, used while loading from and saving to DB
	PetInfo						m_suspendedminion; // pet data for our suspended minion.
	MercInfo					m_mercinfo[MAXMERCS]; // current mercenary
	InspectMessage_Struct		m_inspect_message;

	void NPCSpawn(const Seperator* sep);
	uint32 GetEXPForLevel(uint16 level);

	bool	CanBeInZone();
	void	SendLogoutPackets();
    bool    AddPacket(const EQApplicationPacket *, bool);
    bool    AddPacket(EQApplicationPacket**, bool);
    bool    SendAllPackets();
	LinkedList<CLIENTPACKET *> clientpackets;

	//Zoning related stuff
	void SendZoneCancel(ZoneChange_Struct *zc);
	void SendZoneError(ZoneChange_Struct *zc, int8 err);
	void DoZoneSuccess(ZoneChange_Struct *zc, uint16 zone_id, uint32 instance_id, float dest_x, float dest_y, float dest_z, float dest_h, int8 ignore_r);
	void ZonePC(uint32 zoneID, uint32 instance_id, float x, float y, float z, float heading, uint8 ignorerestrictions, ZoneMode zm);
	void ProcessMovePC(uint32 zoneID, uint32 instance_id, float x, float y, float z, float heading, uint8 ignorerestrictions = 0, ZoneMode zm = ZoneSolicited);
	float	zonesummon_x;
	float	zonesummon_y;
	float	zonesummon_z;
	uint16	zonesummon_id;
	uint8	zonesummon_ignorerestrictions;
	ZoneMode zone_mode;


	Timer	position_timer;
	uint8	position_timer_counter;

	PTimerList p_timers;		//persistent timers
	Timer	hpupdate_timer;
	Timer	camp_timer;
	Timer	process_timer;
	Timer	stamina_timer;
	Timer	zoneinpacket_timer;
	Timer	linkdead_timer;
	Timer	dead_timer;
	Timer	global_channel_timer;
	Timer	shield_timer;
	Timer	fishing_timer;
	Timer	endupkeep_timer;
	Timer	forget_timer;	// our 2 min everybody forgets you timer
	Timer	autosave_timer;
#ifdef REVERSE_AGGRO
	Timer	scanarea_timer;
#endif
	Timer	tribute_timer;

#ifdef PACKET_UPDATE_MANAGER
	UpdateManager update_manager;
#endif

	Timer	proximity_timer;
	Timer	TaskPeriodic_Timer;
	Timer	charm_update_timer;
	Timer	rest_timer;
	Timer	charm_class_attacks_timer;
	Timer	charm_cast_timer;
	Timer	qglobal_purge_timer;
	Timer	TrackingTimer;
	Timer	RespawnFromHoverTimer;
	Timer	merc_timer;

	float	proximity_x;
	float	proximity_y;
	float	proximity_z;


	void	BulkSendInventoryItems();

	faction_map factionvalues;

	uint32 tribute_master_id;

	FILE *SQL_log;
	uint32       max_AAXP;
	uint32		staminacount;
	AA_Array* aa[MAX_PP_AA_ARRAY];		//this list contains pointers into our player profile
	map<uint32,uint8> aa_points;
	bool npcflag;
	uint8 npclevel;
	bool feigned;
	bool zoning;
	bool tgb;
	bool instalog;
	int32	last_reported_mana;
	int32	last_reported_endur;

	unsigned int  AggroCount; // How many mobs are aggro on us.

	unsigned int	RestRegenHP;
	unsigned int	RestRegenMana;
	unsigned int	RestRegenEndurance;

	set<uint32> zone_flags;

	ClientTaskState *taskstate;
	int TotalSecondsPlayed;

	//Anti Spam Stuff
	Timer *KarmaUpdateTimer;
	uint32 TotalKarma;

	Timer *GlobalChatLimiterTimer; //60 seconds
	uint32 AttemptedMessages;

	EQClientVersion ClientVersion;
	uint32	ClientVersionBit;

	int XPRate;

	bool m_ShadowStepExemption;
	bool m_KnockBackExemption;
	bool m_PortExemption;
	bool m_SenseExemption;
    std::map<uint32, uint32> alternate_currency;

	//Connecting debug code.
	enum { //connecting states, used for debugging only
		NoPacketsReceived,		//havent gotten anything
		//this is the point where the client changes to the loading screen
		ReceivedZoneEntry,		//got the first packet, loading up PP
		PlayerProfileLoaded,	//our DB work is done, sending it
		ZoneInfoSent,		//includes PP, tributes, tasks, spawns, time and weather
		//this is the point where the client shows a status bar zoning in
		NewZoneRequested,	//received and sent new zone request
		ClientSpawnRequested,	//client sent ReqClientSpawn
		ZoneContentsSent,		//objects, doors, zone points
		ClientReadyReceived,	//client told us its ready, send them a bunch of crap like guild MOTD, etc
		//this is the point where the client releases the mouse
		ClientConnectFinished	//client finally moved to finished state, were done here
	} conn_state;
	void ReportConnectingState();

	uint8 HideCorpseMode;
	bool PendingGuildInvitation;
	int PendingRezzXP;
	uint32 PendingRezzDBID;
	uint16 PendingRezzSpellID;		// Only used for resurrect while hovering.
	std::string PendingRezzCorpseName;	// Only used for resurrect while hovering.

	std::set<uint32> PlayerBlockedBuffs;
	std::set<uint32> PetBlockedBuffs;
	std::list<string> DraggedCorpses;

	uint8 MaxXTargets;
	bool XTargetAutoAddHaters;

    struct XTarget_Struct XTargets[XTARGET_HARDCAP];

    Timer   ItemTickTimer;
	std::map<std::string,std::string> accountflags;
};

#include "parser.h"
#endif
