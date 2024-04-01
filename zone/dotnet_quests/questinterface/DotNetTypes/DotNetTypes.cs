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
        return  callingMethod?.DeclaringType.Assembly;
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

public static class NPCExtensions
{
    public static void Signal(this NPC npc, NpcEvent e, int signalCode = 0)
    {
        // Get the currently executing assembly
        Assembly currentAssembly = EqFactory.GetCallerAssembly();

        // Get all types defined in the assembly
        Type[] types = currentAssembly.GetTypes();

        // Iterate through the types
        foreach (Type type in types)
        {
            if (type.FullName == npc.GetOrigName()) {
                e.npc = npc;
                e.data = signalCode.ToString();
                if (type != null && type.GetMethod("Signal") != null) {
                    var instance = Activator.CreateInstance(type);
                    var newEvent = new NpcEvent{
                        npc = npc,
                        data = signalCode.ToString(),
                        entityList = e.entityList,
                        zone = e.zone,
                        worldServer = e.worldServer,
                        logSys = e.logSys
                    };
                    type.GetMethod("Signal")?.Invoke(instance, [newEvent]);
                }
            }
        }
    }
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
    void Say(PlayerEvent e) { }
    void Trade(PlayerEvent e) { }
    void Death(PlayerEvent e) { }
    void Spawn(PlayerEvent e) { }
    void Attack(PlayerEvent e) { }
    void Combat(PlayerEvent e) { }
    void Aggro(PlayerEvent e) { }
    void Slay(PlayerEvent e) { }
    void NpcSlay(PlayerEvent e) { }
    void WaypointArrive(PlayerEvent e) { }
    void WaypointDepart(PlayerEvent e) { }
    void Timer(PlayerEvent e) { }
    void Signal(PlayerEvent e) { }
    void Hp(PlayerEvent e) { }
    void Enter(PlayerEvent e) { }
    void Exit(PlayerEvent e) { }
    void EnterZone(PlayerEvent e) { }
    void ClickDoor(PlayerEvent e) { }
    void Loot(PlayerEvent e) { }
    void Zone(PlayerEvent e) { }
    void LevelUp(PlayerEvent e) { }
    void KilledMerit(PlayerEvent e) { }
    void CastOn(PlayerEvent e) { }
    void TaskAccepted(PlayerEvent e) { }
    void TaskStageComplete(PlayerEvent e) { }
    void TaskUpdate(PlayerEvent e) { }
    void TaskComplete(PlayerEvent e) { }
    void TaskFail(PlayerEvent e) { }
    void AggroSay(PlayerEvent e) { }
    void PlayerPickup(PlayerEvent e) { }
    void PopupResponse(PlayerEvent e) { }
    void EnvironmentalDamage(PlayerEvent e) { }
    void ProximitySay(PlayerEvent e) { }
    void Cast(PlayerEvent e) { }
    void CastBegin(PlayerEvent e) { }
    void ScaleCalc(PlayerEvent e) { }
    void ItemEnterZone(PlayerEvent e) { }
    void TargetChange(PlayerEvent e) { }
    void HateList(PlayerEvent e) { }
    void SpellEffectClient(PlayerEvent e) { }
    void SpellEffectNpc(PlayerEvent e) { }
    void SpellEffectBuffTicClient(PlayerEvent e) { }
    void SpellEffectBuffTicNpc(PlayerEvent e) { }
    void SpellFade(PlayerEvent e) { }
    void SpellEffectTranslocateComplete(PlayerEvent e) { }
    void CombineSuccess(PlayerEvent e) { }
    void CombineFailure(PlayerEvent e) { }
    void ItemClick(PlayerEvent e) { }
    void ItemClickCast(PlayerEvent e) { }
    void GroupChange(PlayerEvent e) { }
    void ForageSuccess(PlayerEvent e) { }
    void ForageFailure(PlayerEvent e) { }
    void FishStart(PlayerEvent e) { }
    void FishSuccess(PlayerEvent e) { }
    void FishFailure(PlayerEvent e) { }
    void ClickObject(PlayerEvent e) { }
    void DiscoverItem(PlayerEvent e) { }
    void Disconnect(PlayerEvent e) { }
    void Connect(PlayerEvent e) { }
    void ItemTick(PlayerEvent e) { }
    void DuelWin(PlayerEvent e) { }
    void DuelLose(PlayerEvent e) { }
    void EncounterLoad(PlayerEvent e) { }
    void EncounterUnload(PlayerEvent e) { }
    void Command(PlayerEvent e) { }
    void DropItem(PlayerEvent e) { }
    void DestroyItem(PlayerEvent e) { }
    void FeignDeath(PlayerEvent e) { }
    void WeaponProc(PlayerEvent e) { }
    void EquipItem(PlayerEvent e) { }
    void UnequipItem(PlayerEvent e) { }
    void AugmentItem(PlayerEvent e) { }
    void UnaugmentItem(PlayerEvent e) { }
    void AugmentInsert(PlayerEvent e) { }
    void AugmentRemove(PlayerEvent e) { }
    void EnterArea(PlayerEvent e) { }
    void LeaveArea(PlayerEvent e) { }
    void Respawn(PlayerEvent e) { }
    void DeathComplete(PlayerEvent e) { }
    void UnhandledOpcode(PlayerEvent e) { }
    void Tick(PlayerEvent e) { }
    void SpawnZone(PlayerEvent e) { }
    void DeathZone(PlayerEvent e) { }
    void UseSkill(PlayerEvent e) { }
    void CombineValidate(PlayerEvent e) { }
    void BotCommand(PlayerEvent e) { }
    void Warp(PlayerEvent e) { }
    void TestBuff(PlayerEvent e) { }
    void Combine(PlayerEvent e) { }
    void Consider(PlayerEvent e) { }
    void ConsiderCorpse(PlayerEvent e) { }
    void LootZone(PlayerEvent e) { }
    void EquipItemClient(PlayerEvent e) { }
    void UnequipItemClient(PlayerEvent e) { }
    void SkillUp(PlayerEvent e) { }
    void LanguageSkillUp(PlayerEvent e) { }
    void AltCurrencyMerchantBuy(PlayerEvent e) { }
    void AltCurrencyMerchantSell(PlayerEvent e) { }
    void MerchantBuy(PlayerEvent e) { }
    void MerchantSell(PlayerEvent e) { }
    void Inspect(PlayerEvent e) { }
    void voidBeforeUpdate(PlayerEvent e) { }
    void AaBuy(PlayerEvent e) { }
    void AaGain(PlayerEvent e) { }
    void AaExpGain(PlayerEvent e) { }
    void ExpGain(PlayerEvent e) { }
    void Payload(PlayerEvent e) { }
    void LevelDown(PlayerEvent e) { }
    void GmCommand(PlayerEvent e) { }
    void Despawn(PlayerEvent e) { }
    void DespawnZone(PlayerEvent e) { }
    void BotCreate(PlayerEvent e) { }
    void AugmentInsertClient(PlayerEvent e) { }
    void AugmentRemoveClient(PlayerEvent e) { }
    void EquipItemBot(PlayerEvent e) { }
    void UnequipItemBot(PlayerEvent e) { }
    void DamageGiven(PlayerEvent e) { }
    void DamageTaken(PlayerEvent e) { }
    void ItemClickClient(PlayerEvent e) { }
    void ItemClickCastClient(PlayerEvent e) { }
    void DestroyItemClient(PlayerEvent e) { }
    void DropItemClient(PlayerEvent e) { }
    void MemorizeSpell(PlayerEvent e) { }
    void UnmemorizeSpell(PlayerEvent e) { }
    void ScribeSpell(PlayerEvent e) { }
    void UnscribeSpell(PlayerEvent e) { }
    void LootAdded(PlayerEvent e) { }
    void LdonPointsGain(PlayerEvent e) { }
    void LdonPointsLoss(PlayerEvent e) { }
    void AltCurrencyGain(PlayerEvent e) { }
    void AltCurrencyLoss(PlayerEvent e) { }
    void CrystalGain(PlayerEvent e) { }
    void CrystalLoss(PlayerEvent e) { }
    void TimerPause(PlayerEvent e) { }
    void TimerResume(PlayerEvent e) { }
    void TimerStart(PlayerEvent e) { }
    void TimerStop(PlayerEvent e) { }
    void EntityVariableDelete(PlayerEvent e) { }
    void EntityVariableSet(PlayerEvent e) { }
    void EntityVariableUpdate(PlayerEvent e) { }
    void SpellEffectBot(PlayerEvent e) { }
    void SpellEffectBuffTicBot(PlayerEvent e) { }
}

public struct NpcEvent
{
    public Zone zone;
    public EntityList entityList;
    public NPC npc;
    public Mob mob;
    public WorldServer worldServer;
    public EQEmuLogSys logSys;
    public QuestManager questManager;
    public string data;
    public uint extra_data;
    public List<string> stringList;
    public List<Mob> mobList;
    public List<ItemInstance> itemList;
    public List<EQApplicationPacket> packetList;
}

public struct PlayerEvent
{
    public Zone zone;
    public EntityList entityList;
    public Client player;
    public WorldServer worldServer;
    public EQEmuLogSys logSys;
    public QuestManager questManager;

    public string data;
    public uint extra_data;
    public List<string> stringList;
    public List<Mob> mobList;
    public List<ItemInstance> itemList;
    public List<EQApplicationPacket> packetList;

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