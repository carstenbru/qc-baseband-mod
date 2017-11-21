/*
 PdcchDumpRecord.h
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#ifndef PDCCHDUMPRECORD_H_
#define PDCCHDUMPRECORD_H_

#include <string>

enum record_type_enum {
	PDCCH_LLR_BUFFER_RECORD = 0,
	PDCCH_GPS_RECORD = 1,
	PDCCH_TIME_RECORD = 2,
	PDCCH_MAIN_CELL_INFO_RECORD = 3,
	PDCCH_ADD_CELL_INFO_RECORD = 4,
	PDCCH_DCI_RECORD = 5,
	PDCCH_ALL_RECORDS,
	PDCCH_RECORD_MAX
};

class PdcchDumpRecord {
public:
	PdcchDumpRecord(unsigned int record_version, char* data, unsigned int length);
	virtual ~PdcchDumpRecord();

	virtual std::string to_string() {
		return "not implemented";
	}
	bool equals(PdcchDumpRecord& other);

	virtual char* get_data() {
		return data;
	}
	virtual int get_data_length() {
		return length;
	}
	unsigned int get_record_version() {
		return record_version;
	}
	virtual int get_record_type() = 0;

	unsigned int record_version;
	char* data;
protected:
	unsigned int length;
};

#endif /* PDCCHDUMPRECORD_H_ */
