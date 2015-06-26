#include "global_define.h"
#include "types.h"
#include "clientversions.h"
#include "file_verify.h"
#include "crc32.h"
#include <stdio.h>

#pragma pack(1)
struct VerifyFileStruct
{
	uint32 crc;
	uint32 file_size;
	uint32 offset[256];
	uint32 data[256];
};
#pragma pack()

EQEmu::FileVerify::FileVerify() {
	buffer = nullptr;
	size = 0;
}

EQEmu::FileVerify::~FileVerify() {
	safe_delete_array(buffer);
}

bool EQEmu::FileVerify::Load(const char *file_name) {
	safe_delete_array(buffer);
	size = 0;

	FILE *f = fopen(file_name, "rb");
	if(!f) {
		buffer = nullptr;
		size = 0;
		return false;
	}

	fseek(f, 0U, SEEK_END);
	size = ftell(f);
	rewind(f);

	buffer = new char[size];
	auto result = fread(buffer, 1, size, f);
	fclose(f);
	
	if(result != size) {
		safe_delete_array(buffer);
		size = 0;
	}

	return true;
}

bool EQEmu::FileVerify::Verify(const char *data, uint32 size, ClientVersion version) {
	if(!buffer) {
		return true;
	}

	if(size != sizeof(VerifyFileStruct)) {
		return false;
	}

	VerifyFileStruct *vs = (VerifyFileStruct*)data;
	if(this->size != vs->file_size) {
		return false;
	}

	uint32 crc = CRC32::GenerateNoFlip((uchar*)buffer, this->size);
	if(vs->crc != crc) {
		return false;
	}

	for(int i = 0; i < 256; ++i) {
		uint32 offset = vs->offset[i] * 4;

		if((offset - 4) > this->size) {
			return false;
		}

		uint32 check = *(uint32*)(buffer + offset);
		if(check != vs->data[i]) {
			return false;
		}
	}

	return true;
}

