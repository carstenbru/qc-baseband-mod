/*
 PdcchTimeRecord.h
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#ifndef PDCCHTIMERECORD_H_
#define PDCCHTIMERECORD_H_

#include "PdcchDumpRecord.h"

class PdcchTimeRecord: public PdcchDumpRecord {
public:
	PdcchTimeRecord(unsigned int record_version, char* data, unsigned int length);
	virtual ~PdcchTimeRecord();

	virtual std::string to_string() {
		return "PdcchTimeRecord: " + getTimeString();
	}

	long getTime();
	std::string getTimeString();
	/**
	 * gets the time which is diff_ms milliseconds after the timestamp
	 */
	long getTime(long diff_ms);
	std::string getTimeString(long diff_ms);
};

#endif /* PDCCHTIMERECORD_H_ */
