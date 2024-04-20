using System.Reflection;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

[StructLayout(LayoutKind.Sequential)]
public struct EventArgs
{
    public int EventType;
    public int QuestEventId;
    public IntPtr Npc;
    public IntPtr Mob;
    public IntPtr Client;
    public IntPtr Bot;
    public IntPtr Item;
    public IntPtr Data;
    public IntPtr EncounterName;
    public uint ExtraData;
    public uint SpellID;
    public IntPtr ItemVector;
    public IntPtr MobVector;
    public IntPtr PacketVector;
    public IntPtr StringVector;
}

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

    public static Client CreateClient(nint Cptr, bool own)
    {
        return new Client(Cptr, own);
    }

    public static Bot CreateBot(nint Cptr, bool own)
    {
        return new Bot(Cptr, own);
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

public abstract class INpcEvent
{
    public virtual void Say(NpcEvent e) {}
    public virtual void Trade(NpcEvent e) {}
    public virtual void Death(NpcEvent e) {}
    public virtual void Spawn(NpcEvent e) {}
    public virtual void Attack(NpcEvent e) {}
    public virtual void Combat(NpcEvent e) {}
    public virtual void Aggro(NpcEvent e) {}
    public virtual void Slay(NpcEvent e) {}
    public virtual void NpcSlay(NpcEvent e) {}
    public virtual void WaypointArrive(NpcEvent e) {}
    public virtual void WaypointDepart(NpcEvent e) {}
    public virtual void Timer(NpcEvent e) {}
    public virtual void Signal(NpcEvent e) {}
    public virtual void Hp(NpcEvent e) {}
    public virtual void Enter(NpcEvent e) {}
    public virtual void Exit(NpcEvent e) {}
    public virtual void EnterZone(NpcEvent e) {}
    public virtual void ClickDoor(NpcEvent e) {}
    public virtual void Loot(NpcEvent e) {}
    public virtual void Zone(NpcEvent e) {}
    public virtual void LevelUp(NpcEvent e) {}
    public virtual void KilledMerit(NpcEvent e) {}
    public virtual void CastOn(NpcEvent e) {}
    public virtual void TaskAccepted(NpcEvent e) {}
    public virtual void TaskStageComplete(NpcEvent e) {}
    public virtual void TaskUpdate(NpcEvent e) {}
    public virtual void TaskComplete(NpcEvent e) {}
    public virtual void TaskFail(NpcEvent e) {}
    public virtual void AggroSay(NpcEvent e) {}
    public virtual void PlayerPickup(NpcEvent e) {}
    public virtual void PopupResponse(NpcEvent e) {}
    public virtual void EnvironmentalDamage(NpcEvent e) {}
    public virtual void ProximitySay(NpcEvent e) {}
    public virtual void Cast(NpcEvent e) {}
    public virtual void CastBegin(NpcEvent e) {}
    public virtual void ScaleCalc(NpcEvent e) {}
    public virtual void ItemEnterZone(NpcEvent e) {}
    public virtual void TargetChange(NpcEvent e) {}
    public virtual void HateList(NpcEvent e) {}
    public virtual void SpellEffectClient(NpcEvent e) {}
    public virtual void SpellEffectNpc(NpcEvent e) {}
    public virtual void SpellEffectBuffTicClient(NpcEvent e) {}
    public virtual void SpellEffectBuffTicNpc(NpcEvent e) {}
    public virtual void SpellFade(NpcEvent e) {}
    public virtual void SpellEffectTranslocateComplete(NpcEvent e) {}
    public virtual void CombineSuccess(NpcEvent e) {}
    public virtual void CombineFailure(NpcEvent e) {}
    public virtual void ItemClick(NpcEvent e) {}
    public virtual void ItemClickCast(NpcEvent e) {}
    public virtual void GroupChange(NpcEvent e) {}
    public virtual void ForageSuccess(NpcEvent e) {}
    public virtual void ForageFailure(NpcEvent e) {}
    public virtual void FishStart(NpcEvent e) {}
    public virtual void FishSuccess(NpcEvent e) {}
    public virtual void FishFailure(NpcEvent e) {}
    public virtual void ClickObject(NpcEvent e) {}
    public virtual void DiscoverItem(NpcEvent e) {}
    public virtual void Disconnect(NpcEvent e) {}
    public virtual void Connect(NpcEvent e) {}
    public virtual void ItemTick(NpcEvent e) {}
    public virtual void DuelWin(NpcEvent e) {}
    public virtual void DuelLose(NpcEvent e) {}
    public virtual void EncounterLoad(NpcEvent e) {}
    public virtual void EncounterUnload(NpcEvent e) {}
    public virtual void Command(NpcEvent e) {}
    public virtual void DropItem(NpcEvent e) {}
    public virtual void DestroyItem(NpcEvent e) {}
    public virtual void FeignDeath(NpcEvent e) {}
    public virtual void WeaponProc(NpcEvent e) {}
    public virtual void EquipItem(NpcEvent e) {}
    public virtual void UnequipItem(NpcEvent e) {}
    public virtual void AugmentItem(NpcEvent e) {}
    public virtual void UnaugmentItem(NpcEvent e) {}
    public virtual void AugmentInsert(NpcEvent e) {}
    public virtual void AugmentRemove(NpcEvent e) {}
    public virtual void EnterArea(NpcEvent e) {}
    public virtual void LeaveArea(NpcEvent e) {}
    public virtual void Respawn(NpcEvent e) {}
    public virtual void DeathComplete(NpcEvent e) {}
    public virtual void UnhandledOpcode(NpcEvent e) {}
    public virtual void Tick(NpcEvent e) {}
    public virtual void SpawnZone(NpcEvent e) {}
    public virtual void DeathZone(NpcEvent e) {}
    public virtual void UseSkill(NpcEvent e) {}
    public virtual void CombineValidate(NpcEvent e) {}
    public virtual void BotCommand(NpcEvent e) {}
    public virtual void Warp(NpcEvent e) {}
    public virtual void TestBuff(NpcEvent e) {}
    public virtual void Combine(NpcEvent e) {}
    public virtual void Consider(NpcEvent e) {}
    public virtual void ConsiderCorpse(NpcEvent e) {}
    public virtual void LootZone(NpcEvent e) {}
    public virtual void EquipItemClient(NpcEvent e) {}
    public virtual void UnequipItemClient(NpcEvent e) {}
    public virtual void SkillUp(NpcEvent e) {}
    public virtual void LanguageSkillUp(NpcEvent e) {}
    public virtual void AltCurrencyMerchantBuy(NpcEvent e) {}
    public virtual void AltCurrencyMerchantSell(NpcEvent e) {}
    public virtual void MerchantBuy(NpcEvent e) {}
    public virtual void MerchantSell(NpcEvent e) {}
    public virtual void Inspect(NpcEvent e) {}
    public virtual void AaBuy(NpcEvent e) {}
    public virtual void AaGain(NpcEvent e) {}
    public virtual void AaExpGain(NpcEvent e) {}
    public virtual void ExpGain(NpcEvent e) {}
    public virtual void Payload(NpcEvent e) {}
    public virtual void LevelDown(NpcEvent e) {}
    public virtual void GmCommand(NpcEvent e) {}
    public virtual void Despawn(NpcEvent e) {}
    public virtual void DespawnZone(NpcEvent e) {}
    public virtual void BotCreate(NpcEvent e) {}
    public virtual void AugmentInsertClient(NpcEvent e) {}
    public virtual void AugmentRemoveClient(NpcEvent e) {}
    public virtual void EquipItemBot(NpcEvent e) {}
    public virtual void UnequipItemBot(NpcEvent e) {}
    public virtual void DamageGiven(NpcEvent e) {}
    public virtual void DamageTaken(NpcEvent e) {}
    public virtual void ItemClickClient(NpcEvent e) {}
    public virtual void ItemClickCastClient(NpcEvent e) {}
    public virtual void DestroyItemClient(NpcEvent e) {}
    public virtual void DropItemClient(NpcEvent e) {}
    public virtual void MemorizeSpell(NpcEvent e) {}
    public virtual void UnmemorizeSpell(NpcEvent e) {}
    public virtual void ScribeSpell(NpcEvent e) {}
    public virtual void UnscribeSpell(NpcEvent e) {}
    public virtual void LootAdded(NpcEvent e) {}
    public virtual void LdonPointsGain(NpcEvent e) {}
    public virtual void LdonPointsLoss(NpcEvent e) {}
    public virtual void AltCurrencyGain(NpcEvent e) {}
    public virtual void AltCurrencyLoss(NpcEvent e) {}
    public virtual void CrystalGain(NpcEvent e) {}
    public virtual void CrystalLoss(NpcEvent e) {}
    public virtual void TimerPause(NpcEvent e) {}
    public virtual void TimerResume(NpcEvent e) {}
    public virtual void TimerStart(NpcEvent e) {}
    public virtual void TimerStop(NpcEvent e) {}
    public virtual void EntityVariableDelete(NpcEvent e) {}
    public virtual void EntityVariableSet(NpcEvent e) {}
    public virtual void EntityVariableUpdate(NpcEvent e) {}
    public virtual void SpellEffectBot(NpcEvent e) {}
    public virtual void SpellEffectBuffTicBot(NpcEvent e) {}
}


public abstract class IPlayerEvent
{
    public virtual void Say(PlayerEvent e) {}
    public virtual void EnterZone(PlayerEvent e) {}
    public virtual void EventConnect(PlayerEvent e) {}
    public virtual void EventDisconnect(PlayerEvent e) {}
    public virtual void EnvironmentalDamage(PlayerEvent e) {}
    public virtual void Death(PlayerEvent e) {}
    public virtual void DeathComplete(PlayerEvent e) {}
    public virtual void Timer(PlayerEvent e) {}
    public virtual void DiscoverItem(PlayerEvent e) {}
    public virtual void FishSuccess(PlayerEvent e) {}
    public virtual void ForageSuccess(PlayerEvent e) {}
    public virtual void ClickObject(PlayerEvent e) {}
    public virtual void ClickDoor(PlayerEvent e) {}
    public virtual void Signal(PlayerEvent e) {}
    public virtual void PopupResponse(PlayerEvent e) {}
    public virtual void PlayerPickup(PlayerEvent e) {}
    public virtual void Cast(PlayerEvent e) {}
    public virtual void CastBegin(PlayerEvent e) {}
    public virtual void CastOn(PlayerEvent e) {}
    public virtual void TaskFail(PlayerEvent e) {}
    public virtual void Zone(PlayerEvent e) {}
    public virtual void DuelWin(PlayerEvent e) {}
    public virtual void DuelLose(PlayerEvent e) {}
    public virtual void Loot(PlayerEvent e) {}
    public virtual void TaskStageComplete(PlayerEvent e) {}
    public virtual void TaskAccepted(PlayerEvent e) {}
    public virtual void TaskComplete(PlayerEvent e) {}
    public virtual void TaskUpdate(PlayerEvent e) {}
    public virtual void TaskBeforeUpdate(PlayerEvent e) {}
    public virtual void Command(PlayerEvent e) {}
    public virtual void CombineSuccess(PlayerEvent e) {}
    public virtual void CombineFailure(PlayerEvent e) {}
    public virtual void FeignDeath(PlayerEvent e) {}
    public virtual void EnterArea(PlayerEvent e) {}
    public virtual void LeaveArea(PlayerEvent e) {}
    public virtual void Respawn(PlayerEvent e) {}
    public virtual void UnhandledOpcode(PlayerEvent e) {}
    public virtual void UseSkill(PlayerEvent e) {}
    public virtual void TestBuff(PlayerEvent e) {}
    public virtual void CombineValidate(PlayerEvent e) {}
    public virtual void BotCommand(PlayerEvent e) {}
    public virtual void Warp(PlayerEvent e) {}
    public virtual void Combine(PlayerEvent e) {}
    public virtual void Consider(PlayerEvent e) {}
    public virtual void ConsiderCorpse(PlayerEvent e) {}
    public virtual void EquipItemClient(PlayerEvent e) {}
    public virtual void UnequipItemClient(PlayerEvent e) {}
    public virtual void SkillUp(PlayerEvent e) {}
    public virtual void LanguageSkillUp(PlayerEvent e) {}
    public virtual void AltCurrencyMerchantBuy(PlayerEvent e) {}
    public virtual void AltCurrencyMerchantSell(PlayerEvent e) {}
    public virtual void MerchantBuy(PlayerEvent e) {}
    public virtual void MerchantSell(PlayerEvent e) {}
    public virtual void Inspect(PlayerEvent e) {}
    public virtual void AaBuy(PlayerEvent e) {}
    public virtual void AaGain(PlayerEvent e) {}
    public virtual void AaExpGain(PlayerEvent e) {}
    public virtual void ExpGain(PlayerEvent e) {}
    public virtual void Payload(PlayerEvent e) {}
    public virtual void LevelUp(PlayerEvent e) {}
    public virtual void LevelDown(PlayerEvent e) {}
    public virtual void GmCommand(PlayerEvent e) {}
    public virtual void BotCreate(PlayerEvent e) {}
    public virtual void AugmentInsertClient(PlayerEvent e) {}
    public virtual void AugmentRemoveClient(PlayerEvent e) {}
    public virtual void DamageGiven(PlayerEvent e) {}
    public virtual void DamageTaken(PlayerEvent e) {}
    public virtual void ItemClickCastClient(PlayerEvent e) {}
    public virtual void ItemClickClient(PlayerEvent e) {}
    public virtual void DestroyItemClient(PlayerEvent e) {}
    public virtual void TargetChange(PlayerEvent e) {}
    public virtual void DropItemClient(PlayerEvent e) {}
    public virtual void MemorizeSpell(PlayerEvent e) {}
    public virtual void UnmemorizeSpell(PlayerEvent e) {}
    public virtual void ScribeSpell(PlayerEvent e) {}
    public virtual void UnscribeSpell(PlayerEvent e) {}
    public virtual void LdonPointsGain(PlayerEvent e) {}
    public virtual void LdonPointsLoss(PlayerEvent e) {}
    public virtual void AltCurrencyGain(PlayerEvent e) {}
    public virtual void AltCurrencyLoss(PlayerEvent e) {}
    public virtual void CrystalGain(PlayerEvent e) {}
    public virtual void CrystalLoss(PlayerEvent e) {}
    public virtual void TimerPause(PlayerEvent e) {}
    public virtual void TimerResume(PlayerEvent e) {}
    public virtual void TimerStart(PlayerEvent e) {}
    public virtual void TimerStop(PlayerEvent e) {}
    public virtual void EntityVariableDelete(PlayerEvent e) {}
    public virtual void EntityVariableSet(PlayerEvent e) {}
    public virtual void EntityVariableUpdate(PlayerEvent e) {}
    public virtual void AaLoss(PlayerEvent e) {}
    public virtual void SpellBlocked(PlayerEvent e) {}
}


public abstract class IItemEvent
{
    public virtual void ItemClick(ItemEvent e) {}
    public virtual void ItemClickCast(ItemEvent e) {}
    public virtual void ItemEnterZone(ItemEvent e) {}
    public virtual void Timer(ItemEvent e) {}
    public virtual void WeaponProc(ItemEvent e) {}
    public virtual void Loot(ItemEvent e) {}
    public virtual void EquipItem(ItemEvent e) {}
    public virtual void UnequipItem(ItemEvent e) {}
    public virtual void AugmentItem(ItemEvent e) {}
    public virtual void UnaugmentItem(ItemEvent e) {}
    public virtual void AugmentInsert(ItemEvent e) {}
    public virtual void AugmentRemove(ItemEvent e) {}
    public virtual void TimerPause(ItemEvent e) {}
    public virtual void TimerResume(ItemEvent e) {}
    public virtual void TimerStart(ItemEvent e) {}
    public virtual void TimerStop(ItemEvent e) {}
}


public abstract class ISpellEvent
{
    public virtual void SpellEffectClient(SpellEvent e) {}
    public virtual void SpellEffectBuffTicClient(SpellEvent e) {}
    public virtual void SpellEffectNpc(SpellEvent e) {}
    public virtual void SpellEffectBuffTicNpc(SpellEvent e) {}
    public virtual void SpellFade(SpellEvent e) {}
    public virtual void SpellEffectTranslocateComplete(SpellEvent e) {}
}


public abstract class IEncounterEvent
{
    public virtual void Timer(EncounterEvent e) {}
    public virtual void EncounterLoad(EncounterEvent e) {}
    public virtual void EncounterUnload(EncounterEvent e) {}
}
public abstract class IBotEvent
{
    public virtual void Cast(BotEvent e) {}
    public virtual void CastBegin(BotEvent e) {}
    public virtual void CastOn(BotEvent e) {}
    public virtual void Combat(BotEvent e) {}
    public virtual void Death(BotEvent e) {}
    public virtual void DeathComplete(BotEvent e) {}
    public virtual void PopupResponse(BotEvent e) {}
    public virtual void Say(BotEvent e) {}
    public virtual void Signal(BotEvent e) {}
    public virtual void Slay(BotEvent e) {}
    public virtual void TargetChange(BotEvent e) {}
    public virtual void Timer(BotEvent e) {}
    public virtual void Trade(BotEvent e) {}
    public virtual void UseSkill(BotEvent e) {}
    public virtual void Payload(BotEvent e) {}
    public virtual void EquipItemBot(BotEvent e) {}
    public virtual void UnequipItemBot(BotEvent e) {}
    public virtual void DamageGiven(BotEvent e) {}
    public virtual void DamageTaken(BotEvent e) {}
    public virtual void LevelUp(BotEvent e) {}
    public virtual void LevelDown(BotEvent e) {}
    public virtual void TimerPause(BotEvent e) {}
    public virtual void TimerResume(BotEvent e) {}
    public virtual void TimerStart(BotEvent e) {}
    public virtual void TimerStop(BotEvent e) {}
    public virtual void EntityVariableDelete(BotEvent e) {}
    public virtual void EntityVariableSet(BotEvent e) {}
    public virtual void EntityVariableUpdate(BotEvent e) {}
    public virtual void SpellBlocked(BotEvent e) {}
}

public class EQGlobals
{
    public Zone zone;
    public EntityList entityList;
    public EQEmuLogSys logSys;
    public QuestManager questManager;
    public WorldServer worldServer;
}

public class EQLists
{
    public List<string> stringList;
    public List<Mob> mobList;
    public List<ItemInstance> itemList;
    public List<EQApplicationPacket> packetList;
}

public class EQEvent
{
    public EQEvent(EQGlobals g, EventArgs e)
    {
        zone = g.zone;
        entityList = g.entityList;
        questManager = g.questManager;
        worldServer = g.worldServer;
        logSys = g.logSys;
        stringList = EqFactory.CreateStringVector(e.StringVector, false);
        mobList = EqFactory.CreateMobVector(e.MobVector, false);
        itemList = EqFactory.CreateItemVector(e.ItemVector, false);
        packetList = EqFactory.CreatePacketVector(e.PacketVector, false);
        string? message = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
            ? Marshal.PtrToStringUni(e.Data)
            : Marshal.PtrToStringUTF8(e.Data);
        data = message ?? "";
        extraData = e.ExtraData;
    }

    public void QuestDebug(string message)
    {
        logSys?.QuestDebug(message);
    }

    public void QuestError(string message)
    {
        logSys?.QuestError(message);
    }
    public string data;
    public uint extraData;

    public Zone zone;
    public EntityList entityList;
    public EQEmuLogSys logSys;
    public QuestManager questManager;
    public WorldServer worldServer;

    public List<string> stringList;
    public List<Mob> mobList;
    public List<ItemInstance> itemList;
    public List<EQApplicationPacket> packetList;
}

public class NpcEvent : EQEvent
{
    public NpcEvent(EQGlobals g, EventArgs e) : base(g, e) { }
    public NPC npc;
    public Mob mob;

    uint originalStaleConnectionMs = 0;
    uint originalResendTimeout = 0;
    int originalClientLinkdeadMs = 0;

    public Client? client { get { return mob?.CastToClient(); } }

    public void SetupDebug(int debugMinutes = 20)
    {
        var debugMs = debugMinutes * 60 * 1000;
        var opts = mob.CastToClient().Connection().GetManager().GetOptions();
        originalStaleConnectionMs = opts.daybreak_options.stale_connection_ms;
        originalResendTimeout = opts.daybreak_options.resend_timeout;
        opts.daybreak_options.stale_connection_ms = (uint)debugMs;
        opts.daybreak_options.resend_timeout = (uint)debugMs;
        mob.CastToClient().Connection().GetManager().SetOptions(opts);

        originalClientLinkdeadMs = int.Parse(questinterface.GetRuleValue("Zone:ClientLinkdeadMS"));
        RuleManager.Instance().SetRule("Zone:ClientLinkdeadMS", debugMs.ToString());

        logSys.QuestDebug($"Set all timeout values to {debugMinutes} minutes. You will be disconnected if threads are paused longer than this or client hits 0%");
    }

    public void ResetDebug()
    {
        if (originalClientLinkdeadMs == 0 || originalResendTimeout == 0 || originalClientLinkdeadMs == 0)
        {
            logSys.QuestDebug("SetupDebug was not called before ResetDebug and would have set all timeouts to 0.");
            return;
        }

        Task.Run(() =>
        {
            // Give this a lot of leeway in case it needs time to catch up with pumping messages
            // And evaluating timeout
            Thread.Sleep(5000);
            var opts = mob.CastToClient().Connection().GetManager().GetOptions();
            opts.daybreak_options.stale_connection_ms = originalStaleConnectionMs;
            opts.daybreak_options.resend_timeout = originalResendTimeout;
            mob.CastToClient().Connection().GetManager().SetOptions(opts);

            originalClientLinkdeadMs = int.Parse(questinterface.GetRuleValue("Zone:ClientLinkdeadMS"));
            RuleManager.Instance().SetRule("Zone:ClientLinkdeadMS", originalClientLinkdeadMs.ToString());

            logSys.QuestDebug($"Reset all timeout values");
        });
    }
}

public class PlayerEvent : EQEvent
{
    public PlayerEvent(EQGlobals g, EventArgs e) : base(g, e) { }
    public Client player;
}

public class ItemEvent : EQEvent
{
    public ItemEvent(EQGlobals g, EventArgs e) : base(g, e) { }

    public Mob mob;
    public Client client;
    public ItemInstance item;
}

public class SpellEvent : EQEvent
{
    public SpellEvent(EQGlobals g, EventArgs e) : base(g, e) { }

    public Mob mob;
    public Client client;
    public uint spellID = 0;
}

public class EncounterEvent : EQEvent
{
    public EncounterEvent(EQGlobals g, EventArgs e) : base(g, e) { }

    public string encounterName = "";
}

public class BotEvent : EQEvent
{
    public BotEvent(EQGlobals g, EventArgs e) : base(g, e) { }

    public Bot bot;
    public Client client;
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
        {QuestEventID.EVENT_ENTER_ZONE, "EnterZone"},
        {QuestEventID.EVENT_CONNECT, "Connect"},
        {QuestEventID.EVENT_DISCONNECT, "Disconnect"},
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

    public static readonly Dictionary<QuestEventID, string> ItemMethodMap = new Dictionary<QuestEventID, string>() {
        {QuestEventID.EVENT_ITEM_CLICK, "ItemClick"},
        {QuestEventID.EVENT_ITEM_CLICK_CAST, "ItemClickCast"},
        {QuestEventID.EVENT_ITEM_ENTER_ZONE, "ItemEnterZone"},
        {QuestEventID.EVENT_TIMER, "Timer"},
        {QuestEventID.EVENT_WEAPON_PROC, "WeaponProc"},
        {QuestEventID.EVENT_LOOT, "Loot"},
        {QuestEventID.EVENT_EQUIP_ITEM, "EquipItem"},
        {QuestEventID.EVENT_UNEQUIP_ITEM, "UnequipItem"},
        {QuestEventID.EVENT_AUGMENT_ITEM, "AugmentItem"},
        {QuestEventID.EVENT_UNAUGMENT_ITEM, "UnaugmentItem"},
        {QuestEventID.EVENT_AUGMENT_INSERT, "AugmentInsert"},
        {QuestEventID.EVENT_AUGMENT_REMOVE, "AugmentRemove"},
        {QuestEventID.EVENT_TIMER_PAUSE, "TimerPause"},
        {QuestEventID.EVENT_TIMER_RESUME, "TimerResume"},
        {QuestEventID.EVENT_TIMER_START, "TimerStart"},
        {QuestEventID.EVENT_TIMER_STOP, "TimerStop"}
    };

    public static readonly Dictionary<QuestEventID, string> SpellMethodMap = new Dictionary<QuestEventID, string>() {
        {QuestEventID.EVENT_SPELL_EFFECT_CLIENT, "SpellEffectClient"},
        {QuestEventID.EVENT_SPELL_EFFECT_BUFF_TIC_CLIENT, "SpellEffectBuffTicClient"},
        {QuestEventID.EVENT_SPELL_EFFECT_BUFF_TIC_NPC, "SpellEffectBuffTicNpc"},
        {QuestEventID.EVENT_SPELL_EFFECT_NPC, "SpellEffectNpc"},
        {QuestEventID.EVENT_SPELL_FADE, "SpellFade"},
        {QuestEventID.EVENT_SPELL_EFFECT_TRANSLOCATE_COMPLETE, "SpellEffectTranslocateComplete"},
    };

    public static readonly Dictionary<QuestEventID, string> EncounterMethodMap = new Dictionary<QuestEventID, string>() {
        {QuestEventID.EVENT_TIMER, "Timer"},
        {QuestEventID.EVENT_ENCOUNTER_LOAD, "EncounterLoad"},
        {QuestEventID.EVENT_ENCOUNTER_UNLOAD, "EncounterUnload"},
    };

    public static readonly Dictionary<QuestEventID, string> BotMethodMap = new Dictionary<QuestEventID, string>() {
        {QuestEventID.EVENT_CAST, "Cast"},
        {QuestEventID.EVENT_CAST_BEGIN, "CastBegin"},
        {QuestEventID.EVENT_CAST_ON, "CastOn"},
        {QuestEventID.EVENT_COMBAT, "Combat"},
        {QuestEventID.EVENT_DEATH, "Death"},
        {QuestEventID.EVENT_DEATH_COMPLETE, "DeathComplete"},
        {QuestEventID.EVENT_POPUP_RESPONSE, "PopupResponse"},
        {QuestEventID.EVENT_SAY, "Say"},
        {QuestEventID.EVENT_SIGNAL, "Signal"},
        {QuestEventID.EVENT_SLAY, "Slay"},
        {QuestEventID.EVENT_TARGET_CHANGE, "TargetChange"},
        {QuestEventID.EVENT_TIMER, "Timer"},
        {QuestEventID.EVENT_TRADE, "Trade"},
        {QuestEventID.EVENT_USE_SKILL, "UseSkill"},
        {QuestEventID.EVENT_PAYLOAD, "Payload"},
        {QuestEventID.EVENT_EQUIP_ITEM_BOT, "EquipItemBot"},
        {QuestEventID.EVENT_UNEQUIP_ITEM_BOT, "UnequipItemBot"},
        {QuestEventID.EVENT_DAMAGE_GIVEN, "DamageGiven"},
        {QuestEventID.EVENT_DAMAGE_TAKEN, "DamageTaken"},
        {QuestEventID.EVENT_LEVEL_UP, "LevelUp"},
        {QuestEventID.EVENT_LEVEL_DOWN, "LevelDown"},
        {QuestEventID.EVENT_TIMER_PAUSE, "TimerPause"},
        {QuestEventID.EVENT_TIMER_RESUME, "TimerResume"},
        {QuestEventID.EVENT_TIMER_START, "TimerStart"},
        {QuestEventID.EVENT_TIMER_STOP, "TimerStop"},
        {QuestEventID.EVENT_ENTITY_VARIABLE_DELETE, "EntityVariableDelete"},
        {QuestEventID.EVENT_ENTITY_VARIABLE_SET, "EntityVariableSet"},
        {QuestEventID.EVENT_ENTITY_VARIABLE_UPDATE, "EntityVariableUpdate"},
        {QuestEventID.EVENT_SPELL_BLOCKED, "SpellBlocked"}

    };
}

public enum CastingSlot
{ // hybrid declaration
    Gem1 = 0,
    Gem2 = 1,
    Gem3 = 2,
    Gem4 = 3,
    Gem5 = 4,
    Gem6 = 5,
    Gem7 = 6,
    Gem8 = 7,
    Gem9 = 8,
    Gem10 = 9,
    Gem11 = 10,
    Gem12 = 11,
    MaxGems = 12,
    Ability = 20, // HT/LoH for Tit
    PotionBelt = 21, // Tit uses a different slot for PB
    Item = 22,
    Discipline = 23,
    AltAbility = 0xFF
};