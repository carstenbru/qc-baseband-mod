/*
 SfnIterationAverageWriter.cpp
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#include "SfnIterationAverageWriter.h"

using namespace std;

SfnIterationAverageWriter::SfnIterationAverageWriter(string filename) :
		ResultWriter(filename) {
	set_write_timestamps(true);
	set_write_sfn_iteration(true);
}

SfnIterationAverageWriter::~SfnIterationAverageWriter() {
}

bool SfnIterationAverageWriter::write_data_condition(PdcchDciRecord* dci_record,
		PdcchDumpRecordReader* pdcch_dump_record_reader) {
	if (pdcch_dump_record_reader->get_last_sfn() > dci_record->get_sfn()) {
		if (get_num_subframes() > 100) {
			return true;
		} else {
			clear_samples();
			return false;
		}
	}
	return false;
}
