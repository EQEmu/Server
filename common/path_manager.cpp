#include "path_manager.h"
#include "file.h"
#include "eqemu_logsys.h"

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
	std::string eqemu_server_path = File::FindEqemuConfigPath();

	LogInfo("EQEmu Server path is [{}]", eqemu_server_path);
	std::exit(0);
}
