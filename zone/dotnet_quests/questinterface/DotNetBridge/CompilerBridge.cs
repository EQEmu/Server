using System.Diagnostics;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;

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


    public delegate void InitializeDelegate(InitArgs initArgs);
    public delegate void ReloadDelegate();
    public delegate void QuestEventDelegate(EventArgs npcEventArgs);

    private static Dictionary<string, object> npcMap = new Dictionary<string, object>();
    private static Dictionary<string, object> playerMap = new Dictionary<string, object>();

    private static DateTime lastCheck = DateTime.MinValue;
    private static bool reloading = false;
    private static EQGlobals globals;

    private static void PollForChanges(string path)
    {
        var timer = new System.Timers.Timer(500);
        timer.Elapsed += (sender, args) =>
        {
            if (reloading)
            {
                return;
            }
            var lastWriteTime = Directory.GetFiles(path, "*.cs", SearchOption.TopDirectoryOnly)
                .Max(file => File.GetLastWriteTimeUtc(file));

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
        globals = new EQGlobals()
        {
            zone = zone,
            entityList = entityList,
            logSys = logSys,
            worldServer = worldServer,
            questManager = questManager
        };

        var workingDirectory = Directory.GetCurrentDirectory();
        var zoneDir = Path.Combine(workingDirectory, "dotnet_quests", zone.GetShortName());
        logSys?.QuestDebug($"Watching for *.cs file changes in {zoneDir}");
        Console.WriteLine($"Watching for *.cs file changes in {zoneDir}");
        PollForChanges(zoneDir);
    }

    private static void OnChanged(object sender, FileSystemEventArgs e)
    {
        logSys?.QuestDebug($"Detected change in {e.FullPath} - Reloading dotnet quests");
        Console.WriteLine($"Detected change in {e.FullPath} - Reloading dotnet quests");
        Reload();
    }

    private static CollectibleAssemblyLoadContext? assemblyContext_ = null;
    private static Assembly? questAssembly_;
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
        playerMap.Clear();
        var workingDirectory = Directory.GetCurrentDirectory();
        var directoryPath = $"{workingDirectory}/dotnet_quests/{zoneName}";
        var outPath = $"{workingDirectory}/dotnet_quests/out";

        var projPath = $"{directoryPath}/{zoneName}.csproj";
        if (!File.Exists(projPath))
        {
            Console.WriteLine($"Project path does not exist for zone at {projPath}");
            return;
        }
        if (File.Exists(outPath))
        {
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
        }

        assemblyContext_ = new CollectibleAssemblyLoadContext(outPath);

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
                if (errorOutput.Length > 0 || output.Contains("FAILED"))
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

    public static void QuestEvent(EventArgs questEventArgs)
    {
        if (zone == null || entityList == null || logSys == null || worldServer == null || questAssembly_ == null || questManager == null)
        {
            return;
        }
        try
        {
            switch ((EventSubtype)questEventArgs.EventType)
            {
                case EventSubtype.Event_Npc:
                    NpcEvent(questEventArgs);
                    break;
                case EventSubtype.Event_GlobalNpc:
                    NpcEvent(questEventArgs, true);
                    break;
                case EventSubtype.Event_Player:
                    PlayerEvent(questEventArgs);
                    break;
                case EventSubtype.Event_GlobalPlayer:
                    PlayerEvent(questEventArgs, true);
                    break;
                case EventSubtype.Event_Item:
                    ItemEvent(questEventArgs);
                    break;
                case EventSubtype.Event_Spell:
                    SpellEvent(questEventArgs);
                    break;
                case EventSubtype.Event_Encounter:
                    EncounterEvent(questEventArgs);
                    break;
                case EventSubtype.Event_Bot:
                    BotEvent(questEventArgs);
                    break;
                case EventSubtype.Event_GlobalBot:
                    BotEvent(questEventArgs, true);
                    break;
            }
        }
        catch (Exception e)
        {
            logSys?.QuestError($"Error running quest subtype {questEventArgs.EventType} quest event ID {questEventArgs.QuestEventId} :: {e.Message}");
            var inner = e.InnerException;
            while (inner != null)
            {
                logSys?.QuestError($"Error running quest. Inner Exception: {inner.Message}");
                inner = inner.InnerException;
            }
        }
    }

    private static void NpcEvent(EventArgs npcEventArgs, bool global = false)
    {
        QuestEventID id = (QuestEventID)npcEventArgs.QuestEventId;
        var npc = EqFactory.CreateNPC(npcEventArgs.Npc, false);
        var mob = EqFactory.CreateMob(npcEventArgs.Mob, false);
        var npcName = npc?.GetOrigName() ?? "";
        var uniqueName = npc?.GetName() ?? "";
        if (questAssembly_.GetType(npcName)?.GetMethod(EventMap.NpcMethodMap[id]) != null)
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
            var npcMethod = npcType.GetMethod(EventMap.NpcMethodMap[id]);
            npcMethod?.Invoke(npcObject, [new NpcEvent(globals, npcEventArgs) {
                npc = npc,
                mob = mob,
            }]);
        }
    }

    private static void PlayerEvent(EventArgs playerEventArgs, bool global = false)
    {
        QuestEventID id = (QuestEventID)playerEventArgs.QuestEventId;
        if (questAssembly_.GetType("Player")?.GetMethod(EventMap.PlayerMethodMap[id]) != null)
        {
            var player = EqFactory.CreateClient(playerEventArgs.Client, false);
            object? playerObject;
            var playerType = questAssembly_.GetType("Player");
            var playerName = player.GetName();
            if (playerType == null)
            {
                return;
            }
            if (playerMap.ContainsKey(playerName))
            {
                playerObject = playerMap[playerName];
            }
            else
            {
                playerObject = Activator.CreateInstance(playerType);
                if (playerObject == null)
                {
                    return;
                }
                playerMap[playerName] = playerObject;
            }
            var playerMethod = playerType.GetMethod(EventMap.PlayerMethodMap[id]);
            playerMethod?.Invoke(playerObject, [new PlayerEvent(globals, playerEventArgs) {
                    player = player
            }]);
        }
    }

    private static void ItemEvent(EventArgs itemEventArgs)
    {
        QuestEventID id = (QuestEventID)itemEventArgs.QuestEventId;
        if (questAssembly_.GetType("Item")?.GetMethod(EventMap.ItemMethodMap[id]) != null)
        {
            questAssembly_.GetType("Item").GetMethod(EventMap.ItemMethodMap[id]).Invoke(Activator.CreateInstance(questAssembly_.GetType("Item")), [new ItemEvent(globals, itemEventArgs) {
                client = EqFactory.CreateClient(itemEventArgs.Client, false),
                mob = EqFactory.CreateMob(itemEventArgs.Mob, false),
                item = EqFactory.CreateItemInstance(itemEventArgs.Item, false),
            }]);
        }
    }

    private static void SpellEvent(EventArgs spellEventArgs)
    {
        QuestEventID id = (QuestEventID)spellEventArgs.QuestEventId;
        if (questAssembly_.GetType("Spell")?.GetMethod(EventMap.SpellMethodMap[id]) != null)
        {
            questAssembly_.GetType("Spell").GetMethod(EventMap.SpellMethodMap[id]).Invoke(Activator.CreateInstance(questAssembly_.GetType("Spell")), [new SpellEvent(globals, spellEventArgs) {
                client = EqFactory.CreateClient(spellEventArgs.Client, false),
                mob = EqFactory.CreateMob(spellEventArgs.Mob, false),
                spellID = spellEventArgs.SpellID,
            }]);
        }
    }

    private static void EncounterEvent(EventArgs encounterEventArgs)
    {
        QuestEventID id = (QuestEventID)encounterEventArgs.QuestEventId;
        string? encounter = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
            ? Marshal.PtrToStringUni(encounterEventArgs.EncounterName)
            : Marshal.PtrToStringUTF8(encounterEventArgs.EncounterName);
        string encounterType = $"Encounter_{encounter}";
        if (questAssembly_.GetType(encounterType)?.GetMethod(EventMap.EncounterMethodMap[id]) != null)
        {
            questAssembly_.GetType(encounterType).GetMethod(EventMap.EncounterMethodMap[id]).Invoke(Activator.CreateInstance(questAssembly_.GetType(encounterType)), [new EncounterEvent(globals, encounterEventArgs) {

                encounterName = encounter ?? ""
            }]);
        }
    }

    private static void BotEvent(EventArgs botEventArgs, bool global = false)
    {
        QuestEventID id = (QuestEventID)botEventArgs.QuestEventId;
        var bot = EqFactory.CreateBot(botEventArgs.Bot, false);
        var client = EqFactory.CreateClient(botEventArgs.Client, false);
        if (questAssembly_.GetType("Bot")?.GetMethod(EventMap.BotMethodMap[id]) != null)
        {
            questAssembly_.GetType("Bot").GetMethod(EventMap.BotMethodMap[id]).Invoke(Activator.CreateInstance(questAssembly_.GetType("Bot")), [new BotEvent(globals, botEventArgs) {
                bot = bot,
                client = client,
            }]);
        }
    }

}

public class CollectibleAssemblyLoadContext : AssemblyLoadContext
{
    private readonly string _dependencyPath;

    public CollectibleAssemblyLoadContext(string dependencyPath)  : base(isCollectible: true)
    {
        _dependencyPath = dependencyPath;
    }
   

    protected override Assembly Load(AssemblyName assemblyName)
    {
        if (assemblyName.Name == "DotNetTypes") {
            return null;
        }
        
        // Attempt to load the assembly from the specified dependency path
        string assemblyPath = Path.Combine(_dependencyPath, $"{assemblyName.Name}.dll");
        if (File.Exists(assemblyPath))
        {
            return LoadFromAssemblyPath(assemblyPath);
        }
        // Fallback to default context
        return null;
    }

}