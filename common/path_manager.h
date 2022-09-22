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
};


#endif //EQEMU_PATH_MANAGER_H
