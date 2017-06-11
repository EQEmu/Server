#pragma once

#include <string>
#include <vector>

namespace EQ {
	class Directory
	{
	public:
		Directory(const std::string &path);
		~Directory();

		bool Exists();
		void GetFiles(std::vector<std::string> &files);
	private:
		struct impl;
		impl *m_impl;
	};
}
