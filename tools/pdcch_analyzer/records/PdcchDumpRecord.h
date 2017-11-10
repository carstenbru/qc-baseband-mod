/*
 PdcchDumpRecord.h
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#ifndef PDCCHDUMPRECORD_H_
#define PDCCHDUMPRECORD_H_

#include <string>

class PdcchDumpRecord {
public:
	PdcchDumpRecord(unsigned int record_version, char* data, unsigned int length);
	virtual ~PdcchDumpRecord();

	virtual std::string to_string() { return "not implemented"; }
	bool equals(PdcchDumpRecord other);
protected:
	unsigned int record_version;
	char* data;
	unsigned int length;
};

#endif /* PDCCHDUMPRECORD_H_ */
