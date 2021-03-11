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

#ifndef __EQEMU_TESTS_STRING_UTIL_H
#define __EQEMU_TESTS_STRING_UTIL_H

#include "cppunit/cpptest.h"
#include "../common/string_util.h"

class StringUtilTest : public Test::Suite {
	typedef void(StringUtilTest::*TestFunction)(void);
public:
	StringUtilTest() {
		TEST_ADD(StringUtilTest::StringFormatTest);
		TEST_ADD(StringUtilTest::EscapeStringTest);
		TEST_ADD(StringUtilTest::EscapeStringMemoryTest);
		TEST_ADD(StringUtilTest::SearchDeliminatedStringTest);
	}

	~StringUtilTest() {
	}

	private:
	void StringFormatTest() {
		const char* fmt = "Test: %c %d %4.2f";
		char c = 'a';
		int i = 2014;
		float f = 3.1416;
		
		auto s = StringFormat(fmt, c, i, f);
		TEST_ASSERT_EQUALS(s.length(), 17);
		TEST_ASSERT(s.compare("Test: a 2014 3.14") == 0);
	}

	void EscapeStringTest() {
		std::string t;
		t.resize(10);
		t[0] = 'a';
		t[1] = 'b';
		t[2] = 'c';
		t[3] = '\x00';
		t[4] = '\n';
		t[5] = '\r';
		t[6] = '\\';
		t[7] = '\'';
		t[8] = '\"';
		t[9] = '\x1a';

		auto s = EscapeString(t);
		TEST_ASSERT(s.compare("abc\\x00\\n\\r\\\\\\'\\\"\\x1a") == 0);
	}

	void EscapeStringMemoryTest() {
		char t[10] = { 0 };
		t[0] = 'a';
		t[1] = 'b';
		t[2] = 'c';
		t[3] = '\x00';
		t[4] = '\n';
		t[5] = '\r';
		t[6] = '\\';
		t[7] = '\'';
		t[8] = '\"';
		t[9] = '\x1a';

		auto s = EscapeString(t, 10);
		TEST_ASSERT(s.compare("abc\\x00\\n\\r\\\\\\'\\\"\\x1a") == 0);
	}

	void SearchDeliminatedStringTest() {
		std::string h =
		    "befallen,charasis,dalnir,frontiermtns,gukbottom,iceclad,lakeofillomen,northkarana,qey2hh1,soldunga,southro,wakening,podisease,velketor,akheva,riwwi,bothunder,poair";
		TEST_ASSERT(search_deliminated_string(h, "befallen") == 0);
		TEST_ASSERT(search_deliminated_string(h, "charasis") == 9);
		TEST_ASSERT(search_deliminated_string(h, "dalnir") == 18);
		TEST_ASSERT(search_deliminated_string(h, "frontiermtns") == 25);
		TEST_ASSERT(search_deliminated_string(h, "gukbottom") == 38);
		TEST_ASSERT(search_deliminated_string(h, "iceclad") == 48);
		TEST_ASSERT(search_deliminated_string(h, "lakeofillomen") == 56);
		TEST_ASSERT(search_deliminated_string(h, "northkarana") == 70);
		TEST_ASSERT(search_deliminated_string(h, "qey2hh1") == 82);
		TEST_ASSERT(search_deliminated_string(h, "soldunga") == 90);
		TEST_ASSERT(search_deliminated_string(h, "southro") == 99);
		TEST_ASSERT(search_deliminated_string(h, "wakening") == 107);
		TEST_ASSERT(search_deliminated_string(h, "podisease") == 116);
		TEST_ASSERT(search_deliminated_string(h, "velketor") == 126);
		TEST_ASSERT(search_deliminated_string(h, "akheva") == 135);
		TEST_ASSERT(search_deliminated_string(h, "riwwi") == 142);
		TEST_ASSERT(search_deliminated_string(h, "bothunder") == 148);
		TEST_ASSERT(search_deliminated_string(h, "poair") == 158);
		TEST_ASSERT(search_deliminated_string(h, "pod") == std::string::npos);
		TEST_ASSERT(search_deliminated_string(h, "air") == std::string::npos);
		TEST_ASSERT(search_deliminated_string(h, "bef") == std::string::npos);
		TEST_ASSERT(search_deliminated_string(h, "wwi") == std::string::npos);
	}
};

#endif
