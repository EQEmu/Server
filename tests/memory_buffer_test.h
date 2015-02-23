/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2013 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifndef __EQEMU_TESTS_MEMORY_BUFFER_H
#define __EQEMU_TESTS_MEMORY_BUFFER_H

#include "cppunit/cpptest.h"
#include "../common/memory_buffer.h"

class MemoryBufferTest : public Test::Suite {
	typedef void(MemoryBufferTest::*TestFunction)(void);
public:
	MemoryBufferTest() {
		TEST_ADD(MemoryBufferTest::WriteTest);
		TEST_ADD(MemoryBufferTest::ReadTest);
		TEST_ADD(MemoryBufferTest::ConvertTest);
		TEST_ADD(MemoryBufferTest::ResizeTest);
		TEST_ADD(MemoryBufferTest::CopyTest);
		TEST_ADD(MemoryBufferTest::AssignTest);
		TEST_ADD(MemoryBufferTest::MoveTest);
		TEST_ADD(MemoryBufferTest::ZeroTest);
		TEST_ADD(MemoryBufferTest::ClearTest);
		TEST_ADD(MemoryBufferTest::AddTest)
	}

	~MemoryBufferTest() {
	}

private:
	void WriteTest() {
		uint8 a = 0;
		uint16 b = 5 ;
		uint32 c = 10;
		uint64 d = 15;
		std::string s2 = "test2";

		mb.Write(a);
		mb.Write(b);
		mb.Write(c);
		mb.Write(d);
		mb.Write("test1");
		mb.Write(s2);

		TEST_ASSERT(mb.Size() == 27);
		
		uchar *data = (uchar*)mb;
		TEST_ASSERT(data != nullptr);
		TEST_ASSERT(data[0] == 0);
		TEST_ASSERT(data[1] == 5);
		TEST_ASSERT(data[2] == 0);
		TEST_ASSERT(data[3] == 10);
		TEST_ASSERT(data[4] == 0);
		TEST_ASSERT(data[5] == 0);
		TEST_ASSERT(data[6] == 0);
		TEST_ASSERT(data[7] == 15);
		TEST_ASSERT(data[8] == 0);
		TEST_ASSERT(data[9] == 0);
		TEST_ASSERT(data[10] == 0);
		TEST_ASSERT(data[11] == 0);
		TEST_ASSERT(data[12] == 0);
		TEST_ASSERT(data[13] == 0);
		TEST_ASSERT(data[14] == 0);
		TEST_ASSERT(data[15] == 't');
		TEST_ASSERT(data[16] == 'e');
		TEST_ASSERT(data[17] == 's');
		TEST_ASSERT(data[18] == 't');
		TEST_ASSERT(data[19] == '1');
		TEST_ASSERT(data[20] == 0);
		TEST_ASSERT(data[21] == 't');
		TEST_ASSERT(data[22] == 'e');
		TEST_ASSERT(data[23] == 's');
		TEST_ASSERT(data[24] == 't');
		TEST_ASSERT(data[25] == '2');
		TEST_ASSERT(data[26] == 0);
	}

	void ReadTest() {
		uint8 a = mb.Read<uint8>();
		uint16 b = mb.Read<uint16>();
		uint32 c = mb.Read<uint32>();
		uint64 d = mb.Read<uint64>();
		std::string s1 = mb.ReadString();
		std::string s2 = mb.ReadString();
		
		TEST_ASSERT(a == 0);
		TEST_ASSERT(b == 5);
		TEST_ASSERT(c == 10);
		TEST_ASSERT(d == 15);
		TEST_ASSERT(s1.compare("test1") == 0);
		TEST_ASSERT(s2.compare("test2") == 0);
	}

#pragma pack(1)
	struct ConvertStruct
	{
		uint8 a;
		uint16 b;
		uint32 c;
		uint64 d;
		char test1[6];
		char test2[6];
	};
#pragma pack()

	void ConvertTest()
	{
		uchar *v1 = mb;
		char *v2 = mb;
		ConvertStruct *cs = (ConvertStruct*)mb;

		TEST_ASSERT(v1 != nullptr);
		TEST_ASSERT(v1[0] == 0);
		TEST_ASSERT(v1[1] == 5);
		TEST_ASSERT(v1[2] == 0);
		TEST_ASSERT(v1[3] == 10);
		TEST_ASSERT(v1[4] == 0);
		TEST_ASSERT(v1[5] == 0);
		TEST_ASSERT(v1[6] == 0);
		TEST_ASSERT(v1[7] == 15);
		TEST_ASSERT(v1[8] == 0);
		TEST_ASSERT(v1[9] == 0);
		TEST_ASSERT(v1[10] == 0);
		TEST_ASSERT(v1[11] == 0);
		TEST_ASSERT(v1[12] == 0);
		TEST_ASSERT(v1[13] == 0);
		TEST_ASSERT(v1[14] == 0);
		TEST_ASSERT(v1[15] == 't');
		TEST_ASSERT(v1[16] == 'e');
		TEST_ASSERT(v1[17] == 's');
		TEST_ASSERT(v1[18] == 't');
		TEST_ASSERT(v1[19] == '1');
		TEST_ASSERT(v1[20] == 0);
		TEST_ASSERT(v1[21] == 't');
		TEST_ASSERT(v1[22] == 'e');
		TEST_ASSERT(v1[23] == 's');
		TEST_ASSERT(v1[24] == 't');
		TEST_ASSERT(v1[25] == '2');
		TEST_ASSERT(v1[26] == 0);

		TEST_ASSERT(v2 != nullptr);
		TEST_ASSERT(v2[0] == 0);
		TEST_ASSERT(v2[1] == 5);
		TEST_ASSERT(v2[2] == 0);
		TEST_ASSERT(v2[3] == 10);
		TEST_ASSERT(v2[4] == 0);
		TEST_ASSERT(v2[5] == 0);
		TEST_ASSERT(v2[6] == 0);
		TEST_ASSERT(v2[7] == 15);
		TEST_ASSERT(v2[8] == 0);
		TEST_ASSERT(v2[9] == 0);
		TEST_ASSERT(v2[10] == 0);
		TEST_ASSERT(v2[11] == 0);
		TEST_ASSERT(v2[12] == 0);
		TEST_ASSERT(v2[13] == 0);
		TEST_ASSERT(v2[14] == 0);
		TEST_ASSERT(v2[15] == 't');
		TEST_ASSERT(v2[16] == 'e');
		TEST_ASSERT(v2[17] == 's');
		TEST_ASSERT(v2[18] == 't');
		TEST_ASSERT(v2[19] == '1');
		TEST_ASSERT(v2[20] == 0);
		TEST_ASSERT(v2[21] == 't');
		TEST_ASSERT(v2[22] == 'e');
		TEST_ASSERT(v2[23] == 's');
		TEST_ASSERT(v2[24] == 't');
		TEST_ASSERT(v2[25] == '2');
		TEST_ASSERT(v2[26] == 0);

		TEST_ASSERT(cs != nullptr);
		TEST_ASSERT(cs->a == 0);
		TEST_ASSERT(cs->b == 5);
		TEST_ASSERT(cs->c == 10);
		TEST_ASSERT(cs->d == 15);
		TEST_ASSERT(strcmp(cs->test1, "test1") == 0);
		TEST_ASSERT(strcmp(cs->test2, "test2") == 0);
	}

	void ResizeTest()
	{
		mb.Resize(21);
		TEST_ASSERT(mb.Size() == 21);

		mb.Resize(27);
		uchar *data = (uchar*)mb;
		TEST_ASSERT(data != nullptr);
		TEST_ASSERT(data[21] == 't');
		TEST_ASSERT(data[22] == 'e');
		TEST_ASSERT(data[23] == 's');
		TEST_ASSERT(data[24] == 't');
		TEST_ASSERT(data[25] == '2');
		TEST_ASSERT(data[26] == 0);

		mb.Resize(40);
		data = (uchar*)mb;
		TEST_ASSERT(data != nullptr);
		TEST_ASSERT(mb.Capacity() >= 40);
		TEST_ASSERT(mb.Size() == 40);
		TEST_ASSERT(data[0] == 0);
		TEST_ASSERT(data[1] == 5);
		TEST_ASSERT(data[2] == 0);
		TEST_ASSERT(data[3] == 10);
		TEST_ASSERT(data[4] == 0);
		TEST_ASSERT(data[5] == 0);
		TEST_ASSERT(data[6] == 0);
		TEST_ASSERT(data[7] == 15);
		TEST_ASSERT(data[8] == 0);
		TEST_ASSERT(data[9] == 0);
		TEST_ASSERT(data[10] == 0);
		TEST_ASSERT(data[11] == 0);
		TEST_ASSERT(data[12] == 0);
		TEST_ASSERT(data[13] == 0);
		TEST_ASSERT(data[14] == 0);
		TEST_ASSERT(data[15] == 't');
		TEST_ASSERT(data[16] == 'e');
		TEST_ASSERT(data[17] == 's');
		TEST_ASSERT(data[18] == 't');
		TEST_ASSERT(data[19] == '1');
		TEST_ASSERT(data[20] == 0);
		TEST_ASSERT(data[21] == 't');
		TEST_ASSERT(data[22] == 'e');
		TEST_ASSERT(data[23] == 's');
		TEST_ASSERT(data[24] == 't');
		TEST_ASSERT(data[25] == '2');
		TEST_ASSERT(data[26] == 0);
	}

	void CopyTest()
	{
		EQEmu::MemoryBuffer mb2(mb);

		uchar *data = (uchar*)mb2;
		TEST_ASSERT(data != nullptr);
		TEST_ASSERT(mb.Size() == 40);
		TEST_ASSERT(data[0] == 0);
		TEST_ASSERT(data[1] == 5);
		TEST_ASSERT(data[2] == 0);
		TEST_ASSERT(data[3] == 10);
		TEST_ASSERT(data[4] == 0);
		TEST_ASSERT(data[5] == 0);
		TEST_ASSERT(data[6] == 0);
		TEST_ASSERT(data[7] == 15);
		TEST_ASSERT(data[8] == 0);
		TEST_ASSERT(data[9] == 0);
		TEST_ASSERT(data[10] == 0);
		TEST_ASSERT(data[11] == 0);
		TEST_ASSERT(data[12] == 0);
		TEST_ASSERT(data[13] == 0);
		TEST_ASSERT(data[14] == 0);
		TEST_ASSERT(data[15] == 't');
		TEST_ASSERT(data[16] == 'e');
		TEST_ASSERT(data[17] == 's');
		TEST_ASSERT(data[18] == 't');
		TEST_ASSERT(data[19] == '1');
		TEST_ASSERT(data[20] == 0);
		TEST_ASSERT(data[21] == 't');
		TEST_ASSERT(data[22] == 'e');
		TEST_ASSERT(data[23] == 's');
		TEST_ASSERT(data[24] == 't');
		TEST_ASSERT(data[25] == '2');
		TEST_ASSERT(data[26] == 0);

		data = (uchar*)mb;
		TEST_ASSERT(data != nullptr);
		TEST_ASSERT(mb.Size() == 40);
		TEST_ASSERT(data[0] == 0);
		TEST_ASSERT(data[1] == 5);
		TEST_ASSERT(data[2] == 0);
		TEST_ASSERT(data[3] == 10);
		TEST_ASSERT(data[4] == 0);
		TEST_ASSERT(data[5] == 0);
		TEST_ASSERT(data[6] == 0);
		TEST_ASSERT(data[7] == 15);
		TEST_ASSERT(data[8] == 0);
		TEST_ASSERT(data[9] == 0);
		TEST_ASSERT(data[10] == 0);
		TEST_ASSERT(data[11] == 0);
		TEST_ASSERT(data[12] == 0);
		TEST_ASSERT(data[13] == 0);
		TEST_ASSERT(data[14] == 0);
		TEST_ASSERT(data[15] == 't');
		TEST_ASSERT(data[16] == 'e');
		TEST_ASSERT(data[17] == 's');
		TEST_ASSERT(data[18] == 't');
		TEST_ASSERT(data[19] == '1');
		TEST_ASSERT(data[20] == 0);
		TEST_ASSERT(data[21] == 't');
		TEST_ASSERT(data[22] == 'e');
		TEST_ASSERT(data[23] == 's');
		TEST_ASSERT(data[24] == 't');
		TEST_ASSERT(data[25] == '2');
		TEST_ASSERT(data[26] == 0);
		TEST_ASSERT((void*)mb != (void*)mb2);
	}

	void AssignTest()
	{
		EQEmu::MemoryBuffer mb2 = mb;

		uchar *data = (uchar*)mb2;
		TEST_ASSERT(data != nullptr);
		TEST_ASSERT(mb.Size() == 40);
		TEST_ASSERT(data[0] == 0);
		TEST_ASSERT(data[1] == 5);
		TEST_ASSERT(data[2] == 0);
		TEST_ASSERT(data[3] == 10);
		TEST_ASSERT(data[4] == 0);
		TEST_ASSERT(data[5] == 0);
		TEST_ASSERT(data[6] == 0);
		TEST_ASSERT(data[7] == 15);
		TEST_ASSERT(data[8] == 0);
		TEST_ASSERT(data[9] == 0);
		TEST_ASSERT(data[10] == 0);
		TEST_ASSERT(data[11] == 0);
		TEST_ASSERT(data[12] == 0);
		TEST_ASSERT(data[13] == 0);
		TEST_ASSERT(data[14] == 0);
		TEST_ASSERT(data[15] == 't');
		TEST_ASSERT(data[16] == 'e');
		TEST_ASSERT(data[17] == 's');
		TEST_ASSERT(data[18] == 't');
		TEST_ASSERT(data[19] == '1');
		TEST_ASSERT(data[20] == 0);
		TEST_ASSERT(data[21] == 't');
		TEST_ASSERT(data[22] == 'e');
		TEST_ASSERT(data[23] == 's');
		TEST_ASSERT(data[24] == 't');
		TEST_ASSERT(data[25] == '2');
		TEST_ASSERT(data[26] == 0);

		data = (uchar*)mb;
		TEST_ASSERT(data != nullptr);
		TEST_ASSERT(mb.Size() == 40);
		TEST_ASSERT(data[0] == 0);
		TEST_ASSERT(data[1] == 5);
		TEST_ASSERT(data[2] == 0);
		TEST_ASSERT(data[3] == 10);
		TEST_ASSERT(data[4] == 0);
		TEST_ASSERT(data[5] == 0);
		TEST_ASSERT(data[6] == 0);
		TEST_ASSERT(data[7] == 15);
		TEST_ASSERT(data[8] == 0);
		TEST_ASSERT(data[9] == 0);
		TEST_ASSERT(data[10] == 0);
		TEST_ASSERT(data[11] == 0);
		TEST_ASSERT(data[12] == 0);
		TEST_ASSERT(data[13] == 0);
		TEST_ASSERT(data[14] == 0);
		TEST_ASSERT(data[15] == 't');
		TEST_ASSERT(data[16] == 'e');
		TEST_ASSERT(data[17] == 's');
		TEST_ASSERT(data[18] == 't');
		TEST_ASSERT(data[19] == '1');
		TEST_ASSERT(data[20] == 0);
		TEST_ASSERT(data[21] == 't');
		TEST_ASSERT(data[22] == 'e');
		TEST_ASSERT(data[23] == 's');
		TEST_ASSERT(data[24] == 't');
		TEST_ASSERT(data[25] == '2');
		TEST_ASSERT(data[26] == 0);
		TEST_ASSERT((void*)mb != (void*)mb2);
	}

	void MoveTest()
	{
		EQEmu::MemoryBuffer mb2 = std::move(mb);
		uchar *data = (uchar*)mb;
		uchar *data2 = (uchar*)mb2;
		TEST_ASSERT(data == nullptr);
		TEST_ASSERT(data2 != nullptr);
		TEST_ASSERT(mb.Size() == 0);
		TEST_ASSERT(mb2.Size() == 40);
		TEST_ASSERT(data2[0] == 0);
		TEST_ASSERT(data2[1] == 5);
		TEST_ASSERT(data2[2] == 0);
		TEST_ASSERT(data2[3] == 10);
		TEST_ASSERT(data2[4] == 0);
		TEST_ASSERT(data2[5] == 0);
		TEST_ASSERT(data2[6] == 0);
		TEST_ASSERT(data2[7] == 15);
		TEST_ASSERT(data2[8] == 0);
		TEST_ASSERT(data2[9] == 0);
		TEST_ASSERT(data2[10] == 0);
		TEST_ASSERT(data2[11] == 0);
		TEST_ASSERT(data2[12] == 0);
		TEST_ASSERT(data2[13] == 0);
		TEST_ASSERT(data2[14] == 0);
		TEST_ASSERT(data2[15] == 't');
		TEST_ASSERT(data2[16] == 'e');
		TEST_ASSERT(data2[17] == 's');
		TEST_ASSERT(data2[18] == 't');
		TEST_ASSERT(data2[19] == '1');
		TEST_ASSERT(data2[20] == 0);
		TEST_ASSERT(data2[21] == 't');
		TEST_ASSERT(data2[22] == 'e');
		TEST_ASSERT(data2[23] == 's');
		TEST_ASSERT(data2[24] == 't');
		TEST_ASSERT(data2[25] == '2');
		TEST_ASSERT(data2[26] == 0);

		mb = std::move(mb2);

		data = (uchar*)mb;
		data2 = (uchar*)mb2;
		TEST_ASSERT(data != nullptr);
		TEST_ASSERT(data2 == nullptr);
		TEST_ASSERT(mb.Size() == 40);
		TEST_ASSERT(mb2.Size() == 0);
		TEST_ASSERT(data[0] == 0);
		TEST_ASSERT(data[1] == 5);
		TEST_ASSERT(data[2] == 0);
		TEST_ASSERT(data[3] == 10);
		TEST_ASSERT(data[4] == 0);
		TEST_ASSERT(data[5] == 0);
		TEST_ASSERT(data[6] == 0);
		TEST_ASSERT(data[7] == 15);
		TEST_ASSERT(data[8] == 0);
		TEST_ASSERT(data[9] == 0);
		TEST_ASSERT(data[10] == 0);
		TEST_ASSERT(data[11] == 0);
		TEST_ASSERT(data[12] == 0);
		TEST_ASSERT(data[13] == 0);
		TEST_ASSERT(data[14] == 0);
		TEST_ASSERT(data[15] == 't');
		TEST_ASSERT(data[16] == 'e');
		TEST_ASSERT(data[17] == 's');
		TEST_ASSERT(data[18] == 't');
		TEST_ASSERT(data[19] == '1');
		TEST_ASSERT(data[20] == 0);
		TEST_ASSERT(data[21] == 't');
		TEST_ASSERT(data[22] == 'e');
		TEST_ASSERT(data[23] == 's');
		TEST_ASSERT(data[24] == 't');
		TEST_ASSERT(data[25] == '2');
		TEST_ASSERT(data[26] == 0);
	}

	void ZeroTest()
	{
		mb.Zero();
		uchar *data = (uchar*)mb;
		TEST_ASSERT(data != nullptr);
		TEST_ASSERT(mb.Size() == 40);
		TEST_ASSERT(data[0] == 0);
		TEST_ASSERT(data[1] == 0);
		TEST_ASSERT(data[2] == 0);
		TEST_ASSERT(data[3] == 0);
		TEST_ASSERT(data[4] == 0);
		TEST_ASSERT(data[5] == 0);
		TEST_ASSERT(data[6] == 0);
		TEST_ASSERT(data[7] == 0);
		TEST_ASSERT(data[8] == 0);
		TEST_ASSERT(data[9] == 0);
		TEST_ASSERT(data[10] == 0);
		TEST_ASSERT(data[11] == 0);
		TEST_ASSERT(data[12] == 0);
		TEST_ASSERT(data[13] == 0);
		TEST_ASSERT(data[14] == 0);
		TEST_ASSERT(data[15] == 0);
		TEST_ASSERT(data[16] == 0);
		TEST_ASSERT(data[17] == 0);
		TEST_ASSERT(data[18] == 0);
		TEST_ASSERT(data[19] == 0);
		TEST_ASSERT(data[20] == 0);
		TEST_ASSERT(data[21] == 0);
		TEST_ASSERT(data[22] == 0);
		TEST_ASSERT(data[23] == 0);
		TEST_ASSERT(data[24] == 0);
		TEST_ASSERT(data[25] == 0);
		TEST_ASSERT(data[26] == 0);
		TEST_ASSERT(data[27] == 0);
		TEST_ASSERT(data[28] == 0);
		TEST_ASSERT(data[29] == 0);
		TEST_ASSERT(data[30] == 0);
		TEST_ASSERT(data[31] == 0);
		TEST_ASSERT(data[32] == 0);
		TEST_ASSERT(data[33] == 0);
		TEST_ASSERT(data[34] == 0);
		TEST_ASSERT(data[35] == 0);
		TEST_ASSERT(data[36] == 0);
		TEST_ASSERT(data[37] == 0);
		TEST_ASSERT(data[38] == 0);
		TEST_ASSERT(data[39] == 0);
	}

	void ClearTest()
	{
		mb.Clear();
		TEST_ASSERT(!mb);
		uchar *data = (uchar*)mb;
		TEST_ASSERT(data == nullptr);
	}

	void AddTest()
	{
		EQEmu::MemoryBuffer mb2;
		EQEmu::MemoryBuffer mb3;
		
		mb2 += mb3;

		TEST_ASSERT(!mb2);
		TEST_ASSERT(mb2.Size() == 0);
		
		mb2.Write("test1");
		mb2.Write("test2");

		mb2 += mb3;
		TEST_ASSERT(mb2);
		TEST_ASSERT(mb2.Size() == 12);

		uchar *data = (uchar*)mb2;
		TEST_ASSERT(data != nullptr);
		TEST_ASSERT(data[0] == 't');
		TEST_ASSERT(data[1] == 'e');
		TEST_ASSERT(data[2] == 's');
		TEST_ASSERT(data[3] == 't');
		TEST_ASSERT(data[4] == '1');
		TEST_ASSERT(data[5] == 0);
		TEST_ASSERT(data[6] == 't');
		TEST_ASSERT(data[7] == 'e');
		TEST_ASSERT(data[8] == 's');
		TEST_ASSERT(data[9] == 't');
		TEST_ASSERT(data[10] == '2');
		TEST_ASSERT(data[11] == 0);

		mb3 += mb2;
		TEST_ASSERT(mb3);
		TEST_ASSERT(mb3.Size() == 12);

		data = (uchar*)mb3;
		TEST_ASSERT(data != nullptr);
		TEST_ASSERT(data[0] == 't');
		TEST_ASSERT(data[1] == 'e');
		TEST_ASSERT(data[2] == 's');
		TEST_ASSERT(data[3] == 't');
		TEST_ASSERT(data[4] == '1');
		TEST_ASSERT(data[5] == 0);
		TEST_ASSERT(data[6] == 't');
		TEST_ASSERT(data[7] == 'e');
		TEST_ASSERT(data[8] == 's');
		TEST_ASSERT(data[9] == 't');
		TEST_ASSERT(data[10] == '2');
		TEST_ASSERT(data[11] == 0);

		mb2.Clear();
		mb3.Clear();

		mb2.Write("test1");
		mb3.Write("test2");

		mb2 += mb3;
		TEST_ASSERT(mb2);
		TEST_ASSERT(mb2.Size() == 12);

		data = (uchar*)mb2;
		TEST_ASSERT(data != nullptr);
		TEST_ASSERT(data[0] == 't');
		TEST_ASSERT(data[1] == 'e');
		TEST_ASSERT(data[2] == 's');
		TEST_ASSERT(data[3] == 't');
		TEST_ASSERT(data[4] == '1');
		TEST_ASSERT(data[5] == 0);
		TEST_ASSERT(data[6] == 't');
		TEST_ASSERT(data[7] == 'e');
		TEST_ASSERT(data[8] == 's');
		TEST_ASSERT(data[9] == 't');
		TEST_ASSERT(data[10] == '2');
		TEST_ASSERT(data[11] == 0);
	}

	EQEmu::MemoryBuffer mb;
};

#endif
