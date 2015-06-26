#ifndef EQEMU_COMMON_FILE_VERIFY_H
#define EQEMU_COMMON_FILE_VERIFY_H

namespace EQEmu
{
	class FileVerify
	{
	public:
		FileVerify();
		~FileVerify();

		bool Load(const char *file_name);
		bool Verify(const char *data, uint32 size, ClientVersion version);

	private:
		char *buffer;
		uint32 size;
	};
}

#endif
