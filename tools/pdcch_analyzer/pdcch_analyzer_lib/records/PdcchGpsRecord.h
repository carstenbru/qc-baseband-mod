/*
 PdcchGpsRecord.h
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#ifndef PDCCHGPSRECORD_H_
#define PDCCHGPSRECORD_H_

#include "PdcchDumpRecord.h"

class PdcchGpsRecord: public PdcchDumpRecord {
public:
	PdcchGpsRecord(unsigned int record_version, char* data, unsigned int length);
	virtual ~PdcchGpsRecord();

	virtual std::string to_string();

	double get_latitude() { return *((double*) (data)); };
	double get_longitude() { return *((double*) (data + 8)); };
	double get_altitude() { return *((double*) (data + 16)); };
	float get_bearing() { return *((float*) (data + 24)); };
	float get_accuracy() { return *((float*) (data + 28)); };
	float get_speed() { return *((float*) (data + 32)); };
	uint64_t get_time() { return *((uint64_t*) (data + 36)); };
};

#endif /* PDCCHGPSRECORD_H_ */
