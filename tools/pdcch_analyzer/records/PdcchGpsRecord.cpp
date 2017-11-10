/*
 PdcchGpsRecord.cpp
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#include "PdcchGpsRecord.h"

using namespace std;

PdcchGpsRecord::PdcchGpsRecord(unsigned int record_version, char* data,
		unsigned int length) :
		PdcchDumpRecord(record_version, data, length) {

}

PdcchGpsRecord::~PdcchGpsRecord() {
}

string PdcchGpsRecord::to_string() {
	char buffer[80];

	snprintf(buffer, sizeof(buffer), "PdcchGpsRecord: lat:%f lon:%f", get_latitude(), get_longitude());
	return string(buffer);
}
