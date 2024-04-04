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

namespace fs = std::filesystem;

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

typedef void(CORECLR_DELEGATE_CALLTYPE *init_fn)(init_payload args);
init_fn init = nullptr;

typedef void(CORECLR_DELEGATE_CALLTYPE *reload_fn)();
reload_fn reload_callback = nullptr;

typedef void(CORECLR_DELEGATE_CALLTYPE *quest_event_fn)(event_payload args);
quest_event_fn quest_event_callback = nullptr;

const char_t *dotnet_type = STR("DotNetQuest, DotNetBridge");

bool initialized = false;

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


int event(event_payload p, std::vector<std::any> *extra_pointers)
{
    if (!initialized)
    {
        return 0;
    }
    auto [item_array, mob_array, packet_array, string_array] = to_vectors(extra_pointers);

    p.item_array = &item_array;
    p.mob_array = &mob_array;
    p.packet_array = &packet_array;
    p.string_array = &string_array;

    if (quest_event_callback != nullptr)
    {
        quest_event_callback(p);
        return 0;
    }

    auto rc = runtime_function(
        dotnet_type,
        STR("QuestEvent") /*method_name*/,
        STR("DotNetQuest+QuestEventDelegate, DotNetBridge") /*delegate_type_name*/,
        nullptr,
        nullptr,
        (void **)&quest_event_callback);
    if (rc != 0)
    {
        std::cerr << "Get delegate failed for quest event delegate: " << std::hex << std::showbase << rc << std::endl;
    }
    quest_event_callback(p);

    return 0;
}

int reload_quests()
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
        STR("DotNetQuest+ReloadDelegate, DotNetBridge") /*delegate_type_name*/,
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

int initialize(Zone *zone, EntityList *entity_list, WorldServer *worldserver, EQEmuLogSys *logsys)
{
    if (initialized)
    {
        return 0;
    }

    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path dotnetPath;
    const std::string roslyn_dll("DotNetBridge.dll");
    for (const auto& entry : fs::recursive_directory_iterator(currentPath, fs::directory_options::follow_directory_symlink)) {
        if (entry.is_regular_file() && entry.path().filename() == roslyn_dll) {
            dotnetPath = entry.path();
            break;
        }
    }
    if (dotnetPath.empty()) {
        printf("Could not locate DotNetBridge.dll from working directory %s\n", currentPath.c_str());
        return 1;
    } else {
        printf("Loading .NET lib at %s\n", dotnetPath.c_str());
    }
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
        STR("DotNetQuest+InitializeDelegate, DotNetBridge") /*delegate_type_name*/,
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