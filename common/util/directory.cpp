#include "directory.h"

#ifdef _WIN32
#include "win_dirent.h"
#else
#include <dirent.h>
#endif

EQ::Directory::Directory(const std::string &path)
{
	m_dir = opendir(path.c_str());
}

EQ::Directory::~Directory()
{
	if (m_dir) {
		closedir(m_dir);
	}
}

bool EQ::Directory::Exists()
{
	return m_dir != nullptr;
}

void EQ::Directory::GetFiles(std::vector<std::string>& files)
{
	if (m_dir) {
		struct dirent *ent;
		while ((ent = readdir(m_dir)) != nullptr) {
			switch (ent->d_type) {
			case DT_REG:
				files.push_back(ent->d_name);
				break;
			default:
				break;
			}
		}

		rewinddir(m_dir);
	}
}
