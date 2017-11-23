/*
 ResultWriter.cpp
 
 Carsten Bruns (carst.bruns@gmx.de)
 */
#include "ResultWriter.h"
#include "records/PdcchTimeRecord.h"

using namespace std;

ResultWriter::ResultWriter(string filename) :
		file_stream(filename), first_write(true), num_samples(0), write_timestamps(
				false), write_sfn_iteration(false), write_sfn(false) {
}

ResultWriter::~ResultWriter() {
	file_stream.close();
}

void ResultWriter::add_analyzer(SubframeAnalyzer* subframe_analyzer) {
	analyzers.push_back(subframe_analyzer);
	values.resize(values.size() + subframe_analyzer->get_value_names().size());
}

void ResultWriter::write_file_header(
		PdcchDumpRecordReader* pdcch_dump_record_reader) {
	PdcchTimeRecord* pdcch_time_record =
			(PdcchTimeRecord*) pdcch_dump_record_reader->get_last_record(
					PDCCH_TIME_RECORD);

	file_stream << "#" << pdcch_time_record->getTimeString() << "\n";

	if (write_timestamps) {
		file_stream << "Timestamp" << "\t";
	}
	if (write_sfn_iteration) {
		file_stream << "SFN iteration" << "\t";
	}
	if (write_sfn) {
		file_stream << "SFN" << "\t";
	}

	for (list<SubframeAnalyzer*>::iterator it = analyzers.begin();
			it != analyzers.end(); it++) {
		vector<string> analyzer_value_names = (*it)->get_value_names();
		for (unsigned int i = 0; i < analyzer_value_names.size(); i++) {
			file_stream << analyzer_value_names[i] << "\t";
		}

	}
	file_stream << "\n";
}

void ResultWriter::clear_samples() {
	for (unsigned int i = 0; i < values.size(); i++) {
		values[i] = 0;
	}
	num_samples = 0;
}

void ResultWriter::new_results(PdcchDciRecord* dci_record,
		PdcchDumpRecordReader* pdcch_dump_record_reader) {
	unsigned int val_pos = 0;
	for (list<SubframeAnalyzer*>::iterator it = analyzers.begin();
			it != analyzers.end(); it++) {
		vector<double> analyzer_values = (*it)->get_values();
		for (unsigned int i = 0; i < analyzer_values.size(); i++) {
			values[val_pos++] += analyzer_values[i];
		}
	}
	num_samples++;

	if (pdcch_dump_record_reader->get_last_record(PDCCH_TIME_RECORD) != 0) {  // only write if we have seen a timestamp
		if (write_data_condition(dci_record, pdcch_dump_record_reader)) {
			if (first_write) {
				write_file_header(pdcch_dump_record_reader);
				first_write = false;
			}

			if (write_timestamps) {
				file_stream << pdcch_dump_record_reader->get_time_string(dci_record)
						<< "\t";
			}
			if (write_sfn_iteration) {
				file_stream << pdcch_dump_record_reader->get_sfn_iteration() << "\t";
			}
			if (write_sfn) {
				file_stream << dci_record->get_sfn() << "\t";
			}
			for (unsigned int i = 0; i < values.size(); i++) {
				file_stream << (values[i] / (double) num_samples) << "\t";
			}
			file_stream << "\n";
			clear_samples();
		}
	}
}
