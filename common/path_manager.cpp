#include "path_manager.h"
#include "file.h"
#include "eqemu_logsys.h"
#include "eqemu_config.h"

const std::string &PathManager::GetServerPath() const
{
	return m_server_path;
}

void PathManager::SetServerPath(const std::string &server_path)
{
	PathManager::m_server_path = server_path;
}

void PathManager::LoadPaths()
{
	m_server_path = File::FindEqemuConfigPath();

	if (!EQEmuConfig::LoadConfig(m_server_path)) {
		LogError("[PathManager] Failed to load eqemu config");
		return;
	}
	LogInfo("[PathManager] EQEmu server path [{}]", m_server_path);

	const auto c = EQEmuConfig::get();

	// maps
	std::string filename;
	if (File::Exists(fmt::format("{}{}", m_server_path, c->MapDir))) {
		m_maps_path = fmt::format("{}{}", m_server_path, c->MapDir);
	}
	else if (File::Exists(fmt::format("{}maps", m_server_path))) {
		m_maps_path = fmt::format("{}maps", m_server_path);
	}
	else if (File::Exists(fmt::format("{}Maps", m_server_path))) {
		m_maps_path = fmt::format("{}Maps", m_server_path);
	}
	LogInfo("[PathManager] EQEmu maps path [{}]", m_maps_path);
}

const std::string &PathManager::GetMapsPath() const
{
	return m_maps_path;
}

void PathManager::SetMapsPath(const std::string &maps_path)
{
	PathManager::m_maps_path = maps_path;
}

const std::string &PathManager::GetQuestsPath() const
{
	return m_quests_path;
}

void PathManager::SetQuestsPath(const std::string &quests_path)
{
	PathManager::m_quests_path = quests_path;
}
