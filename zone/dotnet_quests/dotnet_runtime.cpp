// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

// Standard headers
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <filesystem>

#include "dotnet_runtime.h"

// Provided by the AppHost NuGet package and installed as an SDK pack
#include <nethost.h>

// Header files copied from https://github.com/dotnet/core-setup
#include <coreclr_delegates.h>
#include <hostfxr.h>

#include "../../common/item_instance.h"
#include "../questmgr.h"

#ifdef WINDOWS
#include <Windows.h>

#define STR(s) L##s
#define CH(c) L##c
#define DIR_SEPARATOR L'\\'

#define string_compare wcscmp

#else
#include <dlfcn.h>
#include <limits.h>

#define STR(s) s
#define CH(c) c
#define DIR_SEPARATOR '/'
#define MAX_PATH PATH_MAX

#define string_compare strcmp

#endif

#if defined(_WIN32)      // Windows
#elif defined(__linux__) // Linux
#include <unistd.h>
#elif defined(__APPLE__) // macOS
#include <mach-o/dyld.h>
#include <libgen.h>
#endif

using string_t = std::basic_string<char_t>;
using namespace EQ;

namespace
{
    // Globals to hold hostfxr exports
    hostfxr_initialize_for_dotnet_command_line_fn init_for_cmd_line_fptr;
    hostfxr_initialize_for_runtime_config_fn init_for_config_fptr;
    hostfxr_get_runtime_delegate_fn get_delegate_fptr;
    hostfxr_run_app_fn run_app_fptr;
    hostfxr_close_fn close_fptr;

    // Forward declarations
    bool load_hostfxr(const char_t *app);
    load_assembly_and_get_function_pointer_fn get_dotnet_load_assembly(const char_t *assembly);
}

struct lib_args
{
    const char_t *message;
    int number;
};

load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer = nullptr;
get_function_pointer_fn runtime_function = nullptr;

struct init_payload
{
    Zone *zone;
    EntityList *entity_list;
    WorldServer *worldserver;
    QuestManager *questmanager;
    EQEmuLogSys *logsys;
};

struct event_payload
{
    int quest_event_id;
    NPC *npc;
    Mob *mob;
    const char_t *data;
    uint32 extra_data;
    std::vector<ItemInstance *> *item_array;
    std::vector<Mob *> *mob_array;
    std::vector<EQApplicationPacket *> *packet_array;
    std::vector<std::string> *string_array;
};

typedef void(CORECLR_DELEGATE_CALLTYPE *init_fn)(init_payload args);
init_fn init = nullptr;

typedef void(CORECLR_DELEGATE_CALLTYPE *reload_fn)();
reload_fn reload_callback = nullptr;

typedef void(CORECLR_DELEGATE_CALLTYPE *npc_event_fn)(event_payload args);
npc_event_fn npc_event_callback = nullptr;

typedef void(CORECLR_DELEGATE_CALLTYPE *player_event_fn)(event_payload args);
player_event_fn player_event_callback = nullptr;

const string_t dotnetlib_path = STR("RoslynBridge.dll");
const char_t *dotnet_type = STR("DotNetQuest, RoslynBridge");

bool initialized = false;

namespace
{
    std::string getExecutablePath()
    {
        char path[PATH_MAX];
#if defined(_WIN32)
        GetModuleFileNameA(NULL, path, MAX_PATH);
#elif defined(__linux__)
        ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
        if (count == -1)
            return "";
        path[count] = '\0';
#elif defined(__APPLE__)
        uint32_t size = sizeof(path);
        if (_NSGetExecutablePath(path, &size) == 0)
        {
            // Resolve any symlinks and get the absolute path
            char realPath[PATH_MAX];
            realpath(path, realPath);
            strncpy(path, realPath, sizeof(path));
        }
        else
        {
            // Buffer size was too small
            return "";
        }
#endif
        std::string p(path);
        return p.substr(0, p.length() - 4);
    }

}

std::tuple<std::vector<ItemInstance *>, std::vector<Mob *>, std::vector<EQApplicationPacket *>, std::vector<std::string>> to_vectors(std::vector<std::any> *anyVec)
{
    std::vector<ItemInstance *> itemVec;
    std::vector<Mob *> mobVec;
    std::vector<EQApplicationPacket *> packetVec;
    std::vector<std::string> stringVec; 
    if (anyVec == nullptr)
    {
        return std::tie(itemVec, mobVec, packetVec, stringVec);
    }
    for (auto element : *anyVec)
    {
        try
        {
            stringVec.push_back(std::any_cast<std::string>(element));
            continue;
        }
        catch (const std::bad_any_cast &e)
        {
        }

        try
        {
            stringVec.push_back(std::any_cast<std::basic_string<char>>(element));
            continue;
        }
        catch (const std::bad_any_cast &e)
        {
        }

        if (element.type() == typeid(EQApplicationPacket *))
        {
            try
            {
                packetVec.push_back(std::any_cast<EQApplicationPacket *>(element));
                continue;
            }
            catch (const std::bad_any_cast &e)
            {
            }
        }

        if (element.type() == typeid(ItemInstance *))
        {
            try
            {
                itemVec.push_back(std::any_cast<ItemInstance *>(element));
                continue;
            }
            catch (const std::bad_any_cast &e)
            {
            }
        }

        if (element.type() == typeid(Mob *))
        {
            try
            {
                mobVec.push_back(std::any_cast<Mob *>(element));
                continue;
            }
            catch (const std::bad_any_cast &e)
            {
            }
        }
        std::cerr << "Error: One of the elements could not be cast to string: " << element.type().name() << '\n';
    }
    return std::tie(itemVec, mobVec, packetVec, stringVec);
}

#if defined(WINDOWS)
int __cdecl event(QuestEventID event, NPC *npc, Mob *init, std::string data, uint32 extra_data, std::vector<std::any> *extra_pointers, bool player_event)
#else
int event(QuestEventID event, NPC *npc, Mob *init, std::string data, uint32 extra_data, std::vector<std::any> *extra_pointers, bool player_event)
#endif
{
    if (!initialized)
    {
        return 1;
    }
    auto [item_array, mob_array, packet_array, string_array] = to_vectors(extra_pointers);
    event_payload p{
        (int)event,
        npc,
        init,
        data.c_str(),
        extra_data,
        &item_array,
        &mob_array,
        &packet_array,
        &string_array};

    if (player_event)
    {
        if (player_event_callback != nullptr)
        {
            player_event_callback(p);
            return 1;
        }

        auto rc = runtime_function(
            dotnet_type,
            STR("NpcEvent") /*method_name*/,
            STR("DotNetQuest+PlayerEventDelegate, RoslynBridge") /*delegate_type_name*/,
            nullptr,
            nullptr,
            (void **)&player_event_callback);
        if (rc != 0)
        {
            std::cerr << "Get delegate failed for npc event delegate: " << std::hex << std::showbase << rc << std::endl;
        }
        player_event_callback(p);
    }
    else
    {
        if (npc_event_callback != nullptr)
        {
            npc_event_callback(p);
            return 1;
        }

        auto rc = runtime_function(
            dotnet_type,
            STR("NpcEvent") /*method_name*/,
            STR("DotNetQuest+NpcEventDelegate, RoslynBridge") /*delegate_type_name*/,
            nullptr,
            nullptr,
            (void **)&npc_event_callback);
        if (rc != 0)
        {
            std::cerr << "Get delegate failed for npc event delegate: " << std::hex << std::showbase << rc << std::endl;
        }
        npc_event_callback(p);
    }

    return 1;
}

#if defined(WINDOWS)
int __cdecl reload_quests()
#else
int reload_quests()
#endif
{
    if (!initialized)
    {
        return 1;
    }
    if (reload_callback != nullptr)
    {
        reload_callback();
        return 1;
    }

    auto rc = runtime_function(
        dotnet_type,
        STR("Reload") /*method_name*/,
        STR("DotNetQuest+ReloadDelegate, RoslynBridge") /*delegate_type_name*/,
        nullptr,
        nullptr,
        (void **)&reload_callback);
    if (rc != 0)
    {
        std::cerr << "Get delegate failed for reload: " << std::hex << std::showbase << rc << std::endl;
    }

    reload_callback();

    return 1;
}

#if defined(WINDOWS)
int __cdecl initialize(Zone *zone, EntityList *entity_list, WorldServer *worldserver, EQEmuLogSys *logsys)
#else
int initialize(Zone *zone, EntityList *entity_list, WorldServer *worldserver, EQEmuLogSys *logsys)
#endif
{
    if (initialized)
    {
        return 0;
    }

    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path dotnetPath = currentPath / "dotnet" / STR("RoslynBridge.dll");
    //
    // STEP 1: Load HostFxr and get exported hosting functions
    //
    if (!load_hostfxr(dotnetPath.c_str()))
    {
        assert(false && "Failure: load_hostfxr()");
        return EXIT_FAILURE;
    }

    load_assembly_and_get_function_pointer = get_dotnet_load_assembly(dotnetPath.c_str());
    assert(load_assembly_and_get_function_pointer != nullptr && "Failure: get_dotnet_load_assembly()");

    init_payload p{
        zone,
        entity_list,
        worldserver,
        &quest_manager,
        logsys};

    auto rc = runtime_function(
        dotnet_type,
        STR("Initialize") /*method_name*/,
        STR("DotNetQuest+InitializeDelegate, RoslynBridge") /*delegate_type_name*/,
        nullptr,
        nullptr,
        (void **)&init);
    if (rc != 0)
    {
        std::cerr << "Get delegate failed for init: " << std::hex << std::showbase << rc << std::endl;
    }
    init(p);
    initialized = true;

    return EXIT_SUCCESS;
}

/********************************************************************************************
 * Function used to load and activate .NET Core
 ********************************************************************************************/

namespace
{
    // Forward declarations
    void *load_library(const char_t *);
    void *get_export(void *, const char *);

#ifdef WINDOWS
    void *load_library(const char_t *path)
    {
        HMODULE h = ::LoadLibraryW(path);
        assert(h != nullptr);
        return (void *)h;
    }
    void *get_export(void *h, const char *name)
    {
        void *f = ::GetProcAddress((HMODULE)h, name);
        assert(f != nullptr);
        return f;
    }
#else
    void *load_library(const char_t *path)
    {
        void *h = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
        assert(h != nullptr);
        return h;
    }
    void *get_export(void *h, const char *name)
    {
        void *f = dlsym(h, name);
        assert(f != nullptr);
        return f;
    }
#endif

    // <SnippetLoadHostFxr>
    // Using the nethost library, discover the location of hostfxr and get exports
    bool load_hostfxr(const char_t *assembly_path)
    {
        get_hostfxr_parameters params{sizeof(get_hostfxr_parameters), assembly_path, nullptr};
        // Pre-allocate a large buffer for the path to hostfxr
        char_t buffer[MAX_PATH];
        size_t buffer_size = sizeof(buffer) / sizeof(char_t);
        int rc = get_hostfxr_path(buffer, &buffer_size, &params);
        if (rc != 0)
            return false;

        // Load hostfxr and get desired exports
        void *lib = load_library(buffer);
        init_for_cmd_line_fptr = (hostfxr_initialize_for_dotnet_command_line_fn)get_export(lib, "hostfxr_initialize_for_dotnet_command_line");
        init_for_config_fptr = (hostfxr_initialize_for_runtime_config_fn)get_export(lib, "hostfxr_initialize_for_runtime_config");
        get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)get_export(lib, "hostfxr_get_runtime_delegate");
        run_app_fptr = (hostfxr_run_app_fn)get_export(lib, "hostfxr_run_app");
        close_fptr = (hostfxr_close_fn)get_export(lib, "hostfxr_close");

        return (init_for_config_fptr && get_delegate_fptr && close_fptr);
    }
    // </SnippetLoadHostFxr>

    // <SnippetInitialize>
    // Load and initialize .NET Core and get desired function pointer for scenario
    load_assembly_and_get_function_pointer_fn get_dotnet_load_assembly(const char_t *config_path)
    {
        // Load .NET Core
        void *load_assembly_and_get_function_pointer = nullptr;
        hostfxr_handle cxt = nullptr;
        std::vector<const char_t *> args{config_path, STR("app_arg_1"), STR("app_arg_2")};
        int rc = init_for_cmd_line_fptr(args.size(), args.data(), nullptr, &cxt);
        if (rc != 0 || cxt == nullptr)
        {
            std::cerr << "Init failed: " << std::hex << std::showbase << rc << std::endl;
            close_fptr(cxt);
            return nullptr;
        }

        // Get the load assembly function pointer
        rc = get_delegate_fptr(
            cxt,
            hdt_load_assembly_and_get_function_pointer,
            &load_assembly_and_get_function_pointer);
        if (rc != 0 || load_assembly_and_get_function_pointer == nullptr)
            std::cerr << "Get delegate failed: " << std::hex << std::showbase << rc << std::endl;
        void *fn_ptr = nullptr;
        get_delegate_fptr(
            cxt,
            hdt_get_function_pointer,
            &fn_ptr);
        runtime_function = (get_function_pointer_fn)(fn_ptr);
        close_fptr(cxt);
        return (load_assembly_and_get_function_pointer_fn)load_assembly_and_get_function_pointer;
    }

    // </SnippetInitialize>
}