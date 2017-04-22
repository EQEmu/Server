#pragma once

#include <string>
#include <vector>

struct DIR;

namespace EQ {
	class Directory
	{
	public:
		Directory(const std::string &path);
		~Directory();

		bool Exists();
		void GetFiles(std::vector<std::string> &files);
	private:
		DIR *m_dir;
	};
}
