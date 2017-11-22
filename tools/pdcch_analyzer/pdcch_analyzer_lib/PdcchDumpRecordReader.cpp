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

PdcchDumpRecordReader::PdcchDumpRecordReader(string filename,
		bool decode_llr_records) :
		base_filename(filename), cur_split_file(-2), file_stream(base_filename,
				ios::in | ios::binary), sfn_iteration(0), last_sfn(0) {
	if (file_stream.is_open()) {
		cout << "reading file: " << filename << endl;
	} else {
		cur_split_file = -1;
	}
	if (decode_llr_records) {
		pdcchDecoder = new PdcchDecoder();
		pdcchDecoder->connect_to_record_reader(*this);
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
	file_stream.close();
}

PdcchDumpRecord* PdcchDumpRecordReader::read_next_record() {

	uint32_t header_data[2];
	if (!(file_stream.read((char*) header_data, 8))) {
		if (cur_split_file == -2) {  //single file mode
			return 0;
		} else {  //split file mode
			cur_split_file++;  //try if a next file exists
			if (file_stream.is_open()) {
				file_stream.close();
			}
			file_stream.open(base_filename + to_string(cur_split_file),
					ios::in | ios::binary);
			if (!(file_stream.read((char*) header_data, 8))) {
				return 0;
			}
			cout << "reading file: " << base_filename << cur_split_file << endl;
		}
	}

	unsigned int record_type = header_data[1] >> 16;
	unsigned int record_version = header_data[1] & 0xFFFF;
	unsigned int record_payload_length = header_data[0] - 8;

	char* payload_data = new char[record_payload_length];
	file_stream.read(payload_data, record_payload_length);

	PdcchDumpRecord* record = 0;
	unsigned int cur_sfn = last_sfn;
	switch (record_type) {
	case (PDCCH_LLR_BUFFER_RECORD): {
		PdcchDataRecord* pdcchLlrBufferRecord = new PdcchLlrBufferRecord(
				record_version, payload_data, record_payload_length);
		cur_sfn = pdcchLlrBufferRecord->get_sfn();
		record = pdcchLlrBufferRecord;
		break;
	}
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
	case (PDCCH_DCI_RECORD): {
		PdcchDataRecord* pdcchDciRecord = new PdcchDciRecord(record_version,
				payload_data, record_payload_length);
		cur_sfn = pdcchDciRecord->get_sfn();
		record = pdcchDciRecord;
		break;
	}
	default:
		cerr << "unsupported record in dump, type: " << record_type << endl;
		return read_next_record();
	}
	if (record == 0) {
		delete[] payload_data;
	}

	if (last_sfn > cur_sfn) {
		sfn_iteration++;
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
	if (pdcchDecoder != 0) {
		if ((type == PDCCH_DCI_RECORD) || (type == PDCCH_ALL_RECORDS)) {
			pdcchDecoder->register_callback((decoder_callback_t) callback, arg);
		}
	}
}

long PdcchDumpRecordReader::ms_since_last_time_record(
		PdcchDataRecord* data_record) {
	long diff_ms = (get_last_record_sfn_iteration(PDCCH_TIME_RECORD)
			- (long) get_sfn_iteration()) * 10240;
	diff_ms += (get_last_record_sfn(PDCCH_TIME_RECORD)
			- (long) data_record->get_sfn()) * 10;
}

string PdcchDumpRecordReader::get_time_string(PdcchDataRecord* data_record) {
	return ((PdcchTimeRecord*) get_last_record(PDCCH_TIME_RECORD))->getTimeString(
			ms_since_last_time_record(data_record));
}
