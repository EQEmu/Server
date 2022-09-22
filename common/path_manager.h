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
	const std::string &GetMapsPath() const;
	void SetMapsPath(const std::string &maps_path);
	const std::string &GetQuestsPath() const;
	void SetQuestsPath(const std::string &quests_path);
private:
	std::string m_quests_path;
};


#endif //EQEMU_PATH_MANAGER_H
