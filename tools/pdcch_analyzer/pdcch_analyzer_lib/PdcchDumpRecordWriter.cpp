/*
 PdcchDumpRecordWriter.cpp
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#include "PdcchDumpRecordWriter.h"

#include <iostream>
#include <fstream>

using namespace std;

PdcchDumpRecordWriter::PdcchDumpRecordWriter(string filename,
		bool compress_output) :
		split_size(0), bytes_written(0), cur_split_file(0), base_filename(filename) {
	string gz_options = compress_output ? "wb" : "wbT";
	compressed_file = gzopen((base_filename + "0").c_str(), gz_options.c_str());
	if (compressed_file != 0) {
		cout << "writing file: " << filename << endl;
	}
}

PdcchDumpRecordWriter::~PdcchDumpRecordWriter() {
	gzclose(compressed_file);
}

void PdcchDumpRecordWriter::write_record(PdcchDumpRecord* record) {
	unsigned int total_record_size = record->get_data_length() + 8;
	if (split_size != 0) {
		if (bytes_written + total_record_size > split_size * 1000 * 1000) {
			cur_split_file++;
			gzclose(compressed_file);
			compressed_file = gzopen(
					(base_filename + to_string(cur_split_file)).c_str(), "wb");
			cout << "writing file: " << base_filename + to_string(cur_split_file)
					<< endl;
			bytes_written = 0;
		}
	}

	uint32_t header_data[2];
	header_data[0] = total_record_size;
	header_data[1] = (record->get_record_type() << 16)
			| (record->get_record_version() & 0xFFFF);

	gzwrite(compressed_file, (char*) header_data, 8);
	gzwrite(compressed_file, record->get_data(), record->get_data_length());

	bytes_written += total_record_size;
}

