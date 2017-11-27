/*
 dump_analyze.cpp

 Carsten Bruns (carst.bruns@gmx.de)
 */

#include <pdcch_decoder/DciResult.h>
#include <pdcch_decoder/srsLTE/phy_common.h>
#include <pdcch_decoder/srsLTE/ra.h>
#include <PdcchDumpRecordReader.h>
#include <records/PdcchDciRecord.h>
#include <records/PdcchTimeRecord.h>
#include "writers/ResultWriter.h"
#include "writers/FrameAverageWriter.h"
#include "writers/SfnIterationAverageWriter.h"
#include "writers/TimeAverageWriter.h"
#include "analyzers/SubframeAnalyzer.h"
#include "analyzers/PrbCountAnalyzer.h"
#include "analyzers/DataRateAnalyzer.h"
#include "analyzers/DlMcsAnalyzer.h"
#include "analyzers/UlMcsAnalyzer.h"

#include <string.h>
#include <fstream>
#include <iostream>
#include <list>
#include <string>

using namespace std;

typedef struct {
	PdcchDumpRecordReader* pdcch_dump_record_reader;
	list<SubframeAnalyzer*> analyzers;
	list<ResultWriter*> writers;
} dump_analyze_struct_t;

unsigned int rnti_count[65536];

bool dci_callback(PdcchDciRecord* dci_record, void* arg) {
	dump_analyze_struct_t* dump_analyze_struct = (dump_analyze_struct_t*) arg;

	/* run analyzers */
	for (list<SubframeAnalyzer*>::iterator it =
			dump_analyze_struct->analyzers.begin();
			it != dump_analyze_struct->analyzers.end(); it++) {
		(*it)->analyze_subframe(dci_record,
				dump_analyze_struct->pdcch_dump_record_reader);
	}

	/* run writers */
	for (list<ResultWriter*>::iterator it = dump_analyze_struct->writers.begin();
			it != dump_analyze_struct->writers.end(); it++) {
		(*it)->new_results(dci_record,
				dump_analyze_struct->pdcch_dump_record_reader);
	}

	/* information for user: RNTI counters, current position */
	for (list<DciResult*>::iterator it = dci_record->get_dcis()->begin();
			it != dci_record->get_dcis()->end(); it++) {
		DciResult* dci_result = *it;
		rnti_count[dci_result->get_rnti()]++;
	}

	cout << "it: "
			<< dump_analyze_struct->pdcch_dump_record_reader->get_sfn_iteration()
			<< " sfn: " << dci_record->get_sfn() << " subframe: "
			<< dci_record->get_subframe() << endl;

	return false;
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
		cout << "Usage: " << argv[0] << " DUMP_FILE_PATH DUMP_NAME" << endl;
		cout << "\tDUMP_FILE_PATH: path to the input dump file" << endl;
		cout << "\tDUMP_NAME: name of the input file (without .binX file extension)"
				<< endl;
		return 0;
	}

	/* open source and target files */
	string it_sfn_filename = "./it_sfn_";
	it_sfn_filename.append(argv[2]);
	it_sfn_filename.append(".csv");
	ResultWriter* sfn_iteration_average_writer = new SfnIterationAverageWriter(
			it_sfn_filename);
	string sfn_filename = "./sfn_";
	sfn_filename.append(argv[2]);
	sfn_filename.append(".csv");
	ResultWriter* frame_average_writer = new FrameAverageWriter(sfn_filename);
	string time_filename = "./time_";
	time_filename.append(argv[2]);
	time_filename.append(".csv");
	ResultWriter* time_average_writer = new TimeAverageWriter(time_filename, 10*60*1000);

	string filename = argv[1];
	filename.append("/");
	filename.append(argv[2]);
	filename.append(".bin");
	memset(rnti_count, 0, sizeof(unsigned int) * 65536);

	/* create analyzers and add to writers */
	DataRateAnalyzer* data_rate_analyzer = new DataRateAnalyzer();
	PrbCountAnalyzer* prb_count_analyzer = new PrbCountAnalyzer();
	DlMcsAnalyzer* dl_mcs_analyzer = new DlMcsAnalyzer();
	UlMcsAnalyzer* ul_mcs_analyzer = new UlMcsAnalyzer();
	sfn_iteration_average_writer->add_analyzer(data_rate_analyzer);
	sfn_iteration_average_writer->add_analyzer(prb_count_analyzer);
	sfn_iteration_average_writer->add_analyzer(dl_mcs_analyzer);
	sfn_iteration_average_writer->add_analyzer(ul_mcs_analyzer);
	frame_average_writer->add_analyzer(data_rate_analyzer);
	frame_average_writer->add_analyzer(prb_count_analyzer);
	time_average_writer->add_analyzer(dl_mcs_analyzer);
	time_average_writer->add_analyzer(ul_mcs_analyzer);

	/* setup dump_analyze_struct */
	dump_analyze_struct_t dump_analyze_struct;
	dump_analyze_struct.writers.push_back(sfn_iteration_average_writer);
	dump_analyze_struct.writers.push_back(frame_average_writer);
	dump_analyze_struct.writers.push_back(time_average_writer);
	dump_analyze_struct.analyzers.push_back(data_rate_analyzer);
	dump_analyze_struct.analyzers.push_back(prb_count_analyzer);
	dump_analyze_struct.analyzers.push_back(dl_mcs_analyzer);
	dump_analyze_struct.analyzers.push_back(ul_mcs_analyzer);

	/* create record reader, activate decoder and register callbacks */
	PdcchDumpRecordReader pdcch_dump_record_reader(filename, true);
	dump_analyze_struct.pdcch_dump_record_reader = &pdcch_dump_record_reader;
	pdcch_dump_record_reader.register_callback(PDCCH_DCI_RECORD,
			(record_callback_t) &dci_callback, &dump_analyze_struct);

	/* let's go! */
	pdcch_dump_record_reader.read_all_records();

	cout << "finished processing dump" << endl;

	/* output list of seen RNTIs */
	for (unsigned int i = 0; i < 65536; i++) {
		if (rnti_count[i] > 0) {
			cout << "RNTI " << i << ": " << rnti_count[i] << endl;
		}
	}

	/* delete writers in order to close output files clean */
	for (list<ResultWriter*>::iterator it = dump_analyze_struct.writers.begin();
			it != dump_analyze_struct.writers.end(); it++) {
		delete (*it);
	}

	return 0;
}

