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

	if (!m_server_path.empty()) {
		std::filesystem::current_path(m_server_path);
	}

	if (m_server_path.empty()) {
		LogInfo("Failed to load server path");
		return;
	}

	LogInfo("server [{}]", m_server_path);

	if (!EQEmuConfig::LoadConfig()) {
		LogError("Failed to load eqemu config");
		return;
	}

	const auto c = EQEmuConfig::get();

	// maps
	if (File::Exists(fs::path{m_server_path + "/" + c->MapDir}.string())) {
		m_maps_path = fs::relative(fs::path{m_server_path + "/" + c->MapDir}).string();
	}
	else if (File::Exists(fs::path{m_server_path + "/maps"}.string())) {
		m_maps_path = fs::relative(fs::path{m_server_path + "/maps"}).string();
	}
	else if (File::Exists(fs::path{m_server_path + "/Maps"}.string())) {
		m_maps_path = fs::relative(fs::path{m_server_path + "/Maps"}).string();
	}

	// quests
	if (File::Exists(fs::path{m_server_path + "/" + c->QuestDir}.string())) {
		m_quests_path = fs::relative(fs::path{m_server_path + "/" + c->QuestDir}).string();
	}

	// plugins
	if (File::Exists(fs::path{m_server_path + "/" + c->PluginDir}.string())) {
		m_plugins_path = fs::relative(fs::path{m_server_path + "/" + c->PluginDir}).string();
	}

	// lua_modules
	if (File::Exists(fs::path{m_server_path + "/" + c->LuaModuleDir}.string())) {
		m_lua_modules_path = fs::relative(fs::path{m_server_path + "/" + c->LuaModuleDir}).string();
	}

	// lua mods
	if (File::Exists(fs::path{ m_server_path + "/mods" }.string())) {
		m_lua_mods_path = fs::relative(fs::path{ m_server_path + "/mods" }).string();
	}

	// patches
	if (File::Exists(fs::path{m_server_path + "/" + c->PatchDir}.string())) {
		m_patch_path = fs::relative(fs::path{m_server_path + "/" + c->PatchDir}).string();
	}

	// shared_memory_path
	if (File::Exists(fs::path{m_server_path + "/" + c->SharedMemDir}.string())) {
		m_shared_memory_path = fs::relative(fs::path{ m_server_path + "/" + c->SharedMemDir }).string();
	}

	// logging path
	if (File::Exists(fs::path{m_server_path + "/" + c->LogDir}.string())) {
		m_log_path = fs::relative(fs::path{m_server_path + "/" + c->LogDir}).string();
	}

	LogInfo("logs path [{}]", m_log_path);
	LogInfo("lua mods path [{}]", m_lua_mods_path);
	LogInfo("lua_modules path [{}]", m_lua_modules_path);
	LogInfo("maps path [{}]", m_maps_path);
	LogInfo("patches path [{}]", m_patch_path);
	LogInfo("plugins path [{}]", m_plugins_path);
	LogInfo("quests path [{}]", m_quests_path);
	LogInfo("shared_memory path [{}]", m_shared_memory_path);
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
