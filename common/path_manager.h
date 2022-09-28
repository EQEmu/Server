#ifndef EQEMU_PATH_MANAGER_H
#define EQEMU_PATH_MANAGER_H


#include <string>

class PathManager {
public:
	void LoadPaths();

	[[nodiscard]] const std::string &GetLogPath() const;
	[[nodiscard]] const std::string &GetLuaModsPath() const;
	[[nodiscard]] const std::string &GetLuaModulesPath() const;
	[[nodiscard]] const std::string &GetMapsPath() const;
	[[nodiscard]] const std::string &GetPatchPath() const;
	[[nodiscard]] const std::string &GetPluginsPath() const;
	[[nodiscard]] const std::string &GetQuestsPath() const;
	[[nodiscard]] const std::string &GetServerPath() const;
	[[nodiscard]] const std::string &GetSharedMemoryPath() const;

private:
	std::string m_log_path;
	std::string m_lua_mods_path;
	std::string m_lua_modules_path;
	std::string m_maps_path;
	std::string m_patch_path;
	std::string m_plugins_path;
	std::string m_quests_path;
	std::string m_server_path;
	std::string m_shared_memory_path;
};

extern PathManager path;

#endif //EQEMU_PATH_MANAGER_H
