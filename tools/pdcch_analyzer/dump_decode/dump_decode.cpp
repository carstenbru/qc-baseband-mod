/*
 dump_decode.cpp

 Carsten Bruns (carst.bruns@gmx.de)
 */

#include <stdio.h>
#include <string>
#include <iostream>
#include <string.h>

#include "PdcchDumpRecordReader.h"
#include "PdcchDumpRecordWriter.h"

using namespace std;

//size (in MB) after which a new output file is created
#define OUTPUT_FILE_SPLIT_SIZE 2000

bool process_record(PdcchDumpRecord* record, void* arg) {
	cout << "next record: " << record->to_string() << endl;
	PdcchDumpRecordWriter* pdcch_dump_record_writer = (PdcchDumpRecordWriter*) arg;
	if (record->get_record_type() != PDCCH_LLR_BUFFER_RECORD) {
		pdcch_dump_record_writer->write_record(record);
	}

	return false;
}

int main(int argc, char* argv[]) {
	if (argc < 4) {
		cout << "Usage:\n" << argv[0]
				<< " DUMP_FILE_PATH DUMP_NAME [OUTPUT_PATH] OUTPUT_BASE_NAME" << endl;
		cout << "\tDUMP_FILE_PATH: path to the input dump file" << endl;
		cout << "\tDUMP_NAME: name of the input file (without .binX file extension)" << endl;
		cout << "\tOUTPUT_PATH (optional): path to the output dump file, if not provided, DUMP_FILE_PATH is used" << endl;
		cout << "\tOUTPUT_BASE_NAME: desired name of the output file (without .binX file extension)" << endl;
		return 0;
	}

	string in_filename = argv[1];
	in_filename.append("/");
	in_filename.append(argv[2]);
	in_filename.append(".bin");

	string out_filename = (argc > 4) ? argv[3] : argv[1];
	out_filename.append("/");
	out_filename.append((argc > 4) ? argv[4] : argv[3]);
	out_filename.append(".bin");

	/* create writer for output of processed records */
	PdcchDumpRecordWriter pdcch_dump_record_writer(out_filename);
	pdcch_dump_record_writer.set_split_size(OUTPUT_FILE_SPLIT_SIZE);

	/* create record reader, activate decoder and register callbacks */
	PdcchDumpRecordReader pdcch_dump_record_reader(in_filename, true);
	pdcch_dump_record_reader.register_callback(PDCCH_ALL_RECORDS,
			(record_callback_t) &process_record, &pdcch_dump_record_writer);

	/* let's go! */
	pdcch_dump_record_reader.read_all_records();

	cout << "finished processing dump" << endl;

	return 0;
}

