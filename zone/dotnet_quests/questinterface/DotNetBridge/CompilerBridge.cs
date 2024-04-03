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
            if (reloading)
            {
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
        if (!File.Exists(projPath))
        {
            Console.WriteLine($"Project path does not exist for zone at {projPath}");
            return;
        }
        if (File.Exists(outPath)) {
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
                npcMethod?.Invoke(npcObject, [new NpcEvent() {
                        globals = new EQGlobals() {
                            zone = zone,
                            logSys = logSys,
                            worldServer = worldServer,
                            questManager = questManager,
                            entityList = entityList,
                        },
                        lists = new EQLists() {
                            stringList = stringList,
                            mobList = mobList,
                            itemList = itemList,
                            packetList = packetList
                        },
                        npc = npc,
                        mob = mob,
                        extra_data = npcEventArgs.ExtraData,
                        data = message ?? "",
                    }]);
            }
        }
        catch (Exception e)
        {
            logSys?.QuestError($"Error running quest {EqFactory.CreateMob(npcEventArgs.Mob, false).GetOrigName()}::{EventMap.NpcMethodMap[id]} {e.Message}");
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
            if (questAssembly_.GetType("player")?.GetMethod(EventMap.NpcMethodMap[id]) != null)
            {
                questAssembly_.GetType("player").GetMethod(EventMap.NpcMethodMap[id]).Invoke(Activator.CreateInstance(questAssembly_.GetType("player")), [new PlayerEvent() {
                        globals = new EQGlobals() {
                            zone = zone,
                            logSys = logSys,
                            worldServer = worldServer,
                            questManager = questManager,
                            entityList = entityList,
                        },
                        lists = new EQLists() {
                            stringList = stringList,
                            mobList = mobList,
                            itemList = itemList,
                            packetList = packetList
                        },
                        extra_data = npcEventArgs.ExtraData,
                        data = message ?? "",
                        player = EqFactory.CreateMob(npcEventArgs.Mob, false).CastToClient(),
                    }]);
            }
        }
        catch (Exception e)
        {
            logSys?.QuestError($"Error running quest {EqFactory.CreateMob(npcEventArgs.Mob, false).GetOrigName()}::{EventMap.NpcMethodMap[id]} {e.Message}");
            var inner = e.InnerException;
            while (inner != null)
            {
                logSys?.QuestError($"Error running quest. Inner Exception: {inner.Message}");
                inner = inner.InnerException;
            }
        }
    }


}

public class CollectibleAssemblyLoadContext : AssemblyLoadContext
{
    public CollectibleAssemblyLoadContext() : base(isCollectible: true) { }

}