/*
 FrameAverageWriter.cpp
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#include "FrameAverageWriter.h"

using namespace std;

FrameAverageWriter::FrameAverageWriter(string filename) :
		ResultWriter(filename), last_sfn(0) {
	set_write_sfn_iteration(true);
	set_write_sfn(true);
}

FrameAverageWriter::~FrameAverageWriter() {
}

bool FrameAverageWriter::write_data_condition(PdcchDciRecord* dci_record,
		PdcchDumpRecordReader* pdcch_dump_record_reader) {
	return (dci_record->get_subframe() == 9);
}
