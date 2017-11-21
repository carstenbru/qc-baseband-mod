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

	virtual std::string to_string() {
		return "not implemented";
	}
	bool equals(PdcchDumpRecord other);

	virtual char* get_data() {
		return data;
	}
	virtual int get_data_length() {
		return length;
	}

	unsigned int record_version;
	char* data;
protected:
	unsigned int length;
};

#endif /* PDCCHDUMPRECORD_H_ */
