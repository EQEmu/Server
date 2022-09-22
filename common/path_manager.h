#ifndef EQEMU_PATH_MANAGER_H
#define EQEMU_PATH_MANAGER_H


#include <string>

class PathManager {
public:
	[[nodiscard]] const std::string &GetServerPath() const;
	void SetServerPath(const std::string &server_path);
	void LoadPaths();
private:
	std::string m_server_path;
	std::string m_maps_path;
public:
	[[nodiscard]] const std::string &GetMapsPath() const;
	void SetMapsPath(const std::string &maps_path);
	[[nodiscard]] const std::string &GetQuestsPath() const;
	void SetQuestsPath(const std::string &quests_path);
private:
	std::string m_quests_path;
};

extern PathManager path_manager;

#endif //EQEMU_PATH_MANAGER_H
