#include "path_manager.h"
#include "file.h"
#include "eqemu_logsys.h"
#include "eqemu_config.h"

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

	if (!EQEmuConfig::LoadConfig()) {
		LogError("[PathManager] Failed to load eqemu config");
		return;
	}
	LogInfo("[PathManager] server path [{}]", m_server_path);

	const auto c = EQEmuConfig::get();

	// maps
	if (File::Exists(fmt::format("{}/{}", m_server_path, c->MapDir))) {
		m_maps_path = fmt::format("{}/{}", m_server_path, striptrailingslash(c->MapDir));
	}
	else if (File::Exists(fmt::format("{}/maps", m_server_path))) {
		m_maps_path = fmt::format("{}/maps", m_server_path);
	}
	else if (File::Exists(fmt::format("{}/Maps", m_server_path))) {
		m_maps_path = fmt::format("{}/Maps", m_server_path);
	}
	LogInfo("[PathManager] maps path [{}]", m_maps_path);

	// quests
	if (File::Exists(fmt::format("{}/{}", m_server_path, c->QuestDir))) {
		m_quests_path = fmt::format("{}/{}", m_server_path, striptrailingslash(c->QuestDir));
	}
	else if (File::Exists(fmt::format("{}/quests", m_server_path))) {
		m_quests_path = fmt::format("{}/quests", m_server_path);
	}
	LogInfo("[PathManager] quests path [{}]", m_quests_path);

	// plugins
	if (File::Exists(fmt::format("{}/{}", m_server_path, c->PluginDir))) {
		m_plugins_path = fmt::format("{}/{}", m_server_path, striptrailingslash(c->PluginDir));
	}
	else if (File::Exists(fmt::format("{}/plugins", m_server_path))) {
		m_plugins_path = fmt::format("{}/plugins", m_server_path);
	}
	LogInfo("[PathManager] plugins path [{}]", m_plugins_path);

	// lua_modules
	if (File::Exists(fmt::format("{}/{}", m_server_path, c->LuaModuleDir))) {
		m_lua_modules_path = fmt::format("{}/{}", m_server_path, striptrailingslash(c->LuaModuleDir));
	}
	else if (File::Exists(fmt::format("{}/lua_modules", m_server_path))) {
		m_lua_modules_path = fmt::format("{}/lua_modules", m_server_path);
	}
	LogInfo("[PathManager] lua_modules path [{}]", m_lua_modules_path);

	// lua mods
	m_lua_mods_path = fmt::format("{}/mods", m_server_path);
	LogInfo("[PathManager] lua mods path [{}]", m_lua_mods_path);

	// patches
	if (File::Exists(fmt::format("{}/{}", m_server_path, c->PatchDir))) {
		m_patch_path = fmt::format("{}/{}", m_server_path, striptrailingslash(c->PatchDir));
	}
	else if (File::Exists(fmt::format("{}/patches", m_server_path))) {
		m_patch_path = fmt::format("{}/patches", m_server_path);
	}
	LogInfo("[PathManager] patches path [{}]", m_patch_path);

	// shared_memory_path
	if (File::Exists(fmt::format("{}/{}", m_server_path, c->SharedMemDir))) {
		m_shared_memory_path = fmt::format("{}/{}", m_server_path, striptrailingslash(c->SharedMemDir));
	}
	else if (File::Exists(fmt::format("{}/shared_memory", m_server_path))) {
		m_shared_memory_path = fmt::format("{}/shared_memory", m_server_path);
	}
	LogInfo("[PathManager] shared_memory path [{}]", m_shared_memory_path);

	// logging path
	if (File::Exists(fmt::format("{}/{}", m_server_path, c->LogDir))) {
		m_log_path = fmt::format("{}/{}", m_server_path, striptrailingslash(c->LogDir));
	}
	else if (File::Exists(fmt::format("{}/logs", m_server_path))) {
		m_log_path = fmt::format("{}/logs", m_server_path);
	}

	LogInfo("[PathManager] logs path [{}]", m_log_path);
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
