#include "global_define.h"
#include "types.h"
#include "file_verify.h"
#include "crc32.h"
#include <stdio.h>

struct VerifyFileStruct
{
	uint32 crc;
	uint32 file_size;
	uint32 offset[256];
	uint32 data[256];
};


EQEmu::FileVerify::FileVerify(const char *file_name) {
	FILE *f = fopen(file_name, "rb");
	if(!f) {
		buffer = nullptr;
		size = 0;
		return;
	}

	fseek(f, 0U, SEEK_END);
	size = ftell(f);
	rewind(f);

	char *buffer = new char[size];
	auto result = fread(buffer, 1, size, f);
	fclose(f);
	
	if(result != size) {
		safe_delete_array(buffer);
		size = 0;
	}
}

EQEmu::FileVerify::~FileVerify() {
	safe_delete_array(buffer);
}

bool EQEmu::FileVerify::Verify(const char *data, uint32 size) {
	if(!buffer) {
		return true;
	}

	if(size != sizeof(VerifyFileStruct)) {
		return false;
	}

	VerifyFileStruct *vs = (VerifyFileStruct*)data;
	if(size != vs->file_size) {
		return false;
	}

	uint32 crc = CRC32::GenerateNoFlip((uchar*)buffer, size);
	if(vs->crc != crc) {
		return false;
	}

	for(int i = 0; i < 256; ++i) {
		uint32 offset = vs->offset[i] * 4;

		if((offset - 4) > size) {
			return false;
		}

		uint32 check = *(uint32*)(buffer + offset);
		if(check != vs->data[i]) {
			return false;
		}
	}

	return true;
}

//bool VerifyFile(const EQApplicationPacket *app, const char* filename) {
//	FILE *f = fopen(filename, "rb");
//	if(!f) {
//		return false;
//	}
//
//	VerifyFileStruct *vs = (VerifyFileStruct*)app->pBuffer;
//
//	fseek(f, 0U, SEEK_END);
//	auto size = ftell(f);
//	rewind(f);
//
//	if(size != vs->file_size || size < 1024) {
//		fclose(f);
//		return false;
//	}
//
//	char *buffer = new char[size];
//	std::unique_ptr<char> data(buffer);
//	auto result = fread(buffer, 1, size, f);
//	fclose(f);
//
//	if(result != size) {
//		return false;
//	}
//
//	uint32 crc = CRC32::GenerateNoFlip((uchar*)buffer, size);
//	
//	Log.Out(Logs::General, Logs::Status, "CRC %u vs %u", crc, vs->crc);
//
//	for(int i = 0; i < 256; ++i) {
//		uint32 offset = vs->check[i] * 4;
//
//		Log.Out(Logs::General, Logs::Status, "Data: %c%c%c%c vs %c%c%c%c", vs->data[i * 4], vs->data[i * 4 + 1], vs->data[i * 4 + 2], vs->data[i * 4 + 3],
//				buffer[offset], buffer[offset + 1], buffer[offset + 2], buffer[offset + 3]);
//
//		if(buffer[offset] != vs->data[i * 4] || 
//		   buffer[offset + 1] != vs->data[i * 4 + 1] ||
//		   buffer[offset + 2] != vs->data[i * 4 + 2] ||
//		   buffer[offset + 3] != vs->data[i * 4 + 3]) 
//		{
//			return false;
//		}
//	}
//	
//	return true;
//}