/*
 PdcchDumpRecordReader.h
 
 Carsten Bruns (carst.bruns@gmx.de)
 */

#ifndef PDCCHDUMPRECORDREADER_H_
#define PDCCHDUMPRECORDREADER_H_

#include "records/PdcchDumpRecord.h"

#include <string>
#include <fstream>
#include <vector>

enum record_type_enum {
	PDCCH_LLR_BUFFER_RECORD = 0,
	PDCCH_GPS_RECORD = 1,
	PDCCH_TIME_RECORD = 2,
	PDCCH_MAIN_CELL_INFO_RECORD = 3,
	PDCCH_ADD_CELL_INFO_RECORD = 4,
	PDCCH_ALL_RECORDS,
	PDCCH_RECORD_MAX
};

/**
 * callback function type
 *
 * @param record the record
 * @param arg argument defined by user
 * @return true if the record object is still needed, false if it can be deleted
 */
typedef bool (*record_callback_t)(PdcchDumpRecord* record, void* arg);

class PdcchDumpRecordReader {
public:
	/**
	 * constructor
	 *
	 * @param filename base filename (part number will be appended automatically)
	 */
	PdcchDumpRecordReader(std::string filename);
	virtual ~PdcchDumpRecordReader();

	/**
	 * reads the next record from the input stream
	 *
	 * The caller is responsible to free the memory of the result after using it.
	 *
	 *  @param callback_needs_record true if one of the callback functions still needs the record, false if it can be deleted
	 *  @return next record in stream, 0 if there are no more
	 */
	PdcchDumpRecord* read_next_record(bool& callback_needs_record);
	/**
	 * reads all records in the input stream
	 *
	 * Before calling this function, callbacks should be registered
	 * for the different record types in order to process the data
	 */
	void read_all_records();

	/**
	 * register a callback for a record type
	 *
	 * @param callback callback function
	 * @param arg user argument that will be passed to the callback function
	 */
	void register_callback(record_type_enum type, record_callback_t callback,
			void* arg);
private:
	bool call_callback(record_type_enum type, PdcchDumpRecord* record);

	std::string base_filename;
	int cur_split_file;
	std::ifstream file_stream;

	std::vector<record_callback_t> callbacks[PDCCH_RECORD_MAX];
	std::vector<void*> callback_args[PDCCH_RECORD_MAX];
};

#endif /* PDCCHDUMPRECORDREADER_H_ */
