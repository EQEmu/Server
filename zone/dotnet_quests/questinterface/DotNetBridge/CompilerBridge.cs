using System.Diagnostics;
using System.IO;
using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;
using System.Runtime.CompilerServices;


public static class DotNetQuest
{
    public static Zone? zone = null;
    public static EntityList? entityList = null;
    public static EQEmuLogSys? logSys = null;
    public static WorldServer? worldServer = null;
    public static QuestManager? questManager = null;
    public static InitArgs? initArgs = null;

    [StructLayout(LayoutKind.Sequential)]
    public struct InitArgs
    {
        public IntPtr Zone;
        public IntPtr EntityList;
        public IntPtr WorldServer;
        public IntPtr QuestManager;
        public IntPtr LogSys;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct EventArgs
    {
        public int QuestEventId;
        public IntPtr Npc;
        public IntPtr Mob;
        public IntPtr Data;
        public uint ExtraData;
        public IntPtr ItemVector;
        public IntPtr MobVector;
        public IntPtr PacketVector;
        public IntPtr StringVector;
    }


    public delegate void InitializeDelegate(InitArgs initArgs);
    public delegate void ReloadDelegate();
    public delegate void NpcEventDelegate(EventArgs npcEventArgs);
    public delegate void PlayerEventDelegate(EventArgs npcEventArgs);

    private static Dictionary<string, object> npcMap = new Dictionary<string, object>();

    private static FileSystemWatcher? watcher;

    private static DateTime lastCheck = DateTime.MinValue;
    private static bool reloading = false;

    private static void PollForChanges(string path)
    {
        var timer = new System.Timers.Timer(500);
        timer.Elapsed += (sender, args) =>
        {
            if (reloading) {
                return;
            }
            var lastWriteTime = Directory.GetFiles(path, "*.cs", SearchOption.TopDirectoryOnly)
                .Max(file => (File.GetLastWriteTimeUtc(file)));

            if (lastWriteTime > lastCheck)
            {
                reloading = true;
                logSys?.QuestDebug("Detected change in .cs file - Reloading dotnet quests");
                Console.WriteLine("Detected change in .cs file - Reloading dotnet quests");
                Reload();
                reloading = false;
                lastCheck = lastWriteTime;
            }
        };
        timer.Start();
    }
    public static void Initialize(InitArgs initArgs)
    {
        zone = EqFactory.CreateZone(initArgs.Zone, false);
        entityList = EqFactory.CreateEntityList(initArgs.EntityList, false);
        logSys = EqFactory.CreateLogSys(initArgs.LogSys, false);
        worldServer = EqFactory.CreateWorldServer(initArgs.WorldServer, false);
        questManager = EqFactory.CreateQuestManager(initArgs.QuestManager, false);
        DotNetQuest.initArgs = initArgs;

        var workingDirectory = Directory.GetCurrentDirectory();
        var zoneDir = Path.Combine(workingDirectory, "dotnet_quests", zone.GetShortName());
        logSys?.QuestDebug($"Watching for *.cs file changes in {zoneDir}");
        Console.WriteLine($"Watching for *.cs file changes in {zoneDir}");
        PollForChanges(zoneDir);
        // Issues with inotify in docker filesystem with mounted drives--investigate later but use polling for now
        // watcher = new FileSystemWatcher(zoneDir, "*.cs")
        // {
        //     IncludeSubdirectories = true,
        //     NotifyFilter = NotifyFilters.LastWrite | NotifyFilters.FileName | NotifyFilters.DirectoryName
        // };
        // watcher.Changed += OnChanged;
        // watcher.Created += OnChanged;
        // watcher.Deleted += OnChanged;
        // watcher.Renamed += OnChanged;

        // watcher.EnableRaisingEvents = true;
    }

    private static void OnChanged(object sender, FileSystemEventArgs e)
    {
        logSys?.QuestDebug($"Detected change in {e.FullPath} - Reloading dotnet quests");
        Console.WriteLine($"Detected change in {e.FullPath} - Reloading dotnet quests");
        Reload();
    }

    private static CollectibleAssemblyLoadContext? assemblyContext_ = null;
    private static Assembly? questAssembly_ = null;
    public static void Reload()
    {
        if (assemblyContext_ != null)
        {
            assemblyContext_.Unload();
            assemblyContext_ = null;
            questAssembly_ = null;
            GC.Collect();
            GC.WaitForPendingFinalizers();
        }
        var zoneName = zone?.GetShortName();
        npcMap.Clear();
        var workingDirectory = Directory.GetCurrentDirectory();
        var directoryPath = $"{workingDirectory}/dotnet_quests/{zoneName}";
        var outPath = $"{workingDirectory}/dotnet_quests/out";

        var projPath = $"{directoryPath}/{zoneName}.csproj";
        if (!File.Exists(projPath)) {
            Console.WriteLine($"Project path does not exist for zone at {projPath}");
            return;
        }
        // Clean up existing dll and pdb
        string[] filesToDelete = Directory.GetFiles(outPath, $"{zoneName}*.*")
                .ToArray();

        // Delete each file
        foreach (string file in filesToDelete)
        {
            try
            {
                File.Delete(file);
            }
            catch (IOException ex)
            {
                Console.WriteLine($"An error occurred while deleting file {file}: {ex.Message}");
            }
        }
        assemblyContext_ = new CollectibleAssemblyLoadContext();
        
        var zoneGuid = $"{zoneName}-{Guid.NewGuid().ToString().Substring(0, 8)}";
        var startInfo = new ProcessStartInfo
        {
            FileName = "dotnet",
            Arguments = $"build --output {outPath} -r 'p:Configuration=Debug;AssemblyName={zoneGuid} {directoryPath}/{zoneName}.csproj",
            UseShellExecute = false,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            CreateNoWindow = true,
            WorkingDirectory = directoryPath,
        };

        using (var process = Process.Start(startInfo))
        {
            if (process == null)
            {
                logSys?.QuestError($"Process was null when loading zone quests: {zoneName}");
                return;
            }
            try
            {
                string output = process.StandardOutput.ReadToEnd().Trim();
                process.WaitForExit();
                string errorOutput = process.StandardError.ReadToEnd();
                if (errorOutput.Length > 0)
                {
                    logSys?.QuestError($"Error compiling quests:");
                    logSys?.QuestError(errorOutput);
                    logSys?.QuestError(output);
                }
                else
                {
                    Console.WriteLine(output);
                    var questAssemblyPath = $"{outPath}/{zoneGuid}.dll";
                    Console.WriteLine($"Loading quest assembly from: {questAssemblyPath}");
                    questAssembly_ = assemblyContext_.LoadFromAssemblyPath(questAssemblyPath);
                    logSys?.QuestDebug($"Successfully compiled .NET quests with {questAssembly_.GetTypes().Count()} exported types.");
                }
            }
            catch (Exception e)
            {
                logSys?.QuestError($"Exception in loading zone quest {e.Message}");
            }
        }
    }

    public static void NpcEvent(EventArgs npcEventArgs)
    {
        if (zone == null || entityList == null || logSys == null || worldServer == null || questAssembly_ == null || questManager == null)
        {
            return;
        }

        QuestEventID id = (QuestEventID)npcEventArgs.QuestEventId;

        var stringList = EqFactory.CreateStringVector(npcEventArgs.StringVector, false);
        var mobList = EqFactory.CreateMobVector(npcEventArgs.MobVector, false);
        var itemList = EqFactory.CreateItemVector(npcEventArgs.ItemVector, false);
        var packetList = EqFactory.CreatePacketVector(npcEventArgs.PacketVector, false);

        string? message = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
            ? Marshal.PtrToStringUni(npcEventArgs.Data)
            : Marshal.PtrToStringUTF8(npcEventArgs.Data);

        try
        {
            var npc = EqFactory.CreateNPC(npcEventArgs.Npc, false);
            var mob = EqFactory.CreateMob(npcEventArgs.Mob, false);
            var npcName = npc?.GetOrigName() ?? "";
            var uniqueName = npc?.GetName() ?? "";
            if (questAssembly_.GetType(npcName)?.GetMethod(MethodMap[id]) != null)
            {
                object? npcObject;
                var npcType = questAssembly_.GetType(npcName);
                if (npcType == null)
                {
                    return;
                }
                if (npcMap.ContainsKey(uniqueName))
                {
                    npcObject = npcMap[uniqueName];
                }
                else
                {
                    npcObject = Activator.CreateInstance(npcType);
                    if (npcObject == null)
                    {
                        return;
                    }
                    npcMap[uniqueName] = npcObject;
                }
                var npcMethod = npcType.GetMethod(MethodMap[id]);
                npcMethod?.Invoke(npcObject, [new NpcEvent() {
                        npc = npc,
                        mob = mob,
                        zone = zone,
                        logSys = logSys,
                        worldServer = worldServer,
                        questManager = questManager,
                        entityList = entityList,
                        extra_data = npcEventArgs.ExtraData,
                        data = message ?? "",
                        stringList = stringList,
                        mobList = mobList,
                        itemList = itemList,
                        packetList = packetList
                    }]);
            }
        }
        catch (Exception e)
        {
            logSys?.QuestError($"Error running quest {EqFactory.CreateMob(npcEventArgs.Mob, false).GetOrigName()}::{MethodMap[id]} {e.Message}");
            var inner = e.InnerException;
            while (inner != null)
            {
                logSys?.QuestError($"Error running quest. Inner Exception: {inner.Message}");
                inner = inner.InnerException;
            }
        }
    }

    public static void PlayerEvent(EventArgs npcEventArgs)
    {
        if (zone == null || entityList == null || logSys == null || worldServer == null || questAssembly_ == null || questManager == null)
        {
            return;
        }

        QuestEventID id = (QuestEventID)npcEventArgs.QuestEventId;

        var stringList = EqFactory.CreateStringVector(npcEventArgs.StringVector, false);
        var mobList = EqFactory.CreateMobVector(npcEventArgs.MobVector, false);
        var itemList = EqFactory.CreateItemVector(npcEventArgs.ItemVector, false);
        var packetList = EqFactory.CreatePacketVector(npcEventArgs.PacketVector, false);

        string? message = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
            ? Marshal.PtrToStringUni(npcEventArgs.Data)
            : Marshal.PtrToStringUTF8(npcEventArgs.Data);

        try
        {
            if (questAssembly_.GetType("player")?.GetMethod(MethodMap[id]) != null)
            {
                questAssembly_.GetType("player").GetMethod(MethodMap[id]).Invoke(Activator.CreateInstance(questAssembly_.GetType("player")), [new PlayerEvent() {
                        player = EqFactory.CreateMob(npcEventArgs.Mob, false).CastToClient(),
                        zone = zone,
                        logSys = logSys,
                        worldServer = worldServer,
                        questManager = questManager,
                        entityList = entityList,
                        extra_data = npcEventArgs.ExtraData,
                        data = message ?? "",
                        stringList = stringList,
                        mobList = mobList,
                        itemList = itemList,
                        packetList = packetList
                    }]);
            }
        }
        catch (Exception e)
        {
            logSys?.QuestError($"Error running quest {EqFactory.CreateMob(npcEventArgs.Mob, false).GetOrigName()}::{MethodMap[id]} {e.Message}");
            var inner = e.InnerException;
            while (inner != null)
            {
                logSys?.QuestError($"Error running quest. Inner Exception: {inner.Message}");
                inner = inner.InnerException;
            }
        }
    }

    private static readonly Dictionary<QuestEventID, string> MethodMap = new Dictionary<QuestEventID, string> {
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
}

public class CollectibleAssemblyLoadContext : AssemblyLoadContext
{
    public CollectibleAssemblyLoadContext() : base(isCollectible: true) { }

}