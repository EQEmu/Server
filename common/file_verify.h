#ifndef EQEMU_COMMON_FILE_VERIFY_H
#define EQEMU_COMMON_FILE_VERIFY_H

namespace EQEmu
{
	class FileVerify
	{
	public:
		FileVerify(const char *file_name);
		~FileVerify();

		bool Verify(const char *data, uint32 size);

	private:
		char *buffer;
		uint32 size;
	};
}

#endif
