#ifndef EQTIME_H
#define EQTIME_H

#include "../common/eq_packet_structs.h"
#include <string>

using namespace std;

//Struct
struct eqTimeOfDay
{
	TimeOfDay_Struct start_eqtime;
	time_t	start_realtime;
};

//Class Def
class EQTime
{
public:
	//Constructor/destructor
	EQTime(TimeOfDay_Struct start_eq, time_t start_real);
	EQTime();
	~EQTime();

	//Get functions
	int getEQTimeOfDay( TimeOfDay_Struct *eqTimeOfDay ) { return(getEQTimeOfDay(time(nullptr), eqTimeOfDay)); }
	int getEQTimeOfDay( time_t timeConvert, TimeOfDay_Struct *eqTimeOfDay );
	TimeOfDay_Struct getStartEQTime() { return eqTime.start_eqtime; }
	time_t getStartRealTime() { return eqTime.start_realtime; }
	uint32 getEQTimeZone() { return timezone; }
	uint32 getEQTimeZoneHr() { return timezone/60; }
	uint32 getEQTimeZoneMin() { return timezone%60; }

	//Set functions
	int setEQTimeOfDay(TimeOfDay_Struct start_eq, time_t start_real);
	void setEQTimeZone(int32 in_timezone) { timezone=in_timezone; }
	
	//Time math/logic functions
	static bool IsTimeBefore(TimeOfDay_Struct *base, TimeOfDay_Struct *test);	//is test before base
	static void AddMinutes(uint32 minutes, TimeOfDay_Struct *to);
	
	static void ToString(TimeOfDay_Struct *t, string &str);
	
	//Database functions
	//bool loadDB(Database q);
	//bool setDB(Database q);
	bool loadFile(const char *filename);
	bool saveFile(const char *filename);

private:
	//This is our reference clock.
	eqTimeOfDay eqTime;
	//This is our tz offset
	int32 timezone;
};

#endif
