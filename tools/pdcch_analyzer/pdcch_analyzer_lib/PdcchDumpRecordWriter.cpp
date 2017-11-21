/*
 PdcchDumpRecordWriter.cpp
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#include "PdcchDumpRecordWriter.h"

#include <iostream>

using namespace std;

PdcchDumpRecordWriter::PdcchDumpRecordWriter(string filename) :
		split_size(0), bytes_written(0), cur_split_file(0), base_filename(filename), file_stream(
				base_filename + "0", ios::out | ios::binary) {
	if (file_stream.is_open()) {
		cout << "writing file: " << filename << endl;
	}
}

PdcchDumpRecordWriter::~PdcchDumpRecordWriter() {
	file_stream.close();
}

void PdcchDumpRecordWriter::write_record(PdcchDumpRecord* record) {
	unsigned int total_record_size = record->get_data_length() + 8;
	if (split_size != 0) {
		if (bytes_written + total_record_size > split_size * 1000 * 1000) {
			file_stream.close();
			cur_split_file++;
			file_stream.open(base_filename + to_string(cur_split_file),
					ios::out | ios::binary);
			cout << "writing file: " << base_filename + to_string(cur_split_file) << endl;
			bytes_written = 0;
		}
	}

	uint32_t header_data[2];
	header_data[0] = total_record_size;
	header_data[1] = (record->get_record_type() << 16)
			| (record->get_record_version() & 0xFFFF);

	file_stream.write((char*) header_data, 8);
	file_stream.write(record->get_data(), record->get_data_length());

	bytes_written += total_record_size;
}

