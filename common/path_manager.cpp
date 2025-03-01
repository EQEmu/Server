#include "path_manager.h"
#include "file.h"
#include "eqemu_logsys.h"
#include "eqemu_config.h"
#include "strings.h"

#include <filesystem>

namespace fs = std::filesystem;

void PathManager::LoadPaths()
{
	m_server_path = File::FindEqemuConfigPath();

	if (m_server_path.empty()) {
		LogInfo("Failed to load server path");
		return;
	}

	std::filesystem::current_path(m_server_path);

	if (!EQEmuConfig::LoadConfig()) {
		LogError("Failed to load eqemu config");
		return;
	}

	const auto c = EQEmuConfig::get();

	auto resolve_path = [&](const std::string& dir, const std::vector<std::string>& fallback_dirs = {}) -> std::string {
		// relative
		if (File::Exists((fs::path{m_server_path} / dir).string())) {
			return fs::relative(fs::path{m_server_path} / dir).lexically_normal().string();
		}

		// absolute
		if (File::Exists(fs::path{dir}.string())) {
			return fs::absolute(fs::path{dir}).string();
		}

		// fallback search options if specified
		for (const auto& fallback : fallback_dirs) {
			if (File::Exists((fs::path{m_server_path} / fallback).string())) {
				return fs::relative(fs::path{m_server_path} / fallback).lexically_normal().string();
			}
		}

		// if all else fails, just set it to the config value
		return dir;
	};

	m_maps_path          = resolve_path(c->MapDir, {"maps", "Maps"});
	m_quests_path        = resolve_path(c->QuestDir);
	m_plugins_path       = resolve_path(c->PluginDir);
	m_lua_modules_path   = resolve_path(c->LuaModuleDir);
	m_lua_mods_path      = resolve_path("mods");
	m_patch_path         = resolve_path(c->PatchDir);
	m_opcode_path        = resolve_path(c->OpcodeDir);
	m_shared_memory_path = resolve_path(c->SharedMemDir);
	m_log_path           = resolve_path(c->LogDir, {"logs"});

	// Log all paths in a loop
	std::vector<std::pair<std::string, std::string>> paths = {
		{"server", m_server_path},
		{"logs", m_log_path},
		{"lua mods", m_lua_mods_path},
		{"lua_modules", m_lua_modules_path},
		{"maps", m_maps_path},
		{"patches", m_patch_path},
		{"opcode", m_opcode_path},
		{"plugins", m_plugins_path},
		{"quests", m_quests_path},
		{"shared_memory", m_shared_memory_path}
	};

	constexpr int name_width   = 15;
	constexpr int path_width   = 0;
	constexpr int break_length = 70;

	LogInfo("Loading server paths");
	LogInfo("{}", Strings::Repeat("-", break_length));
	for (const auto& [name, in_path] : paths) {
		if (!in_path.empty()) {
			LogInfo("{:>{}} > [{:<{}}]", name, name_width, in_path, path_width);
		}
	}
	LogInfo("{}", Strings::Repeat("-", break_length));
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

const std::string &PathManager::GetOpcodePath() const
{
	return m_opcode_path;
}

const std::string &PathManager::GetLuaModulesPath() const
{
	return m_lua_modules_path;
}

const std::string &PathManager::GetLuaModsPath() const
{
	return m_lua_mods_path;
}
