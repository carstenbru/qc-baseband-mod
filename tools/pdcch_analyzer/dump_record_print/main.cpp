/*
 main.cpp

 Carsten Bruns (carst.bruns@gmx.de)
 */

#include <stdio.h>
#include <string>
#include <iostream>
#include <string.h>

#include "PdcchDumpRecordReader.h"
#include "pdcch_decoder/PdcchDecoder.h"
#include "records/PdcchDumpRecord.h"
#include "records/PdcchLlrBufferRecord.h"
#include "records/PdcchAddCellInfoRecord.h"
#include "records/PdcchTimeRecord.h"

using namespace std;

//TODO use "better" (faster) viterbi decoder implementation (srsLTE has some fancy ones)
//TODO parallelize decoding code
//TODO keep list of active RNTIs in decoder, increase probability for those in next decodings (or save re-encoding for known RNTIs completly), also increase for reserved RNTIs (e.g. paging) -> maybe simple whitelist like OWL (they keep it for two cycles of sfn)
//TODO remove dependency on PdcchAddCellInfoRecord record type from decoder

//TODO support SPS scheduling

//TODO support for more DCIs and ALL UE specific configurations in UESSS, clean implementation..size gets ambiguous then for different UEs, strategy to distinguish needed
//TODO support for DCI_FORMAT3, DCI_FORMAT3A (just iplink transmit power control)

bool process_record(PdcchDumpRecord* record, void* arg) {
	cout << "next record: " << record->to_string() << endl;

	return false;
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
		cout << "Usage: " << argv[0] << " DUMP_FILE_PATH DUMP_NAME" << endl;
		return 0;
	}

	string filename = argv[1];
	filename.append("/");
	filename.append(argv[2]);
	filename.append(".bin");

	/* create record reader and register callbacks */
	PdcchDumpRecordReader pdcch_dump_record_reader(filename, false);
	pdcch_dump_record_reader.register_callback(PDCCH_ALL_RECORDS,
			(record_callback_t) &process_record, 0);

	/* let's go! */
	pdcch_dump_record_reader.read_all_records();

	cout << "finished processing dump" << endl;

	return 0;
}

