/*
 PdcchDumpRecordReader.h
 
 Carsten Bruns (carst.bruns@gmx.de)
 */

#ifndef PDCCHDUMPRECORDREADER_H_
#define PDCCHDUMPRECORDREADER_H_

#include "records/PdcchDumpRecord.h"
#include "pdcch_decoder/PdcchDecoder.h"

#include <string>
#include <fstream>
#include <vector>

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
	 * @param decode_llr_records true to decode PdcchLlrBufferRecords into PdcchDciRecords
	 */
	PdcchDumpRecordReader(std::string filename, bool decode_llr_records);
	virtual ~PdcchDumpRecordReader();

	/**
	 * reads the next record from the input stream
	 *
	 * The memory of the result is freed when the next record with the same type is read by a feature call of this function, if no
	 * callback requested to take ownership (by returning true in the callback method).
	 * The caller should must not free the memory and make a copy in case it needs the result longer.
	 *
	 *  @return next record in stream, 0 if there are no more
	 */
	PdcchDumpRecord* read_next_record();
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

	/**
	 * returns the current "iteration" of the SFN, i.e. how often the SFN overflowed since the beginning of the dump
	 */
	unsigned int get_sfn_iteration() {
		return sfn_iteration;
	}
	/**
	 * gets the last observed SFN value in a PdcchDataRecord
	 *
	 * In a callback for such a record type it will return the previous SFN (not yet updated)
	 */
	unsigned int get_last_sfn() {
		return last_sfn;
	}

	/**
	 * gets the last read record of a certain type, or 0 if such a record was not yet seen
	 *
	 * After a null pointer check, the result can safely be casted to the concrete record type (e.g. PdcchTimeRecord).
	 * In a callback, it will return the previous record (not yet updated)
	 *
	 * @returns the last read record of a certain type, or 0 if such a record was not yet seen
	 */
	PdcchDumpRecord* get_last_record(record_type_enum type) {
		return last_records[type];
	}
	/**
	 * gets the SFN iteration (see get_sfn_iteration()) when the record returned by get_last_record() was seen
	 */
	unsigned int get_last_record_sfn_iteration(record_type_enum type) {
		return last_record_sfn_iteration[type];
	}
	/**
	 * gets the SFN when the record returned by get_last_record() was seen
	 */
	unsigned int get_last_record_sfn(record_type_enum type) {
		return last_record_sfn[type];
	}
	/**
	 * returns the milliseconds since the last time record
	 */
	long ms_since_last_time_record(PdcchDataRecord* data_record);
	/**
	 * returns a string with the current time
	 */
	std::string get_time_string(PdcchDataRecord* data_record);
	void new_record(PdcchDumpRecord* record, bool inc_sfn_it);
private:
	bool call_callback(record_type_enum type, PdcchDumpRecord* record);

	std::string base_filename;
	int cur_split_file;
	std::ifstream file_stream;

	std::vector<record_callback_t> callbacks[PDCCH_RECORD_MAX];
	std::vector<void*> callback_args[PDCCH_RECORD_MAX];

	PdcchDecoder* pdcchDecoder;

	unsigned int sfn_iteration;
	unsigned int last_sfn;

	PdcchDumpRecord* last_records[PDCCH_RECORD_MAX - 1];
	unsigned int last_record_sfn_iteration[PDCCH_RECORD_MAX - 1];
	unsigned int last_record_sfn[PDCCH_RECORD_MAX - 1];
	bool last_records_keep[PDCCH_RECORD_MAX - 1];
};

#endif /* PDCCHDUMPRECORDREADER_H_ */
