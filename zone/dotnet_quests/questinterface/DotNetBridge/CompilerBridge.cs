using System.Diagnostics;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;
using System.Security.Cryptography;
using System.Text;

public class LockfileMutex : IDisposable
{
    private readonly string _fileName;

    private FileStream? _stream;

    public LockfileMutex(string name)
    {
        var assemblyDir = Path.GetDirectoryName(typeof(LockfileMutex).Assembly.Location) ?? throw new FileNotFoundException("cannot determine assembly location");
        var file = Path.GetFullPath(Path.Combine(assemblyDir, name));
        _fileName = file;
    }

    public bool Acquire()
    {
        try
        {
            _stream = new FileStream(_fileName, FileMode.OpenOrCreate, FileAccess.ReadWrite, FileShare.None);
            return true;
        }
        catch (IOException ex) when (ex.Message.Contains(_fileName))
        {
            return false;
        }
    }

    public void Dispose()
    {
        if (_stream != null)
        {
            _stream.Dispose();

            try
            {
                File.Delete(_fileName);
            }
            catch
            {
                // ignored
            }
        }

        GC.SuppressFinalize(this);
    }
}

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
    private static bool questReload = false;
    private static EQGlobals globals;

    private static List<System.Timers.Timer> timers = new List<System.Timers.Timer>();

    private static System.Timers.Timer PollForChanges(string path, Action callback)
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
                logSys?.QuestDebug($"Detected change in .cs file in {path}- Reloading dotnet quests");
                Console.WriteLine($"Detected change in .cs file in {path}- Reloading dotnet quests");
                callback();
                reloading = false;
                lastCheck = lastWriteTime;
            }
        };
        timer.Start();
        return timer;
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


    }

    private static string GetDirHash(string path)
    {
        using (MD5 md5 = MD5.Create())
        {
            foreach (string file in Directory.GetFiles(path, "*.cs", SearchOption.TopDirectoryOnly).Concat(Directory.GetFiles(path, "*.csproj", SearchOption.TopDirectoryOnly)))
            {
                // Hash based on file contents, we don't care about arbitrary resaves
                byte[] contentBytes = File.ReadAllBytes(file);

                // Update MD5 with file content
                md5.TransformBlock(contentBytes, 0, contentBytes.Length, null, 0);
            }

            // Finalize the hash calculation
            md5.TransformFinalBlock(new byte[0], 0, 0); // Necessary to finalize the hash calculation
            return BitConverter.ToString(md5.Hash).Replace("-", string.Empty);
        }
    }

    private static CollectibleAssemblyLoadContext? assemblyContext_ = null;
    private static CollectibleAssemblyLoadContext? globalAssemblyContext = null;
    private static Assembly? questAssembly_;
    private static Assembly? globalAssembly_;
    public static void Reload()
    {
        var workingDirectory = Directory.GetCurrentDirectory();
        var zoneDir = Path.Combine(workingDirectory, "dotnet_quests", zone?.GetShortName() ?? "");
        var globalDir = Path.Combine(workingDirectory, "dotnet_quests", "global");
        logSys?.QuestDebug($"Watching for *.cs file changes in {zoneDir}");
        Console.WriteLine($"Watching for *.cs file changes in {zoneDir}");
        logSys?.QuestDebug($"Watching for *.cs file changes in {globalDir}");
        Console.WriteLine($"Watching for *.cs file changes in {globalDir}");
        foreach (var timer in timers)
        {
            timer.Stop();
        }
        timers.Clear();
        timers.Add(PollForChanges(zoneDir, ReloadZoneAsync));
        timers.Add(PollForChanges(globalDir, ReloadGlobalAsync));
    }
    public static void ReloadZone()
    {
        questReload = true;
        var zoneName = zone?.GetShortName() ?? "";
        var workingDirectory = Directory.GetCurrentDirectory();
        var directoryPath = $"{workingDirectory}/dotnet_quests/{zoneName}";
        var outPath = $"{workingDirectory}/dotnet_quests/out";
        var projPath = $"{directoryPath}/{zoneName}.csproj";
        var hash = GetDirHash(directoryPath);
        var zoneGuid = $"{zoneName}-{hash}";
        var questAssemblyPath = $"{outPath}/{zoneGuid}.dll";
        Console.WriteLine($"Looking for {questAssemblyPath}");
        if (File.Exists(questAssemblyPath))
        {
            logSys?.QuestDebug($"Zone dotnet lib up to date");
            if (assemblyContext_ == null)
            {
                try
                {
                    assemblyContext_ = new CollectibleAssemblyLoadContext(outPath);
                    questAssembly_ = assemblyContext_.LoadFromAssemblyPath(questAssemblyPath);
                    logSys?.QuestDebug($"Successfully loaded .NET quests with {questAssembly_.GetTypes().Count()} exported types.");
                    questReload = false;
                    return;

                }
                catch (Exception e)
                {
                    logSys?.QuestError($"Error loading existing lib, continuing to recompile. {e.Message}");

                }

            }
        }

        if (assemblyContext_ != null)
        {
            assemblyContext_.Unload();
            assemblyContext_ = null;
            questAssembly_ = null;
            GC.Collect();
            GC.WaitForPendingFinalizers();
        }

        npcMap.Clear();
        playerMap.Clear();


        if (!File.Exists(projPath))
        {
            Console.WriteLine($"Project path does not exist for zone at {projPath}");
            return;
        }
        if (Directory.Exists(outPath))
        {
            // Clean up existing dll and pdb
            string[] filesToDelete = Directory.GetFiles(outPath, "*", SearchOption.TopDirectoryOnly)
                                .Where(f => Path.GetFileName(f).StartsWith(zoneName, StringComparison.OrdinalIgnoreCase))
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

        var startInfo = new ProcessStartInfo
        {
            FileName = Path.Combine(workingDirectory + "/bin/dotnet/DotNetCompiler"),
            Arguments = $"{zoneName} {zoneGuid} {outPath} {directoryPath}",
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
        questReload = false;
    }

    public static void ReloadGlobalAsync()
    {
        Task.Run(() => ReloadGlobalWithLock());
    }

    public static void ReloadZoneAsync()
    {
        Task.Run(() => ReloadZone());
    }

    private static void ReloadGlobalWithLock()
    {
        using (var mutex = new LockfileMutex("lockGlobalQuestsReload.lock"))
        {
            var firstAcquire = mutex.Acquire();
            Console.WriteLine($"Acquired first lock: {firstAcquire}");
            var counter = 0;
            if (!firstAcquire)
            {
                while (!mutex.Acquire())
                {
                    if (counter++ % 10 == 0) {
                        Console.WriteLine("Waiting to acquire global lock...");
                    }
                    Thread.Sleep(100);
                }
            }

            ReloadGlobal(firstAcquire);
        }
    }

    public static void ReloadGlobal(bool firstAcquire)
    {
        questReload = true;
        var workingDirectory = Directory.GetCurrentDirectory();
        var directoryPath = $"{workingDirectory}/dotnet_quests/global";
        var outPath = $"{workingDirectory}/dotnet_quests/out";
        var projPath = $"{directoryPath}/global.csproj";
        var hash = GetDirHash(directoryPath);
        var globalGuid = $"global-{hash}";
        var globalAssemblyPath = $"{outPath}/{globalGuid}.dll";

        // If we were not the first acquire we expect this to be built by someone else eventually
        if (!firstAcquire)
        {
            var counter = 0;
            var continueBuild = false;
            while (!File.Exists(globalAssemblyPath))
            {
                Console.WriteLine($"Waiting for another process to build {globalAssemblyPath} before continuing {counter} / 20");
                Thread.Sleep(1000);
                if (counter++ > 20)
                {
                    continueBuild = true;
                    break;
                }
            }
            if (!continueBuild)
            {
                logSys?.QuestDebug($"Zone dotnet lib built by another process");
                if (globalAssemblyContext == null)
                {
                    try
                    {
                        globalAssemblyContext = new CollectibleAssemblyLoadContext(outPath);
                        globalAssembly_ = globalAssemblyContext.LoadFromAssemblyPath(globalAssemblyPath);
                        logSys?.QuestDebug($"Successfully loaded .NET global quests with {globalAssembly_.GetTypes().Count()} exported types.");
                        questReload = false;
                        return;

                    }
                    catch (Exception e)
                    {
                        logSys?.QuestError($"Error loading existing global lib, continuing to recompile. {e.Message}");

                    }

                }
            }
        }

        if (File.Exists(globalAssemblyPath))
        {
            logSys?.QuestDebug($"Zone dotnet lib up to date");
            if (globalAssemblyContext == null)
            {
                try
                {
                    globalAssemblyContext = new CollectibleAssemblyLoadContext(outPath);
                    globalAssembly_ = globalAssemblyContext.LoadFromAssemblyPath(globalAssemblyPath);
                    logSys?.QuestDebug($"Successfully loaded .NET global quests with {globalAssembly_.GetTypes().Count()} exported types.");
                    questReload = false;
                    return;

                }
                catch (Exception e)
                {
                    logSys?.QuestError($"Error loading existing global lib, continuing to recompile. {e.Message}");

                }

            }
        }


        if (globalAssemblyContext != null)
        {
            globalAssemblyContext.Unload();
            globalAssemblyContext = null;
            globalAssembly_ = null;
            GC.Collect();
            GC.WaitForPendingFinalizers();
        }


        if (!File.Exists(projPath))
        {
            Console.WriteLine($"Project path does not exist for global at {projPath}");
            return;
        }
        if (Directory.Exists(outPath))
        {
            // Clean up existing dll and pdb
            string[] filesToDelete = Directory.GetFiles(outPath, "*", SearchOption.TopDirectoryOnly)
                                .Where(f => Path.GetFileName(f).StartsWith("global", StringComparison.OrdinalIgnoreCase))
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

        globalAssemblyContext = new CollectibleAssemblyLoadContext(outPath);

        var startInfo = new ProcessStartInfo
        {
            FileName = Path.Combine(workingDirectory + "/bin/dotnet/DotNetCompiler"),
            Arguments = $"global {globalGuid} {outPath} {directoryPath}",
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
                logSys?.QuestError($"Process was null when loading global quests");
                return;
            }
            try
            {
                string output = process.StandardOutput.ReadToEnd().Trim();
                process.WaitForExit();
                string errorOutput = process.StandardError.ReadToEnd();
                if (errorOutput.Length > 0 || output.Contains("FAILED"))
                {
                    logSys?.QuestError($"Error compiling global quests:");
                    logSys?.QuestError(errorOutput);
                    logSys?.QuestError(output);
                }
                else
                {
                    Console.WriteLine(output);

                    Console.WriteLine($"Loading global assembly from: {globalAssemblyPath}");
                    globalAssembly_ = globalAssemblyContext.LoadFromAssemblyPath(globalAssemblyPath);
                    logSys?.QuestDebug($"Successfully compiled global .NET quests with {globalAssembly_.GetTypes().Count()} exported types.");
                }
            }
            catch (Exception e)
            {
                logSys?.QuestError($"Exception in loading global quests {e.Message}");
            }
        }
        questReload = false;
    }

    public static void QuestEvent(EventArgs questEventArgs)
    {
        if (zone == null || entityList == null || logSys == null || worldServer == null || questManager == null || questReload)
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
            if (questReload)
            {
                return;
            }
            logSys?.QuestError($"Error running quest subtype {questEventArgs.EventType} quest event ID {questEventArgs.QuestEventId} :: {e.Message}");
            var inner = e.InnerException;
            while (inner != null)
            {
                logSys?.QuestError($"Error running quest. Inner Exception: {inner.Message}");
                inner = inner.InnerException;
            }
        }
    }

    private static bool MethodExistsAndIsConcrete(MethodInfo? methodInfo, System.Type parentType)
    {
        return methodInfo != null && methodInfo.DeclaringType != parentType;
    }

    private static void NpcEvent(EventArgs npcEventArgs, bool global = false)
    {
        QuestEventID id = (QuestEventID)npcEventArgs.QuestEventId;
        var npc = EqFactory.CreateNPC(npcEventArgs.Npc, false);
        var mob = EqFactory.CreateMob(npcEventArgs.Mob, false);
        var npcName = npc?.GetOrigName() ?? "";
        var uniqueName = npc?.GetName() ?? "";
        // If we have this case covered for local zone and being invoked through global don't honor global invoke
        if (global && MethodExistsAndIsConcrete(questAssembly_?.GetType(npcName)?.GetMethod(EventMap.NpcMethodMap[id]), typeof(NpcEvent)))
        {
            return;
        }
        var assembly = global ? globalAssembly_ : questAssembly_;
        if (MethodExistsAndIsConcrete(assembly?.GetType(npcName)?.GetMethod(EventMap.NpcMethodMap[id]), typeof(NpcEvent)))
        {
            object? npcObject;
            var npcType = assembly?.GetType(npcName);
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
        // If we have this case covered for local zone and being invoked through global don't honor global invoke
        if (global && MethodExistsAndIsConcrete(questAssembly_?.GetType("Player")?.GetMethod(EventMap.PlayerMethodMap[id]), typeof(PlayerEvent)))
        {
            return;
        }
        var assembly = global ? globalAssembly_ : questAssembly_;
        if (MethodExistsAndIsConcrete(assembly?.GetType("Player")?.GetMethod(EventMap.PlayerMethodMap[id]), typeof(PlayerEvent)))
        {
            var player = EqFactory.CreateClient(playerEventArgs.Client, false);
            object? playerObject;
            var playerType = assembly?.GetType("Player");
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
        if (MethodExistsAndIsConcrete(questAssembly_?.GetType("Item")?.GetMethod(EventMap.ItemMethodMap[id]), typeof(ItemEvent)))
        {
            questAssembly_?.GetType("Item")?.GetMethod(EventMap.ItemMethodMap[id])?.Invoke(Activator.CreateInstance(questAssembly_?.GetType("Item")), [new ItemEvent(globals, itemEventArgs) {
                client = EqFactory.CreateClient(itemEventArgs.Client, false),
                mob = EqFactory.CreateMob(itemEventArgs.Mob, false),
                item = EqFactory.CreateItemInstance(itemEventArgs.Item, false),
            }]);
        }
    }

    private static void SpellEvent(EventArgs spellEventArgs)
    {
        QuestEventID id = (QuestEventID)spellEventArgs.QuestEventId;
        if (MethodExistsAndIsConcrete(questAssembly_?.GetType("Spell")?.GetMethod(EventMap.SpellMethodMap[id]), typeof(SpellEvent)))
        {
            questAssembly_?.GetType("Spell")?.GetMethod(EventMap.SpellMethodMap[id])?.Invoke(Activator.CreateInstance(questAssembly_?.GetType("Spell")), [new SpellEvent(globals, spellEventArgs) {
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
        if (MethodExistsAndIsConcrete(questAssembly_?.GetType(encounterType)?.GetMethod(EventMap.EncounterMethodMap[id]), typeof(EncounterEvent)))
        {
            questAssembly_?.GetType(encounterType)?.GetMethod(EventMap.EncounterMethodMap[id])?.Invoke(Activator.CreateInstance(questAssembly_.GetType(encounterType)), [new EncounterEvent(globals, encounterEventArgs) {

                encounterName = encounter ?? ""
            }]);
        }
    }

    private static void BotEvent(EventArgs botEventArgs, bool global = false)
    {
        QuestEventID id = (QuestEventID)botEventArgs.QuestEventId;
        var bot = EqFactory.CreateBot(botEventArgs.Bot, false);
        var client = EqFactory.CreateClient(botEventArgs.Client, false);
        if (MethodExistsAndIsConcrete(questAssembly_?.GetType("Bot")?.GetMethod(EventMap.BotMethodMap[id]), typeof(BotEvent)))
        {
            questAssembly_?.GetType("Bot")?.GetMethod(EventMap.BotMethodMap[id])?.Invoke(Activator.CreateInstance(questAssembly_.GetType("Bot")), [new BotEvent(globals, botEventArgs) {
                bot = bot,
                client = client,
            }]);
        }
    }

}

public class CollectibleAssemblyLoadContext : AssemblyLoadContext
{
    private readonly string _dependencyPath;

    public CollectibleAssemblyLoadContext(string dependencyPath) : base(isCollectible: true)
    {
        _dependencyPath = dependencyPath;
    }


    protected override Assembly Load(AssemblyName assemblyName)
    {
        if (assemblyName.Name == "DotNetTypes")
        {
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