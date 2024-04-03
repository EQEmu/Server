using System.Reflection;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

public class EqFactory
{
    public static Zone CreateZone(nint Cptr, bool own)
    {
        return new Zone(Cptr, own);
    }
    public static EntityList CreateEntityList(nint Cptr, bool own)
    {
        return new EntityList(Cptr, own);
    }
    public static QuestManager CreateQuestManager(nint Cptr, bool own)
    {
        return new QuestManager(Cptr, own);
    }
    public static NPC CreateNPC(nint Cptr, bool own)
    {
        return new NPC(Cptr, own);
    }

    public static Mob CreateMob(nint Cptr, bool own)
    {
        return new Mob(Cptr, own);
    }

    public static EQEmuLogSys CreateLogSys(nint Cptr, bool own)
    {
        return new EQEmuLogSys(Cptr, own);
    }

    public static WorldServer CreateWorldServer(nint Cptr, bool own)
    {
        return new WorldServer(Cptr, own);
    }

    public static ItemInstance CreateItemInstance(nint Cptr, bool own)
    {
        return new ItemInstance(Cptr, own);
    }

    public static ExtraDataVector CreateExtraDataVector(nint Cptr, bool own)
    {
        return new ExtraDataVector(Cptr, own);
    }


    public static List<ItemInstance> CreateItemVector(nint Cptr, bool own)
    {
        var list = new List<ItemInstance>();
        var vec = new ItemVector(Cptr, own);
        for (var i = 0; i < vec.Count; i++)
        {
            if (vec[i] != null)
            {
                list.Add(vec[i]);
            }
        }

        return list;
    }

    public static List<Mob> CreateMobVector(nint Cptr, bool own)
    {
        var list = new List<Mob>();
        var vec = new MobVector(Cptr, own);
        for (var i = 0; i < vec.Count; i++)
        {
            if (vec[i] != null)
            {
                list.Add(vec[i]);
            }
        }
        return list;
    }

    public static List<string> CreateStringVector(nint Cptr, bool own)
    {
        var list = new List<string>();
        var vec = new StringVector(Cptr, own);
        for (var i = 0; i < vec.Count; i++)
        {
            if (vec[i] != null)
            {
                list.Add(vec[i]);
            }
        }
        return list;
    }

    public static List<EQApplicationPacket> CreatePacketVector(nint Cptr, bool own)
    {
        var list = new List<EQApplicationPacket>();
        var vec = new PacketVector(Cptr, own);
        for (var i = 0; i < vec.Count; i++)
        {
            if (vec[i] != null)
            {
                list.Add(vec[i]);
            }
        }
        return list;
    }

    public static Assembly GetCallerAssembly()
    {
        // Create a new StackTrace that captures filename, line number, and column information.
        StackTrace stackTrace = new StackTrace(fNeedFileInfo: true);
        // Get the calling method (skip 0 as it is the current method, 1 is the immediate caller)
        StackFrame callingFrame = stackTrace.GetFrame(2);
        MethodBase callingMethod = callingFrame.GetMethod();
        return callingMethod?.DeclaringType.Assembly;
    }
}

public static class EQEmuLogSysExtensions
{
    public static void QuestDebug(this EQEmuLogSys logSys, string message, [CallerFilePath] string file = "",
    [CallerMemberName] string member = "",
    [CallerLineNumber] int line = 0)
    {
        logSys.Out(DebugLevel.General, (ushort)LogCategory.QuestDebug, file, member, line, message);
    }

    public static void QuestError(this EQEmuLogSys logSys, string message, [CallerFilePath] string file = "",
    [CallerMemberName] string member = "",
    [CallerLineNumber] int line = 0)
    {
        logSys.Out(DebugLevel.General, (ushort)LogCategory.QuestErrors, file, member, line, message);
    }
}



public static class EntityListExtensions
{
    public static NPC? GetNPCByName(this EntityList entityList, string name)
    {
        foreach (var npc in entityList.GetNPCList())
        {
            if (npc.Value.GetOrigName() == name)
            {
                return npc.Value;
            }
        }
        return null;
    }
}

public static class MobExtensions
{
    public static void QuestSay(this Mob mob, EntityList e, string message) {
        Options journalOptions = new Options {
            speak_mode = SpeakMode.Say,
            journal_mode = Mode.None,
            language = (sbyte)questinterface.CommonTongue,
            message_type = questinterface.NPCQuestSay,
            target_spawn_id = mob.GetID()
        };
        e.QuestJournalledSayClose(mob, 200, mob.GetCleanName(), message, journalOptions);
    }
}

public static class NPCExtensions
{
   
}


// INPCEvent is accessible from an npc script e.g. Guard_Gehnus.csx located under a zone directory
public interface INpcEvent
{
    void Say(NpcEvent e) { }
    void Trade(NpcEvent e) { }
    void Death(NpcEvent e) { }
    void Spawn(NpcEvent e) { }
    void Attack(NpcEvent e) { }
    void Combat(NpcEvent e) { }
    void Aggro(NpcEvent e) { }
    void Slay(NpcEvent e) { }
    void NpcSlay(NpcEvent e) { }
    void WaypointArrive(NpcEvent e) { }
    void WaypointDepart(NpcEvent e) { }
    void Timer(NpcEvent e) { }
    void Signal(NpcEvent e) { }
    void Hp(NpcEvent e) { }
    void Enter(NpcEvent e) { }
    void Exit(NpcEvent e) { }
    void EnterZone(NpcEvent e) { }
    void ClickDoor(NpcEvent e) { }
    void Loot(NpcEvent e) { }
    void Zone(NpcEvent e) { }
    void LevelUp(NpcEvent e) { }
    void KilledMerit(NpcEvent e) { }
    void CastOn(NpcEvent e) { }
    void TaskAccepted(NpcEvent e) { }
    void TaskStageComplete(NpcEvent e) { }
    void TaskUpdate(NpcEvent e) { }
    void TaskComplete(NpcEvent e) { }
    void TaskFail(NpcEvent e) { }
    void AggroSay(NpcEvent e) { }
    void PlayerPickup(NpcEvent e) { }
    void PopupResponse(NpcEvent e) { }
    void EnvironmentalDamage(NpcEvent e) { }
    void ProximitySay(NpcEvent e) { }
    void Cast(NpcEvent e) { }
    void CastBegin(NpcEvent e) { }
    void ScaleCalc(NpcEvent e) { }
    void ItemEnterZone(NpcEvent e) { }
    void TargetChange(NpcEvent e) { }
    void HateList(NpcEvent e) { }
    void SpellEffectClient(NpcEvent e) { }
    void SpellEffectNpc(NpcEvent e) { }
    void SpellEffectBuffTicClient(NpcEvent e) { }
    void SpellEffectBuffTicNpc(NpcEvent e) { }
    void SpellFade(NpcEvent e) { }
    void SpellEffectTranslocateComplete(NpcEvent e) { }
    void CombineSuccess(NpcEvent e) { }
    void CombineFailure(NpcEvent e) { }
    void ItemClick(NpcEvent e) { }
    void ItemClickCast(NpcEvent e) { }
    void GroupChange(NpcEvent e) { }
    void ForageSuccess(NpcEvent e) { }
    void ForageFailure(NpcEvent e) { }
    void FishStart(NpcEvent e) { }
    void FishSuccess(NpcEvent e) { }
    void FishFailure(NpcEvent e) { }
    void ClickObject(NpcEvent e) { }
    void DiscoverItem(NpcEvent e) { }
    void Disconnect(NpcEvent e) { }
    void Connect(NpcEvent e) { }
    void ItemTick(NpcEvent e) { }
    void DuelWin(NpcEvent e) { }
    void DuelLose(NpcEvent e) { }
    void EncounterLoad(NpcEvent e) { }
    void EncounterUnload(NpcEvent e) { }
    void Command(NpcEvent e) { }
    void DropItem(NpcEvent e) { }
    void DestroyItem(NpcEvent e) { }
    void FeignDeath(NpcEvent e) { }
    void WeaponProc(NpcEvent e) { }
    void EquipItem(NpcEvent e) { }
    void UnequipItem(NpcEvent e) { }
    void AugmentItem(NpcEvent e) { }
    void UnaugmentItem(NpcEvent e) { }
    void AugmentInsert(NpcEvent e) { }
    void AugmentRemove(NpcEvent e) { }
    void EnterArea(NpcEvent e) { }
    void LeaveArea(NpcEvent e) { }
    void Respawn(NpcEvent e) { }
    void DeathComplete(NpcEvent e) { }
    void UnhandledOpcode(NpcEvent e) { }
    void Tick(NpcEvent e) { }
    void SpawnZone(NpcEvent e) { }
    void DeathZone(NpcEvent e) { }
    void UseSkill(NpcEvent e) { }
    void CombineValidate(NpcEvent e) { }
    void BotCommand(NpcEvent e) { }
    void Warp(NpcEvent e) { }
    void TestBuff(NpcEvent e) { }
    void Combine(NpcEvent e) { }
    void Consider(NpcEvent e) { }
    void ConsiderCorpse(NpcEvent e) { }
    void LootZone(NpcEvent e) { }
    void EquipItemClient(NpcEvent e) { }
    void UnequipItemClient(NpcEvent e) { }
    void SkillUp(NpcEvent e) { }
    void LanguageSkillUp(NpcEvent e) { }
    void AltCurrencyMerchantBuy(NpcEvent e) { }
    void AltCurrencyMerchantSell(NpcEvent e) { }
    void MerchantBuy(NpcEvent e) { }
    void MerchantSell(NpcEvent e) { }
    void Inspect(NpcEvent e) { }
    void voidBeforeUpdate(NpcEvent e) { }
    void AaBuy(NpcEvent e) { }
    void AaGain(NpcEvent e) { }
    void AaExpGain(NpcEvent e) { }
    void ExpGain(NpcEvent e) { }
    void Payload(NpcEvent e) { }
    void LevelDown(NpcEvent e) { }
    void GmCommand(NpcEvent e) { }
    void Despawn(NpcEvent e) { }
    void DespawnZone(NpcEvent e) { }
    void BotCreate(NpcEvent e) { }
    void AugmentInsertClient(NpcEvent e) { }
    void AugmentRemoveClient(NpcEvent e) { }
    void EquipItemBot(NpcEvent e) { }
    void UnequipItemBot(NpcEvent e) { }
    void DamageGiven(NpcEvent e) { }
    void DamageTaken(NpcEvent e) { }
    void ItemClickClient(NpcEvent e) { }
    void ItemClickCastClient(NpcEvent e) { }
    void DestroyItemClient(NpcEvent e) { }
    void DropItemClient(NpcEvent e) { }
    void MemorizeSpell(NpcEvent e) { }
    void UnmemorizeSpell(NpcEvent e) { }
    void ScribeSpell(NpcEvent e) { }
    void UnscribeSpell(NpcEvent e) { }
    void LootAdded(NpcEvent e) { }
    void LdonPointsGain(NpcEvent e) { }
    void LdonPointsLoss(NpcEvent e) { }
    void AltCurrencyGain(NpcEvent e) { }
    void AltCurrencyLoss(NpcEvent e) { }
    void CrystalGain(NpcEvent e) { }
    void CrystalLoss(NpcEvent e) { }
    void TimerPause(NpcEvent e) { }
    void TimerResume(NpcEvent e) { }
    void TimerStart(NpcEvent e) { }
    void TimerStop(NpcEvent e) { }
    void EntityVariableDelete(NpcEvent e) { }
    void EntityVariableSet(NpcEvent e) { }
    void EntityVariableUpdate(NpcEvent e) { }
    void SpellEffectBot(NpcEvent e) { }
    void SpellEffectBuffTicBot(NpcEvent e) { }
}

// IPlayerEvent is accessible in player.csx located under a zone folder
public interface IPlayerEvent
{
    void Say(PlayerEvent e) {}
    void EnvironmentalDamage(PlayerEvent e) {}
    void Death(PlayerEvent e) {}
    void DeathComplete(PlayerEvent e) {}
    void Timer(PlayerEvent e) {}
    void DiscoverItem(PlayerEvent e) {}
    void FishSuccess(PlayerEvent e) {}
    void ForageSuccess(PlayerEvent e) {}
    void ClickObject(PlayerEvent e) {}
    void ClickDoor(PlayerEvent e) {}
    void Signal(PlayerEvent e) {}
    void PopupResponse(PlayerEvent e) {}
    void PlayerPickup(PlayerEvent e) {}
    void Cast(PlayerEvent e) {}
    void CastBegin(PlayerEvent e) {}
    void CastOn(PlayerEvent e) {}
    void TaskFail(PlayerEvent e) {}
    void Zone(PlayerEvent e) {}
    void DuelWin(PlayerEvent e) {}
    void DuelLose(PlayerEvent e) {}
    void Loot(PlayerEvent e) {}
    void TaskStageComplete(PlayerEvent e) {}
    void TaskAccepted(PlayerEvent e) {}
    void TaskComplete(PlayerEvent e) {}
    void TaskUpdate(PlayerEvent e) {}
    void TaskBeforeUpdate(PlayerEvent e) {}
    void Command(PlayerEvent e) {}
    void CombineSuccess(PlayerEvent e) {}
    void CombineFailure(PlayerEvent e) {}
    void FeignDeath(PlayerEvent e) {}
    void EnterArea(PlayerEvent e) {}
    void LeaveArea(PlayerEvent e) {}
    void Respawn(PlayerEvent e) {}
    void UnhandledOpcode(PlayerEvent e) {}
    void UseSkill(PlayerEvent e) {}
    void TestBuff(PlayerEvent e) {}
    void CombineValidate(PlayerEvent e) {}
    void BotCommand(PlayerEvent e) {}
    void Warp(PlayerEvent e) {}
    void Combine(PlayerEvent e) {}
    void Consider(PlayerEvent e) {}
    void ConsiderCorpse(PlayerEvent e) {}
    void EquipItemClient(PlayerEvent e) {}
    void UnequipItemClient(PlayerEvent e) {}
    void SkillUp(PlayerEvent e) {}
    void LanguageSkillUp(PlayerEvent e) {}
    void AltCurrencyMerchantBuy(PlayerEvent e) {}
    void AltCurrencyMerchantSell(PlayerEvent e) {}
    void MerchantBuy(PlayerEvent e) {}
    void MerchantSell(PlayerEvent e) {}
    void Inspect(PlayerEvent e) {}
    void AaBuy(PlayerEvent e) {}
    void AaGain(PlayerEvent e) {}
    void AaExpGain(PlayerEvent e) {}
    void ExpGain(PlayerEvent e) {}
    void Payload(PlayerEvent e) {}
    void LevelUp(PlayerEvent e) {}
    void LevelDown(PlayerEvent e) {}
    void GmCommand(PlayerEvent e) {}
    void BotCreate(PlayerEvent e) {}
    void AugmentInsertClient(PlayerEvent e) {}
    void AugmentRemoveClient(PlayerEvent e) {}
    void DamageGiven(PlayerEvent e) {}
    void DamageTaken(PlayerEvent e) {}
    void ItemClickCastClient(PlayerEvent e) {}
    void ItemClickClient(PlayerEvent e) {}
    void DestroyItemClient(PlayerEvent e) {}
    void TargetChange(PlayerEvent e) {}
    void DropItemClient(PlayerEvent e) {}
    void MemorizeSpell(PlayerEvent e) {}
    void UnmemorizeSpell(PlayerEvent e) {}
    void ScribeSpell(PlayerEvent e) {}
    void UnscribeSpell(PlayerEvent e) {}
    void LdonPointsGain(PlayerEvent e) {}
    void LdonPointsLoss(PlayerEvent e) {}
    void AltCurrencyGain(PlayerEvent e) {}
    void AltCurrencyLoss(PlayerEvent e) {}
    void CrystalGain(PlayerEvent e) {}
    void CrystalLoss(PlayerEvent e) {}
    void TimerPause(PlayerEvent e) {}
    void TimerResume(PlayerEvent e) {}
    void TimerStart(PlayerEvent e) {}
    void TimerStop(PlayerEvent e) {}
    void EntityVariableDelete(PlayerEvent e) {}
    void EntityVariableSet(PlayerEvent e) {}
    void EntityVariableUpdate(PlayerEvent e) {}
    void AaLoss(PlayerEvent e) {}
    void SpellBlocked(PlayerEvent e) {}
}

public class EQGlobals {
    public Zone? zone;
    public EntityList? entityList;
    public EQEmuLogSys? logSys;
    public QuestManager? questManager;
    public WorldServer? worldServer;
}

public class EQLists {
    public List<string>? stringList;
    public List<Mob>? mobList;
    public List<ItemInstance>? itemList;
    public List<EQApplicationPacket>? packetList;
}

public class EQEvent {
    public EQGlobals? globals;
    public EQLists? lists;
    public string? data;
    public uint extra_data;
}

public class NpcEvent : EQEvent
{
    public NPC? npc;
    public Mob? mob;
}

public class PlayerEvent : EQEvent
{
    public Client? player;

}

[StructLayout(LayoutKind.Sequential)]

public struct Vec3
{
    public float x, y, z;

    public Vec3(float x, float y, float z)
    {
        this.x = x;
        this.y = y;
        this.z = z;
    }
}
[StructLayout(LayoutKind.Sequential)]
public struct Vec4
{
    public float x, y, z, w;


    public Vec4(float x, float y, float z, float w)
    {
        this.x = x;
        this.y = y;
        this.z = z;
        this.w = w;
    }
}

public class EventMap
{
    public static readonly Dictionary<QuestEventID, string> NpcMethodMap = new Dictionary<QuestEventID, string> {
       {QuestEventID.EVENT_SAY, "Say"},
       {QuestEventID.EVENT_TRADE, "Trade"},
        {QuestEventID.EVENT_DEATH, "Death"},
        {QuestEventID.EVENT_SPAWN, "Spawn"},
        {QuestEventID.EVENT_ATTACK, "Attack"},
        {QuestEventID.EVENT_COMBAT, "Combat"},
        {QuestEventID.EVENT_AGGRO, "Aggro"},
        {QuestEventID.EVENT_SLAY, "Slay"},
        {QuestEventID.EVENT_NPC_SLAY, "NpcSlay"},
        {QuestEventID.EVENT_WAYPOINT_ARRIVE, "WaypointArrive"},
        {QuestEventID.EVENT_WAYPOINT_DEPART, "WaypointDepart"},
        {QuestEventID.EVENT_TIMER, "Timer"},
        {QuestEventID.EVENT_SIGNAL, "Signal"},
        {QuestEventID.EVENT_HP, "Hp"},
        {QuestEventID.EVENT_ENTER, "Enter"},
        {QuestEventID.EVENT_EXIT, "Exit"},
        {QuestEventID.EVENT_ENTER_ZONE, "EnterZone"},
        {QuestEventID.EVENT_CLICK_DOOR, "ClickDoor"},
        {QuestEventID.EVENT_LOOT, "Loot"},
        {QuestEventID.EVENT_ZONE, "Zone"},
        {QuestEventID.EVENT_LEVEL_UP, "LevelUp"},
        {QuestEventID.EVENT_KILLED_MERIT, "KilledMerit"},
        {QuestEventID.EVENT_CAST_ON, "CastOn"},
        {QuestEventID.EVENT_TASK_ACCEPTED, "TaskAccepted"},
        {QuestEventID.EVENT_TASK_STAGE_COMPLETE, "TaskStageComplete"},
        {QuestEventID.EVENT_TASK_UPDATE, "TaskUpdate"},
        {QuestEventID.EVENT_TASK_COMPLETE, "TaskComplete"},
        {QuestEventID.EVENT_TASK_FAIL, "TaskFail"},
        {QuestEventID.EVENT_AGGRO_SAY, "AggroSay"},
        {QuestEventID.EVENT_PLAYER_PICKUP, "PlayerPickup"},
        {QuestEventID.EVENT_POPUP_RESPONSE, "PopupResponse"},
        {QuestEventID.EVENT_ENVIRONMENTAL_DAMAGE, "EnvironmentalDamage"},
        {QuestEventID.EVENT_PROXIMITY_SAY, "ProximitySay"},
        {QuestEventID.EVENT_CAST, "Cast"},
        {QuestEventID.EVENT_CAST_BEGIN, "CastBegin"},
        {QuestEventID.EVENT_SCALE_CALC, "ScaleCalc"},
        {QuestEventID.EVENT_ITEM_ENTER_ZONE, "ItemEnterZone"},
        {QuestEventID.EVENT_TARGET_CHANGE, "TargetChange"},
        {QuestEventID.EVENT_HATE_LIST, "HateList"},
        {QuestEventID.EVENT_SPELL_EFFECT_CLIENT, "SpellEffectClient"},
        {QuestEventID.EVENT_SPELL_EFFECT_NPC, "SpellEffectNpc"},
        {QuestEventID.EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT, "SpellEffectBuffTicClient"},
        {QuestEventID.EVENT_SPELL_EFFECT_BUFF_TIC_NPC, "SpellEffectBuffTicNpc"},
        {QuestEventID.EVENT_SPELL_FADE, "SpellFade"},
        {QuestEventID.EVENT_SPELL_EFFECT_TRANSLOCATE_COMPLETE, "SpellEffectTranslocateComplete"},
        {QuestEventID.EVENT_COMBINE_SUCCESS, "CombineSuccess"},
        {QuestEventID.EVENT_COMBINE_FAILURE, "CombineFailure"},
        {QuestEventID.EVENT_ITEM_CLICK, "ItemClick"},
        {QuestEventID.EVENT_ITEM_CLICK_CAST, "ItemClickCast"},
        {QuestEventID.EVENT_GROUP_CHANGE, "GroupChange"},
        {QuestEventID.EVENT_FORAGE_SUCCESS, "ForageSuccess"},
        {QuestEventID.EVENT_FORAGE_FAILURE, "ForageFailure"},
        {QuestEventID.EVENT_FISH_START, "FishStart"},
        {QuestEventID.EVENT_FISH_SUCCESS, "FishSuccess"},
        {QuestEventID.EVENT_FISH_FAILURE, "FishFailure"},
        {QuestEventID.EVENT_CLICK_OBJECT, "ClickObject"},
        {QuestEventID.EVENT_DISCOVER_ITEM, "DiscoverItem"},
        {QuestEventID.EVENT_DISCONNECT, "Disconnect"},
        {QuestEventID.EVENT_CONNECT, "Connect"},
        {QuestEventID.EVENT_ITEM_TICK, "ItemTick"},
        {QuestEventID.EVENT_DUEL_WIN, "DuelWin"},
        {QuestEventID.EVENT_DUEL_LOSE, "DuelLose"},
        {QuestEventID.EVENT_ENCOUNTER_LOAD, "EncounterLoad"},
        {QuestEventID.EVENT_ENCOUNTER_UNLOAD, "EncounterUnload"},
        {QuestEventID.EVENT_COMMAND, "Command"},
        {QuestEventID.EVENT_DROP_ITEM, "DropItem"},
        {QuestEventID.EVENT_DESTROY_ITEM, "DestroyItem"},
        {QuestEventID.EVENT_FEIGN_DEATH, "FeignDeath"},
        {QuestEventID.EVENT_WEAPON_PROC, "WeaponProc"},
        {QuestEventID.EVENT_EQUIP_ITEM, "EquipItem"},
        {QuestEventID.EVENT_UNEQUIP_ITEM, "UnequipItem"},
        {QuestEventID.EVENT_AUGMENT_ITEM, "AugmentItem"},
        {QuestEventID.EVENT_UNAUGMENT_ITEM, "UnaugmentItem"},
        {QuestEventID.EVENT_AUGMENT_INSERT, "AugmentInsert"},
        {QuestEventID.EVENT_AUGMENT_REMOVE, "AugmentRemove"},
        {QuestEventID.EVENT_ENTER_AREA, "EnterArea"},
        {QuestEventID.EVENT_LEAVE_AREA, "LeaveArea"},
        {QuestEventID.EVENT_RESPAWN, "Respawn"},
        {QuestEventID.EVENT_DEATH_COMPLETE, "DeathComplete"},
        {QuestEventID.EVENT_UNHANDLED_OPCODE, "UnhandledOpcode"},
        {QuestEventID.EVENT_TICK, "Tick"},
        {QuestEventID.EVENT_SPAWN_ZONE, "SpawnZone"},
        {QuestEventID.EVENT_DEATH_ZONE, "DeathZone"},
        {QuestEventID.EVENT_USE_SKILL, "UseSkill"},
        {QuestEventID.EVENT_COMBINE_VALIDATE, "CombineValidate"},
        {QuestEventID.EVENT_BOT_COMMAND, "BotCommand"},
        {QuestEventID.EVENT_WARP, "Warp"},
        {QuestEventID.EVENT_TEST_BUFF, "TestBuff"},
        {QuestEventID.EVENT_COMBINE, "Combine"},
        {QuestEventID.EVENT_CONSIDER, "Consider"},
        {QuestEventID.EVENT_CONSIDER_CORPSE, "ConsiderCorpse"},
        {QuestEventID.EVENT_LOOT_ZONE, "LootZone"},
        {QuestEventID.EVENT_EQUIP_ITEM_CLIENT, "EquipItemClient"},
        {QuestEventID.EVENT_UNEQUIP_ITEM_CLIENT, "UnequipItemClient"},
        {QuestEventID.EVENT_SKILL_UP, "SkillUp"},
        {QuestEventID.EVENT_LANGUAGE_SKILL_UP, "LanguageSkillUp"},
        {QuestEventID.EVENT_ALT_CURRENCY_MERCHANT_BUY, "AltCurrencyMerchantBuy"},
        {QuestEventID.EVENT_ALT_CURRENCY_MERCHANT_SELL, "AltCurrencyMerchantSell"},
        {QuestEventID.EVENT_MERCHANT_BUY, "MerchantBuy"},
        {QuestEventID.EVENT_MERCHANT_SELL, "MerchantSell"},
        {QuestEventID.EVENT_INSPECT, "Inspect"},
        {QuestEventID.EVENT_TASK_BEFORE_UPDATE, "TaskBeforeUpdate"},
        {QuestEventID.EVENT_AA_BUY, "AaBuy"},
        {QuestEventID.EVENT_AA_GAIN, "AaGain"},
        {QuestEventID.EVENT_AA_EXP_GAIN, "AaExpGain"},
        {QuestEventID.EVENT_EXP_GAIN, "ExpGain"},
        {QuestEventID.EVENT_PAYLOAD, "Payload"},
        {QuestEventID.EVENT_LEVEL_DOWN, "LevelDown"},
        {QuestEventID.EVENT_GM_COMMAND, "GmCommand"},
        {QuestEventID.EVENT_DESPAWN, "Despawn"},
        {QuestEventID.EVENT_DESPAWN_ZONE, "DespawnZone"},
        {QuestEventID.EVENT_BOT_CREATE, "BotCreate"},
        {QuestEventID.EVENT_AUGMENT_INSERT_CLIENT, "AugmentInsertClient"},
        {QuestEventID.EVENT_AUGMENT_REMOVE_CLIENT, "AugmentRemoveClient"},
        {QuestEventID.EVENT_EQUIP_ITEM_BOT, "EquipItemBot"},
        {QuestEventID.EVENT_UNEQUIP_ITEM_BOT, "UnequipItemBot"},
        {QuestEventID.EVENT_DAMAGE_GIVEN, "DamageGiven"},
        {QuestEventID.EVENT_DAMAGE_TAKEN, "DamageTaken"},
        {QuestEventID.EVENT_ITEM_CLICK_CLIENT, "ItemClickClient"},
        {QuestEventID.EVENT_ITEM_CLICK_CAST_CLIENT, "ItemClickCastClient"},
        {QuestEventID.EVENT_DESTROY_ITEM_CLIENT, "DestroyItemClient"},
        {QuestEventID.EVENT_DROP_ITEM_CLIENT, "DropItemClient"},
        {QuestEventID.EVENT_MEMORIZE_SPELL, "MemorizeSpell"},
        {QuestEventID.EVENT_UNMEMORIZE_SPELL, "UnmemorizeSpell"},
        {QuestEventID.EVENT_SCRIBE_SPELL, "ScribeSpell"},
        {QuestEventID.EVENT_UNSCRIBE_SPELL, "UnscribeSpell"},
        {QuestEventID.EVENT_LOOT_ADDED, "LootAdded"},
        {QuestEventID.EVENT_LDON_POINTS_GAIN, "LdonPointsGain"},
        {QuestEventID.EVENT_LDON_POINTS_LOSS, "LdonPointsLoss"},
        {QuestEventID.EVENT_ALT_CURRENCY_GAIN, "AltCurrencyGain"},
        {QuestEventID.EVENT_ALT_CURRENCY_LOSS, "AltCurrencyLoss"},
        {QuestEventID.EVENT_CRYSTAL_GAIN, "CrystalGain"},
        {QuestEventID.EVENT_CRYSTAL_LOSS, "CrystalLoss"},
        {QuestEventID.EVENT_TIMER_PAUSE, "TimerPause"},
        {QuestEventID.EVENT_TIMER_RESUME, "TimerResume"},
        {QuestEventID.EVENT_TIMER_START, "TimerStart"},
        {QuestEventID.EVENT_TIMER_STOP, "TimerStop"},
        {QuestEventID.EVENT_ENTITY_VARIABLE_DELETE, "EntityVariableDelete"},
        {QuestEventID.EVENT_ENTITY_VARIABLE_SET, "EntityVariableSet"},
        {QuestEventID.EVENT_ENTITY_VARIABLE_UPDATE, "EntityVariableUpdate"},
        {QuestEventID.EVENT_SPELL_EFFECT_BOT, "SpellEffectBot"},
        {QuestEventID.EVENT_SPELL_EFFECT_BUFF_TIC_BOT, "SpellEffectBuffTicBot"},
    };

    public static readonly Dictionary<QuestEventID, string> PlayerMethodMap = new Dictionary<QuestEventID, string>() {
        {QuestEventID.EVENT_SAY, "Say"},
        {QuestEventID.EVENT_ENVIRONMENTAL_DAMAGE, "EnvironmentalDamage"},
        {QuestEventID.EVENT_DEATH, "Death"},
        {QuestEventID.EVENT_DEATH_COMPLETE, "DeathComplete"},
        {QuestEventID.EVENT_TIMER, "Timer"},
        {QuestEventID.EVENT_DISCOVER_ITEM, "DiscoverItem"},
        {QuestEventID.EVENT_FISH_SUCCESS, "FishSuccess"},
        {QuestEventID.EVENT_FORAGE_SUCCESS, "ForageSuccess"},
        {QuestEventID.EVENT_CLICK_OBJECT, "ClickObject"},
        {QuestEventID.EVENT_CLICK_DOOR, "ClickDoor"},
        {QuestEventID.EVENT_SIGNAL, "Signal"},
        {QuestEventID.EVENT_POPUP_RESPONSE, "PopupResponse"},
        {QuestEventID.EVENT_PLAYER_PICKUP, "PlayerPickup"},
        {QuestEventID.EVENT_CAST, "Cast"},
        {QuestEventID.EVENT_CAST_BEGIN, "CastBegin"},
        {QuestEventID.EVENT_CAST_ON, "CastOn"},
        {QuestEventID.EVENT_TASK_FAIL, "TaskFail"},
        {QuestEventID.EVENT_ZONE, "Zone"},
        {QuestEventID.EVENT_DUEL_WIN, "DuelWin"},
        {QuestEventID.EVENT_DUEL_LOSE, "DuelLose"},
        {QuestEventID.EVENT_LOOT, "Loot"},
        {QuestEventID.EVENT_TASK_STAGE_COMPLETE, "TaskStageComplete"},
        {QuestEventID.EVENT_TASK_ACCEPTED, "TaskAccepted"},
        {QuestEventID.EVENT_TASK_COMPLETE, "TaskComplete"},
        {QuestEventID.EVENT_TASK_UPDATE, "TaskUpdate"},
        {QuestEventID.EVENT_TASK_BEFORE_UPDATE, "TaskBeforeUpdate"},
        {QuestEventID.EVENT_COMMAND, "Command"},
        {QuestEventID.EVENT_COMBINE_SUCCESS, "CombineSuccess"},
        {QuestEventID.EVENT_COMBINE_FAILURE, "CombineFailure"},
        {QuestEventID.EVENT_FEIGN_DEATH, "FeignDeath"},
        {QuestEventID.EVENT_ENTER_AREA, "EnterArea"},
        {QuestEventID.EVENT_LEAVE_AREA, "LeaveArea"},
        {QuestEventID.EVENT_RESPAWN, "Respawn"},
        {QuestEventID.EVENT_UNHANDLED_OPCODE, "UnhandledOpcode"},
        {QuestEventID.EVENT_USE_SKILL, "UseSkill"},
        {QuestEventID.EVENT_TEST_BUFF, "TestBuff"},
        {QuestEventID.EVENT_COMBINE_VALIDATE, "CombineValidate"},
        {QuestEventID.EVENT_BOT_COMMAND, "BotCommand"},
        {QuestEventID.EVENT_WARP, "Warp"},
        {QuestEventID.EVENT_COMBINE, "Combine"},
        {QuestEventID.EVENT_CONSIDER, "Consider"},
        {QuestEventID.EVENT_CONSIDER_CORPSE, "ConsiderCorpse"},
        {QuestEventID.EVENT_EQUIP_ITEM_CLIENT, "EquipItemClient"},
        {QuestEventID.EVENT_UNEQUIP_ITEM_CLIENT, "UnequipItemClient"},
        {QuestEventID.EVENT_SKILL_UP, "SkillUp"},
        {QuestEventID.EVENT_LANGUAGE_SKILL_UP, "LanguageSkillUp"},
        {QuestEventID.EVENT_ALT_CURRENCY_MERCHANT_BUY, "AltCurrencyMerchantBuy"},
        {QuestEventID.EVENT_ALT_CURRENCY_MERCHANT_SELL, "AltCurrencyMerchantSell"},
        {QuestEventID.EVENT_MERCHANT_BUY, "MerchantBuy"},
        {QuestEventID.EVENT_MERCHANT_SELL, "MerchantSell"},
        {QuestEventID.EVENT_INSPECT, "Inspect"},
        {QuestEventID.EVENT_AA_BUY, "AaBuy"},
        {QuestEventID.EVENT_AA_GAIN, "AaGain"},
        {QuestEventID.EVENT_AA_EXP_GAIN, "AaExpGain"},
        {QuestEventID.EVENT_EXP_GAIN, "ExpGain"},
        {QuestEventID.EVENT_PAYLOAD, "Payload"},
        {QuestEventID.EVENT_LEVEL_UP, "LevelUp"},
        {QuestEventID.EVENT_LEVEL_DOWN, "LevelDown"},
        {QuestEventID.EVENT_GM_COMMAND, "GmCommand"},
        {QuestEventID.EVENT_BOT_CREATE, "BotCreate"},
        {QuestEventID.EVENT_AUGMENT_INSERT_CLIENT, "AugmentInsertClient"},
        {QuestEventID.EVENT_AUGMENT_REMOVE_CLIENT, "AugmentRemoveClient"},
        {QuestEventID.EVENT_DAMAGE_GIVEN, "DamageGiven"},
        {QuestEventID.EVENT_DAMAGE_TAKEN, "DamageTaken"},
        {QuestEventID.EVENT_ITEM_CLICK_CAST_CLIENT, "ItemClickCastClient"},
        {QuestEventID.EVENT_ITEM_CLICK_CLIENT, "ItemClickClient"},
        {QuestEventID.EVENT_DESTROY_ITEM_CLIENT, "DestroyItemClient"},
        {QuestEventID.EVENT_TARGET_CHANGE, "TargetChange"},
        {QuestEventID.EVENT_DROP_ITEM_CLIENT, "DropItemClient"},
        {QuestEventID.EVENT_MEMORIZE_SPELL, "MemorizeSpell"},
        {QuestEventID.EVENT_UNMEMORIZE_SPELL, "UnmemorizeSpell"},
        {QuestEventID.EVENT_SCRIBE_SPELL, "ScribeSpell"},
        {QuestEventID.EVENT_UNSCRIBE_SPELL, "UnscribeSpell"},
        {QuestEventID.EVENT_LDON_POINTS_GAIN, "LdonPointsGain"},
        {QuestEventID.EVENT_LDON_POINTS_LOSS, "LdonPointsLoss"},
        {QuestEventID.EVENT_ALT_CURRENCY_GAIN, "AltCurrencyGain"},
        {QuestEventID.EVENT_ALT_CURRENCY_LOSS, "AltCurrencyLoss"},
        {QuestEventID.EVENT_CRYSTAL_GAIN, "CrystalGain"},
        {QuestEventID.EVENT_CRYSTAL_LOSS, "CrystalLoss"},
        {QuestEventID.EVENT_TIMER_PAUSE, "TimerPause"},
        {QuestEventID.EVENT_TIMER_RESUME, "TimerResume"},
        {QuestEventID.EVENT_TIMER_START, "TimerStart"},
        {QuestEventID.EVENT_TIMER_STOP, "TimerStop"},
        {QuestEventID.EVENT_ENTITY_VARIABLE_DELETE, "EntityVariableDelete"},
        {QuestEventID.EVENT_ENTITY_VARIABLE_SET, "EntityVariableSet"},
        {QuestEventID.EVENT_ENTITY_VARIABLE_UPDATE, "EntityVariableUpdate"},
        {QuestEventID.EVENT_AA_LOSS, "AaLoss"},
        {QuestEventID.EVENT_SPELL_BLOCKED, "SpellBlocked"}
    };
}