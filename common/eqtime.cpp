/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

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

#include <fstream>
#include "../common/debug.h"
#include "../common/eqtime.h"
#include "../common/eq_packet_structs.h"
#include <memory.h>
#include <iostream>
using namespace std;
	/*#ifdef _CRTDBG_MAP_ALLOC
		#undef new
	#endif*/
	/*#ifdef _CRTDBG_MAP_ALLOC
		#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
	#endif*/

#define EQT_VERSION 1000

//Constructor
//Input: Starting EQ Time, Starting Real Time.
EQTime::EQTime(TimeOfDay_Struct start_eq, time_t start_real)
{
	eqTime.start_eqtime=start_eq;
	eqTime.start_realtime=start_real;
	timezone=0;
}

EQTime::EQTime()
{
	timezone=0;
	memset(&eqTime, 0, sizeof(eqTime));
	//Defaults for time
	TimeOfDay_Struct start;
	start.day=1;
	start.hour=9;
	start.minute=0;
	start.month=1;
	start.year=3100;
	//Set default time zone
	timezone=0;
	//Start EQTimer
	setEQTimeOfDay(start, time(0));
}

EQTime::~EQTime()
{
}

//getEQTimeOfDay - Reads timeConvert and writes the result to eqTimeOfDay
//This function was written by the ShowEQ Project.
//Input: Current Time (as a time_t), a pointer to the TimeOfDay_Struct that will be written to.
//Output: 0=Error, 1=Sucess

int EQTime::getEQTimeOfDay( time_t timeConvert, struct TimeOfDay_Struct *eqTimeOfDay )
{
	/* check to see if we have a reference time to go by. */
	if( eqTime.start_realtime == 0 )
		return 0;

	unsigned long diff = timeConvert - eqTime.start_realtime;

	/* There are 3 seconds per 1 EQ Minute */
	diff /= 3;

	/* Start off timezone offset */

	int32 ntz = timezone;

	/* The minutes range from 0 - 59 */
	diff += eqTime.start_eqtime.minute + (ntz%60);
	eqTimeOfDay->minute = diff % 60;
	diff /= 60;
	ntz /= 60;

	// The hours range from 1-24
	// 1 = 1am
	// 2 = 2am
	// ...
	// 23 = 11 pm
	// 24 = 12 am
	//
	// Modify it so that it works from
	// 0-23 for our calculations
	diff += ( eqTime.start_eqtime.hour - 1) + (ntz%24);
	eqTimeOfDay->hour = (diff%24) + 1;
	diff /= 24;
	ntz /= 24;

	// The days range from 1-28
	// Modify it so that it works from
	// 0-27 for our calculations
	diff += ( eqTime.start_eqtime.day - 1 ) + (ntz%28);
	eqTimeOfDay->day = (diff%28) + 1;
	diff /= 28;
	ntz /= 28;

	// The months range from 1-12
	// Modify it so that it works from
	// 0-11 for our calculations
	diff += ( eqTime.start_eqtime.month - 1 ) + (ntz%12);
	eqTimeOfDay->month = (diff%12) + 1;
	diff /= 12;
	ntz /= 12;

	eqTimeOfDay->year = eqTime.start_eqtime.year + diff + ntz;

	return 1;
}

//setEQTimeOfDay
int EQTime::setEQTimeOfDay(TimeOfDay_Struct start_eq, time_t start_real)
{
	if(start_real==0)
		return 0;
	eqTime.start_eqtime=start_eq;
	eqTime.start_realtime=start_real;
	return 1;
}

//saveFile and loadFile need to use long for the save datatype...
//For some reason, ifstream/ofstream have problems with EQEmu datatypes in files.
bool EQTime::saveFile(const char *filename)
{
	ofstream of;
	of.open(filename);
	if(!of)
	{
		LogFile->write(EQEMuLog::Error, "EQTime::saveFile failed: Unable to open file '%s'", filename);
		return false;
	}
	//Enable for debugging
	//cout << "SAVE: day=" << (long)eqTime.start_eqtime.day << ";hour=" << (long)eqTime.start_eqtime.hour << ";min=" << (long)eqTime.start_eqtime.minute << ";mon=" << (long)eqTime.start_eqtime.month << ";yr=" << eqTime.start_eqtime.year << ";timet=" << eqTime.start_realtime << endl;
	of << EQT_VERSION << endl;
	of << (long)eqTime.start_eqtime.day << endl;
	of << (long)eqTime.start_eqtime.hour << endl;
	of << (long)eqTime.start_eqtime.minute << endl;
	of << (long)eqTime.start_eqtime.month << endl;
	of << eqTime.start_eqtime.year << endl;
	of << eqTime.start_realtime << endl;
	of.close();
	return true;
}

bool EQTime::loadFile(const char *filename)
{
	int version=0;
	long in_data=0;
	ifstream in;
	in.open(filename);
	if(!in)
	{
		LogFile->write(EQEMuLog::Error, "Could not load EQTime file %s", filename);
		return false;
	}
	in >> version;
	in.ignore(80, '\n');
	if(version != EQT_VERSION)
	{
		LogFile->write(EQEMuLog::Error, "'%s' is NOT a valid EQTime file. File version is %i, EQTime version is %i", filename, version, EQT_VERSION);
		return false;
	}
	//in >> eqTime.start_eqtime.day;
	in >> in_data;
	in.ignore(80, '\n');
	eqTime.start_eqtime.day = in_data;
	//in >> eqTime.start_eqtime.hour;
	in >> in_data;
	eqTime.start_eqtime.hour = in_data;
	in.ignore(80, '\n');
	//in >> eqTime.start_eqtime.minute;
	in >> in_data;
	in.ignore(80, '\n');
	eqTime.start_eqtime.minute = in_data;
	//in >> eqTime.start_eqtime.month;
	in >> in_data;
	in.ignore(80, '\n');
	eqTime.start_eqtime.month = in_data;
	in >> eqTime.start_eqtime.year;
	in.ignore(80, '\n');
	in >> eqTime.start_realtime;
	//Enable for debugging...
	//cout << "LOAD: day=" << (long)eqTime.start_eqtime.day << ";hour=" << (long)eqTime.start_eqtime.hour << ";min=" << (long)eqTime.start_eqtime.minute << ";mon=" << (long)eqTime.start_eqtime.month << ";yr=" << eqTime.start_eqtime.year << ";timet=" << eqTime.start_realtime << endl;
	in.close();
	return true;
}


bool EQTime::IsTimeBefore(TimeOfDay_Struct *base, TimeOfDay_Struct *test) {
	if(base->year > test->year)
		return(true);
	if(base->year < test->year)
		return(false);
	//same years
	if(base->month > test->month)
		return(true);
	if(base->month < test->month)
		return(false);
	//same month
	if(base->day > test->day)
		return(true);
	if(base->day < test->day)
		return(false);
	//same day
	if(base->hour > test->hour)
		return(true);
	if(base->hour < test->hour)
		return(false);
	//same hour...
	return(base->minute > test->minute);
}


void EQTime::AddMinutes(uint32 minutes, TimeOfDay_Struct *to) {
	uint32 cur;

	//minutes start at 0, everything else starts at 1
	cur = to->minute;
	cur += minutes;
	if(cur < 60) {
		to->minute = cur;
		return;
	}
	to->minute = cur % 60;
	//carry hours
	cur /= 60;
	cur += to->hour;
	if(cur <= 24) {
		to->hour = cur;
		return;
	}
	to->hour = ((cur-1) % 24) + 1;
	//carry days
	cur = (cur-1) / 24;
	cur += to->day;
	if(cur <= 28) {
		to->day = cur;
		return;
	}
	to->day = ((cur-1) % 28) + 1;
	//carry months
	cur = (cur-1) / 28;
	cur += to->month;
	if(cur <= 12) {
		to->month = cur;
		return;
	}
	to->month = ((cur-1) % 12) + 1;
	//carry years
	to->year += (cur-1) / 12;
}

void EQTime::ToString(TimeOfDay_Struct *t, string &str) {
	char buf[128];
	snprintf(buf, 128, "%.2d/%.2d/%.4d %.2d:%.2d",
		t->month, t->day, t->year, t->hour, t->minute);
	buf[127] = '\0';
	str = buf;
}

