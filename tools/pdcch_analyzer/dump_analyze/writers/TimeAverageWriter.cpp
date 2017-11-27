/*
 TimeAverageWriter.cpp
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#include "TimeAverageWriter.h"

#include "records/PdcchTimeRecord.h"

using namespace std;

TimeAverageWriter::TimeAverageWriter(string filename,
		unsigned long time_interval) :
		ResultWriter(filename), written_last_time(0) {
	this->time_interval = time_interval;

	set_write_timestamps(true);
}

TimeAverageWriter::~TimeAverageWriter() {
}

bool TimeAverageWriter::write_data_condition(PdcchDciRecord* dci_record,
		PdcchDumpRecordReader* pdcch_dump_record_reader) {
	unsigned long cur_time =
			((PdcchTimeRecord*) pdcch_dump_record_reader->get_last_record(
					PDCCH_TIME_RECORD))->getTime(
					pdcch_dump_record_reader->ms_since_last_time_record(dci_record));

	if (((long) cur_time - (long)written_last_time) >= time_interval) {
		written_last_time = cur_time;
		if (get_num_subframes() > 100) {
			return true;
		} else {
			clear_samples();
			return false;
		}
	}
	return false;
}
