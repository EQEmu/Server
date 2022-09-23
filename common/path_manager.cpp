#include "path_manager.h"
#include "file.h"
#include "eqemu_logsys.h"
#include "eqemu_config.h"
#include "strings.h"

#include <filesystem>

namespace fs = std::filesystem;

inline std::string striptrailingslash(const std::string &file_path)
{
	if (file_path.back() == '/' || file_path.back() == '\\') {
		return file_path.substr(0, file_path.length() - 1);
	}

	return file_path;
}

void PathManager::LoadPaths()
{
	m_server_path = File::FindEqemuConfigPath();

	LogInfo("[PathManager] server [{}]", m_server_path);

	if (!EQEmuConfig::LoadConfig()) {
		LogError("[PathManager] Failed to load eqemu config");
		return;
	}

	const auto c = EQEmuConfig::get();

	// maps
	if (File::Exists(fs::path{m_server_path + "/" + c->MapDir})) {
		m_maps_path = fs::canonical(fs::path{m_server_path + "/" + c->MapDir});
	}
	else if (File::Exists(fs::path{m_server_path + "/maps"})) {
		m_maps_path = fs::canonical(fs::path{m_server_path + "/maps"});
	}
	else if (File::Exists(fs::path{m_server_path + "/Maps"})) {
		m_maps_path = fs::canonical(fs::path{m_server_path + "/Maps"});
	}

	// quests
	if (File::Exists(fs::path{m_server_path + "/" + c->QuestDir})) {
		m_quests_path = fs::canonical(fs::path{m_server_path + "/" + c->QuestDir});
	}

	// plugins
	if (File::Exists(fs::path{m_server_path + "/" + c->PluginDir})) {
		m_plugins_path = fs::canonical(fs::path{m_server_path + "/" + c->PluginDir});
	}

	// lua_modules
	if (File::Exists(fs::path{m_server_path + "/" + c->LuaModuleDir})) {
		m_lua_modules_path = fs::canonical(fs::path{m_server_path + "/" + c->LuaModuleDir});
	}

	// lua mods
	m_lua_mods_path = fs::path{m_server_path + "/mods"};

	// patches

	if (File::Exists(fs::path{m_server_path + "/" + c->PatchDir})) {
		m_patch_path = fs::canonical(fs::path{m_server_path + "/" + c->PatchDir});
	}

	// shared_memory_path
	if (File::Exists(fs::path{m_server_path + "/" + c->SharedMemDir})) {
		m_shared_memory_path = fs::canonical(fs::path{m_server_path + "/" + c->SharedMemDir});
	}

	// logging path
	if (File::Exists(fmt::format("{}/{}", m_server_path, c->LogDir))) {
		m_log_path = fs::canonical(fs::path{m_server_path + "/" + c->LogDir});
	}

	LogInfo("[PathManager] logs [{}]", m_log_path);
	LogInfo("[PathManager] lua mods [{}]", m_lua_mods_path);
	LogInfo("[PathManager] lua_modules [{}]", m_lua_modules_path);
	LogInfo("[PathManager] maps [{}]", m_maps_path);
	LogInfo("[PathManager] patches [{}]", m_patch_path);
	LogInfo("[PathManager] plugins [{}]", m_plugins_path);
	LogInfo("[PathManager] quests [{}]", m_quests_path);
	LogInfo("[PathManager] shared_memory [{}]", m_shared_memory_path);
}

const std::string &PathManager::GetServerPath() const
{
	return m_server_path;
}

const std::string &PathManager::GetMapsPath() const
{
	return m_maps_path;
}

const std::string &PathManager::GetQuestsPath() const
{
	return m_quests_path;
}

const std::string &PathManager::GetPluginsPath() const
{
	return m_plugins_path;
}

const std::string &PathManager::GetSharedMemoryPath() const
{
	return m_shared_memory_path;
}

const std::string &PathManager::GetLogPath() const
{
	return m_log_path;
}

const std::string &PathManager::GetPatchPath() const
{
	return m_patch_path;
}

const std::string &PathManager::GetLuaModulesPath() const
{
	return m_lua_modules_path;
}

const std::string &PathManager::GetLuaModsPath() const
{
	return m_lua_mods_path;
}
