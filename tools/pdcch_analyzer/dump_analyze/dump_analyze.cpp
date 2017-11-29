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
#include <map>

#include <boost/algorithm/string.hpp>

using namespace std;

#define PRINT_RNTI_DISTR (0)

typedef struct {
	PdcchDumpRecordReader* pdcch_dump_record_reader;
	list<SubframeAnalyzer*> analyzers;
	list<ResultWriter*> writers;
} dump_analyze_struct_t;

unsigned int rnti_count[65536];
unsigned int last_ue_rnti = 0;

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

	if (dci_record->get_ue_crnti() != last_ue_rnti) {
		cout << "UE C-RNTI changed at it: "
				<< dump_analyze_struct->pdcch_dump_record_reader->get_sfn_iteration()
				<< " sfn: " << dci_record->get_sfn() << " subframe: "
				<< dci_record->get_subframe() << "; new C-RNTI: "
				<< dci_record->get_ue_crnti() << endl;
		last_ue_rnti = dci_record->get_ue_crnti();
	}
	if (dci_record->get_sfn() == 0) {
		if (dci_record->get_subframe() == 0) {
			if ((dump_analyze_struct->pdcch_dump_record_reader->get_sfn_iteration()
					% 10) == 0) {
				cout << "SFN iteration: "
						<< dump_analyze_struct->pdcch_dump_record_reader->get_sfn_iteration()
						<< endl;
			}
		}
	}

	return false;
}

string remove_quotes(string s) {
	return s.substr(1, s.length() - 2);
}

/**
 * checks if the line defines a SubframeAnalyzer and if yes creates one
 */
SubframeAnalyzer* create_subframe_analyzer(vector<string>& words) {
	if (words[0].compare("DataRateAnalyzer") == 0) {
		return new DataRateAnalyzer();
	} else if (words[0].compare("PrbCountAnalyzer") == 0) {
		return new PrbCountAnalyzer();
	} else if (words[0].compare("DlMcsAnalyzer") == 0) {
		return new DlMcsAnalyzer();
	} else if (words[0].compare("UlMcsAnalyzer") == 0) {
		return new UlMcsAnalyzer();
	}
	return 0;
}

/**
 * checks if the line defines a ResultWriter and if yes creates one
 */
ResultWriter* create_result_writer(vector<string>& words, string dataset) {
	if (words.size() < 2) {
		return 0;
	}
	string fname_format = remove_quotes(words[1]);

	char* filename = new char[512];

	if (words[0].compare("FrameAverageWriter") == 0) {
		snprintf(filename, 512, fname_format.c_str(), dataset.c_str());
		return new FrameAverageWriter(filename);
	} else if (words[0].compare("SfnIterationAverageWriter") == 0) {
		snprintf(filename, 512, fname_format.c_str(), dataset.c_str());
		return new SfnIterationAverageWriter(filename);
	} else if (words[0].compare("TimeAverageWriter") == 0) {
		snprintf(filename, 512, fname_format.c_str(), dataset.c_str());
		int interval = atoi(words[2].c_str());
		return new TimeAverageWriter(filename, interval);
	}
	return 0;
}

/**
 * reads analyzer and writers configuration from file
 */
void read_configuration(string cfg_filename,
		dump_analyze_struct_t& dump_analyze_struct, string dataset) {
	ifstream cfg_file(cfg_filename);
	if (!cfg_file.is_open()) {
		cout << "error: could not open configuration file: " << cfg_filename << endl;
		exit(0);
	}
	cout << "using configuration file: " << cfg_filename << endl;
	map<string, SubframeAnalyzer*> analyzers_map;

	string line;
	bool last_analyzer = false;
	SubframeAnalyzer* last_subframe_analyzer = 0;
	ResultWriter* last_result_writer = 0;
	while (getline(cfg_file, line)) {
		line = line.substr(0, line.find("//"));  //remove comments
		boost::trim_left(line);  //trim
		boost::trim_right(line);
		if (line.empty()) {  //skip empty lines
			continue;
		}
		vector<string> words;  //split into words (space as separator character)
		boost::split(words, line, boost::is_any_of(" "));
		if (SubframeAnalyzer* subframe_analyzer = create_subframe_analyzer(words)) { //check if line defines an analyzer
			analyzers_map[remove_quotes(words[1])] = subframe_analyzer;
			dump_analyze_struct.analyzers.push_back(subframe_analyzer);
			last_subframe_analyzer = subframe_analyzer;
			last_analyzer = true;
		} else if (ResultWriter* result_writer = create_result_writer(words,
				dataset)) { //check if line defines a writer
			dump_analyze_struct.writers.push_back(result_writer);
			last_result_writer = result_writer;
			last_analyzer = false;
		} else {
			if (last_analyzer) { //analyzer parameters
				if (last_subframe_analyzer != 0) {
					last_subframe_analyzer->set_parameter(words[0], words[1]);
				}
			} else { //writer parameters
				if (analyzers_map.find(words[0]) != analyzers_map.end()) { //analyzers list
					dump_analyze_struct.writers.back()->add_analyzer(
							analyzers_map[words[0]]);
				} else {
					if (last_result_writer != 0) { //another parameter for the writer
						last_result_writer->set_parameter(words[0], words[1]);
					}
				}
			}
		}
	}
}

int main(int argc, char* argv[]) {
	if (argc != 4) {
		cout << "Usage: " << argv[0] << " DUMP_FILE_PATH DUMP_NAME CFG_FILE"
				<< endl;
		cout << "\tDUMP_FILE_PATH: path to the input dump file" << endl;
		cout << "\tDUMP_NAME: name of the input file (without .binX file extension)"
				<< endl;
		cout << "\tCFG_FILE: analysis configuration file" << endl;
		return 0;
	}

	string filename = argv[1];
	filename.append("/");
	filename.append(argv[2]);
	filename.append(".bin");
	memset(rnti_count, 0, sizeof(unsigned int) * 65536);

	/* read analyzer and writer configuration */
	dump_analyze_struct_t dump_analyze_struct;
	read_configuration(argv[3], dump_analyze_struct, argv[2]);

	/* create record reader, activate decoder and register callbacks */
	PdcchDumpRecordReader pdcch_dump_record_reader(filename, true);
	dump_analyze_struct.pdcch_dump_record_reader = &pdcch_dump_record_reader;
	pdcch_dump_record_reader.register_callback(PDCCH_DCI_RECORD,
			(record_callback_t) &dci_callback, &dump_analyze_struct);

	/* let's go! */
	pdcch_dump_record_reader.read_all_records();

	cout << "finished processing dump" << endl;

	/* output list of seen RNTIs (if enabled) */
	if (PRINT_RNTI_DISTR) {
		for (unsigned int i = 0; i < 65536; i++) {
			if (rnti_count[i] > 0) {
				cout << "RNTI " << i << ": " << rnti_count[i] << endl;
			}
		}
	}

	/* delete writers in order to close output files clean */
	for (list<ResultWriter*>::iterator it = dump_analyze_struct.writers.begin();
			it != dump_analyze_struct.writers.end(); it++) {
		delete (*it);
	}

	return 0;
}

