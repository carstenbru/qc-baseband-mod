/*
 PdcchDumpRecordReader.cpp
 
 Carsten Bruns (carst.bruns@gmx.de)
 */

#include "PdcchDumpRecordReader.h"
#include "records/PdcchLlrBufferRecord.h"
#include "records/PdcchGpsRecord.h"
#include "records/PdcchTimeRecord.h"
#include "records/PdcchMainCellInfoRecord.h"
#include "records/PdcchAddCellInfoRecord.h"
#include "records/PdcchDciRecord.h"

#include <iostream>

using namespace std;

bool new_record_callback(PdcchDumpRecord* record, void* arg) {
	PdcchDumpRecordReader* pdcch_dump_record_reader = (PdcchDumpRecordReader*) arg;
	pdcch_dump_record_reader->new_record(record, false);

	return true;  //we take the ownership for the record
}

PdcchDumpRecordReader::PdcchDumpRecordReader(string filename,
		bool decode_llr_records) :
		base_filename(filename), cur_split_file(-2), sfn_iteration(0), last_sfn(0) {
	input_file = gzopen(base_filename.c_str(), "rb");
	if (input_file != 0) {
		cout << "reading file: " << filename << endl;
	} else {
		cur_split_file = -1;
	}
	if (decode_llr_records) {
		pdcchDecoder = new PdcchDecoder();
		pdcchDecoder->connect_to_record_reader(*this);
		pdcchDecoder->register_callback((decoder_callback_t) new_record_callback,
				(void*) this);
	} else {
		pdcchDecoder = 0;
	}
	for (unsigned int i = 0; i < (PDCCH_RECORD_MAX - 1); i++) {
		last_records[i] = 0;
		last_record_sfn_iteration[i] = 0;
		last_record_sfn[i] = 0;
	}
}

PdcchDumpRecordReader::~PdcchDumpRecordReader() {
	gzclose(input_file);
}

void PdcchDumpRecordReader::new_record(PdcchDumpRecord* record,
		bool inc_sfn_it) {
	int record_type = record->get_record_type();

	unsigned int cur_sfn = last_sfn;
	if ((record_type == PDCCH_LLR_BUFFER_RECORD)
			|| (record_type == PDCCH_DCI_RECORD)) {
		cur_sfn = ((PdcchDataRecord*) record)->get_sfn();
		if ((inc_sfn_it) && (last_sfn > cur_sfn)) {
			sfn_iteration++;
		}
	}

	bool callback_needs_record = false;
	for (unsigned int i = 0; i < callbacks[PDCCH_ALL_RECORDS].size(); i++) {
		callback_needs_record |= callbacks[PDCCH_ALL_RECORDS][i](record,
				callback_args[PDCCH_ALL_RECORDS][i]);
	}
	for (unsigned int i = 0; i < callbacks[record_type].size(); i++) {
		callback_needs_record |= callbacks[record_type][i](record,
				callback_args[record_type][i]);
	}

	last_sfn = cur_sfn;

	if (last_records[record_type] != 0) {
		if (!last_records_keep[record_type]) {
			delete last_records[record_type];
		}
	}

	last_records[record_type] = record;
	last_record_sfn_iteration[record_type] = sfn_iteration;
	last_record_sfn[record_type] = last_sfn;
	last_records_keep[record_type] = callback_needs_record;
}

PdcchDumpRecord* PdcchDumpRecordReader::read_next_record() {
	uint32_t header_data[2];

	if (gzread(input_file, (char*) header_data, 8) <= 0) {
		if (cur_split_file == -2) {  //single file mode
			return 0;
		} else {  //split file mode
			cur_split_file++;  //try if a next file exists
			if (input_file != 0) {
				gzclose(input_file);
			}
			input_file = gzopen((base_filename + to_string(cur_split_file)).c_str(),
					"rb");

			if (gzread(input_file, (char*) header_data, 8) <= 0) {
				return 0;
			}
			cout << "reading file: " << base_filename << cur_split_file << endl;
		}
	}

	unsigned int record_type = header_data[1] >> 16;
	unsigned int record_version = header_data[1] & 0xFFFF;
	unsigned int record_payload_length = header_data[0] - 8;

	char* payload_data = new char[record_payload_length];
	gzread(input_file, payload_data, record_payload_length);

	PdcchDumpRecord* record = 0;
	switch (record_type) {
	case (PDCCH_LLR_BUFFER_RECORD):
		record = new PdcchLlrBufferRecord(record_version, payload_data,
				record_payload_length);
		break;
	case (PDCCH_GPS_RECORD):
		record = new PdcchGpsRecord(record_version, payload_data,
				record_payload_length);
		break;
	case (PDCCH_TIME_RECORD):
		record = new PdcchTimeRecord(record_version, payload_data,
				record_payload_length);
		break;
	case (PDCCH_MAIN_CELL_INFO_RECORD):
		record = new PdcchMainCellInfoRecord(record_version, payload_data,
				record_payload_length);
		break;
	case (PDCCH_ADD_CELL_INFO_RECORD):
		record = new PdcchAddCellInfoRecord(record_version, payload_data,
				record_payload_length);
		break;
	case (PDCCH_DCI_RECORD):
		record = new PdcchDciRecord(record_version, payload_data,
				record_payload_length);
		break;
	default:
		cerr << "unsupported record in dump, type: " << record_type << endl;
		return read_next_record();
	}
	if (record == 0) {
		delete[] payload_data;
	}

	new_record(record, true);

	return record;
}

void PdcchDumpRecordReader::read_all_records() {
	PdcchDumpRecord* record;
	while ((record = read_next_record()) != 0) {
		//actual processing is done in callbacks
	}
}

void PdcchDumpRecordReader::register_callback(record_type_enum type,
		record_callback_t callback, void* arg) {
	if (type < PDCCH_RECORD_MAX) {
		callbacks[type].push_back(callback);
		callback_args[type].push_back(arg);
	}
}

long PdcchDumpRecordReader::ms_since_last_time_record(
		PdcchDataRecord* data_record) {
	long diff_ms = (get_sfn_iteration()
			- (long) get_last_record_sfn_iteration(PDCCH_TIME_RECORD)) * 10240;
	diff_ms += (data_record->get_sfn()
			- (long) get_last_record_sfn(PDCCH_TIME_RECORD)) * 10;
	return diff_ms;
}

string PdcchDumpRecordReader::get_time_string(PdcchDataRecord* data_record) {
	return ((PdcchTimeRecord*) get_last_record(PDCCH_TIME_RECORD))->getTimeString(
			ms_since_last_time_record(data_record));
}
