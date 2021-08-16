#pragma once
#include <iostream>
#include "UDPFuncs.h"
#include <sysinfoapi.h>
#include "customFFB.h"
#include "util.h"
#include <string>

#define MSGLENGTH 10
#define QUEUELENGTH 100
ULONGLONG const loopTimeMS = 10;
struct driverMessage {
	__int8 throttle;
	__int8 stAngle;
	LONGLONG time;
	unsigned char message[MSGLENGTH] = {};
	driverMessage* next;
	ULONGLONG messageID;
	BOOL latestRead = TRUE; // a flag that says whether the last poll/readstate was successful
};
class messageQueue;
void getHRTimeMS(LARGE_INTEGER*, LARGE_INTEGER*, LONGLONG &out);
driverMessage getDriverInputs(LARGE_INTEGER& frequency);