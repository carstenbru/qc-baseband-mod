/*
 PdcchDumpRecordReader.cpp
 
 Carsten Bruns (carst.bruns@gmx.de)
 */

#include "PdcchDumpRecordReader.h"
#include "records/PdcchDataRecord.h"
#include "records/PdcchGpsRecord.h"
#include "records/PdcchTimeRecord.h"
#include "records/PdcchMainCellInfoRecord.h"
#include "records/PdcchAddCellInfoRecord.h"

#include <iostream>

using namespace std;

PdcchDumpRecordReader::PdcchDumpRecordReader(string filename) :
		file_stream(filename, ios::in | ios::binary) {
}

PdcchDumpRecordReader::~PdcchDumpRecordReader() {
	file_stream.close();
}

PdcchDumpRecord* PdcchDumpRecordReader::read_next_record(
		bool& callback_needs_record) {
	uint32_t header_data[2];
	if (!(file_stream.read((char*) header_data, 8))) {
		return 0;
	}

	unsigned int record_type = header_data[1] >> 16;
	unsigned int record_version = header_data[1] & 0xFFFF;
	unsigned int record_payload_length = header_data[0] - 8;

	char* payload_data = new char[record_payload_length];
	file_stream.read(payload_data, record_payload_length);

	PdcchDumpRecord* record = 0;
	switch (record_type) {
	case (PDCCH_DATA_RECORD):
		record = new PdcchDataRecord(record_version, payload_data,
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
	default:
		cerr << "unsupported record in dump, type: " << record_type << endl;
		return read_next_record(callback_needs_record);
	}
	if (record == 0) {
		delete[] payload_data;
	}

	for (unsigned int i = 0; i < callbacks[PDCCH_ALL_RECORDS].size(); i++) {
		callback_needs_record |= callbacks[PDCCH_ALL_RECORDS][i](record,
				callback_args[PDCCH_ALL_RECORDS][i]);
	}
	for (unsigned int i = 0; i < callbacks[record_type].size(); i++) {
		callback_needs_record |= callbacks[record_type][i](record, callback_args[record_type][i]);
	}

	return record;
}

void PdcchDumpRecordReader::read_all_records() {
	PdcchDumpRecord* record;
	bool callback_needs_record = false;
	while ((record = read_next_record(callback_needs_record)) != 0) {
		//actual processing is done in callbacks
		if (!callback_needs_record) {
			delete record;
		}
		callback_needs_record = false;
	}
}

void PdcchDumpRecordReader::register_callback(record_type_enum type,
		record_callback_t callback, void* arg) {
	if (type < PDCCH_RECORD_MAX) {
		callbacks[type].push_back(callback);
		callback_args[type].push_back(arg);
	}
}
