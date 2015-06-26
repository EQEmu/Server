#ifndef EQEMU_COMMON_FILE_VERIFY_MANAGER_H
#define EQEMU_COMMON_FILE_VERIFY_MANAGER_H

#include "file_verify.h"

namespace EQEmu
{
	class FileVerifyManager
	{
	public:
		~FileVerifyManager();

		static FileVerifyManager& Get()
		{
			static FileVerifyManager instance;
			return instance;
		}

		bool VerifySpellFile(const EQApplicationPacket *app, ClientVersion version);
		bool VerifySkillFile(const EQApplicationPacket *app, ClientVersion version);
		bool VerifyBaseDataFile(const EQApplicationPacket *app, ClientVersion version);
		bool VerifyEQGame(const EQApplicationPacket *app, ClientVersion version);
	private:
		FileVerifyManager();
		FileVerifyManager(FileVerifyManager const&);
		void operator=(FileVerifyManager const&);

		struct impl;
		impl *impl_;
	};
}

#endif
