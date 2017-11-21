/*
 PdcchTimeRecord.cpp
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#include "PdcchTimeRecord.h"
#include <time.h>

using namespace std;

PdcchTimeRecord::PdcchTimeRecord(unsigned int record_version, char* data,
		unsigned int length) :
		PdcchDumpRecord(record_version, data, length) {

}

PdcchTimeRecord::~PdcchTimeRecord() {
}

long PdcchTimeRecord::getTime(long diff_ms) {
	return *((long*) data) + diff_ms;
}

string PdcchTimeRecord::getTimeString(long diff_ms) {
	long rawtime = getTime(diff_ms) / 1000;

	char buffer[80];
	struct tm * timeinfo;

	timeinfo = localtime(&rawtime);

	strftime(buffer, 80, "%d.%m.%Y %H:%M:%S", timeinfo);
	return string(buffer);
}

long PdcchTimeRecord::getTime() {
	return getTime(0);
}

string PdcchTimeRecord::getTimeString() {
	return getTimeString(0);
}
